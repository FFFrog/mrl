#include <iostream>

struct A {
    A(int a) {v_ = a;}
    void set(int a);
    void get();

    int v_;
};

