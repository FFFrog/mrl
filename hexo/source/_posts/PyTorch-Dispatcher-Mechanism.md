---
title: PyTorch Dispatcher Mechansim
date: 2023-11-09 10:17:25
categories: PyTorch
tags: Dispatcher
keywords: PyTorch Dispatcher
---

# The Implementation of Dispatcher

## DispatchKey

```C++
DispatchKey: uint16_t (total 132)
1) Functionality Key(46)
    a) non-customizable functionalities:
       BackendSelect
       Python
       ADInplaceOrView
       VmapMode
       ...
       // TODO: make Autocast a functionality key
       AutocastCPU
       AutocastXPU
       AutocastIPU
       AutocastHPU
       AutocastXLA
       AutocastCUDA
       AutocastPrivateUse1
       ...
    b) customizable per backend(5)
       Dense
       Quantized
       Sparse
       NestedTensor
       Autograd
2) per-backend instances of customizable functionalities(15)
    CPU
    CUDA
    HIP
    XLA
    MPS
    IPU
    XPU
    HPU
    VE
    Lazy
    MTIA
    PrivateUse1
    PrivateUse2
    PrivateUse3
    Meta
3) alias key(8)
    Autograd
    CompositeImplicitAutograd
    CompositeExplicitAutograd
    ...
```

## DispatchKeySet

每个Tensor都会带有一个``DispatchKeySet``,用来标识该Tensor的相关分发信息

```C++
uint64_t repr_ = 0;
constexpr DispatchKeySet(Full)
  : repr_((1ULL << (num_backends(/**15**/) + num_functionality_keys(/**47**/) - 1)) - 1) {}
```

## Dispatcher

![Dispatcher Table](/images/Dispatcher_table.png)

```Python
import torch

x = torch.tensor([2, 3])
y = torch.tensor([3, 4])

z = x + y
```

```text
[call] op=[aten::empty.memory_format], key=[BackendSelect]
 [redispatch] op=[aten::empty.memory_format], key=[CPU]
...
[call] op=[aten::empty.memory_format], key=[BackendSelect]
 [redispatch] op=[aten::empty.memory_format], key=[CPU]
...
[call] op=[aten::add.Tensor], key=[AutogradCPU]
 [redispatch] op=[aten::add.Tensor], key=[CPU]
```

![Dispatcher Key](/images/Dispatcher_key.png)

