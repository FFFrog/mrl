#include <iostream>
#include <type_traits>

template <typename T>
concept Cond1 = std::is_integral_v<T>;

template <typename T>
concept Cond2 = std::is_floating_point_v<T>;

template <typename T>
class A;

template <typename T>
requires Cond1<T>
class A<T> {
public:
    void foo() {
        std::cout << "A::foo() for integral types\n";
    }
};

template <typename T>
requires Cond2<T>
class A<T> {
public:
    void foo() {
        std::cout << "A::foo() for floating-point types\n";
    }
};

int main() {
    A<int> obj1;
    A<double> obj2;

    obj1.foo();
    obj2.foo();

    return 0;
}

