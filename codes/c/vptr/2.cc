#include <iostream>
#include <stdio.h>

using namespace std;

class Base1 {
public:
    int ibase1;
    Base1():ibase1(10) {}
    virtual void f() { cout << "Base1::f()" << endl; }
    virtual void g() { cout << "Base1::g()" << endl; }
    virtual void h() { cout << "Base1::h()" << endl; }

};

class Base2 {
public:
    int ibase2;
    Base2():ibase2(20) {}
    virtual void f() { cout << "Base2::f()" << endl; }
    virtual void g() { cout << "Base2::g()" << endl; }
    virtual void h() { cout << "Base2::h()" << endl; }
};

class Base3 {
public:
    int ibase3;
    Base3():ibase3(30) {}
    virtual void f() { cout << "Base3::f()" << endl; }
    virtual void g() { cout << "Base3::g()" << endl; }
    virtual void h() { cout << "Base3::h()" << endl; }
};

class Derive : public Base1, public Base2, public Base3 {
public:
    int iderive;
    Derive():iderive(100) {}
    virtual void f() { cout << "Derive::f()" << endl; }
    virtual void g1() { cout << "Derive::g1()" << endl; }
};

typedef void(*Fun)(void);

int main()
{
    Base1 b1;
    Base2 b2;
    Base3 b3;

    Derive d;

    printf("Base1 虚表地址:%p\n", *(int*)(&b1));
    printf("Base2 虚表地址:%p\n", *(int*)&b2);
    printf("Base3 虚表地址:%p\n", *(int*)&b3);

    printf("Derive 虚表地址:%p\n", *(int*)&d);
    return 0;
}
