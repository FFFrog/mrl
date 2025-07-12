#include <iostream>

extern "C" void conflicted_symbol() {
    std::cout << ">>> GLOBAL version from lib_global.cpp" << std::endl;
}
