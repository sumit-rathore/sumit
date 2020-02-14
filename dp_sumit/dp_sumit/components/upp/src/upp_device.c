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
//!   @file  -  upp_device.c
//
//!   @brief -  Device specific operations; add remove, etc
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################

#include <ibase.h>
#include <callback.h>
#include <usbdefs.h>
#include "upp_log.h"
#include "upp_loc.h"

#include <uart.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

static void UppDeviceAddToSystemWrapper(void *param1, void *param2);
static void UppTopologyChangeStart(void);

static bool UppDeviceUpdateMarkEndpoints(struct Device *devicePtr, bool setEndpoint);

// Static Variables ###############################################################################

static struct
{
	bool topologyChangeActive; 	// TRUE = topology changes are active

} uppDeviceCtx;

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
void UPP_DeviceInit(void)
{
}

//#################################################################################################
// re-initialize the device management component of UPP
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_DeviceReinit(void)
{
    uppDeviceCtx.topologyChangeActive = false;  // no topology changes are active
}

//#################################################################################################
// Hub port just did a feature clear
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UppDeviceClearHubPort(union UppPacketHeaderDw0 hubAddress, uint8_t portNumber)
{
    union UppDeviceLocation route;

    route.routeNumber = UppSetRouteFromHubPort(hubAddress, portNumber);

    UppDeviceRouteRemoved(route.routeNumber);
}


//#################################################################################################
// Generic device removal, given the route string
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool UppDeviceRouteRemoved(uint32_t routeNumber)
{
    if (UPP_RemoveDevice(routeNumber))
    {
        UART_printf("Device removed routestring = %x\n", routeNumber);
        UppTopologyChangeStart();	// topology change has started
        return (true);
    }

    return (false);
}


//#################################################################################################
// Generic device removal, given the device Address
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UppDeviceRemoved(uint8_t deviceAddress)
{
    struct Device *removeDevice = UPP_GetDevice(deviceAddress);

    if(removeDevice != NULL)
    {
        bool found = UppDeviceRouteRemoved(removeDevice->route.routeNumber);
        UART_printf("Device %d removed %d\n", deviceAddress, found);
    }
}


//#################################################################################################
// Generic device removal, given the device Address
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UppDeviceAddToSystem(uint32_t routeNumber)
{
    UppTopologyChangeStart();   // topology change has started

    CALLBACK_Run(UppDeviceAddToSystemWrapper, (void *)routeNumber, NULL);
}

//#################################################################################################
// Sets a new configuration for the given device
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UppDeviceSetConfiguration(struct Device *devicePtr, uint8_t newConfigurationValue)
{
    union EndpointRoute route =
    {
        .deviceAddress              = devicePtr->route.deviceAddress,
        .configurationNumber        = devicePtr->currentConfiguration,
        .interfaceNumber            = 0,
        .alternateInterfaceNumber   = 0
    };

    ilog_UPP_COMPONENT_3(ILOG_MINOR_EVENT, UPP_DEVICE_SET_CONFIFGURATION,
        devicePtr->route.deviceAddress,
        devicePtr->route.routeNumber,
        newConfigurationValue);

    if ((devicePtr == NULL) || (devicePtr->route.deviceAddress == 0))
    {
        UART_printf("invalid params for set config, devicePtr %x address %d\n", devicePtr, devicePtr->route.deviceAddress);
        return;
    }

    UppTopologyChangeStart();   // topology change has started

    // mark the endpoints active with the current configuration to be cleared
    UPP_EndpointMarkOnMask(devicePtr, route.location, UPP_ENDPOINT_CONFIG_MASK, UPP_ENDPOINT_MARKED_CLEAR);

    if (newConfigurationValue != 0) // if the new configuration is zero we don't set any endpoints
    {
        // mark the endpoints with the new configuration to be set (AT setting == 0 for all interfaces)
        route.configurationNumber = newConfigurationValue;
        UPP_EndpointMarkOnMask(devicePtr, route.location, UPP_ENDPOINT_CONFIG_ALT_MASK,  UPP_ENDPOINT_MARKED_SET);
    }

    devicePtr->currentConfiguration = newConfigurationValue;

    route.configurationNumber = newConfigurationValue;
    devicePtr->newInterface = route;

    CALLBACK_Run(UppDeviceUpdateEndpointsCallback, (void *)devicePtr, NULL);
}


//#################################################################################################
// Sets a new alt setting for the interface on the given device
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UppDeviceSetInterface(struct Device *devicePtr, uint8_t interface, uint8_t altSetting)
{
    union EndpointRoute route =
    {
        .deviceAddress              = devicePtr->route.deviceAddress,
        .configurationNumber        = devicePtr->currentConfiguration,
        .interfaceNumber            = interface,
        .alternateInterfaceNumber   = altSetting
    };

    ilog_UPP_COMPONENT_2(ILOG_MINOR_EVENT, UPP_DEVICE_SET_INTERFACE,
        devicePtr->route.deviceAddress,
        route.location);

    devicePtr->newInterface = route;

    UppTopologyChangeStart();   // topology change has started

    // mark the endpoints active with the previous interface to be cleared
    UPP_EndpointMarkOnMask(devicePtr, route.location, UPP_ENDPOINT_INTERFACE_MASK, UPP_ENDPOINT_MARKED_CLEAR);

    // mark the endpoints with the new interface to be set
    UPP_EndpointMarkOnMask(devicePtr, route.location, UPP_ENDPOINT_LOCATION_MASK,  UPP_ENDPOINT_MARKED_SET);

    CALLBACK_Run(UppDeviceUpdateEndpointsCallback, (void *)devicePtr, NULL);
}


//#################################################################################################
// Returns the status of topology changes.
//
// Parameters:
// Return:		TRUE if the topology is being changed; FALSE if not
// Assumptions:
//
//#################################################################################################
bool UppDeviceTopologyChanging(void)
{
    return(uppDeviceCtx.topologyChangeActive);
}


// Static Function Definitions ####################################################################

//#################################################################################################
// Callback to say a device has been removed - remove any endpoints this device may have allocated,
// and then free the device
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void deviceRemovedCallback(void *param1, void *param2)
{
    struct Device *devicePtr = param1;

    union EndpointRoute route =
    {
        .deviceAddress              = devicePtr->route.deviceAddress,
        .configurationNumber        = devicePtr->currentConfiguration,
        .interfaceNumber            = 0,
        .alternateInterfaceNumber   = 0
    };

    // mark the endpoints active with the current configuration to be cleared
    UPP_EndpointMarkOnMask(devicePtr, route.location, UPP_ENDPOINT_CONFIG_MASK, UPP_ENDPOINT_MARKED_CLEAR);

    UppDeviceUpdateMarkEndpoints(devicePtr, UPP_ENDPOINT_MARKED_CLEAR);

    // make sure we free this device's transaction if there are any
    struct UppUsb3Transaction * freeTransaction = UppTransactionGetTransaction(devicePtr->route.deviceAddress);

    if (freeTransaction != NULL)
    {
        UppTransactionFreeTransaction(freeTransaction);
    }

    {
        // send a message to the Rex to remove this device;
        // we will carry on once we get confirmation from the Rex
        struct LexCpuMessage message = { 0 };

        message.deviceAddress = devicePtr->route.deviceAddress;
        message.interface.topologyDevicePtr = devicePtr;

        UppSendCPUCommLexUppMessage(UPP_LEX_TO_REX_REMOVE_DEVICE, &message);
    }
}


//#################################################################################################
// Generic device removal, given the device Address
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void UppDeviceAddToSystemWrapper(void *param1, void *param2)
{
    bool topologyChangeDone = true; // ASSUME no change is required...

    union UppDeviceLocation location =
    {
        .routeNumber = (uint32_t)param1
    };

    switch (UPP_AddDevice(location.routeNumber))
    {
        case UPP_TOPOLOGY_ADD_SUCCESS:               // successfully added a new device
            break;

        case UPP_TOPOLOGY_SAME_DEVICE_EXIST:         // the address and route path is already defined (same device?)
            break;

        case UPP_TOPOLOGY_DEVICE_ROUTE_EXIST:        // the route path is already in use by a different device
            UppDeviceRouteRemoved(location.routeNumber);    // remove the old device (and any others on the chain)
            UPP_PendingAddDevice(location.routeNumber);     // add the new device at this location

            // we need to wait for the previous command to finish before topology change is done
            topologyChangeDone = false;
            break;

        case UPP_TOPOLOGY_DEVICE_ADDRESS_EXIST:      // the address is already in use on a different route
            UppDeviceRemoved(location.deviceAddress);       // remove the other device
            UPP_PendingAddDevice(location.routeNumber);     // add this one

            topologyChangeDone = false;
            break;

        case UPP_TOPOLOGY_FULL:                      // can't allocate any more devices - topology change is done
        default:
            break;
    }

    if (topologyChangeDone)
    {
        CALLBACK_Run(TopologyChangeDoneCallback, NULL, (void *)(uint32_t)(location.deviceAddress));
    }
}

//#################################################################################################
// Helper function to set topology change active
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void UppTopologyChangeStart(void)
{
    uppDeviceCtx.topologyChangeActive = true;  // topology changes started
}

//#################################################################################################
// Callback to say topology change is complete (on the Lex side)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void TopologyChangeFinished(void)
{
    uppDeviceCtx.topologyChangeActive = false;  // topology changes complete

    // signal that the route change is done
    LexQueueStateSendEventWithNoData(UPP_QUEUE_ROUTE_CHANGE_DONE);
}


//#################################################################################################
// Callback to say topology change is complete (on the Lex side)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void TopologyChangeDoneCallback(void *param1, void *param2)
{
    struct Device *devicePtr = param1;     // param1 points to the device

    // send a message to the Rex with the information to finish setting the interface
    // we will carry on once we get this back from the Rex, after all the relevant endpoints have been cleared
    struct LexCpuMessage message = { 0 };

    if (devicePtr != NULL)
    {
        message.deviceAddress = devicePtr->route.deviceAddress;
        message.interface.location = devicePtr->newInterface.location;
    }
    else
    {
        message.deviceAddress = (uint32_t)(param2);
    }

    // Done all required topology changes
    // send a message to Rex that it will bounce back to us when it is done; then we can clean up
    UppSendCPUCommLexUppMessage(UPP_LEX_TO_REX_ROUTE_CHANGE_COMPLETE, &message);
}


void endpointInactiveCallback(void *param1, void *param2)
{

}


//#################################################################################################
// Callback to say topology change is complete (on the Lex side)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UppDeviceUpdateEndpointsCallback(void *param1, void *param2)
{
    struct Device *devicePtr = param1;     // param1 points to the device

    if ( (UppDeviceUpdateMarkEndpoints(devicePtr, UPP_ENDPOINT_MARKED_CLEAR) == false) &&
         (UppDeviceUpdateMarkEndpoints(devicePtr, UPP_ENDPOINT_MARKED_SET  ) == false) )
    {
        CALLBACK_Run( TopologyChangeDoneCallback, devicePtr, NULL);
    }
    else
    {
        // send a message to the Rex with the information to finish updating the endpoints
        // we will carry on once we get this back from the Rex
        struct LexCpuMessage message = { 0 };

        message.interface.topologyDevicePtr = devicePtr;
        message.interface.location          = devicePtr->newInterface.location;
        message.deviceAddress               = devicePtr->route.deviceAddress;

        UppSendCPUCommLexUppMessage(UPP_LEX_TO_REX_INTERFACE_SET_INFO, &message);
    }
}

//#################################################################################################
// Updated the endpoints marked to be set or cleared
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static bool UppDeviceUpdateMarkEndpoints(struct Device *devicePtr, bool setEndpoint)
{
    bool endpointsUpdated = true;   // ASSUME at least 1 endpoint was updated

    // Get the first marked endpoint for this device
    struct Endpoint *endpoint =  UPP_GetNextMarkedEndpoint(devicePtr, NULL, setEndpoint);

    if (endpoint == NULL)
    {
//        ilog_UPP_COMPONENT_0(ILOG_MINOR_ERROR,
//            (setEndpoint  ? UPP_NO_ENDPOINTS_MARKED_SET : UPP_NO_ENDPOINTS_MARKED_CLEAR));

        endpointsUpdated = false;   // no endpoints found to update!
    }
    else
    {
        do
        {
            // send a message to the Rex to set this endpoint;
            // we will clear it on the Lex side when we got the ACK from the Rex
            struct LexCpuMessage message = { 0 };

            if (setEndpoint)
            {
                // allocate a buffer if we are setting an endpoint
                uint8_t result = UppSetEndpointTableEntry(endpoint);

                if (result == UPP_SET_ENDPOINT_NO_BUFFERS)
                {
                    endpoint->info.assignedQueue = UPP_SET_ENDPOINT_NO_BUFFERS;
                }
                else // UPP_SET_ENDPOINT_NO_ACTION or UPP_SET_ENDPOINT_BUFFER_ALLOCATED
                {
                    endpoint->info.assignedQueue = result & ~UPP_SET_ENDPOINT_RESULT_MASK;
                }
            }

            message.endpoint      = *endpoint;
            message.endpointPtr   = endpoint;
            message.deviceAddress = endpoint->route.deviceAddress;

            UppSendCPUCommLexUppMessage(
                (setEndpoint ? UPP_LEX_TO_REX_SET_ENDPOINT : UPP_LEX_TO_REX_CLEAR_ENDPOINT),
                &message);

            endpoint = UPP_GetNextMarkedEndpoint(devicePtr, endpoint, setEndpoint);
        } while (endpoint != NULL);

    }

    return (endpointsUpdated);
}





