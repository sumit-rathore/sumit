///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -
//
//!   @brief -
//
//
//!   @note  -  There is an assumption in this file that the timer is set to
//              1MHz.  If this changes, only this file needs to be updated &
//              timers.c
//
//              Also it assumed in timers.c that the timer2 isr is at 1ms ticks
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <leon_timers.h>
#include <leon2_regs.h>
#include <leon_regs.h>
#include <interrupts.h>
#include <bb_core.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: LEON_TimerInit()
*
* @brief  - Initializes the timer hardware
*
* @return - bool (true: success, false: fail)
*
* @note   - timer is 1MHz.  This is an assumption made all over this file
*
*/
bool LEON_TimerInit(void)
{
    // Initialize the timer system prescaler to tick @ 1MHz
    // this is 60 clock cycles for 1microsecond.  The register write is 1 less than clock cycles -- DOCUMENTED INCORRECTLY
    LEON_WRITE_BF(LEON2_TIMERS_PRESCALER_RELOAD, _RELOAD, (bb_core_getCpuClkFrequency() / 1000000) - 1);

    LEON_WRITE_BF(LEON2_TIMERS_PRESCALER_COUNTER, _COUNTER, 0);

    // Init timer1 to wrap at the max timer value
//    LEON_WRITE_BF(LEON2_TIMERS_TIMER1_COUNTER, _COUNTER, 0); // don't do this, ld will negate
//    this write anyhow
    LEON_WRITE_BF(LEON2_TIMERS_TIMER1_RELOAD, _RELOAD, LEON2_TIMERS_TIMER1_RELOAD_RELOAD_MASK); // max value

    LEON_WRITE_BIT(LEON2_TIMERS_TIMER1_CONTROL, _LD, 1);
    LEON_WRITE_BIT(LEON2_TIMERS_TIMER1_CONTROL, _RL, 1);
    LEON_WRITE_BIT(LEON2_TIMERS_TIMER1_CONTROL, _EN, 1);

    // Init timer2 to trigger at 1ms
    LEON_WRITE_BF(LEON2_TIMERS_TIMER2_COUNTER, _COUNTER, 0);
    LEON_WRITE_BF(LEON2_TIMERS_TIMER2_RELOAD, _RELOAD, 999);//this makes the timer tick a 1ms tick - value is prescaler ticks minus 1

    LEON_WRITE_BIT(LEON2_TIMERS_TIMER2_CONTROL, _LD, 1);
    LEON_WRITE_BIT(LEON2_TIMERS_TIMER2_CONTROL, _RL, 1);
    LEON_WRITE_BIT(LEON2_TIMERS_TIMER2_CONTROL, _EN, 1);

//    LEON_EnableIrq(IRQ_TIMER2);

// Read Timer1 Value
// There is very rare case Timer1 doesn't work. So Test it before passing this routine
// Usually, it passes while loop under count value 3
    uint32_t start = LEON_READ_BF(LEON2_TIMERS_TIMER1_COUNTER, _COUNTER);
    uint32_t count = 0;
    while(start == LEON_READ_BF(LEON2_TIMERS_TIMER1_COUNTER, _COUNTER))
    {
        count++;
        if(count > 100)
        {
            return false;       // Indicating failure
        }
    }

    return true;
}


