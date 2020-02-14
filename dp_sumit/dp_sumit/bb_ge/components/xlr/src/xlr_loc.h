///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
//!   @file  -  xlr_loc.h
//
//!   @brief -  Local header file for the XLR component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XLR_LOC_H
#define XLR_LOC_H

/***************************** Included Headers ******************************/
// Exposed header files for this component
#include <xlr.h>

// Spectareg access
#include <xlr_mmreg_macro.h>

// Interrupt control
#include <leon_traps.h>
#include <interrupts.h>

// Timer control
#include <timing_timers.h>

// ilog file for this component
#include "xlr_log.h"


/************************ Defined Constants and Macros ***********************/
// Define the base addresses for the GRG Asic component
#define XLR_BASE_ADDR   (uint32)(0x400)   // 0x20000400

/******************************** Data Types *********************************/

/*********************************** API *************************************/

// stats.c
void _XLR_flowControlCheck(void) __attribute__ ((section(".lextext")));
void _XLR_errorCountCheck(void) __attribute__ ((section(".lextext")));

// lex.c
void _XLR_fatalInterruptHandler(void) __attribute__((noreturn));

// xlr_msa.c
void _XLR_msaInit(void);

#endif // XLR_LOC_H

