#!/usr/bin/env python

# Anagram test

class Anagram:
    def anagram(self, word1: str, word2: str)->bool:
        return sorted(word1) == sorted(word2)

    def anagram2(self, word1: str, word2: str)->bool:
        dict1 = dict()
        dict2 = dict()

        for i,c in enumerate(word1):
            if c in dict1.keys():
                dict1[c] += 1
            else:
                dict1[c] = 1

        for i,c in enumerate(word2):
            if c in dict2.keys():
                dict2[c] += 1
            else:
                dict2[c] = 1

        for key, value in dict1.items():
            if dict2[key] != value:
                return False

            return True


"""
Sub String
Input: "abcabcbb"
Output: 3
Explanation: The answer is "abc", with the length of 3.
Example 2:

Input: "bbbbb"
Output: 1
Explanation: The answer is "b", with the length of 1.
Example 3:

Input: "pwwkew"
Output: 3
"""
class SubString:
    def subString(self, word: str) -> int:
        s = set()
        longest_subString = 0
        for i,c in enumerate(word):
            if c not in s:
                s.add(c)
            elif len(s) > longest_subString:
                longest_subString = len(s)
                s.clear()
                s.add(c)
        return longest_subString


""" Sub Sequence

Given a string S and a set of words D, find the longest word in D that is a subsequence of S.
Word W is a subsequence of S if some number of characters, possibly zero, can be deleted from S to form W, without reordering the remaining characters.
Note: D can appear in any format (list, hash table, prefix tree, etc.

For example, given the input of S = "abppplee" and D = {"able", "ale", "apple", "bale", "kangaroo"} the correct output would be "apple"
"""

class SubSequence:
    def subS(self, S: str, D: list)->str:
        alphabetCount = dict()
        self.wordDict(S, alphabetCount)

    def wordDict(self, word, dictionary):
        for i,c in enumerate(word):
            if c in dictionary.keys():
                dictionary[c] += 1
            else:
                dictionary[c] = 1

        return "a"



""" Amazon Q1

"""
class ProductSuggestion:
    def threeProductSuggestions(self, numP: int, repo : list, query: str)->list:
        result = list()
        for numChar in range(2, len(query)):
            searchString = query[:numChar]
            result.append(self.Match(numP, repo, searchString))
        return result

    def Match(self, numProducts, repo, query):
        result = list()
        for product in range(0, numProducts):
            if repo[product].startswith(query):
                result.append(repo[product])
        return result

if __name__ == "__main__":
    a = Anagram()
    print("Anagram")
    print(a.anagram("abc", "bac"))
    print(a.anagram2("abc", "bac"))

    b = SubString()
    print("\nSubString")
    print(b.subString("abcabcbb"))
    print(b.subString("bbbbb"))
    print(b.subString("pwwkew"))

    c = SubSequence()
    print("\nSubSequence")
    D = ["able", "ale", "apple", "bale", "kangaroo"]
    S = "abppplee"
    print(c.subS(S,D))


    d = ProductSuggestion()
    print("\nProductSuggestion")
    numP = 5
    repo = ['bags', 'baggage', 'banner', 'box', 'cloths']
    query = 'bags'
    print(*d.threeProductSuggestions(numP, repo, query))
