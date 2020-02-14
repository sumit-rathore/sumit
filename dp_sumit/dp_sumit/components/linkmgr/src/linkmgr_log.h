//#################################################################################################
// Icron Technology Corporation - Copyright 2016
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef LINKMGR_LOG_H
#define LINKMGR_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(LINKMGR_COMPONENT)
    ILOG_ENTRY(LINKMGR_RCV_EVENT,               "Link state machine: In state %d, received event %d new state %d\n")
    ILOG_ENTRY(LINKMGR_RCV_INVALID_EVENT,       "Link state machine: Unexpected event %d for state %d\n")

    ILOG_ENTRY(PHY_ENABLED,                     "*** PHY Enabled ***\n")
    ILOG_ENTRY(PHY_DISABLED,                    "*** PHY Disabled ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_LINK_TO_5G,    "*** Link set to 5G ***\n")
    ILOG_ENTRY(NVM_CONFIG_STATUS_LINK_TO_10G,   "*** Link set to 10G ***\n")
    ILOG_ENTRY(LINK_HW_PHY_START,               "HW Phy Start: %d\n")
    ILOG_ENTRY(LINK_ERROR_DETECTED,             "Link error detected, restarting link\n")
    ILOG_ENTRY(LINK_HW_PHY_RESTART,             "Link is not stable, Restart HW Phy\n")
    ILOG_ENTRY(LINK_HW_PHY_MAC_LINK_RX_STATUS,  "Link Status MAC Link Layer Rx: %d\n")
    ILOG_ENTRY(LINK_HW_PHY_XAUI_STATUS,         "Link Status XAUI: %d\n")
    ILOG_ENTRY(LINK_HW_PHY_LINK_STATUS,         "HW Phy Link Status: %d\n")
    ILOG_ENTRY(LINK_NOT_10G_SPEED,              "Aquantia is not running at 10G speed\n")
    ILOG_ENTRY(LINK_MGR_STATE_TRANSITION,       "Link manager: old state = %d, new state = %d on event = %d\n")
    ILOG_ENTRY(LINK_MGR_INVALID_EVENT,          "Link manager: got invalid event %d in state %d\n")

    ILOG_ENTRY(COMLINK_RCV_EVENT,               "COMLINK In state %d, received event %d new state %d\n")
    ILOG_ENTRY(COMLINK_MSG_RCVD,                "COMLINK Msg Rcvd in state %d, msg %d\n")
    ILOG_ENTRY(COMLINK_STATE,                   "COMLINK SysEvent State 0 == DN, 1 == UP: %d\n")
    ILOG_ENTRY(COMLINK_STATE_EVENT_INVALID,     "Comm Link: Invalid Event %d for state %d\n")
    ILOG_ENTRY(COMLINK_LINKUP_TIMEOUT,          "Comlink couldn't bring up in time\n")
    ILOG_ENTRY(COMLINK_CH_DOWN_DELAYED_LINKUP,  "link down - channel down in delayed link up\n")
    ILOG_ENTRY(COMLINK_CH_DOWN_LINKUP,          "link down - channel down in link up\n")
    ILOG_ENTRY(COMLINK_MCA_CH_STATUS,           "ComlinkMcaChannelStatusHandler got Channel status: %d\n")
    ILOG_ENTRY(COMLINK_SECOND_LINKUP,           "REX START 2nd LinkUp for stable train \n")
    
ILOG_END(LINKMGR_COMPONENT, ILOG_MINOR_ERROR)

#endif // LINKMGR_LOG_H
