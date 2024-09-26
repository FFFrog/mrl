#include <iostream>
#include <boost/type_index.hpp>

using namespace std;

struct base_t {
    virtual ~base_t(){}
};

template<class T>
struct Base : public base_t {
    virtual T t() = 0;
};

template<class T>
struct A : public Base<T> {
    ~A(){}
    virtual T t() override {
        std::cout << "A" << '\n';
        return T{};
    }
};


int main() {
    cout << boost::typeindex::type_id_with_cvr<decltype(std::declval<A<int>>().t())>().pretty_name() << std::endl;
    cout << boost::typeindex::type_id_with_cvr<decltype(std::declval<Base<double>>().t())>().pretty_name() << std::endl;

    return 0;
}

