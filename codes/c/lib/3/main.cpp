#include <iostream>
#include <dlfcn.h>

typedef void (*FuncA)();

int main() {
    void* handle = dlopen("./libA.so", RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        std::cout << "ERROR" << std::endl;
        return 1;
    }

    FuncA func = (FuncA)dlsym(handle, "func_a");
    if (func) {
        func();
    }

    dlclose(handle);
    return 0;
}
