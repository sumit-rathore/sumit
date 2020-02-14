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
#ifndef BB_TOP_A7_H
#define BB_TOP_A7_H

// Includes #######################################################################################
#include <ibase.h>
#include <top_gpio.h>
#include <gpio.h>
#include <bb_top.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

enum Usb3TxMargin
{
    // Setting TX_SWING to 1 halves these values
    USB3_TX_MARGIN_800MV_1200MV_NORMAL,
    USB3_TX_MARGIN_800MV_1200MV,
    USB3_TX_MARGIN_700MV_900MV,
    USB3_TX_MARGIN_400MV_600MV,
    USB3_TX_MARGIN_200MV_400MV // bit2 = 1, bit1 = bit0 = don't care
};

enum Usb3TxDeemph
{
    USB3_TX_DEEMPH_NEG_6DB,
    USB3_TX_DEEMPH_NEG_3_5DB,
    USB3_TX_DEEMPH_NONE
};

enum DpGtpSysLaneSel
{
    DP_GTP_GT0_SYS_LANE_SEL,
    DP_GTP_GT1_SYS_LANE_SEL,
    DP_GTP_GT2_SYS_LANE_SEL,
    DP_GTP_GT3_SYS_LANE_SEL
};

enum DpGtpSysLaneSelMask
{
    DP_GTP_GT0_SYS_LANE_SEL_MASK = 1,
    DP_GTP_GT1_SYS_LANE_SEL_MASK = 2,
    DP_GTP_GT2_SYS_LANE_SEL_MASK = 4,
    DP_GTP_GT3_SYS_LANE_SEL_MASK = 8
};

enum DpGtpSysClkSelCfg
{
    DP_GTP_SYS_CLK_SEL_CFG_REF_PLL0_SRC_PLL0,
    DP_GTP_SYS_CLK_SEL_CFG_REF_PLL0_SRC_PLL1,
    DP_GTP_SYS_CLK_SEL_CFG_REF_PLL1_SRC_PLL0,
    DP_GTP_SYS_CLK_SEL_CFG_REF_PLL1_SRC_PLL1
};

enum DpGtpPllClkSel
{
    DP_GTP_PLL_CLK_SEL_PLL0,
    DP_GTP_PLL_CLK_SEL_PLL1
};

enum DpGtpPllCtrlRefClkSel
{
    DP_GTP_PLL_CTRL_REF_CLK_SEL_RSRVD,
    DP_GTP_PLL_CTRL_REF_CLK_SEL_GTREFCLK0,
    DP_GTP_PLL_CTRL_REF_CLK_SEL_GTREFCLK1,
    DP_GTP_PLL_CTRL_REF_CLK_SEL_GTEASTREFCLK0,
    DP_GTP_PLL_CTRL_REF_CLK_SEL_GTEASTREFCLK1,
    DP_GTP_PLL_CTRL_REF_CLK_SEL_GTWESTREFCLK0,
    DP_GTP_PLL_CTRL_REF_CLK_SEL_GTWESTREFCLK1,
    DP_GTP_PLL_CTRL_REF_CLK_SEL_GTGREFCLK0_1
};

enum DpGtpTxRxPdMode
{
    DP_GTP_TX_RX_PD_MODE_0, // P0 - normal
    DP_GTP_TX_RX_PD_MODE_1, // P0 - power save with low recovery latency
    DP_GTP_TX_RX_PD_MODE_2, // P1 - power save with longer recovery latency
    DP_GTP_TX_RX_PD_MODE_3  // P2 - power save with lowest power
};

enum DpGtpTxRxPrbsSel
{
    DP_GTP_TX_RX_PRBS_OFF, // Std op mode
    DP_GTP_TX_RX_PRBS_7,
    DP_GTP_TX_RX_PRBS_15,
    DP_GTP_TX_RX_PRBS_23,
    DP_GTP_TX_RX_PRBS_31
};

// The values of this enum correspond to the values accepted into the
// bb_chip/bb_top/mdio_ctrl.slave_sel register bitfield.  The values must be manually kept in sync
// with the hardware.
enum MdioMasterSlaveSel
{
    MDIO_SLAVE_FMC_HPC,
    MDIO_SLAVE_KC705_MARVELL,
    MDIO_SLAVE_10G_PCS_PMA,
    MDIO_SLAVE_RXAUI_CORE,
    MDIO_MASTER_MOTHERBOARD = 0,
    MDIO_MASTER_RXAUI = 1
};

enum I2cPortSel
{
    I2C_MASTER_CORE = 0,
    I2C_MASTER_MOTHERBOARD = 1
};

enum GcmFrequenceyDetectSelect
{
    GCM_FREQ_DET_SEL_XMII_RX_CLK,
    GCM_FREQ_DET_SEL_GE_CLM_RX_CLK,
};


enum DpFrequenceyDetectSelect
{
    DP_GT_RXUSRCLK2,
    DP_GT_TXUSRCLK2
};

struct DpFreqCalculate
{
    uint16_t max_count : 14;
    enum DpFrequenceyDetectSelect clk_sel;
};

struct DpFreqDetAuto
{
    uint16_t comp_max_count : 14;
    uint16_t comp_min_count : 14;
};

// Function Declarations ##########################################################################
void bb_top_a7_Init(void);
void bb_top_a7_TriStateI2cScl(bool triState);
void bb_top_a7_SetMdioSlave(enum MdioMasterSlaveSel masterSlaveSel);
void bb_top_a7_ForceSetMdioSlave(uint8_t masterSlaveSel);
void bb_top_a7_SetRtlI2cMuxPort(enum I2cPortSel masterSlaveSel);
void bb_top_a7_ApplyEnableDp159(bool enable);
void bb_top_a7_ApplyEnableDp130(bool enable);
void bb_top_a7_ApplyResetDp130(bool reset);

void bb_top_applySfpTransmitterEnable(bool disable)                         __attribute__((section(".atext")));
void bb_top_applySfpTxFastRate(bool fastRate);
void bb_top_applySfpRxFastRate(bool fastRate);
bool bb_top_hasSfpRxLos(void);
bool bb_top_hasSfpTxFault(void);
bool bb_top_isSfpModuleAbsent(void);
bool bb_top_isRxauiClk156Locked(void);

void bb_top_applyUsb3PhyCtrlRst(bool reset);
void bb_top_applyUsb3PhyCtrlStrappingDone(bool enable)                      __attribute__((section(".atext")));
void bb_top_applyUsb3PhyCtrlOutputEnable(bool enable)                       __attribute__((section(".atext")));
void bb_top_applyUsb3PhyCtrlXtalDisable(bool disable)                       __attribute__((section(".atext")));
void bb_top_applyUsb3PhyCtrlSscDisable(bool disable)                        __attribute__((section(".atext")));
void bb_top_applyUsb3PhyCtrlVbusSwCtrl(bool enable);
void bb_top_applyUsb3PhyCtrlVbusPresent(bool active);


void bb_top_setDpGtpRxClkCtrl(enum DpGtpSysClkSelCfg clkCfg);
void bb_top_setDpGtpTxClkCtrl(enum DpGtpSysClkSelCfg clkCfg);
void bb_top_setGtpPllRefClkSel(enum DpGtpPllClkSel pllClkSel, enum DpGtpPllCtrlRefClkSel refClkSel);
void bb_top_applyGtpPllPowerDown(enum DpGtpPllClkSel pllClkSel, bool powerDn);
bool bb_top_isGtpPllRefClkLost(enum DpGtpPllClkSel pllClkSel);
bool bb_top_isGtpPllLock(enum DpGtpPllClkSel pllClkSel);
void bb_top_setDpGtpRxMiscCtrlPd(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPdMode pdMode);
void bb_top_setDpGtpTxMiscCtrlPd(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPdMode pdMode);
void bb_top_setDpGtpRxPrbsCtrlSel(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPrbsSel prbs);
void bb_top_applyDpGtpRxPrbsCtrlPrbsCntReset(enum DpGtpSysLaneSel laneSel, bool reset);
void bb_top_setDpGtpTxPrbsCtrlSel(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPrbsSel prbs);
void bb_top_a7_applyDpRxSoftReset(bool reset);
void bb_top_a7_applyDpTxSoftReset(bool reset);
void bb_top_a7_clearPendingOorIrq(void);
uint16_t bb_top_a7_getCurrentFrqCount(void);
bool bb_top_a7_isRefClkFasterbit(void);

void bb_top_irqAquantiaEnable(bool enable)                      __attribute__((section(".atext")));
void bb_top_irqAquantiaClear(void)                              __attribute__((section(".atext")));
void bb_top_irqUsbVbusDetect(bool enable)                       __attribute__((section(".atext")));
void bb_top_a7_RxauiAlignedEnable(bool enable)                  __attribute__((section(".atext")));

bool bb_top_isIrqUsbVbusDetectRawSet(void);
void bb_top_TriStateMdioMdc(bool triState);

void bb_top_applyResetGmiiPhy(bool reset)                       __attribute__((section(".atext")));
void bb_top_a7_applyXmiiRxReset(bool reset)                     __attribute__((section(".atext")));
void bb_top_a7_applyXmiiTxReset(bool reset)                     __attribute__((section(".atext")));
void bb_top_systemReset(void)                                   __attribute__((section(".atext")));
void bb_top_a7_nonCpuModuleReset(void)                          __attribute__((section(".atext")));
void bb_top_a7_reloadFpga(void);
void bb_top_a7_disableFpgaWatchdog(void);
bool bb_top_a7_isFpgaFallback(void);
void bb_top_a7_triggerFpgaFallback(void);
void bb_top_a7_changeFpgaImage(bool fallback);
void bb_top_a7_writeUserReg(uint32_t val);
uint32_t bb_top_a7_readUserReg(void);
uint32_t bb_top_a7_readStatusReg(void);

void bb_top_a7_initiateGcmFrequencyDetection(enum GcmFrequenceyDetectSelect clock_select)           __attribute__((section(".atext")));
bool bb_top_a7_hasGcmFrequencyDetectionCompleted(bool)                                              __attribute__((section(".atext")));
uint32_t bb_top_a7_getNominalGcmFrequencyDetected(bool)                                             __attribute__((section(".atext")));
void bb_top_a7_freqDetDpOorIrq(bool enable)                                                         __attribute__((section(".atext")));
void bb_top_a7_freqDetAutoEnable(bool enable)                                                       __attribute__((section(".atext")));
void bb_top_a7_xmiiTxClkSel(bool tx_clk)                                                            __attribute__((section(".atext")));
void bb_top_a7_gmiiCtrlSetTristates(bool set)                                                       __attribute__((section(".atext")));
void bb_top_a7_miiCtrlSetTristates(bool set)                                                        __attribute__((section(".atext")));
void bb_top_a7_xmiiGtxClkEn(bool enable)                                                            __attribute__((section(".atext")));
void bb_top_a7_rs232Reset(bool enable)                                                              __attribute__((section(".atext")));
void bb_top_a7_rs232ChipConfigure(bool enable)                                                      __attribute__((section(".atext")));

#endif // BB_TOP_A7_H
