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
//!   @file  -  imain.c
//
//!   @brief -  USB test mode test harness
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <leon_traps.h>
#include <leon_uart.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <ulm.h>
#include <grg.h>
#include <xlr.h>
#include <xcsr_xusb.h>
#include <xcsr_xicsq.h>

#include <xcsr_direct_access.h>

#include <interrupts.h>

#include "usb_test_mode_cmd.h"
#include "usb_test_mode_log.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void resetXUSBAndUlm(void);

/************************** Function Definitions *****************************/

void imain(void) __attribute__((noreturn));
void imain(void)
{
    irqFlagsT flags;

    // Lockout interrupts for the initialization code
    flags = LEON_LockIrq();

    // Configure the uart
    LEON_UartSetBaudRate115200();
    ICMD_Init();
    LEON_InstallIrqHandler(IRQ_UART_RX, LEON_UartInterruptHandlerRx);
    LEON_EnableIrq(IRQ_UART_RX);
    LEON_InstallIrqHandler(IRQ_UART_TX, LEON_UartInterruptHandlerTx);
    LEON_EnableIrq(IRQ_UART_TX);

    // Setup the timers -- icmd needs this
    LEON_TimerInit();
    LEON_InstallIrqHandler(IRQ_TIMER2, TIMING_TimerInterruptHandler);
    LEON_EnableIrq(IRQ_TIMER2);

    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UnlockIrq(flags);

    while (TRUE)
        ;
}

void TestGenJ(void)
{
    resetXUSBAndUlm();
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, TEST_GEN_J);
    ULM_SetTestMode();
    ULM_GenJ();
}

void TestGenK(void)
{
    resetXUSBAndUlm();
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, TEST_GEN_K);
    ULM_SetTestMode();
    ULM_GenK();
}

void TestSE0_NAK(void)
{
    resetXUSBAndUlm();
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, TEST_SE0_NAK);
    ULM_SetTestMode();
    XLR_NakAllRequests();
    XLR_lexEnable();
}

void TestPacket(void)
{
    uint8 qid;

    resetXUSBAndUlm();
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_USER_LOG, TEST_PACKET);

    // setup ulm for test mode
    ULM_SetTestMode();

    // Initialise cache
     XCSR_Init(GRG_IsDeviceLex(), TRUE); // Setup for direct link, we don't care about MAC addresses

    // allocate Q
    qid = XCSR_XICSQQueueAllocate(QT_DEFAULT);

    // set the QID tracker
    XLR_SetQueueTrakerQid(qid); //TODO: This is a bad name.  Bad name

    // write in response new dynamic q
    XCSR_QueueWriteTestModeFrame(qid);

    // Enable Lex
    XLR_lexEnable();

    // and go
    XLR_StartSendingTestModePacket();
}

static void resetXUSBAndUlm(void)
{
    GRG_Reset(XUSB_RESET);
    GRG_Reset(ULM_RESET);
    GRG_ClearReset(XUSB_RESET);
    GRG_ClearReset(ULM_RESET);
    //TODO: remove the next 2 lines when there is an XCSR function for this & include line way above
    XCSR_XUSB_MODEN_XUTMEN_WRITE_BF(XCSR_BASE_ADDR, 1);
    XCSR_XUSB_MODEN_XURMEN_WRITE_BF(XCSR_BASE_ADDR, 1);
}

