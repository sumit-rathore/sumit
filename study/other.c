// INCLUDE HEADER FILES NEEDED BY YOUR PROGRAM
// SOME LIBRARY FUNCTIONALITY MAY BE RESTRICTED
// DEFINE ANY FUNCTION NEEDED
#include <stdio.h>  // for printing
#include <stdlib.h> // for malloc
#include <string.h> // for memcpy

#define SetBit(number,bit) (number|=(1<<bit-1))		
 
#define ClearBit(number,bit) (number&=(~(1<<bit-1)))
 
#define Toggle(number,bit) (number ^ (1<<bit-1))

/* Apply the constructor attribute to StartupFunction() so that it
    is executed before main() */
void StartupFunction(void) __attribute__ ((constructor));
 
 
/* Apply the destructor attribute to CleanupFunction() so that it
   is executed after main() */
void CleanupFunction(void) __attribute__ ((destructor));

// Inline functions are similar to macros because they both are expanded at compile time,
// but the macros are expanded by the pre-processor, while inline functions are parsed by the compiler.
// The following two are same but inline is preferred when define takes arguments
#define square(x) x*x
inline int square(int x) { return x*x; }

// Standard Macros
   printf("Current File :%s\n", __FILE__ );
   printf("Current Date :%s\n", __DATE__ );
   printf("Current Time :%s\n", __TIME__ );
   printf("Line Number :%d\n", __LINE__ );

// The typedef interpretation is performed by the compiler where as
// #define statements are processed by the pre-processor.

// #define will just copy-paste the definition values at the point of use,
// while typedef is actual definition of a new type


/******* MEMORY LAYOUT **********
This memeory layout is organized in following fashion:
Low to High
Text segment (executable code, machine language)
Data segment (uninitialized and initialized)
Heap segment (dynamic memory)
Stack segment (local variables)
Unmapped or reserved (commandline argument and environment variables)
*/

// Convert char to int
char c = '5';
int x = c - '0';
