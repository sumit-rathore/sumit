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

#include <grg.h>
#include <atmel_crypto.h>
#include <leon_uart.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <leon_traps.h>
#include <main_loop.h>

#include "atmel_mac_log.h"
#include "atmel_mac_cmd.h"


/**************************** External Definitions ***************************/
extern const struct ATMEL_KeyStore ATMEL_secretKeyStore;

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static uint8 testChallengeBuf[ATMEL_MAC_CHALLENGE_SIZE];

/************************ Local Function Prototypes **************************/
static void macTestDone(boolT success);

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
    uint8 major;
    uint8 minor;
    uint8 debug;
    irqFlagsT flags;

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

    // Initialize GRG
    GRG_Init(&major, &minor, &debug);
    LEON_InstallIrqHandler(IRQ_GRG, GRG_InterruptHandler);
    LEON_EnableIrq(IRQ_GRG);

    // Initialize the Atmel crypto chip
    ATMEL_init(NULL);

    // Tell the world that we have started
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();

    // Start the interupts
    LEON_UnlockIrq(flags);

    // Run the main loop
    return &MainLoop;
}


void writeChallengeBuf(uint8 index, uint8 byte)
{
    if (index < ATMEL_MAC_CHALLENGE_SIZE)
    {
        ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, WRITE_BUF, index, byte);
        testChallengeBuf[index] = byte;
    }
    else
    {
        ilog_TEST_HARNESS_COMPONENT_1(ILOG_USER_LOG, WRITE_BUF_INVALID_INDEX, index);
    }
}

void showChallengeBuf(void)
{
    uint8 index;

    for (index = 0; index < ATMEL_MAC_CHALLENGE_SIZE; index++)
    {
        const uint8 byte = testChallengeBuf[index];
        ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, SHOW_BUF, index, byte);
    }
}

void runMacTest(void)
{
    if (ATMEL_runMac(ATMEL_secretKeyStore.key[0], testChallengeBuf, &macTestDone))
    {
        // Submitted
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, SUBMITTED_MAC_CMD);
    }
    else
    {
        // Could not run
        ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, SUBMITTED_MAC_CMD_FAILED);
    }
}

static void macTestDone(boolT success)
{
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, success ? MAC_TEST_SUCCESS : MAC_TEST_FAILED);
}

