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
//!   @file  -  burn_flash_loc.h
//
//!   @brief -  Local header file for burn_flash test harness
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef BURN_FLASH_LOC_H
#define BURN_FLASH_LOC_H

/***************************** Included Headers ******************************/
#include <interrupts.h>

#include <leon_uart.h>
#include <leon_timers.h>
#include <leon_traps.h>
#include <leon_mem_map.h>
#include <flash_raw.h>
#include <timing_timers.h>

#include "burn_flash_cmd.h"
#include "burn_flash_log.h"

/************************ Defined Constants and Macros ***********************/
#define FLASH_TEST_SIZE 131072 /* test 128KB of flash */

// Since the test harness runs with interrupts active, as opposed to the rest of the firmware which expects the opposite
// Care must be taken to ensure interrupts are disabled when running code which also has an interrupt part, such as ilogging
#define TEST_LOG0(args...) do { irqFlagsT tmpFlags = LEON_LockIrq(); ilog_TEST_HARNESS_COMPONENT_0(args); LEON_UnlockIrq(tmpFlags); } while (FALSE)
#define TEST_LOG1(args...) do { irqFlagsT tmpFlags = LEON_LockIrq(); ilog_TEST_HARNESS_COMPONENT_1(args); LEON_UnlockIrq(tmpFlags); } while (FALSE)
#define TEST_LOG2(args...) do { irqFlagsT tmpFlags = LEON_LockIrq(); ilog_TEST_HARNESS_COMPONENT_2(args); LEON_UnlockIrq(tmpFlags); } while (FALSE)
#define TEST_LOG3(args...) do { irqFlagsT tmpFlags = LEON_LockIrq(); ilog_TEST_HARNESS_COMPONENT_3(args); LEON_UnlockIrq(tmpFlags); } while (FALSE)

/******************************** Data Types *********************************/

/******************************** Global Var *********************************/

/*********************************** API *************************************/

// flash_functions.c
void writeFlash(uint32 flashOffset, uint32 * buf, uint32 bufSize);
boolT verifyFlash(uint32 flashAddr, uint32 * buf, uint32 bufSize);
boolT eraseAndVerifyFlash();

#endif // BURN_FLASH_LOC_H

