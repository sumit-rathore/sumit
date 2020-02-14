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
//!   @file  - net_dhcp.c
//
//!   @brief - Provides DHCP Client functionality.
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <net_dhcp.h>
#include <ibase.h>
#include <net_base.h>
#include <net_ethernet.h>
#include <net_ipv4.h>
#include <net_udp.h>
#include <timing_timers.h>
#include <grg.h>
#include <storage_vars.h>
#include <storage_Data.h>
#include <random.h>
#include "net_log.h"
#include "net_ipv4_loc.h"
#include "net_arp_loc.h"
#include "net_link_local_loc.h"


/************************ Defined Constants and Macros ***********************/


#define NET_DHCP_SERVER_PORT                67
#define NET_DHCP_CLIENT_PORT                68
#define NET_DHCP_BROADCAST_ADDRESS          0xffffffff
#define NET_DHCP_DISCOVER_PAYLOAD_LENGTH    257
#define NET_DHCP_REQUEST_PAYLOAD_LENGTH     269
#define NET_DHCP_DEFAULT_RETRY_TIMEOUT      (1 * 1000)          // 1 seconds in msec
#define NET_DHCP_DEFAULT_LEASE_TIMEOUT      (10 * 60 * 1000)    // ten minutes in msec
#define NET_DHCP_MAX_RETRY_INTERVAL         (32 * 1000)         // 32 seconds in msec
#define NET_DHCP_LINK_LOCAL_ADDR_TIMER      3500                // 3.5 seconds
                                                    // = 2 DHCP retries + time to check for server response

// DHCP packet offsets
#define DHCP_OFFSET_OP_CODE                 0
#define DHCP_OFFSET_HW_TYPE                 1
#define DHCP_OFFSET_HW_ADDR_LEN             2
#define DHCP_OFFSET_HOPS                    3
#define DHCP_OFFSET_TRANSACTION_ID          4
#define DHCP_OFFSET_SECONDS                 8
#define DHCP_OFFSET_FLAGS                   10
#define DHCP_OFFSET_CLIENT_ADDR             12
#define DHCP_OFFSET_YOUR_ADDR               16
#define DHCP_OFFSET_SERVER_ADDR             20
#define DHCP_OFFSET_GATEWAY_ADDR            24
#define DHCP_OFFSET_CLIENT_HARDWARE_ADDR    28
#define DHCP_OFFSET_OPTIONS_COOKIE          236
#define DHCP_OFFSET_OPTIONS                 240

#define DHCP_OPTIONS_COOKIE                 0x63825363
#define DHCP_END_OPTION                     0xff

#define DHCP_VENDOR_CLASS_IDENTIFIER "Icron USB Extender"

// op field values
enum DHCP_op
{
    DHCP_OP_REQUEST = 1,
    DHCP_OP_REPLY = 2
};

// htype field values
enum DHCP_htype
{
    DHCP_HTYPE_ETHERNET = 1,
    DHCP_HTYPE_EXPERIMENTAL_ETHERNET,
    DHCP_HTYPE_AMATEUR_RADIO,
    DHCP_HTYPE_PROTEON_PRONET_TOKEN_RING,
    DHCP_HTYPE_CHAOS,
    DHCP_HTYPE_IEEE_802_NETWORKS,
    DHCP_HTYPE_ARCNET,
    DHCP_HTYPE_HYPERCHANNEL,
    DHCP_HTYPE_LANSTAR
};

// See rfc1533 for explanations of the options
enum DHCP_option
{
    // RFC 1497 Vendor Extensions
    DHCP_OPT_PAD                                                = 0,
    DHCP_OPT_END                                                = 255,
    DHCP_OPT_SUBNET_MASK                                        = 1,
    DHCP_OPT_TIME_OFFSET                                        = 2,
    DHCP_OPT_ROUTER                                             = 3,
    DHCP_OPT_TIME_SERVER                                        = 4,
    DHCP_OPT_NAME_SERVER                                        = 5,
    DHCP_OPT_DOMAIN_NAME_SERVER                                 = 6,
    DHCP_OPT_LOG_SERVER                                         = 7,
    DHCP_OPT_COOKIE_SERVER                                      = 8,
    DHCP_OPT_LPR_SERVER                                         = 9,
    DHCP_OPT_IMPRESS_SERVER                                     = 10,
    DHCP_OPT_RESOURCE_LOCATION_SERVER                           = 11,
    DHCP_OPT_HOST_NAME                                          = 12,
    DHCP_OPT_BOOT_FILE_SIZE                                     = 13,
    DHCP_OPT_MERIT_DUMP_FILE                                    = 14,
    DHCP_OPT_DOMAIN_NAME                                        = 15,
    DHCP_OPT_SWAP_SERVER                                        = 16,
    DHCP_OPT_ROOT_PATH                                          = 17,
    DHCP_OPT_EXTENSIONS_PATH                                    = 18,

    // IP layer parameters per host
    DHCP_OPT_IP_FORWARDING_ENABLE_DISABLE                       = 19,
    DHCP_OPT_NON_LOCAL_SOURCE_ROUTING_ENABLE_DISABLE            = 20,
    DHCP_OPT_POLICY_FILTER                                      = 21,
    DHCP_OPT_MAXIMUM_DATAGRAM_REASSEMBLY_SIZE                   = 22,
    DHCP_OPT_DEFAULT_IP_TIME_TO_LIVE                            = 23,
    DHCP_OPT_PATH_MTU_AGING_TIMEOUT                             = 24,
    DHCP_OPT_PATH_MTU_PLATEAU_TABLE                             = 25,

    // IP layer parameters per interface
    DHCP_OPT_INTERFACE_MTU                                      = 26,
    DHCP_OPT_ALL_SUBNETS_ARE_LOCAL                              = 27,
    DHCP_OPT_BROADCAST_ADDRESS                                  = 28,
    DHCP_OPT_PERFORM_MASK_DISCOVERY                             = 29,
    DHCP_OPT_MASK_SUPPLIER                                      = 30,
    DHCP_OPT_PERFORM_ROUTER_DISCOVERY                           = 31,
    DHCP_OPT_ROUTER_SOLICITATION_ADDRESS                        = 32,
    DHCP_OPT_STATIC_ROUTE                                       = 33,

    // link layer parameters per interface
    DHCP_OPT_TRAILER_ENCAPSULATION                              = 34,
    DHCP_OPT_ARP_CACHE_TIMEOUT                                  = 35,
    DHCP_OPT_ETHERNET_ENCAPSULATION                             = 36,

    // TCP parameters
    DHCP_OPT_TCP_DEFAULT_TTL                                    = 37,
    DHCP_OPT_TCP_KEEPALIVE_INTERVAL                             = 38,
    DHCP_OPT_TCP_KEEPALIVE_GARBAGE                              = 39,

    // application and service parameters
    DHCP_OPT_NETWORK_INFORMATION_SERVICE_DOMAIN                 = 40,
    DHCP_OPT_NETWORK_INFORMATION_SERVERS                        = 41,
    DHCP_OPT_NETWORK_TIME_PROTOCOL_SERVERS                      = 42,
    DHCP_OPT_VENDOR_SPECIFIC_INFORMATION                        = 43,
    DHCP_OPT_NETBIOS_OVER_TCP_IP_NAME_SERVER                    = 44,
    DHCP_OPT_NETBIOS_OVER_TCP_IP_DATAGRAM_DISTRIBUTION_SERVER   = 45,
    DHCP_OPT_NETBIOS_OVER_TCP_IP_NODE_TYPE                      = 46,
    DHCP_OPT_NETBIOS_OVER_TCP_IP_SCOPE                          = 47,
    DHCP_OPT_X_WINDOW_SYSTEM_FONT_SERVER                        = 48,
    DHCP_OPT_X_WINDOW_SYSTEM_DISPLAY_MANAGER                    = 49,

    // DHCP extensions
    DHCP_OPT_REQUESTED_IP_ADDRESS                               = 50,
    DHCP_OPT_IP_ADDRESS_LEASE_TIME                              = 51,
    DHCP_OPT_OVERLOAD                                           = 52,
    DHCP_OPT_DHCP_MESSAGE_TYPE                                  = 53,
    DHCP_OPT_SERVER_IDENTIFIER                                  = 54,
    DHCP_OPT_PARAMETER_REQUEST_LIST                             = 55,
    DHCP_OPT_MESSAGE                                            = 56,
    DHCP_OPT_MAXIMUM_DHCP_MESSAGE_SIZE                          = 57,
    DHCP_OPT_RENEWAL_T1_TIME_VALUE                              = 58,
    DHCP_OPT_REBINDING_T2_TIME_VALUE                            = 59,
    DHCP_OPT_CLASS_IDENTIFIER                                   = 60,
    DHCP_OPT_CLIENT_IDENTIFIER                                  = 61
};

// DHCP Message Types - used as data when option is DHCP_OPT_DHCP_MESSAGE_TYPE
enum DHCP_MessageType
{
    DHCPDISCOVER = 1,
    DHCPOFFER    = 2,
    DHCPREQUEST  = 3,
    DHCPDECLINE  = 4,
    DHCPACK      = 5,
    DHCPNAK      = 6,
    DHCPRELEASE  = 7,
    DHCPINFORM   = 8
};


/******************************** Data Types *********************************/

// These states are taken from RFC 2131, Dynamic Host Configuration Protocol.
// You can find a copy of the RFC at "tools.ietf.org/html/rfc2131"
// A copy of the "State-transition diagram for DHCP Clients" can be
// found in ../doc/net_dhcp_state_diagram.txt.
enum NET_dhcpState
{
    DHCP_INIT_BOOT,
    DHCP_REBOOTING,
    DHCP_INIT,
    DHCP_SELECTING,
    DHCP_REQUESTING,
    DHCP_BOUND,
    DHCP_RENEWING,
    DHCP_REBINDING,
};

struct NET_dhcpClientConfig
{
    TIMING_TimerHandlerT dhcpRetryTimer; // for waiting for replies
    TIMING_TimerHandlerT dhcpLeaseRenewalTimer;
    TIMING_TimerHandlerT dhcpLeaseExpiryTimer;
    TIMING_TimerHandlerT linkLocalAddrTimer;
    NET_IPAddress       yourIp;
    NET_IPAddress       subnetMask;
    NET_IPAddress       serverIp;
    NET_IPAddress       gatewayIp;
    uint32              transactionId;
    boolT               enabled;
    boolT               linkUp;
    enum NET_dhcpState  currentState;
    boolT               leaseActive;
    NET_MACAddress      gatewayMac; // used in __NET_dhcpGatewayArpReachabilityTest
};

/***************************** Local Variables *******************************/

static struct NET_dhcpClientConfig dhcpClientConfig;

static const char NET_dhcpVendocClassId[] = DHCP_VENDOR_CLASS_IDENTIFIER;

/************************ Local Function Prototypes **************************/

static void NET_handleDHCPPacket(
    NET_UDPPort destinationPort,
    NET_IPAddress senderIP,
    NET_UDPPort senderPort,
    uint8* data,
    uint16 dataLength) __attribute__((section(".ftext")));
static void NET_dhcpRetryHandler(void) __attribute__((section(".ftext")));
static void NET_dhcpLeaseRenewalHandler(void) __attribute__((section(".ftext")));
static void NET_dhcpLeaseExpiryHandler(void) __attribute__((section(".ftext")));
static void NET_dhcpLinkLocalAddrStart(void) __attribute__((section(".ftext")));
static void NET_sendDHCPDiscover(void) __attribute__((section(".ftext")));
static void NET_sendDHCPRequest(void) __attribute__((section(".ftext")));
static void __NET_dhcpStart(void) __attribute__((section(".ftext")));
static void __NET_dhcpGatewayArpReachabilityTest(void) __attribute__((section(".ftext")));
static void __NET_dhcpGatewayArpReachabilityTestCallback(
    boolT arpLookupFailure,
    const NET_MACAddress mac,
    const NET_IPAddress ip,
    void* callbackData) __attribute__((section(".ftext")));
static const uint8* dhcpOptionFindCode(
    const uint8* dhcpReceivedOptions,
    uint32 optionsLen,
    uint8 opCode) __attribute__((section(".ftext")));
static uint8 dhcpOptionGetOneByteOptionValue(
    uint8 optionCode, const uint8* optionTLV) __attribute__((section(".ftext")));
static uint32 dhcpOptionGetFourByteOptionValue(
    uint8 optionCode, const uint8* optionTLV) __attribute__((section(".ftext")));

/***************** External Visibility Function Definitions ******************/

/**
* FUNCTION NAME: NET_dhcpInitialize()
*
* @brief  - Reads the current operating mode from flash and sets the 'enable' boolean accordingly.
*
* @return - void
*/
void NET_dhcpInitialize(void)
{
    dhcpClientConfig.enabled = FALSE;

    // NOTE: This isn't used until a packet is sent, but we just set it to an initial value to
    // hopefully not conflict with a device with transactionId == 0.
    dhcpClientConfig.transactionId = RANDOM_QuickPseudoRandom32();

    // Initialize the DHCP timers.  The timeout value of all of the timers is changed everytime it
    // is started.
    const boolT timerIsPeriodic = FALSE;
    dhcpClientConfig.dhcpRetryTimer = TIMING_TimerRegisterHandler(
        &NET_dhcpRetryHandler, timerIsPeriodic, NET_DHCP_DEFAULT_RETRY_TIMEOUT);
    dhcpClientConfig.dhcpLeaseRenewalTimer = TIMING_TimerRegisterHandler(
        &NET_dhcpLeaseRenewalHandler, timerIsPeriodic, NET_DHCP_DEFAULT_LEASE_TIMEOUT);
    dhcpClientConfig.dhcpLeaseExpiryTimer = TIMING_TimerRegisterHandler(
        &NET_dhcpLeaseExpiryHandler, timerIsPeriodic, NET_DHCP_DEFAULT_LEASE_TIMEOUT);
    dhcpClientConfig.linkLocalAddrTimer = TIMING_TimerRegisterHandler(
        &NET_dhcpLinkLocalAddrStart, timerIsPeriodic, NET_DHCP_LINK_LOCAL_ADDR_TIMER);

    iassert_NET_COMPONENT_1(
        NET_udpBindPortHandler(NET_DHCP_CLIENT_PORT, &NET_handleDHCPPacket),
        NET_DHCP_COULD_NOT_BIND_TO_PORT,
        NET_DHCP_CLIENT_PORT);

    // Initialize the fallback system, link local addresses.
    _NET_linkLocalInit();
}

/**
* FUNCTION NAME: NET_dhcpEnable()
*
* @brief  - Used for switching from static IP mode to DHCP mode.  Called from the netcfg component.
*
* @return - void
*/
void NET_dhcpEnable(void)
{
    if(dhcpClientConfig.enabled)
    {
        ilog_NET_COMPONENT_0(ILOG_MAJOR_ERROR, NET_DHCP_ALREADY_ENABLED);
    }
    else
    {
        _NET_ipv4ResetNetworkParameters();

        dhcpClientConfig.enabled = TRUE;

        dhcpClientConfig.yourIp = 0; // Don't use something old

        // Create the zerod DHCP server IP if it does not exist
        if(!STORAGE_varExists(DHCP_SERVER_IP))
        {
            STORAGE_varCreate(DHCP_SERVER_IP);
            STORAGE_varSave(DHCP_SERVER_IP);
        }

        __NET_dhcpStart();
    }
}

/**
* FUNCTION NAME: NET_dhcpDisable()
*
* @brief  - Used for switching from DHCP mode to static IP mode.  Called from the netcfg component.
*
* @return - void
*/
void NET_dhcpDisable(void)
{
    if(STORAGE_varExists(DHCP_SERVER_IP))
    {
        STORAGE_varRemove(DHCP_SERVER_IP);
        STORAGE_varSave(DHCP_SERVER_IP);
    }
    dhcpClientConfig.enabled = FALSE;
    dhcpClientConfig.leaseActive = FALSE;
    TIMING_TimerStop(dhcpClientConfig.dhcpLeaseRenewalTimer);
    TIMING_TimerStop(dhcpClientConfig.dhcpLeaseExpiryTimer);
    TIMING_TimerStop(dhcpClientConfig.dhcpRetryTimer);
    TIMING_TimerStop(dhcpClientConfig.linkLocalAddrTimer);
    _NET_linkLocalDisable();
}

/**
* FUNCTION NAME: NET_dhcpOnLinkUp()
*
* @brief  - If DHCP is enabled, send a DHCP Request or Discover depending on whether we've been
*           previously configured or not.  Called from NET_udpOnLinkUp().
*
* @return - void
*/
void NET_dhcpOnLinkUp()
{
    iassert_NET_COMPONENT_0(!dhcpClientConfig.linkUp,
            NET_DHCP_LINK_ALREADY_UP);
    dhcpClientConfig.linkUp = TRUE;

    if (!dhcpClientConfig.enabled)
    {
        return;
    }

    __NET_dhcpGatewayArpReachabilityTest();
}

// Reachability Test
// http://tools.ietf.org/html/draft-ietf-dhc-dna-ipv4-01
static void __NET_dhcpGatewayArpReachabilityTest(void)
{
    NET_MACAddress gatewayMac;

    // Send ARP request to gateway
    // If gateway doesn't reply try again
    // If gateway replies, setup old address, when should it be renewed?
    // if there is no reply go through normal DHCP process
    if (    dhcpClientConfig.leaseActive
        &&  dhcpClientConfig.yourIp
        &&  dhcpClientConfig.gatewayIp
        &&  (gatewayMac = _NET_getLastGatewayMAC()))
    {
        dhcpClientConfig.gatewayMac = gatewayMac; // TODO: shouldn't this be saved much earlier?

        _NET_arpTableLookupDetailed(
                dhcpClientConfig.gatewayIp,
                &__NET_dhcpGatewayArpReachabilityTestCallback,
                NULL, //void* callerData,
                2, // uint8 retries,
                200 /*uint32 timeOutInMs*/);
    }
    else
    {
        __NET_dhcpStart();
    }
}

static void __NET_dhcpGatewayArpReachabilityTestCallback
(
    boolT arpLookupFailure,
    const NET_MACAddress mac,
    const NET_IPAddress ip,
    void* callbackData
)
{
    if (!arpLookupFailure && (dhcpClientConfig.gatewayMac == mac))
    {
        // Success - setup old IP settings
        NET_ipv4SetIPAddress(dhcpClientConfig.yourIp);
        NET_ipv4SetSubnetMask(dhcpClientConfig.subnetMask);
        NET_ipv4SetDefaultGateway(dhcpClientConfig.gatewayIp);

        // Is it time to renew the DHCP lease?
        if (dhcpClientConfig.currentState == DHCP_RENEWING)
        {
            NET_dhcpLeaseRenewalHandler();
        }
    }
    else
    {
        __NET_dhcpStart();
    }
}

static void __NET_dhcpStart(void)
{
    if (dhcpClientConfig.linkUp)
    {
        // The enable function creates the DHCP_SERVER_IP, so we know it should always exist if we
        // are in this branch.
        if(STORAGE_varGet(DHCP_SERVER_IP)->words[0] && dhcpClientConfig.yourIp) // use yourIp, as IP gets cleared on link down
        {
            // We've connected to a DHCP server before so send a DHCP_REQUEST.
            dhcpClientConfig.currentState = DHCP_INIT_BOOT;
            NET_sendDHCPRequest();
        }
        else
        {
            // Never seen DHCP server before so send a DCHP_DISCOVER.
            dhcpClientConfig.currentState = DHCP_INIT;
            NET_sendDHCPDiscover();
        }
        TIMING_TimerResetTimeout(dhcpClientConfig.dhcpRetryTimer, NET_DHCP_DEFAULT_RETRY_TIMEOUT);
        TIMING_TimerStart(dhcpClientConfig.dhcpRetryTimer);
        TIMING_TimerStart(dhcpClientConfig.linkLocalAddrTimer);
    }
}

/**
* FUNCTION NAME: NET_dhcpOnLinkDown()
*
* @brief  - If DHCP is enabled, shut down the retry timer
*           so we don't attempt to send a discover or request
*           with the interface unplugged.
*
*           Called from NET_udpOnLinkDown()
*
* @return - void
*/
void NET_dhcpOnLinkDown()
{
    iassert_NET_COMPONENT_0(dhcpClientConfig.linkUp, NET_DHCP_LINK_ALREADY_DOWN);
    dhcpClientConfig.linkUp = FALSE;
    TIMING_TimerStop(dhcpClientConfig.dhcpRetryTimer);
    TIMING_TimerStop(dhcpClientConfig.linkLocalAddrTimer);

    if(dhcpClientConfig.enabled)
    {
      _NET_ipv4ResetNetworkParameters();
    }

    _NET_linkLocalDisable();
}


// NOTE: in this file, as this is the only user (so far)
void NET_assertHook(void)
{
    ilog_NET_COMPONENT_3(
            ILOG_FATAL_ERROR,
            NET_DHCP_ASSERT1,
            dhcpClientConfig.currentState,
            dhcpClientConfig.enabled,
            dhcpClientConfig.leaseActive);
    ilog_NET_COMPONENT_3(
            ILOG_FATAL_ERROR,
            NET_DHCP_ASSERT2,
            dhcpClientConfig.linkUp,
            dhcpClientConfig.yourIp,
            dhcpClientConfig.serverIp);
}

/***************** Component Visibility Function Definitions *****************/


/******************** File Visibility Function Definitions *******************/


/**
* FUNCTION NAME: dhcpOptionFindCode()
*
* @brief  - Return a pointer to the beginning of the TLV (type, length, value) bytes, containing
*           the requested Option Code.
*
* @return - A pointer to the first byte of the option or NULL if the option code was not found.
*
*   TODO: Could this provide a bit more functionality so callers don't duplicate so much code?
*/
static const uint8* dhcpOptionFindCode(const uint8* dhcpReceivedOptions, uint32 optionsLen, uint8 opCode)
{
    uint32 offset = 0;
    ilog_NET_COMPONENT_1(ILOG_DEBUG, NET_DHCP_FIND_OPT, opCode);
    while ((offset < optionsLen) && (dhcpReceivedOptions[offset] != DHCP_END_OPTION))
    {
        if (opCode == dhcpReceivedOptions[offset])
        {
            ilog_NET_COMPONENT_1(ILOG_DEBUG, NET_DHCP_FOUND_OPT, opCode);
            return &dhcpReceivedOptions[offset];
        }
        // If the current option is DHCP_OPT_PAD, then we only need to skip forward to the next
        // byte, otherwise we skip a number of bytes forward that is specified by the length byte
        // which follows the option byte.
        const uint8 totalOptionSize =
            1 + ((dhcpReceivedOptions[offset] == DHCP_OPT_PAD) ?
                 0 : (1 + dhcpReceivedOptions[offset + 1]));
        offset += totalOptionSize;
    }
    ilog_NET_COMPONENT_1(ILOG_DEBUG, NET_DHCP_OPT_NOT_FOUND, opCode);
    return NULL;
}

/**
* FUNCTION NAME: dhcpOptionGetOneByteOptionValue()
*
* @brief  - Return the one byte DHCP option value from the
*           "type, length, value" field passed in.
*
* @return - The value in the third byte of the field
*/
static uint8 dhcpOptionGetOneByteOptionValue(uint8 optionCode, const uint8* optionTLV)
{
    const uint8 optionVal = optionTLV[2]; // DHCP Message Types are one byte
    ilog_NET_COMPONENT_2(ILOG_DEBUG, NET_DHCP_OPT_CODE_OPT_VALUE, optionCode, optionVal);
    return optionVal;
}

/**
* FUNCTION NAME: dhcpOptionGetFourByteOptionValue()
*
* @brief  - Return the four byte DHCP option value from the
*           "type, length, value" field passed in.
*
* @return - The four byte value starting at the third byte of the field
*/
static uint32 dhcpOptionGetFourByteOptionValue(uint8 optionCode, const uint8* optionTLV)
{
    const uint32 optionVal = NET_unpack32Bits(&optionTLV[2]);
    ilog_NET_COMPONENT_2(ILOG_DEBUG, NET_DHCP_OPT_CODE_OPT_VALUE, optionCode, optionVal);
    return optionVal;
}

/**
* FUNCTION NAME: dhcpOptionGetMsgType()
*
* @brief  - Return the Message Type contained in the "DHCP Option 53" payload
*
* @return - The DHCP Message Type contained in DHCP Option 53 on success,
*           0 on not finding Option 53.
*/
static uint8 dhcpOptionGetMsgType(const uint8* dhcpReceivedOptions, uint32 optionsLength)
{
    const uint8* optionTLV = dhcpOptionFindCode(dhcpReceivedOptions, optionsLength, 53);
    if (optionTLV)
    {
        return dhcpOptionGetOneByteOptionValue(53, optionTLV);
    }
    return 0;
}

/**
* FUNCTION NAME: dhcpOptionGetSubnetMask()
*
* @brief  - Return Subnet Mask contained in "DHCP Option 1" payload
*
* @return - The Subnet Mask contained in DHCP Option 1 on success,
*           0 on not finding Option 1.
*/
static NET_IPAddress dhcpOptionGetSubnetMask(const uint8* dhcpReceivedOptions, uint32 optionsLength)
{
    const uint8* optionTLV = dhcpOptionFindCode(dhcpReceivedOptions, optionsLength, 1);
    if (optionTLV)
    {
        return (NET_IPAddress) dhcpOptionGetFourByteOptionValue(1, optionTLV);
    }
    return 0;
}

/**
* FUNCTION NAME: dhcpOptionGetRenewalTime()
*
* @brief  - Return Renewal Time contained in "DHCP Option 58" payload
*
* @return - The Renewal Time contained in DHCP Option 58 on success,
*           0 on not finding Option 58.
*/
static uint32 dhcpOptionGetRenewalTime(const uint8* dhcpReceivedOptions, uint32 optionsLength)
{
    const uint8* optionTLV = dhcpOptionFindCode(dhcpReceivedOptions, optionsLength, 58);
    if (optionTLV)
    {
        return dhcpOptionGetFourByteOptionValue(58, optionTLV);
    }
    return 0;
}

/**
* FUNCTION NAME: dhcpOptionGetRebindingTime()
*
* @brief  - Return Rebinding Time contained in "DHCP Option 59" payload
*
* @return - The Rebinding Time contained in DHCP Option 59 on success,
*           0 on not finding Option 59.
*/
static uint32 dhcpOptionGetRebindingTime(const uint8* dhcpReceivedOptions, uint32 optionsLength)
{
    const uint8* optionTLV = dhcpOptionFindCode(dhcpReceivedOptions, optionsLength, 59);
    if (optionTLV)
    {
        return dhcpOptionGetFourByteOptionValue(59, optionTLV);
    }
    return 0;
}

/**
* FUNCTION NAME: dhcpOptionGetLeaseTime()
*
* @brief  - Return Lease Time contained in "DHCP Option 51" payload
*
* @return - The Lease Time contained in DHCP Option 51 on success,
*           0 on not finding Option 51.
*/
static uint32 dhcpOptionGetLeaseTime(const uint8* dhcpReceivedOptions, uint32 optionsLength)
{
    const uint8* optionTLV = dhcpOptionFindCode(dhcpReceivedOptions, optionsLength, 51);
    if (optionTLV)
    {
        return dhcpOptionGetFourByteOptionValue(51, optionTLV);
    }
    return 0;
}

/**
* FUNCTION NAME: dhcpOptionGetServerIp()
*
* @brief  - Return DHCP Server IP contained in "DHCP Option 54" payload
*
* @return - The Server IP Address contained in "DHCP Option 54" on success,
*           0 on not finding Option 54.
*/
static NET_IPAddress dhcpOptionGetServerIp(const uint8* dhcpReceivedOptions, uint32 optionsLength)
{
    const uint8* optionTLV = dhcpOptionFindCode(dhcpReceivedOptions, optionsLength, 54);
    if (optionTLV)
    {
        return (NET_IPAddress) dhcpOptionGetFourByteOptionValue(54, optionTLV);
    }
    return 0;
}

/**
* FUNCTION NAME: dhcpOptionGetGatewayIp()
*
* @brief  - Return Gateway IP contained in "DHCP Option 3" payload
*
* @return - The Gateway IP Address contained in DHCP Option 3 on success,
*           0 on not finding Option 3.
*/
static NET_IPAddress dhcpOptionGetGatewayIp(const uint8* dhcpReceivedOptions, uint32 optionsLength)
{
    const uint8* optionTLV = dhcpOptionFindCode(dhcpReceivedOptions, optionsLength, 3);
    if (optionTLV)
    {
        return (NET_IPAddress) dhcpOptionGetFourByteOptionValue(3, optionTLV);
    }
    return 0;
}

/**
* FUNCTION NAME: NET_handleDHCPPacket()
*
* @brief  - Processes incoming packets received for the port we've bound to.
*           This is the DHCP client port number 68 and we process all
*           received DHCP packets here.
*
* @return - void
*/
static void NET_handleDHCPPacket(
    NET_UDPPort destinationPort,
    NET_IPAddress senderIP,
    NET_UDPPort senderPort,
    uint8* data,
    uint16 dataLength)
{
    uint8* dhcpReceivedPkt = data;
    NET_MACAddress receivedLocalMAC;
    uint32 receivedTransId;
    const uint32 optionsLength = dataLength - DHCP_OFFSET_OPTIONS;

    ilog_NET_COMPONENT_1(ILOG_DEBUG, NET_DHCP_HDR0, dhcpReceivedPkt[DHCP_OFFSET_OP_CODE]);
    ilog_NET_COMPONENT_1(ILOG_DEBUG, NET_DHCP_HDR1, dhcpReceivedPkt[DHCP_OFFSET_HW_TYPE]);
    ilog_NET_COMPONENT_1(ILOG_DEBUG, NET_DHCP_HDR2, dhcpReceivedPkt[DHCP_OFFSET_HW_ADDR_LEN]);
    ilog_NET_COMPONENT_1(ILOG_DEBUG, NET_DHCP_HDR3, dhcpReceivedPkt[DHCP_OFFSET_HOPS]);

    receivedTransId = NET_unpack32Bits(&dhcpReceivedPkt[DHCP_OFFSET_TRANSACTION_ID]);
    receivedLocalMAC = NET_unpack48Bits(&dhcpReceivedPkt[DHCP_OFFSET_CLIENT_HARDWARE_ADDR]);

    if (dhcpClientConfig.enabled &&
        dhcpReceivedPkt[DHCP_OFFSET_OP_CODE] == DHCP_OP_REPLY &&
        dhcpReceivedPkt[DHCP_OFFSET_HW_TYPE] == DHCP_HTYPE_ETHERNET &&
        dhcpReceivedPkt[DHCP_OFFSET_HW_ADDR_LEN] == 0x06 &&
        receivedTransId == dhcpClientConfig.transactionId &&
        receivedLocalMAC == NET_ethernetGetSelfMACAddress())
    {
        uint8 dhcpMsgType;
        uint32 receivedCookie = NET_unpack32Bits(&dhcpReceivedPkt[DHCP_OFFSET_OPTIONS_COOKIE]);
        if (receivedCookie != DHCP_OPTIONS_COOKIE)
        {
            // This should never happen, logging just to assist in debug
            ilog_NET_COMPONENT_1(
                ILOG_MAJOR_ERROR, NET_DHCP_OFFER_INVALID_OPT_COOKIE, receivedCookie);
            return;
        }

        // We've received a DHCP packet for us so stop the retry timer.
        TIMING_TimerStop(dhcpClientConfig.dhcpRetryTimer);

        dhcpMsgType = dhcpOptionGetMsgType(&dhcpReceivedPkt[DHCP_OFFSET_OPTIONS], optionsLength);
        switch (dhcpMsgType)
        {
            case DHCPOFFER:
            {
                ilog_NET_COMPONENT_1(ILOG_DEBUG, NET_DHCP_OFFER_RECEIVED, senderIP);
                dhcpClientConfig.currentState = DHCP_SELECTING;
                dhcpClientConfig.yourIp =
                    NET_unpack32Bits(&dhcpReceivedPkt[DHCP_OFFSET_YOUR_ADDR]);
                STORAGE_varGet(DHCP_SERVER_IP)->words[0] =
                    dhcpOptionGetServerIp(&dhcpReceivedPkt[DHCP_OFFSET_OPTIONS], optionsLength);
                NET_sendDHCPRequest();
                TIMING_TimerResetTimeout(dhcpClientConfig.dhcpRetryTimer, NET_DHCP_DEFAULT_RETRY_TIMEOUT);
                TIMING_TimerStart(dhcpClientConfig.dhcpRetryTimer);
                break;
            }

            case DHCPACK:
// NOTE: renewalTime and rebindingTime are not currently used, so disable the related warning
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
            {
                NET_IPAddress ackedIp;
                NET_IPAddress requestedIp;
                ilog_NET_COMPONENT_1(ILOG_DEBUG, NET_DHCP_ACK_RECEIVED, senderIP);
                // As long as I get the expected IP in the ACK, I assume we're good. We could check
                // all fields but it's overkill.  We want to be as forgiving as possible.
                ackedIp = NET_unpack32Bits(&dhcpReceivedPkt[DHCP_OFFSET_YOUR_ADDR]);
                dhcpClientConfig.subnetMask =
                    dhcpOptionGetSubnetMask(&dhcpReceivedPkt[DHCP_OFFSET_OPTIONS], optionsLength);
                dhcpClientConfig.gatewayIp =
                    dhcpOptionGetGatewayIp(&dhcpReceivedPkt[DHCP_OFFSET_OPTIONS], optionsLength);
                const uint32 renewalTime =
                    dhcpOptionGetRenewalTime(&dhcpReceivedPkt[DHCP_OFFSET_OPTIONS], optionsLength);
                const uint32 rebindingTime =
                    dhcpOptionGetRebindingTime(&dhcpReceivedPkt[DHCP_OFFSET_OPTIONS], optionsLength);
                const uint32 leaseTime =
                    dhcpOptionGetLeaseTime(&dhcpReceivedPkt[DHCP_OFFSET_OPTIONS], optionsLength);
                // If we've done a DHCP_DISCOVER, yourIp gets set by the DHCP_OFFER, else we're
                // just doing a DHCP_REQUEST and we're renewing our existing IP.
                requestedIp = dhcpClientConfig.yourIp;
                if (requestedIp == ackedIp)
                {
                    dhcpClientConfig.currentState = DHCP_BOUND;

                    // TODO: RFC2131 section 4.4.1 says "The client SHOULD perform a check on the
                    // suggested address to ensure that the address is not already in use.  For
                    // example, if the client is on a network that supports ARP, the client may
                    // issue an ARP request for the suggested address (sic).  When broadcasting an
                    // ARP request for the suggested address, the client must fill in its own
                    // hardware address as the sender's hardware address, and 0 as the sender's IP
                    // address, to avoid confusing ARP caches in other hosts on the same subnet.".
                    //
                    // If we determine that our IP address is in use, we should send a DHCPDECLINE
                    // message to the server.  If the IP is not in use, we should send an ARP reply
                    // to announce the client's new IP address and clear any outdated ARP caches.

                    // Update flash with DHCP values, we're done.
                    STORAGE_varSave(DHCP_SERVER_IP);
                    NET_ipv4SetIPAddress(ackedIp);
                    NET_ipv4SetSubnetMask(dhcpClientConfig.subnetMask);
                    NET_ipv4SetDefaultGateway(dhcpClientConfig.gatewayIp);

                    _NET_linkLocalDisable();
                    TIMING_TimerStop(dhcpClientConfig.linkLocalAddrTimer);

                    _NET_ipv4DoArpRequestOfGateway();

                    // lease time calculations.  Convert seconds to milliseconds (multiply by 1000)
                    // which is (leaseTime * 1024) - (leaseTime * 24)
                    // which is (leaseTime * 1024) - (leaseTime * 32) + (leaseTime * 8)
                    // which is (leaseTime << 10)  - (leaseTime << 5) + (leaseTime << 3)
                    // NOTE: This is converting a 32 bit second timer into a 32 bit millisecond
                    //       timer.
                    // Check for overflow cases, where a shift by 10 would overflow
                    const uint32 leaseTimeInMilliseconds = (leaseTime > 0x003FFFFF) ?
                        0xf9fffc18 : // 0x003FFFFF * 1000
                        ((leaseTime << 10) - (leaseTime << 5) + (leaseTime << 3));

                    // Start renewal at half the lease time
                    TIMING_TimerResetTimeout(
                        dhcpClientConfig.dhcpLeaseRenewalTimer, leaseTimeInMilliseconds >> 1);
                    TIMING_TimerStart(dhcpClientConfig.dhcpLeaseRenewalTimer);
                    // Set the expiration timer
                    TIMING_TimerResetTimeout(
                        dhcpClientConfig.dhcpLeaseExpiryTimer, leaseTimeInMilliseconds);
                    TIMING_TimerStart(dhcpClientConfig.dhcpLeaseExpiryTimer);
                    dhcpClientConfig.leaseActive = TRUE;

                    ilog_NET_COMPONENT_2(ILOG_DEBUG, NET_DHCP_CONF_VALUES1, ackedIp, leaseTime);
                    ilog_NET_COMPONENT_2(
                        ILOG_DEBUG,
                        NET_DHCP_CONF_VALUES2,
                        dhcpClientConfig.gatewayIp,
                        dhcpClientConfig.subnetMask);
                }
                else
                {
                    // Got unexpected ACK from DHCP server,
                    // so need to start over and send DHCPDISCOVER
                    dhcpClientConfig.currentState = DHCP_INIT;
                    TIMING_TimerResetTimeout(dhcpClientConfig.dhcpRetryTimer, NET_DHCP_DEFAULT_RETRY_TIMEOUT);
                    TIMING_TimerStart(dhcpClientConfig.dhcpRetryTimer);
                }
                break;
            }
#pragma GCC diagnostic pop

            case DHCPNAK:
            {
                // Got NAK'd so need to start over and send DHCPDISCOVER
                ilog_NET_COMPONENT_1(ILOG_DEBUG, NET_DHCP_NAK_RECEIVED, senderIP);
                dhcpClientConfig.currentState = DHCP_INIT;
                TIMING_TimerResetTimeout(dhcpClientConfig.dhcpRetryTimer, NET_DHCP_DEFAULT_RETRY_TIMEOUT);
                TIMING_TimerStart(dhcpClientConfig.dhcpRetryTimer);
                break;
            }

            default:
                ilog_NET_COMPONENT_1(ILOG_MINOR_EVENT, NET_DHCP_UNSUPPORTED_MSG_TYPE, dhcpMsgType);
                break;
        }
    }
    else
    {
        ilog_NET_COMPONENT_0(ILOG_DEBUG, NET_DHCP_PACKET_IGNORED);
    }
}


/**
* FUNCTION NAME: NET_dhcpRetryHandler()
*
* @brief  - Handles retry processing for DHCP Discover and Request messages.
*           Called by the timer interrupt when the DHCP Retry Timer
*           times out.
*
* @return - void
*/
static void NET_dhcpRetryHandler(void)
{
    // Double time timeout until NET_DHCP_MAX_RETRY_INTERVAL is reached
    uint32 currTimeout = TIMING_TimerGetTimeout(dhcpClientConfig.dhcpRetryTimer) << 1;
    if (currTimeout > NET_DHCP_MAX_RETRY_INTERVAL)
    {
        currTimeout = NET_DHCP_MAX_RETRY_INTERVAL;
        dhcpClientConfig.currentState = DHCP_INIT;
    }
    TIMING_TimerResetTimeout(dhcpClientConfig.dhcpRetryTimer, currTimeout);

    if (dhcpClientConfig.currentState == DHCP_INIT)
    {
        NET_sendDHCPDiscover();
    }
    else
    {
        // Else we've timed out when in either the selecting, rebooting, renewing or rebinding
        // state, so resend a request.
        NET_sendDHCPRequest();
    }

    TIMING_TimerStart(dhcpClientConfig.dhcpRetryTimer);
}


/**
* FUNCTION NAME: NET_dhcpLeaseRenewalHandler()
*
* @brief  - Updates state and sends a DHCP Request packet.  Called by the timer interrupt when the
*           DHCP lease expire timer times out.
*
* @return - void
*/
static void NET_dhcpLeaseRenewalHandler(void)
{
    iassert_NET_COMPONENT_1(dhcpClientConfig.enabled, NET_DCHP_UNEXPECTED_STATE, __LINE__);
    dhcpClientConfig.currentState = DHCP_RENEWING;

    if (dhcpClientConfig.linkUp)
    {
        NET_sendDHCPRequest();

        TIMING_TimerResetTimeout(dhcpClientConfig.dhcpRetryTimer, NET_DHCP_DEFAULT_RETRY_TIMEOUT);
        TIMING_TimerStart(dhcpClientConfig.dhcpRetryTimer);
    }
}


/**
* FUNCTION NAME: NET_dhcpLeaseExpiryHandler()
*
* @brief  - Called whenever the DHCP lease has expired
*
* @return - void
*/
static void NET_dhcpLeaseExpiryHandler(void)
{
    iassert_NET_COMPONENT_1(dhcpClientConfig.enabled, NET_DCHP_UNEXPECTED_STATE, __LINE__);
    dhcpClientConfig.leaseActive = FALSE;

    if (dhcpClientConfig.linkUp)
    {
        // Back to the beginning.  Reset all parameters.  NOTE: This scenario will probably result
        // in a link local address setup.
        _NET_ipv4ResetNetworkParameters();
        __NET_dhcpStart();
    }
}

/**
* FUNCTION NAME: NET_dhcpLinkLocalAddrStart()
*
* @brief  - Enable link local addressing as a result of failing to acquire a DHCP address.
*
* @return - void
*/
static void NET_dhcpLinkLocalAddrStart(void)
{
    _NET_linkLocalEnable();
}

/**
* FUNCTION NAME: NET_sendDHCPDiscover()
*
* @brief  - Construct and send a DHCP Discover packet.
*
* @return - void
*/
static void NET_sendDHCPDiscover(void)
{
    // Note: NET_udpAllocateBuffer asserts on failure, so no need to check
    uint8* dhcpPayload = NET_udpAllocateBuffer();

    uint8 *dhcpSetPtr;  // temporary pointer used to set option fields

    // New transaction, create a new transaction ID
    dhcpClientConfig.transactionId = RANDOM_QuickPseudoRandom32();

    // NOTE:
    // The buffer is zerod before allocation, so we only need to write the non-zero values.  Values
    // which would have been written to zero had the buffer not been zerod have been left commented
    // out for clarity.
    dhcpPayload[DHCP_OFFSET_OP_CODE] = DHCP_OP_REQUEST;
    dhcpPayload[DHCP_OFFSET_HW_TYPE] = DHCP_HTYPE_ETHERNET;
    dhcpPayload[DHCP_OFFSET_HW_ADDR_LEN] = 0x06;
    // See note above: dhcpPayload[DHCP_OFFSET_HOPS] = 0x00;
    NET_pack32Bits(dhcpClientConfig.transactionId, &dhcpPayload[DHCP_OFFSET_TRANSACTION_ID]);
    // See note above: NET_pack16Bits(0x0000, &dhcpPayload[DHCP_OFFSET_SECONDS]);
    NET_pack16Bits(0x8000, &dhcpPayload[DHCP_OFFSET_FLAGS]); // Broadcast bit
    // See note above: NET_pack32Bits(0x00000000, &dhcpPayload[DHCP_OFFSET_CLIENT_ADDR]);
    // See note above: NET_pack32Bits(0x00000000, &dhcpPayload[DHCP_OFFSET_YOUR_ADDR]);
    // See note above: NET_pack32Bits(0x00000000, &dhcpPayload[DHCP_OFFSET_SERVER_ADDR]);
    // See note above: NET_pack32Bits(0x00000000, &dhcpPayload[DHCP_OFFSET_GATEWAY_ADDR]);
    NET_pack48Bits(NET_ethernetGetSelfMACAddress(), &dhcpPayload[DHCP_OFFSET_CLIENT_HARDWARE_ADDR]);
    // DHCP hardware addr field is 16 bytes so ensure zeros in remaining bytes
    // See note above: NET_pack16Bits(0x0000, &dhcpPayload[DHCP_OFFSET_CLIENT_HARDWARE_ADDR + 6]);
    // See note above: NET_pack32Bits(0x00000000,
    //               &dhcpPayload[DHCP_OFFSET_CLIENT_HARDWARE_ADDR + 8]);
    // See note above: NET_pack32Bits(0x00000000,
    //               &dhcpPayload[DHCP_OFFSET_CLIENT_HARDWARE_ADDR + 12]);
    // Next 192 bytes are all zeros
    // See note above: memset(&dhcpPayload[DHCP_OFFSET_CLIENT_HARDWARE_ADDR + 16], 0, 192);
    NET_pack32Bits(DHCP_OPTIONS_COOKIE, &dhcpPayload[DHCP_OFFSET_OPTIONS_COOKIE]);

    dhcpSetPtr = &dhcpPayload[DHCP_OFFSET_OPTIONS];

    *dhcpSetPtr++ = DHCP_OPT_DHCP_MESSAGE_TYPE;
    *dhcpSetPtr++ = 0x01; // length of DHCP Option
    *dhcpSetPtr++ = DHCPDISCOVER;

    // If Option 60 is enabled, only then execute the following code
    if ((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >>
           TOPLEVEL_ENABLE_DHCP_OPTION_60_OFFSET) & 0x1)
    {
        *dhcpSetPtr++ = DHCP_OPT_CLASS_IDENTIFIER;
        *dhcpSetPtr++ = sizeof(NET_dhcpVendocClassId); // string length
        memcpy(dhcpSetPtr, NET_dhcpVendocClassId, sizeof(NET_dhcpVendocClassId) );
        dhcpSetPtr += sizeof(NET_dhcpVendocClassId);  // skip over the string
    }

    *dhcpSetPtr++ = DHCP_OPT_CLIENT_IDENTIFIER;
    *dhcpSetPtr++ = 0x07; // 1 byte for HW Type + 6 byte MAC
    *dhcpSetPtr++ = DHCP_HTYPE_ETHERNET;
    NET_pack48Bits(NET_ethernetGetSelfMACAddress(), dhcpSetPtr);
    dhcpSetPtr += 6;

    *dhcpSetPtr++ = DHCP_OPT_PARAMETER_REQUEST_LIST;
    *dhcpSetPtr++ = 0x02; // length of list
    *dhcpSetPtr++ = DHCP_OPT_SUBNET_MASK;
    *dhcpSetPtr++ = DHCP_OPT_ROUTER; // a.k.a. gateway

    *dhcpSetPtr = DHCP_OPT_END;

    NET_udpTransmitPacket(
        dhcpPayload,
        NET_DHCP_BROADCAST_ADDRESS,
        NET_DHCP_DISCOVER_PAYLOAD_LENGTH,
        NET_DHCP_CLIENT_PORT,
        NET_DHCP_SERVER_PORT);
    ilog_NET_COMPONENT_0(ILOG_DEBUG, NET_DHCP_DISCOVER_SENT);
}

/**
* FUNCTION NAME: NET_sendDHCPRequest()
*
* @brief  - Construct and send a DHCP Request packet.
*
* @return - void
*/
static void NET_sendDHCPRequest(void)
{
    // Note: NET_udpAllocateBuffer asserts on failure, so no need to check
    uint8* dhcpPayload = NET_udpAllocateBuffer();

    uint8 *dhcpSetPtr;  // temporary pointer used to set option fields

    // New transaction, create a new transaction ID
    dhcpClientConfig.transactionId = RANDOM_QuickPseudoRandom32();

    // NOTE:
    // The buffer allocated by NET_udpAllocateBuffer is guaranteed to be zeroed, so we only need to
    // write the non-zero values.  The statements which set values to zero are intentionally left
    // commented out to explicitly express what the values are set to while requiring no
    // additionall code space.
    dhcpPayload[DHCP_OFFSET_OP_CODE] = DHCP_OP_REQUEST;
    dhcpPayload[DHCP_OFFSET_HW_TYPE] = DHCP_HTYPE_ETHERNET;
    dhcpPayload[DHCP_OFFSET_HW_ADDR_LEN] = 0x06;
    // See note above: dhcpPayload[DHCP_OFFSET_HOPS] = 0x00;
    NET_pack32Bits(dhcpClientConfig.transactionId, &dhcpPayload[DHCP_OFFSET_TRANSACTION_ID]);
    // See note above: NET_pack16Bits(0x0000, &dhcpPayload[DHCP_OFFSET_SECONDS]);
    NET_pack16Bits(0x8000, &dhcpPayload[DHCP_OFFSET_FLAGS]); // broadcast bit
    if (    (dhcpClientConfig.currentState == DHCP_BOUND)
        ||  (dhcpClientConfig.currentState == DHCP_RENEWING)
        ||  (dhcpClientConfig.currentState == DHCP_REBINDING))
    {
        // See RFC 2131 page 9
        NET_pack32Bits(NET_ipv4GetIPAddress(), &dhcpPayload[DHCP_OFFSET_CLIENT_ADDR]);
    }
    else
    {
        // See note above: NET_pack32Bits(0x00000000, &dhcpPayload[DHCP_OFFSET_CLIENT_ADDR]);
    }
    // See note above: NET_pack32Bits(0x00000000, &dhcpPayload[DHCP_OFFSET_YOUR_ADDR]);
    // See note above: NET_pack32Bits(0x00000000, &dhcpPayload[DHCP_OFFSET_SERVER_ADDR]);
    // See note above: NET_pack32Bits(0x00000000, &dhcpPayload[DHCP_OFFSET_GATEWAY_ADDR]);
    NET_pack48Bits(NET_ethernetGetSelfMACAddress(), &dhcpPayload[DHCP_OFFSET_CLIENT_HARDWARE_ADDR]);
    // DHCP hardware addr field is 16 bytes so ensure zeros in remaining bytes
    // See note above: NET_pack16Bits(0x0000, &dhcpPayload[DHCP_OFFSET_CLIENT_HARDWARE_ADDR + 6]);
    // See note above: NET_pack32Bits(0x00000000, &dhcpPayload[DHCP_OFFSET_CLIENT_HARDWARE_ADDR + 8]);
    // See note above: NET_pack32Bits(0x00000000, &dhcpPayload[DHCP_OFFSET_CLIENT_HARDWARE_ADDR + 12]);
    // Next 192 bytes are all zeros
    // See note above: memset(&dhcpPayload[DHCP_OFFSET_CLIENT_HARDWARE_ADDR + 16], 0, 192);
    NET_pack32Bits(DHCP_OPTIONS_COOKIE, &dhcpPayload[DHCP_OFFSET_OPTIONS_COOKIE]);

    dhcpSetPtr = &dhcpPayload[DHCP_OFFSET_OPTIONS];

    *dhcpSetPtr++ = DHCP_OPT_DHCP_MESSAGE_TYPE;
    *dhcpSetPtr++ = 0x01; // length of DHCP Option
    *dhcpSetPtr++ = DHCPREQUEST;

    // If Option 60 is enabled, only then execute the following code
    if ((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >>
           TOPLEVEL_ENABLE_DHCP_OPTION_60_OFFSET) & 0x1)
    {
        *dhcpSetPtr++ = DHCP_OPT_CLASS_IDENTIFIER;
        *dhcpSetPtr++ = sizeof(NET_dhcpVendocClassId); // string length
        memcpy(dhcpSetPtr, NET_dhcpVendocClassId, sizeof(NET_dhcpVendocClassId) );
        dhcpSetPtr += sizeof(NET_dhcpVendocClassId);  // skip over the string
    }

    *dhcpSetPtr++ = DHCP_OPT_CLIENT_IDENTIFIER;
    *dhcpSetPtr++ = 0x07; // 1 byte for HW Type + 6 byte MAC
    *dhcpSetPtr++ = DHCP_HTYPE_ETHERNET;
    NET_pack48Bits(NET_ethernetGetSelfMACAddress(), dhcpSetPtr);
    dhcpSetPtr += 6;

    *dhcpSetPtr++ = DHCP_OPT_REQUESTED_IP_ADDRESS;
    *dhcpSetPtr++ = 0x04; // length of DHCP Option
    NET_pack32Bits(dhcpClientConfig.yourIp, dhcpSetPtr);
    dhcpSetPtr += 4;

    *dhcpSetPtr++ = DHCP_OPT_SERVER_IDENTIFIER;
    *dhcpSetPtr++ = 0x04; // length of DHCP Option
    memcpy(dhcpSetPtr, STORAGE_varGet(DHCP_SERVER_IP)->bytes, 4);
    dhcpSetPtr += 4;

    *dhcpSetPtr++ = DHCP_OPT_PARAMETER_REQUEST_LIST;
    *dhcpSetPtr++ = 0x02; // length of list
    *dhcpSetPtr++ = DHCP_OPT_SUBNET_MASK;
    *dhcpSetPtr++ = DHCP_OPT_ROUTER; // a.k.a. gateway

    *dhcpSetPtr = DHCP_OPT_END;

    NET_udpTransmitPacket(
        dhcpPayload,
        NET_DHCP_BROADCAST_ADDRESS,
        NET_DHCP_REQUEST_PAYLOAD_LENGTH,
        NET_DHCP_CLIENT_PORT,
        NET_DHCP_SERVER_PORT);

    ilog_NET_COMPONENT_0(ILOG_DEBUG, NET_DHCP_REQUEST_SENT);
}

