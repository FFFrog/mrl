#include <iostream>
#include <optional>

using namespace std;

struct A {
    A() {cout << "A()" << endl;}
    ~A() {cout << "~A()" << endl;}
};

int main()
{
    std::optional<A> a = std::nullopt;
    a.emplace();
    return 0;
}

