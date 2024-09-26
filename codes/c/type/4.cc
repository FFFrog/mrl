#include <iostream>
#include <functional>

using namespace std;

void getValue(int a)
{
    cout << "getValue" << endl;
}

int main()
{
//    std::function<void(int)> a = getValue;          //OK
//    std::function<void(int)>& a = getValue;         //ERROR
//    const std::function<void(int)>& a = getValue;   //OK

    std::function<void(int)>&& a = getValue;        //OK

    a(10);

    return 0;
}
