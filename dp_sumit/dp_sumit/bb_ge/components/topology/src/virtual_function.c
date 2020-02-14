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
//!   @file  - vitual_function.c
//
//!   @brief - contains code needed for virtual function implementations
//
//
//!   @note  - Currently this file is only include for GE builds
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "topology_loc.h"
#include "xcsr_xusb.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: DTT_HostResetVirtualFunction()
*
* @brief  - Resets the root device, which is a virtual function
*
* @return - The new XUSB_AddressT
*
* @note   - 
*
*/
XUSB_AddressT DTT_HostResetVirtualFunction
(
    struct DTT_VF_EndPointHandles * newVF
)
{
    XUSB_AddressT addressZero;
    // NOTE: the root device is always logical address 0
    // Currently it will also be USB address 0, as it is getting reset
    XCSR_initXUSBAddr(&addressZero); // Sets all fields to zero, including logical & USB

    XCSR_setXUSBAddrVirtFuncPointer(&addressZero, newVF);

    iassert_TOPOLOGY_COMPONENT_0(
        _DTT_Reset(addressZero, DTT_UPSTREAM_BUS_OPERATION),
        TOPOLOGY_HOST_RESET_SHOULD_NEVER_FAIL);

    return addressZero;
}


/**
* FUNCTION NAME: DTT_portResetVirtualFunction()
*
* @brief  - resets a non-root virtual function
*
* @return - success, or failure code
*
* @note   - to be used with virtual devices that are not the root virtual device
*
*           Any caller of this would be a virtual function behind a VHub
*/
enum TOPOLOGY_PortResetResult DTT_portResetVirtualFunction
(
    struct DTT_VF_EndPointHandles * newVF,
    XUSB_AddressT parentAddress,
    uint8 portNumber,
    XUSB_AddressT *newAddress // out paramater
)
{
    enum TOPOLOGY_PortResetResult result = TOPOLOGY_PR_NO_MORE_ADDRESSES;
    // find the device to reset
    sint8 laToReset = _DTT_GetLogicalOrCreateNew(parentAddress, portNumber);

    // If we don't have a valid laToReset at this point, then there are no more available addresses
    if(laToReset != -1)
    {
        // Setup the new address for all fields that are non-zero
        // NOTE: leaving in-sys and valid zero, as _DTT_Reset will set those fields
        XCSR_initXUSBAddr(newAddress);
        XCSR_setXUSBAddrLogical(newAddress, laToReset);
        XCSR_setXUSBAddrVirtFuncPointer(newAddress, newVF);

        result =
            _DTT_Reset(*newAddress, DTT_DOWNSTREAM_PORT_OPERATION) ?
                TOPOLOGY_PR_SUCCESS : TOPOLOGY_PR_ADDRESS_ZERO_IN_USE;
    }
    return result;
}


/**
* FUNCTION NAME: DTT_GetVirtualFunction()
*
* @brief  - Gets a pointer to the virtual function structure
*
* @return - A pointer to the virtual function implementing the device with the given address, or
*           null if this device isn't a virtual function.
*
* @note   - This only adds assert checks around the XUSB_AddressT getter function
*
*/
struct DTT_VF_EndPointHandles * DTT_GetVirtualFunction(XUSB_AddressT address)
{
    iassert_TOPOLOGY_COMPONENT_2(
        (XCSR_getXUSBAddrLogical(address) < MAX_USB_DEVICES),
        TOPOLOGY_INVALID_LOGICAL_ADDRESS_ARG, XCSR_getXUSBAddrLogical(address), __LINE__);

    // TODO: Could an assert that the deviceTree and the address value are in sync.  Or just get
    // rid of this altogether.
    //return deviceTree[XCSR_getXUSBAddrLogical(address)].pVirtualFunction;
    return XCSR_getXUSBAddrVirtFuncPointer(address);
}


/**
* FUNCTION NAME: DTT_GetAddressFromVF()
*
* @brief  - Gets an XUSB address from a virtual function
*
* @return - XUSB_AddressT.  On failure the address will have the valid field set to FALSE.
*
* @note   - This is somewhat annoying.  The purpose of this function is so that VFs don't have to
*           care about a SetAddress occuring.
*
*/
XUSB_AddressT DTT_GetAddressFromVF(struct DTT_VF_EndPointHandles *pVF)
{
    sint8 logical_address;
    XUSB_AddressT retVal;
    XCSR_initXUSBAddr(&retVal); // initializes all fields to zero

    for(logical_address = 0; logical_address < MAX_USB_DEVICES; logical_address++)
    {
        struct DeviceTopology* node = _DTT_GetDeviceNode(logical_address);
        if((node->pVirtualFunction == pVF) && _DTT_usbAddrValid(node))
        {
            XCSR_setXUSBAddrUsb(&retVal, node->usbAddr);
            XCSR_setXUSBAddrValid(&retVal, TRUE);
            XCSR_setXUSBAddrLogical(&retVal, logical_address);
            XCSR_setXUSBAddrInSys(&retVal, node->inTopology);
            XCSR_setXUSBAddrVirtFuncPointer(&retVal, node->pVirtualFunction);
            return retVal;
        }
    }

    return retVal;
}


void DTT_VFSendCtrlStallIn(
    struct DTT_VF_EndPointHandle * pVF, uint8 endpoint, XUSB_AddressT address, boolT toggle)
{
    struct XCSR_XICSQueueFrame* frame;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

    // TODO: ilog

    // Build frame
    frame->dataSize = 0; // This is a stall, there is no data
    XCSR_XICSBuildUpstreamFrameHeader(
        frame, address, endpoint, 0, frame->dataSize, XUSB_IN, XUSB_STALL, pVF->epType, toggle);

    // Send the frame to the host
    XCSR_XUSBSendUpstreamUSBFrame(address, endpoint, TRUE, frame);

    // Reset all the handlers
    pVF->in     =   &DTT_VFSendCtrlStallIn;
    pVF->inAck  =   NULL;
    pVF->out    =   &DTT_VFSendCtrlStallOut;
}


void DTT_VFSendCtrlStallOut(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    uint8 pid,
    uint8 * data,
    uint16 dataSize,
    boolT toggle)
{
    struct XCSR_XICSQueueFrame* frame;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

    // TODO: ilog

    // Build frame
    frame->dataSize = 0; // This is a stall, there is no data
    XCSR_XICSBuildUpstreamFrameHeader(
        frame, address, endpoint, 0, frame->dataSize, XUSB_OUT, XUSB_STALL, pVF->epType, toggle);

    // Send the frame to the host
    XCSR_XUSBSendUpstreamUSBFrame(address, endpoint, FALSE, frame);

    // Reset all the handlers
    pVF->in     =   &DTT_VFSendCtrlStallIn;
    pVF->inAck  =   NULL;
    pVF->out    =   &DTT_VFSendCtrlStallOut;
}

