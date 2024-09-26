#include "1.h"


void A::set(int a)
{
    std::cout << "set begin" << std::endl;
    std::cout << "A::set(" << a << ")" << std::endl;
    std::cout << "v_=" << v_ << std::endl;
    v_ = a;
    std::cout << "v_=" << v_ << std::endl;
    std::cout << "set end" << std::endl;
}

void A::get()
{
    std::cout << "get begin" << std::endl;
    std::cout << "v_=" << v_ << std::endl;
    std::cout << "get end" << std::endl;
}
