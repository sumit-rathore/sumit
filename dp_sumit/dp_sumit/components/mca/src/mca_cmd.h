//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef MCA_CMD_H
#define MCA_CMD_H

// Includes #######################################################################################
#include <icmd.h>

// Constants and Macros ###########################################################################
ICMD_FUNCTIONS_CREATE(MCA_COMPONENT)
    ICMD_FUNCTIONS_ENTRY_FLASH(PrintLatencyValueIcmd, "Prints the Latency Value of MCA_CHANNEL[0] if it is not zero", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MCA_ChannelLinkUp, "Channel Link up (Channel number)", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MCA_ChannelLinkDn, "Channel Link dn (Channel number)", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MCA_IcmdPrintFifoLevel, "Set the time interval and start printing pfifo and nfifo levels (Set in msec)\n Setting the interval to Zero will stop the timer", uint32_t)
ICMD_FUNCTIONS_END(MCA_COMPONENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // MCA_CMD_H

