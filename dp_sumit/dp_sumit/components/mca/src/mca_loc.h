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
#ifndef MCA_LOC_H
#define MCA_LOC_H

// Includes #######################################################################################
#include <ibase.h>
#include <mca.h>
#include <timing_timers.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
TIMING_TimerHandlerT mcaFifoPrintIcmdTimer;

// Function Declarations ##########################################################################
void MCA_ControlStatsMonitorChannel( enum MCA_ChannelNumber channel, bool enable)           __attribute__ ((section(".atext")));
void MCA_StatInit( void )                                                                   __attribute__ ((section(".atext")));
void MCA_ControlStatsMonitorCore(bool enable)                                               __attribute__ ((section(".atext")));
void MCA_coreSetReadClearStats(void)                                                        __attribute__ ((section(".atext")));
void MCA_channelSetReadClearStats(enum MCA_ChannelNumber channelNumber)                     __attribute__ ((section(".atext")));

#endif // MCA_LOC_H

