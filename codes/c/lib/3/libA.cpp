#include <iostream>

void func_b();
void func_c();

extern "C" void func_a() {
    std::cout << "libA is trying to call func_c()..." << std::endl;
    func_b();
    func_c();
}
