#include "1.h"

void add1()
{
    int& a = getValue();
    a += 10;
    std::cout << a << std::endl;
}
