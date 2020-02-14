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
//!   @file  -  upp_parse.c
//
//!   @brief -  Parse routines for UPP:
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################

#include <ibase.h>
#include <leon_timers.h>
#include <upp.h>
#include "upp_loc.h"
#include "upp_log.h"

#include <uart.h>
#include <usbdefs.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
typedef bool (*ProcessDescriptor)(void *data);

struct DescriptorProcess
{
    enum UppDescriptorType descriptor;
    ProcessDescriptor parse;
};

// Static Function Declarations ###################################################################

uint8_t UPP_ParseValidDescriptor(enum UppDescriptorType type);

static void UppParseConfigurationDescriptor(      struct UppUsb3Transaction *transaction, uint8_t const *data);
static void UppParseInterfaceDescriptor(          struct UppUsb3Transaction *transaction, uint8_t const *data);
static void UppParseEndpointDescriptor(           struct UppUsb3Transaction *transaction, uint8_t const *data);
static void UppParseSuperSpeedCompanionDescriptor(struct UppUsb3Transaction *transaction, uint8_t const *data);


// Static Variables ###############################################################################

const struct DescriptorProcess descriptors[] =
{
        { UPP_DESCRIPTOR_DEVICE,                    NULL},
        { UPP_DESCRIPTOR_CONFIGURATION,             NULL},
        { UPP_DESCRIPTOR_STRING,                    NULL},
        { UPP_DESCRIPTOR_INTERFACE,                 NULL},
        { UPP_DESCRIPTOR_ENDPOINT,                  NULL},
        { UPP_DESCRIPTOR_RESERVED1,                 NULL},
        { UPP_DESCRIPTOR_RESERVED2,                 NULL},
        { UPP_DESCRIPTOR_INTERFACE_POWER,           NULL},
        { UPP_DESCRIPTOR_OTG,                       NULL},
        { UPP_DESCRIPTOR_DEBUG,                     NULL},
        { UPP_DESCRIPTOR_INTERFACE_ASSOCIATION,     NULL},
        { UPP_DESCRIPTOR_BOS,                       NULL},
        { UPP_DESCRIPTOR_DEVICE_CAPABILITY,         NULL},

//        { UPP_DESCRIPTOR_SUPERSPEED_USB_ENDPOINT_COMPANION, NULL}
};
/*
static struct
{
    uint8_t deviceAddress;
    uint8_t isoEndpointNumber;

} uppParseContext;
*/
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
void UPP_ParseInit(void)
{
}


// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Get the bytes needed for the given descriptor, negative number returned if we can skip the
// descriptor
//
// Parameters:
// Return:
// Assumptions: The first 2 bytes are in the buffer pointed by descriptorPtr
//
//#################################################################################################
uint8_t UPP_ParseValidDescriptor(enum UppDescriptorType type)
{
    uint8_t descriptorSize = 0;

    switch(type)
    {
        case UPP_DESCRIPTOR_CONFIGURATION:
            descriptorSize = sizeof(struct ConfigDescriptor);
            break;

        case UPP_DESCRIPTOR_INTERFACE:
            descriptorSize = sizeof(struct InterfaceDescriptor);
            break;

        case UPP_DESCRIPTOR_ENDPOINT:
            descriptorSize = sizeof(struct EndpointDescriptor);
            break;

        case UPP_DESCRIPTOR_SUPERSPEED_USB_ENDPOINT_COMPANION:
            descriptorSize = sizeof(struct SuperSpeedEndpointCompanionDescriptor);
            break;

        case UPP_DESCRIPTOR_DEVICE:
        case UPP_DESCRIPTOR_STRING:
        case UPP_DESCRIPTOR_RESERVED1:
        case UPP_DESCRIPTOR_RESERVED2:
        case UPP_DESCRIPTOR_INTERFACE_POWER:
        case UPP_DESCRIPTOR_OTG:
        case UPP_DESCRIPTOR_DEBUG:
        case UPP_DESCRIPTOR_INTERFACE_ASSOCIATION:
        case UPP_DESCRIPTOR_BOS:
        case UPP_DESCRIPTOR_DEVICE_CAPABILITY:
        default:
            break;  // invalid descriptor, return 0
    }

    return (descriptorSize);
}


//#################################################################################################
// Parse the descriptors in the given packet
//
// Parameters: transaction - the current transaction, packet
// Return: false if parsing is done, true if more parsing required
// Assumptions:
//
//#################################################################################################
bool UppParseDescriptors(struct UppUsb3Transaction *transaction, struct UppPacketDecode *packet)
{
    uint8_t *descriptorBytePtr = (uint8_t *)transaction->deviceData.descriptorBuffer;

    if (transaction->deviceData.bytesInDescriptorBuffer < UPP_DESCRIPTOR_HEADER_SIZE)
    {
        return (false); // not enough bytes to process a descriptor
    }

    // these 2 variables are only valid if (transaction->bytesInDescriptorBuffer >= 2)
    uint16_t descriptorSize     = descriptorBytePtr[0];
    enum UppDescriptorType type = descriptorBytePtr[1];

    if (descriptorSize == 0)
    {
        UART_printf("parser: descriptor size is zero!!\n");

        transaction->parseFatalError = true;    // this is a bad error - tell the transaction handler what's going on
        return (false);  // exit out, hope we are not called again for this sequence of packets
    }
    else
    {
        uint8_t actualDescriptorSize = UPP_ParseValidDescriptor(type);

        if (actualDescriptorSize == 0)
        {
            // not a descriptor we are interested in
            int16_t bytesNeeded = descriptorSize - transaction->deviceData.bytesInDescriptorBuffer;

            if ( (bytesNeeded > 7) || (bytesNeeded <= 0))
            {
//                UART_printf("parse skip type %d size %d packetDataLength %d desc buffer %d\n", type, descriptorSize,  packet->packetDataLength, transaction->bytesInDescriptorBuffer);
                transaction->deviceData.numberOfSkipBytesNeeded =  UppTransferSkipData(transaction, packet, descriptorSize);

                return (transaction->deviceData.numberOfSkipBytesNeeded == 0);  // exit out, deal with any remaining errors
            }
            else
            {
                return (false);  // need more bytes
            }
        }
        else if (transaction->deviceData.bytesInDescriptorBuffer < actualDescriptorSize)
        {
//            UART_printf("not enough bytes, need %d size %d have %d\n",
//                actualDescriptorSize,
//                descriptorBytePtr[0],
//                transaction->bytesInDescriptorBuffer);

            return (false);  // not enough bytes to parse, yet
        }
    }

//    UART_printf("valid type %d size %d packetDataLength %d desc buffer %d\n", type, descriptorSize, packet->packetDataLength, transaction->bytesInDescriptorBuffer);
    switch(type)
    {
        case UPP_DESCRIPTOR_DEVICE:
            break;

        case UPP_DESCRIPTOR_CONFIGURATION:
            UppParseConfigurationDescriptor(transaction, descriptorBytePtr);
            break;

        case UPP_DESCRIPTOR_INTERFACE:
            UppParseInterfaceDescriptor(transaction, descriptorBytePtr);
            break;

        case UPP_DESCRIPTOR_ENDPOINT:
            UppParseEndpointDescriptor(transaction, descriptorBytePtr);
            break;

        case UPP_DESCRIPTOR_SUPERSPEED_USB_ENDPOINT_COMPANION:
            UppParseSuperSpeedCompanionDescriptor(transaction, descriptorBytePtr);
            break;

        case UPP_DESCRIPTOR_STRING:
        case UPP_DESCRIPTOR_RESERVED1:
        case UPP_DESCRIPTOR_RESERVED2:
        case UPP_DESCRIPTOR_INTERFACE_POWER:
        case UPP_DESCRIPTOR_OTG:
        case UPP_DESCRIPTOR_DEBUG:
        case UPP_DESCRIPTOR_INTERFACE_ASSOCIATION:
        case UPP_DESCRIPTOR_BOS:
        case UPP_DESCRIPTOR_DEVICE_CAPABILITY:
        default:
            break;
    }

    // now that the descriptor has been parsed, delete it from the buffer
    transaction->deviceData.numberOfSkipBytesNeeded =  UppTransferSkipData(transaction, packet, descriptorSize);

    return (transaction->deviceData.numberOfSkipBytesNeeded == 0);  // exit out, deal with any remaining errors
}




// Static Function Definitions ####################################################################

//#################################################################################################
// Parse the configuration descriptor
//
// Parameters: pointer to data containing the descriptor
// Return:
// Assumptions:
//
//#################################################################################################
static void UppParseConfigurationDescriptor(struct UppUsb3Transaction *transaction, uint8_t const *data)
{
    struct ConfigDescriptor const *configDesc = (struct ConfigDescriptor const *)data;

    transaction->deviceData.epRoute.configurationNumber = configDesc->bConfigurationVal;

    ilog_UPP_COMPONENT_3(ILOG_DEBUG, UPP_PARSE_CONFIG_DESCRIPTOR_INFO,
        transaction->deviceAddress,
        configDesc->bConfigurationVal,
        configDesc->bNumInterfaces);
}

//#################################################################################################
// Parse the interface descriptor
//
// Parameters: pointer to data containing the descriptor
// Return:
// Assumptions:
//
//#################################################################################################
static void UppParseInterfaceDescriptor(struct UppUsb3Transaction *transaction, uint8_t const *data)
{
    struct InterfaceDescriptor const *interfaceDesc = (struct InterfaceDescriptor const *)data;

    transaction->deviceData.epRoute.interfaceNumber          = interfaceDesc->bInterfaceNumber;
    transaction->deviceData.epRoute.alternateInterfaceNumber = interfaceDesc->bAlternateSetting;

    ilog_UPP_COMPONENT_3(ILOG_DEBUG, UPP_INTERFACE_DESCRIPTOR_INFO,
        interfaceDesc->bInterfaceNumber,
        interfaceDesc->bAlternateSetting,
        interfaceDesc->bNumEndpoints);
}

//#################################################################################################
// Parse the endpoint descriptor
//
// Parameters: pointer to data containing the descriptor
// Return:
// Assumptions:
//
//#################################################################################################
static void UppParseEndpointDescriptor(struct UppUsb3Transaction *transaction, uint8_t const *data)
{
    struct EndpointDescriptor const *endpoint = (struct EndpointDescriptor const *)data;

    struct UppDeviceIsoEndpointFields *endpointIsoPtr = (struct UppDeviceIsoEndpointFields *)endpoint;

    // only save certain endpoints... (IN, Bulk and ISO)
    if ( (endpointIsoPtr->endpointDirection == EP_DIRECTION_IN) && (endpointIsoPtr->transferType != EP_TYPE_CTRL) )
    {
        // add the previous endpoint to the cache before we start parsing the next one
        UPP_CacheEndpointData(&transaction->deviceData.endpointParsed);

        // endpoint saved, make sure we clear the structure for the next endpoint
        memset(&transaction->deviceData.endpointParsed, 0, sizeof(transaction->deviceData.endpointParsed));

        transaction->deviceData.endpointParsed.route.location = transaction->deviceData.epRoute.location;

        transaction->deviceData.endpointParsed.info.number    = endpointIsoPtr->endpointNumber;
        transaction->deviceData.endpointParsed.info.direction = endpointIsoPtr->endpointDirection;
        transaction->deviceData.endpointParsed.info.type      = endpointIsoPtr->transferType;

        transaction->deviceData.endpointParsed.info.bInterval = endpoint->bInterval;

        //            UART_printf("Endpoint: address %d, number %d type %d dir %d route %x\n",
        //                transaction->deviceAddress,
        //                endpoint.endpointNumber,
        //                endpoint.transferType,
        //                endpoint.endpointDirection,
        //                transaction->endpointParsed.route.location);
        //            UART_printf("[0]: %x [1]: %x [2]: %x [3]: %x\n", data[0], data[1], data[2], data[3]);
    }
}

//#################################################################################################
// Parse the Super Speed Endpoint Companion descriptor
//
// Parameters: pointer to data containing the descriptor
// Return:
// Assumptions:
//
//#################################################################################################
static void UppParseSuperSpeedCompanionDescriptor(struct UppUsb3Transaction *transaction, uint8_t const *data)
{
    if (transaction->deviceData.endpointParsed.route.location != 0)
    {
        // endpoint has been parsed, need to get the max burst
        struct SuperSpeedEndpointCompanionDescriptor const *endpointCompanion =
                        (struct SuperSpeedEndpointCompanionDescriptor const *)data;

        transaction->deviceData.endpointParsed.info.maxBurst = endpointCompanion->bMaxBurst;
    }
}

