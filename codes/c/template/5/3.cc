#include <iostream>
#include <type_traits>

template<typename T>
class A {
public:
    void doSomething() {
        std::cout << "A is doing something!" << std::endl;
    }
};

template<typename T>
requires std::is_pointer_v<T>
class A<T> {
public:
    void doSomething() {
        std::cout << "A<T> is doing something!" << std::endl;
    }
};

template<typename T>
requires std::is_integral_v<T>
class A<T*> {
public:
    void doSomething() {
        std::cout << "A<T*> is doing something!" << std::endl;
    }
};

int main() {
    A<int> obj1;
    obj1.doSomething();

    A<int*> obj2;
    obj2.doSomething();

    return 0;
}

