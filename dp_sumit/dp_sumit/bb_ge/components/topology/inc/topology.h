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
//!   @file  - topology.h
//
//!   @brief - device topology
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TOPOLOGY_H
#define TOPOLOGY_H

/***************************** Included Headers ******************************/
#include <xcsr_xsst.h>


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

// Used for to distinguish between bus/port resets and disconnects
enum DTT_OperationType
{
    DTT_DOWNSTREAM_PORT_OPERATION,
    DTT_UPSTREAM_BUS_OPERATION
};

enum TOPOLOGY_PortResetResult
{
    TOPOLOGY_PR_SUCCESS,
    TOPOLOGY_PR_NO_MORE_ADDRESSES,
    TOPOLOGY_PR_ADDRESS_ZERO_IN_USE
};


//Transaction State
struct transaction_state_t
{
    uint8  * currentRequestData;
    uint16 currentRequestDataLeft;
    uint16 current_wIndex;
    boolT  pid0Next;
};

struct DTT_VF_EndPointHandle
{
    void (*setup)(struct DTT_VF_EndPointHandle * pVF, uint8 bRequestType, uint8 bRequest, uint16
                  wValue, uint16 wIndex, uint16 wLength, XUSB_AddressT address);
    void (*in)   (struct DTT_VF_EndPointHandle * pVF, uint8 endpoint, XUSB_AddressT address,
                  boolT toggle);
    void (*inAck)(struct DTT_VF_EndPointHandle * pVF, uint8 endpoint, XUSB_AddressT address);
    void (*out)  (struct DTT_VF_EndPointHandle * pVF, uint8 endpoint, XUSB_AddressT address,
                  uint8 pid, uint8 * data, uint16 dataSize, boolT toggle);
    struct transaction_state_t state;
    uint8 epType;
    uint8 epMaxPacketSize;
};
struct DTT_VF_EndPointHandles
{
    uint8 numOfEndpoints;
    struct DTT_VF_EndPointHandle endpoint[];
};

typedef void (*DTT_DescriptionHandlerT)(
    uint8 usbAddress,
    uint8 parentUSBAddress,
    uint8 portOnParent,
    boolT isDeviceHub,
    uint16 vendorId,
    uint16 productId,
    void* cbData);


/*********************************** API *************************************/
void DTT_Init(void);

void DTT_assertHook(void);

void DTT_EnableAddress0(void); // Done after a reset

void  DTT_HostReset(void);
enum TOPOLOGY_PortResetResult DTT_PortReset(XUSB_AddressT parentAddress, uint8 portNumber, uint8 vport);

boolT DTT_SetAddress(const XUSB_AddressT oldAddress, XUSB_AddressT * pNewAddress);
void DTT_FinalizeSetAddress(const XUSB_AddressT oldAddress);

void DTT_HostDisconnect(void);
void DTT_PortDisconnect(XUSB_AddressT parentAddress, uint8 portNumber);
void DTT_DeviceDisconnect(XUSB_AddressT devAddress) __attribute__((noinline)); // For device class filtering

XUSB_AddressT DTT_GetAddressFromUSB(uint8 usbAddr) __attribute__ ((section(".lextext")));

void DTT_WriteEndpointData(XUSB_AddressT address,
    uint8 configurationValue,
    uint8 interfaceNumber,
    uint8 alternateSetting,
    uint8 endpointNumber,
    uint8 endpointType,
    enum EndpointDirection endpointDirection,
    boolT blockAccess);

void DTT_AddNewMsaPair(
    XUSB_AddressT address,
    uint8 configuration,
    uint8 interface,
    uint8 alternateSetting,
    uint8 inEndpointNumber,
    uint8 outEndpointNumber);
boolT DTT_IsMsa(XUSB_AddressT xusbAddr);

void DTT_SetOperatingSpeed(XUSB_AddressT parentAddress, uint8 portNumber, enum UsbSpeed speed);
void DTT_SetOperatingSpeedLA(uint8 LA, enum UsbSpeed speed);

void DTT_SetHub(XUSB_AddressT);
boolT DTT_IsHub(XUSB_AddressT);

void DTT_ResetEndpoint(XUSB_AddressT address, uint8 endpoint);
void DTT_ResetInterface(XUSB_AddressT address, uint8 interface);

// Device and Vendor ID
void DTT_CheckAndSetVendorId(XUSB_AddressT address, uint16 idVendor);
void DTT_CheckAndSetProductId(XUSB_AddressT address, uint16 idProduct);

// Configurations
void DTT_SetNumConfigurations(XUSB_AddressT address, uint8 numConfigurations);

// Current status
// Packet parsing
void * DTT_GetDescParserSetupTransactionState(XUSB_AddressT) __attribute__ ((const));
void * DTT_GetSystemControlQueueState(XUSB_AddressT) __attribute__ ((const));

void DTT_SetMaxPacketSizeEndpoint0(XUSB_AddressT, uint8);
uint8 DTT_GetMaxPacketSizeEndpoint0(XUSB_AddressT);

// For debugging (and internal icmds).  These functions can be called anywhere when attempting to
// debug an issue in higher level code
void DTT_showSingleDeviceXSST(uint8 logicalAddress);

// Virtual Function API
XUSB_AddressT DTT_HostResetVirtualFunction(struct DTT_VF_EndPointHandles * newVF);
enum TOPOLOGY_PortResetResult DTT_portResetVirtualFunction(struct DTT_VF_EndPointHandles * newVF, XUSB_AddressT parentAddress, uint8 portNumber, XUSB_AddressT *newAddress);
struct DTT_VF_EndPointHandles * DTT_GetVirtualFunction(XUSB_AddressT address);
XUSB_AddressT DTT_GetAddressFromVF(struct DTT_VF_EndPointHandles *pVF);
// When no handler exists.  To be used for control endpoints
void DTT_VFSendCtrlStallIn(struct DTT_VF_EndPointHandle * pVF, uint8 endpoint, XUSB_AddressT address, boolT toggle);
void DTT_VFSendCtrlStallOut(struct DTT_VF_EndPointHandle * pVF, uint8 endpoint, XUSB_AddressT address, uint8 pid, uint8 * data, uint16 dataSize, boolT toggle);

// For USB TestMode
static inline boolT DTT_IsRootDevice(XUSB_AddressT address);

// Bug workarounds
void DTT_bug1418_SuspendNotify(void);
void DTT_bug1418_NotSuspendNotify(void);

// Remove any unnecessary nodes from Endpoint info list
void DTT_ParseConfigurationDone(XUSB_AddressT address, uint8 configurationNum);

// Set alternate setting for an interface
void DTT_SetInterface(XUSB_AddressT address, uint8 interface, uint8 alternateSetting);

// Set different configuration for a device
boolT DTT_SetConfiguration(XUSB_AddressT address, uint8 configurationValue);


void DTT_describeTopology(DTT_DescriptionHandlerT dhCallback, void* cbData);

/************************ Static Inline Definitions **************************/

/**
* FUNCTION NAME: DTT_IsRootDevice()
*
* @brief  - Determine if the device with the given USB address is the root device
*
* @return - TRUE if the given USB address corresponds to the root device.
*
* @note   - The device with logical address 0 will always be the root device because the first
*           device added to the system will be the root device and that device will be assigned the
*           first available address which will be 0.
*/
static inline boolT DTT_IsRootDevice(XUSB_AddressT address)
{
    return (XCSR_getXUSBAddrLogical(address) == 0);
}


#endif // TOPOLOGY_H

