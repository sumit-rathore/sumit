///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  test_main.c
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
#include <leon_traps.h>

#include "ilog_flood_log.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: imain()
*
* @brief  - ilog flood test harness
*
* @return - never
*
* @note   -
*
*/
void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint32 i = 0;

    // Configure the uart
    LEON_UartSetBaudRate115200();

    // Tell the world that we have started
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);

    // Flood ilogs
    while (TRUE)
    {
        LEON_UartWaitForTx(); /* ensure ilogs are actually transmitted, as LG1 only transfers on uart interrupt which is locked out */
        ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, ILOG_FLOOD, i);
        i++;
    }
}

