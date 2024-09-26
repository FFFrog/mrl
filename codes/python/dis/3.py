# python3 -m dis *.py

import os

var = 1


def func2():
    func1()
    print("func2()")


def func1():
    var = 1
    print("func1()")

func1.var = 10


if __name__ == "__main__":
    func2()
    print(func1.var)
