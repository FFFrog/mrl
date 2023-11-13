---
title: PyTorch Tensor Mechanism
date: 2023-11-09 10:30:48
categories: PyTorch
tags: Tensor
keywords: PyTorch Tensor
---

## The Components of PyTorch

| Module | Detail | Description |
| :---: | :---: | :---: |
| Tensor/Storage | | 视图/|
| nn | init<br>functional | 网络 |
| optimizers | SGD<br>Adagrad<br>RMSprop<br>Adam | 优化器 |
| autograd | | 自动微分 |
| distributed | model parallel<br>data parallel | 分布式 |
| dispatcher | | 分发器 |
| compiler | jit.trace<br>jit.script<br>fx<br>TorchDynamo | 静态图 |
| onnx | | onnx模型转换 |
| hub | | 预训练模型 |
| tensorboard | | 可视化 |

## Directory Structure

* **torch**：import torch 所涉及的 **python** 代码
  * **csrc**：C++源码，pyind11 && python c api相关
  * **csrc/autograd**：梯度自动计算系统的C++实现
  * **autograd**：梯度自动计算系统的Python前端源码
  * **nn**：建立在autograd系统上的神经网络库，包含了深度学习中常用的一些基础神经网络层。
  * **optim**：机器学习中用到的优化算法库
* **aten**：**a tensor library**的缩写，Tensor以及算子实现
  * **src/Aten/core**：aten的核心基础库。目前这个库里面的代码正在逐渐地迁移到c10目录下面
  * **src/Aten/native**：PyTorch的算子库，这个目录下面的*.cc都是stub文件，真正实现在cpu/cuda等目录下
  * **src/Aten/native/cpu**：cpu算子实现
  * **src/Aten/native/cuda**：cuda算子实现
* **c10**：**caffe2 aten** 的缩写，PyTorch的核心库，支持服务端和移动端。
* **tools**：PyTorch中很多相似源码都是脚本通过模板自动生成的，这个文件夹下面就放着自动生成代码的脚本

## Naming Convention

```c
TH* = TorcH
THC* = TorcH Cuda
THCS* = TorcH Cuda Sparse (now defunct)
THCUNN* = TorcH CUda Neural Network (see cunn)
THD* = TorcH Distributed
THNN* = TorcH Neural Network
THS* = TorcH Sparse (now defunct)
THP* = TorcH Python
```

## Python Object

### Samples

```python
import torch

x = torch.Tensor([[1.0], [2.0], [3.0]])
y = torch.Tensor([[2.0], [4.0], [6.0]])
z = x + y

a = 0

type(x)
<class 'torch.Tensor'>
type(type(x))
<class 'torch._C._TensorMeta'>
type(type(type(x)))
<class 'type'>
type(type(type(type(x))))
<class 'type'>

type(a)
<class 'int'>
type(type(a))
<class 'type'>
type(type(type(a)))
<class 'type'>
```

### Structure

```c
typedef struct _object {
    Py_ssize_t ob_refcnt;
    struct _typeobject *ob_type;
} PyObject;

typedef struct {
    PyObject ob_base;
    Py_ssize_t ob_size;
} PyVarObject;
```

```c
typedef struct {
    PyObject ob_base;
    double ob_fval;
} PyFloatObject;
```

![PyFloatObject](https://i.mji.rip/2023/11/13/e68e232289578d6e942dd12f06f9bf4b.png)

```c
typedef struct {
    PyVarObject ob_base;
    PyObject **ob_item;
    Py_ssize_t allocated;
} PyListObject;
```

![PyListObject](https://i.mji.rip/2023/11/13/e7c58d821b844af127cd7b096ea5bcb3.png)

```c
typedef struct _typeobject {
    PyObject_VAR_HEAD
    const char *tp_name; /* For printing, in format "<module>.<name>" */
    Py_ssize_t tp_basicsize, tp_itemsize; /* For allocation */

    /* Methods to implement standard operations */
    destructor tp_dealloc;
    printfunc tp_print;

    getattrfunc tp_getattr;
    setattrfunc tp_setattr;

    // ...
    /* Attribute descriptor and subclassing stuff */
    struct _typeobject *tp_base;

    // ......
} PyTypeObject;
```

![PyObjectRelation](https://i.mji.rip/2023/11/13/920def9cfb668f1e512ceeba1c6afd00.png)

## Pytorch Tensor

```python
class Tensor(torch._C._TensorBase):
    def __deepcopy__(self, memo):
        ...
    def storage(self):
        return self._typed_storage()
    def backward(self, gradient=None, retain_graph=None, create_graph=False, inputs=None):
        torch.autograd.backward(
            self, gradient, retain_graph, create_graph, inputs=inputs
        )
    def register_hook(self, hook):
        return handle
    ...
```

```c++
bool THPVariable_initModule(PyObject* module) {
  THPVariableMetaType.tp_base = &PyType_Type;
  if (PyType_Ready(&THPVariableMetaType) < 0)
    return false;
  Py_INCREF(&THPVariableMetaType);
  PyModule_AddObject(module, "_TensorMeta", (PyObject*)&THPVariableMetaType);

  static std::vector<PyMethodDef> methods;
  THPUtils_addPyMethodDefs(methods, torch::autograd::variable_methods);
  THPUtils_addPyMethodDefs(methods, extra_methods);
  THPVariableType.tp_methods = methods.data();
  if (PyType_Ready(&THPVariableType) < 0)
    return false;
  Py_INCREF(&THPVariableType);
  PyModule_AddObject(module, "_TensorBase", (PyObject*)&THPVariableType);
  torch::autograd::initTorchFunctions(module);
  torch::autograd::initTensorImplConversion(module);
  torch::utils::validate_numpy_for_dlpack_deleter_bug();
  return true;
}

PyMethodDef variable_methods[] = {
  // These magic methods are all implemented on python object to wrap NotImplementedError
  {"__add__", castPyCFunctionWithKeywords(TypeError_to_NotImplemented_<THPVariable_add>), METH_VARARGS | METH_KEYWORDS, NULL},
  {"__radd__", castPyCFunctionWithKeywords(TypeError_to_NotImplemented_<THPVariable_add>), METH_VARARGS | METH_KEYWORDS, NULL},
  {"__iadd__", castPyCFunctionWithKeywords(TypeError_to_NotImplemented_<THPVariable_add_>), METH_VARARGS | METH_KEYWORDS, NULL},
  {"__rmul__", castPyCFunctionWithKeywords(TypeError_to_NotImplemented_<THPVariable_mul>), METH_VARARGS | METH_KEYWORDS, NULL},
  {"__mul__", castPyCFunctionWithKeywords(TypeError_to_NotImplemented_<THPVariable_mul>), METH_VARARGS | METH_KEYWORDS, NULL},
  ...
}

PyTypeObject THPVariableType = {
    PyVarObject_HEAD_INIT(
        &THPVariableMetaType,
        0) "torch._C._TensorBase", /* tp_name */
    sizeof(THPVariable), /* tp_basicsize */
    0, /* tp_itemsize */
    nullptr, /* tp_dealloc */
    0, /* tp_vectorcall_offset */
    nullptr, /* tp_getattr */
    nullptr, /* tp_setattr */
    nullptr, /* tp_reserved */
    nullptr, /* tp_repr */
    nullptr, /* tp_as_number */
    nullptr, /* tp_as_sequence */
    &THPVariable_as_mapping, /* tp_as_mapping */
    nullptr, /* tp_hash  */
    nullptr, /* tp_call */
    nullptr, /* tp_str */
    nullptr, /* tp_getattro */
    nullptr, /* tp_setattro */
    nullptr, /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_HAVE_GC, /* tp_flags */
    nullptr, /* tp_doc */
    // Also set by metaclass
    (traverseproc)THPFunction_traverse, /* tp_traverse */
    (inquiry)THPVariable_clear, /* tp_clear */
    nullptr, /* tp_richcompare */
    0, /* tp_weaklistoffset */
    nullptr, /* tp_iter */
    nullptr, /* tp_iternext */
    nullptr, /* tp_methods */
    nullptr, /* tp_members */
    THPVariable_properties, /* tp_getset */
    nullptr, /* tp_base */
    nullptr, /* tp_dict */
    nullptr, /* tp_descr_get */
    nullptr, /* tp_descr_set */
    0, /* tp_dictoffset */
    nullptr, /* tp_init */
    nullptr, /* tp_alloc */
    THPVariable_pynew, /* tp_new */
};

// Python object that backs torch.autograd.Variable
struct THPVariable {
  PyObject_HEAD;
  // Payload
  c10::MaybeOwned<at::Tensor> cdata;
  // Hooks to be run on backwards pass (corresponds to Python attr
  // '_backwards_hooks', set by 'register_hook')
  PyObject* backward_hooks = nullptr;
};
```

## Creation of Tensor

```c
PyObject* THPVariable_pynew(
    PyTypeObject* type,
    PyObject* args,
    PyObject* kwargs) {
  HANDLE_TH_ERRORS
  TORCH_CHECK(
      type != &THPVariableType,
      "Cannot directly construct _TensorBase; subclass it and then construct that");
  jit::tracer::warn("torch.Tensor", jit::tracer::WARN_CONSTRUCTOR);
  auto tensor = torch::utils::base_tensor_ctor(args, kwargs);
  // WARNING: tensor is NOT guaranteed to be a fresh tensor; e.g., if it was
  // given a raw pointer that will refcount bump
  // NB: base_tensor_ctor can call into dispatched ATen functions (e.g.,
  // alias(), lift_fresh()) which can return Tensor subclasses.  We allow
  // these to be passed on directly.
  return THPVariable_NewWithVar(
      type,
      std::move(tensor),
      c10::impl::PyInterpreterStatus::MAYBE_UNINITIALIZED,
      /*allow_preexisting_pyobj=*/true);
  END_HANDLE_TH_ERRORS
}

Tensor legacy_tensor_generic_ctor_new(
    c10::DispatchKey dispatch_key,
    at::ScalarType scalar_type,
    PyObject* args,
    PyObject* kwargs,
    CtorOrNew ctor_or_new) {
  auto options = dispatchKeyToTensorOptions(dispatch_key);
  static PythonArgParser parser({
      "new(*, Device? device=None)",
      "new(Storage storage)",
      "new(*, int64_t cdata)|hidden",
      // This constructor is no longer legacy, it will also be usable for
      // subclass initialization
      "new(Tensor other)",
      "new(Tensor other, *, Device? device=None)|hidden", // prevent Tensor
                                                          // matching with
                                                          // IntArrayRef,
                                                          // PyObject*
      "new(SymIntArrayRef size, *, Device? device=None)",
      "new(PyObject* data, *, Device? device=None)",
  });

  ...

  ParsedArgs<2> parsed_args;
  auto r = parser.parse(args, kwargs, parsed_args);
  if (r.idx == 0) {
    ...
  } else if (r.idx == 1) {
    ...
  } else if (r.idx == 2) {
    ...
  } else if (r.idx == 3) {
    ...
  } else if (r.idx == 4) {
    ...
  } else if (r.idx == 5) {
    ...
  } else if (r.idx == 6) {
    auto deviceOptional = r.deviceOptional(1);
    ...
    return legacy_new_from_sequence(
        options, scalar_type, deviceOptional, r.pyobject(0));
  }
  throw std::runtime_error("new(): invalid arguments");
}

Tensor legacy_new_from_sequence(
    c10::TensorOptions options,
    at::ScalarType scalar_type,
    c10::optional<Device> device,
    PyObject* data) {
  if (!PySequence_Check(data)) {
    throw TypeError(
        "new(): data must be a sequence (got %s)", Py_TYPE(data)->tp_name);
  }
  return internal_new_from_data(
      options,
      scalar_type,
      device,
      data,
      /*copy_variables=*/false,
      /*copy_numpy=*/false,
      /*type_inference=*/false);
}

Tensor internal_new_from_data(
    c10::TensorOptions options,
    at::ScalarType scalar_type,
    c10::optional<Device> device_opt,
    PyObject* data,
    bool copy_variables,
    bool copy_numpy,
    bool type_inference,
    bool pin_memory = false) {
  
  ...

  auto device = device_opt.has_value() ? *device_opt : options.device();

  auto sizes = compute_sizes(data, scalar_type);

  ScalarType inferred_scalar_type =
      type_inference ? infer_scalar_type(data) : scalar_type;

  Tensor tensor;
  {
    ...
        TensorOptions opts =
            at::initialTensorOptions().dtype(inferred_scalar_type);

        // If the device is Meta, take the shortcut. We don't want to allocate
        // an empty CPU tensor which would break our contract for meta tensors.
        if (device == at::kMeta) {
          return at::empty(sizes, opts.device(device));
        }
        tensor = at::empty(sizes, opts.pinned_memory(pin_memory));
        if (c10::multiply_integers(tensor.sizes()) != 0) {
          recursive_store(
              (char*)tensor.data_ptr(),
              tensor.sizes(),
              tensor.strides(),
              0,
              inferred_scalar_type,
              tensor.dtype().itemsize(),
              data);
        }
    }
    pybind11::gil_scoped_release no_gil;
    maybe_initialize_cuda(device);
    tensor = tensor.to(
        device, inferred_scalar_type, /*non_blocking=*/false, /*copy=*/false);
    ...
  return at::lift_fresh(tensor);
}

// aten::empty.memory_format(SymInt[] size, *, ScalarType? dtype=None, Layout? layout=None, Device? device=None, bool? pin_memory=None, MemoryFormat? memory_format=None) -> Tensor
inline at::Tensor empty(at::IntArrayRef size, at::TensorOptions options={}, c10::optional<at::MemoryFormat> memory_format=c10::nullopt) {
    return at::_ops::empty_memory_format::call(c10::fromIntArrayRefSlow(size), optTypeMetaToScalarType(options.dtype_opt()), options.layout_opt(), options.device_opt(), options.pinned_memory_opt(), c10::impl::check_tensor_options_and_extract_memory_format(options, memory_format));
}

// aten::empty.memory_format(SymInt[] size, *, ScalarType? dtype=None, Layout? layout=None, Device? device=None, bool? pin_memory=None, MemoryFormat? memory_format=None) -> Tensor
at::Tensor empty_memory_format::call(c10::SymIntArrayRef size, c10::optional<at::ScalarType> dtype, c10::optional<at::Layout> layout, c10::optional<at::Device> device, c10::optional<bool> pin_memory, c10::optional<at::MemoryFormat> memory_format) {

    static auto op = create_empty_memory_format_typed_handle();
    return op.call(size, dtype, layout, device, pin_memory, memory_format);
}

static PyObject* THPVariable_NewWithVar(
    PyTypeObject* type,
    Variable _var,
    c10::impl::PyInterpreterStatus status,
    bool allow_preexisting_pyobj) {

  ...

  PyObject* obj = type->tp_alloc(type, 0);
  if (obj) {
    auto v = (THPVariable*)obj;
    // TODO: named constructor to avoid default initialization
    new (&v->cdata) MaybeOwned<Variable>();
    ...
      // Normal codepath
      v->cdata = MaybeOwned<Variable>::owned(std::move(_var));
    ...
  }
  return obj;
}
```

## Opeartion of Tensor

```c
static PyObject * THPVariable_add(PyObject* self_, PyObject* args, PyObject* kwargs)
{
  ...
  const Tensor& self = THPVariable_Unpack(self_);
  static PythonArgParser parser({
    "add(Scalar alpha, Tensor other)|deprecated",
    "add(Tensor other, *, Scalar alpha=1)",
  }, /*traceable=*/true);

  ParsedArgs<2> parsed_args;
  auto _r = parser.parse(self_, args, kwargs, parsed_args);
  ...
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
...
}

// aten::add.Tensor(Tensor self, Tensor other, *, Scalar alpha=1) -> Tensor
inline at::Tensor Tensor::add(const at::Tensor & other, const at::Scalar & alpha) const {
    return at::_ops::add_Tensor::call(const_cast<Tensor&>(*this), other, alpha);

}
// aten::add.Tensor(Tensor self, Tensor other, *, Scalar alpha=1) -> Tensor
at::Tensor add_Tensor::call(const at::Tensor & self, const at::Tensor & other, const at::Scalar & alpha) {
    
    static auto op = create_add_Tensor_typed_handle();
    return op.call(self, other, alpha);
}
```

## Pytorch Dispatcher

### Register

```text
- func: abs.out(Tensor self, *, Tensor(a!) out) -> Tensor(a!)
  device_check: NoCheck   # TensorIterator
  dispatch:
    CPU, CUDA: abs_out
    MPS: abs_out_mps
    SparseCPU, SparseCUDA: abs_sparse_out
    SparseCsrCPU, SparseCsrCUDA: abs_sparse_csr_out
  tags: pointwise
```

```c
TORCH_LIBRARY(aten, m) {
    ...
    m.def("abs.out(Tensor self, *, Tensor(a!) out) -> Tensor(a!)", {at::Tag::pointwise});
    ...
    m.def("add_.Tensor(Tensor(a!) self, Tensor other, *, Scalar alpha=1) -> Tensor(a!)", {at::Tag::pointwise});
    ...
}

TORCH_LIBRARY_IMPL(aten, CPU, m) {
    m.impl("_assert_async", TORCH_FN(wrapper_CPU___assert_async));
    m.impl("_assert_async.msg", TORCH_FN(wrapper_CPU_msg__assert_async));
    m.impl("native_dropout", TORCH_FN(wrapper_CPU__native_dropout));
    m.impl("native_dropout_backward", TORCH_FN(wrapper_CPU__native_dropout_backward));
    m.impl("abs.out", TORCH_FN(wrapper_CPU_out_abs_out));
    m.impl("angle", TORCH_FN(wrapper_CPU__angle));
    ...
}

at::Tensor & wrapper_CPU_out_abs_out(const at::Tensor & self, at::Tensor & out) {
    // No device check
  // DeviceGuard omitted
  return at::native::abs_out(self, out);
}
```

```c
Tensor& abs_out(const Tensor& self, Tensor& result) {
  return unary_op_impl_with_complex_to_float_out(result, self, abs_stub, /*promotes_integer_to_float=*/false);
}
```

### Dispatch

```c
// aten::abs.out(Tensor self, *, Tensor(a!) out) -> Tensor(a!)
at::Tensor & abs_out::call(const at::Tensor & self, at::Tensor & out) {
    
    static auto op = create_abs_out_typed_handle();
    return op.call(self, out);
}

// aten::abs.out(Tensor self, *, Tensor(a!) out) -> Tensor(a!)
static C10_NOINLINE c10::TypedOperatorHandle<abs_out::schema> create_abs_out_typed_handle() {
  return c10::Dispatcher::singleton()
      .findSchemaOrThrow(abs_out::name, abs_out::overload_name)
      .typed<abs_out::schema>();
}

// See [Note: Argument forwarding in the dispatcher] for why Args doesn't use &&
C10_ALWAYS_INLINE Return call(Args... args) const {
return c10::Dispatcher::singleton().call<Return, Args...>(*this, std::forward<Args>(args)...);
}

// See [Note: Argument forwarding in the dispatcher] for why Args doesn't use &&
template<class Return, class... Args>
C10_ALWAYS_INLINE_UNLESS_MOBILE Return Dispatcher::call(const TypedOperatorHandle<Return(Args...)>& op, Args... args) const {
  detail::unused_arg_(args...);  // workaround for a false-positive warning about unused parameters in gcc 5
  auto dispatchKeySet = op.operatorDef_->op.dispatchKeyExtractor()
    .template getDispatchKeySetUnboxed<Args...>(args...);
#ifndef NDEBUG
  DispatchTraceNestingGuard debug_guard;
  if (show_dispatch_trace()) {
      auto nesting_value = dispatch_trace_nesting_value();
      for (int64_t i = 0; i < nesting_value; ++i) std::cerr << " ";
      std::cerr << "[call] op=[" << op.operator_name() << "], key=[" << toString(dispatchKeySet.highestPriorityTypeId()) << "]" << std::endl;
  }
#endif
  const KernelFunction& kernel = op.operatorDef_->op.lookup(dispatchKeySet);
#ifndef PYTORCH_DISABLE_PER_OP_PROFILING
  auto step_callbacks = at::getStepCallbacksUnlessEmpty(at::RecordScope::FUNCTION);
  if (C10_UNLIKELY(step_callbacks.has_value() && op.operatorDef_->op.isObserved())) {
    return callWithDispatchKeySlowPath<Return, Args...>(op, *step_callbacks, dispatchKeySet, kernel, std::forward<Args>(args)...);
  }
#endif  // PYTORCH_DISABLE_PER_OP_PROFILING
  return kernel.template call<Return, Args...>(op, dispatchKeySet, std::forward<Args>(args)...);
}


template<class Return, class... Args>
C10_ALWAYS_INLINE Return KernelFunction::call(const OperatorHandle& opHandle, DispatchKeySet dispatchKeySet, Args... args) const {
    // note: Args above is intentionally not Args&&. We don't want perfect
    // forwarding, which would require Args to be deduced, but instead we
    // want callers to explicitly specify the Args.

    // This should get inlined by compiler
    if (guts::disjunction<has_symint<Args>...>::value) {
      if (sym_unboxed_kernel_func_ != nullptr) {
        auto *functor = boxed_kernel_func_.getFunctor();
        return callUnboxedKernelFunction<Return, Args...>(
            sym_unboxed_kernel_func_, functor, dispatchKeySet, std::forward<Args>(args)...);
      }

      if (unboxed_kernel_func_ != nullptr) {
        auto *functor = boxed_kernel_func_.getFunctor();
        return callUnboxedKernelFunction<Return, typename remove_symint<Args>::type...>(
            unboxed_kernel_func_, functor, dispatchKeySet, unpackSymInt<Args>(args)...);
      }
    } else {
      if (C10_LIKELY(unboxed_kernel_func_ != nullptr)) {
        auto *functor = boxed_kernel_func_.getFunctor();
        return callUnboxedKernelFunction<Return, Args...>(
            unboxed_kernel_func_, functor, dispatchKeySet, std::forward<Args>(args)...);
      }
    }

    return impl::BoxedKernelWrapper<Return(Args...)>::call(
        boxed_kernel_func_,
        opHandle,
        dispatchKeySet,
        std::forward<Args>(args)...
    );
}

template<class Return, class... Args>
inline Return callUnboxedKernelFunction(void* unboxed_kernel_func, OperatorKernel* functor, DispatchKeySet dispatchKeySet, Args&&... args) {
    using ActualSignature = Return (OperatorKernel*, DispatchKeySet, Args...);
    ActualSignature* func = reinterpret_cast<ActualSignature*>(unboxed_kernel_func);
    return (*func)(functor, dispatchKeySet, std::forward<Args>(args)...);
}
```
