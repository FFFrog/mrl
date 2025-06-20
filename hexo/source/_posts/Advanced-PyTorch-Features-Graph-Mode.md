---
title: 'Understanding PyTorch Graph Mode'
date: 2025-06-16 19:12:42
categories: PyTorch
tags: Compiler
keywords: PyTorch Compiler, PyTorch Graph
---

## 简介

PyTorch 最大的优点就是其 Eager 模式下的灵活性，赋予了用户开发、调试方面无与伦比的便利；但是 Eager 模式在生产方面却有着巨大的先天缺陷，由此 PyTorch 从 1.0 版本就致力于推出 PyTorch 的图模式，但是由于 Eager 模式的代码并非完备的构图语义，无法直接将其转换成等价的静态图，因此 PyTorch 前前后后进行了多次尝试，直至 PyTorch 2.0 才基本确定了以 torch.compile 为主路径的图模式。

本系列将围绕 torch.compiler 以下几个主要模块进行代码剖析:

- Dynamo
- AOT Autograd
- Inductor

## 图模式的前世今生

在进行上述模块剖析之前，本篇文章先简单总结一下 PyTorch 这么多年在图模式领域的多种尝试，此处借用 PyTorch 核心贡献者 penguinwu 在 PyTorch 开发者论坛中的经典图片，更多详细信息请参考 [The nuances of PyTorch Graph Capture](https://dev-discuss.pytorch.org/t/the-nuances-of-pytorch-graph-capture/501), 这张图片详细的介绍了 PyTorch 当前阶段各种图捕获能力之间的关联和区别。

![PyTorch 图捕获分类](https://i.miji.bid/2025/01/21/bf31be13684ea036aa17a573399ee9bd.jpeg)

下面这张图片从 PyTorch(Python入口) 代码执行的角度，简单的描述了各种图捕获能力的入口位置，下面将结合上面两张图具体分析各种图捕获能力的原理以及优缺点。

![PyTorch 图模式层次图](https://i.miji.bid/2025/06/20/6e4eea857426422bc9df549ea8c75ffa.png)

### torch.jit

PyTorch 在 v1.0.0 版本的时候首次尝试了静态图捕获，实现了一套 JIT 的能力，引入了 TorchScript IR、两种导出 TorchScript IR 的方式以及 IR 优化器和解释器，可以将 function 或者 nn.Module 转换成 ScriptFunction 或者 ScriptModule，并且可以在非 Python 环境下执行。这两种导出方式分别是

- torch.jit.trace
- torch.jit.script

其中，torch.jit.trace 也是 torch.onnx.export 捕获静态图的基础（PyTorch 2.x 之后，torch.onnx.export 已基于 torch.dynamo）

#### torch.jit.trace

`torch.jit.trace` 是在 C++ 层面利用 DispatchKey `Tracer` 进行实现的，简单来说就是传入数据端到端运行一遍待捕获的方法或者模块，借助 `Tracer` DispatchKey 的高优先级在具体算子执行之前捕获静态图，以 `add.Tensor` 算子为例，详细参考下面代码（源码节选）：

```C++
at::Tensor add_Tensor(c10::DispatchKeySet ks, const at::Tensor & self, const at::Tensor & other, const at::Scalar & alpha) {
  torch::jit::Node* node = nullptr;
  std::shared_ptr<jit::tracer::TracingState> tracer_state;
  if (jit::tracer::isTracing()) {
    tracer_state = jit::tracer::getTracingState();
    at::Symbol op_name;
    op_name = c10::Symbol::fromQualString("aten::add");
    node = tracer_state->createNode(op_name, /*num_outputs=*/0);
    jit::tracer::recordSourceLocation(node);
    jit::tracer::addInputs(node, "self", self);
    jit::tracer::addInputs(node, "other", other);
    jit::tracer::addInputs(node, "alpha", alpha);
    tracer_state->insertNode(node);

    jit::tracer::setTracingState(nullptr);
  }
  auto result =at::_ops::add_Tensor::redispatch(ks & c10::DispatchKeySet(c10::DispatchKeySet::FULL_AFTER, c10::DispatchKey::Tracer), self, other, alpha);
  if (tracer_state) {
    jit::tracer::setTracingState(std::move(tracer_state));
    jit::tracer::addOutput(node, result);
  }
  return result;
}
```

在执行真实的 `add.Tensor` 操作之前会首先执行上述代码，在开启 trace 的条件下，会创建一个 op 名字为 `aten:add` 的节点，并设置其输入和输出，将其加入到静态图中，需要注意一点的是，上面捕获出来的算子是 `aten:add` 而不是 `aten:add.Tensor`，是因为其目的主要是为了方便后续 TorchScript IR 经由 ONNX Pass 转成 ONNX 模型。

优点：

- 开箱即用
- 全图捕获
- 可以处理patch，动态修改等场景
- 不需要 fallback 机制

缺点:

- 不支持动态 shape（特化执行）
- 静态化数据控制流（特化执行）
- 不支持反向图捕获

推荐使用场景：

- 代码不涉及动态控制流
- 输入shape，类型固定，一次捕获，反复重放

#### torch.jit.script

`torch.jit.script` 是在 Python 层面获取分析代码的抽象语法树（AST），进行解析分析，在运行之前捕获静态图，它的出现主要就是为了解决 `torch.jit.trace` 无法处理动态shape以及控制流的问题，但是鉴于 Python 语法极其灵活的特性，`torch.jit.script` 无法覆盖所有可能的场景，从而不可避免的出现各种问题。

优点：

- 支持数据控制流
- 支持动态shape

缺点:

- 不支持反向图捕获
- 不支持patch，动态修改等场景
- 仅支持 Python 语法子集，易用性差
- 需要用户修改适配代码

推荐使用场景：

- 不推荐使用，最多作为 `torch.jit.trace` 补充使用

#### 运行

无论是通过 `torch.jit.trace` 还是 `torch.jit.script` 生成的 TorchScript IR，最终都可以通过 JIT 提供的解释器优化后解释执行，相关代码逻辑请参考[这里](https://github.com/pytorch/pytorch/blob/v2.7.1/torch/csrc/jit/runtime/interpreter.cpp)。

### torch.fx

`torch.fx` 是 PyTorch 于 v1.8.0 版本引入，严格意义上来说，`torch.fx` 的提出主要是便利模型量化，方便用户通过代码插入或者修改图节点，完成 Python 代码到 Python 代码的转换，它的功能入口点是在 C++ 算子的 Python 绑定入口，借助的是 PyTorch 拓展机制  `__torch_function__` 的能力以及符号跟踪机制来实现的。

> Note:
> 这里提到的 torch.fx 指代的是 symbolic_trace，而非 PyTorch 2.0 之后新出的 make_fx（AOT Autograd 的根基）。

```C++
static PyObject * THPVariable_add(PyObject* self_, PyObject* args, PyObject* kwargs)
{
  HANDLE_TH_ERRORS
  const Tensor& self = THPVariable_Unpack(self_);
  static PythonArgParser parser({
    "add(Scalar alpha, Tensor other)|deprecated",
    "add(Tensor other, *, Scalar alpha=1)",
  }, /*traceable=*/true);

  ParsedArgs<2> parsed_args;
  auto _r = parser.parse(self_, args, kwargs, parsed_args);
  if(_r.has_torch_function()) {
    return handle_torch_function(_r, self_, args, kwargs, THPVariableClass, "torch.Tensor");
  }
  switch (_r.idx) {
    case 0: {
      // [deprecated] aten::add(Tensor self, Scalar alpha, Tensor other) -> Tensor

      auto dispatch_add = [](const at::Tensor & self, const at::Scalar & alpha, const at::Tensor & other) -> at::Tensor {
        pybind11::gil_scoped_release no_gil;
        return self.add(other, alpha);
      };
      return wrap(dispatch_add(self, _r.scalar(0), _r.tensor(1)));
    }
    case 1: {
      // aten::add.Tensor(Tensor self, Tensor other, *, Scalar alpha=1) -> Tensor

      auto dispatch_add = [](const at::Tensor & self, const at::Tensor & other, const at::Scalar & alpha) -> at::Tensor {
        pybind11::gil_scoped_release no_gil;
        return self.add(other, alpha);
      };
      return wrap(dispatch_add(self, _r.tensor(0), _r.scalar(1)));
    }
  }
  Py_RETURN_NONE;
  END_HANDLE_TH_ERRORS
}
```

简单来说，PyTorch 分析 `function` 或者 `nn.Module` 获取输入信息，将输入信息包装成实现了 `__torch_function__` 的 `Proxy` 类型，将 `Proxy` 类型作为输入参数进行计算；PyTorch 层次的算子在遇到参数的类型带有 `__torch_function__` 属性的时候会触发上述代码中 `handle_torch_function` 的逻辑，最终将调用 `Proxy` 中定义的 `__torch_function__` 方法，该方法中会捕获 PyTorch 层面的算子操作，生成对应的 FX 图节点

优点：

- 支持模型的动态修改、转换，并提供对应工具集
- 支持动态shape

缺点:

- 不支持动态数据流
- 不支持反向图捕获
- 需要用户修改适配代码

推荐使用场景：

- 量化场景

### torch.lazy

`torch.lazy` 是 PyTorch 于 v1.10.0 版本引入的特性，该特性本身并不受广大用户了解，大多数用户了解更多的是基于它实现的 `torch_xla`, 其中心思想是运行过程中，利用 DispatchKey `Lazy` 动态捕获算子（需要实现加速器后端可以支持的所有算子的捕获逻辑），并将其记录到 IR 图中，而不是进行实际的计算；当遇到不支持的算子或者用户显示 `mark_step` 的时候，会触发捕获图的编译操作，生成捕获图对应的高效 kernel，从而加速整个运行过程。

PyTorch 仓库内部实现了一个简单的 [ts_backend](https://github.com/pytorch/pytorch/blob/v2.7.1/torch/_lazy/ts_backend.py)，其原理与 `torch.xla` 基本完全一致，可以用作机制学习参考，可参见相关[教程](https://github.com/pytorch/pytorch/blob/v2.7.1/torch/csrc/lazy/tutorial.md)

```C++
at::Tensor LazyNativeFunctions::add(const at::Tensor &self, const at::Tensor &other, const at::Scalar &alpha)
{

  if (force_eager_fallback(at::aten::add))
  {
    return at::native::call_fallback_fn_symint<&ltc_eager_fallback, ATEN_OP2(add, Tensor)>::call(
        self,
        other,
        alpha);
  }

  TORCH_LAZY_FN_COUNTER("lazy::");
  auto common_device = torch::lazy::GetBackendDevice(self, other);
  TORCH_INTERNAL_ASSERT(common_device);

  LazyTensorPtr lazy_self = torch::lazy::GetLtcTensorOrCreateForWrappedNumber(self, *common_device);
  LazyTensorPtr lazy_other = torch::lazy::GetLtcTensorOrCreateForWrappedNumber(other, *common_device);
  auto node_alpha = torch::lazy::LazyGraphExecutor::Get()->GetIrValueForScalarFromCodegen(alpha, *common_device);
  torch::lazy::NodePtr node = torch::lazy::ReuseNode<AddTensor>(lazy_self->GetIrValue(), lazy_other->GetIrValue(), node_alpha);
  if (!node)
  {
    auto self_meta = to_meta(self);
    auto other_meta = to_meta(other);
    auto out_meta = at::meta::add(self_meta, other_meta, alpha);

    std::vector<torch::lazy::Shape> shapes{torch::lazy::Shape(out_meta.scalar_type(), out_meta.sizes().vec())};
    TORCH_INTERNAL_ASSERT(shapes.size() == 1);
    if (torch::lazy::symbolicShapeEnabled())
    {
      std::vector<torch::jit::IValue> inputs = {self, other, alpha};
      const char *schema_str = "aten::add.Tensor(Tensor self, Tensor other, *, Scalar alpha=1) -> Tensor";
      applySymbolicShapesOnLT(schema_str, inputs, shapes);
    }

    node = torch::lazy::MakeNode<AddTensor>(lazy_self->GetIrValue(), lazy_other->GetIrValue(), node_alpha, std::move(shapes));
    CacheNode(node);
  }

  auto result = torch::lazy::CreateAtenFromLtcTensor(
      torch::lazy::LazyTensor::Create(std::move(node), *common_device));
  return result;
}
```

优点：

- 开箱即用
- 自动完成子图拆分以及子图编译加速

缺点:

- 不支持反向图捕获
- 不支持动态shape（特化执行）
- 静态化数据控制流（特化执行）
- 不支持细粒度子图拆分，捕获子图重复度高，编译成本高

`torch.lazy` 与 `torch.jit.trace` 有着很高的相似程度，它们之间的差异性如下：

- `torch.jit.trace` 每次都会生成特化后的整图（包括数据类型、Shape，以及控制流），由于控制流被特化，所以可能导致同一个导出模型在不同输入的情况下出现结果不对的情况，需要用户了解代码中是否有控制流等存在。
- `torch.lazy` 每次会缓存生成的编译图，当数据类型，Shape以及子图结构没有变化时复用编译图；如果有变化的时候就会重新构图，编译，执行以及缓存，实现了按需多次捕获，多次播放的功能。

### torch.compiler(v2.0.0 引入)

从 PyTorch 1.0.0 到 1.10.0，PyTorch 编译器团队先后尝试了 `torch.jit`、`torch.fx` 以及 `torch.lazy`，但是他们都有各自的优势，以及各自的缺陷，事实证明，需要一种更加高效，易用性更好的方式来解决上面所有的这些问题；

在这里简单罗列下，新的机制需要解决的问题：

- 开箱即用，用户无感(`torch.lazy` 完全支持, `torch.jit.trace` 需要用户了解生成模型的局限性)
- 支持代码 patch，动态注入（源码层面语法解析走不通）（`torch.lazy`、`torch.jit.trace` 以及 `torch.fx` 均支持）
- 动态 shape 支持，避免反复图捕获、图编译代价(`torch.jit.script`， `torch.fx` 支持)
- 数据依赖控制流支持（`torch.jit.script` 支持）
- 支持捕获反向图
- 支持细粒度子图拆分
- 支持更深层次的源码级算子融合，而不是基于规则的融合
- ...

鉴于上述问题， PyTorch compile 图模式横空出世，将 PyTorch 从 1.0 直接跃升成 2.0，下一篇文章将聚焦介绍基于收敛后字节码分析的 `torch dynamo` 模块的原理。
