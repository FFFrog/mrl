# ONNX Runtime

<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

- [ONNX Runtime](#onnx-runtime)
  - [OpSchema Registry](#opschema-registry)
  - [OpSchema Management](#opschema-management)
  - [Kernel Implementation](#kernel-implementation)
  - [Kernel Registry](#kernel-registry)
  - [Kernel Management](#kernel-management)
  - [Kernel Instantiation](#kernel-instantiation)
  - [Kernel Run](#kernel-run)

<!-- /code_chunk_output -->

## OpSchema Registry

```c++
ONNX_OPERATOR_SET_SCHEMA(
    Add,
    1,
    OpSchema().FillUsing(MathDocGenerator_old("addition")));

OpSchema() : OpSchema("unknown", "unknown", 0) {}

OpSchema& OpSchema::FillUsing(const std::function<void(OpSchema&)>& populator) {
  if (populator) {
    populator(*this);
  }
  return *this;
}

std::function<void(OpSchema&)> MathDocGenerator_old(const char* name) {
  return [=](OpSchema& schema) {
    std::string doc;
    POPULATE_OP_DOC_STR(doc = R"DOC(
Performs element-wise binary {name} (with limited broadcast support).
{broadcast_doc})DOC";
                        ReplaceAll(doc, "{name}", name);
                        ReplaceAll(doc, "{broadcast_doc}", kBroadcastDoc_old););
    schema.SetDoc(doc);
    schema.Attr(
        "broadcast",
        "Pass 1 to enable broadcasting",
        AttributeProto::INT,
        static_cast<int64_t>(0));

    // This attribute was added via AllowConsumed API in OpSchema.
    // After removing the API, we're now using the Attr API to simulate the old
    // definition.
    schema.Attr(
        "consumed_inputs",
        "legacy optimization attribute.",
        AttributeProto::INTS,
        OPTIONAL_VALUE);
    schema.Attr(
        "axis",
        "If set, defines the broadcast dimensions. See doc for details.",
        AttributeProto::INT,
        OPTIONAL_VALUE);
    schema.Input(
        0,
        "A",
        "First operand, should share the type with the second operand.",
        "T");
    schema.Input(
        1,
        "B",
        "Second operand. With broadcasting can be of smaller size than A. "
        "If broadcasting is disabled it should be of the same size.",
        "T");
    schema.Output(0, "C", "Result, has same dimensions and type as A", "T");
    schema.TypeConstraint(
        "T",
        {"tensor(float16)", "tensor(float)", "tensor(double)"},
        "Constrain input and output types to float tensors.");
  };
}

#define ONNX_OPERATOR_SET_SCHEMA(name, ver, impl) \
  ONNX_OPERATOR_SET_SCHEMA_EX(name, Onnx, ONNX_DOMAIN, ver, true, impl)

#define ONNX_OPERATOR_SET_SCHEMA_EX(                                        \
    name, domain, domain_str, ver, dbg_included_in_static_opset, impl)      \
  class ONNX_OPERATOR_SET_SCHEMA_CLASS_NAME(domain, ver, name);             \
  template <>                                                               \
  OpSchema                                                                  \
  GetOpSchema<ONNX_OPERATOR_SET_SCHEMA_CLASS_NAME(domain, ver, name)>() {   \
    return impl.SetName(#name)                                              \
        .SetDomain(domain_str)                                              \
        .SinceVersion(ver)                                                  \
        .SetLocation(__FILE__, __LINE__);                                   \
  }                                                                         \
  size_t dbg_count_check_##name##_##domain##_ver##ver =                     \
      (dbg_included_in_static_opset) ? ONNX_DBG_INCREMENT_COUNT_IN_OPSETS() \
                                     : 0;
```

## OpSchema Management

```c++
inline void RegisterOnnxOperatorSetSchema() {
  RegisterOpSetSchema<OpSet_Onnx_ver1>();
  RegisterOpSetSchema<OpSet_Onnx_ver2>();
  RegisterOpSetSchema<OpSet_Onnx_ver3>();
  RegisterOpSetSchema<OpSet_Onnx_ver4>();
  RegisterOpSetSchema<OpSet_Onnx_ver5>();
  RegisterOpSetSchema<OpSet_Onnx_ver6>();
  RegisterOpSetSchema<OpSet_Onnx_ver7>();
  RegisterOpSetSchema<OpSet_Onnx_ver8>();
  RegisterOpSetSchema<OpSet_Onnx_ver9>();
  RegisterOpSetSchema<OpSet_Onnx_ver10>();
  RegisterOpSetSchema<OpSet_Onnx_ver11>();
  RegisterOpSetSchema<OpSet_Onnx_ver12>();
  RegisterOpSetSchema<OpSet_Onnx_ver13>();
  RegisterOpSetSchema<OpSet_Onnx_ver14>();
  RegisterOpSetSchema<OpSet_Onnx_ver15>();
  RegisterOpSetSchema<OpSet_Onnx_ver16>();
  // 0 means all versions of ONNX schema have been loaded
  OpSchemaRegistry::Instance()->SetLoadedSchemaVersion(0);
}

class OpSet_Onnx_ver1 {
 public:
  static void ForEachSchema(std::function<void(OpSchema&&)> fn) {
    fn(GetOpSchema<ONNX_OPERATOR_SET_SCHEMA_CLASS_NAME(Onnx, 1, Abs)>());
    fn(GetOpSchema<ONNX_OPERATOR_SET_SCHEMA_CLASS_NAME(Onnx, 1, Add)>());
    fn(GetOpSchema<ONNX_OPERATOR_SET_SCHEMA_CLASS_NAME(Onnx, 1, And)>());
    fn(GetOpSchema<ONNX_OPERATOR_SET_SCHEMA_CLASS_NAME(Onnx, 1, ArgMax)>());
    ...
  }
};

template <class T>
void RegisterOpSetSchema(int opset_version_to_load=0) {
  T::ForEachSchema([opset_version_to_load](OpSchema&& schema) {
    RegisterSchema(schema, opset_version_to_load);
  });
};

void RegisterSchema(OpSchema schema, int opset_version_to_load) {
  OpSchemaRegistry::OpSchemaRegisterOnce ONNX_UNUSED registration(schema, opset_version_to_load);
}

class OpSchemaRegisterOnce final {
  public:
  OpSchemaRegisterOnce(OpSchema& op_schema) {
    try {
      op_schema.Finalize();

      auto& m = GetMapWithoutEnsuringRegistration();

      auto& op_name = op_schema.Name();
      auto& op_domain = op_schema.domain();
      auto ver = op_schema.SinceVersion();

      if (m[op_name][op_domain].count(ver)) {
        const auto& schema = m[op_name][op_domain][ver];
        std::stringstream err;
        err << "Trying to register schema with name " << op_name
            << " (domain: " << op_domain << " version: " << ver
            << ") from file " << op_schema.file() << " line "
            << op_schema.line() << ", but it is already registered from file "
            << schema.file() << " line " << schema.line() << std::endl;
        fail_schema(err.str());
      }

      auto ver_range_map = DomainToVersionRange::Instance().Map();
      auto ver_range_it = ver_range_map.find(op_domain);
      if (ver_range_it == ver_range_map.end()) {
        std::stringstream err;
        err << "Trying to register schema with name " << op_name
            << " (domain: " << op_domain << " version: " << ver
            << ") from file " << op_schema.file() << " line "
            << op_schema.line() << ", but it its domain is not"
            << " known by the checker." << std::endl;

        fail_schema(err.str());
      }
      auto lower_bound_incl = ver_range_it->second.first;
      auto upper_bound_incl = ver_range_it->second.second;
      if (!(lower_bound_incl <= ver && upper_bound_incl >= ver)) {
        std::stringstream err;
        err << "Trying to register schema with name " << op_name
            << " (domain: " << op_domain << " version: " << ver
            << ") from file " << op_schema.file() << " line "
            << op_schema.line() << ", but it its version is not "
            << "in the inclusive range [" << lower_bound_incl << ", "
            << upper_bound_incl << "] (usually, this means you "
            << "bumped the operator version but "
            << "forgot to update the version range in DomainToVersionRange "
            << "in onnx/defs/schema.h)." << std::endl;
        fail_schema(err.str());
      }

      m[op_name][op_domain].insert(
          std::pair<int, OpSchema&&>(ver, std::move(op_schema)));

    } catch (const std::exception& e) {
      std::cerr << "Schema error: " << e.what() << std::endl;
    }
  }
};
```

## Kernel Implementation

```c++
template <typename T>
class Add final : public OpKernel {
 public:
  Add(const OpKernelInfo& info) : OpKernel(info) {
  }

  Status Compute(OpKernelContext* context) const override;
};

template <typename T>
Status Add<T>::Compute(OpKernelContext* context) const {
  // BroadcastHelper received as argument may differ from 'helper' when parallelizing within a span
  ProcessBroadcastSpanFuncs funcs{
      [](BroadcastHelper& per_iter_bh) {
        per_iter_bh.OutputEigen<T>() = per_iter_bh.ScalarInput0<T>() + per_iter_bh.EigenInput1<T>().array();
      },
      [](BroadcastHelper& per_iter_bh) {
        per_iter_bh.OutputEigen<T>() = per_iter_bh.EigenInput0<T>().array() + per_iter_bh.ScalarInput1<T>();
      },
      [](BroadcastHelper& per_iter_bh) {
        per_iter_bh.OutputEigen<T>() = per_iter_bh.EigenInput0<T>() + per_iter_bh.EigenInput1<T>();
      }};

  UntypedBroadcastTwo(*context, funcs, 1.0f);
  return Status::OK();
}
```

## Kernel Registry

```c++
REG_ELEMENTWISE_VERSIONED_TYPED_KERNEL(Add, 7, 12, float, Add);

#define REG_ELEMENTWISE_VERSIONED_TYPED_KERNEL(OP_TYPE, VERSION_FROM, VERSION_TO, TYPE, KERNEL_CLASS) \
  ONNX_CPU_OPERATOR_VERSIONED_TYPED_KERNEL(                                                           \
      OP_TYPE,                                                                                        \
      VERSION_FROM, VERSION_TO,                                                                       \
      TYPE,                                                                                           \
      KernelDefBuilder().TypeConstraint("T", DataTypeImpl::GetTensorType<TYPE>()),                    \
      KERNEL_CLASS<TYPE>);

#define ONNX_CPU_OPERATOR_VERSIONED_TYPED_KERNEL(name, startver, endver, type, builder, ...)                         \
  ONNX_OPERATOR_VERSIONED_TYPED_KERNEL_EX(name, kOnnxDomain, startver, endver, type, kCpuExecutionProvider, builder, \
                                          __VA_ARGS__)

#define ONNX_OPERATOR_VERSIONED_TYPED_KERNEL_CLASS_NAME(provider, domain, startver, endver, type, name) \
  provider##_##name##_##domain##_ver##startver##_##endver##_##type

#define ONNX_OPERATOR_VERSIONED_TYPED_KERNEL_EX(name, domain, startver, endver, type, provider, builder, ...)         \
  class ONNX_OPERATOR_VERSIONED_TYPED_KERNEL_CLASS_NAME(provider, domain, startver, endver, type, name);              \
  template <>                                                                                                         \
  KernelCreateInfo                                                                                                    \
  BuildKernelCreateInfo<ONNX_OPERATOR_VERSIONED_TYPED_KERNEL_CLASS_NAME(provider, domain, startver, endver,           \
                                                                        type, name)>() {                              \
    return KernelCreateInfo(                                                                                          \
        builder.SetName(#name)                                                                                        \
            .SetDomain(domain)                                                                                        \
            .SinceVersion(startver, endver)                                                                           \
            .Provider(provider)                                                                                       \
            .Build(),                                                                                                 \
        static_cast<KernelCreatePtrFn>([](FuncManager&, const OpKernelInfo& info, std::unique_ptr<OpKernel>& out) -> Status { out = std::make_unique<__VA_ARGS__>(info); return Status::OK(); })); \
  }

struct KernelCreateInfo {
  std::unique_ptr<KernelDef> kernel_def;  // Owned and stored in the global kernel registry.
  std::add_pointer<OpKernel*(const OpKernelInfo& info)>::type kernel_create_func;
  Status status;
  KernelCreateInfo(std::unique_ptr<KernelDef> definition,
                   KernelCreateFn create_func)
      : kernel_def(std::move(definition)),
        kernel_create_func(create_func) {}
  ...
};

class KernelDefBuilder {
 public:
  explicit KernelDefBuilder()
      : kernel_def_(std::make_unique<KernelDef>()) {}

  KernelDefBuilder& TypeConstraint(const char* arg_name,
                                                    const std::vector<MLDataType>& default_types) {
    return TypeConstraintImpl(arg_name, default_types, nullptr);
  }

  KernelDefBuilder& TypeConstraintImpl(const std::string& arg_name,
                                                        const std::vector<MLDataType>& default_types,
                                                        const std::vector<MLDataType>* enabled_types) {
    // use the enabled types list if provided
    kernel_def_->enabled_type_constraints_[arg_name] = enabled_types ? *enabled_types : default_types;
    kernel_def_->default_type_constraints_[arg_name] = default_types;
    return *this;
  }

  KernelDefBuilder& SetName(const char* op_name) {
    kernel_def_->op_name_ = std::string(op_name);
    return *this;
  }

  KernelDefBuilder& SetDomain(const char* domain) {
    kernel_def_->op_domain_ = std::string(domain);
    return *this;
  }

  KernelDefBuilder& SinceVersion(int since_version) {
    kernel_def_->op_since_version_start_ = since_version;
    return *this;
  }

  KernelDefBuilder& Provider(const char* provider_type) {
    kernel_def_->provider_type_ = std::string(provider_type);
    return *this;
  }

  std::unique_ptr<KernelDef> Build() {
    kernel_def_->CalculateHash();
    return std::move(kernel_def_);
  }

 private:
  // we own the KernelDef until Build() is called.
  std::unique_ptr<KernelDef> kernel_def_;
};
```

```c++
class kCpuExecutionProvider_Add_kOnnxDomain_7_12_float;
template <>
kernelCreateInfo
BuildKernelCreateInfo<kCpuExecutionProvider_Add_kOnnxDomain_7_12_float>() {
  return KernelCreateInfo(                                                                                          \
      KernelDefBuilder().TypeConstraint("T", DataTypeImpl::GetTensorType<TYPE>()).SetName("Add")                    \
          .SetDomain(kOnnxDomain)                                                                                   \
          .SinceVersion(7, 12)                                                                                      \
          .Provider(kCpuExecutionProvider)                                                                          \
          .Build(),                                                                                                 \
      static_cast<KernelCreatePtrFn>([](FuncManager&, const OpKernelInfo& info, std::unique_ptr<OpKernel>& out) -> Status { out = std::make_unique<Add<float>>(info); return Status::OK(); })); \
}
```

## Kernel Management

```c++
common::Status InferenceSession::Initialize() {
  ...
      ORT_RETURN_IF_ERROR_SESSIONID_(kernel_registry_manager_.RegisterKernels(execution_providers_));
  ...
}

class KernelRegistryManager {
 public:
  Status RegisterKernels(const ExecutionProviders& execution_providers) {
    for (auto& provider : execution_providers) {
      auto iter = provider_type_to_registry_.find(provider->Type());
      if (iter != provider_type_to_registry_.end()) {
        return ORT_MAKE_STATUS(ONNXRUNTIME, FAIL, "found duplicated provider ", provider->Type(),
                              " in KernelRegistryManager");
      }

      auto registry = provider->GetKernelRegistry();
      if (!registry) {
        continue;
      }

      provider_type_to_registry_.insert(std::make_pair(provider->Type(), registry));
    }
    return Status::OK();
  }
 private:
  std::unordered_map<std::string, std::shared_ptr<KernelRegistry>> provider_type_to_registry_;
};

std::shared_ptr<KernelRegistry> CPUExecutionProvider::GetKernelRegistry() const {
  static KernelRegistryAndStatus k = GetCpuKernelRegistry();
  // throw if the registry failed to initialize
  ORT_THROW_IF_ERROR(k.st);
  return k.kernel_registry;
}

KernelRegistryAndStatus GetCpuKernelRegistry() {
  KernelRegistryAndStatus ret;
  ret.st = RegisterCPUKernels(*ret.kernel_registry);
  return ret;
}

Status RegisterCPUKernels(KernelRegistry& kernel_registry) {
  ORT_RETURN_IF_ERROR(RegisterOnnxOperatorKernels(kernel_registry));
#ifndef DISABLE_ML_OPS
  ORT_RETURN_IF_ERROR(::onnxruntime::ml::RegisterOnnxMLOperatorKernels(kernel_registry));
#endif
#ifndef DISABLE_CONTRIB_OPS
  ORT_RETURN_IF_ERROR(::onnxruntime::contrib::RegisterCpuContribKernels(kernel_registry));
#endif
#if defined(ENABLE_TRAINING) || defined(ENABLE_TRAINING_OPS)
  ORT_RETURN_IF_ERROR(::onnxruntime::contrib::RegisterCpuTrainingKernels(kernel_registry));
#endif
  return Status::OK();
}

Status RegisterOnnxOperatorKernels(KernelRegistry& kernel_registry) {
  static const BuildKernelCreateInfoFn function_table[] = {
    BuildKernelCreateInfo<void>,  // default entry to avoid the list become empty after ops-reducing
    BuildKernelCreateInfo<ONNX_OPERATOR_KERNEL_CLASS_NAME(kCpuExecutionProvider, kOnnxDomain, 6, Elu)>,
    BuildKernelCreateInfo<ONNX_OPERATOR_KERNEL_CLASS_NAME(kCpuExecutionProvider, kOnnxDomain, 6, HardSigmoid)>,
    BuildKernelCreateInfo<ONNX_OPERATOR_VERSIONED_KERNEL_CLASS_NAME(kCpuExecutionProvider, kOnnxDomain, 6, 15, LeakyRelu)>,
    BuildKernelCreateInfo<ONNX_OPERATOR_VERSIONED_TYPED_KERNEL_CLASS_NAME(kCpuExecutionProvider, kOnnxDomain, 6, 12, float, Relu)>,
    BuildKernelCreateInfo<ONNX_OPERATOR_VERSIONED_TYPED_KERNEL_CLASS_NAME(kCpuExecutionProvider, kOnnxDomain, 6, 12, double, Relu)>,
    BuildKernelCreateInfo<ONNX_OPERATOR_VERSIONED_TYPED_KERNEL_CLASS_NAME(kCpuExecutionProvider, kOnnxDomain, 6, 12, float, Tanh)>,
    BuildKernelCreateInfo<ONNX_OPERATOR_VERSIONED_TYPED_KERNEL_CLASS_NAME(kCpuExecutionProvider, kOnnxDomain, 6, 12, double, Tanh)>,
    BuildKernelCreateInfo<ONNX_OPERATOR_VERSIONED_KERNEL_CLASS_NAME(kCpuExecutionProvider, kOnnxDomain, 7, 8,
                                                                    PRelu)>,
    BuildKernelCreateInfo<ONNX_OPERATOR_KERNEL_CLASS_NAME(kCpuExecutionProvider, kOnnxDomain, 7, Multinomial)>,
    BuildKernelCreateInfo<ONNX_OPERATOR_VERSIONED_TYPED_KERNEL_CLASS_NAME(kCpuExecutionProvider, kOnnxDomain, 7, 12, float, Add)>,
    BuildKernelCreateInfo<ONNX_OPERATOR_VERSIONED_TYPED_KERNEL_CLASS_NAME(kCpuExecutionProvider, kOnnxDomain, 7, 12, double, Add)>,
  };

  for (auto& function_table_entry : function_table) {
    KernelCreateInfo info = function_table_entry();
    if (info.kernel_def != nullptr) {  // filter disabled entries where type is void
      ORT_RETURN_IF_ERROR(kernel_registry.Register(std::move(info)));
    }
  }
}

using KernelCreateMap = std::multimap<std::string, KernelCreateInfo>;
class KernelRegistry {
 public:
  Status Register(KernelCreateInfo&& create_info) {
    const std::string key = GetMapKey(*create_info.kernel_def);
    // Check op version conflicts.
    const auto range = kernel_creator_fn_map_.equal_range(key);
    for (auto i = range.first; i != range.second; ++i) {
      if (i->second.kernel_def &&
          i->second.kernel_def->IsConflict(*create_info.kernel_def)) {
        return Status(common::ONNXRUNTIME, common::FAIL,
                      "Failed to add kernel for " + key +
                          ": Conflicting with a registered kernel with op versions.");
      }
    }
    ...
    // Register the kernel.
    auto it = kernel_creator_fn_map_.emplace(key, std::move(create_info));
    ...
    return Status::OK();
  }

  // Kernel create function map from op name to kernel creation info.
  // key is opname+domain_name+provider_name
  KernelCreateMap kernel_creator_fn_map_;

  // map from kernel def hash to entry in kernel_creator_fn_map_
  std::unordered_map<HashValue, KernelCreateMap::iterator> kernel_def_hash_lookup_;
};
```

```c++
provider_type_to_registry_ = {
    "CPUExecutionProvider" : cpu_kernel_registry,
    "CUDAExecutionProvider" : cuda_kernel_registry
    ...
}

std::multimap<std::string, KernelCreateInfo> kernel_creator_fn_map_ = {
    "Add kOnnxDomain kCpuExecutionProvider" : add_kernel_create_info,
    ...
}
```

## Kernel Instantiation

```c++
common::Status InferenceSession::Initialize() {
  ...
  // apply any transformations to the main graph and any subgraphs
  ORT_RETURN_IF_ERROR_SESSIONID_(TransformGraph(graph, graph_transformation_mgr_,
                                                execution_providers_, kernel_registry_manager_,
                                                insert_cast_transformer_,
                                                *session_state_,
                                                saving_ort_format));
  ...
  ORT_RETURN_IF_ERROR_SESSIONID_(
        session_state_->FinalizeSessionState(model_location_, kernel_registry_manager_,
                                             session_options_,
                                             serialized_session_state,
                                             // need to keep the initializers if saving the optimized model
                                             !saving_model,
                                             saving_ort_format));
  ...
}

Status SessionState::FinalizeSessionState(const std::basic_string<PATH_CHAR_TYPE>& graph_location,
                                          const KernelRegistryManager& kernel_registry_manager,
                                          const SessionOptions& session_options,
                                          const onnxruntime::fbs::SessionState* serialized_session_state,
                                          bool remove_initializers,
                                          bool saving_ort_format) {
  ...
    ORT_RETURN_IF_ERROR(PopulateKernelCreateInfo(kernel_registry_manager, saving_ort_format));
  ...

  return FinalizeSessionStateImpl(graph_location, kernel_registry_manager, nullptr, session_options,
                                  remove_initializers, constant_initializers_use_count);
}

Status SessionState::PopulateKernelCreateInfo(const KernelRegistryManager& kernel_registry_manager,
                                              bool saving_ort_format) {
  for (auto& node : graph_.Nodes()) {
    const KernelCreateInfo* kci = nullptr;
    ...
    auto status = kernel_registry_manager.SearchKernelRegistry(node, &kci);
    ...
    ORT_IGNORE_RETURN_VALUE(
        kernel_create_info_map_.insert({node.Index(), gsl::not_null<const KernelCreateInfo*>(kci)}));
  }
  ...
  return Status::OK();
}

Status KernelRegistryManager::SearchKernelRegistry(const onnxruntime::Node& node,
                                                   /*out*/ const KernelCreateInfo** kernel_create_info) const {
  ...
  const std::string& ptype = node.GetExecutionProviderType();
  ...
  KernelRegistry* p = nullptr;
  auto iter = provider_type_to_registry_.find(ptype);
  if (iter != provider_type_to_registry_.end()) {
    p = iter->second.get();
  }
  if (p != nullptr) {
    status = p->TryFindKernel(node, std::string(), kernel_create_info);
    if (status.IsOK()) {
      return status;
    }
  }
  ...
}

Status KernelRegistry::TryFindKernel(const Node& node,
                                     ProviderType exec_provider,
                                     const KernelCreateInfo** out) const {
  const auto& node_provider = node.GetExecutionProviderType();
  const auto& expected_provider = (node_provider.empty() ? exec_provider : node_provider);

  auto range = kernel_creator_fn_map_.equal_range(GetMapKey(node.OpType(), node.Domain(), expected_provider));

  for (auto i = range.first; i != range.second; ++i) {
    std::string error_str;
    if (VerifyKernelDef(node, *i->second.kernel_def, error_str)) {
      if (out) *out = &i->second;
      return Status::OK();
    }
  }
}

Status SessionState::FinalizeSessionStateImpl(const std::basic_string<PATH_CHAR_TYPE>& graph_location,
                                              const KernelRegistryManager& kernel_registry_manager,
                                              _In_opt_ const Node* parent_node,
                                              const SessionOptions& session_options,
                                              bool remove_initializers,
                                              std::unordered_map<std::string, size_t>& constant_initializers_use_count,
                                              const std::unordered_map<OrtValueName, OrtMemoryInfo>& outer_scope_node_arg_to_location_map,
                                              bool graph_info_already_created) {
  if (!graph_info_already_created) {
    CreateGraphInfo();
  }
  ...
  ORT_RETURN_IF_ERROR(SequentialPlanner::CreatePlan(parent_node, *graph_viewer_, valid_outer_scope_node_args,
                                                    execution_providers_, kernel_create_info_map_,
                                                    subgraphs_kernel_create_info_maps,
                                                    outer_scope_node_arg_to_location_map,
                                                    ort_value_name_idx_map_, context, p_seq_exec_plan_));
  ...
  ORT_RETURN_IF_ERROR(CreateKernels(kernel_registry_manager));
  ...
}

Status SessionState::CreateKernels(const KernelRegistryManager& kernel_registry_manager) {
  const auto& nodes = graph_viewer_->Nodes();
  if (!nodes.empty()) {
    size_t max_nodeid = 0;
    for (const auto& node : nodes) {
      max_nodeid = std::max(max_nodeid, node.Index());
    }
    session_kernels_.clear();
    session_kernels_.resize(max_nodeid + 1);
    for (const auto& node : nodes) {
      // construct and save the kernels
      const KernelCreateInfo& kci = GetNodeKernelCreateInfo(node.Index());

      // the execution provider was required to be valid to find the KernelCreateInfo so we don't need to check it here
      onnxruntime::ProviderType exec_provider_name = node.GetExecutionProviderType();
      const IExecutionProvider& exec_provider = *execution_providers_.Get(exec_provider_name);

      // assumes vector is already resize()'ed to the number of nodes in the graph
      ORT_RETURN_IF_ERROR(kernel_registry_manager.CreateKernel(node, exec_provider, *this, kci, session_kernels_[node.Index()]));
    }
  }
  node_index_info_ = std::make_unique<NodeIndexInfo>(*graph_viewer_, ort_value_name_idx_map_);
  return Status::OK();
}

const KernelCreateInfo& SessionState::GetNodeKernelCreateInfo(NodeIndex node_index) const {
  auto entry = kernel_create_info_map_.find(node_index);
  // invalid node index or FinalizeSessionState should have been called. Either way it's an internal logic error
  ORT_ENFORCE(entry != kernel_create_info_map_.cend());

  return *entry->second;
}

Status KernelRegistryManager::CreateKernel(const onnxruntime::Node& node,
                                           const IExecutionProvider& execution_provider,
                                           SessionState& session_state,
                                           const KernelCreateInfo& kernel_create_info,
                                           std::unique_ptr<OpKernel>& out) const {
  OpKernelInfo kernel_info(node, *kernel_create_info.kernel_def, execution_provider,
                           session_state.GetConstantInitializedTensors(),
                           session_state.GetOrtValueNameIdxMap(),
                           session_state.GetDataTransferMgr());

  return kernel_create_info.kernel_create_func(session_state.GetMutableFuncMgr(), kernel_info, out);
}
```

## Kernel Run

```c++
Status InferenceSession::Run(const RunOptions& run_options,
                             const std::vector<std::string>& feed_names, const std::vector<OrtValue>& feeds,
                             const std::vector<std::string>& output_names, std::vector<OrtValue>* p_fetches,
                             const std::vector<OrtDevice>* p_fetches_device_info) {
  ...
  std::vector<IExecutionProvider*> exec_providers_to_stop;
  exec_providers_to_stop.reserve(execution_providers_.NumProviders());
  ...
  for (auto& xp : execution_providers_) {
    // call OnRunStart and add to exec_providers_to_stop if successful
    auto start_func = [&xp, &exec_providers_to_stop]() {
      auto status = xp->OnRunStart();
      if (status.IsOK())
        exec_providers_to_stop.push_back(xp.get());

      return status;
    };

    ORT_CHECK_AND_SET_RETVAL(start_func());
  }
  ...
  ORT_CHECK_AND_SET_RETVAL(utils::ExecuteGraph(*session_state_, feeds_fetches_manager, feeds, *p_fetches,
                                              session_options_.execution_mode, run_options.terminate, run_logger,
                                              run_options.only_execute_path_to_fetches));
  ...
  for (auto* xp : exec_providers_to_stop) {
    auto status = xp->OnRunEnd(/*sync_stream*/ true);
    ORT_CHECK_AND_SET_RETVAL(status);
  }
  ...
}

static common::Status ExecuteGraphImpl(const SessionState& session_state,
                                       const FeedsFetchesManager& feeds_fetches_manager,
                                       const std::vector<OrtValue>& feeds, std::vector<OrtValue>& fetches,
                                       const std::unordered_map<size_t, IExecutor::CustomAllocator>& fetch_allocators,
                                       ExecutionMode execution_mode, const bool& terminate_flag,
                                       const logging::Logger& logger, const bool only_execute_path_to_fetches = false) {
  std::unique_ptr<IExecutor> p_exec;
  if (execution_mode == ExecutionMode::ORT_SEQUENTIAL) {
    p_exec = std::make_unique<SequentialExecutor>(terminate_flag, only_execute_path_to_fetches);
  } else if (execution_mode == ExecutionMode::ORT_PARALLEL) {
    auto* p_inter_op_thread_pool = session_state.GetInterOpThreadPool();
    if (!p_inter_op_thread_pool) {
      LOGS(logger, WARNING) << "Only one thread was configured for parallel execution. Hence will use sequential execution.";
      p_exec = std::make_unique<SequentialExecutor>(terminate_flag, only_execute_path_to_fetches);
    } else {
      p_exec = std::make_unique<ParallelExecutor>(session_state, terminate_flag);
    }
  }
  ORT_RETURN_IF_ERROR(p_exec->Execute(session_state,
                                    feeds_fetches_info.feeds_mlvalue_idxs, *p_feeds,
                                    feeds_fetches_info.fetches_mlvalue_idxs, *p_fetches, fetch_allocators,
                                    logger));
}

Status SequentialExecutor::Execute(const SessionState& session_state, const std::vector<int>& feed_mlvalue_idxs,
                                   const std::vector<OrtValue>& feeds, const std::vector<int>& fetch_mlvalue_idxs,
                                   std::vector<OrtValue>& fetches,
                                   const std::unordered_map<size_t, CustomAllocator>& fetch_allocators,
                                   const logging::Logger& logger) {
  ...
  const SequentialExecutionPlan& seq_exec_plan = *session_state.GetExecutionPlan();
  const auto& exec_plan_vec = seq_exec_plan.execution_plan;
  ...
  for (const auto& node_exec_plan : exec_plan_vec) {
    ...
    auto node_index = node_exec_plan.node_index;
    ...
    const auto& node = *graph_viewer.GetNode(node_exec_plan.node_index);
    ...
    auto p_op_kernel = session_state.GetKernel(node_index);
    ...
    OpKernelContextInternal op_kernel_context(session_state, frame, *p_op_kernel, logger, terminate_flag_);
    ...
    compute_status = p_op_kernel->Compute(&op_kernel_context);
  }
}

const OpKernel* GetKernel(size_t node_id) const {
  return (node_id < session_kernels_.size()) ? session_kernels_[node_id].get() : nullptr;
}
```
