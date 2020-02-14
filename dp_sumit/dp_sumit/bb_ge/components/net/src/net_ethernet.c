///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  - net_ethernet.c
//
//!   @brief - Provides the functionality for sending and receiving raw
//             ethernet frames.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <net_ethernet.h>
#include "net_ethernet_loc.h"
#include "net_log.h"
#include "net_ipv4_loc.h"
#include "net_arp_loc.h"
#include "net_base.h"


/************************ Defined Constants and Macros ***********************/
//-----------------------------------------------------------------------------
// Offsets into the buffer for the ethernet layer
//-----------------------------------------------------------------------------
#define NET_BUFFER_ETHERNET_BEGIN                   0
#define NET_BUFFER_ETHERNET_DEST_MAC_OFFSET         0
#define NET_BUFFER_ETHERNET_SRC_MAC_OFFSET          6
#define NET_BUFFER_ETHERNET_ETHERTYPE_OFFSET        12
#define NET_BUFFER_ETHERNET_PAYLOAD_OFFSET          14


/******************************** Data Types *********************************/


/***************************** Local Variables *******************************/
static NET_MACAddress localMAC;
static boolT initialized = FALSE;


/************************ Local Function Prototypes **************************/
static inline boolT _NET_isEthernetInitialized(void);
static inline uint8* _NET_ethernetFullFromPayload(uint8* payload);
static inline uint8* _NET_ethernetPayloadFromFull(uint8* frame);


/***************** External Visibility Function Definitions ******************/

/**
* FUNCTION NAME: NET_ethernetGetSelfMACAddress()
*
* @brief  - Gives the MAC address of this device.
*
* @return - The MAC address of this device.
*/
NET_MACAddress NET_ethernetGetSelfMACAddress(void)
{
    return localMAC;
}


/***************** Component Visibility Function Definitions *****************/

/**
* FUNCTION NAME: _NET_ethernetInitialize()
*
* @brief  - Initialized the ethernet level of the networking component.
*
* @return - void
*/
void _NET_ethernetInitialize(NET_MACAddress localMACAddr)
{
    localMAC = localMACAddr;
    initialized = TRUE;
    _NET_ipv4Initialize();
    _NET_arpInitialize();
}

/**
* FUNCTION NAME: _NET_ethernetOnLinkDown()
*
* @brief  - Takes action relating to the ethernet protocol when it is found
*           that the link has gone down.
*
* @return - void
*/
void _NET_ethernetOnLinkDown(void)
{
    _NET_ipv4OnLinkDown();
    _NET_arpOnLinkDown();
}

/**
* FUNCTION NAME: _NET_ethernetOnLinkUp()
*
* @brief  - Takes action relating to the ethernet protocol when it is found
*           that the link has come up.
*
* @return - void
*/
void _NET_ethernetOnLinkUp(void)
{
    _NET_arpOnLinkUp();
    _NET_ipv4OnLinkUp();
}

/**
* FUNCTION NAME: _NET_ethernetTransmitFrame()
*
* @brief  - Populates the ethernet layer headers in the network buffer and then transmits it over
*           the network.
*
* @return - void
*/
void _NET_ethernetTransmitFrame(
    uint8* payload,
    uint16 payloadSize,
    NET_MACAddress dest,
    NET_Ethertype ethertype)
{
    uint8* frame;
    uint16 ethernetFrameSize;
    iassert_NET_COMPONENT_0(_NET_isEthernetInitialized(), NET_NOT_INITIALIZED);

    frame = _NET_ethernetFullFromPayload(payload);

    // destination MAC address
    NET_pack48Bits(dest, &frame[NET_BUFFER_ETHERNET_DEST_MAC_OFFSET]);

    // source MAC address
    NET_pack48Bits(localMAC, &frame[NET_BUFFER_ETHERNET_SRC_MAC_OFFSET]);

    // ethertype
    NET_pack16Bits(ethertype, &frame[NET_BUFFER_ETHERNET_ETHERTYPE_OFFSET]);

    // 46 octets is the minimum size of the ethernet payload, so we make sure that the payload is
    // set to at least that size.  If the actual payload was smaller, then the extra data will just
    // be zeros.
    payloadSize = MAX(46, payloadSize);

    ethernetFrameSize = payloadSize +
        (NET_BUFFER_ETHERNET_PAYLOAD_OFFSET - NET_BUFFER_ETHERNET_BEGIN);

    _NET_transmitFrame(frame, ethernetFrameSize);
}

/**
* FUNCTION NAME: NET_ethernetReceiveFrameHandler()
*
* @brief  - Called when an ethernet frame is received.
*
* @return - void
*
* @note   - We do not need to check the destination MAC address because
*           hardware filters out all ethernet frames which are not destined for
*           us or are not broadcast frames.
*/
void _NET_ethernetReceiveFrameHandler(uint8* frame, uint16 frameSize)
{
    NET_Ethertype et;
    uint8* ethernetPayload;
    uint16 payloadSize;
    iassert_NET_COMPONENT_0(_NET_isEthernetInitialized(), NET_NOT_INITIALIZED);

    et = NET_unpack16Bits(&frame[NET_BUFFER_ETHERNET_ETHERTYPE_OFFSET]);

    ethernetPayload = _NET_ethernetPayloadFromFull(frame);
    payloadSize = frameSize - (ethernetPayload - frame);
    switch(et)
    {
        case NET_ETHERTYPE_IPV4:
            _NET_ipv4ReceivePacketHandler(ethernetPayload, payloadSize);
            break;

        case NET_ETHERTYPE_ARP:
            _NET_arpReceivePacketHandler(ethernetPayload, payloadSize);
            break;

        default:
            // Silently ignore all other ethertypes
            break;
    }
}

/**
* FUNCTION NAME: _NET_ethernetAllocateBuffer()
*
* @brief  - Allocates a buffer and returns a pointer to the payload of the
*           ethernet frame.
*
* @return - A pointer to the ethernet payload of a newly allocated buffer.
*
* @note   - Lower level functions will assert if the memory allocation fails.
*/
uint8* _NET_ethernetAllocateBuffer(void)
{
    return _NET_ethernetPayloadFromFull(_NET_allocateBuffer());
}

/**
* FUNCTION NAME: _NET_ethernetFreeBuffer()
*
* @brief  - Frees the buffer associated with the pointer to an ethernet payload
*           that is passed as a parameter.
*
* @return - None
*/
void _NET_ethernetFreeBuffer(uint8* buffer)
{
    _NET_freeBuffer(_NET_ethernetFullFromPayload(buffer));
}


/******************** File Visibility Function Definitions *******************/

/**
* FUNCTION NAME: _NET_isEthernetInitialized()
*
* @brief  - Gives the MAC address of this device.
*
* @return - The MAC address of this device.
*/
static inline boolT _NET_isEthernetInitialized(void)
{
    return initialized;
}

/**
* FUNCTION NAME: _NET_ethernetFullFromPayload()
*
* @brief  - Gives a pointer to a full ethernet frame based on a pointer to the
*           ethernet payload.
*
* @return - A pointer to the whole ethernet frame.
*/
static inline uint8* _NET_ethernetFullFromPayload(uint8* payload)
{
    return payload - NET_BUFFER_ETHERNET_PAYLOAD_OFFSET;
}

/**
* FUNCTION NAME: _NET_ethernetPayloadFromFull()
*
* @brief  - Gives a pointer to the ethernet payload based given an pointer to
*           the entire ethernet frame.
*
* @return - A pointer to the ethernet paylod.
*/
static inline uint8* _NET_ethernetPayloadFromFull(uint8* frame)
{
    return frame + NET_BUFFER_ETHERNET_PAYLOAD_OFFSET;
}

