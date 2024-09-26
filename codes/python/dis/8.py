# python3 -m dis *.py

import os

class demo:
    def __init__(self):
        print("__init__")
    
    def __del__(self):
        print("__del__")

    def __repr__(self):
        return "__repr__"

def func(a):
    b = demo()
    print(a, b)

if __name__ == "__main__":
    print("start")
    func(1)
    print("end")
