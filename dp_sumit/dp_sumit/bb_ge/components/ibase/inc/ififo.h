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
//!   @file  -  ififo.h
//
//!   @brief -  Provides APIs for working with fifos
//
//
//!   @note  -  Currently exports a macro which creates static variables & functions
//
///////////////////////////////////////////////////////////////////////////////
#ifndef IFIFO_H
#define IFIFO_H

/***************************** Included Headers ******************************/
#include <itypes.h>

/*********************************** API *************************************/
// Exported C API:
// Scroll down to IFIFO_CREATE_FIFO_LOCK_UNSAFE()

/****************************** Documentation ********************************/

// NOTE
// ----
// 1) These are not fail safe functions!!!!
//    They are helper functions that operate directly on the fifo
//    DO NOT CALL fifoRead OR fifoWrite WITHOUT CALLING fifoFull, fifoEmpty, OR fifoSpaceAvail FIRST
// 2) Ensure locks are held, or interrupts are disabled when calling fifo functions
//    IE: between a fifoEmpty() call and a fifoRead() call, there needs to be guarantee of non-interruption

// Object oriented nature
// ----------------------
// This could easily be a C++ template class, but it differs to provide more optimization and reduce stack calls (needed when on the LEON)
// The equivalent C++ would be (please note: my C++ is a little rusty)
//template <typename element_type>
//class ififo {
//    private:
//        element_type * fifo;
//        uint32 fifoWriteIndex;
//        uint32 fifoReadIndex;
//        uint32 fifoSize;
//    public:
//        ififo(uint32 fifoSizeArg);
//        ~ififo(void);
//        boolT fifoEmpty(void);
//        boolT fifoFull(void);
//        uint32 fifoSpaceAvail(void);
//        uint32 fifoSpaceUsed(void);
//        void fifoWrite(element_type);
//        void fifoOverwrite(element_type);
//        elment_type fifoRead(void);
//        elment_type fifoPeekRead(void);
//        elment_type * fifoPeekReadPtr(void);
//        elment_type * fifoPeekLastWritePtr(void);
//};
//
// To implement this in C with efficiently:
//  * the template is replaced with a macro
//  * instead of creating on the fly on the stack, this is created in bss as a file scope variable/structure
//      * constructor/destructor are now gone as the array in .bss
//  * C could implement the public functions with function pointers, but
//      * there would be no inline optimizations possible
//      * this would cause another function call on the stack (which is very limited on the LEON, as the window overflow is not working/implemented)
//      * so instead static inline functions are declared within this file scope
//
//  If this was a C++ style class there is an option to provide a lock/unlock virtual method that higher level code could implement
//  However this isn't possible in the current form without adding function pointers for C use

// Implementation details
// ----------------------
// * Currently the fifo maintain 2 array index pointers.
// * When they are the same the fifo is empty.
// * The write pointer is never allowed to catch up the read pointer.  fifoFull checks for this
// * As we are using array index pointers, they both start at 0.
//  * The write pointer will now get ahead of the read pointer after a write
//  * After elements are read, the read pointer will catch up to the write pointer and the fifo will be empty
// * Once a pointer reaches the max value, it will wrap back to 0
// * Anytime the write pointer is less than the read pointer, this indicates the write pointer has wrapped, but not the read pointer
// * When the write pointer is 1 less then the read pointer, then the fifo is considered full, even though there is room for 1 more entry
//  * This is done to simplify the fifoEmpty function to just check if the pointers are the same

// Future enhancements
//--------------------
// 1) TODO: Could use uint8 or uint16 for size and index pointers to save space, pass in as extra argument
// 2) Could be changed to real libary calls that operate on a struct {
//    uint32 size; uint32 writeIndex; uint32 readIndex; uint8 fifo[_size_]};
//    where _size_ would be defined in an initialization build time macro
//    or runtime init function if we support malloc
//    See the above section on a C++ implementation.  The drawback is another stack window is needed
// 3) Could provide a slightly higher API, which takes care of the unsafe bits: locking & empty/full conditions
// 4) Better support for critical sections
//  As the user is grabbing locks. and releasing locks we need to ensure that the compiler doesn't optimize the accesses, such that an access moves outside of the locking section
//  This used to be a given when everything was volatile, but that is an overkill, as the index's won't change between a call to fifoEmpty and the subsequent call to fifoWrite
//  Solution:
//   We can provide a C sequence point (sequence points are from the C standard that describe volatile ordering behaviour), by having the user call the following
//   acquire_lock(..);       //Seqeunce point: locking mechanism for whatever OS is in place or disables interrupts
//   fifo_prefix_fifoOrder();//Sequence point: calls volatile asm() with no statements, but it has a clobber attribute of the memory of the struct fifo_prefix
//   {
//      // Critical section
//      // code that calls the fifo API's to check if fifo's are empty/full, then writes/reads, etc.
//   }
//   fifo_prefix_fifoOrder();//Sequence point: calls volatile asm() with no statements, but it has a clobber attribute of the memory of the struct fifo_prefix
//   release_lock(..);       //Seqeunce point: locking mechanism for whatever OS is in place or disables interrupts
//
//  If this was C++ the acquire_lock() method from a child class could chain up with the fifoOrder() from the base class
// 5) If the element_type was a struct, it might be easier to work with pointers than full elements
//    This would mean new functions:
//      element_type * fifoWritePtr(void);
//      element_type * fifoReadPtr(void);
//    In this scenario the fifoReadPtr() function would return the address of the element, as it is popped out of the fifo
//    The caller must extract all the struct data fields, before doing any fifo write commands, as that could overwrite that location
//    The fifoWritePtr() function updates the fifo index's right away, and then returns the pointer to the element in the fifo
//    The caller must populate all the struct data fields, before any fifo read commands, as that could overwrite that location




/***************************** Macro Definition ******************************/

#ifdef __MSP430__
//Hack to ensure no optimization are done for __builtin_popcount
#define __builtin_popcount(x) 0
#endif

// Create API for operating on a local fifo confined to a single C file
// NOTE: the argument fifo_size, is the size of the fifo, but the fifo never fills the last entry
//       Space available is actually one less than fifo_size
#define IFIFO_CREATE_FIFO_LOCK_UNSAFE(fifo_prefix, element_type, fifo_size) \
static inline boolT          fifo_prefix ## _fifoEmpty(void)            __attribute__((always_inline));\
static inline boolT          fifo_prefix ## _fifoFull(void)             __attribute__((always_inline));\
static inline uint32         fifo_prefix ## _fifoSpaceAvail(void)       __attribute__((always_inline));\
static inline uint32         fifo_prefix ## _fifoSpaceUsed(void)        __attribute__((always_inline));\
static inline void           fifo_prefix ## _fifoWrite(element_type)    __attribute__((always_inline));\
static inline element_type   fifo_prefix ## _fifoRead(void)             __attribute__((always_inline));\
static inline void           fifo_prefix ## _fifoOverwrite(element_type)__attribute__((always_inline));\
static inline element_type   fifo_prefix ## _fifoPeekRead(void)         __attribute__((always_inline));\
static inline element_type * fifo_prefix ## _fifoPeekReadPtr(void)      __attribute__((always_inline));\
static inline element_type * fifo_prefix ## _fifoPeekLastWritePtr(void) __attribute__((always_inline));\
static struct {                                                                             \
    element_type fifo[fifo_size];                                                           \
    uint32 fifoWriteIndex;                                                                  \
    uint32 fifoReadIndex;                                                                   \
} fifo_prefix;                                                                              \
/*                                                                              */          \
/* FUNCTION NAME: fifoEmpty()                                                   */          \
/*                                                                              */          \
/* @brief  - Checks if the fifo is empty                                        */          \
/*                                                                              */          \
/* @return - boolT : TRUE/FALSE.  Answers question "Is the fifo empty?"         */          \
/*                                                                              */          \
static inline boolT fifo_prefix ## _fifoEmpty(void)                                         \
{                                                                                           \
    return (fifo_prefix.fifoWriteIndex == fifo_prefix.fifoReadIndex);                       \
}                                                                                           \
/*                                                                              */          \
/* FUNCTION NAME: fifoFull()                                                    */          \
/*                                                                              */          \
/* @brief  - Checks if the fifo is full                                         */          \
/*                                                                              */          \
/* @return - boolT : TRUE/FALSE.  Answers question "Is the fifo full?"          */          \
/*                                                                              */          \
static inline boolT fifo_prefix ## _fifoFull(void)                                          \
{                                                                                           \
    return  (   ((fifo_prefix.fifoReadIndex == 0)                                           \
                && (fifo_prefix.fifoWriteIndex == (fifo_size - 1)))                         \
            ||  (fifo_prefix.fifoReadIndex == (fifo_prefix.fifoWriteIndex + 1)));           \
}                                                                                           \
/*                                                                              */          \
/* FUNCTION NAME: fifoSpaceAvail()                                              */          \
/*                                                                              */          \
/* @brief  - Checks how much space is available on the fifo                     */          \
/*                                                                              */          \
/* @return - The number of elements available                                   */          \
/*                                                                              */          \
static inline uint32 fifo_prefix ## _fifoSpaceAvail(void)                                   \
{                                                                                           \
    sint32 spread = fifo_prefix.fifoReadIndex - fifo_prefix.fifoWriteIndex;                 \
                                                                                            \
    if (spread > 0)                                                                         \
    {                                                                                       \
        /* spread indicates how many elements are left                          */          \
        /* except the fact that we can't fill it                                */          \
        return (uint32)(spread - 1);                                                        \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        /* spread indicates the negative of how many elements are used          */          \
        return (uint32)(fifo_size + spread -1);                                             \
    }                                                                                       \
}                                                                                           \
/*                                                                              */          \
/* FUNCTION NAME: fifoSpaceUsed()                                               */          \
/*                                                                              */          \
/* @brief  - Checks how much space is used on the fifo                          */          \
/*                                                                              */          \
/* @return - The number of elements used                                        */          \
/*                                                                              */          \
static inline uint32 fifo_prefix ## _fifoSpaceUsed(void)                                    \
{                                                                                           \
    sint32 spread = fifo_prefix.fifoWriteIndex - fifo_prefix.fifoReadIndex;                 \
                                                                                            \
    if (spread >= 0)                                                                        \
    {                                                                                       \
        /* spread indicates how many elements are used                          */          \
        return (uint32)(spread);                                                            \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        /* spread indicates the negative of how many elements are used          */          \
        return (uint32)(fifo_size + spread);                                                \
    }                                                                                       \
}                                                                                           \
/*                                                                              */          \
/* FUNCTION NAME: fifoWrite()                                                   */          \
/*                                                                              */          \
/* @brief  - Writes to the fifo                                                 */          \
/*                                                                              */          \
/* @return - void                                                               */          \
/*                                                                              */          \
/* @note   - UNSAFE: Always call fifoFull or fifoSpaceAvail before calling fifoWrite */     \
/*           As fifoWrite doesn't check if there is space available             */          \
/*                                                                              */          \
/*           TODO: Could change name to fifoWriteUnsafe to emphasize this       */          \
/*                                                                              */          \
static inline void fifo_prefix ## _fifoWrite                                                \
(                                                                                           \
    element_type newItem   /* The item to write to the fifo                     */          \
)                                                                                           \
{                                                                                           \
    fifo_prefix.fifo[fifo_prefix.fifoWriteIndex] = newItem;                                 \
                                                                                            \
    /* Update write index pointer, and handle the wrap if it reaches the end */             \
    fifo_prefix.fifoWriteIndex++;                                                           \
    if (__builtin_popcount(fifo_size) == 1)                                                 \
    {                                                                                       \
        /* The gcc built in has counted only 1 binary 1 in fifo_size */                     \
        /* Therefore subtracting 1 will reveal a mask of all the lower bits */              \
        /* Instead of checking for the max value, we can just mask out the upper bits */    \
        const uint32 fifo_mask = fifo_size - 1;                                             \
        fifo_prefix.fifoWriteIndex &= fifo_mask;                                            \
    }                                                                                       \
    else if (fifo_prefix.fifoWriteIndex == fifo_size)                                       \
    {                                                                                       \
        fifo_prefix.fifoWriteIndex = 0;                                                     \
    }                                                                                       \
}                                                                                           \
/*                                                                              */          \
/* FUNCTION NAME: fifoRead()                                                    */          \
/*                                                                              */          \
/* @brief  - Reads an element off the fifo                                      */          \
/*                                                                              */          \
/* @return - The element read                                                   */          \
/*                                                                              */          \
/* @note   - UNSAFE: Always call fifoEmpty before calling fifoRead              */          \
/*           As fifoRead doesn't check if there is anything on the fifo         */          \
/*                                                                              */          \
/*           TODO: Could change name to fifoReadUnsafe to emphasize this        */          \
/*                                                                              */          \
static inline element_type fifo_prefix ## _fifoRead(void)                                   \
{                                                                                           \
    element_type readElement = fifo_prefix.fifo[fifo_prefix.fifoReadIndex];                 \
                                                                                            \
    /* Update read index pointer, and handle the wrap if it reaches the end */              \
    fifo_prefix.fifoReadIndex++;                                                            \
    if (__builtin_popcount(fifo_size) == 1)                                                 \
    {                                                                                       \
        /* The gcc built in has counted only 1 binary 1 in fifo_size */                     \
        /* Therefore subtracting 1 will reveal a mask of all the lower bits */              \
        /* Instead of checking for the max value, we can just mask out the upper bits */    \
        const uint32 fifo_mask = fifo_size - 1;                                             \
        fifo_prefix.fifoReadIndex &= fifo_mask;                                             \
    }                                                                                       \
    else if (fifo_prefix.fifoReadIndex == fifo_size)                                        \
    {                                                                                       \
        fifo_prefix.fifoReadIndex = 0;                                                      \
    }                                                                                       \
                                                                                            \
    return readElement;                                                                     \
}                                                                                           \
/*                                                                              */          \
/* FUNCTION NAME: fifoOverwrite()                                               */          \
/*                                                                              */          \
/* @brief  - Writes to the fifo, and overwrites oldest element if fifo is full  */          \
/*                                                                              */          \
/* @return - void                                                               */          \
/*                                                                              */          \
/* @note   - Intent is for debug, so after assert recent events are available   */          \
/*                                                                              */          \
static inline void fifo_prefix ## _fifoOverwrite                                            \
(                                                                                           \
    element_type newItem   /* The item to write to the fifo                     */          \
)                                                                                           \
{                                                                                           \
    if (fifo_prefix ## _fifoFull())                                                         \
    {                                                                                       \
        /* Remove oldest data, so there is more room on fifo */                             \
        fifo_prefix ## _fifoRead();                                                         \
    }                                                                                       \
                                                                                            \
    fifo_prefix ## _fifoWrite(newItem);                                                     \
}                                                                                           \
/*                                                                              */          \
/* FUNCTION NAME: fifoPeekRead()                                                */          \
/*                                                                              */          \
/* @brief  - Peeks at the next element to be read                               */          \
/*                                                                              */          \
/* @return - The next element to be read                                        */          \
/*                                                                              */          \
/* @note   - UNSAFE: Always call fifoEmpty before calling fifoPeekRead          */          \
/*           As fifoPeekRead doesn't check if there is anything on the fifo     */          \
/*                                                                              */          \
/*           TODO: Could change name to fifoPeekReadUnsafe to emphasize this    */          \
/*                                                                              */          \
static inline element_type fifo_prefix ## _fifoPeekRead(void)                               \
{                                                                                           \
    return fifo_prefix.fifo[fifo_prefix.fifoReadIndex];                                     \
}                                                                                           \
/*                                                                              */          \
/* FUNCTION NAME: fifoPeekReadPtr()                                             */          \
/*                                                                              */          \
/* @brief  - Peeks at the next element to be read                               */          \
/*                                                                              */          \
/* @return - The pointer to the next element to be read                         */          \
/*                                                                              */          \
/* @note   - This is useful for struct element_type's, where the caller wants   */          \
/*           to update the state of the next element, but not process it yet    */          \
/*                                                                              */          \
/*           UNSAFE: Always call fifoEmpty before calling fifoPeekReadPtr       */          \
/*           As fifoPeekReadPtr doesn't check if there is anything on the fifo  */          \
/*                                                                              */          \
/*           TODO: Could change name to fifoPeekReadPtrUnsafe to emphasize this */          \
/*                                                                              */          \
static inline element_type * fifo_prefix ## _fifoPeekReadPtr(void)                          \
{                                                                                           \
    return &fifo_prefix.fifo[fifo_prefix.fifoReadIndex];                                    \
}                                                                                           \
/*                                                                              */          \
/* FUNCTION NAME: fifoPeekLastWritePtr()                                        */          \
/*                                                                              */          \
/* @brief  - Peeks at the last element to be written                            */          \
/*                                                                              */          \
/* @return - The pointer to the next element to be read                         */          \
/*                                                                              */          \
/* @note   - This is useful for data that was written to the fifo, and not yet  */          \
/*           processed, where the writer, now has more information to add       */          \
/*                                                                              */          \
/*           Intended for debugging where a running log is kept of the steps    */          \
/*           the program has gone through.  If the most recent step is repeated */          \
/*           then it doesn't need to be doubly reported, or it could be updated */          \
/*                                                                              */          \
/*           UNSAFE: Always call fifoEmpty before calling fifoPeekLastWritePtr  */          \
/*           As fifoPeekLastWritePtr doesn't check if there is anything valid   */          \
/*                                                                              */          \
/*           TODO: Change name to fifoPeekLastWritePtrUnsafe to emphasize this? */          \
/*                                                                              */          \
static inline element_type * fifo_prefix ## _fifoPeekLastWritePtr(void)                     \
{                                                                                           \
    uint32 lastWriteIndex;                                                                  \
    if (__builtin_popcount(fifo_size) == 1)                                                 \
    {                                                                                       \
        /* The gcc built in has counted only 1 binary 1 in fifo_size */                     \
        /* Therefore subtracting 1 will reveal a mask of all the lower bits */              \
        /* Instead of checking for the max value, we can just mask out the upper bits */    \
        const uint32 fifo_mask = fifo_size - 1;                                             \
        lastWriteIndex = fifo_prefix.fifoWriteIndex - 1;                                    \
        lastWriteIndex &= fifo_mask;                                                        \
    }                                                                                       \
    else if (fifo_prefix.fifoWriteIndex == 0)                                               \
    {                                                                                       \
        lastWriteIndex = fifo_size - 1;                                                     \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        lastWriteIndex = fifo_prefix.fifoWriteIndex - 1;                                    \
    }                                                                                       \
    return &fifo_prefix.fifo[lastWriteIndex];                                               \
}

#endif // IFIFO_H

