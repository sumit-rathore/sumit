//#################################################################################################
// Icron Technology Corporation - Copyright 2017
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef MAC_LOG_H
#define MAC_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(MAC_COMPONENT)
    ILOG_ENTRY(LL_TX_MAC_ENABLED,                                         "MAC Tx Enabled\n")
    ILOG_ENTRY(LL_RX_MAC_ENABLED,                                         "MAC Rx Enabled\n")
    ILOG_ENTRY(LL_TX_MAC_DISABLED,                                        "MAC Tx Disabled\n")
    ILOG_ENTRY(LL_RX_MAC_DISABLED,                                        "MAC Rx Disabled\n")
    ILOG_ENTRY(LL_TX_LINK_LAYER3_ENABLED,                                 "Link Layer 3 Tx Enabled\n")
    ILOG_ENTRY(LL_RX_LINK_LAYER3_ENABLED,                                 "Link Layer 3 Rx Enabled\n")
    ILOG_ENTRY(LL_TX_LINK_LAYER3_DISABLED,                                "Link Layer 3 Tx Disabled\n")
    ILOG_ENTRY(LL_RX_LINK_LAYER3_DISABLED,                                "Link Layer 3 Rx Disabled\n")

    ILOG_ENTRY(LL_RX_LINK_LAYER_IRQ,                                      "Link Layer Rx IRQ pending:%x link ok:%d remote fault:%d\n")

    ILOG_ENTRY(LL_TX_MAC_STATS0_PAUSE_FRAME_COUNT,                        "STAT:MAC link_layer_tx->mac->stats0->pause_frame: %d\n")
    ILOG_ENTRY(LL_TX_MAC_STATS0_FSM_BEYOND_PTP_COUNT,                     "STAT:MAC link_layer_tx->mac->stats0->fsm_count_beyond_ptp: %d\n")
    ILOG_ENTRY(LL_TX_MAC_STATS0_FSM_ROLLOVER_COUNT,                       "STAT:MAC link_layer_tx->mac->stats0->fsm_rollover: %d\n")
    ILOG_ENTRY(LL_TX_MEDIARS_PFIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR,     "STAT:MAC link_layer_tx->media_rs->pfifo->write_engine->stats0->pkt_max_byte_cnt_err: %d\n")
    ILOG_ENTRY(LL_TX_MEDIARS_PFIFO_WRENG_STATS0_FIFO_FULL_ERR,            "STAT:MAC link_layer_tx->media_rs->pfifo->write_engine->stats0->fifo_full_err: %d\n")
    ILOG_ENTRY(LL_TX_MEDIARS_PFIFO_WRENG_STATS0_PKT_ERR,                  "STAT:MAC link_layer_tx->media_rs->pfifo->write_engine->stats0->pkt_err: %d\n")
    ILOG_ENTRY(LL_TX_MEDIARS_PFIFO_WRENG_STATS0_PKT_SOP_ERR,              "STAT:MAC link_layer_tx->media_rs->pfifo->write_engine->stats0->pkt_sop_err: %d\n")
    ILOG_ENTRY(LL_TX_MEDIARS_PFIFO_WRENG_STATS0_DRP_PKT_RD,               "STAT:MAC link_layer_tx->media_rs->pfifo->write_engine->stats0->drp_pkt_rd: %d\n")
    ILOG_ENTRY(LL_TX_MEDIARS_PFIFO_WRENG_STATS0_DRP_PKT_WR,               "STAT:MAC link_layer_tx->media_rs->pfifo->write_engine->stats0->drp_pkt_wr: %d\n")
    ILOG_ENTRY(LL_TX_MEDIARS_PFIFO_WRENG_STATS0_DRP_PKT,                  "STAT:MAC link_layer_tx->media_rs->pfifo->read_engine->stats0->drp_pkt: %d\n")
    ILOG_ENTRY(LL_TX_PTP_GUARD_STATS0_SOP_ERROR,                          "STAT:MAC link_layer_tx->ptp_guard->stats0->missing_sop_err: %d\n")
    ILOG_ENTRY(LL_TX_PTP_GUARD_STATS0_EOP_ERROR,                          "STAT:MAC link_layer_tx->ptp_guard->stats0->missing_eop_err:  %d\n")
    ILOG_ENTRY(LL_TX_PTP_GUARD_STATS0_MAX_CYCLE_ERROR,                    "STAT:MAC link_layer_tx->ptp_guard->stats0->max_cycle_err: %d\n")

    ILOG_ENTRY(LL_RX_MEDIARS_STATS1_XGMII_DATA_ERROR_COUNT,               "STAT:MAC link_layer_rx->media_rs->stats1->xgmii_data_error: %d\n")
    ILOG_ENTRY(LL_RX_MEDIARS_STATS1_XGMII_MISSING_SFD_COUNT,              "STAT:MAC link_layer_rx->media_rs->stats1->xgmii_missing_sfd: %d\n")
    ILOG_ENTRY(LL_RX_MEDIARS_STATS1_XGMII_WRONG_LANE_START_COUNT,         "STAT:MAC link_layer_rx->media_rs->stats1->xgmii_wrong_lane_start: %d\n")
    // ILOG_ENTRY(LL_RX_MEDIARS_STATS1_XGMII_OUT_EOP_COUNT,                  "STAT:MAC link_layer_rx->media_rs->stats1->xgmii_out_eop: %d\n")
    // ILOG_ENTRY(LL_RX_MEDIARS_STATS1_XGMII_OUT_SOP_COUNT,                  "STAT:MAC link_layer_rx->media_rs->stats1->xgmii_out_sop: %d\n")
    // ILOG_ENTRY(LL_RX_MEDIARS_STATS1_XGMII_LINK_OK_COUNT,                  "STAT:MAC link_layer_rx->media_rs->stats1->xgmii_link_ok: %d\n")
    // ILOG_ENTRY(LL_RX_MEDIARS_STATS1_XGMII_LOCAL_FAULT_COUNT,              "STAT:MAC link_layer_rx->media_rs->stats1->xgmii_local_fault: %d\n")
    // ILOG_ENTRY(LL_RX_MEDIARS_STATS1_XGMII_REMOTE_FAULT_COUNT,             "STAT:MAC link_layer_rx->media_rs->stats1->xgmii_remote_fault: %d\n")
    // ILOG_ENTRY(LL_RX_MEDIARS_STATS1_XGMII_LINK_INTERRUPT_COUNT,           "STAT:MAC link_layer_rx->media_rs->stats1->xgmii_link_int: %d\n")
    ILOG_ENTRY(LL_RX_MAC_STATS0_PAUSE_FRAME_COUNT,                        "STAT:MAC link_layer_rx->mac->stats0->pause_frame: %d\n")
    ILOG_ENTRY(LL_RX_MAC_STATS0_OUT_FSM_ROLLOVER_COUNT,                   "STAT:MAC link_layer_rx->mac->stats0->out_fsm_rollover: %d\n")
    ILOG_ENTRY(LL_RX_MAC_STATS0_IN_FSM_ROLLOVER_COUNT,                    "STAT:MAC link_layer_rx->mac->stats0->in_fsm_rollover: %d\n")
    ILOG_ENTRY(LL_RX_MAC_STATS0_DBG_SEQ_NUM_IN_ERR,                       "STAT:MAC link_layer_rx->mac->stats0->dbg_seq_num_in_err: %d\n")
    ILOG_ENTRY(LL_RX_MCI_STATS0_FILTER_FIFO_OVERFLOW,                     "STAT:MAC link_layer_rx->mac_client_interface->client_array->stats0->filter_fifo_overflow: %d\n")
    ILOG_ENTRY(LL_RX_MCI_STATS0_ONE_CYCLE_PKT,                            "STAT:MAC link_layer_rx->mac_client_interface->client_array->stats0->one_cycle_pkt: %d\n")
    ILOG_ENTRY(LL_RX_MCI_STATS0_IN_FSM_ROLLOVER,                          "STAT:MAC link_layer_rx->mac_client_interface->client_array->stats0->in_fsm_rollover: %d\n")
    ILOG_ENTRY(LL_RX_MAC_PFIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR,         "STAT:MAC link_layer_rx->mac->pfifo->write_engine->stats0->pkt_max_byte_cnt_err: %d\n")
    ILOG_ENTRY(LL_RX_MAC_PFIFO_WRENG_STATS0_FIFO_FULL_ERR,                "STAT:MAC link_layer_rx->mac->pfifo->write_engine->stats0->fifo_full_err: %d\n")
    ILOG_ENTRY(LL_RX_MAC_PFIFO_WRENG_STATS0_PKT_ERR,                      "STAT:MAC link_layer_rx->mac->pfifo->write_engine->stats0->pkt_err: %d\n")
    ILOG_ENTRY(LL_RX_MAC_PFIFO_WRENG_STATS0_PKT_SOP_ERR,                  "STAT:MAC link_layer_rx->mac->pfifo->write_engine->stats0->pkt_sop_err: %d\n")
    ILOG_ENTRY(LL_RX_MAC_PFIFO_WRENG_STATS0_DRP_PKT_RD,                   "STAT:MAC link_layer_rx->mac->pfifo->write_engine->stats0->drp_pkt_rd: %d\n")
    ILOG_ENTRY(LL_RX_MAC_PFIFO_WRENG_STATS0_DRP_PKT_WR,                   "STAT:MAC link_layer_rx->mac->pfifo->write_engine->stats0->drp_pkt_wr: %d\n")
    ILOG_ENTRY(LL_RX_MAC_PFIFO_RDENG_STATS0_DRP_PKT,                      "STAT:MAC link_layer_rx->mac->pfifo->read_engine->stats0->drp_pkt: %d\n")
    ILOG_ENTRY(LL_RX_PTP_GUARD_STATS0_SOP_ERROR,                          "STAT:MAC link_layer_rx->ptp_guard->stats0->missing_sop_err: %d\n")
    ILOG_ENTRY(LL_RX_PTP_GUARD_STATS0_EOP_ERROR,                          "STAT:MAC link_layer_rx->ptp_guard->stats0->missing_eop_err:  %d\n")
    ILOG_ENTRY(LL_RX_PTP_GUARD_STATS0_MAX_CYCLE_ERROR,                    "STAT:MAC link_layer_rx->ptp_guard->stats0->max_cycle_err: %d\n")
    ILOG_ENTRY(L3_TX_ETH_FRAMER_PFIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR,  "STAT:MAC layer3_tx->eth_framer->pfifo->write_engine->stats0->pkt_max_byte_cnt_err: %d\n")
    ILOG_ENTRY(L3_TX_ETH_FRAMER_PFIFO_WRENG_STATS0_FIFO_FULL_ERR,         "STAT:MAC layer3_tx->eth_framer->pfifo->write_engine->stats0->fifo_full_err: %d\n")
    ILOG_ENTRY(L3_TX_ETH_FRAMER_PFIFO_WRENG_STATS0_PKT_ERR,               "STAT:MAC layer3_tx->eth_framer->pfifo->write_engine->stats0->pkt_err: %d\n")
    ILOG_ENTRY(L3_TX_ETH_FRAMER_PFIFO_WRENG_STATS0_PKT_SOP_ERR,           "STAT:MAC layer3_tx->eth_framer->pfifo->write_engine->stats0->pkt_sop_err: %d\n")
    ILOG_ENTRY(L3_TX_ETH_FRAMER_PFIFO_WRENG_STATS0_DRP_PKT_RD,            "STAT:MAC layer3_tx->eth_framer->pfifo->write_engine->stats0->drp_pkt_rd: %d\n")
    ILOG_ENTRY(L3_TX_ETH_FRAMER_PFIFO_WRENG_STATS0_DRP_PKT_WR,            "STAT:MAC layer3_tx->eth_framer->pfifo->write_engine->stats0->drp_pkt_wr: %d\n")
    ILOG_ENTRY(L3_TX_ETH_FRAMER_PFIFO_RDENG_STATS0_DRP_PKT,               "STAT:MAC layer3_tx->eth_framer->pfifo->read_engine->Stats0->drp_pkt: %d\n")
    ILOG_ENTRY(L3_TX_ETH_FRAMER_PTP_GUARD_STATS0_SOP_ERROR,               "STAT:MAC layer3_tx->eth_framer->ptp_guard->stats0->missing_sop_err: %d\n")
    ILOG_ENTRY(L3_TX_ETH_FRAMER_PTP_GUARD_STATS0_EOP_ERROR,               "STAT:MAC layer3_tx->eth_framer->ptp_guard->stats0->missing_eop_err:  %d\n")
    ILOG_ENTRY(L3_TX_ETH_FRAMER_PTP_GUARD_STATS0_MAX_CYCLE_ERROR,         "STAT:MAC layer3_tx->eth_framer->ptp_guard->stats0->max_cycle_err: %d\n")
    ILOG_ENTRY(L3_RX_ETH_DE_FRAMER_STATS0_NON_ETH_PKT,                    "STAT:MAC layer3_rx->eth_de_framer->stats0->non_eth_pkt: %d\n")
    ILOG_ENTRY(L3_RX_PTP_GUARD_STATS0_SOP_ERROR,                          "STAT:MAC layer3_rx->ptp_guard->stats0->missing_sop_err: %d\n")
    ILOG_ENTRY(L3_RX_PTP_GUARD_STATS0_EOP_ERROR,                          "STAT:MAC layer3_rx->ptp_guard->stats0->missing_eop_err:  %d\n")
    ILOG_ENTRY(L3_RX_PTP_GUARD_STATS0_MAX_CYCLE_ERROR,                    "STAT:MAC layer3_rx->ptp_guard->stats0->max_cycle_err: %d\n")

ILOG_END(MAC_COMPONENT, ILOG_MINOR_EVENT)

#endif // MAC_LOG_H

