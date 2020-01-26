#!/usr/bin/env python

from sclass import Simple

class Simple2(Simple):
    def __init__(self, str):
        print("inside Simple2 constructor")
        Simple.__init__(self.str)

    def display(self):
        self.showMsg("called from display()")

    def show(self):
        print("Overridden show() method")
        Simple.show(self)


class Different:
    def show(self):
        print("Not derived from Simple")


if __name__ == "__main__":
    x = Simple2("Simple2 constructor argument")
    x.display()
    x.show()
    x.showMsg("Inside main")
    def f(obj): obj.show() # one-line definition
    f(x)
    f(Different())
