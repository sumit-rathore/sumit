///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013
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
//!   @file  - net_icmp.c
//
//!   @brief - This module implements echo reply in response to echo requests
//             over ICMP.
//
//
//!   @note  - At most 128 bytes of data will be echoed back to the client.
//             If more than 128 bytes are sent by the client, the echoed data
//             will be truncated.
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <net_base.h>
#include "net_icmp_loc.h"
#include "net_ipv4_loc.h"
#include "net_log.h"

/************************ Defined Constants and Macros ***********************/
#define NET_BUFFER_ICMP_BEGIN           0
#define NET_BUFFER_ICMP_TYPE_OFFSET     NET_BUFFER_ICMP_BEGIN
#define NET_BUFFER_ICMP_CODE_OFFSET     (NET_BUFFER_ICMP_TYPE_OFFSET + 1)
#define NET_BUFFER_ICMP_CHECKSUM_OFFSET (NET_BUFFER_ICMP_CODE_OFFSET + 1)
#define NET_BUFFER_ICMP_REST_OFFSET     (NET_BUFFER_ICMP_CHECKSUM_OFFSET + 2)

// Because ping is such a simple protocol and ICMP is not currently used for anything else, we just
// implement ping in this same file.  If in future, we start to implement more ICMP message types,
// we should consider splitting each message type into its own file.
#define NET_BUFFER_PING_IDENTIFIER_OFFSET       NET_BUFFER_ICMP_REST_OFFSET
#define NET_BUFFER_PING_SEQUENCE_NUMBER_OFFSET  (NET_BUFFER_PING_IDENTIFIER_OFFSET + 2)
#define NET_BUFFER_PING_DATA_OFFSET             (NET_BUFFER_PING_SEQUENCE_NUMBER_OFFSET + 2)

#define NET_ICMP_TYPE_ECHO_REPLY        0
#define NET_ICMP_CODE_ECHO_REPLY        0

#define NET_ICMP_TYPE_ECHO_REQUEST      8
#define NET_ICMP_CODE_ECHO_REQUEST      0

#define MAX_ECHO_REPLY_LENGTH           128


/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void _NET_transmitEchoReply(
    NET_IPAddress destination,
    uint16 identifier,
    uint16 sequenceNumber,
    uint8* data,
    uint16 dataLength) __attribute__((section(".ftext")));
static uint16 _NET_icmpComputeChecksum(
    const uint8* icmpPacket, uint16 packetSize) __attribute__((section(".ftext")));



/**
* FUNCTION NAME: _NET_icmpReceivePacketHandler()
*
* @brief  - Handler for all incoming ICMP traffic.
*
* @return - void.
*
* @note   - Only echo requests (pings) are currently handled.
*/
void _NET_icmpReceivePacketHandler(uint8* packet, uint16 packetSize, NET_IPAddress senderIP)
{
    if (NET_ipv4IsNetworkConfigured() && packetSize >= NET_BUFFER_PING_DATA_OFFSET)
    {
        const uint8 type = packet[NET_BUFFER_ICMP_TYPE_OFFSET];
        const uint8 code = packet[NET_BUFFER_ICMP_CODE_OFFSET];
        if (type == NET_ICMP_TYPE_ECHO_REQUEST && code == NET_ICMP_CODE_ECHO_REQUEST)
        {
            const uint16 receivedChecksum = NET_unpack16Bits(&packet[NET_BUFFER_ICMP_CHECKSUM_OFFSET]);
            const uint16 computedChecksum = _NET_icmpComputeChecksum(packet, packetSize);
            if (receivedChecksum == computedChecksum)
            {
                const uint16 identifier = NET_unpack16Bits(&packet[NET_BUFFER_PING_IDENTIFIER_OFFSET]);
                const uint16 sequenceNumber =
                    NET_unpack16Bits(&packet[NET_BUFFER_PING_SEQUENCE_NUMBER_OFFSET]);
                const uint16 truncatedDataSize =
                    MIN(MAX_ECHO_REPLY_LENGTH, (packetSize - NET_BUFFER_PING_DATA_OFFSET));
                _NET_transmitEchoReply(
                    senderIP,
                    identifier,
                    sequenceNumber,
                    &packet[NET_BUFFER_PING_DATA_OFFSET],
                    truncatedDataSize);
            }
            else
            {
                ilog_NET_COMPONENT_2(
                    ILOG_MINOR_EVENT, NET_PING_CHECKSUM_INVALID, receivedChecksum, computedChecksum);
            }
        }
    }
}


/**
* FUNCTION NAME: _NET_transmitEchoReply()
*
* @brief  - Sends a ping echo reply.
*
* @return - void.
*/
static void _NET_transmitEchoReply(
    NET_IPAddress destination,
    uint16 identifier,
    uint16 sequenceNumber,
    uint8* data,
    uint16 dataLength)
{
    uint8* echoReply = _NET_ipv4AllocateBuffer();

    echoReply[NET_BUFFER_ICMP_TYPE_OFFSET] = NET_ICMP_TYPE_ECHO_REPLY;
    echoReply[NET_BUFFER_ICMP_CODE_OFFSET] = NET_ICMP_CODE_ECHO_REPLY;
    NET_pack16Bits(identifier, &echoReply[NET_BUFFER_PING_IDENTIFIER_OFFSET]);
    NET_pack16Bits(sequenceNumber, &echoReply[NET_BUFFER_PING_SEQUENCE_NUMBER_OFFSET]);
    memcpy(&echoReply[NET_BUFFER_PING_DATA_OFFSET], data, dataLength);
    NET_pack16Bits(
        _NET_icmpComputeChecksum(echoReply, NET_BUFFER_PING_DATA_OFFSET + dataLength),
        &echoReply[NET_BUFFER_ICMP_CHECKSUM_OFFSET]);

    _NET_ipv4TransmitPacket(
        echoReply, (dataLength + NET_BUFFER_PING_DATA_OFFSET), destination, NET_IPV4_PROTOCOL_ICMP);
}


/**
* FUNCTION NAME: _NET_icmpComputeChecksum()
*
* @brief  - Computes the ICMP checksum over the header and data.
*
* @return - The checksum.
*/
static uint16 _NET_icmpComputeChecksum(const uint8* icmpPacket, uint16 packetSize)
{
    // TODO: the same checksum algorithm exists in IPV4.  Maybe we can make this code common?
    uint32 checksum = 0;

    uint16 i;
    for(i = NET_BUFFER_ICMP_BEGIN; i < packetSize; i += 2)
    {
        // Do not use the checksum bytes in computation of the checksum
        if(i != NET_BUFFER_ICMP_CHECKSUM_OFFSET)
        {
            if (i == packetSize - 1)
            {
                // If the packet size is odd, then we need to pad in a zero for the last byte so
                // that the checksum can be computed correctly.
                const uint8 data[2] = {icmpPacket[i], 0};
                checksum += NET_unpack16Bits(data);
            }
            else
            {
                checksum += NET_unpack16Bits(&icmpPacket[i]);
            }
        }
    }
    checksum = ~((checksum & 0x0000FFFF) + (checksum >> 16));

    return (uint16)checksum;
}

