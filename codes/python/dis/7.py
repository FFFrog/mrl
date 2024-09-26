# python3 -m dis *.py

import os

def func(a, *args, **kwargs):
    print(a)
    print(args)
    print(kwargs)

if __name__ == "__main__":
    func(1, 2, 3, c=1, d=3)
