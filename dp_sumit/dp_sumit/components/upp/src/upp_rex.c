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
#include <interrupts.h>
#include <cpu_comm.h>
#include <ulp.h>
#include <upp.h>
#include "upp_loc.h"
#include "upp_log.h"

#include <uart.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

static void RexUppReceiveCpuMsgHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength) __attribute__((section(".rexftext")));


// Static Variables ###############################################################################

static struct
{
    struct Endpoint cachedEndpoints[UPP_MAX_QUEUE_BUFFERS];

} rexUppContext;

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
void UPP_RexInit(void)
{
    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_UPP,        RexUppReceiveCpuMsgHandler);             // used for Generic USB messages between Lex/Rex
}


// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Re-initializes the Rex data structures
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################

void UPP_RexReinit(void)
{
    memset(rexUppContext.cachedEndpoints, 0, sizeof (rexUppContext.cachedEndpoints));
}


//#################################################################################################
// Sends the specified CPU message to the Lex
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################

void UppSendCPUCommRexUppMessage(enum UppRexSendMessage messageType, struct RexCpuMessage *message)
{
    CPU_COMM_sendMessage(CPU_COMM_TYPE_UPP, messageType, (const uint8_t *)message, sizeof(struct RexCpuMessage));
}

//#################################################################################################
// The UPP ISR on the Rex side
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_RexUppISR(void)
{
#ifdef BLACKBIRD_ISO
    uint32_t rexUppIntsIrq0 = UppHalGetIrq0Interrupts();    // clear any IR0 interrupts (shouldn't be any!)
    uint32_t rexUppIntsIrq1 = UppHalGetIrq1Interrupts();

    ilog_UPP_COMPONENT_1(ILOG_MAJOR_EVENT, UPP_INTERRUPT, rexUppIntsIrq1);

    if (rexUppIntsIrq1 & UPP_IRQ1_RAW_ISO_NOT_RESPOND_MASK)
    {
        for (int queueNumber = 1; queueNumber <= UPP_MAX_ISO_QUEUE_BUFFERS; queueNumber++)
        {
            if (rexUppIntsIrq1 & SET_BITMASK(UPP_IRQ1_RAW_ISO_NOT_RESPOND_OFFSET + queueNumber))
            {
                struct RexCpuMessage message = {0};
                struct Endpoint *endpointPtr = &rexUppContext.cachedEndpoints[queueNumber];

                if (endpointPtr != NULL)
                {
                    UppHalControlEndpoint(endpointPtr, false); // disable this endpointPtr

                    message.deviceAddress = endpointPtr->route.deviceAddress;
                    message.endpoint      = *endpointPtr;   // copy over the endpoint so the Lex knows which one we are dealing with
                    UppSendCPUCommRexUppMessage(UPP_REX_TO_LEX_ENDPOINT_NOT_RESPONDING, &message);
                }
            }
        }
    }
    else
    {
        ilog_UPP_COMPONENT_2(ILOG_MINOR_ERROR, UPP_UNHANDLED_ISR, rexUppIntsIrq0, rexUppIntsIrq1);
    }
#endif
}


// Static Function Definitions ####################################################################

//#################################################################################################
// The handler for REX receiving a CPU message from the LEX.
//
// Parameters:
//      msg                     - message data
//      msgLength               - message length
// Return:
// Assumptions:
//
//#################################################################################################
static void RexUppReceiveCpuMsgHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    struct LexCpuMessage *rxMsg = (struct LexCpuMessage *)msg;
    struct RexCpuMessage message = {0};

    message.deviceAddress = rxMsg->deviceAddress;

    switch (subType)
    {
        case UPP_LEX_TO_REX_SET_ENDPOINT:
        {
            ilog_UPP_COMPONENT_3(ILOG_MINOR_EVENT, UPP_LEX_MSG_SET_ENDPOINT,
                            rxMsg->endpoint.route.deviceAddress,
                            rxMsg->endpoint.info.number,
                            rxMsg->endpoint.info.assignedQueue);

            UppHalControlEndpoint(&rxMsg->endpoint, true);

            // send a message to the Lex that this endpoint was set
            message.endpointPtr = rxMsg->endpointPtr;
            UppSendCPUCommRexUppMessage(UPP_REX_TO_LEX_ENDPOINT_SET, &message);

            // save this endpoint that we are setting
            rexUppContext.cachedEndpoints[rxMsg->endpoint.info.assignedQueue] = rxMsg->endpoint;
        }
        break;

        case UPP_LEX_TO_REX_CLEAR_ENDPOINT:
        {
            ilog_UPP_COMPONENT_3(ILOG_MINOR_EVENT, UPP_LEX_MSG_CLEAR_ENDPOINT,
                            rxMsg->endpoint.route.deviceAddress,
                            rxMsg->endpoint.info.number,
                            rxMsg->endpoint.info.assignedQueue);

            UppHalControlEndpoint(&rxMsg->endpoint, false);

            // send a message to the Lex that this endpoint was cleared
            message.endpointPtr = rxMsg->endpointPtr;
            UppSendCPUCommRexUppMessage(UPP_REX_TO_LEX_ENDPOINT_CLEARED, &message);
        }
        break;

        case UPP_LEX_TO_REX_REMOVE_DEVICE:
        {
            ilog_UPP_COMPONENT_2(ILOG_MINOR_EVENT, UPP_LEX_MSG_REMOVE_DEVICE,
                rxMsg->deviceAddress,
                (uint32_t)rxMsg->interface.topologyDevicePtr);

            // send a message to the Lex that this device was removed
            message.topologyDevicePtr = rxMsg->interface.topologyDevicePtr;
            UppSendCPUCommRexUppMessage(UPP_REX_TO_LEX_DEVICE_REMOVED, &message);
        }
        break;

        case UPP_LEX_TO_REX_INTERFACE_SET_INFO:
        {
            ilog_UPP_COMPONENT_2(ILOG_MINOR_EVENT, UPP_LEX_MSG_SET_INTERFACE,
                rxMsg->deviceAddress,
                rxMsg->interface.location);

            // send this info back to the Lex to finish setting the new interface
            message.interface = rxMsg->interface;
            UppSendCPUCommRexUppMessage(UPP_REX_TO_LEX_INTERFACE_SET_INFO, &message);
        }
        break;

        case UPP_LEX_TO_REX_ROUTE_CHANGE_COMPLETE:
        {
            ilog_UPP_COMPONENT_2(ILOG_MINOR_EVENT, UPP_REX_MSG_ROUTE_CHANGE_DONE,
                rxMsg->deviceAddress,
                rxMsg->interface.location);

             // send a message to the Lex acknowledging route change completed
            UppSendCPUCommRexUppMessage(UPP_REX_TO_LEX_ROUTE_CHANGE_COMPLETE, &message);
        }
        break;

        case UPP_LEX_TO_REX_UPP_ENABLE_STATUS:
            if (rxMsg->lexIsoEnabled)
            {
                UPP_EnableIso();
            }
            else
            {
                UPP_DisableIso();
            }
            break;

        default:
            break;

    }
}


