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
//!   @file  - net_udp.c
//
//!   @brief - Implements the UDP transport layer protocol.
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <net_udp.h>
#include "net_udp_loc.h"
#include "net_ipv4_loc.h"
#include "net_dhcp_loc.h"
#include "net_pair_discovery_loc.h"
#include "net_log.h"


/************************ Defined Constants and Macros ***********************/
#define _NET_UDP_MAX_BOUND_PORTS 4


/******************************** Data Types *********************************/
struct _NET_PortMapping
{
    NET_UDPPort port;
    NET_UDPPortHandler handler;
};

/***************************** Local Variables *******************************/
struct _NET_PortMapping receivePortHandlers[_NET_UDP_MAX_BOUND_PORTS];
uint8 numPortsBound;


/************************ Local Function Prototypes **************************/
static sint8 _NET_udpFindPortHandlerIndex(
    NET_UDPPort) __attribute__((section(".ftext")));
static inline uint8* _NET_udpFullFromPayload(uint8* payload);
static inline uint8* _NET_udpPayloadFromFull(uint8* packet);


/***************** External Visibility Function Definitions ******************/

/**
* FUNCTION NAME: NET_udpBindPortHandler()
*
* @brief  - Binds a function to receive incoming UDP traffic on a specific
*           port.
*
* @return - TRUE if the binding is successful.  FALSE indicates that the
*           maximum number of bindings has been reached or that the port is
*           already bound.
*/
boolT NET_udpBindPortHandler(
    NET_UDPPort port, NET_UDPPortHandler portHandler)
{
    boolT success = FALSE;
    if(numPortsBound < _NET_UDP_MAX_BOUND_PORTS)
    {
        if(_NET_udpFindPortHandlerIndex(port) == -1)
        {
            struct _NET_PortMapping pm =
                { .port = port, .handler = portHandler };
            receivePortHandlers[numPortsBound++] = pm;
            success = TRUE;
        }
    }

    return success;
}

/**
* FUNCTION NAME: NET_udpRemovePortHandler()
*
* @brief  - Removes the UDP port -> function binding.
*
* @return - TRUE if the deregistration was successful.  A result of FALSE
*           indicates that the port never had a handler registered.
*/
boolT NET_udpRemovePortHandler(NET_UDPPort port)
{
    boolT success = FALSE;

    sint8 removalIdx = _NET_udpFindPortHandlerIndex(port);
    if(removalIdx != -1)
    {
        if(removalIdx != (numPortsBound - 1))
        {
            receivePortHandlers[removalIdx] =
                receivePortHandlers[--numPortsBound];
        }
    }

    return success;
}


/**
* FUNCTION NAME: NET_udpLookupPortHandler()
*
* @brief  - Gets the handler function associated with a given UDP port.
*
* @return - A function pointer to the handler or NULL if the port has no
*           handler.
*/
NET_UDPPortHandler NET_udpLookupPortHandler(NET_UDPPort port)
{
    sint8 idx = _NET_udpFindPortHandlerIndex(port);

    return idx == -1 ? NULL : receivePortHandlers[idx].handler;
}

/**
* FUNCTION NAME: NET_udpTransmitPacket()
*
* @brief  - Fills in the header information of the UDP packet and then sends it.
*
* @return - void
*/
void NET_udpTransmitPacket(
    uint8* udpPayload,
    NET_IPAddress dest,
    uint16 udpDataLength,
    NET_UDPPort srcPort,
    NET_UDPPort destPort)
{
    uint8* udpPacket = _NET_udpFullFromPayload(udpPayload);
    // source port
    NET_pack16Bits(
        srcPort, &udpPacket[NET_BUFFER_UDP_SRC_PORT_OFFSET]);

    // destination port
    NET_pack16Bits(
        destPort, &udpPacket[NET_BUFFER_UDP_DEST_PORT_OFFSET]);

    // length - the +8 is for header length
    NET_pack16Bits(
        udpDataLength + 8, &udpPacket[NET_BUFFER_UDP_LENGTH_OFFSET]);

    // checksum - optional, we just fill with zeros
    NET_pack16Bits(0, &udpPacket[NET_BUFFER_UDP_CHECKSUM_OFFSET]);

    _NET_ipv4TransmitPacket(udpPacket, udpDataLength + 8, dest, NET_IPV4_PROTOCOL_UDP);
}

/**
* FUNCTION NAME: NET_udpAllocateBuffer()
*
* @brief  - Allocates a buffer and returns a pointer to the payload of the
*           UDP packet.
*
* @return - A pointer to the UDP payload of a newly allocated buffer.
*
* @note   - Lower level functions will assert if the memory allocation fails.
*/
uint8* NET_udpAllocateBuffer(void)
{
    return _NET_udpPayloadFromFull(_NET_ipv4AllocateBuffer());
}

/**
* FUNCTION NAME: NET_udpFreeBuffer()
*
* @brief  - Frees the buffer associated with the pointer to an UDP payload
*           that is passed as a parameter.
*
* @return - None
*/
void NET_udpFreeBuffer(uint8* buffer)
{
    _NET_ipv4FreeBuffer(_NET_udpFullFromPayload(buffer));
}


/***************** Component Visibility Function Definitions *****************/

/**
* FUNCTION NAME: _NET_udpInitialize()
*
* @brief  - Initializes the udp functionality.
*
* @return - void
*/
void _NET_udpInitialize(void)
{
    numPortsBound = 0;

    // Initialize services that run on top of UDP
    NET_dhcpInitialize();
    _NET_pairInit();
}

/**
* FUNCTION NAME: _NET_udpOnLinkDown()
*
* @brief  - Takes action relating to the udp protocol when it is found that the
*           link has gone down.
*
* @return - void
*/
void _NET_udpOnLinkDown(void)
{
    _NET_pairOnLinkDown();
    NET_dhcpOnLinkDown();
}

/**
* FUNCTION NAME: _NET_udpOnLinkUp()
*
* @brief  - Takes action relating to the udp protocol when it is found the link
*           has come up.
*
* @return - void
*/
void _NET_udpOnLinkUp(void)
{
    NET_dhcpOnLinkUp();
    _NET_pairOnLinkUp();
}

/**
* FUNCTION NAME: _NET_udpReceivePacketHandler()
*
* @brief  - Handles a received UDP packet.
*
* @return - void
*/
void _NET_udpReceivePacketHandler(
    uint8* packet, uint16 packetSize, NET_IPAddress senderIP)
{
    uint16 udpLength =
        NET_unpack16Bits(&packet[NET_BUFFER_UDP_LENGTH_OFFSET]);

    if(packetSize == udpLength)
    {
        uint16 destPort =
            NET_unpack16Bits(&packet[NET_BUFFER_UDP_DEST_PORT_OFFSET]);
        uint16 senderPort =
            NET_unpack16Bits(&packet[NET_BUFFER_UDP_SRC_PORT_OFFSET]);
        NET_UDPPortHandler portHandler = NET_udpLookupPortHandler(destPort);
        if(portHandler != NULL)
        {
            (*portHandler)(
                destPort,
                senderIP,
                senderPort,
                _NET_udpPayloadFromFull(packet),
                packetSize - NET_BUFFER_UDP_PAYLOAD_OFFSET);
        }
    }
    else
    {
        ilog_NET_COMPONENT_2(
            ILOG_MAJOR_EVENT,
            NET_UDP_PACKET_SIZE_DOES_NOT_MATCH_DATA,
            udpLength,
            packetSize);
    }
}


/******************** File Visibility Function Definitions *******************/

/**
* FUNCTION NAME: _NET_udpFindPortHandlerIndex()
*
* @brief  - Gives the array index into the receivePortHandlers array of the
*           port handler for the given port.
*
* @return - void
*/
static sint8 _NET_udpFindPortHandlerIndex(NET_UDPPort port)
{
    sint8 idx = -1;
    uint8 i;
    for(i = 0; i < numPortsBound; i++)
    {
        if(receivePortHandlers[i].port == port)
        {
            idx = i;
            break;
        }
    }
    return idx;
}

/**
* FUNCTION NAME: _NET_udpFullFromPayload()
*
* @brief  - Gives a pointer to a full udp frame based on a pointer to the udp
*           payload.
*
* @return - A pointer to the whole udp frame.
*/
static inline uint8* _NET_udpFullFromPayload(uint8* payload)
{
    return payload - NET_BUFFER_UDP_PAYLOAD_OFFSET;
}

/**
* FUNCTION NAME: _NET_udpPayloadFromFull()
*
* @brief  - Gives a pointer to the udp payload based given an pointer to the
*           entire udp frame.
*
* @return - A pointer to the udp paylod.
*/
static inline uint8* _NET_udpPayloadFromFull(uint8* packet)
{
    return packet + NET_BUFFER_UDP_PAYLOAD_OFFSET;
}
