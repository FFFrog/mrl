#include <iostream>
#include <functional>

using typing = std::function<void(void)>

struct A {
    A(int value) {
        _v = value;
        std::cout << "A(" << _v << ")" << std::endl;
    }

    ~A() {
        std::cout << "~A(" << _v << ")" << std::endl;
    }

    int _v;
};


