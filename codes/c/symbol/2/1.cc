#include <iostream>

static int g_value = 10;

int get()
{
    static int f_value = 1;

    return f_value;
}

int main()
{
    std::cout << g_value << std::endl;
    std::cout << get() << std::endl;

    return 0;
}
