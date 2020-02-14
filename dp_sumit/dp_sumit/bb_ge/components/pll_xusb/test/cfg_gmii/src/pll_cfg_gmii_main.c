///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013
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
//!   @file  -  pll_cfg_gmii_main.c
//
//!   @brief -  test for PLL cfg
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <interrupts.h>

#include <leon_uart.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <leon_traps.h>
#include <tasksch.h>

#include <pll.h>
#include <grg.h>

#include "pll_cfg_gmii_log.h"
#include "pll_cfg_gmii_cmd.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: imain()
*
* @brief  - test harness startup function
*
* @return - never
*
* @note   -
*
*/
void * imain(void)
{
    irqFlagsT flags;
    uint8 dummy;

    // Lockout interrupts for the initialization code
    flags = LEON_LockIrq();

     // Configure the uart
    LEON_UartSetBaudRate115200();
    LEON_InstallIrqHandler(IRQ_UART_TX, LEON_UartInterruptHandlerTx);
    LEON_EnableIrq(IRQ_UART_TX);

    //init the timers
    LEON_TimerInit();
    LEON_InstallIrqHandler(IRQ_TIMER2, TIMING_TimerInterruptHandler);
    LEON_EnableIrq(IRQ_TIMER2);

    // Is Icmd Needed?
    ICMD_Init();
    LEON_InstallIrqHandler(IRQ_UART_RX, LEON_UartInterruptHandlerRx);
    LEON_EnableIrq(IRQ_UART_RX);

    // Initialize the task scheduler
    TASKSCH_Init();

    // Initialize the PLL's
    GRG_Init(&dummy, &dummy, &dummy);
    PLL_Init();

    // Tell the world that we have started
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();

    // Run the test
    PLL_CfgGMII(FALSE);

    // Start the interupts
    LEON_UnlockIrq(flags);

    // Run the main loop
    return &TASKSCH_MainLoop;
}

