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
        longest = 0
        count = 0
        last_seen = {}

        for index, character in enumerate(s):
            count += 1
            if character in last_seen:
                count = min(count, index - last_seen[character])
            last_seen[character] = index
            longest = max(count, longest)

        return longest

if __name__ == "__main__":
    input1 = "abcabcbb"
    input2 = "bbbbb"
    input3 = "pwwkew"

    x = Solution()
    print(x.lengthOfLongestSubstring(input1))
    print(x.lengthOfLongestSubstring(input2))
    print(x.lengthOfLongestSubstring(input3))
