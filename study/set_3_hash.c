#include <stdio.h>
#include <stdlib.h> // for malloc

#define SIZE 50000

typedef struct{
    int a;
    int b;
    int c;
} *Record;

int hash(int key) {
    int r = key % SIZE;
    return r < 0 ? r + SIZE : r;
}

void appendRecord(Record r, int *arr, int index)
{
    arr[index] = r->a;
    arr[index+1] = r->b;
    arr[index+2] = r->c;
}

void insert(int *keys, int *values, int key, int value) {
    int index = hash(key);
    while (values[index]) {
        index = (index + 1) % SIZE;
    }
    keys[index] = key;
    values[index] = value;
}

int search(int *keys, int *values, int key) {
    int index = hash(key);
    while (values[index]) {
        if (keys[index] == key) {
            return values[index];
        }
        index = (index + 1) % SIZE;
    }
    return 0;
}

/* Swap Function
   Swaps input 1 and input 2
*/
void Swap(int *x, int *y)
{
    int temp = *x;
    *x = *y;
    *y = temp;
}

// Function to run quicksort on an array of integers
// l is the leftmost starting index, which begins at 0
// r is the rightmost starting index, which begins at array length - 1
void quicksort(int arr[], int l, int r)
{
    // Base case: No need to sort arrays of length <= 1
    if (l >= r)
    {
        return;
    }
    
    // Choose pivot to be the last element in the subarray
    int pivot = arr[r];

    // Index indicating the "split" between elements smaller than pivot and 
    // elements greater than pivot
    int cnt = l;

    // Traverse through array from l to r
    for (int i = l; i <= r; i++)
    {
        // If an element less than or equal to the pivot is found...
        if (arr[i] <= pivot)
        {
            // Then swap arr[cnt] and arr[i] so that the smaller element arr[i] 
            // is to the left of all elements greater than pivot
            Swap (&arr[cnt], &arr[i]);

            // Make sure to increment cnt so we can keep track of what to swap
            // arr[i] with
            cnt++;
        }
    }
    
    // NOTE: cnt is currently at one plus the pivot's index 
    // (Hence, the cnt-2 when recursively sorting the left side of pivot)
    quicksort(arr, l, cnt-2); // Recursively sort the left side of pivot
    quicksort(arr, cnt, r);   // Recursively sort the right side of pivot
}


int* threeSet(int* nums, int numsSize){
    int keys[SIZE];
    int values[SIZE] = {0};
    quicksort(nums, 0, numsSize-1);
    for (int i = 0; i < numsSize-1; i++) {
        int left = i+1;
        int right = numsSize -1;
        int sumTwo = (nums[i] + nums[i+1]) * -1;
        int value = search(keys, values, sumTwo);
        if (value) {
            int *ansSet = (int *) malloc(sizeof(int) * 3);
            ansSet[0] = nums[i];
            ansSet[1] = nums[i+1];
            ansSet[2] = value;
            return ansSet;
        }
        insert(keys, values, nums[i], i + 1);
    }
    return NULL;
}

int main()
{
    int nums[6] = {-1,0,1,2,-1,-4};
    int *ans;


    ans = threeSet(&nums[0], (sizeof(nums)/sizeof(nums[0])));
    printf("%d\n", ans[0]);
    printf("%d\n", ans[1]);
    printf("%d\n", ans[2]);
    return 0;
}
