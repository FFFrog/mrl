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
    h1 = dlopen("./lib2.so", RTLD_NOW | RTLD_LOCAL);
    std::cout << "h1:------------" << std::endl;
    func f1 = (func)dlsym(h1, "_Z7preparev");
    std::cout << "f1:------------" << std::endl;
    f1();

    // ignore the error logic
    h2 = dlopen("./lib1.so", RTLD_NOW | RTLD_LOCAL);
    std::cout << "h2:------------" << std::endl;
    func f2 = (func)dlsym(h2, "_Z4add1v");
    std::cout << "f2:------------" << std::endl;
    f2();

    // Case 2:
    // ignore the error logic
    // h1 = dlopen("./lib2.so", RTLD_NOW | RTLD_LOCAL);
    // std::cout << "h1:------------" << std::endl;
    // func f1 = (func)dlsym(h1, "_Z7preparev");
    // std::cout << "f1:------------" << std::endl;
    // f1();

    // Case:
    dlclose(h2);
    dlclose(h1);

    std::cout << "main end" << std::endl;

    return 0;
}
