/*
 * For a given array, find a set where
 * a2 + b2 = c2
 * input = {3,1,4,6,5}
 * output = 3,4,5
 *
 * input2 = {10,4,6,12,5}
 * output2 = false -- it doesn't exist
 *
 * Simple method is to use 3 loops with O(n3)
 * Using a hash, we can bring it down to O(max*max)
 * where max = maximum element in the array
*/

/*
 *Method 2 (Use Sorting)
We can solve this in O(n2) time by sorting the array first.

1) Do square of every element in input array. This step takes O(n) time.

2) Sort the squared array in increasing order. This step takes O(nLogn) time.

3) To find a triplet (a, b, c) such that a2 = b2 + c2, do following.


Fix ‘a’ as last element of sorted array.
Now search for pair (b, c) in subarray between first element and ‘a’.
A pair (b, c) with given sum can be found in O(n) time using meet in
middle algorithm discussed in method 1 of this post.
If no pair found for current ‘a’, then move ‘a’ one position back and repeat step 3.2.
*/

#include <stdio.h> 
  
// Function to check if the 
// Pythagorean triplet exists or not 
bool checkTriplet(int arr[], int n) 
{ 
    int maximum = 0; 
  
    // Find the maximum element 
    for (int i = 0; i < n; i++) { 
        maximum = max(maximum, arr[i]); 
    } 
  
    // Hashing array 
    int hash[maximum + 1] = { 0 }; 
  
    // Increase the count of array elements 
    // in hash table 
    for (int i = 0; i < n; i++) 
        hash[arr[i]]++; 
  
    // Iterate for all possible a 
    for (int i = 1; i < maximum + 1; i++) { 
  
        // If a is not there 
        if (hash[i] == 0) 
            continue; 
  
        // Iterate for all possible b 
        for (int j = 1; j < maximum + 1; j++) { 
  
            // If a and b are same and there is only one a 
            // or if there is no b in original array 
            if ((i == j && hash[i] == 1) || hash[j] == 0) 
                continue; 
  
            // Find c 
            int val = sqrt(i * i + j * j); 
  
            // If c^2 is not a perfect square 
            if ((val * val) != (i * i + j * j)) 
                continue; 
  
            // If c exceeds the maximum value 
            if (val > maximum) 
                continue; 
  
            // If there exists c in the original array, 
            // we have the triplet 
            if (hash[val]) { 
                return true; 
            } 
        } 
    } 
    return false; 
} 
// Driver Code 
int main() 
{ 
    int arr[] = { 3, 2, 4, 6, 5 }; 
    int n = sizeof(arr) / sizeof(arr[0]); 
    if (checkTriplet(arr, n)) 
        cout << "Yes"; 
    else
        cout << "No"; 
} 
