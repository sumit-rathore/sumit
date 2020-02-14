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
//!   @brief -  test harness to verify image on flash
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
#include <leon_mem_map.h>

#include <xmodem.h>

#include "verify_image_log.h"

/************************ Defined Constants and Macros ***********************/
// Since the test harness runs with interrupts active, as opposed to the rest of the firmware which expects the opposite
// Care must be taken to ensure interrupts are disabled when running code which also has an interrupt part, such as ilogging
#define TEST_LOG0(args...) do { irqFlagsT tmpFlags = LEON_LockIrq(); ilog_TEST_HARNESS_COMPONENT_0(args); LEON_UnlockIrq(tmpFlags); } while (FALSE)
#define TEST_LOG1(args...) do { irqFlagsT tmpFlags = LEON_LockIrq(); ilog_TEST_HARNESS_COMPONENT_1(args); LEON_UnlockIrq(tmpFlags); } while (FALSE)
#define TEST_LOG2(args...) do { irqFlagsT tmpFlags = LEON_LockIrq(); ilog_TEST_HARNESS_COMPONENT_2(args); LEON_UnlockIrq(tmpFlags); } while (FALSE)
#define TEST_LOG3(args...) do { irqFlagsT tmpFlags = LEON_LockIrq(); ilog_TEST_HARNESS_COMPONENT_3(args); LEON_UnlockIrq(tmpFlags); } while (FALSE)


/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
uint8 * flash_p = (uint8 *) SERIAL_FLASH_BASE_ADDR; /* start checking from start of flash */

/************************ Local Function Prototypes **************************/
static uint32 * incomingDataHandler(uint32 * buf, uint32 bufSize);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: imain()
*
* @brief  - verify image test harness
*
* @return - never
*
* @note   -
*
*/
void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    irqFlagsT flags;
    uint32 buf1[XMODEM_BLOCK_SIZE_32BIT_WORDS];
    uint32 buf2[XMODEM_BLOCK_SIZE_32BIT_WORDS];

    flags = LEON_LockIrq();
    // Configure the uart & its interrupt
    LEON_UartSetBaudRate115200();
#ifdef LIONSGATE
    LEON_InstallIrqHandler(IRQ_UART, LEON_UartInterruptHandler);
    LEON_EnableIrq(IRQ_UART);
#else
    LEON_InstallIrqHandler(IRQ_UART_RX, LEON_UartInterruptHandlerRx);
    LEON_EnableIrq(IRQ_UART_RX);
    LEON_InstallIrqHandler(IRQ_UART_TX, LEON_UartInterruptHandlerTx);
    LEON_EnableIrq(IRQ_UART_TX);
#endif

    // Configure timer and its interrupt
    LEON_TimerInit();
    LEON_InstallIrqHandler(IRQ_TIMER2, TIMING_TimerInterruptHandler);
    LEON_EnableIrq(IRQ_TIMER2);

    // Basic XModem configuration
    XMODEM_InterruptModeInit();

    // Announce that we are running
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();
    LEON_UnlockIrq(flags);

    // Do the Xmodem transfer
    if (XMODEM_InterruptModeReceive(&incomingDataHandler, buf1, buf2))
    {
        TEST_LOG0(ILOG_USER_LOG, XMODEM_SUCCESS);
    }
    else
    {
        TEST_LOG0(ILOG_USER_LOG, XMODEM_FAILURE);
    }

    // That's it, but there is no way to finish, so lets loop forever
    while (TRUE)
        ;
}


/**
* FUNCTION NAME: incomingDataHandler()
*
* @brief  - xmodem handler, moves data received from the xmodem into a buffer
*
* @return - the same buffer to reuse
*
* @note   -
*
*/
static uint32 * incomingDataHandler(uint32 * orig_buf, uint32 bufSize)
{
    uint32 * buf = orig_buf;
    uint8 * buf_p;
    uint8 i;
    static uint8 numOfErrors = 0;

    /* check each word in the buffer */
    while (bufSize)
    {
        buf_p = CAST(buf, uint32 *, uint8 *); /* access individual bytes in word */

        for (i = 0; i < 4; i++)
        {
            /* each byte in the buffer should match its corresponding byte in flash */
            if (*flash_p != *buf_p)
            {
                TEST_LOG3(ILOG_USER_LOG, UNMATCH, (uint32) flash_p, *flash_p, *buf_p);
                TEST_LOG3(ILOG_USER_LOG, BUF_INFO,
                    orig_buf[0],    // First Word of buffer
                    *buf_p,         // Current Word of buffer
                    orig_buf[(bufSize >> 2) - 1]);  // Last word of buffer
                numOfErrors++;

                if (numOfErrors > 8)
                {
                    return NULL; // Too many errors, let's just bail, and
                                 // report the error back to the user
                }
            }

            /* increment flash and buffer pointers */
            flash_p++;
            buf_p++;
        }

        buf++;

        bufSize -= 4;
    }

    return orig_buf;
}

