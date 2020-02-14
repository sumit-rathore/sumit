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
#ifndef BB_TOP_K7_H
#define BB_TOP_K7_H

// Includes #######################################################################################
#include <ibase.h>
#include <top_gpio.h>
#include <gpio.h>
#include <bb_top.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

enum BbTopLinkSel
{
    BB_TOP_LINK_SEL_PCS_PMA,
    BB_TOP_LINK_SEL_RXAUI
};

enum PcsPmaPmdType
{
    PCS_PMA_PMD_TYPE_10GBASE_SR,
    PCS_PMA_PMD_TYPE_10GBASE_LR,
    PCS_PMA_PMD_TYPE_10GBASE_ER
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

enum DpGtxSysLaneSel
{
    DP_GTX_GT0_SYS_LANE_SEL,
    DP_GTX_GT1_SYS_LANE_SEL,
    DP_GTX_GT2_SYS_LANE_SEL,
    DP_GTX_GT3_SYS_LANE_SEL
};

enum DpGtxCpllRefSel
{
    DP_GTX_GT_RESERVED,
    DP_GTX_GTREFCLK0,
    DP_GTX_GTREFCLK1,
    DP_GTX_GTNORTHREFCLK0,
    DP_GTX_GTNORTHREFCLK1,
    DP_GTX_GTSOUTHREFCLK0,
    DP_GTX_GTSOUTHREFCLK1,
    DP_GTX_GTGRREFCLK
};

enum DpGtxTxRxPdMode
{
    DP_GTX_TX_RX_PD_MODE_0, // P0 - normal
    DP_GTX_TX_RX_PD_MODE_1, // P0 - power save with low recovery latency
    DP_GTX_TX_RX_PD_MODE_2, // P1 - power save with longer recovery latency
    DP_GTX_TX_RX_PD_MODE_3  // P2 - power save with lowest power
};



// Function Declarations ##########################################################################
void bb_top_k7_Init(void);
void bb_top_ApplyResetI2cSwitch(bool reset);

void bb_top_k7_TriStateI2cScl(bool triState);
void bb_top_k7_SetMdioSlave(enum MdioMasterSlaveSel masterSlaveSel);
void bb_top_k7_ForceSetMdioSlave(uint8_t masterSlaveSel);

void bb_top_ApplyResetHpcI2cGpio(bool reset);
void bb_top_ApplyResetLpcI2cGpio(bool reset);
void bb_top_ApplyResetDejitterChip(bool reset);

void bb_top_k7_ApplyResetPhyRx(bool reset);
void bb_top_k7_ApplyResetPhyGtx(bool reset);

void bb_top_k7_ApplyResetEthernetPhy(bool reset);

void bb_top_k7_SetRtlI2cMuxPort(enum I2cPortSel masterSlaveSel);

uint8_t bb_top_GetHpcId(void);
uint8_t bb_top_GetLpcId(void);

void bb_top_k7_ApplyEnableDp159(bool enable);
void bb_top_k7_ApplyEnableDp130(bool enable);
void bb_top_k7_ApplyResetDp130(bool reset);

void bb_top_ApplyEnableDejitterInterrupt(bool enable);
void bb_top_LinkSel(enum BbTopLinkSel link_sel);
bool bb_top_k7_isLinkSelRxaui(void);
bool bb_top_k7_isBoardRevZero(void);

uint16_t bb_top_k7_drpRead(uint16_t drpAddr, uint8_t drpEnMask);

void bb_top_setDpGtxCpllRefClkSel(enum DpGtxSysLaneSel laneSel, enum DpGtxCpllRefSel refClkSel);

#ifdef PLATFORM_A7_K7
void bb_top_setDpGtpTxClkCtrl(enum DpGtpSysLaneSel laneSel, enum DpGtpSysClkSelCfg clkCfg);
void bb_top_setGtpPllRefClkSel(enum DpGtpPllClkSel pllClkSel, enum DpGtpPllCtrlRefClkSel refClkSel);
#endif

void bb_top_applyGtxCpllPowerDown(enum DpGtxSysLaneSel laneSel, bool powerDn);

#ifdef PLATFORM_A7_K7
bool bb_top_isGtpPllRefClkLost(enum DpGtpPllClkSel pllClkSel);
bool bb_top_isGtpPllLock(enum DpGtpPllClkSel pllClkSel);
#endif

void bb_top_setDpGtxRxMiscCtrlLpm(enum DpGtxSysLaneSel laneSel, bool enLpm);
void bb_top_setDpGtxRxMiscCtrlPd(enum DpGtxSysLaneSel laneSel, enum DpGtxTxRxPdMode pdMode);
void bb_top_setDpGtxTxMiscCtrlPd(enum DpGtxSysLaneSel laneSel, enum DpGtxTxRxPdMode pdMode);

#ifdef PLATFORM_A7_K7
void bb_top_setDpGtpRxPrbsCtrlSel(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPrbsSel prbs);
void bb_top_applyDpGtpRxPrbsCtrlPrbsCntReset(enum DpGtpSysLaneSel laneSel, bool reset);
void bb_top_setDpGtpTxPrbsCtrlSel(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPrbsSel prbs);
#endif
void bb_top_k7_applyDpRxSoftReset(bool reset);
void bb_top_k7_applyDpTxSoftReset(bool reset);

void bb_top_set10GEthCtrlPcsPmaPmdType(enum PcsPmaPmdType type);
void bb_top_apply10GEthCtrlPcsPmaReset(bool reset);
void bb_top_invert10GEthCtrlPcsPmaGt0TxPolarity(bool inverted);
void bb_top_invert10GEthCtrlPcsPmaGt0RxPolarity(bool inverted);
void bb_top_k7_nonCpuModuleReset(void);

#endif // BB_TOP_K7_H
