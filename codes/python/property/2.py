import os


class Meta(type):
    value: int = 0

class A(metaclass=Meta):
    pass

class B(metaclass=Meta):
    pass

if __name__ == "__main__":
    print("*" * 10)
    print(id(Meta.value),  Meta.value)
    print(id(A.value), A.value)
    print(id(B.value), B.value)
    print("*" * 10)
    Meta.value += 1
    print(id(Meta.value),  Meta.value)
    print(id(A.value), A.value)
    print(id(B.value), B.value)
    print("*" * 10)
    A.value += 1
    print(id(Meta.value),  Meta.value)
    print(id(A.value), A.value)
    print(id(B.value), B.value)
    print("*" * 10)
    B.value += 1
    print(id(Meta.value),  Meta.value)
    print(id(A.value), A.value)
    print(id(B.value), B.value)
