///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2011, 2012
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
//!   @file  - topology_loc.h
//
//!   @brief - local header file for the topology
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TOPOLOGY_LOC_H
#define TOPOLOGY_LOC_H

/***************************** Included Headers ******************************/
#include <options.h>
#include <ibase.h>

#include <topology.h>
#include "topology_log.h"
#include "topology_cmd.h"

#include <grg.h>
#include <xcsr_xsst.h>
#include <xcsr_xicsq.h>

#include <xlr_msa.h>

#include <tasksch.h>
#include <timing_timers.h>

/************************ Defined Constants and Macros ***********************/
// The root device logical address will always be 0
#define ROOT_LOGICAL_ADDRESS 0

// Vendor ID 0x0471, Product ID 0x0815
#define MS_IR_DEVICE_VENDOR_ID  0x0471
#define MS_IR_DEVICE_PRODUCT_ID 0x0815

// The Logitech C910 webcam, which doesn't report endpoint 2 in its descriptors
#define LOGITECH_C910_DEVICE_VENDOR_ID  0x046D
#define LOGITECH_C910_DEVICE_PRODUCT_ID 0x0821

#define NOT_MSA 0

#define TOPOLOGY_MSA_LA_UNDEFINED 0

/******************************** Data Types *********************************/

typedef uint8 memPoolIndex_t;

struct DeviceTopology {

    // This union exists in order to save memory because the newUsbAddr field is only required at
    // times when the descriptorParserState field is not required.
    union {
        // Setup packet parsing info -- size defined in options.h
        uint8 descriptorParserState[DESCPARSER_SIZEOF_SETUP_RESPONSE] __attribute__((aligned(2))); // NOTE: 14 bytes used here
        uint8 newUsbAddr;
    } setupTransactionState;

    uint8 maxPacketSizeEndpoint0;

    memPoolIndex_t endpointDataList; // memory pool index of endpoint data list

    uint8 systemControlQueueState[SYSTEM_CONTROL_QUEUE_SIZEOF_STATUS] __attribute__((aligned(4)));

    // MSA information
    uint8 msaLA;             // The logical address of this MSA device

    // Virtual function information
    struct DTT_VF_EndPointHandles *pVirtualFunction;

    uint16 idProduct;
    uint16 idVendor;
    boolT deviceChanged;     // Set when a change is detected in idProduct or idVendor
                             // Cleared after the endpoints are cleaned up
    // Vital device info
    uint8 usbAddr;           // Usb address of the device
    uint8 portOnParent;      // The port number of the parent hub to which this device is connected
    uint8 parentLA;          // The logical address of the parent device
    uint8 childLA;           // The logical address of the child device connected to this device
    uint8 siblingLA;         // The logical address of the sibling device to this device
    uint8 highEndpoint;      // Highest endpoint number for the device


    // all bit fields grouped together
    struct {
        uint8 padding:5;

        uint8 isConnected:1;       // Is the device connected
        uint8 inTopology:1;        // Whether this address is in use or not
        uint8 requiresCleanup:1;   // When TRUE, indicates that the LAT and XSST must be cleaned up once
        // the topologyState.xsstCleanUpTimer expires
    };

    enum UsbSpeed speed;     // Speed of the device
    boolT isHub;             // Is this device a hub?

    // The configuration value is set to 0 on device reset
    uint8 configurationValue;

    // The number of configurations reported in the device descriptor.
    uint8 numConfigurations;

    // This value is a bit vector where setting the 0th bit means that configuration #1 has
    // been parsed and setting the 7th bit means that the configuration #8 has been parsed.
    // NOTE: Technically, the way this variable is used isn't really valid because configuration
    //       numbers do not need to be sequential.  For example, a device descriptor could state
    //       that there are 3 configuration descriptors, but the configurations could be numbered
    //       1, 7, 25 rather than 1, 2, 3.  In practice, there are no known devices which use
    //       configuration numbers that are non-sequential or do not start at 1.
    uint8 configurationsParsed;

};

typedef uint16 endpointData_t;

struct EndpointData
{
    uint8 configurationValue;
    uint8 interfaceNumber;
    uint8 alternateSetting;
    memPoolIndex_t next;
    struct
    {
        endpointData_t padding2:            2;
        endpointData_t isActive:            1; // Is this endpoint in an active alternate setting?
        endpointData_t endpointDirection:   1;
        endpointData_t msaPairEpNumber:     4; // 0: non-MSA
        endpointData_t padding1:            1;
        endpointData_t blockAccess:         1; // 1: blockAccess using i/oDcf in the XSST
        endpointData_t endpointType:        2;
        endpointData_t endpointNumber:      4;
    };
} __attribute__ ((aligned(4)));


/***************************** Global Variables ******************************/
// This is defined in device.c
extern struct DeviceTopology deviceTree[MAX_USB_DEVICES];


/*********************************** API *************************************/
void _DTT_XSSTMonInit(void);
void _DTT_XSSTMonitorProcess(XUSB_AddressT, uint8 endpoint, const boolT isHub);
void _DTT_ClearEndpoints(uint8 logicalAddress);

XUSB_AddressT _DTT_GetAddressFromLogical(uint8 logicalAddr) __attribute__ ((section(".lextext")));

sint8 _DTT_GetLogicalOrCreateNew(XUSB_AddressT parentAddress, uint8 portNumber);
boolT _DTT_Reset(XUSB_AddressT addressZero, enum DTT_OperationType resetType);


struct DeviceTopology * _DTT_GetDeviceNode(
    uint8 logicalAddr) __attribute__ ((noinline, section(".lextext"), const));

struct EndpointData* _DTT_memPoolIndexToEndpoint(
    memPoolIndex_t index) __attribute__ ((const));


void _DTT_freeMsa(XUSB_AddressT xusbAddr, struct DeviceTopology * pNode, boolT informRex)
    __attribute__ ((section(".lextext")));
void _DTT_clearMsaPair(MSA_AddressT msaAddr, uint8 ep)
    __attribute__ ((section(".lextext")));
void _DTT_ConfigureMsa(XUSB_AddressT xusbAddr, uint8 epIn, uint8 epOut)
    __attribute__ ((section(".lextext")));
boolT _DTT_isMsaActive(uint8 usbAddr, uint8 ep, enum EndpointDirection epDir)
    __attribute__ ((section(".lextext")));

static inline boolT _DTT_usbAddrValid(const struct DeviceTopology* dt)
{ return dt->inTopology && (dt->isConnected || dt->requiresCleanup); }

// for debugging
void _DTT_showEndpointData(uint8 logicalAddress);

// Interrupt Endpoint Monitor API
void _DTT_MarkInterruptEndpoints(uint8 logicalAddress, uint8 endpointNum);
void _DTT_ClearInterruptEndpoints(uint8 logicalAddress);
void _DTT_InterruptEndpointMonitorInit(void);

boolT isSplitDevice(uint8 deviceLA);

#endif // TOPOLOGY_LOC_H

