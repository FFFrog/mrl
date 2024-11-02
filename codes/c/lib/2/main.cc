#include <iostream>
#include <stdio.h>
#include <dlfcn.h>

int main() {
    std::cout << "main begin" << std::endl;
    void* h1;
    void* h2;

    typedef void (*func)();
    char* error;

    // Case 1:
    // ignore the error logic
    std::cout << "Load Start: liba.so" << std::endl;
    h1 = dlopen("./liba.so", RTLD_NOW | RTLD_GLOBAL);
    std::cout << "Load End: liba.so" << std::endl;

    func f1 = (func)dlsym(h1, "_Z4addAv");

    std::cout << "Run Start: liba.so" << std::endl;
    f1();
    std::cout << "Run End: liba.so" << std::endl;

    // ignore the error logic
    std::cout << "Load Start: libb.so" << std::endl;
    h2 = dlopen("./libb.so", RTLD_NOW | RTLD_LOCAL);
    std::cout << "Load End: libb.so" << std::endl;

    func f2 = (func)dlsym(h2, "_Z4addBv");

    std::cout << "Run Start: libb.so" << std::endl;
    f2();
    std::cout << "Run End: libb.so" << std::endl;


    // Case:
    dlclose(h1);
    dlclose(h2);

    std::cout << "main end" << std::endl;

    return 0;
}
