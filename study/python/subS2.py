#!/usr/bin/env python

"""
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
Explanation: The answer is "wke", with the length of 3.
             Note that the answer must be a substring, "pwke" is a subsequence and not a substring.
"""

class Solution:
    def lengthOfLongestSubstring(self, s: str) -> int:
        char_set = set()
        size = 0
        char_list = list()
        for i,c in enumerate(s):
            if c not in char_set:
                char_set.add(c)
                size += 1
            else:
                char_set.clear()
                char_set.add(c)
                char_list.append(size)
                size = 1

        return max(char_list)

if __name__ == "__main__":
    input1 = "abcabcdbb"
    input2 = "bbbbb"
    input3 = "pwwkew"

    x = Solution()
    print(x.lengthOfLongestSubstring(input1))
    print(x.lengthOfLongestSubstring(input2))
    print(x.lengthOfLongestSubstring(input3))
