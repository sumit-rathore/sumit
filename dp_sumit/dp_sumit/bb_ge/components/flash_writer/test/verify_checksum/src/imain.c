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
//!   @file  -  imain.c
//
//!   @brief -  Flash checksum verification test harness
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

#include <interrupts.h>

#include "verify_checksum_cmd.h"
#include "verify_checksum_log.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

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
#ifdef LIONSGATE
    LEON_InstallIrqHandler(IRQ_UART, LEON_UartInterruptHandler);
    LEON_EnableIrq(IRQ_UART);
#else
    LEON_InstallIrqHandler(IRQ_UART_RX, LEON_UartInterruptHandlerRx);
    LEON_EnableIrq(IRQ_UART_RX);
    LEON_InstallIrqHandler(IRQ_UART_TX, LEON_UartInterruptHandlerTx);
    LEON_EnableIrq(IRQ_UART_TX);
#endif

    // Setup the timers -- icmd needs this
    LEON_TimerInit();
    LEON_InstallIrqHandler(IRQ_TIMER2, TIMING_TimerInterruptHandler);
    LEON_EnableIrq(IRQ_TIMER2);

    TestCheckSum(1337);
    LEON_UnlockIrq(flags);

    while (TRUE)
    {
        //Things to try

        // 1)
        // Lock interrupts flags = LEON_LockIrq();
        // call leon timer read (leon_timers.h)
        // check if X time units have passed
        // if so, ilog
        // unlock interrupt    LEON_UnlockIrq(flags);

        // 2)
        //  During init code above, register timer function (leon_timers.h)
        //  Have the timer function ilog every second

        // If we aren't locked up. in the periodic code, start trying to do low level debug reads
        //  IE.     *(uint32 *)(address of leon uart status register)
        //          Find that in components/leon/src/leon_loc.h
        //      try and peer into the leon component uart.c file, for its internals
    }
}

void TestCheckSum(uint32 count)
{
    volatile uint8* pFlash = (uint8*) SERIAL_FLASH_BASE_ADDR;
    uint32 checksum = 0;

    // save the original value of count, as we will return it later
    uint32 originalCount = count;

    while (count)
    {
        // add value at pFlash to checksum
        checksum += *pFlash;

    // iterate until count is 0
    count--;
    pFlash++;
    }

    ilog_TEST_HARNESS_COMPONENT_2(ILOG_USER_LOG, TEST_CHECKSUM, originalCount, checksum);
}
