# Input: ["eat", "tea", "tan", "ate", "nat", "bat"],
# Output:
# [
#   ["ate","eat","tea"],
#   ["nat","tan"],
#   ["bat"]
# ]

# O(n * mlogm)

def group_anagrams(strs):
	dic = {}
        # keys are sorted anagram
        # values are list of all words that match said anagram

	for i in range(len(strs)):
		sorted_anagram = "".join(sorted(strs[i]))
		group_strs = dic.get(sorted_anagram, [])
		group_strs.append(strs[i])

		dic[sorted_anagram] = group_strs

	return list(dic.values())

print(group_anagrams(["eat", "tea", "tan", "ate", "nat", "bat"]))

# O(n*m)
# Prime factorizations for each letter
