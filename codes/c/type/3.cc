#include <iostream>
#include <boost/type_index.hpp>

using namespace std;

struct A {
    void get(const int& a);
    static void set(const int& a);
};

void A::get(const int& a) {
    cout << "void get(const int& a)" << endl;
}

void A::set(const int& a) {
    cout << "static void set(const int& a)" << endl;
}

int main()
{
    cout << boost::typeindex::type_id_with_cvr<decltype(&A::get)>().pretty_name() << std::endl;
    cout << boost::typeindex::type_id_with_cvr<decltype(&A::set)>().pretty_name() << std::endl;

    return 0;
}
