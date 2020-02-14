///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2007, 2008
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
//!   @file  -  iassert.h
//
//!   @brief -  Ensures all assert macros/static inlines are available
//
//
//!   @note  -  real work is done in ilog.h as asserts are heavily dependant on
//              logging
//
///////////////////////////////////////////////////////////////////////////////
#ifndef IASSERT_H
#define IASSERT_H

/***************************** Included Headers ******************************/
// Runtime asserts functions are including in the ilog.h file as they are heavily dependant on the logging
#include <ilog.h>

/******************************** Data Types *********************************/
struct assertInfo_s
{
    uint32 header;
    uint32 arg1;
    uint32 arg2;
    uint32 arg3;
    uint8 assertCount; // should initialized to 0
};

typedef void (*assertHookEndFunctionT)(struct assertInfo_s * assertInfo) __attribute__ ((noreturn));

/*********************************** API *************************************/
void iassert_Init(void (*assertHookStartFunctionArg)(void), assertHookEndFunctionT assertHookEndFunctionArg);

#endif // #ifndef IASSERT_H

