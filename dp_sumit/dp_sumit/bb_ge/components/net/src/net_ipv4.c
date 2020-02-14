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
//!   @file  - net_ipv4.c
//
//!   @brief -
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <net_ipv4.h>
#include "net_ipv4_loc.h"
#include <net_ethernet.h>
#include "net_arp_loc.h"
#include "net_icmp_loc.h"
#include "net_udp_loc.h"
#include "net_log.h"
#include <net_base.h>
#include <storage_vars.h>
#include <storage_Data.h>


/************************ Defined Constants and Macros ***********************/

//-----------------------------------------------------------------------------
// Offsets into the network buffer for the IPv4 layer
//-----------------------------------------------------------------------------
#define NET_BUFFER_IPV4_BEGIN                               0
#define NET_BUFFER_IPV4_VERSION_AND_HEADER_LENGTH_OFFSET    0
#define NET_BUFFER_IPV4_DSCP_AND_ECN_OFFSET                 1
#define NET_BUFFER_IPV4_TOTAL_LENGTH_OFFSET                 2
#define NET_BUFFER_IPV4_IDENTIFICATION_OFFSET               4
#define NET_BUFFER_IPV4_FLAGS_AND_FRAGMENT_OFFSET_OFFSET    6
#define NET_BUFFER_IPV4_TTL_OFFSET                          8
#define NET_BUFFER_IPV4_PROTOCOL_OFFSET                     9
#define NET_BUFFER_IPV4_HEADER_CHECKSUM_OFFSET              10
#define NET_BUFFER_IPV4_SRC_IP_ADDR_OFFSET                  12
#define NET_BUFFER_IPV4_DEST_IP_ADDR_OFFSET                 16
#define NET_BUFFER_IPV4_PAYLOAD_OFFSET                      20

#define NET_IPV4_VERSION_NUMBER                     0x04
#define NET_IPV4_HEADER_LENGTH                      0x05
#define NET_IPV4_DIFFERENTIATED_SERVICES            0x00
#define NET_IPV4_EXPLICIT_CONGESTION_NOTIFICATION   0x00
#define NET_IPV4_FLAGS                              0x02
#define NET_IPV4_TTL                                0x20

#define NET_IPV4_UNINITIALIZED_IP_ADDRESS           0x00000000

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static uint16 identificationCounter;
static NET_MACAddress _NET_gatewayMAC;



/************************ Local Function Prototypes **************************/
static uint16 _NET_computeIPv4HeaderChecksum(
    const uint8* buffer) __attribute__((section(".ftext")));
static void _NET_onMACAddrResolvedCallback(
    boolT arpLookupFailure,
    const NET_MACAddress mac,
    const NET_IPAddress ip,
    void* netBuffer) __attribute__((section(".ftext")));
static inline boolT _NET_ipv4IsBroadcastAddress(NET_IPAddress ip);
static NET_IPAddress _NET_ipv4GetLocalBroadcastAddress(void) __attribute__((section(".ftext")));
static inline boolT _NET_ipv4IsIPInSubnet(NET_IPAddress ip, NET_IPAddress subnet);
static inline uint8* _NET_ipv4FullFromPayload(uint8* payload);
static inline uint8* _NET_ipv4PayloadFromFull(uint8* frame);


/***************** External Visibility Function Definitions ******************/

/**
* FUNCTION NAME: NET_ipv4IsNetworkConfigured()
*
* @brief  - Checks if the network is configured
*
* @return - Returns true, if the IP and subnet mask are both non-zero
*
* @note   - This doesn't check gateway, as there is no need to have a gateway configured
*/
boolT NET_ipv4IsNetworkConfigured(void)
{
    return (    (STORAGE_varGet(IP_ADDR)->words[0] != NET_IPV4_UNINITIALIZED_IP_ADDRESS)
            &&  (STORAGE_varGet(SUBNET_MASK)->words[0] != NET_IPV4_UNINITIALIZED_IP_ADDRESS));
}


/***************** Component Visibility Function Definitions *****************/

/**
* FUNCTION NAME: _NET_ipv4Initialize()
*
* @brief  - Initializes the ipv4 functionality.
*
* @return - void
*/
void _NET_ipv4Initialize(void)
{
    // Create the IP, subnet mask and default gateway persistent variables if they do not exist
    if(!STORAGE_varExists(IP_ADDR))
    {
        STORAGE_varCreate(IP_ADDR);
        STORAGE_varSave(IP_ADDR);
    }
    if(!STORAGE_varExists(SUBNET_MASK))
    {
        STORAGE_varCreate(SUBNET_MASK);
        STORAGE_varSave(SUBNET_MASK);
    }
    if(!STORAGE_varExists(DEFAULT_GATEWAY))
    {
        STORAGE_varCreate(DEFAULT_GATEWAY);
        STORAGE_varSave(DEFAULT_GATEWAY);
    }

    _NET_udpInitialize();
}

/**
* FUNCTION NAME: NET_ipv4GetIPAddress()
*
* @brief  - Gets the IP address of this device.
*
* @return - The IP address of this device.
*/
NET_IPAddress NET_ipv4GetIPAddress(void)
{
    return STORAGE_varGet(IP_ADDR)->words[0];
}

/**
* FUNCTION NAME: NET_ipv4SetIPAddress()
*
* @brief  - Sets the IP address and stores it in flash.
*
* @return - None.
*/
void NET_ipv4SetIPAddress(NET_IPAddress ip)
{
    STORAGE_varGet(IP_ADDR)->words[0] = ip;
    STORAGE_varSave(IP_ADDR);
}

/**
* FUNCTION NAME: NET_ipv4GetSubnetMask()
*
* @brief  - Gets the subnet mask.
*
* @return - The subnet mask address.
*/
NET_IPAddress NET_ipv4GetSubnetMask(void)
{
    return STORAGE_varGet(SUBNET_MASK)->words[0];
}

/**
* FUNCTION NAME: NET_ipv4SetSubnetMask()
*
* @brief  - Sets the subnet mask and stores it in flash.
*
* @return - None.
*/
void NET_ipv4SetSubnetMask(NET_IPAddress subnetMask)
{
    STORAGE_varGet(SUBNET_MASK)->words[0] = subnetMask;
    STORAGE_varSave(SUBNET_MASK);
}

/**
* FUNCTION NAME: NET_ipv4GetDefaultGateway()
*
* @brief  - Gets the default gateway address.
*
* @return - The default gateway address.
*/
NET_IPAddress NET_ipv4GetDefaultGateway(void)
{
    return STORAGE_varGet(DEFAULT_GATEWAY)->words[0];
}

/**
* FUNCTION NAME: NET_ipv4SetDefaultGateway()
*
* @brief  - Sets the default gateway and stores it in flash.
*
* @return - None.
*/
void NET_ipv4SetDefaultGateway(NET_IPAddress defaultGateway)
{
    STORAGE_varGet(DEFAULT_GATEWAY)->words[0] = defaultGateway;
    STORAGE_varSave(DEFAULT_GATEWAY);
}

/**
* FUNCTION NAME: NET_ipv4GetDhcpServer()
*
* @brief  - Gets the DHCP server.
*
* @return - The DHCP Server.
*/
NET_IPAddress NET_ipv4GetDhcpServer(void)
{
    return STORAGE_varGet(DHCP_SERVER_IP)->words[0];
}

/**
* FUNCTION NAME: _NET_ipv4OnLinkDown()
*
* @brief  - Takes action relating to the ipv4 protocol when it is found that the
*           link has gone down.
*
* @return - void
*/
void _NET_ipv4OnLinkDown(void)
{
    _NET_udpOnLinkDown();
}

/**
* FUNCTION NAME: _NET_ipv4OnLinkUp()
*
* @brief  - Takes action relating to the ipv4 protocol when it is found the link
*           has come up.
*
* @return - void
*/
void _NET_ipv4OnLinkUp(void)
{
    _NET_udpOnLinkUp();
}

/**
* FUNCTION NAME: _NET_ipv4ReceivePacketHandler()
*
* @brief  - Handles a received IP packet.
*
* @return - void
*/
void _NET_ipv4ReceivePacketHandler(uint8* packet, uint16 packetSize)
{
    uint16 checksum = NET_unpack16Bits(
        &packet[NET_BUFFER_IPV4_HEADER_CHECKSUM_OFFSET]);
    uint16 computedChecksum = _NET_computeIPv4HeaderChecksum(packet);
    if(checksum != computedChecksum)
    {
        ilog_NET_COMPONENT_2(
            ILOG_MINOR_EVENT,
            NET_IPV4_HEADER_CHECKSUM_RX_MISMATCH,
            checksum,
            computedChecksum);
    }
    else
    {
        // In the broadcast only netcfg variant, we want to accept all IP
        // packets because we won't have a properly configured IP address.
        NET_IPAddress destIP = NET_unpack32Bits(
                &packet[NET_BUFFER_IPV4_DEST_IP_ADDR_OFFSET]);

        if (((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >>
              TOPLEVEL_USE_BCAST_NET_CFG_PROTO_OFFSET) & 0x1) ||
            destIP == NET_ipv4GetIPAddress() ||
            _NET_ipv4IsBroadcastAddress(destIP))
        {
            uint16 flagsPlusFragmentOffset = NET_unpack16Bits(
                &packet[NET_BUFFER_IPV4_FLAGS_AND_FRAGMENT_OFFSET_OFFSET]);
            // More fragments is bit 2 of the flags field
            boolT moreFragments = (flagsPlusFragmentOffset & (1 << 13)) != 0;
            // The fragment offset field is 13 bits wide
            uint16 fragmentOffset = (flagsPlusFragmentOffset & 0x1FFF);
            if(moreFragments || fragmentOffset != 0)
            {
                ilog_NET_COMPONENT_0(
                    ILOG_MINOR_EVENT, NET_IPV4_RECEIVED_FRAGMENTED_PACKET);
            }
            else
            {
                const uint16 ipv4TotalLength = NET_unpack16Bits(
                    &packet[NET_BUFFER_IPV4_TOTAL_LENGTH_OFFSET]);
                if(ipv4TotalLength > packetSize)
                {
                    // The IPv4 packet says it is larger than the amount of
                    // data we have in our buffer.
                    ilog_NET_COMPONENT_2(
                        ILOG_MINOR_EVENT,
                        NET_IPV4_PACKET_LARGER_THAN_DATA,
                        ipv4TotalLength,
                        packetSize);
                }
                else
                {
                    const uint8 protocol =
                        packet[NET_BUFFER_IPV4_PROTOCOL_OFFSET];
                    uint8* ipv4Payload = _NET_ipv4PayloadFromFull(packet);
                    uint16 payloadSize = ipv4TotalLength - (ipv4Payload - packet);
                    NET_IPAddress senderIP = NET_unpack32Bits(
                        &packet[NET_BUFFER_IPV4_SRC_IP_ADDR_OFFSET]);
                    switch(protocol)
                    {
                        case NET_IPV4_PROTOCOL_UDP:
                        {
                            _NET_udpReceivePacketHandler(ipv4Payload, payloadSize, senderIP);
                            break;
                        }
                        case NET_IPV4_PROTOCOL_ICMP:
                        {
                            _NET_icmpReceivePacketHandler(ipv4Payload, payloadSize, senderIP);
                            break;
                        }
                        default:
                        {
                            ilog_NET_COMPONENT_3(
                                ILOG_MINOR_EVENT,
                                NET_IPV4_UNHANDLED_PROTOCOL,
                                protocol,
                                senderIP,
                                destIP);
                            break;
                        }
                    }
                }
            }
        }
        // possible to rx any packet when connected with a hub, so don't log
        // else condition as unexpected
    }
}

/**
* FUNCTION NAME: _NET_ipv4TransmitPacket()
*
* @brief  - Fills in the header information of the IP packet and then sends it.
*
* @return - void
*/
void _NET_ipv4TransmitPacket(
    uint8* payload, uint16 payloadSize, NET_IPAddress dest, uint8 protocol)
{
    boolT isDestBroadcast = _NET_ipv4IsBroadcastAddress(dest);
    uint8* packet = _NET_ipv4FullFromPayload(payload);

    // version + header length
    packet[NET_BUFFER_IPV4_VERSION_AND_HEADER_LENGTH_OFFSET] =
        ((NET_IPV4_VERSION_NUMBER << 4) | NET_IPV4_HEADER_LENGTH);

    // differentiated services code point + explicit congestion notification
    packet[NET_BUFFER_IPV4_DSCP_AND_ECN_OFFSET] =
        ((NET_IPV4_DIFFERENTIATED_SERVICES << 2) |
         NET_IPV4_EXPLICIT_CONGESTION_NOTIFICATION);

    // total length
    NET_pack16Bits(
        (NET_BUFFER_IPV4_PAYLOAD_OFFSET - NET_BUFFER_IPV4_BEGIN) + payloadSize,
        &packet[NET_BUFFER_IPV4_TOTAL_LENGTH_OFFSET]);

    // identification - Increment a global 16 bit counter after each packet sent
    NET_pack16Bits(
        identificationCounter++,
        &packet[NET_BUFFER_IPV4_IDENTIFICATION_OFFSET]);

    // flags + (fragment offset - Always 0 because we don't fragment)
    NET_pack16Bits(
        ((NET_IPV4_FLAGS << 13) | 0),
        &packet[NET_BUFFER_IPV4_FLAGS_AND_FRAGMENT_OFFSET_OFFSET]);

    // time to live
    packet[NET_BUFFER_IPV4_TTL_OFFSET] = NET_IPV4_TTL;

    // protocol
    packet[NET_BUFFER_IPV4_PROTOCOL_OFFSET] = protocol;

    // source ip address
    NET_pack32Bits(
        NET_ipv4GetIPAddress(),
        &packet[NET_BUFFER_IPV4_SRC_IP_ADDR_OFFSET]);

    // destination ip address
    NET_pack32Bits(
        dest,
        &packet[NET_BUFFER_IPV4_DEST_IP_ADDR_OFFSET]);

    // header checksum
    NET_pack16Bits(
        _NET_computeIPv4HeaderChecksum(packet),
        &packet[NET_BUFFER_IPV4_HEADER_CHECKSUM_OFFSET]);

    // Prevent ARP table querying when the destination IP address is a
    // broadcast address.  If the IP is a broadcast address, then the
    // MAC address should be the broadcast MAC.
    if(!isDestBroadcast)
    {
        NET_IPAddress destOrGatewayIPAddr = (
            _NET_ipv4IsIPInSubnet(dest, (NET_ipv4GetIPAddress() & NET_ipv4GetSubnetMask()))
            ? dest : NET_ipv4GetDefaultGateway());

        // Find Destination MAC address using either the destination IP
        // or the default gateway's IP
        _NET_arpTableLookup(
                destOrGatewayIPAddr, &_NET_onMACAddrResolvedCallback, packet);
    }
    else
    {
        _NET_onMACAddrResolvedCallback(
                FALSE, NET_ETHERNET_BROADCAST_MAC_ADDRESS, dest, packet);
    }
}

/**
* FUNCTION NAME: _NET_ipv4AllocateBuffer()
*
* @brief  - Allocates a buffer and returns a pointer to the payload of the
*           ipv4 frame.
*
* @return - A pointer to the ipv4 payload of a newly allocated buffer.
*
* @note   - Lower level functions will assert if the memory allocation fails.
*/
uint8* _NET_ipv4AllocateBuffer(void)
{
    return _NET_ipv4PayloadFromFull(_NET_ethernetAllocateBuffer());
}

/**
* FUNCTION NAME: _NET_ipv4FreeBuffer()
*
* @brief  - Frees the buffer associated with the pointer to an ipv4 payload
*           that is passed as a parameter.
*
* @return - None
*/
void _NET_ipv4FreeBuffer(uint8* buffer)
{
    _NET_ethernetFreeBuffer(_NET_ipv4FullFromPayload(buffer));
}


/**
* FUNCTION NAME: _NET_ipv4ResetNetworkParameters()
*
* @brief  - Resets the IP address, default gateway and subnet mask to all
*           zeros.
*
* @return - None
*/
void _NET_ipv4ResetNetworkParameters(void)
{
    NET_ipv4SetIPAddress(NET_IPV4_UNINITIALIZED_IP_ADDRESS);
    NET_ipv4SetSubnetMask(NET_IPV4_UNINITIALIZED_IP_ADDRESS);
    NET_ipv4SetDefaultGateway(NET_IPV4_UNINITIALIZED_IP_ADDRESS);
}

// returns 0 on no available data
NET_MACAddress _NET_getLastGatewayMAC(void)
{
    return _NET_gatewayMAC;
}

void _NET_ipv4DoArpRequestOfGateway(void)
{
    _NET_arpTableLookup(
            NET_ipv4GetDefaultGateway(), &_NET_onMACAddrResolvedCallback, NULL);
}

/******************** File Visibility Function Definitions *******************/

/**
* FUNCTION NAME: _NET_computeIPv4HeaderChecksum()
*
* @brief  - Computes the IPv4 header checksum according to the algorithm
*           defined in http://tools.ietf.org/html/rfc1071.
*
* @return - The checksum value
*
* @note   - The computed checksum is returned and not written into the network
*           buffer.
*/
static uint16 _NET_computeIPv4HeaderChecksum(const uint8* buffer)
{
    uint32 checksum = 0;

    uint8 i;
    for(i = NET_BUFFER_IPV4_BEGIN; i < NET_BUFFER_IPV4_PAYLOAD_OFFSET; i += 2)
    {
        // Do not use the checksum bytes in computation of the checksum
        if(i != NET_BUFFER_IPV4_HEADER_CHECKSUM_OFFSET)
        {
            checksum += NET_unpack16Bits(&buffer[i]);
        }
    }
    checksum = ~((checksum & 0x0000FFFF) + (checksum >> 16));

    return (uint16)checksum;
}

/**
* FUNCTION NAME: _NET_onMACAddrResolvedCallback()
*
* @brief  - Transmits the pending IP packet to the ethernet layer once the MAC
*           address of the destination is resolved.
*
* @return - None
*/
static void _NET_onMACAddrResolvedCallback(
    boolT arpLookupFailure,
    const NET_MACAddress mac,
    const NET_IPAddress ip,
    void* netBuffer)
{
    uint8* ipv4Packet = netBuffer;
    if(!arpLookupFailure)
    {
        // Success
        if (NET_ipv4GetDefaultGateway() == ip)
        {
            // update the last Gateway MAC address.
            _NET_gatewayMAC = mac;
        }

        if (ipv4Packet != NULL) // Otherwise this was an ARP request to update gateway
        {
            const uint16 ethernetPayloadSize = NET_unpack16Bits(
                    &ipv4Packet[NET_BUFFER_IPV4_TOTAL_LENGTH_OFFSET]);

            _NET_ethernetTransmitFrame(
                    ipv4Packet, ethernetPayloadSize, mac, NET_ETHERTYPE_IPV4);
        }
    }
    else
    {
        ilog_NET_COMPONENT_0(ILOG_MINOR_EVENT, NET_IPV4_ARP_LOOKUP_FAILURE);
        if (ipv4Packet != NULL) // Otherwise this was an ARP request to update gateway
        {
            _NET_ethernetFreeBuffer(ipv4Packet);
        }
    }
}

/**
* FUNCTION NAME: _NET_ipv4IsBroadcastAddress()
*
* @brief  - Returns TRUE if the provided address is the broadcast address for
*           the current network.
*
* @return - None
*/
static inline boolT _NET_ipv4IsBroadcastAddress(NET_IPAddress ip)
{
    return (ip == NET_IPV4_LIMITED_BROADCAST_ADDRESS) || (ip == _NET_ipv4GetLocalBroadcastAddress());
}

/**
* FUNCTION NAME: _NET_ipv4GetLocalBroadcastAddress()
*
* @brief  - Gives the local broadcast address.  That means the address with all
*           bits of the subnet mask matching the local IP address and all bits
*           outside of the subnet mask set to 1.
*
* @return - None
*/
static NET_IPAddress _NET_ipv4GetLocalBroadcastAddress(void)
{
    return (~NET_ipv4GetSubnetMask() | NET_ipv4GetIPAddress());
}


/**
* FUNCTION NAME: _NET_ipv4IsIPInSubnet()
*
* @brief  - Tells if and IP address such as 192.168.1.100 is in a subnet such
*           as 192.168.1.0.
*
* @return - TRUE if ip is a member of subnet.
*/
static inline boolT _NET_ipv4IsIPInSubnet(NET_IPAddress ip, NET_IPAddress subnet)
{
    return (ip | subnet) == ip;
}

/**
* FUNCTION NAME: _NET_ipv4FullFromPayload()
*
* @brief  - Gives a pointer to a full ipv4 frame based on a pointer to the ipv4
*           payload.
*
* @return - A pointer to the whole ipv4 frame.
*/
static inline uint8* _NET_ipv4FullFromPayload(uint8* payload)
{
    return payload - NET_BUFFER_IPV4_PAYLOAD_OFFSET;
}

/**
* FUNCTION NAME: _NET_ipv4PayloadFromFull()
*
* @brief  - Gives a pointer to the ipv4 payload based given an pointer to the
*           entire ipv4 frame.
*
* @return - A pointer to the ipv4 paylod.
*/
static inline uint8* _NET_ipv4PayloadFromFull(uint8* packet)
{
    return packet + NET_BUFFER_IPV4_PAYLOAD_OFFSET;
}
