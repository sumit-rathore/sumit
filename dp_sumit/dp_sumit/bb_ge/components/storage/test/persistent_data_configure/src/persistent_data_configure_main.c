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
//!   @file  -  persistent_data_configure_main.c
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
#include <grg.h>
#include <storage_Data.h>
#include <atmel_crypto.h>
#include <eeprom.h>

#include "persistent_data_configure_log.h"
#include "persistent_data_configure_cmd.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void imainPart2(void);

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
    uint8 major;
    uint8 minor;
    uint8 debug;
    enum STORAGE_TYPE storageType;

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
    TASKSCH_Init();

    // GRG initialization : which only verifies register ID's
    GRG_Init(&major, &minor, &debug);
    LEON_InstallIrqHandler(IRQ_GRG, &GRG_InterruptHandler);
    LEON_EnableIrq(IRQ_GRG);

    // Initialize the Atmel crypto chip
#ifdef GE_CORE
    storageType = MEMORY_ONLY_STORAGE;
#else
    enum GRG_PlatformID platform = GRG_GetPlatformID();
    switch (platform)
    {
        case GRG_PLATFORMID_SPARTAN6:
            ATMEL_init(NULL);
            storageType = USE_ATMEL_STORAGE;
            break;

        case GRG_PLATFORMID_KINTEX7_DEV_BOARD:
            storageType = USE_FLASH_STORAGE;
            break;

        case GRG_PLATFORMID_ASIC:
            EEPROM_Init(0, 0, 128);
            storageType = USE_EEPROM_STORAGE;
            break;

        default:
            iassert_TEST_HARNESS_COMPONENT_1(FALSE, UNSUPPORTED_PLATFORMID, platform);
            break;
    }

#endif
    STORAGE_persistentDataInitialize(&imainPart2, storageType);

    // Start the interupts
    LEON_UnlockIrq(flags);

    // Run the main loop
    return &TASKSCH_MainLoop;
}

static void imainPart2(void)
{
    // Tell the world that we have started
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();
}
