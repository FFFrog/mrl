# python3 -m dis *.py

import dis

def g():
    pass

def f():
    pass

class OuterClass:
    def __init__(self):
        pass
    def get(self):
        pass
    class InnerClass:
        def __init__(self, x):
            self.x = x

        def square(self):
            return self.x ** 2
