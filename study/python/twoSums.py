"""
Given nums = [2, 7, 11, 15], target = 9,

Because nums[0] + nums[1] = 2 + 7 = 9,
return [0, 1].
"""

class Solution:
    def twoSum(self, nums, target):
        if len(nums) < 3:
            print("error")

        first_index = 0
        last_index = 0
        for index in range(len(nums)):
            if nums[index] == target:
                if (0 in nums) and (nums.index(0) != index):
                    first_index = index
                    last_index = nums.index(0)
                    break
            else:
                diff = target - nums[index]
                if (diff in nums) and (nums.index(diff) != index):
                    first_index = index
                    last_index = nums.index(diff)
                    break


        return [first_index, last_index]


if __name__ == "__main__":
    nums = [-3,4,3,90]
    target = 0
    x = Solution()
    print(x.twoSum(nums, target))
