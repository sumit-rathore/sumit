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
//!   @file  -  clm_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef CLM_LOG_H
#define CLM_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(CLM_COMPONENT)
    // Verify id & version of the CLM register block
    ILOG_ENTRY(REG_FAILURE, "Failure verifying registers\n")
    ILOG_ENTRY(CLM_INVALID_XUSB_CHIP_MINOR_REVISION_ERROR_LOG, "Invalid minor XUSB chip revision, expecting 0x%x, read 0x%x\n")
    ILOG_ENTRY(CLM_INVALID_XUSB_CHIP_MAJOR_REVISION_ERROR_LOG, "Invalid major XUSB chip revision, expecting 0x%x, read 0x%x\n")
    // Init error
    // Cfg specific messages
    ILOG_ENTRY(INVALID_CFG, "Invalid configuration %d selected\n")
    ILOG_ENTRY(CFG_TBI, "Configuring CLM for TBI\n")
    ILOG_ENTRY(CFG_GMII, "Configuring CLM for GMII\n")
    ILOG_ENTRY(CFG_MII, "Configuring CLM for MII\n")
    ILOG_ENTRY(CFG_CLEI8, "Configuring CLM for CLEI8\n")
    ILOG_ENTRY(CFG_CLEI1, "Configuring CLM for CLEI1\n")
    ILOG_ENTRY(CFG_CLEI2, "Configuring CLM for CLEI2\n")
    ILOG_ENTRY(CFG_CLEI4, "Configuring CLM for CLEI4\n")
    // generic stat messages
    // specific stat messages
    // TX stat errors
    // RX stat errors
    // MAC
    // New variables
    ILOG_ENTRY(VPORT_ENABLE, "VPort Enable %d\n")
    ILOG_ENTRY(VPORT_DISABLE, "VPort Disable %d\n")
    ILOG_ENTRY(VPORT_POSTPONE_ENABLE, "VPort Enable %d is being postponed\n")
    // Generic stats
    ILOG_ENTRY(RX_STATS_REG, "RX Stats reg is 0x%x\n")
    ILOG_ENTRY(TX_STATS_REG, "TX Stats reg is 0x%x\n")
    ILOG_ENTRY(INVALID_VPORT_STATE_TRANSITION, "Invalid vport state transition for vport %d from state %d to state %d\n")

    // CLM interrupt messages

    // TxStatsETR

    // RxStatsETR

    // new messages
    ILOG_ENTRY(ENABLE_DEFAULT_INTERRUPTS, "Enabling default interrupts\n")
    ILOG_ENTRY(CLM_LEX_VPORT_DST, "Setting LEX vport %d destination MAC address: MSW=0x%x LSW=0x%x\n")
    ILOG_ENTRY(CLM_VPORT_ALREADY_ENABLED, "Trying to enable VPort %d, but it is already enabled\n")
    ILOG_ENTRY(ADJUST_TX_WAIT_4_RESP_THRES_OLD, "Adjust TX wait for response threshold FROM GMII=%d, MII=%d\n")
    ILOG_ENTRY(ADJUST_TX_WAIT_4_RESP_THRES_NEW, "Adjust TX wait for response threshold TO GMII=%d, MII=%d\n")
    ILOG_ENTRY(CLM_SPECTAREG_READ, "Read CLM Register: 0x%x, Value: 0x%x\n")
    ILOG_ENTRY(ADJUST_TX_WAIT_4_RESP_LIMIT_OLD, "Adjust TX wait for response limit FROM limit=%d, CntThresh=%d\n")
    ILOG_ENTRY(ADJUST_TX_WAIT_4_RESP_LIMIT_NEW, "Adjust TX wait for response limit TO limit=%d, CntThresh=%d\n")
    ILOG_ENTRY(INVALID_ICMD_SETTING, "Invalid icmd setting %d\n")
    ILOG_ENTRY(ADJUST_TX_QID_THRESH, "Adjust TX QID THRESH from %d to %d\n")
    ILOG_ENTRY(CLM_SETTING_SRC_MAC_ADDR, "Setting the source MAC address to MSW=0x%x, LSW=0x%x\n")
    ILOG_ENTRY(REDUCED_PIN_COUNT, "Setting reduced pin count mode\n")
    ILOG_ENTRY(CFG_LTBI, "Configuring CLM for LTBI\n")
    ILOG_ENTRY(PHY_SPEED, "Measured PHY speed is: %dMHz, Timeout value is: %dus, Register=%d\n")
    ILOG_ENTRY(ETHERTYPE_LOG, "Wrote CLM.EtherType with value 0x%x\n")
    ILOG_ENTRY(READ_UNINITIALIZED_LINK_TYPE, "Tried to use uninitialized link type %d\n")
    ILOG_ENTRY(CLM_UNEXPECTED_CTM_INPUT_CLK_FREQ, "CTM input clock frequency is unexpected, frequency in MHz = %d\n")
    ILOG_ENTRY(CTM_INPUT_CLK_FREQ, "The CTM input reference clock frequency is %d MHz\n")
ILOG_END(CLM_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef CLM_LOG_H

