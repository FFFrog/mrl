#include <iostream>
#include <boost/type_index.hpp>

using namespace std;

template <typename T>
void set(T a)
{}

int get(int a, int& b)
{
    return a + b;
}

int main()
{
    cout << boost::typeindex::type_id_with_cvr<decltype(get)>().pretty_name() << std::endl;
    cout << boost::typeindex::type_id_with_cvr<decltype(&get)>().pretty_name() << std::endl;
    cout << boost::typeindex::type_id_with_cvr<decltype(&set<int>)>().pretty_name() << std::endl;
    cout << boost::typeindex::type_id_with_cvr<decltype(&set<double>)>().pretty_name() << std::endl;

    printf("%p\n", get);
    printf("%p\n", &get);
    printf("%p\n", &set<int>);
    printf("%p\n", &set<double>);
    return 0;
}
