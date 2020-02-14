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
#ifndef RS232_LOG_H
#define RS232_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################

ILOG_CREATE(RS232_COMPONENT)
    ILOG_ENTRY(RS232_UNEXPECTED_CPU_MSG_LENGTH, "Unexpected length of CPU message received, msgLength = %d\n")
    ILOG_ENTRY(RS232_RCV_CPU_MSG,               "Received a CPU message, RS232 device state %d\n")
    ILOG_ENTRY(RS232_MCA_CHANNEL_STATUS,        "RS232 MCA channel status %d\n")
ILOG_END(RS232_COMPONENT, ILOG_DEBUG)

#endif // RS232_LOG_H
