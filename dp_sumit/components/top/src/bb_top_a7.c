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

#ifdef PLATFORM_A7

// Includes #######################################################################################
#include <options.h>
#include <bb_top.h>
#include <bb_top_a7.h>
#include <bb_top_a7_regs.h>
#include <bb_core.h>
#include "bb_top_log.h"
#include <leon_timers.h>
#include <mca.h>
#include <cpu_comm.h>
#include <led.h>
#include <uart.h>
#include <event.h>
#include <module_addresses_regs.h>
// #include <uart.h>  // for debugging

// Constants and Macros ###########################################################################
#define ICAP_CRC_REG     0
#define ICAP_CMD_REG     (0x04 << 13)
#define ICAP_CTL0_REG    (0x05 << 13)
#define ICAP_CTL1_REG    (0x18 << 13)
#define ICAP_STAT_REG    (0x07 << 13)
#define ICAP_COR0_REG    (0x09 << 13)
#define ICAP_IDCODE_REG  (0x0C << 13)
#define ICAP_AXSS_REG    (0x0D << 13)
#define ICAP_TIMER_REG   (0x11 << 13)
#define ICAP_WBSTAR_REG  (0x10 << 13)
#define ICAP_BOOTSTS_REG (0x16 << 13)
#define JUMP_TIMEOUT     0x401D0EA9

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static volatile bb_top_s* bb_top_registers;

// Static Function Declarations ###################################################################
static void bb_top_a7_writePreamble(void);
static void bb_top_a7_writeIcapRegister(uint32_t reg, uint32_t val);
static uint32_t bb_top_a7_readIcapRegister(uint32_t reg);

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Initialize register pointer
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_Init(void)
{
    bb_top_registers = (volatile bb_top_s*) bb_chip_bb_top_s_ADDRESS;
}


//#################################################################################################
// Apply or remove tri-stating of the SCL line on the I2C bus.
//
// Parameters:
//      triState            - True if the I2C SCL line should be tri-stated.
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_TriStateI2cScl(bool triState)
{
    bb_top_registers->i2c_ctrl.bf.master_scl_tri = triState;
}


//#################################################################################################
// Set MDIO slave
//
// Parameters:
//      slave               - The MDIO slave to select.
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_SetMdioSlave(enum MdioMasterSlaveSel masterSlaveSel)
{
    // enums are different so we'll need to remap
    // TODO Come up with a better approach for single FW
    if ((masterSlaveSel == MDIO_SLAVE_10G_PCS_PMA) || (masterSlaveSel == MDIO_SLAVE_RXAUI_CORE))
    {
        masterSlaveSel = MDIO_MASTER_RXAUI;
    }
    else
    {
        masterSlaveSel = MDIO_MASTER_MOTHERBOARD;
    }
    bb_top_registers->mdio_ctrl.bf.master_bus_sel = masterSlaveSel;
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
void bb_top_a7_ForceSetMdioSlave(uint8_t masterSlaveSel)
{
    if (masterSlaveSel == MDIO_SLAVE_RXAUI_CORE)
    {
        masterSlaveSel = MDIO_MASTER_RXAUI;
    }
    bb_top_registers->mdio_ctrl.bf.master_bus_sel = masterSlaveSel;
}


//#################################################################################################
// Reset/Enable the RS232 module.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_rs232Reset(bool enable)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.rs232_rst = enable;
}


//#################################################################################################
// RS232 Chip Configure.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_rs232ChipConfigure(bool enable)
{
    if (enable)
    {
        bb_top_registers->rs232_ctrl.bf.force_off_b = 1;
        bb_top_registers->rs232_ctrl.bf.en_b = 0;
    }
    else
    {
        bb_top_registers->rs232_ctrl.bf.force_off_b = 0;
        bb_top_registers->rs232_ctrl.bf.en_b = 1;
    }
}


//#################################################################################################
// Reset all modules except the LEON2.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_nonCpuModuleReset(void)
{
    // Clean up all FPGA modules except the main cpu reset
    // and flash(This cause instruction trap error if we add it) reset
    bb_top_registers->grm.s.soft_rst_ctrl.dw = ~( BB_TOP_GRM_SOFT_RST_CTRL_CPU_RST_MASK |
                                                  BB_TOP_GRM_SOFT_RST_CTRL_SPI_FLASH_CTRL_RST_MASK );
}

//#################################################################################################
// Set I2C Mux port
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_SetRtlI2cMuxPort(enum I2cPortSel masterSlaveSel)
{
    bb_top_registers->i2c_ctrl.bf.master_bus_sel = masterSlaveSel;
}


//#################################################################################################
// Enable Dp159 chip
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_ApplyEnableDp159(bool enable)
{
    if (enable)
    {
        GpioSet(GPIO_CONN_DP_REDRV_RETMR_EN_A);
    }
    else
    {
        GpioClear(GPIO_CONN_DP_REDRV_RETMR_EN_A);
    }
}


//#################################################################################################
// Enable Dp130 chip
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_ApplyEnableDp130(bool enable)
{
    if (enable)
    {
        GpioSet(GPIO_CONN_DP_REDRV_RETMR_EN_A);
    }
    else
    {
        GpioClear(GPIO_CONN_DP_REDRV_RETMR_EN_A);
    }
}


//#################################################################################################
// Reset Dp130 chip
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_ApplyResetDp130(bool reset)
{
    if (reset)
    {
        GpioClear(GPIO_CONN_DP_REDRV_RETMR_RST_B_A);
    }
    else
    {
        GpioSet(GPIO_CONN_DP_REDRV_RETMR_RST_B_A);
    }
}

//#################################################################################################
// Enable SFP Transmitter
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applySfpTransmitterEnable(bool enable)
{
    bb_top_registers->sfp_ctrl.bf.tx_disable = !enable;
}


//#################################################################################################
// SFP TX Rate Select - fast TX signaling > 4.25GBd, slow <= 4.25GBd
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applySfpTxFastRate(bool fastRate)
{
    bb_top_registers->sfp_ctrl.bf.rs1 = fastRate;
}


//#################################################################################################
// SFP RX Rate Select - fast TX signaling > 4.25GBd, slow <= 4.25GBd
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applySfpRxFastRate(bool fastRate)
{
    bb_top_registers->sfp_ctrl.bf.rs0 = fastRate;
}


//#################################################################################################
// SFP RX Los Of Signal
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_hasSfpRxLos(void)
{
    return (bb_top_registers->sfp_status.bf.rx_los == 1);
}


//#################################################################################################
// SFP TX Fault
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_hasSfpTxFault(void)
{
    return (bb_top_registers->sfp_status.bf.tx_fault == 1);
}


//#################################################################################################
// SFP Module Absent
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_isSfpModuleAbsent(void)
{
    return (bb_top_registers->sfp_status.bf.mod_abs == 1);
}


//#################################################################################################
// USB3 PHY Control - apply reset
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applyUsb3PhyCtrlRst(bool reset)
{
    // invert because reset is active low
    bb_top_registers->usb3phy_ctrl.bf.rst_b = !reset;
}


//#################################################################################################
// USB3 PHY Control - strap control
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applyUsb3PhyCtrlStrappingDone(bool enable)
{
    bb_top_registers->usb3phy_ctrl.bf.strapping_done = enable;
}


//#################################################################################################
// USB3 PHY Control - enable output
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applyUsb3PhyCtrlOutputEnable(bool enable)
{
    bb_top_registers->usb3phy_ctrl.bf.out_enable = enable;
}


//#################################################################################################
// USB3 PHY Control - Strapping: xtal disable
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applyUsb3PhyCtrlXtalDisable(bool disable)
{
    bb_top_registers->usb3phy_ctrl.bf.xtal_dis = disable;
}


//#################################################################################################
// USB3 PHY Control - Strapping: ssc disable
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applyUsb3PhyCtrlSscDisable(bool disable)
{
    bb_top_registers->usb3phy_ctrl.bf.ssc_dis = disable;
}


//#################################################################################################
// USB3 PHY Control - enable SW control of Vbus to ULP
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applyUsb3PhyCtrlVbusSwCtrl(bool enable)
{
    bb_top_registers->usb3phy_ctrl.bf.power_present_sel = enable;
}


//#################################################################################################
// USB3 PHY Control - sets Vus present true on or off, depending if active is true or false
//
// Parameters:
// Return:
// Assumptions: This only takes effect if power_present_sel is set to SW control (true)
//#################################################################################################
void bb_top_applyUsb3PhyCtrlVbusPresent(bool active)
{
    bb_top_registers->usb3phy_ctrl.bf.power_present = active;
}


//#################################################################################################
// RX CLK CTRL
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setDpGtpRxClkCtrl(enum DpGtpSysClkSelCfg clkCfg)
{

    bb_top_registers->dp_gtp_rx.s.rx_clk_ctrl.bf.gt0_rxsysclksel = clkCfg;
    bb_top_registers->dp_gtp_rx.s.rx_clk_ctrl.bf.gt1_rxsysclksel = clkCfg;
    bb_top_registers->dp_gtp_rx.s.rx_clk_ctrl.bf.gt2_rxsysclksel = clkCfg;
    bb_top_registers->dp_gtp_rx.s.rx_clk_ctrl.bf.gt3_rxsysclksel = clkCfg;

}


//#################################################################################################
// TX CLK CTRL
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setDpGtpTxClkCtrl(enum DpGtpSysClkSelCfg clkCfg)
{
    bb_top_registers->dp_gtp_tx.s.tx_clk_ctrl.bf.gt0_txsysclksel = clkCfg;
    bb_top_registers->dp_gtp_tx.s.tx_clk_ctrl.bf.gt1_txsysclksel = clkCfg;
    bb_top_registers->dp_gtp_tx.s.tx_clk_ctrl.bf.gt2_txsysclksel = clkCfg;
    bb_top_registers->dp_gtp_tx.s.tx_clk_ctrl.bf.gt3_txsysclksel = clkCfg;
}

//#################################################################################################
// PLL Control set ref clock
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setGtpPllRefClkSel(enum DpGtpPllClkSel pllClkSel, enum DpGtpPllCtrlRefClkSel refClkSel)
{
    if (pllClkSel == DP_GTP_PLL_CLK_SEL_PLL0)
    {
        bb_top_registers->dp_gtp_common.s.pll_ctrl.bf.pll0refclksel = refClkSel;
    }
    else
    {
        bb_top_registers->dp_gtp_common.s.pll_ctrl.bf.pll1refclksel = refClkSel;
    }
}


//#################################################################################################
// PLL Control power down  pll
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applyGtpPllPowerDown(enum DpGtpPllClkSel pllClkSel, bool powerDn)
{
    if (pllClkSel == DP_GTP_PLL_CLK_SEL_PLL0)
    {
        bb_top_registers->dp_gtp_common.s.pll_ctrl.bf.pll0pd = powerDn;
    }
    else
    {
        bb_top_registers->dp_gtp_common.s.pll_ctrl.bf.pll1pd = powerDn;
    }
}


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


//#################################################################################################
// RX MISC CTRL gt power down
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setDpGtpRxMiscCtrlPd(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPdMode pdMode)
{
    switch(laneSel)
    {
        case DP_GTP_GT0_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_misc_ctrl.bf.gt0_rxpd = pdMode;
            break;
        case DP_GTP_GT1_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_misc_ctrl.bf.gt1_rxpd = pdMode;
            break;
        case DP_GTP_GT2_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_misc_ctrl.bf.gt2_rxpd = pdMode;
            break;
        case DP_GTP_GT3_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_misc_ctrl.bf.gt3_rxpd = pdMode;
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
void bb_top_setDpGtpTxMiscCtrlPd(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPdMode pdMode)
{
    switch(laneSel)
    {
        case DP_GTP_GT0_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_tx.s.tx_misc_ctrl.bf.gt0_txpd = pdMode;
            break;
        case DP_GTP_GT1_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_tx.s.tx_misc_ctrl.bf.gt1_txpd = pdMode;
            break;
        case DP_GTP_GT2_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_tx.s.tx_misc_ctrl.bf.gt2_txpd = pdMode;
            break;
        case DP_GTP_GT3_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_tx.s.tx_misc_ctrl.bf.gt3_txpd = pdMode;
            break;
        default:
            // insert some annoying assert because assertions don't frustrate developers, never
            break;
    }
}


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
            bb_top_registers->dp_gtp_rx.s.rx_prbs_ctrl.bf.gt0_rxprbscntreset = reset;
            break;
        case DP_GTP_GT1_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_prbs_ctrl.bf.gt1_rxprbscntreset = reset;
            break;
        case DP_GTP_GT2_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_prbs_ctrl.bf.gt2_rxprbscntreset = reset;
            break;
        case DP_GTP_GT3_SYS_LANE_SEL:
            bb_top_registers->dp_gtp_rx.s.rx_prbs_ctrl.bf.gt3_rxprbscntreset = reset;
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


//#################################################################################################
// DP RX Soft Reset
//
// Parameters:
//          reset - apply or not
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_applyDpRxSoftReset(bool reset)
{
    bb_top_registers->dp_gtp_rx.s.rx_rst_ctrl.bf.soft_reset_rx = reset;
}


//#################################################################################################
// DP TX Soft Reset
//
// Parameters:
//          reset - apply or not
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_applyDpTxSoftReset(bool reset)
{
    bb_top_registers->dp_gtp_tx.s.tx_rst_ctrl.bf.soft_reset_tx = reset;
}


//#################################################################################################
// Enable Aquantia Interrupts
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_irqAquantiaEnable(bool enable)
{
    bb_top_registers->irq.s.enable.bf.mdio_int_b = enable;
}


//#################################################################################################
// Clear Aquantia Interrupts
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_irqAquantiaClear(void)
{
    bb_top_registers->irq.s.pending.bf.mdio_int_b = 1;
}


//#################################################################################################
// Enable Lex VBus Interrupt
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_irqUsbVbusDetect(bool enable)
{
    bb_top_registers->irq.s.enable.bf.usb_vbus_detect = enable;
}


//#################################################################################################
// Enable RXAUI aligned status change interrupt (for Phy link up/down)
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_RxauiAlignedEnable(bool enable)
{
    bb_top_registers->irq.s.enable.bf.rxaui_align_status = enable;
}

//#################################################################################################
// Enable Get USB Vbus Detect Raw
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_isIrqUsbVbusDetectRawSet(void)
{
    return (bb_top_registers->irq.s.raw.bf.usb_vbus_detect == 1);
}


//#################################################################################################
// Apply or remove tri-stating of the SCL line on the MDIO bus.
//
// Parameters:
//      triState            - True if the I2C SCL line should be tri-stated.
// Return:
// Assumptions:
//#################################################################################################
void bb_top_TriStateMdioMdc(bool triState)
{
    bb_top_registers->mdio_ctrl.bf.master_mdc_tri = triState;
}


//#################################################################################################
// Apply or remove gmii PHY reset.
//
// Parameters:
//      reset            - true if applying reset
// Return:
// Assumptions:
//          * GMII PHY on motherboard of Artix platform
//#################################################################################################
void bb_top_applyResetGmiiPhy(bool reset)
{
    if (reset)
    {
        bb_top_registers->xmii_ctrl.bf.rst_b = 0;           // put the GMII phy into reset
        bb_top_registers->xmii_ctrl.bf.gtx_clk_en = 0;      // then disable the tx clock
    }
    else
    {
        bb_top_registers->xmii_ctrl.bf.gtx_clk_en = 1;      // enable the tx clock
        bb_top_registers->xmii_ctrl.bf.rst_b = 1;           // bring GMII phy out of reset
    }
}


//#################################################################################################
// Apply reset to GMII rx reset
//
// Parameters:
//      reset            - true if applying reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_applyXmiiRxReset(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.xmii_rx_rst = reset;
}


//#################################################################################################
// XMII Select TX CLK
//
// Parameters:
//      tx_clk      - tx_clk if true, gtx if not
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_xmiiTxClkSel(bool tx_clk)
{
    bb_top_registers->gcm.s.control.bf.xmii_tx_clk_sel = tx_clk;
}


//#################################################################################################
// Enable XMII GTX CLK
//
// Parameters:
//      clock_select     - clock to detect
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_xmiiGtxClkEn(bool enable)
{
    bb_top_registers->xmii_ctrl.bf.gtx_clk_en = enable;
}


//#################################################################################################
// Set/Clear Tristates for XMII Ctrl in GMII mode
//
// Parameters:
//      set         - tristate all if true, clear if false
// Return:
// Assumptions:
// TODO Make these separate if needed
//#################################################################################################
void bb_top_a7_gmiiCtrlSetTristates(bool set)
{
    bb_top_registers->xmii_ctrl.bf.tx_en_tri = set;
    bb_top_registers->xmii_ctrl.bf.tx_er_tri = set;
    bb_top_registers->xmii_ctrl.bf.txd_tri = set ? 0xFF : 0;
}


//#################################################################################################
// Set/Clear Tristates for XMII Ctrl in MII mode
//
// Parameters:
//      set         - tristate all if true, clear if false
// Return:
// Assumptions:
// TODO Make these separate if needed
//#################################################################################################
void bb_top_a7_miiCtrlSetTristates(bool set)
{
    bb_top_registers->xmii_ctrl.bf.tx_en_tri = set;
    bb_top_registers->xmii_ctrl.bf.tx_er_tri = set;
    bb_top_registers->xmii_ctrl.bf.txd_tri = set ? 0xFF : 0xF0; // 7:4 tristated 3:0 not for MII config "non-tristated" mode
}


//#################################################################################################
// Apply reset to GMII tx reset
//
// Parameters:
//      reset            - true if applying reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_applyXmiiTxReset(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.xmii_tx_rst = reset;
}


//#################################################################################################
// Initiate GCM Frequency Detection
//
// Parameters:
//      clock_select     - clock to detect
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_initiateGcmFrequencyDetection(enum GcmFrequenceyDetectSelect clock_select)
{
    bb_top_registers->gcm.s.freq_det.bf.max_count = 0xff;
    bb_top_registers->gcm.s.freq_det.bf.clk_sel = clock_select;
    bb_top_registers->gcm.s.freq_det.bf.go = 1;
}


//#################################################################################################
// Check if GCM Frequency Detection completed
//
// Parameters:
//      clock_select     - dp 1: measure dp clock.
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_a7_hasGcmFrequencyDetectionCompleted(bool dp)
{
    if(dp)
    {
        return (bb_top_registers->gcm.s.freq_det_dp.bf.go == 0);
    }
    else
    {
        return (bb_top_registers->gcm.s.freq_det.bf.go == 0);
    }
}


//#################################################################################################
// Get Nominal GCM Frequency Detected
//
// Parameters:
//      clock_select     - dp 1: measure dp clock.
// Return:
// Assumptions:
//#################################################################################################
uint32_t bb_top_a7_getNominalGcmFrequencyDetected(bool dp)
{
    uint32_t scaler =  1000;
    uint32_t count = 0;
    uint32_t max_count = 0;
    uint32_t nominalFrequency = 0;
    uint32_t frequencyAdjust = 0;
    bool ref_clk_faster;

    if(dp)
    {
        count = bb_top_registers->gcm.s.freq_det_dp.bf.count;
        max_count = bb_top_registers->gcm.s.freq_det_dp.bf.max_count;
        ref_clk_faster = bb_top_registers->gcm.s.freq_det_dp.bf.ref_clk_faster;
    }
    else
    {
        count = bb_top_registers->gcm.s.freq_det.bf.count;
        max_count = bb_top_registers->gcm.s.freq_det.bf.max_count;
        ref_clk_faster = bb_top_registers->gcm.s.freq_det.bf.ref_clk_faster;
    }

    if ((count == 0) || (max_count == 0))
    {
        return nominalFrequency;
    }
    if (ref_clk_faster == 1)
    {
        frequencyAdjust = (((count + 3) * scaler)/max_count);
    }
    else
    {
        frequencyAdjust = (((max_count + 3) * scaler)/count);
    }

    nominalFrequency = (bb_core_getCpuClkFrequency() / scaler) * frequencyAdjust;
    return nominalFrequency;
}

//#################################################################################################
// Enable or disable the Freq detect interrupt
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_freqDetDpOorIrq(bool enable)
{
    bb_top_registers->irq.s.enable.bf.freq_det_dp_oor = enable;
}

//#################################################################################################
// Enable or disable the automatic Freq detect
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_a7_freqDetAutoEnable(bool enable)
{
    bb_top_registers->gcm.s.freq_det_dp_auto.bf.auto_enable = enable;
}

//#################################################################################################
// System Reset
//
// Parameters:
// Return:
// Assumptions:
// * DD says this will automatically clear itself and allow the system to restart
// Also, this resets the clock domains (all) which should reset all DD submodules
//#################################################################################################
void bb_top_systemReset(void)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.cpu_rst = 1;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void bb_top_a7_clearPendingOorIrq(void)
{
    bb_top_registers->irq.s.pending.bf.freq_det_dp_oor = 1;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
uint16_t bb_top_a7_getCurrentFrqCount(void)
{
    return bb_top_registers->gcm.s.freq_det_dp.bf.count;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool bb_top_a7_isRefClkFasterbit(void)
{
    return bb_top_registers->gcm.s.freq_det_dp_auto.bf.comp_ref_clk_faster;
}

//#################################################################################################
// Trigger reload of FPGA in current or golden
//
// Parameters:
// Return:
// Assumptions:
// * At some point we'll figure out how to force-boot into golden FPGA as the default is it will
// check the Current and boot that automatically - it checks Golden header, sees it should try the
// Current image, and boots the current image
//#################################################################################################
void bb_top_a7_reloadFpga()
{
    // Data Source: Table 7.1 from UG470, Example Bitstream For IPROG through ICAP2
    // set it to the golden image on older units it should just jump to the current image again
    // on newer units it will run the Golden and jump to the current if everything checks out
    bb_top_a7_changeFpgaImage(false);
}

//#################################################################################################
// Trigger reload of FPGA in current or golden
//
// Parameters:
// Return:
// Assumptions:
// * At some point we'll figure out how to force-boot into golden FPGA as the default is it will
// check the Current and boot that automatically - it checks Golden header, sees it should try the
// Current image, and boots the current image
//#################################################################################################
void bb_top_a7_changeFpgaImage(bool fallback)
{

    // Data Source: Table 7.1 from UG470, Example Bitstream For IPROG through ICAP2
    bb_top_a7_writePreamble();

    if(fallback)
    {
        // Do we need to enable the fallback mechanism
        // ICAP_CTL0_REG bit 10
        bb_top_a7_writeIcapRegister(ICAP_TIMER_REG, JUMP_TIMEOUT); // set the timeout at which point we should fallback
//        bb_top_a7_writeIcapRegister(ICAP_WBSTAR_REG, 0x00010000); // Fallback Boot Start Address Old FPGA files
        bb_top_a7_writeIcapRegister(ICAP_WBSTAR_REG, 0x00010001); // Fallback Boot Start Address
    }
    else
        bb_top_a7_writeIcapRegister(ICAP_WBSTAR_REG, 0x00000000); // Fallback Boot Start Address

    bb_top_a7_writeIcapRegister(ICAP_CMD_REG, 0x0000000F); // IPROG Command - dhip will progB at this point
}

//#################################################################################################
// Write the timer register
//
// Parameters:
// Return:
// Assumptions:
// * At some point we'll figure out how to force-boot into golden FPGA as the default is it will
// check the Current and boot that automatically - it checks Golden header, sees it should try the
// Current image, and boots the current image
//#################################################################################################
void bb_top_a7_writeTimerReg(uint32_t val)
{

    // Data Source: Table 7.1 from UG470, Example Bitstream For IPROG through ICAP2
    bb_top_a7_writeIcapRegister(ICAP_TIMER_REG, val);
}

//#################################################################################################
// Write the user access register
//
// Parameters:
// Return:
// Assumptions:
// * At some point we'll figure out how to force-boot into golden FPGA as the default is it will
// check the Current and boot that automatically - it checks Golden header, sees it should try the
// Current image, and boots the current image
//#################################################################################################
void bb_top_a7_writeUserReg(uint32_t val)
{

    // Data Source: Table 7.1 from UG470, Example Bitstream For IPROG through ICAP2
    bb_top_a7_writePreamble();
    bb_top_a7_writeIcapRegister(ICAP_AXSS_REG, val);
}

//#################################################################################################
// read the user access register
//
// Parameters:
// Return:
// Assumptions:
// * At some point we'll figure out how to force-boot into golden FPGA as the default is it will
// check the Current and boot that automatically - it checks Golden header, sees it should try the
// Current image, and boots the current image
//#################################################################################################
uint32_t bb_top_a7_readUserReg(void)
{

    // Data Source: Table 7.1 from UG470, Example Bitstream For IPROG through ICAP2
    bb_top_a7_writePreamble();
    return(bb_top_a7_readIcapRegister(ICAP_AXSS_REG));
}

//#################################################################################################
// read the user access register
//
// Parameters:
// Return:
// Assumptions:
// * At some point we'll figure out how to force-boot into golden FPGA as the default is it will
// check the Current and boot that automatically - it checks Golden header, sees it should try the
// Current image, and boots the current image
//#################################################################################################
uint32_t bb_top_a7_readStatusReg(void)
{

    // Data Source: Table 7.1 from UG470, Example Bitstream For IPROG through ICAP2
    bb_top_a7_writePreamble();
    return(bb_top_a7_readIcapRegister(ICAP_BOOTSTS_REG));
}


//#################################################################################################
// write the command preamble....not sure why we have to do this
//
// Parameters:
// Return:  The value from the ICAP register
// Assumptions:
//#################################################################################################
static void bb_top_a7_writePreamble(void)
{
//    UART_printf("Write Preamble\n");
    bb_top_registers->icap.s.wdata.bf.val = 0xFFFFFFFF; // dummy word
    bb_top_registers->icap.s.wdata.bf.val = 0xFFFFFFFF; // dummy word
    bb_top_registers->icap.s.wdata.bf.val = 0xAA995566; // sync word
    bb_top_registers->icap.s.wdata.bf.val = 0x20000000; // Type 1 NO OP
}
//#################################################################################################
// write to the ICAP component one word
//
// Parameters:
// Return:  The value from the ICAP register
// Assumptions:
//#################################################################################################
static void bb_top_a7_writeIcapRegister(uint32_t reg, uint32_t val)
{
    bb_top_registers->icap.s.wdata.bf.val = (0x30000001 | reg);
    bb_top_registers->icap.s.wdata.bf.val = val;
    bb_top_registers->icap.s.wdata.bf.val = 0x20000000; // Type 1 NO OP
//    ilog_TOP_COMPONENT_2(ILOG_USER_LOG, BB_TOP_WRITE_ICAP, (0x30000001 | reg), val);

}

//#################################################################################################
// read from the ICAP component one word
//
// Parameters:
// Return:  The value from the ICAP register
// Assumptions:
//#################################################################################################
static uint32_t bb_top_a7_readIcapRegister(uint32_t reg)
{
    uint32_t val;
    bb_top_registers->icap.s.wdata.bf.val = (0x28000001 | reg);
    bb_top_registers->icap.s.wdata.bf.val = 0x20000000; // Type 1 NO OP
    bb_top_registers->icap.s.wdata.bf.val = 0x20000000; // Type 1 NO OP

    bb_top_registers->icap.s.rdata.bf.val = 0x20000000; // Type 1 NO OP to read register
    val = bb_top_registers->icap.s.rdata.bf.val;
    if(val == 0x20000000)
        val = bb_top_registers->icap.s.rdata.bf.val;

//    ilog_TOP_COMPONENT_2(ILOG_USER_LOG, BB_TOP_READ_ICAP, (0x28000001 | reg), val);

    return(val);
}

//#################################################################################################
// FPGA Watchdog timer disable commands
//
// Parameters:
// Return:
// Assumptions:
// * Used when the CRC of the FW is good - used by ROM
//#################################################################################################
void bb_top_a7_disableFpgaWatchdog(void)
{
    // Data Source: Table 7.7 from UG470, Example Bitstream For IPROG through ICAP2

    bb_top_a7_writePreamble();
    bb_top_a7_writeIcapRegister(ICAP_TIMER_REG, 0); // Disable watchdog timer
    bb_top_a7_writeIcapRegister(ICAP_CMD_REG, 0x0000000D); // DESYNC
}


//#################################################################################################
// FPGA Watchdog timer is set to 1 to trigger the fallback upon timeout
//
// Parameters:
// Return:
// Assumptions:
// * Used when the ROM CRC check of FW fails
//#################################################################################################
void bb_top_a7_triggerFpgaFallback(void)
{
    // Data Source: Table 7.6 from UG470, Example Bitstream For Reloading the Watchdog with LTIMER
    bb_top_a7_writePreamble();
    bb_top_a7_writeIcapRegister(ICAP_TIMER_REG, 0x80000001); // Reset Watchdog Timeout Value, USR Monitoring
}


//#################################################################################################
// FPGA Writes and then read the BootSTS register via ICAP to determine if current boot is Fallback
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_a7_isFpgaFallback(void)
{
    if(bb_top_readStatusReg() & 0x00000002)
        return true;
    return false;
}

// Static Function Definitions ####################################################################

#endif // PLATFORM_A7
