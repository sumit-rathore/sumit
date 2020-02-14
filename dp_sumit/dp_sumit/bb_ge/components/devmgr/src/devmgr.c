///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2007, 2008
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron  officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron  and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//  @file  - devMgr.c
//
//  @brief - manage the enumeration process
//
//  @note  - describe notes here.
//
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <leon_timers.h>
#include <options.h>
#include <devmgr.h>
#include <topology.h>
#include <ulm.h>
#include <xcsr_xsst.h>

#include "devmgr_log.h"


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static void (*sendDevConStatusToBB)(uint8_t, bool);

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: DEVMGR_Init()
*
* @brief  - Initialization function for this component
*
* @return - nothing
*
* @note   -
*
*/
void DEVMGR_Init(void (*sendDevConStatusToBb)(uint8_t, bool))
{
    sendDevConStatusToBB = sendDevConStatusToBb;
}


/**
* FUNCTION NAME: DEVMGR_ProcessPortResetRequest()
*
* @brief  - Deal with the port reset request from the host
*
* @return - TRUE if the port reset was successful or FALSE if address zero was
*           already in use prior to the reset.
*
* @note   -
*
*/
boolT DEVMGR_ProcessPortResetRequest
(
    XUSB_AddressT parentAddress,
    uint8 port
#ifdef GOLDENEARS
    , uint8 vPort
#endif
)
{
    boolT portResetSuccess = TRUE;
    enum TOPOLOGY_PortResetResult prResult;
    ilog_DEVMGR_COMPONENT_3(
        ILOG_MAJOR_EVENT, PORT_RESET_HUB_DETAILS,
        port, XCSR_getXUSBAddrLogical(parentAddress), XCSR_getXUSBAddrUsb(parentAddress));

    // 1) Cleanup old topology
#ifdef GOLDENEARS
    prResult = DTT_PortReset(parentAddress, port, vPort);
#else
    prResult = DTT_PortReset(parentAddress, port);
#endif
    if (prResult == TOPOLOGY_PR_SUCCESS)
    {
        // 2) configure new address 0

        // 3) enable address 0
        DTT_EnableAddress0();
    }
    else if(prResult == TOPOLOGY_PR_NO_MORE_ADDRESSES)
    {
        ilog_DEVMGR_COMPONENT_3(
            ILOG_MAJOR_ERROR, PORT_RESET_FAILED,
            port, XCSR_getXUSBAddrLogical(parentAddress), XCSR_getXUSBAddrUsb(parentAddress));
    }
    else if(prResult == TOPOLOGY_PR_ADDRESS_ZERO_IN_USE)
    {
        portResetSuccess = FALSE;
    }

    return portResetSuccess;
}


/**
* FUNCTION NAME: DEVMGR_ProcessClearFeatureRequest()
*
* @brief  - Process the port clear feature request
*
* @return - nothing
*
* @note   -
*
*/
void DEVMGR_ProcessClearFeatureRequest
(
    XUSB_AddressT parentAddress,
    uint8 port
)
{
    ilog_DEVMGR_COMPONENT_2(ILOG_DEBUG, CLR_FEATURE_DISABLE_PORT, port, XCSR_getXUSBAddrLogical(parentAddress));
    DTT_PortDisconnect(parentAddress, port);
}


/**
* FUNCTION NAME: DEVMGR_ProcessClearTTBuffer()
*
* @brief  - Process the Hub Clear TT Buffer request
*
* @return - void
*
* @note   -
*
*/
void DEVMGR_ProcessClearTTBuffer
(
    XUSB_AddressT hubAddress,
    XUSB_AddressT devAddress,
    uint8 endpoint,
    uint8 epType
    // NOTE: We don't take the port #, as it is only valid for Multi-TT hubs, and not Single-TT hubs
)
{
    DTT_ResetEndpoint(devAddress, endpoint);
}


/**
* FUNCTION NAME: DEVMGR_HandlePortStatusResponse()
*
* @brief  -  Interpret the the port status response data
*
* @return - nothing
*
* @note   -
*
*/
void DEVMGR_HandlePortStatusResponse
(
    XUSB_AddressT parentAddress,   // Address of the parent device
    uint8 port,  // Port number on the hub
    uint16 portStatus  // Port status
)
{
    enum UsbSpeed speed;
    DEVMGR_COMPONENT_ilogCodeT msg;

    ilog_DEVMGR_COMPONENT_2(ILOG_DEBUG, REX_HUB_PORT_STATUS_HANDLER, XCSR_getXUSBAddrLogical(parentAddress), port);

    if (PORT_CONNECTION_STS(GET_LSB_FROM_16(portStatus)))
    {
        if (PORT_ENABLE_STS(GET_LSB_FROM_16(portStatus)))
        {
            if (PORT_HIGH_SPEED_STS(portStatus) == 1)
            {
                msg = REX_HUB_PORT_STATUS_HANDLER_PORT_EN_HS;
                speed = USB_SPEED_HIGH;
            }
            else if (PORT_LOW_SPEED_STS(portStatus) == 1)
            {
                msg = REX_HUB_PORT_STATUS_HANDLER_PORT_EN_LS;
                speed = USB_SPEED_LOW;
            }
            else
            {
                msg = REX_HUB_PORT_STATUS_HANDLER_PORT_EN_FS;
                speed = USB_SPEED_FULL;
            }

            ilog_DEVMGR_COMPONENT_2(ILOG_DEBUG, msg, port, XCSR_getXUSBAddrLogical(parentAddress));
            // Note that we are actually taking the slower of the two speeds here
            DTT_SetOperatingSpeed(parentAddress, port, MAX(ULM_ReadDetectedSpeed(), speed));
            sendDevConStatusToBB(port, true); // connected
        }

        // now wait for the PortReset
    }// end if CONNECTED
    else
    {
        // Not connected
        DTT_PortDisconnect(parentAddress, port);
        sendDevConStatusToBB(port, false); // disconnected
    }//end if-else connected
}


/**
* FUNCTION NAME: DEVMGR_ProcessSetAddress()
*
* @brief  - Set the address for the device into the XSST
*
* @return - Return TRUE if everything went ok, return FALSE if the topology reported an error
*
* @note   -
*
*/
boolT DEVMGR_ProcessSetAddress
(
    XUSB_AddressT oldAddress,
    XUSB_AddressT * pNewAddress
)
{
    boolT topologySuccess = DTT_SetAddress(oldAddress, pNewAddress);

    ilog_DEVMGR_COMPONENT_2(
        ILOG_MINOR_ERROR, SET_ADDR_ADD_DEV,
        XCSR_getXUSBAddrUsb(*pNewAddress), XCSR_getXUSBAddrLogical(*pNewAddress));

    return topologySuccess;
}


/**
* FUNCTION NAME: DEVMGR_HandleSetAddressResponse()
*
* @brief  - Handle the response to Set Address command
*
* @return - nothing
*
* @note   -
*
*/
void DEVMGR_HandleSetAddressResponse
(
    XUSB_AddressT address
)
{
    ilog_DEVMGR_COMPONENT_0(ILOG_MINOR_EVENT, SET_ADDR_RESPONSE);

    // Ensure the Set Address takes affect
    DTT_FinalizeSetAddress(address);
}


/**
* FUNCTION NAME: DEVMGR_ProcessUpstreamBusReset()
*
* @brief  - Process an upstream bus reset
*
* @return - void
*
* @note   - Step 1 for processing a reset
*
*/
void DEVMGR_ProcessUpstreamBusReset(void)
{
    DTT_HostReset();
}


/**
* FUNCTION NAME: DEVMGR_ProcessUpstreamBusResetNegDone()
*
* @brief  - Process an upstream bus reset speed negiotate event
*
* @return - void
*
* @note   - Step 2 for processing a reset
*
*/
void DEVMGR_ProcessUpstreamBusResetNegDone(enum UsbSpeed speed)
{
}


/**
* FUNCTION NAME: DEVMGR_ProcessUpstreamBusResetDone()
*
* @brief  - Process an upstream bus reset done event
*
* @return - void
*
* @note   - Step 3 for processing a reset
*
*/
void DEVMGR_ProcessUpstreamBusResetDone(void)
{
    // enable control endpoint
    DTT_EnableAddress0();
}


/**
* FUNCTION NAME: DEVMGR_ProcessUpstreamDisconnect()
*
* @brief  - Process an upstream disconnect event
*
* @return - void
*
* @note   -
*
*/
void DEVMGR_ProcessUpstreamDisconnect(void)
{
    // Disconnect devices in the topology
    DTT_HostDisconnect();
}
