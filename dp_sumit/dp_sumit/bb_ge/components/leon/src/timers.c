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
#include "leon_regs.h"

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
    WriteLeonRegister(
        LEON_TIMER2_CONTROL_OFFSET,
        (LEON_TIMER2_CONTROL_EN_BIT_MASK |
         LEON_TIMER2_CONTROL_RL_BIT_MASK |
         LEON_TIMER2_CONTROL_INT_CLR_BIT_MASK |
         LEON_TIMER2_CONTROL_INT_EN_BIT_MASK));
}

uint8 LEON_TimerGetClockTicksLSB(void)
{

    const uint32 microSeconds = _LEON_ReadLeonRegister(_LEON_TIMER1_COUNTER_OFFSET);
    const uint32 preScaler = _LEON_ReadLeonRegister(LEON_PRESCALER_COUNTER_OFFSET);
    const uint32 cycles = preScaler + (60 * microSeconds); //NOTE: the compiler does shifts instead of multiply

    return cycles & 0xFF; // Return just the least signficant byte
}

