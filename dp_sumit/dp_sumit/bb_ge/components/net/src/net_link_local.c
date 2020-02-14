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
//!   @file  - net_link_local.c
//
//!   @brief - Implementation of RFC 3927 IPv4 Link Local Addresses 169.254/16
//
//!   @note  - Used by DHCP client, when no DHCP server can be found
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "net_link_local_loc.h"
#include "net_arp_loc.h"
#include <net_ipv4.h>
#include <timing_timers.h>
#include <random.h>

#include "net_log.h"

/************************ Defined Constants and Macros ***********************/

#define __NET_LINK_LOCAL_ARP_PROBE_NUM  3 // taken from RFC 3927
#define __NET_LINK_LOCAL_BASE_ADDR      (NET_IPAddress)((169 << 24) | (254 << 16))
#define __NET_LINK_LOCAL_NETMASK        (NET_IPAddress)((255 << 24) | (255 << 16)) 

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static struct {
    TIMING_TimerHandlerT timer;
    NET_IPAddress testIpv4Addr;
    uint8 arpProbesSent;
    boolT enabled; // The state of this C file
} __NET_linkLocalState;

/************************ Local Function Prototypes **************************/
static void __NET_linkLocalStart(void) __attribute__((section(".ftext")));
static void __NET_linkLocalDoArpCheck(void) __attribute__((section(".ftext")));
static void __NET_linkLocalArpCallback(boolT arpLookupFailure, const NET_MACAddress mac,
    const NET_IPAddress ip, void* callbackData) __attribute__((section(".ftext")));

/***************** External Visibility Function Definitions ******************/

/***************** Component Visibility Function Definitions *****************/
void _NET_linkLocalInit(void)
{
    __NET_linkLocalState.timer = TIMING_TimerRegisterHandler(
         &__NET_linkLocalStart,
         FALSE, // non-periodic
         1 // dummy value, as the timeout gets re-initialized every time
        );
}


void _NET_linkLocalEnable(void)
{
    NET_IPAddress curAddr;

    if (__NET_linkLocalState.enabled)
    {
        return;
    }

    ilog_NET_COMPONENT_0(ILOG_MAJOR_EVENT, LINK_LOCAL_ENABLE);

    curAddr = NET_ipv4GetIPAddress();
    iassert_NET_COMPONENT_1(curAddr == 0, LINK_LOCAL_ENABLED_WITH_VALID_IP, curAddr);

    __NET_linkLocalState.enabled = TRUE;
    __NET_linkLocalState.testIpv4Addr = 0;
    __NET_linkLocalState.arpProbesSent = 0;

    // From RFC 3927 the first ARP probe should start with a random delay of between 0 and 1 second
    // This will will do 0 to 1.024 seconds.
    TIMING_TimerResetTimeout(__NET_linkLocalState.timer, RANDOM_QuickPseudoRandom32() & 0x3FF);
    TIMING_TimerStart(__NET_linkLocalState.timer);
}


void _NET_linkLocalDisable(void)
{
    if (!__NET_linkLocalState.enabled)
    {
        return;
    }

    ilog_NET_COMPONENT_0(ILOG_MAJOR_EVENT, LINK_LOCAL_DISABLE);

    TIMING_TimerStop(__NET_linkLocalState.timer);
    __NET_linkLocalState.enabled = FALSE;

    // If the local adddresses are still being used, remove them, they should never persist
    if ((NET_ipv4GetIPAddress() & __NET_LINK_LOCAL_NETMASK) == __NET_LINK_LOCAL_BASE_ADDR)
    {
        NET_ipv4SetIPAddress(0);
        NET_ipv4SetSubnetMask(0);
        NET_ipv4SetDefaultGateway(0);
    }
    else
    {
        // It is possible that the addresses were updated, by the caller before informing net_link_local.c
    }
}


/******************** File Visibility Function Definitions *******************/

static void __NET_linkLocalStart(void)
{
    if (__NET_linkLocalState.enabled)
    {
        __NET_linkLocalDoArpCheck();
    }
}

static void __NET_linkLocalArpCallback
(
    boolT arpLookupFailure,
    const NET_MACAddress mac,
    const NET_IPAddress ip,
    void* callbackData
)
{
    if (!__NET_linkLocalState.enabled)
    {
        return;
    }

    __NET_linkLocalState.arpProbesSent++;

    if (!arpLookupFailure)
    {
        // Conflict, clear the address and probe count
        __NET_linkLocalState.testIpv4Addr = 0;
        __NET_linkLocalState.arpProbesSent = 0;
    }


    if (__NET_linkLocalState.arpProbesSent == __NET_LINK_LOCAL_ARP_PROBE_NUM)
    {
        // Success, we can use this address
        ilog_NET_COMPONENT_1(ILOG_MAJOR_EVENT, LINK_LOCAL_NEW_ADDR, __NET_linkLocalState.testIpv4Addr);
        NET_ipv4SetIPAddress(__NET_linkLocalState.testIpv4Addr);
        NET_ipv4SetSubnetMask(__NET_LINK_LOCAL_NETMASK);
        NET_ipv4SetDefaultGateway(0); // no gateway, this is a local IP
    }
    else
    {
        // keep doing ARP checks
        __NET_linkLocalDoArpCheck();
    }
}


static void __NET_linkLocalDoArpCheck(void)
{
    uint32 timeOutInMs;

    // Do we need to test a new IP address?
    if (!__NET_linkLocalState.testIpv4Addr)
    {
        // Create a new address
        // NOTE addresses must be in the rage 169.254.[1-254].xxx
        // Addresses 169.254.0.xxx and 169.254.255.xxx are reserved for future use in RFC 3927
        uint16 random16;

        do { // TODO: what is the worst case for this // It might be really bad
            random16 = RANDOM_QuickPseudoRandom16();
        } while ((GET_MSB_FROM_16(random16) == 0) || (GET_MSB_FROM_16(random16) == 0xFF)); // TODO: Should we change 0 to 1, and 0xFF to 0xFE?

        __NET_linkLocalState.testIpv4Addr =  __NET_LINK_LOCAL_BASE_ADDR | random16;
        __NET_linkLocalState.arpProbesSent = 0;

        ilog_NET_COMPONENT_1(ILOG_DEBUG, LINK_LOCAL_TEST_ADDR, __NET_linkLocalState.testIpv4Addr);
    }

    // The timeout must range between 1 and 2 seconds.
    // This approximates the timeout from 1 to 2.024 milliseconds
    timeOutInMs = 1000 + (RANDOM_QuickPseudoRandom32() & 0x3FF);

    // Send a probing ARP request
    ilog_NET_COMPONENT_2(
            ILOG_DEBUG,
            LINK_LOCAL_SENDING_ARP,
            __NET_linkLocalState.testIpv4Addr,
            timeOutInMs);
    _NET_arpTableLookupDetailed(
        __NET_linkLocalState.testIpv4Addr,
        __NET_linkLocalArpCallback,
        NULL, // callbackData 
        0,  // time outs are processed here, so there is variation on each request
        timeOutInMs);
}


