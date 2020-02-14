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
#ifndef I2CD_DP159API_H
#define I2CD_DP159API_H

// Includes #######################################################################################
#include <callback.h>
#include <itypes.h>
#include <bb_top_dp.h>
// Constants and Macros ###########################################################################

// Data Types #####################################################################################
enum Dp159BW
{
    DP159_RX_LINK_BW_540GBPS = 0x14,
    DP159_RX_LINK_BW_270GBPS = 0x0A,
    DP159_RX_LINK_BW_162GBPS = 0x06
};

// Function Declarations ##########################################################################
bool I2CD_dp159InitConfig(void)                                                 __attribute__((section(".lexftext")));
void I2CD_linkTrainingCRPhase(
    void (*linkTrainingClkRecoveryCompleteHandler)(void),
    uint8_t laneCnt,
    enum MainLinkBandwidth bw)                                                  __attribute__((section(".lexftext")));
void I2CD_dp159Reinitialize(void (*resetCompleteHandler)(void))                 __attribute__((section(".lexftext")));
void I2CD_linkTrainingPollForPllLock(
    void (*callback)(bool, enum MainLinkBandwidth, enum LaneCount))             __attribute__((section(".lexftext")));
bool I2CD_dp159InitSuccess(void)                                                __attribute__((section(".lexftext")));
void _I2CD_linkTrainingPllPollFinished(void (*callback)(void))                  __attribute__((section(".lexftext")));
void _I2CD_linkTrainingTPS23Received(void (*callback)(void))                    __attribute__((section(".lexftext")));
void _I2CD_linkTrainingResetRxLane(void (*callback)(void))                          __attribute__((section(".lexftext")));
#endif // I2CD_DP159API_H

