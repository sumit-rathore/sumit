///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  linkmgr_utils.c
//
//!   @brief -  generic functions for the link manager
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "linkmgr_loc.h"
#include <storage_Data.h>
#include <ibase_version.h>
#include <linkmgr.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
struct linkMgrState linkState; // declared extern in linkmgr_loc.h


/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

void _LINKMGR_UtilsInit(void)
{
    // Initialize the LED
    GRG_TurnOffLed(LI_LED_SYSTEM_LINK);
}


void LINKMGR_ProcessLinkMsg(
    uint8 vport, XCSR_CtrlLinkMessageT message, uint32 data, uint64 extraData)
{
    if (linkState.phyMgr.phyLinkState != PHY_LINK_UP)
    {
        // We received a message when there is no network connection.  Presumably a stale message
        // that was received right before the network went down.  Ignore this stale message.
    }
    else // Network is good
    {
        iassert_LINKMGR_COMPONENT_0(
            linkState.xusbLinkMgr.msgHandler != NULL, MESSAGE_HANDLER_UNDEFINED);
        (*linkState.xusbLinkMgr.msgHandler)(vport, message, data, extraData);
    }
}


/**
* FUNCTION NAME: _LINKMGR_updateLinkLed()
*
* @brief  - Updates the link LED behaviour by reading various state variables.
*
* @return - void.
*/
void _LINKMGR_updateLinkLed(void)
{
    if(LINKMGR_isPushButtonPairingActive())
    {
        // fast blink
        GRG_ToggleLed(LI_LED_SYSTEM_LINK, LPR_FAST);
    }
    else if (
            linkState.phyMgr.phyLinkState == PHY_LINK_UP
        &&
            (       linkState.linkMode == LINK_MODE_DIRECT
#ifndef GE_CORE
                ||  _LINKMGR_deviceHasPair()
#endif
            )
    )
    {
        boolT anyPairsActive = FALSE;
        if(GRG_IsDeviceLex())
        {
            uint8 i;
            uint8 lastVPort;
            LINKMGR_getVPortBounds(&i, &lastVPort);
            while (i <= lastVPort && !anyPairsActive)
            {
                anyPairsActive = (_LINKMGR_getState(i) == LINK_STATE_ACTIVE);
                i++;
            }
        }
        else
        {
            anyPairsActive = (linkState.xusbLinkMgr.rex.xusbLinkState == LINK_STATE_ACTIVE);
        }

        if(anyPairsActive)
        {
            // solid
            GRG_TurnOnLed(LI_LED_SYSTEM_LINK);
        }
        else
        {
            // slow blink
            GRG_ToggleLed(LI_LED_SYSTEM_LINK, LPR_SLOW);
        }
    }
    else
    {
        // off
        GRG_TurnOffLed(LI_LED_SYSTEM_LINK);
    }
}


/**
* FUNCTION NAME: LINKMGR_isDevicePairedWith()
*
* @brief  - Checks if this device is paired with the device with the given MAC address.
*
* @return - TRUE if the pairing exists.
*/
boolT LINKMGR_isDevicePairedWith(uint64 pairMacAddr)
{
    uint8 i = 0;
    uint8 stopVp = 0;
    LINKMGR_getVPortBounds(&i, &stopVp);
    while(i <= stopVp)
    {
        const enum storage_varName vpVar = TOPLEVEL_lexPairedMacAddrVarForVport(i);
        if(STORAGE_varExists(vpVar))
        {
            if(STORAGE_varGet(vpVar)->doubleWord == (pairMacAddr << 16))
            {
                return TRUE;
            }
        }
        i++;
    }
    return FALSE;
}


/**
* FUNCTION NAME: LINKMGR_hasOpenPairing()
*
* @brief  - Tells whether this device is able to pair with an additional device.
*
* @return - TRUE if this device is able to pair to an additional device.
*/
boolT LINKMGR_hasOpenPairing(void)
{
    uint8 i = 0;
    uint8 stopVp = 0;
    LINKMGR_getVPortBounds(&i, &stopVp);
    while(i <= stopVp)
    {
        const enum storage_varName vpVar = TOPLEVEL_lexPairedMacAddrVarForVport(i);
        if(!STORAGE_varExists(vpVar))
        {
            return TRUE;
        }
        i++;
    }
    return FALSE;
}


#ifndef GE_CORE
/**
* FUNCTION NAME: _LINKMGR_deviceHasPair()
*
* @brief  - Checks if a this device is associated with any other device(s).
*
* @return - TRUE if this device has at least one pair.
*/
boolT _LINKMGR_deviceHasPair(void)
{
    uint8 i = 0;
    uint8 stopVp = 0;
    LINKMGR_getVPortBounds(&i, &stopVp);
    while(i <= stopVp)
    {
        if(STORAGE_varExists(TOPLEVEL_lexPairedMacAddrVarForVport(i)))
        {
            return TRUE;
        }
        i++;
    }

    return FALSE;
}
#endif


// icmd
void showLinkState(void)
{
    ilog_LINKMGR_COMPONENT_3(
        ILOG_USER_LOG,
        LINK_STATE,
        linkState.phyMgr.phyLinkState,
        linkState.phyMgr.curLinkType,
        GRG_IsDeviceLex() ?
            linkState.xusbLinkMgr.lex.xusbLinkState :
            linkState.xusbLinkMgr.rex.xusbLinkState);
}
void LINKMGR_assertHook(void) __attribute__ ((alias("showLinkState")));


/**
* FUNCTION NAME: LINKMGR_viewLinkMode()
*
* @brief  - Returns the link mode from within the link state struct.  This function is required
*           because the linkState variable is not visible outside of the LINKMGR component.
*
* @return - The link mode that the link manager was initialized with.
*/
enum CLM_XUSBLinkMode LINKMGR_viewLinkMode(void)
{
    return linkState.linkMode;
}


/**
* FUNCTION NAME: LINKMGR_addDeviceLink()
*
* @brief  - Calls the registered function for adding a link.
*
* @return - TRUE if the link request was accepted or FALSE otherwise.
*/
boolT LINKMGR_addDeviceLink(uint64 macAddress)
{
    const boolT linkAdded = linkState.xusbLinkMgr.addDeviceLink(macAddress);
    if(linkAdded && linkState.xusbLinkMgr.pairingAddedNotification != NULL)
    {
        linkState.xusbLinkMgr.pairingAddedNotification();
    }
    return linkAdded;
}


/**
* FUNCTION NAME: LINKMGR_removeDeviceLink()
*
* @brief  - Calls the registered function for removing a link.
*
* @return - TRUE if the request was accepted or FALSE otherwise.
*/
boolT LINKMGR_removeDeviceLink(uint64 macAddress)
{
    return linkState.xusbLinkMgr.removeDeviceLink(macAddress);
}


/**
* FUNCTION NAME: LINKMGR_removeAllDeviceLinks()
*
* @brief  - Removes all device links by calling the function pointer in the specific link manager.
*
* @return - void.
*/
void LINKMGR_removeAllDeviceLinks(void)
{
    linkState.xusbLinkMgr.removeAllDeviceLinks();
}


/**
* FUNCTION NAME: LINKMGR_getVPortBounds()
*
* @brief  - Writes the first and last vport number into two integers.  The special vport 0 is
*           always excluded.
*
* @return - void.
*
* @note   - firstVPort and lastVPort are output parameters.
*/
void LINKMGR_getVPortBounds(uint8* firstVPort, uint8* lastVPort)
{
    if(GRG_IsDeviceLex() && linkState.linkMode == LINK_MODE_MULTI_REX)
    {
        // The valid number of vports is 1 to 4 inclusive.  We map 0 to 1 and greater than 4 to 4.
        const uint8 numVports =
            MAX(1, MIN(STORAGE_varGet(VHUB_CONFIGURATION)->bytes[0], NUM_OF_VPORTS - 1));
        *firstVPort = 1;
        *lastVPort = numVports;
    }
    else
    {
        *firstVPort = ONLY_REX_VPORT;
        *lastVPort = ONLY_REX_VPORT;
    }
}


/**
* FUNCTION NAME: LINKMGR_setPairingAddedCallback()
*
* @brief  - Sets the function to be called when a new pair is successfully added.
*
* @return - void.
*/
void LINKMGR_setPairingAddedCallback(void (*pairingAddedCallback)(void))
{
    iassert_LINKMGR_COMPONENT_2(
        linkState.xusbLinkMgr.pairingAddedNotification == NULL,
        LINKMGR_ADD_PAIRING_CALLBACK_EXISTS,
        (uint32)linkState.xusbLinkMgr.pairingAddedNotification,
        (uint32)pairingAddedCallback);
    linkState.xusbLinkMgr.pairingAddedNotification = pairingAddedCallback;
}

// TODO: The firmware checks are not required and should be removed
/**
* FUNCTION NAME: LINKMGR_doesFirmwareSupportUnitBrands()
*
* @brief  - Tells whether the given firmware version supports the concept of unit branding and
*           supports exchanging it during link negotiation.
*
* @return - TRUE if the firmware supports unit brands.
*
* @note   - Version 1.1.1 and later support unit brands.
*/
//boolT LINKMGR_doesFirmwareSupportUnitBrands(uint8 major, uint8 minor, uint8 rev)
//{
//    return (   IBASE_compareFirmwareVersions(major, minor, rev, 1, 1, 1) >= 0);
//}


/**
* FUNCTION NAME: LINKMGR_doesFirmwareSupportEndpointFiltering()
*
* @brief  - Tells whether the given firmware version supports endpoint filtering.
*
* @return - TRUE if the firmware version supports endpoint filtering.
*
* @note   - Version 1.3.1 and later support endpoint filtering.
*/
//boolT LINKMGR_doesFirmwareSupportEndpointFiltering(uint8 major, uint8 minor, uint8 rev)
//{
//    return (   IBASE_compareFirmwareVersions(major, minor, rev, 1, 3, 1) >= 0);
//}


/**
* FUNCTION NAME: LINKMGR_doesFirmwareSupportCapabilityNegotiation()
*
* @brief  - Tells whether the given firmware version supports negotiation of capabilities.
*
* @return - TRUE if the firmware version supports negotiation of capabilities.
*
* @note   - Version 1.3.1 and later support negotiation of capabilities.
*/
//boolT LINKMGR_doesFirmwareSupportCapabilityNegotiation(uint8 major, uint8 minor, uint8 rev)
//{
//    return (IBASE_compareFirmwareVersions(major, minor, rev, 1, 3, 1) >= 0);
//}


/**
* FUNCTION NAME: LINKMGR_getLinkState()
*
* @brief  - Tells whether caller the link state
*
* @return - uint8 representing the link's state.
*
* @note   - .
*/
uint8 LINKMGR_getLinkState(void)
{
    return (GRG_IsDeviceLex() ?
            linkState.xusbLinkMgr.lex.xusbLinkState :
            linkState.xusbLinkMgr.rex.xusbLinkState);
}


/**
* FUNCTION NAME: _LINKMGR_vidIsInvalid()
*
* @brief  - Returns True if the variant ID is invalid
*
* @note   - .
*/
bool _LINKMGR_vidIsInvalid(enum GRG_VariantID vid)
{
    return (vid != GRG_VARIANT_ASIC_ITC2054) && (vid != GRG_VARIANT_ASIC_ITC2051);
}
