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
// Hardware driver for the MAC TX and RX blocks.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <options.h>
#include <ilog.h>
#include <interrupts.h>
#include <timing_timers.h>
#include <configuration.h>
#include <bb_top.h>
#include <stats_mon.h>
#include <mac.h>
#include "mac_log.h"
#include "mac_loc.h"
#include <bb_core.h>
#include <module_addresses_regs.h>

// Constants and Macros ###########################################################################
#define MAC_FRAME_RATE_MASK     (0xFFF) // 12 bits wide

#define MAC_LINK_LAYER_RX_IRQ \
       (LINK_LAYER_RX_IRQ0_ENABLE_LINK_INT      | \
        LINK_LAYER_RX_IRQ0_ENABLE_REMOTE_FAULT  | \
        LINK_LAYER_RX_IRQ0_ENABLE_LOCAL_FAULT   | \
        LINK_LAYER_RX_IRQ0_ENABLE_LINK_OK)

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct
{
    MacLinkRxStatusChangeHandler notifyStatusChange; // handler to notify on status change

} macContext;

// Static Function Declarations ###################################################################
static void MAC_layer3TxConfigure(void);
static void MAC_linkLayerTxConfigure(void);
static void MAC_linkLayerRxConfigure(void);

// static void MAC_linkLayerTxRxConfigure(void);

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initializes the MAC firmware module.
//
// Parameters:
//      linkLayerRx         - Base address of the link layer RX address block.
//      linkLayerTx         - Base address of the link layer TX address block.
//      layer3Rx            - Base address of the layer3 RX addressblock.
//      layer3Tx            - Base address of the layer3 TX addressblock.
//      statsMonitorPeriod  - Period in ms of the stats monitor timer.
// Return:
// Assumptions:
//      * This function should be called exactly once at startup.
//#################################################################################################
void MAC_Init(void)
{

    linkLayerRx = (volatile link_layer_rx_s*) bb_chip_link_layer_rx_s_ADDRESS;
    linkLayerTx = (volatile link_layer_tx_s*) bb_chip_link_layer_tx_s_ADDRESS;
    layer3Tx    = (volatile layer3_tx_s*) bb_chip_layer3_tx_s_ADDRESS;
    layer3Rx    = (volatile layer3_rx_s*) bb_chip_layer3_rx_s_ADDRESS;

    MAC_StatInit();
    TOPLEVEL_setPollingMask(SECONDARY_INT_LINK_LAYER_RX_MSK);
}


//#################################################################################################
// Enables the link layer Tx hardware registers.
//
// Parameters:
// Return:
// Assumptions:
//      * This function should be called every time the Phy layer comes up
//      Tx will be emabled after Rx, So MAC_StartStatsMonitor is here
//#################################################################################################
void MAC_EnableTx(void)
{
    ilog_MAC_COMPONENT_0(ILOG_USER_LOG, LL_TX_MAC_ENABLED);

    // Take the MAC TX out of reset
    bb_top_ApplyResetMacTx(false);

    // Configure default register settings for LinkLayerTx
    MAC_linkLayerTxConfigure();

    // Determine the configured link rate and set the MAC's bandwidth accordingly
    ConfigPhyParams *phyParams =  &(Config_GetBuffer()->phyParams);

    if (Config_ArbitrateGetVar(CONFIG_VAR_PHY_PARAMS, phyParams))
    {
        const enum ConfigBlockLinkSpeed linkSpeed =
            phyParams->phyConfig & ((1 << CONFIG_BLOCK_PHY_LINK_SPEED_BIT_0) | (1 << CONFIG_BLOCK_PHY_LINK_SPEED_BIT_1));

        switch(linkSpeed)
        {
            case CONFIG_BLOCK_LINK_SPEED_5G:
                MAC_changeFrameRate(0);
                linkLayerTx->mac.s.bandwidth_limit.bf.bandwidth_limit = MAC_5G_BANDWIDTH_LIMIT;
                linkLayerTx->mac.s.bandwidth_gap.bf.bandwidth_gap = MAC_5G_BANDWIDTH_GAP;
                break;

            case CONFIG_BLOCK_LINK_SPEED_10G:
                MAC_changeFrameRate(0);
                break;

            case CONFIG_BLOCK_LINK_SPEED_1G:
            case CONFIG_BLOCK_LINK_SPEED_2_5G:
            default:
                break;
        }
    }

    // This enables the debug logic which tags each Ethernet Frame with a sequence number. The
    // sequence number is inserted in the Frame Destination Address field.
    // NOTE: Because of the aforementioned change, if we go through a switch, the frame will not go
    // through
    // *** During bringup, the logic should be enabled on both Tx and Rx before data stream is
    // available
    // *** If toggled via an icmd, the logic must be enabled on Tx side before it is enabled on Rx side
    linkLayerTx->mac.s.mac_config.bf.dbg_seq_num_en = 0x01;
}


//#################################################################################################
// Shuts down the link layer Tx hardware registers.
//
// Parameters:
// Return:
// Assumptions:
//      * This function should be called every time the Phy layer goes down
//      Tx will be disabled first before Rx, So MAC_StopStatsMonitor is here
//      Tx is controlled from High level to Low level, so MAC is enabled after L3 enable
//#################################################################################################
void MAC_DisableTx(void)
{
    ilog_MAC_COMPONENT_0(ILOG_USER_LOG, LL_TX_MAC_DISABLED);

    bb_top_ApplyResetMacTx(true);       // put the MAC Tx layer into reset
}

//#################################################################################################
// Enables the link layer Rx hardware registers.
//
// Parameters:
// Return:
// Assumptions:
//      * This function should be called every time the Phy layer comes up
//
//#################################################################################################
void MAC_EnableRx(MacLinkRxStatusChangeHandler changeNotification)
{
    // save the notify change handler
    macContext.notifyStatusChange = changeNotification;

    ilog_MAC_COMPONENT_0(ILOG_USER_LOG, LL_RX_MAC_ENABLED);

    // Take the MAC RX out of reset
    bb_top_ApplyResetMacRx(false);

    // Configure default register settings for LinkLayerRx
    MAC_linkLayerRxConfigure();

    linkLayerRx->mac.s.pfifo.s.write_engine.s.config0.s.mode.bf.pkt_strm = 0;           // set to stream mode
    linkLayerRx->mac.s.pfifo.s.write_engine.s.config0.s.mode.bf.drp_on_pkt_err = 0;     // don't drop on packet errors
    linkLayerRx->mac.s.pfifo.s.write_engine.s.config0.s.mode.bf.flw_ctrl_en = 0;        // disable flow control


    // This enables the debug logic which tags each Ethernet Frame with a sequence number. The
    // sequence number is inserted in the Frame Destination Address field.
    // NOTE: Because of the aforementioned change, if we go through a switch, the frame will not go
    // through
    // *** During bringup, the logic should be enabled on both Tx and Rx before data stream is
    // available
    // *** If toggled via an icmd, the logic must be enabled on Tx side before it is enabled on Rx side
    linkLayerRx->mac.s.mac_config.bf.dbg_seq_num_en = 0x01;
}

//#################################################################################################
// Shuts down the link layer Rx hardware registers.
//
// Parameters:
// Return:
// Assumptions:
//      * This function should be called every time the Phy layer goes down
//      Rx is controlled from Low level to High level, so MAC is disabled first and then disable L3
//#################################################################################################
void MAC_DisableRx(void)
{
    ilog_MAC_COMPONENT_0(ILOG_USER_LOG, LL_RX_MAC_DISABLED);

    //LEON_DisableIrq2Bits(SECONDARY_INT_LINK_LAYER_RX_MSK);
    TOPLEVEL_clearPollingMask(SECONDARY_INT_LINK_LAYER_RX_MSK);
    bb_top_ApplyResetMacRx(true);       // put the MAC Rx layer into reset
}

//#################################################################################################
// Enables the layer3 Tx hardware registers.
//
// Parameters:
// Return:
// Assumptions:
//      * This function should be called every time the Phy layer comes up
//
//#################################################################################################
void MAC_EnableLayer3Tx(void)
{
    ilog_MAC_COMPONENT_0(ILOG_USER_LOG, LL_TX_LINK_LAYER3_ENABLED);

    // Clear reset and configure layer 3 block
    bb_top_ApplyResetLayer3Tx(false);

    // Configure default register settings for LinkLayerTx
    MAC_layer3TxConfigure();
}

//#################################################################################################
// Disables the layer3 Tx hardware registers.
//
// Parameters:
// Return:
// Assumptions:
//      * This function should be called every time the Phy layer comes up
//
//#################################################################################################
void MAC_DisableLayer3Tx(void)
{
    ilog_MAC_COMPONENT_0(ILOG_USER_LOG, LL_TX_LINK_LAYER3_DISABLED);

    bb_top_ApplyResetLayer3Tx(true);    // put Layer 3 Tx into Reset
}

//#################################################################################################
// Enables the layer3 Receive hardware registers.
//
// Parameters:
// Return:
// Assumptions:
//      * This function should be called every time the Phy layer comes up
//
//#################################################################################################
void MAC_EnableLayer3Rx(void)
{
    ilog_MAC_COMPONENT_0(ILOG_USER_LOG, LL_RX_LINK_LAYER3_ENABLED);

    // Clear reset and configure layer 3 block
    bb_top_ApplyResetLayer3Rx(false);
}

//#################################################################################################
// Disables the layer3 Receive hardware registers.
//
// Parameters:
// Return:
// Assumptions:
//      * This function should be called every time the Phy layer comes up
//
//#################################################################################################
void MAC_DisableLayer3Rx(void)
{
    ilog_MAC_COMPONENT_0(ILOG_USER_LOG, LL_RX_LINK_LAYER3_DISABLED);

    bb_top_ApplyResetLayer3Rx(true);    // put Layer 3 Rx into Reset
}

//#################################################################################################
// changes frm_rate based on desired ethernet frame gap.
//
// Parameters:
//      ethFrameGap     - packet gap in bytes.
// Return:
// Assumptions:
//#################################################################################################
void MAC_changeFrameRate(uint16_t ethFrameGap)
{
    // frm_rate is implemented by multiplying by the bus_size (64bits for narrow, 128 for wide)
#ifdef PLATFORM_A7 // wide MAC, 128 bit
    uint16_t frm_rate = ethFrameGap * 8 / 128;
#endif
#ifdef PLATFORM_K7 // narrow MAC, 64 bit
    uint16_t frm_rate = ethFrameGap * 8 / 64;
#endif

    linkLayerTx->mac.s.frm_rate.bf.frm_rate = frm_rate & MAC_FRAME_RATE_MASK;
}


//#################################################################################################
// turns on or off remote fault generation
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void MAC_RemoteFaultEnable(bool enable)
{
    if (enable)
    {
        linkLayerRx->irq0.s.enable.dw = 0;  // disable all link state interrupts for now

        linkLayerTx->control.bf.force_xgmii_rf = true;
    }
    else
    {
        // turn on the Rx Link interrupts
        //LEON_EnableIrq2Bits(SECONDARY_INT_LINK_LAYER_RX_MSK);
        TOPLEVEL_setPollingMask(SECONDARY_INT_LINK_LAYER_RX_MSK);
        // we are ready - set up to see what the other side says
        linkLayerRx->irq0.s.enable.dw = MAC_LINK_LAYER_RX_IRQ;
        MAC_LinkLayerRxIsr();   // Read and clear the current state of the interrupts

        // turn off sending of remote faults
        linkLayerTx->control.bf.force_xgmii_rf = false;
    }
}

//#################################################################################################
// ISR for MAC link layer Rx events
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void MAC_LinkLayerRxIsr(void)
{
    // these are status interrupts - no processing needs to be done to clear them except
    // acknowledging them, in case there are more
    uint32_t pendingInterrupts = (linkLayerRx->irq0.s.pending.dw & linkLayerRx->irq0.s.enable.dw); // get the pending interrupts
    linkLayerRx->irq0.s.pending.dw = pendingInterrupts;          // clear them

    // get a snapshot of the current interrupts
    link_layer_rx_irq0_raw  raw = linkLayerRx->irq0.s.raw;

    ilog_MAC_COMPONENT_3(ILOG_USER_LOG, LL_RX_LINK_LAYER_IRQ,
        pendingInterrupts,
        raw.bf.link_ok,
        raw.bf.remote_fault);

    if (raw.bf.link_ok)
    {
        macContext.notifyStatusChange(true);        // link ok!  Tell the link manager
    }
    else if (raw.bf.remote_fault)
    {
        macContext.notifyStatusChange(false);       // remote fault!  link down
    }
}

// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################

//#################################################################################################
// Register default settings for Link layer TX and RX.
//
// Parameters:
// Return:
// Assumptions:
//      * This function should only be called once during MAC layer enable
//      * Both link layer Tx and link layer Rx must be out of reset before this function is called
//
//#################################################################################################
// static void MAC_linkLayerTxRxConfigure(void)
// {
//     // Disable self-test for RX and TX
//     linkLayerRx->self_test_support.s.set_up.bf.enable = 0;
//     linkLayerTx->self_test_support.s.set_up.bf.enable = 0;

//     linkLayerTx->media_rs.s.pfifo.s.write_engine.s.config0.s.notify_limit.bf.ntfy_lmt = 24;

//     // Enable XGMII and disable GMII for RX and TX
//     link_layer_tx_media_rs_enable txEn = { .dw=linkLayerTx->media_rs.s.enable.dw };
//     txEn.bf.tx_gmii_en = 0;
//     txEn.bf.tx_xgmii_en = 1;
//     linkLayerTx->media_rs.s.enable.dw = txEn.dw;

//     link_layer_rx_media_rs_enable rxEn = { .dw=linkLayerRx->media_rs.s.enable.dw };
//     rxEn.bf.rx_gmii_en = 0;
//     rxEn.bf.rx_xgmii_en = 1;
//     linkLayerRx->media_rs.s.enable.dw = rxEn.dw;
// }

//#################################################################################################
// Register default settings for Link layer TX.
//
// Parameters:
// Return:
// Assumptions:
//      * This function should only be called once during MAC layer enable
//      * link layer Tx must be out of reset before this function is called
//
//#################################################################################################
static void MAC_linkLayerTxConfigure(void)
{
    // Disable self-test for TX
    linkLayerTx->self_test_support.s.set_up.bf.enable = 0;

    linkLayerTx->media_rs.s.pfifo.s.write_engine.s.config0.s.notify_limit.bf.ntfy_lmt = 24;

    // Enable XGMII and disable GMII for TX
    linkLayerTx->media_rs.s.enable.bf.tx_gmii_en = 0;
    linkLayerTx->media_rs.s.enable.bf.tx_xgmii_en = 1;
}

//#################################################################################################
// Register default settings for Link layer RX.
//
// Parameters:
// Return:
// Assumptions:
//      * This function should only be called once during MAC layer enable
//      * link layer Rx must be out of reset before this function is called
//
//#################################################################################################
static void MAC_linkLayerRxConfigure(void)
{
    // Disable self-test for RX
    linkLayerRx->self_test_support.s.set_up.bf.enable = 0;

    // Enable XGMII and disable GMII for RX
    linkLayerRx->media_rs.s.enable.bf.rx_gmii_en = 0;
    linkLayerRx->media_rs.s.enable.bf.rx_xgmii_en = 1;
}

//#################################################################################################
// Register default settings for Layer3 TX and RX.
//
// Parameters:
// Return:
// Assumptions:
//      * This function should only be called once during MAC layer enable
//      * Both layer3Tx and layer3Rx must be out of reset before this function is called
//
//#################################################################################################
static void MAC_layer3TxConfigure(void)
{
    if(bb_core_getLinkMode() == CORE_LINK_MODE_AQUANTIA)
    {
#ifdef BB_ISO
        layer3Tx->eth_framer.s.config0.s.max_byte.bf.max_byte           = 128;
        layer3Tx->eth_framer.s.config0.s.timer_wait.bf.timer_wait       = 0;
        layer3Tx->eth_framer.s.config0.s.mtu_size.bf.mtu_size           = 448;
#else
        layer3Tx->eth_framer.s.config0.s.max_byte.bf.max_byte           = 128;
        layer3Tx->eth_framer.s.config0.s.timer_wait.bf.timer_wait       = 0;
        //layer3Tx->eth_framer.s.config0.s.mtu_size.bf.mtu_size           = 448;
        //layer3Tx->eth_framer.s.config0.s.mtu_size.bf.mtu_size           = 512;
        if(bb_top_IsDeviceLex())
        {
            layer3Tx->eth_framer.s.config0.s.mtu_size.bf.mtu_size           = 800;
        }
        else
        {
            layer3Tx->eth_framer.s.config0.s.mtu_size.bf.mtu_size           = 576;
            //layer3Tx->eth_framer.s.config0.s.mtu_size.bf.mtu_size           = 1500;
        }
#endif
    }
    else if(bb_core_getLinkMode() == CORE_LINK_MODE_ONE_LANE_SFP_PLUS)
    {
#ifdef BB_ISO
        layer3Tx->eth_framer.s.config0.s.max_byte.bf.max_byte = 80;
        layer3Tx->eth_framer.s.config0.s.mtu_size.bf.mtu_size = 512;
#else
        layer3Tx->eth_framer.s.config0.s.max_byte.bf.max_byte = 80;
        layer3Tx->eth_framer.s.config0.s.mtu_size.bf.mtu_size = 400;
#endif
    }

    // setup the PTP guard
    // fix_ptp_violations is activated by default
    // layer3Tx->eth_framer.s.eth_framer_ptp_guard.s.eth_framer_ptp_guard_config0.s.fix_ptp_violations.bf.activate = 1;

    layer3Tx->eth_framer.s.ptp_guard.s.config0.s.max_cycles.bf.max_cycles = 0xC0;
}
