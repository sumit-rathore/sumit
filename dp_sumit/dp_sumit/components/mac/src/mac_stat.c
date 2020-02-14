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
#include <module_addresses_regs.h>
#include <stats_mon.h>
#include <mac.h>
#include "mac_log.h"
#include "mac_loc.h"

// Constants and Macros ###########################################################################
#define MAC_STATS_POLL_INTERVAL     (1000/STATISTIC_INTERVAL_TICK)        // poll the MAC stats every second

// Relative, should not change -- if changes - print
#define MAC_STAT_FPGA_REG_NOCHNG_REL(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_RELATIVE_VALUE, MAC_COMPONENT, ilogCode)

// Relative, should not change -- if changes - print istatus
#define MAC_STAT_FPGA_REG_NOCHNG_REL_ISTATUS(fpgaAddress, iStatusCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_RELATIVE_VALUE | STATMON_PARAM_ISTATUS_DISPLAY, MAC_COMPONENT, iStatusCode)

// Relative, should change -- if no changes - print
#define MAC_STAT_FPGA_REG_CHANGE_REL(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_CHANGE | STATMON_PARAM_FLAG_RELATIVE_VALUE, MAC_COMPONENT, ilogCode)

// Absolute, should not change -- if changes - print
#define MAC_STAT_FPGA_REG_NOCHNG_ABS(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, 0, MAC_COMPONENT, ilogCode)

// Aboslute, should change -- if no changes - print
#define MAC_STAT_FPGA_REG_CHANGE_ABS(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_CHANGE, MAC_COMPONENT, ilogCode)

// Macros to get the relative address of the register from the rtl submodule instead of bb_chip
#define MAC_LINK_LAYER_TX_ADDRESS(componentStatAddress) (bb_chip_link_layer_tx_s_ADDRESS + componentStatAddress)
#define MAC_LINK_LAYER_RX_ADDRESS(componentStatAddress) (bb_chip_link_layer_rx_s_ADDRESS + componentStatAddress)
#define MAC_LAYER3_TX_ADDRESS(componentStatAddress) (bb_chip_layer3_tx_s_ADDRESS + componentStatAddress)
#define MAC_LAYER3_RX_ADDRESS(componentStatAddress) (bb_chip_layer3_rx_s_ADDRESS + componentStatAddress)
// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static const StatFpgaReg MacStatU8Regs[] =
{
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_TX_ADDRESS(link_layer_tx_mac_stats0_fsm_count_beyond_ptp_ADDRESS),                                   LL_TX_MAC_STATS0_FSM_BEYOND_PTP_COUNT),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_TX_ADDRESS(link_layer_tx_mac_stats0_fsm_rollover_ADDRESS),                                           LL_TX_MAC_STATS0_FSM_ROLLOVER_COUNT),

    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_TX_ADDRESS(link_layer_tx_media_rs_pfifo_write_engine_stats0_pkt_max_byte_cnt_err_ADDRESS),           LL_TX_MEDIARS_PFIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_TX_ADDRESS(link_layer_tx_media_rs_pfifo_write_engine_stats0_fifo_full_err_ADDRESS),                  LL_TX_MEDIARS_PFIFO_WRENG_STATS0_FIFO_FULL_ERR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_TX_ADDRESS(link_layer_tx_media_rs_pfifo_write_engine_stats0_pkt_err_ADDRESS),                        LL_TX_MEDIARS_PFIFO_WRENG_STATS0_PKT_ERR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_TX_ADDRESS(link_layer_tx_media_rs_pfifo_write_engine_stats0_pkt_sop_err_ADDRESS),                    LL_TX_MEDIARS_PFIFO_WRENG_STATS0_PKT_SOP_ERR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_TX_ADDRESS(link_layer_tx_media_rs_pfifo_write_engine_stats0_drp_pkt_rd_ADDRESS),                     LL_TX_MEDIARS_PFIFO_WRENG_STATS0_DRP_PKT_RD),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_TX_ADDRESS(link_layer_tx_media_rs_pfifo_write_engine_stats0_drp_pkt_wr_ADDRESS),                     LL_TX_MEDIARS_PFIFO_WRENG_STATS0_DRP_PKT_WR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_TX_ADDRESS(link_layer_tx_media_rs_pfifo_read_engine_stats0_drp_pkt_ADDRESS),                         LL_TX_MEDIARS_PFIFO_WRENG_STATS0_DRP_PKT),

    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_stats0_out_fsm_rollover_ADDRESS),                                       LL_RX_MAC_STATS0_OUT_FSM_ROLLOVER_COUNT),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_stats0_in_fsm_rollover_ADDRESS),                                        LL_RX_MAC_STATS0_IN_FSM_ROLLOVER_COUNT),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_client_interface_client_array_stats0_filter_fifo_overflow_ADDRESS),     LL_RX_MCI_STATS0_FILTER_FIFO_OVERFLOW),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_client_interface_client_array_stats0_in_fsm_rollover_ADDRESS),          LL_RX_MCI_STATS0_IN_FSM_ROLLOVER),

    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_pfifo_write_engine_stats0_pkt_max_byte_cnt_err_ADDRESS),                LL_RX_MAC_PFIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_pfifo_write_engine_stats0_fifo_full_err_ADDRESS),                       LL_RX_MAC_PFIFO_WRENG_STATS0_FIFO_FULL_ERR),
//    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_pfifo_write_engine_stats0_pkt_err_ADDRESS),                             LL_RX_MAC_PFIFO_WRENG_STATS0_PKT_ERR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_pfifo_write_engine_stats0_pkt_sop_err_ADDRESS),                         LL_RX_MAC_PFIFO_WRENG_STATS0_PKT_SOP_ERR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_pfifo_write_engine_stats0_drp_pkt_rd_ADDRESS),                          LL_RX_MAC_PFIFO_WRENG_STATS0_DRP_PKT_RD),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_pfifo_write_engine_stats0_drp_pkt_wr_ADDRESS),                          LL_RX_MAC_PFIFO_WRENG_STATS0_DRP_PKT_WR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_pfifo_read_engine_stats0_drp_pkt_ADDRESS),                              LL_RX_MAC_PFIFO_RDENG_STATS0_DRP_PKT),

    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_TX_ADDRESS(layer3_tx_eth_framer_pfifo_write_engine_stats0_pkt_max_byte_cnt_err_ADDRESS),                 L3_TX_ETH_FRAMER_PFIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_TX_ADDRESS(layer3_tx_eth_framer_pfifo_write_engine_stats0_fifo_full_err_ADDRESS),                        L3_TX_ETH_FRAMER_PFIFO_WRENG_STATS0_FIFO_FULL_ERR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_TX_ADDRESS(layer3_tx_eth_framer_pfifo_write_engine_stats0_pkt_err_ADDRESS),                              L3_TX_ETH_FRAMER_PFIFO_WRENG_STATS0_PKT_ERR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_TX_ADDRESS(layer3_tx_eth_framer_pfifo_write_engine_stats0_pkt_sop_err_ADDRESS),                          L3_TX_ETH_FRAMER_PFIFO_WRENG_STATS0_PKT_SOP_ERR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_TX_ADDRESS(layer3_tx_eth_framer_pfifo_write_engine_stats0_drp_pkt_rd_ADDRESS),                           L3_TX_ETH_FRAMER_PFIFO_WRENG_STATS0_DRP_PKT_RD),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_TX_ADDRESS(layer3_tx_eth_framer_pfifo_write_engine_stats0_drp_pkt_wr_ADDRESS),                           L3_TX_ETH_FRAMER_PFIFO_WRENG_STATS0_DRP_PKT_WR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_TX_ADDRESS(layer3_tx_eth_framer_pfifo_read_engine_stats0_drp_pkt_ADDRESS),                               L3_TX_ETH_FRAMER_PFIFO_RDENG_STATS0_DRP_PKT),
};
static uint8_t MacStatStatU8data[ARRAYSIZE(MacStatU8Regs)];

static const StatFpgaReg MacStatU16Regs[] =
{
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_TX_ADDRESS(link_layer_tx_ptp_guard_stats0_missing_sop_err_ADDRESS),                                  LL_RX_PTP_GUARD_STATS0_SOP_ERROR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_TX_ADDRESS(link_layer_tx_ptp_guard_stats0_missing_eop_err_ADDRESS),                                  LL_RX_PTP_GUARD_STATS0_EOP_ERROR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_TX_ADDRESS(link_layer_tx_ptp_guard_stats0_max_cycle_err_ADDRESS),                                    LL_RX_PTP_GUARD_STATS0_MAX_CYCLE_ERROR),

    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_ptp_guard_stats0_missing_sop_err_ADDRESS),                                  LL_TX_PTP_GUARD_STATS0_SOP_ERROR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_ptp_guard_stats0_missing_eop_err_ADDRESS),                                  LL_TX_PTP_GUARD_STATS0_EOP_ERROR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_ptp_guard_stats0_max_cycle_err_ADDRESS),                                    LL_TX_PTP_GUARD_STATS0_MAX_CYCLE_ERROR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_media_rs_stats1_xgmii_data_error_ADDRESS),                                  LL_RX_MEDIARS_STATS1_XGMII_DATA_ERROR_COUNT),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_media_rs_stats1_xgmii_missing_sfd_ADDRESS),                                 LL_RX_MEDIARS_STATS1_XGMII_MISSING_SFD_COUNT),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_media_rs_stats1_xgmii_wrong_lane_start_ADDRESS),                            LL_RX_MEDIARS_STATS1_XGMII_WRONG_LANE_START_COUNT),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_client_interface_client_array_stats0_one_cycle_pkt_ADDRESS),            LL_RX_MCI_STATS0_ONE_CYCLE_PKT),

    // MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_media_rs_stats1_xgmii_link_ok_ADDRESS),                                     LL_RX_MEDIARS_STATS1_XGMII_LINK_OK_COUNT),
    // MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_media_rs_stats1_xgmii_local_fault_ADDRESS),                                 LL_RX_MEDIARS_STATS1_XGMII_LOCAL_FAULT_COUNT),
    // MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_media_rs_stats1_xgmii_remote_fault_ADDRESS),                                LL_RX_MEDIARS_STATS1_XGMII_REMOTE_FAULT_COUNT),
    // MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_media_rs_stats1_xgmii_link_int_ADDRESS),                                    LL_RX_MEDIARS_STATS1_XGMII_LINK_INTERRUPT_COUNT),

    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_TX_ADDRESS(layer3_tx_eth_framer_ptp_guard_stats0_missing_sop_err_ADDRESS),                               L3_TX_ETH_FRAMER_PTP_GUARD_STATS0_SOP_ERROR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_TX_ADDRESS(layer3_tx_eth_framer_ptp_guard_stats0_missing_eop_err_ADDRESS),                               L3_TX_ETH_FRAMER_PTP_GUARD_STATS0_EOP_ERROR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_TX_ADDRESS(layer3_tx_eth_framer_ptp_guard_stats0_max_cycle_err_ADDRESS),                                 L3_TX_ETH_FRAMER_PTP_GUARD_STATS0_MAX_CYCLE_ERROR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_RX_ADDRESS(layer3_rx_eth_de_framer_stats0_non_eth_pkt_ADDRESS),                                          L3_RX_ETH_DE_FRAMER_STATS0_NON_ETH_PKT),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_RX_ADDRESS(layer3_rx_ptp_guard_stats0_missing_sop_err_ADDRESS),                                          L3_RX_PTP_GUARD_STATS0_SOP_ERROR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_RX_ADDRESS(layer3_rx_ptp_guard_stats0_missing_eop_err_ADDRESS),                                          L3_RX_PTP_GUARD_STATS0_EOP_ERROR),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LAYER3_RX_ADDRESS(layer3_rx_ptp_guard_stats0_max_cycle_err_ADDRESS),                                            L3_RX_PTP_GUARD_STATS0_MAX_CYCLE_ERROR),
};
static uint16_t MacStatStatU16data[ARRAYSIZE(MacStatU16Regs)];

static const StatFpgaReg MacStatU32Regs[] =
{
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_TX_ADDRESS(link_layer_tx_mac_stats0_pause_frame_ADDRESS),          LL_TX_MAC_STATS0_PAUSE_FRAME_COUNT),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_stats0_pause_frame_ADDRESS),          LL_RX_MAC_STATS0_PAUSE_FRAME_COUNT),
    MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_stats0_dbg_seq_num_in_err_ADDRESS),   LL_RX_MAC_STATS0_DBG_SEQ_NUM_IN_ERR),
//    MAC_STAT_FPGA_REG_NOCHNG_REL_ISTATUS( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_mac_stats0_crc_err_ADDRESS),      ISTATUS_MAC_CRC_ERROR),

    // MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_media_rs_stats1_xgmii_out_eop_ADDRESS),                                     LL_RX_MEDIARS_STATS1_XGMII_OUT_EOP_COUNT),
    // MAC_STAT_FPGA_REG_NOCHNG_REL( MAC_LINK_LAYER_RX_ADDRESS(link_layer_rx_media_rs_stats1_xgmii_out_sop_ADDRESS),                                     LL_RX_MEDIARS_STATS1_XGMII_OUT_SOP_COUNT),
};

static void _MAC_setReadClearStats(void);
static uint32_t MacStatStatU32data[ARRAYSIZE(MacStatU32Regs)];

const StatRegistration Mac8bitStats = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_8_BITS,
        MacStatU8Regs,
        MacStatStatU8data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MAC_STATS_POLL_INTERVAL);

const StatRegistration Mac16bitsStats = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_16_BITS,
        MacStatU16Regs,
        MacStatStatU16data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MAC_STATS_POLL_INTERVAL);

const StatRegistration Mac32bitsStats = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_32_BITS,
        MacStatU32Regs,
        MacStatStatU32data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MAC_STATS_POLL_INTERVAL);


// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// MAC stat initialization function
//
// Parameters:
//
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################

void MAC_StatInit(void)
{
    STATSMON_RegisterStatgroup(&Mac8bitStats);
    STATSMON_RegisterStatgroup(&Mac16bitsStats);
    STATSMON_RegisterStatgroup(&Mac32bitsStats);
}
//#################################################################################################
// Start the link layer stats monitor timer, which will periodically check error stats and log them
// when they change.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void MAC_StartStatsMonitor(void)
{
    // set the rd2clr_config register so it will automatically clear the stat when read
    _MAC_setReadClearStats();
    STATSMON_StatgroupControl(&Mac8bitStats,   true);
    STATSMON_StatgroupControl(&Mac16bitsStats, true);
    STATSMON_StatgroupControl(&Mac32bitsStats, true);
}

//#################################################################################################
// Stop the link layer stats monitor timer.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void MAC_StopStatsMonitor(void)
{
    STATSMON_StatgroupControl(&Mac8bitStats,   false);
    STATSMON_StatgroupControl(&Mac16bitsStats, false);
    STATSMON_StatgroupControl(&Mac32bitsStats, false);
}


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################

//#################################################################################################
// sets the rd2clr_config registers so it will automatically clear the stat when read
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void _MAC_setReadClearStats(void)
{
    linkLayerTx->mac.s.stats0.s.rd2clr_config.dw =
        link_layer_tx_mac_stats0_rd2clr_config_WRITEMASK;
    linkLayerTx->media_rs.s.pfifo.s.write_engine.s.stats0.s.rd2clr_config.dw =
        link_layer_tx_media_rs_pfifo_write_engine_stats0_rd2clr_config_WRITEMASK;
    linkLayerTx->media_rs.s.pfifo.s.read_engine.s.stats0.s.rd2clr_config.dw =
        link_layer_tx_media_rs_pfifo_read_engine_stats0_rd2clr_config_WRITEMASK;
    linkLayerTx->media_rs.s.stats0.s.rd2clr_config.dw =
        link_layer_tx_media_rs_stats0_rd2clr_config_WRITEMASK;
    linkLayerTx->media_rs.s.stats1.s.rd2clr_config.dw =
        link_layer_tx_media_rs_stats1_rd2clr_config_WRITEMASK;
    linkLayerTx->media_rs.s.stats2.s.rd2clr_config.dw =
        link_layer_tx_media_rs_stats2_rd2clr_config_WRITEMASK;
    linkLayerTx->ptp_guard.s.stats0.s.rd2clr_config.dw =
        link_layer_tx_ptp_guard_stats0_rd2clr_config_WRITEMASK;

    linkLayerRx->mac_client_interface.s.client_array.s.stats0.s.rd2clr_config.dw =
        link_layer_rx_mac_client_interface_client_array_stats0_rd2clr_config_WRITEMASK;
    linkLayerRx->mac.s.pfifo.s.write_engine.s.stats0.s.rd2clr_config.dw =
        link_layer_rx_mac_pfifo_write_engine_stats0_rd2clr_config_WRITEMASK;
    linkLayerRx->mac.s.pfifo.s.read_engine.s.stats0.s.rd2clr_config.dw =
        link_layer_rx_mac_pfifo_read_engine_stats0_rd2clr_config_WRITEMASK;
    linkLayerRx->mac.s.stats0.s.rd2clr_config.dw =
        link_layer_rx_mac_stats0_rd2clr_config_WRITEMASK;
    linkLayerRx->media_rs.s.stats0.s.rd2clr_config.dw =
        link_layer_rx_media_rs_stats0_rd2clr_config_WRITEMASK;
    linkLayerRx->media_rs.s.stats1.s.rd2clr_config.dw =
        link_layer_rx_media_rs_stats1_rd2clr_config_WRITEMASK;
    linkLayerRx->ptp_guard.s.stats0.s.rd2clr_config.dw =
        link_layer_rx_ptp_guard_stats0_rd2clr_config_WRITEMASK;
    linkLayerRx->self_test_support.s.media_analyzer.s.stats0.s.rd2clr_config.dw =
        link_layer_rx_self_test_support_media_analyzer_stats0_rd2clr_config_WRITEMASK;

    layer3Tx->eth_framer.s.stats0.s.rd2clr_config.dw =
        layer3_tx_eth_framer_stats0_rd2clr_config_WRITEMASK;
    layer3Tx->eth_framer.s.pfifo.s.write_engine.s.stats0.s.rd2clr_config.dw =
        layer3_tx_eth_framer_pfifo_write_engine_stats0_rd2clr_config_WRITEMASK;
    layer3Tx->eth_framer.s.pfifo.s.read_engine.s.stats0.s.rd2clr_config.dw =
        layer3_tx_eth_framer_pfifo_read_engine_stats0_rd2clr_config_WRITEMASK;
    layer3Tx->eth_framer.s.ptp_guard.s.stats0.s.rd2clr_config.dw =
        layer3_tx_eth_framer_ptp_guard_stats0_rd2clr_config_WRITEMASK;

    layer3Rx->eth_de_framer.s.stats0.s.rd2clr_config.dw =
        layer3_rx_eth_de_framer_stats0_rd2clr_config_WRITEMASK;
    layer3Rx->ptp_guard.s.stats0.s.rd2clr_config.dw =
        layer3_rx_ptp_guard_stats0_rd2clr_config_WRITEMASK;
}
