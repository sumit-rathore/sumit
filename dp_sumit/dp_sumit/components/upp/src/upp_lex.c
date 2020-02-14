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
//!   @brief -  Contains the code for the USB Protocol layer Partner (UPP) project:
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################

#include <ibase.h>
#include <upp_regs.h>
#include <cpu_comm.h>
#include <event.h>
#include <ulp.h>
#include <upp.h>
#include "upp_loc.h"
#include "upp_queue_manager.h"
#include "upp_log.h"

#include <uart.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

static void LexUppReceiveCpuMsgHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength) __attribute__((section(".lexftext")));
static void LexUppEndpointRemoved(struct Endpoint *endpointPtr)                                 __attribute__((section(".lexftext")));
static void LexCommLinkEventHandler(uint32_t linkUp, uint32_t userContext)                      __attribute__((section(".lexftext")));

// Static Variables ###############################################################################

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize the UPP component
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_LexInit(void)
{
    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_UPP, LexUppReceiveCpuMsgHandler);             // used for Generic USB messages between Lex/Rex

    EVENT_Subscribe(ET_COMLINK_STATUS_CHANGE, LexCommLinkEventHandler, 0);

    UppQueueManagementInit();
    UPP_TransactionInit();
    UPP_DeviceInit();
    UPP_ParseInit();
    UPP_TopologyInit(deviceRemovedCallback, TopologyChangeDoneCallback);
}


//#################################################################################################
// The UPP ISR on the Lex side
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_LexUppISR(void)
{
    uint32_t lexUppIntsIrq0 = UppHalGetIrq0Interrupts();
    uint32_t lexUppIntsIrq1 = UppHalGetIrq1Interrupts();    // clear any IR1 interrupts (shouldn't be any!)

    ilog_UPP_COMPONENT_1(ILOG_DEBUG, UPP_INTERRUPT, lexUppIntsIrq0);

    if (lexUppIntsIrq0 & (UPP_IRQ0_PENDING_H2D_PKT_MASK | UPP_IRQ0_PENDING_D2H_PKT_MASK))
    {
        LexQueueStateSendEventWithNoData(UPP_QUEUE_ISR_EVENT);
        UPP_HalControlQueueInterrupts(false);   // make sure interrupts are off
    }
    else
    {
        ilog_UPP_COMPONENT_2(ILOG_MINOR_ERROR, UPP_UNHANDLED_ISR, lexUppIntsIrq0, lexUppIntsIrq1);
    }
}

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Sends the specified CPU message to the Rex
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################

void UppSendCPUCommLexUppMessage(enum UppLexSendMessage messageType, struct LexCpuMessage *message)
{
    CPU_COMM_sendMessage(CPU_COMM_TYPE_UPP, messageType, (const uint8_t *)message, sizeof(struct LexCpuMessage));
}

//#################################################################################################
// Sets the location information based on the given information
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################

uint32_t UppSetLocation(union UppPacketHeaderDw0 hubAddress)
{
    union UppDeviceLocation route = {0};    // make sure fields not set are zeroed

    route.routeStringHub1 = hubAddress.routeStringHub1;
    route.routeStringHub2 = hubAddress.routeStringHub2;
    route.routeStringHub3 = hubAddress.routeStringHub3;
    route.routeStringHub4 = hubAddress.routeStringHub4;
    route.routeStringHub5 = hubAddress.routeStringHub5;
    route.deviceAddress   = hubAddress.deviceAddress;

    return(route.routeNumber);
}


//#################################################################################################
// Sets the location information based on the given information
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################

uint32_t UppSetRouteFromHubPort(union UppPacketHeaderDw0 hubAddress, uint8_t portNumber)
{
    union UppDeviceLocation route;

    route.routeNumber = UppSetLocation(hubAddress);

    // create a full route string based on what we have and the port number
    if (route.routeStringHub1 == 0)
    {
        route.routeStringHub1 = portNumber;
    }
    else if (route.routeStringHub2 == 0)
    {
        route.routeStringHub2 = portNumber;
    }
    else if (route.routeStringHub3 == 0)
    {
        route.routeStringHub3 = portNumber;
    }
    else if (route.routeStringHub4 == 0)
    {
        route.routeStringHub4 = portNumber;
    }
    else if (route.routeStringHub5 == 0)
    {
        route.routeStringHub5 = portNumber;
    }

    return(route.routeNumber);
}


// Static Function Definitions ####################################################################

//#################################################################################################
// The handler for LEX receiving a CPU message from the REX.
//
// Parameters:
//      msg                     - message data
//      msgLength               - message length
// Return:
// Assumptions:
//
//#################################################################################################
static void LexUppReceiveCpuMsgHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    struct RexCpuMessage *rxMsg = (struct RexCpuMessage *)msg;

    if (UppQueueManagementIsDisabled())
    {
//        return;  // ignore these messages until we start processing packets, again
    }

    switch (subType)
    {
        case UPP_REX_TO_LEX_ENDPOINT_SET:
        {
            ilog_UPP_COMPONENT_3(ILOG_DEBUG, UPP_REX_MSG_SET_ENDPOINT,
                            rxMsg->endpointPtr->route.deviceAddress,
                            rxMsg->endpointPtr->info.number,
                            rxMsg->endpointPtr->info.assignedQueue);

            UPP_SetEndpointActiveState(rxMsg->endpointPtr, true);
            UppHalControlEndpoint(     rxMsg->endpointPtr, true);
        }
        break;

        case UPP_REX_TO_LEX_ENDPOINT_CLEARED:
        {
            ilog_UPP_COMPONENT_3(ILOG_DEBUG, UPP_REX_MSG_CLEAR_ENDPOINT,
                rxMsg->endpointPtr->route.deviceAddress,
                rxMsg->endpointPtr->info.number,
                rxMsg->endpointPtr->info.assignedQueue);

            LexUppEndpointRemoved(rxMsg->endpointPtr);
        }
        break;

        case UPP_REX_TO_LEX_ENDPOINT_NOT_RESPONDING:
        {
            ilog_UPP_COMPONENT_3(ILOG_MAJOR_EVENT, UPP_REX_MSG_ENDPOINT_NOT_RESPONSIVE,
                rxMsg->endpoint.route.deviceAddress,
                rxMsg->endpoint.info.number,
                rxMsg->endpoint.info.assignedQueue);

            UppHalControlEndpoint(&rxMsg->endpoint, false);
//           LexUppEndpointRemoved(&rxMsg->endpoint);
        }
        break;

        case UPP_REX_TO_LEX_DEVICE_REMOVED:
        {
            ilog_UPP_COMPONENT_1(ILOG_MINOR_EVENT, UPP_REX_MSG_DEVICE_REMOVED,
                            (uint32_t)rxMsg->interface.topologyDevicePtr);

            // Rex has removed this endpoint, we can free it here
            UPP_FreeDevice(rxMsg->interface.topologyDevicePtr);
        }
        break;

        case UPP_REX_TO_LEX_INTERFACE_SET_INFO:
        {
            ilog_UPP_COMPONENT_3(ILOG_DEBUG, UPP_REX_MSG_SET_INTERFACE,
                rxMsg->interface.topologyDevicePtr->route.deviceAddress,
                rxMsg->interface.location,
                rxMsg->interface.requestedOp);

            // go update some more endpoints
            CALLBACK_Run(
                UppDeviceUpdateEndpointsCallback,
                rxMsg->interface.topologyDevicePtr,
                NULL);
        }
        break;

        case UPP_REX_TO_LEX_ROUTE_CHANGE_COMPLETE:
        {
            TopologyChangeFinished();
            ilog_UPP_COMPONENT_1(ILOG_DEBUG, UPP_LEX_MSG_ROUTE_CHANGE_DONE, rxMsg->deviceAddress);
        }
        break;

        default:
            break;

    }
}

//#################################################################################################
// Handler for communication link up/down interrupts.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexUppEndpointRemoved(struct Endpoint *endpointPtr)
{
    UPP_SetEndpointActiveState(endpointPtr, false);
    UppHalControlEndpoint(     endpointPtr, false);

    if (endpointPtr->info.assignedQueue != UPP_SET_ENDPOINT_NO_BUFFERS)
    {
        iassert_UPP_COMPONENT_2(
            endpointPtr->info.assignedQueue == UppFreeEndpointTableEntry(
                endpointPtr->route.deviceAddress,
                endpointPtr->info.number),
                UPP_LEX_INVALID_BUFFER_ID_ENDPOINT,
                endpointPtr->info.number,
                endpointPtr->route.deviceAddress);
    }
}

//#################################################################################################
// Handler for communication link up/down interrupts.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LexCommLinkEventHandler(uint32_t linkUp, uint32_t userContext)
{
    // when the link goes up, tell the Rex whether we are enabled or not
    if (linkUp)
    {
        struct LexCpuMessage lexMessage = { 0 };

        lexMessage.lexIsoEnabled = UppIsIsoEnabled();
        UppSendCPUCommLexUppMessage(UPP_LEX_TO_REX_UPP_ENABLE_STATUS, &lexMessage);
    }
}

