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
//!   @file  -  itypes.h
//
//!   @brief -  standard types for all projects
//
//
//!   @note  - currently defines types for sparc, which probably apply to most 32bit processors
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ITYPES_H
#define ITYPES_H

/***************************** Included Headers ******************************/
#include <stddef.h> // This is to include size_t
#include <stdint.h>
#include <stdbool.h>

/******************************** Data Types *********************************/

// This could get moved to a Leon component or a MIPS or an ARM component

// NOTE: these integer typesdefs are deprecated. New code should use the exact-width
//       integer types from stdint.h.
// TODO: remove these typedefs once they're no longer needed.

// Standard type size definitions
typedef unsigned char           uint8;
typedef signed char             sint8;

typedef unsigned short          uint16;
typedef signed short            sint16;

typedef unsigned long           uint32;
typedef signed long             sint32;

typedef unsigned long long int  uint64;
typedef signed long long int    sint64;

// NOTE: the boolT typedef is deprecated. New code should use the "bool" type.
// TODO: remove the boolT typedef once it's no longer needed.
typedef bool boolT;


/************************ Defined Constants and Macros ***********************/

// NOTE: these typedefs are deprecated. New code should use the standard "true"
//       and "false" typedefs.
// TODO: remove these typedefs once they're no longer needed.
#define TRUE            ((boolT)(1 == 1))
#define FALSE           ((boolT)(1 == 0))

// Define the NULL pointer
#ifdef NULL
#undef NULL // This gets defined as an int in the MSP compiler stddef.h
#endif
#define NULL                   ((void *)0)

#endif //#ifndef ITYPES_H

