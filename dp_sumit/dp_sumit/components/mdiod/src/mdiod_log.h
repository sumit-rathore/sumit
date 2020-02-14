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
//!   @file  -  MDIOD_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MDIOD_LOG_H
#define MDIOD_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(MDIOD_COMPONENT)
    /* phy_driver logs */
    // TODO: Put the logs into its submodule
    ILOG_ENTRY(MDIOD_FOUND_PHY_AT_MDIO_ADDR, "Found a PHY at MDIO address %d\n")
    ILOG_ENTRY(MDIOD_NULL_COMPLETION_HANDLER, "Null completion handler passed, line %d\n")
    ILOG_ENTRY(MDIOD_PHY_FIFO_FULL, "PHY MDIO operation FIFO full. Can't submit Callback=0x%x, Active Callback=0x%x, Line=%d\n")
    ILOG_ENTRY(UNSUPPORTED_BMSR_ERCAP, "The PHY does not suppport extended capability registers\n")
    ILOG_ENTRY(UNSUPPORTED_BMSR_ANEGCAPABLE, "The PHY does not support auto negotiation\n")
    ILOG_ENTRY(UNSUPPORTED_BMSR_ESTATEN, "The PHY does not support 1000Base-T extended status register\n")
    ILOG_ENTRY(UNSUPPORTED_ESTATUS_1000_TFULL, "The PHY does not support 1000Base-T full duplex\n")
    ILOG_ENTRY(USELESS_PHY_SETTING, "Useless PHY Setting\n")
    ILOG_ENTRY(LINK_UP, "PHY link up\n")
    ILOG_ENTRY(LINK_DOWN, "PHY link down\n")

    /* phy query icmd messages */
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG, "PHY control reg 0x%x\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE1, "  PHY in reset\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE2, "  PHY in loopback mode\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE3, "  PHY auto-negotiate enabled\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE4, "  PHY auto-negotiate NOT enabled\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE5, "  PHY in power down mode\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE6, "  PHY is electrically isolated from GMII\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE7, "  PHY auto-negotiate restarting\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE8, "  PHY auto-negotiate restart complete\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE9, "  PHY full duplex\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE10, "  PHY half duplex\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE11, "  PHY collision test enabled\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE12, "  PHY speed selected 1000Mbps\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE13, "  PHY speed selected 100Mbps\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_CONTROL_REG_DECODE14, "  PHY speed selected 10Mbps\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG, "PHY status reg 0x%x\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE1, "  PHY 100BASE-T4 capable\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE2, "  PHY 100BASE-TX full duplex capable\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE3, "  PHY 100BASE-X half duplex capable\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE4, "  PHY 10BASE-T full duplex capable\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE5, "  PHY 10BASE-T half duplex capable\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE6, "  PHY 100BASE-T2 full duplex capable\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE7, "  PHY 100BASE-T2 half duplex capable\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE8, "  PHY extended status information in reg 0x0F\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE9, "  PHY auto-negotiation complete\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE10, "  PHY remote fault detected\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE11, "  PHY auto-negotiate capable\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE12, "  PHY auto-negotiate NOT capable\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE13, "  PHY link up\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE14, "  PHY link down\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE15, "  PHY jabber condition detected\n")
    ILOG_ENTRY(ICMD_DECODE_PHY_STATUS_REG_DECODE16, "  PHY extended register capabilities\n")
    ILOG_ENTRY(MDIOD_ETH_PHY_NOT_FOUND, "No ethernet PHY was detected.\n")

    ILOG_ENTRY(MDIOD_DEBUG_D_X, "MDIOD DGB LINE %d VAL 0x%x\n")

    ILOG_ENTRY(MDIOD_ETH_PHY_ISR_NOT_GEN, "MDIOD not EnetPHY generated ISR\n")
    ILOG_ENTRY(MDIOD_ETH_PHY_ISR_NOT_LINK_GEN, "MDIOD not EnetPHY Link generated ISR, value: %x\n")
    ILOG_ENTRY(MDIOD_ETH_PHY_ISR_GEN, "MDIOD EnetPHY generated ISR: 0x%x\n")

    ILOG_ENTRY(ENET_PHY_CHANGE_PHY_SPEED, "Changing PHY speed to %d\n")
    ILOG_ENTRY(UNSUPPORTED_1000MBPS_SPEED_CHANGE_REQUEST, "Requested speed of 1000Mbps not supported by this PHY\n")
ILOG_END(MDIOD_COMPONENT, ILOG_DEBUG) //ILOG_MINOR_EVENT)

#endif // #ifndef MDIOD_LOG_H

