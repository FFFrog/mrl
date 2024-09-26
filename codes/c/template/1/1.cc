#include <iostream>

template <typename T, typename... Types>
void addone(T &firstArg, Types &... args) {
  firstArg ++;
  if constexpr (sizeof...(args) > 0) {
    addone(args...);
  }
}

template <typename T, typename... Types>
void print(T const &firstArg, Types const &... args) {
  std::cout << firstArg << '\n';
  if constexpr (sizeof...(args) > 0) {
    print(args...);
  }
}


template <typename... Types>
void test(Types&... args)
{
    print(args...);
    addone(args...);
    print(args...);
}


int main()
{
    int a = 1;
    int b = 2;
    int c = 3;

    test(a, b, c);

    return 0;
}

