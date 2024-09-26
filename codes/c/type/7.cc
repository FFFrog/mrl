#include <iostream>
#include <boost/type_index.hpp>

using namespace std;

int a[10] = {0};

int main()
{
    cout << boost::typeindex::type_id_with_cvr<decltype(a)>().pretty_name() << std::endl;
    cout << boost::typeindex::type_id_with_cvr<decltype(&a)>().pretty_name() << std::endl;

    return 0;
}
