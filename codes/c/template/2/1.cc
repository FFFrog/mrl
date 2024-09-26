#include <iostream>
#include <tuple>
#include <optional>

template <typename GuardT, typename... Args>
struct RAIIContextManager {
  explicit RAIIContextManager(Args&&... args)
      : args_(std::forward<Args>(args)...) {
          std::cout << "init" << std::endl;
      }

  void enter() {
    std::cout << "enter" << std::endl;
    std::cout << std::tuple_size<decltype(args_)>::value << std::endl;
    auto emplace = [&](Args... args) {
      guard_.emplace(std::forward<Args>(args)...);
      std::cout << "emplace" << std::endl;
    };
    std::cout << "Before:" << std::endl;
    std::apply(std::move(emplace), args_);
    std::cout << "After:" << std::endl;
  }

  void exit() {
    std::cout << "exit" << std::endl;
    guard_ = std::nullopt;
  }

 private:
  std::optional<GuardT> guard_;
  std::tuple<Args...> args_;
};

int main()
{
    // RAIIContextManager<int,int> a(1);
    RAIIContextManager<int> a;
    a.enter();
    a.exit();
    return 0;
}
