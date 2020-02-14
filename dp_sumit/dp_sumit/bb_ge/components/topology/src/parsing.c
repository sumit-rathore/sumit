///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  - parsing.c
//
//!   @brief - tracks the current setup packet parsing state for a device
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "topology_loc.h"
#include <storage_Data.h>


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void handleNewDevice(XUSB_AddressT, struct DeviceTopology *, uint16 idVendor, uint16 idProduct) __attribute__((noinline));

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: DTT_GetDescParserSetupTransactionState()
*
* @brief  - Returns a pointer to the structure representing the descriptor
*           parser's state during a setup transaction for the device with the
*           given address.
*
* @return - void pointer that can be interpreted by the descriptor parser
*/
void * DTT_GetDescParserSetupTransactionState(XUSB_AddressT address)
{
    const uint8 logicalAddr = XCSR_getXUSBAddrLogical(address);
    return _DTT_GetDeviceNode(logicalAddr)->setupTransactionState.descriptorParserState;
}


/**
* FUNCTION NAME: DTT_GetSystemControlQueueState()
*
* @brief  - Returns a pointer to the current System Queue state for the given
*           device
*
* @return - void * to the structure that is defined in the system control queue
*/
void * DTT_GetSystemControlQueueState(XUSB_AddressT address)
{
    const uint8 logicalAddr = XCSR_getXUSBAddrLogical(address);
    return _DTT_GetDeviceNode(logicalAddr)->systemControlQueueState;
}


/**
* FUNCTION NAME: DTT_SetHub()
*
* @brief  - Set whether or not this device is a hub
*
* @return - nothing
*/
void DTT_SetHub
(
    XUSB_AddressT address  // xusb address
)
{
    _DTT_GetDeviceNode(XCSR_getXUSBAddrLogical(address))->isHub = TRUE;
}


/**
* FUNCTION NAME: DTT_IsHub()
*
* @brief  - Checks if this is a hub
*
* @return - TRUE if it is a hub, FALSE otherwise
*/
boolT DTT_IsHub(XUSB_AddressT address)
{
    const uint8 logicalAddr = XCSR_getXUSBAddrLogical(address);
    return _DTT_GetDeviceNode(logicalAddr)->isHub;
}

void DTT_SetMaxPacketSizeEndpoint0(XUSB_AddressT address, uint8 maxPacketSize)
{
    _DTT_GetDeviceNode(XCSR_getXUSBAddrLogical(address))->maxPacketSizeEndpoint0 = maxPacketSize;
}


uint8 DTT_GetMaxPacketSizeEndpoint0(XUSB_AddressT address)
{
    return _DTT_GetDeviceNode(XCSR_getXUSBAddrLogical(address))->maxPacketSizeEndpoint0;
}


/**
* FUNCTION NAME: DTT_CheckAndSetVendorId
*
* @brief  -
*
* @return - void
*/
void DTT_CheckAndSetVendorId
(
    XUSB_AddressT address,
    uint16 idVendor
)
{
    const uint8 logicalAddr = XCSR_getXUSBAddrLogical(address);
    struct DeviceTopology* deviceNode = _DTT_GetDeviceNode(logicalAddr);

    // If the new Vendor ID is not equal to the existing Vendor ID
    if (deviceNode->idVendor != idVendor)
    {
        deviceNode->deviceChanged = TRUE;

        ilog_TOPOLOGY_COMPONENT_3(
            ILOG_DEBUG,
            FOUND_NEW_VENDOR,
            XCSR_getXUSBAddrUsb(address),
            deviceNode->idVendor,
            idVendor);

        deviceNode->idVendor = idVendor;
    }
}


/**
* FUNCTION NAME: DTT_CheckAndSetProductId()
*
* @brief  -
*
* @return - void
*/
void DTT_CheckAndSetProductId
(
    XUSB_AddressT address,
    uint16 idProduct
)
{
    const uint8 logicalAddr = XCSR_getXUSBAddrLogical(address);
    struct DeviceTopology* deviceNode = _DTT_GetDeviceNode(logicalAddr);
    const uint16 idVendor = deviceNode->idVendor;

    if (deviceNode->idProduct != idProduct)
    {
        ilog_TOPOLOGY_COMPONENT_3(
            ILOG_DEBUG,
            FOUND_NEW_PRODUCT,
            XCSR_getXUSBAddrUsb(address),
            deviceNode->idProduct,
            idProduct);
        deviceNode->deviceChanged = TRUE;
        deviceNode->idProduct = idProduct;
    }

    handleNewDevice(address, deviceNode, idVendor, idProduct);
}


/**
* FUNCTION NAME: DTT_SetNumConfigurations()
*
* @brief  - Sets the number of configurations defined on the given device as read from the USB
*           device descriptor.
*
* @return - void.
*/
void DTT_SetNumConfigurations(XUSB_AddressT address, uint8 numConfigurations)
{
    const uint8 logicalAddr = XCSR_getXUSBAddrLogical(address);
    struct DeviceTopology* deviceNode = _DTT_GetDeviceNode(logicalAddr);
    const uint8 numConfigurationsSupported = sizeof(deviceNode->configurationsParsed) * 8;

    deviceNode->numConfigurations = numConfigurations;
    if (numConfigurations > numConfigurationsSupported)
    {
        ilog_TOPOLOGY_COMPONENT_3(
            ILOG_WARNING,
            TOPOLOGY_TOO_MANY_CONFIGURATIONS,
            logicalAddr,
            numConfigurations,
            numConfigurationsSupported);
    }
}


/**
* FUNCTION NAME: handleNewDevice()
*
* @brief  - Whenever a new device is found.  Clean up the old device
*
* @return - void
*
* @note   - This is not in IRAM, as LG1 IRAM is full
*/
static void handleNewDevice
(
    XUSB_AddressT address,
    struct DeviceTopology *deviceNode,
    uint16 idVendor,
    uint16 idProduct
)
{
    if (deviceNode->deviceChanged)
    {
        ilog_TOPOLOGY_COMPONENT_3(ILOG_MAJOR_EVENT, NEW_DEVICE, XCSR_getXUSBAddrUsb(address), idVendor, idProduct);
        deviceNode->deviceChanged = FALSE;

        // Clean up the MSA
        _DTT_freeMsa(address, deviceNode, FALSE);

        deviceNode->highEndpoint = 0;
        deviceNode->isHub = FALSE;

        deviceNode->configurationValue = 0;
        deviceNode->numConfigurations = 0;
        deviceNode->configurationsParsed = 0;

        // Clear Endpoint information in data structure
        _DTT_ClearEndpoints(XCSR_getXUSBAddrLogical(address));

        // Do specific device work arounds
        if ((idVendor == LOGITECH_C910_DEVICE_VENDOR_ID) && (idProduct == LOGITECH_C910_DEVICE_PRODUCT_ID))
        {
            uint8 interface;
            uint8 alternateSetting;
            uint8 endpointNumber;
            uint8 endpointType;
            uint8 configuration;
            uint8 endpointDirection;
            uint8 blockAccess;

            // C910 workarounds
            ilog_TOPOLOGY_COMPONENT_1(ILOG_WARNING, LOGITECH_C910_FOUND, XCSR_getXUSBAddrUsb(address));

            configuration = 1;
            endpointDirection = EP_DIRECTION_IN;
            interface = 0;
            alternateSetting = 0;
            endpointNumber = 2;
            endpointType = EP_TYPE_ISO;
            blockAccess = ((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >>
                            TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET) & 0x1) == 0;

            // Interface: 2, MSA: FALSE, Alternate Setting: 0, Endpoint: 2
            DTT_WriteEndpointData(
                address,
                configuration,
                interface,
                alternateSetting,
                endpointNumber,
                endpointType,
                endpointDirection,
                blockAccess);
        }
    }
}

