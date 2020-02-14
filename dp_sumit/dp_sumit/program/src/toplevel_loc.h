///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  toplevel_loc.h
//
//!   @brief -  Top level local header file
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TOPLEVEL_LOC_H
#define TOPLEVEL_LOC_H

/***************************** Included Headers ******************************/
// Toplevel Project exposed headers
#include <options.h>
#include <interrupts.h>

// Low level utilities

#include <timing_timers.h>
#include <main_loop.h>
// Drivers
#include <bb_chip_regs.h>
#include <bb_top_regs.h>
#include <leon2_regs.h>
#include <leon_timers.h>
#include <leon_traps.h>
#include <mdio.h>

// Higher level components
#include <iassert.h>
#include <icmd.h>

// Logging/Asserting/Debugging
#include "toplevel_log.h"
#include "toplevel_cmd.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/
void BBTop_si5326IrqDoneHandler(void);
void BB_interruptInit(void);
void reInitializeGmiiMca(void) __attribute__ ((section(".atext")));
void deInitializeGmiiMca(void) __attribute__ ((section(".atext")));

#endif // TOPLEVEL_LOC_H

