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
#ifndef MAC_H
#define MAC_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
typedef void (*MacLinkRxStatusChangeHandler)(bool linkStatus);

// Function Declarations ##########################################################################
void MAC_Init( void )                               __attribute__ ((section(".atext")));

void MAC_LinkLayerRxIsr(void);

void MAC_EnableRx(MacLinkRxStatusChangeHandler changeNotification)      __attribute__ ((section(".atext")));
void MAC_EnableTx(void)                                                 __attribute__ ((section(".atext")));
void MAC_EnableLayer3Tx(void)                                           __attribute__ ((section(".atext")));
void MAC_EnableLayer3Rx(void)                       __attribute__ ((section(".atext")));
void MAC_DisableLayer3Tx(void)                      __attribute__ ((section(".atext")));
void MAC_DisableLayer3Rx(void)                      __attribute__ ((section(".atext")));
void MAC_DisableTx(void)                            __attribute__ ((section(".atext")));
void MAC_DisableRx(void)                            __attribute__ ((section(".atext")));

// gap in bytes between each ethernet frame
void MAC_changeFrameRate(uint16_t ethFrameGap)      __attribute__ ((section(".atext")));

void MAC_RemoteFaultEnable(bool enable)             __attribute__ ((section(".atext")));

void MAC_StartStatsMonitor(void)                    __attribute__ ((section(".atext")));
void MAC_StopStatsMonitor(void)                     __attribute__ ((section(".atext")));

#endif // MAC_H

