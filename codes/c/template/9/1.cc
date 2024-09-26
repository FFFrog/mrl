#include <iostream>

template <typename T>
auto get() {
    return 0;
}

template<>
auto get<int>() {
    return 1;
}

template<>
auto get<double>() {
    return 2.0;
}

int main()
{
    std::cout << get<int>() << std::endl;
    std::cout << get<double>() << std::endl;
    return 0;
}
