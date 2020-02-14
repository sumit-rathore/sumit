///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  -  ipool.h
//
//!   @brief -  Provides APIs for working with memory pools
//
//
//!   @note  -  Currently exports a macro which creates static variables & functions
//
///////////////////////////////////////////////////////////////////////////////
#ifndef IPOOL_H
#define IPOOL_H

/***************************** Included Headers ******************************/
#include <ibase.h>

/****************************** Internal Types *******************************/
//Internal list for keeping track of free pointers.  DO NOT USE, INTERNAL ONLY
struct _ipool_free_list { struct _ipool_free_list * next; };

/*********************************** API *************************************/

//Internal function
struct _ipool_free_list * _IPOOL_init (void * mempool, uint32_t elementSize, uint32_t numberOfElements);

// Create API for operating on a local memory pool confined to a single C file

// Exported API:
// static inline void PREFIX_poolInit(void);
// static inline elementType * PREFIX_poolAlloc(void);
// static inline void PREFIX_poolFree(elementType *);
// static inline uint32_t PREFIX_poolPtrToIndex(elementType *);
// static inline elementType * PREFIX_poolIndexToPtr(uint32_t index);
// static inline uint32_t PREFIX_poolGetNumOfUsedElements();
// static inline uint32_t PREFIX_poolGetNumOfFreeElements();

// NOTE:
// Ensure locks are held, or interrupts are disabled when calling ipool functions
// Ensure the PREFIX_poolInit() function is called before calling any PREFIX_poolAlloc()
//
// If sizeof(elementType), is not an exponent of 2, PREFIX_poolPtrToIndex will be inefficient

// Implementation details:
// ipool keeps track of the free elements as a linked list.  Where
// each element of the linked list is stored at the location of a
// free element.
//
// Each time a new elment is requested with the alloc function, ipool pops the
// head off of the linked list of free elements
//
// Each time an element is freed it is just pushed onto the linked list of
// free elements
//
// The Init function, creates the free linked list, and verifies the macro
// arguments

// TODO: could add a debug version which contains a function pointer for asserts
// This can't be done here, as ibase is the lowest level component and below the assert code
// This could be called in the poolFree and the poolPtrToIndex functions for invalid pointers
// The high level code would then do the asserts, and logging.
// This might actually be more useful in the high level code, as the high level code, would
// assert that the topology_ipool had a failure, and not a generic ipool that could be
// anywhere in the code

// TODO: We currently don't use a mempool and O(n) search an array for a free element, because
// debugging is easier to dump the whole array on an assert.  Not easy with a mempool, as there
// is no linked list of allocated elements
// TODO: Could add a new type, iboolarray, which would be an array of all allocated elements
// Then we could have *_poolFree() checks
// We could also have an API to take (*)() arg, and call for each allocated element.
// Useful for RexSch:rexmsa.c to (un)pause all transactions on flow control

// Memory Pool Creation Macro
#define IPOOL_CREATE(ipool_prefix, elementType, numberOfElements)                           \
static struct _ipool_free_list * ipool_prefix ## _free_list_head;                           \
static elementType ipool_prefix ## _pool[numberOfElements] __attribute__ ((aligned(4)));    \
static uint32_t ipool_prefix ## _ipoolNumOfUsedElements;                                    \
                                                                                            \
/*                                                                                      */  \
/* FUNCTION NAME: PREFIX_poolInit()                                                     */  \
/*                                                                                      */  \
/* @brief  -    Initialize a memory pool                                                */  \
/*                                                                                      */  \
/* @return -    void                                                                    */  \
/*                                                                                      */  \
static inline void ipool_prefix ## _poolInit(void)                                          \
{                                                                                           \
    /* Verify the type can hold a ptr for our free list */                                  \
    COMPILE_TIME_ASSERT(sizeof(elementType) >= sizeof(struct _ipool_free_list));            \
    COMPILE_TIME_ASSERT(numberOfElements != 0);                                             \
                                                                                            \
    /* Initialize the free pointer list */                                                  \
    ipool_prefix ## _free_list_head =                                                       \
        _IPOOL_init(ipool_prefix ## _pool, sizeof(elementType), numberOfElements);          \
    ipool_prefix ## _ipoolNumOfUsedElements = 0;                                            \
}                                                                                           \
                                                                                            \
/*                                                                                      */  \
/* FUNCTION NAME: PREFIX_poolAlloc()                                                    */  \
/*                                                                                      */  \
/* @brief  -    Allocate a new elment from the pool                                     */  \
/*                                                                                      */  \
/* @return -    new element, or NULL on failure (mempool is full)                       */  \
/*                                                                                      */  \
static inline elementType * ipool_prefix ## _poolAlloc(void)  __attribute__((always_inline)); \
static inline elementType * ipool_prefix ## _poolAlloc(void)                                \
{                                                                                           \
    struct _ipool_free_list * old_head = ipool_prefix ## _free_list_head;                   \
                                                                                            \
    if (old_head)                                                                           \
    {                                                                                       \
        ipool_prefix ## _free_list_head = old_head->next;                                   \
        ipool_prefix ## _ipoolNumOfUsedElements++;               \
                                                                                            \
        return CAST(old_head, struct _ipool_free_list *, elementType *);                    \
    }                                                                                       \
                                                                                            \
    return NULL;                                                                            \
}                                                                                           \
                                                                                            \
/*                                                                                      */  \
/* FUNCTION NAME: PREFIX_poolFree()                                                     */  \
/*                                                                                      */  \
/* @brief  -    Free an element and return it back to the pool                          */  \
/*                                                                                      */  \
/* @return -    void                                                                    */  \
/*                                                                                      */  \
/* @note   -    There is no check to see if an element is actually inside the pool      */  \
/*              There is no check to see if an element was allocated, or returned twice */  \
/*                                                                                      */  \
static inline void ipool_prefix ## _poolFree(elementType * e)   __attribute__((always_inline)); \
static inline void ipool_prefix ## _poolFree(elementType * e)                               \
{                                                                                           \
    struct _ipool_free_list * old_head = ipool_prefix ## _free_list_head;                   \
    ipool_prefix ## _free_list_head = CAST(e, elementType *, struct _ipool_free_list *);    \
    ipool_prefix ## _free_list_head->next = old_head;                                       \
    ipool_prefix ## _ipoolNumOfUsedElements--;                                              \
}                                                                                           \
                                                                                            \
/*                                                                                      */  \
/* FUNCTION NAME: PREFIX_poolPtrToIndex()                                               */  \
/*                                                                                      */  \
/* @brief  -    Convert a 4 byte pointer into a shorter index into memory pool array    */  \
/*                                                                                      */  \
/* @return -    uint32_t index into memory pool array                                   */  \
/*                                                                                      */  \
/* @note   -    The return value can't be larger than the # of elements in the pool so  */  \
/*              it is safe to assume uint8_t when there are fewer than 256 entries and  */  \
/*              it is safe to assume uint16_t when there are fewer than 65536 entries.  */  \
/*                                                                                      */  \
static inline uint32_t ipool_prefix ## _poolPtrToIndex(elementType * e) __attribute__((always_inline, const)); \
static inline uint32_t ipool_prefix ## _poolPtrToIndex(elementType * e)                      \
{                                                                                           \
    if (e == NULL)                                                                          \
    {                                                                                       \
        return 0;                                                                           \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        return                                                                              \
                (       (uint32_t)(e)                            /* Address of element */   \
                    -   ((uint32_t)(&ipool_prefix ## _pool[0]))) /* Address of mempool */   \
            /   sizeof(elementType) + 1;                                                    \
    }                                                                                       \
}                                                                                           \
                                                                                            \
/*                                                                                      */  \
/* FUNCTION NAME: PREFIX_poolIndexToPtr()                                               */  \
/*                                                                                      */  \
/* @brief  -    Convert a memory pool index into a pointer to that entry                */  \
/*                                                                                      */  \
/* @return -    pointer to the element at specified index                               */  \
/*                                                                                      */  \
/* @note   -    See notes on PREFIX_poolPrtToIndex()                                    */  \
/*                                                                                      */  \
static inline elementType * ipool_prefix ## _poolIndexToPtr(uint32_t index) __attribute__((always_inline, const)); \
static inline elementType * ipool_prefix ## _poolIndexToPtr(uint32_t index)                   \
{                                                                                           \
    if (index == 0)                                                                         \
    {                                                                                       \
        return NULL;                                                                        \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        return &ipool_prefix ## _pool[index - 1];                                           \
    }                                                                                       \
}                                                                                           \
                                                                                            \
/*                                                                                      */  \
/* FUNCTION NAME: PREFIX_poolGetNumOfUsedElements()                                     */  \
/*                                                                                      */  \
/* @brief  -    Return the number of elements that have been used                       */  \
/*                                                                                      */  \
/* @return -    the number of elements that have been used                              */  \
/*                                                                                      */  \
/* @note   -                                                                            */  \
/*                                                                                      */  \
static inline uint32_t ipool_prefix ## _poolGetNumOfUsedElements()                          \
{                                                                                           \
    return ipool_prefix ## _ipoolNumOfUsedElements;                                         \
}                                                                                           \
                                                                                            \
/*                                                                                      */  \
/* FUNCTION NAME: PREFIX_poolGetNumOfFreeElements()                                     */  \
/*                                                                                      */  \
/* @brief  -    Return the number of elements that have been freed                      */  \
/*                                                                                      */  \
/* @return -    the number of elements that have been freed                             */  \
/*                                                                                      */  \
/* @note   -                                                                            */  \
/*                                                                                      */  \
static inline uint32_t ipool_prefix ## _poolGetNumOfFreeElements()                          \
{                                                                                           \
    return numberOfElements - (ipool_prefix ## _ipoolNumOfUsedElements);                    \
}


#endif // IPOOL_H

