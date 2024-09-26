#include <iostream>

template<typename T>
struct C
{
    C(T v) {
        std::cout << v << std::endl;
    }
};

template <typename T, typename V>
void pp(C<V> x)
{
    V a;
    T b;

    a = 1;
    b = 2;

    std::cout << a << " " << b << std::endl;
}


int main()
{
    C<int> x(1);
    pp<double>(x);

    return 0;
}
