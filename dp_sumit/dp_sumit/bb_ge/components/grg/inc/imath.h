///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010 - 2014
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
//!  @file  -  imath.h
//
//!  @brief -  Include file for math functions
//
//!  @note  -  The LEON soft CPU does not have hardware multiply or divide 
//             support. So we have hand-coded some basic integer multiply
//             and divide routines.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef IMATH_H
#define IMATH_H


/***************************** Included Headers ******************************/
#include <itypes.h>


/*************************** Function declarations ***************************/

void GRG_int16Divide(uint16 numerator, uint16 denominator, uint16 * quotient, uint16 * remainder);

uint32 GRG_int16Multiply(uint16 multiplier, uint16 multiplicand);

#endif
