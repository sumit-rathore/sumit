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
// TODO
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################


// Includes #######################################################################################
#include <leon_timers.h>
#include <bb_chip_regs.h>
#include <dp_aux_hpd_regs.h>
#include <dp_aux.h>
#include <timing_timers.h>
#include "aux_loc.h"
#include "aux_log.h"
#include <uart.h>
// Constants and Macros ###########################################################################
#define AUX_HPD_DEBOUNCE_TIMER      100 // Debouncce time from Spec 1.4 for detecting source connect or disconnect
// Data Types #####################################################################################
struct AUX_LexIsrCtx
{
    IsrCallback isrCallback;                      // ISR callback handler
};

// Static Function Declarations ###################################################################
// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct AUX_LexIsrCtx auxLexIsrCtx;

// Exported Function Definitions ##################################################################
//#################################################################################################
// Aux Lex ISR Init function
//
// Parameters: call back handler to inform Aux ISR to Policy maker
// Return:
// Assumptions:
//#################################################################################################
void AUX_LexIsrInit(IsrCallback callback)
{
    auxLexIsrCtx.isrCallback = callback;
}

//#################################################################################################
// The main ISR for the Lex's AUX subsystem.
//
// Parameters:
// Return:
// Assumptions: AUX_LexIsrInit should be called before enabling Lex ISR
//#################################################################################################
// TODO look into handling only one IRQ per ISR call.
void AUX_LexISR(void)
{
    static LEON_TimerValueT enterTime;
    static LEON_TimerValueT preExitTime;
    static LEON_TimerValueT postExitTime;

    enterTime = LEON_TimerRead();

    // get and clear the interrupt(s) that caused us to be triggered
    uint32_t auxInts = AUX_GetPendingAuxInterrupts();

    ilog_DP_AUX_COMPONENT_1(ILOG_DEBUG, AUX_ENTERING_ISR, auxInts);

    if ((auxInts & BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_DISCONNECT_DET_MASK) ||
        (auxInts & BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_CONNECT_DET_MASK))
    {
        AUX_AckPendingAuxInterrupts(BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_DISCONNECT_DET_MASK |
            BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_CONNECT_DET_MASK);
        auxInts &= ~(BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_DISCONNECT_DET_MASK |
            BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_CONNECT_DET_MASK);

        const bool hostConnection = AUX_GetHostConnectedInfo();
        const uint8_t disconnectCount = AUX_SourceDisconnectCounter();
        const uint8_t connectCount = AUX_SourceConnectCounter();
        AUX_SourceEdgeCounterRst();

        ilog_DP_AUX_COMPONENT_3(ILOG_MAJOR_EVENT, AUX_LEX_SRC_IRQ_COUNT, 
                                connectCount, disconnectCount, hostConnection);

        if ((!hostConnection) && (disconnectCount > connectCount))
        {
            DP_LexDisableStreamIrq();
        }
        else if (connectCount > disconnectCount)
        {
            DP_LexRestoreStreamIrq();
        }
    }

    if (auxInts & BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_DETECTED_MASK)
    {
        auxInts &= ~BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_DETECTED_MASK;
        auxLexIsrCtx.isrCallback(BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_DETECTED_MASK);

        const bool hostConnection = AUX_GetHostConnectedInfo();
        DP_LexClearBackupIrq();
        if(!hostConnection)
        {
          //  AUX_DisableAuxInterrupts(BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_RX_FIFO_OVERFLOW_MASK |
          //          BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_RX_FIFO_PKT_RECEIVED_MASK);

           auxInts &= ~dp_aux_hpd_irq_pending_WRITEMASK;
           AUX_DisableAuxInterrupts(dp_aux_hpd_irq_pending_WRITEMASK);
        }
        else
        {
            AUX_EnableAuxInterrupts(BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_RX_FIFO_OVERFLOW_MASK |
                    BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_RX_FIFO_PKT_RECEIVED_MASK);
            AUX_AckPendingAuxInterrupts(BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_DP_SOURCE_DETECTED_MASK);
        }
    }

    if ((auxInts & BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_RX_FIFO_OVERFLOW_MASK) != 0)
    {
        // TODO I would prefer this be an assert, but sometimes the host spams
        // us with requests even when we have HPD deasserted.
        ilog_DP_AUX_COMPONENT_0(ILOG_MAJOR_ERROR, AUX_RX_FIFO_OVERFLOW);
        auxInts &= ~BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_RX_FIFO_OVERFLOW_MASK;
        AUX_AckPendingAuxInterrupts(BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_RX_FIFO_OVERFLOW_MASK);
    }
    if (auxInts & BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_RX_FIFO_PKT_RECEIVED_MASK)
    {
        LexProcessAuxRequest(AUX_LEX_RX);
        auxInts &= ~BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_RX_FIFO_PKT_RECEIVED_MASK;
        AUX_AckPendingAuxInterrupts(BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_RX_FIFO_PKT_RECEIVED_MASK);
    }

    preExitTime = LEON_TimerRead();

    ilog_DP_AUX_COMPONENT_2(ILOG_DEBUG,
                             AUX_LEX_ISR_TIME,
                             LEON_TimerCalcUsecDiff(enterTime, preExitTime),
                             LEON_TimerCalcUsecDiff(postExitTime, enterTime));
    iassert_DP_AUX_COMPONENT_2(auxInts == 0, AUX_UNHANDLED_INTERRUPT, __LINE__, auxInts);
    postExitTime = LEON_TimerRead();
}


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################