---
title: PyTorch Operator Register
date: 2023-11-09 10:27:53
categories: PyTorch
tags: Dispatcher
keywords: PyTorch Operator Register
---

```c++
class TorchLibraryInit final {
private:
  using InitFn = void(Library&);
  Library lib_;

public:
  TorchLibraryInit(
      Library::Kind kind,
      InitFn* fn,
      const char* ns,
      c10::optional<c10::DispatchKey> k,
      const char* file,
      uint32_t line)
      : lib_(kind, ns, k, file, line) {
    fn(lib_);
  }
};

Library::Library(Kind kind, std::string ns, c10::optional<c10::DispatchKey> k, const char* file, uint32_t line)
  : kind_(kind)
  , ns_(ns == "_" ? c10::nullopt : c10::make_optional(std::move(ns)))
  , dispatch_key_(k.value_or(CatchAll) == CatchAll ? c10::nullopt : k)
  , file_(file)
  , line_(line)
{
  switch (kind_) {
    case DEF:
      // Only DEFs require library uniqueness; fragments
      // don't register a library
      registrars_.emplace_back(
        c10::Dispatcher::singleton().registerLibrary(
          *ns_, debugString(file_, line_)
        )
      );
      [[fallthrough]];
    case FRAGMENT:
      TORCH_CHECK(
        ns_.has_value(),
        toString(kind_), ": cannot define ", toString(kind_), " with the wildcard namespace _ "
        "(every ", toString(kind_), " defines operators for a distinct namespace!) "
        "Did you mean to use TORCH_LIBRARY_IMPL instead?  "
        ERROR_CONTEXT
      );
      TORCH_INTERNAL_ASSERT(!dispatch_key_.has_value(), ERROR_CONTEXT);
      break;
    case IMPL:
      // Nothing to do, everything is OK
      break;
  }
}

#define TORCH_LIBRARY(ns, m)                                                   \
  static void TORCH_LIBRARY_init_##ns(torch::Library&);                        \
  static const torch::detail::TorchLibraryInit TORCH_LIBRARY_static_init_##ns( \
      torch::Library::DEF,                                                     \
      &TORCH_LIBRARY_init_##ns,                                                \
      #ns,                                                                     \
      c10::nullopt,                                                            \
      __FILE__,                                                                \
      __LINE__);                                                               \
  void TORCH_LIBRARY_init_##ns(torch::Library& m)

// static void TORCH_LIBRARY_init_aten(torch::Library&);
// static const torch::detail::TorchLibraryInit TORCH_LIBRARY_static_init_aten( torch::Library::DEF, &TORCH_LIBRARY_init_aten, "aten", c10::nullopt, "/home/mrl/Git.d/pytorch/pytorch/build/aten/src/ATen/RegisterSchema.cpp", 6);
// void TORCH_LIBRARY_init_aten(torch::Library& m)
TORCH_LIBRARY(aten, m) {
  m.def("_cast_Byte(Tensor self, bool non_blocking=False) -> Tensor", {});
  ...
}

Library& Library::_def(c10::FunctionSchema&& schema, c10::OperatorName* out_name, const std::vector<at::Tag>& tags, _RegisterOrVerify rv) & {
  auto ns_opt = schema.getNamespace();
  if (ns_opt.has_value()) {
    TORCH_CHECK(*ns_opt == *ns_,
      "Explicitly provided namespace (", *ns_opt, ") in schema string "
      "does not match namespace of enclosing ", toString(kind_), " block (", *ns_, ").  "
      "Move this definition to the (unique) TORCH_LIBRARY block corresponding to this namespace "
      "(and consider deleting the namespace from your schema string.)  ",
      ERROR_CONTEXT
    );
  } else {
    bool b = schema.setNamespaceIfNotSet(ns_->c_str());
    TORCH_INTERNAL_ASSERT(b, ERROR_CONTEXT);
  }
  if (out_name) {
    *out_name = schema.operator_name(); // copy!
  }
  switch (rv) {
    case _RegisterOrVerify::REGISTER:
      registrars_.emplace_back(
        c10::Dispatcher::singleton().registerDef(
          std::move(schema),
          debugString(file_, line_),
          tags
        )
      );
      break;
    case _RegisterOrVerify::VERIFY:
      c10::Dispatcher::singleton().waitForDef(schema);
      break;
  }
  return *this;
}

RegistrationHandleRAII Dispatcher::registerDef(FunctionSchema schema, std::string debug, std::vector<at::Tag> tags) {
  // we need a lock to avoid concurrent writes
  std::lock_guard<std::mutex> lock(mutex_);

  OperatorName op_name = schema.operator_name();
  auto op = findOrRegisterName_(op_name);

  op.operatorDef_->op.registerSchema(std::move(schema), std::move(debug), std::move(tags));
  listeners_->callOnOperatorRegistered(op);

  // NB: do not increment the counts until AFTER error checking
  ++op.operatorDef_->def_count;
  ++op.operatorDef_->def_and_impl_count;

  cond_var_.notify_all();

  return RegistrationHandleRAII([this, op, op_name] {
    deregisterDef_(op, op_name);
  });
}
```

```c++
void wrapper_CPU___assert_async(const at::Tensor & self) {
  return at::native::_assert_async_cpu(self);
}

#define TORCH_LIBRARY_IMPL(ns, k, m) _TORCH_LIBRARY_IMPL(ns, k, m, C10_UID)

#define _TORCH_LIBRARY_IMPL(ns, k, m, uid)                                \
  static void C10_CONCATENATE(                                            \
      TORCH_LIBRARY_IMPL_init_##ns##_##k##_, uid)(torch::Library&);       \
  static const torch::detail::TorchLibraryInit C10_CONCATENATE(           \
      TORCH_LIBRARY_IMPL_static_init_##ns##_##k##_, uid)(                 \
      torch::Library::IMPL,                                               \
      (c10::impl::dispatch_key_allowlist_check(c10::DispatchKey::k)       \
           ? &C10_CONCATENATE(TORCH_LIBRARY_IMPL_init_##ns##_##k##_, uid) \
           : [](torch::Library&) -> void {}),                             \
      #ns,                                                                \
      c10::make_optional(c10::DispatchKey::k),                            \
      __FILE__,                                                           \
      __LINE__);                                                          \
  void C10_CONCATENATE(                                                   \
      TORCH_LIBRARY_IMPL_init_##ns##_##k##_, uid)(torch::Library & m)

// static void TORCH_LIBRARY_IMPL_init_aten_CPU_3(torch::Library&);
// static const torch::detail::TorchLibraryInit TORCH_LIBRARY_IMPL_static_init_aten_CPU_3( torch::Library::IMPL, (c10::impl::dispatch_key_allowlist_check(c10::DispatchKey::CPU) ? &TORCH_LIBRARY_IMPL_init_aten_CPU_3 : [](torch::Library&) -> void {}), "aten", c10::make_optional(c10::DispatchKey::CPU), "/home/mrl/Git.d/pytorch/pytorch/build/aten/src/ATen/RegisterCPU.cpp", 31151);
// void TORCH_LIBRARY_IMPL_init_aten_CPU_3(torch::Library & m)
TORCH_LIBRARY_IMPL(aten, CPU, m) {
  m.impl("_assert_async",
  TORCH_FN(wrapper_CPU___assert_async));
  ...
}

#define TORCH_FN(func) TORCH_FN_TYPE(func)()

#define TORCH_FN_TYPE(func)                                           \
  ::c10::CompileTimeFunctionPointer<                                  \
      std::remove_pointer_t<std::remove_reference_t<decltype(func)>>, \
      func>

// FuncType_ = void(const at::Tensor &)
// FuncType = void(const at::Tensor &)
// func_ptr_ = wrapper_CPU___assert_async
template <class FuncType_, FuncType_* func_ptr_>
struct CompileTimeFunctionPointer final {
  static_assert(
      guts::is_function_type<FuncType_>::value,
      "TORCH_FN can only wrap function types.");
  using FuncType = FuncType_;

  static constexpr FuncType* func_ptr() {
    return func_ptr_;
  }
};
```

```c++
// Name = char const*
// Func = CompileTimeFunctionPointer<void(const at::Tensor &), wrapper_CPU___assert_async>
template <typename Name, typename Func>
Library& impl(
    Name name,
    Func&& raw_f,
    _RegisterOrVerify rv = _RegisterOrVerify::REGISTER) & {
    CppFunction f(std::forward<Func>(raw_f));
    return _impl(name, std::move(f), rv);
}


template <class T>
struct is_compile_time_function_pointer : std::false_type {};
template <class FuncType, FuncType* func_ptr>
struct is_compile_time_function_pointer<
    CompileTimeFunctionPointer<FuncType, func_ptr>> : std::true_type {};

// FuncPtr = CompileTimeFunctionPointer<void(const at::Tensor &), wrapper_CPU___assert_async>
template <typename FuncPtr>
explicit CppFunction(
    FuncPtr f,
    std::enable_if_t<
        c10::is_compile_time_function_pointer<FuncPtr>::value,
        std::nullptr_t> = nullptr)
    : func_(c10::KernelFunction::makeFromUnboxedFunction(f)),
    cpp_signature_(
        c10::impl::CppSignature::make<typename FuncPtr::FuncType>()),
    schema_(c10::detail::inferFunctionSchemaFromFunctor<
            typename FuncPtr::FuncType>()),
    debug_() {}

// FuncPtr = CompileTimeFunctionPointer<void(const at::Tensor &), wrapper_CPU___assert_async>
template<class FuncPtr, bool AllowLegacyTypes = false>
inline KernelFunction KernelFunction::makeFromUnboxedFunction(FuncPtr func_ptr) {
    (void)func_ptr; // Suppress unused variable warning
    return makeFromUnboxedFunctor<AllowLegacyTypes, typename impl::WrapFunctionIntoFunctor<FuncPtr>::type>(
        guts::make_unique_base<OperatorKernel, typename impl::WrapFunctionIntoFunctor<FuncPtr>::type>()
    );
}

// FuncPtr = CompileTimeFunctionPointer<void(const at::Tensor &), wrapper_CPU___assert_async>
template<class FuncPtr>
struct WrapFunctionIntoFunctor final {
  using type = detail::WrapFunctionIntoFunctor_<
      FuncPtr,
      typename guts::function_traits<typename FuncPtr::FuncType>::return_type,
      typename guts::function_traits<typename FuncPtr::FuncType>::parameter_types
  >;
};

template <class Func>
struct function_traits {
  static_assert(
      !std::is_same<Func, Func>::value,
      "In function_traits<Func>, Func must be a plain function type.");
};
// Result = void
// Args = const at::Tensor &
// func_type = void(const at::Tensor &)
// return_type = void
// parameter_types = typelist::typelist<const at::Tensor&>
// static constexpr auto number_of_parameters = 1
template <class Result, class... Args>
struct function_traits<Result(Args...)> {
  using func_type = Result(Args...);
  using return_type = Result;
  using parameter_types = typelist::typelist<Args...>;
  static constexpr auto number_of_parameters = sizeof...(Args);
};

template<class FuncPtr, class ReturnType, class ParameterList> class WrapFunctionIntoFunctor_ {};
// FuncPtr = CompileTimeFunctionPointer<void(const at::Tensor &), wrapper_CPU___assert_async>
// ReturnType = void
// Parameters = const at::Tensor &
// FuncPtr::func_ptr() = wrapper_CPU___assert_async
template<class FuncPtr, class ReturnType, class... Parameters>
class WrapFunctionIntoFunctor_<FuncPtr, ReturnType, guts::typelist::typelist<Parameters...>> final : public c10::OperatorKernel {
public:
    C10_ALWAYS_INLINE decltype(auto) operator()(Parameters... args) {
    return (*FuncPtr::func_ptr())(std::forward<Parameters>(args)...);
    }
};

// Base = OperatorKernel
// Child = WrapFunctionIntoFunctor_<CompileTimeFunctionPointer<void(const at::Tensor &), wrapper_CPU___assert_async>, void, typelist::typelist<const at::Tensor &>>
// Args = 
template <typename Base, typename Child, typename... Args>
typename std::enable_if<
    !std::is_array<Base>::value && !std::is_array<Child>::value &&
        std::is_base_of<Base, Child>::value,
    std::unique_ptr<Base>>::type
make_unique_base(Args&&... args) {
  return std::unique_ptr<Base>(new Child(std::forward<Args>(args)...));
}

// AllowLegacyTypes = false
// KernelFunctor = WrapFunctionIntoFunctor_<CompileTimeFunctionPointer<void(const at::Tensor &), wrapper_CPU___assert_async>, void, typelist::typelist<const at::Tensor &>>
template<bool AllowLegacyTypes, class KernelFunctor>
inline KernelFunction KernelFunction::makeFromUnboxedFunctor(std::unique_ptr<OperatorKernel> kernelFunctor) {
    auto* unboxed_fn = &impl::wrap_kernel_functor_unboxed<KernelFunctor>::call;
    void* void_unboxed_fn = reinterpret_cast<void*>(unboxed_fn);
    bool is_symint = fn_has_symint<decltype(unboxed_fn)>::value;
    return KernelFunction(
        std::move(kernelFunctor),
        &impl::make_boxed_from_unboxed_functor<KernelFunctor, AllowLegacyTypes>::call,
        is_symint ? nullptr : void_unboxed_fn,
        is_symint ? void_unboxed_fn : nullptr
    );
}

// KernelFunctor = WrapFunctionIntoFunctor_<CompileTimeFunctionPointer<void(const at::Tensor &), wrapper_CPU___assert_async>, void, typelist::typelist<const at::Tensor &>>
template<class KernelFunctor>
using wrap_kernel_functor_unboxed = wrap_kernel_functor_unboxed_<KernelFunctor, typename guts::infer_function_traits_t<KernelFunctor>::func_type>;

// T = WrapFunctionIntoFunctor_<CompileTimeFunctionPointer<void(const at::Tensor &), wrapper_CPU___assert_async>, void, typelist::typelist<const at::Tensor &>>
template <typename T>
using infer_function_traits_t = typename infer_function_traits<T>::type;

template <typename Functor>
struct infer_function_traits {
  using type = function_traits<
      c10::guts::detail::strip_class_t<decltype(&Functor::operator())>>;
};

// ALL = void(WrapFunctionIntoFunctor::*)(const at::Tensor &)
// Result = void
// Args = const at::Tensor &
template <typename Result, typename... Args>
struct infer_function_traits<Result (*)(Args...)> {
  using type = function_traits<Result(Args...)>;
};

template <typename Result, typename... Args>
struct infer_function_traits<Result(Args...)> {
  using type = function_traits<Result(Args...)>;
};

template<class KernelFunctor, class OpSignature>
struct wrap_kernel_functor_unboxed_ final {};

// KernelFunctor = WrapFunctionIntoFunctor_<CompileTimeFunctionPointer<void(const at::Tensor &), wrapper_CPU___assert_async>, void, typelist::typelist<const at::Tensor &>>
// ReturnType = void
// ParameterTypes = const at::Tensor &
template<class KernelFunctor, class ReturnType, class... ParameterTypes>
struct wrap_kernel_functor_unboxed_<KernelFunctor, ReturnType(ParameterTypes...)> final {
static ReturnType call(OperatorKernel* functor, DispatchKeySet, ParameterTypes... args) {
    KernelFunctor* functor_ = static_cast<KernelFunctor*>(functor);
    return (*functor_)(std::forward<ParameterTypes>(args)...);
}
};

// KernelFunctor = WrapFunctionIntoFunctor_<CompileTimeFunctionPointer<void(const at::Tensor &), wrapper_CPU___assert_async>, void, typelist::typelist<const at::Tensor &>>
// AllowDeprecatedTypes = false
// ReturnType = void
// ArgTypes = 
template<class KernelFunctor, bool AllowDeprecatedTypes>
struct make_boxed_from_unboxed_functor final {
static void call(OperatorKernel* functor, const OperatorHandle&, DispatchKeySet dispatchKeySet, Stack* stack) {
    using ReturnType = typename guts::infer_function_traits_t<KernelFunctor>::return_type;
    using ArgTypes = typename c10::remove_DispatchKeySet_arg_from_func<KernelFunctor>::parameter_types;
    constexpr bool has_outputs = !std::is_same<void, ReturnType>::value;
    constexpr size_t num_inputs = guts::typelist::size<ArgTypes>::value;
    if constexpr (has_outputs) {
    using ReturnType_ = ::std::decay_t<ReturnType>;
    ReturnType_ output = call_functor_with_args_from_stack<KernelFunctor, AllowDeprecatedTypes>(functor, dispatchKeySet, stack);
    torch::jit::drop(*stack, num_inputs);
    push_outputs<ReturnType_, AllowDeprecatedTypes>::call(::std::move(output), stack);
    } else {
    call_functor_with_args_from_stack<KernelFunctor, AllowDeprecatedTypes>(functor, dispatchKeySet, stack);
    torch::jit::drop(*stack, num_inputs);
    }
}
};

// Functor = WrapFunctionIntoFunctor_<CompileTimeFunctionPointer<void(const at::Tensor &), wrapper_CPU___assert_async>, void, typelist::typelist<const at::Tensor &>>
template<class Functor, bool AllowDeprecatedTypes, size_t... ivalue_arg_indices,  typename... ArgTypes>
std::decay_t<typename guts::infer_function_traits_t<Functor>::return_type>
call_functor_with_args_from_stack_(OperatorKernel* functor, DispatchKeySet dispatchKeySet, Stack* stack, std::index_sequence<ivalue_arg_indices...>, guts::typelist::typelist<ArgTypes...>*) {
    (void)(stack);
    return wrap_kernel_functor_unboxed<Functor>::call(functor, dispatchKeySet,
        ivalue_to_arg<typename decay_if_not_tensor<ArgTypes>::type, AllowDeprecatedTypes>::call(
        torch::jit::peek(*stack, ivalue_arg_indices, sizeof...(ivalue_arg_indices))
    )...);
}

// Functor = WrapFunctionIntoFunctor_<CompileTimeFunctionPointer<void(const at::Tensor &), wrapper_CPU___assert_async>, void, typelist::typelist<const at::Tensor &>>
template<class Functor, bool AllowDeprecatedTypes>
std::decay_t<typename guts::infer_function_traits_t<Functor>::return_type>
call_functor_with_args_from_stack(OperatorKernel* functor, DispatchKeySet dispatchKeySet, Stack* stack) {
    using ArgTypes = typename c10::remove_DispatchKeySet_arg_from_func<Functor>::parameter_types;
    constexpr size_t num_ivalue_args = guts::typelist::size<ArgTypes>::value;
    return call_functor_with_args_from_stack_<Functor, AllowDeprecatedTypes>(functor, dispatchKeySet, stack, std::make_index_sequence<num_ivalue_args>(), static_cast<ArgTypes*>(nullptr));
}


template <class FuncType>
using remove_DispatchKeySet_arg_from_func = guts::make_function_traits_t<
    typename guts::infer_function_traits_t<FuncType>::return_type,
    typename std::conditional_t<
        std::is_same<
            DispatchKeySet,
            typename guts::typelist::head_with_default_t<
                void,
                typename guts::infer_function_traits_t<
                    FuncType>::parameter_types>>::value,
        guts::typelist::drop_if_nonempty_t<
            typename guts::infer_function_traits_t<FuncType>::parameter_types,
            1>,
        typename guts::infer_function_traits_t<FuncType>::parameter_types>>;


inline KernelFunction::KernelFunction(std::unique_ptr<OperatorKernel> functor, InternalBoxedKernelFunction* boxed_kernel_func, void* unboxed_kernel_func, void* sym_unboxed_kernel_func = nullptr)
  : boxed_kernel_func_(std::move(functor), boxed_kernel_func)
  , unboxed_kernel_func_(unboxed_kernel_func)
  , sym_unboxed_kernel_func_(sym_unboxed_kernel_func)
{}

inline BoxedKernel::BoxedKernel(std::unique_ptr<OperatorKernel> functor, InternalBoxedKernelFunction* boxed_kernel_func)
: functor_(std::move(functor))
, boxed_kernel_func_(boxed_kernel_func)
{}

template <typename Func>
    Library& fallback(Func&& raw_f) & {
    CppFunction f((std::forward<Func>(raw_f)));
    return _fallback(std::move(f));
}

Library& Library::_fallback(CppFunction&& f) & {
  auto dispatch_key = f.dispatch_key_.has_value() ? f.dispatch_key_ : dispatch_key_;
  for (auto k : c10::getRuntimeDispatchKeySet(*dispatch_key)) {
    auto idx = getDispatchTableIndexForDispatchKey(k);
    if (idx < 0) continue;
    registrars_.emplace_back(
      c10::Dispatcher::singleton().registerFallback(
        k,
        std::move(f.func_),
        debugString(std::move(f.debug_), file_, line_)
      )
    );
  }
  return *this;
}

Library& Library::_impl(const char* name_str, CppFunction&& f, _RegisterOrVerify rv) & {
  at::OperatorName name = _parseNameForLib(name_str);
  auto dispatch_key = f.dispatch_key_.has_value() ? f.dispatch_key_ : dispatch_key_;
  switch (rv) {
    case _RegisterOrVerify::REGISTER:
      registrars_.emplace_back(
        c10::Dispatcher::singleton().registerImpl(
          std::move(name),
          dispatch_key,
          std::move(f.func_),
          std::move(f.cpp_signature_),
          std::move(f.schema_),
          debugString(std::move(f.debug_), file_, line_)
        )
      );
      break;
    case _RegisterOrVerify::VERIFY:
      c10::Dispatcher::singleton().waitForImpl(name, dispatch_key);
      break;
  }
  return *this;

RegistrationHandleRAII Dispatcher::registerImpl(
  OperatorName op_name,
  c10::optional<DispatchKey> dispatch_key,
  KernelFunction kernel,
  c10::optional<impl::CppSignature> cpp_signature,
  std::unique_ptr<FunctionSchema> inferred_function_schema,
  std::string debug
) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto op = findOrRegisterName_(op_name);

  auto handle = op.operatorDef_->op.registerKernel(
    *this,
    dispatch_key,
    std::move(kernel),
    std::move(cpp_signature),
    std::move(inferred_function_schema),
    std::move(debug)
  );

  ++op.operatorDef_->def_and_impl_count;

  cond_var_.notify_all();

  return RegistrationHandleRAII([this, op, op_name, dispatch_key, handle] {
    deregisterImpl_(op, op_name, dispatch_key, handle);
  });
}
```

```c++
#define DECLARE_DISPATCH(fn, name)         \
  struct name : DispatchStub<fn, name> {   \
    name() = default;                      \
    name(const name&) = delete;            \
    name& operator=(const name&) = delete; \
  };                                       \
  extern TORCH_API struct name name

#define DEFINE_DISPATCH(name) struct name name

#define REGISTER_ARCH_DISPATCH(name, arch, fn) \
  template <> name::FnPtr TORCH_API DispatchStub<name::FnPtr, struct name>::arch = fn;

template <typename DispatchStub>
struct RegisterCUDADispatch {
  RegisterCUDADispatch(DispatchStub &stub, typename DispatchStub::FnPtr value) {
    stub.set_cuda_dispatch_ptr(value);
  }
};

#define REGISTER_CUDA_DISPATCH(name, fn) \
  static RegisterCUDADispatch<struct name> name ## __register(name, fn);

#if defined(__CUDACC__)
#define REGISTER_DISPATCH(name, fn) REGISTER_CUDA_DISPATCH(name, fn)
...
#elif defined(CPU_CAPABILITY)
#define REGISTER_DISPATCH(name, fn) REGISTER_ARCH_DISPATCH(name, CPU_CAPABILITY, fn)
#define REGISTER_NO_AVX512_DISPATCH(name)       \
  REGISTER_AVX512_DISPATCH(name, nullptr)
#endif
```

```c++
using elu_fn = void (*)(TensorIteratorBase&, const c10::Scalar&, const c10::Scalar&, const c10::Scalar&);

// struct elu_stub : DispatchStub<elu_fn, elu_stub> {
//   elu_stub() = default;
//   elu_stub(const elu_stub&) = delete;
//   elu_stub& operator=(const elu_stub&) = delete;
// };
// extern struct elu_stub elu_stub
DECLARE_DISPATCH(elu_fn, elu_stub);

// struct elu_stub elu_stub
DEFINE_DISPATCH(elu_stub);

// template <>
// elu_stub::FnPtr DispatchStub<elu_stub::FnPtr, struct name>::DEFAULT = elu_kernel;
REGISTER_DISPATCH(elu_stub, &elu_kernel); // CPU

// static RegisterCUDADispatch<struct elu_stub> elu_stub__register(elu_stub, elu_kernel);
REGISTER_DISPATCH(elu_stub, &elu_kernel); // CUDA
```

```c++
at::Tensor wrapper_CPU_elu(const at::Tensor & self, const at::Scalar & alpha, const at::Scalar & scale, const at::Scalar & input_scale) {
  structured_elu_out_functional op;
  op.meta(self, alpha, scale, input_scale);
  op.impl(self, alpha, scale, input_scale, *op.outputs_[0]);
  return std::move(op.outputs_[0]).take();
}

struct structured_elu_out_functional final : public at::native::structured_elu_out {
    void set_output_strided(
        int64_t output_idx, IntArrayRef sizes, IntArrayRef strides,
        TensorOptions options, DimnameList names
    ) override {
        outputs_[output_idx] = create_out(sizes, strides, options);
        if (!names.empty()) {
          namedinference::propagate_names(*outputs_[output_idx], names);
        }
        // super must happen after, so that downstream can use maybe_get_output
        // to retrieve the output
        at::native::structured_elu_out::set_output_raw_strided(output_idx, sizes, strides, options, names);
    }
    void set_output_raw_strided(
        int64_t output_idx, IntArrayRef sizes, IntArrayRef strides,
        TensorOptions options, DimnameList names
    ) override {
        outputs_[output_idx] = create_out(sizes, strides, options);
        if (!names.empty()) {
          namedinference::propagate_names(*outputs_[output_idx], names);
        }
        // super must happen after, so that downstream can use maybe_get_output
        // to retrieve the output
        at::native::structured_elu_out::set_output_raw_strided(output_idx, sizes, strides, options, names);
    }
    const Tensor& maybe_get_output(int64_t output_idx) override {
      return *outputs_[output_idx];
    }
    std::array<c10::ExclusivelyOwned<Tensor>, 1> outputs_;
};

structured_elu_out::impl (
  const Tensor& self, const Scalar& alpha, const Scalar& scale, const Scalar& input_scale, const Tensor& result
) {
  elu_stub(device_type(), *this, alpha, scale, input_scale);
}

template <typename rT, typename T, typename... Args>
struct DispatchStub<rT (*)(Args...), T> {
  using FnPtr = rT (*) (Args...);

  DispatchStub() = default;
  DispatchStub(const DispatchStub&) = delete;
  DispatchStub& operator=(const DispatchStub&) = delete;

private:
  FnPtr get_call_ptr(DeviceType device_type) {
    return reinterpret_cast<FnPtr>(
      impl.get_call_ptr(device_type
      , reinterpret_cast<void*>(DEFAULT)
#ifdef HAVE_AVX512_CPU_DEFINITION
      , reinterpret_cast<void*>(AVX512)
#endif
#ifdef HAVE_AVX2_CPU_DEFINITION
      , reinterpret_cast<void*>(AVX2)
#endif
#ifdef HAVE_VSX_CPU_DEFINITION
      , reinterpret_cast<void*>(VSX)
#endif
#ifdef HAVE_ZVECTOR_CPU_DEFINITION
      , reinterpret_cast<void*>(ZVECTOR)
#endif
      )
    );
  }

public:
  template <typename... ArgTypes>
  rT operator()(DeviceType device_type, ArgTypes&&... args) {
    FnPtr call_ptr = get_call_ptr(device_type);
    return (*call_ptr)(std::forward<ArgTypes>(args)...);
  }

  void set_cuda_dispatch_ptr(FnPtr fn_ptr) {
    impl.cuda_dispatch_ptr = reinterpret_cast<void*>(fn_ptr);
  }

  void set_hip_dispatch_ptr(FnPtr fn_ptr) {
    impl.hip_dispatch_ptr = reinterpret_cast<void*>(fn_ptr);
  }

  void set_mps_dispatch_ptr(FnPtr fn_ptr) {
    impl.mps_dispatch_ptr = reinterpret_cast<void*>(fn_ptr);
  }

  void set_privateuse1_dispatch_ptr(FnPtr fn_ptr) {
    impl.privateuse1_dispatch_ptr = reinterpret_cast<void*>(fn_ptr);
  }

  static TORCH_API FnPtr DEFAULT;
#ifdef HAVE_AVX512_CPU_DEFINITION
  static TORCH_API FnPtr AVX512;
#endif
#ifdef HAVE_AVX2_CPU_DEFINITION
  static TORCH_API FnPtr AVX2;
#endif
#ifdef HAVE_VSX_CPU_DEFINITION
  static TORCH_API FnPtr VSX;
#endif
#ifdef HAVE_ZVECTOR_CPU_DEFINITION
  static TORCH_API FnPtr ZVECTOR;
#endif
private:
  DispatchStubImpl impl;
};
```
