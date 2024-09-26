#include <iostream>

using namespace std;

__attribute__((weak)) int getValue()
{
    return 1;
}

int main()
{
    cout << getValue() << endl;
    return 0;
}
