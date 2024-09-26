#include <dlfcn.h>
#include <1.h>

typedef void (*fn1)(A*, int);
typedef void (*fn2)(A*);

int main() {
    // create a new object;
    A a(1);

    // load the lib
    void* handle = dlopen("./lib1.so", RTLD_NOW | RTLD_LOCAL);

    // load the symbol
    fn1 ptr_set = (fn1)dlsym(handle, "_ZN1A3setEi");
    fn2 ptr_get = (fn2)dlsym(handle, "_ZN1A3getEv");

    // Prototype: void A::set(int a)
    // call the function
    // the first parameters is used as this
    // the second parameters is used as parameter
    ptr_set(&a, 20);
    std::cout << "-----------------------" << std::endl;
    ptr_get(&a);

    // close the lib
    dlclose(handle);

    return 0;
}

