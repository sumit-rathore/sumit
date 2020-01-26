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
#ifndef BB_TOP_GE_H
#define BB_TOP_GE_H

// Includes #######################################################################################
#include <ibase.h>
#include <itypes.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
typedef void (*WatchdogCallback)(void);

enum GeUartSlaveSel
{
    GE_UART_SLAVE_SEL_MOTHERBOARD,
    GE_UART_SLAVE_SEL_GE_UART
};

// Function Declarations ##########################################################################
void bb_top_ge_Init(void)                                                               __attribute__((section(".atext")));
void bb_top_SetGEToRunMode(WatchdogCallback resetHandler, WatchdogCallback runHandler)  __attribute__((section(".atext")));
void bb_top_SetGEToResetMode(void)                                                      __attribute__((section(".atext")));
void bb_top_SetGEToBootloaderMode(void)                                                 __attribute__((section(".atext")));
void bb_top_StartGEWatchdogRunningTimer(void)                                           __attribute__((section(".atext")));
void bb_top_StopGEWatchdogResetTimer(void)                                              __attribute__((section(".atext")));
void bb_top_StopGEWatchdogRunningTimer(void)                                            __attribute__((section(".atext")));
void bb_top_ApplyGEReset(bool reset)                                                    __attribute__((section(".atext")));
bool bb_top_isGEResetOn(void)                                                           __attribute__((section(".atext")));
void bb_top_ApplyGEInterrupt(bool setInt);
void bb_top_ControlGeDataPhy(bool reset);
void bb_top_ApplyGEVbusDetect(bool setVbus);

#endif // BB_TOP_GE_H
