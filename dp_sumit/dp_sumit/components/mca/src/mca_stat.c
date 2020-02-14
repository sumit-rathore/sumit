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

//#################################################################################################
// Module Description
//#################################################################################################
// Implementations of monitoring MCA status.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <bb_chip_regs.h>
#include <stats_mon.h>
#include <mca.h>
#include "mca_log.h"
#include "mca_loc.h"
#include <mca_core_regs.h>
#include <mca_channel_regs.h>
// #include <uart.h>  // for debugging
// Constants and Macros ###########################################################################

// Macros to get the relative address of the register from the rtl submodule instead of bb_chip
#define MCA_CORE_ADDRESS(componentStatAddress) (bb_chip_mca_core_s_ADDRESS + componentStatAddress)
#define MCA_CHANNEL_ADDRESS(componentStatAddress) (bb_chip_mca_channel_s_ADDRESS + componentStatAddress)

#define MCA_STATS_POLL_INTERVAL     (1000/STATISTIC_INTERVAL_TICK)        // poll the MAC stats every second
#define CHANNEL_OFFSET              0x100

// Relative, should not change -- if changes - print
#define MCA_STAT_FPGA_REG_NOCHNG_REL(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_RELATIVE_VALUE, MCA_COMPONENT, ilogCode)

// Relative, should not change with channel information -- if changes - print
#define MCA_STAT_FPGA_REG_NOCHNG_REL_CH(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_RELATIVE_VALUE | STATMON_PARAM_INDEX_DISPLAY, MCA_COMPONENT, ilogCode)

// Relative, should change -- if no changes - print
#define MCA_STAT_FPGA_REG_CHANGE_REL(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_CHANGE | STATMON_PARAM_FLAG_RELATIVE_VALUE, MCA_COMPONENT, ilogCode)

// Relative, should change with channel information -- if no changes - print
#define MCA_STAT_FPGA_REG_CHANGE_REL_CH(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, \
        STATMON_PARAM_FLAG_CHANGE | STATMON_PARAM_FLAG_RELATIVE_VALUE | STATMON_PARAM_INDEX_DISPLAY, \
        MCA_COMPONENT, ilogCode)

// Absolute, should not change -- if changes - print
#define MCA_STAT_FPGA_REG_NOCHNG_ABS(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, 0, MCA_COMPONENT, ilogCode)

// Aboslute, should change -- if no changes - print
#define MCA_STAT_FPGA_REG_CHANGE_ABS(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_CHANGE, MCA_COMPONENT, ilogCode)

#define MCA_CHANNEL_SIZE                    0x0100

// Global Variables ###############################################################################

// Static Variables ###############################################################################

static const StatFpgaReg McaStatChannelU8Regs[] =
{
//  8 bit data of MCA CH TX
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_tx_stats0_cmd_fifo_ADDRESS),                MCA_TX_STAT_CMD_FIFO),
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_tx_stats0_dp_fifo_rd_drp_pkt_ADDRESS),      MCA_TX_STAT_DP_FIFO_RD_DRP_PKT),
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_tx_stats0_dp_fifo_wr_drp_pkt_ADDRESS),      MCA_TX_STAT_DP_FIFO_WR_DRP_PKT),
//    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_tx_stats0_dp_fifo_wr_pkt_err_ADDRESS),      MCA_TX_STAT_DP_FIFO_WR_PKT_ERR),
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_tx_stats0_dp_fifo_fifo_full_err_ADDRESS),   MCA_TX_STAT_DP_FIFO_FIFO_FULL_ERR),
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_tx_stats0_dp_fifo_pkt_max_err_ADDRESS),     MCA_TX_STAT_DP_FIFO_PKT_MAX_ERR),
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_tx_stats0_dp_grd_no_sop_err_ADDRESS),       MCA_TX_STAT_DP_GRD_NO_SOP_ERR),
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_tx_stats0_dp_grd_no_eop_err_ADDRESS),       MCA_TX_STAT_DP_GRD_NO_EOP_ERR),
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_tx_stats0_dp_grd_max_err_ADDRESS),          MCA_TX_STAT_DP_GRD_MAX_ERR),

//  8 bit data of MCA CH RX
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_rx_stats0_dp_fifo_rd_drp_pkt_ADDRESS),      MCA_RX_STAT_DP_FIFO_RD_DRP_PKT),
//    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_rx_stats0_dp_fifo_wr_drp_pkt_ADDRESS),      MCA_RX_STAT_DP_FIFO_WR_DRP_PKT),
//    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_rx_stats0_dp_fifo_wr_pkt_err_ADDRESS),      MCA_RX_STAT_DP_FIFO_WR_PKT_ERR),
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_rx_stats0_dp_fifo_fifo_full_err_ADDRESS),   MCA_RX_STAT_DP_FIFO_FIFO_FULL_ERR),
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_rx_stats0_dp_fifo_pkt_max_err_ADDRESS),     MCA_RX_STAT_DP_FIFO_PKT_MAX_ERR),
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_rx_stats0_dp_grd_no_sop_err_ADDRESS),       MCA_RX_STAT_DP_GRD_NO_SOP_ERR),
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_rx_stats0_dp_grd_no_eop_err_ADDRESS),       MCA_RX_STAT_DP_GRD_NO_EOP_ERR),
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_rx_stats0_dp_grd_max_err_ADDRESS),          MCA_RX_STAT_DP_GRD_MAX_ERR),

};
static uint8_t McaStatStatChannel0U8data[ARRAYSIZE(McaStatChannelU8Regs)];
static uint8_t McaStatStatChannel1U8data[ARRAYSIZE(McaStatChannelU8Regs)];
static uint8_t McaStatStatChannel2U8data[ARRAYSIZE(McaStatChannelU8Regs)];
static uint8_t McaStatStatChannel3U8data[ARRAYSIZE(McaStatChannelU8Regs)];
static uint8_t McaStatStatChannel4U8data[ARRAYSIZE(McaStatChannelU8Regs)];
static uint8_t McaStatStatChannel5U8data[ARRAYSIZE(McaStatChannelU8Regs)];

static const StatFpgaReg McaStatCoreU8Regs[] =
{
//  8 bit data of MCA Core Tx
    MCA_STAT_FPGA_REG_NOCHNG_REL( MCA_CORE_ADDRESS(core_tx_stats0_pfifo_ADDRESS),     MCA_CORE_TX_PFIFO),
    MCA_STAT_FPGA_REG_NOCHNG_REL( MCA_CORE_ADDRESS(core_tx_stats0_nfifo_ADDRESS),     MCA_CORE_TX_NFIFO),
    MCA_STAT_FPGA_REG_NOCHNG_REL( MCA_CORE_ADDRESS(core_tx_stats0_lbad_ADDRESS),      MCA_CORE_TX_LBAD),
    MCA_STAT_FPGA_REG_NOCHNG_REL( MCA_CORE_ADDRESS(core_tx_stats0_lrtry_ADDRESS),     MCA_CORE_TX_LRTRY),

//  8 bit data of MCA Core Rx
    MCA_STAT_FPGA_REG_NOCHNG_REL( MCA_CORE_ADDRESS(core_rx_stats0_lbad_ADDRESS),     MCA_CORE_RX_LBAD),
    MCA_STAT_FPGA_REG_NOCHNG_REL( MCA_CORE_ADDRESS(core_rx_stats0_lrtry_ADDRESS),    MCA_CORE_RX_LRTRY),
    MCA_STAT_FPGA_REG_NOCHNG_REL( MCA_CORE_ADDRESS(core_rx_stats0_lcrd_ADDRESS),     MCA_CORE_RX_LCRD),
};
static uint8_t McaStatStatCoreU8data[ARRAYSIZE(McaStatCoreU8Regs)];


static const StatFpgaReg McaStatChannelU16Regs[] =
{
//  16 bit data of MCA CH Link
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_link_stats0_u0_to_rec_ADDRESS),    MCA_LINK_STAT_U0_TO_REC),
    MCA_STAT_FPGA_REG_NOCHNG_REL_CH( MCA_CHANNEL_ADDRESS(channel_link_stats0_pol_fail_ADDRESS),     MCA_LINK_STAT_POL_FAIL),
};
static uint16_t McaStatStatChannel0U16data[ARRAYSIZE(McaStatChannelU16Regs)];
static uint16_t McaStatStatChannel1U16data[ARRAYSIZE(McaStatChannelU16Regs)];
static uint16_t McaStatStatChannel2U16data[ARRAYSIZE(McaStatChannelU16Regs)];
static uint16_t McaStatStatChannel3U16data[ARRAYSIZE(McaStatChannelU16Regs)];
static uint16_t McaStatStatChannel4U16data[ARRAYSIZE(McaStatChannelU16Regs)];
static uint16_t McaStatStatChannel5U16data[ARRAYSIZE(McaStatChannelU16Regs)];

static const StatFpgaReg McaStatCoreU16Regs[] =
{
//  16 bit data of MCA Core Tx
    //MCA_STAT_FPGA_REG_NOCHNG_REL( bb_chip_mca_core_tx_stats0_cmd_fifo_dcount_ADDRESS,               MCA_CORE_TX_CMD_FIFO_DCOUNT),

//  16 bit data of MCA Core Rx
    //MCA_STAT_FPGA_REG_NOCHNG_REL( bb_chip_mca_core_rx_stats0_cmd_fifo_dcount_ADDRESS,               MCA_CORE_RX_CMD_FIFO_DCOUNT),
};
static uint16_t McaStatStatCoreU16data[ARRAYSIZE(McaStatCoreU16Regs)];


static const StatFpgaReg McaStatChannelU32Regs[] =
{
//  32 bit data of MCA CH Link
    // MCA_STAT_FPGA_REG_NOCHNG_REL_CH( bb_chip_mca_channel_link_stats0_tx_frm_ADDRESS,                MCA_LINK_STAT_TX_FRM),
    // MCA_STAT_FPGA_REG_NOCHNG_REL_CH( bb_chip_mca_channel_link_stats0_tx_mcup_ADDRESS,               MCA_LINK_STAT_TX_MCUP),
    // MCA_STAT_FPGA_REG_NOCHNG_REL_CH( bb_chip_mca_channel_link_stats0_rx_frm_ADDRESS,                MCA_LINK_STAT_RX_FRM),
    // MCA_STAT_FPGA_REG_NOCHNG_REL_CH( bb_chip_mca_channel_link_stats0_rx_mcup_ADDRESS,               MCA_LINK_STAT_RX_MCUP),

//  32 bit data of MCA CH Tx
    // MCA_STAT_FPGA_REG_NOCHNG_REL_CH( bb_chip_mca_channel_tx_stats0_dp_pfifo_ADDRESS,                MCA_TX_STAT_DP_PFIFO),
    // MCA_STAT_FPGA_REG_NOCHNG_REL_CH( bb_chip_mca_channel_tx_stats0_dp_nfifo_ADDRESS,                MCA_TX_STAT_DP_NFIFO),

//  32 bit data of MCA CH Rx
    // MCA_STAT_FPGA_REG_NOCHNG_REL_CH( bb_chip_mca_channel_rx_stats0_dp_pfifo_ADDRESS,                MCA_RX_STAT_DP_PFIFO),
    // MCA_STAT_FPGA_REG_NOCHNG_REL_CH( bb_chip_mca_channel_rx_stats0_dp_nfifo_ADDRESS,                MCA_RX_STAT_DP_NFIFO),
};
static uint32_t McaStatStatChannel0U32data[ARRAYSIZE(McaStatChannelU32Regs)];
static uint32_t McaStatStatChannel1U32data[ARRAYSIZE(McaStatChannelU32Regs)];
static uint32_t McaStatStatChannel2U32data[ARRAYSIZE(McaStatChannelU32Regs)];
static uint32_t McaStatStatChannel3U32data[ARRAYSIZE(McaStatChannelU32Regs)];
static uint32_t McaStatStatChannel4U32data[ARRAYSIZE(McaStatChannelU32Regs)];
static uint32_t McaStatStatChannel5U32data[ARRAYSIZE(McaStatChannelU32Regs)];

static const StatFpgaReg McaStatCoreU32Regs[] =
{
//  32 bit data of MCA Core
    MCA_STAT_FPGA_REG_NOCHNG_REL( MCA_CORE_ADDRESS(core_irq0_enable_ADDRESS),        MCA_CORE_IRQ0_ENABLE),
};
static uint32_t McaStatStatCoreU32data[ARRAYSIZE(McaStatCoreU32Regs)];

//stats for channel0
const StatRegistration Mca8bitStatsChannelCpuComm = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_8_BITS,
        McaStatChannelU8Regs,
        McaStatStatChannel0U8data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_CPU_COMM,
        CHANNEL_OFFSET);

const StatRegistration Mca16bitStatsChannelCpuComm = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_16_BITS,
        McaStatChannelU16Regs,
        McaStatStatChannel0U16data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_CPU_COMM,
        CHANNEL_OFFSET);

const StatRegistration Mca32bitStatsChannelCpuComm = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_32_BITS,
        McaStatChannelU32Regs,
        McaStatStatChannel0U32data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_CPU_COMM,
        CHANNEL_OFFSET);

//stats for channel1
const StatRegistration Mca8bitStatsChannelDp = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_8_BITS,
        McaStatChannelU8Regs,
        McaStatStatChannel1U8data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_DP,
        CHANNEL_OFFSET);

const StatRegistration Mca16bitStatsChannelDp = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_16_BITS,
        McaStatChannelU16Regs,
        McaStatStatChannel1U16data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_DP,
        CHANNEL_OFFSET);

const StatRegistration Mca32bitStatsChannelDp = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_32_BITS,
        McaStatChannelU32Regs,
        McaStatStatChannel1U32data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_DP,
        CHANNEL_OFFSET);

//stats for channel2
const StatRegistration Mca8bitStatsChannelUsb3 = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_8_BITS,
        McaStatChannelU8Regs,
        McaStatStatChannel2U8data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_USB3,
        CHANNEL_OFFSET);

const StatRegistration Mca16bitStatsChannelUsb3 = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_16_BITS,
        McaStatChannelU16Regs,
        McaStatStatChannel2U16data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_USB3,
        CHANNEL_OFFSET);

const StatRegistration Mca32bitStatsChannelUsb3 = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_32_BITS,
        McaStatChannelU32Regs,
        McaStatStatChannel2U32data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_USB3,
        CHANNEL_OFFSET);

//stats for channel3
const StatRegistration Mca8bitStatsChannelGE = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_8_BITS,
        McaStatChannelU8Regs,
        McaStatStatChannel3U8data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_GE,
        CHANNEL_OFFSET);

const StatRegistration Mca16bitStatsChannelGE = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_16_BITS,
        McaStatChannelU16Regs,
        McaStatStatChannel3U16data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_GE,
        CHANNEL_OFFSET);

const StatRegistration Mca32bitStatsChannelGE = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_32_BITS,
        McaStatChannelU32Regs,
        McaStatStatChannel3U32data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_GE,
        CHANNEL_OFFSET);

//stats for channel4
const StatRegistration Mca8bitStatsChannelGMII = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_8_BITS,
        McaStatChannelU8Regs,
        McaStatStatChannel4U8data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_GMII,
        CHANNEL_OFFSET);

const StatRegistration Mca16bitStatsChannelGMII = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_16_BITS,
        McaStatChannelU16Regs,
        McaStatStatChannel4U16data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_GMII,
        CHANNEL_OFFSET);

const StatRegistration Mca32bitStatsChannelGMII = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_32_BITS,
        McaStatChannelU32Regs,
        McaStatStatChannel4U32data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_GMII,
        CHANNEL_OFFSET);

//stats for channel5
const StatRegistration Mca8bitStatsChannelRS232 = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_8_BITS,
        McaStatChannelU8Regs,
        McaStatStatChannel5U8data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_RS232,
        CHANNEL_OFFSET);

const StatRegistration Mca16bitStatsChannelRS232 = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_16_BITS,
        McaStatChannelU16Regs,
        McaStatStatChannel5U16data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_RS232,
        CHANNEL_OFFSET);

const StatRegistration Mca32bitStatsChannelRS232 = STATMON_REGISTRATION_INIT_OFFSET_INDEX(
        STATISTIC_DATA_SIZE_32_BITS,
        McaStatChannelU32Regs,
        McaStatStatChannel5U32data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL,
        MCA_CHANNEL_NUMBER_RS232,
        CHANNEL_OFFSET);

//stats for core
const StatRegistration Mca8bitStatsCore = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_8_BITS,
        McaStatCoreU8Regs,
        McaStatStatCoreU8data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL);

const StatRegistration Mca16bitStatsCore = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_16_BITS,
        McaStatCoreU16Regs,
        McaStatStatCoreU16data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL);

const StatRegistration Mca32bitStatsCore = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_32_BITS,
        McaStatCoreU32Regs,
        McaStatStatCoreU32data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MCA_STATS_POLL_INTERVAL);

const StatRegistration *mca8BitChannelStats[] = { 
        &Mca8bitStatsChannelCpuComm,
        &Mca8bitStatsChannelDp,
        &Mca8bitStatsChannelUsb3,
        &Mca8bitStatsChannelGE,
        &Mca8bitStatsChannelGMII,
        &Mca8bitStatsChannelRS232
};

const StatRegistration *mca16bitChannelStats[] = { 
        &Mca16bitStatsChannelCpuComm,
        &Mca16bitStatsChannelDp,
        &Mca16bitStatsChannelUsb3,
        &Mca16bitStatsChannelGE,
        &Mca16bitStatsChannelGMII,
        &Mca16bitStatsChannelRS232
};

const StatRegistration *mca32bitChannelStats[] = { 
        &Mca32bitStatsChannelCpuComm,
        &Mca32bitStatsChannelDp,
        &Mca32bitStatsChannelUsb3,
        &Mca32bitStatsChannelGE,
        &Mca32bitStatsChannelGMII,
        &Mca32bitStatsChannelRS232
};

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// MCA initialization function
//
// Parameters:
//
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void MCA_StatInit(void)
{
    int channelnum;
    for(channelnum=MCA_CHANNEL_NUMBER_CPU_COMM; channelnum <NUM_OF_MCA_CHANNEL_NUMBER ; channelnum++)
    {
        STATSMON_RegisterStatgroup(mca8BitChannelStats[channelnum]);
        STATSMON_RegisterStatgroup(mca16bitChannelStats[channelnum]);
    //    STATSMON_RegisterStatgroup(mca32bitChannelStats[channelnum]);
    }
    STATSMON_RegisterStatgroup(&Mca8bitStatsCore);
    // STATSMON_RegisterStatgroup(&Mca16bitStatsCore);
    // STATSMON_RegisterStatgroup(&Mca32bitStatsCore);
}


//#################################################################################################
// Start and Stop the link layer stats monitor timer.
//
// Parameters: channelIndex: Channel number to monitor
//             enable: True to start and false to stop
// Return:
// Assumptions:
//#################################################################################################
void MCA_ControlStatsMonitorChannel(enum MCA_ChannelNumber channel, bool enable)
{
    iassert_MCA_COMPONENT_1( (channel<NUM_OF_MCA_CHANNEL_NUMBER), MCA_INVALID_CHANNEL_INDEX, channel);
    // set the rd2clr_config register so it will automatically clear the stat when read
    MCA_channelSetReadClearStats(channel);
    STATSMON_StatgroupControl(mca8BitChannelStats[channel], enable);
    STATSMON_StatgroupControl(mca16bitChannelStats[channel], enable);
    // STATSMON_StatgroupControl(mca32bitChannelStats[channel], enable);
}

//#################################################################################################
// Start and Stop the core stats monitor timer.
//
// Parameters: enable: True to start and false to stop
// Return:
// Assumptions:
//#################################################################################################

void MCA_ControlStatsMonitorCore(bool enable)
{
    // set the rd2clr_config register so it will automatically clear the stat when read
    MCA_coreSetReadClearStats();
    STATSMON_StatgroupControl(&Mca8bitStatsCore,  enable);
    // STATSMON_StatgroupControl(&Mca16bitStatsCore, enable);
    // STATSMON_StatgroupControl(&Mca32bitStatsCore, enable);
}



