// using c++20 please due to concept & requires

#include <iostream>
#include <type_traits>

template<typename T>
concept Cond1 = std::is_integral_v<T>;

template<typename T>
concept Cond2 = std::is_floating_point_v<T>;

template<typename T>
requires Cond1<T>
class A {
public:
    void doSomething() {
        std::cout << "A (Cond1) is doing something!" << std::endl;
    }
};

// A<T> 特化版本的原则
// 比主模版有着更细化的规则
// 不能与主模板产生冲突，主模版是更通用的情况
// 这里报错的原因就是和主模版冲突
template<typename T>
requires Cond2<T>
class A<T> {
public:
    void doSomething() {
        std::cout << "A (Cond2) is doing something!" << std::endl;
    }
};

int main() {
    A<int> obj1;
    obj1.doSomething();

    A<float*> obj2;
    obj2.doSomething();

    return 0;
}

