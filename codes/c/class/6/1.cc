// 从汇编角度查看构造/析构初始化调用流程

#include <iostream>

using namespace std;

struct Member {
    Member();
    ~Member();
};

Member::Member() {
    cout << "Member::Member()" << endl;
}

Member::~Member() {
    cout << "Member::~Member()" << endl;
}

struct Base {
    Base();
    ~Base();
};

Base::Base() {
    cout << "Base::Base()" << endl;
}

Base::~Base() {
    cout << "~Base::Base()" << endl;
}

struct Derive : public Base {
    Derive();
    ~Derive();

    Member m;
};

Derive::Derive() {
    cout << "Derive::Derive()" << endl;
}

Derive::~Derive() {
    cout << "Derive::~Derive()" << endl;
}

int main()
{
    Derive D;

    return 0;
}
