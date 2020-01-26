/**********************
Given an array of meeting time intervals consisting of start and end times
[[s1,e1],[s2,e2],...] (si < ei), 
find the minimum number of conference rooms required.

Example 1:

Input: [[0, 30],[5, 10],[15, 20]]
Output: 2


Example 2:

Input: [[7,10],[2,4]]
Output: 1


Find number of rooms for
[(1,10), (2,7), (3,19), (8, 12), (10, 20), (11, 30)]
****************************/


#include <stdio.h>

typedef enum {false, true} bool;

typedef struct{
    int startTime;
    int endTime;
} MeetingList;


int startTimeList[6]    = {1, 2, 3, 8, 10, 11};
int endTimeList[6]      = {10, 7, 19, 12, 20, 30};

bool needNewRoom (int newStartTime, int index)
{
    if(newStartTime < index)
        return false;

    return true;
};


int main(void)
{
    MeetingList meetingList[] = {{1, 10}, {2, 7}, {3, 9},
                                {8, 12}, {10, 20}, {11, 30}};


    int length = (int) sizeof(meetingList) / sizeof(meetingList[0]);

    printf("Length = %d\n", length);

    MeetingList *index = meetingList;

    int startIndex = 0;
    int endIndex = 0;
    int rooms = 0;
    int i = 0;
    while (i < length)
    {
        if(startTimeList[startIndex] < endTimeList[endIndex])
        {
            rooms ++;
            startIndex++;
        }
        else
        {
            endIndex++;
        }
        i ++;
    };

    printf("Total rooms required = %d\n", rooms);

    return true;
}

