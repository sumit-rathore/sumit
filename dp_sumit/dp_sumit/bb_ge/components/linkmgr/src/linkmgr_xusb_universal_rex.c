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
#include <storage_vars.h>
#include <storage_Data.h>
#include <xcsr_xicsq.h>
#include <timing_timers.h>
#include <ibase_version.h>

/************************ Defined Constants and Macros ***********************/
#define TX_PROBE_INITIAL_RETRY_DELAY 1000
#define TX_PROBE_MAXIMUM_RETRY_DELAY 32000

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static TIMING_TimerHandlerT txProbeTimer; // for waiting for replies
uint8 lexVidStored;

/************************ Local Function Prototypes **************************/
static void _LINKMGR_lexToRexMsgHandler(
    uint8 vport,
    XCSR_CtrlLinkMessageT message,
    uint32 data,
    uint64 extraData) __attribute__ ((section (".rextext")));
static void _LINKMGR_sendRexPairResponse(
    uint64 ownMacAddr, uint64 destLexMacAddr) __attribute__ ((section (".rextext")));
static void _LINKMGR_mlpIrqRex(void);
static void _LINKMGR_onPhyLinkUpRex(void);
static void _LINKMGR_onPhyLinkDownRex(void);
static void _LINKMGR_vportNegotiationTimeoutHandler(void);
static void _LINKMGR_rexMlpLinkAcquisitionTimeoutHandler(void);
static void _LINKMGR_takeDownLinkRex(boolT clmInReset);
static boolT _LINKMGR_addDeviceLinkRex(
    uint64 deviceMACAddr) __attribute__ ((section (".rextext")));
static boolT _LINKMGR_removeDeviceLinkRex(
    uint64 deviceMACAddr) __attribute__ ((section (".rextext")));
static void _LINKMGR_removeAllDeviceLinksRex(void);
static void _LINKMGR_setRexLinkUp(void);

static void _LINKMGR_rexEnableTxProbe(void);
static void _LINKMGR_rexDisableTxProbe(void);
static void _LINKMGR_rexDelayTxProbe(void);
static void _LINKMGR_rexSendTxProbe(void);


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _LINKMGR_rexInit()
*
* @brief  - Initializes the REX link manager.
*/
void _LINKMGR_rexInit(void)
{
    {
        const boolT isPeriodic = FALSE;
        const uint32 timeoutInMs = 20;
        linkState.xusbLinkMgr.rex.vportNegotiationTimer = TIMING_TimerRegisterHandler(
            &_LINKMGR_vportNegotiationTimeoutHandler, isPeriodic, timeoutInMs);
    }
    {
        const boolT isPeriodic = FALSE;
        const uint32 timeoutInMs = 100;
        linkState.xusbLinkMgr.mlpAcquisitionTimer = TIMING_TimerRegisterHandler(
            &_LINKMGR_rexMlpLinkAcquisitionTimeoutHandler, isPeriodic, timeoutInMs);
    }
    {
        const boolT isPeriodic = FALSE;
        txProbeTimer = TIMING_TimerRegisterHandler(
        &_LINKMGR_rexSendTxProbe, isPeriodic, TX_PROBE_INITIAL_RETRY_DELAY);
    }

    linkState.xusbLinkMgr.phyLinkUp             = &_LINKMGR_onPhyLinkUpRex;
    linkState.xusbLinkMgr.phyLinkDown           = &_LINKMGR_onPhyLinkDownRex;
    linkState.xusbLinkMgr.mlpIrq                = &_LINKMGR_mlpIrqRex;
    linkState.xusbLinkMgr.msgHandler            = &_LINKMGR_lexToRexMsgHandler;
    linkState.xusbLinkMgr.addDeviceLink         = &_LINKMGR_addDeviceLinkRex;
    linkState.xusbLinkMgr.removeDeviceLink      = &_LINKMGR_removeDeviceLinkRex;
    linkState.xusbLinkMgr.removeAllDeviceLinks  = &_LINKMGR_removeAllDeviceLinksRex;
}


/**
* FUNCTION NAME: _LINKMGR_lexToRexMsgHandler()
*
* @brief  - Handles messages sent from the LEX to the REX.
*/
static void _LINKMGR_lexToRexMsgHandler(
    uint8 vport, XCSR_CtrlLinkMessageT message, uint32 data, uint64 extraData)
{
#ifndef GE_CORE
    // Use 0 as own MAC when we are in direct mode
    const uint64 ownMac = linkState.linkMode == LINK_MODE_DIRECT ?
        0 : (STORAGE_varGet(MAC_ADDR)->doubleWord >> 16);
#else
    const uint64 ownMac = 0;
#endif

    ilog_LINKMGR_COMPONENT_2(ILOG_DEBUG, LINKMGR_RECEIVED_MSG, message, vport);

    switch(message)
    {
        case LEX_SEEKS_PAIR:
            if(linkState.xusbLinkMgr.rex.xusbLinkState == LINK_STATE_INACTIVE)
            {
                uint64 lexMac;
                uint32 lexOUI = ((uint64)data & 0x00ffffff);
                // If the OUI sent by the LEX is 0, then we are talking to an older LEX which we
                // know must have an Icron OUI.
                if (lexOUI == 0)
                {
                    lexOUI = 0x00001B13;  // Set to the Icron OUI
                }
                lexMac = ((((uint64)lexOUI) << 24) | ((extraData >> 32) & 0x00ffffff));

                if(   linkState.linkMode == LINK_MODE_DIRECT
#ifndef GE_CORE
                   || (STORAGE_varExists(REX_PAIRED_MAC_ADDR) &&
                       STORAGE_varGet(REX_PAIRED_MAC_ADDR)->doubleWord >> 16 == lexMac)
#endif
                )
                {
                    // This REX has a stored pairing with the LEX that sent the LEX_SEEKS_PAIR
                    // message.
                    _LINKMGR_sendRexPairResponse(ownMac, lexMac);

                    // Delay probe packet transmission since we just sent a packet anyway.
                    _LINKMGR_rexDelayTxProbe();
                }
            }
            break;

        case LEX_VPORT_ASSIGN:
            {
                const uint64 rexMac = extraData & 0x00ffffff;
                const uint64 lexMac = (extraData >> 32) & 0x00ffffff;
                const uint8 assignedVport = data;

#ifndef GE_CORE
                if (    linkState.linkMode == LINK_MODE_DIRECT
                    ||  STORAGE_varExists(REX_PAIRED_MAC_ADDR))
#endif
                {
                    uint64 currentLexMac = 0;
#ifndef GE_CORE
                    if (linkState.linkMode != LINK_MODE_DIRECT)
                    {
                        currentLexMac = STORAGE_varGet(REX_PAIRED_MAC_ADDR)->doubleWord >> 16;
                    }
#endif
                    // Compare only the lower 24 bits of the MAC addresses
                    if(    linkState.linkMode == LINK_MODE_DIRECT
                        || (   rexMac == (ownMac & 0x00ffffff)
                            && lexMac == (currentLexMac & 0x00ffffff)))
                    {
                        // Now we know that this message was destined for us
                        if(linkState.xusbLinkMgr.rex.xusbLinkState == LINK_STATE_NEGOTIATING_VPORT)
                        {
                            CLM_configureVportMacDst(assignedVport, currentLexMac);

                            GRG_RexSetVportID(assignedVport);
                            CLM_VPortEnable(assignedVport);

                            TIMING_TimerStop(linkState.xusbLinkMgr.rex.vportNegotiationTimer);
                            linkState.xusbLinkMgr.rex.xusbLinkState =
                                LINK_STATE_ACQUIRING_MLP_LINK;
                            TIMING_TimerStart(linkState.xusbLinkMgr.mlpAcquisitionTimer);
                            _LINKMGR_updateLinkLed();
                        }
                    }
                }
            }
            break;

        case LEX_COMPATIBILITY_QUERY:
            if(linkState.xusbLinkMgr.rex.xusbLinkState == LINK_STATE_VERIFYING_COMPATIBILITY)
            {
                const uint8 lexFwMajor    = ((data >> 16) & 0xFF);
                const uint8 lexFwMinor    = ((data >>  8) & 0xFF);
                const uint8 lexFwRevision = ((data >>  0) & 0xFF);
                // The 3 high order bits contain the Rex's VID if it's an ASIC with
                // firmware version >= 1.5.9. Clear them because they aren't really "capabiltities".
                const uint8 lexVid = (extraData >> _LEX_VID_OFFSET) & _LEX_VID_MASK;
                const uint64 lexCapabilities = extraData & ~(_LEX_VID_MASK << _LEX_VID_OFFSET);
                uint64 rexCapabilities = STORAGE_varGet(CONFIGURATION_BITS)->doubleWord;
//                const boolT lexSupportsBrands =
//                    LINKMGR_doesFirmwareSupportUnitBrands(lexFwMajor, lexFwMinor, lexFwRevision);
                boolT vetoConnection = FALSE;

                lexVidStored = lexVid;

                ilog_LINKMGR_COMPONENT_3(
                    ILOG_MINOR_EVENT,
                    LINKMGR_CHECK_COMPATIBILITY_PAIR,
                    lexFwMajor,
                    lexFwMinor,
                    lexFwRevision);
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
                    _LINKMGR_vidIsInvalid(lexVid) || _LINKMGR_vidIsInvalid(GRG_GetVariantID());

                if (vetoConnection)
                {
                    ilog_LINKMGR_COMPONENT_2(
                            ILOG_FATAL_ERROR,
                            LINKMGR_VETOING_INCOMPATIBLE_VIDS,
                            lexVid,
                            GRG_GetVariantID());
                }

                /*
                // Do not allow extenders to pair if they are using different netcfg
                // implementations.  There's no technical reason why they couldn't pair, but this
                // makes Crestron units not pair with non-Crestron units which is the preference of
                // the customer.
                vetoConnection =
                    (lexCapabilities & (1 << TOPLEVEL_USE_BCAST_NET_CFG_PROTO_OFFSET)) !=
                    (rexCapabilities & (1 << TOPLEVEL_USE_BCAST_NET_CFG_PROTO_OFFSET));

                vetoConnection = vetoConnection ||
                    (GRG_GetVariantID() == GRG_VARIANT_ASIC_ITC1151 && lexVid != GRG_VARIANT_ASIC_ITC1151 );

                vetoConnection = vetoConnection ||
                    (GRG_GetVariantID() == GRG_VARIANT_ASIC_ITC2053 && lexVid != GRG_VARIANT_ASIC_ITC2053 );


                // ASICs can only talk to same variant
                // Don't check 1151, that was covered above
                // If LEX is ASIC - if REX also ASIC then VID must match
                //                - if REX FPGA - do not veto
                // If LEX is FPGA - do not veto
                vetoConnection = vetoConnection ||
                    ((lexVid == GRG_VARIANT_ASIC_ITC2051 ||
                      lexVid == GRG_VARIANT_ASIC_ITC2052 ||
                      lexVid == GRG_VARIANT_ASIC_ITC2054) &&
                    (GRG_GetPlatformID() == GRG_PLATFORMID_ASIC) && lexVid != GRG_GetVariantID());

                const enum TOPLEVEL_UnitBrand rexBrand = TOPLEVEL_getBrandNumber();
                vetoConnection =
                    vetoConnection || (!lexSupportsBrands && rexBrand != TOPLEVEL_BRAND_LEGACY);

                const boolT lexSupportsEndpointFiltering =
                    LINKMGR_doesFirmwareSupportEndpointFiltering(
                        lexFwMajor, lexFwMinor, lexFwRevision);

                // If LEX firmware does not support device interface filtering and filtering is
                // enabled on the REX, then veto the connection.
                const boolT filteringEnabledOnRex =
                    (       rexCapabilities
                        &   (   (1 << TOPLEVEL_BLOCK_MASS_STORAGE_OFFSET)
                            |   (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET)
                            |   (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET)
                            |   (1 << TOPLEVEL_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET))
                    ) != 0;

                vetoConnection =
                    vetoConnection || (!lexSupportsEndpointFiltering && filteringEnabledOnRex);

                // Veto connection if REX has ISO blocked, but LEX doesn't and LEX is too old to
                // support filtering.
                vetoConnection = vetoConnection || (!lexSupportsEndpointFiltering &&
                    !(rexCapabilities & (1 << TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET)) &&
                    (lexCapabilities & (1 << TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET)));
                */

//               const boolT lexSupportsCapabilityNegotiation =
//                   LINKMGR_doesFirmwareSupportCapabilityNegotiation(
//                       lexFwMajor, lexFwMinor, lexFwRevision);

                if (!vetoConnection)
                {
                    // If lex firmware does not support capability negotiation, we send back the
                    // LEX's CONFIGURATION_BITS value (bits 0 through 6) except for the MSA and ISO
                    // support settings.  For those values, we send back the result of ANDing the
                    // LEX and REX values together.  We only send back bits 0 through 6 because the
                    // older firmware versions will veto the connection if any of the transmitted
                    // bits do not match their settings.
//                    if (!lexSupportsCapabilityNegotiation)
//                    {
                    const uint64 mask =
                        ((1 << TOPLEVEL_SUPPORT_MSA_OFFSET) | (1 << TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET));
                    rexCapabilities =
                        (lexCapabilities & (~mask)) | ((rexCapabilities & mask) & (lexCapabilities & mask));
//                    }

                    const uint64 rexVariantId = GRG_GetVariantID();
                    // Place the VID in the high order bits of the capabilities
                    rexCapabilities |= rexVariantId << _REX_VID_OFFSET;

                    XCSR_XICSSendMessageWithExtraData(
                        LINK_MESSAGE,
                        REX_COMPATIBILITY_RESPONSE,
                        vport,
                        ((SOFTWARE_MAJOR_REVISION << 16) |
                         (SOFTWARE_MINOR_REVISION <<  8) |
                         (SOFTWARE_DEBUG_REVISION <<  0)
                        ),
                        rexCapabilities);
//                    if (lexSupportsBrands)
//                    {
                        linkState.xusbLinkMgr.rex.xusbLinkState = LINK_STATE_VERIFYING_BRAND;
//                    }
                }
                else
                {
                    const boolT clmInReset = FALSE;
                    ilog_LINKMGR_COMPONENT_1(ILOG_MAJOR_EVENT, LINKMGR_PAIR_INCOMPATIBLE, vport);
                    // Send a veto message to the LEX if the LEX firmware is new enough to
                    // understand it.
//                    if (lexSupportsCapabilityNegotiation)
//                    {
                        XCSR_XICSSendMessageWithExtraData(
                            LINK_MESSAGE, REX_VETO_CONNECTION, vport, 0, 0);
//                    }
                    _LINKMGR_takeDownLinkRex(clmInReset);
                }
            }
            break;

        case LEX_BRAND_QUERY:
            if(linkState.xusbLinkMgr.rex.xusbLinkState == LINK_STATE_VERIFYING_BRAND)
            {
                const enum TOPLEVEL_UnitBrand lexBrand = data & 0xFFFF;
                const enum TOPLEVEL_UnitBrand rexBrand = TOPLEVEL_getBrandNumber();
                const boolT vetoConnection = (rexBrand != lexBrand);

                if (!vetoConnection)
                {
                    XCSR_XICSSendMessageWithExtraData(
                        LINK_MESSAGE, REX_BRAND_RESPONSE, vport, rexBrand, 0);
                }
                else
                {
                    const boolT clmInReset = FALSE;
                    ilog_LINKMGR_COMPONENT_1(ILOG_MAJOR_EVENT, LINKMGR_PAIR_INCOMPATIBLE, vport);
                    _LINKMGR_takeDownLinkRex(clmInReset);
                }
            }
            break;

        case LEX_LINK_CONFIRMATION:
            if(linkState.xusbLinkMgr.rex.xusbLinkState == LINK_STATE_VERIFYING_BRAND ||
               linkState.xusbLinkMgr.rex.xusbLinkState == LINK_STATE_VERIFYING_COMPATIBILITY)
            {
                _LINKMGR_setRexLinkUp();
            }
            break;

        default:
            ilog_LINKMGR_COMPONENT_2(ILOG_MAJOR_ERROR, INVALID_MESSAGE, message, __LINE__);
            break;
    }
}


/**
* FUNCTION NAME: _LINKMGR_sendRexPairResponse()
*
* @brief  - Sends a rex pair response message to the given lex.
*
* @return - void.
*/
static void _LINKMGR_sendRexPairResponse(uint64 ownMacAddr, uint64 destLexMacAddr)
{
    const uint8 replyOnVport = 0;
    const uint32 lower24bitsOwnMac = (ownMacAddr >> 0 ) & 0x00ffffff;
    const uint32 upper24bitsOwnMac = (ownMacAddr >> 24) & 0x00ffffff;

    XCSR_XICSSendMessageWithExtraData(
        LINK_MESSAGE,
        REX_PAIR_RESPONSE,
        replyOnVport,
        upper24bitsOwnMac,
        (((destLexMacAddr & 0x00ffffff) << 32) | lower24bitsOwnMac));
    TIMING_TimerStart(linkState.xusbLinkMgr.rex.vportNegotiationTimer);
    linkState.xusbLinkMgr.rex.xusbLinkState = LINK_STATE_NEGOTIATING_VPORT;
}


/**
* FUNCTION NAME: _LINKMGR_mlpIrqRex()
*
* @brief  - This function is called when a vport link goes up or down.
*
* @note   - Given that this is on the REX, there should only ever be a single active vport.
*/
static void _LINKMGR_mlpIrqRex(void)
{
    // We mask off the vport 0 bit because it doesn't represent an MLP link like the other bits do
    const uint8 vportMasked = (CLM_GetVportStatusMask() & ~0x1);
    const boolT vpLinkUp = vportMasked != 0;
    const boolT vpLinkWasUp = linkState.xusbLinkMgr.rex.xusbLinkState == LINK_STATE_ACTIVE;

    if(vpLinkUp && !vpLinkWasUp)
    { // Link just came up
        linkState.xusbLinkMgr.rex.xusbLinkState = LINK_STATE_VERIFYING_COMPATIBILITY;
        _LINKMGR_rexDisableTxProbe();
    }
    else if(!vpLinkUp && vpLinkWasUp)
    { // Link just went down
        const boolT clmInReset = FALSE;
        _LINKMGR_takeDownLinkRex(clmInReset);
        _LINKMGR_rexEnableTxProbe();
    }
    _LINKMGR_updateLinkLed();
}


/**
* FUNCTION NAME: _LINKMGR_onPhyLinkUpRex()
*
* @brief  - Called once a PHY link is established.
*/
static void _LINKMGR_onPhyLinkUpRex(void)
{
    CLM_onPhyLinkUp(linkState.linkMode);
    _LINKMGR_updateLinkLed();

    _LINKMGR_rexEnableTxProbe();
}


/**
* FUNCTION NAME: _LINKMGR_onPhyLinkDownRex()
*
* @brief  - Called when the PHY link is lost.
*/
static void _LINKMGR_onPhyLinkDownRex(void)
{
    const boolT clmInReset = TRUE;
    _LINKMGR_takeDownLinkRex(clmInReset);
    _LINKMGR_rexDisableTxProbe();
}


/**
* FUNCTION NAME: _LINKMGR_addDeviceLinkRex()
*
* @brief  - Set which LEX this REX is paired with.
*
* @return - TRUE if the pairing was accepted, FALSE if the pairing was declined due to an existing
*           pairing.
*/
static boolT _LINKMGR_addDeviceLinkRex(uint64 deviceMACAddr)
{
    boolT paired = FALSE;

#ifndef GE_CORE
    if(!STORAGE_varExists(REX_PAIRED_MAC_ADDR))
    {
        union STORAGE_VariableData* var = STORAGE_varCreate(REX_PAIRED_MAC_ADDR);
        var->doubleWord = (deviceMACAddr << 16);
        STORAGE_varSave(REX_PAIRED_MAC_ADDR);
        // In point to point and multi rex mode, vport0 needs to be adjusted to be directed
        // at the correct MAC address
        if(linkState.linkMode == LINK_MODE_POINT_TO_POINT || linkState.linkMode == LINK_MODE_MULTI_REX)
        {
            const uint8 vport = 0;
            CLM_configureVportMacDst(vport, deviceMACAddr);
        }
        paired = TRUE;
    }
#endif
    return paired;
}


/**
* FUNCTION NAME: _LINKMGR_removeDeviceLinkRex()
*
* @brief  - Removes an existing linked device if one exists.
*
* @return - TRUE if a link was removed or FALSE if there was no existing link or the MAC address
*           passed as a parameter does not match the current link.
*/
static boolT _LINKMGR_removeDeviceLinkRex(uint64 deviceMACAddr)
{
    boolT pairingRemoved = FALSE;
#ifndef GE_CORE
    if(STORAGE_varExists(REX_PAIRED_MAC_ADDR))
    {
        union STORAGE_VariableData* var = STORAGE_varGet(REX_PAIRED_MAC_ADDR);
        if((var->doubleWord >> 16) == deviceMACAddr)
        {
            const boolT clmInReset = FALSE;
            STORAGE_varRemove(REX_PAIRED_MAC_ADDR);
            STORAGE_varSave(REX_PAIRED_MAC_ADDR);
            _LINKMGR_takeDownLinkRex(clmInReset);
            pairingRemoved = TRUE;
        }
    }
#endif
    return pairingRemoved;
}


/**
* FUNCTION NAME: _LINKMGR_removeAllDeviceLinksRex()
*
* @brief  - Removes all device pairings.
*
* @return - void.
*/
static void _LINKMGR_removeAllDeviceLinksRex(void)
{
#ifndef GE_CORE
    ilog_LINKMGR_COMPONENT_0(ILOG_MAJOR_EVENT, LINKMGR_REMOVE_ALL_LINKS);
    if(STORAGE_varExists(REX_PAIRED_MAC_ADDR))
    {
        const boolT clmInReset = FALSE;
        STORAGE_varRemove(REX_PAIRED_MAC_ADDR);
        STORAGE_varSave(REX_PAIRED_MAC_ADDR);
        _LINKMGR_takeDownLinkRex(clmInReset);
    }
#endif
}


/**
* FUNCTION NAME: _LINKMGR_vportNegotiationTimeoutHandler()
*
* @brief  - Cleans up the state of the system when a LEX does not respond to this REX with a
*           LEX_VPORT_ASSIGN message prior to the timeout.
*/
static void _LINKMGR_vportNegotiationTimeoutHandler(void)
{
    ilog_LINKMGR_COMPONENT_0(ILOG_DEBUG, LINKMGR_REX_VP_NEGOTIATION_TIMEOUT);

    if(linkState.xusbLinkMgr.rex.xusbLinkState == LINK_STATE_NEGOTIATING_VPORT)
    {
        // This means that we sent out a REX_PAIR_RESPONSE message after receiving a LEX_SEEKS_PAIR
        // message, but we did not receive a LEX_VPORT_ASSIGN message before the timeout occured.
        linkState.xusbLinkMgr.rex.xusbLinkState = LINK_STATE_INACTIVE;
    }
    else
    {
        iassert_LINKMGR_COMPONENT_1(
            FALSE, LINKMGR_INVALID_VP_NEG_TIMEOUT, linkState.xusbLinkMgr.rex.xusbLinkState);
    }
}


/**
* FUNCTION NAME: _LINKMGR_rexMlpLinkAcquisitionTimeoutHandler()
*
* @brief  - If the REX fails to establish an MLP link with the paired LEX in the allotted time, the
*           link is reset.
*/
static void _LINKMGR_rexMlpLinkAcquisitionTimeoutHandler(void)
{
    const boolT clmInReset = FALSE;

    ilog_LINKMGR_COMPONENT_0(ILOG_DEBUG, LINKMGR_MLP_ACQUISITION_TIMEOUT);

    _LINKMGR_takeDownLinkRex(clmInReset);
}


/**
* FUNCTION NAME: _LINKMGR_takeDownLinkRex()
*
* @brief  - Takes the link down and returns the link manager to the correct state.
*/
static void _LINKMGR_takeDownLinkRex(boolT clmInReset)
{
    const uint8 activeVport = GRG_RexGetVportID();
    TIMING_TimerStop(linkState.xusbLinkMgr.mlpAcquisitionTimer);
    TIMING_TimerStop(linkState.xusbLinkMgr.rex.vportNegotiationTimer);
    if(!clmInReset)
    {
        CLM_VPortDisable(activeVport);
    }
    XCSR_vportLinkDown(activeVport);
    REXULM_HandleCLMLinkDown();
    linkState.xusbLinkMgr.rex.xusbLinkState = LINK_STATE_INACTIVE;
    _LINKMGR_updateLinkLed();
}


/**
* FUNCTION NAME: _LINKMGR_setRexLinkUp()
*
* @brief  - Brings the REX link up.
*
* @return - void.
*/
static void _LINKMGR_setRexLinkUp(void)
{
    TIMING_TimerStop(linkState.xusbLinkMgr.mlpAcquisitionTimer);
    linkState.xusbLinkMgr.rex.xusbLinkState = LINK_STATE_ACTIVE;
    REXULM_HandleCLMLinkUp();
    _LINKMGR_updateLinkLed();
}


/**
* FUNCTION NAME: _LINKMGR_rexEnableTxProbe()
*
* @brief  - Enable periodic transmission of probe packets.
*
* @return - void.
*
* @note   - Only start the timer and send probes if we are not in point-to-point mode.
*/
static void _LINKMGR_rexEnableTxProbe(void)
{
    if (linkState.linkMode != LINK_MODE_DIRECT)
    {
        TIMING_TimerResetTimeout(txProbeTimer, TX_PROBE_INITIAL_RETRY_DELAY);
        _LINKMGR_rexSendTxProbe();
    }
}


/**
* FUNCTION NAME: _LINKMGR_rexDisableTxProbe()
*
* @brief  - Disable transmission of probe packets from the REX.
*
* @return - void.
*/
static void _LINKMGR_rexDisableTxProbe(void)
{
    TIMING_TimerStop(txProbeTimer);
}


/**
* FUNCTION NAME: _LINKMGR_rexDelayTxProbe()
*
* @brief  - Reset the current timeout until the next probe packet is sent out.
*
* @return - void.
*/
static void _LINKMGR_rexDelayTxProbe(void)
{
    TIMING_TimerStop(txProbeTimer);
    TIMING_TimerStart(txProbeTimer);
}


/**
* FUNCTION NAME: _LINKMGR_rexSendTxProbe()
*
* @brief  - Transmits a message to the LEX that this REX wants to pair with.
*
* @return - void.
*
* @note   - This function exists to ensure that MAC address tables inside routers between the LEX
*           and REX are updated.  Otherwise the REX would wait indefinitely for link negotiation to
*           be initiated by the LEX, but the packets would never be delivered because some switch
*           thinks that the REX is behind a different port.
*/
static void _LINKMGR_rexSendTxProbe(void)
{
    const uint32 currTimeout = TIMING_TimerGetTimeout(txProbeTimer);
    // Double the timer to do exponential backoff.
    uint32 nextTimeout = currTimeout << 1;
#ifndef GE_CORE
    uint64 ownMac = 0;
    uint64 lexMac = 0;

    if (STORAGE_varExists(REX_PAIRED_MAC_ADDR))
    {
        ownMac = STORAGE_varGet(MAC_ADDR)->doubleWord >> 16;
        lexMac = STORAGE_varGet(REX_PAIRED_MAC_ADDR)->doubleWord >> 16;
        _LINKMGR_sendRexPairResponse(ownMac, lexMac);
    }
#endif

    if (nextTimeout > TX_PROBE_MAXIMUM_RETRY_DELAY)
    {
        nextTimeout = currTimeout;
    }
    TIMING_TimerResetTimeout(txProbeTimer, nextTimeout);
    TIMING_TimerStart(txProbeTimer);
}
