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
//!   @file  -  block_protect_recovery_main.c
//
//!   @brief -  
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <options.h>
#include <ibase.h>
#include <leon_uart.h>
#include <interrupts.h>
#include <leon_timers.h>
#include <leon_traps.h>
#include <flash_raw.h>

#include "block_protect_recovery_log.h"
#include "block_protect_recovery_cmd.h"

/************************ Defined Constants and Macros ***********************/
/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
/************************ Local Function Prototypes **************************/
static void _BlockProtectRecoveryImage(void)__attribute__ ((section(".ftext")));

/************************** Function Definitions *****************************/
/**
* FUNCTION NAME: _BlockProtectRecoveryImage()
*
* @brief  - Recovery image is between address 0 to (0x200000 - 1)
*
* @return - never
*
* @note   - Based on Page 20 of winbond W25Q32FV datasheet (Rev D):
*           - To protect 0h to 1FFFFFh, need to write the following registers:
*               - WPS = 0
*               - CMP = 0
*               - SEC = 0
*               - TB  = 1
*               - BP2 = 1
*               - BP1 = 1
*               - BP0 = 0
*
*/

static void _BlockProtectRecoveryImage(void)
{
    uint8 statusReg1 = 0;
    uint8 statusReg2 = 2; // Enable support for quad SPI
    uint8 statusReg3 = 0;

    statusReg1 |= (1 << 3); // BP1
    statusReg1 |= (1 << 4); // BP2
    statusReg1 |= (1 << 5); // TB

    ilog_TEST_HARNESS_COMPONENT_2(ILOG_DEBUG, WRITE, 1, statusReg1);

    FLASHRAW_writeStatusRegister(1, statusReg1);

    ilog_TEST_HARNESS_COMPONENT_2(ILOG_DEBUG, READ, 1, FLASHRAW_readStatusRegister(1));

    ilog_TEST_HARNESS_COMPONENT_2(ILOG_DEBUG, WRITE, 2, statusReg2);

    FLASHRAW_writeStatusRegister(2, statusReg2);

    ilog_TEST_HARNESS_COMPONENT_2(ILOG_DEBUG, READ, 2, FLASHRAW_readStatusRegister(2));

    ilog_TEST_HARNESS_COMPONENT_2(ILOG_DEBUG, WRITE, 3, statusReg3);

    FLASHRAW_writeStatusRegister(3, statusReg3);

    ilog_TEST_HARNESS_COMPONENT_2(ILOG_DEBUG, READ, 3, FLASHRAW_readStatusRegister(3));
}

/**
* FUNCTION NAME: imain()
*
* @brief  - 
*
* @return - never
*
* @note   -
*
*/
void * imain(void)__attribute__((noinline, noreturn));
void * imain(void)
{
    // Lockout interrupts for the initialization code
    LEON_LockIrq();
    {
        // Configure the uart
        LEON_UartSetBaudRate115200();
        LEON_InstallIrqHandler(IRQ_UART_TX, LEON_UartInterruptHandlerTx);
        LEON_EnableIrq(IRQ_UART_TX);

        //init the timers
        LEON_TimerInit();
        LEON_EnableIrq(IRQ_TIMER2);
    }

    // We are ready to receive
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTED);

    _BlockProtectRecoveryImage();

    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, FINISHED);

    {
        LEON_UartWaitForTx(); //blocks until entire buffer is transmitted

        // Loop forever, waiting for the user to reset the chip
        while (1)
            ;
    }
}

