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
//!   @file  -  xrr_loc.h
//
//!   @brief -  Local header file for the XRR component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XRR_LOC_H
#define XRR_LOC_H

/***************************** Included Headers ******************************/
// Exposed header files for this component
#include <xrr.h>

// Spectareg access
#include <xrr_mmreg_macro.h>

// Interrupt control
#include <leon_traps.h>
#include <interrupts.h>

// Timer control
#include <timing_timers.h>

// ilog file for this component
#include "xrr_log.h"


/************************ Defined Constants and Macros ***********************/
// Define the base addresses for the GRG Asic component
#define XRR_BASE_ADDR   (uint32)(0x300)   // 0x20000300

/******************************** Data Types *********************************/

/*********************************** API *************************************/

// stats.c
void _XRR_FlowControlCheck(void) __attribute__ ((section(".rextext")));
void _XRR_ErrorCountCheck(void) __attribute__ ((section(".rextext")));

#endif // XRR_LOC_H


