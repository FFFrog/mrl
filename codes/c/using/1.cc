#include <iostream>

using namespace std;

class A {
    public:
        A(int i) {cout << "A" << i << endl;}
        A(int i, int j) {cout << "A" << i << j << endl;}
};

class B : public A {
    public:
        using A::A;
};

int main()
{
    B b(1, 1);
    return 0;
}
