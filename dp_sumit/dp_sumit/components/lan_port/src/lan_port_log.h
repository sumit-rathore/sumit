//#################################################################################################
// Icron Technology Corporation - Copyright 2017
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef LAN_PORT_LOG_H
#define LAN_PORT_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################

ILOG_CREATE(LAN_PORT_COMPONENT)
    ILOG_ENTRY(LAN_PORT_COMLINK_UP_EVENT,           "Lan Port, comlink event, link is ** UP **\n")
    ILOG_ENTRY(LAN_PORT_COMLINK_DOWN_EVENT,         "Lan Port, comlink event, link is ** DOWN **\n")
    ILOG_ENTRY(LAN_PORT_PHY_INIT_COMPLETED_1G,      "1G GMII PHY init completed\n")
    ILOG_ENTRY(LAN_PORT_UNEXPECTED_CPU_MSG_LENGTH,  "Unexpected length of CPU message received, msgLength = %d\n")
    ILOG_ENTRY(LAN_PORT_RCV_CPU_MSG,                "Received a CPU message, PHY Link Remote State %d, PHY Remote Speed %d\n")
    ILOG_ENTRY(LAN_PORT_TX_CPU_MSG,                 "Transmitting a CPU message, Header %d, Speed %d\n")
    ILOG_ENTRY(LAN_PORT_PHY_INIT_COMPLETED_100M,    "100M MII PHY init completed\n")
    ILOG_ENTRY(LAN_PORT_DEVICE_STATE_CHANGE,        "Lan Port device state %d at speed %d\n")
    ILOG_ENTRY(LAN_PORT_MCA_CHANNEL_STATUS,         "Lan Port channel status %d\n")
ILOG_END(LAN_PORT_COMPONENT, ILOG_DEBUG)

#endif // LAN_PORT_LOG_H
