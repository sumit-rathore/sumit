#!/usr/bin/env python

# Implement an algorithm to determine if a string has all unique characters.

# "abc" -> True
# "" -> True
# "aabc" -> False

# Brute Force - O(n^2)
# Sorting - O(nlogn + n) -> O(nlogn)
# Hashset - O(n)

class UniqueChar:
    def __init__(self):
        pass

    def unique_char(self, str):
        # A set will only allow unique chars in it
        # if the char not in set, add it to set
        # If it is already there, return false
        char_list = set()

        for i,c  in enumerate(str):
            if c in char_list:
                return False
            else:
                char_list.add(c)
        return True


if __name__ == "__main__":
    x = UniqueChar()
    print(x.unique_char("abc"))
    print(x.unique_char(""))
    print(x.unique_char("abca"))

