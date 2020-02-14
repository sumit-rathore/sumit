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
//!   @file  - net_pair_discovery.c
//
//!   @brief - Defines a simple protocol on top of UDP to negotiate pairings.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "net_pair_discovery_loc.h"
#include "net_udp.h"
#include "net_log.h"
#include <net_ethernet.h>
#include <net_base.h>
#include <storage_Data.h>
#include <storage_vars.h>
#include <ibase.h>
#include <timing_timers.h>
#include <grg.h>
#include <linkmgr.h>

/************************ Defined Constants and Macros ***********************/
#define _NET_PAIR_PORT_V1 9387
#define _NET_PAIR_PORT_V2_AND_UP 9388

#define _NET_MSGT_LEX_ADVERTISE 0 // Uses lexMacAddr
#define _NET_MSGT_REX_REQUEST   1 // Uses lexMacAddr and rexMacAddr
#define _NET_MSGT_LEX_ASSIGN    2 // Uses lexMacAddr and rexMacAddr

// Helper macros for sending messages
#define _NET_lexSendAdvertise(_protocolVersion_, _lexMac_) \
    _NET_sendPairingMessage(_protocolVersion_, _NET_MSGT_LEX_ADVERTISE, _lexMac_, 0);
#define _NET_rexSendRequest(_protocolVersion_, _lexMac_, _rexMac_) \
    _NET_sendPairingMessage(_protocolVersion_, _NET_MSGT_REX_REQUEST, _lexMac_, _rexMac_);
#define _NET_lexSendAssign(_protocolVersion_, _lexMac_, _rexMac_) \
    _NET_sendPairingMessage(_protocolVersion_, _NET_MSGT_LEX_ASSIGN, _lexMac_, _rexMac_);

/******************************** Data Types *********************************/

// This is the on-wire format for every message in this protocol.  Not all fields are used for
// every message type.  See the message type definitions for descriptions of which fields are
// valid.
struct _NET_PairPacketV1
{
    uint8 messageType;
    uint8 lexMacAddr[6];
    uint8 rexMacAddr[6];
} __attribute__((__packed__));

struct _NET_PairPacketV2
{
    uint8 protocolVersion;
    uint8 messageType;
    uint8 lexMacAddr[6];
    uint8 rexMacAddr[6];
    uint16 brand;
} __attribute__((__packed__));


enum _NET_DiscoveryState
{
    DISCOVERY_STATE_DISABLED,
    DISCOVERY_STATE_IDLE,
    DISCOVERY_STATE_WAITING_FOR_RESPONSE,
};


/***************************** Local Variables *******************************/
static struct
{
    enum _NET_DiscoveryState state;
    TIMING_TimerHandlerT waitForResponseTimer;
    union
    {
        struct
        {
            TIMING_TimerHandlerT pairingBroadcastTimer;
        } lex;
        struct
        {
            NET_MACAddress respondedToMac;
            TIMING_TimerHandlerT legacyAdvertiseReceiveTimer;
            boolT acceptLegacyAdvertisePackets;
        } rex;
    };
} _pairDiscoveryGlobals;


/************************ Local Function Prototypes **************************/
static void _NET_pairPacketHandler(
    NET_UDPPort destPort,
    NET_IPAddress senderIP,
    NET_UDPPort senderPort,
    uint8* data,
    uint16 dataLength) __attribute__ ((section (".ftext")));
static boolT _NET_createPairTo(NET_MACAddress pairMac);
static void _NET_pairingBroadcastTimeoutHandler(void);
static void _NET_waitForResponseTimeoutHandler(void) __attribute__ ((section (".ftext")));
static void _NET_legacyAdvertiseReceiveTimeoutHandler(void) __attribute__ ((section (".ftext")));
static void _NET_beginPairingDiscovery(void) __attribute__ ((section (".ftext")));
static void _NET_endPairingDiscovery(void) __attribute__ ((section (".ftext")));
static void _NET_endPairingDiscoveryInternal(void) __attribute__ ((section (".ftext")));
static void _NET_sendPairingMessage(
    uint8 protocolVersion,
    uint8 messageType,
    NET_MACAddress lexMac,
    NET_MACAddress rexMac) __attribute__ ((section (".ftext")));
static void _NET_pairingAddedNotification(void);
static boolT _NET_canContinuePairing(void) __attribute__ ((section (".ftext")));

/***************** External Visibility Function Definitions ******************/

/***************** Component Visibility Function Definitions *****************/
void _NET_pairInit(void)
{
    _pairDiscoveryGlobals.state = DISCOVERY_STATE_DISABLED;

    if(GRG_IsDeviceLex())
    {
        const boolT isPeriodic = TRUE;
        const uint32 timeoutInMs = 1000;
        _pairDiscoveryGlobals.lex.pairingBroadcastTimer = TIMING_TimerRegisterHandler(
            &_NET_pairingBroadcastTimeoutHandler, isPeriodic, timeoutInMs);
    }
    else
    {
        const boolT isPeriodic = FALSE;
        const uint32 timeoutInMs = 6000;
        _pairDiscoveryGlobals.rex.legacyAdvertiseReceiveTimer = TIMING_TimerRegisterHandler(
            &_NET_legacyAdvertiseReceiveTimeoutHandler, isPeriodic, timeoutInMs);
        _pairDiscoveryGlobals.rex.acceptLegacyAdvertisePackets = FALSE;
    }

    {
        const boolT isPeriodic = FALSE;
        const uint32 timeoutInMs = 20;
        _pairDiscoveryGlobals.waitForResponseTimer = TIMING_TimerRegisterHandler(
            &_NET_waitForResponseTimeoutHandler, isPeriodic, timeoutInMs);
    }

    LINKMGR_setButtonPairingCallbacks(&_NET_beginPairingDiscovery, &_NET_endPairingDiscovery);

    LINKMGR_setPairingAddedCallback(&_NET_pairingAddedNotification);

    // Only listen to the legacy port if we want to pair with legacy devices
    const boolT pairWithLegacy = TOPLEVEL_getBrandNumber() == TOPLEVEL_BRAND_LEGACY;
    if (pairWithLegacy)
    {
        iassert_NET_COMPONENT_1(
            NET_udpBindPortHandler(_NET_PAIR_PORT_V1, &_NET_pairPacketHandler),
            NET_UDP_COULD_NOT_BIND_TO_PORT,
            _NET_PAIR_PORT_V1);
    }
    iassert_NET_COMPONENT_1(
        NET_udpBindPortHandler(_NET_PAIR_PORT_V2_AND_UP, &_NET_pairPacketHandler),
        NET_UDP_COULD_NOT_BIND_TO_PORT,
        _NET_PAIR_PORT_V2_AND_UP);
}


void _NET_pairOnLinkDown(void)
{
    _pairDiscoveryGlobals.state = DISCOVERY_STATE_DISABLED;
    TIMING_TimerStop(_pairDiscoveryGlobals.waitForResponseTimer);
}


void _NET_pairOnLinkUp(void)
{
    _pairDiscoveryGlobals.state = DISCOVERY_STATE_IDLE;
}


/******************** File Visibility Function Definitions *******************/

static void _NET_pairPacketHandler(
    NET_UDPPort destPort,
    NET_IPAddress senderIP,
    NET_UDPPort senderPort,
    uint8* data,
    uint16 dataLength)
{
    uint8 protoVersion = 0;
    uint8 messageType = 0;
    NET_MACAddress unpackedLexMac = 0;
    NET_MACAddress unpackedRexMac = 0;
    uint16 theirBrand = TOPLEVEL_BRAND_LEGACY;

    // If push button pairing is not active, just return
    if (!LINKMGR_isPushButtonPairingActive())
    {
        return;
    }

    if (destPort == _NET_PAIR_PORT_V1)
    {
        if (dataLength == sizeof(struct _NET_PairPacketV1))
        {
            struct _NET_PairPacketV1* packet = (struct _NET_PairPacketV1*)data;
            protoVersion = 1;
            messageType = packet->messageType;
            unpackedLexMac = NET_unpack48Bits(packet->lexMacAddr);
            unpackedRexMac = NET_unpack48Bits(packet->rexMacAddr);
        }
    }
    else if (destPort == _NET_PAIR_PORT_V2_AND_UP)
    {
        if (dataLength == sizeof(struct _NET_PairPacketV2))
        {
            struct _NET_PairPacketV2* packet = (struct _NET_PairPacketV2*)data;
            if (packet->protocolVersion == 2)
            {
                protoVersion = 2;
                messageType = packet->messageType;
                unpackedLexMac = NET_unpack48Bits(packet->lexMacAddr);
                unpackedRexMac = NET_unpack48Bits(packet->rexMacAddr);
                theirBrand = packet->brand;
            }
        }
    }

    // Just return if we should not respond to the packet
    {
        uint16 ourBrand = TOPLEVEL_getBrandNumber();
        const boolT refusePairLegacy = ourBrand != TOPLEVEL_BRAND_LEGACY;

        if (protoVersion == 1 && refusePairLegacy)
        {
            return;
        }
        else if (protoVersion == 2)
        {
            if (GRG_IsDeviceRex())
            {
                // We received a v2 packet so stop accepting legacy packets for now.
                _pairDiscoveryGlobals.rex.acceptLegacyAdvertisePackets = FALSE;
                TIMING_TimerStop(_pairDiscoveryGlobals.rex.legacyAdvertiseReceiveTimer);
            }

            if (theirBrand != ourBrand)
            {
                return;
            }

        }
    }

    if(GRG_IsDeviceLex())
    {
        if(   messageType == _NET_MSGT_REX_REQUEST
           && _pairDiscoveryGlobals.state == DISCOVERY_STATE_WAITING_FOR_RESPONSE
           && unpackedLexMac == STORAGE_varGet(MAC_ADDR)->doubleWord >> 16)
        {
            if(_NET_createPairTo(unpackedRexMac))
            {
                _NET_lexSendAssign(protoVersion, unpackedLexMac, unpackedRexMac);
                _NET_endPairingDiscoveryInternal();
            }
        }
    }
    else
    { // rex
        switch (messageType)
        {
            case _NET_MSGT_LEX_ADVERTISE:
                if (_pairDiscoveryGlobals.state == DISCOVERY_STATE_IDLE)
                {
                    boolT processPacket = TRUE;
                    if (protoVersion == 1)
                    {
                        if (!_pairDiscoveryGlobals.rex.acceptLegacyAdvertisePackets)
                        {
                            processPacket = FALSE;
                            if (!TIMING_TimerEnabled(_pairDiscoveryGlobals.rex.legacyAdvertiseReceiveTimer))
                            {
                                // We have received a protocol v1 advertise packet and the
                                // acceptLegacyAdvertisePackets is FALSE and the
                                // legacyAdvertiseReceiveTimer is not running, so we start the
                                // timer.
                                TIMING_TimerStart(_pairDiscoveryGlobals.rex.legacyAdvertiseReceiveTimer);
                            }
                        }
                    }

                    if (processPacket)
                    {
                        _NET_rexSendRequest(protoVersion, unpackedLexMac, STORAGE_varGet(MAC_ADDR)->doubleWord >> 16);
                        _pairDiscoveryGlobals.rex.respondedToMac = unpackedLexMac;
                        _pairDiscoveryGlobals.state = DISCOVERY_STATE_WAITING_FOR_RESPONSE;
                        TIMING_TimerStart(_pairDiscoveryGlobals.waitForResponseTimer);
                    }
                }
                break;

            case _NET_MSGT_LEX_ASSIGN:
                if (_pairDiscoveryGlobals.state == DISCOVERY_STATE_WAITING_FOR_RESPONSE &&
                    unpackedRexMac == (STORAGE_varGet(MAC_ADDR)->doubleWord >> 16) &&
                    unpackedLexMac == _pairDiscoveryGlobals.rex.respondedToMac)
                {
                    if(_NET_createPairTo(unpackedLexMac))
                    {
                        _NET_endPairingDiscoveryInternal();
                    }
                }
                break;

            default:
                // Just ignore unknown message types
                break;
        }
    }
}


static boolT _NET_createPairTo(NET_MACAddress pairMac)
{
    boolT linkExists = LINKMGR_isDevicePairedWith(pairMac);
    if(!linkExists)
    {
        if(LINKMGR_viewLinkMode() == LINK_MODE_POINT_TO_POINT || (LINKMGR_viewLinkMode() == LINK_MODE_MULTI_REX && GRG_IsDeviceRex()))
        {
            // Unconditionally unpair because we want to overwrite the current pair (if
            // there is one) in point to point mode.
            LINKMGR_removeAllDeviceLinks();
        }

        linkExists = LINKMGR_addDeviceLink(pairMac);
    }
    return linkExists;
}



/**
* FUNCTION NAME: ()
*
* @brief  - Sends out one pair discovery message.  If this device supports communication with
*           legacy devices, it sends out the legacy broadcast once every two messages.
*
* @return - void.
*
* @note   - This is a lex only function.
*/
static void _NET_pairingBroadcastTimeoutHandler(void)
{
    uint8 protoVersionToSend = 2;
    static boolT sendLegacy = TRUE;
    sendLegacy = !sendLegacy;
    const boolT pairWithLegacy = TOPLEVEL_getBrandNumber() == TOPLEVEL_BRAND_LEGACY;
    if(sendLegacy && pairWithLegacy)
    {
        protoVersionToSend = 1;
    }

    if(_pairDiscoveryGlobals.state == DISCOVERY_STATE_IDLE)
    {
        _NET_lexSendAdvertise(protoVersionToSend, STORAGE_varGet(MAC_ADDR)->doubleWord >> 16);
        TIMING_TimerStart(_pairDiscoveryGlobals.waitForResponseTimer);
        _pairDiscoveryGlobals.state = DISCOVERY_STATE_WAITING_FOR_RESPONSE;
    }
}


static void _NET_waitForResponseTimeoutHandler(void)
{
    _pairDiscoveryGlobals.state = DISCOVERY_STATE_IDLE;
    if(GRG_IsDeviceLex())
    {
    }
    else
    { // Device is Rex
        _pairDiscoveryGlobals.rex.respondedToMac = 0;
    }
}


/**
* FUNCTION NAME: _NET_legacyAdvertiseReceiveTimeoutHandler()
*
* @brief  - Timer handler which switches between allowing the reception of legacy pairing advertise
*           packets or not.
*
* @return - void.
*/
static void _NET_legacyAdvertiseReceiveTimeoutHandler(void)
{
    if (_pairDiscoveryGlobals.rex.acceptLegacyAdvertisePackets)
    {
        _pairDiscoveryGlobals.rex.acceptLegacyAdvertisePackets = FALSE;
    }
    else
    {
        _pairDiscoveryGlobals.rex.acceptLegacyAdvertisePackets = TRUE;
        // Start the timer so that we will stop accepting legacy packets when the timeout happens.
        TIMING_TimerStart(_pairDiscoveryGlobals.rex.legacyAdvertiseReceiveTimer);
    }
}


static void _NET_beginPairingDiscovery(void)
{
    if(_NET_canContinuePairing())
    {
        if(GRG_IsDeviceLex())
        {
            TIMING_TimerStart(_pairDiscoveryGlobals.lex.pairingBroadcastTimer);
            // Simulate an immediate timeout to send out a broadcast
            _NET_pairingBroadcastTimeoutHandler();
        }
    }
    else
    {
        _NET_endPairingDiscoveryInternal();
    }
}


static void _NET_endPairingDiscovery(void)
{
    _pairDiscoveryGlobals.state = DISCOVERY_STATE_IDLE;
    TIMING_TimerStop(_pairDiscoveryGlobals.waitForResponseTimer);
    if(GRG_IsDeviceLex())
    {
        TIMING_TimerStop(_pairDiscoveryGlobals.lex.pairingBroadcastTimer);
    }
    else
    {
        _pairDiscoveryGlobals.rex.respondedToMac = 0;
    }
}


static void _NET_endPairingDiscoveryInternal(void)
{
    // This call will result in a callback from the LINKMGR layer to the _NET_endPairingDiscovery()
    // function, but it will also properly reset the state of the button management.
    LINKMGR_completeButtonPairing();
}


/**
* FUNCTION NAME: _NET_sendPairingMessage()
*
* @brief  - Sends a message over UDP.
*
* @return - void.
*
* @note   - This function should not be called directly.  Use the _NET_(lex|rex)Send* macros.
*/
static void _NET_sendPairingMessage(
    uint8 protocolVersion, uint8 messageType, NET_MACAddress lexMac, NET_MACAddress rexMac)
{
    uint8* buffer = NET_udpAllocateBuffer();
    const NET_UDPPort port = (protocolVersion == 1) ? _NET_PAIR_PORT_V1 : _NET_PAIR_PORT_V2_AND_UP;
    uint16 dataLength = 0;

    if (protocolVersion == 1)
    {
        struct _NET_PairPacketV1* packet = (struct _NET_PairPacketV1*)buffer;
        dataLength = sizeof(struct _NET_PairPacketV1);
        packet->messageType = messageType;
        NET_pack48Bits(lexMac, packet->lexMacAddr);
        NET_pack48Bits(rexMac, packet->rexMacAddr);
    }
    else if (protocolVersion == 2)
    {
        struct _NET_PairPacketV2* packet = (struct _NET_PairPacketV2*)buffer;
        dataLength = sizeof(struct _NET_PairPacketV2);
        packet->protocolVersion = protocolVersion;
        packet->messageType = messageType;
        NET_pack48Bits(lexMac, packet->lexMacAddr);
        NET_pack48Bits(rexMac, packet->rexMacAddr);
        packet->brand = TOPLEVEL_getBrandNumber();
    }

    NET_udpTransmitPacket(buffer, NET_IPV4_LIMITED_BROADCAST_ADDRESS, dataLength, port, port);
}


/**
* FUNCTION NAME: _NET_pairingAddedNotification()
*
* @brief  - This is a callback function intended to be called when a new pairing is added to the
*           system through means other than push button pairing.  This allows us to cancel push
*           button pairing if it is no longer valid.
*
* @return - void.
*/
static void _NET_pairingAddedNotification(void)
{
    if(!_NET_canContinuePairing())
    {
        _NET_endPairingDiscoveryInternal();
    }
}

/**
* FUNCTION NAME: _NET_canContinuePairing()
*
* @brief  - Checks rules to see whether it is possible to continue push button pairing based on the
*           pair(s) of this unit and the link mode.
*
* @return - TRUE if this unit may remain in push button pairing mode or FALSE otherwise.
*/
static boolT _NET_canContinuePairing(void)
{
    return (LINKMGR_viewLinkMode() == LINK_MODE_MULTI_REX && GRG_IsDeviceLex()) ?
        LINKMGR_hasOpenPairing() :
        TRUE;
}
