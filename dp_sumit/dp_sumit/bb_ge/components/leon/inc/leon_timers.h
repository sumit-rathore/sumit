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
#include <_leon_reg_access.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

// Abstract type for a time value
struct _LEON_TimerValue;
typedef struct _LEON_TimerValue * LEON_TimerValueT;


/*********************************** API *************************************/
// For initializing the HW
void LEON_TimerInit(void);

// Working with delays
void LEON_TimerWaitMicroSec(uint32) __attribute__ ((section (".ftext")));

void LEON_clearTimerInterrupt(void);

// Measuring time.
// Need to force inline even though it is a single opcode.  Bad compiler, very bad.
static inline LEON_TimerValueT LEON_TimerRead(void) __attribute__ ((always_inline));
static inline uint32 LEON_TimerCalcUsecDiff(
    LEON_TimerValueT oldTimeValue, LEON_TimerValueT newTimeValue);

uint8 LEON_TimerGetClockTicksLSB(void) __attribute__ ((section (".ftext")));


/******************************* Static inline definitions ********************************/

/**
* FUNCTION NAME: LEON_TimerRead()
*
* @brief  - Reads the current time
*
* @return - An abstract type to the user LEON_TimerValueT
*
* @note   - The user needs to call this module for operations to LEON_TimerValueT
*
*           Exposed here since it is only a single opcode
*/
static inline LEON_TimerValueT LEON_TimerRead(void)
{
    // Keith K. took a look at the RTL, and for the counter register, the reserved bits are set to
    // 0.  No need to do a mask here for LG1.
    //
    // TODO: The comment above references LG1.  Is the statement also true for GE?  Where did the
    // commented out code below originate from?  Is there a strong reason to keep it?
    //
    //return Uint32ToTimerValue(ReadLeonRegister(LEON_TIMER1_COUNTER_OFFSET) & _LEON_TIMER1_COUNTER_MASK);
    return CAST(_LEON_ReadLeonRegister(_LEON_TIMER1_COUNTER_OFFSET), uint32, LEON_TimerValueT);
}


/**
* FUNCTION NAME: LEON_TimerCalcUsecDiff()
*
* @brief  - Calculates the difference between two time values
*
* @return - The time elapsed from the old time value to the new time value
*
* @note   - Exposed here since it is used by LG1 xmodem transfer to replace IRAM & in critical sections
*           So it must be in IRAM, and it can't be in IRAM.
*           If it is here it is just inlined into the caller
*/
static inline uint32 LEON_TimerCalcUsecDiff
(
    LEON_TimerValueT oldTimeValue,  // An older time value
    LEON_TimerValueT newTimeValue   // A newer time value
)
{
    uint32 oldTime = CAST(oldTimeValue, LEON_TimerValueT, uint32);
    uint32 newTime = CAST(newTimeValue, LEON_TimerValueT, uint32);

    // NOTE: The leon timer is a decrementor

    if (newTime > oldTime) // Check for timer wrap
    {
        // Timer has wrapped, increment the oldTime by the wrap value
        oldTime = oldTime + _LEON_TIMER1_COUNTER_MASK;
    }

    return oldTime - newTime;
}

#endif // LEON_TIMERS_H

