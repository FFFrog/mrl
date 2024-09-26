#include <iostream>
#include <functional>

using namespace std;

void getValue(int a)
{
    cout << a << endl;
}

int main()
{
    std::function<void(int)> a = getValue;
    a(1);
    return 0;
}
