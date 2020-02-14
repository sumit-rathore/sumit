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
#ifndef LAN_PORT_LOC_H
#define LAN_PORT_LOC_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// CPU messages sent from one device to the other
enum LanPortCpuCommMessage
{
    LAN_PORT_CPU_COMM_DEVICE_CONNECTED,     // device connected to the remote's Lan Port
    LAN_PORT_CPU_COMM_DEVICE_REMOVED,       // device disconnected from the remote's Lan Port
};


// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // LAN_PORT_LOC_H
