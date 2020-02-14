//#################################################################################################
// Icron Technology Corporation - Copyright 2016
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
// This file contains common ULP (USB Link Protocol) code for the Lex and Rex.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// The current design expects a VBus detection on the fly to enable USB3 and USB2. The normal
// sequence of ULP events lists as follow:
//      * LEX detects VBus enable from vbus_det interrupt.
//      * LEX sends a VBus detected CPU message to REX.
//      * REX sets up USB3. Once it is done, notify LEX through a CPU message.
//      * LEX sets up USB3 and send REX a CPU message to enable GE at REX.
//      * REX brings GE out of reset and send LEX a CPU message.
//      * LEX enables GE.
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <ulp_core_regs.h>
#include <ulp_phy_regs.h>
#include <xusb3_regs.h>
#include <bb_top.h>
#include <ulp.h>
#include "ulp_log.h"
#include "ulp_loc.h"
#include <stats_mon.h>
#include <module_addresses_regs.h>
// #include <uart.h>

// Constants and Macros ###########################################################################
// Macros to get the relative address of the register from the rtl submodule instead of bb_chip
#define ULP_CORE_ADDRESS(componentStatAddress) (bb_chip_ulp_core_s_ADDRESS + componentStatAddress)
#define ULP_PHY_ADDRESS(componentStatAddress) (bb_chip_ulp_phy_s_ADDRESS + componentStatAddress)
#define ULP_XUSB_ADDRESS(componentStatAddress) (bb_chip_xusb3_s_ADDRESS + componentStatAddress)

#define ULP_STATS_POLL_INTERVAL     (1000/STATISTIC_INTERVAL_TICK)        // poll the ULP stats every second

// Relative, should not change -- if changes - print
#define ULP_STAT_FPGA_REG_NOCHNG_REL(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_RELATIVE_VALUE, ULP_COMPONENT, ilogCode)

// Relative, should not change -- if changes - print iStatus
#define ULP_STAT_FPGA_REG_NOCHNG_REL_ISTATUS(fpgaAddress, iStatusCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_RELATIVE_VALUE | STATMON_PARAM_ISTATUS_DISPLAY, ULP_COMPONENT, iStatusCode)

// Relative, should change -- if no changes - print
#define ULP_STAT_FPGA_REG_CHANGE_REL(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_CHANGE | STATMON_PARAM_FLAG_RELATIVE_VALUE, ULP_COMPONENT, ilogCode)

// Absolute, should not change -- if changes - print
#define ULP_STAT_FPGA_REG_NOCHNG_ABS(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, 0, ULP_COMPONENT, ilogCode)

// Aboslute, should change -- if no changes - print
#define ULP_STAT_FPGA_REG_CHANGE_ABS(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_CHANGE, ULP_COMPONENT, ilogCode)

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Static Variables ###############################################################################
/*
static struct UlpMainContext
{
} ulpContext;
*/
static const StatFpgaReg UlpCoreStatU8Regs[] =
{
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_tx_lfps_cnt_in_err_ADDRESS),                                 ULP_CORE_STATS_TX_LFPS_CNT_IN_ERR),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_rx_lfps_cnt_in_err_ADDRESS),                                 ULP_CORE_STATS_RX_LFPS_CNT_IN_ERR),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_tx_framer_ptp_violated_ADDRESS),                             ULP_CORE_STATS_TX_FRAMER_PTP_VIOLATED),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_link_training_ptp_violated_ADDRESS),                         ULP_CORE_STATS_LINK_TRAINING_PTP_VIOLATED),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_link_command_ptp_violated_ADDRESS),                          ULP_CORE_STATS_LINK_CMD_PTP_VIOLATED),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_lmp_ptp_violated_ADDRESS),                                   ULP_CORE_STATS_LMP_PTP_VIOLATED),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_stats0_link_partner_ptp_violated_ADDRESS),                             XUSB3_STATS_LINK_PARTNER_PTP_VIOLATED),
//    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_stats0_not_rdy_pkt_drp_ADDRESS),                                       XUSB3_STATS_NOT_READY_PACKET_DROPPED),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_link_command_rx_event_fifo_overflow_ADDRESS),                ULP_CORE_STATS_LINK_CMD_RX_EVENT_FIFO_OVERFLOW),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_link_command_rx_event_fifo_underflow_ADDRESS),               ULP_CORE_STATS_LINK_CMD_RX_EVENT_FIFO_UNDERFLOW),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_lmp_rx_event_fifo_overflow_ADDRESS),                         ULP_CORE_STATS_LMP_RX_EVENT_FIFO_OVERFLOW),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_lmp_rx_event_fifo_underflow_ADDRESS),                        ULP_CORE_STATS_LMP_RX_EVENT_FIFO_UNDERFLOW),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_remote_rx_hdr_buff_crdt_cnt_in_err_ADDRESS),                 ULP_CORE_STATS_REMOTE_RX_HDR_BUFF_CRDT_IN),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_rx_partner_fifo_write_engine_stats0_pkt_max_byte_cnt_err_ADDRESS),     XUSB3_RX_PFIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_rx_partner_fifo_write_engine_stats0_fifo_full_err_ADDRESS),            XUSB3_RX_PFIFO_WRENG_STATS0_FIFO_FULL_ERR),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_rx_partner_fifo_write_engine_stats0_pkt_err_ADDRESS),                  XUSB3_RX_PFIFO_WRENG_STATS0_PKT_ERR),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_rx_partner_fifo_write_engine_stats0_pkt_sop_err_ADDRESS),              XUSB3_RX_PFIFO_WRENG_STATS0_PKT_SOP_ERR),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_rx_partner_fifo_write_engine_stats0_drp_pkt_rd_ADDRESS),               XUSB3_RX_PFIFO_WRENG_STATS0_DRP_PKT_RD),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_rx_partner_fifo_write_engine_stats0_drp_pkt_wr_ADDRESS),               XUSB3_RX_PFIFO_WRENG_STATS0_DRP_PKT_WR),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_rx_partner_fifo_read_engine_stats0_drp_pkt_ADDRESS),                   XUSB3_RX_PFIFO_RDENG_STATS0_DRP_PKT),

};

static uint8_t UlpCoreStatU8data[ARRAYSIZE(UlpCoreStatU8Regs)];

static const StatFpgaReg UlpCoreStatU16Regs[] =
{
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_go2_u0_from_recovery_ADDRESS),   ULP_CORE_STATS_GO2_U0_FROM_RECOVERY),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_rx_go_recovery_ADDRESS),         ULP_CORE_STATS_RX_GO_RECOVERY),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_snt_lrty_ADDRESS),               ULP_CORE_STATS_SENT_LRTY),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_snt_lbad_ADDRESS),               ULP_CORE_STATS_SENT_LBAD),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_rcvd_lrty_ADDRESS),              ULP_CORE_STATS_RCVD_LRTY),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_stats0_rcvd_lbad_ADDRESS),              ULP_CORE_STATS_RCVD_LBAD),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_stats0_unknown_pkt_drp_ADDRESS),           XUSB3_STATS_UNKNOWN_PKT_DRP),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_stats0_dwn_stream_busy_drp_ADDRESS),       XUSB3_STATS_DWN_STREAM_BUSY_DRP),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_stats0_drop_lone_dpp_ADDRESS),             XUSB3_STATS_DROP_LONE_DPP),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_stats0_rcvd_lone_dph_ADDRESS),             XUSB3_STATS_RCVD_LONE_DPH),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_stats0_timedout_2join_dpp_ADDRESS),        XUSB3_STATS_TIMEDOUT_2JOIN_DPP),


    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_ptp_guard_2core_stats0_missing_sop_err_ADDRESS),       XUSB3_PTPGUARD_2CORE_MISSING_SOP_ERR),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_ptp_guard_2core_stats0_missing_eop_err_ADDRESS),       XUSB3_PTPGUARD_2CORE_MISSING_EOP_ERR),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_XUSB_ADDRESS(xusb3_ptp_guard_2core_stats0_max_cycle_err_ADDRESS),         XUSB3_PTPGUARD_2CORE_MAX_CYCLE_ERR),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_ptp_guard_2phy_stats0_missing_sop_err_ADDRESS),     ULP_CORE_PTPGUARD_2PHY_MISSING_SOP_ERR),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_ptp_guard_2phy_stats0_missing_eop_err_ADDRESS),     ULP_CORE_PTPGUARD_2PHY_MISSING_EOP_ERR),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_ptp_guard_2phy_stats0_max_cycle_err_ADDRESS),       ULP_CORE_PTPGUARD_2PHY_MAX_CYCLE_ERR),
// TODO: These stats are only available in the ISO build
//    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_ptp_guard_2core_stats0_missing_sop_err_ADDRESS),     ULP_CORE_PTPGUARD_2CORE_MISSING_SOP_ERR),
//    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_ptp_guard_2core_stats0_missing_eop_err_ADDRESS),     ULP_CORE_PTPGUARD_2CORE_MISSING_EOP_ERR),
//    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_CORE_ADDRESS(ulp_core_ptp_guard_2core_stats0_max_cycle_err_ADDRESS),       ULP_CORE_PTPGUARD_2CORE_MAX_CYCLE_ERR),
};

static uint16_t UlpCoreStatU16data[ARRAYSIZE(UlpCoreStatU16Regs)];

static const StatFpgaReg UlpPhyStat0U16Regs[] =
{
    ULP_STAT_FPGA_REG_NOCHNG_REL_ISTATUS( ULP_PHY_ADDRESS(ulp_phy_stats0_rx_8b10b_err_ADDRESS),        ISTATUS_ULP_8B_10B_DECODER_ERROR),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_PHY_ADDRESS(ulp_phy_stats0_rx_elastic_buff_overflow_ADDRESS),    ULP_PHY_STATS0_RX_ELASTIC_BUFF_OVERFLOW),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_PHY_ADDRESS(ulp_phy_stats0_rx_elastic_buff_underflow_ADDRESS),   ULP_PHY_STATS0_RX_ELASTIC_BUFF_UNDERFLOW),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_PHY_ADDRESS(ulp_phy_stats0_rx_disparity_err_ADDRESS),            ULP_PHY_STATS0_RX_DISPARITY_ERR),
};

static uint16_t UlpPhyStat0U16data[ARRAYSIZE(UlpPhyStat0U16Regs)];

static const StatFpgaReg UlpPhyStat1U32Regs[] =
{
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_PHY_ADDRESS(ulp_phy_stats1_skp_insert_in_err_ADDRESS),           ULP_PHY_STATS1_SKIP_INSERT_IN_ERR),
    ULP_STAT_FPGA_REG_NOCHNG_REL_ISTATUS( ULP_PHY_ADDRESS(ulp_phy_stats1_crc5_chk_failed_ADDRESS),     ISTATUS_ULP_CRC5_CHK_FAILED),
    ULP_STAT_FPGA_REG_NOCHNG_REL_ISTATUS( ULP_PHY_ADDRESS(ulp_phy_stats1_crc16_chk_failed_ADDRESS),    ISTATUS_ULP_CRC16_CHK_FAILED),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_PHY_ADDRESS(ulp_phy_stats1_dpp_abort_ADDRESS),                   ULP_PHY_STATS1_DPP_ABORT),
    ULP_STAT_FPGA_REG_NOCHNG_REL( ULP_PHY_ADDRESS(ulp_phy_stats1_rx_framer_ptp_violated_ADDRESS),      ULP_PHY_STATS1_RX_FRAMER_PTP_VIOLATED),
};

static uint32_t UlpPhyStat1U32data[ARRAYSIZE(UlpPhyStat1U32Regs)];

const StatRegistration UlpCore8Stats = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_8_BITS,
        UlpCoreStatU8Regs,
        UlpCoreStatU8data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        ULP_STATS_POLL_INTERVAL);

const StatRegistration UlpCore16Stats = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_16_BITS,
        UlpCoreStatU16Regs,
        UlpCoreStatU16data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        ULP_STATS_POLL_INTERVAL);

const StatRegistration UlpPhyStat016Stats = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_16_BITS,
        UlpPhyStat0U16Regs,
        UlpPhyStat0U16data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        ULP_STATS_POLL_INTERVAL);

const StatRegistration UlpPhyStat132Stats = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_32_BITS,
        UlpPhyStat1U32Regs,
        UlpPhyStat1U32data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        ULP_STATS_POLL_INTERVAL);


// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// Ulp stat initialization function
//
// Parameters:
//
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################

void ULP_StatInit(void)
{
    STATSMON_RegisterStatgroup(&UlpCore8Stats);
    STATSMON_RegisterStatgroup(&UlpCore16Stats);
    STATSMON_RegisterStatgroup(&UlpPhyStat016Stats);
    STATSMON_RegisterStatgroup(&UlpPhyStat132Stats);
}

//#################################################################################################
// Enables or disables monitoring of the ULP statistics
//
// Parameters:
// Return:
// Assumptions: Reads a byte from address 0xC0B00000
//#################################################################################################
void ulp_StatMonCoreControl(bool enable)
{
    // set the rd2clr_config register so it will automatically clear the stat when read
    _ULP_coreHalSetReadClearStats();
    STATSMON_StatgroupControl(&UlpCore8Stats,       enable);
    STATSMON_StatgroupControl(&UlpCore16Stats,      enable);
}

//#################################################################################################
// Enables or disables monitoring of the ULP statistics
//
// Parameters:
// Return:
// Assumptions: Reads a byte from address 0xC0B00000
//#################################################################################################
void ulp_StatMonPhyControl(bool enable)
{
    // set the rd2clr_config register so it will automatically clear the stat when read
    _ULP_phyHalSetReadClearStats();
    STATSMON_StatgroupControl(&UlpPhyStat016Stats,  enable);
    STATSMON_StatgroupControl(&UlpPhyStat132Stats,  enable);
}


// Static Function Definitions ####################################################################
