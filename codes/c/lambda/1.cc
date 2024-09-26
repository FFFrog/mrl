#include <iostream>

int g_v = 0;

void global_f() {
  auto f =  []() {
      g_v = 1;
  };

  f();
  std::cout << g_v << std::endl;
}

void static_f() {
  static int s_v = 10;
  auto f =  []() {
      s_v = 11;
  };

  f();
  std::cout << s_v << std::endl;
}

void f() {
  int v = 10;
  auto f =  []() {
      v = 11;
  };

  f();
  std::cout << v << std::endl;
}

int main()
{
    global_f();
    static_f();

    f();        //Error: assignment of read-only variable

    return 0;
}
