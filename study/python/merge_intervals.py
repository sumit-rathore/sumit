"""
Given a collection of intervals, merge all overlapping intervals.

Example 1:

Input: [[1,3],[2,6],[8,10],[15,18]]
Output: [[1,6],[8,10],[15,18]]
Explanation: Since intervals [1,3] and [2,6] overlaps, merge them into [1,6].
Example 2:

Input: [[1,4],[4,5]]
Output: [[1,5]]
Explanation: Intervals [1,4] and [4,5] are considered overlapping
"""

class Solution:
    @staticmethod
    def sort_by_start_of_interval(interval:  List[List[int]]) -> int:
        return interval[0]

    def merge(self, intervals: List[List[int]]) -> List[List[int]]:
        if len(intervals) < 2:
            return intervals

        merged_intervals = []
        intervals.sort(key=self.sort_by_start_of_interval)
        start, end = intervals[0]
        for i in range(1, len(intervals)):
            next_start, next_end = intervals[i]
            is_within_interval = next_start <= end
            is_non_mergerable_interval = not is_within_interval

            if is_within_interval:
                end = max(end, next_end)

            if is_non_mergerable_interval:
                merged_intervals.append([start, end])

                start, end = next_start, next_end

        # add last interval
        # since our algorithm focuses on looking at the previous interval
        # we need to expliclty handel the last interval case since there
        # is no item after it to trigger the append.
        merged_intervals.append([start, end])
        return merged_intervals


if __name__ == "__main__":
    Input = [[1,3],[2,6],[8,10],[15,18]]
    Input2 = [[1,4],[4,5]]
    x = Solution()
    print(x.merge(Input))

