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

//#################################################################################################
// Module Description
//#################################################################################################
// This module provides the api interface to the TI DP130 Redriver.
// The intent of this module is to queue the sequence of writes/reads required by the lower level
// driver to send off to the device.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// Each function is meant to be a simple high level interface for the DP operations, abstracting
// the operations required when changing a parameter, such as lane count.
//#################################################################################################


// Includes #######################################################################################
#include <i2cd_dp130.h>
#include <i2cd_dp130api.h>
#include "i2cd_log.h"
#include <ibase.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
enum DpcdAddrState
{
    DPCD_ADDR_WRITE_NOT_REQUIRED,
    DPCD_ADDR_WRITE_REQUIRED
};

// Global Variables ###############################################################################

// Static Variables ###############################################################################

static struct
{
    void (*taskCompletionHandler)(void);
    enum DpcdAddrState dpcdAddrState;
    union I2cdDp130EnLinkTrainingAuxSrc reg04;
    union I2cdDp130DpcdAddr dpcdAddr;
    union I2cdDp130DpcdData dpcdData;
    union I2cdDp130AllLanesLevels levels;
} dp130ApiCtx __attribute__((section(".rexbss")));


// Static Function Declarations ###################################################################
// static void _I2CD_dp130SetEqualizationLevels(void) __attribute__((section(".rexftext")));
static void _I2CD_dp130WriteToDpcdAddressData(void) __attribute__((section(".rexftext")));

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize the dp130 chip, performing many writes
//
// Parameters:
//      initCompleteHandler - call back when configuration completes
// Return:
// Assumptions:
//      * This function assumes chip has been reset or powered up,
//        and 400ms passed from releasing reset (operation available after 400ms)
//#################################################################################################
// void I2CD_dp130InitConfig(void)
// {
//     I2CD_dp130SetEqEnable();
// }


//#################################################################################################
// Set AEQ Levels for all Lanes, firstly ensure link training is disabled,
//
// Parameters:
//      taskCompleteHandler     - call back after setting lane levels
// Return:
// Assumptions:
//      * This function disables link_training_enable and assumes it is re-enabled elsewhere
//      * This function assumes caller will set EQ_I2C_ENABLE
//#################################################################################################
// static void _I2CD_dp130SetEqualizationLevels(void)
// {
//     dp130ApiCtx.levels.bf.aeq_l3_lane3_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l2_lane3_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l1_lane3_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l0_lane3_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l3_lane2_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l2_lane2_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l1_lane2_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l0_lane2_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l3_lane1_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l2_lane1_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l1_lane1_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l0_lane1_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l3_lane0_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l2_lane0_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l1_lane0_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.aeq_l0_lane0_set = DP130_AEQ_LANE_GAIN_HBR_0_DB;
//     dp130ApiCtx.levels.bf.eq_i2c_enable = 1;
//     I2CD_setDp130AllLanesLevels(&(dp130ApiCtx.levels), dp130ApiCtx.taskCompletionHandler);
// }


//#################################################################################################
// Write to DPCD registers, this function simply enqueues writes, use I2CD_dp130PerformWrites to
// initiate I2C transactions
//
// Parameters:
//      success - write operation status
// Return:
// Assumptions:
//      * This function assumes caller will use I2CD_dp130PerformWrites to start i2c operations
//#################################################################################################
void I2CD_dp130WriteToDpcdRegisters(
    uint32_t addr, uint8_t data, void (*taskCompleteHandler)(void))
{
    dp130ApiCtx.taskCompletionHandler = taskCompleteHandler;

    // Flag so we can control callbacks
    if (dp130ApiCtx.dpcdAddr.bf.dpcd_addr != addr)
    {
        dp130ApiCtx.dpcdAddrState = DPCD_ADDR_WRITE_REQUIRED;
    }
    dp130ApiCtx.dpcdAddr.bf.dpcd_addr = addr;
    dp130ApiCtx.dpcdData.bf.dpcd_data = data;

    // disable link training before setting EQ levels
    if (dp130ApiCtx.reg04.bf.link_training_enable == 1)
    {
        dp130ApiCtx.reg04.bf.link_training_enable = 0;
        I2CD_setDp130EnLinkTrainingAuxSrc(&dp130ApiCtx.reg04, &_I2CD_dp130WriteToDpcdAddressData);
    }
    else
    {
        // no link training disable required
        // need to write address?
        if (dp130ApiCtx.dpcdAddrState == DPCD_ADDR_WRITE_REQUIRED)
        {
            dp130ApiCtx.dpcdAddrState = DPCD_ADDR_WRITE_NOT_REQUIRED;
            I2CD_setDp130DpcdAddr(&(dp130ApiCtx.dpcdAddr), &_I2CD_dp130WriteToDpcdAddressData);
        }
        else
        {
            I2CD_setDp130DpcdData(&(dp130ApiCtx.dpcdData), dp130ApiCtx.taskCompletionHandler);
        }
    }
}


//#################################################################################################
// Write DPCD Address and data, we've stored addres and data and flag if we need to write address
// or not, so we can reuse this function
//
// Parameters:
// Return:
// Assumptions:
//      * This function assumes caller will use I2CD_dp130PerformWrites to start i2c operations
//#################################################################################################
static void _I2CD_dp130WriteToDpcdAddressData(void)
{
    // callback of link_training_enable = 0
    // do we need to write the address?
    if (dp130ApiCtx.dpcdAddrState == DPCD_ADDR_WRITE_REQUIRED)
    {
        dp130ApiCtx.dpcdAddrState = DPCD_ADDR_WRITE_NOT_REQUIRED;
        // when we callback we'll write the data
        I2CD_setDp130DpcdAddr(&(dp130ApiCtx.dpcdAddr), &_I2CD_dp130WriteToDpcdAddressData);
    }
    else
    {
        // callback of write address
        // write data
        I2CD_setDp130DpcdData(&(dp130ApiCtx.dpcdData), dp130ApiCtx.taskCompletionHandler);
    }
}


//#################################################################################################
// Write to DP130 to enable/disable link training
//
// Parameters:
//      success - write operation status
// Return:
// Assumptions:
//      * This function assumes caller will use I2CD_dp130PerformWrites to start i2c operations
//#################################################################################################
void I2CD_dp130SetLinkTraining(bool enabled, void (*taskCompleteHandler)(void))
{
    // check to see if we can save 3 I2C writes (transactions, not block writes)
    dp130ApiCtx.reg04.bf.link_training_enable = enabled ? 1 : 0;
    I2CD_setDp130EnLinkTrainingAuxSrc(&dp130ApiCtx.reg04, taskCompleteHandler);
}


//#################################################################################################
// Write to DP130 to reset DP130 chip
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_dp130ResetChip(void (*taskCompleteHandler)(void))
{
    union I2cdDp130I2cSoftRstDpcdRst rst = { .bf.i2c_soft_reset = 1, .bf.dpcd_reset = 0 };
    I2CD_setDp130I2cSoftRstDpcdRst(&rst, taskCompleteHandler);
}


//#################################################################################################
// Write to DP130 to reset DPCD registers
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_dp130ResetDpcd(void (*taskCompleteHandler)(void))
{
    union I2cdDp130I2cSoftRstDpcdRst rst = { .bf.i2c_soft_reset = 0, .bf.dpcd_reset = 1 };
    I2CD_setDp130I2cSoftRstDpcdRst(&rst, taskCompleteHandler);
}


