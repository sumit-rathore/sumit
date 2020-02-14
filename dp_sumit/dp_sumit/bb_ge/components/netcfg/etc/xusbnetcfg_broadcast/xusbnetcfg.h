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
#include <stdint.h>
#include <stdbool.h>

#define IP4_STR_LEN 16
#define MAC_STR_LEN 18
#define MAX_USB_DEVICES 32
#define REPLY_DEV_INFO_VENDOR_LENGTH 32
#define REPLY_DEV_INFO_PRODUCT_LENGTH 32
#define REPLY_DEV_INFO_REVISION_LENGTH 12
#define REPLY_DEV_INFO_MAX_PAIRS        7
#define REPLY_CNFG_RESP_DATA_VENDOR_LENGTH 32
#define REPLY_CNFG_RESP_DATA_PRODUCT_LENGTH 32
#define REPLY_CNFG_RESP_DATA_REVISION_LENGTH 12
#define REPLY_CNFG_RESP_DATA_MAX_PAIRS        7
#define TIMEOUT_ONE_SEC 1

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


enum ReplyCommandType
{
    REPLY_CMD_REPLY_DEVICE_INFORMATION,
    REPLY_CMD_ACKNOWLEDGE,
    REPLY_CMD_REPLY_DEVICE_TOPOLOGY,
    REPLY_CMD_REPLY_UNHANDLED_COMMAND,
    REPLY_CMD_REPLY_CONFIGURATION_RESPONSE_DATA,
    REPLY_CMD_NEGATIVE_ACKNOWLEDGE
};

struct ResponseData
{
    enum ReplyCommandType command;

    /// Address of the device.
    uint8_t macAddress[6];
    union
    {
        // client <- device
        struct
        {
            char vendor[REPLY_DEV_INFO_VENDOR_LENGTH];
            char product[REPLY_DEV_INFO_PRODUCT_LENGTH];
            char revision[REPLY_DEV_INFO_REVISION_LENGTH];
            uint8_t lexOrRex;
            uint8_t supportedProtocolVersion;
            uint8_t numPairs;
            uint8_t pairedWithMacAddress[REPLY_DEV_INFO_MAX_PAIRS][6];
        } replyDeviceInformation;

        // client <- device
        struct
        {
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
            /// 1 - block non ISO devices
            /// 2 - block mass storage devices
            /// 3 - block all but HID and Hub devices
            /// 4 - block all but HID, HUB and smartcard devices
            uint8_t currentFilterStatus;
            /// MAC address of the device
            uint8_t mac[6];
            /// MAC address of the paired devices
            uint8_t pairedMacs[7][6];
            /// Port number of the connected unit
            uint16_t portNumber;
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
        } replyConfigurationResponseData;

        // client <- device
        struct
        {
        } acknowledge;

        // client <- device
        struct
        {
            uint8_t         numDevices;
            struct
            {
                /// The USB address of a device
                uint8_t     usbAddress;
                /// The USB address of the parent of this device.
                uint8_t     usbAddressOfParent;
                /// The port number of this device on the parent device.
                uint8_t     portOnParent;
                /// 1 if this device is a hub or 0 otherwise
                uint8_t     isHub;
                /// The USB vendor ID of the device
                uint16_t    usbVendorId;
                /// The USB product ID of the device
                uint16_t    usbProductId;
            } devices[MAX_USB_DEVICES];
        } replyDeviceTopology;

        // client <- device
        struct
        {
        } replyUnhandledCommand;

        // client <- device
        struct
        {
        } negativeAcknowledge;
    };
};



struct IcronUsbNetCfg;


/**
* @brief    Initializes Icron USB Network Configuration
*
* @return   Pointer to an IcronUsbNetCfgT structure, NULL on failure
*/
struct IcronUsbNetCfg *ICRON_usbNetCfgInit(void);


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
* @param[in] targetIpAddr   Pointer to a string containing target ip address,
*                           can be broadcast
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for responses from the network
*
* @return   Number of devices found
*/
int ICRON_requestDeviceInformation(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress);

/**
* @brief    Loads the user provided IcronConfigurationResponseData structure with data
*           corresponding to the requested item.
*
* @param[in,out] netCfg     Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in] targetIpAddr   Pointer to a string containing target ip address,
*                           can be broadcast
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for responses from the network
* @param[out] reply         Structure that contains:
*                           - USB2 High speed
*                           - Support for MSA
*                           - Vhub status
*                           - Filter status
*                           - MAC Address
*                           - Mac Address of the paired unit
*                           - Port Number
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
int ICRON_requestConfigurationResponseData(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress);


/**
* @brief    Pings a single device at IP specified in call to
*           ICRON_usbNetCfgInit()
*           CR-DF: This description seems wrong.  I don't think that the device
*           pinged has anything to do with the init function call.
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in] targetIpAddr   Pointer to a string containing target ip address,
*                           can be broadcast
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for response from device
*
* @return   0 on success, -1 on failure
*/
int ICRON_pingDevice(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress);

/**
* @brief    Sends a request to the specified device and reset the device
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in]  cmd           Structure that contains:
*                           - targetIPAddr
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for a response to the request
*
* @return  0    Device located and reset successfully
*         -1    Could not locate the device because UDP packet was not received.
*/
int ICRON_resetDevice(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress);

/**
* @brief    Sends a Pair To Device request containing the user provided MAC
*           address, to the specified device and waits for the device to
*           acknowledge the request
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in] targetIpAddr   Pointer to a string containing target ip address,
*                           can be broadcast
*           CR-DF: I think this command is the one request which really should
*                  not be a broadcast because it only makes sense to tell one
*                  device to pair to a given MAC
* @param[in] pairToMac      Pointer to a six byte array containing the MAC
*                           address to pair too
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for a response to the request
*
* @return   zero if found, -1 if not found.
* CR-DF: perhaps this should be "0 on success or -1 on failure"
*/
int ICRON_pairToDevice(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress,
    const uint8_t* pairToMac);

/**
* @brief    Sends a request to the specified device and starts LED locator pattern
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in]  cmd           Structure that contains:
*                           - targetIPAddr
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for a response to the request
*
* @return  0    Device located succesfully and led pattern started.
*         -1    Could not locate the device because UDP packet was not received.
*/
int ICRON_ledLocatorOn(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress);

/**
* @brief    Sends a request to the specified device and stops LED locator pattern
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in]  cmd           Structure that contains:
*                           - targetIPAddr
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for a response to the request
*
* @return  0    Device located succesfully and led pattern started.
*         -1    Could not locate the device because UDP packet was not received.
*/
int ICRON_ledLocatorOff(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress);

/**
* @brief    Sends a Remove Device Pairing request to the specified device and
*           waits for it to acknowledge the request
*
* @param[in,out] currCfg        Pointer to a previously intialized IcronUsbNetCfgT structure
* @param[in] targetMACAddress   String containing the MAC address of the device to remove a pairing
*                               from
* @param[in] pairedMacAddress   The MAC address pairing to remove. 
* @param[in] msgId              A user provided message Id
* @param[in] timeOut            Time to wait for a response to the request
*
* @return   zero if found, -1 if not found.
* CR-DF: perhaps this should be "0 on success or -1 on failure"
*/
int ICRON_removeDevicePairing(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress,
    const uint8_t* pairedMACAddress);


/**
* @brief    Loads the user provided array of IcronDevTopologyT structures with
*           topology information returned from the specified LEX.  The array
*           should contain MAX_USB_DEVICES elements, the maximum number of USB
*           devices supported by a LEX.
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
* @param[in] targetIpAddr   Pointer to a string containing target ip address,
*                           can be broadcast
*       CR-DF: It looks like we only support receiving from a single device
*       right now.
* @param[out] userCopy      Pointer to a user allocated IcronDevTopologyT
*                           array.
* @param[in] msgId          A user provided message Id
* @param[in] timeOut        Time to wait for a response to the request
*
* @return   Number of records loaded in the array, -1 if not found.
*/
int ICRON_requestDeviceTopology(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress);


/**
* @brief    Sends a Remove All Pairings request to the specified device and waits for it to
*           acknowledge the request.  The result is that any pairings on the device will be
*           removed.
*
* @param[in,out] currCfg            Pointer to a previously intialized IcronUsbNetCfgT structure
* @param[in] networkBroadcastIP     Pointer to a string containing the network broadcast IP address
* @param[in] msgId                  A user provided message Id
* @param[in] timeOut                Time to wait for a response to the request
*
* @return   TRUE on success and FALSE on error.
*/
int ICRON_removeAllPairings(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress);

/**
* @brief    Sends a message to set the filtering strategy on the device.
*
* @param[in,out] currCfg            Pointer to a previously intialized IcronUsbNetCfgT structure
* @param[in] timeOut                Time to wait for a response to the request
* @param[in] networkBroadcastIP     Pointer to a string containing the network broadcast IP address
* @param[in] targetMACAddress       String containing the MAC address of the device
* @param[in] filteringStrategy      Fltering strategy that needs to be set on the device
*
* @return   TRUE on success and FALSE on error.
*/
int ICRON_useFilteringStrategy(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress,
    uint8_t filteringStrategy);

#endif // XUSBNETCFG_H

