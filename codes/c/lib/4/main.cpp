#include <iostream>
#include <dlfcn.h>

typedef void (*Func)();

int main() {
    std::cout << "--- Loading global library..." << std::endl;
    dlopen("./lib_global.so", RTLD_LAZY | RTLD_GLOBAL);

    std::cout << "--- Loading local library..." << std::endl;
    void* handle = dlopen("./lib.so", RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        std::cerr << "dlopen failed: " << dlerror() << std::endl;
        return 1;
    }

    Func func = (Func)dlsym(handle, "func");
    if (!func) {
        std::cerr << "dlsym failed: " << dlerror() << std::endl;
        return 1;
    }

    func();

    dlclose(handle);

    return 0;
}
