//#################################################################################################
// Icron Technology Corporation - Copyright 2018
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
// Icmd function definitions for the UPP component
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################
#include "upp_loc.h"
#include "upp_cmd.h"
#include "upp_log.h"
// #include <uart.h>   // For test
#include <configuration.h>

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################
static void dRemovedCallback(void *param1, void *param2)            __attribute__((section(".flashcode")));
static void dTopologyDoneCallback(void *param1, void *param2)       __attribute__((section(".flashcode")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// bool uppBypassed = false;
// bool uppControlTransferEnabled = true;

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// Find a device with address
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_FindDeviceWithAddressIcmd(uint8_t adress)
{
   ilog_UPP_COMPONENT_2(ILOG_USER_LOG, UPP_DEVICE_ADDR, adress, (uint32_t)UPP_GetDevice(adress));
}

//#################################################################################################
// Add a device
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_DeviceAddIcmd(uint8_t addr, uint8_t hub1, uint8_t hub2, uint8_t hub3, uint8_t hub4, uint8_t hub5)
{
   const union UppDeviceLocation testTopology =
   {
       .routeStringHub1 = hub1,
       .routeStringHub2 = hub2,
       .routeStringHub3 = hub3,
       .routeStringHub4 = hub4,
       .routeStringHub5 = hub5,
       .deviceAddress   = addr,
   };

   UppDeviceAddToSystem(testTopology.routeNumber);
   UPP_PrintDevices();
}

//#################################################################################################
// Remove a device
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_DeviceRemoveIcmd(uint8_t hub1, uint8_t hub2, uint8_t hub3, uint8_t hub4, uint8_t hub5)
{
   const union UppDeviceLocation testTopology =
   {
       .routeStringHub1 = hub1,
       .routeStringHub2 = hub2,
       .routeStringHub3 = hub3,
       .routeStringHub4 = hub4,
       .routeStringHub5 = hub5,
   };

   UPP_RemoveDevice(testTopology.routeNumber);
}

//#################################################################################################
// Create a test topology enviornment
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_CreateTestTopology(void)
{
    const union UppDeviceLocation testTopologyPattern[] = {
        {
            .routeStringHub1 = 2,
            .deviceAddress = 1
        },
        {
            .routeStringHub1 = 4,
            .deviceAddress = 3
        },
        {
            .routeStringHub1 = 2,
            .routeStringHub2 = 1,
            .deviceAddress = 5
        },
        {
            .routeStringHub1 = 2,
            .routeStringHub2 = 3,
            .deviceAddress = 4
        },
        {
            .routeStringHub1 = 2,
            .routeStringHub2 = 4,
            .deviceAddress = 10
        },
        {
            .routeStringHub1 = 2,
            .routeStringHub2 = 3,
            .routeStringHub3 = 5,
            .deviceAddress = 6
        },
    };

    struct UppEndpointData endpointsPattern[] = {
        {
            .route.location = 0x04000100,          // Device 4, Config 0, Interface: 1, AltInterface: 0
            .info.number = 5
        },
        {
            .route.location = 0x04000100,          // Device 4, Config 0, Interface: 1, AltInterface: 0
            .info.number = 3
        },
        {
            .route.location = 0x04000101,          // Device 4, Config 0, Interface: 1, AltInterface: 1
            .info.number = 1
        },
        {
            .route.location = 0x04000101,          // Device 4, Config 0, Interface: 1, AltInterface: 1
            .info.number = 3
        },
    };

    UPP_TopologyInit(dRemovedCallback, dTopologyDoneCallback);

    for(uint8_t i=0; i < ARRAYSIZE(testTopologyPattern); i++)
    {
        UPP_AddDevice(testTopologyPattern[i].routeNumber);
    }

    for(uint8_t i=0; i < ARRAYSIZE(endpointsPattern); i++)
    {
        UPP_AddEndpoint(NULL, &endpointsPattern[i]);
    }
}

//#################################################################################################
// Set device endpoint
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_SetDeviceEndpointIcmd(uint32_t endpointRoute, uint8_t endpointNum)
{
    struct UppEndpointData endpoint = {
        .route.location = endpointRoute,
        .info.direction   = EP_DIRECTION_IN,
        .info.type        = EP_TYPE_ISO,
        .info.number      = endpointNum,
    };

    struct Device *devicePtr = UPP_GetDevice(endpoint.route.deviceAddress);

    if(devicePtr)
    {
        UPP_AddEndpoint(devicePtr, &endpoint);
    }
}

//#################################################################################################
// Set device endpoint
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_GetDeviceEndpointIcmd(uint32_t endpointRoute, uint8_t endpointNum)
{
    union EndpointRoute route =
    {
        .location = endpointRoute
    };

    struct Device *devicePtr = UPP_GetDevice(route.deviceAddress);

    if (devicePtr)
    {
        struct Endpoint *endpoint = UPP_GetEndpoint(devicePtr, route.location, endpointNum);

        if(endpoint)
        {
            ilog_UPP_COMPONENT_3(ILOG_MINOR_EVENT, UPP_DEVICE_ENDPOINT,
                    endpoint->route.location, endpoint->info.number,endpoint->info.type);
        }
    }
}

//#################################################################################################
// Set device endpoint
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_SetActiveInterfaceIcmd(uint32_t endpointRoute)
{
//    union EndpointRoute route =
//    {
//                    .location = endpointRoute
//    };
//    struct Device *devicePtr = UPP_GetDevice(route.deviceAddress);
//
////    if (devicePtr)
////    {
////        UPP_TopologySetInterface(devicePtr, &route);
////    }
}

//#################################################################################################
// Enable UPP
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_EnableIsoIcmd(void)
{
    // uppBypassed = false;
    UPP_EnableIso();
}

//#################################################################################################
// Disable UPP
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_DisableIsoIcmd(void)
{
    // uppBypassed = true;
    UPP_DisableIso();
}

//#################################################################################################
// Disable UPP
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_DisableUppControlTransferIcmd(void)
{
    // uppControlTransferEnabled = false;
    ConfigUsbExtendedConfig *usbExtendedConfig =  &(Config_GetBuffer()->usbExtendedConfig);
    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_USB_EXTENDED_CONFIG, usbExtendedConfig))
    {
        usbExtendedConfig->UPPcontrol &= ~(1 << CONFIG_ENABLE_CONTROL_TRANSFER);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_USB_EXTENDED_CONFIG, usbExtendedConfig);
    }
    ilog_UPP_COMPONENT_0(ILOG_USER_LOG, UPP_ICMD_CONTROL_TRANSFER_DISABLED);
}

//#################################################################################################
// Disable UPP
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_EnableUppControlTransferIcmd(void)
{
    // uppControlTransferEnabled = true;
    ConfigUsbExtendedConfig *usbExtendedConfig =  &(Config_GetBuffer()->usbExtendedConfig);
    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_USB_EXTENDED_CONFIG, usbExtendedConfig))
    {
        usbExtendedConfig->UPPcontrol |= (1 << CONFIG_ENABLE_CONTROL_TRANSFER);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_USB_EXTENDED_CONFIG, usbExtendedConfig);
    }
    ilog_UPP_COMPONENT_0(ILOG_USER_LOG, UPP_ICMD_CONTROL_TRANSFER_ENABLED);
}


// Static Function Definitions ####################################################################
//#################################################################################################
// remove device callback
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void dRemovedCallback(void *param1, void *param2)
{
   struct Device *device = (struct Device *)param1;
   union UppDeviceLocation *route = (union UppDeviceLocation *)param2;

   ilog_UPP_COMPONENT_3(ILOG_USER_LOG, UPP_DEVICE_REMOVE,
       route->deviceAddress,
       route->routeNumber & 0xFFFFF000,
       (uint32_t)device);

   UPP_FreeDevice(device);
}

//#################################################################################################
// topology change done test callback
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void dTopologyDoneCallback(void *param1, void *param2)
{
}

//#################################################################################################
// toggle between iso and no iso
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_toggleIsoIcmd(void)
{
    if(UppIsIsoEnabled())
    {
        UPP_DisableIso();
        ILOG_istatus(ISO_DISABLED, 0);
        ILOG_istatus(RESET_UNIT, 0);
               
    }
    else
    {
        UPP_EnableIso();
        ILOG_istatus(ISO_ENABLED, 0);
        ILOG_istatus(RESET_UNIT, 0);
    }
}