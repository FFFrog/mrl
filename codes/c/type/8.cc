#include <iostream>

void get(){
    std::cout << "get()" << std::endl;
}

template <typename Func>
void wrapper(Func f)
{
    f();
}

template <typename Func>
void wrapper2(Func* f)
{
    f();
}

int main()
{
    wrapper(get);
    wrapper(&get);
    wrapper2(get);
    wrapper2(&get);
    return 0;
}

