//#################################################################################################
// Icron Technology Corporation - Copyright 2018
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
// Implementations of monitoring UPP status.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <stats_mon.h>
#include <upp_regs.h>
#include <module_addresses_regs.h>
#include "upp_log.h"
#include "upp_loc.h"
// Constants and Macros ###########################################################################
// Macros to get the relative address of the register from the rtl submodule instead of bb_chip
#define UPP_ADDRESS(componentStatAddress) (bb_chip_upp_s_ADDRESS + componentStatAddress)

#define UPP_STATS_POLL_INTERVAL     (1000/STATISTIC_INTERVAL_TICK)        // poll the UPP stats every second

// Relative, should not change -- if changes - print
#define UPP_STAT_FPGA_REG_NOCHNG_REL(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_RELATIVE_VALUE, UPP_COMPONENT, ilogCode)

// Relative, should change -- if no changes - print
#define UPP_STAT_FPGA_REG_CHANGE_REL(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_CHANGE | STATMON_PARAM_FLAG_RELATIVE_VALUE, UPP_COMPONENT, ilogCode)

// Relative, should not change -- if changes - print red
#define UPP_STAT_FPGA_REG_NOCHNG_REL_CRI(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_RELATIVE_VALUE | STATMON_PARAM_MAJOR_DISPLAY, UPP_COMPONENT, ilogCode)

// Absolute, should not change -- if changes - print
#define UPP_STAT_FPGA_REG_NOCHNG_ABS(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, 0, UPP_COMPONENT, ilogCode)

// Aboslute, should change -- if no changes - print
#define UPP_STAT_FPGA_REG_CHANGE_ABS(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, STATMON_PARAM_FLAG_CHANGE, UPP_COMPONENT, ilogCode)


// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Static Variables ###############################################################################
static const StatFpgaReg UppStatU8Regs[] =
{
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_id_mgr_fifo_write_engine_stats0_pkt_max_byte_cnt_err_ADDRESS),        UPP_ID_MGR_FIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL_CRI ( UPP_ADDRESS(upp_id_mgr_fifo_write_engine_stats0_fifo_full_err_ADDRESS),          UPP_ID_MGR_FIFO_WRENG_STATS0_FIFO_FULL_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_id_mgr_fifo_write_engine_stats0_pkt_err_ADDRESS),                     UPP_ID_MGR_FIFO_WRENG_STATS0_PKT_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_id_mgr_fifo_write_engine_stats0_pkt_sop_err_ADDRESS),                 UPP_ID_MGR_FIFO_WRENG_STATS0_PKT_SOP_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_id_mgr_fifo_write_engine_stats0_drp_pkt_rd_ADDRESS),                  UPP_ID_MGR_FIFO_WRENG_STATS0_DRP_PKT_RD),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_id_mgr_fifo_write_engine_stats0_drp_pkt_wr_ADDRESS),                  UPP_ID_MGR_FIFO_WRENG_STATS0_DRP_PKT_WR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_id_mgr_fifo_read_engine_stats0_drp_pkt_ADDRESS),                      UPP_ID_MGR_FIFO_WRENG_STATS0_DRP_PKT),

    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_iso_rex_fifo_write_engine_stats0_pkt_max_byte_cnt_err_ADDRESS),       UPP_ISO_REX_FIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL_CRI( UPP_ADDRESS(upp_iso_rex_fifo_write_engine_stats0_fifo_full_err_ADDRESS),          UPP_ISO_REX_FIFO_WRENG_STATS0_FIFO_FULL_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_iso_rex_fifo_write_engine_stats0_pkt_err_ADDRESS),                    UPP_ISO_REX_FIFO_WRENG_STATS0_PKT_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_iso_rex_fifo_write_engine_stats0_pkt_sop_err_ADDRESS),                UPP_ISO_REX_FIFO_WRENG_STATS0_PKT_SOP_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_iso_rex_fifo_write_engine_stats0_drp_pkt_rd_ADDRESS),                 UPP_ISO_REX_FIFO_WRENG_STATS0_DRP_PKT_RD),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_iso_rex_fifo_write_engine_stats0_drp_pkt_wr_ADDRESS),                 UPP_ISO_REX_FIFO_WRENG_STATS0_DRP_PKT_WR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_iso_rex_fifo_read_engine_stats0_drp_pkt_ADDRESS),                     UPP_ISO_REX_FIFO_WRENG_STATS0_DRP_PKT),

    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_ctrl_trfr_h2d_fifo_write_engine_stats0_pkt_max_byte_cnt_err_ADDRESS), UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL_CRI( UPP_ADDRESS(upp_ctrl_trfr_h2d_fifo_write_engine_stats0_fifo_full_err_ADDRESS),    UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_FIFO_FULL_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_ctrl_trfr_h2d_fifo_write_engine_stats0_pkt_err_ADDRESS),              UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_PKT_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_ctrl_trfr_h2d_fifo_write_engine_stats0_pkt_sop_err_ADDRESS),          UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_PKT_SOP_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_ctrl_trfr_h2d_fifo_write_engine_stats0_drp_pkt_rd_ADDRESS),           UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_DRP_PKT_RD),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_ctrl_trfr_h2d_fifo_write_engine_stats0_drp_pkt_wr_ADDRESS),           UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_DRP_PKT_WR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_ctrl_trfr_h2d_fifo_read_engine_stats0_drp_pkt_ADDRESS),               UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_DRP_PKT),

    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_ctrl_trfr_d2h_fifo_write_engine_stats0_pkt_max_byte_cnt_err_ADDRESS), UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL_CRI( UPP_ADDRESS(upp_ctrl_trfr_d2h_fifo_write_engine_stats0_fifo_full_err_ADDRESS),    UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_FIFO_FULL_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_ctrl_trfr_d2h_fifo_write_engine_stats0_pkt_err_ADDRESS),              UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_PKT_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_ctrl_trfr_d2h_fifo_write_engine_stats0_pkt_sop_err_ADDRESS),          UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_PKT_SOP_ERR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_ctrl_trfr_d2h_fifo_write_engine_stats0_drp_pkt_rd_ADDRESS),           UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_DRP_PKT_RD),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_ctrl_trfr_d2h_fifo_write_engine_stats0_drp_pkt_wr_ADDRESS),           UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_DRP_PKT_WR),
    UPP_STAT_FPGA_REG_NOCHNG_REL( UPP_ADDRESS(upp_ctrl_trfr_d2h_fifo_read_engine_stats0_drp_pkt_ADDRESS),               UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_DRP_PKT),

};

static uint8_t UppStatU8data[ARRAYSIZE(UppStatU8Regs)];

static const StatFpgaReg UppStatU16Regs[] = 
{
    UPP_STAT_FPGA_REG_NOCHNG_ABS ( UPP_ADDRESS(upp_ctrl_trfr_h2d_fifo_write_engine_stats0_nfifo_dcount_ADDRESS),     UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_NFIFO_DCOUNT),
    UPP_STAT_FPGA_REG_NOCHNG_ABS( UPP_ADDRESS(upp_ctrl_trfr_h2d_fifo_write_engine_stats0_pfifo_dcount_ADDRESS),     UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_PFIFO_DCOUNT),
    UPP_STAT_FPGA_REG_NOCHNG_ABS( UPP_ADDRESS(upp_ctrl_trfr_d2h_fifo_write_engine_stats0_nfifo_dcount_ADDRESS),     UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_NFIFO_DCOUNT),
    UPP_STAT_FPGA_REG_NOCHNG_ABS( UPP_ADDRESS(upp_ctrl_trfr_d2h_fifo_write_engine_stats0_pfifo_dcount_ADDRESS),     UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_PFIFO_DCOUNT),
};

static uint16_t UppStatU16data[ARRAYSIZE(UppStatU16Regs)];



const StatRegistration Upp8Stats = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_8_BITS,
        UppStatU8Regs,
        UppStatU8data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        UPP_STATS_POLL_INTERVAL);

const StatRegistration Upp16Stats = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_16_BITS,
        UppStatU16Regs,
        UppStatU16data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        UPP_STATS_POLL_INTERVAL);
// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################
//#################################################################################################
// Upp stat initialization function
//
// Parameters:
//
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################

void UPP_StatInit(void)
{
    STATSMON_RegisterStatgroup(&Upp8Stats);
    STATSMON_RegisterStatgroup(&Upp16Stats);
}

//#################################################################################################
// Enables or disables monitoring of the Upp statistics
//
// Parameters:
// Return:
// Assumptions: Reads a byte from address 0xC0B00000
//#################################################################################################
void Upp_StatMonControl(bool enable)
{
    // set the rd2clr_config register so it will automatically clear the stat when read
    UPPHalSetReadClearStats();
    STATSMON_StatgroupControl(&Upp8Stats,       enable);
    STATSMON_StatgroupControl(&Upp16Stats,       enable);

}
// Static Function Definitions ####################################################################
