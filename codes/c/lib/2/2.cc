#include "common.h"
#include <vector>

std::vector<typing>& getValue()
{
    static std::vector<typing>;
    std::cout << "getValue()" << std::endl;
}
