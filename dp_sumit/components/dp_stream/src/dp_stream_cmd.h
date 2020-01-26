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
#ifndef DP_STREAM_CMD_H
#define DP_STREAM_CMD_H

// Includes #######################################################################################
#include <icmd.h>

// Constants and Macros ###########################################################################
ICMD_FUNCTIONS_CREATE(DP_STREAM_COMPONENT)
#if !defined BB_ISO && !defined BB_USB
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_STREAM_GetVideoInfoIcmd, "Prints out the video information", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_STREAM_SdpStatsIcmd, "Prints out the DP SDP stats", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_STREAM_freqMeasureIcmd, "Measure DP clock", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_STREAM_RexEnableAudioIcmd, "Enables audio on REX", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_STREAM_LexPrintErrCount,   "Prints all the error count\n", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_STREAM_LexResetIdlePatternCnt,  "Resets idle pattern counter", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(DP_STREAM_RexSetAdjustWidthOffset,  "Set dp_width_total_minus/plus_offset0/1. arg0:-off1, 1:+off1, 2:-off0, 3:+off0)", uint8_t, uint8_t, uint8_t, uint8_t)
#endif
ICMD_FUNCTIONS_END(DP_STREAM_COMPONENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // DP_STREAM_CMD_H
