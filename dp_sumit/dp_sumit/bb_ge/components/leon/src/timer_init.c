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
#include "leon_regs.h"

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
* @return - void
*
* @note   - timer is 1MHz.  This is an assumption made all over this file
*
*/
void LEON_TimerInit(void)
{
    // Initialize the timer system prescaler to tick @ 1MHz
    WriteLeonRegister( LEON_PRESCALER_RELOAD_OFFSET, 59);   // this is 60 clock cycles for 1microsecond.  The register write is 1 less than clock cycles -- DOCUMENTED INCORRECTLY
    WriteLeonRegister( LEON_PRESCALER_COUNTER_OFFSET, 0 );

    // Init timer1 to wrap at the max timer value
    WriteLeonRegister( LEON_TIMER1_COUNTER_OFFSET, 0 );
    WriteLeonRegister( LEON_TIMER1_RELOAD_OFFSET, LEON_TIMER1_COUNTER_MASK); // Max value
    WriteLeonRegister( LEON_TIMER1_CONTROL_OFFSET,
        LEON_TIMER1_CONTROL_EN_BIT_MASK | LEON_TIMER1_CONTROL_RL_BIT_MASK | LEON_TIMER1_CONTROL_LD_BIT_MASK);

    // Init timer2 to trigger at 1ms
    WriteLeonRegister( LEON_TIMER2_COUNTER_OFFSET, 0);
    WriteLeonRegister( LEON_TIMER2_RELOAD_OFFSET, 999);//this makes the timer tick a 1ms tick - value is prescaler ticks minus 1
    WriteLeonRegister( LEON_TIMER2_CONTROL_OFFSET,
        LEON_TIMER2_CONTROL_EN_BIT_MASK | LEON_TIMER2_CONTROL_RL_BIT_MASK | LEON_TIMER2_CONTROL_LD_BIT_MASK \
            | LEON_TIMER2_CONTROL_INT_CLR_BIT_MASK | LEON_TIMER2_CONTROL_INT_EN_BIT_MASK);
}


