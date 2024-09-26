#include <iostream>

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
