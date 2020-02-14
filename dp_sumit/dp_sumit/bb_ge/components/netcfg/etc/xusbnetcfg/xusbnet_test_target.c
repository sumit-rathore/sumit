///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file xusbnet_test_target.c
//!         Test scaffolding to verify UDP packets from xusbnetcfg utility.
//!         This program acts as a fake USB extender device and responds to
//!         commands from protocol version 0 and version 1.
//
//!   @note TESTING USING ONE SOCKET FOR SEND AND RECEIVE.
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#ifdef __MINGW32__
#include <winsock2.h>
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

#define XUSBNETCFG_PORT 6137
#define IP4_STR_LEN 16
#define XUSBNETCFG_CMD_MAX_LEN 16
#define XUSBNETCFG_RESPONSE_MAX_LEN 128


int main(int argc, char *argv[])
{
    int timedOut = 0;
    struct sockaddr_in mySock;
    // int clientAddrLen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientAddr;
#ifdef __MINGW32__
    int
#else
    socklen_t
#endif
        clientAddrLen = sizeof(clientAddr);
    uint8_t recvMsg[XUSBNETCFG_CMD_MAX_LEN];
    uint8_t sendMsg[XUSBNETCFG_RESPONSE_MAX_LEN];
    const int sendMsgSize = XUSBNETCFG_RESPONSE_MAX_LEN;
    const int recvMsgSize = XUSBNETCFG_CMD_MAX_LEN;
    int mySock_fd;
    int rc = 0;
    int opt;
    char localIpAddr[IP4_STR_LEN];
    int dbg = 0;

    while ((opt = getopt(argc, argv, "dl:")) != -1)
    {
        switch(opt)
        {
        case 'd':
            dbg = 1;
        case 'l':
            strncpy(localIpAddr, optarg, IP4_STR_LEN);
            break;
        case ':':
        case '?':
        default:
            printf("Usage: %s -d -l <local ip>\n",
                   argv[0]);
            exit(-1);
        }
    }
    if (argc == 1)
    {
        printf("Usage: %s -d -l <local ip>\n", argv[0]);
        exit(-1);
    }

    printf("Executing xusbnet_test_target with source ip = %s, debug = %d\n",
           localIpAddr, dbg);

    /* Create and configure receive socket */
    mySock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (mySock_fd == -1)
    {
        perror("Failed to create socket");
        exit(-1);
    }
    if (dbg) printf("mySock_fd = %d\n", mySock_fd);
    memset(&mySock, 0, sizeof(struct sockaddr_in));
    mySock.sin_family = AF_INET;
    mySock.sin_port = htons(XUSBNETCFG_PORT);
    if (strcmp(localIpAddr, "any") == 0)
    {
        mySock.sin_addr.s_addr = htonl(INADDR_ANY);
        printf("Binding to INADDR_ANY\n");
    }
    else
    {
#ifdef __MINGW32__
    mySock.sin_addr.s_addr = inet_addr(localIpAddr);
    if (mySock.sin_addr.s_addr == INADDR_NONE)
#else
        // This function is only on Vista & later for Windows hosts, so for windows we use the above code
        rc = inet_pton(AF_INET, localIpAddr, &mySock.sin_addr);
        if (rc == 0)
#endif
        {
            perror("Failed to set local address");
            exit(-1);
        }
        printf("Binding to %s\n", localIpAddr);
    }
    rc = bind(mySock_fd, (struct sockaddr *) &mySock, sizeof(mySock));
    if (rc == -1) {
        perror("Failed to bind socket");
        exit(-1);
    }
#ifdef __MINGW32__
    // The FIONBIO uses an arg (flags) of non-zero to indicate non-block
    unsigned long flags = 1;
    rc = ioctlsocket(mySock_fd, FIONBIO, &flags);
    if (rc == SOCKET_ERROR)
#else
    rc = fcntl(mySock_fd, F_SETFL, O_NONBLOCK);
    if (rc == -1)
#endif
    {
        perror("Call to fcntl() failed on socket");
        exit(-1);
    }

    /* Receive Messages */
    memset((char *)&recvMsg[0], 0, recvMsgSize);
    if (dbg) printf("Calling recvfrom(), timedOut = %d\n", timedOut);
    while (timedOut < 180)
    {
        if (dbg) printf("Calling recvfrom(), recvSock_fd = %d\n", mySock_fd);
        if (dbg) printf("clientAddr.sin_port = %d\n", clientAddr.sin_port);
        if (dbg) printf("clientAddrlen = %d\n", clientAddrLen);
        printf(".");
        fflush(NULL);
        rc = recvfrom(mySock_fd, (char *)&recvMsg[0], recvMsgSize,
                            0, (struct sockaddr *) &clientAddr, &clientAddrLen);
                            // 0, (struct sockaddr *) &clientAddr, &len);
        if (rc == -1 && errno != EAGAIN)
        {
            perror("Call to recvfrom() failed");
            // exit(-1);
        }
        else if (rc > 0)
        {
            printf("\nMessage received, length = %d:\n", rc);
            printf("  MagicNumber:0x%x,0x%x,0x%x,0x%x\n",
                    recvMsg[0],
                    recvMsg[1],
                    recvMsg[2],
                    recvMsg[3]);
            printf("  MessageId:0x%x,0x%x,0x%x,0x%x\n",
                    recvMsg[4],
                    recvMsg[5],
                    recvMsg[6],
                    recvMsg[7]);
            printf("  ProtocolVersion: 0x%x\n",
                    recvMsg[8]);
            printf("  Command: 0x%x\n",
                    recvMsg[9]);
            int version = (int)recvMsg[8];
            int command = (int)recvMsg[9];
            int sendMsgLen = 10;

            /* Send Response */
            memset((char *)&sendMsg[0], 0, sendMsgSize);

            sendMsg[0] = 0x2f; // icronMagicNumber, 4 bytes
            sendMsg[1] = 0x03;
            sendMsg[2] = 0xf4;
            sendMsg[3] = 0xa2;
            sendMsg[4] = 0x1; // icronMsgId, 4 bytes
            sendMsg[5] = 0x2;
            sendMsg[6] = 0x3;
            sendMsg[7] = 0x4;
            if (version == 0 && command == 0)
            {
                /* Reply Device Information */
                sendMsg[8] = 0x0; // Version
                sendMsg[9] = 0x1; // Command
                sendMsg[10] = 0xdd; // MAC
                sendMsg[11] = 0xee;
                sendMsg[12] = 0xaa;
                sendMsg[13] = 0xdd;
                sendMsg[14] = 0xee;
                sendMsg[15] = 0xdd;
                sendMsg[16] = 0x0a; // Ip Address
                sendMsg[17] = 0x0a;
                sendMsg[18] = 0x0a;
                sendMsg[19] = 0x0a;
                sendMsg[20] = 0x01; // Supported Protocol Version
                strcpy((char*)&sendMsg[21], "ICRON");
                strcpy((char*)&sendMsg[53], "USB Ranger 2244");
                strcpy((char*)&sendMsg[85], "2.2.4.4");
                sendMsgLen = 97;
            }
            else if (version == 0 && command == 2)
            {
                /* Pong */
                sendMsg[8] = 0x0; // Version
                sendMsg[9] = 0x3; // Command
                sendMsgLen = 10;
            }
            else if (version == 1 && command == 0)
            {
                /* Reply Device Information */
                sendMsg[8] = 0x1; // Version
                sendMsg[9] = 0x1; // Command
                sendMsg[10] = 0x01; // Lex or Rex
                sendMsg[11] = 0x01; // Is Paired
                sendMsg[12] = 0xaa; // Paired to MAC
                sendMsg[13] = 0xbb;
                sendMsg[14] = 0xcc;
                sendMsg[15] = 0xdd;
                sendMsg[16] = 0xee;
                sendMsg[17] = 0xff;
                sendMsgLen = 18;

            }
            else if ((version == 1 && command == 2) || (version == 1 && command == 3))
            {
                /* Acknowledge Pairing Request */
                sendMsg[8] = 0x1; // Version
                sendMsg[9] = 0x4; // Command
                sendMsgLen = 10;
            }
            else if (version == 1 && command == 5)
            {
                /* Reply Device Topology */
                sendMsg[8] = 0x1; // Version
                sendMsg[9] = 0x6; // Command
                sendMsg[10] = 0x01; // USB Address
                sendMsg[11] = 0x02; // USB Address of Parent
                sendMsg[12] = 0x03; // Port on Parent
                sendMsg[13] = 0x01; // Is Device a Hub
                sendMsg[14] = 0x0a; // USB Address
                sendMsg[15] = 0x0b; // USB Address of Parent
                sendMsg[16] = 0x0c; // Port on Parent
                sendMsg[17] = 0x00; // Is Device a Hub
                sendMsg[18] = 0x07; // USB Address
                sendMsg[19] = 0x08; // USB Address of Parent
                sendMsg[20] = 0x09; // Port on Parent
                sendMsg[21] = 0x00; // Is Device a Hub
                sendMsgLen = 22;
            }
            /* Send message */
            // printf("Calling sendto(), sending %d bytes.\n", sendMsgSize);
            // rc = sendto(sendSock_fd, &sendMsg[0], sendMsgSize, 0,
            printf("Calling sendto(), sending %d bytes, fd = %d\n", sendMsgLen, mySock_fd);
            rc = sendto(mySock_fd, (char *)&sendMsg[0], sendMsgLen, 0,
                            (const struct sockaddr *) &clientAddr, clientAddrLen);
            if (rc == -1) {
                perror("Failed to send message");
                exit(-1);
            }
        }
#ifdef __MINGW32__
        Sleep(1000);
#else
        sleep(1);
#endif
        timedOut++;
    }

    printf("Exiting xusbnet_test_target.\n");
    /* Print recvMsg */

    return 0;
}
