///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009
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
//!   @file  -  leon_timers.h
//
//!   @brief -  Contains the API for using the Leon timers
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LEON_TIMERS_H
#define LEON_TIMERS_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <ibase.h>
//#include <_leon_reg_access.h>
//#include <leon2_regs.h>
//#include <leon_regs.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

// Abstract type for a time value
struct _LEON_TimerValue;
typedef struct _LEON_TimerValue * LEON_TimerValueT;


/*********************************** API *************************************/
// For initializing the HW
bool LEON_TimerInit(void) __attribute__((section(".atext")));

// Working with delays
void LEON_TimerWaitMicroSec(uint32_t) __attribute__ ((section (".ftext")));

void LEON_clearTimerInterrupt(void);

// Measuring time.
// Need to force inline even though it is a single opcode.  Bad compiler, very bad.
LEON_TimerValueT LEON_TimerRead(void);
uint32_t LEON_TimerCalcUsecDiff(
    LEON_TimerValueT oldTimeValue, LEON_TimerValueT newTimeValue);

uint8_t LEON_TimerGetClockTicksLSB(void) __attribute__ ((section (".ftext")));
uint32_t LEON_TimerGetClockTicks(void) __attribute__ ((section (".ftext")));


/******************************* Static inline definitions ********************************/


#endif // LEON_TIMERS_H

