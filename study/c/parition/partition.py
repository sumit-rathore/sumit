class Solution:
    def canPartition(self, num_list):
        lens = len(num_list)
        if lens == 1:
            return False
        total = sum(num_list)
        if total % 2 == 1:
            return False
        target = total // 2
        num_list.sort(reverse=True)
        for i in range(lens):
            tem = 0
            for j in range(i, lens):
                if tem + num_list[j] == target:
                    return True
                elif tem + num_list[j] < target:
                    tem += num_list[j]
        return False


solution = Solution()

l = [1, 5, 11, 5]
print(solution.canPartition(l))
