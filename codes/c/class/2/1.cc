#include <iostream>

class A {
    private:
        static int v_;
};

int A::v_ = 0;

int main()
{
    // std::cout << A::v_ << std::endl; //Error
    return 0;
}
