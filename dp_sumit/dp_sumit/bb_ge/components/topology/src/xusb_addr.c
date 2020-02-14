///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011, 2012
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
//!   @file  -  xusb_addr.c
//
//!   @brief -  Handles the use cases for XUSB_AddressT.  Mostly initialization
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "topology_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void _DTT_GetAddressAssertHelper(uint8 usbAddr, uint32 line) __attribute__((noinline, noreturn));

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _DTT_GetAddressAssertHelper()
*
* @brief  - Asserts with a topology invalid message
*
* @return - never
*
* @note   - Helper function to save IRAM
*
*/
static void _DTT_GetAddressAssertHelper(uint8 usbAddr, uint32 line)
{
    iassert_TOPOLOGY_COMPONENT_2(
        FALSE, TOPOLOGY_INVALID_WHEN_GET_ADDR_FROM_USB, usbAddr, line);
    __builtin_unreachable();
}


/**
* FUNCTION NAME: DTT_GetAddressFromUSB()
*
* @brief  - Get the XUSB address from a USB Address
*
* @return - A fully populated XUSB address
*/
XUSB_AddressT DTT_GetAddressFromUSB
(
    uint8 usbAddr  // usb address to look up
)
{
    XUSB_AddressT address;
    struct DeviceTopology * pNode;

    // Make sure the requested usb address is valid
    if (MAX_USB_ADDRESS < usbAddr)
        _DTT_GetAddressAssertHelper(usbAddr, __LINE__);

    // Get address information from the LAT
    address = XCSR_XSSTGetXUSBAddrFromLAT(usbAddr);

    // address now is populated from the LAT, but since the LAT contains all
    // 128 possible addresses, while there are far fewer logical addresses, we
    // need to ensure the validity of this logical address
    if (XCSR_getXUSBAddrValid(address))
    {
        // _DTT_GetDeviceNode() has a check to see if the logical address is valid
        // set up the pNode pointer
        pNode = _DTT_GetDeviceNode(XCSR_getXUSBAddrLogical(address));

        // And ensure the usb address valid bit matches SW information
        if (pNode->usbAddr != usbAddr)
            _DTT_GetAddressAssertHelper(usbAddr, __LINE__);

        // USB address is valid
        if (!_DTT_usbAddrValid(pNode))
            _DTT_GetAddressAssertHelper(usbAddr, __LINE__);

        XCSR_setXUSBAddrVirtFuncPointer(&address, pNode->pVirtualFunction);
    }

    // Regardless of LG1 or GE, if the address isn't valid, it must not be in-sys
    if (!XCSR_getXUSBAddrValid(address))
    {
        if (XCSR_getXUSBAddrInSys(address))
            _DTT_GetAddressAssertHelper(usbAddr, __LINE__);
    }
    return address;
}


/**
* FUNCTION NAME: _DTT_GetAddressFromLogical()
*
* @brief  - Get the XUSB address from a logical address
*
* @return - A fully populated XUSB address
*/
XUSB_AddressT _DTT_GetAddressFromLogical
(
    uint8 logicalAddr
)
{
    XUSB_AddressT address;
    struct DeviceTopology * pNode;

    pNode = _DTT_GetDeviceNode(logicalAddr);

    if (_DTT_usbAddrValid(pNode))
    {
        address = DTT_GetAddressFromUSB(pNode->usbAddr);
        iassert_TOPOLOGY_COMPONENT_3(
            logicalAddr == XCSR_getXUSBAddrLogical(address), GET_ADDR_FROM_LOGICAL_MISMATCH,
            logicalAddr, XCSR_getXUSBAddrUsb(address), XCSR_getXUSBAddrLogical(address));
    }
    else
    {
        // This address isn't valid. Don't do a lookup involving the LAT
        XCSR_initXUSBAddr(&address);
        XCSR_setXUSBAddrUsb(&address, pNode->usbAddr);
        XCSR_setXUSBAddrLogical(&address, logicalAddr);
        XCSR_setXUSBAddrInSys(&address, 0);
        XCSR_setXUSBAddrValid(&address, 0);
        XCSR_setXUSBAddrVirtFuncPointer(&address, pNode->pVirtualFunction);
    }

    return address;
}

