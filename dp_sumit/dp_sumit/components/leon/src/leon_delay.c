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
//!   @file  -  leon_delay.c
//
//!   @brief -  handles blocking for a short delay
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

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: LEON_TimerWaitMicroSec()
*
* @brief  - Blocks for the number of useconds requested
*
* @return - void
*
* @note   - This could be a static inline exposed to the user
*
*/
void LEON_TimerWaitMicroSec
(
    uint32_t delay    // Number of useconds to block for
)
{
    LEON_TimerValueT origTimerValue = LEON_TimerRead();

    while (LEON_TimerCalcUsecDiff(origTimerValue, LEON_TimerRead()) < delay)
        ;
}



