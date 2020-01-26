/*************************************************
 *Given a non-empty array containing only positive integers,
 find if the array can be partitioned into two subsets such
 that the sum of elements in both subsets is equal.

Note:

Each of the array element will not exceed 100.
The array size will not exceed 200.
 

Example 1:

Input: [1, 5, 11, 5]

Output: true

Explanation: The array can be partitioned as [1, 5, 5] and [11].
 

Example 2:

Input: [1, 2, 3, 5]

Output: false

Explanation: The array cannot be partitioned into equal sum subsets.

**************************************************/

#include<stdio.h>

#define MAX_ARRAY_SIZE 100
#define MAX_ARRAY_ELEMENT 200

// we want it to partition into two subsets
// that makes it a bit easy
//
// sort the list into ascending order
// max sum = 200 * 100 / 2 = 10000
//
// if 10000 - sizeof half list = 0
// divide it right in the middle
//
// ALSO, the total_sum % 2 == 1 must not be true
// if the total can't be divided by 2, it can't be divided into lists

int list[] = {1, 5, 11, 5};

int length = (int) sizeof(list) / sizeof(list[0]);
// Assume list length is even, if not, add zero as a member to make it even
int halfIndex = lenght/2;


// keep on adding the smallest item to the biggest item till the result is
// equal to HALF SUM
// if greater than HALF_SUM, exit with false



