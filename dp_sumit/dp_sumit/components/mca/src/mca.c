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
// Implementations of functions common to the Lex and Rex CPU communcations.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <options.h>
#include <bb_chip_regs.h>
#include <bb_top.h>
#include <mca_core_regs.h>
#include <mca_channel_regs.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <interrupts.h>
#include <mca.h>
#include "mca_log.h"
#include "mca_loc.h"
#include <bb_core.h>

#include <uart.h>  // for debugging

// Constants and Macros ###########################################################################

// should only take up to 100us for Tx and Rx to be enabled on the core
#define MCA_HAL_CORE_MAX_TX_RX_ENABLE_TIME  100

#define MCA_HAL_CHANNEL_READY               0x01        // if set, means the current channel is ready

// MCA Core interrupts
#define MCA_HAL_CORE_IRQ_ENABLES \
    (                                                     \
           CORE_IRQ0_RAW_RX_DP_GRD_SOP_ERR_MASK         | \
           CORE_IRQ0_RAW_RX_DP_GRD_EOP_ERR_MASK         | \
           CORE_IRQ0_RAW_RX_DP_GRD_MAX_ERR_MASK         | \
           CORE_IRQ0_RAW_RX_LNK_CMD_FIFO_FULL_ERR_MASK  | \
           CORE_IRQ0_RAW_TX_LNK_CMD_FIFO_FULL_ERR_MASK  | \
           CORE_IRQ0_RAW_TX_DP_GRD_SOP_ERR_MASK         | \
           CORE_IRQ0_RAW_TX_DP_GRD_EOP_ERR_MASK         | \
           CORE_IRQ0_RAW_TX_DP_GRD_MAX_ERR_MASK         | \
           CORE_IRQ0_RAW_TX_DP_FIFO_DRP_PKT_WR_MASK     | \
           CORE_IRQ0_RAW_TX_DP_FIFO_DRP_PKT_RD_MASK     | \
           CORE_IRQ0_RAW_TX_DP_FIFO_PKT_SOP_ERR_MASK    | \
           CORE_IRQ0_RAW_TX_DP_FIFO_PKT_ERR_MASK        | \
           CORE_IRQ0_RAW_TX_DP_FIFO_FULL_ERR_MASK       | \
           CORE_IRQ0_RAW_TX_DP_FIFO_PKT_MAX_SZ_ERR_MASK | \
           CORE_IRQ0_RAW_TX_DP_FIFO_DRP_PKT_MASK        | \
           CORE_IRQ0_RAW_DP_READY_MASK                  | \
           CORE_IRQ0_RAW_DP_RX_READY_MASK               | \
           CORE_IRQ0_RAW_DP_TX_READY_MASK)

#define MCA_HAL_CORE_IRQ_GUARD_MASK \
    (                                                     \
           CORE_IRQ0_RAW_RX_DP_GRD_SOP_ERR_MASK         | \
           CORE_IRQ0_RAW_RX_DP_GRD_EOP_ERR_MASK         | \
           CORE_IRQ0_RAW_RX_DP_GRD_MAX_ERR_MASK         | \
           CORE_IRQ0_RAW_TX_DP_GRD_SOP_ERR_MASK         | \
           CORE_IRQ0_RAW_TX_DP_GRD_EOP_ERR_MASK         | \
           CORE_IRQ0_RAW_TX_DP_GRD_MAX_ERR_MASK)

// MCA channel interrupts
#define MCA_HAL_CHANNEL_IRQ_ENABLES \
       (                                                  \
        CHANNEL_IRQ0_RAW_CH_RX_GRD_MAX_ERR_MASK         | \
        CHANNEL_IRQ0_RAW_CH_RX_GRD_NO_EOP_ERR_MASK      | \
        CHANNEL_IRQ0_RAW_CH_RX_GRD_NO_SOP_ERR_MASK      | \
        CHANNEL_IRQ0_RAW_CH_RX_FIFO_FULL_ERR_MASK       | \
        CHANNEL_IRQ0_RAW_CH_TX_GRD_MAX_ERR_MASK         | \
        CHANNEL_IRQ0_RAW_CH_TX_GRD_NO_EOP_ERR_MASK      | \
        CHANNEL_IRQ0_RAW_CH_TX_GRD_NO_SOP_ERR_MASK      | \
        CHANNEL_IRQ0_RAW_CH_TX_CMD_FIFO_FULL_ERR_MASK   | \
        CHANNEL_IRQ0_RAW_CH_TX_FIFO_FULL_ERR_MASK       | \
        CHANNEL_IRQ0_RAW_LTSSM_REC_IDL_MASK             | \
        CHANNEL_IRQ0_RAW_LTSSM_REC_ACT_MASK  | \
        CHANNEL_IRQ0_RAW_LTSSM_U0_MASK       | \
/*        CHANNEL_IRQ0_RAW_LTSSM_POL_IDL_MASK  | */ \
/*        CHANNEL_IRQ0_RAW_LTSSM_POL_ACT_MASK  | */ \
        CHANNEL_IRQ0_RAW_LTSSM_INACT_MASK    | \
        CHANNEL_IRQ0_RAW_LTSSM_DIS           | \
        CHANNEL_IRQ0_RAW_CH_READY_MASK       | \
        CHANNEL_IRQ0_RAW_CH_RX_READY_MASK    | \
        CHANNEL_IRQ0_RAW_CH_TX_READY_MASK)
/*
        CHANNEL_IRQ0_RAW_LTSSM_REC_IDL_MASK  // channel up, recovery almost complete
        CHANNEL_IRQ0_RAW_LTSSM_REC_ACT_MASK  // channel up, recovery in progress
        CHANNEL_IRQ0_RAW_LTSSM_U0_MASK       // channel up, regular state
        CHANNEL_IRQ0_RAW_LTSSM_POL_IDL_MASK  // channel down initializing almost down
        CHANNEL_IRQ0_RAW_LTSSM_POL_ACT_MASK  // channel down, initializing
        CHANNEL_IRQ0_RAW_LTSSM_INACT_MASK    // channel down, needs disable + enable to bring it back up
        CHANNEL_IRQ0_RAW_LTSSM_DIS           // channel in the disable state
        CHANNEL_IRQ0_RAW_CH_READY_MASK       // link + Tx + Rx all ready
        CHANNEL_IRQ0_RAW_CH_RX_READY_MASK    // Rx ready
        CHANNEL_IRQ0_RAW_CH_TX_READY_MASK    // Tx ready
*/

#define MCA_HAL_CHANNEL_READY_MASK  (CHANNEL_IRQ0_RAW_CH_READY_MASK | CHANNEL_IRQ0_RAW_CH_RX_READY_MASK | CHANNEL_IRQ0_RAW_CH_TX_READY_MASK)

#define MCA_HAL_INTERRUPT_MASK \
    ( SECONDARY_INT_MCA_CORE_INT_MSK        \
    | SECONDARY_INT_MCA_CHANNEL_0_INT_MSK   \
    | SECONDARY_INT_MCA_CHANNEL_1_INT_MSK   \
    | SECONDARY_INT_MCA_CHANNEL_2_INT_MSK   \
    | SECONDARY_INT_MCA_CHANNEL_3_INT_MSK   \
    | SECONDARY_INT_MCA_CHANNEL_4_INT_MSK   \
    | SECONDARY_INT_MCA_CHANNEL_5_INT_MSK   \
    )

#define MCA_DISABLE_TIME        (10)        // MCA disable wait time 10ms

typedef void (*DisableHandlers)(void);      // Disable timer handler type definition

static const uint32_t MCA_interrupt_ChannelMask[] =
{
    SECONDARY_INT_MCA_CHANNEL_0_INT_MSK,
    SECONDARY_INT_MCA_CHANNEL_1_INT_MSK,
    SECONDARY_INT_MCA_CHANNEL_2_INT_MSK,
    SECONDARY_INT_MCA_CHANNEL_3_INT_MSK,
    SECONDARY_INT_MCA_CHANNEL_4_INT_MSK,
    SECONDARY_INT_MCA_CHANNEL_5_INT_MSK
};

// Data Types #####################################################################################

struct mcaChannelInfo
{
    enum MCA_ChannelStatus channelStatus;  // the current status of this MCA channel

    // status change callback pointers
    McaChannelStatusChangeCallback channelStatusCallback;
    // Error inform callback pointers
    McaChannelErrorCallback errorCallback;

    uint32_t channelIrqPendingOld;  // to prevent logs kill system
};

struct mcaControl
{
    uint32_t mcaDnProcessing    : 1;        // mca dn is under processing
    uint32_t mcaUpRequest       : 1;        // mca up request is waiting to process
};

volatile bb_chip_s * const mcaBbChip = (volatile void * const)(bb_chip_s_ADDRESS);
// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct
{
    TIMING_TimerHandlerT mcaDisableTimer[NUM_OF_MCA_CHANNEL_NUMBER];
    // specific info on each channel
    struct mcaChannelInfo mcaChannel[NUM_OF_MCA_CHANNEL_NUMBER];
    // mca control flags
    struct mcaControl control[NUM_OF_MCA_CHANNEL_NUMBER];

    bool mcaTxEnabled;              // true if the MCA module is enabled
    bool mcaRxEnabled;              // true if the MCA module is enabled
} mcaContext;

// Static Function Declarations ###################################################################

static void McaHal_ChannelIrqHandler(enum MCA_ChannelNumber channelNumber);
static void MCA_CoreConfigure(void);
static enum MCA_ChannelStatus McaHal_GetChannelStatus(enum MCA_ChannelNumber channelNumber, uint32_t channelIrq);
static void MCA_ConfigureMcupPldMaxCycles(enum MCA_ChannelNumber channelNumber);
static void MCA_DisableTimerHandler(enum MCA_ChannelNumber channelNumber);
static void MCA_DisableTimerHandler_CPU_COMM(void);
static void MCA_DisableTimerHandler_DP(void);
static void MCA_DisableTimerHandler_USB3(void);
static void MCA_DisableTimerHandler_GE(void);
static void MCA_DisableTimerHandler_GMII(void);
static void MCA_DisableTimerHandler_RS232(void);
static void MCA_ChannelStatusCallback(enum MCA_ChannelNumber channelNumber, enum MCA_ChannelStatus channelStatus);
static void MCA_ChannelCheckError(enum MCA_ChannelNumber channelNumber, uint32_t channelPendingIrq);
static void MCA_ChannelDisable(enum MCA_ChannelNumber channelNumber);
static void MCA_ChannelLinkSetup(enum MCA_ChannelNumber channelNumber);
static void MCA_ChannelPrintFifoLevel(void);


static const DisableHandlers disableTimerHandlers[NUM_OF_MCA_CHANNEL_NUMBER] =
{
    [MCA_CHANNEL_NUMBER_CPU_COMM] = MCA_DisableTimerHandler_CPU_COMM,
    [MCA_CHANNEL_NUMBER_DP] = MCA_DisableTimerHandler_DP,
    [MCA_CHANNEL_NUMBER_USB3] = MCA_DisableTimerHandler_USB3,
    [MCA_CHANNEL_NUMBER_GE] = MCA_DisableTimerHandler_GE,
    [MCA_CHANNEL_NUMBER_GMII] = MCA_DisableTimerHandler_GMII,
    [MCA_CHANNEL_NUMBER_RS232] = MCA_DisableTimerHandler_RS232,
};

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
void MCA_Init( void )
{
    for (uint8_t i = MCA_CHANNEL_NUMBER_CPU_COMM; i < NUM_OF_MCA_CHANNEL_NUMBER; i++)
    {
        mcaContext.mcaDisableTimer[i] = TIMING_TimerRegisterHandler(
            disableTimerHandlers[i], false, MCA_DISABLE_TIME);
    }
    MCA_StatInit();
    TOPLEVEL_setPollingMask(SECONDARY_INT_MCA_CORE_INT_MSK);
    mcaFifoPrintIcmdTimer = TIMING_TimerRegisterHandler(
            MCA_ChannelPrintFifoLevel, true, 1);
}


//#################################################################################################
// MCA Rx enable function
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called post MCA_Init and can be recalled.
//
//#################################################################################################
void MCA_EnableRx(void)
{
    if (!mcaContext.mcaRxEnabled)
    {
        ilog_MCA_COMPONENT_0(ILOG_MINOR_EVENT, MCA_ENABLE_RX);
        mcaContext.mcaRxEnabled = true;
        bb_top_ApplyResetMcaRx(false);      // MCA Rx reset is cleared
    }
}

//#################################################################################################
// MCA Tx enable function
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called post MCA_Init and can be recalled.
//
//#################################################################################################
void MCA_EnableTx(void)
{
    if (!mcaContext.mcaTxEnabled)
    {
        ilog_MCA_COMPONENT_0(ILOG_MINOR_EVENT, MCA_ENABLE_TX);
        mcaContext.mcaTxEnabled = true;
        bb_top_ApplyResetMcaTx(false);      // MCA Tx reset is cleared
        MCA_CoreConfigure();
        MCA_ControlStatsMonitorCore(true);
    }
}


//#################################################################################################
// Disable the MCA Tx
//
// Parameters:
// Return:
// Assumptions:
//      * MCA Tx is disabled before MCA Rx
//
//#################################################################################################
void MCA_DisableTx(void)
{
    ilog_MCA_COMPONENT_0(ILOG_MINOR_EVENT, MCA_DISABLE_TX);

    mcaContext.mcaTxEnabled = false;

    bb_top_ApplyResetMcaTx(true);                   // put the MCA Tx core into reset

    TOPLEVEL_clearPollingMask(MCA_HAL_INTERRUPT_MASK);   // make sure all MCA interrupts are disabled
//    LEON_ClearIrq2Bits(MCA_HAL_INTERRUPT_MASK);

    MCA_ControlStatsMonitorCore(false);

    for(uint8_t channelNumber = MCA_CHANNEL_NUMBER_CPU_COMM; channelNumber < NUM_OF_MCA_CHANNEL_NUMBER; channelNumber++)
    {
        mcaContext.mcaChannel[channelNumber].channelStatus = MCA_CHANNEL_STATUS_CHANNEL_DISABLED;
        MCA_ControlStatsMonitorChannel(channelNumber, false);
    }
}

//#################################################################################################
// Disable the MCA Rx
//
// Parameters:
// Return:
// Assumptions:
//      * MCA Tx is disabled before MCA Rx
//
//#################################################################################################
void MCA_DisableRx(void)
{
    ilog_MCA_COMPONENT_0(ILOG_MINOR_EVENT, MCA_DISABLE_RX);
    mcaContext.mcaRxEnabled = false;
    bb_top_ApplyResetMcaRx(true);                   // put the MCA Tx core into reset
}


//#################################################################################################
// One time init of the given MCA channel.  Initializes software structures used for this
// MCA channel
//
// Parameters:
//      channelNumber       - MCA channel to configure
//      statusCallback      - the function to call whenever the channel status changes
//
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_ChannelInit(
    enum MCA_ChannelNumber channelNumber,
    McaChannelStatusChangeCallback statusCallback,
    McaChannelErrorCallback errorCallback)
{
    mcaContext.mcaChannel[channelNumber].channelStatusCallback = statusCallback;
    mcaContext.mcaChannel[channelNumber].errorCallback = errorCallback;
}


//#################################################################################################
// Enable the Link between the Lex and Rex side of the channel.  Will call the channel status
// callback previously registered in MCA_ChannelInit when there is a change of channel state
//
// Parameters:
//      channelNumber       - MCA channel to configure
//
// Return:
// Assumptions:
//
//#################################################################################################
static void MCA_ChannelLinkSetup(enum MCA_ChannelNumber channelNumber)
{
    if (mcaContext.mcaRxEnabled == false || mcaContext.mcaTxEnabled == false)
    {
        ilog_MCA_COMPONENT_1(ILOG_FATAL_ERROR, MCA_CHANNEL_LINK_NOT_ENABLED, channelNumber);
        return;
    }

    ilog_MCA_COMPONENT_1(ILOG_USER_LOG, MCA_CHANNEL_LINK_SETUP, channelNumber);

    MCA_ControlStatsMonitorChannel(channelNumber, true);

    volatile channel_s *mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[channelNumber].s);

    // make sure the retry buffer is set for this channel
    mcaChannel->tx.s.control0.s.rtry_buf.dw = channelNumber;

    // enable and clear the interrupts we want to poll
    mcaChannel->irq0.s.pending.dw = channel_irq0_pending_WRITEMASK; // clear all pending interupts
    mcaChannel->irq0.s.enable.dw  = MCA_HAL_CHANNEL_IRQ_ENABLES;    // enable only the ones we are interested in

    TOPLEVEL_setPollingMask(MCA_interrupt_ChannelMask[channelNumber]);

    // TODO: add error checking - ie if link is already enabled?
    mcaChannel->control0.bf.lnk_en = 1;
    mcaChannel->control0.bf.rx_en  = 0;   // make sure Rx and Tx are off
    mcaChannel->control0.bf.tx_en  = 0;

    // defaults: packet mode, drop on packet error
    mcaChannel->rx.s.control0.s.dp.bf.pkt_strm = 1;
    mcaChannel->rx.s.control0.s.dp.bf.drp_on_pkt_err = 1;

    // see email thread in U:\Projects\Blackbird\Firmware\Documents\emails\MCA_changes_rev01.htm
    // for when this was requested
    mcaChannel->link.s.control0.s.ltssm.bf.crd_lmt_en           = 1;
    mcaChannel->link.s.control0.s.ltssm.bf.pnd_lmt_en           = 1;
    mcaChannel->link.s.control0.s.ltssm.bf.u0rectimeout_lmt_en  = 1;
    // mcaChannel->link.s.control0.s.ltssm.bf.pnd_lmt_en           = 0;
    // mcaChannel->link.s.control0.s.ltssm.bf.u0rectimeout_lmt_en  = 0;
    mcaChannel->link.s.control0.s.ltssm.bf.u0ltimeout_lmt_en    = 1;
    mcaChannel->link.s.control0.s.ltssm.bf.rec_idl_lmt_en       = 1;
    mcaChannel->link.s.control0.s.ltssm.bf.rec_idl_rpt_en       = 1;
    mcaChannel->link.s.control0.s.ltssm.bf.rec_act_rpt_en       = 1;
    mcaChannel->link.s.control0.s.ltssm.bf.pol_idl_lmt_en       = 1;
    mcaChannel->link.s.control0.s.ltssm.bf.pol_idl_rpt_en       = 1;
    mcaChannel->link.s.control0.s.ltssm.bf.pol_act_rpt_en       = 1;

    // override the defaults with channel specific settings

    if (channelNumber == MCA_CHANNEL_NUMBER_CPU_COMM)
    {
        // test requested by Terry for Latency for CPU comm channel
        mcaChannel->link.s.control0.s.test.bf.measure_latency = 1;
    }
    else if (channelNumber == MCA_CHANNEL_NUMBER_DP)
    {
        mcaChannel->rx.s.control0.s.dp.bf.pkt_strm = 0;
        mcaChannel->rx.s.control0.s.dp.bf.drp_on_pkt_err = 0;

        mcaChannel->tx.s.control0.s.dp.bf.force_retry = 0;
        mcaChannel->tx.s.control0.s.dp.bf.block_retry = 1;
        mcaChannel->tx.s.control0.s.dp.bf.flw_ctrl_en = 0;
    }
    else if (channelNumber == MCA_CHANNEL_NUMBER_USB3)
    {
        mcaChannel->tx.s.control0.s.dp.bf.hys_en = 1;
        // mcaChannel->tx.s.control0.s.fifo_hys.dw = 0x00a00032;
        mcaChannel->tx.s.control0.s.fifo_hys.dw = 0x03E80032;

        // Settings for RX
        mcaChannel->rx.s.control0.s.dp.bf.pkt_strm = 0;
        mcaChannel->rx.s.control0.s.ntfy_mode.dw = 16;
    }

    MCA_ConfigureMcupPldMaxCycles(channelNumber);
}


//#################################################################################################
// Enables the Tx and Rx sides of the channel.  Assumes the link is already established.
// Will call the channel status callback previously registered in MCA_ChannelInit when there is a
// change of channel state
//
// Parameters:
//      channelNumber       - MCA channel to configure
//
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_ChannelTxRxSetup(enum MCA_ChannelNumber channelNumber)
{
    if (mcaContext.mcaRxEnabled == false || mcaContext.mcaTxEnabled == false)
    {
        ilog_MCA_COMPONENT_1(ILOG_FATAL_ERROR, MCA_CHANNEL_TX_RX_NOT_ENABLED, channelNumber);
        return;
    }

    volatile channel_s *mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[channelNumber].s);

    // TODO: add error checking?  ie if link wasn't enabled yet?
    mcaChannel->control0.bf.lnk_en = 1;
    mcaChannel->control0.bf.rx_en  = 1;
    mcaChannel->control0.bf.tx_en  = 1;
}

//#################################################################################################
// This is wrapper function to control MCA_ChannelLinkSetup and MCA_ChannelDisable
//
// Parameters:
//      channelNumber       - MCA channel to configure
//
// Return:
// Assumptions:
//
//#################################################################################################
bool MCA_ChannelLinkUp(enum MCA_ChannelNumber channelNumber)
{
    bool linkUpSuccess = true;
    enum MCA_ChannelStatus status = MCA_GetChannelStatus(channelNumber);
    ilog_MCA_COMPONENT_2(ILOG_MAJOR_EVENT, MCA_CHANNEL_LINKUP, channelNumber, status);

    if(mcaContext.control[channelNumber].mcaDnProcessing)
    {
        ilog_MCA_COMPONENT_1(ILOG_MAJOR_EVENT, MCA_CHANNEL_LINKUP_REQ, channelNumber);
        mcaContext.control[channelNumber].mcaUpRequest = true;
    }
    else if(status == MCA_CHANNEL_STATUS_CHANNEL_DISABLED)
    {
        MCA_ChannelLinkSetup(channelNumber);
    }
    else
    {
        ilog_MCA_COMPONENT_2(ILOG_MAJOR_ERROR, MCA_UP_FAILED, channelNumber, status);
        linkUpSuccess = false;
    }

    return linkUpSuccess;
}

//#################################################################################################
// This is wrapper function to control MCA_ChannelLinkSetup and MCA_ChannelDisable
//
// Parameters:
//      channelNumber       - MCA channel to configure
//
// Return:
// Assumptions:
//
//#################################################################################################
bool MCA_ChannelLinkDn(enum MCA_ChannelNumber channelNumber)
{
    bool linkDnSuccess = true;

    if (mcaContext.mcaRxEnabled == false || mcaContext.mcaTxEnabled == false)
    {
        ilog_MCA_COMPONENT_1(ILOG_MAJOR_ERROR, MCA_CHANNEL_DISABLE_NOT_ENABLED, channelNumber);
        linkDnSuccess = false;
    }
    else
    {
        enum MCA_ChannelStatus status = MCA_GetChannelStatus(channelNumber);
        ilog_MCA_COMPONENT_2(ILOG_MAJOR_EVENT, MCA_CHANNEL_LINKDN, channelNumber, status);

        mcaContext.control[channelNumber].mcaUpRequest = false;     // clear waiting request
        MCA_ChannelDisable(channelNumber);

        if(status != MCA_CHANNEL_STATUS_CHANNEL_DISABLED && !mcaContext.control[channelNumber].mcaDnProcessing)
        {
            mcaContext.control[channelNumber].mcaDnProcessing = true;
            TIMING_TimerStart(mcaContext.mcaDisableTimer[channelNumber]);
        }
        else
        {
            ilog_MCA_COMPONENT_3(ILOG_MAJOR_ERROR, MCA_DN_FAILED,
                channelNumber, status, mcaContext.control[channelNumber].mcaDnProcessing);

            linkDnSuccess = false;
        }
    }

    return linkDnSuccess;
}

//#################################################################################################
// Gets the current status of the MCA channel
//
// Parameters:
//
// Return:
// Assumptions:
//
//#################################################################################################
enum MCA_ChannelStatus MCA_GetChannelStatus(enum MCA_ChannelNumber channelNumber)
{
    volatile channel_s *mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[channelNumber].s);
    uint32_t channelIrq = mcaChannel->irq0.s.raw.dw & MCA_HAL_CHANNEL_IRQ_ENABLES;

    // return the current status
    return (McaHal_GetChannelStatus(channelNumber, channelIrq));
}


//#################################################################################################
// MCA channel 0 IRQ handler
//
// Parameters:
//
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_CoreIrq(void)
{
    volatile core_s *mcaCore = (volatile core_s *)&(mcaBbChip->mca_core.s);

    uint32_t corePendingIrq    = mcaCore->irq0.s.pending.dw;
    uint32_t coreEnaIrq        = mcaCore->irq0.s.enable.dw;
    corePendingIrq &= coreEnaIrq;
    mcaCore->irq0.s.pending.dw = corePendingIrq;

    uint32_t coreIrq = (mcaCore->irq0.s.raw.dw & coreEnaIrq);


    if (coreIrq & MCA_HAL_CORE_IRQ_GUARD_MASK)
    {
        ilog_MCA_COMPONENT_1(ILOG_FATAL_ERROR, CORE_GUARD_ERROR, coreIrq);
        ifail_MCA_COMPONENT_0(MCA_CORE_GUARD_ERROR);

    }
    ilog_MCA_COMPONENT_2(ILOG_USER_LOG, MCA_CORE_INTERRUPT, corePendingIrq, coreIrq);
}


//#################################################################################################
// MCA channel 0 IRQ handler
//
// Parameters:
//
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_Channel0Irq(void)
{
    McaHal_ChannelIrqHandler(MCA_CHANNEL_NUMBER_CPU_COMM);
}


//#################################################################################################
// MCA channel 0 IRQ handler
//
// Parameters:
//
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_Channel1Irq(void)
{
    McaHal_ChannelIrqHandler(MCA_CHANNEL_NUMBER_DP);
}


//#################################################################################################
// MCA channel 0 IRQ handler
//
// Parameters:
//
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_Channel2Irq(void)
{
    McaHal_ChannelIrqHandler(MCA_CHANNEL_NUMBER_USB3);
}


//#################################################################################################
// MCA channel 0 IRQ handler
//
// Parameters:
//
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_Channel3Irq(void)
{
    McaHal_ChannelIrqHandler(MCA_CHANNEL_NUMBER_GE);
}


//#################################################################################################
// MCA channel 0 IRQ handler
//
// Parameters:
//
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_Channel4Irq(void)
{
    McaHal_ChannelIrqHandler(MCA_CHANNEL_NUMBER_GMII);
}


//#################################################################################################
// MCA channel 0 IRQ handler
//
// Parameters:
//
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_Channel5Irq(void)
{
    McaHal_ChannelIrqHandler(MCA_CHANNEL_NUMBER_RS232);
}


//#################################################################################################
// Drop MCA Tx packet
//
// Parameters: drop (true: drop, false: pass)
//
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_ChannelDropTxPacket(enum MCA_ChannelNumber channelNumber, bool drop)
{
    volatile channel_s *mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[channelNumber].s);
    mcaChannel->tx.s.control0.s.dp.bf.drp_on_sop = drop;
}

//#################################################################################################
// This is used to recover a channel which is inactive.
//
// Parameters: channel number, enable or disable
//
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_ChannelLinkControl(enum MCA_ChannelNumber channelNumber, bool enable)
{
    volatile channel_s *mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[channelNumber].s);
    mcaChannel->control0.bf.lnk_en = enable;
}


// Component Scope Function Definitions ###########################################################
//#################################################################################################
// sets the rd2clr_config for mca core registers so it will automatically clear the stat when read
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_coreSetReadClearStats(void)
{
    volatile core_s *mcaCore = (volatile core_s *)&(mcaBbChip->mca_core.s);
    mcaCore->tx.s.stats0.s.rd2clr_config.dw = core_tx_stats0_rd2clr_config_WRITEMASK;
    mcaCore->rx.s.stats0.s.rd2clr_config.dw = core_rx_stats0_rd2clr_config_WRITEMASK;
}

//#################################################################################################
// sets the rd2clr_config for mca channel registers so it will automatically clear the stat when read
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void MCA_channelSetReadClearStats(enum MCA_ChannelNumber channelNumber)
{
    volatile channel_s *mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[channelNumber].s);
    // make sure all the stats are cleared automatically once read
    mcaChannel->link.s.stats0.s.rd2clr_config.dw            = channel_link_stats0_rd2clr_config_WRITEMASK;
    mcaChannel->rx.s.stats0.s.rd2clr_config.dw              = channel_rx_stats0_rd2clr_config_WRITEMASK;
    mcaChannel->tx.s.stats0.s.rd2clr_config.dw              = channel_tx_stats0_rd2clr_config_WRITEMASK;
    mcaChannel->test_mode.s.tx.s.stats0.s.rd2clr_config.dw  = channel_test_mode_tx_stats0_rd2clr_config_WRITEMASK;
    mcaChannel->test_mode.s.rx.s.stats0.s.rd2clr_config.dw  = channel_test_mode_rx_stats0_rd2clr_config_WRITEMASK;
}


// Static Function Definitions ####################################################################

//#################################################################################################
// Handles the specified channel interrupt
//
// Parameters:
// Return:
// Assumptions:
//      * Called after Phy link is established
//
//#################################################################################################
static void McaHal_ChannelIrqHandler(enum MCA_ChannelNumber channelNumber)
{
    volatile channel_s *mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[channelNumber].s);

    // get and clear the interrupt(s) that caused us to be triggered
    uint32_t channelPendingIrq    = mcaChannel->irq0.s.pending.dw;
    uint32_t channelEnableIrq     = mcaChannel->irq0.s.enable.dw;
    channelPendingIrq &= channelEnableIrq;
    mcaChannel->irq0.s.pending.dw = channelPendingIrq;

    uint32_t channelIrq = mcaChannel->irq0.s.raw.dw & channelEnableIrq;

    MCA_ChannelCheckError(channelNumber, channelPendingIrq);

    // bool guardErrorAssert = false;
//     if (channelPendingIrq & CHANNEL_IRQ0_RAW_CH_RX_FIFO_FULL_ERR_MASK)
//     {
//         UART_printf("MCA_CHANNEL[%d]_RX_FIFO_FULL_ERR = 0x%x, pfifo = 0x%x nfifo = 0x%x\n",
//             channelNumber,
//              mcaChannel->rx.s.stats0.s.dp_fifo_fifo_full_err.bf.cnt,
//             mcaChannel->rx.s.stats0.s.dp_pfifo.bf.cnt,
//             mcaChannel->rx.s.stats0.s.dp_nfifo.bf.cnt);
//         iassert_MCA_COMPONENT_0(false, MCA_RX_FIFO_FULL_ERROR);
//
//     }
    // if (channelPendingIrq & CHANNEL_IRQ0_RAW_CH_RX_GRD_NO_SOP_ERR_MASK)
    // {
    //     UART_printf("MCA_CHANNEL[%d]_RX_GRD_NO_SOP_ERR = 0x%x, pfifo = 0x%x nfifo = 0x%x\n",
    //         channelNumber,
    //         mcaChannel->rx.s.stats0.s.dp_grd_no_sop_err.bf.cnt,
    //         mcaChannel->rx.s.stats0.s.dp_pfifo.bf.cnt,
    //         mcaChannel->rx.s.stats0.s.dp_nfifo.bf.cnt);

    //     guardErrorAssert = true;
    // }
    // if (channelPendingIrq & CHANNEL_IRQ0_RAW_CH_RX_GRD_NO_EOP_ERR_MASK)
    // {
    //     UART_printf("MCA_CHANNEL[%d]_RX_GRD_NO_EOP_ERR = 0x%x, pfifo = 0x%x nfifo = 0x%x\n",
    //         channelNumber,
    //         mcaChannel->rx.s.stats0.s.dp_grd_no_eop_err.bf.cnt,
    //         mcaChannel->rx.s.stats0.s.dp_pfifo.bf.cnt,
    //         mcaChannel->rx.s.stats0.s.dp_nfifo.bf.cnt);

    //     guardErrorAssert = true;
    // }
    // if (channelPendingIrq & CHANNEL_IRQ0_RAW_CH_RX_GRD_MAX_ERR_MASK)
    // {
    //     UART_printf("MCA_CHANNEL[%d]_RX_GRD_MAX_ERR = 0x%x, pfifo = 0x%x nfifo = 0x%x\n",
    //         channelNumber,
    //         mcaChannel->rx.s.stats0.s.dp_grd_max_err.bf.cnt,
    //         mcaChannel->rx.s.stats0.s.dp_pfifo.bf.cnt,
    //         mcaChannel->rx.s.stats0.s.dp_nfifo.bf.cnt);

    //     guardErrorAssert = true;
    // }
    // if (channelPendingIrq & CHANNEL_IRQ0_RAW_CH_TX_FIFO_FULL_ERR_MASK)
    // {
    //     UART_printf("MCA_CHANNEL[%d]_TX_FIFO_FULL_ERR = 0x%x, pfifo = 0x%x nfifo = 0x%x\n",
    //         channelNumber,
    //         mcaChannel->tx.s.stats0.s.dp_fifo_fifo_full_err.bf.cnt,
    //         mcaChannel->tx.s.stats0.s.dp_pfifo.bf.cnt,
    //         mcaChannel->tx.s.stats0.s.dp_nfifo.bf.cnt);
    // }
    // if (channelPendingIrq & CHANNEL_IRQ0_RAW_CH_TX_GRD_NO_SOP_ERR_MASK)
    // {
    //     UART_printf("MCA_CHANNEL[%d]_TX_GRD_NO_SOP_ERR = 0x%x, pfifo = 0x%x nfifo = 0x%x\n",
    //         channelNumber,
    //         mcaChannel->tx.s.stats0.s.dp_grd_no_sop_err.bf.cnt,
    //         mcaChannel->tx.s.stats0.s.dp_pfifo.bf.cnt,
    //         mcaChannel->tx.s.stats0.s.dp_nfifo.bf.cnt);

    //     guardErrorAssert = true;
    // }
    // if (channelPendingIrq & CHANNEL_IRQ0_RAW_CH_TX_GRD_NO_EOP_ERR_MASK)
    // {
    //     UART_printf("MCA_CHANNEL[%d]_TX_GRD_NO_EOP_ERR = 0x%x, pfifo = 0x%x nfifo = 0x%x\n",
    //         channelNumber,
    //         mcaChannel->tx.s.stats0.s.dp_grd_no_eop_err.bf.cnt,
    //         mcaChannel->tx.s.stats0.s.dp_pfifo.bf.cnt,
    //         mcaChannel->tx.s.stats0.s.dp_nfifo.bf.cnt);

    //     guardErrorAssert = true;
    // }
    // if (channelPendingIrq & CHANNEL_IRQ0_RAW_CH_TX_GRD_MAX_ERR_MASK)
    // {
    //     UART_printf("MCA_CHANNEL[%d]_TX_GRD_MAX_ERR = 0x%x, pfifo = 0x%x nfifo = 0x%x\n",
    //         channelNumber,
    //         mcaChannel->tx.s.stats0.s.dp_grd_max_err.bf.cnt,
    //         mcaChannel->tx.s.stats0.s.dp_pfifo.bf.cnt,
    //         mcaChannel->tx.s.stats0.s.dp_nfifo.bf.cnt);

    //     guardErrorAssert = true;
    // }

//   if (guardErrorAssert && ((channelNumber == MCA_CHANNEL_NUMBER_CPU_COMM) || (channelNumber == MCA_CHANNEL_NUMBER_USB3)))
//   {
//        iassert_MCA_COMPONENT_0(false, MCA_GUARD_ERROR);
//   }

    enum MCA_ChannelStatus channelStatus = McaHal_GetChannelStatus(channelNumber, channelIrq);
    MCA_ChannelStatusCallback(channelNumber, channelStatus);

}


//#################################################################################################
// Gets the status of the specified channel
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static enum MCA_ChannelStatus McaHal_GetChannelStatus(enum MCA_ChannelNumber channelNumber, uint32_t channelIrq)
{
    enum MCA_ChannelStatus channelStatus = MCA_CHANNEL_STATUS_CHANNEL_DISABLED;
    if (channelIrq & CHANNEL_IRQ0_RAW_LTSSM_DIS)
    {
        channelStatus = MCA_CHANNEL_STATUS_CHANNEL_DISABLED;
    }
    else if (channelIrq & CHANNEL_IRQ0_RAW_LTSSM_INACT_MASK)
    {
        // if the link is in the inactive state, we want to say the channel
        // is disabled, regardless of anything else
        channelStatus = MCA_CHANNEL_STATUS_LINK_DOWN;
    }
    else if ( (channelIrq & MCA_HAL_CHANNEL_READY_MASK) == MCA_HAL_CHANNEL_READY_MASK)
    {
        // channel is ready! (Link is assumed to be in U0)
        channelStatus = MCA_CHANNEL_STATUS_CHANNEL_READY;
    }
    else if (channelIrq &
        (CHANNEL_IRQ0_RAW_LTSSM_U0_MASK |
         CHANNEL_IRQ0_RAW_LTSSM_REC_IDL_MASK |
         CHANNEL_IRQ0_RAW_LTSSM_REC_ACT_MASK) )
    {
        channelStatus = MCA_CHANNEL_STATUS_LINK_ACTIVE;
    }

    return(channelStatus);
}

//#################################################################################################
// Disables the given MCA channel
//
// Parameters:
//      channelNumber       - MCA channel to disable
//
// Return:
// Assumptions:
//
//#################################################################################################
static void MCA_ChannelDisable(enum MCA_ChannelNumber channelNumber)
{
    MCA_ControlStatsMonitorChannel(channelNumber, false);

    if (mcaContext.mcaRxEnabled == false || mcaContext.mcaTxEnabled == false)
    {
        ilog_MCA_COMPONENT_1(ILOG_FATAL_ERROR, MCA_CHANNEL_DISABLE_NOT_ENABLED, channelNumber);
        return;
    }

    volatile channel_s *mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[channelNumber].s);

    if (mcaChannel->control0.bf.lnk_en == 0)
    {
        uint32_t channelIrq = mcaChannel->irq0.s.raw.dw;

        // link is already disabled - call the status saying that we are disabled,
        // so the client code doesn't have to worry about this situation
        enum MCA_ChannelStatus channelStatus = McaHal_GetChannelStatus(channelNumber, channelIrq);

        if (channelStatus == MCA_CHANNEL_STATUS_CHANNEL_DISABLED)
        {
            // Update the status
            mcaContext.mcaChannel[channelNumber].channelStatus = channelStatus;
        }
    }

    mcaChannel->control0.bf.lnk_en = 0;
    mcaChannel->control0.bf.rx_en  = 0;
    mcaChannel->control0.bf.tx_en  = 0;

    // enable and clear the interrupts we want to see
    mcaChannel->irq0.s.enable.dw  = 0;
    mcaChannel->irq0.s.pending.dw = MCA_HAL_CHANNEL_IRQ_ENABLES;

    TOPLEVEL_clearPollingMask(MCA_interrupt_ChannelMask[channelNumber]);
}

//#################################################################################################
// sets the mcup_pld_max_cycles for all mca channels
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void MCA_ConfigureMcupPldMaxCycles(enum MCA_ChannelNumber channelNumber)
{
    volatile channel_s *mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[channelNumber].s);
    mcaChannel->tx.s.control0.s.dp.bf.mcup_pld_max_cycles = 0x7;
    switch(channelNumber)
    {
        case MCA_CHANNEL_NUMBER_CPU_COMM:
            mcaChannel->link.s.control0.s.test.bf.dp_tx_hdr_ovrd_time = 0x1;
            break;

        case MCA_CHANNEL_NUMBER_DP:
#if !defined BB_ISO && !defined BB_USB
            mcaChannel->tx.s.control0.s.dp.bf.mcup_pld_max_cycles = 0xa;
#endif
            break;

        case MCA_CHANNEL_NUMBER_USB3:
            if(bb_core_getLinkMode() == CORE_LINK_MODE_AQUANTIA)
            {
#if !defined BB_ISO && !defined BB_USB
                mcaChannel->tx.s.control0.s.dp.bf.mcup_pld_max_cycles = 0xa;
#endif
            }
            else if(bb_core_getLinkMode() == CORE_LINK_MODE_ONE_LANE_SFP_PLUS)
            {
#ifdef BB_ISO
                mcaChannel->tx.s.control0.s.dp.bf.mcup_pld_max_cycles = 0x8;
#endif
            }
            break;

        case MCA_CHANNEL_NUMBER_GE:
#if !defined BB_ISO && !defined BB_USB
            if(bb_top_IsDeviceLex())
            {
                mcaChannel->tx.s.control0.s.dp.bf.mcup_pld_max_cycles = 0x3;
            }
#endif
            break;

        case MCA_CHANNEL_NUMBER_GMII:
#if !defined BB_ISO && !defined BB_USB
            if(bb_top_IsDeviceLex())
            {
                mcaChannel->tx.s.control0.s.dp.bf.mcup_pld_max_cycles = 0x3;
            }
#endif
            break;

        case MCA_CHANNEL_NUMBER_RS232:
#if !defined BB_ISO && !defined BB_USB
            if(bb_top_IsDeviceLex())
            {
                mcaChannel->tx.s.control0.s.dp.bf.mcup_pld_max_cycles = 0x3;
            }
#endif
            break;

        case NUM_OF_MCA_CHANNEL_NUMBER:
        default:
            // TODO: add assert
            break;
    }

}


//#################################################################################################
// Configures MCA core registers
//
// Parameters:
// Return:
// Assumptions: It can't be separated by Rx and Tx. (Tx should be enabled to work)
//
//#################################################################################################
void MCA_CoreConfigure(void)
{
    volatile core_s *mcaCore = (volatile core_s *)&(mcaBbChip->mca_core.s);

    mcaCore->tx.s.control0.s.dp.bf.stats_chid_en = 0x3F;
    mcaCore->rx.s.control0.s.dp.bf.stats_chid_en = 0x3F;

    mcaCore->control0.bf.dp_rx_en = 1;
    mcaCore->control0.bf.dp_tx_en = 1;


    if(bb_core_getLinkMode() == CORE_LINK_MODE_AQUANTIA)
    {
        mcaCore->link.s.control0.s.ltssm_rec_tmr.s.act_rpt.bf.lmt       = 0x14;
        mcaCore->link.s.control0.s.ltssm_rec_tmr.s.idle_rpt.bf.lmt      = 0x14;
        mcaCore->link.s.control0.s.ltssm_rec_tmr.s.idle.bf.lmt          = 0x1312D;     //500us
        mcaCore->link.s.control0.s.ltssm_u0_tmr.s.u0ltimeout.bf.lmt     = 0x61b;

        // This timer sets the maximum time limit to wait for a response to be received from the remote port to a retyr-able MCUP
        mcaCore->link.s.control0.s.ltssm_u0_tmr.s.pnd.bf.lmt            = 0x00003D09;   // deault register value (approx 100us)
        // mcaCore->link.s.control0.s.ltssm_u0_tmr.s.pnd.bf.lmt            = 0x00000C35;   // pending timeout (approx 20us: Raven value)
        // mcaCore->link.s.control0.s.ltssm_u0_tmr.s.pnd.bf.lmt            = 0xffffffff;   // pending timeout endless

        mcaCore->link.s.control0.s.ltssm_u0_tmr.s.crd.bf.lmt            = 0x00000C35;   // credit timer

        // This timer sets the limit on how long to wait without seeeing received traffic (link ups/data etc) from the remote end. default is about 500us
        mcaCore->link.s.control0.s.ltssm_u0_tmr.s.u0rectimeout.bf.lmt   = 0x1312D;      // default register value (500us)
        // mcaCore->link.s.control0.s.ltssm_u0_tmr.s.u0rectimeout.bf.lmt   = 0xf43;     // approx 25us which is Raven's value
        // mcaCore->link.s.control0.s.ltssm_u0_tmr.s.u0rectimeout.bf.lmt   = 0xffffffff;
    }
    else if(bb_core_getLinkMode() == CORE_LINK_MODE_ONE_LANE_SFP_PLUS)
    {
        mcaCore->link.s.control0.s.ltssm_rec_tmr.s.act_rpt.bf.lmt       = 0xa;
        mcaCore->link.s.control0.s.ltssm_rec_tmr.s.idle_rpt.bf.lmt      = 0xa;
        mcaCore->link.s.control0.s.ltssm_rec_tmr.s.idle.bf.lmt          = 0x17d784;
        mcaCore->link.s.control0.s.ltssm_u0_tmr.s.u0ltimeout.bf.lmt     = 0x30e;
        mcaCore->link.s.control0.s.ltssm_u0_tmr.s.pnd.bf.lmt            = 0x0000061B;   // pending timeout
        mcaCore->link.s.control0.s.ltssm_u0_tmr.s.crd.bf.lmt            = 0x0000061B;   // credit timer
        mcaCore->link.s.control0.s.ltssm_u0_tmr.s.u0rectimeout.bf.lmt   = 0x7a2;
    }

    mcaCore->irq0.s.enable.dw = MCA_HAL_CORE_IRQ_ENABLES;

//    TOPLEVEL_setPollingMask(SECONDARY_INT_MCA_CORE_INT_MSK);

    // wait until the rx and tx lines are ready
    LEON_TimerValueT startTime = LEON_TimerRead();

    // Block because less complicated and should be less then 100us
    // the system link is just in the process of coming up, so not much should be going on
    while ( !(mcaCore->status0.bf.dp_rx_ready && mcaCore->status0.bf.dp_tx_ready) &&
        (LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < MCA_HAL_CORE_MAX_TX_RX_ENABLE_TIME) )
        ;  // everything done in the while loop

    ilog_MCA_COMPONENT_0(ILOG_DEBUG, MCA_ENABLE_CORE_UP);
    // assert if we are not ready
    iassert_MCA_COMPONENT_2(
        (mcaCore->status0.bf.dp_rx_ready && mcaCore->status0.bf.dp_tx_ready),
        MCA_CORE_NOT_READY, mcaCore->status0.bf.dp_rx_ready, mcaCore->status0.bf.dp_tx_ready);
}

//#################################################################################################
// Disable timer handlers for each channel
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void MCA_DisableTimerHandler_CPU_COMM(void)
{
    MCA_DisableTimerHandler(MCA_CHANNEL_NUMBER_CPU_COMM);
}
static void MCA_DisableTimerHandler_DP(void)
{
    MCA_DisableTimerHandler(MCA_CHANNEL_NUMBER_DP);
}
static void MCA_DisableTimerHandler_USB3(void)
{
    MCA_DisableTimerHandler(MCA_CHANNEL_NUMBER_USB3);
}
static void MCA_DisableTimerHandler_GE(void)
{
    MCA_DisableTimerHandler(MCA_CHANNEL_NUMBER_GE);
}
static void MCA_DisableTimerHandler_GMII(void)
{
    MCA_DisableTimerHandler(MCA_CHANNEL_NUMBER_GMII);
}
static void MCA_DisableTimerHandler_RS232(void)
{
    MCA_DisableTimerHandler(MCA_CHANNEL_NUMBER_RS232);
}

//#################################################################################################
// Disable timer common handler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void MCA_DisableTimerHandler(enum MCA_ChannelNumber channelNumber)
{
    volatile channel_s *mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[channelNumber].s);
    uint32_t channelIrq = mcaChannel->irq0.s.raw.dw & MCA_HAL_CHANNEL_IRQ_ENABLES;
    enum MCA_ChannelStatus channelStatus = McaHal_GetChannelStatus(channelNumber, channelIrq);

    iassert_MCA_COMPONENT_2(
        channelStatus == MCA_CHANNEL_STATUS_CHANNEL_DISABLED,
        MCA_DISABLE_TIMEOUT, channelNumber, channelIrq);

    ilog_MCA_COMPONENT_1(ILOG_MAJOR_EVENT, MCA_CHANNEL_LINKDN_DONE, channelNumber);

    MCA_ChannelStatusCallback(channelNumber, channelStatus);

    mcaContext.control[channelNumber].mcaDnProcessing = false;

    if(mcaContext.control[channelNumber].mcaUpRequest)
    {
        mcaContext.control[channelNumber].mcaUpRequest = false;
        MCA_ChannelLinkUp(channelNumber);
    }
}

//#################################################################################################
// Check a channel's status change and call handler for the channel
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void MCA_ChannelStatusCallback(enum MCA_ChannelNumber channelNumber,
                                      enum MCA_ChannelStatus channelStatus)
{
    if(mcaContext.mcaChannel[channelNumber].channelStatus != channelStatus)
    {
        mcaContext.mcaChannel[channelNumber].channelStatus = channelStatus;

        if (mcaContext.mcaChannel[channelNumber].channelStatusCallback != NULL)
        {
            mcaContext.mcaChannel[channelNumber].channelStatusCallback(channelStatus);
        }
    }
}

//#################################################################################################
// Check a channel's error and call handler for the channel
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void MCA_ChannelCheckError(enum MCA_ChannelNumber channelNumber, uint32_t channelPendingIrq)
{
    volatile channel_s *mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[channelNumber].s);

    ilog_MCA_COMPONENT_3(ILOG_USER_LOG, MCA_CHANNEL_INTERRUPT,
                channelNumber,
                channelPendingIrq,
                mcaChannel->irq0.s.raw.dw);

    if(mcaContext.mcaChannel[channelNumber].errorCallback != NULL)
    {
        if(channelPendingIrq == mcaContext.mcaChannel[channelNumber].channelIrqPendingOld)
        {
            if(channelPendingIrq & CHANNEL_IRQ0_RAW_CH_RX_FIFO_FULL_ERR_MASK)
            {
                ilog_MCA_COMPONENT_2(ILOG_MAJOR_ERROR, MCA_CHANNEL_RX_FIFO_FULL, channelNumber, channelPendingIrq);
                mcaChannel->irq0.s.enable.dw &= ~CHANNEL_IRQ0_RAW_CH_RX_FIFO_FULL_ERR_MASK;
                mcaContext.mcaChannel[channelNumber].errorCallback(MCA_CHANNEL_ERROR_RX_FIFO_OVERFLOW);
            }
            if(channelPendingIrq & CHANNEL_IRQ0_RAW_CH_RX_GRD_MAX_ERR_MASK)
            {
                ilog_MCA_COMPONENT_2(ILOG_MAJOR_ERROR, MCA_CHANNEL_RX_GRD_MAX_ERR, channelNumber, channelPendingIrq);
                mcaChannel->irq0.s.enable.dw &= ~CHANNEL_IRQ0_RAW_CH_RX_GRD_MAX_ERR_MASK;
                mcaContext.mcaChannel[channelNumber].errorCallback(MCA_CHANNEL_ERROR_RX_GRD_MAX_ERROR);
            }
            if(channelPendingIrq & CHANNEL_IRQ0_RAW_CH_TX_FIFO_FULL_ERR_MASK)
            {
                ilog_MCA_COMPONENT_2(ILOG_MAJOR_ERROR, MCA_CHANNEL_RX_GRD_MAX_ERR, channelNumber, channelPendingIrq);
                mcaChannel->irq0.s.enable.dw &= ~CHANNEL_IRQ0_RAW_CH_TX_FIFO_FULL_ERR_MASK;
                mcaContext.mcaChannel[channelNumber].errorCallback(MCA_CHANNEL_ERROR_TX_FIFO_FULL_ERR);
            }
        }
        else
        {
            mcaContext.mcaChannel[channelNumber].channelIrqPendingOld = channelPendingIrq;
        }
    }
}

//#################################################################################################
// Print P amd N Fifo levels for all the channels
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void MCA_ChannelPrintFifoLevel(void)
{
    volatile channel_s *mcaChannel = NULL;
    for (uint8_t i = 0; i <= 5; i++)
    {
        if(bb_top_IsDeviceLex())
        {
            mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[i].s);
            ilog_MCA_COMPONENT_3(ILOG_MAJOR_EVENT, MCA_CHANNEL_NP_FIFO, i,
                                mcaChannel->tx.s.stats0.s.dp_pfifo.bf.cnt,
                                mcaChannel->tx.s.stats0.s.dp_nfifo.bf.cnt);

            ilog_MCA_COMPONENT_2(ILOG_MAJOR_EVENT, MCA_BANDWIDTH, i,
                                mcaChannel->tx.s.stats0.s.bandwidth.bf.cnt);
        }
        else
        {
            mcaChannel = (volatile channel_s *)&(mcaBbChip->mca_channel[i].s);
            ilog_MCA_COMPONENT_3(ILOG_MAJOR_EVENT, MCA_CHANNEL_NP_FIFO, i,
                    mcaChannel->rx.s.stats0.s.dp_pfifo.bf.cnt,
                    mcaChannel->rx.s.stats0.s.dp_nfifo.bf.cnt);
        }

    }
}
