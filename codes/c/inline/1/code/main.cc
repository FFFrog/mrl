#include <iostream>
#include <stdio.h>
#include <dlfcn.h>

extern void add1();
extern void add2();

int main() {
    void* h1;
    void* h2;

    typedef void (*func)();
    char* error;

    // ignore the error logic
    h1 = dlopen("./lib1.so", RTLD_NOW | RTLD_LOCAL);
    func add1 = (func)dlsym(h1, "_Z4add1v");

    h2 = dlopen("./lib2.so", RTLD_NOW | RTLD_LOCAL);
    func add2 = (func)dlsym(h2, "_Z4add2v");

    add1();
    add2();

    dlclose(h1);
    dlclose(h2);

    return 0;
}
