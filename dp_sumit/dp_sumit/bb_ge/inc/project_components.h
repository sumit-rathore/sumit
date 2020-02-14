///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
    COMPONENT_CREATE(TOPLEVEL_COMPONENT)
    COMPONENT_CREATE(TEST_HARNESS_COMPONENT)
    COMPONENT_CREATE(ILOG_COMPONENT)
    COMPONENT_CREATE(FLASH_WRITER_COMPONENT)
    COMPONENT_CREATE(SPECTAREG_COMPONENT)
    COMPONENT_CREATE(GRG_COMPONENT)
    COMPONENT_CREATE(XMODEM_COMPONENT)
    COMPONENT_CREATE(REXSCH_COMPONENT)
    COMPONENT_CREATE(ULM_COMPONENT)
    COMPONENT_CREATE(CLM_COMPONENT)
    COMPONENT_CREATE(XLR_COMPONENT)
    COMPONENT_CREATE(XCSR_COMPONENT)
    COMPONENT_CREATE(PLL_COMPONENT)
    COMPONENT_CREATE(LINKMGR_COMPONENT)
    COMPONENT_CREATE(ICMD_COMPONENT)
    COMPONENT_CREATE(REXULM_COMPONENT)
    COMPONENT_CREATE(LEXULM_COMPONENT)
    COMPONENT_CREATE(TOPOLOGY_COMPONENT)
    COMPONENT_CREATE(DESCPARSER_COMPONENT)
    COMPONENT_CREATE(DEVMGR_COMPONENT)
    COMPONENT_CREATE(SYS_CTRL_Q_COMPONENT)
    COMPONENT_CREATE(VHUB_COMPONENT)
    COMPONENT_CREATE(FLASH_DATA_COMPONENT)
    COMPONENT_CREATE(TASKSCH_COMPONENT)
    COMPONENT_CREATE(NET_COMPONENT)
    COMPONENT_CREATE(NETCFG_COMPONENT)
    COMPONENT_CREATE(XRR_COMPONENT)
    COMPONENT_CREATE(TIMING_COMPONENT)
    COMPONENT_CREATE(ATMEL_CRYPTO_COMPONENT)
    COMPONENT_CREATE(RANDOM_COMPONENT)
    COMPONENT_CREATE(KC705_COMPONENT)
    COMPONENT_CREATE(STORAGE_COMPONENT)
    COMPONENT_CREATE(EEPROM_COMPONENT)
    COMPONENT_CREATE(GEBB_COMM_COMPONENT)
COMPONENT_LIST_END()

#endif // #ifndef PROJECT_COMPONENTS_H

