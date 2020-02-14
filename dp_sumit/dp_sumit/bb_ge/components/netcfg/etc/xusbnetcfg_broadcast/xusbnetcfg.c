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
//!   @file xusbnetcfg.c
//!         Command line utility to configure LEXs and REXs over the network
//!         using UDP.
//
//!   @note TESTING USING xusbnetcfglib.a
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
#include "xusbnetcfg.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>


// This allows for a maximum number of 32 devices to respond to a request device information
// broadcast. TODO: allocate memory dynamically to remove this limitation.
#define MAX_MESSAGES_RECEIVED_FROM_CMD  32

enum CommandType
{
    REQUEST_DEVICE_INFO,
    REQUEST_CONFIGURATION_RESPONSE_DATA,
    PING,
    PAIR_TO_DEVICE,
    REMOVE_DEVICE_PAIRING,
    REQUEST_DEVICE_TOPOLOGY,
    REMOVE_ALL_PAIRINGS,
    USE_FILTERING_STRATEGY,
    RESET_DEVICE,
    LED_LOCATOR_ON,
    LED_LOCATOR_OFF
};

enum _NETCFG_FilteringStrategy
{
    _Allow_All_Devices = 0,
    _Block_All_Devices_Except_HID_And_Hub,
    _Block_Mass_Storage,
    _Block_All_Devices_Except_HID_Hub_And_Smartcard,
    _Block_All_Devices_Except_Audio_And_Vendor_Specific
};

struct Command
{
    enum CommandType commandType;
    union
    {
        struct
        {
            char networkBroadcastIP[IP4_STR_LEN];
            uint8_t targetMACAddress[6];
        } requestDeviceInfo;
        struct
        {
            char networkBroadcastIP[IP4_STR_LEN];
            uint8_t targetMACAddress[6];
        } requestConfigResponseData;
        struct
        {
            char networkBroadcastIP[IP4_STR_LEN];
            uint8_t targetMACAddress[6];
        } ping;
        struct
        {
            char networkBroadcastIP[IP4_STR_LEN];
            uint8_t targetMACAddress[6];
        } ledLocatorOn;
        struct
        {
            char networkBroadcastIP[IP4_STR_LEN];
            uint8_t targetMACAddress[6];
        } ledLocatorOff;
        struct
        {
            char networkBroadcastIP[IP4_STR_LEN];
            uint8_t targetMACAddress[6];
        } resetDevice;
        struct
        {
            char networkBroadcastIP[IP4_STR_LEN];
            uint8_t targetMACAddress[6];
        } requestExtendedDeviceInfo;
        struct
        {
            char networkBroadcastIP[IP4_STR_LEN];
            uint8_t targetMACAddress[6];
            uint8_t macAddressToPairTo[6];
        } pairToDevice;
        struct
        {
            char networkBroadcastIP[IP4_STR_LEN];
            uint8_t targetMACAddress[6];
            uint8_t pairedMACAddress[6];
        } removeDevicePairing;
        struct
        {
            char networkBroadcastIP[IP4_STR_LEN];
            uint8_t targetMACAddress[6];
        } requestDeviceTopology;
        struct
        {
            char networkBroadcastIP[IP4_STR_LEN];
            uint8_t targetMACAddress[6];
        } removeAllPairings;
        struct
        {
            char networkBroadcastIP[IP4_STR_LEN];
            uint8_t targetMACAddress[6];
            uint8_t filteringStrategy;
        } useFilteringStrategy;
    } commands;
};


/**
* @brief    Displays information explaining how to execute the program.
*
* @param[in] executable The name of the application.
*/
static void usage(const char* executable);

/**
* @brief    Converts a MAC address string in the form "NN:NN:NN:NN:NN:NN" where
*           NN represents a pair of hexidecimal digits [0-9a-fA-F] into an
*           array of 6 bytes.
*
* @param[in] macAddrStr    MAC address string to convert to raw bytes.
* @param[out] macAddrArray Array to write the converted MAC address into.
*                          There should be space for 6 bytes.
*
* @return   true if the string was converted successfully or false otherwise.
*
* @note     Do not rely on the contents of macAddrArray in the case where this
*           function returns false.
*/
static bool parseMACAddress(const char* macAddrStr, uint8_t* macAddrArray);

/**
* @brief    Verifies that the input string is of the form ddd.ddd.ddd.ddd where
*           "ddd" is 1 to 3 decimal digits creating a number no greater than
*           255.  If the IP address satisfies these rules, it is copied to the
*           output buffer.
*
* @param[in] ipAddrIn   IP address string to validate and copy.
* @param[out] ipAddrOut Location to write the validated IP address into.
*
* @return   true if the string was converted successfully or false otherwise.
*
* @note     Do not rely on the contents of ipAddrOut in the case where this
*           function returns false.
*/
static bool validateAndCopyIPAddress(const char* ipAddrIn, char* ipAddrOut);

/**
* @brief    Performs a simple check that the two argument numbers are the same
*           and prints and error message if they are not.
*
* @param[in] have     The number of arguments passed to the program
* @param[in] required The number of arguments required for the given command
*
* @return   true if the numbers match.
*/
static bool requireNumArgs(unsigned int have, unsigned int required);

static bool lookupAlias(
    const char* alias,
    enum CommandType* cmd);

static bool parseArgs(int argc, char* argv[], struct Command* command);

static void printCnfgRespData(struct ResponseData *rxMessages);

static void usage(const char* executable)
{
    printf("Usage:\n");
    printf("%s request_device_info network_broadcast_ip target_mac_addr\n", executable);
    printf("%s request_configuration_response_data network_broadcast_ip target_mac_addr\n", executable);
    printf("%s ping network_broadcast_ip target_mac_addr\n", executable);
    printf("%s pair_to_device network_broadcast_ip target_mac_addr pair_to_mac_addr\n", executable);
    printf("%s remove_device_pairing network_broadcast_ip target_mac_addr paired_mac_addr\n", executable);
    printf("%s request_device_topology network_broadcast_ip target_mac_addr\n", executable);
    printf("%s remove_all_pairings network_broadcast_ip target_mac_addr\n", executable);
    printf("%s use_filtering_strategy network_broadcast_ip target_mac_addr filtering_strategy\n", executable);
    printf("%s led_locator_on network_broadcast_ip target_mac_addr\n", executable);
    printf("%s led_locator_off network_broadcast_ip target_mac_addr\n", executable);
    printf("%s reset_device network_broadcast_ip target_mac_addr\n", executable);
    printf("\n");
    printf("All MAC addresses are expressed in xx:xx:xx:xx:xx:xx form where " \
           "\"xx\" is a pair of hexadecimal digits.  All IP addresses " \
           "should be expressed in the form ddd.ddd.ddd.ddd where \"ddd\" " \
           "is 1 to 3 decimal digits with a value not exceeding 255.\n");
}

// This function takes an array of ints, and copies it to an array of bytes
// If any value is not in the 8-bit range 0-255 then it will return false, otherwise true
static bool convertWordToBytes(uint8_t * byteArray, const int * intArray, size_t arrayLen)
{
    int i;
    bool ret = true;

    for (i = 0; (i < arrayLen) && (ret); i++)
    {
        if ((intArray[i] >= 0) && (intArray[i] < 256)) // only process valid 8-bit numbers
        {
            byteArray[i] = (uint8_t)intArray[i];
        }
        else
        {
            ret = false;
        }
    }

    return ret;
}

static bool parseMACAddress(const char* macAddrStr, uint8_t* macAddrArray)
{
    int tmp[6];
    const int numMatched = sscanf(
        macAddrStr,
        "%x:%x:%x:%x:%x:%x",    // Note:    Windows doesn't support %hhx
        &tmp[0],                //          So we read full ints, and convert
        &tmp[1],                //          below to uint8_t
        &tmp[2],
        &tmp[3],
        &tmp[4],
        &tmp[5]);

    // All the values are now in a 32 bit array of ints
    // ensure each value is a valid 8 bit number, and copy that into the macAddrArray
    return ((numMatched == 6) && convertWordToBytes(macAddrArray, tmp, 6));
}

static bool validateAndCopyIPAddress(const char* ipAddrIn, char* ipAddrOut)
{
    uint8_t byteArray[4];
    int intArray[4];
    const int numMatched = sscanf(
        ipAddrIn, "%d.%d.%d.%d", &intArray[3], &intArray[2], &intArray[1], &intArray[0]);

    if ((numMatched == 4) && convertWordToBytes(byteArray, intArray, 4))
    {
        snprintf(ipAddrOut, IP4_STR_LEN, "%d.%d.%d.%d", byteArray[3], byteArray[2], byteArray[1], byteArray[0]);
        return true;
    }
    else
    {
        return false;
    }
}

static bool requireNumArgs(unsigned int have, unsigned int required)
{
    if(have == required)
    {
        return true;
    }
    else
    {
        printf("The command was given the wrong number of arguments\n");
        return false;
    }
}

static bool parseArgs(int argc, char* argv[], struct Command* command)
{
    bool parseSuccess = false;

    if(argc > 2)
    {
        char* cmdStr = argv[1];
        if(lookupAlias(cmdStr, &(command->commandType)))
        {
            switch(command->commandType)
            {
                case REQUEST_DEVICE_INFO:
                    parseSuccess = requireNumArgs(argc, 4);
                    if(parseSuccess)
                    {
                        parseSuccess &= validateAndCopyIPAddress(
                            argv[2], (char*)&(command->commands.requestDeviceInfo.networkBroadcastIP));
                        parseSuccess &= parseMACAddress(
                            argv[3], (uint8_t*)&(command->commands.requestDeviceInfo.targetMACAddress));
                    }
                    break;

                case REQUEST_CONFIGURATION_RESPONSE_DATA:
                    parseSuccess = requireNumArgs(argc, 4);
                    if(parseSuccess)
                    {
                        parseSuccess &= validateAndCopyIPAddress(
                            argv[2], (char*)&(command->commands.requestConfigResponseData.networkBroadcastIP));
                        parseSuccess &= parseMACAddress(
                            argv[3], (uint8_t*)&(command->commands.requestConfigResponseData.targetMACAddress));
                    }
                    break;

                case PING:
                    parseSuccess = requireNumArgs(argc, 4);
                    if(parseSuccess)
                    {
                        parseSuccess &= validateAndCopyIPAddress(
                            argv[2], (char*)&(command->commands.ping.networkBroadcastIP));
                        parseSuccess &= parseMACAddress(
                            argv[3], (uint8_t*)&(command->commands.ping.targetMACAddress));
                    }
                    break;

                case RESET_DEVICE:
                    parseSuccess = requireNumArgs(argc, 4);
                    if(parseSuccess)
                    {
                        parseSuccess &= validateAndCopyIPAddress(
                            argv[2], (char*)&(command->commands.resetDevice.networkBroadcastIP));
                        parseSuccess &= parseMACAddress(
                            argv[3], (uint8_t*)&(command->commands.resetDevice.targetMACAddress));
                    }
                    break;

                case LED_LOCATOR_ON:
                    parseSuccess = requireNumArgs(argc, 4);
                    if(parseSuccess)
                    {
                        parseSuccess &= validateAndCopyIPAddress(
                            argv[2], (char*)&(command->commands.ledLocatorOn.networkBroadcastIP));
                        parseSuccess &= parseMACAddress(
                            argv[3], (uint8_t*)&(command->commands.ledLocatorOn.targetMACAddress));
                    }
                    break;


                case LED_LOCATOR_OFF:
                    parseSuccess = requireNumArgs(argc, 4);
                    if(parseSuccess)
                    {
                        parseSuccess &= validateAndCopyIPAddress(
                            argv[2], (char*)&(command->commands.ledLocatorOff.networkBroadcastIP));
                        parseSuccess &= parseMACAddress(
                            argv[3], (uint8_t*)&(command->commands.ledLocatorOff.targetMACAddress));
                    }
                    break;


                case PAIR_TO_DEVICE:
                    parseSuccess = requireNumArgs(argc, 5);
                    if(parseSuccess)
                    {
                        parseSuccess &= validateAndCopyIPAddress(
                            argv[2], (char*)&(command->commands.pairToDevice.networkBroadcastIP));
                        parseSuccess &= parseMACAddress(
                            argv[3], (uint8_t*)&(command->commands.pairToDevice.targetMACAddress));
                        parseSuccess &= parseMACAddress(
                            argv[4], (uint8_t*)&(command->commands.pairToDevice.macAddressToPairTo));
                    }
                    break;

                case REMOVE_DEVICE_PAIRING:
                    parseSuccess = requireNumArgs(argc, 5);
                    if(parseSuccess)
                    {
                        parseSuccess &= validateAndCopyIPAddress(
                            argv[2], (char*)&(command->commands.removeDevicePairing.networkBroadcastIP));
                        parseSuccess &= parseMACAddress(
                            argv[3], (uint8_t*)&(command->commands.removeDevicePairing.targetMACAddress));
                        parseSuccess &= parseMACAddress(
                            argv[4], (uint8_t*)&(command->commands.removeDevicePairing.pairedMACAddress));
                    }
                    break;

                case REQUEST_DEVICE_TOPOLOGY:
                    parseSuccess = requireNumArgs(argc, 4);
                    if(parseSuccess)
                    {
                        parseSuccess &= validateAndCopyIPAddress(
                            argv[2], (char*)&(command->commands.requestDeviceTopology.networkBroadcastIP));
                        parseSuccess &= parseMACAddress(
                            argv[3], (uint8_t*)&(command->commands.requestDeviceTopology.targetMACAddress));
                    }
                    break;

                case REMOVE_ALL_PAIRINGS:
                    parseSuccess = requireNumArgs(argc, 4);
                    if(parseSuccess)
                    {
                        parseSuccess &= validateAndCopyIPAddress(
                            argv[2], (char*)&(command->commands.removeAllPairings.networkBroadcastIP));
                        parseSuccess &= parseMACAddress(
                            argv[3], (uint8_t*)&(command->commands.removeAllPairings.targetMACAddress));
                    }
                    break;

                case USE_FILTERING_STRATEGY:
                    parseSuccess = requireNumArgs(argc, 5);
                    if(parseSuccess)
                    {
                        parseSuccess &= validateAndCopyIPAddress(
                            argv[2], (char*)&(command->commands.useFilteringStrategy.networkBroadcastIP));
                        parseSuccess &= parseMACAddress(
                            argv[3], (uint8_t*)&(command->commands.useFilteringStrategy.targetMACAddress));
                        parseSuccess &= sscanf(argv[4], "%d",
                                (int *)&(command->commands.useFilteringStrategy.filteringStrategy));
                    }
                    break;

                default:
                    printf("Alias resolved to an unknown command.\n");
                    assert(false);
                    break;
            }
        }
        else
        {
            printf("Unknown command alias \"%s\"\n", cmdStr);
        }
    }

    return parseSuccess;
}

static bool lookupAlias(
    const char* alias,
    enum CommandType* cmd)
{
    if(strcmp(alias, "request_device_info") == 0)
    {
        *cmd = REQUEST_DEVICE_INFO;
    }
    else if(strcmp(alias, "request_configuration_response_data") == 0)
    {
        *cmd = REQUEST_CONFIGURATION_RESPONSE_DATA;
    }
    else if(strcmp(alias, "ping") == 0)
    {
        *cmd = PING;
    }
    else if(strcmp(alias, "reset_device") == 0)
    {
        *cmd = RESET_DEVICE;
    }
    else if(strcmp(alias, "led_locator_on") == 0)
    {
        *cmd = LED_LOCATOR_ON;
    }
    else if(strcmp(alias, "led_locator_off") == 0)
    {
        *cmd = LED_LOCATOR_OFF;
    }
    else if(strcmp(alias, "pair_to_device") == 0)
    {
        *cmd = PAIR_TO_DEVICE;
    }
    else if(strcmp(alias, "remove_device_pairing") == 0)
    {
        *cmd = REMOVE_DEVICE_PAIRING;
    }
    else if(strcmp(alias, "request_device_topology") == 0)
    {
        *cmd = REQUEST_DEVICE_TOPOLOGY;
    }
    else if(strcmp(alias, "remove_all_pairings") == 0)
    {
        *cmd = REMOVE_ALL_PAIRINGS;
    }
    else if(strcmp(alias, "use_filtering_strategy") == 0)
    {
        *cmd = USE_FILTERING_STRATEGY;
    }
    else
    {
        return false;
    }

    return true;
}


static void printUnhandled(struct ResponseData* responseData)
{
    printf(
        "Device with MAC Address %02x:%02x:%02x:%02x:%02x:%02x replied with an unhandled command message.\n",
        responseData->macAddress[0],
        responseData->macAddress[1],
        responseData->macAddress[2],
        responseData->macAddress[3],
        responseData->macAddress[4],
        responseData->macAddress[5]);
}


int main(int argc, char* argv[])
{
    struct Command cmd;
    struct ResponseData rxMessages[MAX_MESSAGES_RECEIVED_FROM_CMD];
    if(parseArgs(argc, argv, &cmd))
    {
        struct IcronUsbNetCfg* myCfg = ICRON_usbNetCfgInit();
        if (myCfg == NULL)
        {
            printf("Call to ICRON_usbNetCfgInit() failed.\n");
            exit(-1);
        }
        switch(cmd.commandType)
        {
            case REQUEST_DEVICE_INFO:
                {
                    unsigned int i;
                    const int count = ICRON_requestDeviceInformation(
                        myCfg,
                        TIMEOUT_ONE_SEC,
                        rxMessages,
                        cmd.commands.requestDeviceInfo.networkBroadcastIP,
                        cmd.commands.requestDeviceInfo.targetMACAddress);
                    printf("Count = %d\n", count);
                    if (count == -1)
                    {
                        printf("Request Device Info failed\n");
                    }
                    else
                    {
                        printf("Request Device Info found %d devices\n", count);
                    }
                    for (i = 0; i < count; i++)
                    {
                        switch(rxMessages[i].command)
                        {
                            unsigned int j;
                            case REPLY_CMD_REPLY_DEVICE_INFORMATION:
                                printf("Device with MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
                                       rxMessages[i].macAddress[0],
                                       rxMessages[i].macAddress[1],
                                       rxMessages[i].macAddress[2],
                                       rxMessages[i].macAddress[3],
                                       rxMessages[i].macAddress[4],
                                       rxMessages[i].macAddress[5]);
                                printf("  Paired Devices:\n");
                                for(j = 0; j < rxMessages[i].replyDeviceInformation.numPairs; j++)
                                {
                                    printf(
                                        "    %02x:%02x:%02x:%02x:%02x:%02x\n",
                                        rxMessages[i].replyDeviceInformation.pairedWithMacAddress[j][0],
                                        rxMessages[i].replyDeviceInformation.pairedWithMacAddress[j][1],
                                        rxMessages[i].replyDeviceInformation.pairedWithMacAddress[j][2],
                                        rxMessages[i].replyDeviceInformation.pairedWithMacAddress[j][3],
                                        rxMessages[i].replyDeviceInformation.pairedWithMacAddress[j][4],
                                        rxMessages[i].replyDeviceInformation.pairedWithMacAddress[j][5]);
                                }
                                printf(
                                    "  LexOrRex = %s\n",
                                    (rxMessages[i].replyDeviceInformation.lexOrRex == 0) ? "LEX" : "REX");
                                printf(
                                    "  Supported Protocol Version = %d\n",
                                    rxMessages[i].replyDeviceInformation.supportedProtocolVersion);
                                printf("  Vendor = %s\n", rxMessages[i].replyDeviceInformation.vendor);
                                printf("  Product = %s\n", rxMessages[i].replyDeviceInformation.product);
                                printf("  Revision = %s\n", rxMessages[i].replyDeviceInformation.revision);
                                break;

                            case REPLY_CMD_REPLY_UNHANDLED_COMMAND:
                                printUnhandled(&rxMessages[i]);
                                break;

                            default:
                                DEBUG("Should not get any other type of packet");
                                assert(false);
                                break;
                        }
                    }
                }
                break;

            case REQUEST_CONFIGURATION_RESPONSE_DATA:
                {
                    unsigned int i = 0;
                    const int count = ICRON_requestConfigurationResponseData(
                        myCfg,
                        TIMEOUT_ONE_SEC,
                        rxMessages,
                        cmd.commands.requestConfigResponseData.networkBroadcastIP,
                        cmd.commands.requestConfigResponseData.targetMACAddress);

                    if (count == -1)
                    {
                        printf("Request Configuration Response Data failed\n");
                    }
                    else if (count == 0)
                    {
                        printf("Request Configuration Response Data could not find any device");
                    }
                        switch(rxMessages[i].command)
                        {

                            case REPLY_CMD_REPLY_CONFIGURATION_RESPONSE_DATA:
                                printCnfgRespData(rxMessages);
                                break;

                            case REPLY_CMD_REPLY_UNHANDLED_COMMAND:
                                printUnhandled(&rxMessages[i]);
                                break;

                            default:
                                DEBUG("Should not get any other type of packet");
//                                assert(false);
                                break;

                        }
                }

                break;

            case PING:
                {
                    int rc = ICRON_pingDevice(
                        myCfg,
                        TIMEOUT_ONE_SEC,
                        rxMessages,
                        cmd.commands.ping.networkBroadcastIP,
                        cmd.commands.ping.targetMACAddress);
                    if (rc == 1)
                    {
                        switch(rxMessages[0].command)
                        {
                            case REPLY_CMD_ACKNOWLEDGE:
                                printf("Device responding!\n");
                                break;

                            case REPLY_CMD_REPLY_UNHANDLED_COMMAND:
                                printUnhandled(&rxMessages[0]);
                                break;

                            default:
                                DEBUG("Should not get any other type of packet");
                                assert(false);
                                break;
                        }
                    }
                    else
                    {
                        printf("Device did not respond!\n");
                    }
                }
                break;

            case RESET_DEVICE:
                {
                    int rc = ICRON_resetDevice(
                        myCfg,
                        TIMEOUT_ONE_SEC,
                        rxMessages,
                        cmd.commands.resetDevice.networkBroadcastIP,
                        cmd.commands.resetDevice.targetMACAddress);
                    if (rc == 1)
                    {
                        switch(rxMessages[0].command)
                        {
                            case REPLY_CMD_ACKNOWLEDGE:
                                printf("Device successfully reset\n");
                                break;

                            case REPLY_CMD_REPLY_UNHANDLED_COMMAND:
                                printUnhandled(&rxMessages[0]);
                                break;

                            default:
                                DEBUG("Should not get any other type of packet");
                                assert(false);
                                break;
                        }
                    }
                    else
                    {
                        printf("Device could not be reset\n");
                    }
                }
                break;

            case LED_LOCATOR_ON:
                {
                    int rc = ICRON_ledLocatorOn(
                        myCfg,
                        TIMEOUT_ONE_SEC,
                        rxMessages,
                        cmd.commands.ledLocatorOn.networkBroadcastIP,
                        cmd.commands.ledLocatorOn.targetMACAddress);
                    if (rc == 1)
                    {
                        switch(rxMessages[0].command)
                        {
                            case REPLY_CMD_ACKNOWLEDGE:
                                printf("Device located successfully and Led Pattern started\n");
                                break;

                            case REPLY_CMD_REPLY_UNHANDLED_COMMAND:
                                printUnhandled(&rxMessages[0]);
                                break;

                            default:
                                DEBUG("Should not get any other type of packet");
                                assert(false);
                                break;
                        }
                    }
                    else
                    {
                        printf("Request to locate device failed\n");
                    }
                }
                break;

            case LED_LOCATOR_OFF:
                {
                    int rc = ICRON_ledLocatorOff(
                        myCfg,
                        TIMEOUT_ONE_SEC,
                        rxMessages,
                        cmd.commands.ledLocatorOff.networkBroadcastIP,
                        cmd.commands.ledLocatorOff.targetMACAddress);
                    if (rc == 1)
                    {
                        switch(rxMessages[0].command)
                        {
                            case REPLY_CMD_ACKNOWLEDGE:
                                printf("Device located successfully and Led Pattern stopped\n");
                                break;

                            case REPLY_CMD_REPLY_UNHANDLED_COMMAND:
                                printUnhandled(&rxMessages[0]);
                                break;

                            default:
                                DEBUG("Should not get any other type of packet");
                                assert(false);
                                break;
                        }
                    }
                    else
                    {
                        printf("Request to locate device failed\n");
                    }
                }
                break;

            case PAIR_TO_DEVICE:
                {
                    int rc = ICRON_pairToDevice(
                        myCfg,
                        TIMEOUT_ONE_SEC,
                        rxMessages,
                        cmd.commands.pairToDevice.networkBroadcastIP,
                        cmd.commands.pairToDevice.targetMACAddress,
                        cmd.commands.pairToDevice.macAddressToPairTo);
                    if (rc == 1)
                    {
                        switch(rxMessages[0].command)
                        {
                            case REPLY_CMD_ACKNOWLEDGE:
                                printf("Device paired successfully\n");
                                break;

                            case REPLY_CMD_REPLY_UNHANDLED_COMMAND:
                                printUnhandled(&rxMessages[0]);
                                break;

                            case REPLY_CMD_NEGATIVE_ACKNOWLEDGE:
                                printf("Received a NAK in response.  Is the target already paired?\n");
                                break;

                            default:
                                DEBUG("Should not get any other type of packet");
                                assert(false);
                                break;
                        }
                    }
                    else
                    {
                        printf("Pair To Device request failed!\n");
                    }
                }
                break;

            case REMOVE_DEVICE_PAIRING:
                {
                    int rc = ICRON_removeDevicePairing(
                        myCfg,
                        TIMEOUT_ONE_SEC,
                        rxMessages,
                        cmd.commands.removeDevicePairing.networkBroadcastIP,
                        cmd.commands.removeDevicePairing.targetMACAddress,
                        cmd.commands.removeDevicePairing.pairedMACAddress);
                    if (rc == 1)
                    {
                        switch(rxMessages[0].command)
                        {
                            case REPLY_CMD_ACKNOWLEDGE:
                                printf("Device pairing removed successfully\n");
                                break;

                            case REPLY_CMD_REPLY_UNHANDLED_COMMAND:
                                printUnhandled(&rxMessages[0]);
                                break;

                            case REPLY_CMD_NEGATIVE_ACKNOWLEDGE:
                                printf("Received a NAK in response.  The target is not paired to the MAC address specified.\n");
                                break;

                            default:
                                DEBUG("Should not get any other type of packet");
                                assert(false);
                                break;
                        }
                    }
                    else
                    {
                        printf("Remove Device Pairing request failed!\n");
                    }
                }
                break;

            case REQUEST_DEVICE_TOPOLOGY:
                {
                    int rc = ICRON_requestDeviceTopology(
                        myCfg,
                        TIMEOUT_ONE_SEC,
                        rxMessages,
                        cmd.commands.requestDeviceTopology.networkBroadcastIP,
                        cmd.commands.requestDeviceTopology.targetMACAddress);
                    if (rc == 1)
                    {
                        switch(rxMessages[0].command)
                        {
                            case REPLY_CMD_REPLY_DEVICE_TOPOLOGY:
                                {
                        printf("command %d\n", rxMessages[0].command);
                                    unsigned int i;
                                    printf("Get Device Topology returned %d records:\n", rxMessages[0].replyDeviceTopology.numDevices);
                                    for (i = 0; i < rxMessages[0].replyDeviceTopology.numDevices; i++)
                                    {
                                        printf("USB Address: %d\n", rxMessages[0].replyDeviceTopology.devices[i].usbAddress);
                                        printf("  Parent USB Address = %d\n", rxMessages[0].replyDeviceTopology.devices[i].usbAddressOfParent);
                                        printf("  Port On Parent = %d\n", rxMessages[0].replyDeviceTopology.devices[i].portOnParent);
                                        printf("  Is Device A Hub = %d\n", rxMessages[0].replyDeviceTopology.devices[i].isHub);
                                        printf("  USB Vendor ID = 0x%04X\n", rxMessages[0].replyDeviceTopology.devices[i].usbVendorId);
                                        printf("  USB Product ID = 0x%04X\n", rxMessages[0].replyDeviceTopology.devices[i].usbProductId);
                                    }
                                }
                                break;

                            case REPLY_CMD_REPLY_UNHANDLED_COMMAND:
                                printUnhandled(&rxMessages[0]);
                                break;

                            case REPLY_CMD_NEGATIVE_ACKNOWLEDGE:
                                printf("Received a NAK in response.  This command is not valid on REX devices.\n");
                                break;

                            default:
                                DEBUG("Should not get any other type of packet");
                                assert(false);
                                break;
                        }
                    }
                    else
                    {
                        printf("Request device topology failed!\n");
                    }
                }
                break;

            case REMOVE_ALL_PAIRINGS:
                {
                    int rc = ICRON_removeAllPairings(
                        myCfg,
                        TIMEOUT_ONE_SEC,
                        rxMessages,
                        cmd.commands.removeAllPairings.networkBroadcastIP,
                        cmd.commands.removeAllPairings.targetMACAddress);
                    if (rc == 1)
                    {
                        switch(rxMessages[0].command)
                        {
                            case REPLY_CMD_ACKNOWLEDGE:
                                printf("All pairings removed successfully\n");
                                break;

                            case REPLY_CMD_REPLY_UNHANDLED_COMMAND:
                                printUnhandled(&rxMessages[0]);
                                break;

                            default:
                                DEBUG("Should not get any other type of packet");
                                assert(false);
                                break;
                        }
                    }
                    else
                    {
                        printf("Remove All Pairings request failed!\n");
                    }
                }
                break;

            case USE_FILTERING_STRATEGY:
                {
                    int rc = ICRON_useFilteringStrategy(
                        myCfg,
                        TIMEOUT_ONE_SEC,
                        rxMessages,
                        cmd.commands.useFilteringStrategy.networkBroadcastIP,
                        cmd.commands.useFilteringStrategy.targetMACAddress,
                        cmd.commands.useFilteringStrategy.filteringStrategy);
                    if (rc == 1)
                    {
                        switch(rxMessages[0].command)
                        {
                            case REPLY_CMD_ACKNOWLEDGE:
                                printf("The specified filtering strategy was set.\n");
                                break;

                            case REPLY_CMD_REPLY_UNHANDLED_COMMAND:
                                printUnhandled(&rxMessages[0]);
                                break;

                            case REPLY_CMD_NEGATIVE_ACKNOWLEDGE:
                                printf("The specified filtering strategy could not be set.\n");
                                break;

                            default:
                                DEBUG("Should not get any other type of packet");
                                assert(false);
                                break;
                        }
                    }
                    else
                    {
                        printf("Use Filtering Strategy request failed!\n");
                    }
                }
                break;

            default:
                printf("Command type is unknown.\n");
                assert(false);
                break;
        }
    }
    else
    {
        usage(argv[0]);
    }
    return 0;
}




static void printCnfgRespData(struct ResponseData *rxMessages)
{
    int currentDev, macByteNum, numEmptyBytes;
    unsigned int i = 0;
    unsigned int numPairedMacs;

    printf("  USB2 high speed = %s\n", (rxMessages[i].replyConfigurationResponseData.highSpeed == 1)?"Enabled":"Disabled");
    printf("  Support MSA = %s\n", (rxMessages[i].replyConfigurationResponseData.msa == 1)?"Enabled":"Disabled");
    printf("  VHub Status = %s\n", (rxMessages[i].replyConfigurationResponseData.vhub == 1)?"Enabled":"Disabled");

    enum _NETCFG_FilteringStrategy currentFiltering = rxMessages[i].replyConfigurationResponseData.currentFilterStatus;
    switch (currentFiltering)
    {
        case _Allow_All_Devices:
            printf("  Current Filter Status = %s\n", "No Filter");
        break;

        case _Block_All_Devices_Except_HID_And_Hub:
            printf("  Current Filter Status = %s\n", "Block all but HID and Hub devices");
        break;

        case _Block_Mass_Storage:
            printf("  Current Filter Status = %s\n", "Block Mass Storage devices");
        break;

        case _Block_All_Devices_Except_HID_Hub_And_Smartcard:
            printf("  Current Filter Status = %s\n", "Block all but HID, Hub and smartcard devices");
        break;

        default:
        break;
    }

    printf("  Device with MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
           rxMessages[i].macAddress[0],
           rxMessages[i].macAddress[1],
           rxMessages[i].macAddress[2],
           rxMessages[i].macAddress[3],
           rxMessages[i].macAddress[4],
           rxMessages[i].macAddress[5]);

    numPairedMacs = ((sizeof(rxMessages[i].replyConfigurationResponseData.pairedMacs))/(6*sizeof(uint8_t)));

    for (currentDev = 0; currentDev < numPairedMacs; currentDev++)
    {
        for(numEmptyBytes=0, macByteNum= 0; macByteNum<6; macByteNum++)
        {
            if(rxMessages[i].replyConfigurationResponseData.pairedMacs[currentDev][macByteNum]== 0)
            {
                numEmptyBytes++;
            }
        }
            // If all bytes of the pairedMac are empty, do not print it
            if (numEmptyBytes<5)
            {
                printf(
                    "  Paired MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
                    rxMessages[i].replyConfigurationResponseData.pairedMacs[currentDev][0],
                    rxMessages[i].replyConfigurationResponseData.pairedMacs[currentDev][1],
                    rxMessages[i].replyConfigurationResponseData.pairedMacs[currentDev][2],
                    rxMessages[i].replyConfigurationResponseData.pairedMacs[currentDev][3],
                    rxMessages[i].replyConfigurationResponseData.pairedMacs[currentDev][4],
                    rxMessages[i].replyConfigurationResponseData.pairedMacs[currentDev][5]);
            }
    }

        printf("  Port Number = %d\n", rxMessages[i].replyConfigurationResponseData.portNumber);

        printf("  Vhub - number of downstream ports = %d\n", rxMessages[i].replyConfigurationResponseData.numVhubPorts);
        printf("  Vhub - VID = 0x%04x\n", rxMessages[i].replyConfigurationResponseData.vhubVid);
        printf("  Vhub - PID = 0x%04x\n", rxMessages[i].replyConfigurationResponseData.vhubPid);

        printf("  Brand ID = %d\n", rxMessages[i].replyConfigurationResponseData.brandId);
        // If the Brand String is Zero, print 0
        // If the Brand String does not exist, then it defaults to Legacy
        if(rxMessages[i].replyConfigurationResponseData.vendor[0] == 0)
        {
            printf("  Brand String is Empty\n" );
        }
        else
        {
            printf("  Brand String = %s\n", rxMessages[i].replyConfigurationResponseData.vendor);
        }

        printf("  Product = %s\n", rxMessages[i].replyConfigurationResponseData.product);
        printf("  Revision = %s\n", rxMessages[i].replyConfigurationResponseData.revision);


}

