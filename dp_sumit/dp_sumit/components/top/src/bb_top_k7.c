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
// Implementations of functions common to the Lex and Rex CPU communcations.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

#ifdef PLATFORM_K7
// Includes #######################################################################################
#include <options.h>
#include <bb_top.h>
#include <bb_top_k7.h>
#include <bb_top_k7_regs.h>
#include <bb_core.h>
#include "bb_top_log.h"
#include <module_addresses_regs.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static volatile bb_top_s* bb_top_registers;

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Initialize register pointer
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_Init(void)
{
    bb_top_registers = (volatile bb_top_s*) bb_chip_bb_top_s_ADDRESS;
}


//#################################################################################################
// Reset all modules except the LEON2.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_nonCpuModuleReset(void)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.phy_rx_rst = 1;
    bb_top_registers->grm.s.soft_rst_ctrl.bf.phy_gtx_rst = 1;
    bb_top_registers->grm.s.soft_rst_ctrl.bf.ulp_phy_rst = 1;
    bb_top_registers->grm.s.soft_rst_ctrl.bf.ulp_core_rst = 1;
    bb_top_registers->grm.s.soft_rst_ctrl.bf.dp_sink_rst = 1;
    bb_top_registers->grm.s.soft_rst_ctrl.bf.dp_source_rst = 1;
    bb_top_registers->grm.s.soft_rst_ctrl.bf.mca_rx_rst = 1;
    bb_top_registers->grm.s.soft_rst_ctrl.bf.mca_tx_rst = 1;
    bb_top_registers->grm.s.soft_rst_ctrl.bf.l3_rx_rst = 1;
    bb_top_registers->grm.s.soft_rst_ctrl.bf.l3_tx_rst = 1;
    bb_top_registers->grm.s.soft_rst_ctrl.bf.dll_rx_rst = 1;
    bb_top_registers->grm.s.soft_rst_ctrl.bf.dll_tx_rst = 1;
}


//#################################################################################################
// Control the reset signal to the I2C switch chip on the KC705.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetI2cSwitch(bool reset)
{
    // active low reset
    bb_top_registers->kc705.s.rst_ctrl.bf.i2c_mux_rst_n = reset ? 0 : 1;
}


//#################################################################################################
// Select PCS_PMA or RXAUI for link control.
//
// Parameters:
//      link_sel            - 0 for PCS_PMA, 1 for RXAUI
// Return:
// Assumptions:
//#################################################################################################
void bb_top_LinkSel(enum BbTopLinkSel link_sel)
{
    bb_top_registers->link_ctrl.bf.link_sel = link_sel;
}


//#################################################################################################
// Is PCS_PMA or RXAUI for link control.
//
// Parameters:
// Return:
//      link_sel            - 0 for PCS_PMA, 1 for RXAUI
// Assumptions:
//#################################################################################################
bool bb_top_k7_isLinkSelRxaui(void)
{
    return (bb_top_registers->kc705.s.gpio_dip_sw11.bf.link_sel == 1);
}


//#################################################################################################
// Is board rev 1.0 or 1.1.
//
// Parameters:
// Return:
//      link_sel            - 0 for PCS_PMA, 1 for RXAUI
// Assumptions:
//#################################################################################################
bool bb_top_k7_isBoardRevZero(void)
{
    return (bb_top_registers->kc705.s.gpio_dip_sw11.bf.board_rev == 0);
}


//#################################################################################################
// Apply or remove tri-stating of the SCL line on the I2C bus.
//
// Parameters:
//      triState            - True if the I2C SCL line should be tri-stated.
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_TriStateI2cScl(bool triState)
{
    bb_top_registers->i2c_ctrl.bf.scl_tri = triState ? 1 : 0;
}


//#################################################################################################
// Set MDIO slave
//
// Parameters:
//      slave               - The MDIO slave to select.
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_SetMdioSlave(enum MdioMasterSlaveSel masterSlaveSel)
{
    bb_top_registers->mdio_ctrl.bf.slave_sel = masterSlaveSel;
}


//#################################################################################################
// Set MDIO slave without first acquiring its mutex. This is generally unsafe and should only be
// used when the caller is sure that no other tasks are performing MDIO transactions.
//
// Parameters:
//      slave               - The MDIO slave to select.
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_ForceSetMdioSlave(uint8_t masterSlaveSel)
{
    bb_top_registers->mdio_ctrl.bf.slave_sel = masterSlaveSel;
}


//#################################################################################################
// Control the reset signal to the I2C GPIO expander on the board connected to the KC705's FMC_HPC
// connector.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetHpcI2cGpio(bool reset)
{
    bb_top_registers->dev7_fmc_hpc.s.gpio.bf.i2c_rst_n = reset ? 0 : 1;
}


//#################################################################################################
// Control the reset signal to the I2C GPIO expander on the board connected to the KC705's FMC_LPC
// connector.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetLpcI2cGpio(bool reset)
{
    bb_top_registers->usb32_fmc_lpc.s.gpio.bf.fmc_rst = reset ? 1 : 0;
}


//#################################################################################################
// Control the reset signal to the Silicon Labs Si5326 de-jitter chip.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetDejitterChip(bool reset)
{
    bb_top_registers->kc705.s.rst_ctrl.bf.si5326_rst_n = reset ? 0 : 1;
}


//#################################################################################################
// Control the reset signal to the Marvell gigabit ethernet PHY.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_ApplyResetEthernetPhy(bool reset)
{
    bb_top_registers->kc705.s.rst_ctrl.bf.m88e1111_rst_n = reset ? 0 : 1;
}


//#################################################################################################
// Control the reset signal to the ethernet PHY RX.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_ApplyResetPhyRx(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.phy_rx_rst = reset ? 1 : 0;
}


//#################################################################################################
// Control the reset signal to the ethernet PHY GTX.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_ApplyResetPhyGtx(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.phy_gtx_rst = reset ? 1 : 0;
}


//#################################################################################################
// Set I2C Mux port
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_SetRtlI2cMuxPort(enum I2cPortSel masterSlaveSel)
{
    // rtlMux for KC705 0 - TI Switch, 1/2 DP159/DP130
    bb_top_registers->i2c_ctrl.bf.slave_sel = masterSlaveSel;
}


//#################################################################################################
// Get HPC ID
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint8_t bb_top_GetHpcId(void)
{
    uint8_t id = 0xFF;
   id = bb_top_registers->kc705.s.fmc_hpc_id.bf.id ;
   return id;
}


//#################################################################################################
// Get LPC ID
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint8_t bb_top_GetLpcId(void)
{
    uint8_t id = 0xFF;
    id = bb_top_registers->kc705.s.fmc_lpc_id.bf.id;
    return id;
}


//#################################################################################################
// Enable Dp159 chip
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_ApplyEnableDp159(bool enable)
{
    if (enable)
    {
        bb_top_registers->tb_fmch_dp3.s.sn65dp159_control.bf.enable = 1;
    }
    else
    {
        bb_top_registers->tb_fmch_dp3.s.sn65dp159_control.bf.enable = 0;
    }
}


//#################################################################################################
// Enable Dp130 chip
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_ApplyEnableDp130(bool enable)
{
    if (enable)
    {
        bb_top_registers->tb_fmch_dp3.s.sn75dp130_control.bf.enable = 1;
    }
    else
    {
        bb_top_registers->tb_fmch_dp3.s.sn75dp130_control.bf.enable = 0;
    }
}


//#################################################################################################
// Reset Dp130 chip
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_ApplyResetDp130(bool reset)
{
    if (reset)
    {
        bb_top_registers->tb_fmch_dp3.s.sn75dp130_control.bf.rst = 1;
    }
    else
    {
        bb_top_registers->tb_fmch_dp3.s.sn75dp130_control.bf.rst = 0;
    }
}


//#################################################################################################
// MCA Interrupt Enable
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyEnableDejitterInterrupt(bool enable)
{
    if (enable)
    {
        bb_top_registers->irq.s.enable.bf.si5326_int_alm = 1;
    }
    else
    {
        bb_top_registers->irq.s.enable.bf.si5326_int_alm = 0;
    }
}


//#################################################################################################
// Perform DRP Read
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint16_t bb_top_k7_drpRead(uint16_t drpAddr, uint8_t drpEnMask)
{
    // It's only valid to read from one DRP bus at a time
    iassert_TOP_COMPONENT_1(
        drpEnMask == BB_TOP_DRP_DRP_EN_MASK_DP_GT0 ||
        drpEnMask == BB_TOP_DRP_DRP_EN_MASK_DP_GT1 ||
        drpEnMask == BB_TOP_DRP_DRP_EN_MASK_DP_GT2 ||
        drpEnMask == BB_TOP_DRP_DRP_EN_MASK_DP_GT3 ||
        drpEnMask == BB_TOP_DRP_DRP_EN_MASK_DP_GT0_TXOUTCLK_MMCM ||
        drpEnMask == BB_TOP_DRP_DRP_EN_MASK_GT_COMMON_LINK ||
        drpEnMask == BB_TOP_DRP_DRP_EN_MASK_RXAUI_GT0 ||
        drpEnMask == BB_TOP_DRP_DRP_EN_MASK_RXAUI_GT1,
        BB_TOP_DRP_READ_INVALID_MASK,
        drpEnMask);

    // Configure the DRP hardware for read access
    const bb_top_drp_ctrl drpCtrl = { .bf = {
        .drp_addr = drpAddr,
        .drp_we = 0
    }};
    bb_top_registers->drp.s.drp_ctrl.dw = drpCtrl.dw;

    // Wait for all DRP buses to be idle
    while ((bb_top_registers->drp.s.drp_en_mask.dw & bb_top_drp_en_mask_READMASK) != 0);

    // Enable the bus we wish to read from and initiate the read
    bb_top_registers->drp.s.drp_en_mask.dw = drpEnMask;

    // Wait for the read to complete
    while ((bb_top_registers->drp.s.drp_en_mask.dw & bb_top_drp_en_mask_READMASK) != 0);

    // Return the read data
    return bb_top_registers->drp.s.drp_read_data.bf.drp_do;
}

#ifdef PLATFORM_A7_K7
//#################################################################################################
// RX CLK CTRL
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setDpGtpRxClkCtrl(enum DpGtpSysLaneSel laneSel, enum DpGtpSysClkSelCfg clkCfg)
{
    switch(laneSel)
    {
        case DP_GTP_GT0_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_clk_ctrl.bf.gt0_rxsysclksel = clkCfg;
            break;
        case DP_GTP_GT1_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_clk_ctrl.bf.gt1_rxsysclksel = clkCfg;
            break;
        case DP_GTP_GT2_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_clk_ctrl.bf.gt2_rxsysclksel = clkCfg;
            break;
        case DP_GTP_GT3_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_clk_ctrl.bf.gt3_rxsysclksel = clkCfg;
            break;
        default:
            // insert some annoying assert because assertions don't frustrate developers, never
            break;
    }
}


//#################################################################################################
// TX CLK CTRL
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setDpGtpTxClkCtrl(enum DpGtpSysLaneSel laneSel, enum DpGtpSysClkSelCfg clkCfg)
{
    switch(laneSel)
    {
        case DP_GTP_GT0_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_tx.s.tx_clk_ctrl.bf.gt0_txsysclksel = clkCfg;
            break;
        case DP_GTP_GT1_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_tx.s.tx_clk_ctrl.bf.gt1_txsysclksel = clkCfg;
            break;
        case DP_GTP_GT2_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_tx.s.tx_clk_ctrl.bf.gt2_txsysclksel = clkCfg;
            break;
        case DP_GTP_GT3_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_tx.s.tx_clk_ctrl.bf.gt3_txsysclksel = clkCfg;
            break;
        default:
            // insert some annoying assert because assertions don't frustrate developers, never
            break;
    }
}
#endif


//#################################################################################################
// PLL Control set ref clock
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setDpGtxCpllRefClkSel(enum DpGtxSysLaneSel laneSel, enum DpGtxCpllRefSel refClkSel)
{
    switch(laneSel)
    {
        case DP_GTX_GT0_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_common.s.cpll_ctrl.bf.gt0_cpllrefclksel = refClkSel;
            break;
        case DP_GTX_GT1_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_common.s.cpll_ctrl.bf.gt1_cpllrefclksel = refClkSel;
            break;
        case DP_GTX_GT2_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_common.s.cpll_ctrl.bf.gt2_cpllrefclksel = refClkSel;
            break;
        case DP_GTX_GT3_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_common.s.cpll_ctrl.bf.gt3_cpllrefclksel = refClkSel;
            break;
        default:
            // insert some annoying assert because assertions don't frustrate developers, never
            break;
    }
}


//#################################################################################################
// PLL Control power down  pll
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applyGtxCpllPowerDown(enum DpGtxSysLaneSel laneSel, bool powerDn)
{
    switch(laneSel)
    {
        case DP_GTX_GT0_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_common.s.cpll_ctrl.bf.gt0_cpllpd = powerDn ? 1 : 0;
            break;
        case DP_GTX_GT1_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_common.s.cpll_ctrl.bf.gt1_cpllpd = powerDn ? 1 : 0;
            break;
        case DP_GTX_GT2_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_common.s.cpll_ctrl.bf.gt2_cpllpd = powerDn ? 1 : 0;
            break;
        case DP_GTX_GT3_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_common.s.cpll_ctrl.bf.gt3_cpllpd = powerDn ? 1 : 0;
            break;
        default:
            // insert some annoying assert because assertions don't frustrate developers, never
            break;
    }
}


#ifdef PLATFORM_A7_K7
//#################################################################################################
// PLL Status ref clock lost
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_isGtpPllRefClkLost(enum DpGtpPllClkSel pllClkSel)
{
    if (pllClkSel == DP_GTP_PLL_CLK_SEL_PLL0)
    {
        return (bb_top_registers->dp_gtp_common.s.pll_status.bf.pll0refclklost == 1);
    }
    else
    {
        return (bb_top_registers->dp_gtp_common.s.pll_status.bf.pll1refclklost == 1);
    }
}


//#################################################################################################
// PLL Status pll lock
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_isGtpPllLock(enum DpGtpPllClkSel pllClkSel)
{
    if (pllClkSel == DP_GTP_PLL_CLK_SEL_PLL0)
    {
        return (bb_top_registers->dp_gtp_common.s.pll_status.bf.pll0lock == 1);
    }
    else
    {
        return (bb_top_registers->dp_gtp_common.s.pll_status.bf.pll1lock == 1);
    }
}
#endif


//#################################################################################################
// RX MISC CTRL gt enable Low Power Mode
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setDpGtxRxMiscCtrlLpm(enum DpGtxSysLaneSel laneSel, bool enLpm)
{
    switch(laneSel)
    {
        case DP_GTX_GT0_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_rx.s.rx_misc_ctrl.bf.gt0_rxlpmen = enLpm ? 1 : 0;
            break;
        case DP_GTX_GT1_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_rx.s.rx_misc_ctrl.bf.gt1_rxlpmen = enLpm ? 1 : 0;
            break;
        case DP_GTX_GT2_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_rx.s.rx_misc_ctrl.bf.gt2_rxlpmen = enLpm ? 1 : 0;
            break;
        case DP_GTX_GT3_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_rx.s.rx_misc_ctrl.bf.gt3_rxlpmen = enLpm ? 1 : 0;
            break;
        default:
            // insert some annoying assert because assertions don't frustrate developers, never
            break;
    }
}


//#################################################################################################
// RX MISC CTRL gt power down
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setDpGtxRxMiscCtrlPd(enum DpGtxSysLaneSel laneSel, enum DpGtxTxRxPdMode pdMode)
{
    switch(laneSel)
    {
        case DP_GTX_GT0_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_rx.s.rx_misc_ctrl.bf.gt0_rxpd = pdMode;
            break;
        case DP_GTX_GT1_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_rx.s.rx_misc_ctrl.bf.gt1_rxpd = pdMode;
            break;
        case DP_GTX_GT2_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_rx.s.rx_misc_ctrl.bf.gt2_rxpd = pdMode;
            break;
        case DP_GTX_GT3_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_rx.s.rx_misc_ctrl.bf.gt3_rxpd = pdMode;
            break;
        default:
            // insert some annoying assert because assertions don't frustrate developers, never
            break;
    }
}


//#################################################################################################
// TX MISC CTRL gt power down
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setDpGtxTxMiscCtrlPd(enum DpGtxSysLaneSel laneSel, enum DpGtxTxRxPdMode pdMode)
{
    switch(laneSel)
    {
        case DP_GTX_GT0_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_tx.s.tx_misc_ctrl.bf.gt0_txpd = pdMode;
            break;
        case DP_GTX_GT1_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_tx.s.tx_misc_ctrl.bf.gt1_txpd = pdMode;
            break;
        case DP_GTX_GT2_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_tx.s.tx_misc_ctrl.bf.gt2_txpd = pdMode;
            break;
        case DP_GTX_GT3_SYS_LANE_SEL:
            bb_top_registers->dp_gtx_tx.s.tx_misc_ctrl.bf.gt3_txpd = pdMode;
            break;
        default:
            // insert some annoying assert because assertions don't frustrate developers, never
            break;
    }
}


#ifdef PLATFORM_A7_K7
//#################################################################################################
// RX PRBS CTRL prbs sel
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setDpGtpRxPrbsCtrlPrbsSel(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPrbsSel prbs)
{
    switch(laneSel)
    {
        case DP_GTP_GT0_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_prbs_ctrl.bf.gt0_rxprbssel = prbs;
            break;
        case DP_GTP_GT1_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_prbs_ctrl.bf.gt1_rxprbssel = prbs;
            break;
        case DP_GTP_GT2_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_prbs_ctrl.bf.gt2_rxprbssel = prbs;
            break;
        case DP_GTP_GT3_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_prbs_ctrl.bf.gt3_rxprbssel = prbs;
            break;
        default:
            // insert some annoying assert because assertions don't frustrate developers, never
            break;
    }
}


//#################################################################################################
// RX PRBS CTRL prbs cnt reset
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applyDpGtpRxPrbsCtrlPrbsCntReset(enum DpGtpSysLaneSel laneSel, bool reset)
{
    switch(laneSel)
    {
        case DP_GTP_GT0_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_prbs_ctrl.bf.gt0_rxprbscntreset = reset ? 1 : 0;
            break;
        case DP_GTP_GT1_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_prbs_ctrl.bf.gt1_rxprbscntreset = reset ? 1 : 0;
            break;
        case DP_GTP_GT2_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_prbs_ctrl.bf.gt2_rxprbscntreset = reset ? 1 : 0;
            break;
        case DP_GTP_GT3_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_prbs_ctrl.bf.gt3_rxprbscntreset = reset ? 1 : 0;
            break;
        default:
            // insert some annoying assert because assertions don't frustrate developers, never
            break;
    }
}


//#################################################################################################
// TX MISC CTRL gt power down
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setDpGtpTxPrbsCtrlSel(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPrbsSel prbs)
{
    switch(laneSel)
    {
        case DP_GTP_GT0_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_tx.s.tx_prbs_ctrl.bf.gt0_txprbssel = prbs;
            break;
        case DP_GTP_GT1_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_tx.s.tx_prbs_ctrl.bf.gt1_txprbssel = prbs;
            break;
        case DP_GTP_GT2_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_tx.s.tx_prbs_ctrl.bf.gt2_txprbssel = prbs;
            break;
        case DP_GTP_GT3_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_tx.s.tx_prbs_ctrl.bf.gt3_txprbssel = prbs;
            break;
        default:
            // insert some annoying assert because assertions don't frustrate developers, never
            break;
    }
}
#endif


//#################################################################################################
// 10G Ethernet Control - PCS PMA - pma pmd type
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_set10GEthCtrlPcsPmaPmdType(enum PcsPmaPmdType type)
{
    bb_top_registers->ten_gig_eth_pcs_pma.s.control.bf.pma_pmd_type = type;
}


//#################################################################################################
// 10G Ethernet Control - PCS PMA - reset
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_apply10GEthCtrlPcsPmaReset(bool reset)
{
    bb_top_registers->ten_gig_eth_pcs_pma.s.control.bf.reset = reset ? 1 : 0;
}


//#################################################################################################
// 10G Ethernet Control - PCS PMA - GT0 Control - tx polarity
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_invert10GEthCtrlPcsPmaGt0TxPolarity(bool inverted)
{
    bb_top_registers->ten_gig_eth_pcs_pma.s.gt0_control.bf.gt0_txpolarity = inverted ? 1 : 0;
}


//#################################################################################################
// 10G Ethernet Control - PCS PMA - GT0 Control - rx polarity
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_invert10GEthCtrlPcsPmaGt0RxPolarity(bool inverted)
{
    bb_top_registers->ten_gig_eth_pcs_pma.s.gt0_control.bf.gt0_rxpolarity = inverted ? 1 : 0;
}


//#################################################################################################
// DP RX Soft Reset
//
// Parameters:
//          reset - apply or not
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_applyDpRxSoftReset(bool reset)
{
    bb_top_registers->dp_gtx_rx.s.rx_rst_ctrl.bf.soft_reset_rx = reset ? 1 : 0;
}


//#################################################################################################
// DP TX Soft Reset
//
// Parameters:
//          reset - apply or not
// Return:
// Assumptions:
//#################################################################################################
void bb_top_k7_applyDpTxSoftReset(bool reset)
{
    bb_top_registers->dp_gtx_tx.s.tx_rst_ctrl.bf.soft_reset_tx = reset ? 1 : 0;
}

// Static Function Definitions ####################################################################


#endif // PLATORM_K7
