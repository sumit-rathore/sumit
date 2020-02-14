///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  -  ram_test_main.c
//
//!   @brief -  test harness main file template for GE project
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
#include <iassert.h>

#include <grg_mmreg_macro.h>
#include <xcsr_mmreg_macro.h>
#include <grg.h>
#include <grg_gpio.h>
#include <leon_cpu.h>
#include <pll.h>
#include <xcsr_xicsq.h>
#include <xcsr_direct_access.h>
#include <xcsr_xsst.h>
#include <xlr_msa.h>
#include <xlr_mmreg_macro.h>

#include "pll_test_log.h"
#include "pll_test_cmd.h"

/************************ Defined Constants and Macros ***********************/
// Typically defined in grg_loc.h, but we don't have access to it
#define GRG_BASE_ADDR       (uint32)(0x000)       // 0x20000000

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
//static void* imain(void) __attribute__((noreturn));


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: imain()
*
* @brief  - test harness template startup function
*
* @return - never
*
* @note   -
*
*/
void * imain(void)
{
    irqFlagsT flags;
    enum linkType link;

    // Lockout interrupts for the initialization code
    flags = LEON_LockIrq();

     // Configure the uart
    LEON_UartSetBaudRate115200();
    LEON_EnableIrq(IRQ_UART_TX);

    //init the timers
    LEON_TimerInit();
    LEON_EnableIrq(IRQ_TIMER2);

    // Set GPIO 1 as output and initially clear.
    GRG_GPIO_OUT_WRITE_REG(GRG_BASE_ADDR, 0x00000000);
    GRG_GPIO_DIR_WRITE_REG(GRG_BASE_ADDR, 0x00000002);

    // Is Icmd Needed?
    /*
    ICMD_Init();
    iassert_Init(NULL, &ICMD_PollingLoop);
    LEON_EnableIrq(IRQ_UART_RX);
    */

    // Initialize the task scheduler
    TASKSCH_Init();

    PLL_Init();

    // Tell the world that we have started
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();

    link = GRG_GetLinkType();

    // Run the test
    switch (link)
    {
        case GMII:
            PLL_CfgGMII(FALSE);
            break;

        case RGMII:
            PLL_CfgGMII(TRUE);
            break;

        case MII:
        case MII_VALENS:
            PLL_CfgMII(FALSE);
            break;

        case TBI:
        case LTBI:
            PLL_CfgTbi(FALSE);
            break;

        case RTBI:
            PLL_CfgTbi(TRUE);
            break;

        case CLEI1:
        case CLEI2:
        case CLEI4:
        case CLEI8:
            PLL_CfgClei();
            break;

        default:
            PLL_CfgMII(FALSE);
            break;

        }

    // Set GPIO 1 high to indicate test is done
    GRG_GPIO_OUT_WRITE_REG(GRG_BASE_ADDR, 0x00000002);
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, TEST_COMPLETED);
    LEON_UartWaitForTx();

    // Start the interupts
    LEON_UnlockIrq(flags);

    // Run the main loop
    return &TASKSCH_MainLoop;
}
