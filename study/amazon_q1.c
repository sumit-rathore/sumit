#include <stdio.h>  // for printing
#include <stdlib.h> // for malloc
#include <string.h> // for memcpy

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

int main()
{
    return 0;
}
