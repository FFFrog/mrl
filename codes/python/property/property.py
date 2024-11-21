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


class Foo:
    x = Descriptor()


f = Foo()

print(Foo.x)
print(type(Foo.x))

print(Foo.x.__get__)
print(type(Foo.x.__get__))

print(f.x)
print(type(f.x))
