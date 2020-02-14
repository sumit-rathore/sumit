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
//!   @file  - netcfg_packet_handler.c
//
//!   @brief - Provides a function for handling incoming UDP configuration
//             requests.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <netcfg_packet_handler.h>
#include "netcfg_log.h"
#include "netcfg_loc.h"
#include <net_ethernet.h>
#include <net_base.h>
#include <grg.h>
#include <grg_led.h>
#include <topology.h>
#include <storage_vars.h>
#include <storage_Data.h>
#include <linkmgr.h>
#include <net_dhcp.h>
#include <timing_timers.h>

/************************ Defined Constants and Macros ***********************/
// Timeout value in milliseconds for the response of _CMD_THREE_RESET_DEVICE
//#define _RESET_DEVICE_RESPONSE_TIMER       10
#define _VENDOR_STRING_LENGTH     32
#define _PRODUCT_STRING_LENGTH    32
#define _REVISION_STRING_LENGTH   12

// Network configuration packed data offsets - general
#define _MAGIC_NUMBER_OFFSET        0
#define _MESSAGE_ID_OFFSET          (_MAGIC_NUMBER_OFFSET + 4)
#define _PROTOCOL_VERSION_OFFSET    (_MESSAGE_ID_OFFSET + 4)
#define _COMMAND_OFFSET             (_PROTOCOL_VERSION_OFFSET + 1)
#define _ZERO_OFFSET                (_COMMAND_OFFSET + 1)
#define _THREE_OFFSET               (_COMMAND_OFFSET + 1)

// Network configuration packed data offsets - proto. ver. 0
#define _REPLY_DEVICE_INFORMATION_MAC_ADDRESS_OFFSET                (_ZERO_OFFSET)
#define _REPLY_DEVICE_INFORMATION_IP_ADDRESS_OFFSET                 (_REPLY_DEVICE_INFORMATION_MAC_ADDRESS_OFFSET + 6)
#define _REPLY_DEVICE_INFORMATION_NETWORK_ACQUISITION_MODE_OFFSET   (_REPLY_DEVICE_INFORMATION_IP_ADDRESS_OFFSET + 4)
#define _REPLY_DEVICE_INFORMATION_SUPPORTED_PROTO_VER_OFFSET        (_REPLY_DEVICE_INFORMATION_NETWORK_ACQUISITION_MODE_OFFSET + 1)
#define _REPLY_DEVICE_INFORMATION_VENDOR_OFFSET                     (_REPLY_DEVICE_INFORMATION_SUPPORTED_PROTO_VER_OFFSET + 1)
#define _REPLY_DEVICE_INFORMATION_PRODUCT_OFFSET                    (_REPLY_DEVICE_INFORMATION_VENDOR_OFFSET + _VENDOR_STRING_LENGTH)
#define _REPLY_DEVICE_INFORMATION_REVISION_OFFSET                   (_REPLY_DEVICE_INFORMATION_PRODUCT_OFFSET + _PRODUCT_STRING_LENGTH)

// Network configuration packed data offsets - proto. ver. 3
#define _REPLY_EXTENDED_DEVICE_INFORMATION_LEXREX_OFFSET            (_THREE_OFFSET)
#define _REPLY_EXTENDED_DEVICE_INFORMATION_PAIRED_WITH_MAC_OFFSET(pairingNum)   (_REPLY_EXTENDED_DEVICE_INFORMATION_LEXREX_OFFSET + 1 + (pairingNum * 6))
#define _PAIR_TO_DEVICE_MAC_ADDRESS_OFFSET                          (_THREE_OFFSET)
#define _REMOVE_DEVICE_PAIRING_PAIRED_MAC_ADDRESS_OFFSET            (_THREE_OFFSET)
#define _REPLY_DEVICE_TOPOLOGY_DEVICES_OFFSET                       (_THREE_OFFSET)
#define _REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum)             (_REPLY_DEVICE_TOPOLOGY_DEVICES_OFFSET + (deviceNum * 8))
#define _REPLY_DEVICE_TOPOLOGY_USB_ADDR_OFFSET(deviceNum)           (_REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum) + 0)
#define _REPLY_DEVICE_TOPOLOGY_USB_ADDR_PARENT_OFFSET(deviceNum)    (_REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum) + 1)
#define _REPLY_DEVICE_TOPOLOGY_PORT_ON_PARENT_OFFSET(deviceNum)     (_REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum) + 2)
#define _REPLY_DEVICE_TOPOLOGY_IS_DEVICE_HUB_OFFSET(deviceNum)      (_REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum) + 3)
#define _REPLY_DEVICE_TOPOLOGY_VENDOR_ID_OFFSET(deviceNum)          (_REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum) + 4)
#define _REPLY_DEVICE_TOPOLOGY_PRODUCT_ID_OFFSET(deviceNum)         (_REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum) + 6)
#define _USE_DHCP_TARGET_MAC_ADDRESSS_OFFSET                        (_THREE_OFFSET)
#define _USE_STATIC_IP_TARGET_MAC_ADDRESSS_OFFSET                   (_THREE_OFFSET)
#define _USE_STATIC_IP_IPV4_ADDRESSS_OFFSET                         (_USE_STATIC_IP_TARGET_MAC_ADDRESSS_OFFSET + 6)
#define _USE_STATIC_IP_SUBNET_MASK_OFFSET                           (_USE_STATIC_IP_IPV4_ADDRESSS_OFFSET + 4)
#define _USE_STATIC_IP_DEFAULT_GATEWAY_OFFSET                       (_USE_STATIC_IP_SUBNET_MASK_OFFSET + 4)
#define _USE_FILTERING_STRATEGY_FILTERING_STRATEGY_OFFSET           (_THREE_OFFSET)


#define _MAGIC_NUMBER 0x2f03f4a2

#define _LEXREX_LEX                           0
#define _LEXREX_REX                           1

#define _SUPPORTED_PROTOCOL_VERSION 3

// protocol zero commands
#define _CMD_ZERO_REQUEST_DEVICE_INFORMATION  0
#define _CMD_ZERO_REPLY_DEVICE_INFORMATION    1
#define _CMD_ZERO_PING                        2
#define _CMD_ZERO_ACKNOWLEDGE                 3
#define _CMD_ZERO_REQUEST_CONFIGURATION_RESPONSE_DATA 4
#define _CMD_ZERO_REPLY_CONFIGURATION_RESPONSE_DATA 5

// protocol three commands
#define _CMD_THREE_REQUEST_EXTENDED_DEVICE_INFORMATION  0
#define _CMD_THREE_REPLY_EXTENDED_DEVICE_INFORMATION    1
#define _CMD_THREE_PAIR_TO_DEVICE                       2
#define _CMD_THREE_REMOVE_DEVICE_PAIRING                3
#define _CMD_THREE_REQUEST_DEVICE_TOPOLOGY              4
#define _CMD_THREE_REPLY_DEVICE_TOPOLOGY                5
#define _CMD_THREE_USE_DHCP                             6
#define _CMD_THREE_USE_STATIC_IP                        7
#define _CMD_THREE_NEGATIVE_ACKNOWLEDGE                 8
#define _CMD_THREE_USE_FILTERING_STRATEGY               9
#define _CMD_THREE_LED_LOCATOR_ON                       10
#define _CMD_THREE_LED_LOCATOR_OFF                       11
#define _CMD_THREE_RESET_DEVICE                         12
#define _CMD_THREE_REQUEST_CONFIGURATION_RESPONSE_DATA  13
#define _CMD_THREE_REPLY_CONFIGURATION_RESPONSE_DATA    14

/******************************** Data Types *********************************/
typedef struct MessageType
{
    /// An integer from 0-255.  All devices will support protocol 0 and
    /// one other protocol version.
    uint8 protocolVersion;
    /// An integer from 0-255.  This is the identifier of the command.
    uint8 command;
} __attribute__((packed)) MessageTypeT;

typedef struct GeneralInfo
{
    /// Value (0x2F03F4A2) which gives some confidence that the data
    /// which follows is an ICRON configuration message.
    uint32 magicNumber;
    /// When the client sends a request, it chooses any value to insert
    /// in this field.  The device responding to the request will set this
    /// field in the reply to the same value it received in the request.
    uint32 messageID;
    MessageTypeT messageType;
} __attribute__((packed)) GeneralInfoT ;

// Structure representing reply of request_configuration_response_data
typedef struct ReplyCnfgRespData
{
    /// General packet
    GeneralInfoT generalInfo;
    /// Is Highspeed enabled
    /// 1 for enabled, 0 for disabled
    uint8 highSpeed;
    /// Is MSA enabled
    /// 1 for enabled, 0 for disabled
    uint8 msa;
    /// Vhub Status
    /// 1 for enabled, 0 for disabled
    uint8 vhub;
    /// Current Filter Status
    /// 0 - no filter
    /// 1 - block non ISO devices
    /// 2 - block mass storage devices
    /// 3 - block all but HID and Hub devices
    /// 4 - block all but HID, HUB and smartcard devices
    /// 5 - block all but Audio and Vendor Specific devices
    uint8 currentFilterStatus;
    /// The mechanism by which this device obtains its network configuration.
    /// 0 = DHCP, 1 = Static.
    uint8 ipAcquisitionMode;
    /// Reserved for future use
    uint8 reserve_1[1];
    /// MAC address of the device
    uint8 mac[6];
    /// Reserved for future use
    uint8 reserve_2[2];
    /// MAC address of the paired devices
    uint8 pairedMacs[7][6];
    /// Port number of the connected unit
    uint16 portNumber;
    /// IP address of the device
    uint8 ipAddr[4];
    /// SubNet Mask of the device
    uint8 subNetM[4];
    /// Default gateway of the device
    uint8 gateway[4];
    /// DHCP server of the device
    uint8 dhcpServer[4];
    /// Vhub - Number of Downstream ports
    uint8 numVhubPorts;
    /// Reserved for future use
    uint8 reserve_3[1];
    /// Vhub - VID
    uint16 vhubVid;
    /// Vhub - PID
    uint16 vhubPid;
    /// BrandID of the device
    uint16 brandId;
    /// NUL terminated UTF-8 string specifying the device's vendor
    uint8 vendor[_VENDOR_STRING_LENGTH];
    /// NUL terminated UTF-8 string specifying the device's product name
    char product[_PRODUCT_STRING_LENGTH];
    /// NUL terminated UTF-8 string specifying the device's revision
    char revision[_REVISION_STRING_LENGTH];
} __attribute__((packed)) IcronReplyCnfgRespDataT;


// This type is used for context passed back to the
// _topologyDescriptionCallback to allow that function to fill in the response
// buffer with the topology details.
struct DeviceTopologyResponseContext
{
    uint8* responseBuffer;
    uint8 currentDeviceIndex;
};

// This struct stores a function point and the data that the stored function
// will operate on.  The problem we encountered was that sometimes actions may
// need to be taken after the response packet has been sent back to the client.
// use_static_ip is an example of this.  The client was not getting the
// Acknowledge packet because it was being sent from the device with the new IP
// address instead of the old one.
static struct OnCompletionData
{
    void (*onCompletionFunction)(void);
    union
    {
        struct
        {
            NET_IPAddress ip;
            NET_IPAddress subnetMask;
            NET_IPAddress defaultGateway;
        } useStaticIP;
    } params;
} onCompletionData;

enum _NETCFG_FilteringStrategy
{
    _Allow_All_Devices = 0,
    _Block_All_Devices_Except_HID_And_Hub,
    _Block_Mass_Storage,
    _Block_All_Devices_Except_HID_Hub_And_Smartcard,
    _Block_All_Devices_Except_Audio_And_Vendor_Specific
};

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

static uint16 Icron_handleMessageAndCreateResponse(
    uint8* input, uint16 inputLength, uint8** response) __attribute__((section(".icron_xusbcfg_text")));
static uint16 Icron_requestDeviceInformation(uint8** response) __attribute__((section(".icron_xusbcfg_text")));
static uint16 Icron_ping(uint8** response) __attribute__((section(".icron_xusbcfg_text")));
static uint16 Icron_requestExtendedDeviceInformation(uint8** response) __attribute__((section(".icron_xusbcfg_text")));
static uint16 Icron_requestDeviceTopology(uint8** response) __attribute__((section(".icron_xusbcfg_text")));
static uint16 Icron_pairToDevice(
    NET_MACAddress pairToDeviceMACAddress, uint8** response) __attribute__((section(".icron_xusbcfg_text")));
static uint16 Icron_removeDevicePairing(
    NET_MACAddress linkedMACAddress, uint8** response) __attribute__((section(".icron_xusbcfg_text")));
static uint16 Icron_useDHCP(uint8** response) __attribute__((section(".icron_xusbcfg_text")));
static uint16 Icron_useStaticIP(
    NET_IPAddress ip,
    NET_IPAddress subnetMask,
    NET_IPAddress defaultGateway,
    uint8** response) __attribute__((section(".icron_xusbcfg_text")));
static uint16 Icron_useFilteringStrategy(
    enum _NETCFG_FilteringStrategy filteringStrategy,
    uint8** response) __attribute__((section(".icron_xusbcfg_text")));
static uint16 Icron_requestDeviceLocator(bool enable, uint8** response) __attribute__((section(".icron_xusbcfg_text")));
static uint16 Icron_resetDevice(uint8** response) __attribute__((section(".icron_xusbcfg_text")));
static uint16 Icron_requestConfigurationResponseData(uint8** response) __attribute__((section(".icron_xusbcfg_text")));
static void Icron_completeSettingIP(void) __attribute__((section(".icron_xusbcfg_text")));
static void Icron_completeSettingDHCP(void) __attribute__((section(".icron_xusbcfg_text")));
static boolT Icron_isLengthAndCommandCorrect(
    uint8 protocolVersion,
    uint8 command,
    uint16 inputLength) __attribute__((section(".icron_xusbcfg_text")));
static void Icron_setNetworkAcquisitionModeInFlash(
    enum TOPLEVEL_VarNetworkAcquisitionMode mode) __attribute__((section(".icron_xusbcfg_text")));


/***************** External Visibility Function Definitions ******************/


/***************** Component Visibility Function Definitions *****************/

/**
* FUNCTION NAME: NETCFG_Icron_getNetworkAcquisitionModeFromFlash()
*
* @brief  - Gets the network acquisition mode setting out of flash storage.
*
* @return - 0 means DHCP mode or 1 means static IP allocation.
*
* @note   - This function will return 0 -> DHCP in the case that
*           NETWORK_ACQUISITION_MODE is not set in flash.
*/
enum TOPLEVEL_VarNetworkAcquisitionMode NETCFG_Icron_getNetworkAcquisitionModeFromFlash(void)
{
    union STORAGE_VariableData* networkAcquisitionMode;
    if(!STORAGE_varExists(NETWORK_ACQUISITION_MODE))
    {
        networkAcquisitionMode = STORAGE_varCreate(NETWORK_ACQUISITION_MODE);
        // Default to DHCP
        networkAcquisitionMode->bytes[0] = TOPLEVEL_NETWORK_ACQUISITION_DHCP;
        STORAGE_varSave(NETWORK_ACQUISITION_MODE);
    }
    else
    {
        networkAcquisitionMode = STORAGE_varGet(NETWORK_ACQUISITION_MODE);
    }
    return networkAcquisitionMode->bytes[0];
}

/******************** File Visibility Function Definitions *******************/

/**
* FUNCTION NAME: NETCFG_Icron_handleUDPPacket()
*
* @brief  - Handles a UDP traffic sent to the port bound to the network based
*           device configuration functionality.
*
* @return - void
*/
void NETCFG_Icron_handleUDPPacket(
    NET_UDPPort destinationPort,
    NET_IPAddress senderIP,
    NET_UDPPort senderPort,
    uint8* data,
    uint16 dataLength)
{

    // Reset the onCompletion function pointer to NULL so that we don't just
    // call the function from last time
    onCompletionData.onCompletionFunction = NULL;

    // Do very preliminary sanity checking: Is the length long enough to
    // contain valid data and is the magic number correct.
    if(dataLength >= (_COMMAND_OFFSET + 1))
    {
        if(NET_unpack32Bits(&data[_MAGIC_NUMBER_OFFSET]) == _MAGIC_NUMBER)
        {
            uint8* response = NULL;
            uint16 responseLength =
                Icron_handleMessageAndCreateResponse(data, dataLength, &response);
            if(response != NULL && responseLength > 0)
            {
                NET_pack32Bits(
                    _MAGIC_NUMBER,
                    &response[_MAGIC_NUMBER_OFFSET]);

                NET_pack32Bits(
                    NET_unpack32Bits(&data[_MESSAGE_ID_OFFSET]),
                    &response[_MESSAGE_ID_OFFSET]);


                NET_udpTransmitPacket(
                    response,
                    senderIP,
                    responseLength,
                    STORAGE_varGet(NETCFG_PORT_NUMBER)->doubleWord,
                    senderPort);

                ilog_NETCFG_COMPONENT_1(ILOG_MINOR_EVENT, UDPpacket, senderIP);

                if(onCompletionData.onCompletionFunction)
                {
                    (*onCompletionData.onCompletionFunction)();
                }
            }
        }
    }
}

/**
* FUNCTION NAME: Icron_isLengthAndCommandCorrect()
*
* @brief  - Checks to see if the received data length is appropriate for the
*           command received.
*
* @return - TRUE if the length is correct for the given command and the command
*           is one that is expected to be received by a LEX or REX (i.e. not a
*           device to client command).
*/
static boolT Icron_isLengthAndCommandCorrect(
    uint8 protocolVersion, uint8 command, uint16 inputLength)
{
    uint16 expectedLength = 0;

    switch(protocolVersion)
    {
        case 0:
            switch(command)
            {
                case _CMD_ZERO_REQUEST_DEVICE_INFORMATION:
                case _CMD_ZERO_PING:
                    expectedLength = _ZERO_OFFSET;
                    break;

                // LEX and REX devices do not accept these commands
                case _CMD_ZERO_REPLY_DEVICE_INFORMATION:
                case _CMD_ZERO_ACKNOWLEDGE:
                    break;

                default:
                    break;
            }
            break;

        case _SUPPORTED_PROTOCOL_VERSION:
            switch(command)
            {
                case _CMD_THREE_REQUEST_EXTENDED_DEVICE_INFORMATION:
                case _CMD_THREE_REQUEST_DEVICE_TOPOLOGY:
                case _CMD_THREE_REQUEST_CONFIGURATION_RESPONSE_DATA:
                case _CMD_THREE_LED_LOCATOR_ON:
                case _CMD_THREE_LED_LOCATOR_OFF:
                case _CMD_THREE_RESET_DEVICE:
                    expectedLength = _THREE_OFFSET;
                    break;

                case _CMD_THREE_PAIR_TO_DEVICE:
                    expectedLength = _PAIR_TO_DEVICE_MAC_ADDRESS_OFFSET + 6;
                    break;

                case _CMD_THREE_REMOVE_DEVICE_PAIRING:
                    expectedLength = _REMOVE_DEVICE_PAIRING_PAIRED_MAC_ADDRESS_OFFSET + 6;
                    break;

                case _CMD_THREE_USE_DHCP:
                    expectedLength = _USE_DHCP_TARGET_MAC_ADDRESSS_OFFSET + 6;
                    break;

                case _CMD_THREE_USE_STATIC_IP:
                    expectedLength = _USE_STATIC_IP_DEFAULT_GATEWAY_OFFSET + 4;
                    break;

                case _CMD_THREE_USE_FILTERING_STRATEGY:
                    expectedLength = _USE_FILTERING_STRATEGY_FILTERING_STRATEGY_OFFSET + 1;
                    break;

                // LEX and REX devices do not accept these commands
                case _CMD_THREE_REPLY_EXTENDED_DEVICE_INFORMATION:
                case _CMD_THREE_REPLY_DEVICE_TOPOLOGY:
                case _CMD_THREE_REPLY_CONFIGURATION_RESPONSE_DATA:
                case _CMD_THREE_NEGATIVE_ACKNOWLEDGE:
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
    return expectedLength != 0 && inputLength == expectedLength;
}

/**
* FUNCTION NAME: Icron_handleMessageAndCreateResponse()
*
* @brief  - Processes the incoming message and allocates a response and fills
*           in the data.
*
* @return - The length of the data in response.
*
* @note   - If no response is to be sent, the output parameter response will be
*           a NULL pointer.
*/
static uint16 Icron_handleMessageAndCreateResponse(
    uint8* input, uint16 inputLength, uint8** response)
{
    uint16 responseLength;
    *response = NET_udpAllocateBuffer();

    if(!Icron_isLengthAndCommandCorrect(
            input[_PROTOCOL_VERSION_OFFSET],
            input[_COMMAND_OFFSET],
            inputLength))
    {
        goto abortResponse;
    }

    switch(input[_PROTOCOL_VERSION_OFFSET])
    {
        case 0:
            switch(input[_COMMAND_OFFSET])
            {
                case _CMD_ZERO_REQUEST_DEVICE_INFORMATION:
                    responseLength = Icron_requestDeviceInformation(response);
                    break;

                case _CMD_ZERO_PING:
                    responseLength = Icron_ping(response);
                    break;

                case _CMD_ZERO_ACKNOWLEDGE:
                case _CMD_ZERO_REPLY_DEVICE_INFORMATION:
                    // TODO: log that these commands aren't valid for a LEX or
                    // REX
                    goto abortResponse;

                default:
                    goto abortResponse;
            }
            break;

        case _SUPPORTED_PROTOCOL_VERSION:
            switch(input[_COMMAND_OFFSET])
            {
                case _CMD_THREE_REQUEST_EXTENDED_DEVICE_INFORMATION:
                    responseLength =
                        Icron_requestExtendedDeviceInformation(response);
                    break;

                case _CMD_THREE_REQUEST_CONFIGURATION_RESPONSE_DATA:
                    responseLength =
                        Icron_requestConfigurationResponseData(response);
                    break;

                case _CMD_THREE_REQUEST_DEVICE_TOPOLOGY:
                    responseLength = Icron_requestDeviceTopology(response);
                    break;

                case _CMD_THREE_PAIR_TO_DEVICE:
                    {
                        NET_MACAddress pairToDeviceMACAddress =
                            NET_unpack48Bits(&input[_PAIR_TO_DEVICE_MAC_ADDRESS_OFFSET]);
                        responseLength =
                            Icron_pairToDevice(pairToDeviceMACAddress, response);
                    }
                    break;

                case _CMD_THREE_REMOVE_DEVICE_PAIRING:
                    {
                        NET_MACAddress pairedMACAddress =
                            NET_unpack48Bits(&input[_REMOVE_DEVICE_PAIRING_PAIRED_MAC_ADDRESS_OFFSET]);
                        responseLength = Icron_removeDevicePairing(pairedMACAddress, response);
                    }
                    break;

                case _CMD_THREE_USE_DHCP:
                    {
                        NET_MACAddress targetMACAddress =
                            NET_unpack48Bits(&input[_USE_DHCP_TARGET_MAC_ADDRESSS_OFFSET]);
                        if(targetMACAddress == NET_ethernetGetSelfMACAddress())
                        {
                            responseLength = Icron_useDHCP(response);
                        }
                        else
                        {
                            goto abortResponse;
                        }
                    }
                    break;

                case _CMD_THREE_USE_STATIC_IP:
                    {
                        NET_MACAddress targetMACAddress =
                            NET_unpack48Bits(&input[_USE_STATIC_IP_TARGET_MAC_ADDRESSS_OFFSET]);
                        if(targetMACAddress == NET_ethernetGetSelfMACAddress())
                        {
                            NET_IPAddress ip =
                                NET_unpack32Bits(&input[_USE_STATIC_IP_IPV4_ADDRESSS_OFFSET]);
                            NET_IPAddress subnetMask =
                                NET_unpack32Bits(&input[_USE_STATIC_IP_SUBNET_MASK_OFFSET]);
                            NET_IPAddress gateway =
                                NET_unpack32Bits(&input[_USE_STATIC_IP_DEFAULT_GATEWAY_OFFSET]);
                            responseLength = Icron_useStaticIP(ip, subnetMask, gateway, response);
                        }
                        else
                        {
                            goto abortResponse;
                        }
                    }
                    break;

                case _CMD_THREE_USE_FILTERING_STRATEGY:
                    {
                        enum _NETCFG_FilteringStrategy filteringStrategy =
                            input[_USE_FILTERING_STRATEGY_FILTERING_STRATEGY_OFFSET];

                        responseLength = Icron_useFilteringStrategy(filteringStrategy, response);
                    }
                    break;

                case _CMD_THREE_LED_LOCATOR_ON:
                    responseLength = Icron_requestDeviceLocator(true, response);
                    break;

                case _CMD_THREE_LED_LOCATOR_OFF:
                    responseLength = Icron_requestDeviceLocator(false, response);
                    break;

                case _CMD_THREE_RESET_DEVICE:
                    responseLength = Icron_resetDevice(response);
                    break;


                case _CMD_THREE_REPLY_EXTENDED_DEVICE_INFORMATION:
                case _CMD_THREE_REPLY_DEVICE_TOPOLOGY:
                case _CMD_THREE_REPLY_CONFIGURATION_RESPONSE_DATA:
                    // TODO: log that these commands aren't valid for a LEX or
                    // REX
                    goto abortResponse;

                default:
                    goto abortResponse;
            }
            break;

        default:
            goto abortResponse;
    }

    return responseLength;

abortResponse:
    NET_udpFreeBuffer(*response);
    *response = NULL;
    return 0;
}


/**
* FUNCTION NAME: Icron_requestDeviceInformation()
*
* @brief  - Creates a reply device information message in response to a request
*           device information message.
*
* @return - The length of the data in response.
*/
static uint16 Icron_requestDeviceInformation(uint8** response)
{
    static char product[] __attribute__((section(".data"))) = "USB Over Network";
    static char revision[] __attribute__((section(".data"))) = SOFTWARE_REVISION_STR;

    (*response)[_PROTOCOL_VERSION_OFFSET] = 0;
    (*response)[_COMMAND_OFFSET] = _CMD_ZERO_REPLY_DEVICE_INFORMATION;

    NET_pack48Bits(
        NET_ethernetGetSelfMACAddress(),
        &(*response)[_REPLY_DEVICE_INFORMATION_MAC_ADDRESS_OFFSET]);

    NET_pack32Bits(
        NET_ipv4GetIPAddress(),
        &(*response)[_REPLY_DEVICE_INFORMATION_IP_ADDRESS_OFFSET]);

    (*response)[_REPLY_DEVICE_INFORMATION_NETWORK_ACQUISITION_MODE_OFFSET] =
        (uint8)NETCFG_Icron_getNetworkAcquisitionModeFromFlash();

    (*response)[_REPLY_DEVICE_INFORMATION_SUPPORTED_PROTO_VER_OFFSET] =
        _SUPPORTED_PROTOCOL_VERSION;

    TOPLEVEL_copyBrandName(&(*response)[_REPLY_DEVICE_INFORMATION_VENDOR_OFFSET]);
    memcpy(&(*response)[_REPLY_DEVICE_INFORMATION_PRODUCT_OFFSET], product, sizeof(product));
    memcpy(&(*response)[_REPLY_DEVICE_INFORMATION_REVISION_OFFSET], revision, sizeof(revision));

    return (_REPLY_DEVICE_INFORMATION_REVISION_OFFSET + _REVISION_STRING_LENGTH);
}

/**
* FUNCTION NAME: Icron_requestConfigurationResponseData()
*
* @brief  - Creates a reply configuration response data message in response to a request
*           configuration response data message.
*
* @return - The length of the data in response.
*/
static uint16 Icron_requestConfigurationResponseData(uint8** response)
{
    // start Vport number of paired devices when Vhub is enabled
    uint8 startVport;
    // end Vport number of paired devices when Vhub is enabled
    uint8 endVport;
    // number of paired devices when Vhub is enabled
    uint8 numPairsFound = 0;
    // product name string
    static char product[] __attribute__((section(".data"))) = "USB Over Network";
    // revision number string
    static char revision[] __attribute__((section(".data"))) = SOFTWARE_REVISION_STR;

    // use structure pointer CnfgRD to navigate through ReplyCnfgRespData structure
    IcronReplyCnfgRespDataT *CnfgRD = (IcronReplyCnfgRespDataT *)*response;

    CnfgRD->generalInfo.messageType.protocolVersion = _SUPPORTED_PROTOCOL_VERSION;
    CnfgRD->generalInfo.messageType.command = _CMD_THREE_REPLY_CONFIGURATION_RESPONSE_DATA;

    CnfgRD->highSpeed = ((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >> TOPLEVEL_SUPPORT_USB2_HISPEED_OFFSET) & 0x1);
    CnfgRD->msa = ((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >> TOPLEVEL_SUPPORT_MSA_OFFSET) & 0x1);
    CnfgRD->vhub = ((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >> TOPLEVEL_ENABLE_VHUB_OFFSET) & 0x1);

    // check the filter status and send the respective enum type as response
    if (((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >> TOPLEVEL_BLOCK_MASS_STORAGE_OFFSET) & 0x1) == 1)
    {
        CnfgRD->currentFilterStatus = _Block_Mass_Storage;
    }
    else if (((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >> TOPLEVEL_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET) & 0x1) == 1)
    {
        CnfgRD->currentFilterStatus = _Block_All_Devices_Except_HID_And_Hub;
    }
    else if (((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >> TOPLEVEL_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET) & 0x1) == 1)
    {
        CnfgRD->currentFilterStatus = _Block_All_Devices_Except_HID_Hub_And_Smartcard;
    }
    else if (((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >> TOPLEVEL_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET) & 0x1) == 1)
    {
        CnfgRD->currentFilterStatus = _Block_All_Devices_Except_Audio_And_Vendor_Specific;
    }
    else
    {
        CnfgRD->currentFilterStatus = _Allow_All_Devices;
    }

    CnfgRD->ipAcquisitionMode = (uint8)NETCFG_Icron_getNetworkAcquisitionModeFromFlash();

    NET_pack48Bits(NET_ethernetGetSelfMACAddress(), CnfgRD->mac);

    // get start and end ports of the connected devices when Vhub is enabled
    LINKMGR_getVPortBounds(&startVport, &endVport);
    for(uint8 i = startVport; i <= endVport; i++)
    {
        enum storage_varName flashVar = TOPLEVEL_lexPairedMacAddrVarForVport(i);
        if(STORAGE_varExists(flashVar))
        {
            NET_pack48Bits((STORAGE_varGet(flashVar)->doubleWord >> 16),CnfgRD->pairedMacs[numPairsFound]);
            numPairsFound++;
        }
    }

    CnfgRD->portNumber = (STORAGE_varGet(NETCFG_PORT_NUMBER)->halfWords[3]);

    NET_pack32Bits(NET_ipv4GetIPAddress(), CnfgRD->ipAddr);
    NET_pack32Bits(NET_ipv4GetSubnetMask(), CnfgRD->subNetM);
    NET_pack32Bits(NET_ipv4GetDefaultGateway(), CnfgRD->gateway);

    // If Network Acquisition Mode is static, fill DHCP server with 255.255.255.255
    if(NETCFG_Icron_getNetworkAcquisitionModeFromFlash() != TOPLEVEL_NETWORK_ACQUISITION_DHCP)
    {
        for(uint8 i = 0; i<4; i++)
        {
            CnfgRD->dhcpServer[i] = 255;
        }
    }
    else
    {
        NET_pack32Bits(NET_ipv4GetDhcpServer(), CnfgRD->dhcpServer);
    }


    CnfgRD->numVhubPorts = (STORAGE_varGet(VHUB_CONFIGURATION)->bytes[0]);
    CnfgRD->vhubVid = (STORAGE_varGet(VHUB_CONFIGURATION)->halfWords[1]);
    CnfgRD->vhubPid = (STORAGE_varGet(VHUB_CONFIGURATION)->halfWords[2]);

    NET_pack16Bits(TOPLEVEL_getBrandNumber(),(uint8*) &CnfgRD->brandId);
    TOPLEVEL_copyBrandName(CnfgRD->vendor);

    memcpy(CnfgRD->product, product, sizeof(CnfgRD->product));
    memcpy(CnfgRD->revision, revision, sizeof(CnfgRD->revision));

    return (sizeof(IcronReplyCnfgRespDataT));
}

/**
* FUNCTION NAME: Icron_ping()
*
* @brief  - Creates an acknowledge message in response to a ping message.
*
* @return - The length of the data in response.
*/
static uint16 Icron_ping(uint8** response)
{
    (*response)[_PROTOCOL_VERSION_OFFSET] = 0;
    (*response)[_COMMAND_OFFSET] = _CMD_ZERO_ACKNOWLEDGE;
    return _ZERO_OFFSET;
}

/**
* FUNCTION NAME: Icron_requestExtendedDeviceInformation()
*
* @brief  - Creates a reply extended device information message in response to
*           a request extended device information message.
*
* @return - The length of the data in response.
*/
static uint16 Icron_requestExtendedDeviceInformation(uint8** response)
{
    uint8 startVport;
    uint8 endVport;
    uint8 numPairsFound = 0;

    (*response)[_PROTOCOL_VERSION_OFFSET] = _SUPPORTED_PROTOCOL_VERSION;
    (*response)[_COMMAND_OFFSET] = _CMD_THREE_REPLY_EXTENDED_DEVICE_INFORMATION;
    (*response)[_REPLY_EXTENDED_DEVICE_INFORMATION_LEXREX_OFFSET] =
        GRG_IsDeviceRex() ? _LEXREX_REX : _LEXREX_LEX;

    LINKMGR_getVPortBounds(&startVport, &endVport);
    for(uint8 i = startVport; i <= endVport; i++)
    {
        enum storage_varName flashVar = TOPLEVEL_lexPairedMacAddrVarForVport(i);
        if(STORAGE_varExists(flashVar))
        {
            NET_pack48Bits(
                (STORAGE_varGet(flashVar)->doubleWord >> 16),
                &(*response)[_REPLY_EXTENDED_DEVICE_INFORMATION_PAIRED_WITH_MAC_OFFSET(numPairsFound)]);
            numPairsFound++;
        }
    }

    return _REPLY_EXTENDED_DEVICE_INFORMATION_PAIRED_WITH_MAC_OFFSET(numPairsFound);
}

/**
* FUNCTION NAME: Icron_topologyDescriptionCallback()
*
* @brief  - Populates the response data for a reply device topology message.
*           This function is a callback driven by DTT_describeTopology.
*
* @return - None.
*/
static void Icron_topologyDescriptionCallback(
    uint8 usbAddress,
    uint8 parentUSBAddress,
    uint8 portOnParent,
    boolT isDeviceHub,
    uint16 vendorId,
    uint16 productId,
    void* responseContext)
{
    struct DeviceTopologyResponseContext* context = responseContext;
    context->responseBuffer[ _REPLY_DEVICE_TOPOLOGY_USB_ADDR_OFFSET(
        context->currentDeviceIndex)] = usbAddress;
    context->responseBuffer[_REPLY_DEVICE_TOPOLOGY_USB_ADDR_PARENT_OFFSET(
        context->currentDeviceIndex)] = parentUSBAddress;
    context->responseBuffer[_REPLY_DEVICE_TOPOLOGY_PORT_ON_PARENT_OFFSET(
        context->currentDeviceIndex)] = portOnParent;
    context->responseBuffer[_REPLY_DEVICE_TOPOLOGY_IS_DEVICE_HUB_OFFSET(
        context->currentDeviceIndex)] = isDeviceHub;
    NET_pack16Bits(
        vendorId,
        &(context->responseBuffer[
            _REPLY_DEVICE_TOPOLOGY_VENDOR_ID_OFFSET(context->currentDeviceIndex)]));
    NET_pack16Bits(
        productId,
        &(context->responseBuffer[
            _REPLY_DEVICE_TOPOLOGY_PRODUCT_ID_OFFSET(context->currentDeviceIndex)]));
    context->currentDeviceIndex++;
}

/**
* FUNCTION NAME: Icron_requestDeviceTopology()
*
* @brief  - Creates a reply device topology message in response to a request
*           device topology message.
*
* @return - The length of the data in response.
*/
static uint16 Icron_requestDeviceTopology(uint8** response)
{
    uint16 length;
    if(GRG_IsDeviceLex())
    {
        struct DeviceTopologyResponseContext context =
        { .responseBuffer=*response, .currentDeviceIndex=0 };
        (*response)[_PROTOCOL_VERSION_OFFSET] = _SUPPORTED_PROTOCOL_VERSION;
        (*response)[_COMMAND_OFFSET] = _CMD_THREE_REPLY_DEVICE_TOPOLOGY;
        DTT_describeTopology(&Icron_topologyDescriptionCallback, &context);
        length = _REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(context.currentDeviceIndex);
    }
    else
    {
        // The topology information is stored in the LEX, so a REX device NAKs
        // this request
        (*response)[_PROTOCOL_VERSION_OFFSET] = _SUPPORTED_PROTOCOL_VERSION;
        (*response)[_COMMAND_OFFSET] = _CMD_THREE_NEGATIVE_ACKNOWLEDGE;
        length = _THREE_OFFSET;
    }
    return length;
}

/**
* FUNCTION NAME: Icron_pairToDevice()
*
* @brief  - Creates an accept pairing message in response to a pair to device
*           message.
*
* @return - The length of the data in response.
*/
static uint16 Icron_pairToDevice(NET_MACAddress pairToDeviceMACAddress, uint8** response)
{
    uint16 length;
    if(LINKMGR_addDeviceLink(pairToDeviceMACAddress))
    {
        (*response)[_PROTOCOL_VERSION_OFFSET] = 0;
        (*response)[_COMMAND_OFFSET] = _CMD_ZERO_ACKNOWLEDGE;
        length = _ZERO_OFFSET;
    }
    else
    {
        (*response)[_PROTOCOL_VERSION_OFFSET] = _SUPPORTED_PROTOCOL_VERSION;
        (*response)[_COMMAND_OFFSET] = _CMD_THREE_NEGATIVE_ACKNOWLEDGE;
        length = _THREE_OFFSET;
    }

    return length;
}

/**
* FUNCTION NAME: Icron_removeDevicePairing()
*
* @brief  - Eliminates the pairing between this device and it's pair.
*
* @return - The length of the data in response.
*/
static uint16 Icron_removeDevicePairing(NET_MACAddress linkedMACAddress, uint8** response)
{
    const boolT unpairAccepted = LINKMGR_removeDeviceLink(linkedMACAddress);
    uint16 length;
    if(unpairAccepted)
    {
        (*response)[_PROTOCOL_VERSION_OFFSET] = 0;
        (*response)[_COMMAND_OFFSET] = _CMD_ZERO_ACKNOWLEDGE;
        length = _ZERO_OFFSET;
    }
    else
    {
        (*response)[_PROTOCOL_VERSION_OFFSET] = _SUPPORTED_PROTOCOL_VERSION;
        (*response)[_COMMAND_OFFSET] = _CMD_THREE_NEGATIVE_ACKNOWLEDGE;
        length = _THREE_OFFSET;
    }

    return length;
}

/**
* FUNCTION NAME: Icron_useDHCP()
*
* @brief  - Use DHCP as the mechanism to obtain a network configuration.
*
* @return - The length of the data in response.
*
* @note   - Calling this function when already in DHCP mode will have no
*           effect.
*/
static uint16 Icron_useDHCP(uint8** response)
{
    (*response)[_PROTOCOL_VERSION_OFFSET] = 0;
    (*response)[_COMMAND_OFFSET] = _CMD_ZERO_ACKNOWLEDGE;

    onCompletionData.onCompletionFunction = &Icron_completeSettingDHCP;



    return _ZERO_OFFSET;
}

/**
* FUNCTION NAME: Icron_useStaticIP()
*
* @brief  - Use the static network configuration parameters passed to this
*           function.
*
* @return - The length of the data in response.
*/
static uint16 Icron_useStaticIP(
    NET_IPAddress ip,
    NET_IPAddress subnetMask,
    NET_IPAddress defaultGateway,
    uint8** response)
{
    (*response)[_PROTOCOL_VERSION_OFFSET] = 0;
    (*response)[_COMMAND_OFFSET] = _CMD_ZERO_ACKNOWLEDGE;

    onCompletionData.onCompletionFunction = &Icron_completeSettingIP;
    onCompletionData.params.useStaticIP.ip = ip;
    onCompletionData.params.useStaticIP.subnetMask = subnetMask;
    onCompletionData.params.useStaticIP.defaultGateway = defaultGateway;

    return _ZERO_OFFSET;
}

/**
* FUNCTION NAME: Icron_useFilteringStrategy
*
* @brief  - Use the filtering strategy provided in the parameter.
*
* @return - The length of the data in response.
*/
static uint16 Icron_useFilteringStrategy(
    enum _NETCFG_FilteringStrategy filteringStrategy,
    uint8** response)
{
    uint16 responseLength = _ZERO_OFFSET;

    (*response)[_PROTOCOL_VERSION_OFFSET] = 0;
    (*response)[_COMMAND_OFFSET] = _CMD_ZERO_ACKNOWLEDGE;

    bool setFilterFailed = false;

    // CONFIGURATION_BITS flash var is always initialized
    if(STORAGE_varExists(CONFIGURATION_BITS))
    {
        union STORAGE_VariableData* cfgBits = STORAGE_varGet(CONFIGURATION_BITS);
        uint64 * cfg = &(cfgBits->doubleWord);

        if((*cfg >> TOPLEVEL_ENABLE_DCF_OFFSET) & 0x1)
        {
            static const uint64 filterMask = (
                      (1 << TOPLEVEL_BLOCK_MASS_STORAGE_OFFSET)
                    | (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET)
                    | (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET)
                    | (1 << TOPLEVEL_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET) );

            uint64 filterSetting = *cfg & filterMask; // get the current filter settings

            switch(filteringStrategy)
            {
                case _Allow_All_Devices:
                    filterSetting = 0;  // clear all filtering bits
                    break;

                case _Block_All_Devices_Except_HID_And_Hub:
                    filterSetting = (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET);
                    break;

                case _Block_Mass_Storage:
                    filterSetting = (1 << TOPLEVEL_BLOCK_MASS_STORAGE_OFFSET);
                    break;

                case _Block_All_Devices_Except_HID_Hub_And_Smartcard:
                    filterSetting = (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET);
                    break;

                case _Block_All_Devices_Except_Audio_And_Vendor_Specific:
                    filterSetting = (1 << TOPLEVEL_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET);
                    break;

                default:
                    setFilterFailed = true;
                    break;
            }

            *cfg &= ~filterMask;    // clear all filtering bits
            *cfg |=  filterSetting; // set the new filter
            STORAGE_varSave(CONFIGURATION_BITS);
        }
        else
        {
            setFilterFailed = true;
        }

        if (setFilterFailed)
        {
            responseLength = _THREE_OFFSET;
            (*response)[_PROTOCOL_VERSION_OFFSET] = _SUPPORTED_PROTOCOL_VERSION;
            (*response)[_COMMAND_OFFSET] = _CMD_THREE_NEGATIVE_ACKNOWLEDGE;
        }

    }
    return responseLength;
}

/**
* FUNCTION NAME: Icron_requestDeviceLocator
*
* @brief  - Turns the LED locator pattern off or on
*
* @return - The length of the data in response.
*/
static uint16 Icron_requestDeviceLocator(bool enable, uint8** response)
{
    // turn the LED locator off/on
    if (enable)
    {
        GRG_SetLocatorLedsPattern();
    }
    else
    {
        GRG_ClearLocatorLedsPattern();
    }

    (*response)[_PROTOCOL_VERSION_OFFSET] = 0;
    (*response)[_COMMAND_OFFSET] = _CMD_ZERO_ACKNOWLEDGE;
    return _ZERO_OFFSET;

}

/**
* FUNCTION NAME: Icron_resetDevice()
*
* @brief  - Creates an acknowledge message in response to resetting a device.
*           The goal is to wait for the response to be received and then call
*           GRG_ResetChip()
*           Calling GRG_Reset right away resets everything before the response
*           is received. That causes IsRecvGeneralPacketCorrect to return -1
*           To avoid that, responseTimer waits for 10 miliseconds before calling
*           GRG_ResetChip. This gives enough time for the response to be received
*
* @return - The length of the data in response.
*/
static uint16 Icron_resetDevice(uint8** response)
{
    (*response)[_PROTOCOL_VERSION_OFFSET] = 0;
    (*response)[_COMMAND_OFFSET] = _CMD_ZERO_ACKNOWLEDGE;

    // Start the timer
    NETCFG_StartResetTimer();
    return _ZERO_OFFSET;
}

/**
* FUNCTION NAME: Icron_completeSettingIP()
*
* @brief  - Completes the setting of the network configuration following a
*           use_static_ip message.  This is done to allow sending the
*           acknowledge message from the same IP that it was received on.
*
* @return - None.
*/
static void Icron_completeSettingIP(void)
{
    if(NETCFG_Icron_getNetworkAcquisitionModeFromFlash() == TOPLEVEL_NETWORK_ACQUISITION_DHCP)
    {
        NET_dhcpDisable();
        Icron_setNetworkAcquisitionModeInFlash(TOPLEVEL_NETWORK_ACQUISITION_STATIC);
    }


    NET_ipv4SetIPAddress(onCompletionData.params.useStaticIP.ip);
    NET_ipv4SetSubnetMask(onCompletionData.params.useStaticIP.subnetMask);
    NET_ipv4SetDefaultGateway(onCompletionData.params.useStaticIP.defaultGateway);
}

/**
* FUNCTION NAME: Icron_completeSettingDHCP()
*
* @brief  - Completes the setting of the network configuration following a
*           use_dhcp_ip message.  This is done to allow sending the
*           acknowledge message from the same IP that it was received on.
*
* @return - None.
*/
static void Icron_completeSettingDHCP(void)
{
    if(NETCFG_Icron_getNetworkAcquisitionModeFromFlash() != TOPLEVEL_NETWORK_ACQUISITION_DHCP)
    {
        NET_dhcpEnable();
        Icron_setNetworkAcquisitionModeInFlash(TOPLEVEL_NETWORK_ACQUISITION_DHCP);
    }
}

/**
* FUNCTION NAME: _setNetworkAcquisitionModeInFlash()
*
* @brief  - Sets the network acquisition mode in flash to either 0 -> DHCP or 1
*           -> static IP.
*
* @return - None.
*/
static void Icron_setNetworkAcquisitionModeInFlash(enum TOPLEVEL_VarNetworkAcquisitionMode mode)
{
    union STORAGE_VariableData* networkAcquisitionMode =
        STORAGE_varExists(NETWORK_ACQUISITION_MODE) ?
            STORAGE_varGet(NETWORK_ACQUISITION_MODE) :
            STORAGE_varCreate(NETWORK_ACQUISITION_MODE);
    networkAcquisitionMode->bytes[0] = mode;
    STORAGE_varSave(NETWORK_ACQUISITION_MODE);
}
