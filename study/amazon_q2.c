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
typedef struct BoundedArray
{
    int size;
    int *arr;
}boundedarray;

struct integarpair
{
    int first;
    int second;
    struct integerpair *next;
};

typedef struct integerpair integerPair;

boundedarray* criticalRouters(int numRouters, int numLinks, int** links)
{
    boundedarray *criticalRout;
    criticalRout = (boundedarray*)malloc(sizeof(boundedarray));

    int routes[numRouters];

    for(int i = 0; i < numLinks; i++)
    {

    }
    criticalRout->size = sizeof(routes)/sizeof(routes);
    criticalRout->arr = routes;
    return criticalRout;
}

int main()
{
    return 0;
}
