///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2014
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
//!   @file  -  imath.c
//
//!   @brief -  Arithmetic functions for LEON CPU
//
//!   @note  -  The current implementation of the LEON processor does not
//              have hardware support for division and multiplication.
//              Any software implementations of needed math
//              algorithms or functions should be placed here. All public
//              declarations should be placed in imath.h
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <imath.h>
#include "grg_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static uint16 _mostSigBitPosition(uint16 value);


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: GRG_int16Multiply()
*
* @brief  - Multiplies two unsigned 16 bit integers
*
* @return - The 32 bit result of the multiplication
*
* @note   - not deterministic - the more bits that are set in the multiplier,
*           the longer this routine will take.
*/
uint32 GRG_int16Multiply
(
     uint16 multiplier,
     uint16 multiplicand
)
{
    uint32 result = 0;
    uint32 shift;
    uint16 i;
    uint16 mask = 0x1;

    shift = multiplicand;

    for (i = 0; i < 15; i++)
    {
        if ((mask & multiplier) != 0)
        {
            result += shift;
        }

        mask = mask << 1;
        shift = shift << 1;
    }

    return (result);
}

/**
* FUNCTION NAME: GRG_int16Divide()
*
* @brief  - Divides two unsigned 16 bit integers
*
* @return - The quotient and remainder via the two pointers
*
* @note   - Execution time is linear in the number of bits in the input.
*           The worst case scenario is having the msb
*           of the numerator set - the first if clause will execute 16
*           times in that case.
*/
void GRG_int16Divide
(
    uint16 numerator,
    uint16 denominator,
    uint16 * quotient,
    uint16 * remainder
)
{
    uint16 i;
    uint16 loopCount;
    uint16 quot = 0;
    uint16 rem = 0;
    uint16 bitmask;

    // Divide by zero is fatal - everyone knows that
#ifndef FLASH_WRITER
    iassert_GRG_COMPONENT_0(denominator != 0, DIVIDE_BY_ZERO);
#endif

    if (numerator == 0)          // Zero divided by anything is going to be zero
    {
        *quotient = 0;
        *remainder = 0;
        return;
    }

    loopCount = _mostSigBitPosition(numerator);
    bitmask = 1 << loopCount;   // Start from the most significan bit of the numerator
    loopCount++;

    for (i = 0; i < loopCount; i++)
    {
        rem = rem << 1;
        if ((numerator & bitmask) != 0)
        {
            rem |= 1; // Set the lsb of the remainder if this bit of the numerator is 1
        }
        if (rem >= denominator)
        {
            rem = rem - denominator;
            quot |= bitmask;
        }
        bitmask = bitmask >> 1;
    }

    *quotient = quot;
    *remainder = rem;
}

/**
* FUNCTION NAME: _mostSigBitPosition()
*
* @brief  - Figures out the size (in bit positions) of a number
*
* @return - The bit position of the last set bit
*
* @note   - Used by integer divide - for example a number 0100b
*           will return 2 - bit 2 is the last set bit in the
*           number - basically it removes leading zeroes at the
*           bit level.
*/
static uint16 _mostSigBitPosition(uint16 value)
{
    uint16 mask = 1; // Start from the lsb
    uint16 count = 0;
    uint16 i;

    for (i = 0; i < 16; i++)
    {
        if ((mask & value) != 0)
        {
            count = i;
        }
        mask = mask << 1;
    }

    return (count);
}

/**
* FUNCTION NAME: icmdDivide()
*
* @brief  - This is an icmd for performing a division
*
* @return - Sends out an ilog
*
* @note   - Bound to the icmd resp divide16bit
*/
void icmdDivide(uint16 numerator, uint16 denominator)
{
    uint16 result;
    uint16 remainder;

    if (denominator == 0)
    {
        ilog_GRG_COMPONENT_1(ILOG_USER_LOG, INVALID_ICMD_ARG, 0);
    }
    else
    {
        GRG_int16Divide(numerator, denominator, &result, &remainder);
        ilog_GRG_COMPONENT_2(ILOG_USER_LOG, INT16_DIVIDE, result, remainder);
    }
}

/**
* FUNCTION NAME: icmdMultiply()
*
* @brief  - This is an icmd for multiplication
*
* @return - Sends out an ilog
*
* @note   - Bound to the icmd resp multiply16bit
*/
void icmdMultiply(uint16 multiplier, uint16 multiplicand)
{
    uint32 result;

    result = GRG_int16Multiply(multiplier, multiplicand);
    ilog_GRG_COMPONENT_1(ILOG_USER_LOG, INT16_MULTIPLY, result);
}

