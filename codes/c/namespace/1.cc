#include <iostream>

namespace A {
    static int s_g_value = 10;
    int g_value = 20;

    namespace {
        static int s_a_value = 30;
        int a_value = 40;
    }
}

namespace A { 
    void get() {
        std::cout << a_value << std::endl;
        std::cout << s_a_value << std::endl;
    }
}

int main()
{
    A::get();

    return 0;
}
