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
#ifndef MCA_LOG_H
#define MCA_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################
ILOG_CREATE(MCA_COMPONENT)
    ILOG_ENTRY(MCA_INVALID_CH_ASSERT, "Channel %d not valid!\n")
    ILOG_ENTRY(MCA_CORE_NOT_READY, "MCA core not ready, Rx =%d Tx = %d\n")
    ILOG_ENTRY(MCA_CORE_INTERRUPT,    "Core IRQ pending interrupt 0x%08x, raw interrupt state 0x%08x\n")
    ILOG_ENTRY(MCA_CHANNEL_INTERRUPT, "Channel %d IRQ pending interrupt 0x%08x, raw interrupt state 0x%08x\n")

    ILOG_ENTRY(MCA_ENABLE_CORE_UP, "MCA Enable Core Up \n ")
    ILOG_ENTRY(MCA_ENABLE_TX,  "MCA Tx Enabled\n ")
    ILOG_ENTRY(MCA_DISABLE_TX, "MCA Tx Disabled\n ")
    ILOG_ENTRY(MCA_ENABLE_RX,  "MCA Rx Enabled\n ")
    ILOG_ENTRY(MCA_DISABLE_RX, "MCA Rx Disabled\n ")
    ILOG_ENTRY(MCA_CHANNEL_LINK_SETUP, "MCA Channel link setup channel %d\n")
    ILOG_ENTRY(MCA_CHANNEL_LINK_NOT_ENABLED, "MCA Channel Link Setup: Channel %d Not Enabled\n ")
    ILOG_ENTRY(MCA_CHANNEL_TX_RX_NOT_ENABLED, "MCA Channel Tx Rx Setup: Channel %d Not Enabled\n ")
    ILOG_ENTRY(MCA_CHANNEL_DISABLE_NOT_ENABLED, "MCA Channel Disable: Channel %d Not Enabled\n ")

    ILOG_ENTRY(MCA_DISABLE_TIMEOUT, "MCA CH[%d] Disable timeout, irq = 0x%x\n")
    ILOG_ENTRY(MCA_UP_FAILED, "MCA CH[%d] Up couldn't handled. status = 0x%x\n")
    ILOG_ENTRY(MCA_DN_FAILED, "MCA CH[%d] is down already. status = 0x%x, down processing = %d\n")
    ILOG_ENTRY(MCA_CHANNEL_LINKUP, "MCA Channel[%d] Link Up, Status: %d\n")
    ILOG_ENTRY(MCA_CHANNEL_LINKUP_REQ, "MCA Channel[%d] Link Up Requested while Link Dn processing\n")
    ILOG_ENTRY(MCA_CHANNEL_LINKDN, "MCA Channel[%d] Link Dn, Status: %d\n")
    ILOG_ENTRY(MCA_CHANNEL_LINKDN_REQ, "MCA Channel[%d] Link Down Requested while Link Up processing\n")
    ILOG_ENTRY(MCA_CHANNEL_LINKDN_DONE, "MCA Channel[%d] Link Down finished\n")
    ILOG_ENTRY(MCA_CHANNEL_RX_FIFO_FULL, "MCA Channel[%d] Rx fifo full (Pending Irq: 0x%x). Disable this channel\n")
    ILOG_ENTRY(MCA_CHANNEL_RX_GRD_MAX_ERR, "MCA Channel[%d] Rx Grd Max Error (Pending Irq: 0x%x). Disable this channel\n")

    // New MCA STAT Logs down below =======================================
    ILOG_ENTRY(MCA_LINK_STAT_U0_TO_REC, "STAT:MCA mca_channel[%d]->link->stats0->u0_to_rec = 0x%x\n")
    ILOG_ENTRY(MCA_LINK_STAT_POL_FAIL, "STAT:MCA mca_channel[%d]->link->stats0->pol_fail = 0x%x\n")
    ILOG_ENTRY(MCA_LINK_STAT_TX_FRM, "STAT:MCA mca_channel[%d]->link->stats0->tx_frm = 0x%x\n")
    ILOG_ENTRY(MCA_LINK_STAT_TX_MCUP, "STAT:MCA mca_channel[%d]->link->stats0->tx_mcup = 0%x\n")
    ILOG_ENTRY(MCA_LINK_STAT_RX_FRM, "STAT:MCA mca_channel[%d]->link->stats0->rx_frm = 0x%x\n")
    ILOG_ENTRY(MCA_LINK_STAT_RX_MCUP, "STAT:MCA mca_channel[%d]->link->stats0->rx_mcup = 0x%x\n")

    ILOG_ENTRY(MCA_TX_STAT_CMD_FIFO, "STAT:MCA mca_channel[%d]->tx->stats0->cmd_fifo = 0x%x\n")
    ILOG_ENTRY(MCA_TX_STAT_DP_PFIFO, "STAT:MCA mca_channel[%d]->tx->stats0->dp_pfifo = 0x%x\n")
    ILOG_ENTRY(MCA_TX_STAT_DP_NFIFO, "STAT:MCA mca_channel[%d]->tx->stats0->dp_nfifo = 0x%x\n")
    ILOG_ENTRY(MCA_TX_STAT_DP_FIFO_RD_DRP_PKT, "STAT:MCA mca_channel[%d]->tx->stats0->dp_fifo_rd_drp_pkt = 0x%x\n")
    ILOG_ENTRY(MCA_TX_STAT_DP_FIFO_WR_DRP_PKT, "STAT:MCA mca_channel[%d]->tx->stats0->dp_fifo_wr_drp_pkt = 0x%x\n")
    ILOG_ENTRY(MCA_TX_STAT_DP_FIFO_WR_PKT_ERR, "STAT:MCA mca_channel[%d]->tx->stats0->dp_fifo_wr_pkt_err = 0x%x\n")
    ILOG_ENTRY(MCA_TX_STAT_DP_FIFO_FIFO_FULL_ERR, "STAT:MCA mca_channel[%d]->tx->stats0->dp_fifo_fifo_full_err = 0x%x\n")
    ILOG_ENTRY(MCA_TX_STAT_DP_FIFO_PKT_MAX_ERR, "STAT:MCA mca_channel[%d]->tx->stats0->dp_fifo_pkt_max_err = 0x%x\n")
    ILOG_ENTRY(MCA_TX_STAT_DP_GRD_NO_SOP_ERR, "STAT:MCA mca_channel[%d]->tx->stats0->dp_grd_no_sop_err = 0x%x\n")
    ILOG_ENTRY(MCA_TX_STAT_DP_GRD_NO_EOP_ERR, "STAT:MCA mca_channel[%d]->tx->stats0->dp_grd_no_eop_err = 0x%x\n")
    ILOG_ENTRY(MCA_TX_STAT_DP_GRD_MAX_ERR, "STAT:MCA mca_channel[%d]->tx->stats0->dp_grd_max_err = 0x%x\n")

    ILOG_ENTRY(MCA_RX_STAT_DP_PFIFO, "STAT:MCA mca_channel[%d]->rx->stats0->dp_pfifo = 0x%x\n")
    ILOG_ENTRY(MCA_RX_STAT_DP_NFIFO, "STAT:MCA mca_channel[%d]->rx->stats0->dp_nfifo = 0x%x\n")
    ILOG_ENTRY(MCA_RX_STAT_DP_FIFO_RD_DRP_PKT, "STAT:MCA mca_channel[%d]->rx->stats0->dp_fifo_rd_drp_pkt = 0x%x\n")
    ILOG_ENTRY(MCA_RX_STAT_DP_FIFO_WR_DRP_PKT, "STAT:MCA mca_channel[%d]->rx->stats0->dp_fifo_wr_drp_pkt = 0x%x\n")
    ILOG_ENTRY(MCA_RX_STAT_DP_FIFO_WR_PKT_ERR, "STAT:MCA mca_channel[%d]->rx->stats0->dp_fifo_wr_pkt_err = 0x%x\n")
    ILOG_ENTRY(MCA_RX_STAT_DP_FIFO_FIFO_FULL_ERR, "STAT:MCA mca_channel[%d]->rx->stats0->dp_fifo_fifo_full_err = 0x%x\n")
    ILOG_ENTRY(MCA_RX_STAT_DP_FIFO_PKT_MAX_ERR, "STAT:MCA mca_channel[%d]->rx->stats0->dp_fifo_pkt_max_err = 0x%x\n")
    ILOG_ENTRY(MCA_RX_STAT_DP_GRD_NO_SOP_ERR, "STAT:MCA mca_channel[%d]->rx->stats0->dp_grd_no_sop_err = 0x%x\n")
    ILOG_ENTRY(MCA_RX_STAT_DP_GRD_NO_EOP_ERR, "STAT:MCA mca_channel[%d]->rx->stats0->dp_grd_no_eop_err = 0x%x\n")
    ILOG_ENTRY(MCA_RX_STAT_DP_GRD_MAX_ERR, "STAT:MCA mca_channel[%d]->rx->stats0->dp_grd_max_err = 0x%x\n")

    ILOG_ENTRY(MCA_CORE_TX_PFIFO, "STAT:MCA mca_core->tx->stats0->pfifo = 0x%x\n")
    ILOG_ENTRY(MCA_CORE_TX_NFIFO, "STAT:MCA mca_core->tx->stats0->nfifo = 0x%x\n")
    ILOG_ENTRY(MCA_CORE_TX_CMD_FIFO_DCOUNT, "STAT:MCA mca_core->tx->stats0->cmd_fifo_dcount = 0x%x\n")
    ILOG_ENTRY(MCA_CORE_TX_LBAD, "STAT:MCA mca_core->tx->stats0->lbad = 0x%x\n")
    ILOG_ENTRY(MCA_CORE_TX_LRTRY, "STAT:MCA mca_core->tx->stats0->lrtry = 0x%x\n")

    ILOG_ENTRY(MCA_CORE_RX_CMD_FIFO_DCOUNT, "STAT:MCA mca_core->rx->stats0->cmd_fifo_dcount = 0x%x\n")
    ILOG_ENTRY(MCA_CORE_RX_LBAD, "STAT:MCA mca_core->rx->stats0->lbad = 0x%x\n")
    ILOG_ENTRY(MCA_CORE_RX_LRTRY, "STAT:MCA mca_core->rx->stats0->lrtry = 0x%x\n")
    ILOG_ENTRY(MCA_CORE_RX_LCRD, "STAT:MCA mca_core->rx->stats0->lcrd = 0x%x\n")

    ILOG_ENTRY(MCA_CORE_IRQ0_ENABLE, "STAT:MCA mca_core->irq0->enable = 0x%x\n")

    ILOG_ENTRY(MCA_INVALID_CHANNEL_INDEX, " 0x%x INVALID CHANNEL INDEX TO MONITOR \n")
    ILOG_ENTRY(MCA_RX_FIFO_FULL_ERROR, "Channel Rx FIFO full ERROR ASSERT !!\n")
    ILOG_ENTRY(MCA_GUARD_ERROR, "Channel GUARD ERROR ASSERT !!\n")
    ILOG_ENTRY(MCA_CORE_GUARD_ERROR, "Core GUARD ERROR ASSERT !!\n")
    ILOG_ENTRY(MCA_CHANNEL_0_LATENCY_VALUE, "Channel 0 latency value = 0x%x\n")
    ILOG_ENTRY(MCA_CHANNEL_0_LATENCY_ERROR, "Timed out waiting for MCA Channel 0 latency value to change from zero\n")

    ILOG_ENTRY(CORE_GUARD_ERROR, "Core Guard Error! IRQ = %d\n")
    ILOG_ENTRY(DEBUG_ASSERT_BB, "CAUSE AN ASSERT IN BB FOR DEBUGGING\n")
    ILOG_ENTRY(MCA_CHANNEL_NP_FIFO, "MCA Channel[%d] dp_pfifo = %04d dp_nfifo = %04d\n")
    ILOG_ENTRY(MCA_BANDWIDTH, "MCA Channel[%d] Bandwidth = %04d\n")
ILOG_END(MCA_COMPONENT, ILOG_MINOR_EVENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // MCA_LOG_H
