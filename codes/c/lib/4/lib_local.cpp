#include <iostream>

extern "C" void conflicted_symbol() {
    std::cout << ">>>LOCAL version from lib_local.so" << std::endl;
}
