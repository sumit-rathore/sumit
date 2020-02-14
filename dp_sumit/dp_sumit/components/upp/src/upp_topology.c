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
//!   @file  -  upp_devices.c
//
//!   @brief -  Contains the code for the USB Protocol layer Partner (UPP) project:
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################
#include <ibase.h>
#include <ipool.h>
#include <callback.h>
#include <upp.h>
#include "upp_loc.h"
#include "upp_log.h"
#include <uart.h>    //For test

// Constants and Macros ###########################################################################

// Data Types #####################################################################################


// Static Function Declarations ###################################################################
static enum UppAddDeviceErrorCode UPP_ValidateAddRoute(uint32_t);
static void UPP_DisconnectLinkedList(struct UppDpLinkInfo *removeLink);
static void UPP_ConnectLinkedList(struct UppDpLinkInfo *addLink, bool (*compare)(void *, void *));
static bool UPP_DeviceSort(void *compare, void *new);
static bool UPP_EndpointSort(void *compare, void *new);
static void UPP_FreeEndpoint(struct Device *device, struct Endpoint *endpoint);
static struct Endpoint* UPP_FindEndpointWithRoute(struct Device *devicePtr, uint32_t routeNumber);

static bool UPP_NextDeviceRemove(void);
static void UPP_TopologyChangeDone(uint8_t deviceAddress);

static void UPP_PrintDevice(void *param1, void *param2);

// Static Variables ###############################################################################
IPOOL_CREATE(uppDevices, struct Device, MAX_NUM_USB_DEVICE);
IPOOL_CREATE(uppEndpoints, struct Endpoint, MAX_NUM_UPP_ENDPOINT);

static struct
{
    CallbackFunctionPtr deviceRemoveCallback;           // callback for notifying a device to be remove, param1: struct Device*, param2: device's route
    CallbackFunctionPtr topologyUpdateDone;             // callback for notifying when a topology update is complete

    struct Device       *activeListHead;                // Linked list header for active device
    struct Device       *activeListTail;                // Linked list tail   for active device

    union UppDeviceLocation pendingDeviceAdd;           // device waiting to be added after removal - zero if not valid

    // variables used to remove devices from a route
    struct Device *devicePtr;                           // the current device we are removing from a route
    union UppDeviceLocation location;                   // the route we want to remove
    uint32_t mask;                                      // the mask that belongs with this route

    // cached endpoints
    struct UppEndpointData endpointSaved[MAX_NUM_UPP_ENDPOINT];
    uint8_t savedIndex;         // the next position after the last endpoint was saved

} uppTopologyCtx         __attribute__((section(".lexbss")));

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// Devices init & cleanup
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_TopologyInit(  CallbackFunctionPtr deviceRemoveCallback,       // callback for notifying a device to be remove
                        CallbackFunctionPtr TopologyChangeEndCallback)  // callback for notifying when a topology change is done!
{
    uppTopologyCtx.deviceRemoveCallback     = deviceRemoveCallback;
    uppTopologyCtx.topologyUpdateDone       = TopologyChangeEndCallback;

    UPP_TopologyReinit();
}

//#################################################################################################
// Reset all topology information
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_TopologyReinit(void)
{
    uppDevices_poolInit();
    uppEndpoints_poolInit();
    UPP_PurgeEndpointCache();

    uppTopologyCtx.activeListHead = NULL;
    uppTopologyCtx.activeListTail = NULL;
    uppTopologyCtx.pendingDeviceAdd.routeNumber = 0;
    uppTopologyCtx.devicePtr = NULL;
    uppTopologyCtx.location.routeNumber = 0;
    uppTopologyCtx.mask = 0;
}

// #################################################################################################
// Add a new device
//
// Parameters:  UppDeviceLocation structure address
// Return:      enum UppAddDeviceErrorCode
// Assumptions:
//
// #################################################################################################
enum UppAddDeviceErrorCode UPP_AddDevice(uint32_t routeNumber)
{
    union UppDeviceLocation location;

    location.routeNumber = routeNumber;

    enum UppAddDeviceErrorCode errorCode = UPP_ValidateAddRoute(routeNumber);

    if(errorCode == UPP_TOPOLOGY_ADD_SUCCESS)
    {
        // device does not exist, add it to the system
        struct Device *newDevice = uppDevices_poolAlloc();

        if(newDevice)
        {
            memset(newDevice, 0, sizeof(struct Device));

            // update new device information
            newDevice->route.routeNumber  = location.routeNumber;

            ilog_UPP_COMPONENT_3(ILOG_MAJOR_EVENT, UPP_DEVICE_ADD, newDevice->route.deviceAddress,
                newDevice->route.routeNumber & ROUTE_PATH_MASK, (uint32_t)newDevice);

            struct UppDpLinkInfo addDevice =
            {
                .linkHead = (struct UppDoubleLink **)(&uppTopologyCtx.activeListHead),
                .linkTail = (struct UppDoubleLink **)(&uppTopologyCtx.activeListTail),
                .element  = &(newDevice->link)
            };

            UPP_ConnectLinkedList(&addDevice, UPP_DeviceSort);
        }
        else
        {
            // can't add any more devices!
            ilog_UPP_COMPONENT_0(ILOG_MAJOR_ERROR, UPP_MAX_DEVICE_OVER);
            errorCode = UPP_TOPOLOGY_FULL;
        }
    }

    return errorCode;
}

// #################################################################################################
// Add a new device
//
// Parameters:  UppDeviceLocation structure address
// Return:      enum UppAddDeviceErrorCode
// Assumptions:
//
// #################################################################################################
void UPP_PendingAddDevice(uint32_t routeNumber)
{
    iassert_UPP_COMPONENT_1(
        uppTopologyCtx.pendingDeviceAdd.routeNumber == 0,
        UPP_DEVICE_ALREADY_PENDING,
        uppTopologyCtx.pendingDeviceAdd.routeNumber);

    uppTopologyCtx.pendingDeviceAdd.routeNumber = routeNumber;
}


//#################################################################################################
// Remove a device
//
// Parameters:  UppDeviceLocation structure address
// Return:      bool (success/fail)
// Assumptions:
//
//#################################################################################################
bool UPP_RemoveDevice(uint32_t routeNumber)
{
    struct Device *devicePtr =     uppTopologyCtx.activeListHead;
    bool topologyChanged = false;

    if (devicePtr != NULL)
    {
        uint32_t testMask = 0xF0000000;             // Check from routeStringHub1
        uint32_t mask     = 0x00000000;             // Mask result

        for(uint8_t i=0; i< MAX_NUM_TIER ; i++)     // Create mask value to compare routeNumber
        {
            if(routeNumber & testMask)
            {
                mask |= testMask;
                testMask = testMask >> 4;
            }
            else
            {
                break;
            }
        }

        uppTopologyCtx.devicePtr = devicePtr;
        uppTopologyCtx.mask = mask;
        uppTopologyCtx.location.routeNumber = routeNumber;

        topologyChanged = UPP_NextDeviceRemove();	// remove the 1st device in the list
    }

    return (topologyChanged);
}


//#################################################################################################
// Free device, and all associated endpoints
//
// Parameters: device pointer
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_FreeDevice(struct Device *device)
{
    uint8_t deviceAddress = device->route.deviceAddress;

    // make sure the device pointer is valid
    iassert_UPP_COMPONENT_2(
        ( (device == uppTopologyCtx.devicePtr) && (device != NULL)),
        UPP_INVALID_DEVICE_FREE,
        (uint32_t)device,
        (uint32_t)uppTopologyCtx.devicePtr);

    ilog_UPP_COMPONENT_3(ILOG_MAJOR_EVENT, UPP_DEVICE_FREE, device->route.deviceAddress,
        device->route.routeNumber & ROUTE_PATH_MASK, (uint32_t)device);

    // free all the endpoints associated with this device
    {
        struct Endpoint *endpoint = device->endpointHead;
        struct UppDoubleLink *next;

        while(endpoint)                         // Free endpoints before freeing device
        {
            next = endpoint->link.next;
            UPP_FreeEndpoint(device, endpoint);
            endpoint = (struct Endpoint *)next;
        }
    }

    // go on to the next device
    uppTopologyCtx.devicePtr = (struct Device *)uppTopologyCtx.devicePtr->link.next;

    // finally, free this device
    {
        struct UppDpLinkInfo removeLink =
        {
            .linkHead = (struct UppDoubleLink **)(&uppTopologyCtx.activeListHead),
            .linkTail = (struct UppDoubleLink **)(&uppTopologyCtx.activeListTail),
            .element  = &(device->link)
        };

        UPP_DisconnectLinkedList(&removeLink);
        uppDevices_poolFree(device);
    }

    if ( UPP_NextDeviceRemove() == FALSE)
    {
        UPP_TopologyChangeDone(deviceAddress);
    }
}

//#################################################################################################
// Save an endpoint data from a device into the cache
//
// Parameters:  endpoint information
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_CacheEndpointData(struct UppEndpointData *endpointData)
{
    uppTopologyCtx.savedIndex++; // go on to the next saved index, in case we are still pointing to the last one we saved

    for (uint8_t count = 0 ; count < MAX_NUM_UPP_ENDPOINT; count++, uppTopologyCtx.savedIndex++)
    {
        if (uppTopologyCtx.savedIndex >= MAX_NUM_UPP_ENDPOINT)
        {
            uppTopologyCtx.savedIndex = 0;
        }

        if (uppTopologyCtx.endpointSaved[uppTopologyCtx.savedIndex].route.location == 0)
        {
            memcpy(
                &uppTopologyCtx.endpointSaved[uppTopologyCtx.savedIndex],
                endpointData,
                sizeof(uppTopologyCtx.endpointSaved[uppTopologyCtx.savedIndex]));

            break;  // saved the endpoint, exit
        }
    }
}


//#################################################################################################
// Move all of the endpoint data from the cache to topology; clear all of the device entries
// from the cache
//
// Parameters:  device pointer
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_AddEndpointDataFromCache(struct Device *device)
{
    uint8_t deviceAddress = device->route.deviceAddress;

    for (uint8_t count = 0; count < MAX_NUM_UPP_ENDPOINT; count++)
    {
        if (uppTopologyCtx.endpointSaved[count].route.deviceAddress == deviceAddress)
        {
            UPP_AddEndpoint(device,&uppTopologyCtx.endpointSaved[count]);

            memset(
                &uppTopologyCtx.endpointSaved[count],
                0,
                sizeof(uppTopologyCtx.endpointSaved[count]));
        }
    }
}

//#################################################################################################
// Clears all the cached endpoint data associated with a device
//
// Parameters:  the address of the device we want to clear the endpoint data
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_ClearDeviceEndpointCache(uint8_t deviceAddress)
{
    for (uint8_t count = 0; count < MAX_NUM_UPP_ENDPOINT; count++)
    {
        if (uppTopologyCtx.endpointSaved[count].route.deviceAddress == deviceAddress)
        {
            memset(
                &uppTopologyCtx.endpointSaved[count],
                0,
                sizeof(uppTopologyCtx.endpointSaved[count]));
        }
    }
}

//#################################################################################################
// Clear all data from the endpoint cache
//
// Parameters:  endpoint information
// Return:      new allocated endpoint address
// Assumptions:
//
//#################################################################################################
void UPP_PurgeEndpointCache(void)
{
    memset(uppTopologyCtx.endpointSaved, 0, sizeof(uppTopologyCtx.endpointSaved));
    uppTopologyCtx.savedIndex = 0;
}


//#################################################################################################
// Set an endpoint information
//
// Parameters:  endpoint information
// Return:      new allocated endpoint address
// Assumptions:
//
//#################################################################################################
struct Endpoint* UPP_AddEndpoint(struct Device *device, struct UppEndpointData *endpoint)
{
    struct Endpoint *newEndpoint = NULL;
    struct Device   *devicePtr = device;

    if (endpoint->route.location != 0)
    {
        if(devicePtr == NULL)
        {
            devicePtr = UPP_GetDevice(endpoint->route.deviceAddress);
            iassert_UPP_COMPONENT_2(devicePtr != NULL, UPP_NULL_DEVICE, endpoint->route.deviceAddress, __LINE__);
        }

        // Check if this endpoint already exists, update this w/o allocate new pool
        if(UPP_GetEndpoint(devicePtr, endpoint->route.location, endpoint->info.number) == NULL)
        {
            newEndpoint = uppEndpoints_poolAlloc();

            if(newEndpoint)
            {
                memset(newEndpoint, 0, sizeof(struct Endpoint));
                newEndpoint->info  = endpoint->info;
                newEndpoint->route = endpoint->route;

                struct UppDpLinkInfo addLink =
                {
                    .linkHead = (struct UppDoubleLink **)(&devicePtr->endpointHead),
                    .linkTail = (struct UppDoubleLink **)(&devicePtr->endpointTail),
                    .element  = &(newEndpoint->link)
                };

                UPP_ConnectLinkedList(&addLink, UPP_EndpointSort);

                ilog_UPP_COMPONENT_3(ILOG_DEBUG, UPP_NEW_ENDPOINT,
                        endpoint->route.location, endpoint->info.number, endpoint->info.type);
            }
            else
            {
                ilog_UPP_COMPONENT_0(ILOG_MAJOR_ERROR, UPP_MAX_ENDPOINT_OVER);
            }
        }
        else
        {
            // endpoint already exists!
        }
    }

    return newEndpoint;
}


//#################################################################################################
// Get device information by device address
//
// Parameters: device address
// Return:
// Assumptions:
//
//#################################################################################################
struct Device* UPP_GetDevice(uint8_t deviceAddress)
{
    struct Device *devicePtr = uppTopologyCtx.activeListHead;
    struct Device *deviceFound = NULL;

    while(devicePtr)                // search the device from active list
    {
        if(devicePtr->route.deviceAddress == deviceAddress)
        {
            deviceFound = devicePtr;
            break;
        }
        devicePtr = (struct Device *)devicePtr->link.next;
    }

    return deviceFound;
}

//#################################################################################################
// Get device information by device route
//
// Parameters: device address
// Return:
// Assumptions:
//
//#################################################################################################
//struct Device* UPP_GetDeviceFromRoute(union UppPacketHeaderDw0 header)
//{
//    struct Device *devicePtr = uppTopologyCtx.activeListHead;
//    struct Device *deviceFound = NULL;
//
//    union UppDeviceLocation location;
//
//    location.routeNumber = UppSetLocation(header);
//
//    while(devicePtr)                // search the device from active list
//    {
//        if(devicePtr->route.deviceAddress == location.deviceAddress)
//        {
//            deviceFound = devicePtr;
//            break;
//        }
//        devicePtr = (struct Device *)devicePtr->link.next;
//    }
//
//    return deviceFound;
//}

//#################################################################################################
// Gets the specified endpoint information, returns NULL if not found
//
// Parameters:  endpoint route, endpoint number
// Return:      endpoint pointer
// Assumptions:
//
//#################################################################################################
struct Endpoint* UPP_GetEndpoint(struct Device *devicePtr, uint32_t route, uint8_t endpointNumber)
{
    struct Endpoint *endpointPtr = UPP_FindEndpointWithRoute(devicePtr, route);
    struct Endpoint *endpoint = NULL;

    while( (endpointPtr != NULL) &&
           (endpointPtr->route.location == route) )
    {
        if(endpointPtr->info.number == endpointNumber)
        {
            endpoint = endpointPtr;
            break;
        }

        endpointPtr = (struct Endpoint *)endpointPtr->link.next;
    }

    return endpoint;
}

//#################################################################################################
// Marks the endpoints that match the given configuration to be cleared/set
//
// Parameters:  device pointer, location we are interested in
// Return:      true if we marked at least one endpoint to be cleared
// Assumptions:
//
//#################################################################################################
bool UPP_EndpointMarkOnMask(struct Device *devicePtr, uint32_t location, uint32_t mask, bool setEndpoint)
{
    if (location == 0)
    {
        return (NULL);  // invalid location given!
    }

    bool found = false;
    struct Endpoint *endpoint = devicePtr->endpointHead;

    location &= mask;

    while (endpoint != NULL)
    {
        if (setEndpoint)
        {
            if ( (endpoint->route.location & mask) == location)
            {
                endpoint->info.set = 1;
                found = true;
            }
        }
        else
        {
            if ( (endpoint->info.active) &&
               ( (endpoint->route.location & mask) == location) )
            {
                endpoint->info.clear = 1;
                found = true;
            }
        }

        endpoint = (struct Endpoint *)endpoint->link.next;
    }

    return(found);
}


//#################################################################################################
// Get the next endpoint in the list that is marked to be cleared,
// return NULL if at the end. If NULL pointer is given, return the first endpoint that matches,
// NULL if none
//
// Parameters:  device pointer, pointer to the current endpoint in the list
//              bool clear = 0 (UPP_ENDPOINT_MARKED_CLEAR), set = true (UPP_ENDPOINT_MARKED_SET)
// Return:      the next endpoint
// Assumptions:
//
//#################################################################################################
struct Endpoint* UPP_GetNextMarkedEndpoint(struct Device *devicePtr, struct Endpoint *endpoint, bool setEndpoint)
{
    if (endpoint == NULL)
    {
        // no endpoint given, find the first one
        endpoint = devicePtr->endpointHead;     // point to the start of the list
    }
    else
    {
        endpoint = (struct Endpoint *)endpoint->link.next;
    }

    while (endpoint != NULL)
    {
        if (setEndpoint)
        {
            if  (endpoint->info.set == true)
            {
                break;
            }
        }
        else
        {
            if  (endpoint->info.clear == true)
            {
                break;
            }
        }

        endpoint = (struct Endpoint *)endpoint->link.next;
    }

    return endpoint;
}


//#################################################################################################
// Sets the active state of the endpoint based on the given state
//
// Parameters:  pointer to the current endpoint in the list, location we are interested in
// Return:      the next endpoint
// Assumptions: The endpoint found is not active
//
//#################################################################################################
void UPP_SetEndpointActiveState(struct Endpoint *endpoint, bool activeState)
{
    if (activeState)
    {
        endpoint->info.set = false;
    }
    else
    {
        endpoint->info.clear = false;
    }

    endpoint->info.active = activeState;
}

//#################################################################################################
// Remove a endpoint (For Test)
//
// Parameters: device address and endpoint number
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_RemoveEndpoint(uint8_t deviceAddress, uint8_t endpointNumber)
{
    struct Device *devicePtr = UPP_GetDevice(deviceAddress);
    struct Endpoint *endpointPtr = devicePtr->endpointHead;

    while(endpointPtr)
    {
        if(endpointPtr->info.number == endpointNumber)
        {
            UPP_FreeEndpoint(devicePtr, endpointPtr);
            break;
        }

        endpointPtr = (struct Endpoint *)endpointPtr->link.next;
    }
}

//#################################################################################################
// Print active devices (For Test)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_PrintDevices(void)
{
    struct Device *devicePtr = uppTopologyCtx.activeListHead;

    CALLBACK_RunSingle(UPP_PrintDevice, devicePtr, NULL);
}

//#################################################################################################
// Print one device (For Test)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void UPP_PrintDevice(void *param1, void *param2)
{
    struct Device *devicePtr = param1;

    if(devicePtr)
    {
        ilog_UPP_COMPONENT_2(ILOG_USER_LOG, UPP_DEVICE_LOCATION,
            devicePtr->route.deviceAddress,
            devicePtr->route.routeNumber & ROUTE_PATH_MASK);

        ilog_UPP_COMPONENT_2(ILOG_DEBUG, UPP_DEVICE_NEXT, (uint32_t)devicePtr->link.next, (uint32_t)devicePtr->link.prev);

        struct Endpoint *endpointPtr = devicePtr->endpointHead;
        while(endpointPtr)
        {
            UART_WaitForTx();

            ilog_UPP_COMPONENT_3(ILOG_USER_LOG, UPP_DEVICE_ENDPOINT,
                endpointPtr->info.number, endpointPtr->info.type, endpointPtr->route.location);

            ilog_UPP_COMPONENT_3(ILOG_USER_LOG, UPP_DEVICE_ENDPOINT_DETAIL,
                ((endpointPtr->info.active << 12) | (endpointPtr->info.set << 8) | (endpointPtr->info.clear << 4) | (endpointPtr->info.direction)),
                endpointPtr->info.maxBurst,
                endpointPtr->info.bInterval);

            ilog_UPP_COMPONENT_3(ILOG_DEBUG, UPP_ENDPOINT_NEXT,
                (uint32_t)endpointPtr, (uint32_t)endpointPtr->link.next, (uint32_t)endpointPtr->link.prev);

            endpointPtr = (struct Endpoint *)endpointPtr->link.next;
        }

        devicePtr = (struct Device *)devicePtr->link.next;

        CALLBACK_RunSingle(UPP_PrintDevice, devicePtr, NULL);
    }
    else
    {
        ilog_UPP_COMPONENT_2(ILOG_USER_LOG, UPP_NUM_DEVICES,
            uppDevices_poolGetNumOfUsedElements(), uppDevices_poolGetNumOfFreeElements());

        ilog_UPP_COMPONENT_2(ILOG_USER_LOG, UPP_NUM_ENDPOINTS,
            uppEndpoints_poolGetNumOfUsedElements(), uppEndpoints_poolGetNumOfFreeElements());
    }
}

// Static Function Definitions ####################################################################
// #################################################################################################
// Validate Add device path and address
//
// Parameters:  UppDeviceLocation structure address
// Return:      enum UppAddDeviceErrorCode
// Assumptions:
//
// #################################################################################################
static enum UppAddDeviceErrorCode UPP_ValidateAddRoute(uint32_t routeNumber)
{
    enum UppAddDeviceErrorCode errorCode = UPP_TOPOLOGY_ADD_SUCCESS;

    struct Device *device = uppTopologyCtx.activeListHead;
    union UppDeviceLocation location;

    location.routeNumber = routeNumber;

    while (device)
    {
        if((device->route.routeNumber & ROUTE_PATH_MASK) == (location.routeNumber & ROUTE_PATH_MASK))
        {
            if(device->route.deviceAddress == location.deviceAddress)
            {
                ilog_UPP_COMPONENT_2(ILOG_MAJOR_ERROR, UPP_ADD_SAME_DEVICE,
                    location.deviceAddress,
                    device->route.routeNumber & ROUTE_PATH_MASK);

                errorCode = UPP_TOPOLOGY_SAME_DEVICE_EXIST;
            }
            else
            {
                ilog_UPP_COMPONENT_2(ILOG_MAJOR_ERROR, UPP_ADD_ROUTE_EXIST,
                    device->route.routeNumber & ROUTE_PATH_MASK,
                    device->route.deviceAddress);

                errorCode = UPP_TOPOLOGY_DEVICE_ROUTE_EXIST;
            }
            break;
        }
        else if(device->route.deviceAddress == location.deviceAddress)
        {
            ilog_UPP_COMPONENT_1(ILOG_MAJOR_ERROR, UPP_ADD_ADDRESS_EXIST, location.deviceAddress);
            errorCode = UPP_TOPOLOGY_DEVICE_ADDRESS_EXIST;
            break;
        }

        device =  (struct Device *)device->link.next;
    }

    return errorCode;
}

//#################################################################################################
// Remove a device
//
// Parameters:  UppDeviceLocation structure address
// Return:      bool (success/fail)
// Assumptions:
//
//#################################################################################################
static bool UPP_NextDeviceRemove(void)
{
    while (uppTopologyCtx.devicePtr)
    {
        // find all the devices that are included in this route mask
        if( (uppTopologyCtx.location.routeNumber & uppTopologyCtx.mask) ==
            (uppTopologyCtx.devicePtr->route.routeNumber & uppTopologyCtx.mask))
        {
            ilog_UPP_COMPONENT_3(ILOG_MAJOR_EVENT, UPP_FOUND_REMOVE_DEVICE,
                uppTopologyCtx.devicePtr->route.deviceAddress,
                uppTopologyCtx.devicePtr->route.routeNumber & ROUTE_PATH_MASK,
                (uint32_t)uppTopologyCtx.devicePtr);

            CALLBACK_Run(uppTopologyCtx.deviceRemoveCallback, uppTopologyCtx.devicePtr, NULL);
            return (true); // found a device to remove
        }

        uppTopologyCtx.devicePtr = (struct Device *)uppTopologyCtx.devicePtr->link.next;
    }

    return (false);  // no device found to remove
}

//#################################################################################################
// Remove a device
//
// Parameters:  UppDeviceLocation structure address
// Return:      bool (success/fail)
// Assumptions:
//
//#################################################################################################
static void UPP_TopologyChangeDone(uint8_t deviceAddress)
{
    if (uppTopologyCtx.pendingDeviceAdd.routeNumber)
    {
        enum UppAddDeviceErrorCode result = UPP_AddDevice(uppTopologyCtx.pendingDeviceAdd.routeNumber);

        if ( (result != UPP_TOPOLOGY_ADD_SUCCESS) && (result != UPP_TOPOLOGY_SAME_DEVICE_EXIST) )
        {
            UART_printf("Result for pending device add %d\n", result);
        }
    }

    uppTopologyCtx.pendingDeviceAdd.routeNumber = 0;
    CALLBACK_Run(uppTopologyCtx.topologyUpdateDone, NULL, (void *) (uint32_t)deviceAddress);
}


// #################################################################################################
// Free endpoint
//
// Parameters:  device pointer, endpoint pointer
// Return:
// Assumptions:
//
// #################################################################################################
static void UPP_FreeEndpoint(struct Device *device, struct Endpoint *endpoint)
{
    ilog_UPP_COMPONENT_3(ILOG_DEBUG, UPP_ENDPOINT_FREE,
        endpoint->info.number, endpoint->info.type, endpoint->route.location);

    struct UppDpLinkInfo removeLink =
    {
        .linkHead = (struct UppDoubleLink **)(&device->endpointHead),
        .linkTail = (struct UppDoubleLink **)(&device->endpointTail),
        .element  = &(endpoint->link)
    };

    UPP_DisconnectLinkedList(&removeLink);
    uppEndpoints_poolFree(endpoint);
}

// #################################################################################################
// Disconnect linked list
//
// Parameters:  Element to be removed, linked list head
// Return:
// Assumptions: Next and Prev is located on top of element's structure
//
// #################################################################################################
static void UPP_DisconnectLinkedList(struct UppDpLinkInfo *removeLink)
{
    struct UppDoubleLink **head = removeLink->linkHead;                // address of linkHead
    struct UppDoubleLink **tail = removeLink->linkTail;                // address of linkHead
    struct UppDoubleLink *removeElement = removeLink->element;

    struct UppDoubleLink *prev = removeElement->prev;
    struct UppDoubleLink *next = removeElement->next;

//    UART_printf("Disconnect1 head %x tail %x prev %x next %x\n", *head, *tail, prev, next);

    if(*head == removeElement)                                                  // Is this the head element?
    {
        if(*tail == removeElement)                                                  // Is this the only element?
        {
            *head = NULL;
            *tail = NULL;
        }
        else
        {
            *head = next;
            next->prev = NULL;
        }
    }
    else if(*tail == removeElement)                                                  // Is this the last element?
    {
        *tail = prev;
        prev->next = NULL;
    }
    else
    {
        // somewhere in the middle of the queue
        // now take it out of the queue
        next->prev = prev;
        prev->next = next;
    }

//    UART_printf("Disconnect2 head %x tail %x prev->n %x next->p %x\n", *head, *tail, prev->next, next->prev);

}

// #################################################################################################
// Connect linked list
//
// Parameters:  Element to be added, linked list head, callback for compare
// Return:
// Assumptions: Next and Prev is located on top of element's structure
//
// #################################################################################################
static void UPP_ConnectLinkedList(struct UppDpLinkInfo *addLink, bool (*compare)(void *, void *))
{
    struct UppDoubleLink **head = addLink->linkHead;                // address of linkHead
    struct UppDoubleLink **tail = addLink->linkTail;                // address of linkHead
    struct UppDoubleLink *newElement = addLink->element;

//    UART_printf("Connect1 head %x tail %x element %x\n", *head, *tail, newElement);

    if(*head == NULL)                                                  // Is this the only element?
    {
        *head = newElement;                               // head = newElement
        *tail = newElement;                               // tail = newElement
    }
    else if ((compare == NULL) || (compare(*tail, newElement) == FALSE))    // FALSE: new element is the last element
    {
        (*tail)->next = newElement;     // tail element's next  = newElement
        newElement->prev = *tail;       // newElement->prev = tail element
        *tail = newElement;             // tail = newElement
    }
    else
    {
        struct UppDoubleLink *elementPtr = *head;       // first element address that linkHead is pointing

        while(elementPtr)
        {
            if(compare(elementPtr, newElement))         // TRUE: new element needs to be inserted before the current element
            {
                newElement->next = elementPtr;          // newElement->next = element
                newElement->prev = elementPtr->prev;    // newElement->prev = element->prev

                if(elementPtr->prev == NULL)
                {
                    *head = newElement;                 // No prev -> newElement becomes head
                }
                else
                {
                    elementPtr->prev->next = newElement;
                }

                elementPtr->prev = newElement;          // element->prev = newElement
                break;
            }

            elementPtr = elementPtr->next;              // element = element->next
        }
    }

//    UART_printf("Connect2 head %x tail %x next %x prev %x\n", *head, *tail, newElement->next, newElement->prev);
}


// #################################################################################################
// UPP_DeviceSort
//
// Parameters:  compared device, target new device
// Return:
// Assumptions: TRUE means new device need to be located before compared device
//
// #################################################################################################
static bool UPP_DeviceSort(void *compare, void *new)
{
    struct Device *comparedDevice = (struct Device *)compare;
    struct Device *newDevice = (struct Device *)new;
    return (comparedDevice->route.routeNumber > newDevice->route.routeNumber);
}

// #################################################################################################
// UPP_EndpointSort
//
// Parameters:  compared endpoint, target new endpoint
// Return:
// Assumptions: TRUE means new endpoint need to be located before compared endpoint
//
// #################################################################################################
static bool UPP_EndpointSort(void *compare, void *new)
{
    struct Endpoint *comparedEndpoint = (struct Endpoint *)compare;
    struct Endpoint *newEndpoint = (struct Endpoint *)new;

    if(comparedEndpoint->route.location != newEndpoint->route.location)
    {
        return (comparedEndpoint->route.location > newEndpoint->route.location);
    }
    else
    {
        return (comparedEndpoint->info.number > newEndpoint->info.number);
    }
}

// #################################################################################################
// UPP_FindEndpointWithRoute
//
// Parameters:  endpoint head of a device, target endpoint route
// Return:
// Assumptions: Endpoint route is sorted
//              returns the first endpoint pointer of given endpoint route
//
// #################################################################################################
static struct Endpoint* UPP_FindEndpointWithRoute(struct Device *devicePtr, uint32_t route)
{
    struct Endpoint *endpointPtr = devicePtr->endpointHead;

    while ( (endpointPtr != NULL) && (endpointPtr->route.location != route) )
    {
        endpointPtr = (struct Endpoint *)endpointPtr->link.next;
    }

    return endpointPtr;
}


