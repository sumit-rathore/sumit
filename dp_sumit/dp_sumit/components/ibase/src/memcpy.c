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
//!   @file  -  memcpy.c
//
//!   @brief -  Provides the ANSI C library memcpy function
//
//
//!   @note  -  The C compiler assumes memcpy always exists so it may use it in optimizations
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
* FUNCTION NAME: memcpy()
*
* @brief  - C standard memcpy
*
* @return - A pointer to the destination address
*
* @note   - The C compiler assumes memcpy always exists so it may use it in optimizations
*
*/
void * memcpy
(
    void * dst,         // Destination to copy to
    const void * src,   // Source to copy from
    size_t num          // Number of bytes to copy
)
{
    uint32_t       * curDstWord = dst;
    const uint32_t * curSrcWord = src;
    uint8_t        * curDstByte;
    const uint8_t  * curSrcByte;

    // If this is word aligned, lets do word copies
    if ((((size_t)dst & 0x3) | ((size_t)src & 0x3)) == 0)
    {
        while (num >> 2)
        {
            *curDstWord = *curSrcWord;
            curDstWord++;
            curSrcWord++;
            num -= 4;
        }
    }

    // If there are any bytes left, or if it wasn't word aligned to start
    // Update the byte pointers for a byte copy
    curDstByte = (uint8_t *)curDstWord;
    curSrcByte = (uint8_t *)curSrcWord;

    // Do a byte copy
    while (num)
    {
        *curDstByte = *curSrcByte;
        curDstByte++;
        curSrcByte++;
        num--;
    }

    return dst;
}

//#################################################################################################
// A slower, flash based memcpy intended to be used when copying to IRAM
//
// Parameters:
//      dst             - destination pointer
//      src             - source pointer
//      num             - number of bytes to copy
// Return:
// Assumptions:
//
//#################################################################################################
void * flashmemcpy
(
    void * dst,         // Destination to copy to
    const void * src,   // Source to copy from
    size_t num          // Number of bytes to copy
)
{
    uint32_t * curDstWord = dst;
    const uint32_t * curSrcWord = src;
    uint8_t * curDstByte;
    const uint8_t * curSrcByte;

    // If this is word aligned, lets do word copies
    if ((((size_t)dst & 0x3) | ((size_t)src & 0x3)) == 0)
    {
        while (num >> 2)
        {
            *curDstWord = *curSrcWord;
            curDstWord++;
            curSrcWord++;
            num = num - 4;
        }
    }

    // If there are any bytes left, or if it wasn't word aligned to start
    // Update the byte pointers for a byte copy
    curDstByte = (uint8_t *)curDstWord;
    curSrcByte = (uint8_t *)curSrcWord;

    // Do a byte copy
    while (num)
    {
        *curDstByte = *curSrcByte;
        curDstByte++;
        curSrcByte++;
        num--;
    }

    return dst;
}
