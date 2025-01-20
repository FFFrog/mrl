---
title: The Introduction of PyTorch Compiler
date: 2025-01-20 10:20:00
categories: PyTorch
tags: Compiler
keywords: PyTorch Compiler, PyTorch Graph
---

## GPU 架构介绍

### CUDA 编程

```C++
add<<<grid_size, block_size>>> (a, b, c); 

__global__ void add(int* a, int* b, int *c)
{
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    c[index] = a[index] + b[index];
}
```

**解释**:

- **gridDim** 定义 **x**, **y**, **z** 三个维度分别有多少个 **block**
- **blockDim** 定义 **x**, **y**, **z** 三个维度分别有多少个 **thread**
- **blockIdx** 定义一个 **block** 的位置， **x**, **y**, **z** 三个维度的 **index**
- **threadIdx** 定义一个 **thread** 的位置，**x**, **y**, **z** 三个维度的 **index**

![Block 原理](https://i.mij.rip/2025/01/20/a9dd78cc9255b8964bb74737e6a2c951.png)

### GPU 硬件结构

Volta GV100 GPU:
![Volta GV100 GPU](https://i.miji.bid/2025/01/20/b90832b2d8a24c8b79333de3cf448571.png)

Volta GV100 SM:
![Volta GV100 SM](https://i.miji.bid/2025/01/20/780876979f5bc96f6c4f0eb4b6eecde5.jpeg)

### 对应关系

**Block对应关系**:

![Block 对应关系](https://i.miji.bid/2025/01/20/57c8d8cb7ddb13d1959a15ec7acdeccb.png)

**Thread对应关系**:

一个 **thread** 对应 **SubCore** 里面的 **SP/DP**

**访问内存时延**:

| 类型     | 时延(ns) |
| :---:    | :----: |
| 寄存器    | <1     |
| 共享内存  | 5-10    |
| L1 Cache | 7-15   |
| L2 Cache | 30-100 |
| HBM2     | 100-300|

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

![PyTorch Eager模型整体流程](https://i.miji.bid/2025/01/20/c2b249aba9e5e2bb9616bc1aad3f27b7.png)

### PyTorch Eager 性能问题

- SM 并行度
- 内存墙
- 调用代价

## PyTorch 图模式

### 前世今生

- jit.trace
- jit.script
- fx trace
- lazytensor

### PyTorch Compiler

## QA

### Triton 是否是必须的
