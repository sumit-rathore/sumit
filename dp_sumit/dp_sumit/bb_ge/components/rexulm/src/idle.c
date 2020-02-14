///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - idle.c
//
//!   @brief - contains the idle task for the RexULM
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "rexulm_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: _REXULM_idleTaskSofIncomingCheck()
*
* @brief  - Poll the CLM for received SOF packets
*           This indicates the upstream is operating
*
* @return - void
*
* @note   -
*
*/
void _REXULM_idleTaskSofIncomingCheck(
    TASKSCH_TaskT task, // This task
    uint32 taskArg      // Encodes a pointer to the base address of rex
)
{
    struct rexulmState * pRex = (struct rexulmState *)taskArg;
    // Ensure the upstream is operating,
    // otherwise why are we waiting for SOF packets
    iassert_REXULM_COMPONENT_1( pRex->upstreamPort == OPERATING,
                                IDLE_TASK_UPSTREAM_INVALID_STATE,
                                pRex->upstreamPort);

    if (CLM_checkForSelectedPacketRx())
    {
        // SOF received!
        boolT clearOperation = FALSE;
        _REXULM_MarkTime(TIME_MARKER_SOF_PACKET_RX);
        ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, IDLE_TASK_SOF_RX);
        rex.sofSynced = TRUE;

        // Inform the Rex Scheduler
        clearOperation = REXSCH_LexSofReceived();
        switch (pRex->downstreamPort)
        {
            case BUS_RESETTING:
                // Check if RexSch okay'd the completion of the operation
                if (clearOperation)
                {
                    // Return to normal operation with host
                    ULM_ClearRexExtendedReset();
                }
                break;

            case RESUMING:
                // Check if RexSch okay'd the completion of the operation
                if (clearOperation)
                {
                    // Return to normal operation with host
                    ULM_ClearRexExtendedResume();
                }
                break;

            case DISCONNECTED:  // This shouldn't happen
            case OPERATING:     // This shouldn't happen
            case SUSPENDING:    // This shouldn't happen
            case SUSPENDED:     // This shouldn't happen
            default:
                iassert_REXULM_COMPONENT_1(FALSE,
                                IDLE_TASK_DOWNSTREAM_INVALID_STATE,
                                pRex->downstreamPort);

                break;
        }

        TASKSCH_StopTask(task);
        _REXULM_UpdateSystemState();
    }
}


/**
* FUNCTION NAME: _REXULM_idleTaskSuspending()
*
* @brief  - Waits for a number of SOF characters to be transmitted before suspending
*           This just works around a few buggy devices
*
* @return - void
*
* @note   -
*
*/
void _REXULM_idleTaskSuspending(
    TASKSCH_TaskT task, // This task
    uint32 taskArg      // Encodes a pointer to the base address of rex
)
{
    struct rexulmState * pRex = (struct rexulmState *)taskArg;

    // Ensure the downstream is suspending,
    // otherwise why are we here
    iassert_REXULM_COMPONENT_1( pRex->downstreamPort == SUSPENDING,
                                IDLE_TASK_DOWNSTREAM_INVALID_STATE,
                                pRex->downstreamPort);

#ifndef BUILD_FOR_SIM
    // Ensure enough SOF's have been sent.  Value is based on testing with the Mac mini keyboard
    // For low speed, there are no SOF interrupts, ensure enough keep alive's have been transmitted
    if (    (REXSCH_getTransmittedSofCount() > 2)
        ||  ((pRex->opSpeed == USB_SPEED_LOW) && (LEON_TimerCalcUsecDiff(rex.sofSyncStarted, LEON_TimerRead()) > 3000)))
#endif
    {
        // Ok to suspend
        _REXULM_MarkTime(TIME_MARKER_IDLE_TASK_TO_START_SUSPEND);
        ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, IDLE_TASK_TO_START_SUSPEND);
        TASKSCH_StopTask(task);

        // Disable Rex Scheduler
        REXSCH_Disable();
        // Generate a USB suspend
        ULM_GenerateRexUsbSuspend();

        _REXULM_UpdateSystemState();
    }
}

/**
* FUNCTION NAME: _REXULM_idleTaskBusResetting()
*
* @brief  - Waits for a number of SOF characters to be transmitted before suspending
*           This just works around a few buggy devices
*
* @return - void
*
* @note   -
*
*/
void _REXULM_idleTaskBusResetting(
    TASKSCH_TaskT task, // This task
    uint32 taskArg      // Encodes a pointer to the base address of rex
)
{
    struct rexulmState * pRex = (struct rexulmState *)taskArg;

    // Ensure the downstream is bus resetting,
    // otherwise why are we here
    iassert_REXULM_COMPONENT_1( pRex->downstreamPort == BUS_RESETTING,
                                IDLE_TASK_DOWNSTREAM_INVALID_STATE,
                                pRex->downstreamPort);

#ifndef BUILD_FOR_SIM
    // Ensure enough SOF's have been sent.  Value is based on testing with the Mac mini keyboard
    // For low speed, there are no SOF interrupts, ensure enough keep alive's have been transmitted
    if (    (pRex->opSpeed == USB_SPEED_HIGH && REXSCH_getTransmittedSofCount() > 2)
        ||  (pRex->opSpeed == USB_SPEED_FULL && REXSCH_getTransmittedSofCount() > 0)
        ||  (pRex->opSpeed == USB_SPEED_LOW  && LEON_TimerCalcUsecDiff(rex.rstSofSyncStarted, LEON_TimerRead()) > 3000))
#endif
    {
        // Ok to suspend
        _REXULM_MarkTime(TIME_MARKER_IDLE_TASK_TO_START_BUS_RST);
        ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, IDLE_TASK_TO_START_BUS_RST);
        TASKSCH_StopTask(task);

        // Disable Rex Scheduler
        REXSCH_Disable();
        // Generate an extended bus reset so it won't stop until a message from the Lex is received to stop the reset
        ULM_GenerateRexUsbExtendedReset(pRex->opSpeed);

        _REXULM_UpdateSystemState();
    }
}


