///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012, 2013
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
//!   @file  -  project_components.h
//
//!   @brief -  This file lists all the components in the project
//
//!   @note  -  This is used for asserts and logging messages
//
///////////////////////////////////////////////////////////////////////////////

// Check if this is used for standard component numbers, or a special parser
// We need to ensure that the enum is only defined once
// Special parsers can do their own checks
#if defined(COMPONENT_PARSER) || !defined(PROJECT_COMPONENTS_H)
#ifndef COMPONENT_PARSER
#define PROJECT_COMPONENTS_H
#endif

#include <icomponent.h>

COMPONENT_LIST_CREATE()
    COMPONENT_CREATE(ISTATUS_COMPONENT)
    COMPONENT_CREATE(ROM_COMPONENT)
    COMPONENT_CREATE(TOPLEVEL_COMPONENT)
    COMPONENT_CREATE(TEST_COMPONENT)
    COMPONENT_CREATE(EVENT_COMPONENT)
    COMPONENT_CREATE(CALLBACK_COMPONENT)
    COMPONENT_CREATE(UTIL_COMPONENT)
    COMPONENT_CREATE(DP_COMPONENT)
    COMPONENT_CREATE(DP_AUX_COMPONENT)
    COMPONENT_CREATE(DP_STREAM_COMPONENT)
    COMPONENT_CREATE(BGRG_COMPONENT)
    COMPONENT_CREATE(CPU_COMM_COMPONENT)
    COMPONENT_CREATE(EEPROM_COMPONENT)
    COMPONENT_CREATE(FLASH_DATA_COMPONENT)   //Please never change position of this component, it is being used in excom
    COMPONENT_CREATE(GPIO_COMPONENT)
    COMPONENT_CREATE(HPD_COMPONENT)
    COMPONENT_CREATE(I2C_COMPONENT)
    COMPONENT_CREATE(LED_COMPONENT)
    COMPONENT_CREATE(I2CD_COMPONENT)
    COMPONENT_CREATE(ICMD_COMPONENT)
    COMPONENT_CREATE(ILOG_COMPONENT)
    COMPONENT_CREATE(LAN_PORT_COMPONENT)
    COMPONENT_CREATE(MAC_COMPONENT)
    COMPONENT_CREATE(MCA_COMPONENT)
    COMPONENT_CREATE(MDIO_COMPONENT)
    COMPONENT_CREATE(PCS_PMA_COMPONENT)
    COMPONENT_CREATE(SFI_COMPONENT)
    COMPONENT_CREATE(TEST_HARNESS_COMPONENT)
    COMPONENT_CREATE(TIMING_COMPONENT)
    COMPONENT_CREATE(ATMEL_CRYPTO_COMPONENT)
    COMPONENT_CREATE(ULP_COMPONENT)
    COMPONENT_CREATE(XMODEM_COMPONENT)
    COMPONENT_CREATE(UART_COMPONENT)
    COMPONENT_CREATE(XAUI_COMPONENT)
    COMPONENT_CREATE(CRC_COMPONENT)
    COMPONENT_CREATE(SHA2_COMPONENT)
    COMPONENT_CREATE(CORE_COMPONENT)
    COMPONENT_CREATE(TOP_COMPONENT)
    COMPONENT_CREATE(MDIOD_COMPONENT)
    COMPONENT_CREATE(BBGE_COMM_COMPONENT)
    COMPONENT_CREATE(LINKMGR_COMPONENT)
    COMPONENT_CREATE(CONFIG_COMPONENT)
    COMPONENT_CREATE(GE_PROGRAM_COMPONENT)
    COMPONENT_CREATE(COMMAND_COMPONENT)
    COMPONENT_CREATE(RANDOM_COMPONENT)
    COMPONENT_CREATE(STATS_MON_COMPONENT)
    COMPONENT_CREATE(RS232_COMPONENT)
    COMPONENT_CREATE(XADC_COMPONENT)
    COMPONENT_CREATE(AQUANTIA_COMPONENT)
    COMPONENT_CREATE(FIBER5G_COMPONENT)
    COMPONENT_CREATE(IDT_CLK_COMPONENT)
    COMPONENT_CREATE(MAIN_LOOP_COMPONENT)
    COMPONENT_CREATE(UPP_COMPONENT)                //Please never change position of this component, it is being used in excom
    COMPONENT_CREATE(I2C_SLAVE_COMPONENT)
COMPONENT_LIST_END()

#endif // #ifndef PROJECT_COMPONENTS_H

