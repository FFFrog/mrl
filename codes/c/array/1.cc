#include <iostream>
#include <memory>

class Person {
public:
    std::string name;
    int age;

    Person(const std::string& n, int a) : name(n), age(a) {
        std::cout << "Person" << std::endl;
    }
    ~Person() {std::cout << "~Person" << std::endl;}
};

int main() {
    // Person **persons = new Person* [3]; //OK, but not initialized to nullptr
    Person **persons = new Person* [3](); //OK, initialized to nullptr

    persons[0] = new Person("Alice", 25);
    persons[1] = new Person("Bob", 30);
    persons[2] = new Person("Charlie", 35);

    for (int i = 0; i < 3; ++i) {
        std::cout << "Person " << i+1 << ": " << persons[i]->name << " (age: " << persons[i]->age << ")" << std::endl;
    }

    for (int i = 0; i < 3; ++i)
        delete persons[i];

    delete[] persons;

    return 0;
}
