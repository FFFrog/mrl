#include <iostream>

using namespace std;

template <typename T>
struct A {
    A(T v) {cout << "A()" << endl;}
};

template <typename Ret, typename... Args>
struct A<Ret(Args...)> {
    using T = Ret(Args...);
    A(T v ) {cout << "A<Ret(Args...)>" << endl;}
};

template <typename Ret, typename... Args>
struct A<Ret(*)(Args...)> {
    using T = Ret(*)(Args...);
    A(T v ) {cout << "A<Ret(*)(Args...)>" << endl;}
};

void getValue(int a, double b)
{
    cout << a << " " << b << endl;
}

int main()
{
    A(1);
    A a(1);

    A b(getValue);
    A c(&getValue);
    A<void(int,double)> d(&getValue);
    A<void(*)(int,double)> e(&getValue);

    return 0;
}
