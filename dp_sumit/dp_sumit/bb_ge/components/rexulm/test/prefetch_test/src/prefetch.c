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
//!   @file  -  prefetch.c
//
//!   @brief -  This just initializes the system including the RexULM
//
//
//!   @note  -  The idea is that the RexULM will start the prefetch
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <rexulm.h>

#include <leon_traps.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <leon_uart.h>

#ifdef GOLDENEARS
#include <xrr.h>
#endif
#include <xcsr_xusb.h>
#include <ulm.h>
#include <grg.h>

#include <interrupts.h>

#include <icmd.h>
#include <iassert.h>

#include "prefetch_log.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

void imain(void) __attribute__ ((noreturn));
void imain(void)
{
    irqFlagsT flags;

    // Lockout interrupts for the initialization code
    flags = LEON_LockIrq();

    // Configure the uart
    LEON_UartSetBaudRate115200();
    ICMD_Init();
#ifdef GOLDENEARS
    LEON_InstallIrqHandler(IRQ_UART_RX, &LEON_UartInterruptHandlerRx);
    LEON_InstallIrqHandler(IRQ_UART_TX, &LEON_UartInterruptHandlerTx);
    LEON_EnableIrq(IRQ_UART_RX);
    LEON_EnableIrq(IRQ_UART_TX);
#else // LG1
    LEON_InstallIrqHandler(IRQ_UART, &LEON_UartInterruptHandler);
    LEON_EnableIrq(IRQ_UART);
#endif

    // Uart is good
    // Initialize assert handling
    // broadcast our startup status
    iassert_Init(&ULM_DisconnectUsbPort, &ICMD_PollingLoop);

    // Setup the timers
    LEON_TimerInit();
    LEON_InstallIrqHandler(IRQ_TIMER2, &TIMING_TimerInterruptHandler);
    LEON_EnableIrq(IRQ_TIMER2);

    // Configure the ULM as Rex
    boolT isASIC = (GRG_GetPlatformID() == GRG_PLATFORMID_ASIC);
    ULM_Init(FALSE, isASIC);

    // Setup the Cache, static Qs etc, with suitable defaults (mostly don't care's as this test doesn't involve a Lex)
    XCSR_Init(/* isLex */ FALSE, /* isDirectLink */ TRUE); // Setup for direct link, we don't care about MAC addresses
#ifdef GOLDENEARS
    XRR_Init(); // Setup for Rex
#endif

    // Enable the XCSR interrupts
#ifdef GOLDENEARS
    LEON_EnableIrq(IRQ_XCSR3);
#endif

    // Enable the Rexulm
    REXULM_Init();

    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MINOR_EVENT, TEST_INIT_COMPLETE);

    LEON_UartWaitForTx();
    LEON_UnlockIrq(flags);

    // Loop forever
    while (TRUE)
        ;
}


