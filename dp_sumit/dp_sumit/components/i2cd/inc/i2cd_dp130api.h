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
#ifndef I2CD_DP130API_H
#define I2CD_DP130API_H

// Includes #######################################################################################
#include <itypes.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
enum dp130AeqLaneGainHbrSetting
{
    DP130_AEQ_LANE_GAIN_HBR_0_DB,
    DP130_AEQ_LANE_GAIN_HBR_1_5_DB,
    DP130_AEQ_LANE_GAIN_HBR_3_DB,
    DP130_AEQ_LANE_GAIN_HBR_4_DB,
    DP130_AEQ_LANE_GAIN_HBR_5_DB,
    DP130_AEQ_LANE_GAIN_HBR_6_DB,
    DP130_AEQ_LANE_GAIN_HBR_7_DB,
    DP130_AEQ_LANE_GAIN_HBR_9_DB
};

// HBR2 currently not supported, 2016_01_20
enum dp130AeqLaneGainHbr2Setting
{
    DP130_AEQ_LANE_GAIN_HBR2_0_DB,
    DP130_AEQ_LANE_GAIN_HBR2_3_5_DB,
    DP130_AEQ_LANE_GAIN_HBR2_6_DB,
    DP130_AEQ_LANE_GAIN_HBR2_8_DB,
    DP130_AEQ_LANE_GAIN_HBR2_10_DB,
    DP130_AEQ_LANE_GAIN_HBR2_13_DB,
    DP130_AEQ_LANE_GAIN_HBR2_15_DB,
    DP130_AEQ_LANE_GAIN_HBR2_18_DB
};


// Function Declarations ##########################################################################
void I2CD_dp130WriteToDpcdRegisters(
    uint32_t addr, uint8_t data, void (*taskCompleteHandler)(void));
void I2CD_dp130SetLinkTraining(bool enabled, void (*taskCompleteHandler)(void));
void I2CD_dp130ResetChip(void (*taskCompleteHandler)(void));
void I2CD_dp130ResetDPCD(void (*taskCompleteHandler)(void));

#endif // I2CD_DP130API_H

