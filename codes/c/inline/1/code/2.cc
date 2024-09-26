#include "1.h"

void add2()
{
    int& a = getValue();
    a += 20;
    std::cout << a << std::endl;
}
