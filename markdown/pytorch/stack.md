# The Implementation of Stack

```c++
using Stack = std::vector<c10::IValue>;

template <class... Args>
torch::jit::Stack boxArgs(Args... args) {
  // TODO Reuse stack vector instead of allocating?
  torch::jit::Stack stack;
  stack.reserve(sizeof...(Args));
  torch::jit::push(stack, std::forward<Args>(args)...);
  return stack;
}
```
