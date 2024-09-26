#include "1.h"
#include <typeinfo>

using namespace std;

int get()
{
    A a;
    typeid(a).name();
    a.get();
    return 1;
}
