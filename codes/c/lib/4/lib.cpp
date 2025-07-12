#include <iostream>

extern "C" void conflicted_symbol();

extern "C" void func() {
    conflicted_symbol();
}
