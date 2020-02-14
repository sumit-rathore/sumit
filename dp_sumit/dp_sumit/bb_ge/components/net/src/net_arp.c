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
//!   @file  - net_arp.c
//
//!   @brief - Provides support for the address resolution protocol (ARP)
//             defined in RFC 826 (http://tools.ietf.org/html/rfc826).  ARP is
//             also described on Wikipedia
//             (http://en.wikipedia.org/wiki/Address_Resolution_Protocol).
//
//
//!   @note  - The current implementation is very simple.  A small unordered
//             table is used because it is expected that we will be
//             communicating with a small number of devices.  If there ends up
//             being a lot of churn in the ARP table then we may want to modify
//             it to allow for more entries.  If the table grows too large, the
//             lack of ordering in the table will become a problem.  At that
//             point it might make sense to implement the ARP table using a
//             hash table data structure.
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <net_arp.h>
#include "net_arp_loc.h"
#include "net_log.h"
#include <net_base.h>
#include <timing_timers.h>


/************************ Defined Constants and Macros ***********************/

//-----------------------------------------------------------------------------
// Offsets into the network buffer for the ARP layer
//-----------------------------------------------------------------------------
#define NET_BUFFER_ARP_BEGIN                                0
#define NET_BUFFER_ARP_HTYPE_OFFSET                         0
#define NET_BUFFER_ARP_PTYPE_OFFSET                         2
#define NET_BUFFER_ARP_HLEN_OFFSET                          4
#define NET_BUFFER_ARP_PLEN_OFFSET                          5
#define NET_BUFFER_ARP_OPER_OFFSET                          6
#define NET_BUFFER_ARP_SHA_OFFSET                           8
#define NET_BUFFER_ARP_SPA_OFFSET                           14
#define NET_BUFFER_ARP_THA_OFFSET                           18
#define NET_BUFFER_ARP_TPA_OFFSET                           24
#define NET_BUFFER_ARP_END                                  (NET_BUFFER_ARP_TPA_OFFSET + 4)

#define ARP_OPER_REQUEST    1
#define ARP_OPER_REPLY      2

#define ARP_HTYPE_ETHERNET  1
#define ARP_PTYPE_IPV4      0x0800

#define ARP_TABLE_SIZE 8

// The timer ticks every 10 seconds.  This means that a value of 3 would result
// in an ARP entry staying in the cache for a maximum of 30s and a minimum of
// 20s depending on when the entry was inserted relative to the previous timer
// tick:
// 0 to 10s:    3->2
// 10s:         2->1
// 10s:         1->0 (== no longer valid)
#define ARP_TABLE_FRESHNESS_COUNTDOWN_RESET 7
#define ARP_TABLE_FRESHNESS_TIMER_TICK 10000

// The number of times to retry an ARP request.
#define ARP_NUM_RETRIES 1

// The timeout of each ARP request in milliseconds
#define ARP_REQUEST_TIMEOUT 1000


/******************************** Data Types *********************************/

// NOTE: If the definition of this struct changes such that 0 is no longer a suitable default value
//       for all members, then _NET_arpTableInit() will have to be updated to use a different
//       initialization method.
struct NET_ARPEntry
{
    NET_MACAddress mac;
    NET_IPAddress ip;
    uint8 freshness;
};

struct NET_ARPTable
{
    struct NET_ARPEntry table[ARP_TABLE_SIZE];
    uint8 lastWrittenIndex;
};

struct NET_PendingARPRequest
{
    NET_IPAddress ip;
    NET_ARPLookupCallback cb;
    void* callbackData;
    boolT inUse;
    uint8 retryCount;
};


/***************************** Local Variables *******************************/
static struct NET_ARPTable arpTable;
static struct NET_PendingARPRequest pendingRequest;
static TIMING_TimerHandlerT arpRetryTimer;
static TIMING_TimerHandlerT arpFreshnessTimer;


/************************ Local Function Prototypes **************************/
static void _NET_arpTableInit(struct NET_ARPTable* at);
static void _NET_setARPTableEntry(
    struct NET_ARPTable* at,
    NET_IPAddress ip,
    NET_MACAddress mac) __attribute__((section(".ftext")));
static uint8 _NET_findARPStorage(
    struct NET_ARPTable* at) __attribute__((section(".ftext")));
static sint8 _NET_findArpIndex(
    struct NET_ARPTable* at,
    NET_IPAddress ip) __attribute__((section(".ftext")));
static void _NET_transmitARPPacket(
    uint16 operation,
    NET_MACAddress senderMAC,
    NET_IPAddress senderIP,
    NET_MACAddress targetMAC,
    NET_IPAddress targetIP) __attribute__((section(".ftext")));
static void _NET_arpRequestTimeoutHandler(void) __attribute__((section(".ftext")));
static void _NET_arpFreshnessTimerHandler(void) __attribute__((section(".ftext")));
static void _NET_arpClearPendingRequest(void) __attribute__((section(".ftext")));


/***************** External Visibility Function Definitions ******************/

/***************** Component Visibility Function Definitions *****************/

/**
* FUNCTION NAME: _NET_arpInitialize()
*
* @brief  - Initializes the ARP functionality.
*
* @return - void
*/
void _NET_arpInitialize(void)
{
    arpRetryTimer = TIMING_TimerRegisterHandler(
        &_NET_arpRequestTimeoutHandler, TRUE, ARP_REQUEST_TIMEOUT);

    arpFreshnessTimer = TIMING_TimerRegisterHandler(
        &_NET_arpFreshnessTimerHandler,
        TRUE,
        ARP_TABLE_FRESHNESS_TIMER_TICK);

    _NET_arpTableInit(&arpTable);
    _NET_arpClearPendingRequest();

    TIMING_TimerStart(arpFreshnessTimer);
}

/**
* FUNCTION NAME: _NET_arpOnLinkDown()
*
* @brief  - Takes action relating to the ARP protocol when it is found that the
*           link has gone down.
*
* @return - void
*/
void _NET_arpOnLinkDown(void)
{
    _NET_arpTableInit(&arpTable);
    _NET_arpClearPendingRequest();
}

/**
* FUNCTION NAME: _NET_arpOnLinkUp()
*
* @brief  - Takes action relating to the ARP protocol when it is found the link
*           has come up.
*
* @return - void
*/
void _NET_arpOnLinkUp(void)
{
}

/**
* FUNCTION NAME: _NET_arpTableLookup()
*
* @brief  - Performs a lookup in the ARP table for the MAC address associated with the given IP
*           address.  The callback function is called when the lookup completes.
*
* @return - void
*
* @note   - In the case where the MAC address for the given IP is already known by the ARP table,
*           this function will call the callback before returning.  If a lookup is required, the
*           callback will not be called until a later time when the ARP reply packet is received
*           that provides the MAC address of the desired IP.  If no ARP reply is received before the
*           timeout, the callback will be called passing "TRUE" as the value for the
*           arpLookupFailure parameter.  It is the responsibility of the callback to handle this
*           condition.  The callerData void pointer will be passed along to the callback so it is up
*           to the callback to properly cast that data and free it if required.
*/
void _NET_arpTableLookup(NET_IPAddress ip, NET_ARPLookupCallback callbackFunc, void* callerData)
{
    _NET_arpTableLookupDetailed(ip, callbackFunc, callerData, ARP_NUM_RETRIES, ARP_TABLE_FRESHNESS_TIMER_TICK);
}
void _NET_arpTableLookupDetailed
(
    NET_IPAddress ip,
    NET_ARPLookupCallback callbackFunc,
    void* callerData,
    uint8 retries,
    uint32 timeOutInMs
)
{
    sint8 idx = _NET_findArpIndex(&arpTable, ip);
    if(idx != -1)
    {
        (*callbackFunc)(FALSE, arpTable.table[idx].mac, ip, callerData);
    }
    else if(!pendingRequest.inUse)
    {
        pendingRequest.inUse = TRUE;
        TIMING_TimerResetTimeout(arpRetryTimer, timeOutInMs);
        TIMING_TimerStart(arpRetryTimer);
        pendingRequest.retryCount = retries;
        pendingRequest.ip = ip;
        pendingRequest.cb = callbackFunc;
        pendingRequest.callbackData = callerData;

        _NET_transmitARPPacket(
            ARP_OPER_REQUEST,
            NET_ethernetGetSelfMACAddress(),
            NET_ipv4GetIPAddress(),
            0, // target MAC is zero because this is what we are requesting
            ip);
    }
    else
    {
        // The MAC for the given IP is not known and there is already a pending
        // ARP request, so we set the arpLookupFailureParameter in the callback
        // to TRUE.
        (*callbackFunc)(TRUE, 0, ip, callerData);
    }
}

/**
* FUNCTION NAME: _NET_arpReceivePacketHandler()
*
* @brief  - Handles a received ARP packet by updating the ARP table and executing the callback
*           function associated with a previous ARP lookup if applicable.
*
* @return - void
*
* @note   -
*/
void _NET_arpReceivePacketHandler(uint8* packet, uint16 packetSize)
{
    uint16 operation;
    NET_MACAddress senderMAC;
    NET_IPAddress senderIP;
    NET_IPAddress targetIP;
    uint8 hwAddrLen;
    uint8 protoAddrLen;

    // Make sure that we received enough data for an ARP packet.  We may have
    // received some padding data as well due to minimum octet count
    // restrictions at the ethernet layer.
    if(packetSize < (NET_BUFFER_ARP_END - NET_BUFFER_ARP_BEGIN))
    {
        ilog_NET_COMPONENT_2(
            ILOG_MINOR_EVENT,
            NET_ARP_UNEXPECTED_PACKET_SIZE,
            packetSize,
            (NET_BUFFER_ARP_END - NET_BUFFER_ARP_BEGIN));
        return;
    }

    // We are expecting 6-byte MAC addresses and 4-byte IPv4 addresses.  Ignore the packet if this
    // is not the case.
    hwAddrLen = packet[NET_BUFFER_ARP_HLEN_OFFSET];
    protoAddrLen = packet[NET_BUFFER_ARP_PLEN_OFFSET];
    if(hwAddrLen != 6 && protoAddrLen != 4)
        return;

    operation = NET_unpack16Bits(&packet[NET_BUFFER_ARP_OPER_OFFSET]);
    if(operation != ARP_OPER_REQUEST && operation != ARP_OPER_REPLY)
    {
        ilog_NET_COMPONENT_1(
            ILOG_WARNING,
            NET_ARP_INVALID_OPERATION,
            operation);
        return; // Invalid operation
    }

    senderMAC = NET_unpack48Bits(&packet[NET_BUFFER_ARP_SHA_OFFSET]);
    senderIP = NET_unpack32Bits(&packet[NET_BUFFER_ARP_SPA_OFFSET]);

    // We don't care if this was a REQUEST or REPLY packet because in either case it contains the
    // most recent IP -> MAC mapping.
    _NET_setARPTableEntry(&arpTable, senderIP, senderMAC);

    targetIP = NET_unpack32Bits(&packet[NET_BUFFER_ARP_TPA_OFFSET]);
    if(operation == ARP_OPER_REQUEST &&
       NET_ipv4IsNetworkConfigured() &&
       targetIP == NET_ipv4GetIPAddress())
    {
        _NET_transmitARPPacket(
            ARP_OPER_REPLY,
            NET_ethernetGetSelfMACAddress(),
            NET_ipv4GetIPAddress(),
            senderMAC,
            senderIP);
    }

    // Check if there is TX buffer that can now be sent because the ARP table has been updated.
    if(pendingRequest.inUse && senderIP == pendingRequest.ip)
    {
        void* callbackData = pendingRequest.callbackData;
        _NET_arpClearPendingRequest();
        (*pendingRequest.cb)(FALSE, senderMAC, senderIP, callbackData);
    }
}


/******************** File Visibility Function Definitions *******************/

/**
* FUNCTION NAME: _NET_arpTableInit()
*
* @brief  - Initializes the ARP table supplied as a parameter.
*
* @return - void
*/
static void _NET_arpTableInit(struct NET_ARPTable* at)
{
    at->lastWrittenIndex = ARP_TABLE_SIZE - 1;
    memset(at->table, 0, sizeof(at->table));
}


/**
* FUNCTION NAME: _NET_setARPTableEntry()
*
* @brief  - Associates the provided IP address with the provided MAC address.  If there was a
*           previous association with the given IP address, it will be overwritten.
*
* @return - void
*
* @note   -
*
*/
static void _NET_setARPTableEntry(struct NET_ARPTable* at, NET_IPAddress ip, NET_MACAddress mac)
{
    uint8 writeToIndex;
    sint8 matchingIndex = _NET_findArpIndex(at, ip);
    if(matchingIndex != -1)
    {
        writeToIndex = matchingIndex;
    }
    else
    {
        writeToIndex = _NET_findARPStorage(at);
    }
    at->table[writeToIndex].ip = ip;
    at->table[writeToIndex].mac = mac;
    at->table[writeToIndex].freshness = ARP_TABLE_FRESHNESS_COUNTDOWN_RESET;
    at->lastWrittenIndex = writeToIndex;
}

/**
* FUNCTION NAME: _NET_findARPStorage()
*
* @brief  - Returns the index in the ARP table where the next table entry write
*           should occur.
*
* @return - void
*
* @note   - It is the responsibility of the client to set the lastWrittenIndex
*           variable in the ARP table after performing the write.  The entry
*           returned will be the entry with the lowest freshness.  In the case
*           of a tie, it will prefer the entry that follows lastWrittenEntry
*           most closely.
*/
static uint8 _NET_findARPStorage(struct NET_ARPTable* at)
{
    uint8 i =
        at->lastWrittenIndex == (ARP_TABLE_SIZE - 1) ? 0 : (at->lastWrittenIndex + 1);
    uint8 stalestIndex = i;
    uint8 stalestVal  = ARP_TABLE_FRESHNESS_COUNTDOWN_RESET;

    while(i != at->lastWrittenIndex)
    {
        if(at->table[i].freshness < stalestVal)
        {
            stalestVal = at->table[i].freshness;
            stalestIndex = i;
            if(stalestVal == 0)
            {
                break;
            }
        }
        i = (i == ARP_TABLE_SIZE - 1) ? 0 : i + 1;
    }
    return stalestIndex;
}

/**
* FUNCTION NAME: _NET_findARPIndex()
*
* @brief  - Initializes the ARP functionality.
*
* @return - The index in the ARP table associated with the given IP address or -1 when no entry for
*           that IP address was found.
*
* @note   -
*/
static sint8 _NET_findArpIndex(struct NET_ARPTable* at, NET_IPAddress ip)
{
    uint8 i;
    for(i = 0; i < ARP_TABLE_SIZE; i++)
    {
        if(at->table[i].freshness > 0 && at->table[i].ip == ip)
            return i;
    }
    return -1;
}

/**
* FUNCTION NAME: _NET_transmitARPPacket()
*
* @brief  - Convenience function for building and sending an ARP packet.
*
* @return - None
*/
static void _NET_transmitARPPacket(
    uint16 operation,
    NET_MACAddress senderMAC,
    NET_IPAddress senderIP,
    NET_MACAddress targetMAC,
    NET_IPAddress targetIP)
{
    uint16 arpDataSize;
    uint8* arpPacket = _NET_ethernetAllocateBuffer();
    NET_MACAddress sendTo = operation == ARP_OPER_REQUEST ?
        NET_ETHERNET_BROADCAST_MAC_ADDRESS : targetMAC;

    NET_pack16Bits(
        ARP_HTYPE_ETHERNET,
        &arpPacket[NET_BUFFER_ARP_HTYPE_OFFSET]);
    NET_pack16Bits(
        ARP_PTYPE_IPV4,
        &arpPacket[NET_BUFFER_ARP_PTYPE_OFFSET]);
    arpPacket[NET_BUFFER_ARP_HLEN_OFFSET] = 6;
    arpPacket[NET_BUFFER_ARP_PLEN_OFFSET] = 4;
    NET_pack16Bits(
        operation,
        &arpPacket[NET_BUFFER_ARP_OPER_OFFSET]);
    NET_pack48Bits(senderMAC, &arpPacket[NET_BUFFER_ARP_SHA_OFFSET]);
    NET_pack32Bits(senderIP, &arpPacket[NET_BUFFER_ARP_SPA_OFFSET]);
    NET_pack48Bits(targetMAC, &arpPacket[NET_BUFFER_ARP_THA_OFFSET]);
    NET_pack32Bits(targetIP, &arpPacket[NET_BUFFER_ARP_TPA_OFFSET]);

    arpDataSize = (NET_BUFFER_ARP_END - NET_BUFFER_ARP_BEGIN);

    _NET_ethernetTransmitFrame(arpPacket, arpDataSize, sendTo, NET_ETHERTYPE_ARP);
}

/**
* FUNCTION NAME: _NET_arpRequestTimeoutHandler()
*
* @brief  - Timer callback function that indicates that we believe that our
*           previous ARP request will not be responded to.  We either retry the
*           ARP request or call the ARP callback indicating that this lookup
*           failed.
*
* @return - None
*/
static void _NET_arpRequestTimeoutHandler(void)
{
    // This timer should never fire if there is not a pending request
    iassert_NET_COMPONENT_0(pendingRequest.inUse, NET_ARP_UNEXPECTED_REQUEST_TIMEOUT);
    if(pendingRequest.retryCount != 0)
    {
        pendingRequest.retryCount--;
        // re-send
        _NET_transmitARPPacket(
            ARP_OPER_REQUEST,
            NET_ethernetGetSelfMACAddress(),
            NET_ipv4GetIPAddress(),
            0, // target MAC is zero because this is what we are requesting
            pendingRequest.ip);
    }
    else
    {
        void* callbackData = pendingRequest.callbackData;
        NET_IPAddress ip = pendingRequest.ip;
        _NET_arpClearPendingRequest();
        (*pendingRequest.cb)(TRUE, 0, ip, callbackData);
    }
}

/**
* FUNCTION NAME: _NET_arpFreshnessTimerHandler()
*
* @brief  - Timer callback function that decrements the "freshness" of every
*           value in the ARP table.
*
* @return - None
*/
static void _NET_arpFreshnessTimerHandler(void)
{
    uint8 i;
    for(i = 0; i < ARP_TABLE_SIZE; i++)
    {
        if(arpTable.table[i].freshness)
        {
            arpTable.table[i].freshness--;
        }
    }
}

/**
* FUNCTION NAME: _NET_arpClearPendingRequest()
*
* @brief  - Clears the pending request and stops the timeout timer.
*
* @return - None
*/
static void _NET_arpClearPendingRequest(void)
{
    pendingRequest.inUse = FALSE;
    TIMING_TimerStop(arpRetryTimer);
}
