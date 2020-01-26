#ifndef HASHTABLE_H
#define HASHTABLE_H 1

typedef struct {
   int key;
   int value;
} *Record;

// Function Prototypes
void init(void);
void insert(Record);
Record search(int);
void delete(int);

#endif
