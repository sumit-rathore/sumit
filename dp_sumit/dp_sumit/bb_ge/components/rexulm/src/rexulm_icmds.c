///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2015
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
//!   @file  - rexulm_icmds.c
//
//!   @brief - icmds for the rexulm component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <ibase.h>
#include <timing_timers.h>
#include "rexulm_log.h"
#include "rexulm_cmd.h"
#include "rexulm_loc.h"



/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

void REXULM_adjustPrefetchTimer(uint32 newTimeout)
{
#ifdef REXULM_NO_PREFETCH_DEVICE_SPEED
    ilog_REXULM_COMPONENT_0(ILOG_USER_LOG, UNSUPPORTED_ICMD);
#else
    ilog_REXULM_COMPONENT_2(
        ILOG_USER_LOG,
        CHANGING_PREFETCH_TIMER,
        TIMING_TimerGetTimeout(rex.preFetchDelayTimer),
        newTimeout);
    TIMING_TimerResetTimeout(rex.preFetchDelayTimer, newTimeout);
#endif
}


void REXULM_adjustConnectTimer(uint32 newTimeout)
{
    ilog_REXULM_COMPONENT_2(
        ILOG_USER_LOG,
        CHANGING_CONNECT_TIMER,
        TIMING_TimerGetTimeout(rex.disconnectTimer),
        newTimeout);
    TIMING_TimerResetTimeout(rex.disconnectTimer, newTimeout);
}


/**
* FUNCTION NAME: REXULM_showCurrentState()
*
* @brief  - icmd function to display the current state
*
* @return - void
*
* @note   - This isn't in IRAM.  Last measured at 7 opcodes on Rex.  Also we can't place in rextext
*           in case the user accidently runs on Lex.  So it would have to be .ftext, which we have
*           very little of.
*/
void REXULM_showCurrentState(void)
{
    if (GRG_IsDeviceRex())
    {
        _REXULM_logCurrentState(ILOG_USER_LOG);
    }
    else
    {
        ilog_REXULM_COMPONENT_0(ILOG_USER_LOG, THIS_IS_LEX);
    }
}
void REXULM_assertHook(void) __attribute__((alias("REXULM_showCurrentState")));
