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
//!   @file xusbnetcfglib.c
//!         Command line utility to configure LEXs and REXs over the network
//!         using UDP.
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/

#ifdef __MINGW32__
#include <winsock2.h>
#include <WS2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include "xusbnetcfg.h"


#define MAGIC_NUMBER        0xA9C4D8F4
#define MAC_ADDR_LENGTH     6
#define XUSBNETCFG_PORT     6137

// On the wire command codes
#define CMD_REQUEST_DEVICE_INFORMATION              0
#define CMD_REPLY_DEVICE_INFORMATION                1
#define CMD_PING                                    2
#define CMD_ACKNOWLEDGE                             3
#define CMD_PAIR_TO_DEVICE                          4
#define CMD_REMOVE_DEVICE_PAIRING                   5
#define CMD_REQUEST_DEVICE_TOPOLOGY                 6
#define CMD_REPLY_DEVICE_TOPOLOGY                   7
#define CMD_REPLY_UNHANDLED_COMMAND                 8
#define CMD_NEGATIVE_ACKNOWLEDGE                    9
#define CMD_REMOVE_ALL_PAIRINGS                     10
#define CMD_USE_FILTERING_STRATEGY                  11
#define CMD_REQUEST_CONFIGURATION_RESPONSE_DATA     12
#define CMD_REPLY_CONFIGURATION_RESPONSE_DATA       13
#define CMD_RESET_DEVICE                            14
#define CMD_LED_LOCATOR_ON                          15
#define CMD_LED_LOCATOR_OFF                         16

// The structs below define the over the wire format of the protocol.  Multi-byte values are
// big-endian.  These structs are used only for packing and unpacking data being
// transmitted/received.

/// Common data to all messages
struct MsgBase
{
    /// Value 0xA9C4D8F4 that begins all packets to help identify it as belonging to this protocol.
    uint32_t        magicNumber;
    /// When the client sends a request, it chooses any value to insert
    /// in this field.  The device responding to the request will set this
    /// field in the reply to the same value it received in the request.
    uint32_t        messageId;
    /// The command identifier.  Corresponds to the CMD_* defines.
    uint8_t         command;
    /// The MAC address of the device which is the target of this message
    uint8_t         deviceMacAddress[6];
} __attribute__((packed));

/// Message for requesting information from a device.
struct MsgRequestDeviceInformation
{
    struct MsgBase  base;
} __attribute__((packed));

/// Information about a device.
struct MsgReplyDeviceInformation
{
    struct MsgBase  base;
    /// NUL terminated UTF-8 string specifying the device's vendor
    char            vendor[REPLY_DEV_INFO_VENDOR_LENGTH];
    /// NUL terminated UTF-8 string specifying the device's product name
    char            product[REPLY_DEV_INFO_PRODUCT_LENGTH];
    /// NUL terminated UTF-8 string specifying the device's revision
    char            revision[REPLY_DEV_INFO_REVISION_LENGTH];
    /// Is the device a local extender or a remote extender. 0 = LEX, 1 = REX
    uint8_t         lexOrRex;
    /// The protocol version that this device supports.  A client may have knowledge of which
    /// commands to use as a result of reading this value.
    uint8_t         supportedProtocolVersion;
    /// MAC addresses of the extenders that this device is paired with
    uint8_t         pairedWithMacAddress[REPLY_DEV_INFO_MAX_PAIRS][6];
} __attribute__((packed));

/// Message for requesting configuration response data from a device.
struct MsgRequestConfigurationResponseData
{
    struct MsgBase  base;
} __attribute__((packed));

/// Configuration Response Data from a device.
struct MsgReplyConfigurationResponseData
{
    // MsgBase
    struct MsgBase  base;
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
    /// The fill to make the structure aligned
    uint8_t fill1[1];
    /// MAC address of the device
    uint8_t mac[6];
    /// The fill to make the structure aligned
    uint8_t fill2[2];
    /// MAC address of the paired devices
    uint8_t pairedMacs[7][6];
    /// Port number of the connected unit
    uint16_t portNumber;
    /// Vhub - Number of Downstream ports
    uint8_t numVhubPorts;
    /// The fill to make the structure aligned
    uint8_t fill3 [1];
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
} __attribute__((packed)) replyConfigurationResponseData;

/// Checks if a device is responsive.
struct MsgPing
{
    struct MsgBase  base;
} __attribute__((packed));

/// Resets the device.
struct MsgResetDevice
{
    struct MsgBase  base;
} __attribute__((packed));

/// Turns on LED Locator.
struct MsgLedLocatorOn
{
    struct MsgBase  base;
} __attribute__((packed));


/// Turns off LED Locator.
struct MsgLedLocatorOff
{
    struct MsgBase  base;
} __attribute__((packed));

/// General acknowledgement packet which is sent from device to client in response to many client
/// requests.
struct MsgAck
{
    struct MsgBase  base;
} __attribute__((packed));

/// A message for establishing a new device pairing.
struct MsgPairToDevice
{
    struct MsgBase  base;
    /// The MAC address of the device to pair to.
    uint8_t         pairToDeviceMacAddress[6];
} __attribute__((packed));

/// A message for removing an existing pairing.
struct MsgRemoveDevicePairing
{
    struct MsgBase  base;
    /// The MAC address of the pairing to eliminate.  This is required because a device may have
    /// multiple pairs.
    uint8_t         pairedMacAddress[6];
} __attribute__((packed));

/// Request the topology information stored within the extender.  Only the LEX will reply with
/// useful information.  Rex will NAK.
struct MsgRequestDeviceTopology
{
    struct MsgBase  base;
} __attribute__((packed));

/// The device topology information sent in response to a RequestDeviceTopology message.
struct MsgReplyDeviceTopology
{
    struct MsgBase  base;
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
    } __attribute__((packed)) devices[MAX_USB_DEVICES];
} __attribute__((packed));

/// A message sent from a device to a client to indicate that a request was received, but the
/// device does not understand this type of message.
struct MsgReplyUnhandledCommand
{
    struct MsgBase base;
} __attribute__((packed));

/// A message sent from a device to a client to indicate that a request was received, but was not
/// accepted.
struct MsgNegativeAcknowledge
{
    struct MsgBase base;
} __attribute__((packed));

/// When received by a device, this message will result in the removal of all stored pairings from
/// the device.
struct MsgRemoveAllPairings
{
    struct MsgBase base;
} __attribute__((packed));

/// When received by a device, this message will set the filtering strategy.
struct MsgUseFilteringStrategy
{
    struct MsgBase base;
    uint8_t filteringStrategy;
} __attribute__((packed));

// Aggregate structs together so that we can easily see how big of a buffer we need to support
// receive or transmit messages.
union GenericClientToDeviceMessage
{
    struct MsgRequestDeviceInformation          requestDeviceInformation;
    struct MsgRequestConfigurationResponseData  requestConfigurationResponseData;
    struct MsgPing                              ping;
    struct MsgResetDevice                       resetDevice;
    struct MsgLedLocatorOn                      ledLocatorOn;
    struct MsgLedLocatorOff                     ledLocatorOff;
    struct MsgPairToDevice                      pairToDevice;
    struct MsgRemoveDevicePairing               removeDevicePairing;
    struct MsgRequestDeviceTopology             requestDeviceTopology;
    struct MsgRemoveAllPairings                 removeAllPairings;
} __attribute__((packed));

union GenericDeviceToClientMessage
{
    struct MsgReplyDeviceInformation            replyDeviceInformation;
    struct MsgReplyConfigurationResponseData    replyConfigurationResponseData;
    struct MsgAck                               ack;
    struct MsgReplyDeviceTopology               replyDeviceTopology;
    struct MsgNegativeAcknowledge               negativeAcknowledge;
} __attribute__((packed));

union GenericMessage
{
    union GenericClientToDeviceMessage clientToDevice;
    union GenericDeviceToClientMessage deviceToClient;
} __attribute__((packed));


struct ValidResponse
{
    uint8_t sent;
    uint8_t response;
};

// This data structure lists pairs with request and a valid response.  Note that
// CMD_REPLY_UNHANDLED_COMMAND is a valid response for any request
const static struct ValidResponse validResponses[] =
{
    {CMD_REQUEST_DEVICE_INFORMATION,            CMD_REPLY_DEVICE_INFORMATION},
    {CMD_PING,                                  CMD_ACKNOWLEDGE},
    {CMD_PAIR_TO_DEVICE,                        CMD_ACKNOWLEDGE},
    {CMD_PAIR_TO_DEVICE,                        CMD_NEGATIVE_ACKNOWLEDGE},
    {CMD_REMOVE_DEVICE_PAIRING,                 CMD_ACKNOWLEDGE},
    {CMD_REMOVE_DEVICE_PAIRING,                 CMD_NEGATIVE_ACKNOWLEDGE},
    {CMD_REQUEST_DEVICE_TOPOLOGY,               CMD_REPLY_DEVICE_TOPOLOGY},
    {CMD_REQUEST_DEVICE_TOPOLOGY,               CMD_NEGATIVE_ACKNOWLEDGE},
    {CMD_REMOVE_ALL_PAIRINGS,                   CMD_ACKNOWLEDGE},
    {CMD_USE_FILTERING_STRATEGY,                CMD_ACKNOWLEDGE},
    {CMD_USE_FILTERING_STRATEGY,                CMD_NEGATIVE_ACKNOWLEDGE},
    {CMD_REQUEST_CONFIGURATION_RESPONSE_DATA,   CMD_REPLY_CONFIGURATION_RESPONSE_DATA},
    {CMD_RESET_DEVICE,                          CMD_ACKNOWLEDGE},
    {CMD_LED_LOCATOR_ON,                        CMD_ACKNOWLEDGE},
    {CMD_LED_LOCATOR_OFF,                       CMD_ACKNOWLEDGE}
};


struct IcronUsbNetCfg
{
    fd_set socketFdSet;
    struct sockaddr_in sendTrgtAddr;            // required, though could be passed to SendUdpPkt()
    int sendTrgtAddrLen;
    struct sockaddr_in recvTrgtAddr;
    socklen_t recvTrgtAddrLen;
    union GenericClientToDeviceMessage sendMsg;
    union GenericDeviceToClientMessage recvMsg;
    // TODO: The sendMsgSize global could be removed if sendUdpPkt took a parameter
    unsigned int sendMsgSize;
    int cfgSock_fd;
};

/************************ Local Function Prototypes **************************/

/**
* @brief    Sets the destination address of the packet to be sent.
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfg
*                           structure
* @param[in] targetIpAddr   Pointer to a string containing remote ip address,
*                           can be broadcast
*
* @return   0 on success, -1 on failure
*/
static int SetTargetAddress(struct IcronUsbNetCfg* currCfg, const char *targetIpAddr);

/**
* @brief    Sets up the socket for configuring REXs and LEXs.
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfg
*                           structure
*
* @return   0 on success, -1 on failure
*/
static int InitSocket(struct IcronUsbNetCfg* currCfg);

/**
* @brief    Sets up the socket to broadcast mode
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfg
*                           structure
*
* @return   0 on success, -1 on failure
*/
static int SetSockOptAllowBroadcast(struct IcronUsbNetCfg* currCfg);

/**
* @brief    Closes the socket for this user.
*
* @param[in] currCfg    Pointer to a previously intialized IcronUsbNetCfg
*                       structure
*
* @return   0 on success, -1 on failure
*/
static int CloseSocket(struct IcronUsbNetCfg* currCfg);

/**
* @brief    Sends the provided buffer out the users socket.
*
* @param[in] currCfg    Pointer to a previously intialized IcronUsbNetCfg
*                       structure
*
* CR-DF: It looks like the implementation is just passing back the result of a
* sendto function call.  The man page indicates that it returns -1 on error or
* the number of bytes sent in the success case.  Either change the docs or
* change the implementation to always return 0 on success.
*
* @return   0 on success, -1 on failure
*/
static int SendUdpPkt(struct IcronUsbNetCfg* currCfg);

/**
* @brief    Receives a udp packet off the users socket.
*
* @param[in] currCfg    Pointer to a previously intialized IcronUsbNetCfg
*                       structure
* @param[in] timeOut    seconds to wait for response
*
* @return   number of bytes received on success, -1 on failure
*/
static int RecvUdpPkt(struct IcronUsbNetCfg* currCfg, int timeOut);

static bool receivedPacketValid(const struct IcronUsbNetCfg* currCfg, int numBytesReceived);
static enum ReplyCommandType lookupReplyCommandType(uint8_t wireCommandValue);
static void setBasePacketAttributes(
    struct MsgBase* base, uint32_t msgId, uint8_t command, const uint8_t* targetMACAddress);


/************************** Function Definitions *****************************/


static int SetTargetAddress(struct IcronUsbNetCfg* currCfg, const char *targetIpAddr)
{
    memset(&currCfg->sendTrgtAddr, 0, sizeof(struct sockaddr_in));
    currCfg->sendTrgtAddr.sin_family = AF_INET;
    currCfg->sendTrgtAddr.sin_port = htons(XUSBNETCFG_PORT);
    // currCfg->sendTrgtAddr.sin_addr.s_addr = htonl(INADDR_ANY);
#ifdef __MINGW32__
    currCfg->sendTrgtAddr.sin_addr.s_addr = inet_addr(targetIpAddr);
    if (currCfg->sendTrgtAddr.sin_addr.s_addr == INADDR_NONE && strncmp(targetIpAddr, "255.255.255.255", 16) != 0)
#else
    // This function is only on Vista & later for Windows hosts, so for windows we use the above code
    if (0 == inet_pton(AF_INET, targetIpAddr, &currCfg->sendTrgtAddr.sin_addr))
#endif
    {
        PRINTF(strerror(errno));
        return -1;
    }
    currCfg->sendTrgtAddrLen = sizeof(struct sockaddr_in);
    return 0;
}


static int InitSocket(struct IcronUsbNetCfg* currCfg)
{
    int rc;

    if (currCfg == NULL) return -1;

    /* Create and configure socket */
    currCfg->cfgSock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (currCfg->cfgSock_fd == -1) {
        PRINTF(strerror(errno));
        return -1;
    }
#if 0
    /* Allow us to send broadcast messages */
    rc = setsockopt(currCfg->cfgSock_fd, SOL_SOCKET,
                    SO_BROADCAST, &optVal, optLen);
    if (rc == -1) {
        PRINTF(strerror(errno));
        return -1;
    }
#endif
    /* Set socket to non-blocking */
#ifdef __MINGW32__
    // The FIONBIO uses an arg (flags) of non-zero to indicate non-block
    unsigned long flags = 1;
    rc = ioctlsocket(currCfg->cfgSock_fd, FIONBIO, &flags);
    if (rc == SOCKET_ERROR)
#else
    rc = fcntl(currCfg->cfgSock_fd, F_SETFL, O_NONBLOCK);
    if (rc == -1)
#endif
    {
        PRINTF(strerror(errno));
        return -1;
    }
    currCfg->recvTrgtAddrLen = sizeof(currCfg->recvTrgtAddr);

    /* Set up the file descriptor set for use by select */
    FD_ZERO(&currCfg->socketFdSet);
    FD_SET(currCfg->cfgSock_fd, &currCfg->socketFdSet);

    return 0;
}

static int SetSockOptAllowBroadcast(struct IcronUsbNetCfg* currCfg)
{
    int rc;
#ifdef __MINGW32__
    char optVal = 1;
#else
    int optVal = 1;
#endif
    int optLen = sizeof(optVal);

    if (currCfg == NULL) return -1;

    rc = setsockopt(currCfg->cfgSock_fd, SOL_SOCKET,
                    SO_BROADCAST, &optVal, optLen);
    if (rc == -1) {
        PRINTF(strerror(errno));
    }
    return rc;
}


static int CloseSocket(struct IcronUsbNetCfg* currCfg)
{
    int rc = 0;
#ifdef __MINGW32__
    rc = closesocket(currCfg->cfgSock_fd);
#else
    rc = close(currCfg->cfgSock_fd);
#endif
    if (rc == -1)
    {
        PRINTF(strerror(errno));
    }
    return rc;
}


static int SendUdpPkt(struct IcronUsbNetCfg* currCfg)
{
    int rc;

    if (currCfg == NULL) return -1;
    DEBUG("currCfg->cfgSock_fd = %d\n", currCfg->cfgSock_fd);
    DEBUG("currCfg->sendMsgSize = %d\n", currCfg->sendMsgSize);
    DEBUG("currCfg->sendTrgtAddrlen = %d\n", currCfg->sendTrgtAddrLen);
    rc = sendto(
            currCfg->cfgSock_fd,
            (char *)&currCfg->sendMsg,
            currCfg->sendMsgSize,
            0,
            (const struct sockaddr *) &currCfg->sendTrgtAddr,
            currCfg->sendTrgtAddrLen);
    if (rc == -1)
    {
        PRINTF(strerror(errno));
    }
    return rc;
}


// CR-DF: I believe that this function is making n=timeOut calls to recvfrom on
// a non-blocking socket and therefore each call to recvfrom will either
// receive zero or one packets and then sleep for 1 second.  On a network with
// 30 devices, we would have to set timeout to >= 30 to even have a chance at
// getting all of the responses even if all of the devices reply in under one
// second.  I think we need to use a different I/O model that will allow us to
// receive as many packets as possible and stop receiving after n=timeOut
// seconds.
static int RecvUdpPkt(struct IcronUsbNetCfg* currCfg, int timeout)
{
    int rc = 0;
    if (currCfg == NULL) return -1;

    while (1)
    {
        struct timeval selectTimeOut;
        fd_set writefds;
        fd_set exceptfds;
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);

        // TBD, use of time out in this case, as long as we receive responses within the time out
        // period, would be the time from the last packet received.

        selectTimeOut.tv_sec = timeout;
        selectTimeOut.tv_usec = 0;
        rc = select(FD_SETSIZE, &currCfg->socketFdSet, &writefds, &exceptfds, &selectTimeOut);
        if (rc == -1)
        {
            PRINTF(strerror(errno));
            return -1;
        }
        else if(rc == 0)
        {
            DEBUG("Timed out from select() call");
            // We've timed out
            break;
        }
        else
        {
            DEBUG("Calling recvfrom(), rc = %d\n", rc);
            rc = recvfrom(
                currCfg->cfgSock_fd,
                (char *)&currCfg->recvMsg,
                sizeof(currCfg->recvMsg),
                0,
                // CR-DF: It looks like recvTrgtAddr will always be NULL.  Should we be setting
                // recvTrgtAddr somewhere if we find that the address we sent the request out to is
                // not a broadcast address?
                (struct sockaddr *) &currCfg->recvTrgtAddr,
                &currCfg->recvTrgtAddrLen);
            DEBUG("Returned from recvfrom(), rc = %d\n", rc);
#ifdef __MINGW32__
            if (rc == -1)
            {
                PRINTF("Windows failure %d\n", WSAGetLastError());
                return -1;
            }
#else
            if ((rc == -1) && (errno != EAGAIN)) {
                PRINTF(strerror(errno));
                return -1;
            }
#endif
            else if(receivedPacketValid(currCfg, rc))
            {
                // Got a valid packet, return it to the caller
                DEBUG("Got a valid packet, return it to the caller\n");
                break;
            }
            else
            {
                printf("Received packet is invalid\n");
            }
        }
    }
    return rc;
}


struct IcronUsbNetCfg* ICRON_usbNetCfgInit(void)
{
    struct IcronUsbNetCfg* currCfg = NULL;
#ifdef __MINGW32__
    int iResult;
    WSADATA wsaData;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        PRINTF("WSAStartup failed: %d\n", iResult);
        return NULL;
    }
#endif

    currCfg = calloc(1, sizeof(struct IcronUsbNetCfg));
    if (currCfg != NULL)
    {
        int rc;
        rc = InitSocket(currCfg);
        if (rc == -1)
        {
            free(currCfg);
            currCfg = NULL;
        }
    }
    return currCfg;
}


// TODO: It seems like this is never called.  Should it be removed?
int ICRON_usbNetCfgRelease(struct IcronUsbNetCfg* currCfg)
{
    if (currCfg == NULL)
    {
        return -1;
    }
    CloseSocket(currCfg);
    free(currCfg);

#ifdef __MINGW32__
    if (WSACleanup() != 0)
    {
        PRINTF("WSACleanup failed\n");
    }
#endif
    return 0;
}


int ICRON_requestDeviceInformation(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress)
{
    const uint32_t msgId = rand();
    unsigned int replyNumber = 0;

    int rc = SetSockOptAllowBroadcast(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    memset(&currCfg->sendMsg, 0, sizeof(currCfg->sendMsg));
    SetTargetAddress(currCfg, networkBroadcastIP);
    setBasePacketAttributes((struct MsgBase*)(&currCfg->sendMsg), msgId, CMD_REQUEST_DEVICE_INFORMATION, targetMACAddress);
    currCfg->sendMsgSize = sizeof(struct MsgRequestDeviceInformation);

    rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }

    // TODO: should make sure that we don't receive more responses than we can handle based on the
    // number of elements in the response array
    while(1)
    {
        rc = RecvUdpPkt(currCfg, timeout);
        if(rc == 0)
        {
            // This indicates that a timeout occurred
            break;
        }
        else if(rc == -1)
        {
            DEBUG("Got an error when receiving responses to a request device info");
        }
        else
        {
            struct MsgBase* base = (struct MsgBase*)(&currCfg->recvMsg);
            response[replyNumber].command = lookupReplyCommandType(base->command);
            memcpy(response[replyNumber].macAddress, base->deviceMacAddress, MAC_ADDR_LENGTH);
            switch(response[replyNumber].command)
            {
                case REPLY_CMD_REPLY_DEVICE_INFORMATION:
                    {
                        struct MsgReplyDeviceInformation* rdi =
                            (struct MsgReplyDeviceInformation*)(&currCfg->recvMsg);
                        memcpy(
                            response[replyNumber].replyDeviceInformation.vendor,
                            rdi->vendor,
                            sizeof(response[replyNumber].replyDeviceInformation.vendor));
                        memcpy(
                            response[replyNumber].replyDeviceInformation.product,
                            rdi->product,
                            sizeof(response[replyNumber].replyDeviceInformation.product));
                        memcpy(
                            response[replyNumber].replyDeviceInformation.revision,
                            rdi->revision,
                            sizeof(response[replyNumber].replyDeviceInformation.revision));
                        response[replyNumber].replyDeviceInformation.lexOrRex = rdi->lexOrRex;
                        response[replyNumber].replyDeviceInformation.supportedProtocolVersion =
                            rdi->supportedProtocolVersion;
                        // Lower level code has already verified that the packet is an acceptable
                        // length
                        response[replyNumber].replyDeviceInformation.numPairs =
                            (rc - offsetof(struct MsgReplyDeviceInformation, pairedWithMacAddress)) / MAC_ADDR_LENGTH;
                        memcpy(
                            response[replyNumber].replyDeviceInformation.pairedWithMacAddress,
                            rdi->pairedWithMacAddress,
                            MAC_ADDR_LENGTH * response[replyNumber].replyDeviceInformation.numPairs);
                    }
                    break;

                case REPLY_CMD_REPLY_UNHANDLED_COMMAND:
                    // no command specific data to copy over
                    break;

                default:
                    break;
            }
            replyNumber++;
        }
    }

    return replyNumber;
}

int ICRON_requestConfigurationResponseData(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress)
{
    const uint32_t msgId = rand();
    unsigned int replyNumber = 0;

    // Allow broadcast data
    int rc = SetSockOptAllowBroadcast(currCfg);
    if (rc == -1)
    {
        printf("SetSockOptAllowBroadcast failed\n");
        return rc;
    }

    memset(&currCfg->sendMsg, 0, sizeof(currCfg->sendMsg));
    SetTargetAddress(currCfg, networkBroadcastIP);
    setBasePacketAttributes((struct MsgBase*)(&currCfg->sendMsg), msgId, CMD_REQUEST_CONFIGURATION_RESPONSE_DATA, targetMACAddress);
    currCfg->sendMsgSize = sizeof(struct MsgRequestConfigurationResponseData);
    rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
            printf("SendUdpPkt failed\n");
        return rc;
    }

    while (1)
    {
        rc = RecvUdpPkt(currCfg, timeout);
        if(rc == 0)
        {
            // This indicates that a timeout occurred
            break;
        }
        else if(rc == -1)
        {
            DEBUG("Got an error when receiving responses to a request device info");
        }
        else
        {
            struct MsgBase* base = (struct MsgBase*)(&currCfg->recvMsg);
            response[replyNumber].command = lookupReplyCommandType(base->command);
            memcpy(response[replyNumber].macAddress, base->deviceMacAddress, MAC_ADDR_LENGTH);
            switch(response[replyNumber].command)
            {
                case REPLY_CMD_REPLY_CONFIGURATION_RESPONSE_DATA:
                    {
                        struct MsgReplyConfigurationResponseData* devInfo =
                            (struct MsgReplyConfigurationResponseData*)(&currCfg->recvMsg);

                        response[replyNumber].replyConfigurationResponseData.highSpeed = devInfo->highSpeed;
                        response[replyNumber].replyConfigurationResponseData.msa = devInfo->msa;
                        response[replyNumber].replyConfigurationResponseData.vhub = devInfo->vhub;
                        response[replyNumber].replyConfigurationResponseData.currentFilterStatus = devInfo->currentFilterStatus;

                        memcpy(response[replyNumber].replyConfigurationResponseData.mac, devInfo->mac, 6);
                        int numPairedMacs = ((sizeof(devInfo->pairedMacs))/(6*sizeof(uint8_t)));
                        int mac;
                        for (mac = 0; mac < numPairedMacs; mac++)
                            {
                                memcpy(response[replyNumber].replyConfigurationResponseData.pairedMacs[mac], devInfo->pairedMacs[mac], 6);
                            }

                        // Port Number is stored as Big Endian but is printed on a
                        // Little Endian machine. Therefore, ntohs is used to do the
                        // endian conversion.
                        response[replyNumber].replyConfigurationResponseData.portNumber = ntohs(devInfo->portNumber);

                        response[replyNumber].replyConfigurationResponseData.numVhubPorts = devInfo->numVhubPorts;
                        // VID, PID and Brand Id are stored as Big Endian but are
                        // printed on a Little Endian machine. Therefore, ntohs
                        // is used to do the endian conversion.
                        response[replyNumber].replyConfigurationResponseData.vhubVid = ntohs(devInfo->vhubVid);
                        response[replyNumber].replyConfigurationResponseData.vhubPid = ntohs(devInfo->vhubPid);

                        response[replyNumber].replyConfigurationResponseData.brandId = ntohs(devInfo->brandId);
                        memcpy(response[replyNumber].replyConfigurationResponseData.vendor, devInfo->vendor,
                               sizeof(response[replyNumber].replyConfigurationResponseData.vendor));

                        memcpy(response[replyNumber].replyConfigurationResponseData.product, devInfo->product,
                               sizeof(response[replyNumber].replyConfigurationResponseData.product));
                        memcpy(response[replyNumber].replyConfigurationResponseData.revision, devInfo->revision,
                               sizeof(response[replyNumber].replyConfigurationResponseData.revision));
                    }
                    break;

                case REPLY_CMD_REPLY_UNHANDLED_COMMAND:
                    // no command specific data to copy over
                    break;

                default:
                    break;
            }
            replyNumber++;
        }
    }

    return replyNumber;
//    return 1;
}

int ICRON_pingDevice(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress)
{
    const uint32_t msgId = rand();
    int rc;

    memset(&currCfg->sendMsg, 0, sizeof(currCfg->sendMsg));
    SetTargetAddress(currCfg, networkBroadcastIP);
    setBasePacketAttributes((struct MsgBase*)(&currCfg->sendMsg), msgId, CMD_PING, targetMACAddress);
    currCfg->sendMsgSize = sizeof(struct MsgPing);

    rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    rc = RecvUdpPkt(currCfg, timeout);
    if (rc != -1)
    {
        DEBUG("ping returned %d bytes\n", rc);
        if(rc != 0)
        {
            struct MsgBase* base = (struct MsgBase*)(&currCfg->recvMsg);
            response->command = lookupReplyCommandType(base->command);
            memcpy(response->macAddress, base->deviceMacAddress, MAC_ADDR_LENGTH);

            // Both ACK and UNHANDLED_COMMAND are valid, but neither one has additional fields to copy
            // over.
            rc = 1;
        }
    }
    return rc;
}

int ICRON_resetDevice(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress)
{
    const uint32_t msgId = rand();
    int rc;

    memset(&currCfg->sendMsg, 0, sizeof(currCfg->sendMsg));
    SetTargetAddress(currCfg, networkBroadcastIP);
    setBasePacketAttributes((struct MsgBase*)(&currCfg->sendMsg), msgId, CMD_RESET_DEVICE, targetMACAddress);
    currCfg->sendMsgSize = sizeof(struct MsgResetDevice);

    rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    rc = RecvUdpPkt(currCfg, timeout);
    if (rc != -1)
    {
        if(rc != 0)
        {
            struct MsgBase* base = (struct MsgBase*)(&currCfg->recvMsg);
            response->command = lookupReplyCommandType(base->command);
            memcpy(response->macAddress, base->deviceMacAddress, MAC_ADDR_LENGTH);

            // Both ACK and UNHANDLED_COMMAND are valid, but neither one has additional fields to copy
            // over.
            rc = 1;
        }
    }
    return rc;
}

int ICRON_ledLocatorOn(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress)
{
    const uint32_t msgId = rand();
    int rc;

    memset(&currCfg->sendMsg, 0, sizeof(currCfg->sendMsg));
    SetTargetAddress(currCfg, networkBroadcastIP);
    setBasePacketAttributes((struct MsgBase*)(&currCfg->sendMsg), msgId, CMD_LED_LOCATOR_ON, targetMACAddress);
    currCfg->sendMsgSize = sizeof(struct MsgLedLocatorOn);

    rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    rc = RecvUdpPkt(currCfg, timeout);
    if (rc != -1)
    {
        if(rc != 0)
        {
            struct MsgBase* base = (struct MsgBase*)(&currCfg->recvMsg);
            response->command = lookupReplyCommandType(base->command);
            memcpy(response->macAddress, base->deviceMacAddress, MAC_ADDR_LENGTH);

            // Both ACK and UNHANDLED_COMMAND are valid, but neither one has additional fields to copy
            // over.
            rc = 1;
        }
    }
    return rc;
}

int ICRON_ledLocatorOff(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress)
{
    const uint32_t msgId = rand();
    int rc;

    memset(&currCfg->sendMsg, 0, sizeof(currCfg->sendMsg));
    SetTargetAddress(currCfg, networkBroadcastIP);
    setBasePacketAttributes((struct MsgBase*)(&currCfg->sendMsg), msgId, CMD_LED_LOCATOR_OFF, targetMACAddress);
    currCfg->sendMsgSize = sizeof(struct MsgLedLocatorOff);

    rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    rc = RecvUdpPkt(currCfg, timeout);
    if (rc != -1)
    {
        if(rc != 0)
        {
            struct MsgBase* base = (struct MsgBase*)(&currCfg->recvMsg);
            response->command = lookupReplyCommandType(base->command);
            memcpy(response->macAddress, base->deviceMacAddress, MAC_ADDR_LENGTH);

            // Both ACK and UNHANDLED_COMMAND are valid, but neither one has additional fields to copy
            // over.
            rc = 1;
        }
    }
    return rc;
}


int ICRON_pairToDevice(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress,
    const uint8_t* pairToMac)
{
    int rc;
    const uint32_t msgId = rand();
    struct MsgPairToDevice* pairToDeviceMsg = (struct MsgPairToDevice*)&currCfg->sendMsg;

    memset(&currCfg->sendMsg, 0, sizeof(currCfg->sendMsg));
    SetTargetAddress(currCfg, networkBroadcastIP);
    setBasePacketAttributes((struct MsgBase*)(&currCfg->sendMsg), msgId, CMD_PAIR_TO_DEVICE, targetMACAddress);
    memcpy(pairToDeviceMsg->pairToDeviceMacAddress, pairToMac, MAC_ADDR_LENGTH);
    currCfg->sendMsgSize = sizeof(struct MsgPairToDevice);

    rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    rc = RecvUdpPkt(currCfg, timeout);
    if (rc != -1)
    {
        DEBUG("pair to device request returned %d bytes\n", rc);
        if(rc != 0)
        {
            struct MsgBase* base = (struct MsgBase*)(&currCfg->recvMsg);
            response->command = lookupReplyCommandType(base->command);
            memcpy(response->macAddress, base->deviceMacAddress, MAC_ADDR_LENGTH);

            // ACK, NAK and UNHANDLED_COMMAND are valid, but neither one has additional fields to copy
            // over.
            rc = 1;
        }
    }
    return rc;
}


int ICRON_removeDevicePairing(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress,
    const uint8_t* pairedMACAddress)
{
    int rc;
    const uint32_t msgId = rand();
    struct MsgRemoveDevicePairing* removeDevicePairingMsg = (struct MsgRemoveDevicePairing*)(&currCfg->sendMsg);

    memset(&currCfg->sendMsg, 0, sizeof(currCfg->sendMsg));
    SetTargetAddress(currCfg, networkBroadcastIP);
    setBasePacketAttributes((struct MsgBase*)(&currCfg->sendMsg), msgId, CMD_REMOVE_DEVICE_PAIRING, targetMACAddress);
    memcpy(removeDevicePairingMsg->pairedMacAddress, pairedMACAddress, MAC_ADDR_LENGTH);
    currCfg->sendMsgSize = sizeof(struct MsgRemoveDevicePairing);

    rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    rc = RecvUdpPkt(currCfg, timeout);
    if (rc != -1)
    {
        DEBUG("request returned %d bytes\n", rc);
        if(rc != 0)
        {
            struct MsgBase* base = (struct MsgBase*)(&currCfg->recvMsg);
            response->command = lookupReplyCommandType(base->command);
            memcpy(response->macAddress, base->deviceMacAddress, MAC_ADDR_LENGTH);

            // ACK, NAK and UNHANDLED_COMMAND are valid, but neither one has additional fields to copy
            // over.
            rc = 1;
        }
    }
    return rc;
}


int ICRON_requestDeviceTopology(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress)
{
    int rc;
    const uint32_t msgId = rand();

    memset(&currCfg->sendMsg, 0, sizeof(currCfg->sendMsg));
    SetTargetAddress(currCfg, networkBroadcastIP);
    setBasePacketAttributes((struct MsgBase*)(&currCfg->sendMsg), msgId, CMD_REQUEST_DEVICE_TOPOLOGY, targetMACAddress);
    currCfg->sendMsgSize = sizeof(struct MsgRequestDeviceTopology);

    rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    rc = RecvUdpPkt(currCfg, timeout);
    if (rc != -1)
    {
        DEBUG("request device topology returned %d bytes\n", rc);
        if(rc != 0)
        {
            struct MsgBase* base = (struct MsgBase*)(&currCfg->recvMsg);
            response->command = lookupReplyCommandType(base->command);
            memcpy(response->macAddress, base->deviceMacAddress, MAC_ADDR_LENGTH);

            switch(response->command)
            {
                case REPLY_CMD_REPLY_DEVICE_TOPOLOGY:
                    {
                        unsigned int i;
                        struct MsgReplyDeviceTopology* rdt = (struct MsgReplyDeviceTopology*)(&currCfg->recvMsg);
                        response->replyDeviceTopology.numDevices =
                            (rc - offsetof(struct MsgReplyDeviceTopology, devices)) / sizeof(rdt->devices[0]);
                        for(i = 0; i < response->replyDeviceTopology.numDevices; i++)
                        {
                            response->replyDeviceTopology.devices[i].usbAddress = rdt->devices[i].usbAddress;
                            response->replyDeviceTopology.devices[i].usbAddressOfParent = rdt->devices[i].usbAddressOfParent;
                            response->replyDeviceTopology.devices[i].portOnParent = rdt->devices[i].portOnParent;
                            response->replyDeviceTopology.devices[i].isHub = rdt->devices[i].isHub;
                            response->replyDeviceTopology.devices[i].usbVendorId = ntohs(rdt->devices[i].usbVendorId);
                            response->replyDeviceTopology.devices[i].usbProductId = ntohs(rdt->devices[i].usbProductId);
                        }
                    }
                    break;

                // REPLY_UNHANDLED_COMMAND is also a possibility, but it has no additional fields to
                // copy over.
                default:
                    break;
            }
            rc = 1;
        }
    }
    return rc;
}


int ICRON_removeAllPairings(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress)
{
    const uint32_t msgId = rand();
    int rc;
    memset(&currCfg->sendMsg, 0, sizeof(currCfg->sendMsg));
    SetTargetAddress(currCfg, networkBroadcastIP);
    setBasePacketAttributes((struct MsgBase*)(&currCfg->sendMsg), msgId, CMD_REMOVE_ALL_PAIRINGS, targetMACAddress);
    currCfg->sendMsgSize = sizeof(struct MsgRemoveAllPairings);

    rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    rc = RecvUdpPkt(currCfg, timeout);
    if (rc != -1)
    {
        DEBUG("remove all pairings returned %d bytes\n", rc);
        if(rc != 0)
        {
            struct MsgBase* base = (struct MsgBase*)(&currCfg->recvMsg);
            response->command = lookupReplyCommandType(base->command);
            memcpy(response->macAddress, base->deviceMacAddress, MAC_ADDR_LENGTH);

            // Both ACK and UNHANDLED_COMMAND are valid, but neither one has additional fields to copy
            // over.
            rc = 1;
        }
    }
    return rc;
}

int ICRON_useFilteringStrategy(
    struct IcronUsbNetCfg* currCfg,
    unsigned int timeout,
    struct ResponseData* response,
    const char* networkBroadcastIP,
    const uint8_t* targetMACAddress,
    uint8_t filteringStrategy)
{
    const uint32_t msgId = rand();
    int rc;
    struct MsgUseFilteringStrategy* useFilteringStrategyMsg = (struct MsgUseFilteringStrategy*)(&currCfg->sendMsg);

    memset(&currCfg->sendMsg, 0, sizeof(currCfg->sendMsg));
    SetTargetAddress(currCfg, networkBroadcastIP);
    setBasePacketAttributes((struct MsgBase*)(&currCfg->sendMsg), msgId, CMD_USE_FILTERING_STRATEGY, targetMACAddress);
    useFilteringStrategyMsg->filteringStrategy = filteringStrategy;
    currCfg->sendMsgSize = sizeof(struct MsgUseFilteringStrategy);

    rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    rc = RecvUdpPkt(currCfg, timeout);
    if (rc != -1)
    {
        DEBUG("Use filtering strategy returned %d bytes\n", rc);
        if(rc != 0)
        {
            struct MsgBase* base = (struct MsgBase*)(&currCfg->recvMsg);
            response->command = lookupReplyCommandType(base->command);
            memcpy(response->macAddress, base->deviceMacAddress, MAC_ADDR_LENGTH);

            // Both ACK and UNHANDLED_COMMAND are valid, but neither one has additional fields to copy
            // over.
            rc = 1;
        }
    }
    return rc;
}

static void setBasePacketAttributes(
    struct MsgBase* base, uint32_t msgId, uint8_t command, const uint8_t* targetMACAddress)
{
    base->magicNumber = htonl(MAGIC_NUMBER);
    base->messageId = msgId;
    base->command = command;
    memcpy(base->deviceMacAddress, targetMACAddress, MAC_ADDR_LENGTH);
}


static enum ReplyCommandType lookupReplyCommandType(uint8_t wireCommandValue)
{
    enum ReplyCommandType result;
    switch(wireCommandValue)
    {
        case CMD_REPLY_DEVICE_INFORMATION:
            result = REPLY_CMD_REPLY_DEVICE_INFORMATION;
            break;
        case CMD_REPLY_CONFIGURATION_RESPONSE_DATA:
            result = REPLY_CMD_REPLY_CONFIGURATION_RESPONSE_DATA;
            break;
        case CMD_ACKNOWLEDGE:
            result = REPLY_CMD_ACKNOWLEDGE;
            break;
        case CMD_REPLY_DEVICE_TOPOLOGY:
            result = REPLY_CMD_REPLY_DEVICE_TOPOLOGY;
            break;
        case CMD_REPLY_UNHANDLED_COMMAND:
            result = REPLY_CMD_REPLY_UNHANDLED_COMMAND;
            break;
        case CMD_NEGATIVE_ACKNOWLEDGE:
            result = REPLY_CMD_NEGATIVE_ACKNOWLEDGE;
            break;
        default:
            DEBUG("invalid wire command value");
            // TODO: being a library, we should never assert
            assert(false);
            break;
    }
    return result;
}

static bool receivedPacketValid(const struct IcronUsbNetCfg* currCfg, int numBytesReceived)
{
    // If the packet received isn't at least as big as the base structure, then it cannot possibly
    // be valid.
    bool packetValid = false;

    if(numBytesReceived >= sizeof(struct MsgBase))
    {
        // Verify that the incoming message has the same id as the one we sent out
        struct MsgBase* recvBase = (struct MsgBase*)(&currCfg->recvMsg);
        struct MsgBase* sendBase = (struct MsgBase*)(&currCfg->sendMsg);
        if(recvBase->messageId == sendBase->messageId && ntohl(recvBase->magicNumber) == MAGIC_NUMBER)
        {
            unsigned int i;
            for(i = 0; i < (sizeof(validResponses) / sizeof(struct ValidResponse)); i++)
            {
                // Check if the command received is an allowable response to the command sent
                if(validResponses[i].sent == sendBase->command && validResponses[i].response == recvBase->command)
                {
                    // Check if the message is a valid size for the specified command
                    switch(recvBase->command)
                    {
                        case CMD_REPLY_DEVICE_INFORMATION:
                            if(numBytesReceived <= sizeof(struct MsgReplyDeviceInformation))
                            {
                                const int remainingSize =
                                    numBytesReceived -
                                    offsetof(struct MsgReplyDeviceInformation, pairedWithMacAddress);
                                if(remainingSize >= 0)
                                {
                                    // We cast 0 to a MsgReplyDeviceInformation struct in order to
                                    // get the size of the pairedWithMacAddress member
                                    packetValid =
                                        (remainingSize % sizeof(
                                            ((struct MsgReplyDeviceInformation*)0)->pairedWithMacAddress[0]) == 0);
                                }
                            }
                            break;

                        case CMD_REPLY_CONFIGURATION_RESPONSE_DATA:
                            packetValid = (numBytesReceived == sizeof(struct MsgReplyConfigurationResponseData));
                            DEBUG("NumBytesReceived = %d, Expected size = %d\n", numBytesReceived, sizeof(struct MsgReplyConfigurationResponseData));
                            break;

                        case CMD_ACKNOWLEDGE:
                            packetValid = (numBytesReceived == sizeof(struct MsgAck));
                            break;

                        case CMD_REPLY_DEVICE_TOPOLOGY:
                            if(numBytesReceived <= sizeof(struct MsgReplyDeviceTopology))
                            {
                                const int remainingSize =
                                    numBytesReceived - offsetof(struct MsgReplyDeviceTopology, devices);
                                if(remainingSize >= 0)
                                {
                                    // We cast 0 to a MsgReplyDeviceTopology struct in order to
                                    // get the size of the pairedWithMacAddress member
                                    packetValid =
                                        (remainingSize % sizeof(
                                            ((struct MsgReplyDeviceTopology*)0)->devices[0]) == 0);
                                }
                            }
                            break;

                        case CMD_REPLY_UNHANDLED_COMMAND:
                            packetValid = (numBytesReceived == sizeof(struct MsgReplyUnhandledCommand));
                            break;

                        case CMD_NEGATIVE_ACKNOWLEDGE:
                            packetValid = (numBytesReceived == sizeof(struct MsgNegativeAcknowledge));
                            break;
                    }
                }
            }
        }
    }

    return packetValid;
}

