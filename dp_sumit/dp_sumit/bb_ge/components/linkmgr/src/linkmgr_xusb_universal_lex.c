///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - linkmgr_xusb_universal_lex.c
//
//!   @brief -
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "linkmgr_loc.h"
#include <xcsr_xicsq.h>
#include <storage_vars.h>
#include <storage_Data.h>
#include <xlr.h>
#ifndef GE_CORE
#include <vhub.h>
#endif

/************************ Defined Constants and Macros ***********************/
#define _LINKMGR_setState(_vport_, _new_state_) _LINKMGR_setState_(_vport_, _new_state_, __LINE__)

/******************************** Data Types *********************************/

// The bit offsets of the rex settings within the xusbLinkMgr.lex.rexLinkSettings and
// xusbLinkMgr.lex.pendingRexSettings entries.
// note that this fits into a byte, so only offset <= 7 allowed!
enum _LINKMGR_RexSettingOffsets
{
    REX_SETTING_SUPPORT_MSA_OFFSET,
    REX_SETTING_ALLOW_ISO_OFFSET,
    REX_SETTING_BLOCK_MASS_STORAGE_OFFSET,
    REX_SETTING_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET,
    REX_SETTING_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET,
    REX_SETTING_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET
};

/***************************** Local Variables *******************************/
static uint8 rexVidStored;

/************************ Local Function Prototypes **************************/
static void _LINKMGR_rexToLexMsgHandler(
    uint8 vport,
    XCSR_CtrlLinkMessageT message,
    uint32 data, uint64 extraData) __attribute__ ((section (".lextext")));
static void _LINKMGR_mlpIrqLex(void) __attribute__ ((section (".lextext")));
static void _LINKMGR_onPhyLinkUpLex(void);
static void _LINKMGR_onPhyLinkDownLex(void);
static boolT _LINKMGR_addDeviceLinkLex(
    uint64 deviceMACAddr) __attribute__ ((section (".lextext")));
static boolT _LINKMGR_removeDeviceLinkLex(
    uint64 deviceMACAddr) __attribute__ ((section (".lextext")));
static void _LINKMGR_takeDownLinkLex(uint8 vport) __attribute__ ((section (".lextext")));
static void _LINKMGR_seekLinkTimeoutHandler(void) __attribute__ ((section (".lextext")));
static void _LINKMGR_lexMlpLinkAcquisitionTimeoutHandler(
    void) __attribute__ ((section (".lextext")));
static void _LINKMGR_lexSendLinkProbe(void) __attribute__ ((section (".lextext")));
static void _LINKMGR_setState_(
    uint8 vport,
    enum _LINKMGR_xusbLinkState state,
    uint16 line) __attribute__ ((section (".lextext")));
static void _LINKMGR_removeAllDeviceLinksLex(void);
static sint8 _LINKMGR_lexFindVportOfPair(
    uint64 pairMacAddr) __attribute__ ((section (".lextext")));
#ifndef GE_CORE
static sint8 _LINKMGR_lexVhubFindFreeVport(void) __attribute__ ((section (".lextext")));
#endif
static boolT _LINKMGR_lexAllPairsActive(void);
static void _LINKMGR_checkStopMlpAcquisitionTimer(void) __attribute__ ((section (".lextext")));
static void _LINKMGR_setLexLinkUp(uint8 vport) __attribute__ ((section (".lextext")));
static void _LINKMGR_setPendingRexLinkOptions(
    uint8 vport,
    uint64 capabilities) __attribute__ ((section (".lextext")));
static void _LINKMGR_activatePendingRexLinkOptions(
    uint8 vport) __attribute__ ((section (".lextext")));



/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: LINKMGR_getRexLinkOptions()
*
* @brief  - Populates output parameters with the link options that were received from the REX for
*           the given vport.
*
* @return - void.
*
* @note   - It only makes sense to call this function on the LEX.
*/
uint64 LINKMGR_getRexLinkOptions(uint8 vport)
{
    const uint8 vportVal = linkState.xusbLinkMgr.lex.rexLinkSettings[vport];
    uint64 rexCapabilities = 0;

    if (vportVal & (1 << REX_SETTING_SUPPORT_MSA_OFFSET) )
    {
        rexCapabilities |= (1 << TOPLEVEL_SUPPORT_MSA_OFFSET);
    }

    if (vportVal & (1 << REX_SETTING_ALLOW_ISO_OFFSET) )
    {
        rexCapabilities |= (1 << TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET);
    }

    if (vportVal & (1 << REX_SETTING_BLOCK_MASS_STORAGE_OFFSET) )
    {
        rexCapabilities |= (1 << TOPLEVEL_BLOCK_MASS_STORAGE_OFFSET);
    }

    if (vportVal & (1 << REX_SETTING_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET) )
    {
        rexCapabilities |= (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET);
    }

    if (vportVal & (1 << REX_SETTING_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET) )
    {
        rexCapabilities |= (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET);
    }

    if (vportVal & (1 << REX_SETTING_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET) )
    {
        rexCapabilities |= (1 << TOPLEVEL_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET);
    }

    return (rexCapabilities);
}


/**
* FUNCTION NAME: _LINKMGR_lexInit()
*
* @brief  - Initialized the LEX link manager.
*/
void _LINKMGR_lexInit(void)
{
    {
        // Broadcast a message whenever we have a vport with a pair and a link
        // that is not established.
        const boolT isPeriodic = TRUE;
        const uint32 timeoutInMs = 1000;
        linkState.xusbLinkMgr.lex.seekLinkTimer = TIMING_TimerRegisterHandler(
            &_LINKMGR_seekLinkTimeoutHandler, isPeriodic, timeoutInMs);
    }
    {
        const boolT isPeriodic = FALSE;
        const uint32 timeoutInMs = 100;
        linkState.xusbLinkMgr.mlpAcquisitionTimer = TIMING_TimerRegisterHandler(
            _LINKMGR_lexMlpLinkAcquisitionTimeoutHandler, isPeriodic, timeoutInMs);
    }
    // Equivalent to setting all vports to LINK_STATE_INCATIVE
    linkState.xusbLinkMgr.lex.xusbLinkState = 0;
    linkState.xusbLinkMgr.lex.probableVetoCount = 0;

    linkState.xusbLinkMgr.phyLinkUp             = &_LINKMGR_onPhyLinkUpLex;
    linkState.xusbLinkMgr.phyLinkDown           = &_LINKMGR_onPhyLinkDownLex;
    linkState.xusbLinkMgr.mlpIrq                = &_LINKMGR_mlpIrqLex;
    linkState.xusbLinkMgr.msgHandler            = &_LINKMGR_rexToLexMsgHandler;
    linkState.xusbLinkMgr.addDeviceLink         = &_LINKMGR_addDeviceLinkLex;
    linkState.xusbLinkMgr.removeDeviceLink      = &_LINKMGR_removeDeviceLinkLex;
    linkState.xusbLinkMgr.removeAllDeviceLinks  = &_LINKMGR_removeAllDeviceLinksLex;

    {
        // See timer description in timeout handler function header
        const boolT isPeriodic = TRUE;
        const uint32 timeoutInMs = 1000;
        linkState.xusbLinkMgr.lex.linkUpProbeTimer = TIMING_TimerRegisterHandler(
            &_LINKMGR_lexSendLinkProbe, isPeriodic, timeoutInMs);
        TIMING_TimerStart(linkState.xusbLinkMgr.lex.linkUpProbeTimer);
    }
}


/**
* FUNCTION NAME: _LINKMGR_rexToLexMsgHandler()
*
* @brief  - Handles messages sent from the REX to the LEX.
*/
static void _LINKMGR_rexToLexMsgHandler(
    uint8 vport, XCSR_CtrlLinkMessageT message, uint32 data, uint64 extraData)
{
    ilog_LINKMGR_COMPONENT_2(ILOG_DEBUG, LINKMGR_RECEIVED_MSG, message, vport);

    switch(message)
    {
        case REX_PAIR_RESPONSE:
        {
            const uint8 replyToVport = 0;
            // Using only the lower 24 bits of LEX's MAC address
            const uint64 lexMac = (extraData >> 32) & 0x00ffffff;
            uint64 rexMac;
            uint32 rexOUI = ((uint64)data & 0x00ffffff);
#ifndef GE_CORE
            // Use 0 as own MAC when we are in direct mode
            // Using only the lower 24 bits of own MAC address
            const uint64 ownMac = linkState.linkMode == LINK_MODE_DIRECT ?
                0 : (STORAGE_varGet(MAC_ADDR)->doubleWord >> 16) & 0x00ffffff;
#else
            const uint64 ownMac = 0;
#endif

            // If the OUI sent by the REX is 0, then we are talking to an older
            // REX which we know must have an Icron OUI.
            if (rexOUI == 0)
            {
                rexOUI = 0x00001B13;  // Set to the Icron OUI
            }
            rexMac = ((((uint64)rexOUI) << 24) | (extraData & 0x00ffffff));

            if(linkState.linkMode == LINK_MODE_DIRECT || lexMac == ownMac)
            {
                const sint8 vportForRex = _LINKMGR_lexFindVportOfPair(rexMac);
                if(vportForRex != -1 && _LINKMGR_getState(vportForRex) == LINK_STATE_INACTIVE)
                {
                    XCSR_XICSSendMessageWithExtraData(
                        LINK_MESSAGE,
                        LEX_VPORT_ASSIGN,
                        replyToVport,
                        vportForRex,
                        extraData); // We just re-use extraData because MACs are unchanged
                    CLM_configureVportMacDst(vportForRex, rexMac);
                    CLM_VPortEnable(vportForRex);
                    _LINKMGR_setState(vportForRex, LINK_STATE_ACQUIRING_MLP_LINK);
                    TIMING_TimerStart(linkState.xusbLinkMgr.mlpAcquisitionTimer);
                }
            }
            break;
        }

        case REX_VETO_CONNECTION:
            if(_LINKMGR_getState(vport) == LINK_STATE_VERIFYING_COMPATIBILITY)
            {
                linkState.xusbLinkMgr.lex.probableVetoCount = 0;
                // We received a veto from the REX, so we might as well take the link down
                // explicitly rather than waiting for a timeout.
                _LINKMGR_takeDownLinkLex(vport);
            }
            break;

        case REX_COMPATIBILITY_RESPONSE:
            if(_LINKMGR_getState(vport) == LINK_STATE_VERIFYING_COMPATIBILITY)
            {
                const uint8 rexFwMajor    = ((data >> 16) & 0xFF);
                const uint8 rexFwMinor    = ((data >>  8) & 0xFF);
                const uint8 rexFwRevision = ((data >>  0) & 0xFF);

                // The 3 high order bits contain the Rex's VID if it's an ASIC with
                // firmware version >= 1.5.9. Clear them because they aren't really "capabiltities".
                const uint8 rexVid = (extraData >> _REX_VID_OFFSET) & _REX_VID_MASK;
                const uint64 rexCapabilities = extraData & ~(_REX_VID_MASK << _REX_VID_OFFSET) ;
                const uint64 lexCapabilities = STORAGE_varGet(CONFIGURATION_BITS)->doubleWord;
                boolT vetoConnection = FALSE;
//                boolT rexSupportsBrands = FALSE;

                rexVidStored = rexVid;
                // Store in memory whether the Rex on this vport supports the
                // SOFTWARE_MESSAGE_LEX2REX CPU message type. This is necessary in order
                // to not cause asserts on old Rex firmware versions that assert upon receiving
                // unknown CPU message types.
                XCSR_XICSSetRexSupportsSwMessages(vport, rexFwMajor, rexFwMinor, rexFwRevision);

                // We have successfully received a compatibility response, so that means that the
                // REX didn't veto the connection.  Reset the probably veto count to 0.
                linkState.xusbLinkMgr.lex.probableVetoCount = 0;

                ilog_LINKMGR_COMPONENT_3(
                    ILOG_MINOR_EVENT,
                    LINKMGR_CHECK_COMPATIBILITY_PAIR,
                    rexFwMajor,
                    rexFwMinor,
                    rexFwRevision);
                ilog_LINKMGR_COMPONENT_3(
                    ILOG_MINOR_EVENT,
                    LINKMGR_CHECK_COMPATIBILITY_LOCAL,
                    SOFTWARE_MAJOR_REVISION,
                    SOFTWARE_MINOR_REVISION,
                    SOFTWARE_DEBUG_REVISION);

                ilog_LINKMGR_COMPONENT_2(
                    ILOG_MINOR_EVENT,
                    LINKMGR_CHECK_CONFIGURATION,
                    lexCapabilities,
                    rexCapabilities);

                // Only allow VID 2051 and 2054 to talk to each other
                // Any other VID should result in a fatal error
                vetoConnection =
                    _LINKMGR_vidIsInvalid(rexVid) || _LINKMGR_vidIsInvalid(GRG_GetVariantID());

                if (vetoConnection)
                {
                    ilog_LINKMGR_COMPONENT_2(
                            ILOG_FATAL_ERROR,
                            LINKMGR_VETOING_INCOMPATIBLE_VIDS,
                            GRG_GetVariantID(),
                            rexVid);
                }

                _LINKMGR_setPendingRexLinkOptions(
                    vport,
                    rexCapabilities);

                if (!vetoConnection)
                {
                    XCSR_XICSSendMessageWithExtraData(
                        LINK_MESSAGE, LEX_LINK_CONFIRMATION, vport, 0, 0);
                    _LINKMGR_setLexLinkUp(vport);
                }
                else
                {
                    ilog_LINKMGR_COMPONENT_1(ILOG_MAJOR_EVENT, LINKMGR_PAIR_INCOMPATIBLE, vport);
                    // We veto the connection, so take the link down.
                    _LINKMGR_takeDownLinkLex(vport);
                }

                _LINKMGR_checkStopMlpAcquisitionTimer();
                _LINKMGR_updateLinkLed();
            }
            break;

        case REX_BRAND_RESPONSE:
            if(_LINKMGR_getState(vport) == LINK_STATE_VERIFYING_BRAND)
            {
                const enum TOPLEVEL_UnitBrand lexBrand = TOPLEVEL_getBrandNumber();
                const enum TOPLEVEL_UnitBrand rexBrand = data & 0xFFFF;
                const boolT vetoConnection = (rexBrand != lexBrand);

                if (!vetoConnection)
                {
                    XCSR_XICSSendMessageWithExtraData(
                        LINK_MESSAGE, LEX_LINK_CONFIRMATION, vport, 0, 0);
                    _LINKMGR_setLexLinkUp(vport);
                }
                else
                {
                    ilog_LINKMGR_COMPONENT_1(ILOG_MAJOR_EVENT, LINKMGR_PAIR_INCOMPATIBLE, vport);
                    // We veto the connection, so take the link down.
                    _LINKMGR_takeDownLinkLex(vport);
                }

                _LINKMGR_checkStopMlpAcquisitionTimer();
                _LINKMGR_updateLinkLed();
            }
            break;

        default:
            ilog_LINKMGR_COMPONENT_2(ILOG_MAJOR_ERROR, INVALID_MESSAGE, message, __LINE__);
            break;
    }
}


/**
* FUNCTION NAME: _LINKMGR_mlpIrqLex()
*
* @brief  - This function is called when a vport link goes up or down.
*/
static void _LINKMGR_mlpIrqLex(void)
{
    // Each bit in vportMask represents link up=1 or link down=0
    const uint8 vportMask = CLM_GetVportStatusMask();
    uint8 vport;
    uint8 aLinkHasGoneUp = FALSE;
    uint8 aLinkHasGoneDown = FALSE;
    uint8 firstVPort;
    uint8 lastVPort;
    LINKMGR_getVPortBounds(&firstVPort, &lastVPort);

    for(vport = firstVPort; vport <= lastVPort; vport++)
    {
        const uint8 vpBit = (1 << vport);
        const boolT vpLinkUp = (vportMask & vpBit) != 0;
        const enum _LINKMGR_xusbLinkState state = _LINKMGR_getState(vport);
        const boolT vpLinkWasUp =
            (      (state == LINK_STATE_ACTIVE)
                || (state == LINK_STATE_VERIFYING_COMPATIBILITY)
                || (state == LINK_STATE_VERIFYING_BRAND));
        boolT overrideConfigBits = FALSE;

        // We only care if this vport has transitioned link state
        if(vpLinkUp && !vpLinkWasUp)
        { // Link just came up
            uint64 configBitsToSend = STORAGE_varGet(CONFIGURATION_BITS)->doubleWord;
            // If the probableVetoCount is 2 or more then we assume that the other side is vetoing
            // the connection and we lie about our configuration when sending the compatibility
            // query to the REX.  The reason for this is that in older firmware we required an
            // exact match in CONFIGURATION_BITS during link up.  Since the LEX deals with the MSA
            // and ISO blocking now, we just need to convince the REX to link to us and then we can
            // enforce the minimum feature set of the union of the LEX and REX configurations.

            const boolT ethernetFramingEnabled =
                    (   STORAGE_varGet(CONFIGURATION_BITS)->doubleWord
                     >> TOPLEVEL_USE_ETHERNET_FRAMING_OFFSET)
                & 0x1;
            if (ethernetFramingEnabled)
            {
                if (linkState.xusbLinkMgr.lex.probableVetoCount == 2)
                {
                    // This was the 2304GE-LAN configuration that shipped in N05, N04
                    configBitsToSend = 0x3F;
                    overrideConfigBits = TRUE;
                }
                else if (linkState.xusbLinkMgr.lex.probableVetoCount >= 3)
                {
                    // This was the 2304-LAN configuration that shipped in N03
                    configBitsToSend = 0x39;
                    linkState.xusbLinkMgr.lex.probableVetoCount = 0;
                    overrideConfigBits = TRUE;
                }
            }
            else
            {
                if (linkState.xusbLinkMgr.lex.probableVetoCount == 2)
                {
                    // This was the 2304 configuration that shipped in N05, N04
                    configBitsToSend = 0x37;
                    linkState.xusbLinkMgr.lex.probableVetoCount = 0;
                    overrideConfigBits = TRUE;
                }
            }
            aLinkHasGoneUp = TRUE;

            // If we believe we are communicating with a modern unit, then place the variant ID in
            // the high order bits of the capabilities.  Older units will veto the connection if
            // the variant ID is included in the data.
            if (!overrideConfigBits)
            {

                const uint64 lexVariantId = GRG_GetVariantID();
                configBitsToSend |= lexVariantId << _LEX_VID_OFFSET;
            }

            _LINKMGR_setState(vport, LINK_STATE_VERIFYING_COMPATIBILITY);
            XCSR_XICSSendMessageWithExtraData(
                LINK_MESSAGE,
                LEX_COMPATIBILITY_QUERY,
                vport,
                ((SOFTWARE_MAJOR_REVISION << 16) |
                 (SOFTWARE_MINOR_REVISION <<  8) |
                 (SOFTWARE_DEBUG_REVISION <<  0)),
                configBitsToSend);
        }
        else if(!vpLinkUp && vpLinkWasUp)
        { // Link just went down
            aLinkHasGoneDown = TRUE;
            if (state == LINK_STATE_VERIFYING_COMPATIBILITY)
            {
                linkState.xusbLinkMgr.lex.probableVetoCount++;
            }

            _LINKMGR_takeDownLinkLex(vport);
        }
    }

    if(aLinkHasGoneUp && !aLinkHasGoneDown && _LINKMGR_lexAllPairsActive())
    {
        // If a link went up and no link went down, then we can stop broadcasting providing
        // that all pairs are in the active state.
        TIMING_TimerStop(linkState.xusbLinkMgr.lex.seekLinkTimer);
    }

    if(aLinkHasGoneDown)
    {
        TIMING_TimerStart(linkState.xusbLinkMgr.lex.seekLinkTimer);
    }

    // Enable SOFs when any Vport link is up
    XLR_controlSofForwarding(vportMask != 0);

    _LINKMGR_updateLinkLed();
}


/**
* FUNCTION NAME: _LINKMGR_onPhyLinkUpLex()
*
* @brief  - Called when a PHY link is established on the LEX.
*/
static void _LINKMGR_onPhyLinkUpLex(void)
{
    if(linkState.linkMode == LINK_MODE_DIRECT
#ifndef GE_CORE
       || _LINKMGR_deviceHasPair()
#endif
    )
    {
        TIMING_TimerStart(linkState.xusbLinkMgr.lex.seekLinkTimer);
    }
    CLM_onPhyLinkUp(linkState.linkMode);
    _LINKMGR_updateLinkLed();
}


/**
* FUNCTION NAME: _LINKMGR_onPhyLinkDownLex()
*
* @brief  - Called when a the PHY link is lost.
*/
static void _LINKMGR_onPhyLinkDownLex(void)
{
#ifndef GE_CORE
    uint8 vport;
#endif
    TIMING_TimerStop(linkState.xusbLinkMgr.mlpAcquisitionTimer);

    // We don't need to (and are unable to) disable the vports because the CLM
    // will be in reset.

#ifndef GE_CORE
    if(linkState.linkMode == LINK_MODE_MULTI_REX)
    {
        uint8 firstVPort;
        uint8 lastVPort;
        LINKMGR_getVPortBounds(&firstVPort, &lastVPort);
        for(vport = firstVPort; vport <= lastVPort; vport++)
        {
            if (_LINKMGR_getState(vport) == LINK_STATE_ACTIVE)
            {
                VHUB_DevicePortMessage(DOWNSTREAM_DISCONNECT, vport);
                XCSR_vportLinkDown(vport);
            }
        }
    }
    else
#endif
    {
        LEX_LinkDownNotification();
        XCSR_vportLinkDown(ONLY_REX_VPORT);
    }

    TIMING_TimerStop(linkState.xusbLinkMgr.lex.seekLinkTimer);
    // Equivalent to setting all vports to LINK_STATE_INCATIVE
    linkState.xusbLinkMgr.lex.xusbLinkState = 0;

    // Disable SOFs because we may not have gotten the MLP link down IRQ
    XLR_controlSofForwarding(FALSE);

    _LINKMGR_updateLinkLed();
}


/**
* FUNCTION NAME: _LINKMGR_addDeviceLinkLex()
*
* @brief  - Try to pair this LEX to the REX with the given MAC address.
*
* @return - TRUE if the pairing request was accepted or FALSE if the LEX is already paired
*           (linkMode == point to point) or if there are no more free vports or the LEX was already
*           paired with the MAC address supplied (linkMode == multi-rex aka vhub).
*/
static boolT _LINKMGR_addDeviceLinkLex(uint64 deviceMACAddr)
{
    boolT pairingAdded = FALSE;
#ifndef GE_CORE
    if(linkState.linkMode == LINK_MODE_POINT_TO_POINT)
    {
        const enum storage_varName vpVar = TOPLEVEL_lexPairedMacAddrVarForVport(ONLY_REX_VPORT);
        if(!STORAGE_varExists(vpVar))
        {
            union STORAGE_VariableData* var = STORAGE_varCreate(vpVar);
            var->doubleWord = (deviceMACAddr << 16);
            STORAGE_varSave(vpVar);
            // In point to point mode, vport0 needs to be adjusted to be directed at the correct
            // MAC address
            CLM_configureVportMacDst(0, deviceMACAddr);
            pairingAdded = TRUE;
        }
    }
    else if(linkState.linkMode == LINK_MODE_MULTI_REX)
    {
        if(!LINKMGR_isDevicePairedWith(deviceMACAddr))
        {
            const sint8 freeVp = _LINKMGR_lexVhubFindFreeVport();
            if(freeVp != -1)
            {
                const enum storage_varName vpVar = TOPLEVEL_lexPairedMacAddrVarForVport(freeVp);
                STORAGE_varCreate(vpVar)->doubleWord = (deviceMACAddr << 16);
                STORAGE_varSave(vpVar);
                pairingAdded = TRUE;
            }
            else
            {
                // There are no more free vports
            }
        }
        else
        {
            // The pairing already exists
        }
    }

    if(pairingAdded)
    {
        TIMING_TimerStart(linkState.xusbLinkMgr.lex.seekLinkTimer);
        _LINKMGR_updateLinkLed();
    }
#endif

    return pairingAdded;
}


/**
* FUNCTION NAME: _LINKMGR_removeDeviceLinkLex()
*
* @brief  - Removes the link to the REX with the given MAC address.
*
* @return - TRUE if the link existed and was removed or FALSE if there was no link to begin with or
*           the LEX was paired with a different REX than the one passed as a parameter.
*/
static boolT _LINKMGR_removeDeviceLinkLex(uint64 deviceMACAddr)
{
    boolT pairingRemoved = FALSE;
#ifndef GE_CORE
    uint8 i;
    uint8 startVport;
    uint8 endVport;

    iassert_LINKMGR_COMPONENT_2(
        linkState.linkMode == LINK_MODE_POINT_TO_POINT || linkState.linkMode == LINK_MODE_MULTI_REX,
        LINKMGR_UNEXPECTED_LINK_MODE,
        linkState.linkMode,
        __LINE__);

    if(linkState.linkMode == LINK_MODE_MULTI_REX)
    {
        startVport = 1;
        endVport = NUM_OF_VPORTS - 1;
    }
    else // We can assume LINK_MODE_POINT_TO_POINT
    {
        startVport = ONLY_REX_VPORT;
        endVport = ONLY_REX_VPORT;
    }
    for(i = startVport; i <= endVport && !pairingRemoved; i++)
    {
        const enum storage_varName vpToRemove = TOPLEVEL_lexPairedMacAddrVarForVport(i);
        if(STORAGE_varExists(vpToRemove))
        {
            if((STORAGE_varGet(vpToRemove)->doubleWord >> 16) == deviceMACAddr)
            {
                STORAGE_varRemove(vpToRemove);
                STORAGE_varSave(vpToRemove);
                pairingRemoved = TRUE;
                _LINKMGR_takeDownLinkLex(i);
                if(_LINKMGR_lexAllPairsActive())
                {
                    TIMING_TimerStop(linkState.xusbLinkMgr.lex.seekLinkTimer);
                }
                _LINKMGR_updateLinkLed();
            }
        }
    }
#endif

    return pairingRemoved;
}


/**
* FUNCTION NAME: _LINKMGR_removeAllDeviceLinksLex()
*
* @brief  - Removes all device pairings.
*
* @return - void.
*/
static void _LINKMGR_removeAllDeviceLinksLex(void)
{
#ifndef GE_CORE
    uint8 i;
    ilog_LINKMGR_COMPONENT_0(ILOG_MAJOR_EVENT, LINKMGR_REMOVE_ALL_LINKS);
    // Although not all variables are used in point-to-point mode, there is no harm in checking the
    // unused variables and removing them if they do exist.
    for(i = 1; i < NUM_OF_VPORTS; i++)
    {
        const enum storage_varName vpToRemove = TOPLEVEL_lexPairedMacAddrVarForVport(i);
        if(STORAGE_varExists(vpToRemove))
        {
            STORAGE_varRemove(vpToRemove);
            STORAGE_varSave(vpToRemove);
            _LINKMGR_takeDownLinkLex(i);
        }
    }
    TIMING_TimerStop(linkState.xusbLinkMgr.lex.seekLinkTimer);
#endif
}


/**
* FUNCTION NAME: _LINKMGR_takeDownLinkLex()
*
* @brief  - Takes the link down and returns the link manager to the correct state.
*/
static void _LINKMGR_takeDownLinkLex(uint8 vport)
{
    if (_LINKMGR_getState(vport) == LINK_STATE_ACTIVE)
    {
#ifndef GE_CORE
        if(linkState.linkMode == LINK_MODE_MULTI_REX)
        {
            VHUB_DevicePortMessage(DOWNSTREAM_DISCONNECT, vport);
        }
        else
#endif
        {
            LEX_LinkDownNotification();
        }
    }

    CLM_VPortDisable(vport);
    XCSR_vportLinkDown(vport);

    _LINKMGR_setState(vport, LINK_STATE_INACTIVE);
    ilog_LINKMGR_COMPONENT_1(ILOG_MAJOR_EVENT, XUSB_VPORT_LINK_DOWN, vport);
}

/**
* FUNCTION NAME: _LINKMGR_seekLinkTimeoutHandler()
*
* @brief  - This function is called periodically on a timer to allow the LEX to broadcast that it
*           is looking for a pair.  This allows any LEX on the network to discover when a paired
*           REX is available.
*/
static void _LINKMGR_seekLinkTimeoutHandler(void)
{
    const uint8 sendOnVport = 0;
#ifndef GE_CORE
    // Use 0 as own MAC when we are in direct mode
    const uint64 ownMac = linkState.linkMode == LINK_MODE_DIRECT ?
        0 : (STORAGE_varGet(MAC_ADDR)->doubleWord >> 16);
#else
    const uint64 ownMac = 0;
#endif
    const uint64 lower24bitsMacAddrs = (ownMac & 0x00ffffff) << 32;
    const uint32 upper24bitsMacAddrs = ((ownMac >> 24) & 0x00ffffff);

    XCSR_XICSSendMessageWithExtraData(
        LINK_MESSAGE,
        LEX_SEEKS_PAIR,
        sendOnVport,
        upper24bitsMacAddrs,
        lower24bitsMacAddrs);
    ilog_LINKMGR_COMPONENT_0(ILOG_DEBUG, LINKMGR_LEX_BROADCAST);
}


/**
* FUNCTION NAME: _LINKMGR_lexMlpLinkAcquisitionTimeoutHandler()
*
* @brief  - Called when an MLP link is not established before the timeout occurs.  This resets the
*           link so that the devices will be in a position to attempt to establish a link again.
*/
static void _LINKMGR_lexMlpLinkAcquisitionTimeoutHandler(void)
{
    // Take down any links that are in the acquiring MLP state
    uint8 i;
    uint8 endVport;

    ilog_LINKMGR_COMPONENT_0(ILOG_DEBUG, LINKMGR_MLP_ACQUISITION_TIMEOUT);

    LINKMGR_getVPortBounds(&i, &endVport);
    while(i <= endVport)
    {
        const enum _LINKMGR_xusbLinkState state =_LINKMGR_getState(i);
        if(state == LINK_STATE_ACQUIRING_MLP_LINK ||
           state == LINK_STATE_VERIFYING_COMPATIBILITY ||
           state == LINK_STATE_VERIFYING_BRAND)
        {
            _LINKMGR_takeDownLinkLex(i);
        }
        i++;
    }
    TIMING_TimerStart(linkState.xusbLinkMgr.lex.seekLinkTimer);
}


/**
* FUNCTION NAME: _LINKMGR_lexSendLinkProbe()
*
* @brief  - There is a condition described by Bug4232 where if the link is disconnected in a way
*           such that the LEX does not get a PLL loss of lock interrupt, then the RTL will not
*           notify software that the link is down until a retryable MLP packet cannot be delivered.
*           In the case of ISO out devices (such as a webcam), there may never be a retryable
*           packet unless we artificially introduce one.  This timer handler sends a retryable
*           packet to enable software to reliably be informed of link down scenarios.
*
* @return - void.
*
* @note   - Only send the message to rex devices which will not assert when they receive an
*           unexpected message.
*/
static void _LINKMGR_lexSendLinkProbe(void)
{
    uint8 i;
    uint8 endVport;
    LINKMGR_getVPortBounds(&i, &endVport);
    while (i <= endVport)
    {
        if (_LINKMGR_getState(i) == LINK_STATE_ACTIVE && XCSR_XICSGetRexSupportsSwMessages(i))
        {
            XCSR_XICSSendMessage(SOFTWARE_MESSAGE_LEX2REX, LEX_LINK_UP_PROBE, i);
        }
        i++;
    }
}


/**
* FUNCTION NAME: _LINKMGR_setState_()
*
* @brief  - Set the state of a given vport.
*/
static void _LINKMGR_setState_(uint8 vport, enum _LINKMGR_xusbLinkState state, uint16 line)
{
    const uint8 bitPos = vport * 4;
    uint32 clearMask = ~(0xF << bitPos);
    linkState.xusbLinkMgr.lex.xusbLinkState =
        (linkState.xusbLinkMgr.lex.xusbLinkState & clearMask) | (state << bitPos);
    ilog_LINKMGR_COMPONENT_3(ILOG_DEBUG, XUSB_SET_STATE, vport, state, line);
}


/**
* FUNCTION NAME: _LINKMGR_getState()
*
* @brief  - Gets the state of the given vport.
*
* @return - The state for the given vport.
*/
enum _LINKMGR_xusbLinkState _LINKMGR_getState(uint8 vport)
{
    const uint8 bitPos = vport * 4;
    return (linkState.xusbLinkMgr.lex.xusbLinkState >> bitPos) & 0xF;
}


/**
* FUNCTION NAME: _LINKMGR_lexFindVportOfPair()
*
* @brief  - Finds the vport that is associated with the given MAC address.
*
* @return - The found vport or -1 if not found.
*/
static sint8 _LINKMGR_lexFindVportOfPair(uint64 pairMacAddr)
{
#ifndef GE_CORE
    uint8 pairVport = -1;
    if(linkState.linkMode == LINK_MODE_DIRECT)
    {
        pairVport = ONLY_REX_VPORT;
    }
    else
    {
        uint8 stopVp = 0;
        uint8 i = 0;

        LINKMGR_getVPortBounds(&i, &stopVp);
        while(i <= stopVp)
        {
            const enum storage_varName vpVar = TOPLEVEL_lexPairedMacAddrVarForVport(i);
            if(STORAGE_varExists(vpVar))
            {
                if(STORAGE_varGet(vpVar)->doubleWord == (pairMacAddr << 16))
                {
                    pairVport = i;
                    break;
                }
            }
            i++;
        }
    }

    return pairVport;
#else
    return ONLY_REX_VPORT;
#endif
}


#ifndef GE_CORE
/**
* FUNCTION NAME: _LINKMGR_lexVhubFindFreeVport()
*
* @brief  - Finds a free vport number that is not yet allocated to any MAC address pairing.
*
* @return - The free vport number or -1 if there are no more vports available or this device is
*           already paired with the MAC address specified.
*
* @note   - This function is valid only under vhub operation.
*/
static sint8 _LINKMGR_lexVhubFindFreeVport(void)
{
    uint8 i;
    uint8 lastVPort;
    LINKMGR_getVPortBounds(&i, &lastVPort);
    while (i <= lastVPort)
    {
        const enum storage_varName vpVar = TOPLEVEL_lexPairedMacAddrVarForVport(i);
        if(!STORAGE_varExists(vpVar))
        {
            return i;
        }
        i++;
    }

    return -1;
}
#endif


/**
* FUNCTION NAME: _LINKMGR_lexAllPairsActive()
*
* @brief  - Tells whether all of this device's link pairs are active.
*
* @return - TRUE if all pairs are active.
*/
static boolT _LINKMGR_lexAllPairsActive(void)
{
#ifndef GE_CORE
    uint8 i = 0;
    uint8 stopVp = 0;
    boolT areAllPairsActive = TRUE;

    LINKMGR_getVPortBounds(&i, &stopVp);
    while((i <= stopVp) && areAllPairsActive)
    {
        const enum storage_varName vpVar = TOPLEVEL_lexPairedMacAddrVarForVport(i);
        if(STORAGE_varExists(vpVar))
        {
            switch(_LINKMGR_getState(i))
            {
                case LINK_STATE_INACTIVE:
                case LINK_STATE_ACQUIRING_MLP_LINK:
                    areAllPairsActive = FALSE;
                    break;

                case LINK_STATE_VERIFYING_COMPATIBILITY:
                case LINK_STATE_VERIFYING_BRAND:
                case LINK_STATE_ACTIVE:
                    // MLP link is up, this link is considered active
                    break;

                default:
                    iassert_LINKMGR_COMPONENT_1(FALSE, LEX_INVALID_STATE, _LINKMGR_getState(i));
            }
        }
        i++;
    }

    return areAllPairsActive;
#else
    return _LINKMGR_getState(ONLY_REX_VPORT) == LINK_STATE_ACTIVE;
#endif
}


static void _LINKMGR_checkStopMlpAcquisitionTimer(void)
{
    // Check if we should stop the mlpAcquisitionTimer
    boolT stopMlpAcquisitionTimer = TRUE;
    uint8 i;
    for(i = 1; i < NUM_OF_VPORTS; i++)
    {
        const enum _LINKMGR_xusbLinkState state = _LINKMGR_getState(i);
        if(state == LINK_STATE_ACQUIRING_MLP_LINK ||
           state == LINK_STATE_VERIFYING_COMPATIBILITY ||
           state == LINK_STATE_VERIFYING_BRAND)
        {
            stopMlpAcquisitionTimer = FALSE;
            break;
        }
    }
    if (stopMlpAcquisitionTimer)
    {
        TIMING_TimerStop(linkState.xusbLinkMgr.mlpAcquisitionTimer);
    }
}

static void _LINKMGR_setLexLinkUp(uint8 vport)
{
    _LINKMGR_activatePendingRexLinkOptions(vport);
    if(linkState.linkMode == LINK_MODE_MULTI_REX)
    {
        // TODO
    }
    else
    {
        // TODO: it seems odd that this doesn't take a vport arg
        LEX_HandleCLMLinkUp();
    }
    ilog_LINKMGR_COMPONENT_1(ILOG_MAJOR_EVENT, XUSB_VPORT_LINK_UP, vport);
    _LINKMGR_setState(vport, LINK_STATE_ACTIVE);
}

/**
* FUNCTION NAME: _LINKMGR_setPendingRexLinkOptions()
*
* @brief  - Stores the pending REX options.  The options given won't be put into effect until
*           _LINKMGR_activatePendingRexLinkOptions is called.
*
* @return - void.
*/
static void _LINKMGR_setPendingRexLinkOptions(
    uint8 vport,
    uint64 rexCapabilities)
{
    uint8 rexSettings = 0;

    if (rexCapabilities & (1 << TOPLEVEL_SUPPORT_MSA_OFFSET) )
    {
        rexSettings |= 1 << REX_SETTING_SUPPORT_MSA_OFFSET;
    }

    if (rexCapabilities & (1 << TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET) )
    {
        rexSettings |= 1 << REX_SETTING_ALLOW_ISO_OFFSET;
    }

    if (rexCapabilities & (1 << TOPLEVEL_BLOCK_MASS_STORAGE_OFFSET) )
    {
        rexSettings |= 1 << REX_SETTING_BLOCK_MASS_STORAGE_OFFSET;
    }

    if (rexCapabilities & (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET) )
    {
        rexSettings |= 1 << REX_SETTING_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET;
    }

    if (rexCapabilities & (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET) )
    {
        rexSettings |= 1 << REX_SETTING_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET;
    }

    if (rexCapabilities & (1 << TOPLEVEL_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET) )
    {
        rexSettings |= 1 << REX_SETTING_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET;
    }

    linkState.xusbLinkMgr.lex.pendingRexLinkSettings[vport] = rexSettings;
}

/**
* FUNCTION NAME: _LINKMGR_activatePendingRexLinkOptions()
*
* @brief  - Activates the pending options set by a previous call to
*            _LINKMGR_setPendingRexLinkOptions.
*
* @return - void.
*/
static void _LINKMGR_activatePendingRexLinkOptions(uint8 vport)
{
    linkState.xusbLinkMgr.lex.rexLinkSettings[vport] =
        linkState.xusbLinkMgr.lex.pendingRexLinkSettings[vport];
}

