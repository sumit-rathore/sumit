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

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "xusbnetcfg.h"

/************************ Local Function Prototypes **************************/

/**
* @brief    Sets the destination address of the packet to be sent.
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT structure
* @param[in] targetIpAddr   Pointer to a string containing remote ip address, can be broadcast
* @param[in] udpPort        The UDP port number to connect to
*
* @return   0 on success, -1 on failure
*/
static int SetTargetAddress(IcronUsbNetCfgT *currCfg, const char *targetIpAddr, uint16_t udpPort);

/**
* @brief    Sets up the socket for configuring REXs and LEXs.
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
*
* @return   0 on success, -1 on failure
*/
static int InitSocket(IcronUsbNetCfgT *currCfg);

/**
* @brief    Sets up the socket to broadcast mode
*
* @param[in,out] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                           structure
*
* @return   0 on success, -1 on failure
*/
static int SetSockOptAllowBroadcast(IcronUsbNetCfgT *currCfg);

/**
* @brief    Closes the socket for this user.
*
* @param[in] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                       structure
*
* @return   0 on success, -1 on failure
*/
static int CloseSocket(IcronUsbNetCfgT *currCfg);

/**
* @brief    Sends the provided buffer out the users socket.
*
* @param[in] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                       structure
*
* @return   The number of bytes transmitted on success, -1 on failure.
*/
static int SendUdpPkt(IcronUsbNetCfgT *currCfg);

/**
* @brief    Receives a udp packet off the users socket.
*
* @param[in] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                       structure
*
* @return   number of bytes received on success, -1 on failure
*/
static int RecvUdpPkt(IcronUsbNetCfgT* currCfg);

/**
* @brief    Subtracts timeval y from timeval x and puts the result in timeval
*           result.
*
* @param[in] currCfg    Pointer to a previously intialized IcronUsbNetCfgT
*                       structure
* @param[out] result    The result of x - y
* @param[in] x          The timeval to subtract from
* @param[in] y          The amount being subtracted
*
* @return   true if the timeval in result is >= 0
*/
static bool SubtractTimeval(struct timeval* result, struct timeval* x, struct timeval* y);

/**
* @brief    Checks whether the general packet is correct.
*
* @param[in] numBytesReceived   size of the received packet
* @param[in] replyInfo          structure containing the general packet info
* @param[in] msgId              message ID that was sent by the client
* @param[in] sentMessageType    protocol version and command ID that was sent by the client
*
* @return    true               if packet is correct or false otherwise.
*/
static int IsRecvGeneralPacketCorrect(
    int numBytesReceived, GeneralInfoT* replyInfo, uint32_t msgId, MessageTypeT sentMessageType);
/**
* @brief    Find the responses that are applicable for a given protocol
*           version and command ID combination.
*
* @param[in] sentMessageType    protocol version and command ID that was sent by the client
*
* @param[out] expectedResponse  first expected message type
*
* @param[out] alternateResponse second expected message type
*
* @return
*
* @note       A maximum of 2 responses are valid for a given command
*/
static void LookUpProtocolCommandResponses(
    MessageTypeT sentMessageType,
    MessageTypeT* expectedResponse,
    MessageTypeT* alternateResponse);

/**
* @brief Builds a general packet in network byte order
*
* @param[in] messageID      When the client sends a request, it chooses any value
*                           to insert in this field.  The device responding to the
*                           request will set this field in the reply to the same
*                           value it received in the request
*
* @param[in] messageType    protocol version and command ID that was sent by the client
*
* @param[out] packet        Pointer to the general packet in network byte order
*/
static void CreateGeneralInfoPacket(
    uint32_t messageID, MessageTypeT messageType, GeneralInfoT* packet);

/************************** Function Definitions *****************************/


static int SetTargetAddress(IcronUsbNetCfgT *currCfg, const char *targetIpAddr, uint16_t udpPort)
{
    memset(&currCfg->sendTrgtAddr, 0, sizeof(struct sockaddr_in));
    currCfg->sendTrgtAddr.sin_family = AF_INET;
    currCfg->sendTrgtAddr.sin_port = htons(udpPort);
    // currCfg->sendTrgtAddr.sin_addr.s_addr = htonl(INADDR_ANY);
#ifdef __MINGW32__
    currCfg->sendTrgtAddr.sin_addr.s_addr = inet_addr(targetIpAddr);
    if (currCfg->sendTrgtAddr.sin_addr.s_addr == INADDR_NONE &&
        strncmp(targetIpAddr, "255.255.255.255", 16) != 0)
#else
    // This function is only on Vista & later for Windows hosts, so for windows we use the above
    // code.
    if (0 == inet_pton(AF_INET, targetIpAddr, &currCfg->sendTrgtAddr.sin_addr))
#endif
    {
        PRINTF(strerror(errno));
        return -1;
    }
    currCfg->sendTrgtAddrLen = sizeof(struct sockaddr_in);
    return 0;
}


static int InitSocket(IcronUsbNetCfgT *currCfg)
{
    if (currCfg == NULL) return -1;

    /* Create and configure socket */
    currCfg->cfgSock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (currCfg->cfgSock_fd == -1) {
        PRINTF(strerror(errno));
        return -1;
    }
    currCfg->recvTrgtAddrLen = sizeof(currCfg->recvTrgtAddr);

    /* Set up the file descriptor set for use by select */
    FD_ZERO(&currCfg->socketFdSet);
    FD_SET(currCfg->cfgSock_fd, &currCfg->socketFdSet);

    return 0;
}


static int SetSockOptAllowBroadcast(IcronUsbNetCfgT *currCfg)
{
    int rc;
#ifdef __MINGW32__
    char optVal = 1;
#else
    int optVal = 1;
#endif
    int optLen = sizeof(optVal);

    if (currCfg == NULL) return -1;

    rc = setsockopt(
        currCfg->cfgSock_fd, SOL_SOCKET, SO_BROADCAST, &optVal, optLen);
    if (rc == -1) {
        PRINTF(strerror(errno));
    }
    return rc;
}


static int CloseSocket(IcronUsbNetCfgT *currCfg)
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


static int SendUdpPkt(IcronUsbNetCfgT *currCfg)
{
    int rc;

    if (currCfg == NULL) return -1;
    DEBUG("currCfg->cfgSock_fd = %d\n", currCfg->cfgSock_fd);
    DEBUG("currCfg->sendMsgSize = %d\n", currCfg->sendMsgSize);
    DEBUG("currCfg->sendTrgtAddrlen = %d\n", currCfg->sendTrgtAddrLen);
    rc = sendto(currCfg->cfgSock_fd, (char *)&currCfg->sendMsg[0],
                currCfg->sendMsgSize, 0,
                (const struct sockaddr *) &currCfg->sendTrgtAddr,
                currCfg->sendTrgtAddrLen);
    if (rc == -1) {
        PRINTF(strerror(errno));
    }
    return rc;
}


static bool SubtractTimeval(struct timeval* result, struct timeval* x, struct timeval* y)
{
    int64_t secDiff = x->tv_sec - y->tv_sec;
    int64_t usecDiff = x->tv_usec - y->tv_usec;
    while (usecDiff < 0)
    {
        secDiff -= 1;
        usecDiff += 1000000;
    }
    result->tv_sec = secDiff;
    result->tv_usec = usecDiff;
    return (result->tv_sec > 0);
}


static int RecvUdpPkt(IcronUsbNetCfgT* currCfg)
{
    int rc = 0;

    if (currCfg == NULL) return -1;

    // Store the time that we start this function at.
    struct timeval startTime;
    gettimeofday(&startTime, NULL);

    /*
    TBD, use of time out in this case, as long as we receive responses
    within the time out period, would be the time from the
    last packet received.
    */
    fd_set writefds;
    fd_set exceptfds;

    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    rc = select(FD_SETSIZE, &currCfg->socketFdSet, &writefds, &exceptfds,
                &currCfg->selectTimeout);

    if (rc == -1)
    {
        PRINTF(strerror(errno));
        return -1;
    }
    else if(rc == 0)
    {
        /* We've timed out */
        DEBUG("Timed out from select() call")
    }
    else
    {
        DEBUG("Calling recvfrom(), rc = %d\n", rc);
        rc = recvfrom(
            currCfg->cfgSock_fd,
            (char*)&currCfg->recvMsg[0],
            currCfg->maxRecvMsgSize,
            0,
            (struct sockaddr*)&currCfg->recvTrgtAddr,
            &currCfg->recvTrgtAddrLen);

        DEBUG("Returned from recvfrom(), rc = %d\n", rc);
#ifdef __MINGW32__
        if (rc == -1)
        {
            PRINTF("Windows failure %d\n", WSAGetLastError());
            return -1;
        }
#else
        if (rc == -1)
        {
            PRINTF(strerror(errno));
            return -1;
        }
#endif
        else if ((rc < currCfg->minRecvMsgSize) || (rc > currCfg->maxRecvMsgSize))
        {
            printf("The size of the received packet is outside of the allowable range.");
            return -1;
        }
    }

    struct timeval endTime;
    gettimeofday(&endTime, NULL);
    struct timeval elapsedTime;
    SubtractTimeval(&elapsedTime, &endTime, &startTime);
    if (!SubtractTimeval(&(currCfg->selectTimeout), &(currCfg->selectTimeout), &elapsedTime))
    {
        currCfg->selectTimeout.tv_sec = 0;
        currCfg->selectTimeout.tv_usec = 0;
    }

    return rc;
}


IcronUsbNetCfgT *ICRON_usbNetCfgInit(void)
{
    IcronUsbNetCfgT *currCfg = NULL;
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

    currCfg = calloc(1, sizeof(IcronUsbNetCfgT));
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


int ICRON_usbNetCfgRelease(IcronUsbNetCfgT *currCfg)
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

static int IsRecvGeneralPacketCorrect(
        int numBytesReceived,
        GeneralInfoT* replyInfo,
        uint32_t msgId,
        MessageTypeT sentMessageType)
{
    uint32_t recvMagicNumber = ntohl(replyInfo->magicNumber);
    uint32_t recvMsgId = ntohl(replyInfo->messageID);
    MessageTypeT recvMessageType = replyInfo->messageType;
    MessageTypeT recvExpectedMessageType1;
    MessageTypeT recvExpectedMessageType2;
    int rc = 0;

    // check Magic Number, MsgId, and expected responses
    LookUpProtocolCommandResponses(sentMessageType,
                                   &recvExpectedMessageType1,
                                   &recvExpectedMessageType2);

    if ((recvMagicNumber != MAGIC_NUMBER) || (recvMsgId != msgId) ||
        ((!memcmp(&recvMessageType, &recvExpectedMessageType1, sizeof(MessageTypeT))) &&
         ((!memcmp(&recvMessageType, &recvExpectedMessageType2, sizeof(MessageTypeT))))))
    {
        DEBUG("Magic number: %X\t" \
                "Message ID  : %X\t" \
                "Recv Protocol : %d\t" \
                "Recv Cmd ID : %d\n", recvMagicNumber, recvMsgId,
                recvMessageType.protocolVersion, recvMessageType.command);

        DEBUG("Expected mag: %X\t" \
               "Expected msg: %X\t" \
               "Expected ID : %d\t" \
               "Alternate ID: %d\n", MAGIC_NUMBER, msgId,
               recvExpectedMessageType1.command, recvExpectedMessageType2.command);
        DEBUG("Magic Number, message ID or reply command identifier are incorrect\n");
        rc = -1;
    }
    return rc;
}


static void LookUpProtocolCommandResponses(
        MessageTypeT sentMessageType,
        MessageTypeT* expectedResponse,
        MessageTypeT* alternateResponse)
{
    int numResponses = sizeof(ProtoCmdRespsMapping) / sizeof(struct ExpectedResponse);

    int resp;
    for (resp = 0; resp < numResponses; resp++)
    {
        if (!memcmp(&sentMessageType,
                    &ProtoCmdRespsMapping[resp].sentMessageType,
                    sizeof(MessageTypeT)))
        {
            *expectedResponse = ProtoCmdRespsMapping[resp].recvMessageType1;
            *alternateResponse = ProtoCmdRespsMapping[resp].recvMessageType2;
        }
    }
}


static void CreateGeneralInfoPacket(
        uint32_t messageID,
        MessageTypeT messageType,
        GeneralInfoT* packet)
{
    packet->magicNumber = htonl(MAGIC_NUMBER);
    packet->messageID = htonl(messageID);
    packet->messageType = messageType;
}


int ICRON_requestDevInfo(
        IcronUsbNetCfgT *currCfg,
        RequestT request,
        uint32_t msgId,
        int timeOut,
        ReplyT *reply)
{
    MessageTypeT sentMessageType = RequestDeviceInfo_0;
    GeneralInfoT packet;
    CreateGeneralInfoPacket(msgId, sentMessageType, &packet);
    // Allow broadcast data
    int rc = SetSockOptAllowBroadcast(currCfg);
    if (rc == -1)
    {
        return rc;
    }

    SetTargetAddress(currCfg, request.general.ipAddress, request.general.udpPort);
    currCfg->sendMsgSize = sizeof(GeneralInfoT);
    currCfg->minRecvMsgSize = sizeof(IcronReplyDevInfoT);
    currCfg->maxRecvMsgSize = sizeof(IcronReplyDevInfoT);

    memset(&currCfg->sendMsg[0], 0, XUSBNETCFG_MAX_CMD_LEN);
    memcpy(&currCfg->sendMsg[0], &packet, currCfg->sendMsgSize);

    rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }

    int numDevices = 0;
    GeneralInfoT* replyPacket = NULL;

    // Wait for more than one replyDeviceInfo packets
    currCfg->selectTimeout.tv_sec = timeOut;
    currCfg->selectTimeout.tv_usec = 0;
    while (1)
    {
        rc = RecvUdpPkt(currCfg);
        if (rc == -1)
        {
            return rc;
        }
        else if (rc == sizeof(IcronReplyDevInfoT))
        {
            replyPacket = (GeneralInfoT *)&currCfg->recvMsg[0];

            int numBytesReceived = rc;
            rc = IsRecvGeneralPacketCorrect(numBytesReceived,
                                            replyPacket,
                                            msgId,
                                            sentMessageType);
            if (rc != -1)
            {
                IcronReplyDevInfoT* devInfo = (IcronReplyDevInfoT* )&currCfg->recvMsg[0];

                memcpy(reply[numDevices].u.replyDevInfo.mac, devInfo->mac, 6);

                // IP Address is not converted to host byte order because IP address
                // is printed using inet_ntoa(*arg) where *arg must be in network
                // byte order
                reply[numDevices].u.replyDevInfo.ipAddr = devInfo->ipAddr;
                reply[numDevices].u.replyDevInfo.networkAcquisitionMode = devInfo->networkAcquisitionMode;
                reply[numDevices].u.replyDevInfo.supportedProtocol = devInfo->supportedProtocol;
                memcpy(reply[numDevices].u.replyDevInfo.vendor, devInfo->vendor,
                        REPLY_DEV_INFO_VENDOR_LENGTH);
                memcpy(reply[numDevices].u.replyDevInfo.product, devInfo->product,
                        REPLY_DEV_INFO_PRODUCT_LENGTH);
                memcpy(reply[numDevices].u.replyDevInfo.revision, devInfo->revision,
                        REPLY_DEV_INFO_REVISION_LENGTH);
                currCfg->protocolVersion = devInfo->supportedProtocol;

                numDevices++;
            }
            else
            {
                return rc;
            }
        }
        else if (rc > 0)
        {
            PRINTF("WARNING: Received response of unexpected length, "
                    "expected %d, received %d\n",
                    currCfg->maxRecvMsgSize, rc);

            return -1;
        }
        // Timeout has occurred
        else if (rc == 0)
        {
            break;
        }
    }

    // TODO: InitSocket shouldn't be called here. We should fix it
    // Reinitialize the socket for future calls
    InitSocket(currCfg);

    DEBUG("%d devices found\n", numDevices);

    return numDevices;
}

int ICRON_requestCnfgRespData(
        IcronUsbNetCfgT *currCfg,
        RequestT request,
        uint32_t msgId,
        int timeOut,
        ReplyT *reply)
{
    MessageTypeT sentMessageType = RequestCnfgRespData_3;
    GeneralInfoT packet;
    CreateGeneralInfoPacket(msgId, sentMessageType, &packet);
    // Allow broadcast data
    int rc = SetSockOptAllowBroadcast(currCfg);
    if (rc == -1)
    {
        printf("RC1 = %d\n", rc);
        return rc;
    }

    SetTargetAddress(currCfg, request.general.ipAddress, request.general.udpPort);
    currCfg->sendMsgSize = sizeof(GeneralInfoT);
    currCfg->minRecvMsgSize = sizeof(IcronReplyCnfgRespDataT);
    currCfg->maxRecvMsgSize = sizeof(IcronReplyCnfgRespDataT);

    memset(&currCfg->sendMsg[0], 0, XUSBNETCFG_MAX_CMD_LEN);
    memcpy(&currCfg->sendMsg[0], &packet, currCfg->sendMsgSize);

    rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {        printf("RC2 = %d\n", rc);
        return rc;
    }

    int numDevices = 0;
    GeneralInfoT* replyPacket = NULL;

    // Wait for more than one replyDeviceInfo packets
    currCfg->selectTimeout.tv_sec = timeOut;
    currCfg->selectTimeout.tv_usec = 0;

    while (1)
    {
        rc = RecvUdpPkt(currCfg);
        if (rc == -1)
        {        printf("RC3 = %d\n", rc);
            return rc;
        }
        else if (rc == sizeof(IcronReplyCnfgRespDataT))
        {
            replyPacket = (GeneralInfoT *)&currCfg->recvMsg[0];

            int numBytesReceived = rc;
            rc = IsRecvGeneralPacketCorrect(numBytesReceived,
                                            replyPacket,
                                            msgId,
                                            sentMessageType);
            if (rc != -1)
            {
                IcronReplyCnfgRespDataT* devInfo = (IcronReplyCnfgRespDataT* )&currCfg->recvMsg[0];

                reply[numDevices].u.replyCnfgRespData.highSpeed = devInfo->highSpeed;
                reply[numDevices].u.replyCnfgRespData.msa = devInfo->msa;
                reply[numDevices].u.replyCnfgRespData.vhub = devInfo->vhub;
                reply[numDevices].u.replyCnfgRespData.currentFilterStatus = devInfo->currentFilterStatus;
                reply[numDevices].u.replyCnfgRespData.ipAcquisitionMode = devInfo->ipAcquisitionMode;

                memcpy(reply[numDevices].u.replyCnfgRespData.mac, devInfo->mac, 6);
                int numPairedMacs = ((sizeof(devInfo->pairedMacs))/(6*sizeof(uint8_t)));
                int mac;
                for (mac = 0; mac < numPairedMacs; mac++)
                    {
                        memcpy(reply[numDevices].u.replyCnfgRespData.pairedMacs[mac], devInfo->pairedMacs[mac], 6);
                    }

                // Port Number is stored as Big Endian but is printed on a
                // Little Endian machine. Therefore, ntohs is used to do the
                // endian conversion.
                reply[numDevices].u.replyCnfgRespData.portNumber = ntohs(devInfo->portNumber);

                // IP Address, Subnet Mask, Default Gateway and DHCP Server are
                // not converted to host byte order because they are printed
                // using inet_ntoa(*arg) where *arg must be in network
                // byte order
                reply[numDevices].u.replyCnfgRespData.ipAddr = devInfo->ipAddr;
                reply[numDevices].u.replyCnfgRespData.subNetM = devInfo->subNetM;
                reply[numDevices].u.replyCnfgRespData.gateway = devInfo->gateway;
                reply[numDevices].u.replyCnfgRespData.dhcpServer = devInfo->dhcpServer;

                reply[numDevices].u.replyCnfgRespData.numVhubPorts = devInfo->numVhubPorts;
                // VID, PID and Brand Id are stored as Big Endian but are
                // printed on a Little Endian machine. Therefore, ntohs
                // is used to do the endian conversion.
                reply[numDevices].u.replyCnfgRespData.vhubVid = ntohs(devInfo->vhubVid);
                reply[numDevices].u.replyCnfgRespData.vhubPid = ntohs(devInfo->vhubPid);

                reply[numDevices].u.replyCnfgRespData.brandId = ntohs(devInfo->brandId);
                memcpy(reply[numDevices].u.replyCnfgRespData.vendor, devInfo->vendor,
                       REPLY_CNFG_RESP_DATA_VENDOR_LENGTH);

                memcpy(reply[numDevices].u.replyCnfgRespData.product, devInfo->product,
                        REPLY_CNFG_RESP_DATA_PRODUCT_LENGTH);
                memcpy(reply[numDevices].u.replyCnfgRespData.revision, devInfo->revision,
                        REPLY_CNFG_RESP_DATA_REVISION_LENGTH);

                numDevices++;
            }
            else
            {
                return rc;
            }
        }
        else if (rc > 0)
        {
            PRINTF("WARNING: Received response of unexpected length, "
                    "expected %d, received %d\n",
                    currCfg->maxRecvMsgSize, rc);

            return -1;
        }
        // Timeout has occurred
        else if (rc == 0)
        {
            break;
        }
    }

    DEBUG("%d devices found\n", numDevices);

    return numDevices;
}


int ICRON_pingDevice(IcronUsbNetCfgT *currCfg, struct Request request, uint32_t msgId, int timeOut)
{
    MessageTypeT sentMessageType = Ping_0;
    GeneralInfoT packet;
    CreateGeneralInfoPacket(msgId, sentMessageType, &packet);

    SetTargetAddress(currCfg, request.general.ipAddress, request.general.udpPort);

    currCfg->sendMsgSize = sizeof(GeneralInfoT);
    currCfg->minRecvMsgSize = sizeof(GeneralInfoT);
    currCfg->maxRecvMsgSize = sizeof(GeneralInfoT);

    memset(&currCfg->sendMsg[0], 0,       XUSBNETCFG_MAX_CMD_LEN);
    memcpy(&currCfg->sendMsg[0], &packet, currCfg->sendMsgSize);

    int rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    currCfg->selectTimeout.tv_sec = timeOut;
    currCfg->selectTimeout.tv_usec = 0;
    rc = RecvUdpPkt(currCfg);

    // Error or "receive UDP" timed out
    if ((rc == -1) || (rc == 0))
    {
        rc = -1;
        return rc;
    }
    DEBUG("ping returned %d bytes\n", rc);

    GeneralInfoT* replyPacket = (GeneralInfoT *)&currCfg->recvMsg[0];

    int numBytesReceived = rc;
    rc = IsRecvGeneralPacketCorrect(numBytesReceived,
                                    replyPacket,
                                    msgId,
                                    sentMessageType);
    return rc;
}


int ICRON_getExtendedDevInfo(
        IcronUsbNetCfgT *currCfg,
        struct Request request,
        uint32_t msgId,
        int timeOut,
        struct Reply *reply)
{
    MessageTypeT sentMessageType;

    switch (currCfg->protocolVersion)
    {
        case 1:
            sentMessageType = RequestExtendedDeviceInfo_1;
            currCfg->minRecvMsgSize = sizeof(IcronReplyExtDevInfoTVersion1);
            currCfg->maxRecvMsgSize = sizeof(IcronReplyExtDevInfoTVersion1);
            break;

        case 2:
            sentMessageType = RequestExtendedDeviceInfo_2;
            currCfg->maxRecvMsgSize = sizeof(IcronReplyExtDevInfoTVersion2);
            // Minimum size is when no Paired MAC Addresses are returned
            currCfg->minRecvMsgSize = currCfg->maxRecvMsgSize -
                7*6*sizeof(uint8_t);
            break;

        case 3:
            sentMessageType = RequestExtendedDeviceInfo_3;
            currCfg->maxRecvMsgSize = sizeof(IcronReplyExtDevInfoTVersion2);
            // Minimum size is when no Paired MAC Addresses are returned
            currCfg->minRecvMsgSize = currCfg->maxRecvMsgSize -
                7*6*sizeof(uint8_t);
            break;

        default:
            return -1;
    }

    GeneralInfoT packet;
    CreateGeneralInfoPacket(msgId, sentMessageType, &packet);

    SetTargetAddress(currCfg, request.general.ipAddress, request.general.udpPort);
    currCfg->sendMsgSize = sizeof(GeneralInfoT);

    memset(&currCfg->sendMsg[0], 0,       XUSBNETCFG_MAX_CMD_LEN);
    memcpy(&currCfg->sendMsg[0], &packet, currCfg->sendMsgSize);

    int rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    currCfg->selectTimeout.tv_sec = timeOut;
    currCfg->selectTimeout.tv_usec = 0;
    rc = RecvUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    DEBUG("request returned %d bytes\n", rc);

    GeneralInfoT* replyPacket = (GeneralInfoT *)&currCfg->recvMsg[0];

    int numBytesReceived = rc;
    rc = IsRecvGeneralPacketCorrect(numBytesReceived,
                                    replyPacket,
                                    msgId,
                                    sentMessageType);
    if (rc != -1)
    {
        // ReplyExtendedDeviceInfo_1
        if (!memcmp(&ReplyExtendedDeviceInfo_1, &replyPacket->messageType, sizeof(MessageTypeT)))
        {
            IcronReplyExtDevInfoTVersion1* extDevInfo =
                (IcronReplyExtDevInfoTVersion1 *)&currCfg->recvMsg[0];

            reply[0].u.replyExtDevInfo.lexOrRex = extDevInfo->lexOrRex;

            if (extDevInfo->isPaired)
            {
                rc = 1; // Only 1 paired MAC in Protocol 1
                memcpy(reply[0].u.replyExtDevInfo.pairedMacs[0], extDevInfo->pairedMac, 6);
            }
            else
            {
                rc = 0;
            }
        }
        // ReplyExtendedDeviceInfo_2 or ReplyExtendedDeviceInfo_3
        else if (   (!memcmp(&ReplyExtendedDeviceInfo_2, &replyPacket->messageType, sizeof(MessageTypeT)))
                 || (!memcmp(&ReplyExtendedDeviceInfo_3, &replyPacket->messageType, sizeof(MessageTypeT))))
        {
            IcronReplyExtDevInfoTVersion2* extDevInfo =
                (IcronReplyExtDevInfoTVersion2 *)&currCfg->recvMsg[0];

            reply[0].u.replyExtDevInfo.lexOrRex = extDevInfo->lexOrRex;

            int numPairedMacs = (numBytesReceived -
                    sizeof(GeneralInfoT) - sizeof(uint8_t))/(6 * sizeof(uint8_t));
            printf("Number of Paired Macs = %d\n", numPairedMacs);
            int mac;
            for (mac = 0; mac < numPairedMacs; mac++)
            {
                memcpy(reply[0].u.replyExtDevInfo.pairedMacs[mac],
                    extDevInfo->pairedMacs[mac], 6);
            }

            rc = numPairedMacs;
        }
    }
    return rc;
}


int ICRON_pairToDevice(
        IcronUsbNetCfgT *currCfg, struct Request request, uint32_t msgId, int timeOut)
{
    MessageTypeT sentMessageType;
    GeneralInfoT generalPacket;

    switch(currCfg->protocolVersion)
    {
        case 1:
            sentMessageType = PairToDevice_1;
            break;
        case 2:
            sentMessageType = PairToDevice_2;
            break;
        case 3:
            sentMessageType = PairToDevice_3;
            break;
        default:
            return -1;
    }

    CreateGeneralInfoPacket(msgId, sentMessageType, &generalPacket);

    IcronPairToDeviceT packet;
    memcpy(&packet.generalInfo, &generalPacket, sizeof(GeneralInfoT));
    memcpy(&packet.pairToMac, &request.cmdSpecific.pairToDevice.macAddressToPairTo, 6);

    SetTargetAddress(currCfg, request.general.ipAddress, request.general.udpPort);

    currCfg->sendMsgSize = sizeof(IcronPairToDeviceT);
    currCfg->minRecvMsgSize = sizeof(GeneralInfoT);
    currCfg->maxRecvMsgSize = sizeof(GeneralInfoT);

    memset(&currCfg->sendMsg[0], 0,       XUSBNETCFG_MAX_CMD_LEN);
    memcpy(&currCfg->sendMsg[0], &packet, currCfg->sendMsgSize);

    int rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    currCfg->selectTimeout.tv_sec = timeOut;
    currCfg->selectTimeout.tv_usec = 0;
    rc = RecvUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    DEBUG("request returned %d bytes\n", rc);

    GeneralInfoT* replyPacket = (GeneralInfoT *)&currCfg->recvMsg[0];

    int numBytesReceived = rc;
    rc = IsRecvGeneralPacketCorrect(numBytesReceived,
                                    replyPacket,
                                    msgId,
                                    sentMessageType);
    if (rc != -1)
    {
        // Acknowledge_0
        if (!memcmp(&Acknowledge_0, &replyPacket->messageType, sizeof(MessageTypeT)))
        {
            rc = 1;
        }
        // NegativeAcknowledge_2 or NegativeAcknowledge_3
        else if (   (!memcmp(&NegativeAcknowledge_2, &replyPacket->messageType, sizeof(MessageTypeT)))
                 || (!memcmp(&NegativeAcknowledge_3, &replyPacket->messageType, sizeof(MessageTypeT))))
        {
            rc = 2;
        }
        else
        {
            rc = -1;
        }
    }
    return rc;
}

/*

   Return Values:

   -1   : Error
    1   : ACK
    2   : NAK

*/

int ICRON_removeDevicePairing(
        IcronUsbNetCfgT *currCfg, RequestT request, uint32_t msgId, int timeOut)
{
    MessageTypeT sentMessageType;
    GeneralInfoT generalPacket;

    SetTargetAddress(currCfg, request.general.ipAddress, request.general.udpPort);

    currCfg->minRecvMsgSize = sizeof(GeneralInfoT);
    currCfg->maxRecvMsgSize = sizeof(GeneralInfoT);

    DEBUG("Protocol version 1\n");
    switch(currCfg->protocolVersion)
    {
        case 1:
        {
            sentMessageType = RemoveDevicePairing_1;
            IcronRemoveDevicePairingTVersion1 packet;
            CreateGeneralInfoPacket(msgId, sentMessageType, &generalPacket);
            memcpy(&packet.generalInfo, &generalPacket, sizeof(GeneralInfoT));

            currCfg->sendMsgSize =  sizeof(IcronRemoveDevicePairingTVersion1);

            memset(&currCfg->sendMsg[0], 0,       XUSBNETCFG_MAX_CMD_LEN);
            memcpy(&currCfg->sendMsg[0], &packet, currCfg->sendMsgSize);
            break;
        }
        case 2:
        {
            sentMessageType = RemoveDevicePairing_2;
            IcronRemoveDevicePairingTVersion2 packet;
            CreateGeneralInfoPacket(msgId, sentMessageType, &generalPacket);
            memcpy(&packet.generalInfo, &generalPacket, sizeof(GeneralInfoT));
            memcpy(
                &packet.unpairFromMac,
                &request.cmdSpecific.removeDevicePairing.macAddressToUnpairFrom,
                6);

            currCfg->sendMsgSize =  sizeof(IcronRemoveDevicePairingTVersion2);

            memset(&currCfg->sendMsg[0], 0,       XUSBNETCFG_MAX_CMD_LEN);
            memcpy(&currCfg->sendMsg[0], &packet, currCfg->sendMsgSize);
            break;
        }
        case 3:
        {
            sentMessageType = RemoveDevicePairing_3;
            IcronRemoveDevicePairingTVersion2 packet;
            CreateGeneralInfoPacket(msgId, sentMessageType, &generalPacket);
            memcpy(&packet.generalInfo, &generalPacket, sizeof(GeneralInfoT));
            memcpy(
                &packet.unpairFromMac,
                &request.cmdSpecific.removeDevicePairing.macAddressToUnpairFrom,
                6);

            currCfg->sendMsgSize =  sizeof(IcronRemoveDevicePairingTVersion2);

            memset(&currCfg->sendMsg[0], 0,       XUSBNETCFG_MAX_CMD_LEN);
            memcpy(&currCfg->sendMsg[0], &packet, currCfg->sendMsgSize);
            break;
        }
        default:
            return -1;
    }

    int rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    currCfg->selectTimeout.tv_sec = timeOut;
    currCfg->selectTimeout.tv_usec = 0;
    rc = RecvUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    DEBUG("request returned %d bytes\n", rc);

    GeneralInfoT* replyPacket = (GeneralInfoT *)&currCfg->recvMsg[0];

    int numBytesReceived = rc;
    rc = IsRecvGeneralPacketCorrect(numBytesReceived,
                                    replyPacket,
                                    msgId,
                                    sentMessageType);
    if (rc != -1)
    {
        // Acknowledge_0
        if (!memcmp(&Acknowledge_0, &replyPacket->messageType, sizeof(MessageTypeT)))
        {
            DEBUG("Acknowledgement\n");
            rc = 1;
        }
        // NegativeAcknowledge_2 or NegativeAcknowledge_3
        else if (   (!memcmp(&NegativeAcknowledge_2, &replyPacket->messageType, sizeof(MessageTypeT)))
                 || (!memcmp(&NegativeAcknowledge_3, &replyPacket->messageType, sizeof(MessageTypeT))))
        {
            DEBUG("Negative Acknowledgement\n");
            rc = 2;
        }
        else
        {
            DEBUG("Error\n");
            rc = -1;
        }
    }
    else
    {
        DEBUG("Error\n");
    }

    return rc;
}


/*

   -1   : Error
   -2   : Device is REX
 Number : Number of devices between 0 and 31

*/
int ICRON_getDevTopology(
        IcronUsbNetCfgT *currCfg,
        struct Request request,
        uint32_t msgId,
        int timeOut,
        struct Reply *reply)
{
    MessageTypeT sentMessageType;
    GeneralInfoT packet;

    SetTargetAddress(currCfg, request.general.ipAddress, request.general.udpPort);
    currCfg->sendMsgSize = sizeof(GeneralInfoT);

    switch(currCfg->protocolVersion)
    {
        case 1:
            sentMessageType = RequestDeviceTopology_1;
            CreateGeneralInfoPacket(msgId, sentMessageType, &packet);
            currCfg->maxRecvMsgSize = sizeof(IcronReplyDevTopologyTVersion1);
            currCfg->minRecvMsgSize = currCfg->maxRecvMsgSize -
                MAX_USB_DEVICES * sizeof(struct DevInfoVersion1);
            break;
        case 2:
            sentMessageType = RequestDeviceTopology_2;
            CreateGeneralInfoPacket(msgId, sentMessageType, &packet);
            currCfg->maxRecvMsgSize = sizeof(IcronReplyDevTopologyTVersion2);
            currCfg->minRecvMsgSize = currCfg->maxRecvMsgSize -
                MAX_USB_DEVICES * sizeof(struct DevInfoVersion2);
            break;
        case 3:
            sentMessageType = RequestDeviceTopology_3;
            CreateGeneralInfoPacket(msgId, sentMessageType, &packet);
            currCfg->maxRecvMsgSize = sizeof(IcronReplyDevTopologyTVersion2);
            currCfg->minRecvMsgSize = currCfg->maxRecvMsgSize -
                MAX_USB_DEVICES * sizeof(struct DevInfoVersion2);
            break;
        default:
            return -1;
    }

    memset(&currCfg->sendMsg[0], 0,       XUSBNETCFG_MAX_CMD_LEN);
    memcpy(&currCfg->sendMsg[0], &packet, currCfg->sendMsgSize);
    memset(&currCfg->recvMsg[0], 0,       XUSBNETCFG_MAX_CMD_LEN);

    int rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    currCfg->selectTimeout.tv_sec = timeOut;
    currCfg->selectTimeout.tv_usec = 0;
    rc = RecvUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }

    // In protocol 1, REX will not respond with any devices
    if ((rc == 0) && (currCfg->protocolVersion == 1))
    {
        rc = -2;
        return rc;
    }

    DEBUG("request returned %d bytes\n", rc);

    GeneralInfoT* replyPacket = (GeneralInfoT *)&currCfg->recvMsg[0];

    int numBytesReceived = rc;
    rc = IsRecvGeneralPacketCorrect(numBytesReceived, replyPacket, msgId, sentMessageType);
    if (rc != -1)
    {
        int numDevices;

        // ReplyDeviceTopology_1
        if (!memcmp(&ReplyDeviceTopology_1, &replyPacket->messageType, sizeof(MessageTypeT)))
        {
            IcronReplyDevTopologyTVersion1* devTopology =
                (IcronReplyDevTopologyTVersion1 *)&currCfg->recvMsg[0];

            numDevices = (numBytesReceived - sizeof(GeneralInfoT)) / sizeof(struct DevInfoVersion1);

            int dev;
            for (dev = 0; dev < numDevices; dev++)
            {
                reply[dev].u.replyDeviceTopology.usbAddr = devTopology->devices[dev].usbAddr;
                reply[dev].u.replyDeviceTopology.usbParentAddr = devTopology->devices[dev].usbParentAddr;
                reply[dev].u.replyDeviceTopology.portOnParent = devTopology->devices[dev].portOnParent;
                reply[dev].u.replyDeviceTopology.isHub = devTopology->devices[dev].isHub;
                reply[dev].u.replyDeviceTopology.doesVendorIdExist = false;
                reply[dev].u.replyDeviceTopology.usbVendorId = 0;
                reply[dev].u.replyDeviceTopology.doesProductIdExist = false;
                reply[dev].u.replyDeviceTopology.usbProductId = 0;
            }
            rc = numDevices;
        }
        // ReplyDeviceTopology_2
        else if (  (!memcmp(&ReplyDeviceTopology_2, &replyPacket->messageType, sizeof(MessageTypeT)))
                || (!memcmp(&ReplyDeviceTopology_3, &replyPacket->messageType, sizeof(MessageTypeT))))
        {
            IcronReplyDevTopologyTVersion2* devTopology =
                (IcronReplyDevTopologyTVersion2 *)&currCfg->recvMsg[0];

            numDevices = (numBytesReceived - sizeof(GeneralInfoT)) / sizeof(struct DevInfoVersion2);

            int dev;
            for (dev = 0; dev < numDevices; dev++)
            {
                reply[dev].u.replyDeviceTopology.usbAddr = devTopology->devices[dev].usbAddr;
                reply[dev].u.replyDeviceTopology.usbParentAddr = devTopology->devices[dev].usbParentAddr;
                reply[dev].u.replyDeviceTopology.portOnParent = devTopology->devices[dev].portOnParent;
                reply[dev].u.replyDeviceTopology.isHub = devTopology->devices[dev].isHub;
                reply[dev].u.replyDeviceTopology.doesVendorIdExist = true;
                reply[dev].u.replyDeviceTopology.usbVendorId = ntohs(devTopology->devices[dev].usbVendorId);
                reply[dev].u.replyDeviceTopology.doesProductIdExist = true;
                reply[dev].u.replyDeviceTopology.usbProductId = ntohs(devTopology->devices[dev].usbProductId);
            }
            rc = numDevices;
        }
        // NegativeAcknowledge_2
        else if (   (!memcmp(&NegativeAcknowledge_2, &replyPacket->messageType, sizeof(MessageTypeT)))
                 || (!memcmp(&NegativeAcknowledge_3, &replyPacket->messageType, sizeof(MessageTypeT))))
        {
            rc = -2;
        }
    }
    return rc;
}


int ICRON_useDHCP(IcronUsbNetCfgT *currCfg, RequestT request, uint32_t msgId, int timeOut)
{
    MessageTypeT sentMessageType;
    GeneralInfoT generalPacket;

    switch(currCfg->protocolVersion)
    {
        case 1:
            sentMessageType = UseDHCP_1;
            break;
        case 2:
            sentMessageType = UseDHCP_2;
            break;
        case 3:
            sentMessageType = UseDHCP_3;
            break;
        default:
            return -1;
    }

    CreateGeneralInfoPacket(msgId, sentMessageType, &generalPacket);

    UseDHCPT packet;
    memcpy(&packet.generalInfo, &generalPacket, sizeof(GeneralInfoT));
    memcpy(&packet.targetMacAddress, &request.cmdSpecific.useDHCP.macAddress, 6);

    SetTargetAddress(currCfg, request.general.ipAddress, request.general.udpPort);
    currCfg->sendMsgSize = sizeof(UseDHCPT);
    currCfg->minRecvMsgSize = sizeof(GeneralInfoT);
    currCfg->maxRecvMsgSize = sizeof(GeneralInfoT);

    memset(&currCfg->sendMsg[0], 0,       XUSBNETCFG_MAX_CMD_LEN);
    memcpy(&currCfg->sendMsg[0], &packet, currCfg->sendMsgSize);

    int rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    currCfg->selectTimeout.tv_sec = timeOut;
    currCfg->selectTimeout.tv_usec = 0;
    rc = RecvUdpPkt(currCfg);

    // Device will not respond if DHCP setup failed
    if ((rc == -1) || (rc == 0))
    {
        return rc;
    }

    GeneralInfoT* replyPacket = (GeneralInfoT *)&currCfg->recvMsg[0];

    int numBytesReceived = rc;
    rc = IsRecvGeneralPacketCorrect(numBytesReceived, replyPacket, msgId, sentMessageType);
    // DHCP setup was successful
    if (rc != -1)
    {
        rc = 1;
    }
    return rc;
}


int ICRON_useStaticIP(IcronUsbNetCfgT *currCfg, RequestT request, uint32_t msgId, int timeOut)
{
    MessageTypeT sentMessageType;
    GeneralInfoT generalPacket;

    switch(currCfg->protocolVersion)
    {
        case 1:
            sentMessageType = UseStatic_1;
            break;
        case 2:
            sentMessageType = UseStatic_2;
            break;
        case 3:
            sentMessageType = UseStatic_3;
            break;
        default:
            return -1;
    }

    CreateGeneralInfoPacket(msgId, sentMessageType, &generalPacket);

    UseStaticT packet;
    memcpy(&packet.generalInfo, &generalPacket, sizeof(GeneralInfoT));
    memcpy(&packet.targetMacAddress, &request.cmdSpecific.useStatic.macAddress, 6);

    // Addresses are already in network byte order
    packet.ipAddress = request.cmdSpecific.useStatic.ipAddress;
    packet.subnetMask = request.cmdSpecific.useStatic.subnetMask;
    packet.defaultGateway = request.cmdSpecific.useStatic.defaultGateway;

    SetTargetAddress(currCfg, request.general.ipAddress, request.general.udpPort);

    currCfg->sendMsgSize = sizeof(UseStaticT);
    currCfg->minRecvMsgSize = sizeof(GeneralInfoT);
    currCfg->maxRecvMsgSize = sizeof(GeneralInfoT);

    memset(&currCfg->sendMsg[0], 0,       XUSBNETCFG_MAX_CMD_LEN);
    memcpy(&currCfg->sendMsg[0], &packet, currCfg->sendMsgSize);

    int rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    currCfg->selectTimeout.tv_sec = timeOut;
    currCfg->selectTimeout.tv_usec = 0;
    rc = RecvUdpPkt(currCfg);
    if ((rc == -1) || (rc == 0))
    {
        return rc;
    }

    GeneralInfoT* replyPacket = (GeneralInfoT *)&currCfg->recvMsg[0];

    int numBytesReceived = rc;
    rc = IsRecvGeneralPacketCorrect(numBytesReceived, replyPacket, msgId, sentMessageType);
    // Static IP setup was successful
    if (rc != -1)
    {
        rc = 1;
    }
    return rc;
}

int ICRON_useFilteringStrategy(
        IcronUsbNetCfgT *currCfg, RequestT request, uint32_t msgId, int timeOut)
{
    MessageTypeT sentMessageType = UseFilteringStrategy_3;
    GeneralInfoT generalPacket;

    CreateGeneralInfoPacket(msgId, sentMessageType, &generalPacket);

    UseFilteringStrategyT packet;
    memcpy(&packet.generalInfo, &generalPacket, sizeof(GeneralInfoT));

    packet.filteringStrategy = request.cmdSpecific.useFilteringStrategy.filteringStrategy;

    SetTargetAddress(currCfg, request.general.ipAddress, request.general.udpPort);
    currCfg->sendMsgSize = sizeof(UseFilteringStrategyT);
    currCfg->minRecvMsgSize = sizeof(GeneralInfoT);
    currCfg->maxRecvMsgSize = sizeof(GeneralInfoT);

    memset(&currCfg->sendMsg[0], 0,       XUSBNETCFG_MAX_CMD_LEN);
    memcpy(&currCfg->sendMsg[0], &packet, currCfg->sendMsgSize);

    int rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    currCfg->selectTimeout.tv_sec = timeOut;
    currCfg->selectTimeout.tv_usec = 0;
    rc = RecvUdpPkt(currCfg);
    if ((rc == -1) || (rc == 0))
    {
        return rc;
    }

    GeneralInfoT* replyPacket = (GeneralInfoT *)&currCfg->recvMsg[0];

    int numBytesReceived = rc;
    rc = IsRecvGeneralPacketCorrect(numBytesReceived, replyPacket, msgId, sentMessageType);
    if (rc != -1)
    {
        // Acknowledgement
        if (!memcmp(&Acknowledge_0, &replyPacket->messageType, sizeof(MessageTypeT)))
        {
            rc = 1;
        }
        // Negative Acknowledgement
        else
        {
            rc = 0;
        }
    }
    return rc;
}


int SendControlCommand(
            IcronUsbNetCfgT *currCfg, struct Request request, uint32_t msgId, int timeOut, MessageTypeT sentMessageType)
{
    GeneralInfoT generalPacket;
    CreateGeneralInfoPacket(msgId, sentMessageType, &generalPacket);

    SetTargetAddress(currCfg, request.general.ipAddress, request.general.udpPort);

    currCfg->sendMsgSize = sizeof(GeneralInfoT);
    currCfg->minRecvMsgSize = sizeof(GeneralInfoT);
    currCfg->maxRecvMsgSize = sizeof(GeneralInfoT);

    memset(&currCfg->sendMsg[0], 0, XUSBNETCFG_MAX_CMD_LEN);
    memcpy(&currCfg->sendMsg[0], &generalPacket, currCfg->sendMsgSize);

    int rc = SendUdpPkt(currCfg);
    if (rc == -1)
    {
        return rc;
    }
    currCfg->selectTimeout.tv_sec = timeOut;
    currCfg->selectTimeout.tv_usec = 0;
    rc = RecvUdpPkt(currCfg);
    // Error or "receive UDP" timed out
    if ((rc == -1) || (rc == 0))
    {
        return rc;
    }

    GeneralInfoT* replyPacket = (GeneralInfoT *)&currCfg->recvMsg[0];

    int numBytesReceived = rc;
    rc = IsRecvGeneralPacketCorrect(numBytesReceived,
                                    replyPacket,
                                    msgId,
                                    sentMessageType);
      return numBytesReceived;
}
