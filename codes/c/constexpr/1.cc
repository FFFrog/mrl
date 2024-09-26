#include <iostream>

using namespace std;

class Point {
public:
    constexpr Point(int x, int y) : x_(x), y_(y) {}

    constexpr int getX() const {
        return x_;
    }

    constexpr int getY() const {
        return y_;
    }

    int add() const {
        return x_ + y_;
    }

private:
    int x_;
    int y_;
};

int main() {
    constexpr Point p(2, 3);
    constexpr int x = p.getX();
    static_assert(x == 2, "X coordinate initialization error");

    cout << x << endl;
    cout << p.add() << endl;
    cout << &p << endl;
    return 0;
}

