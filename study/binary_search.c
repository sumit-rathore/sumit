#include <stdio.h>

/* Regular Seach
 * We go over all elements in an array
 * logic is O(n2) time complex as follows:
*/
int linearSearch(int *arr, int size, int x)
{
    for(int i=0; i<size; i++)
    {
        if(arr[i] == x)
        {
            return i;
        }
    }
    return -1;
}

/* Binary search
 * IT REQUIRES ARRAY TO BE SORTED
    1) Compare element to search with the middle element.
    2) If element matches with middle element, we return the mid index.
    3) Else If element is greater than the mid element, then element can only lie in right half sub array after the mid element. So we recur for right half.
    4) Else (element is smaller) recur for the left half.
*/
/* Driver Functions */
int BinarySearch_Iterative(int A[], int size, int element);
int BinarySearch_FirstOccurrence(int A[], int size, int element);
int BinarySearch_Recursive(int A[], int start, int end, int element);
 
int main()
{
#define size 9
    int arr[size] = {1,2,3,4,5,6,7,8,9};
    printf("element is located at: %d\n", linearSearch(&arr[0], size, 5));
    int A[] = {0,12,6,12,12,18,34,45,55,99};
	
	printf(" BinarySearch 55 at Index = %d \n",BinarySearch_Iterative(A,10,55));
	printf(" BinarySearch 17 at Index = %d \n",BinarySearch_Recursive(A,0,9,34));
	printf(" BinarySearch 12's first occurrence at Index = %d \n",BinarySearch_FirstOccurrence(A,9,12));
	
    return 0;
}

/* Recursive method */
int BinarySearch_Recursive(int A[], int start, int end, int element)
{
    // if start become greater than end, then element was not present
    if(start>end) return -1;

    // Calculating mid at every recursive call
    int mid = (start+end)/2;

    // Check if element is present at mid
    if( A[mid] == element )	return mid;

    // If element greater, ignore left half
    else if( element < A[mid] )	
        BinarySearch_Recursive(A, start, mid-1, element);

    // If element is smaller, ignore right half
    else 
        BinarySearch_Recursive(A, mid+1, end, element);
}

/* Binary search to find first or last occurrence of element */
int BinarySearch_FirstOccurrence(int A[], int size, int element)
{
	int result;
	int start = 0;
	int end = size-1;
	while(start<=end)
	{
		int mid = (start+end)/2;
		if( A[mid] ==  element)			// Check if element is present at mid
		{
			result = mid; 	
			end = mid - 1;				// Change this statement "start = mid + 1", for last occurrence
		}
		else if( element < A[mid] )	
			end = mid-1;				// If element greater, ignore left half
		
		else 
			start = mid+1;				// If element is smaller, ignore right half
	}
	return result;						// if we reach here, then return element's occurrence index
}
