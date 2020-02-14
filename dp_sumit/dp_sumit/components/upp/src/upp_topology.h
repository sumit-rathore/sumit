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
#ifndef UPP_TOPOLOGY_H
#define UPP_TOPOLOGY_H

// Includes #######################################################################################
#include <ibase.h>

// Constants and Macros ###########################################################################
#define MAX_NUM_USB_DEVICE          32
#define MAX_NUM_UPP_ENDPOINT        (254)
#define MAX_NUM_TIER                5
#define ROUTE_PATH_MASK             0xFFFFF000

#define UPP_INTERFACE_ALT_MASK      0x00FFFFFF      // mask to get the interface alt setting location
#define UPP_INTERFACE_LOCATION_MASK 0x00FFFF00      // mask to get the interface location
#define UPP_INTERFACE_CONFIG_MASK   0x00FF0000      // mask to just get the interface for this configuration

#define NOTIFY_ACTIVE_ENDPOINT      true            // endpoint becomes active from inactive
#define NOTIFY_INACTIVE_ENDPOINT    false           // endpoint becomes inactive from active

// when looking for a device's interface endpoint, don't care about the alt setting
#define UPP_ENDPOINT_INTERFACE_MASK     0xFFFFFF00

// only care about the device address and configuration
#define UPP_ENDPOINT_CONFIG_MASK        0xFFFF0000

// only care about the device address, configuration, and Alt interface setting = 0
#define UPP_ENDPOINT_CONFIG_ALT_MASK    0xFFFF00FF

// we want the exact location
#define UPP_ENDPOINT_LOCATION_MASK      0xFFFFFFFF

#define UPP_INVALID_INDEX               0xFF  // invalid index used to mark an empty index

// Data Types #####################################################################################
enum UppAddDeviceErrorCode
{
    UPP_TOPOLOGY_ADD_SUCCESS,               // successfully add a new device
    UPP_TOPOLOGY_FULL,                      // can't allocate more device
    UPP_TOPOLOGY_DEVICE_ROUTE_EXIST,        // the route path is already using
    UPP_TOPOLOGY_DEVICE_ADDRESS_EXIST,      // the address is already using
    UPP_TOPOLOGY_SAME_DEVICE_EXIST,         // the address and route path is already defined (same device?)
};

enum UPPEndpointMarkOp
{
    UPP_ENDPOINT_MARKED_CLEAR = 0,  // Endpoint is marked or should be marked to clear
    UPP_ENDPOINT_MARKED_SET,        // Endpoint is marked or should be marked to be set
};


// this structure allows us to define a unique key for every device we are tracking
union UppDeviceLocation
{
    struct
    {
        uint32_t routeStringHub1 :4;        // the route string, tier #1 Hub (see USB3.1 section 8.9)
        uint32_t routeStringHub2 :4;        // the route string, tier #2 Hub
        uint32_t routeStringHub3 :4;        // the route string, tier #3 Hub
        uint32_t routeStringHub4 :4;        // the route string, tier #4 Hub
        uint32_t routeStringHub5 :4;        // the route string, tier #5 Hub
        uint32_t reserved :4;               // zeroed
        uint32_t deviceAddress :8;          // Address of this device
    };

    uint32_t routeNumber;                   // To compare route topology as one number
};

union EndpointRoute
{
    struct
    {
        uint32_t deviceAddress              :8;     // Top level device address
        uint32_t configurationNumber        :8;     // the configuration this endpoint belongs to
        uint32_t interfaceNumber            :8;     // the interface this endpoint belongs to
        uint32_t alternateInterfaceNumber   :8;     // the alternate interface this endpoint belongs to
    };
    uint32_t location;                         // 32 bit representation of the route, for sorting
};

union EndpointInfo
{
    struct
    {
        uint32_t reserved                   : 6;    // unused bits
        uint32_t active                     : 1;    // true if this endpoint's interface, alt setting is active
        uint32_t set                        : 1;    // true if this endpoint should be set active
        uint32_t clear                      : 1;    // true if this endpoint should be cleared (set inactive)
        uint32_t direction                  : 1;    // 1:device to host
        uint32_t type                       : 2;    // space for enum EndpointTransferType (Control, ISO, Bulk, Interrupt)
        uint32_t number                     : 4;    // 1-15 (0 reserved for control transfers)
        uint32_t assignedQueue              : 8;    // FPGA assigned queue, 0 = common
        uint32_t bInterval                  : 4;    // see section 9.6.6 USB 3.1 spec, endpoint descriptor
        uint32_t maxBurst                   : 4;    // taken from bMaxBurst on super speed companion descriptor
    };

    uint32_t endpointInfo;
};

struct UppDoubleLink
{
    struct UppDoubleLink *next;                     // the next item in the list, NULL if at start
    struct UppDoubleLink *prev;                     // the previous item on the list, NULL if at the end
};

struct UppEndpointData
{
    union EndpointRoute route;                      // which device this endpoint belongs to, and configuration, interface
    union EndpointInfo  info;                       // essential values of EndpointDescriptor
};

// next and prev should be located on top
// list ordered by endpoint route
struct Endpoint
{
    struct UppDoubleLink link;                      // link in this list, must be first member of this structure

    union EndpointRoute route;                      // which device this endpoint belongs to, and configuration, interface
    union EndpointInfo  info;                       // essential values of EndpointDescriptor
};

struct UppDpLinkInfo
{
    struct UppDoubleLink **linkHead;                // indicating address of link header having first element's address
    struct UppDoubleLink **linkTail;                // indicating address of link tail having last element's address

    struct UppDoubleLink *element;                  // element to be added to linked list
};

// marks a configuration+interface+alternate interface supported by this device
union UppConfigInterface
{
    struct {
        uint32_t active                     :1;     // true = this interface is active
        uint32_t reserved                   :7;
        uint32_t configurationNumber        :8;
        uint32_t interfaceNumber            :8;
        uint32_t alternateInterfaceNumber   :8;
    };
    uint32_t configInterface;
};

//union UppConfigInterface interfacePool[UPP_MAX_CONFIG_INTERFACE];
//struct doubleListNodeIdx interfaceList[UPP_MAX_CONFIG_INTERFACE];
//struct doubleListIndexInfo
//{
//
//};

// next and prev should be located on top
// list ordered by route number
struct Device
{
    struct UppDoubleLink link;                      // link in this list, must be first member of this structure

    union UppDeviceLocation route;                  // route string and address

    uint8_t newConfiguration;                       // which configuration we are using
    uint8_t currentConfiguration;                   // which configuration we are using

    union EndpointRoute newInterface;               // new interface we want to set, 0 if done

    struct Endpoint *endpointHead;                  // first endpoint of this device
    struct Endpoint *endpointTail;                  // For sorting endpoint faster in case of sorted descriptor case
};

void UPP_TopologyInit(  CallbackFunctionPtr deviceRemoveCallback,       // callback for notifying a device to be remove
                        CallbackFunctionPtr TopologyChangeEndCallback) // callback for notifying when a topology change is done!
    __attribute__ ((section(".lexatext")));

void UPP_TopologyReinit(void)                                   __attribute__ ((section(".lexatext")));

// endpoint cache functions
void UPP_CacheEndpointData(struct UppEndpointData *endpointData)    __attribute__((section(".lexftext")));
void UPP_AddEndpointDataFromCache(struct Device *device)            __attribute__((section(".lexftext")));
void UPP_ClearDeviceEndpointCache(uint8_t deviceAddress)            __attribute__((section(".lexftext")));
void UPP_PurgeEndpointCache(void)                                   __attribute__((section(".lexftext")));

enum UppAddDeviceErrorCode UPP_AddDevice(uint32_t routeNumber)  __attribute__((section(".lexftext")));  // add a device
void UPP_PendingAddDevice(uint32_t routeNumber)                 __attribute__((section(".lexftext")));  // add a device after a removal is complete
bool UPP_RemoveDevice(uint32_t routeNumber)                     __attribute__((section(".lexftext")));  // remove a device and all connected devices from ther topology map
void UPP_FreeDevice(struct Device *device)                      __attribute__((section(".lexftext")));  // Frees a device and all of its associated endpoints
struct Device* UPP_GetDevice(uint8_t deviceAddress)             __attribute__((section(".lexftext")));  // Gets the specified device, returns NULL if doesn't exist


// Set a endpoint and return the added endpoint struct pointer
struct Endpoint* UPP_AddEndpoint(   struct Device *device,  struct UppEndpointData *endpoint)                       __attribute__((section(".lexftext")));
struct Endpoint* UPP_GetEndpoint(struct Device *device, uint32_t routeNumber, uint8_t endpointNumber)               __attribute__((section(".lexftext")));

void UPP_SetEndpointActiveState(struct Endpoint *endpoint, bool activeState)                                        __attribute__((section(".lexftext")));

bool UPP_EndpointMarkOnMask(struct Device *devicePtr, uint32_t location, uint32_t mask, bool setEndpoint)           __attribute__((section(".lexftext")));
struct Endpoint* UPP_GetNextMarkedEndpoint(struct Device *devicePtr, struct Endpoint *endpoint, bool setEndpoint)   __attribute__((section(".lexftext")));


// Test functions
void UPP_RemoveEndpoint(uint8_t deviceAddress, uint8_t endpointNumber);
void UPP_PrintDevices(void);




#endif // UPP_TOPOLOGY_H
