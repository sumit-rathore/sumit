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
//!   @file  -  flash_icmds_main.c
//
//!   @brief -
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
#include <main_loop.h>

#include <sfi.h>
#include <ibase.h>

#include "flash_icmds_log.h"
#include "flash_icmds_cmd.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

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
void * imain(void)
{
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

    // Tell the world that we have started
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();

    // Start the interupts
    LEON_UnlockIrq(flags);

    // Run the main loop
    return &MainLoop;
}

// send instruction & wait for completion
//eg, Write enable, erase
void SFISendInstruction(uint8_t SFIInstruction)
{
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, SENDING_INSTRUCTION, SFIInstruction);
    LEON_SFISendInstruction(SFIInstruction);
}

// send instruction, wait for completion, and grab data to return
//eg, read status status
void SFISendReadStatus(uint8_t SFIInstruction, uint8_t flashLen)
{
    LEON_FlashDataLengthT len;

    switch (flashLen)
    {
        case 1:
            len = LEON_FLASH_1_BYTE_DATA;
            break;

        case 2:
            len = LEON_FLASH_2_BYTE_DATA;
            break;

        case 3:
            len = LEON_FLASH_3_BYTE_DATA;
            break;

        case 4:
            len = LEON_FLASH_4_BYTE_DATA;
            break;

        default:
            ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_ERROR, INVALID_COND, __LINE__);
            return;
            break;
    }

    ilog_TEST_HARNESS_COMPONENT_1(
            ILOG_USER_LOG,
            READ_STATUS_RETURNED,
            LEON_SFISendReadStatus(SFIInstruction, len));
}

// send instruction
//eg, send read fast read command
void SFISendReadInstruction(uint8_t SFIInstruction, uint8_t flashLen)
{
    LEON_FlashDataLengthT len;

    switch (flashLen)
    {
        case 1:
            len = LEON_FLASH_1_BYTE_DATA;
            break;

        case 2:
            len = LEON_FLASH_2_BYTE_DATA;
            break;

        case 3:
            len = LEON_FLASH_3_BYTE_DATA;
            break;

        case 4:
            len = LEON_FLASH_4_BYTE_DATA;
            break;

        default:
            ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_ERROR, INVALID_COND, __LINE__);
            return;
            break;
    }

    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, SENDING_READ, SFIInstruction);
    LEON_SFISendReadInstruction(SFIInstruction, len);
}

//sends instruction and data to flash registers
void SFISendByteWrite(uint8_t SFIInstruction, uint32_t flashOffset, uint8_t data)
{
    LEON_SFISendWrite(SFIInstruction, flashOffset, &data, 1);
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, WROTE_BYTE, data);
}

//sends instruction and data to flash registers
void SFISendShortWrite(uint8_t SFIInstruction, uint32_t flashOffset, uint16_t data)
{
    LEON_SFISendWrite(SFIInstruction, flashOffset, CAST(&data, uint16_t *, uint8_t *), 2);
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, WROTE_SHORT, data);
}


//sends instruction and data to flash registers
void SFISendWordWrite(uint8_t SFIInstruction, uint32_t flashOffset, uint32_t data)
{
    LEON_SFISendWrite(SFIInstruction, flashOffset, CAST(&data, uint32_t *, uint8_t *), 4);
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, WROTE_WORD, data);
}

