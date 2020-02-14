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
//!   @file  -  ipool.c
//
//!   @brief -  memory pool routines
//
//
//!   @note  -  ipool.h implements the bulk of the work as static inlines
//              This only implements the initialization
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <ipool.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _IPOOL_init()
*
* @brief  - Initializes a memory pool.  Specifically the internal free pointer list
*
* @return - the head of the free pointer list
*
* @note   - Caller ensures validity of arguments, including numberOfElements != 0
*           Works closely with PREFIX_poolInit()
*/
struct _ipool_free_list * _IPOOL_init
(
    void* mempool,              // Pointer to a memory pool
    uint32 elementSize,         // Size of each element
    uint32 numberOfElements     // Number of elements in the memory pool
)
{
    // Cast each element of the mempool to a struct _ipool_free_list and initialize the "next"
    // pointer element to point to the next element of the pool.  The final element of the memory
    // pool will point to NULL.
    struct _ipool_free_list* it = mempool;
    while (numberOfElements > 1)
    {
        it->next = (struct _ipool_free_list *)((uint8 *)it + elementSize);
        it = it->next;
        numberOfElements--;
    }
    it->next = NULL;

    // The head of the list is just the beginning of mempool.
    return mempool;
}

