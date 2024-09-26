/*
 * readelf -W -s type_info | awk '{print $8}' | xargs -I {} c++filt -n {} | grep A
 *
 * ...
 * typeinfo name for A
 * A::get()
 * typeinfo for A
 * ...
 *
 */

#include <iostream>

using namespace std;

class A {
    public:
        A() {cout << "A" << endl;}
        virtual void get() {
            cout << "get" << endl;
        }
};

int main()
{
    A a;
    a.get();
    return 0;
}
