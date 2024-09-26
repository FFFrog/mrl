#include <iostream>

using namespace std;

struct A {
    A(int value) {
        v_ = value;
        cout << "A(" << v_ << ")" << endl;
    }

    ~A() {
        cout << "~A(" << v_ << ")" << endl;
    }

    int v_;
};

void show() {
    static A a(1);
}

int main()
{
    cout << "main begin" << endl;
    show();
    cout << "main end" << endl;
    return 0;
}
