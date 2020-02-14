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
//!   @file  - topology_icmds.c
//
//!   @brief - icmds for the topology component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "topology_loc.h"
#include <leon_uart.h>
#include <icmd.h>


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void _DTT_showVirtualFunction(struct DTT_VF_EndPointHandles* vf);


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: DTT_showSingleDeviceXSST()
*
* @brief  - Show the XSST entry for a single device
*
* @return - void
*
* @note   - This function is not exposed as an icmd, but is used by other icmds and is useful for
*           debugging.
*/
void DTT_showSingleDeviceXSST
(
    uint8 logicalAddress
)
{
    // Check if this is a valid logical address first
    if (logicalAddress < MAX_USB_DEVICES)
    {
        if (GRG_IsDeviceLex())
        {
            XUSB_AddressT address;

            // Check if the device exists
            address = _DTT_GetAddressFromLogical(logicalAddress);

            if (XCSR_getXUSBAddrValid(address))
            {
                XCSR_LATShowEntry(XCSR_getXUSBAddrUsb(address));
                XCSR_XSSTShowEntry(address);
            }
            else
            {
                uint8 usbAddr;
                // The selected logical address is not valid in the LAT, however still display the XSST.
                // Find an unused usb address to assign to the passed in logical address so we can
                // retrieve the info in the XSST and display it
                for (usbAddr = 127; usbAddr > 0; usbAddr--)
                {
                    address = DTT_GetAddressFromUSB(usbAddr);
                    if (!XCSR_getXUSBAddrValid(address))
                    {
                        XCSR_setXUSBAddrLogical(&address, logicalAddress);
                        ilog_TOPOLOGY_COMPONENT_1(
                                ILOG_USER_LOG,
                                TOPOLOGY_ICMD_VIEWING_ADDRESS_NOT_IN_SYS2,
                                XCSR_getXUSBAddrLogical(address));
                        XCSR_XSSTUpdateAddress(address, FALSE);
                        XCSR_XSSTShowEntry(address);
                        // Clean up the logical address lookup table
                        XCSR_XSSTClearLAT(address);
                        break;
                    }
                }
            }
        }
        else
        {
            ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_ICMD_NOT_LEX);
        }
    }
    else
    {
        ilog_TOPOLOGY_COMPONENT_1(ILOG_USER_LOG, TOPOLOGY_INVALID_LA_IN_ICMD, logicalAddress);
    }
}


/**
* FUNCTION NAME: _DTT_showDeviceXSST()
*
* @brief  - Show the XSST entry for a single device
*
* @return - void
*/
void _DTT_showDeviceXSST
(
    uint8 logicalAddress // Device logical address
)
{
    ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_ICMD_SHOW_DEVICE);
    DTT_showSingleDeviceXSST(logicalAddress);
}


/**
* FUNCTION NAME: _DTT_showAllDeviceXSST()
*
* @brief  - Show the XSST entry for all devices in-sys
*
* @return - void
*/
void _DTT_showAllDeviceXSST(void)
{
    ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_ICMD_SHOW_DEVICE_ALL);

    if (GRG_IsDeviceLex())
    {
        uint8 logicalAddr;

        for (logicalAddr = 0; logicalAddr < MAX_USB_DEVICES; logicalAddr++)
        {
            XUSB_AddressT address = _DTT_GetAddressFromLogical(logicalAddr);
            if (XCSR_getXUSBAddrValid(address) == 0)
            {
                //find a usb address to temporarily assign to the LA to display it
                DTT_showSingleDeviceXSST(logicalAddr);
            }
            else
            {
                XCSR_XSSTShowEntry(address);
            }
            LEON_UartWaitForTx();
        }
    }
    else
    {
        ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_ICMD_NOT_LEX);
    }
}


/**
* FUNCTION NAME: _DTT_showSingleDeviceTopology()
*
* @brief  - An icmd to display the topology information of a single device.
*
* @return - void.
*/
void _DTT_showSingleDeviceTopology(uint8 LA, uint8 view)
{
    const struct DeviceTopology* node = _DTT_GetDeviceNode(LA);
    const boolT compactOutput = (view == 0);

    ilog_TOPOLOGY_COMPONENT_2(ILOG_USER_LOG, TOPOLOGY_SHOW_TOPOLOGY_1, LA, node->usbAddr);

    ilog_TOPOLOGY_COMPONENT_3(
        ILOG_USER_LOG, TOPOLOGY_SHOW_TOPOLOGY_2, node->parentLA, node->childLA, node->siblingLA);

    ilog_TOPOLOGY_COMPONENT_3(
        ILOG_USER_LOG, TOPOLOGY_SHOW_TOPOLOGY_3,
        node->portOnParent, node->speed, node->highEndpoint);

    ilog_TOPOLOGY_COMPONENT_2(
        ILOG_USER_LOG, TOPOLOGY_SHOW_TOPOLOGY_4, node->isHub, node->isConnected);

    ilog_TOPOLOGY_COMPONENT_3(
        ILOG_USER_LOG, TOPOLOGY_SHOW_TOPOLOGY_5,
        node->requiresCleanup,
        node->maxPacketSizeEndpoint0,
        node->systemControlQueueState[0]);

    ilog_TOPOLOGY_COMPONENT_2(
        ILOG_USER_LOG, TOPOLOGY_SHOW_TOPOLOGY_6,
        node->msaLA,
        node->configurationValue);

    // log vendor ID & product ID
    ilog_TOPOLOGY_COMPONENT_3(
        ILOG_USER_LOG, TOPOLOGY_SHOW_TOPOLOGY_7,
        node->idVendor,
        node->idProduct,
        node->deviceChanged);

    if(node->pVirtualFunction && !compactOutput)
    {
        _DTT_showVirtualFunction(node->pVirtualFunction);
    }

    if(!compactOutput)
    {
        // Show any stored endpoint data that exists
        _DTT_showEndpointData(LA);
    }

}


/**
* FUNCTION NAME: _DTT_showAllDeviceTopologyByLA()
*
* @brief  - Show the topology table listed by logical address
*
* @return - void
*/
void _DTT_showAllDeviceTopologyByLA(uint8 view)
{
    uint8 LA;
    ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_ICMD_SHOW_TOPOLOGY_BY_LA);

    if (GRG_IsDeviceLex())
    {
        for(LA = 0; LA < MAX_USB_DEVICES; LA++)
        {
            if (_DTT_GetDeviceNode(LA)->inTopology)
            {
                _DTT_showSingleDeviceTopology(LA, view);
                // Pause for the uart buffer to empty
                LEON_UartWaitForTx();
            }
        }
    }
    else
    {
        ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_ICMD_NOT_LEX);
    }
}


/**
* FUNCTION NAME: _DTT_showAllDeviceTopologyByUSB()
*
* @brief  - Show the topology table listed by usb address
*
* @return - void
*/
void _DTT_showAllDeviceTopologyByUSB(uint8 view)
{
    ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_ICMD_SHOW_TOPOLOGY_BY_USB);

    if (GRG_IsDeviceLex())
    {
        uint8 usbAddress;
        for(usbAddress = 0; usbAddress <= MAX_USB_ADDRESS; usbAddress++)
        {
            //check for a device that is in-sys
            XUSB_AddressT address = DTT_GetAddressFromUSB(usbAddress);
            if (XCSR_getXUSBAddrInSys(address))
            {
                _DTT_showSingleDeviceTopology(XCSR_getXUSBAddrLogical(address), view);
                // Pause for the uart buffer to empty
                LEON_UartWaitForTx();
            }
        }
    }
    else
    {
        ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_ICMD_NOT_LEX);
    }
}


/**
* FUNCTION NAME: WriteXSSTCmd()
*
* @brief  - Do a rmw of a word to the XSST
*
* @return - void
*
* @note   -
*
*/
void WriteXSSTCmd
(
    uint8 usbAddress,   // Usb address to write to
    uint8 endptNum,     // Endpoint number to write to
    uint32 xsstValMSW,  // Value to write
    uint32 maskMSW,     // Which bits to modify
    uint32 xsstValLSW,  // Value to write
    uint32 maskLSW      // Which bits to modify
)
{
    ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_ICMD_WRITE_XSST);

    if (GRG_IsDeviceLex())
    {
        ilog_TOPOLOGY_COMPONENT_2(ILOG_USER_LOG, TOPOLOGY_ICMD_WRITING_XSST1, xsstValMSW, maskMSW);
        ilog_TOPOLOGY_COMPONENT_2(ILOG_USER_LOG, TOPOLOGY_ICMD_WRITING_XSST1, xsstValLSW, maskLSW);
        ilog_TOPOLOGY_COMPONENT_2(
            ILOG_USER_LOG, TOPOLOGY_ICMD_WRITING_XSST2, usbAddress, endptNum);
        XCSR_XSSTWriteMask(
            usbAddress, endptNum, MAKE_U64(xsstValMSW, xsstValLSW), MAKE_U64(maskMSW, maskLSW));
    }
    else
    {
        ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_ICMD_NOT_LEX);
    }
}


/**
* FUNCTION NAME: ShowXsst()
*
* @brief  - Show the XSST entry for a specified logical address
*
* @return - void
*/
void ShowXsst
(
    uint8 LA,  // USB address to see the LAT for
    uint8 ep
)
{
    if (GRG_IsDeviceLex())
    {
        struct XCSR_Xsst xsst;
        XUSB_AddressT address;

        ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_ICMD_SHOW_XSST);
        XCSR_initXUSBAddr(&address);
        XCSR_setXUSBAddrLogical(&address, LA);
        address = _DTT_GetAddressFromLogical(LA);

        xsst.sst = XCSR_XSSTRead(XCSR_getXUSBAddrUsb(address), ep);
        ilog_TOPOLOGY_COMPONENT_3(
            ILOG_USER_LOG,
            DISPLAY_XSST,
            ((LA << 16) | ep),
            GET_MSW_FROM_64(xsst.sst),
            GET_LSW_FROM_64(xsst.sst));
    }
    else
    {
        ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_ICMD_NOT_LEX);
    }
}


/**
* FUNCTION NAME: _DTT_showVirtualFunction()
*
* @brief  - Displays the attributes of the supplied virtual function.
*
* @return - void.
*/
static void _DTT_showVirtualFunction(struct DTT_VF_EndPointHandles* vf)
{
    uint8 i;
    ilog_TOPOLOGY_COMPONENT_1(ILOG_USER_LOG, SHOW_VIRT_FN, (uint32)vf);
    for(i = 0; i < vf->numOfEndpoints; i++)
    {
        struct DTT_VF_EndPointHandle* ep = &(vf->endpoint[i]);
        ilog_TOPOLOGY_COMPONENT_2(ILOG_USER_LOG, SHOW_VIRT_FN_1, i, ep->epType);
        ilog_TOPOLOGY_COMPONENT_2(ILOG_USER_LOG, SHOW_VIRT_FN_2, (uint32)ep->in, (uint32)ep->inAck);
        ilog_TOPOLOGY_COMPONENT_2(ILOG_USER_LOG, SHOW_VIRT_FN_3, (uint32)ep->out, (uint32)ep->setup);
    }
}


/**
* FUNCTION NAME: _DTT_showEndpointData
*
* @brief  - Print interface, endpoint, type and alternate setting
*           for a given device node.
*         - Function can be called using ICommands
*
* @return - None.
*/
void _DTT_showEndpointData
(
    uint8 logicalAddress
)
{
    struct DeviceTopology* deviceNode = _DTT_GetDeviceNode(logicalAddress);
    struct EndpointData* endpoint = _DTT_memPoolIndexToEndpoint(deviceNode->endpointDataList);

    ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_SHOW_ENDPOINTS_INFO);

    while(endpoint != NULL)
    {
        ilog_TOPOLOGY_COMPONENT_1(
            ILOG_USER_LOG, CONFIGURATION_VALUE, endpoint->configurationValue);

        ilog_TOPOLOGY_COMPONENT_3(
            ILOG_USER_LOG, ENDPOINT_INFO,
            endpoint->interfaceNumber, endpoint->endpointNumber, endpoint->endpointType);

        ilog_TOPOLOGY_COMPONENT_3(
            ILOG_USER_LOG, ENDPOINT_INFO2,
            endpoint->endpointDirection, endpoint->msaPairEpNumber, endpoint->alternateSetting);

        endpoint = _DTT_memPoolIndexToEndpoint(endpoint->next);
    }
}


// Deprecated icmds.  We call through to the ICMD_deprecatedIcmdFunction to output a message
// indicating that the icmd is deprecated.
void DEPRECATED_ShowTopologyByUsb(void)
{
    ICMD_deprecatedIcmdFunction();
}


/**
* FUNCTION NAME: ShowLat()
*
* @brief  - Show the LAT entry for a specified usb address
*
* @return - void
*
* @todo   - Consider moving this to XCSR since it doesn't depend on anything in topology
*/
void ShowLat
(
    uint8 usbAddress // USB address to see the LAT for
)
{
    ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, TOPOLOGY_ICMD_SHOW_DEVICE);
    XCSR_LATShowEntry(usbAddress);
}

