///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2018
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  -  upp.c
//
//!   @brief -  Contains the code to manage packets from the H2D (host to device) and
//              D2H (device to host) queues
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################

#include <ibase.h>
#include <upp_regs.h>
#include <interrupts.h>
#include <cpu_comm.h>
#include <callback.h>
#include <state_machine.h>

#include <ulp.h>
#include <upp.h>
#include "upp_loc.h"
#include "upp_queue_manager.h"
#include "upp_log.h"

#include <uart.h>

// Constants and Macros ###########################################################################


// Data Types #####################################################################################

enum QueueManagerState
{
    QUEUES_DISABLED,                    // 0 Queue processing is disabled
    QUEUES_IDLE,                        // 1 waiting for packets on queues
    QUEUES_PROCESS_H2D,                 // 2 Processing packets from the host
    QUEUES_PROCESS_D2H,                 // 3 Processing packets from the device
    QUEUES_WAIT_TOPOLOGY_CHANGE,        // 4 Topology change in progress; wait until done before processing more packets

    NUM_STATES_QUEUE_MANAGER            // number of states
};

struct UppManageContext                     //
{
    struct UtilSmInfo stateMachineInfo; // variables for the Queue management state machine

};


// Static Function Declarations ###################################################################
static void LexQueueEventCallback(void *param1, void *param2);

static enum QueueManagerState LexQueueDisabledHandler(           enum UppQueueManagerEvent event, enum QueueManagerState currentState);
static enum QueueManagerState LexQueueIdleHandler(               enum UppQueueManagerEvent event, enum QueueManagerState currentState);
static enum QueueManagerState LexQueueHostPacketHandler(         enum UppQueueManagerEvent event, enum QueueManagerState currentState);
static enum QueueManagerState LexQueueDevicePacketHandler(       enum UppQueueManagerEvent event, enum QueueManagerState currentState);
static enum QueueManagerState LexQueueWaitTopologyChangeHandler( enum UppQueueManagerEvent event, enum QueueManagerState currentState);

static enum QueueManagerState LexQueueCommonHandler(          enum UppQueueManagerEvent event, enum QueueManagerState currentState);

static enum QueueManagerState LexQueuePacketToProcess( void );

static void LexQueueSetDisabled( void );

// Static Variables ###############################################################################

static const EventHandler queueStateTable[NUM_STATES_QUEUE_MANAGER] =
{
    [QUEUES_DISABLED]               = LexQueueDisabledHandler,
    [QUEUES_IDLE]                   = LexQueueIdleHandler,
    [QUEUES_PROCESS_H2D]            = LexQueueHostPacketHandler,
    [QUEUES_PROCESS_D2H]            = LexQueueDevicePacketHandler,
    [QUEUES_WAIT_TOPOLOGY_CHANGE]   = LexQueueWaitTopologyChangeHandler,
};


static struct UppManageContext manageQueueContext =
{
    .stateMachineInfo.stateHandlers = queueStateTable,
    .stateMachineInfo.logInfo.info.reserved = 0,
    .stateMachineInfo.logInfo.info.logLevel = (uint8_t)ILOG_DEBUG,
//    .stateMachineInfo.logInfo.info.logLevel = (uint8_t)ILOG_MINOR_EVENT,
    .stateMachineInfo.logInfo.info.ilogComponent = UPP_COMPONENT,
    .stateMachineInfo.logInfo.info.ilogId = UPP_QUEUE_STATE_TRANSITION
};

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################




// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Initialize the UPP component
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UppQueueManagementInit(void)
{
    // For initialization, send Disable Enter event (Default state is DISABLE)
    LexQueueStateSendEventWithNoData(UPP_QUEUE_EVENT_ENTER);
}

//#################################################################################################
// Disables the UPP queue state machine
//
// Parameters:
// Return:
// Assumptions:  UPP may be in reset
//
//#################################################################################################
void UppQueueManagementDisable(void)
{
    // disable the queue management state machine.
    manageQueueContext.stateMachineInfo.currentState = QUEUES_DISABLED;
    manageQueueContext.stateMachineInfo.prevState    = QUEUES_DISABLED;
    manageQueueContext.stateMachineInfo.event        = UPP_QUEUE_DISABLE;
    manageQueueContext.stateMachineInfo.eventData    = NULL;

    LexQueueSetDisabled();
}

//#################################################################################################
// Queue manager callback event generation
//
// Parameters: event
// Return:
// Assumptions:
//
//#################################################################################################
void LexQueueStateSendEventWithNoData(enum UppQueueManagerEvent event)
{
    uint32_t eventx = (uint8_t)event;

    CALLBACK_Run(LexQueueEventCallback, (void *)eventx, NULL);
}


//#################################################################################################
// Gets the current state of the Queue management state machine.  Returns TRUE if disabled
//
// Parameters: event
// Return:
// Assumptions:
//
//#################################################################################################
bool UppQueueManagementIsDisabled(void)
{
    return((manageQueueContext.stateMachineInfo.currentState == QUEUES_DISABLED));
}

// Static Function Definitions ####################################################################

//#################################################################################################
// Event callback handler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexQueueEventCallback(void *param1, void *param2)
{
    UTILSM_PostEvent(&manageQueueContext.stateMachineInfo,
                    (uint32_t)param1,
                    (const union LexPmEventData *) param2);
}

//#################################################################################################
// QUEUES_DISABLED handler
//      Wait for UPP_QUEUE_ENABLE event and move to IDLE state to start processing the queues
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' is the next state for UPP_QUEUE_EVENT_EXIT
//
//#################################################################################################
static enum QueueManagerState LexQueueDisabledHandler( enum UppQueueManagerEvent event, enum QueueManagerState currentState)
{
    enum UppQueueManagerEvent nextState = currentState;

    if (event == UPP_QUEUE_EVENT_ENTER)
    {
        LexQueueSetDisabled();
    }
    else if (event == UPP_QUEUE_ENABLE)
    {
        // we are starting up again - clear the diag buffer now, rather then when we shut down, so we can still
        // look at the contents while down
        UPP_DiagReinit();
        nextState = QUEUES_IDLE;
    }

    return nextState;
}


//#################################################################################################
// QUEUES_IDLE handler
//      Wait for UPP_QUEUE_HOST_PACKET_AVAILABLE event to start processing the host packets
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' is the next state for UPP_QUEUE_EVENT_EXIT
//
//#################################################################################################
static enum QueueManagerState LexQueueIdleHandler( enum UppQueueManagerEvent event, enum QueueManagerState currentState)
{
    enum UppQueueManagerEvent nextState = currentState;

//    UART_printf("idle event %d\n", event);

    if (event == UPP_QUEUE_EVENT_ENTER)
    {
        UppHalGetIrq0Interrupts();  // clear any pending interrupts

        // see if there are any packets to process; turn interrupts on if not
        if (UppDeviceTopologyChanging())
        {
            // wait for the topology change to be over before processing more packets
            nextState = QUEUES_WAIT_TOPOLOGY_CHANGE;
        }
        else
        {
            enum QueueManagerState requestedState = LexQueuePacketToProcess();

            if (requestedState != QUEUES_IDLE)
            {
                nextState = requestedState;
            }
            else
            {
                // no more packets to process; close any transfers that have already received
                // the status packet
                UppTransactionDataQueueEmpty();
                UppTransactionFreeFinishedHostTransfers();  // release any host OUT transfers that are done
                UPP_HalControlQueueInterrupts(true);        // wait for packets
            }
        }
    }
    else if (event == UPP_QUEUE_ISR_EVENT)
    {
        uint32_t pendingInterrupt = UppHalGetIrq0Interrupts();  // get/clear any pending interrupts
        enum QueueManagerState requestedState = LexQueuePacketToProcess();

        if (requestedState != QUEUES_IDLE)
        {
            nextState = requestedState;
        }
        else
        {
            // spurious interrupt, ignore
            ilog_UPP_COMPONENT_1(ILOG_MINOR_ERROR, UPP_QUEUE_SPURIOUS_INTERRUPT_EVENT, pendingInterrupt);
        }
    }
    else if (event == UPP_QUEUE_EVENT_EXIT)
    {
        UPP_HalControlQueueInterrupts(false);   // we are polling the packets, make sure interrupts are off
    }
    else
    {
        nextState = LexQueueCommonHandler(event, currentState);
    }

    return nextState;
}

//#################################################################################################
// QUEUES_PROCESS_H2D handler
//      Wait for UPP_QUEUE_HOST_PACKET_AVAILABLE event to start processing the host packets
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' is the next state for UPP_QUEUE_EVENT_EXIT
//
//#################################################################################################
static enum QueueManagerState LexQueueHostPacketHandler( enum UppQueueManagerEvent event, enum QueueManagerState currentState)
{
    enum UppQueueManagerEvent nextState = currentState;

//    UART_printf("host event %d\n", event);

    if (event == UPP_QUEUE_EVENT_ENTER)
    {
        if (UppDeviceTopologyChanging())
        {
            // wait for the topology change to be over before processing more packets
            nextState = QUEUES_WAIT_TOPOLOGY_CHANGE;
        }
        else if (UppHalDownstreamPacketAvailable())
        {
            LexQueueStateSendEventWithNoData(UPP_QUEUE_HOST_PACKET_AVAILABLE);
        }
        else
        {
            nextState = QUEUES_IDLE;
        }
    }
    else if (event == UPP_QUEUE_EVENT_EXIT)
    {
        //
    }
    else if (event == UPP_QUEUE_HOST_PACKET_AVAILABLE)
    {
        UppTransactionDownstreamPacket();

        // all topology changes are triggered by UppHalReadDownstreamPacket(), so
        // we check again
        if (UppDeviceTopologyChanging())
        {
            // wait for the topology change to be over before processing more packets
            nextState = QUEUES_WAIT_TOPOLOGY_CHANGE;
        }
        else
        {
            enum QueueManagerState requestedState = LexQueuePacketToProcess();

            if (requestedState != QUEUES_PROCESS_H2D)
            {
                nextState = requestedState;
            }
            else
            {
                LexQueueStateSendEventWithNoData(UPP_QUEUE_HOST_PACKET_AVAILABLE);
            }
        }
    }
    else
    {
        nextState = LexQueueCommonHandler(event, currentState);
    }

    return nextState;
}

//#################################################################################################
// QUEUES_PROCESS_D2H handler
//      Process all device packets on the queue
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' is the next state for UPP_QUEUE_EVENT_EXIT
//
//#################################################################################################
static enum QueueManagerState LexQueueDevicePacketHandler( enum UppQueueManagerEvent event, enum QueueManagerState currentState)
{
    enum UppQueueManagerEvent nextState = currentState;

    //    UART_printf("device event %d\n", event);

    if (event == UPP_QUEUE_EVENT_ENTER)
    {
        if (UppDeviceTopologyChanging())
        {
            // wait for the topology change to be over before processing more packets
            nextState = QUEUES_WAIT_TOPOLOGY_CHANGE;
        }
        else if (UppHalUpstreamPacketAvailable())
        {
            LexQueueStateSendEventWithNoData(UPP_QUEUE_DEVICE_PACKET_AVAILABLE);
        }
        else
        {
            nextState = QUEUES_IDLE;
        }
    }
    else if (event == UPP_QUEUE_DEVICE_PACKET_AVAILABLE)
    {
        // see if there are any packets to process; go to idle if not
        UppTransactionUpstreamPacket();

        enum QueueManagerState requestedState = LexQueuePacketToProcess();

        if (requestedState != QUEUES_PROCESS_D2H)
        {
            nextState = requestedState;
            UppTransactionFreeFinishedTransfers();
        }
        else
        {
            LexQueueStateSendEventWithNoData(UPP_QUEUE_DEVICE_PACKET_AVAILABLE);
        }
    }
    else
    {
        nextState = LexQueueCommonHandler(event, currentState);
    }

    return nextState;
}


//#################################################################################################
// QUEUES_WAIT_TOPOLOGY_CHANGE handler
//      Process all device packets on the queue
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' is the next state for UPP_QUEUE_EVENT_EXIT
//
//#################################################################################################
static enum QueueManagerState LexQueueWaitTopologyChangeHandler( enum UppQueueManagerEvent event, enum QueueManagerState currentState)
{
    enum UppQueueManagerEvent nextState = currentState;

    if (event == UPP_QUEUE_ROUTE_CHANGE_DONE)
    {
        UppTransactionFreeFinishedHostTransfers();  // release any host OUT transfers that are done

        nextState = QUEUES_IDLE;
    }
    else
    {
        nextState = LexQueueCommonHandler(event, currentState);
    }

//    UART_printf("Topology change event %d next state %d\n", event, nextState);

    return nextState;
}

//#################################################################################################
// QUEUES_DISABLED handler
//      Wait for UPP_QUEUE_ENABLE event and move to IDLE state to start processing the queues
//
// Parameters:
// Return:
// Assumptions: parameter 'currentState' is the next state for UPP_QUEUE_EVENT_EXIT
//
//#################################################################################################
static enum QueueManagerState LexQueueCommonHandler( enum UppQueueManagerEvent event, enum QueueManagerState currentState)
{
    enum UppQueueManagerEvent nextState = currentState;

    switch (event)
    {
        case UPP_QUEUE_EVENT_ENTER:                          // A state started
        case UPP_QUEUE_EVENT_EXIT:                           // A state finished
            break;  // ignore these events

        case UPP_QUEUE_DISABLE:                              // UPP Queue management is off
            nextState = QUEUES_DISABLED;                     //  - go to the disabled state
            break;

        case UPP_QUEUE_ENABLE:                               // UPP Queue management is on
        case UPP_QUEUE_ISR_EVENT:                            // UPP Queue ISR event happened
        case UPP_QUEUE_HOST_PACKET_AVAILABLE:                // Control transfer packet from the host is available
        case UPP_QUEUE_DEVICE_PACKET_AVAILABLE:              // Control transfer packet from the device is available
        case UPP_QUEUE_HOST_STATUS_PACKET_RX:                // Host status packet was received
        case UPP_QUEUE_HOST_DEVICE_DATA_STATUS:              // Host transfer started, event data = device address transfer with
        case UPP_QUEUE_ROUTE_CHANGE_DONE:                    // Topology change done
        default:
            ilog_UPP_COMPONENT_2(ILOG_MINOR_ERROR, UPP_QUEUE_INVALID_EVENT, event, currentState);
            break;
    }

    return nextState;
}


//#################################################################################################
// Determine what packet queue needs to be processed, either the host or device queue.
//   If there are no packets to process, then we need to go back to idle
//
// If there are packets in both queues, take the older one
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static enum QueueManagerState LexQueuePacketToProcess( void )
{
    enum QueueManagerState requestedState = QUEUES_IDLE; // assume no packets to process

    bool hostPacketAvailable   = UppHalDownstreamPacketAvailable();
    bool devicePacketAvailable = UppHalUpstreamPacketAvailable();

    if (hostPacketAvailable)
    {
        requestedState = QUEUES_PROCESS_H2D;  // go process a host packet

        if (devicePacketAvailable)
        {
            // packets are available in both queues - choose the one that is older (older = lower timestamp)
            int16_t hostTimestamp   = UppHalGetDownstreamTimeStamp();
            int16_t deviceTimestamp = UppHalGetUpstreamTimeStamp();

            if (UppHalCompareTimestamps(deviceTimestamp, hostTimestamp))
            {
                // device packet is older then the host packet, go get it
                requestedState = QUEUES_PROCESS_D2H;  // go process a device packet
            }
        }
    }
    else if (devicePacketAvailable)
    {
        // only device packets are available
        requestedState = QUEUES_PROCESS_D2H;  // go process a device packet
    }

    return (requestedState);
}


//#################################################################################################
// Called when the disabled state is entered
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexQueueSetDisabled( void )
{
    UppTransactionReinit();
    UppEndpointTableReinit();
    UPP_DeviceReinit();
    UPP_TopologyReinit();
}
