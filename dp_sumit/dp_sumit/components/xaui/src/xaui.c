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
// The PCS/PMA is a Xilinx IP that supports our link.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <itypes.h>
#include <bb_top_regs.h>
#include <bb_top.h>
#include <xaui.h>
#include "xaui_log.h"
#include <leon_timers.h>
#include <stats_mon.h>
#include <module_addresses_regs.h>

// Constants and Macros ###########################################################################
#define XAUI_STATUS_SYNCED  (0x3F)
#define XAUI_STATUS_TX_ALIGN_COMPLETE_OFFSET (0x01)     // bit 0 of bb_top->rxaui->status (0x80002168)
#define XAUI_TX_ALIGN_TIME     (500)                       // wait maximum 500us until XAUI Tx align

#define XAUI_STATS_POLL_INTERVAL        ((3*1000)/STATISTIC_INTERVAL_TICK)  // poll the stats every 3 seconds

#define XAUI_ADDRESS(componentAddress)  (bb_chip_bb_top_s_ADDRESS + componentAddress)

#define XAUI_STAT_REG_CHANGE_REL( \
        fpgaAddress, \
        ilogCode) \
        STATMON_FPGA_READ_PARAM( \
            fpgaAddress, \
            STATMON_PARAM_FLAG_RELATIVE_VALUE, \
            XAUI_COMPONENT, \
            ilogCode)

// Data Types #####################################################################################
volatile bb_top_s * top_rxaui = (volatile void *)(bb_chip_bb_top_s_ADDRESS);

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct
{
    volatile bb_top_rxaui *xaui;                // pointer to the XAUI registers
    XauiStatusChangeHandler notifyStatusChange; // handler to notify on status change
} _XAUI;

static const StatFpgaReg XauiStatChannelU32Regs[] =
{
    XAUI_STAT_REG_CHANGE_REL(XAUI_ADDRESS(bb_top_rxaui_stats0_gt0_disp_err_ADDRESS),     XAUI_STAT_GT0_DISP_ERR),
    XAUI_STAT_REG_CHANGE_REL(XAUI_ADDRESS(bb_top_rxaui_stats0_gt0_not_in_table_ADDRESS), XAUI_STAT_GT0_NOT_IN_TBL),
    XAUI_STAT_REG_CHANGE_REL(XAUI_ADDRESS(bb_top_rxaui_stats0_gt1_disp_err_ADDRESS),     XAUI_STAT_GT1_DISP_ERR),
    XAUI_STAT_REG_CHANGE_REL(XAUI_ADDRESS(bb_top_rxaui_stats0_gt1_not_in_table_ADDRESS), XAUI_STAT_GT1_NOT_IN_TBL),
    XAUI_STAT_REG_CHANGE_REL(XAUI_ADDRESS(bb_top_rxaui_stats0_missing_start_ADDRESS),    XAUI_STAT_MISSING_START),
};

static uint32_t XauiStatU32data[ARRAYSIZE(XauiStatChannelU32Regs)];

const StatRegistration Xaui32bitStats = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_32_BITS,
        XauiStatChannelU32Regs,
        XauiStatU32data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        XAUI_STATS_POLL_INTERVAL);

// Static Function Declarations ###################################################################


// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialization function for the XAUI module
//
// Parameters:
//      AUIBaseAddr      - The address of the RXAUI hardware block.
//      notifyCompletionHandler - Completion handler when debug port shows all status bits are set
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void XAUI_Init(XauiStatusChangeHandler notifyChangeHandler)
{
     _XAUI.xaui = (volatile bb_top_rxaui *)&(top_rxaui->rxaui.s);
     _XAUI.notifyStatusChange = notifyChangeHandler;

    STATSMON_RegisterStatgroup(&Xaui32bitStats);
}

//#################################################################################################
// Disables XAUI Rx
//
// Parameters: none
// Return:
// Assumptions:
//
//#################################################################################################
void XAUI_DisableRx(void)
{
    bb_top_ApplyResetRxLinkStats(true);
}


//#################################################################################################
// Enables XAUI Rx
//
// Parameters: none
// Return:
// Assumptions:
//
//#################################################################################################
void XAUI_EnableRx(void)
{
    bb_top_ApplyResetRxLinkStats(false);
}


//#################################################################################################
// Starts and stops the XAUI module
//
// Parameters: enable - true = on, false = off
// Return:
// Assumptions:
//
//#################################################################################################
void XAUI_Control(bool enable)
{
//    UART_printf("XAUI_Control %d\n", enable);

    if (enable)
    {
        // setting values before releasing reset
        // Set the type_sel to DTE XGXS, and Device address: MDIO_DEVTYPE_DTE_XS is available
        // See page67 Table 3-3 of PG083 November 19, 2014
        _XAUI.xaui->control.bf.type_sel = 0x2;
        _XAUI.xaui->control.bf.fix_missing_start = 1;       // fix missing FBs
        _XAUI.xaui->gt_misc_ctrl.bf.gt1_rxpolarity = 1;
        _XAUI.xaui->stats0.s.rd2clr_config.dw = bb_top_rxaui_stats0_rd2clr_config_WRITEMASK;

        _XAUI.xaui->control.bf.reset = 0;

        // Check XAUI align before release Aquantia reset
        LEON_TimerValueT startTime = LEON_TimerRead();
        while(!XAUI_isTxAlignCompleted())
        {
            iassert_XAUI_COMPONENT_0(
                LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < XAUI_TX_ALIGN_TIME,
                XAUI_FAIL_TX_ALIGN);
        }
    }
    else
    {
        XAUI_RxAlignmentReporting(false);                   // turn off the Rx aligned status reporting
        _XAUI.xaui->control.bf.reset = 1;                   // disabled - put XAUI into reset
    }
}

//#################################################################################################
// Puts Xaui through a Rx re-sync cycle
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void XAUI_ReSyncRx(void)
{
    // sanity check to make sure we are not in reset when this is called (like from icmd)
    if (_XAUI.xaui->control.bf.reset == 0)
    {
        bb_top_rxaui_gt_rst_ctrl  gtRstCtrl = _XAUI.xaui->gt_rst_ctrl;

        // Toggle the GT0, Gt1 elastic buffer reset lines

        // put them into reset
        gtRstCtrl.bf.gt0_rxbufreset = 1;
        gtRstCtrl.bf.gt1_rxbufreset = 1;
        _XAUI.xaui->gt_rst_ctrl.dw = gtRstCtrl.dw;

        // and take them out of reset
        gtRstCtrl.bf.gt0_rxbufreset = 0;
        gtRstCtrl.bf.gt1_rxbufreset = 0;
        _XAUI.xaui->gt_rst_ctrl.dw = gtRstCtrl.dw;

        ilog_XAUI_COMPONENT_0(ILOG_USER_LOG, XAUI_RESET_RX_BUFFERS);
    }
}


//#################################################################################################
// ISR handler for RXAUI alignment changes
//
// Parameters:
// Return:
// Assumptions: Interrupt occurs when rxaui.status[5]:Align Status changes
//#################################################################################################
void XAUI_AlignmentStatusIsr(void)
{
    bool xauiOn = XAUI_CheckRxAlignedStatus();

    _XAUI.notifyStatusChange(xauiOn);

    ilog_XAUI_COMPONENT_1(ILOG_USER_LOG, XAUI_RX_ALIGN,xauiOn);
}


//#################################################################################################
// Reads the TX Phase Align Complete status
//
// Parameters:
// Return:  true if the alignment has finished, false otherwise
// Assumptions: It takes about 120us to set after XAUI inits
//
//#################################################################################################
bool XAUI_isTxAlignCompleted(void)
{
    return _XAUI.xaui->status.bf.debug & XAUI_STATUS_TX_ALIGN_COMPLETE_OFFSET;
}


//#################################################################################################
// Enable/Disable Rx alignment status indications
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void XAUI_RxAlignmentReporting(bool enable)
{
    if (enable)
    {
        XAUI_AlignmentStatusIsr();              // get the current status
        bb_top_a7_RxauiAlignedEnable(true);     // turn the Rx aligned interrupt on
    }
    else
    {
        bb_top_a7_RxauiAlignedEnable(false);                // turn the Rx aligned interrupt off
        STATSMON_StatgroupControl(&Xaui32bitStats, false);  // disable XAUI stats
    }

}

//#################################################################################################
// Checks the current status of the XAUI.  Returns true if there is Rx Alignment.  If the module is in
// reset (disabled) then there is no alignment
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool XAUI_CheckRxAlignedStatus(void)
{
    // see if we are synced and aligned (and not in reset!)
    return ((_XAUI.xaui->status.bf.debug == XAUI_STATUS_SYNCED)
            && (_XAUI.xaui->control.bf.reset == 0));
}

//#################################################################################################
// Checks the current status of the Gt0 and Gt1 stats.  Returns true if non zero
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool XAUI_GtxErrorStatus(void)
{
    bool error =
        _XAUI.xaui->stats0.s.gt0_disp_err.dw +
        _XAUI.xaui->stats0.s.gt1_disp_err.dw +
        _XAUI.xaui->stats0.s.gt0_not_in_table.dw +
        _XAUI.xaui->stats0.s.gt1_not_in_table.dw +
        _XAUI.xaui->stats0.s.missing_start.dw;

    return (error);
}

//#################################################################################################
// Toggle GTX Rx reset, to start a full channel Rx reset sequence
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void XAUI_ToggleGtRxReset(void)
{
    ilog_XAUI_COMPONENT_0(ILOG_USER_LOG, XAUI_RX_TOGGLE_RESET);

    _XAUI.xaui->gt_rst_ctrl.bf.gtrxreset = 1;
    _XAUI.xaui->gt_rst_ctrl.bf.gtrxreset = 0;

    LEON_TimerWaitMicroSec(10); // wait a bit of time for the reset to propagate

}

//#################################################################################################
// Toggle GTX Rx reset, to start a full channel Rx reset sequence
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void XAUI_EnableStats(void)
{
    STATSMON_StatgroupControl(&Xaui32bitStats, true);    // enable XAUI stats
}



// Component Scope Function Definitions ###########################################################

//#################################################################################################
// ICMD wrapper to reset the elasticity buffers
//
// Parameters:
// Return:
// Assumptions: Interrupt occurs when rxaui.status[5]:Align Status changes
//#################################################################################################
void Xaui_ResetBuffers(void)
{
    XAUI_ReSyncRx();
}

// Static Function Definitions ####################################################################


