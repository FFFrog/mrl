#include <iostream>

struct A {
    static constexpr int value = 10;
    static int second;
};

int A::second = 20;

int main()
{
    std::cout << A::value << std::endl;
    std::cout << A::second << std::endl;

    return 0;
}
