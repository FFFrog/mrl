# python3 -m dis *.py

import dis

def help(a, b, c, d, e):
    return a + b

def a():
    l = (1,2,3)

def b():
    l = [1,2,3]

def c():
    a = 1

def d():
    print("xxxx")

def e(a, b, c):
    help(a, b, c, d, e)
