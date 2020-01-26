#!/usr/bin/env python

class Person:
    def __init__(self, name, age):
        self.name = name
        self.age = age

    def myFunc(self):
        print("My name is " + self.name)

# Inheritence
"""
    Pass is used to avoid compile errors when object is empty
    Class student is inheriting all properties of class Person
"""
class Student(Person):
    pass

if __name__ == "__main__":
    p1 = Person("John", 36)
    print(p1.name)
    print(p1.age)

    x = Student("Mike", 20)
    x.myFunc()

    p1.myFunc()
    del p1.age #this would delete the property age form p1 object
    del p1 # delete the object p1
