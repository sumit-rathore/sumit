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
//!   @file  -  memset.c
//
//!   @brief -  Provides the ANSI C library memset function
//
//
//!   @note  -  The C compiler assumes memset always exists so it may use it in optimizations
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <ibase.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: memset()
*
* @brief  - C standard memset
*
* @return - A pointer to the destination address
*
* @note   - The C compiler assumes memset always exists so it may use it in optimizations
*
*/
void * memset
(
    void * dst, // Destination address
    int c,      // Byte to set
    size_t n    // Number of bytes to set
)
{
    uint8_t * curByteAddr = dst;
    uint8_t byte = (uint8_t)c;
    uint32_t word;

    // Initialize a word to do 32 memset operations.
    // This is done in 2 steps so type promotion will work with the shifts on 16bit architectures
    word = byte;
    word = (word) | (word << 8) | (word << 16) | (word << 24);

    // set bytes until we are word aligned
    while (n && (CAST(curByteAddr, uint8_t *, size_t) & 0x3))
    {
        *curByteAddr = byte;
        curByteAddr++;
        n--;
    }

    // set words while we can
    while (n >> 2)
    {
        // Cast the byte pointer to a word pointer, and write a word
        *(CAST(curByteAddr, uint8_t *, uint32_t *)) = word;

        curByteAddr += 4;
        n -= 4;
    }

    // Finish the setting for non-aligned end
    while (n)
    {
        *curByteAddr = byte;
        curByteAddr++;
        n--;
    }

    return dst;
}

//#################################################################################################
// A slower, flash based memset intended to be used when copying to IRAM
//
// Parameters:
//      dst             - destination pointer
//      c               - byte to set memory region to
//      n               - size of memory region to set
// Return:
// Assumptions:
//      * This function will work with a localized static variable to track operation state.
//      * Rather than have a series of functions as callbacks for all the
//      * writes, use this function update the state variable, permitting state
//      * machine state change operation
//#################################################################################################
void * flashmemset
(
    void * dst, // Destination address
    int c,      // Byte to set
    size_t n    // Number of bytes to set
)
{
    uint8_t * curByteAddr = dst;
    uint8_t byte = (uint8_t)c;
    uint32_t word;

    // Initialize a word to do 32 memset operations.
    // This is done in 2 steps so type promotion will work with the shifts on 16bit architectures
    word = byte;
    word = (word) | (word << 8) | (word << 16) | (word << 24);

    // set bytes until we are word aligned
    while (n && (CAST(curByteAddr, uint8_t *, size_t) & 0x3))
    {
        *curByteAddr = byte;
        curByteAddr++;
        n--;
    }

    // set words while we can
    while (n >> 2)
    {
        // Cast the byte pointer to a word pointer, and write a word
        *(CAST(curByteAddr, uint8_t *, uint32_t *)) = word;

        curByteAddr += 4;
        n -= 4;
    }

    // Finish the setting for non-aligned end
    while (n)
    {
        *curByteAddr = byte;
        curByteAddr++;
        n--;
    }

    return dst;
}
