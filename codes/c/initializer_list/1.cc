#include <iostream>
#include <array>

using namespace std;

struct C {
    int a;
    int b;
    int c;
};

int main()
{
    std::array<int,3 > a = {1,2,3};
    std::array<int,3 > b = {{4,5,6}};

    C c = {{7, 8, 9}};

    for(const int& i : a) {
        cout << i << endl;
    }

    for(const int& i : b) {
        cout << i << endl;
    }

    std::cout << c.a << c.b << c.c << std::endl;

    return 0;
}
