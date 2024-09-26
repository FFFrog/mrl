#include <iostream>

template <typename T, typename U>
void get(T a, U b)
{
    std::cout << "void get(T a, U b)" << std::endl;
}

template <typename T>
void get(T a, double b)
{
    std::cout << "void get(T a, double b)" << std::endl;
}

template <>
void get<int>(int a, double b)
{
    std::cout << "void get<int>(int a, double b)" << std::endl;
}

template <>
void get<int, double>(int a, double b)
{
    std::cout << "void get<int, double>(int a, double b)" << std::endl;
}

int main()
{
    get(1, 1);
    get(1, 2.0);
    get(2.0, 2.0);

    return 0;
}
