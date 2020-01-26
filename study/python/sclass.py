#!/usr/bin/env python

class Simple:
    def __init__(self, str):
        print("Inside the simple constructor")
        self.s = str

    def show(self):
        print(self.s)

    def showMsg(self, msg):
        print(msg + ':', self.show())

if __name__ == "__main__":
    x = Simple("Constructor Argument")
    x.show()
    x.showMsg("Sumit")

