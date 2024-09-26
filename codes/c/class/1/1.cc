#include <iostream>

using namespace std;

struct A
{
    void get() noexcept {
        if (this == nullptr)
            cout << "0x0" << endl;
        else
            cout << this << endl;
    }
};


int main() {
    A* a = nullptr;
    a->get();

    auto b = A();
    a = &b;
    a->get();

    return 0;
}
