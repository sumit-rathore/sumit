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
//!   @file  -  linkmgr_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef LINKMGR_LOG_H
#define LINKMGR_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(LINKMGR_COMPONENT)
    /* Generic messages */
    ILOG_ENTRY(INVALID_MESSAGE, "Invalid message %d at line %d\n")
    /* Phy query icmd messages */
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
    /* Phy Mgr messages */
    ILOG_ENTRY(PHY_MGR_UNEXPECTED_STATE, "PhyMgr: unexpected state %d at line %d\n")
    ILOG_ENTRY(PHY_MGR_LINK_UP, "PhyMgr: link up\n")
    ILOG_ENTRY(PHY_MGR_LINK_DOWN, "PhyMgr: link down\n")
    /* XUSB Link Manger */
    ILOG_ENTRY(XUSB_VPORT_LINK_UP, "XUSB Link up on vport %d\n")
    ILOG_ENTRY(XUSB_VPORT_LINK_DOWN, "XUSB Link down on vport %d\n")
    ILOG_ENTRY(XUSB_SET_STATE, "XUSB Link Manager setting Vport %d to state %d at line %d\n")
    // new messages
    ILOG_ENTRY(UNKNOWN_LINK_MODE, "Link manager initialization received an unexpected link mode (%d)\n")
    ILOG_ENTRY(MESSAGE_HANDLER_UNDEFINED, "The link manager's message handler is undefined\n")
    ILOG_ENTRY(LINKMGR_RECEIVED_MSG, "Received a message (%d) on vport %d\n")
    ILOG_ENTRY(LINKMGR_LEX_BROADCAST, "Broadcasted an XUSB announcement message\n")
    ILOG_ENTRY(LINKMGR_UNEXPECTED_LINK_MODE, "Execution reached code not intended for this link mode (%d) at line %d\n")
    ILOG_ENTRY(LINKMGR_INITIALIZING, "Link manager initializing with link mode = %d\n")
    ILOG_ENTRY(LINKMGR_PAIR_BUTTON_EVENT, "Pairing button pressed: %d.  Button state at interrupt: %d\n")
    ILOG_ENTRY(LINKMGR_REMOVE_ALL_LINKS, "Removing all stored link pairings.\n")
    ILOG_ENTRY(LINKMGR_INVALID_VP_NEG_TIMEOUT, "Reached the vport negotiation timeout in an unexpected state. linkState=%d\n")
    ILOG_ENTRY(LINKMGR_EXPECTING_NULL_CALLBACK, "Found an existing callback when trying to set the callbacks to begin=0x%x, end=0x%x\n")
    ILOG_ENTRY(LINKMGR_ADD_PAIRING_CALLBACK_EXISTS, "Found an existing callback 0x%x when trying to set the add pairing callback to 0x%x\n")
    ILOG_ENTRY(LINKMGR_MLP_ACQUISITION_TIMEOUT, "Timed out while trying to acquire an MLP link.\n")
    ILOG_ENTRY(LINKMGR_REX_VP_NEGOTIATION_TIMEOUT, "Timed out while waiting to be assigned a vport.\n")
    ILOG_ENTRY(LINKMGR_PAIR_INCOMPATIBLE, "The paired USB extender on vport %d is not compatible with this device.\n")
    ILOG_ENTRY(LINKMGR_CHECK_COMPATIBILITY_PAIR, "Checking compatibility against pair with firmware version %d.%d.%d.\n")
    ILOG_ENTRY(LINKMGR_CHECK_COMPATIBILITY_LOCAL, "The local firmware version is %d.%d.%d.\n")
    ILOG_ENTRY(ICMD_NOT_SUPPORTED_IN_THIS_BUILD, "ICMD is not supported in this build\n")
    ILOG_ENTRY(LEX_INVALID_STATE, "LEX state is invalid. Current LEX state is %d\n")
    ILOG_ENTRY(LINKMGR_CHECK_CONFIGURATION, "Comparing configuration of LEX: 0x%x, with configuration of REX: 0x%x\n")
    ILOG_ENTRY(UNHANDLED_LINK_TYPE, "Unhandled link type %d\n")
    ILOG_ENTRY(CRM_PLL_LOSS_OF_LOCK, "CRM PLL loss of lock\n")
    ILOG_ENTRY(CTM_PLL_LOSS_OF_LOCK, "CTM PLL loss of lock\n")
    ILOG_ENTRY(USELESS_PHY_SETTING, "Useless phy setting\n")
    ILOG_ENTRY(PHY_ID2, "Phy ID 2 is 0x%.4x\n")
    ILOG_ENTRY(PHY_ID3, "Phy ID 3 is 0x%.4x\n")
    ILOG_ENTRY(LINK_STATE, "PhyMgr is in state 0x%x, link type is %d, XUSB link manger is in state 0x%x\n")
    ILOG_ENTRY(INIT_LINK_TYPE, "PhyMgr Initialization for link type %d\n")
    ILOG_ENTRY(LINKMGR_MDIO_OPERATION_ALREADY_ACTIVE, "Submitted an ethernet PHY MDIO operation, when one is active.  Submitted Callback=0x%x, Active Callback=0x%x, Line=%d\n")
    ILOG_ENTRY(LINKMGR_INVALID_BUTTON_STATE, "Invalid button state %d on line %d\n")
    ILOG_ENTRY(LINKMGR_FOUND_PHY_AT_MDIO_ADDR, "Found a PHY at MDIO address %d\n")
    ILOG_ENTRY(LINKMGR_NULL_COMPLETION_HANDLER, "Null completion handler passed, line %d\n")
    ILOG_ENTRY(DEPRECATED_LINKMGR_DEBUGX, "__#### DEBUG 0x%x ####__\n")
    ILOG_ENTRY(LINKMGR_DEBUG_CLM_FLUSH_Q, "Flushing CLM Queues\n")
    ILOG_ENTRY(LINKMGR_VETOING_INCOMPATIBLE_VIDS, "Vetoing connection due to incompatible variant IDs; LexVID = %d, RexVID = %d\n")
    ILOG_ENTRY(LINKMGR_PHY_HAS_INVALID_IDENTIFIER, "The ethernet PHY ID2=0x%x and PHY ID3=0x%x are invalid for the ITC2053.\n")
ILOG_END(LINKMGR_COMPONENT, ILOG_MINOR_EVENT)


#endif // #ifndef LINKMGR_LOG_H

