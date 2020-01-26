// INCLUDE HEADER FILES NEEDED BY YOUR PROGRAM
// SOME LIBRARY FUNCTIONALITY MAY BE RESTRICTED
// DEFINE ANY FUNCTION NEEDED
#include <stdio.h>  // for printing
#include <stdlib.h> // for malloc
#include <string.h> // for memcpy

typedef enum{inactive, active} state;
typedef struct BoundedArray
{
    int size;
    int *arr;
}boundedarray;

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
    printf("\n\n");
}

// FUNCTION SIGNATURE BEGINS, THIS FUNCTION IS REQUIRED
boundedarray* cellCompete(int* states, int days) 
{
    boundedarray *cellCompeteArray;
    cellCompeteArray = (boundedarray*)malloc(sizeof(boundedarray));

    //int sizeStates = sizeof(states)/sizeof(states[0]);
    int sizeStates = sizeof(states);
    int origStates[sizeStates];

    for(int index = 0; index < days; index++)
    {
        memcpy(origStates, states, sizeof(origStates));
        printf("orig Array:\n");
        PrintArray(&origStates[0], sizeStates);

        states[0] = origStates[1] == active ? active : inactive;
        for(int i = 1; i < sizeStates - 1; i++)
        {
            // boundary cases are pending
            if(origStates[i-1] == origStates[i+1])
            {
                states[i] = inactive;
            }
            else
            {
                states[i] = active;
            }
        }
    }
    cellCompeteArray->size = sizeStates;
    cellCompeteArray->arr = states;
    return cellCompeteArray;
    // WRITE YOUR CODE HERE  
}


int main()
{
    int input[8]  = {1,0,0,0,0,1,0,0};
    //expected = 0 1 0 0 1 0 1 0
    int days = 1;
   
    printf("user input:\n");
    PrintArray(&input[0], (sizeof(input)/sizeof(input[0])));
    boundedarray *cellArray;
    cellArray = cellCompete(&input[0], days);

    printf("final answer:\n");
    printf("size = %d\n", cellArray->size);
    PrintArray(cellArray->arr, cellArray->size);

    return 0;
}
