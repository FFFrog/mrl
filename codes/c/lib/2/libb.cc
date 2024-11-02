#include <iostream>

void addBase()
{
    std::cout << "addBase() in libb.so" << std::endl;
}

void addB()
{
    addBase();
    std::cout << "addB()" << std::endl;
}
