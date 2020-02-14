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
//!   @file  -  get_and_echo_polling.c
//
//!   @brief -
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <xmodem.h>
#include <leon_timers.h>
#include <leon_uart.h>
#include <interrupts.h>
#include "get_and_echo_polling_log.h"

/************************ Defined Constants and Macros ***********************/
#define TEST_BUF_SIZE 1024

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static union
{
    uint8 testBuf[TEST_BUF_SIZE];
    uint32 testWordBuf[TEST_BUF_SIZE / 4];
} dataArray;
static uint16 testBufIndex = 0;

/************************ Local Function Prototypes **************************/
static uint32 * incomingDataHandler(uint32 * buf, uint32 bufSize);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: imain()
*
* @brief  - the main entry for the test, starts the uart and the timer, sends an ilog and sets the xmodem_polled up to receive
*
* @return -
*
* @note   -
*
*/
void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint32 buf[XMODEM_BLOCK_SIZE_32BIT_WORDS];
    uint32 i;
    boolT xmodemSuccess;

     // Configure the uart
    LEON_UartSetBaudRate115200();

    // Configure timer
    LEON_TimerInit();


    // Announce that we are running
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();

    //Delay 15 seconds
    for (i = 0; i < 15; i++)
    {
        LEON_TimerWaitMicroSec(1000000);
    }

    // Do the Xmodem transfer
    xmodemSuccess = XMODEM_PolledModeReceive(&incomingDataHandler, buf);

    //Delay 15 seconds
    for (i = 0; i < 15; i++)
    {
        LEON_TimerWaitMicroSec(1000000);
    }

    // Let the user know if the protocol library reported success or failure
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, (xmodemSuccess ? SUCCESS : FAIL));

    // echo back everything we just received
    for (i = 0; i < testBufIndex; i++)
    {
       LEON_UartByteTx(dataArray.testBuf[i]);
    }

    LEON_UartByteTx('\r');
    LEON_UartByteTx('\n');
    LEON_UartWaitForTx();

    // That's it, but there is no way to finish, so lets loop forever
    while (TRUE)
        ;
}


/**
* FUNCTION NAME: incomingDataHandler()
*
* @brief  - stores data which has come in a test buffer
*
* @return - the same buffer to reuse
*
* @note   -
*
*/
static uint32 * incomingDataHandler
(
    uint32 * orig_buf, //the buffer that the incoming data is in
    uint32 bufSize //the size of the buffer that contains the incoming data
)
{
    uint32 * buf = orig_buf;

    while ((testBufIndex < TEST_BUF_SIZE) && (bufSize))
    {
        dataArray.testWordBuf[testBufIndex / 4] = *buf;
        buf++;
        testBufIndex += 4;
        bufSize -= 4;
    }

    return orig_buf;
}


