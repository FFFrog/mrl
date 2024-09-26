#include <iostream>

class Base {
public:
    Base() {
        std::cout << "Base Constructor\n";
    }
};

class Member {
public:
    Member() {
        std::cout << "Member Constructor\n";
    }
    Member(int i) {
        std::cout << "Member(int i) Constructor\n";
    }
};

class Derived : public Base {
public:
    Derived() : member(1) {
        std::cout << "Derived Constructor\n";
    }

    Member member;
};

int main() {
    Derived d;
    return 0;
}
