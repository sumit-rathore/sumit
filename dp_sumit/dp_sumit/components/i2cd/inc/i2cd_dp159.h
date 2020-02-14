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
#ifndef I2CD_DP159_H
#define I2CD_DP159_H

// Includes #######################################################################################
#include <itypes.h>
#include <i2cd_dp159api.h>

// Constants and Macros ###########################################################################
// Required by the Xmacro below
#define I2CD_DP159_DEVICE_ADDRESS           (0x5E)
#define I2CD_DP159_DEVICE_SPEED             I2C_SPEED_FAST
#define I2CD_DP159_DEVICE_SW_PORT           (0xFF) // Not through switch
#ifdef PLATFORM_A7
#define I2CD_DP159_DEVICE_RTLMUX_PORT       (1) // Motherboard
#endif
#ifdef PLATFORM_K7
#define I2CD_DP159_DEVICE_RTLMUX_PORT       (1) // Inrevium
#endif
// Register Addresses, first byte of data sent to DP159
#define I2CD_DP159_MISC_CTRL_REG0           (0x09)
#define I2CD_DP159_MISC_CTRL_REG1           (0x0A)
#define I2CD_DP159_HDMI_CTRL_REG0           (0x0B)
#define I2CD_DP159_HDMI_CTRL_REG1           (0x0C)

#define I2CD_DP159_EQ_CTRL_REG              (0x0D)

#define I2CD_DP159_EYESCAN_CTRL_REG0        (0x0E)
#define I2CD_DP159_EYESCAN_CTRL_REG1        (0x0F)
#define I2CD_DP159_EYESCAN_CTRL_REG2        (0x10)
#define I2CD_DP159_EYESCAN_CTRL_REG3        (0x11)
#define I2CD_DP159_EYESCAN_CTRL_REG4        (0x12)
#define I2CD_DP159_EYESCAN_CTRL_REG5        (0x13)
#define I2CD_DP159_EYESCAN_CTRL_REG6        (0x14)
#define I2CD_DP159_EYESCAN_CTRL_REG7        (0x15)
#define I2CD_DP159_EYESCAN_CTRL_REG8        (0x16)
#define I2CD_DP159_EYESCAN_CTRL_REG9        (0x17)
#define I2CD_DP159_EYESCAN_CTRL_REGA        (0x18)
#define I2CD_DP159_EYESCAN_CTRL_REGB        (0x19)
#define I2CD_DP159_EYESCAN_CTRL_REGC        (0x1A)
#define I2CD_DP159_EYESCAN_CTRL_REGD        (0x1B)
#define I2CD_DP159_EYESCAN_CTRL_REGE        (0x1C)
#define I2CD_DP159_EYESCAN_CTRL_REGF        (0x1D)
#define I2CD_DP159_EYESCAN_CTRL_REG10       (0x1E)
#define I2CD_DP159_EYESCAN_CTRL_REG11       (0x1F)
#define I2CD_DP159_EYESCAN_CTRL_REG12       (0x20)

#define I2CD_DP159_PLL_CTRL_STAT_REG0       (0x00)
#define I2CD_DP159_PLL_CTRL_STAT_REG1       (0x01)
#define I2CD_DP159_PLL_CTRL_STAT_REG2       (0x02)
#define I2CD_DP159_PLL_CTRL_STAT_REG3       (0x04)
#define I2CD_DP159_PLL_CTRL_STAT_REG4       (0x05)
#define I2CD_DP159_PLL_CTRL_STAT_REG5       (0x08)
#define I2CD_DP159_PLL_CTRL_STAT_REG6       (0x0B)
#define I2CD_DP159_PLL_CTRL_STAT_REG7       (0x0D)
#define I2CD_DP159_PLL_CTRL_STAT_REG8       (0x0E)
#define I2CD_DP159_PLL_CTRL_STAT_REG9       (0xA1)
#define I2CD_DP159_PLL_CTRL_STAT_REGA       (0xA4)

#define I2CD_DP159_TX_CTRL_REG0             (0x10)
#define I2CD_DP159_TX_CTRL_REG1             (0x11)
#define I2CD_DP159_TX_CTRL_REG2             (0x12)
#define I2CD_DP159_TX_CTRL_REG3             (0x13)
#define I2CD_DP159_TX_CTRL_REG4             (0x14)

#define I2CD_DP159_RX_CTRL_REG0             (0x30)
#define I2CD_DP159_RX_CTRL_REG1             (0x31)
#define I2CD_DP159_RX_CTRL_REG2             (0x32)
#define I2CD_DP159_RX_CTRL_REG3             (0x33)
#define I2CD_DP159_RX_CTRL_REG4             (0x34)
#define I2CD_DP159_RX_CTRL_REG5             (0x3C)

#define I2CD_DP159_RX_EQ_CTRL_STAT_REG0     (0x4C)
#define I2CD_DP159_RX_EQ_CTRL_STAT_REG1     (0x4D)
#define I2CD_DP159_RX_EQ_CTRL_STAT_REG2     (0x4E)
#define I2CD_DP159_RX_EQ_CTRL_STAT_REG3     (0x4F)

#define I2CD_DP159_PAGE_SEL_REG             (0xFF)

#define DP159_I2C_DATA_RATE_100KBPS             (0x02)
#define DP159_I2C_DATA_RATE_400KBPS             (0x03)
#define DP159_MISC_CTRL1_INIT                   (0x36)
#define DP159_MISC_CTRL2_INIT                   (0x7B)
#define DP159_MISC_CTRL2_INIT_COMPLETE          (0x3B)
#define DP159_EQ_CTRL_INIT                      (0x80)
#define DP159_HDMI_CTRL_HDMI_SEL_HDMI           (0x00)
#define DP159_HDMI_CTRL_HDMI_SEL_DVI            (0x01)
#define DP159_HDMI_CTRL_TWPST1_NO_DE_EMPH       (0x00)
#define DP159_HDMI_CTRL_TWPST1_2DB_DE_EMPH      (0x01)
#define DP159_HDMI_CTRL2_INIT ((DP159_HDMI_CTRL_VSWING_DATA_INC_21_PRCNT << 5) | \
                               (DP159_HDMI_CTRL_VSWING_CLK_INC_21_PRCNT << 2)  | \
                                DP159_HDMI_CTRL_TWPST1_2DB_DE_EMPH)
#define DP159_EYE_SCAN_CTRL_REG_PV_CP20_16BITS  (0x00)
#define DP159_EYE_SCAN_CTRL_REG_PV_CP20_20BITS  (0x01)
#define DP159_EYE_SCAN_CTRL_TST_CFG_PVDPEN_TSTSEL_INIT \
    (((DP159_LANE0 | DP159_LANE1 | DP159_LANE2 | DP159_LANE3) << 4) | \
     DP159_EYE_SCAN_CTRL_REG_DP_TST_SEL_FIFO_ERR)
#define DP159_PLL_CTRL_STAT_CFG_EN_BANDGAP      (0x02)
#define DP159_PLL_CTRL_STAT_PLL_FB_DIV0         (0x80)
#define DP159_PLL_CTRL_STAT_PLL_FB_DIV1         (0x00)
#define DP159_PLL_CTRL_STAT_PLL_FB_DIV_INT      (0x10)
#define DP159_PLL_CTRL_STAT_PLL_FB_DIV_FR       (0x00)
#define DP159_PLL_CTRL_STAT_PLL_FB_PRE_DIV      (0x00)
#define DP159_PLL_CTRL_STAT_CLK_IS_LANE_3       (0x01)
#define DP159_PLL_CTRL_STAT_CLK_IS_LANE_0       (0x02)
#define DP159_PLL_CTRL_STAT_CDR_CLK_SRC_LANE_3  (0x00)
#define DP159_PLL_CTRL_STAT_CDR_CLK_SRC_LANE_0  (0x01)
#define DP159_PLL_CTRL_STAT_CDR_PLL_CML_MUX_EN  (0x01)
#define DP159_PLL_CTRL_STAT_CDR_PLL_CML_MUX_DEN (0x00)
#define DP159_PLL_CTRL_STAT_CDR_CLKSRC_LN0_MXEN (0x03)
#define DP159_PLL_CTRL_STAT_CP_FULL             (0x3F)
#define DP159_PLL_CTRL_STAT_CP_HBR2             (0x2B) // 5F in draft doc
#define DP159_PLL_CTRL_STAT_CP_HBR              (0x27)
#define DP159_PLL_CTRL_STAT_CP_RBR              (0x1F)
#define DP159_PLL_CTRL_STAT_SEL_CFG_INIT        (0x33)
#define DP159_PLL_CTRL_STAT_PHYTRM_OVR_INIT     (0x02)
#define DP159_PLL_CTRL_STAT_P1_REG01_OVR_INIT   (0x02)
#define DP159_TX_CTRL_SLEW                      (0x00)
#define DP159_TX_CTRL_SWING_600MV               (0x03)
#define DP159_TX_CTRL_INIT ((DP159_LANE0 | DP159_LANE1 | DP159_LANE2 | DP159_LANE3) << 4)
#define DP159_TX_CTRL_RATE_INV_POL_INIT ((DP159_TX_CTRL_TX_RATE_FULL << 6) | \
                                         (DP159_TX_CTRL_TX_TERM_75_TO_150_OHMS << 4))
#define DP159_TX_CTRL_SLEW_INIT ((DP159_TX_CTRL_SLEW << 4) | DP159_TX_CTRL_SWING_600MV)
#define DP159_TX_CTRL_LD_FIR_UPD_INIT0 \
    (((DP159_LANE0 | DP159_LANE1 | DP159_LANE2 | DP159_LANE3) << 4) | \
      (DP159_LANE0 | DP159_LANE1 | DP159_LANE2 | DP159_LANE3))
#define DP159_TX_CTRL_LD_FIR_UPD_INIT1          (0x00)
#define DP159_RX_CTRL_EQ_LEV                    (0x08)
#define DP159_RX_CTRL_CDR_CTRL_STL              (0x04)
#define DP159_RX_CTRL_INIT ((DP159_LANE1 | DP159_LANE2 | DP159_LANE3) << 4)
#define DP159_RX_EQ_CTRL_FTC_LEV_INIT ((DP159_RX_EQ_CTRL_ZERO_FREQ_HBR2 << 4) | \
                                        DP159_RX_CTRL_EQ_LEV)
#define DP159_RX_EQ_CTRL_STAT_INIT              (0x01)
#define DP159_RX_CTRL_ENOC_INIT                 (0x01)
#define DP159_RX_CTRL_LD_PD_INTERPOLATOR_INIT0 \
    ((DP159_LANE0 | DP159_LANE1 | DP159_LANE2 | DP159_LANE3) << 4)
#define DP159_RX_CTRL_LD_PD_INTERPOLATOR_INIT1  (0x00)
#define DP159_RX_CTRL_EQ_LD_INIT  ((DP159_LANE0 | DP159_LANE1 | DP159_LANE2 | DP159_LANE3) << 4)
#define DP159_LBW_HBR2                          DP159_RX_LINK_BW_540GBPS
#define DP159_LBW_HBR                           DP159_RX_LINK_BW_270GBPS
#define DP159_LBW_RBR                           DP159_RX_LINK_BW_162GBPS


// size is in bytes
#define INIT_STRUCT(_regAddr, _size, _page) {.regAddr = _regAddr, .size = _size, .page = _page}

#define REG_LIST \
    X(MISC_CTRL1, INIT_STRUCT(I2CD_DP159_MISC_CTRL_REG0, 1, 0)) \
    X(MISC_CTRL2, INIT_STRUCT(I2CD_DP159_MISC_CTRL_REG1, 1, 0)) \
    X(HDMI_CTRL1, INIT_STRUCT(I2CD_DP159_HDMI_CTRL_REG0, 1, 0)) \
    X(HDMI_CTRL2, INIT_STRUCT(I2CD_DP159_HDMI_CTRL_REG1, 1, 0)) \
    X(EQ_CTRL, INIT_STRUCT(I2CD_DP159_EQ_CTRL_REG, 1, 0)) \
    X(EYESCAN_CTRL0, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REG0, 1, 0)) \
    X(EYESCAN_CTRL1, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REG1, 1, 0)) \
    X(EYESCAN_CTRL_CUSTPATT_CFG, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REG2, 1, 0)) \
    X(EYESCAN_CTRL_CUSTPATT_DATA0, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REG3, 1, 0)) \
    X(EYESCAN_CTRL_CUSTPATT_DATA1, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REG4, 1, 0)) \
    X(EYESCAN_CTRL_CUSTPATT_DATA2, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REG5, 1, 0)) \
    X(EYESCAN_CTRL_PATT_VERTHR, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REG6, 1, 0)) \
    X(EYESCAN_CTRL_TEST_CFG, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REG7, 1, 0)) \
    X(EYESCAN_CTRL_TEST_CFG_PVDPEN_TSTSEL, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REG8, 1, 0)) \
    X(EYESCAN_CTRL_TEST_CFG_INTS, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REG9, 1, 0)) \
    X(EYESCAN_CTRL_LANE0_BERT_CNT0, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REGA, 1, 0)) \
    X(EYESCAN_CTRL_LANE0_BERT_CNT1, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REGB, 1, 0)) \
    X(EYESCAN_CTRL_LANE1_BERT_CNT0, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REGC, 1, 0)) \
    X(EYESCAN_CTRL_LANE1_BERT_CNT1, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REGD, 1, 0)) \
    X(EYESCAN_CTRL_LANE2_BERT_CNT0, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REGE, 1, 0)) \
    X(EYESCAN_CTRL_LANE2_BERT_CNT1, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REGF, 1, 0)) \
    X(EYESCAN_CTRL_LANE3_BERT_CNT0, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REG10, 1, 0)) \
    X(EYESCAN_CTRL_LANE3_BERT_CNT1, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REG11, 1, 0)) \
    X(EYESCAN_CTRL_AUX_TXSR_SWING, INIT_STRUCT(I2CD_DP159_EYESCAN_CTRL_REG12, 1, 0)) \
    X(PLL_CTRLSTAT_CFG, INIT_STRUCT(I2CD_DP159_PLL_CTRL_STAT_REG0, 1, 1)) \
    X(PLL_CTRLSTAT_CP_CFG0, INIT_STRUCT(I2CD_DP159_PLL_CTRL_STAT_REG1, 1, 1)) \
    X(PLL_CTRLSTAT_CP_CFG1, INIT_STRUCT(I2CD_DP159_PLL_CTRL_STAT_REG2, 1, 1)) \
    X(PLL_CTRLSTAT_FBDIV0, INIT_STRUCT(I2CD_DP159_PLL_CTRL_STAT_REG3, 1, 1)) \
    X(PLL_CTRLSTAT_FBDIV1, INIT_STRUCT(I2CD_DP159_PLL_CTRL_STAT_REG4, 1, 1)) \
    X(PLL_CTRLSTAT_FB_PREDIV, INIT_STRUCT(I2CD_DP159_PLL_CTRL_STAT_REG5, 1, 1)) \
    X(PLL_CTRLSTAT_SEL_CFG, INIT_STRUCT(I2CD_DP159_PLL_CTRL_STAT_REG6, 1, 1)) \
    X(PLL_CTRLSTAT_CLK_LN_SEL, INIT_STRUCT(I2CD_DP159_PLL_CTRL_STAT_REG7, 1, 1)) \
    X(PLL_CTRLSTAT_CDR_CFG, INIT_STRUCT(I2CD_DP159_PLL_CTRL_STAT_REG8, 1, 1)) \
    X(PLL_CTRLSTAT_PHY_TRIM_OVR, INIT_STRUCT(I2CD_DP159_PLL_CTRL_STAT_REG9, 1, 1)) \
    X(PLL_CTRLSTAT_P1_REG01_OVR, INIT_STRUCT(I2CD_DP159_PLL_CTRL_STAT_REGA, 1, 1)) \
    X(TX_CTRL_LN_EN, INIT_STRUCT(I2CD_DP159_TX_CTRL_REG0, 1, 1)) \
    X(TX_CTRL_RATE_INV_POL, INIT_STRUCT(I2CD_DP159_TX_CTRL_REG1, 1, 1)) \
    X(TX_CTRL_SLW_CTRL, INIT_STRUCT(I2CD_DP159_TX_CTRL_REG2, 1, 1)) \
    X(TX_CTRL_LD_FIR_UPD, INIT_STRUCT(I2CD_DP159_TX_CTRL_REG3, 1, 1)) \
    X(TX_CTRL_HDMI_TWPST1_DE_EMPH, INIT_STRUCT(I2CD_DP159_TX_CTRL_REG4, 1, 1)) \
    X(RX_CTRL_LN_EN, INIT_STRUCT(I2CD_DP159_RX_CTRL_REG0, 1, 1)) \
    X(RX_CTRL_RATE_INV_POL, INIT_STRUCT(I2CD_DP159_RX_CTRL_REG1, 1, 1)) \
    X(RX_CTRL_LD_PD_INTERPOLATOR, INIT_STRUCT(I2CD_DP159_RX_CTRL_REG2, 1, 1)) \
    X(RX_CTRL_EQ_LD, INIT_STRUCT(I2CD_DP159_RX_CTRL_REG3, 1, 1)) \
    X(RX_CTRL_ENOC, INIT_STRUCT(I2CD_DP159_RX_CTRL_REG4, 1, 1)) \
    X(RX_CTRL_CDR_STAT, INIT_STRUCT(I2CD_DP159_RX_CTRL_REG5, 1, 1)) \
    X(RX_EQ_CTRL_STAT, INIT_STRUCT(I2CD_DP159_RX_EQ_CTRL_STAT_REG0, 1, 1)) \
    X(RX_EQ_CTRL_STAT_FTC_LEV, INIT_STRUCT(I2CD_DP159_RX_EQ_CTRL_STAT_REG1, 1, 1)) \
    X(RX_EQ_CTRL_STAT_LEV_MON, INIT_STRUCT(I2CD_DP159_RX_EQ_CTRL_STAT_REG2, 1, 1))

// Data Types #####################################################################################

enum Dp159Regs
{
#define X(reg, unused) reg,
REG_LIST
#undef X
};

enum dp159MiscCtrlDevFuncMode
{
DP159_DEV_FUNC_MODE_REDRIVER_FULL_RANGE,
DP159_DEV_FUNC_MODE_AUTO_REDRIVER_RETIMER,
DP159_DEV_FUNC_MODE_AUTO_RETIMER_HDMI2_0,
DP159_DEV_FUNC_MODE_RETIMER_FULL_RANGE
};

enum dp159HdmiCtrlTxTermCtrl
{
DP159_HDMI_CTRL_TX_TERM_CTRL_NO_TERM,
DP159_HDMI_CTRL_TX_TERM_CTRL_150_TO_300_OHMS,
DP159_HDMI_CTRL_TX_TERM_CTRL_RSRVD,
DP159_HDMI_CTRL_TX_TERM_CTRL_75_TO_150_OHMS
};

enum dp159HdmiCtrlVswingData
{
DP159_HDMI_CTRL_VSWING_DATA_VSADJ,
DP159_HDMI_CTRL_VSWING_DATA_INC_7_PRCNT,
DP159_HDMI_CTRL_VSWING_DATA_INC_14_PRCNT,
DP159_HDMI_CTRL_VSWING_DATA_INC_21_PRCNT,
DP159_HDMI_CTRL_VSWING_DATA_DEC_30_PRCNT,
DP159_HDMI_CTRL_VSWING_DATA_DEC_21_PRCNT,
DP159_HDMI_CTRL_VSWING_DATA_DEC_14_PRCNT,
DP159_HDMI_CTRL_VSWING_DATA_DEC_7_PRCNT
};

enum dp159HdmiCtrlVswingClk
{
DP159_HDMI_CTRL_VSWING_CLK_VSADJ,
DP159_HDMI_CTRL_VSWING_CLK_INC_7_PRCNT,
DP159_HDMI_CTRL_VSWING_CLK_INC_14_PRCNT,
DP159_HDMI_CTRL_VSWING_CLK_INC_21_PRCNT,
DP159_HDMI_CTRL_VSWING_CLK_DEC_30_PRCNT,
DP159_HDMI_CTRL_VSWING_CLK_DEC_21_PRCNT,
DP159_HDMI_CTRL_VSWING_CLK_DEC_14_PRCNT,
DP159_HDMI_CTRL_VSWING_CLK_DEC_7_PRCNT
};

enum dp159EqCtrlDataLaneEqHdmi14b
{
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_1_4B_0_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_1_4B_4_5_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_1_4B_6_5_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_1_4B_8_5_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_1_4B_10_5_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_1_4B_12_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_1_4B_14_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_1_4B_16_5_DB
};

enum dp159EqCtrlDataLaneEqHdmi20
{
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_2_0_0_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_2_0_4_5_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_2_0_6_5_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_2_0_8_5_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_2_0_10_5_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_2_0_12_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_2_0_14_DB,
DP159_EQ_CTRL_DATA_LANE_EQ_HDMI_2_0_16_5_DB
};

enum dp159EqCtrlClockLaneEqHdmi14b
{
DP159_EQ_CTRL_CLOCK_LANE_EQ_HDMI_1_4B_0_DB,
DP159_EQ_CTRL_CLOCK_LANE_EQ_HDMI_1_4B_1_5_DB,
DP159_EQ_CTRL_CLOCK_LANE_EQ_HDMI_1_4B_3_DB,
DP159_EQ_CTRL_CLOCK_LANE_EQ_HDMI_RSRVD
};

enum dp159EqCtrlClockLaneEqHdmi20
{
DP159_EQ_CTRL_CLOCK_LANE_EQ_HDMI_2_0_0_DB,
DP159_EQ_CTRL_CLOCK_LANE_EQ_HDMI_2_0_1_5_DB,
DP159_EQ_CTRL_CLOCK_LANE_EQ_HDMI_2_0_3_DB,
DP159_EQ_CTRL_CLOCK_LANE_EQ_HDMI_2_0_4_5_DB
};

// PRBS pattern length
enum dp159EyeScanCtrlRegPvLen
{
DP159_EYE_SCAN_CTRL_REG_PV_LEN_PRBS7,
DP159_EYE_SCAN_CTRL_REG_PV_LEN_PRBS11,
DP159_EYE_SCAN_CTRL_REG_PV_LEN_PRBS23,
DP159_EYE_SCAN_CTRL_REG_PV_LEN_PRBS31,
DP159_EYE_SCAN_CTRL_REG_PV_LEN_PRBS15,
DP159_EYE_SCAN_CTRL_REG_PV_LEN_PRBS15_5,
DP159_EYE_SCAN_CTRL_REG_PV_LEN_PRBS20,
DP159_EYE_SCAN_CTRL_REG_PV_LEN_PRBS20_7
};

enum dp159EyeScanCtrlRegPvSel
{
DP159_EYE_SCAN_CTRL_REG_PV_SEL_DISABLED,
DP159_EYE_SCAN_CTRL_REG_PV_SEL_PRBS,
DP159_EYE_SCAN_CTRL_REG_PV_SEL_CLOCK,
DP159_EYE_SCAN_CTRL_REG_PV_SEL_CUSTOM
};

enum dp159Lanes
{
DP159_LANE0 = 0x1,
DP159_LANE1 = 0x2,
DP159_LANE2 = 0x4,
DP159_LANE3 = 0x8
};

enum dp159EyeScanCtrlRegDpTstSel
{
DP159_EYE_SCAN_CTRL_REG_DP_TST_SEL_DATA_ERR,
DP159_EYE_SCAN_CTRL_REG_DP_TST_SEL_FIFO_ERR,
DP159_EYE_SCAN_CTRL_REG_DP_TST_SEL_FIFO_OVRFLW_ERR,
DP159_EYE_SCAN_CTRL_REG_DP_TST_SEL_FIFO_UNDRFLW_ERR,
DP159_EYE_SCAN_CTRL_REG_DP_TST_SEL_TMDS_DESKW_STATUS
};

enum dp159PllCtrlStatRegSelLpfres
{
DP159_PLL_CTRL_STAT_REG_SEL_LPFRES_1K,
DP159_PLL_CTRL_STAT_REG_SEL_LPFRES_2K,
DP159_PLL_CTRL_STAT_REG_SEL_LPFRES_4K,
DP159_PLL_CTRL_STAT_REG_SEL_LPFRES_8K,
DP159_PLL_CTRL_STAT_REG_SEL_LPFRES_500,
DP159_PLL_CTRL_STAT_REG_SEL_LPFRES_16K = 0x7
};

enum dp159PllCtrlStatRegCpEn
{
DP159_PLL_CTRL_STAT_REG_CP_EN_DISABLED,
DP159_PLL_CTRL_STAT_REG_CP_EN_PLL_MODE,
DP159_PLL_CTRL_STAT_REG_CP_EN_PD_MODE = 0x2,
DP159_PLL_CTRL_STAT_REG_CP_EN_FD_MODE = 0x4
};

enum dp159TxCtrlRegTxRate
{
DP159_TX_CTRL_TX_RATE_FULL,
DP159_TX_CTRL_TX_RATE_HALF,
DP159_TX_CTRL_TX_RATE_QUARTER,
DP159_TX_CTRL_TX_RATE_EIGHTH
};

enum dp159TxCtrlRegTxTerm
{
DP159_TX_CTRL_TX_TERM_NONE,
DP159_TX_CTRL_TX_TERM_150_TO_300_OHMS,
DP159_TX_CTRL_TX_TERM_RSRVD,
DP159_TX_CTRL_TX_TERM_75_TO_150_OHMS
};

enum dp159TxCtrlRegHdmiTwpst1DeEmph
{
DP159_TX_CTRL_HDMI_TWPST1_DE_EMPH_NONE,
DP159_TX_CTRL_HDMI_TWPST1_DE_EMPH_2_DB,
DP159_TX_CTRL_HDMI_TWPST1_DE_EMPH_RSRVD,
DP159_TX_CTRL_HDMI_TWPST1_DE_EMPH_5_DB
};

enum dp159RxEqZeroFreqCtrl
{
DP159_RX_EQ_CTRL_ZERO_FREQ_HBR2, /* 805 MHz */
DP159_RX_EQ_CTRL_ZERO_FREQ_HBR,  /* 402 MHz */
DP159_RX_EQ_CTRL_ZERO_FREQ_RBR,  /* 216 MHz */
DP159_RX_EQ_CTRL_ZERO_FREQ_135MHZ
};

/*
 * Page 0 Structs
 */
union I2cdDp159MiscCtrl1
{
    uint8_t raw;
    struct
    {
        // REG 0x09
        // SWAP_EN - enable swapping input main link lanes
        // LANE_POLARITY - swap input data & clock lane polarity
        // X-Mode - 0 normal operation
        //          1 X-mode enabled
        // SIG_EN - not documented, 1b for X-mode
        // PD_EN - forced power down by I2C
        // HPD_AUTO_PWRDWN_DISABLE - enter pwr dn mode based on HPD_SNK
        // I2C_DR_CTL - I2C data rates
        //              00 - 5 kbps
        //              01 - 10 kbps
        //              10 - 100 kbps
        //              11 - 400 kbps
        uint8_t swap_en:1; // 7
        uint8_t lane_polarity:1;
        uint8_t x_mode:1; // 1b for X-mode
        uint8_t sig_en:1; // 1b for X-mode
        uint8_t pd_en:1;
        uint8_t hpd_auto_pwrdn_disable:1;
        uint8_t i2c_dr_ctrl:2; // 1:0
    } bf;
};

union I2cdDp159MiscCtrl2
{
    uint8_t raw;
    struct
    {
        // REG 0x0A
        // APPLICATION_MODE_SELECTION - source - adaptive EQ mid b/w 6.5 - 7.5 dB
        // HPDSNK_GATE_EN - set functional relationship b/w HPD_SNK and HPD_SRC
        //                  0 - HPD_SNK passed through to HPD_SRC
        //                  1 - HPD_SNK not pass through to HPD_SRC
        // EQ_ADA_EN - enables equalizer working state, 0 - fixed EQ, 1 -adaptive EQ
        // EQ_EN - enables receiver EQ
        // AUX_BRG_EN - enable AUX bridge, valid for 48-pin only
        // APPLY_RXTX_CHANGES - self clearing, write 1 to apply new slew, tx_term, twpst1,
        //                      eqen, eqadapten, swing, eqftc, eqlev settings to clock and
        //                      data lanes
        // DEV_FUNC_MODE - device working mode function
        //                 00 - redriver mode across full range 250 Mbps to 6 Gbps
        //                 01 - automatic redriver/retimer crossover at 1.0 Gbps
        //                 10 - automatic retimer for HDMI2.0
        //                 11 - retimer mode accross full range 250 Mbps to 6 Gbps
        //                 device needs to toggle pwr setting 1->0->1 when
        //                 moving between modes, for proper initializing
        uint8_t application_mode_sel:1; // 7
        uint8_t hpdsnk_gate_en:1;
        uint8_t eq_ada_en:1;
        uint8_t eq_en:1;
        uint8_t aux_brg_en:1;
        uint8_t apply_rxtx_changes:1;
        uint8_t dev_func_mode:2; // 1:0
    } bf;
};

union I2cdDp159HdmiCtrl1
{
    uint8_t raw;
    struct
    {
        // REG 0x0B
         // SLEW_CTL - slew rate control, 00 - fastest, 11 - slowest
         // HDMI_SEL - 0 - HDMI, 1 - DVI
         // TX_TERM_CTL - HDMI TX termination
         //               00 - no termination
         //               01 - 150 to 300 Ohms
         //               10 - reserved
         //               11 - 75 to 150 Ohms
         // TMDS_CLOCK_RATIO_STATUS - updated from I2C SRC interface interaction
         //                           0 - TMDS clock is 1/10 of TMDS bit pd
         //                           1 - TMDS clock is 1/40 of TMDS bit pd
         // DDC_TRAIN_SET - DDC training block function status
         //                 0 - DDC training enable
         //                 1 - DDC training disable
        uint8_t slew_ctrl:2; // 7:6
        uint8_t hdmi_sel:1;
        uint8_t tx_term_ctrl:2;
        uint8_t rsrvd2:1;
        uint8_t tmds_clock_ratio_status:1;
        uint8_t ddc_train_set:1; // 0
   } bf;
};

union I2cdDp159HdmiCtrl2
{
    uint8_t raw;
    struct
    {
        // REG 0x0C
        // VSWING_DATA - data output swing control
        //               000 - Vsadj set
        //               001 - +7%
        //               010 - +14%
        //               011 - +21%
        //               100 - -30%
        //               101 - -21%
        //               110 - -14%
        //               111 - -7%
        // VSWING_CLK - clock output swing control (same as data)
        // HDMI_TWPST1 - HDMI de-emphasis FIR post-cursor-1 signed tap weight
        //               00 - no de-emphasis
        //               01 - 2 dB de-emphasis
        //               10/11 - reserved
        uint8_t vswing_data:3; // 7:5
        uint8_t vswing_clk:3;
        uint8_t hdmi_twpst1:2; // 1:0
   } bf;
};

union I2cdDp159EqCtrl
{
    uint8_t raw;
    struct
    {
        // REG 0x0D
        // AUX_CFG - 0 AUX normal operation
        //           1 AUX lanes function as clock output
        // TX_CLK_TST - 0 clock on AUX lane is 1/20 of datarate
        //              1 clock on AUX lane is 1/40 of datarate
        // DATA_LANE_EQ - sets fixed eq values
        //                HDMI1.4b          HDMI2.0
        //                000 - 0 dB        000 - 0 dB
        //                001 - 4.5 dB      001 - 3 dB
        //                010 - 6.5 dB      010 - 5 dB
        //                011 - 8.5 dB      011 - 7.5 dB
        //                100 - 10.5 dB     100 - 9.5 dB
        //                101 - 12 dB       101 - 11 dB
        //                110 - 14 dB       110 - 13 dB
        //                111 - 16.5 dB     111 - 14.5 dB
        // CLOCK_LANE_EQ - sets fixed eq values
        //                 HDMI1.4b         HDMI2.0
        //                 00 - 0 dB        00 - 0 dB
        //                 01 - 1.5 dB      01 - 1.5 dB
        //                 10 - 3 dB        10 - 3 dB
        //                 11 - rsrvd       11 - 4.5 dB
        // CLOCK_VOD - 0 - clock VOD is half set value when TMDS_CLOCK_RATIO_STATUS states
        //                 HDMI2.0 enable
        //             1 - disables TMDS_CLOCK_RATIO_STATUS control of clock VOD so output
        //                 is full swing
        uint8_t aux_cfg:1; // 7
        uint8_t tx_clk_tst:1;
        uint8_t data_lane_eq_val:3;
        uint8_t clock_lane_eq_val:2;
        uint8_t clock_vod_half:1; // 0
    } bf;
};

union I2cdDp159EyeScanCtrl
{
    uint8_t raw[2];
    struct
    {
        // REG 0x0E
        // PV_SYNC - pattern timing pulse, 1 bit per lane, read only
        // PV_LD - load pattern verifier controls into RX lanes, when '1' PV_TO, PV_SEL, PV_LEN
        //         PV_CP20, PV_CP are enabled and latched, 1 bit per lane
        uint16_t pv_sync:4; // 7:4
        uint16_t pv_cp:4; // 3:0
        // REG 0x0F
        // PV_FAIL - pattern verification mismatch detected, 1 bit per lane, read only
        // PV_TIP - pattern search/training in progress, 1 bit per lane, read only
        uint16_t pv_fail:4; // 7:4
        uint16_t pv_tip:4; // 3:0
    } bf;
};

union I2cdDp159EyeScanCtrlCustPattCfg
{
    uint8_t raw;
    struct
    {
        // REG 0x10
        // PV_CP20 - custom pattern length 0 - 16 bits, 1 - 20 bits
        // PV_LEN - PRBS pattern length
        //          000 - PRBS7
        //          001 - PRBS11
        //          010 - PRBS23
        //          011 - PRBS31
        //          100 - PRBS15
        //          101 - PRBS15
        //          110 - PRBS20
        //          111 - PRBS20
        // PV_SEL - pattern select control
        //          000 - disabled
        //          001 - PRBS
        //          010 - clock
        //          011 - custom
        //          1xx - timing only w/ sync pulse spacing defined by PV_LEN
        uint8_t pv_cp20:1; // 7
        uint8_t rsrvd6:1;
        uint8_t pv_len:3;
        uint8_t pv_sel:3; // 2:0
     } bf;
};

// NOTE: when writing you must byte-wise reorder this to ensure registers map up correctly with
// data
union I2cdDp159EyeScanCtrlCustPattData
{
    uint8_t raw[3];
    struct
    {
       // REG 0x13/0x12/0x11
       // PV_CP - custom pattern data
        uint32_t rsrvd7_4:4; // 7:4
        uint32_t pv_cp:20; // 19:0
        uint32_t :8;
     } bf;
};

union I2cdDp159EyeScanCtrlPattVerThr
{
    uint8_t raw;
    struct
    {
        // REG 0x14
        // PV_THR - pattern verifier retain threshold
        uint8_t rsrvd7_3:5; // 7:3
        uint8_t pv_thr:3; // 2:0
    } bf;
};

union I2cdDp159EyeScanCtrlTestCfg
{
    uint8_t raw;
    struct
    {
       // REG 0x15
        // DESKEW_CMPLT - indicates TMDS lane skew completed
        // BERT_CLR - clear BERT counter
        // TST_INTQ_CLR - clear latched interrupt flag
        // TST_SEL - test interrupt source select
        uint8_t deskew_cmplt:1; // 7
        uint8_t rsrvd6_5:2;
        uint8_t bert_clr:1;
        uint8_t tst_intq_clr:1;
        uint8_t tst_sel:3; // 2:0
    } bf;
};

union I2cdDp159EyeScanCtrlTestCfgPvDpEnTstSel
{
    uint8_t raw;
    struct
    {
        // REG 0x16
        // PV_DP_EN - enable datapath verified based on DP_TST_SEL, 1 bit per lane
        // DP_TST_SEL - selects pattern reported by BERT_CNT, TST_INT[0], TST_INTQ[0],
        //              PV_DEP_EN is non-zero
        //              000 - TMDS - disparity or data errors
        //              001 - FIFO errors
        //              010 - FIFO overflow errors
        //              011 - FIFO underflow errors
        //              100 - TMDS deskew status
        //              101 - 111 - reserved
        uint8_t pv_dp_en:4; // 7:4
        uint8_t rsrvd3:1;
        uint8_t dp_tst_sel:3; // 2:0
    } bf;
};

union I2cdDp159EyeScanCtrlTestCfgInts
{
    uint8_t raw;
    struct
    {
        // REG 0x17
        // TST_INTQ - latched interrupt flag, 1 bit per lane
        // TST_INT - test interrupt flag, 1 bit per lane
        uint8_t tst_intq:4; // 7:4
        uint8_t tst_int:4; // 3:0
    } bf;
};

union I2cdDp159EyeScanCtrlLane0BertCnt
{
    uint8_t raw[2];
    struct
    {
        // BERT_CNT - BERT error count
        // REG 0x19/18
        uint16_t rsrvd7_4_lane0:4;
        uint16_t bert_cnt_lane0:12;
    } bf;
};

union I2cdDp159EyeScanCtrlLane1BertCnt
{
    uint8_t raw[2];
    struct
    {
        // BERT_CNT - BERT error count
        // REG 0x1B/1A
        uint16_t rsrvd7_4_lane1:4;
        uint16_t bert_cnt_lane1:12;
    } bf;
};

union I2cdDp159EyeScanCtrlLane2BertCnt
{
    uint8_t raw[2];
    struct
    {
        // BERT_CNT - BERT error count
        // REG 0x1D/1C
        uint16_t rsrvd7_4_lane2:4;
        uint16_t bert_cnt_lane2:12;
    } bf;
};

union I2cdDp159EyeScanCtrlLane3BertCnt
{
    uint8_t raw[2];
    struct
    {
        // BERT_CNT - BERT error count
        // REG 0x1F/1E
        uint16_t rsrvd7_4_lane3:4;
        uint16_t bert_cnt_lane3:12;
    } bf;
};

union I2cdDp159EyeScanCtrlAuxTxSrSwing
{
    uint8_t raw;
    struct
    {
        // REG 0x20
        // AUX_TX_SR - slew rate control for AUX output
        // AUX_SWING - swing control for AUX output
        //             000 - 270 mV
        //             001 - 355 mV
        //             010 - 450 mV
        //             011 - 535 mV
        //             100 - 625 mV
        //             101 - 710 mV
        //             110 - 800 mV
        //             111 - not allowed
        uint8_t rsrvd7_4:4; // 7:4
        uint8_t aux_tx_sr:1;
        uint8_t aux_swing:3; // 2:0
    } bf;
};

/*
 * Page 1 Structs
 */
union I2cdDp159PllCtrlStatCfg
{
    uint8_t raw;
    struct
    {
        // REG 0x00
        // PLL_CLOCK - PLL analog lock indicator, read only
        // LOCK_COMPLETE - digital lock detect output status, read only
        // A_LOCK_OVR - override analog PLL_LOCK, force true
        // D_LOCK_OVR - override digital lock detect, force true
        // EXP_LPFRES - expands range of PLL loop filter when set
        // EN_BANDGAP - enable bandgap, must be set before enabling PLL
        // EN_PLL - enable PLL
        uint8_t pll_clock:1; // 7
        uint8_t lock_complete:1;
        uint8_t a_lock_ovr:1;
        uint8_t d_lock_ovr:1;
        uint8_t rsrvd3:1;
        uint8_t exp_lpfres:1;
        uint8_t en_bandgap:1;
        uint8_t en_pll:1;
    } bf;
};

union I2cdDp159PllCtrlStatCpCfg1
{
    uint8_t raw[1];
    struct
    {
        // REG 0x01
        // CP_EN - charge pump enable and mode
        //         000 - disabled
        //         001 - PLL mode
        //         010 - PD mode
        //         100 - FD mode
        uint16_t rsrvd7_3:5; // 7:3
        uint16_t cp_en:3; // 2:0
    } bf;
};

union I2cdDp159PllCtrlStatCpCfg2
{
    uint8_t raw[1];
    struct
    {
        // REG 0x02
        // CP_CURRENT - charge pump current control, the greater the value the faster the PLL
        //              lock time and higher BW, but also increase PLL phase jitter
        uint16_t rsrvd7:1; // 7
        uint16_t cp_current:7; // 6:0
    } bf;
};

// NOTE: when writing you must byte-wise reorder this to ensure registers map up correctly with
// data
union I2cdDp159PllCtrlStatPllFbDiv
{
    uint8_t raw[2];
    struct
    {
        // REG 0x05/04
        // PLL_FBDIV - PLL feedback divider ratio
        //             10:3 is integer, 0-255
        //             2:0 is fraction, 1/8 of decimal (0 - 0.875)
        uint16_t rsrvd7_3:5; // 7:3
        uint16_t pll_fbdiv_integer:8;
        uint16_t pll_fbdiv_fraction:3;
    } bf;
};

union I2cdDp159PllCtrlStatPllFbPreDiv
{
    uint8_t raw;
    struct
    {
        // REG 0x08
        // PLL_PREDIV - should always be programmed to 0 in X-mode
        uint8_t pll_prediv;
    } bf;
};

union I2cdDp159PllCtrlStatSelCfg
{
    uint8_t raw;
    struct
    {
        // REG 0x0B
        // SEL_HIFVCO - shift VCO to higher frequency
        // DIS_NRST - disables PLL startup circuit
        // SEL_ICPCONST - select constant charge pump current
        // SEL_ICPPTAT - select PTAT charge pump current
        // SEL_LOWKPD - select low phase detection gain
        // SEL_LOWKVCO - select low VCO gain
        // SEL_LPFRES - contents of field with value of EXP_LPFRES field will set loop filter
        //              resistance
        //              000 - 1k
        //              x01 - 2k
        //              x10 - 4k
        //              011 - 8k
        //              100 - 500
        //              111 - 16k
        uint8_t sel_hifvco:1; // 7
        uint8_t dis_nrst:1;
        uint8_t sel_icpconst:1;
        uint8_t sel_icpptat:1;
        uint8_t sel_lowkpd:1;
        uint8_t sel_lowkvco:1;
        uint8_t sel_lpfres:2;
    } bf;
};

union I2cdDp159PllCtrlStatClkLnSel
{
    uint8_t raw;
    struct
    {
       // REG 0x0D
        // CLOCK_LN_SEL - selects between lane3 or lane0 as clock
        //                0 - lane 3
        //                1 - lane 0 - X-mode
        uint8_t rsrvd7_2:6; // 7:2
        uint8_t clock_ln_sel:1;
        uint8_t rsrvd0:1; // 0
   } bf;
};

union I2cdDp159PllCtrlStatCdrCfg
{
    uint8_t raw;
    struct
    {
        // REG 0x0E
        // CDR_CONFIG[4] - selects divide ratio fixed or programmable
        //                 0 - fixed divide ratio of 16 - X-mode
        //                 1 - divide ratio of PLL_PREDIV
        // CDR_CONFIG[3/2] - reserved
        // CDR_CONFIG[1] - CDR clock source
        //                 0 - lane 3
        //                 1 - lane 0 - X-mode
        // CDR_CONFIG[0] - CDR/PLL input CML stage/mux enable
        //                 0 - disabled
        //                 1 - enabled - X-mode
        uint8_t rsrvd7_5:3; // 7:5
        uint8_t cdr_cfg4:1;
        uint8_t cdr_cfg3rsrvd:1;
        uint8_t cdr_cfg2rsrvd:1;
        uint8_t cdr_cfg1:1;
        uint8_t cdr_cfg0:1; // 0
    } bf;
};

union I2cdDp159PllCtrlStatPhyTrimOvr
{
    uint8_t raw;
    struct
    {
        // REG 0xA1
        // PHY_TRIM_OVERRIDE - 0 - PHY trim values determined by eFuse
        //                     1 - PHY trim values determined by I2C - X-mode
        uint8_t rsrvd7_2a1:6; // 7:2
        uint8_t phy_trim_override:1;
        uint8_t rsrvd0a1:1; // 0
    } bf;
};

union I2cdDp159PllCtrlStatP1Reg1Ovr
{
    uint8_t raw;
    struct
    {
        // REG 0xA4
        // P1_REG01_OVERRIDE - 1 - X-mode
        uint8_t rsrvd7_2:6; // 7:2
        uint8_t p1_reg01_override:1;
        uint8_t rsrvd0:1; // 0
    } bf;
};

union I2cdDp159TxCtrlLnEn
{
    uint8_t raw;
    struct
    {
        // REG 0x10
        // PD_TXA - power down TX analog blocks, 1 bit per lane
        // ENTX - enable transmit, 1 bit per lane
        uint8_t pd_txa:4; // 7:4
        uint8_t entx:4; // 3:0
    } bf;
};

union I2cdDp159TxCtrlRateInvPol
{
    uint8_t raw;
    struct
    {
        // REG 0x11
        // TX_RATE - sub rate select
        //           00 - full rate
        //           01 - half rate
        //           10 - quarter rate
        //           11 - eigth rate
        // TX_TERM - termination control
        //           00 - no termination
        //           01 - 150 to 300 Ohms
        //           10 - reserved
        //           11 - 75 to 150 Ohms - X-mode
        // TX_INVPAIR - invert output data polarity, 1 bit per lane
        uint8_t tx_rate:2; // 7:6
        uint8_t tx_term:2; // 5:4
        uint8_t tx_invpair:4; // 3:0
    } bf;
};

union I2cdDp159TxCtrlSlwCtl
{
    uint8_t raw;
    struct
    {
        // REG 0x12
        // SLEW_CTL[1:0] - slew control
        // SWING[2:0] - TX swing for all lanes, see Page0 Reg 0x0C bits 7:5
        uint8_t rsrvd7_6:2; // 7:6
        uint8_t slew_ctl:2;
        uint8_t rsrvd3:1;
        uint8_t swing:3;
    } bf;
};

union I2cdDp159TxCtrlTxLdFirUpd
{
    uint8_t raw;
    struct
    {
       // REG 0x13
        // TX_LD - load control for TX_RATE, TX_INVPAIR, SLEW_CTL, 1 bit per lane
        // FIR_UPD - load FIR coefficients into TX lane, '1' - HDMI_TWPST1 values enabled,
        //           1 bit per lane
        uint8_t tx_ld:4; // 7:4
        uint8_t fir_upd:4; // 3:0
    } bf;
};

union I2cdDp159TxCtrlHdmiTwpst1DeEmph
{
    uint8_t raw;
    struct
    {
        // REG 0x14
        // HDMI_TWPST1 - 00 - no de-emphasis
        //               01 - 2dB of de-emphasis
        //               10 - rsrvd
        //               11 - 5dB of de-emphasis
        uint8_t rsrvd7_2:6; // 7:2
        uint8_t hdmi_twpst1:2; // 1:0
    } bf;
};

union I2cdDp159RxCtrlLnEn
{
    uint8_t raw;
    struct
    {
        // REG 0x30
        // PD_RXA - power down RX analog blocks, 1 bit per lane
        // ENRX - enable receiver, 1 bit per lane
        uint8_t pd_rxa:4; // 7:4
        uint8_t enrx:4; // 3:0
    } bf;
};

union I2cdDp159RxCtrlRateInvPol
{
    uint8_t raw;
    struct
    {
        // REG 0x31
        // RX_RATE - 00 - full rate
        //           01 - half rate
        //           10 - quarter rate
        //           11 - eigth rate
        // RX_EQ_BYPASS - bypass control for eq in clock channel
        //                0 - eq active - X-mode
        //                1 - eq bypassed
        // RX_CLK_OFFSET - input offset control for clock channel
        //                 0 - no input offset - X-mode
        //                 1 - offset added to clock channel
        uint8_t rx_rate:2; // 7:6
        uint8_t rx_eq_bypass:1;
        uint8_t rx_clk_offset:1;
        uint8_t rx_invpair:4; // 3:0
    } bf;
};

union I2cdDp159RxCtrlLdPdInterpolator
{
    uint8_t raw;
    struct
    {
        // REG 0x32
        // RX_LD - load control for RX_RATE, RX_INVPAIR, ENOC, 1 bit per lane
        // PD_RXINT - power down RX interpolator, 1 bit per lane
        uint8_t rx_ld:4; // 7:4
        uint8_t pd_rxint:4; // 3:0
    } bf;
};

union I2cdDp159RxCtrlEqLd
{
    uint8_t raw;
    struct
    {
        // REG 0x33
        // EQ_LD - load control for EQHOLD, EQADAPTFAST, EQADAPTEN, ENEQ, EQLEV, EQFTC,
        //         1 bit per lane
        uint8_t eq_ld:4; // 7:4
        uint8_t rsrvd3_0:4; // 3:0
    } bf;
};

union I2cdDp159RxCtrlEnoc
{
    uint8_t raw;
    struct
    {
        // REG 0x34
        // ENOC - enable offset correction
        uint8_t rsrvd7_1:7; // 7:1
        uint8_t enoc:1; // 0
    } bf;
};

union I2cdDp159RxCtrlCdrCtrlStat
{
    uint8_t raw;
    struct
    {
        // REG 0x3C
        // CDR_THR[3:0] - CDR voting threshold
        // CDR_STL[2:0] - CDR setting time control, decreasing will increase digital CDR BW but
        //                also make DP159 susceptible to hunting jitter
        uint8_t cdr_thr:4; // 7:4
        uint8_t rsrvd3:1;
        uint8_t cdr_stl:3; // 2:0
    } bf;
};

union I2cdDp159RxEqCtrlStat
{
    uint8_t raw;
    struct
    {
        // REG 0x4C
        // EQHOLD - eq hold adaptive state
        // EQADAPFAST - eq shorten adaptive filter time
        // EQADAPTEN - eq enable adaptive mode
        // ENEQ - enable eq
        uint8_t rsrvd7_5:3; // 7:5
        uint8_t eqhold:1;
        uint8_t rsrvd3:1;
        uint8_t eqadaptfast:1;
        uint8_t eqadapten:1;
        uint8_t eneq:1; // 0
    } bf;
};

union I2cdDp159RxEqCtrlStatFtcLev
{
    uint8_t raw;
    struct
    {
        // REG 0x4D
        // EQFTC - eq zero frequency control
        //         00 - 805 MHz for HBR2
        //         01 - 402 MHz for HBR
        //         10 - 216 MHz for RBR
        //         11 - 135 MHz
        // EQLEV - eq level override control for non-adaptive mode, greater value the higher
        //         the EQ level
        uint8_t rsrvd7_6:2; // 7:6
        uint8_t eqftc:2; // 5:4
        uint8_t eqlev:4; // 3:0
    } bf;
};

union I2cdDp159RxEqCtrlStatLevlMon
{
    uint8_t raw[2];
    struct
    {
        // REG 0x4F/4E
        // EQLEVLMON - 3:0 - lane 0
        //             7:4 - lane 1
        //             11:8 - lane 2
        //             15:12 - lane 3
        uint16_t eqlevmon:16;
    } bf;
};

// Function Declarations ##########################################################################
void I2CD_dp159Init( void)  __attribute__((section(".lexatext")));

void I2CD_setDp159(
    const void* data, uint8_t reg, void (*writeCompleteHandler)(void), uint8_t writeBytes)  __attribute__((section(".lexftext")));
void I2CD_getDp159(
    uint8_t reg, void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount))             __attribute__((section(".lexftext")));
bool I2CD_setDp159Blocking(const void* data, uint8_t reg)                                   __attribute__((section(".lexftext")));
bool I2CD_setDp159BlockingBytes(
    const void* data, uint8_t reg, uint8_t writeCnt)                                        __attribute__((section(".lexftext")));
uint32_t getDP159RegAddr(uint8_t reg);
#endif // I2CD_DP159_H

