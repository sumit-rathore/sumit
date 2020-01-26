#include <stdio.h>

/* Swap Function
   Swaps input 1 and input 2
*/
void Swap(int *x, int *y)
{
    int temp = *x;
    *x = *y;
    *y = temp;
}

/* Print Array
   Prints the array
*/
void PrintArray(int *array, int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
}

/* Selection Sort
    Find the minimum element of the list and swap it
    Easiest way to find the smallest or the largest element
*/
void SelectionSort(int A[], int size)
{
    int i;
    int j;
    int min;

    for(i = 0; i < size; i++)
    {
        min = i;
        for(j = i+1; j <size; j++)
        {
            if(A[j] < A[i])
            {
                min = j;
            }
        }
        // Swapping the element now
        Swap(&A[min], &A[i]);
    }
}

/* Bubble Sort

    Swap the adjacent elements -- the largest one ends up to their
    right
*/
void BubbleSort(int A[], int size)
{
    for(int i=0; i < size; i++)
    {
        for(int j=0; j < size; j++)
        {
            if(A[j] > A[j+1])
            {
               // Swap(&A[min], &A[i]);
            }
        }
    }
}

/* Insertion Sort
    Shift elements one by one
*/
void InsertionSort(int A[], int size)
{
	for(int i=1; i<size; i++)
	{
		int value = A[i];
		int hole = i;
		while( hole>0 && A[hole-1]>value)
		{
			A[hole] = A[hole-1];
			hole--;
		}
		A[hole] = value ;
	}
}

/* Merge Sort
 * Divide and conquer algorithm or Merge two arrays
*/
void Merge(int A[], int L[], int R[], int L_len, int R_len)
{
	int i=0,j=0,k=0;
	
	while( i<L_len && j<R_len )
	{
		if( L[i] <= R[j] )
		{
			A[k] = L[i];
			i++;
		}
		else
		{
			A[k] = R[j];
			j++;
		}
		k++;
	}
	while( i<L_len)
	{
		A[k] = L[i];
		i++;k++;
	}
	while( j<R_len )
	{
		A[k] = R[j];
		j++;k++;
	}
}
 
void MergeSort(int A[],int size)
{
	if(size<2)return;
	int L_len = size/2;
	int R_len = size - L_len;
	
	int L[L_len],R[R_len];
	
	for(int i=0;i<L_len;i++)
	{
		L[i] = A[i];
	}
	for(int j=0;j<R_len;j++)
	{
		R[j] = A[j+L_len];
	}
	
	MergeSort(L,L_len);
	MergeSort(R,R_len);
	Merge(A,L,R,L_len,R_len);
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

int main()
{
    int A[9] = {1,4,6,2,3,7,9,8,5};
    int sizeA = sizeof(A) / sizeof(A[0]);
    PrintArray(&A[0], sizeA);

    // Selection Sort
    //SelectionSort(&A[0], sizeA);
    quicksort(&A[0], 0, sizeA -1);
    PrintArray(&A[0], sizeA);

    return 0;
}
