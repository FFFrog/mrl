#include <iostream>
#include <stdio.h>

using namespace std;

class Base
{
public:
	Base() {};
	~Base() {};

    void tt0() {
        cout << a << endl;
    }

private:
    int a{666};
};

class Derive : public Base
{
public:
	Derive() {};
	~Derive() {};

	virtual void tt1() {
		cout << b << endl;
	};

private:
	int b{100};
};

int main()
{
	Base b1;
	printf("Base 虚表地址:%p\n", *(int *)&b1);
	printf("Base 虚表地址:%d\n", *(int *)&b1);
	printf("Base 虚表地址:%p\n", &b1);

	Derive d;
	printf("Derive 虚表地址:%p\n", *(int *)&d);
	printf("Derive 虚表地址:%d\n", *(int *)&d);
	printf("Derive 虚表地址:%p\n", &d);
    d.tt1();

    Base* b2 = static_cast<Base*>(&d);
	printf("Base(After cast) 虚表地址:%p\n", *(int *)b2);
	printf("Base(After cast) 虚表地址:%d\n", *(int *)b2);
	printf("Base(After cast) 虚表地址:%p\n", b2);
    b2->tt0();

	return 0;
}

