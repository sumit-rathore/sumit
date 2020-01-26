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
#ifndef BB_TOP_H
#define BB_TOP_H

// Includes #######################################################################################
#include <ibase.h>
#include <top_gpio.h>
#include <gpio.h>
#include <bb_top_a7_regs.h>
#ifdef PLATFORM_K7
#include <bb_top_k7.h>
#endif
#ifdef PLATFORM_A7
#include <bb_top_a7.h>
#endif

// Constants and Macros ###########################################################################
#define MDIO_MUX_NOT_PRESENT    (0xF)
#define I2C_MUX_NOT_PRESENT     (0xFF)
#define I2C_SWITCH_NOT_PRESENT  (0xFF)

// Data Types #####################################################################################
// Core board BOM types for Blackbird
enum CORE_REV_BB
{
    BOM_23_00200_A01,
    BOM_23_00200_A02,
    BOM_23_00200_A03
};

// Function Declarations ##########################################################################
void bb_top_Init(void);
// wrappers
void bb_top_TriStateI2cScl(bool triState);
void bb_top_SetMdioSlave(enum MdioMasterSlaveSel masterSlaveSel);
void bb_top_ForceSetMdioSlave(uint8_t masterSlaveSel);
void bb_top_ApplyResetPhyGtx(bool reset);
// common
void bb_top_ApplyResetMcaTxRx(bool reset);
void bb_top_ApplyResetMcaTx(bool reset);
void bb_top_ApplyResetMcaRx(bool reset);
void bb_top_ApplyResetMacTx(bool reset);
void bb_top_ApplyResetMacRx(bool reset);
bool bb_top_DpSourceInReset(void);
void bb_top_ApplyResetDpSource(bool reset);
void bb_top_ApplyDpSourceTicoDCtrlReset(bool reset);
bool bb_top_DpSinkInReset(void);
bool bb_top_DpEncoderInReset(void);
void bb_top_ApplyResetDpSink(bool reset);
void bb_top_ApplyResetEncoder(bool reset);
void bb_top_ApplyResetUlpPhyClkPll(bool reset);
void bb_top_UlpPhyTxClockControl(bool enable)                                       __attribute__((section(".atext")));
void bb_top_ApplyResetUlpCore(bool reset);
void bb_top_ApplyResetUlpPhy(bool reset);
void bb_top_ApplyResetXusb(bool reset);
void bb_top_ApplyResetUpp(bool reset); // MCR
bool bb_top_IsDeviceLex(void);
void bb_top_LinkClockEnable(bool enable);
void bb_top_ApplyResetRxLinkStats(bool reset);
void bb_top_ApplyResetLayer3Rx(bool reset);
void bb_top_ApplyResetLayer3Tx(bool reset);
bool bb_top_isUlpPhyClkLocked(void);
void bb_top_ResetGpio(bool reset);
void bb_top_ResetSpiFlash(bool reset);
void bb_top_ResetBBUart(bool reset);
void bb_top_ResetMdioMaster(bool reset);
void bb_top_ResetI2CMaster(bool reset);
void bb_top_ResetI2CSlave(bool reset);
void bb_top_ResetGeUart(bool reset);
void bb_top_freqDetDpAutoEnable(bool enable);
// wrappers
void bb_top_SetRtlI2cMuxPort(enum I2cPortSel port);
void bb_top_ApplyEnableDp159(bool enable);
void bb_top_ApplyEnableDp130(bool enable);
void bb_top_ApplyResetDp130(bool reset);
void bb_top_ResetAuxHpd(bool reset);

// common
void bb_top_drpWrite(uint16_t drpAddr, uint16_t writeData, uint16_t drpEnMask);
uint16_t bb_top_drpRead(uint16_t drpAddr, bb_top_drp_drp_en_mask drpEnMask);
void bb_top_drpReadModWrite(uint16_t drpAddr,
                     uint16_t writeData,
                     uint16_t writeMask,
                     bb_top_drp_drp_en_mask drpEnMask);
// common
bool bb_top_IsASIC(void);
uint8_t bb_top_getRxauiStatusDebug(void);
// potential wrapper conversion
#ifdef PLATFORM_A7_K7
void bb_top_setDpGtpRxClkCtrl(enum DpGtpSysLaneSel laneSel, enum DpGtpSysClkSelCfg clkCfg);
void bb_top_setDpGtpTxClkCtrl(enum DpGtpSysLaneSel laneSel, enum DpGtpSysClkSelCfg clkCfg);
void bb_top_setGzpPllRefClkSel(enum DpGtpPllClkSel pllClkSel, enum DpGtpPllCtrlRefClkSel refClkSel);
void bb_top_applyGtpPllPowerDown(enum DpGtpPllClkSel pllClkSel, bool powerDn);
bool bb_top_isGtpPllRefClkLost(enum DpGtpPllClkSel pllClkSel);
bool bb_top_isGtpPllLock(enum DpGtpPllClkSel pllClkSel);
void bb_top_setDpGtpRxMiscCtrlPd(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPdMode pdMode);
void bb_top_setDpGtpTxMiscCtrlPd(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPdMode pdMode);
void bb_top_setDpGtpRxPrbsCtrlSel(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPrbsSel prbs);
void bb_top_applyDpGtpRxPrbsCtrlPrbsCntReset(enum DpGtpSysLaneSel laneSel, bool reset);
void bb_top_setDpGtpTxPrbsCtrlSel(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPrbsSel prbs);
#endif  //PLATFORM_A7_K7

void bb_top_setDpGtpRxMiscCtrlPolarity(bool set);
void bb_top_setDpGtpTxMiscCtrlPolarity(bool set);
void bb_top_applyDpRxSoftReset(bool reset);
void bb_top_applyDpTxSoftReset(bool reset);

// wrapper
void bb_top_ApplyResetEthernetPhy(bool reset);

// common
void bb_top_irqSfpEnable(bool enable);

// Encapsulation functions to abstract HW variations, where possible
void bb_top_setupI2c(void);

void bb_top_nonCpuModuleReset(void);
void bb_top_reloadFpga(void);
bool bb_top_IsFpgaGoldenImage(void);
void bb_top_disableFpgaWatchdog(void);
bool bb_top_isFpgaFallback(void);

uint32_t bb_top_readUserReg(void);
void bb_top_writeUserReg(uint32_t val);
uint32_t bb_top_readStatusReg(void);

void bb_top_triggerFpgaFallback(void);
void bb_top_switchFpgaImage(void);
void bb_top_initiateXmiiRxClkDetect(void)               __attribute__((section(".atext")));
bool bb_top_isXmiiRxClkDetectComplete(void)             __attribute__((section(".atext")));
uint32_t bb_top_getXmiiRxClockFrequency(void)           __attribute__((section(".atext")));
void bb_top_xmiiTxClkDetect(bool tx_clk)                __attribute__((section(".atext")));
void bb_top_xmiiGtxClkEn(bool enable)                   __attribute__((section(".atext")));
void bb_top_gmiiCtrlSetTristates(bool set)              __attribute__((section(".atext")));
void bb_top_miiCtrlSetTristates(bool set)               __attribute__((section(".atext")));
void bb_top_applyXmiiRxReset(bool reset)                __attribute__((section(".atext")));
void bb_top_applyXmiiTxReset(bool reset)                __attribute__((section(".atext")));
enum CORE_REV_BB bb_top_getCoreBoardRev(void)           __attribute__((section(".atext")));

void bb_top_rs232Reset(bool enable)                     __attribute__((section(".atext")));
void bb_top_rs232ChipConfigure(bool enable)             __attribute__((section(".atext")));
bool bb_top_isDash3(void)                               __attribute__((section(".atext")));


#endif // BB_TOP_H
