#include <iostream>

inline int& getValue()
{
    static int a = 1;
    printf("%p\n", &a);
    return a;
}

