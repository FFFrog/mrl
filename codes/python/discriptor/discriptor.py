import os


class Descriptor:
    def __get__(self, instance, owner):
        if instance is None:
            print('__get__(): Accessing x from the class', self, instance, owner)
            return self
        
        print('__get__(): Accessing x from the object', self, instance, owner)
        return self

    def __set__(self, instance, value):
        print('__set__(): Setting x on the object', instance)


class Base:
    value = Descriptor()


base = Base()

print(Base.value)
print(type(Base.value))

print(Base.value.__get__)
print(type(Base.value.__get__))

print(base.value)
print(type(base.value))
