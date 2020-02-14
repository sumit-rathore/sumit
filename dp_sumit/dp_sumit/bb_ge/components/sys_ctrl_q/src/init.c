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
//!   @file  -  init.c
//
//!   @brief -  Initialization for the system control queue component
//
//
//!   @note  -  This component reads everything from the system control queue
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "sys_ctrl_q_loc.h"
#include <leon_traps.h>
#include <interrupts.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: SYSCTRLQ_Init()
*
* @brief  - Initialization function
*
* @return - void
*
* @note   -
*
*/
void SYSCTRLQ_Init(void)
{
    ilog_SYS_CTRL_Q_COMPONENT_0(ILOG_DEBUG, INIT);

    // setup default logging level
    ilog_SetLevel(ILOG_MAJOR_EVENT, SYS_CTRL_Q_COMPONENT);

    // Register ISR
    LEON_InstallIrqHandler(IRQ_XCSR0, &_SYSCTRLQ_ISR);

    // Enable interrupt in XCSR & LEON
    XCSR_XUSBEnableSystemQueueInterrupts();
    LEON_EnableIrq(IRQ_XCSR0);
}

