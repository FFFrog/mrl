#include <iostream>
#include <boost/type_index.hpp>

using namespace std;

template <typename T>
class A {
    public:
        template <typename P>
        A(T s, P a)
        {
            cout << s << endl;
            cout << a << endl;
        }
};

template <typename T>
T show(A<T> a)
{
    return 1;
}

int main()
{
    A<int> a(1, 10);
    cout << boost::typeindex::type_id_with_cvr<decltype(a)>().pretty_name() << std::endl;
    cout << show(a) << endl;;

    return 0;
}
