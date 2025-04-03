import os

g_value = 1


class Base:
    def __init__(self):
        super().__init__()

    def outer(self, arg_1, closure_1):
        global g_value
        g_value = 10

        arg_2 = 1
        colsure_2 = arg_1 + arg_2

        def inner():
            return closure_1 + colsure_2

        return inner
