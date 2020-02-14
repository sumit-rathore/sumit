///////////////////////////////////////////////////////////////////////////////
//
//    Icron Technology Corporation - Copyright 2012
//
//
//    This source file and the information contained in it are confidential and
//    proprietary to Icron Technology Corporation. The reproduction or disclosure,
//    in whole or in part, to anyone outside of Icron without the written approval
//    of a Icron officer under a Non-Disclosure Agreement, or to any employee of
//    Icron who has not previously obtained written authorization for access from
//    the individual responsible for the source code, will have a significant
//    detrimental effect on Icron and is expressly prohibited.
//
///////////////////////////////////////////////////////////////////////////////
//
//!   @file xusbnetcfg.h
//!         Command line utility to configure LEXs and REXs over the network
//!         using UDP. This file provides exported function prototypes and
//!         structures.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XUSBNETCFG_H
#define XUSBNETCFG_H

/***************************** Included Headers ******************************/
#ifdef __MINGW32__
#include <WS2tcpip.h>
#endif
#include <stdint.h>

#define MAGIC_NUMBER 0x2F03F4A2
#define IP4_STR_LEN 16
#define MAC_STR_LEN 18
#define MAX_USB_DEVICES 32
#define REPLY_DEV_INFO_VENDOR_LENGTH 32
#define REPLY_DEV_INFO_PRODUCT_LENGTH 32
#define REPLY_DEV_INFO_REVISION_LENGTH 12
#define MAX_PAIRED_MACS 7
#define MAX_PROTOCOL_SUPPORTED 2
#define REPLY_CNFG_RESP_DATA_VENDOR_LENGTH 32
#define REPLY_CNFG_RESP_DATA_PRODUCT_LENGTH 32
#define REPLY_CNFG_RESP_DATA_REVISION_LENGTH 12

#define TIMEOUT_ONE_SEC 1
#define TIMEOUT_TWO_SEC 2

#define XUSBNETCFG_DEFAULT_PORT 6137
#define XUSBNETCFG_MAX_CMD_LEN (sizeof(IcronReplyDevTopologyTVersion2))
#define MINIMUM_PACKET_SIZE (sizeof(GeneralInfoT))

//#define ICRON_XUSBNETCFG_DEBUG

#ifdef ICRON_XUSBNETCFG_DEBUG
#define DEBUG(...) \
    { printf("%s:%d:%s(): ", __FILE__, __LINE__, __func__); \
      printf(__VA_ARGS__); \
      puts(""); }
#else
#define DEBUG(...)
#endif

#define PRINTF(...) \
    { printf("%s:%d:%s(): ", __FILE__, __LINE__, __func__); \
      printf(__VA_ARGS__); \
      puts(""); }

typedef struct MessageType
{
    /// An integer from 0-255.  All devices will support protocol 0 and
    /// one other protocol version.
    uint8_t protocolVersion;
    /// An integer from 0-255.  This is the identifier of the command.
    uint8_t command;
} __attribute__((packed)) MessageTypeT;

// Message Type Constants

// Protocol version 0
// Constants are used for building command to response mapping array
#define REQUEST_DEV_INFO_MSG_TYPE_0     { 0, 0 }
#define REPLY_DEV_INFO_MSG_TYPE_0       { 0, 1 }
#define PING_MSG_TYPE_0                 { 0, 2 }
#define ACKNOWLEDGE_MSG_TYPE_0          { 0, 3 }
#define UNDEFINED_MSG_TYPE_0            { 0, 4 }

// Note: Global variables are used for comparing responses
static const MessageTypeT RequestDeviceInfo_0           = REQUEST_DEV_INFO_MSG_TYPE_0;
static const MessageTypeT ReplyDeviceInfo_0             = REPLY_DEV_INFO_MSG_TYPE_0;
static const MessageTypeT Ping_0                        = PING_MSG_TYPE_0;
static const MessageTypeT Acknowledge_0                 = ACKNOWLEDGE_MSG_TYPE_0;
static const MessageTypeT Undefined_0                   = UNDEFINED_MSG_TYPE_0;

// Protocol version 1
#define REQUEST_EXT_DEV_INFO_MSG_TYPE_1 { 1, 0 }
#define REPLY_EXT_DEV_INFO_MSG_TYPE_1   { 1, 1 }
#define PAIR_TO_DEV_MSG_TYPE_1          { 1, 2 }
#define REMOVE_DEV_PAIRING_MSG_TYPE_1   { 1, 3 }
#define REQUEST_DEV_TOPOLOGY_MSG_TYPE_1 { 1, 4 }
#define REPLY_DEV_TOPOLOGY_MSG_TYPE_1   { 1, 5 }
#define USE_DHCP_MSG_TYPE_1             { 1, 6 }
#define USE_STATIC_MSG_TYPE_1           { 1, 7 }
#define UNDEFINED_MSG_TYPE_1            { 1, 8 }

static const MessageTypeT RequestExtendedDeviceInfo_1   = REQUEST_EXT_DEV_INFO_MSG_TYPE_1;
static const MessageTypeT ReplyExtendedDeviceInfo_1     = REPLY_EXT_DEV_INFO_MSG_TYPE_1;
static const MessageTypeT PairToDevice_1                = PAIR_TO_DEV_MSG_TYPE_1;
static const MessageTypeT RemoveDevicePairing_1         = REMOVE_DEV_PAIRING_MSG_TYPE_1;
static const MessageTypeT RequestDeviceTopology_1       = REQUEST_DEV_TOPOLOGY_MSG_TYPE_1;
static const MessageTypeT ReplyDeviceTopology_1         = REPLY_DEV_TOPOLOGY_MSG_TYPE_1;
static const MessageTypeT UseDHCP_1                     = USE_DHCP_MSG_TYPE_1;
static const MessageTypeT UseStatic_1                   = USE_STATIC_MSG_TYPE_1;
static const MessageTypeT Undefined_1                   = UNDEFINED_MSG_TYPE_1;

// Protocol version 2
#define REQUEST_EXT_DEV_INFO_MSG_TYPE_2 { 2, 0 }
#define REPLY_EXT_DEV_INFO_MSG_TYPE_2   { 2, 1 }
#define PAIR_TO_DEV_MSG_TYPE_2          { 2, 2 }
#define REMOVE_DEV_PAIRING_MSG_TYPE_2   { 2, 3 }
#define REQUEST_DEV_TOPOLOGY_MSG_TYPE_2 { 2, 4 }
#define REPLY_DEV_TOPOLOGY_MSG_TYPE_2   { 2, 5 }
#define USE_DHCP_MSG_TYPE_2             { 2, 6 }
#define USE_STATIC_MSG_TYPE_2           { 2, 7 }
#define NEGATIVE_ACKNOWLEDGE_MSG_TYPE_2 { 2, 8 }
#define UNDEFINED_MSG_TYPE_2            { 2, 9 }
#define REQUEST_CNFG_RESP_DATA_MSG_TYPE_2 { 2, 10 }
#define REPLY_CNFG_RESP_DATA_MSG_TYPE_2   { 2, 11 }

static const MessageTypeT RequestExtendedDeviceInfo_2   = REQUEST_EXT_DEV_INFO_MSG_TYPE_2;
static const MessageTypeT ReplyExtendedDeviceInfo_2     = REPLY_EXT_DEV_INFO_MSG_TYPE_2;
static const MessageTypeT PairToDevice_2                = PAIR_TO_DEV_MSG_TYPE_2;
static const MessageTypeT RemoveDevicePairing_2         = REMOVE_DEV_PAIRING_MSG_TYPE_2;
static const MessageTypeT RequestDeviceTopology_2       = REQUEST_DEV_TOPOLOGY_MSG_TYPE_2;
static const MessageTypeT ReplyDeviceTopology_2         = REPLY_DEV_TOPOLOGY_MSG_TYPE_2;
static const MessageTypeT UseDHCP_2                     = USE_DHCP_MSG_TYPE_2;
static const MessageTypeT UseStatic_2                   = USE_STATIC_MSG_TYPE_2;
static const MessageTypeT NegativeAcknowledge_2         = NEGATIVE_ACKNOWLEDGE_MSG_TYPE_2;
static const MessageTypeT Undefined_2                   = UNDEFINED_MSG_TYPE_2;
static const MessageTypeT RequestCnfgRespData_2         = REQUEST_CNFG_RESP_DATA_MSG_TYPE_2;
static const MessageTypeT ReplyCnfgRespData_2           = REPLY_CNFG_RESP_DATA_MSG_TYPE_2;

// Protocol version 3
#define REQUEST_EXT_DEV_INFO_MSG_TYPE_3     { 3, 0 }
#define REPLY_EXT_DEV_INFO_MSG_TYPE_3       { 3, 1 }
#define PAIR_TO_DEV_MSG_TYPE_3              { 3, 2 }
#define REMOVE_DEV_PAIRING_MSG_TYPE_3       { 3, 3 }
#define REQUEST_DEV_TOPOLOGY_MSG_TYPE_3     { 3, 4 }
#define REPLY_DEV_TOPOLOGY_MSG_TYPE_3       { 3, 5 }
#define USE_DHCP_MSG_TYPE_3                 { 3, 6 }
#define USE_STATIC_MSG_TYPE_3               { 3, 7 }
#define NEGATIVE_ACKNOWLEDGE_MSG_TYPE_3     { 3, 8 }
#define USE_FILTERING_STRATEGY_MSG_TYPE_3   { 3, 9 }
#define LED_LOCATOR_ON_MSG_TYPE_3           { 3, 10 }
#define LED_LOCATOR_OFF_MSG_TYPE_3          { 3, 11 }
#define RESET_DEVICE_MSG_TYPE_3             { 3, 12 }
#define REQUEST_CNFG_RESP_DATA_MSG_TYPE_3   { 3, 13 }
#define REPLY_CNFG_RESP_DATA_MSG_TYPE_3     { 3, 14 }
#define UNDEFINED_MSG_TYPE_3                { 3, 15 }

static const MessageTypeT RequestExtendedDeviceInfo_3   = REQUEST_EXT_DEV_INFO_MSG_TYPE_3;
static const MessageTypeT ReplyExtendedDeviceInfo_3     = REPLY_EXT_DEV_INFO_MSG_TYPE_3;
static const MessageTypeT PairToDevice_3                = PAIR_TO_DEV_MSG_TYPE_3;
static const MessageTypeT RemoveDevicePairing_3         = REMOVE_DEV_PAIRING_MSG_TYPE_3;
static const MessageTypeT RequestDeviceTopology_3       = REQUEST_DEV_TOPOLOGY_MSG_TYPE_3;
static const MessageTypeT ReplyDeviceTopology_3         = REPLY_DEV_TOPOLOGY_MSG_TYPE_3;
static const MessageTypeT UseDHCP_3                     = USE_DHCP_MSG_TYPE_3;
static const MessageTypeT UseStatic_3                   = USE_STATIC_MSG_TYPE_3;
static const MessageTypeT NegativeAcknowledge_3         = NEGATIVE_ACKNOWLEDGE_MSG_TYPE_3;
static const MessageTypeT UseFilteringStrategy_3        = USE_FILTERING_STRATEGY_MSG_TYPE_3;
static const MessageTypeT Undefined_3                   = UNDEFINED_MSG_TYPE_3;
static const MessageTypeT LedLocatorOn_3                = LED_LOCATOR_ON_MSG_TYPE_3;
static const MessageTypeT LedLocatorOff_3               = LED_LOCATOR_OFF_MSG_TYPE_3;
static const MessageTypeT ResetDevice_3                 = RESET_DEVICE_MSG_TYPE_3;
static const MessageTypeT RequestCnfgRespData_3         = REQUEST_CNFG_RESP_DATA_MSG_TYPE_3;
static const MessageTypeT ReplyCnfgRespData_3           = REPLY_CNFG_RESP_DATA_MSG_TYPE_3;

enum CommandType
{
    REQUEST_DEVICE_INFO,
    PING,
    REQUEST_EXTENDED_DEVICE_INFO,
    PAIR_TO_DEVICE,
    REMOVE_DEVICE_PAIRING,
    REQUEST_DEVICE_TOPOLOGY,
    USE_DHCP,
    USE_STATIC_IP,
    LED_LOCATOR_ON,
    LED_LOCATOR_OFF,
    RESET_DEVICE,
    USE_FILTERING_STRATEGY,
    REQUEST_CONFIGURATION_RESPONSE_DATA
};

/*
*   Information in all packets will be stored in big-endian format
*/

// General Information contained in all command types
typedef struct GeneralInfo
{
    /// Value (0x2F03F4A2) which gives some confidence that the data
    /// which follows is an ICRON configuration message.
    uint32_t magicNumber;
    /// When the client sends a request, it chooses any value to insert
    /// in this field.  The device responding to the request will set this
    /// field in the reply to the same value it received in the request.
    uint32_t messageID;
    MessageTypeT messageType;
} __attribute__((packed)) GeneralInfoT ;

/// Stores the data returned in a reply device information message sent in
/// response to a request device information message.
typedef struct ReplyDevInfo
{
    /// General packet
    GeneralInfoT generalInfo;

    /// MAC address of the device
    uint8_t mac[6];
    /// IP address of the device
    uint32_t ipAddr;
    /// The mechanism by which this device obtains its network configuration.
    /// 0 = DHCP, 1 = Static.
    uint8_t networkAcquisitionMode;
    /// Configuration protocol version supported by the device (in addition to
    /// protocol version 0 which is supported by all devices)
    uint8_t supportedProtocol;
    /// NUL terminated UTF-8 string specifying the device's vendor
    char vendor[REPLY_DEV_INFO_VENDOR_LENGTH];
    /// NUL terminated UTF-8 string specifying the device's product name
    char product[REPLY_DEV_INFO_PRODUCT_LENGTH];
    /// NUL terminated UTF-8 string specifying the device's revision
    char revision[REPLY_DEV_INFO_REVISION_LENGTH];
} __attribute__((packed)) IcronReplyDevInfoT;

typedef struct ReplyCnfgRespData
{
    /// General packet
    GeneralInfoT generalInfo;
    /// Is Highspeed enabled
    /// 1 for enabled, 0 for disabled
    uint8_t highSpeed;
    /// Is MSA enabled
    /// 1 for enabled, 0 for disabled
    uint8_t msa;
    /// Vhub Status
    /// 1 for enabled, 0 for disabled
    uint8_t vhub;
    /// Current Filter Status
    /// 0 - no filter
    /// 1 - block mass storage devices
    /// 2 - block all but HID and Hub devices
    /// 3 - block all but HID, HUB and smartcard devices
    /// 4 - block all but Audio and Vendor Specific devices
    uint8_t currentFilterStatus;
    /// The mechanism by which this device obtains its network configuration.
    /// 0 = DHCP, 1 = Static.
    uint8_t ipAcquisitionMode;
    /// Reserved for future use
    uint8_t reserve_1[1];
    /// MAC address of the device
    uint8_t mac[6];
    /// Reserved for future use
    uint8_t reserve_2[2];
    /// MAC addresses of the extender that this device is paired with
    uint8_t pairedMacs[7][6];
    /// Port number of the connected device
    uint16_t portNumber;
    /// IP address of the device
    uint32_t ipAddr;
    /// SubNet Mask of the device
    uint32_t subNetM;
    /// Default gateway of the device
    uint32_t gateway;
    /// DHCP server of the device
    uint32_t dhcpServer;
    /// Vhub - Number of Downstream ports
    uint8_t numVhubPorts;
    /// Vhub - VID
    uint16_t vhubVid;
    /// Vhub - PID
    uint16_t vhubPid;
    /// BrandID of the device
   uint16_t brandId;
    /// NUL terminated UTF-8 string specifying the device's vendor
    char vendor[REPLY_CNFG_RESP_DATA_VENDOR_LENGTH];
    /// NUL terminated UTF-8 string specifying the device's product name
    char product[REPLY_CNFG_RESP_DATA_PRODUCT_LENGTH];
    /// NUL terminated UTF-8 string specifying the device's revision
    char revision[REPLY_CNFG_RESP_DATA_REVISION_LENGTH];
} __attribute__((packed)) IcronReplyCnfgRespDataT;

/// For Protocol version 1,
/// Stores the data returned in a reply extended device information message
/// sent in response to a request extended device information message.
typedef struct ReplyExtDevInfoVersion1
{
    /// General packet
    GeneralInfoT generalInfo;

    /// Is the device a local extender or a remote extender. 0 = LEX, 1 = REX
    uint8_t lexOrRex;
    /// Is the device currently paired to another device. 0 = NO, 1 = YES
    uint8_t isPaired;
    /// MAC address of the extender that this device is paired with
    uint8_t pairedMac[6];
} __attribute__((packed)) IcronReplyExtDevInfoTVersion1;

/// For Protocol version 2,
/// Stores the data returned in a reply extended device information message
/// sent in response to a request extended device information message.
typedef struct ReplyExtDevInfoVersion2
{
    /// General packet
    GeneralInfoT generalInfo;

    /// Is the device a local extender or a remote extender. 0 = LEX, 1 = REX
    uint8_t lexOrRex;
    /// MAC addresses of the extender that this device is paired with
    uint8_t pairedMacs[7][6];
} __attribute__((packed)) IcronReplyExtDevInfoTVersion2;

/// Stores the data that is sent in a pair to device message.
typedef struct PairToDevice
{
    /// General packet
    GeneralInfoT generalInfo;

    /// MAC address of the extender with which the current device will be paired
    uint8_t pairToMac[6];
} __attribute__((packed)) IcronPairToDeviceT;

/// For Protocol 1,
/// Stores the data that is sent in a remove device pairing message.
typedef struct RemoveDevicePairingVersion1
{
    /// General packet
    GeneralInfoT generalInfo;
} __attribute__((packed)) IcronRemoveDevicePairingTVersion1;

/// For Protocol 2,
/// Stores the data that is sent in a remove device pairing message.
typedef struct RemoveDevicePairingVersion2
{
    /// General packet
    GeneralInfoT generalInfo;

    /// MAC address of the extender from which the current device will be unpaired
    uint8_t unpairFromMac[6];
} __attribute__((packed)) IcronRemoveDevicePairingTVersion2;

/// For Protocol 1,
/// Stores the device toplogy information of a single USB device.  A request
/// device topology message returns the topology information for every USB
/// device in the system in a reply device topology message.
typedef struct ReplyDevTopologyVersion1
{
    /// General packet
    GeneralInfoT generalInfo;

    struct DevInfoVersion1 {
        /// USB address of this device
        uint8_t usbAddr;
        /// USB address of the parent of this device
        uint8_t usbParentAddr;
        /// USB hub port number which this device is attached to. Devices attached
        /// directly to a host controller will have port 0 listed as their parent.
        uint8_t portOnParent;
        /// Is this device a HUB: 0 = NO, 1 = YES
        uint8_t isHub;
    } __attribute__((packed)) devices[MAX_USB_DEVICES];

} __attribute__((packed)) IcronReplyDevTopologyTVersion1;

/// For Protocol 2,
/// Stores the device toplogy information of a single USB device.  A request
/// device topology message returns the topology information for every USB
/// device in the system in a reply device topology message.
typedef struct ReplyDevTopologyVersion2
{
    /// General packet
    GeneralInfoT generalInfo;

    struct DevInfoVersion2 {
        /// USB address of this device
        uint8_t usbAddr;
        /// USB address of the parent of this device
        uint8_t usbParentAddr;
        /// USB hub port number which this device is attached to. Devices attached
        /// directly to a host controller will have port 0 listed as their parent.
        uint8_t portOnParent;
        /// Is this device a HUB: 0 = NO, 1 = YES
        uint8_t isHub;
        /// USB Vendor ID
        uint16_t usbVendorId;
        /// USB Product ID
        uint16_t usbProductId;
    } __attribute__((packed)) devices[MAX_USB_DEVICES];

} __attribute__((packed)) IcronReplyDevTopologyTVersion2;

/// Stores the data that is sent in a use DHCP message.
typedef struct UseDHCP
{
    /// General packet
    GeneralInfoT generalInfo;

    /// MAC address of the device
    uint8_t targetMacAddress[6];
} __attribute__((packed)) UseDHCPT;

/// Stores the data that is sent in a use Static IP message.
typedef struct UseStatic
{
    /// General packet
    GeneralInfoT generalInfo;

    /// MAC address of the device
    uint8_t targetMacAddress[6];
    /// IP address of the device
    uint32_t ipAddress;
    /// Subnet Mask of the device
    uint32_t subnetMask;
    /// Default Gateway of the device
    uint32_t defaultGateway;
} __attribute__((packed)) UseStaticT;

typedef struct UseFilteringStrategy
{
    GeneralInfoT generalInfo;

    uint8_t filteringStrategy;
} __attribute__((packed)) UseFilteringStrategyT;

/// Structure for passing data from the client to the libray.
/// All data will be kept in host endian format.
/// Request packets are sent from the client to the device.
typedef struct Request
{
    struct
    {
        char ipAddress[IP4_STR_LEN];
        uint16_t udpPort;
    } general;
    enum CommandType commandType;
    union
    {
        struct
        {
        } requestDeviceInfo;

        struct
        {
        } requestConfigurationResponseData;

        struct
        {
        } ping;

        struct
        {
        } requestExtendedDeviceInfo;

        struct
        {
            uint8_t macAddressToPairTo[6];
        } pairToDevice;

        struct
        {
            uint8_t macAddressToUnpairFrom[6];
        } removeDevicePairing;

        struct
        {
        } requestDeviceTopology;

        struct
        {
            uint8_t macAddress[6];
        } useDHCP;

        struct
        {
            uint8_t macAddress[6];
            uint32_t ipAddress;
            uint32_t subnetMask;
            uint32_t defaultGateway;
        } useStatic;

        struct
        {
            uint8_t filteringStrategy;
        } useFilteringStrategy;
    } cmdSpecific;
} RequestT;

/// Structure for presenting data from the library to the client.
/// All data will be kept in host endian format except where noted.
/// Reply packets are sent from the device to the client.
typedef struct Reply
{
    union
    {
        struct
        {
            uint8_t mac[6];
            uint32_t ipAddr;    // Big-Endian format
            uint8_t networkAcquisitionMode;
            uint8_t supportedProtocol;
            char vendor[REPLY_DEV_INFO_VENDOR_LENGTH];
            char product[REPLY_DEV_INFO_PRODUCT_LENGTH];
            char revision[REPLY_DEV_INFO_REVISION_LENGTH];
        } replyDevInfo;

        struct
        {
            uint8_t highSpeed;
            uint8_t msa;
            uint8_t vhub;
            uint8_t currentFilterStatus;
            uint8_t ipAcquisitionMode;
            /// Reserved for future use
            uint8_t reserve_1[1];
            uint8_t mac[6];
            /// Reserved for future use
            uint8_t reserve_2[2];
            uint8_t pairedMacs[7][6];
            uint16_t portNumber;
            uint32_t ipAddr;    // Big-Endian format
            uint32_t subNetM;
            uint32_t gateway;
            uint32_t dhcpServer;
            uint8_t numVhubPorts;
            uint16_t vhubVid;
            uint16_t vhubPid;
            uint16_t brandId;
            char vendor[REPLY_CNFG_RESP_DATA_VENDOR_LENGTH];
            char product[REPLY_CNFG_RESP_DATA_PRODUCT_LENGTH];
            char revision[REPLY_CNFG_RESP_DATA_REVISION_LENGTH];
        } replyCnfgRespData;

        struct
        {
            uint8_t lexOrRex;
            uint8_t pairedMacs[7][6];
        } replyExtDevInfo;

        struct
        {
            uint8_t usbAddr;
            uint8_t usbParentAddr;
            uint8_t portOnParent;
            uint8_t isHub;
            _Bool doesVendorIdExist;
            uint16_t usbVendorId;
            _Bool doesProductIdExist;
            uint16_t usbProductId;
        } replyDeviceTopology;
    } u;
} ReplyT;

/// Icron NetCfg program state structure
typedef struct IcronUsbNetCfg
{
    int timedOut;
    fd_set socketFdSet;
    struct timeval selectTimeout;
    struct sockaddr_in sendTrgtAddr;
    int sendTrgtAddrLen;
    struct sockaddr_in recvTrgtAddr;
    socklen_t recvTrgtAddrLen;
    uint8_t sendMsg[XUSBNETCFG_MAX_CMD_LEN];
    uint8_t recvMsg[XUSBNETCFG_MAX_CMD_LEN];
    unsigned int sendMsgSize;
    unsigned int minRecvMsgSize;
    unsigned int maxRecvMsgSize;
    int cfgSock_fd;
    uint8_t protocolVersion;
} IcronUsbNetCfgT;

struct ExpectedResponse
{
    MessageTypeT sentMessageType;
    MessageTypeT recvMessageType1;
    MessageTypeT recvMessageType2;
};

static const struct ExpectedResponse ProtoCmdRespsMapping[] =
{

    // Sent Command,                       Expected Response,               Alternate Response
    { REQUEST_DEV_INFO_MSG_TYPE_0,         REPLY_DEV_INFO_MSG_TYPE_0,       UNDEFINED_MSG_TYPE_0 },
    { PING_MSG_TYPE_0,                     ACKNOWLEDGE_MSG_TYPE_0,          UNDEFINED_MSG_TYPE_0 },
    { REQUEST_EXT_DEV_INFO_MSG_TYPE_1,     REPLY_EXT_DEV_INFO_MSG_TYPE_1,   UNDEFINED_MSG_TYPE_1 },
    { PAIR_TO_DEV_MSG_TYPE_1,              ACKNOWLEDGE_MSG_TYPE_0,          UNDEFINED_MSG_TYPE_1 },
    { REMOVE_DEV_PAIRING_MSG_TYPE_1,       ACKNOWLEDGE_MSG_TYPE_0,          UNDEFINED_MSG_TYPE_1 },
    { REQUEST_DEV_TOPOLOGY_MSG_TYPE_1,     REPLY_DEV_TOPOLOGY_MSG_TYPE_1,   UNDEFINED_MSG_TYPE_1 },
    { USE_DHCP_MSG_TYPE_1,                 ACKNOWLEDGE_MSG_TYPE_0,          UNDEFINED_MSG_TYPE_1 },
    { USE_STATIC_MSG_TYPE_1,               ACKNOWLEDGE_MSG_TYPE_0,          UNDEFINED_MSG_TYPE_1 },
    { REQUEST_EXT_DEV_INFO_MSG_TYPE_2,     REPLY_EXT_DEV_INFO_MSG_TYPE_2,   UNDEFINED_MSG_TYPE_2 },
    { REQUEST_CNFG_RESP_DATA_MSG_TYPE_2,   REPLY_CNFG_RESP_DATA_MSG_TYPE_2, UNDEFINED_MSG_TYPE_2 },
    { PAIR_TO_DEV_MSG_TYPE_2,              ACKNOWLEDGE_MSG_TYPE_0,          NEGATIVE_ACKNOWLEDGE_MSG_TYPE_2 },
    { REMOVE_DEV_PAIRING_MSG_TYPE_2,       ACKNOWLEDGE_MSG_TYPE_0,          NEGATIVE_ACKNOWLEDGE_MSG_TYPE_2 },
    { REQUEST_DEV_TOPOLOGY_MSG_TYPE_2,     REPLY_DEV_TOPOLOGY_MSG_TYPE_2,   NEGATIVE_ACKNOWLEDGE_MSG_TYPE_2 },
    { USE_DHCP_MSG_TYPE_2,                 ACKNOWLEDGE_MSG_TYPE_0,          UNDEFINED_MSG_TYPE_2 },
    { USE_STATIC_MSG_TYPE_2,               ACKNOWLEDGE_MSG_TYPE_0,          UNDEFINED_MSG_TYPE_2 },
    { REQUEST_EXT_DEV_INFO_MSG_TYPE_3,     REPLY_EXT_DEV_INFO_MSG_TYPE_3,   UNDEFINED_MSG_TYPE_3 },
    { PAIR_TO_DEV_MSG_TYPE_3,              ACKNOWLEDGE_MSG_TYPE_0,          NEGATIVE_ACKNOWLEDGE_MSG_TYPE_3 },
    { REMOVE_DEV_PAIRING_MSG_TYPE_3,       ACKNOWLEDGE_MSG_TYPE_0,          NEGATIVE_ACKNOWLEDGE_MSG_TYPE_3 },
    { REQUEST_DEV_TOPOLOGY_MSG_TYPE_3,     REPLY_DEV_TOPOLOGY_MSG_TYPE_3,   NEGATIVE_ACKNOWLEDGE_MSG_TYPE_3 },
    { USE_DHCP_MSG_TYPE_3,                 ACKNOWLEDGE_MSG_TYPE_0,          UNDEFINED_MSG_TYPE_3 },
    { USE_STATIC_MSG_TYPE_3,               ACKNOWLEDGE_MSG_TYPE_0,          UNDEFINED_MSG_TYPE_3 },
    { LED_LOCATOR_ON_MSG_TYPE_3,           ACKNOWLEDGE_MSG_TYPE_0,          UNDEFINED_MSG_TYPE_3 },
    { LED_LOCATOR_OFF_MSG_TYPE_3,          ACKNOWLEDGE_MSG_TYPE_0,          UNDEFINED_MSG_TYPE_3 },
    { RESET_DEVICE_MSG_TYPE_3,             ACKNOWLEDGE_MSG_TYPE_0,          UNDEFINED_MSG_TYPE_3 },
    { USE_FILTERING_STRATEGY_MSG_TYPE_3,   ACKNOWLEDGE_MSG_TYPE_0,          UNDEFINED_MSG_TYPE_3 },
    { REQUEST_CNFG_RESP_DATA_MSG_TYPE_3,   REPLY_CNFG_RESP_DATA_MSG_TYPE_3, UNDEFINED_MSG_TYPE_3 }
};

/**
* @brief    Initializes Icron USB Network Configuration
*
* @return   Pointer to an IcronUsbNetCfgT structure, NULL on failure
*/
IcronUsbNetCfgT* ICRON_usbNetCfgInit(void);


/**
* @brief    Frees resources allocated by Icron USB Network Config
*
* @param[in,out] netCfg Pointer to a previously intialized IcronUsbNetCfgT
*                       structure
*
* @return   0 on success, -1 on failure
*/
int ICRON_usbNetCfgRelease(struct IcronUsbNetCfg *netCfg);


/**
* @brief    Probes the network for Icron LEXs and REXs.
*
* @param[in,out] netCfg     Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in] cmd            Structure that contains:
*                           - targetIPAddr
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for responses from the network
* @param[out] reply         Structure that contains:
*                           - MAC Address
*                           - IP Address
*                           - Network Acquisition Mode
*                           - Supported Protocol Version
*                           - Vendor String
*                           - Product String
*                           - Revision Number
*
* @return   numDevices  Number of devices found
*           -1          Error
*/
int ICRON_requestDevInfo(IcronUsbNetCfgT *currCfg,
                         RequestT cmd,
                         uint32_t msgId,
                         int timeOut,
                         ReplyT *reply);


/**
* @brief    Loads the user provided IcronConfigRespDataT structure with data
*           corresponding to the requested item.
* @param[in,out] netCfg     Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in] cmd            Structure that contains:
*                           - targetIPAddr
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for responses from the network
* @param[out] reply         Structure that contains:
*                           - USB2 High speed
*                           - Support for MSA
*                           - Vhub status
*                           - Filter status
*                           - IP Acquisition Mode
*                           - MAC Address
*                           - Mac Address of the paired unit
*                           - Port Number
*                           - IP Address
*                           - Subnet Mask
*                           - Defualt Gateway
*                           - DHCP server
*                           - Vhub - number of downstream ports
*                           - Vhub - VID
*                           - Vhub - PID
*                           - Brand ID
*                           - Brand String
*                           - Product Name
*                           - Revision Number

*
* @return   numDevices  Number of devices found
*           -1          Error
*/
int ICRON_requestCnfgRespData(IcronUsbNetCfgT *currCfg,
                         RequestT cmd,
                         uint32_t msgId,
                         int timeOut,
                         ReplyT *reply);


/**
* @brief   Sends a ping message to the Extender
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in] cmd            Structure that contains:
*                           - targetIPAddr
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for response from device
*
* @return   0   success
*          -1   failure
*/
int ICRON_pingDevice(IcronUsbNetCfgT *currCfg,
                     RequestT cmd,
                     uint32_t msgId,
                     int timeOut);

/**
* @brief    Loads the user provided IcronExtendedDevInfoT structure with data
*           corresponding to the requested item.
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in]  cmd           Structure that contains:
*                           - targetIPAddr
* @param[in]  msgId         A user provided message Id
* @param[in]  timeOut       Time to wait for a response to the request
* @param[out] reply         Structure that contains:
*                           - lexOrRex
*                           - 0 to 7 paired MAC Addresses
*
* @return   numPairedDevices    number of paired MAC Addresses
*           lexOrRex            1 - "REX" or 0 - "LEX"
*/
int ICRON_getExtendedDevInfo(IcronUsbNetCfgT *currCfg,
                             RequestT cmd,
                             uint32_t msgId,
                             int timeOut,
                             ReplyT *reply);

/**
* @brief    Sends a Pair To Device request containing the user provided MAC
*           address, to the specified device and waits for the device to
*           acknowledge the request
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in]  cmd           Structure that contains:
*                           - targetIPAddr
*                           - pairToMac
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for a response to the request
*
* @return  1    Device paired succesfully.
*          2    Device pairing failed because either the device is already
*               paired or all vports are in use.
*         -1    Pair To Device request failed.
*/
int ICRON_pairToDevice(IcronUsbNetCfgT *currCfg,
                       RequestT cmd,
                       uint32_t msgId,
                       int timeOut);

/**
* @brief    Sends a Remove Device Pairing request to the specified device and
*           waits for it to acknowledge the request
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in]  cmd           Structure that contains:
*                           - targetIPAddr
*                           - unpairFromMac
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for a response to the request
*
* @return  1    Device pairing removed succesfully.
*          2    The two specified devices are not paired to each other.
*         -1    Remove device pairing request failed.
*/
int ICRON_removeDevicePairing(IcronUsbNetCfgT *currCfg,
                              RequestT cmd,
                              uint32_t msgId,
                              int timeOut);

/**
* @brief    Loads the user provided array of IcronDevTopologyT structures with
*           topology information returned from the specified LEX.  The array
*           should contain MAX_USB_DEVICES elements, the maximum number of USB
*           devices supported by a LEX.
*
* @param[in,out] currCfg     Pointer to a previously intialized IcronUsbNetCfgT
*                            structure.
* @param[in]  cmd            Structure that contains:
*                            - targetIPAddr
* @param[in]  msgId          A user provided message Id.
* @param[in]  timeOut        Time to wait for a response to the request.
* @param[out] reply          Structure that contains:
*                            - USB Address
*                            - USB Parent Address
*                            - Port On Parent
*                            - isHub
*                            - USB Product ID in Protocol 2
*                            - USB Vendor ID in Protocol 2
*
* @return  numDevices   0 or more device records
*          -2           Extender Device is REX
*          others       Error
*/
int ICRON_getDevTopology(IcronUsbNetCfgT *currCfg,
                         RequestT cmd,
                         uint32_t msgId,
                         int timeOut,
                         ReplyT *reply);

/**
* @brief    Sends a message to configure a device to use DHCP as the method to
*           obtain IP, subnet and gateway information.
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in] cmd            Structure that contains:
*                           - targetIPAddr
*                           - macOfDeviceToConfigure
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for a response to the request
*
* @return   1    Device set to use a static IP successfully
*           0    Specified MAC Address is not valid for the target IP Address
*          -1    Error
*/
int ICRON_useDHCP(IcronUsbNetCfgT *currCfg,
                  RequestT cmd,
                  uint32_t msgId,
                  int timeOut);

/**
* @brief    Sends a message to configure a device to use a static IP
*           configuration.
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in] cmd            Structure that contains:
*                           - targetIPAddr
*                           - macOfDeviceToConfigure
*                           - newIPAddress
*                           - newSubnetMask
*                           - newDefaultGateway
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for a response to the request
*
* @return   1    Device set to use a static IP successfully
*           0    Specified MAC Address is not valid for the target IP Address
*          -1    Error
*/
int ICRON_useStaticIP(IcronUsbNetCfgT *currCfg,
                      RequestT cmd,
                      uint32_t msgId,
                      int timeOut);

/**
* @brief    Sends a message to set the filtering strategy on the device.
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in] cmd            Structure that contains:
*                           - targetIPAddr
*                           - filtering strategy
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for a response to the request
*
* @return   1    Device filtering strategy was set successfully
*           0    Device filtering strategy was not set
*          -1    Error
*/
int ICRON_useFilteringStrategy(
        IcronUsbNetCfgT *currCfg,
        RequestT cmd,
        uint32_t msgId,
        int timeOut);


/**
* @brief    Sends a request to the specified device and executes specified commands
*
* @param[in,out] currCfg        Pointer to a previously intialized IcronUsbNetCfgT
*                               structure
* @param[in]  cmd               Structure that contains:
*                               - targetIPAddr
* @param[in] msgId              A user provided message Id
* @param[in] timeOut            Time to wait for a response to the request
* @param[in] sentMessageType    The type of command to be executed
*
* @return  0    Device located succesfully and command executed.
*         -1    Could not locate the device because UDP packet was not received.
*/
int SendControlCommand(
            IcronUsbNetCfgT *currCfg,
            struct Request request,
            uint32_t msgId,
            int timeOut,
            MessageTypeT sentMessageType);


#endif // XUSBNETCFG_H
