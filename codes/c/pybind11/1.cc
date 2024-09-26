// g++ -O3 -Wall -shared -std=c++11 -fPIC `python3 -m pybind11 --includes` 1.cc -o example`python3-config --extension-suffix`

#include <pybind11/pybind11.h>

class MyClass {
public:
    MyClass(int value) : value(value) {}
    int getValue() const { return value; }
    void setValue(int newValue) { value = newValue; }

private:
    int value;
};

MyClass* createMyClass(int value) {
    return new MyClass(value);
}

namespace py = pybind11;

PYBIND11_MODULE(example, m) {
    py::class_<MyClass>(m, "MyClass")
        .def(py::init<int>())
        .def("get_value", &MyClass::getValue)
        .def("set_value", &MyClass::setValue);

    m.def("create_my_class", &createMyClass, py::return_value_policy::take_ownership);
}
