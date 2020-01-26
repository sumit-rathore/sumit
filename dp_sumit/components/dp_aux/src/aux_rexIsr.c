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
#include <dp_aux.h>
#include "aux_loc.h"
#include "aux_log.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
struct AUX_RexIsrCtx
{
    IsrCallback isrCallback;        // ISR callback handler
};

// Static Function Declarations ###################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct AUX_RexIsrCtx auxRexIsrCtx;


// Exported Function Definitions ##################################################################
//#################################################################################################
// Aux Rex ISR Init function
//
// Parameters: call back handler to inform Aux ISR to Policy maker
// Return:
// Assumptions:
//#################################################################################################
void AUX_RexIsrInit(IsrCallback callback)
{
    auxRexIsrCtx.isrCallback = callback;
}

//#################################################################################################
// The main ISR for the Rex's AUX subsystem.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
// TODO look into handling only one IRQ per ISR call.
void AUX_RexISR(void)
{
    const LEON_TimerValueT startTime = LEON_TimerRead();

    // get and clear the interrupt(s) that caused us to be triggered
    uint32_t auxInts = AUX_GetPendingAuxInterrupts();

    ilog_DP_AUX_COMPONENT_1(ILOG_DEBUG, AUX_ENTERING_ISR, auxInts);

    if (auxInts & BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_PENDING_RX_FIFO_PKT_RECEIVED)
    {
        RexStepAuxStateMachine(AUX_REX_RX);
        auxInts &= ~BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_PENDING_RX_FIFO_PKT_RECEIVED;
        AUX_AckPendingAuxInterrupts(BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_PENDING_RX_FIFO_PKT_RECEIVED);
    }
    if (auxInts & BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_GO_BIT_CLEAR)
    {
        RexStepAuxStateMachine(AUX_REX_TX);
        auxInts &= ~BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_GO_BIT_CLEAR;
        AUX_AckPendingAuxInterrupts(BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_GO_BIT_CLEAR);
    }
    if (auxInts & BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_REPLY_TIMEOUT)
    {
        RexStepAuxStateMachine(AUX_REX_REPLY_TIMEOUT);
        auxInts &= ~BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_REPLY_TIMEOUT;
        AUX_AckPendingAuxInterrupts(BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_REPLY_TIMEOUT);
    }

    // TODO need to look into the sequencing of these interrupts:
    // how to handle the case where CONNECT and DISCONNECT are both set, etc.
    // This should probably be handled in StepStateMachine.
    if (auxInts & BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_DISCONNECT)
    {
        auxRexIsrCtx.isrCallback(BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_DISCONNECT);
        auxInts &= ~BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_DISCONNECT;
        AUX_AckPendingAuxInterrupts(BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_DISCONNECT);
    }
    if (auxInts & BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_CONNECT)
    {
        auxRexIsrCtx.isrCallback(BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_CONNECT);
        auxInts &= ~BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_CONNECT;
        AUX_AckPendingAuxInterrupts(BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_CONNECT);
    }
    if (auxInts & BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_IRQ)
    {
        auxRexIsrCtx.isrCallback(BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_IRQ);
        auxInts &= ~BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_IRQ;
        AUX_AckPendingAuxInterrupts(BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_IRQ);
    }
    if (auxInts & BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_REPLUG)
    {
        auxRexIsrCtx.isrCallback(BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_REPLUG);
        auxInts &= ~BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_REPLUG;
        AUX_AckPendingAuxInterrupts(BB_CHIP_DP_SINK_AUX_HPD_IRQ_PENDING_HPD_REPLUG);
    }

    iassert_DP_AUX_COMPONENT_2(auxInts == 0, AUX_UNHANDLED_INTERRUPT, __LINE__, auxInts);
    ilog_DP_AUX_COMPONENT_1(ILOG_DEBUG,
                             AUX_ISR_TIME,
                             LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()));
}



// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################


