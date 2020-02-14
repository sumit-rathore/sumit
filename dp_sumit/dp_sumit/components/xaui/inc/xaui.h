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
#ifndef XAUI_H
#define XAUI_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
typedef void (*XauiStatusChangeHandler)(bool linkStatus);

// Function Declarations ##########################################################################
void XAUI_Init(
    XauiStatusChangeHandler changeNotification)             __attribute__((section(".atext")));
void XAUI_Control(bool enable)                              __attribute__((section(".atext")));
void XAUI_EnableRx(void)                                    __attribute__((section(".atext")));
void XAUI_DisableRx(void)                                   __attribute__((section(".atext")));
void XAUI_ReSyncRx(void)                                    __attribute__((section(".atext")));
bool XAUI_isTxAlignCompleted(void)                          __attribute__((section(".atext")));
void XAUI_AlignmentStatusIsr(void);
bool XAUI_CheckRxAlignedStatus(void);
bool XAUI_GtxErrorStatus(void)                              __attribute__((section(".atext")));
void XAUI_ToggleGtRxReset(void)                             __attribute__((section(".atext")));
void XAUI_RxAlignmentReporting(bool enable)                 __attribute__((section(".atext")));
void XAUI_EnableStats(void)                                 __attribute__((section(".atext")));
#endif // XAUI_H

