#include <iostream>

struct A {
    static void get();
    static void set();
};

void A::set()
{}

int main()
{
    A a;

    a.set();
    A::set();

    return 0;
}
