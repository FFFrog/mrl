#include "1.h"

// g++ -fvisibility=hidden -c 1.cc

struct alignas(2) BFloat16 {
  uint16_t x;

  BFloat16() = default;
  inline operator float() const;
};

template <typename T>
void get(T a) {
    std::cout << "void get(T a)" << std::endl;
}

// global && default
template <>
void get<int>(int a)
{
    std::cout << "void get<int>(int a)" << std::endl;
}
template <>
void get<float>(float a)
{
    std::cout << "void get<float>(float a)" << std::endl;
}
template <>
void get<BFloat16>(BFloat16 a)
{
    std::cout << "void get<BFloat16>(BFloat16 a)" << std::endl;
}

// weak && default
template void get<double>(double a);
template void get<char>(char a);

// global && hidden
void get()
{
    std::cout << "void get()" << std::endl;
}
