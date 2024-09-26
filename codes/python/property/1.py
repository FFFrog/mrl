import os

class Meta(type):
    value: int = 0

    @property
    def get(cls):
        print("*" * 10)
        print(cls.__name__, id(cls), id(cls.value), cls.value)
        print("*" * 10)
        cls.value += 1
        print(cls.__name__, id(cls), id(cls.value), cls.value)
        print("*" * 10)
        cls.value += 2
        print(cls.__name__, id(cls), id(cls.value), cls.value)
        return cls.value

class A(metaclass=Meta):
    pass

class B(metaclass=Meta):
    pass

if __name__ == "__main__":
    print(A.get)
    print(B.get)
