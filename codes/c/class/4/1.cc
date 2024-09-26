#include<iostream>
using namespace std;

class Container;

class Component {
    public:
        Component(){
            cout << "Component ctor ..." << endl;
        }
        Component(Component& a){
            cout << "Component copy ..." << endl;
        }
        Component& operator=(Component& a){
            cout << "Component == ..." << endl;
            this == &a;
            return *this;
        }
        ~Component(){
            cout << "Component dtor ..." << endl;
        }
};

class Container {
    public:
        Container(){
            cout << "Container ctor ..." << endl;
        }
        Container(Component& c){
            this->c = c;
            cout << "Container copy ..." << endl;
        }
        ~Container(){
            cout << "Container dtor ..." << endl;
        }
        operator Component() {
            return c;
        }

        Component c;
};


int main()
{
    Component a;
    Container b(a);
    Component c = b;

    return 0;
}
