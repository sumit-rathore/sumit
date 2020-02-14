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
//!   @file  -  timers.c
//
//!   @brief -  Contains code for the timer isr handling
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <leon_timers.h>
#include <options.h>
#include <leon2_regs.h>
#include <leon_regs.h>
#include <_leon_reg_access.h>
#include <bb_core.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: LEON_clearTimerInterrupt()
*
* @brief  - Provides a convenient way to clear the timer interrupt.
*
* @return - void.
*/
void LEON_clearTimerInterrupt(void)
{
    // Clear the interrupt interrupt.  No need for a R-M-W as we just need to leave it initialized
    // Cannot use LEON_WRITE_BF macro because bitfields in TIMER2_CONTROL don't
    // have MASK and OFFSET associated with each bitfield
    LEON_WRITE_BIT(LEON2_TIMERS_TIMER2_CONTROL, _RL, 1);
    LEON_WRITE_BIT(LEON2_TIMERS_TIMER2_CONTROL, _EN, 1);
}

uint8_t LEON_TimerGetClockTicksLSB(void)
{
    const uint32_t clkFreq = bb_core_getCpuClkFrequency() / 0xF4240; // convert freq to us
    const uint32_t microSeconds = LEON_READ_BF(LEON2_TIMERS_TIMER1_COUNTER, _COUNTER);
    const uint32_t preScaler = LEON_READ_BF(LEON2_TIMERS_PRESCALER_COUNTER, _COUNTER);
    const uint32_t cycles = preScaler + (clkFreq * microSeconds); //NOTE: the compiler does shifts instead of multiply

    return cycles & 0xFF; // Return just the least signficant byte
}

uint32_t LEON_TimerGetClockTicks(void)
{
    const uint32_t clkFreq = bb_core_getCpuClkFrequency() / 0xF4240; // convert freq to us
    const uint32_t microSeconds = LEON_READ_BF(LEON2_TIMERS_TIMER1_COUNTER, _COUNTER);
    const uint32_t preScaler = LEON_READ_BF(LEON2_TIMERS_PRESCALER_COUNTER, _COUNTER);
    const uint32_t cycles = preScaler + (clkFreq * microSeconds); //NOTE: the compiler does shifts instead of multiply

    return cycles;
}

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
LEON_TimerValueT LEON_TimerRead(void)
{
    // Keith K. took a look at the RTL, and for the counter register, the reserved bits are set to
    // 0.  No need to do a mask here for LG1.
    //
    // TODO: The comment above references LG1.  Is the statement also true for GE?  Where did the
    // commented out code below originate from?  Is there a strong reason to keep it?
    //
    //return Uint32ToTimerValue(ReadLeonRegister(LEON_TIMER1_COUNTER_OFFSET) & _LEON_TIMER1_COUNTER_MASK);
    return CAST(LEON_READ_BF(LEON2_TIMERS_TIMER1_COUNTER, _COUNTER), uint32_t, LEON_TimerValueT);
}


/**
* FUNCTION NAME: LEON_TimerCalcUsecDiff()
*
* @brief  - Calculates the difference between two time values
*
* @return - The time elapsed from the old time value to the new time value, in microseconds
*
* @note   - Exposed here since it is used by LG1 xmodem transfer to replace IRAM & in critical sections
*           So it must be in IRAM, and it can't be in IRAM.
*           If it is here it is just inlined into the caller
*/
uint32_t LEON_TimerCalcUsecDiff
(
    LEON_TimerValueT oldTimeValue,  // An older time value
    LEON_TimerValueT newTimeValue   // A newer time value
)
{
    uint32_t oldTime = CAST(oldTimeValue, LEON_TimerValueT, uint32_t);
    uint32_t newTime = CAST(newTimeValue, LEON_TimerValueT, uint32_t);

    // NOTE: The leon timer is a decrementor

    if (newTime > oldTime) // Check for timer wrap
    {
        // Timer has wrapped, increment the oldTime by the wrap value
        oldTime = oldTime + LEON2_TIMERS_TIMER1_COUNTER_COUNTER_MASK;
    }

    return oldTime - newTime;
}


