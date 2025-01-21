---
title: The Introduction of PyTorch Compiler
date: 2025-01-20 10:20:00
categories: PyTorch
tags: Compiler
keywords: PyTorch Compiler, PyTorch Graph
---

> 本文目标是单从性能角度来说明 torch.compiler 的相关优化

## GPU 架构介绍

### GPU 软件编程(CUDA)

> CUDA 是 C++ 的一个超集，支持几乎所有的 C++ 语法和特性, 提供了一些额外的关键字和库函数，使得 C++ 代码能够在 NVIDIA GPU 上执行

CUDA 加法的简单实现（二维, 无SMEM, 无同步）

```C++
__global__ void add(int* a, int* b, int* c)
{
    int blockId = blockIdx.x + blockIdx.y * gridDim.x;
    int threadId = threadIdx.x + threadIdx.y * blockDim.x;
    int index = blockId * (blockDim.x * blockDim.y) + threadId;

    c[index] = a[index] + b[index];
}

dim3 grid(2, 2);
dim3 block(2, 2);
add<<<grid, block>>> (a, b, c); 
```

Index 计算解析：

![Block 原理](https://i.mij.rip/2025/01/20/a9dd78cc9255b8964bb74737e6a2c951.png)

**解释**:

- **gridDim** 定义 **x**, **y**, **z** 三个维度分别有多少个 **block**
- **blockDim** 定义 **x**, **y**, **z** 三个维度分别有多少个 **thread**
- **blockIdx** 定义一个 **block** 的位置， **x**, **y**, **z** 三个维度的 **index**
- **threadIdx** 定义一个 **thread** 的位置，**x**, **y**, **z** 三个维度的 **index**

**注意**:

- 用户需要决定如何组织grid以及block(也就是大家经常听到的tilling分块过程)
- 根据分块信息计算出目标索引

### GPU 硬件结构

Volta GV100 GPU:
![Volta GV100 GPU](https://i.miji.bid/2025/01/20/b90832b2d8a24c8b79333de3cf448571.png)

Volta GV100 SM:
![Volta GV100 SM](https://i.miji.bid/2025/01/20/780876979f5bc96f6c4f0eb4b6eecde5.jpeg)

### 软硬件协作

**分块信息如下：**

Grid: 3 \* 2(共6个Blocks)
Block: 32 \* 6(每个Block有192个thread)

**GPU 信息如下：**

SM: 2个，每个SM只有一个 SubCore（描述简单）
DP: 32个（描述简单）
SP: 32个（描述简单）

**执行过程:**

![执行过程](https://i.miji.bid/2025/01/21/f6093f7aeaf3738584b2b202951441e0.png)

**结论**:

- Block 数量:
  * 太少：无法充分利用多SM的并行能力
  * 太多：Block创建上限；SM资源限制；Block切换涉及资源申请、释放，调度代价高；
- Thread 数量：
  * 太少：无法 overlap 访存耗时
  * 太多：Thread创建上限；SM资源限制；SM并发度低

## PyTorch Eager

### PyTorch Eager 整体流程

```Python
# encording: UTF-8

import torch

class Model(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.x = torch.randn(3, 3, device="cuda")

    def forward(self, input):
        x1 = self.x + input
        x2 = self.x - input
        x3 = x1 * x2
        return x3
```

```Python
Disassembly of <code object forward at 0x7f356e4515b0, file "a.py", line 9>:
 10           0 LOAD_FAST                0 (self)
              2 LOAD_ATTR                0 (x)
              4 LOAD_FAST                1 (input)
              6 BINARY_ADD
              8 LOAD_FAST                0 (self)
             10 LOAD_ATTR                0 (x)
             12 LOAD_FAST                1 (input)
             14 BINARY_SUBTRACT
             16 BINARY_MULTIPLY
             18 RETURN_VALUE
```

PyTorch Eager 模式整体执行流程：

![PyTorch Eager模型整体流程](https://i.miji.bid/2025/01/20/c2b249aba9e5e2bb9616bc1aad3f27b7.png)

### PyTorch Eager 性能问题

- 调用链路代价(标号1)
  * 问题：从 Python 字节码解析运行，到 PyTorch 的 C 模块拓展，经过 Dispatcher 机制调度到 C++ 实现的 CUDA 算子，通过算法计算 grid 以及 block，通过 CUDA Runtime 语法糖发射算子，内部调度，驱动，GPU计算出结果；
  * 解决方法：捕获计算图，非Python环境或者算子融合或者图推理
- 内存墙(标号2)
  * 问题：每次计算后都会经历从 HBM2 -> L2 Cache -> L1 Cache -> Register File -> L1 Cache -> L2 Cache -> HBM2 的过程，访存耗时十分显著
  * 解决方法：1) 基于预先规则的算子融合 2) 算子层面的基于内存的细粒度融合, 充分利用缓存能力
- SM 并行度(标号3)
  * 问题：较为固定的 grid 以及 block 的计算方式，无法发挥出 GPU 的并行计算能力
  * 解决方法：根据硬件特性、输入数据大小、数据类型等动态生成 grid 和 block，提供自动调优能力

**补充**:

**访问内存时延**:

| 类型     | 时延(ns) |
| :---:    | :----: |
| 寄存器    | <1     |
| 共享内存  | 5-10    |
| L1 Cache | 7-15   |
| L2 Cache | 30-100 |
| HBM2     | 100-300|

## PyTorch 图模式

### 前世今生

到目前为止(PyTorch 2.6), PyTorch 已经推出了多种关于图模式的方法论, 如下所示：

- jit.trace(C++ trace Dispatch Key + TorchScript IR)
  * 优点：开箱即用
  * 缺点：不支持动态控制流，不支持反向图捕获
  * 特点：1, 2.1
- jit.script(Python AST + TorchScript)
  * 优点：支持动态控制流
  * 缺点：用户体验差，无法识别Python动态语义
  * 特点：1, 2.1
- torch.fx & aot_autograd(分别基于 \__torch_function__ 和 \__torch_dispatcher__(make_fx))
  * 优点：便于模型转换，Dynamo的承载
  * 缺点：用户感知，不支持动态控制流
  * 特点：1, 2.1
- lazytensor(基于 Lazy Dispatch Key + LazyTensor subclass 推迟计算 +  xla编译器)
  * 优点：开箱即用，可以不感知
  * 缺点：动态输入下，频繁反复捕获编译
  * 特点：1, 2.1

官方对于上述几种图捕获的方式进行简单的分类:

![PyTorch 图捕获分类](https://i.miji.bid/2025/01/21/bf31be13684ea036aa17a573399ee9bd.jpeg)

### PyTorch Compiler

为了综合解决上述性能问题，PyTorch 在 2.0 版本发布了一套图模式的船新版本， **torch.compiler**

**Dynamo 技术路线：**

![Dynamo](https://i.miji.bid/2025/01/21/87b68d97d1b73aaf5c9de52a466034ff.png)

**Inductor 技术路线：**

![Inductor](https://i.miji.bid/2025/01/21/723d1db2f4698dcd7db2dcaa8a29167b.jpeg)

torch.compiler 如何解决上述问题？

- **调用链路**：
  * 通过Dynamo捕获静态图（整图或者子图），经由 Inductor 编译后生成一个或者多个triton融合算子，减少整体调用链路
  * 通过 torch.export 导出整图，非Python环境运行

- **内存墙**：
  * 通过基于内存+算子类型的细粒度融合能力，最大程度利用Cache提升访存速度，降低不必要的访存耗时
  * Block内通过triton进行自动优化，自动管理共享内存，同步等

- **SM 并行度**：
  * 根据设备特性（SM数量，SubCore数量，SM、Thread上限，SM 资源等等）以及启发式算法生成多种分块策略
  * 通过 triton 自动调优能力选择性能最优实现

**补充**:

torch.compiler 的优势不仅仅是上述，还有 反向图支持，图优化(图拆分)，重计算等等

## QA

### Triton 是否是必须的

不是， Triton 的存在可以大大降低 Inductor 生成代码的复杂度，使其只需要关注SM层面的相关优化，SM内部的优化交由 Triton 负责，整体复杂度指数级下降
