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


// Includes #######################################################################################
#include <options.h>
#include <bb_top.h>
#include <bb_top_ge.h>
#include <leon_timers.h>
#include <bb_chip_regs.h>
#include <bb_top_regs.h>
#include <bb_core.h>
#include <crc.h>
#include <module_addresses_regs.h>
#include <interrupts.h>
#include <uart.h>

#include "bb_top_log.h"

// Constants and Macros ###########################################################################

// the amount of time to wait for the link PLL to lock (microseconds)
#define BB_TOP_LINK_LOCK_TIMEOUT    (2*1000)    // 2ms
#define BB_TOP_CORE_IDENTIFICATION_MASK_1  0xf0   //if board is -2 or -3
#define BB_TOP_CORE_IDENTIFICATION_MASK_2  0x0f   //To get BOM type
#define BB_TOP_CORE_DASH_THREE             0x10

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static volatile bb_top_s* bb_top_registers;
static volatile bb_top_drp *drp;

// Static Function Declarations ###################################################################
static void drpWrite(uint16_t drpAddr, uint16_t writeData, bb_top_drp_drp_en_mask drpEnMask);

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize register pointer
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_Init(void)
{
    bb_top_registers = (volatile bb_top_s*) bb_chip_bb_top_s_ADDRESS;
    bb_top_registers->irq.s.enable.dw = 0;
    bb_top_registers->irq.s.pending.dw = bb_top_irq_pending_WRITEMASK;

#ifdef PLATFORM_K7
    bb_top_k7_Init();
#endif
#ifdef PLATFORM_A7
    bb_top_a7_Init();
#endif
    bb_top_ge_Init();

    TOPLEVEL_setPollingMask(SECONDARY_INT_BBTOP_INT_MSK);
}

//#################################################################################################
// Initialize the pointer
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void bb_top_drpInit(void)
{
    drp = (volatile bb_top_drp*) bb_chip_bb_top_drp_ADDRESS;
}


//#################################################################################################
// switch running image
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_switchFpgaImage(void)
{
    // if we are in the fallback we will load current
    // if we are in the current load we will load fallback
    bb_top_a7_changeFpgaImage(bb_top_IsFpgaGoldenImage());
}

//#################################################################################################
// Apply or remove tri-stating of the SCL line on the I2C bus.
//
// Parameters:
//      triState            - True if the I2C SCL line should be tri-stated.
// Return:
// Assumptions:
//#################################################################################################
void bb_top_TriStateI2cScl(bool triState)
{
#ifdef PLATFORM_K7
    bb_top_k7_TriStateI2cScl(triState);
#endif
#ifdef PLATFORM_A7
    bb_top_a7_TriStateI2cScl(triState);
#endif
}


//#################################################################################################
// Set MDIO slave
//
// Parameters:
//      slave               - The MDIO slave to select.
// Return:
// Assumptions:
//#################################################################################################
void bb_top_SetMdioSlave(enum MdioMasterSlaveSel masterSlaveSel)
{
#ifdef PLATFORM_K7
    bb_top_k7_SetMdioSlave(masterSlaveSel);
#endif
#ifdef PLATFORM_A7
    bb_top_a7_SetMdioSlave(masterSlaveSel);
#endif
}


//#################################################################################################
// Apply or remove reset to RS232 module
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_rs232Reset(bool enable)
{
#ifdef PLATFORM_A7
    bb_top_a7_rs232Reset(enable);
#endif
}


//#################################################################################################
// RS232 Chip Configure
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_rs232ChipConfigure(bool enable)
{
#ifdef PLATFORM_A7
    bb_top_a7_rs232ChipConfigure(enable);
#endif
}


//#################################################################################################
// Reset all modules except the LEON2.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_nonCpuModuleReset(void)
{
#ifdef PLATFORM_K7
    bb_top_k7_nonCpuModuleReset();
#endif
#ifdef PLATFORM_A7
    bb_top_a7_nonCpuModuleReset();
#endif
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
void bb_top_ForceSetMdioSlave(uint8_t masterSlaveSel)
{
#ifdef PLATFORM_K7
    bb_top_k7_ForceSetMdioSlave(masterSlaveSel);
#endif
#ifdef PLATFORM_A7
    bb_top_a7_ForceSetMdioSlave(masterSlaveSel);
#endif
}


//#################################################################################################
// Link clock control - turns the link clock on, then blocks until it is locked
//                      or turns the link clock off
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void bb_top_LinkClockEnable(bool enable)
{
    if (enable)
    {
        // turn the PLL on, wait for it to lock
        bb_top_registers->gcm.s.control.bf.link_gt_common_pll0reset = 0;

        // wait for the lock...
        LEON_TimerValueT startTime = LEON_TimerRead();
        while((bb_top_registers->gcm.s.status.bf.link_mmcm_lock == 0))
        {
            iassert_TOP_COMPONENT_0(
                LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < BB_TOP_LINK_LOCK_TIMEOUT,
                BB_TOP_LINK_LOCK_WAIT_TIMEOUT);
        }
    }
    else
    {
        // turn the clock off - probably not used, just put here for completeness
        bb_top_registers->gcm.s.control.bf.link_gt_common_pll0reset = 1;
    }
}



//#################################################################################################
// Control the reset signal to the RXAUI Rx hardware.
//
// Parameters:
//      reset               - when true, put the layer 3 Rx hardware in reset; otherwise,
//                            take it out of reset.
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetRxLinkStats(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.link_stats_rx_rst = reset;
}


//#################################################################################################
// Control the reset signal to the layer 3 Rx hardware.
//
// Parameters:
//      reset               - when true, put the layer 3 Rx hardware in reset; otherwise,
//                            take it out of reset.
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetLayer3Rx(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.l3_rx_rst = reset;
}


//#################################################################################################
// Control the reset signal to the layer 3 Rx hardware.
//
// Parameters:
//      reset               - when true, put the layer 3 Tx hardware in reset; otherwise,
//                            take it out of reset.
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetLayer3Tx(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.l3_tx_rst = reset;
}


//#################################################################################################
// Control the reset signal to the MCA TX and RX hardware modules.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetMcaTxRx(bool reset)
{
    bb_top_grm_soft_rst_ctrl rst_ctrl = { .dw = bb_top_registers->grm.s.soft_rst_ctrl.dw };
    if (reset)
    {
        rst_ctrl.bf.mca_core_rx_rst = 1;
        rst_ctrl.bf.mca_core_tx_rst = 1;
    }
    else
    {
        rst_ctrl.bf.mca_core_rx_rst = 0;
        rst_ctrl.bf.mca_core_tx_rst = 0;
    }
    bb_top_registers->grm.s.soft_rst_ctrl.dw = rst_ctrl.dw;
}

//#################################################################################################
// Control the reset signal to the MCA TX hardware modules.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetMcaTx(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.mca_core_tx_rst = reset;
}

//#################################################################################################
// Control the reset signal to the MCA RX hardware modules.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetMcaRx(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.mca_core_rx_rst = reset;
}


//#################################################################################################
// Control the reset signal to the MAC transmitter hardware.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetMacTx(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.dll_tx_rst = reset;
}

//#################################################################################################
// Control the reset signal to the MAC receiver hardware.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetMacRx(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.dll_rx_rst = reset;
}

//#################################################################################################
// Control the reset signal to the DisplayPort sink hardware.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetDpSink(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.dp_sink_rst = reset ? 1 : 0;
}


//#################################################################################################
// Control the reset signal to the DisplayPort encoder hardware.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetEncoder(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.tico_e_ctrl_rst = reset ? 1 : 0;
}


//#################################################################################################
// Returns true if the encoder is in reset
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_DpEncoderInReset(void)
{
    return bb_top_registers->grm.s.soft_rst_ctrl.bf.tico_e_ctrl_rst;
}

//#################################################################################################
// Returns true if the sink is in reset
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_DpSinkInReset(void)
{
    return bb_top_registers->grm.s.soft_rst_ctrl.bf.dp_sink_rst;
}

//#################################################################################################
// Returns true if the source is in reset
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_DpSourceInReset(void)
{
    return bb_top_registers->grm.s.soft_rst_ctrl.bf.dp_source_rst;
}

//#################################################################################################
// Control the reset signal to the DisplayPort source tico_d_ctrl_reset.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyDpSourceTicoDCtrlReset(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.tico_d_ctrl_rst = reset ? 1 : 0;
}


//#################################################################################################
// Control the reset signal to the DisplayPort source hardware.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetDpSource(bool reset)
{
    ilog_TOP_COMPONENT_1(ILOG_MAJOR_EVENT, BB_TOP_DP_SOURCE_RST, reset);
    bb_top_registers->grm.s.soft_rst_ctrl.bf.dp_source_rst = reset;
}


//#################################################################################################
// Determine if device is LEX or REX
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_IsDeviceLex(void)
{
    return !bb_core_isRex();
}


//#################################################################################################
// Control the reset signal to the ULP USB3 PHY clock PLL.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetUlpPhyClkPll(bool reset)
{
    // section 5.3.1.4 Power up sequence of the TUSB1310A USB phy chip, says to
    // wait around 300 microseconds after bringing the Phy PLL out of reset
    bb_top_registers->gcm.s.control.bf.ulp_phy_clk_pll_rst = reset ;
}

//#################################################################################################
// Control the USB Phy Tx clock
//
// Parameters:
//      reset               - true to enable, false to disable
// Return:
// Assumptions:
//#################################################################################################
void bb_top_UlpPhyTxClockControl(bool enable)
{
    bb_top_registers->usb3phy_ctrl.bf.tx_clk_en = enable ;
}

//#################################################################################################
// Control the reset signal to the USB3 Phy driver.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetUlpPhy(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.ulp_phy_rst = reset;
}

//#################################################################################################
// Control the reset signal to the XUSB driver.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetXusb(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.xusb_rst = reset;
}

//#################################################################################################
// Control the reset signal to the upp driver, and XUSB3 partner buffers (shared reset line).
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetUpp(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.upp_rst = reset;
}


//#################################################################################################
// Control the reset signal to the ULP core.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetUlpCore(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.ulp_core_rst = reset;
}


//#################################################################################################
// Set I2C Mux port
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_SetRtlI2cMuxPort(enum I2cPortSel port)
{
    // rtlMux for KC705 0 - TI Switch, 1/2 DP159/DP130
#ifdef PLATFORM_K7
    bb_top_k7_SetRtlI2cMuxPort(port);
#endif
#ifdef PLATFORM_A7
    bb_top_a7_SetRtlI2cMuxPort(port);
#endif
}


//#################################################################################################
// Enable Dp159 chip
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyEnableDp159(bool enable)
{
#ifdef PLATFORM_K7
    bb_top_k7_ApplyEnableDp159(enable);
#endif
#ifdef PLATFORM_A7
    bb_top_a7_ApplyEnableDp159(enable);
#endif
}


//#################################################################################################
// Enable Dp130 chip
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyEnableDp130(bool enable)
{
#ifdef PLATFORM_K7
    bb_top_k7_ApplyEnableDp130(enable);
#endif
#ifdef PLATFORM_A7
    bb_top_a7_ApplyEnableDp130(enable);
#endif
}


//#################################################################################################
// Reset Dp130 chip
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetDp130(bool reset)
{
#ifdef PLATFORM_K7
    bb_top_k7_ApplyResetDp130(reset);
#endif
#ifdef PLATFORM_A7
    bb_top_a7_ApplyResetDp130(reset);
#endif
}


//#################################################################################################
// Perform DRP Write
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_drpWrite(uint16_t drpAddr,
    uint16_t writeData,
    uint16_t drpEnMask)
{
    // Configure the DRP hardware for write access
    const bb_top_drp_drp_ctrl drpCtrl = { .bf = {
        .drp_addr = drpAddr,
        .drp_we = 1,
        .drp_di = writeData
    }};
    bb_top_registers->drp.s.drp_ctrl.dw = drpCtrl.dw;

    // Wait for all DRP buses to be idle
    while ((bb_top_registers->drp.s.drp_en_mask.dw & bb_top_drp_drp_en_mask_READMASK) != 0);

    // Enable the bus(es) we wish to write to and initiate the write
    bb_top_registers->drp.s.drp_en_mask.dw = drpEnMask;
}


//#################################################################################################
// Perform DRP Read
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint16_t bb_top_drpRead(uint16_t drpAddr, bb_top_drp_drp_en_mask drpEnMask)
{
    iassert_TOP_COMPONENT_1(drpEnMask.dw && !(drpEnMask.dw & (drpEnMask.dw - 1)),
            BB_TOP_DP_INVALID_DRP_READ,
            drpEnMask.dw);

    // Configure the DRP hardware for read access
    bb_top_drp_drp_ctrl drpCtrl;
    drpCtrl.dw = 0;
    drpCtrl.bf.drp_addr = drpAddr;
    drpCtrl.bf.drp_we = 0;

    bb_top_registers->drp.s.drp_ctrl.dw = drpCtrl.dw;

    LEON_TimerValueT startTime = LEON_TimerRead();
    // Wait for all DRP buses to be idle
    while ((bb_top_registers->drp.s.drp_en_mask.dw & bb_top_drp_drp_en_mask_READMASK) != 0);
    {
        iassert_TOP_COMPONENT_1(LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < 10000,
            BB_TOP_DRP_READ_TIME_OVER, __LINE__);
    };

    // Enable the bus we wish to read from and initiate the read
    bb_top_registers->drp.s.drp_en_mask.dw = drpEnMask.dw;

    startTime = LEON_TimerRead();
    // Wait for the read to complete
    while ((bb_top_registers->drp.s.drp_en_mask.dw & bb_top_drp_drp_en_mask_READMASK) != 0);
    {
        iassert_TOP_COMPONENT_1(LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < 10000,
            BB_TOP_DRP_READ_TIME_OVER, __LINE__);
    };

    // Return the read data
    return bb_top_registers->drp.s.drp_read_data.bf.drp_do;
}


//#################################################################################################
// Function to perform DRP Read Modify Write
//
// Parameters:
//              drpAddr - address to read from
//              writeData - data to write
//              writeMask - for writing
//              drpEnMask - mask to prevent reading incorrect bit(s)
// Return:
// Assumptions:
//#################################################################################################
void bb_top_drpReadModWrite(uint16_t drpAddr,
                     uint16_t writeData,
                     uint16_t writeMask,
                     bb_top_drp_drp_en_mask drpEnMask)
{
    // TODO for now we do individual RMWs for each bit in drpEnMask. DRP addresses
    // that do not require RMWs for which we wish to write the same value could be
    // batched together in one write, but for now we take the slower and simpler route.

    // NOTE: if the drp_en_mask register definition changes, the loop condition below
    // will need to be updated. This static assert attempts to enforce this.
    COMPILE_TIME_ASSERT(
        bb_top_drp_drp_en_mask_WRITEMASK  == 0x1FFF &&
                        BB_TOP_DRP_DRP_EN_MASK_DP_GT0 == 0x01 &&
                        BB_TOP_DRP_DRP_EN_MASK_XADC == 0x1000
                        );
    for (uint32_t i = BB_TOP_DRP_DRP_EN_MASK_DP_GT0; // 0x1 - bit 0
         i <= BB_TOP_DRP_DRP_EN_MASK_XADC; // 0x1000 - bit 12
         i <<= 1)
    {
        if (drpEnMask.dw & i)
        {
            // Perform the read
            const bb_top_drp_drp_en_mask readMask = { .dw = i };
            const uint16_t readVal = bb_top_drpRead(drpAddr, readMask);

            // Configure the DRP hardware for write access
            const bb_top_drp_drp_ctrl drpCtrl = { .bf = {
                .drp_addr = drpAddr,
                .drp_we = 1,
                .drp_di = (readVal & ~writeMask) | writeData
            }};
            drp->drp_ctrl.dw = drpCtrl.dw;

            LEON_TimerValueT startTime = LEON_TimerRead();

            // Wait for all DRP buses to be idle
            while ((drp->drp_en_mask.dw & bb_top_drp_drp_en_mask_READMASK) != 0)
            {
                iassert_TOP_COMPONENT_1(LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < 1000000,
                    BB_TOP_DRP_WRITE_TIMER_OVER,
                    drp->drp_en_mask.dw
                );
            }

            // Enable the bus(es) we wish to write to and initiate the write
            drp->drp_en_mask.dw = drpEnMask.dw;
        }
    }
}


//#################################################################################################
// Perform DRP Write - call local function
//
// Parameters:
//              drpAddr - address to write to
//              writeData - data to write
//              drpEnMask - mask to prevent writing to incorrect bit(s)
// Return:
// Assumptions:
//#################################################################################################
void bb_top_dpDrpWrite(uint16_t drpAddr, uint16_t writeData, uint32_t drpEnMask)
{
    drpWrite(drpAddr, writeData, (bb_top_drp_drp_en_mask) drpEnMask);
    ilog_TOP_COMPONENT_3(ILOG_USER_LOG, BB_TOP_DP_DRP_WRITE, drpAddr, drpEnMask, writeData);
}

//#################################################################################################
// Get technology type - asic or fpga
//
// Parameters:
// Return: true if ASIC
// Assumptions:
//#################################################################################################
bool bb_top_IsASIC(void)
{
    return (bb_top_registers->sys_config.bf.technology == 1);
}


//#################################################################################################
// Get type of FPGA - fallback (golden) or multiboot (current)
//
// Parameters:
// Return: true if the image is fallback (golden)
// Assumptions:
//#################################################################################################
bool bb_top_IsFpgaGoldenImage(void)
{
    // image_type is 0 if FPGA is fallback image
    if(bb_top_registers->sys_config.bf.image_type == 0)
        return true;
    return false;
}


//#################################################################################################
// TX MISC CTRL gt power down
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setDpGtpTxMiscCtrlPolarity(bool set)
{
    bb_top_registers->dp_gtp_tx.s.tx_misc_ctrl.bf.gt0_txpolarity = set;
    bb_top_registers->dp_gtp_tx.s.tx_misc_ctrl.bf.gt1_txpolarity = set;
    bb_top_registers->dp_gtp_tx.s.tx_misc_ctrl.bf.gt2_txpolarity = set;
    bb_top_registers->dp_gtp_tx.s.tx_misc_ctrl.bf.gt3_txpolarity = set;
}


//#################################################################################################
// RX GT Polarity
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setDpGtpRxMiscCtrlPolarity(bool set)
{
    bb_top_registers->dp_gtp_rx.s.rx_misc_ctrl.bf.gt0_rxpolarity = set;
    bb_top_registers->dp_gtp_rx.s.rx_misc_ctrl.bf.gt1_rxpolarity = set;
    bb_top_registers->dp_gtp_rx.s.rx_misc_ctrl.bf.gt2_rxpolarity = set;
    bb_top_registers->dp_gtp_rx.s.rx_misc_ctrl.bf.gt3_rxpolarity = set;

}


//#################################################################################################
// Get RXAui debug status, when all 1's we're up
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint8_t bb_top_getRxauiStatusDebug(void)
{
    return (bb_top_registers->rxaui.s.status.bf.debug);
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
        bb_top_registers->dp_gtp_common.s.pll_ctrl.bf.pll0pd = powerDn ? 1 : 0;
    }
    else
    {
        bb_top_registers->dp_gtp_common.s.pll_ctrl.bf.pll1pd = powerDn ? 1 : 0;
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
#ifdef PLATFORM_K7
void bb_top_setDpGtRxMiscCtrlPd(enum DpGtxSysLaneSel laneSel, enum DpGtxTxRxPdMode pdMode)
{
    bb_top_setDpGtxRxMiscCtrlPd(laneSel, pdMode);
#endif
#ifdef PLATFORM_A7
void bb_top_setDpGtRxMiscCtrlPd(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPdMode pdMode)
{
    bb_top_setDpGtpRxMiscCtrlPd(laneSel, pdMode);
#endif
}


//#################################################################################################
// TX MISC CTRL gt power down
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
#ifdef PLATFORM_K7
void bb_top_setDpGtTxMiscCtrlPd(enum DpGtxSysLaneSel laneSel, enum DpGtxTxRxPdMode pdMode)
{
    bb_top_setDpGtxTxMiscCtrlPd(laneSel, pdMode);
#endif
#ifdef PLATFORM_A7
void bb_top_setDpGtTxMiscCtrlPd(enum DpGtpSysLaneSel laneSel, enum DpGtpTxRxPdMode pdMode)
{
    bb_top_setDpGtpTxMiscCtrlPd(laneSel, pdMode);
#endif
}
#endif


//#################################################################################################
// DP RX Soft Reset
//
// Parameters:
//          reset - apply or not
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applyDpRxSoftReset(bool reset)
{
#ifdef PLATFORM_K7
    bb_top_k7_applyDpRxSoftReset(reset);
#endif
#ifdef PLATFORM_A7
    bb_top_a7_applyDpRxSoftReset(reset);
#endif
}



//#################################################################################################
// DP TX Soft Reset
//
// Parameters:
//          reset - apply or not
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applyDpTxSoftReset(bool reset)
{
#ifdef PLATFORM_K7
    bb_top_k7_applyDpTxSoftReset(reset);
#endif
#ifdef PLATFORM_A7
    bb_top_a7_applyDpTxSoftReset(reset);
#endif
}



//#################################################################################################
// Initialize I2C - encapsulate bb_top functions that may/may not be required for i2c bringup
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_setupI2c(void)
{
#ifdef PLATFORM_K7
    bb_top_ApplyResetI2cSwitch(false);
#endif
    bb_top_TriStateI2cScl(false);
}


//#################################################################################################
// Enable SFP Interrupts
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_irqSfpEnable(bool enable)
{
    bb_top_registers->irq.s.enable.bf.sfp_los = enable;
}


//#################################################################################################
// Control the reset signal to the GMII gigabit ethernet PHY.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyResetEthernetPhy(bool reset)
{
#ifdef PLATFORM_K7
    bb_top_k7_ApplyResetEthernetPhy(reset);
#endif
#ifdef PLATFORM_A7
    bb_top_applyResetGmiiPhy(reset);
#endif
}


//#################################################################################################
// Control the reset signal to the GMII gigabit ethernet PHY.
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_isUlpPhyClkLocked(void)
{
    return bb_top_registers->gcm.s.status.bf.ulp_phy_clk_lock == 1;
}


//#################################################################################################
// Apply Xmii RX Reset
//
// Parameters:
//      reset       - apply the reset or not
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applyXmiiRxReset(bool reset)
{
#ifdef PLATFORM_A7
    bb_top_a7_applyXmiiRxReset(reset);
#endif
}


//#################################################################################################
// Apply Xmii TX Reset
//
// Parameters:
//      reset       - apply the reset or not
// Return:
// Assumptions:
//#################################################################################################
void bb_top_applyXmiiTxReset(bool reset)
{
#ifdef PLATFORM_A7
    bb_top_a7_applyXmiiTxReset(reset);
#endif
}


//#################################################################################################
// Select XMII TX Clock
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_xmiiTxClkDetect(bool tx_clk)
{
#ifdef PLATFORM_A7
    bb_top_a7_xmiiTxClkSel(tx_clk);
#endif
}


//#################################################################################################
// Enable XMII GTX CLK
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_xmiiGtxClkEn(bool enable)
{
#ifdef PLATFORM_A7
    bb_top_a7_xmiiGtxClkEn(enable);
#endif
}


//#################################################################################################
// Set Tristates for GMII Mode
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_gmiiCtrlSetTristates(bool set)
{
#ifdef PLATFORM_A7
    bb_top_a7_gmiiCtrlSetTristates(set);
#endif
}


//#################################################################################################
// Set Tristates for MII Mode
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_miiCtrlSetTristates(bool set)
{
#ifdef PLATFORM_A7
    bb_top_a7_miiCtrlSetTristates(set);
#endif
}


//#################################################################################################
// Initiate XMII RX Clock Detection
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_initiateXmiiRxClkDetect(void)
{
#ifdef PLATFORM_A7
    bb_top_a7_initiateGcmFrequencyDetection(GCM_FREQ_DET_SEL_XMII_RX_CLK);
#endif
}


//#################################################################################################
// Has XMII RX Clock Detection Completed?
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_isXmiiRxClkDetectComplete(void)
{
#ifdef PLATFORM_A7
    return bb_top_a7_hasGcmFrequencyDetectionCompleted(false);
#else
    return true;
#endif
}


//#################################################################################################
// Get XMII RX Clock Detected frequency?
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint32_t bb_top_getXmiiRxClockFrequency(void)
{
    LEON_TimerValueT startTime = LEON_TimerRead();
    bb_top_initiateXmiiRxClkDetect();
    // Block because less complicated and Remco says worst-case time is 3.4 usec
    while ((!bb_top_isXmiiRxClkDetectComplete()) &&
          (LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < 6))
    {;}

#ifdef PLATFORM_A7
    return bb_top_a7_getNominalGcmFrequencyDetected(false);
#else
    return 0;
#endif
}


//#################################################################################################
// Reload the FPGA
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_reloadFpga(void)
{
#ifdef PLATFORM_A7
    return bb_top_a7_reloadFpga();
#endif
}


//#################################################################################################
// Disable Watchdog - this prevents automatic fallback - used on Multiboot image
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_disableFpgaWatchdog(void)
{
#ifdef PLATFORM_A7
    return bb_top_a7_disableFpgaWatchdog();
#endif
}


//#################################################################################################
// Toggle between Golden and Current image
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
#ifdef PLATFORM_A7
void bb_top_triggerFallbackFpgaIcmd(void)
{
   return bb_top_triggerFpgaFallback();             //if system is in current image switch to golden
}
#endif


//#################################################################################################
// Read FPGA boot history status via ICAP to determine if BOOTSTS sees a fallback event or not
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
#ifdef PLATFORM_A7
bool bb_top_isFpgaFallback(void)
{
    return bb_top_a7_isFpgaFallback();
}

//#################################################################################################
// Reset the Watchdog timer to a short interval to trigger reboot into fallback image
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################

void bb_top_triggerFpgaFallback(void)
{
    return bb_top_a7_triggerFpgaFallback();
}

//#################################################################################################
// Read the user scratch register in the FPGA
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint32_t bb_top_readUserReg(void)
{
    uint32_t val = bb_top_a7_readUserReg();
    ilog_TOP_COMPONENT_1(ILOG_USER_LOG, BB_TOP_READ_USER_REG,val);
    return val;
}

//#################################################################################################
// Get Core type, -3 or -2, returns true if core is dash 3
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_isDash3(void)
{
    if((bb_top_registers->sys_config.bf.env_info
                     & BB_TOP_CORE_IDENTIFICATION_MASK_1) == BB_TOP_CORE_DASH_THREE)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//#################################################################################################
// Write the user register in the FPGA
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_writeUserReg(uint32_t val)
{
    ilog_TOP_COMPONENT_1(ILOG_USER_LOG, BB_TOP_WRITE_USER_REG,val);
    bb_top_a7_writeUserReg(val);
}

//#################################################################################################
// read the status register in the FPGA
//
// Parameters:
// Return: the status register value
// Assumptions:
//#################################################################################################
uint32_t bb_top_readStatusReg(void)
{
   return(bb_top_a7_readStatusReg());
}
#endif

//#################################################################################################
// Get BOM type
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
enum CORE_REV_BB bb_top_getCoreBoardRev(void)
{
    return bb_top_registers->sys_config.bf.env_info;
}

//#################################################################################################
// Reset the aux
//
// Parameters: reset - true puts the AUX module in reset, false allows normal operation
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ResetAuxHpd(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.dp_aux_hpd_rst = reset;
}

//#################################################################################################
// Reset control for Gpio ctrl
//
// Parameters: reset (1: reset on, 0: out of reset)
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ResetGpio(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.gpio_ctrl_rst = 0;
}

//#################################################################################################
// Reset control for Spi flash
//
// Parameters: reset (1: reset on, 0: out of reset)
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ResetSpiFlash(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.spi_flash_ctrl_rst = 0;
}

//#################################################################################################
// Reset control for BB Uart
//
// Parameters: reset (1: reset on, 0: out of reset)
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ResetBBUart(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.bb_uart_rst = 0;
}

//#################################################################################################
// Reset control for MDIO
//
// Parameters: reset (1: reset on, 0: out of reset)
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ResetMdioMaster(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.mdio_master_rst = reset;
}

//#################################################################################################
// Reset control for I2C Master
//
// Parameters: reset (1: reset on, 0: out of reset)
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ResetI2CMaster(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.i2c_master_rst = reset;
}

//#################################################################################################
// Reset control for I2C Slave
//
// Parameters: reset (1: reset on, 0: out of reset)
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ResetI2CSlave(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.i2c_slave_rst = reset;
}

//#################################################################################################
// Reset control for GE UART
//
// Parameters: reset (1: reset on, 0: out of reset)
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ResetGeUart(bool reset)
{
    bb_top_registers->grm.s.soft_rst_ctrl.bf.ge_uart_rst = reset;
}

//#################################################################################################
// Calculate the CRC for the target image
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_CalcTargetImageCrc(void)
{
    int crcImageSize = *(int *)0xC0A00004;
    uint32_t crc = 0;

    crcInit();
    UART_printf("Target size: 0x%x\n",crcImageSize);

    if(crcImageSize < 0x80000)
    {
        crc = crcFast((const uint8_t *)0xC0A00100, crcImageSize);
    }

    UART_printf("Target CRC: 0x%x\n",crc);
}

//#################################################################################################
// Calculate the CRC of the FPGA image
//
// Parameters: reset (1: reset on, 0: out of reset)
// Return:
// Assumptions:
//#################################################################################################
void bb_top_CalcFpgaImageCrc(void)
{
    uint32_t crcImageSize = *(int *)0xC0000000;
    uint32_t *crcHalfWord;

    uint64_t crc = crcFastInit();

    UART_printf("FPGA size: 0x%x\n",crcImageSize);
    if(crcImageSize < 0x600000)
    {
        crc = crcFastBlock((uint8_t *)0xC0000010, crcImageSize, crc);
    }

    crc = crcFastFinalize(crc);

    crcHalfWord = (uint32_t *)&crc;
    UART_printf("FPGA CRC: 0x%x%x\n",crcHalfWord[0],crcHalfWord[1]);

}

//#################################################################################################
// Checks what the FPGA core type is
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_coreTypeIcmd(void)
{
    ilog_TOP_COMPONENT_1(ILOG_MAJOR_EVENT, BB_TOP_CORE_TYPE, bb_top_isDash3() ? 3 : 2);
}

// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################

//#################################################################################################
// Funcion to perform DRP write
//
// Parameters:
//              drpAddr - address to write to
//              writeData - data to be written
//              drpEnMask - mask to prevent writing to incorrect bit(s)
// Return:
// Assumptions:
//#################################################################################################
static void drpWrite(uint16_t drpAddr, uint16_t writeData, bb_top_drp_drp_en_mask drpEnMask)
{
    // Configure the DRP hardware for write access
    const bb_top_drp_drp_ctrl drpCtrl = { .bf = {
        .drp_addr = drpAddr,
        .drp_we = 1,
        .drp_di = writeData
    }};
    drp->drp_ctrl.dw = drpCtrl.dw;

    LEON_TimerValueT startTime = LEON_TimerRead();
    // Wait for all DRP buses to be idle
    while (((drp->drp_en_mask.dw & bb_top_drp_drp_en_mask_READMASK) != 0)
           && (LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < 10000))
    {;}

    if ((drp->drp_en_mask.dw & bb_top_drp_drp_en_mask_READMASK) != 0)
    {
        ilog_TOP_COMPONENT_0(ILOG_FATAL_ERROR, BB_TOP_DRP_WRITE_WAIT_TIMEOUT);
    }
    // Enable the bus(es) we wish to write to and initiate the write
    drp->drp_en_mask.dw = drpEnMask.dw;
}

