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
//!   @file  - netcfg_packet_handler_broadcast.c
//
//!   @brief - Provides a function for handling incoming UDP configuration
//             requests.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
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
#include <timing_timers.h>

/************************ Defined Constants and Macros ***********************/
// Timeout value in mili seconds for the response of _CMD_THREE_RESET_DEVICE
#define _RESET_DEVICE_RESPONSE_TIMER       10
#define _VENDOR_STRING_LENGTH     32
#define _PRODUCT_STRING_LENGTH    32
#define _REVISION_STRING_LENGTH   12

// Network configuration packed data offsets - general
#define _MAGIC_NUMBER_OFFSET            0
#define _MESSAGE_ID_OFFSET              (_MAGIC_NUMBER_OFFSET + 4)
#define _COMMAND_OFFSET                 (_MESSAGE_ID_OFFSET + 4)
#define _DEVICE_MAC_ADDRESS_OFFSET      (_COMMAND_OFFSET + 1)
#define _COMMAND_SPECIFIC_DATA_OFFSET   (_DEVICE_MAC_ADDRESS_OFFSET + 6)

// Network configuration packed data offsets
#define _REPLY_DEVICE_INFORMATION_VENDOR_OFFSET                     (_COMMAND_SPECIFIC_DATA_OFFSET)
#define _REPLY_DEVICE_INFORMATION_PRODUCT_OFFSET                    (_REPLY_DEVICE_INFORMATION_VENDOR_OFFSET + _VENDOR_STRING_LENGTH)
#define _REPLY_DEVICE_INFORMATION_REVISION_OFFSET                   (_REPLY_DEVICE_INFORMATION_PRODUCT_OFFSET + _PRODUCT_STRING_LENGTH)
#define _REPLY_DEVICE_INFORMATION_LEXREX_OFFSET                     (_REPLY_DEVICE_INFORMATION_REVISION_OFFSET + _REVISION_STRING_LENGTH)
#define _REPLY_DEVICE_INFORMATION_SUPPORTED_PROTO_VER_OFFSET        (_REPLY_DEVICE_INFORMATION_LEXREX_OFFSET + 1)
#define _REPLY_DEVICE_INFORMATION_PAIRED_WITH_MAC_OFFSET(pairingNum)   (_REPLY_DEVICE_INFORMATION_SUPPORTED_PROTO_VER_OFFSET + 1 + (pairingNum * 6))
#define _PAIR_TO_DEVICE_MAC_ADDRESS_OFFSET                          (_COMMAND_SPECIFIC_DATA_OFFSET)
#define _REMOVE_DEVICE_PAIRING_PAIRED_MAC_ADDRESS_OFFSET            (_COMMAND_SPECIFIC_DATA_OFFSET)
#define _REPLY_DEVICE_TOPOLOGY_DEVICES_OFFSET                       (_COMMAND_SPECIFIC_DATA_OFFSET)
#define _REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum)             (_REPLY_DEVICE_TOPOLOGY_DEVICES_OFFSET + (deviceNum * 8))
#define _REPLY_DEVICE_TOPOLOGY_USB_ADDR_OFFSET(deviceNum)           (_REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum) + 0)
#define _REPLY_DEVICE_TOPOLOGY_USB_ADDR_PARENT_OFFSET(deviceNum)    (_REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum) + 1)
#define _REPLY_DEVICE_TOPOLOGY_PORT_ON_PARENT_OFFSET(deviceNum)     (_REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum) + 2)
#define _REPLY_DEVICE_TOPOLOGY_IS_DEVICE_HUB_OFFSET(deviceNum)      (_REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum) + 3)
#define _REPLY_DEVICE_TOPOLOGY_VENDOR_ID_OFFSET(deviceNum)          (_REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum) + 4)
#define _REPLY_DEVICE_TOPOLOGY_PRODUCT_ID_OFFSET(deviceNum)         (_REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(deviceNum) + 6)
#define _USE_FILTERING_STRATEGY_FILTERING_STRATEGY_OFFSET           (_COMMAND_SPECIFIC_DATA_OFFSET)

#define _MAGIC_NUMBER 0xa9c4d8f4

#define _LEXREX_LEX                           0
#define _LEXREX_REX                           1

#define _SUPPORTED_PROTOCOL_VERSION 2

// commands
#define _CMD_REQUEST_DEVICE_INFORMATION             0
#define _CMD_REPLY_DEVICE_INFORMATION               1
#define _CMD_PING                                   2
#define _CMD_ACKNOWLEDGE                            3
#define _CMD_PAIR_TO_DEVICE                         4
#define _CMD_REMOVE_DEVICE_PAIRING                  5
#define _CMD_REQUEST_DEVICE_TOPOLOGY                6
#define _CMD_REPLY_DEVICE_TOPOLOGY                  7
#define _CMD_REPLY_UNHANDLED_COMMAND                8
#define _CMD_NEGATIVE_ACKNOWLEDGE                   9
#define _CMD_REMOVE_ALL_PAIRINGS                    10
#define _CMD_USE_FILTERING_STRATEGY                 11
#define _CMD_REQUEST_CONFIGURATION_RESPONSE_DATA    12
#define _CMD_REPLY_CONFIGURATION_RESPONSE_DATA      13
#define _CMD_RESET_DEVICE                           14
#define _CMD_LED_LOCATOR_ON                         15
#define _CMD_LED_LOCATOR_OFF                        16

/******************************** Data Types *********************************/

// This type is used for context passed back to the
// _topologyDescriptionCallback to allow that function to fill in the response
// buffer with the topology details.
struct DeviceTopologyResponseContext
{
    uint8* responseBuffer;
    uint8 currentDeviceIndex;
};

struct MsgBase
{
    /// Value 0xA9C4D8F4 that begins all packets to help identify it as belonging to this protocol.
    uint32        magicNumber;
    /// When the client sends a request, it chooses any value to insert
    /// in this field.  The device responding to the request will set this
    /// field in the reply to the same value it received in the request.
    uint32        messageId;
    /// The command identifier.  Corresponds to the CMD_* defines.
    uint8         command;
    /// The MAC address of the device which is the target of this message
    uint8         deviceMacAddress[6];
} __attribute__((packed));
// Structure representing reply of request_configuration_response_data
typedef struct ReplyCnfgRespData
{
    // MsgBase
    struct MsgBase  base;
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
    uint8 currentFilterStatus;
    /// The fill to make the structure aligned
    uint8 fill1[1];
    /// MAC address of the device
    uint8 mac[6];
    /// The fill to make the structure aligned
    uint8 fill2[2];
    /// MAC address of the paired devices
    uint8 pairedMacs[7][6];
    /// Port number of the connected unit
    uint16 portNumber;
    /// Vhub - Number of Downstream ports
    uint8 numVhubPorts;
    /// The fill to make the structure aligned
    uint8 fill3 [1];
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
} __attribute__((packed)) replyConfigurationResponseData;

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
static uint16 Crestron_handleMessageAndCreateResponse(
    uint8* input, uint16 inputLength, uint8** response) __attribute__((section(".crestron_xusbcfg_text")));
static uint16 Crestron_requestDeviceInformation(uint8** response) __attribute__((section(".crestron_xusbcfg_text")));
static uint16 Crestron_requestConfigurationResponseData(uint8** response) __attribute__((section(".crestron_xusbcfg_text")));
static uint16 Crestron_ping(uint8** response) __attribute__((section(".crestron_xusbcfg_text")));
static uint16 Crestron_resetDevice(uint8** response) __attribute__((section(".crestron_xusbcfg_text")));
static uint16 Crestron_requestDeviceLocator(bool enable, uint8** response) __attribute__((section(".crestron_xusbcfg_text")));
static uint16 Crestron_requestDeviceTopology(uint8** response) __attribute__((section(".crestron_xusbcfg_text")));
static uint16 Crestron_pairToDevice(
    NET_MACAddress pairToDeviceMACAddress, uint8** response) __attribute__((section(".crestron_xusbcfg_text")));
static uint16 Crestron_removeDevicePairing(NET_MACAddress linkedMACAddress, uint8** response);
static uint16 Crestron_removeAllPairings(uint8** response);
static uint16 Crestron_useFilteringStrategy(
        enum _NETCFG_FilteringStrategy filteringStrategy,
        uint8** response)__attribute__((section(".crestron_xusbcfg_text")));
static uint16 Crestron_unhandledCommand(uint8** response) __attribute__((section(".crestron_xusbcfg_text")));
static boolT Crestron_isLengthAndCommandCorrect(
    uint8 command,
    uint16 inputLength) __attribute__((section(".crestron_xusbcfg_text")));


/***************** External Visibility Function Definitions ******************/


/***************** Component Visibility Function Definitions *****************/


/******************** File Visibility Function Definitions *******************/

/**
* FUNCTION NAME: NETCFG_Crestron_handleUDPPacket()
*
* @brief  - Handles a UDP traffic sent to the port bound to the network based
*           device configuration functionality.
*
* @return - void
*/
void NETCFG_Crestron_handleUDPPacket(
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
            NET_MACAddress deviceMACAddress =
                NET_unpack48Bits(&data[_DEVICE_MAC_ADDRESS_OFFSET]);
            if(deviceMACAddress == NET_ethernetGetSelfMACAddress() ||
               deviceMACAddress == NET_ETHERNET_BROADCAST_MAC_ADDRESS)
            {
                uint8* response = NULL;
                uint16 responseLength =
                    Crestron_handleMessageAndCreateResponse(data, dataLength, &response);

                if(response != NULL && responseLength > 0)
                {
                    NET_pack32Bits(
                        _MAGIC_NUMBER,
                        &response[_MAGIC_NUMBER_OFFSET]);

                    NET_pack32Bits(
                        NET_unpack32Bits(&data[_MESSAGE_ID_OFFSET]),
                        &response[_MESSAGE_ID_OFFSET]);

                    NET_pack48Bits(
                        NET_ethernetGetSelfMACAddress(),
                        &response[_DEVICE_MAC_ADDRESS_OFFSET]);

                    NET_udpTransmitPacket(
                        response,
                        NET_IPV4_LIMITED_BROADCAST_ADDRESS,
                        responseLength,
                        STORAGE_varGet(NETCFG_PORT_NUMBER)->doubleWord,
                        senderPort);

                    if(onCompletionData.onCompletionFunction)
                    {
                        (*onCompletionData.onCompletionFunction)();
                    }
                }
            }
        }
    }
}

/**
* FUNCTION NAME: _isLengthAndCommandCorrect()
*
* @brief  - Checks to see if the received data length is appropriate for the
*           command received.
*
* @return - TRUE if the length is correct for the given command and the command
*           is one that is expected to be received by a LEX or REX (i.e. not a
*           device to client command).
*/
static boolT Crestron_isLengthAndCommandCorrect(uint8 command, uint16 inputLength)
{
    uint16 expectedLength = 0;

    switch(command)
    {
        case _CMD_REQUEST_DEVICE_INFORMATION:
        case _CMD_REQUEST_CONFIGURATION_RESPONSE_DATA:
        case _CMD_PING:
        case _CMD_RESET_DEVICE:
        case _CMD_LED_LOCATOR_ON:
        case _CMD_LED_LOCATOR_OFF:
            expectedLength = _COMMAND_SPECIFIC_DATA_OFFSET;
            break;

        // LEX and REX devices do not accept these commands
        case _CMD_REPLY_DEVICE_INFORMATION:
        case _CMD_REPLY_CONFIGURATION_RESPONSE_DATA:
        case _CMD_ACKNOWLEDGE:
            break;

        case _CMD_REMOVE_DEVICE_PAIRING:
            expectedLength = _REMOVE_DEVICE_PAIRING_PAIRED_MAC_ADDRESS_OFFSET + 6;
            break;

        case _CMD_REQUEST_DEVICE_TOPOLOGY:
            expectedLength = _COMMAND_SPECIFIC_DATA_OFFSET;
            break;

        case _CMD_PAIR_TO_DEVICE:
            expectedLength = _PAIR_TO_DEVICE_MAC_ADDRESS_OFFSET + 6;
            break;

        // LEX and REX devices do not accept these commands
        case _CMD_REPLY_DEVICE_TOPOLOGY:
            break;

        case _CMD_REMOVE_ALL_PAIRINGS:
            expectedLength = _COMMAND_SPECIFIC_DATA_OFFSET;
            break;

        case _CMD_REPLY_UNHANDLED_COMMAND:
            break;

        case _CMD_USE_FILTERING_STRATEGY:
            expectedLength = _USE_FILTERING_STRATEGY_FILTERING_STRATEGY_OFFSET + 1;
            break;

        default:
            break;
    }
    return expectedLength != 0 && inputLength == expectedLength;
}

/**
* FUNCTION NAME: Crestron_handleMessageAndCreateResponse()
*
* @brief  - Processes the incoming message and allocates a response and fills
*           in the data.
*
* @return - The length of the data in response.
*
* @note   - If no response is to be sent, the output parameter response will be
*           a NULL pointer.
*/
static uint16 Crestron_handleMessageAndCreateResponse(
    uint8* input, uint16 inputLength, uint8** response)
{
    uint16 responseLength = 0;
    *response = NET_udpAllocateBuffer();

    if(!Crestron_isLengthAndCommandCorrect(
            input[_COMMAND_OFFSET],
            inputLength))
    {
        goto abortResponse;
    }

    ilog_NETCFG_COMPONENT_1(ILOG_MINOR_EVENT, NETCFG_RECEIVED_REQUEST, input[_COMMAND_OFFSET]);
    switch(input[_COMMAND_OFFSET])
    {
        case _CMD_REQUEST_DEVICE_INFORMATION:
            responseLength = Crestron_requestDeviceInformation(response);
            break;

        case _CMD_REQUEST_CONFIGURATION_RESPONSE_DATA:
            responseLength = Crestron_requestConfigurationResponseData(response);
            break;

        case _CMD_PING:
            responseLength = Crestron_ping(response);
            break;

        case _CMD_RESET_DEVICE:
            responseLength = Crestron_resetDevice(response);
            break;

        case _CMD_LED_LOCATOR_ON:
            responseLength = Crestron_requestDeviceLocator(true, response);
            break;

         case _CMD_LED_LOCATOR_OFF:
            responseLength = Crestron_requestDeviceLocator(false, response);
            break;

        case _CMD_ACKNOWLEDGE:
        case _CMD_REPLY_DEVICE_INFORMATION:
        case _CMD_REPLY_CONFIGURATION_RESPONSE_DATA:
            // TODO: log that these commands aren't valid for a LEX or
            // REX
            goto abortResponse;

        case _CMD_REQUEST_DEVICE_TOPOLOGY:
            responseLength = Crestron_requestDeviceTopology(response);
            break;

        case _CMD_PAIR_TO_DEVICE:
            {
                NET_MACAddress pairToDeviceMACAddress =
                    NET_unpack48Bits(&input[_PAIR_TO_DEVICE_MAC_ADDRESS_OFFSET]);
                responseLength = Crestron_pairToDevice(pairToDeviceMACAddress, response);
            }
            break;

        case _CMD_REMOVE_DEVICE_PAIRING:
            {
                NET_MACAddress pairedMACAddress =
                    NET_unpack48Bits(&input[_REMOVE_DEVICE_PAIRING_PAIRED_MAC_ADDRESS_OFFSET]);
                responseLength = Crestron_removeDevicePairing(pairedMACAddress, response);
            }
            break;

        case _CMD_REPLY_DEVICE_TOPOLOGY:
        case _CMD_REPLY_UNHANDLED_COMMAND:
        case _CMD_NEGATIVE_ACKNOWLEDGE:
            // TODO: log that these commands aren't valid for a LEX or
            // REX
            goto abortResponse;

        case _CMD_REMOVE_ALL_PAIRINGS:
            responseLength = Crestron_removeAllPairings(response);
            break;

        case _CMD_USE_FILTERING_STRATEGY:
            {
                enum _NETCFG_FilteringStrategy filteringStrategy =
                    input[_USE_FILTERING_STRATEGY_FILTERING_STRATEGY_OFFSET];
                responseLength = Crestron_useFilteringStrategy(
                        filteringStrategy,
                        response);
            }
            break;

        default:
            responseLength = Crestron_unhandledCommand(response);
            break;
    }

    if(responseLength == 0)
    {
abortResponse:
        NET_udpFreeBuffer(*response);
        *response = NULL;
    }
    return responseLength;
}

/**
* FUNCTION NAME: Crestron_requestDeviceInformation()
*
* @brief  - Creates a reply device information message in response to a request
*           device information message.
*
* @return - The length of the data in response.
*/
static uint16 Crestron_requestDeviceInformation(uint8** response)
{
    static char product[] __attribute__((section(".data"))) = "USB Over Network";
    static char revision[] __attribute__((section(".data"))) = SOFTWARE_REVISION_STR;
    uint8 startVport;
    uint8 endVport;
    uint8 numPairsFound = 0;

    (*response)[_COMMAND_OFFSET] = _CMD_REPLY_DEVICE_INFORMATION;

    TOPLEVEL_copyBrandName(&(*response)[_REPLY_DEVICE_INFORMATION_VENDOR_OFFSET]);
    memcpy(&(*response)[_REPLY_DEVICE_INFORMATION_PRODUCT_OFFSET], product, sizeof(product));
    memcpy(&(*response)[_REPLY_DEVICE_INFORMATION_REVISION_OFFSET], revision, sizeof(revision));

    (*response)[_REPLY_DEVICE_INFORMATION_LEXREX_OFFSET] =
        GRG_IsDeviceLex() ? _LEXREX_LEX : _LEXREX_REX;

    (*response)[_REPLY_DEVICE_INFORMATION_SUPPORTED_PROTO_VER_OFFSET] =
        _SUPPORTED_PROTOCOL_VERSION;


    LINKMGR_getVPortBounds(&startVport, &endVport);
    for(uint8 i = startVport; i <= endVport; i++)
    {
        enum storage_varName flashVar = TOPLEVEL_lexPairedMacAddrVarForVport(i);
        if(STORAGE_varExists(flashVar))
        {
            NET_pack48Bits(
                (STORAGE_varGet(flashVar)->doubleWord >> 16),
                &(*response)[_REPLY_DEVICE_INFORMATION_PAIRED_WITH_MAC_OFFSET(numPairsFound)]);
            numPairsFound++;
        }
    }

    return _REPLY_DEVICE_INFORMATION_PAIRED_WITH_MAC_OFFSET(numPairsFound);
}

/**
* FUNCTION NAME: Crestron_requestConfigurationResponseData()
*
* @brief  - Creates a reply configuration response data message in response to a request
*           configuration response data message.
*
* @return - The length of the data in response.
*/
static uint16 Crestron_requestConfigurationResponseData(uint8** response)
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

    // use structure pointer CnfgRD to navigate through replyConfigurationResponseData structure
    replyConfigurationResponseData *CnfgRD = (replyConfigurationResponseData *)*response;

    CnfgRD->base.command = _CMD_REPLY_CONFIGURATION_RESPONSE_DATA;

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
    else
    {
        CnfgRD->currentFilterStatus = _Allow_All_Devices;
    }

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

    CnfgRD->numVhubPorts = (STORAGE_varGet(VHUB_CONFIGURATION)->bytes[0]);
    CnfgRD->vhubVid = (STORAGE_varGet(VHUB_CONFIGURATION)->halfWords[1]);
    CnfgRD->vhubPid = (STORAGE_varGet(VHUB_CONFIGURATION)->halfWords[2]);

    NET_pack16Bits(TOPLEVEL_getBrandNumber(),(uint8*) &CnfgRD->brandId);
    TOPLEVEL_copyBrandName(CnfgRD->vendor);

    memcpy(CnfgRD->product, product, sizeof(CnfgRD->product));
    memcpy(CnfgRD->revision, revision, sizeof(CnfgRD->revision));

    return (sizeof(replyConfigurationResponseData));
}

/**
* FUNCTION NAME: Crestron_ping()
*
* @brief  - Creates an acknowledge message in response to a ping message.
*
* @return - The length of the data in response.
*/
static uint16 Crestron_ping(uint8** response)
{
    (*response)[_COMMAND_OFFSET] = _CMD_ACKNOWLEDGE;
    return _COMMAND_SPECIFIC_DATA_OFFSET;
}


/**
* FUNCTION NAME: Crestron_resetDevice()
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
static uint16 Crestron_resetDevice(uint8** response)
{
//    (*response)[_COMMAND_OFFSET] = _COMMAND_SPECIFIC_DATA_OFFSET;
    (*response)[_COMMAND_OFFSET] = _CMD_ACKNOWLEDGE;

    // Start the timer
    NETCFG_StartResetTimer();
    return _COMMAND_SPECIFIC_DATA_OFFSET;
}

/**
* FUNCTION NAME: Crestron_requestDeviceLocator
*
* @brief  - Turns the LED locator pattern off or on
*
* @return - The length of the data in response.
*/
static uint16 Crestron_requestDeviceLocator(bool enable, uint8** response)
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

    (*response)[_COMMAND_OFFSET] = _CMD_ACKNOWLEDGE;
    return _COMMAND_SPECIFIC_DATA_OFFSET;

}

/**
* FUNCTION NAME: _topologyDescriptionCallback()
*
* @brief  - Populates the response data for a reply device topology message.
*           This function is a callback driven by DTT_describeTopology.
*
* @return - None.
*/
static void Crestron_topologyDescriptionCallback(
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
* FUNCTION NAME: _requestDeviceTopology()
*
* @brief  - Creates a reply device topology message in response to a request
*           device topology message.
*
* @return - The length of the data in response.
*/
static uint16 Crestron_requestDeviceTopology(uint8** response)
{
    uint16 length;

    if(GRG_IsDeviceLex())
    {
        struct DeviceTopologyResponseContext context =
        { .responseBuffer=*response, .currentDeviceIndex=0 };
        (*response)[_COMMAND_OFFSET] = _CMD_REPLY_DEVICE_TOPOLOGY;
        DTT_describeTopology(&Crestron_topologyDescriptionCallback, &context);
        length = _REPLY_DEVICE_TOPOLOGY_DEVICE_OFFSET(context.currentDeviceIndex);
    }
    else
    {
        (*response)[_COMMAND_OFFSET] = _CMD_NEGATIVE_ACKNOWLEDGE;
        length = _COMMAND_SPECIFIC_DATA_OFFSET;
    }

    return length;
}

/**
* FUNCTION NAME: Crestron_pairToDevice()
*
* @brief  - Creates an accept pairing message in response to a pair to device
*           message.
*
* @return - The length of the data in response.
*/
static uint16 Crestron_pairToDevice(NET_MACAddress pairToDeviceMACAddress, uint8** response)
{
    (*response)[_COMMAND_OFFSET] =
        (LINKMGR_addDeviceLink(pairToDeviceMACAddress)) ?
            _CMD_ACKNOWLEDGE : _CMD_NEGATIVE_ACKNOWLEDGE;

    return _COMMAND_SPECIFIC_DATA_OFFSET;
}

/**
* FUNCTION NAME: Crestron_removeDevicePairing()
*
* @brief  - Eliminates the pairing between this device and the pair specified.
*
* @return - The length of the data in response.
*/
static uint16 Crestron_removeDevicePairing(NET_MACAddress linkedMACAddress, uint8** response)
{
    (*response)[_COMMAND_OFFSET] =
        (LINKMGR_removeDeviceLink(linkedMACAddress)) ?
            _CMD_ACKNOWLEDGE : _CMD_NEGATIVE_ACKNOWLEDGE;

    return _COMMAND_SPECIFIC_DATA_OFFSET;
}

/**
* FUNCTION NAME: Crestron_removeAllPairings()
*
* @brief  - Unconditionally remove all pairings from this device.
*
* @return - the length of the data in response.
*/
static uint16 Crestron_removeAllPairings(uint8** response)
{
    LINKMGR_removeAllDeviceLinks();
    (*response)[_COMMAND_OFFSET] = _CMD_ACKNOWLEDGE;
    return _COMMAND_SPECIFIC_DATA_OFFSET;
}

/**
* FUNCTION NAME: Crestron_useFilteringStrategy()
*
* @brief  - Use the filtering strategy provided in the parameter.
*
* @return - the length of the data in response.
*/
static uint16 Crestron_useFilteringStrategy(
        enum _NETCFG_FilteringStrategy filteringStrategy,
        uint8** response)
{
    (*response)[_COMMAND_OFFSET] = _CMD_ACKNOWLEDGE;

    bool setFilterFailed = false;
    // CONFIGURATION_BITS flash var is always initialized
    if(STORAGE_varExists(CONFIGURATION_BITS))
    {
        union STORAGE_VariableData* cfgBits = STORAGE_varGet(CONFIGURATION_BITS);
        uint64* cfg = &(cfgBits->doubleWord);

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
            (*response)[_COMMAND_OFFSET] = _CMD_NEGATIVE_ACKNOWLEDGE;
        }
    }
    return _COMMAND_SPECIFIC_DATA_OFFSET;
}

/**
* FUNCTION NAME: Crestron_unhandledCommand()
*
* @brief  - Creates a reply unhandled command message in response to a command
*           sent with an unknown command number.
*
* @return - The length of the data in response.
*/
static uint16 Crestron_unhandledCommand(uint8** response)
{
    (*response)[_COMMAND_OFFSET] = _CMD_REPLY_UNHANDLED_COMMAND;
    return _COMMAND_SPECIFIC_DATA_OFFSET;
}
