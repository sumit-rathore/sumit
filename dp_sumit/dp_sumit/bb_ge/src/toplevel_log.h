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
//!   @file  -  toplevel_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TOPLEVEL_LOG_H
#define TOPLEVEL_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TOPLEVEL_COMPONENT)
    ILOG_ENTRY(SW_VERSION, "----> ICRON Technologies Extreme USB Goldenears Project - Software version v%d.%d.%d <----\n")
    ILOG_ENTRY(CHIP_REV, "Chip ID is Major 0x%x, Minor 0x%x, Debug 0x%x\n")
    ILOG_ENTRY(THIS_IS_LEX, "This is the Lex\n")
    ILOG_ENTRY(THIS_IS_REX, "This is the Rex\n")
    ILOG_ENTRY(USB_1_1_SPEED, "Running at USB 1.1 speed\n")
    ILOG_ENTRY(USB_2_0_SPEED, "Running at USB 2.0 speed\n")
    ILOG_ENTRY(KINTEX_PLATFORM, "This is a Kintex Platform\n") // Used to virtex.  They use the same Platform ID
    ILOG_ENTRY(SPARTAN_PLATFORM, "This is a Spartan Platform\n")
    ILOG_ENTRY(LINK_INIT_COMPLETE, "----> Link Initialization Complete <----\n")
    ILOG_ENTRY(APP_INIT_COMPLETE, "----> App Initialization Complete <----\n")
    ILOG_ENTRY(BUILD_DATE, "SW build was done on %04d/%02d/%02d\n")
    ILOG_ENTRY(BUILD_TIME, "SW build was done at %02d:%02d:%02d\n")
    ILOG_ENTRY(RECEIVED_SOMETHING_ON_CPU_RXQ, "Received control link sub type %d msg %d and data 0x%x on CPU RX Q\n")
    ILOG_ENTRY(UNEXPECTED_TRAP, "Unexpected trap occured, PC was 0x%x, nPC was 0x%x\n")
    ILOG_ENTRY(UNEXPECTED_TRAP_WITHOUT_WINDOWS, "Unexpected trap occured without spare windows, last i7 is 0x%x, previous was 0x%x, previous to that was 0x%x\n")
    ILOG_ENTRY(UNKNOWN_LINK_TYPE_MSG, "Received a link message of unknown type %d.  Message value: %d.\n")
    ILOG_ENTRY(GOT_MSG_FOR_LEXULM_OR_VHUB_ON_REX, "Got msg %d for Lexulm or VHub on the Rex\n")
    ILOG_ENTRY(GOT_MSG_FOR_REXULM_ON_LEX, "Got msg %d for Rexulm on the Lex\n")
    ILOG_ENTRY(CPU_RX_ERR, "CPU RX Q err %d at line %d\n")
    ILOG_ENTRY(UNEXPECTED_Q_OPERATION_STATE, "Unexpected queue operation state (%d)\n")
    ILOG_ENTRY(ENTERED_CPU_RX_ISR, "Entered the CPU RX ISR\n")
    ILOG_ENTRY(RUNNING_CPU_RX_TASK, "Running the CPU RX task in state %d\n")
    ILOG_ENTRY(MAC_FAILED, "MAC challenge failed\n")
    ILOG_ENTRY(MAC_PASSED, "MAC challenge passed\n")
    ILOG_ENTRY(MAC_HAS_INVALID_INDEX, "MAC has invalid index %d\n")
    ILOG_ENTRY(INVALID_NETWORK_MODE, "VHub without layer 2 networking is not a supported configuration\n")
    ILOG_ENTRY(DISABLING_SYS, "Disabling System\n")
    ILOG_ENTRY(UART_BOOT_IN_GE_CORE, "UART boot is not supported by GE core\n")
    ILOG_ENTRY(TOPLEVEL_DEVICE_IS_LEX, "This device is a LEX\n")
    ILOG_ENTRY(TOPLEVEL_DEVICE_IS_REX, "This device is a REX\n")
    ILOG_ENTRY(NO_CFG_VAR_EXISTS, "NO CONFIGURATION VARIABLE EXISTS, USING A DEFAULT SETTING\n")
    ILOG_ENTRY(UNKNOWN_PLATFORM, "Unknown platform %d\n")
    ILOG_ENTRY(SPARTAN_UON_VARIANT, "Spartan UoN Variant\n")
    ILOG_ENTRY(SPARTAN_CORE2300_VARIANT, "Spartan Core2300 Variant\n")
    ILOG_ENTRY(ASIC_PLATFORM, "GE ASIC platform\n")
    ILOG_ENTRY(UNKNOWN_VARIANT, "Unknown Variant %d\n")
    ILOG_ENTRY(I2C_FAILURE, "i2c failure at line %d\n")
    ILOG_ENTRY(PENDING_IRQ, "Pending Leon Interrupts: 0x%x\n")
    ILOG_ENTRY(ENABLED_IRQ, "Enabled Leon Interrupts: 0x%x\n")
    ILOG_ENTRY(DEBUG_BUILD, "This is a debug build\n")
    ILOG_ENTRY(UNSUPPORTED_VID, "Unsupported variant ID %d\n")
    ILOG_ENTRY(VARIANT_ID, "Variant ID is %d\n")
    ILOG_ENTRY(L2_ENABLED_BUT_NO_MAC_ADDR, "Layer 2 framing is enabled but no MAC address is stored! Falling back to direct-link mode.\n")
    ILOG_ENTRY(CYPRESS_HX3_PROGRAMMING_FAILURE, "Failed while programming the Cypress HX3 hub firmware.\n")
    ILOG_ENTRY(CYPRESS_HX3_PROGRAMMING_SUCCESS, "Cypress HX3 hub firmware successfully programmed.\n")
    ILOG_ENTRY(CYPRESS_HX3_NOT_FOUND, "Cypress HX3 hub not found.\n")
    ILOG_ENTRY(CYPRESS_HX3_RETRY, "Cypress HX3 NAKed while receiving firmware at offset=%d.  Retry count=%d\n")
    ILOG_ENTRY(EEPROM_INSTALLED, "EEPROM is installed\n")
    ILOG_ENTRY(EEPROM_NOT_INSTALLED, "EEPROM is not installed\n")
    ILOG_ENTRY(ILLEGAL_LINK_TYPE_WITH_2053, "It is not valid to use link type %d with the ITC2053 ASIC.\n")
    ILOG_ENTRY(ECO1_FLAG_VALUE, "ECO flag is %d (1 = ECO detected)\n")
    ILOG_ENTRY(DEBUG_ASSERT, "CAUSE AN ASSERT IN GE FOR DEBUGGING\n %x, %x, %x")
ILOG_END(TOPLEVEL_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef TOPLEVEL_LOG_H

