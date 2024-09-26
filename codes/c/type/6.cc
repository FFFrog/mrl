#include <iostream>
#include <boost/type_index.hpp>

using namespace std;

struct A {
    A() = default;
    int value;
};

int main()
{
    cout << boost::typeindex::type_id_with_cvr<decltype(&A::value)>().pretty_name() << std::endl;

    return 0;
}
