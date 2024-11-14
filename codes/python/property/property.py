import os

class Descriptor:

    def __get__(self, instance, owner):
        if instance is None:
            print('__get__(): Accessing x from the class', instance,owner)
            return self
        
        print('__get__(): Accessing x from the object', instance,owner)
        return 'X from descriptor'

    def __set__(self, instance, value):
        print('__set__(): Setting x on the object', instance)

class Foo:
    x = Descriptor()

Foo.x

a = Foo()
a.x
