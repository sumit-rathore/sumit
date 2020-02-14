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
#ifndef I2CD_DP130_H
#define I2CD_DP130_H

// Includes #######################################################################################
#include <itypes.h>


// Constants and Macros ###########################################################################


// Data Types #####################################################################################

enum dp130SquelchSensitivity
{
    DP130_SQUELCH_SENSITIVITY_40MVPP,
    DP130_SQUELCH_SENSITIVITY_80MVPP,
    DP130_SQUELCH_SENSITIVITY_160MVPP,
    DP130_SQUELCH_SENSITIVITY_250MVPP
};

enum dp130AuxDdcMuxCfg
{
    DP130_AUX_DDC_MUX_CFG_SRC_DDC,
    DP130_AUX_DDC_MUX_CFG_SRC_TMDS,
    DP130_AUX_DDC_MUX_CFG_SRC_HPD_SNK,
    DP130_AUX_DDC_MUX_CFG_UNDEF
};

enum dp130Boost
{
    DP130_BOOST_PRE_EMPH_DEC_10_PRCNT,
    DP130_BOOST_PRE_EMPH_NOMINAL,
    DP130_BOOST_PRE_EMPH_INC_10_PRCNT,
    DP130_BOOST_PRE_EMPH_RSRVD
};

enum dp130DpTmdsVpre
{
    DP130_DP_TMDS_VPRE_NO_TMDS_PRE_EMPH,
    DP130_DP_TMDS_VPRE_LOW_TMDS_PRE_EMPH,
    DP130_DP_TMDS_VPRE_HIGH_TMDS_PRE_EMPH,
    DP130_DP_TMDS_VPRE_RSRVD
};

enum dp130AeqLaneLevels
{
    DP130_AEQ_LANE0_L0_L1,
    DP130_AEQ_LANE0_L2_L3,
    DP130_AEQ_LANE1_L0_L1,
    DP130_AEQ_LANE1_L2_L3,
    DP130_AEQ_LANE2_L0_L1,
    DP130_AEQ_LANE2_L2_L3,
    DP130_AEQ_LANE3_L0_L1,
    DP130_AEQ_LANE3_L2_L3
};

union I2cdDp130AutoShutDnFrcPwrDn
{
    uint8_t raw;
    struct
    {
        // REG 0x01
        // AUTO_POWERDOWN_DISABLE - 0 automatically enters Standby mode based on HPD_SNK
        //                        - 1 will not automatically enter Standby mode
        // FORCE_SHUTDOWN_MODE - 0 forced to Shutdown mode
        //                     - 1 Shutdown mode is determined by EN input
        uint8_t rsrvd7_2:6;
        uint8_t auto_powerdown_disable:1;
        uint8_t force_shutdown_mode:1; // 0
    } bf;
};

union I2cdDp130Squelch
{
    uint8_t raw;
    struct
    {
        // REG 0x03
         // SQUELCH_SENSITIVITY - 00 IN0p/n threshold set to 40mVpp
         //                     - 01 IN0p/n threshold set to 80mVpp
         //                     - 10 IN0p/n threshold set to 160mVpp
         //                     - 11 IN0p/n threshold set to 250mVpp
         // SQUELCH_ENABLE - 0 IN0p/n squelch detection enabled (default)
         //                - 1 IN0p/n squelch detection disabled
        uint8_t rsrvd7_6:2;
        uint8_t squelch_sensitivity:2;
        uint8_t squelch_enable:1;
        uint8_t rsrvd2_0:3; // 2:0
   } bf;
};

union I2cdDp130EnLinkTrainingAuxSrc
{
    uint8_t raw;
    struct
    {
        // REG 0x04
        // TI_TEST - this field defaults to zero and should not be modified
        // LINK_TRAINING_ENABLE - 0 Link training disabled, VOD and Pre-emph cfgd
        //                          through I2C, EQ fixed when this bit is zero
        //                      - 1 Link training is enabled (default)
        // AUX_DDC_MUX_CFG - 00 AUX_SNK is switched to AUX_SRC for DDC src CAD_SNK (default)
        //                   01 AUX_SNK is switched to AUX_SRC based on CAD_SNK input
        //                   10 AUX_SNK is switched to AUX_SRC side based on HPD_SNK input
        //                      DDC src interface remains disabled
        //                   11 undefined
        uint8_t rsrvd7_4:4; // 7:4
        uint8_t link_training_enable:1;
        uint8_t aux_ddc_mux_cfg:2; // 1:0
   } bf;
};

union I2cdDp130AllLanesLevels
{
    uint8_t raw[8];
    struct
    {
        // REG 0x05 - 0x0C
        //   EQ_I2C_ENABLE applicable to address 0x05 only, Lane0 L0 L1
        // EQ_I2C_ENABLE - 0 EQ settings controlled by device (default)
        //               - 1 EQ settings controlled by I2C registers
        //   EQ_I2C_EN set, DisplayPort sink selected, Link Training enabled, and
        //   Link Training results in Level 0-3 pre-emphasis
        // AEQ_La_LANE_SET - 000 0 dB EQ
        //                  - 001 1.5 dB (HBR), 3.5 dB (HBR2)
        //                  - 010 3 dB (HBR)  , 6 dB (HBR2)
        //                  - 011 4 dB (HBR)  , 8 dB (HBR2)
        //                  - 100 5 dB (HBR)  , 10 dB (HBR2)
        //                  - 101 6 dB (HBR)  , 13 dB (HBR2)
        //                  - 110 7 dB (HBR)  , 15 dB (HBR2)
        //                  - 111 9 dB (HBR)  , 18 dB (HBR2)
        // AEQ_Lb_LANE_SET - 000 0 dB EQ
        //                  - 001 1.5 dB (HBR), 3.5 dB (HBR2)
        //                  - 010 3 dB (HBR)  , 6 dB (HBR2)
        //                  - 011 4 dB (HBR)  , 8 dB (HBR2)
        //                  - 100 5 dB (HBR)  , 10 dB (HBR2)
        //                  - 101 6 dB (HBR)  , 13 dB (HBR2)
        //                  - 110 7 dB (HBR)  , 15 dB (HBR2)
        //                  - 111 9 dB (HBR)  , 18 dB (HBR2)

        // REG 0x05
        uint8_t eq_i2c_enable:1; // 7
        uint8_t aeq_l0_lane0_set:3;
        uint8_t lane0_l0_l1_rsrvd3:1;
        uint8_t aeq_l1_lane0_set:3; // 2:0
        // REG 0x06
        uint8_t lane0_l2_l3_rsrvd7:1; // 7
        uint8_t aeq_l2_lane0_set:3;
        uint8_t lane0_l2_l3_rsrvd3:1;
        uint8_t aeq_l3_lane0_set:3; // 2:0
        // REG 0x07
        uint8_t lane1_l0_l1_rsrvd7:1; // 7
        uint8_t aeq_l0_lane1_set:3;
        uint8_t lane1_l0_l1_rsrvd3:1;
        uint8_t aeq_l1_lane1_set:3; // 2:0
        // REG 0x08
        uint8_t lane1_l2_l3_rsrvd7:1; // 7
        uint8_t aeq_l2_lane1_set:3;
        uint8_t lane1_l2_l3_rsrvd3:1;
        uint8_t aeq_l3_lane1_set:3; // 2:0
        // REG 0x09
        uint8_t lane2_l0_l1_rsrvd7:1; // 7
        uint8_t aeq_l0_lane2_set:3;
        uint8_t lane2_l0_l1_rsrvd3:1;
        uint8_t aeq_l1_lane2_set:3; // 2:0
        // REG 0x0A
        uint8_t lane2_l2_l3_rsrvd7:1; // 7
        uint8_t aeq_l2_lane2_set:3;
        uint8_t lane2_l2_l3_rsrvd3:1;
        uint8_t aeq_l3_lane2_set:3; // 2:0
        // REG 0x0B
        uint8_t lane3_l0_l1_rsrvd7:1; // 7
        uint8_t aeq_l0_lane3_set:3;
        uint8_t lane3_l0l1_rsrvd3:1;
        uint8_t aeq_l1_lane3_set:3; // 2:0
        // REG 0x0C
        uint8_t lane3_l2_l3_rsrvd7:1; // 7
        uint8_t aeq_l2_lane3_set:3;
        uint8_t lane3_l2l3_rsrvd3:1;
        uint8_t aeq_l3_lane3_set:3; // 2:0
    } bf;
};

union I2cdDp130BoostDpTmdsVodVpre
{
    uint8_t raw;
    struct
    {
        // REG 0x15
        // BOOST - controls output pre-emph amplitude when DP Snk selected, also impacts TMDS
        //         output mode for DP sink when DP sink CAD_SNK is high
        //       - 00 - Pre-emph -10%, Vod -10% if pre-emph disabled
        //       - 01 - Pre-emph normal (default)
        //       - 10 - Pre-emph +10%, Vod +10% if pre-emph disabled
        //       - 11 - Reserved
        // DP_TMDS_VOD - 0 low TMDS swing (default)
        //             - 1 high TMDS swing
        // DP_TMDS_VPRE - 00 no TMDS pre-emph (default)
        //              - 01 low TMDS pre-emph
        //              - 10 high TMDS pre-emph
        //              - 11 rsrvd
        uint8_t rsrvd7_5:3; // 7:5
        uint8_t boost:2;
        uint8_t dp_tmds_vod:1;
        uint8_t dp_tmds_vpre:2; // 1:0
    } bf;
};

union I2cdDp130HpdCadTstMode
{
    uint8_t raw;
    struct
    {
        // REG 0x17
        // HPD_TEST_MODE - 0 normal HPD mode, HPD_SRC reflects status of HPD_SNK (default)
        //               - 1 test mode HPD_SNK pull high internally, HPD_SRC driven high, and main
        //                   link activated depending on squelch setting - allows certain
        //                   tests without being connected to display sink
        // CAD_OUTPUT_INVERT - 0 CAD_SRC output high means TMDS cable adapter detected (dflt)
        //                   - 1 CAD_SRC output low means TMDS cable adapter detected
        // CAD_TEST_MODE - 0 normal CAD mode, CAD_SRC reflects status of CAD_SNK, based on val
        //                   of CAD_OUTPUT_INVERT
        //               - 1 test mode, CAD_SRC indicates TMDS mode depending on CAD_OUTPUT_INVERT,
        //                   CAD_SNK input ignored, allows testing without TMDS display sink
        uint8_t rsrvd7_4:4; // 7:4
        uint8_t hpd_test_mode:1;
        uint8_t rsrvd2:1;
        uint8_t cad_output_invert:1;
        uint8_t cad_test_mode:1; // 0
    } bf;
};

union I2cdDp130I2cSoftRstDpcdRst // write only!
{
    uint8_t raw;
    struct
    {
       // REG 0x1B
       // I2C_SOFT_RESET - 1 resets all I2C regs to default values, write only
       // DPCD_RESET - 1 resets DPCD register bits (DPCD addrs 103h-106h, AEQ_Lx_LANEy_SET bits),
       //              write only
        uint8_t i2c_soft_reset:1; // 7
        uint8_t dpcd_reset:1;
        uint8_t rsrvd5_0:6; // 5:0
    } bf;
};

union I2cdDp130DpcdAddr
{
    uint8_t raw[3];
    struct
    {
        // REG 0x1C/0x1D/0x1E
        // DPCD_ADDR - 19:16 - offset 0x1C
        //           - 15:08 - offset 0x1D
        //           - 07:00 - offset 0x1E
        uint32_t rsrvd7_4:4; // 7:4
        uint32_t dpcd_addr:20; // 19:0
        uint32_t :8; // inaccessible padding to ensure we line up with raw
    } bf;
};

union I2cdDp130DpcdData
{
    uint8_t raw;
    struct
    {
        // REG 0x1F
        // DPCD_DATA - data to write to/read from DPCD reg address by DpcdAddr
        uint8_t dpcd_data;
    } bf;
};

union I2cdDp130DevIdRevBitInvert
{
    uint8_t raw;
    struct
    {
       // REG 0x20
        // DEV_ID_REV - all zeros, read only
        // BIT_INVERT - value read is inverse of written, default read is zero
        uint8_t dev_id_rev:7; // 7:1
        uint8_t bit_invert:1; // 0
    } bf;
};

// Function Declarations ##########################################################################
void I2CD_dp130Init(void)                       __attribute__((section(".rexatext")));
void I2CD_dp130Disable(void)                    __attribute__((section(".rexatext")));
void I2CD_dp130Enable(void (*callback)(bool))   __attribute__((section(".rexatext")));
bool I2CD_dp130InitSuccess(void)                __attribute__((section(".rexatext")));

void I2CD_setDp130AutoShutDnFrcPwrDn(
    const union I2cdDp130AutoShutDnFrcPwrDn* reg, void (*writeCompleteHandler)(void));
void I2CD_getDp130AutoShutDnFrcPwrDn(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount));

void I2CD_setDp130Squelch(const union I2cdDp130Squelch* reg, void (*writeCompleteHandler)(void));
void I2CD_getDp130Squelch(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount));

void I2CD_setDp130EnLinkTrainingAuxSrc(
    const union I2cdDp130EnLinkTrainingAuxSrc* reg, void (*writeCompleteHandler)(void));
void I2CD_getDp130EnLinkTrainingAuxSrc(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount));

void I2CD_setDp130AllLanesLevels(
    const union I2cdDp130AllLanesLevels* reg, void (*writeCompleteHandler)(void));
void I2CD_getDp130AllLanesLevels(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount));

void I2CD_setDp130BoostDpTmdsVodVpre(
    const union I2cdDp130BoostDpTmdsVodVpre* reg, void (*writeCompleteHandler)(void));
void I2CD_getDp130BoostDpTmdsVodVpre(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount));

void I2CD_setDp130HpdCadTstMode(
    const union I2cdDp130HpdCadTstMode* reg, void (*writeCompleteHandler)(void));
void I2CD_getDp130HpdCadTstMode(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount));

void I2CD_setDp130I2cSoftRstDpcdRst(
    const union I2cdDp130I2cSoftRstDpcdRst* reg, void (*writeCompleteHandler)(void));
void I2CD_getDp130I2cSoftRstDpcdRst(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount));

void I2CD_setDp130DpcdAddr(
    const union I2cdDp130DpcdAddr* reg, void (*writeCompleteHandler)(void));
void I2CD_getDp130DpcdAddr(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount));

void I2CD_setDp130DpcdData(
    const union I2cdDp130DpcdData* reg, void (*writeCompleteHandler)(void));
void I2CD_getDp130DpcdData(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount));

void I2CD_setDp130DevIdRevBitInvert(
    const union I2cdDp130DevIdRevBitInvert* reg, void (*writeCompleteHandler)(void));
void I2CD_getDp130DevIdRevBitInvert(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount));



#endif // I2CD_DP130_H

