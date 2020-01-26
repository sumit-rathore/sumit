#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Table size. Should be a prime number that is roughly 2x the
// number of expected records for optimal performance.
#define SIZE 50000

// Our record table definition, an array of M records.
Record record_table[SIZE];


typedef struct
{
    int key;
    int value;
} *Record

static int hash(int key)
{
    return key % SIZE;
}

static void init(void)
{
	int index;
	for (index = 0; i < SIZE; ++index)
    {
		record_table[index] = NULL;
	}
}

static void insert(Record record)
{
   int index = hash(record->key);
   while (record_table[index] != NULL)
   {
      index = (index + 1) % SIZE;
   }
   record_table[index] = record;
}

Record search(int key) {
   int index = hash(key);
   while (record_table[i] != NULL) {
      if (key == record_table[i]->key) {
         return record_table[i];
      } else {
         i = (i + 1) % M;
      }
   }
   return NULL;
}

int main()
{
    return 0;
}
