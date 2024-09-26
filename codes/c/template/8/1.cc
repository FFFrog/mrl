#include <iostream>

template <typename T>
T get(){
    return 1;
}

void set() {
    std::cout << "void set()" << std::endl;
}

//int set() {
//    std::cout << "int set()" << std::endl;
//
//    return 0;
//}

int main()
{
    std::cout << get<int>() << std::endl;
    std::cout << get<double>() << std::endl;

    return 0;
}
