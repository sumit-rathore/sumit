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

enum _NETCFG_FilteringStrategy
{
    _Allow_All_Devices = 0,
    _Block_All_Devices_Except_HID_And_Hub,
    _Block_Mass_Storage,
    _Block_All_Devices_Except_HID_Hub_And_Smartcard,
    _Block_All_Devices_Except_Audio_And_Vendor_Specific
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
* @brief    Converts an IP address string in the form "ddd.ddd.ddd.ddd" into a
*           32 bit integer.
*
* @param[in] ipAddrStr IP address string to convert to a 32 bit integer.
* @param[out] ipAddr   32 bit integer to write the IP data into.
*
* @return   true if the string was converted successfully or false otherwise.
*
* @note     Do not rely on the contents of ipAddr in the case where this
*           function returns false.
*/
static bool parseIPAddress(const char* ipAddrStr, uint32_t* ipAddr);

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
* @brief    Verifies that the input string is of the form
*           ddd.ddd.ddd.ddd[:ppppp] where "ddd" is 1 to 3 decimal digits
*           creating a number no greater than 255.  The square brackets "[]"
*           denote an optional component in which a colon is followed by 1 to 5
*           decimal digits with a value in the range 1-65535.  This optional
*           component is a port number. If all supplied components are valid,
*           then are copied into the output parameters and the function will
*           return true.
*
* @param[in] addrStr    IP address and optional port string to validate and copy.
* @param[out] ipAddrOut Location to write the validated IP address into.
* @param[out] portNum   The value where the port number will be written to.
*
* @return   true if the string was converted successfully or false otherwise.
*/
static bool validateAndCopyIPAddressWithOptionalPort(
    const char* addrStr, char* ipAddrOut, uint16_t* portNum);

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

/**
* @brief    Map command strings to CommandType enums.
*
* @param[in]  argc            Number of arguments in argv
* @param[in]  argv            Command line arguments to the program
* @param[out] cmd             CommandType enum for the given command string
* @param[out] protocolVersion Protocol version
* @param[out] forcedProtocol  true if the command is forced protocol
*
* @return   true if a corresponding CommandType enum was found.
*/
static bool lookupAlias(int argc,
                        char* argv[],
                        enum CommandType* cmd,
                        uint8_t* protocolVersion,
                        bool* forcedProtocol);

/**
* @brief    Parse Args based on the protocol version
*
* @param[in]  argc, argv
* @param[in]  myCfg      Global variables structure
* @param[out] command    structure that contains input arguments
*                        like IP address, MAC address, etc. which are
*                        parsed from argv.
*
* @return   true if the numbers match.
*/
static bool parseArgs(int argc,
                      char* argv[],
                      IcronUsbNetCfgT* myCfg,
                      RequestT* command);

/**
* @brief    Prints Configuration Response Data
*
* @param[in]  ReplyT *reply
* @param[out] prints responses respective to values in the
*             structure replyCnfgRespData
*
*/
static void printCnfgRespData (ReplyT *reply);

static void usage(const char* executable)
{
    printf("Usage:\n");
    printf("%s request_device_info ip[:port]\n", executable);
    printf("%s ping ip[:port]\n", executable);
    printf("%s [forced protocol_version] request_extended_device_info ip[:port]\n", executable);
    printf("%s [forced protocol_version] pair_to_device ip[:port] mac_address\n", executable);
    printf("%s [forced protocol_version] request_device_topology ip[:port]\n", executable);
    printf("%s request_configuration_response_data ip[:port]\n", executable);
    printf("%s [forced protocol_version] use_dhcp target_ip_address[:port] mac_address\n", executable);
    printf("%s [forced protocol_version] use_static_ip target_ip_address[:port] mac_address ip subnet_mask default_gateway\n", executable);
    printf("%s [forced protocol_version] use_filtering_strategy ip[:port] filtering_strategy\n", executable);
    printf("For protocol 1:\n");
    printf("\t%s [forced protocol_version] remove_device_pairing ip[:port]\n", executable);
    printf("For protocol > 2:\n");
    printf("\t%s [forced protocol_version] remove_device_pairing ip[:port] mac_address\n", executable);
    printf("\n");
    printf("%s[forced protocol_version] led_locator_on ip[:port]\n", executable);
    printf("%s[forced protocol_version] led_locator_off ip[:port]\n", executable);
    printf("%s[forced protocol_version] reset_device ip[:port]\n", executable);
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
        snprintf(
            ipAddrOut,
            IP4_STR_LEN,
            "%d.%d.%d.%d",
            byteArray[3],
            byteArray[2],
            byteArray[1],
            byteArray[0]);
        return true;
    }
    else
    {
        return false;
    }
}


static bool validateAndCopyIPAddressWithOptionalPort(
    const char* addrStr, char* ipAddrOut, uint16_t* portNum)
{
    bool result = true;
    char* colon = strchr(addrStr, ':');
    long portVal = -1; // -1 is a sentinal value used below
    if (colon == NULL)
    {
        // If no ':' character was found then it is assumed that there is no
        // port number.
    }
    else
    {
        const int numericBase = 10;
        char* endPtr;
        portVal = strtol(++colon, &endPtr, numericBase);
        const long maxUdpPortNum = 0xFFFF;
        if (endPtr == colon)
        {
            // Nothing parsed.  This is a failure because there should never be a colon with
            // non-numeric data to follow.
            result = false;
        }
        else if (portVal <= 0 || portVal > maxUdpPortNum)
        {
            // Parsed value is outside of allowable range for a UDP port number.
            result = false;
        }
        else
        {
            // Port value is valid, but do not copy it into portNum yet because
            // we haven't validated the IP address component.
        }
    }

    if (result)
    {
        if (validateAndCopyIPAddress(addrStr, ipAddrOut))
        {
            if (portVal != -1)
            {
                *portNum = portVal;
            }
        }
        else
        {
            result = false;
        }
    }

    return result;
}


static bool parseIPAddress(const char* ipAddrStr, uint32_t* ipAddr)
{
    bool success;
#ifdef __MINGW32__
    // Since INADDR_NONE is 0xFFFFFFFF which is a valid IP address so there is no way to tell the
    // difference between a failure and inet_addr being called with "255.255.255.255" as a
    // parameter.  For that reason, we explicitly check for "255.255.255.255" before calling
    // inet_addr.
    if (strncmp(ipAddrStr, "255.255.255.255", IP4_STR_LEN) == 0)
    {
        *ipAddr = 0xFFFFFFFF;
        success = true;
    }
    else
    {
        *ipAddr = inet_addr(ipAddrStr);
        success = (*ipAddr != INADDR_NONE);
    }
#else
    struct in_addr address;
    success = inet_aton(ipAddrStr, &address);
    if(success)
    {
        *ipAddr = address.s_addr;
    }
#endif
    return success;
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


static bool parseArgs(int argc, char* argv[], IcronUsbNetCfgT* myCfg, RequestT* command)
{
    bool parseSuccess = false;

    if(argc > 2)
    {
        bool forcedProtocol = false;
        parseSuccess = lookupAlias(
            argc, argv, &command->commandType, &myCfg->protocolVersion, &forcedProtocol);
        if (parseSuccess)
        {
            // Number of args
            int argNum = 0;

            argNum += 1;    // ./xusbnetcfg.exe
            if (forcedProtocol)
            {
                argNum += 2; // forcedFlag protocol_number
            }
            argNum += 1;    // command

            if ((command->commandType != REQUEST_DEVICE_INFO) &&
                    (command->commandType != PING) &&
                    !forcedProtocol)
            {
                if (validateAndCopyIPAddressWithOptionalPort(
                        argv[argNum], command->general.ipAddress, &command->general.udpPort))
                {
                    uint32_t msgId = rand();
                    ReplyT reply[1];
                    int numDevices = ICRON_requestDevInfo(myCfg, *command, msgId, TIMEOUT_ONE_SEC, reply);

                    DEBUG("Protocol version: %d\n", myCfg->protocolVersion);
                    if (numDevices <= 0)
                    {
                        printf("Could not retrieve the protocol version " \
                                "for the device with the specified IP Address.\n");
                        printf("Device with the specified IP address could not be found.\n");
                        printf("\n");
                        exit(1);
                    }
                }
                else
                {
                    printf("Please enter a valid IP address.\n");
                    return false;
                }
            }

            parseSuccess = argc > argNum;
            if (parseSuccess)
            {
                parseSuccess = validateAndCopyIPAddressWithOptionalPort(
                        argv[argNum], command->general.ipAddress, &command->general.udpPort);
                argNum++;
            }

            switch(command->commandType)
            {
                case REQUEST_DEVICE_INFO:
                    break;

                case PING:
                    break;

                case REQUEST_EXTENDED_DEVICE_INFO:
                    break;

                case REQUEST_CONFIGURATION_RESPONSE_DATA:
                    break;

                case PAIR_TO_DEVICE:
                    parseSuccess = requireNumArgs(argc, argNum + 1);
                    if(parseSuccess)
                    {
                        parseSuccess = parseMACAddress(
                            argv[argNum],
                            (uint8_t*)&(command->cmdSpecific.pairToDevice.macAddressToPairTo));
                    }
                    break;

                case LED_LOCATOR_ON:
                    break;

                case LED_LOCATOR_OFF:
                    break;

                case RESET_DEVICE:
                    break;

                case REMOVE_DEVICE_PAIRING:
                    if (myCfg->protocolVersion == 1)
                    {
                    }
                    else if (myCfg->protocolVersion >= 2)
                    {
                        parseSuccess = requireNumArgs(argc, argNum + 1);
                        if(parseSuccess)
                        {
                            parseSuccess = parseMACAddress(
                                argv[argNum],
                                (uint8_t*)&(command->cmdSpecific.removeDevicePairing.macAddressToUnpairFrom));
                        }
                    }
                    break;

                case REQUEST_DEVICE_TOPOLOGY:
                    break;

                case USE_DHCP:
                    parseSuccess = requireNumArgs(argc, argNum + 1);
                    if(parseSuccess)
                    {
                        parseSuccess = parseMACAddress(
                                argv[argNum], (uint8_t*)&(command->cmdSpecific.useDHCP.macAddress));
                    }
                    break;

                case USE_STATIC_IP:
                    parseSuccess = requireNumArgs(argc, argNum + 4);
                    if(parseSuccess)
                    {
                        parseSuccess &= parseMACAddress(
                                argv[argNum++], (uint8_t*)&(command->cmdSpecific.useStatic.macAddress));
                        parseSuccess &= parseIPAddress(
                                argv[argNum++], &(command->cmdSpecific.useStatic.ipAddress));
                        parseSuccess &= parseIPAddress(
                                argv[argNum++], &(command->cmdSpecific.useStatic.subnetMask));
                        parseSuccess &= parseIPAddress(
                                argv[argNum++], &(command->cmdSpecific.useStatic.defaultGateway));
                    }
                    break;

                case USE_FILTERING_STRATEGY:
                    parseSuccess = requireNumArgs(argc, argNum + 1);
                    if(parseSuccess)
                    {
                        parseSuccess = sscanf(
                            argv[argNum],
                            "%d",
                            (int *)&(command->cmdSpecific.useFilteringStrategy.filteringStrategy));
                    }
                    break;

                default:
                    printf("Alias resolved to an unknown command.\n");
                    assert(false);
                    break;
            }
        }
    }

    return parseSuccess;
}

static bool lookupAlias(
    int argc,
    char* argv[],
    enum CommandType* cmd,
    uint8_t* protocolVersion,
    bool* forcedProtocol)
{
    int argNum = 1;

    if(strcmp(argv[argNum], "forced") == 0)
    {
        // in forced mode,
        // xusbnetcfg.exe forced 2 request_device_topology 192.168.2.11
        // need two extra args i.e. "found 2"
        if(argc > 4)
        {
            *protocolVersion = atoi(argv[2]);
            *forcedProtocol = true;
            argNum += 2;
        }
        else
        {
            return false;
        }
    }

    char* alias = argv[argNum];

    if(strcmp(alias, "request_device_info") == 0)
    {
        *cmd = REQUEST_DEVICE_INFO;
    }
    else if(strcmp(alias, "ping") == 0)
    {
        *cmd = PING;
    }
    else if(strcmp(alias, "request_extended_device_info") == 0)
    {
        *cmd = REQUEST_EXTENDED_DEVICE_INFO;
    }
      else if(strcmp(alias, "pair_to_device") == 0)
    {
        *cmd = PAIR_TO_DEVICE;
    }
    else if(strcmp(alias, "led_locator_on") == 0)
    {
        *cmd = LED_LOCATOR_ON;
    }
    else if(strcmp(alias, "led_locator_off") == 0)
    {
        *cmd = LED_LOCATOR_OFF;
    }
    else if(strcmp(alias, "reset_device") == 0)
    {
        *cmd = RESET_DEVICE;
    }
    else if(strcmp(alias, "remove_device_pairing") == 0)
    {
        *cmd = REMOVE_DEVICE_PAIRING;
    }
    else if(strcmp(alias, "request_device_topology") == 0)
    {
        *cmd = REQUEST_DEVICE_TOPOLOGY;
    }
    else if(strcmp(alias, "use_dhcp") == 0)
    {
        *cmd = USE_DHCP;
    }
    else if(strcmp(alias, "use_static_ip") == 0)
    {
        *cmd = USE_STATIC_IP;
    }
    else if(strcmp(alias, "use_filtering_strategy") == 0)
    {
        *cmd = USE_FILTERING_STRATEGY;
    }
    else if(strcmp(alias, "request_configuration_response_data") == 0)
    {
        *cmd = REQUEST_CONFIGURATION_RESPONSE_DATA;
    }
    else
    {
        return false;
    }

    return true;
}


int main(int argc, char* argv[])
{
    RequestT cmd;
    ReplyT reply[MAX_USB_DEVICES];
    IcronUsbNetCfgT* myCfg = ICRON_usbNetCfgInit();

    if (myCfg == NULL)
    {
        printf("Call to Initialize Windows Network software failed.\n");
        exit(-1);
    }

    // Initialize the port number with the default value.  This may be
    // overridden by the user.
    cmd.general.udpPort = XUSBNETCFG_DEFAULT_PORT;

    if(parseArgs(argc, argv, myCfg, &cmd))
    {
        switch(cmd.commandType)
        {
            case REQUEST_DEVICE_INFO:
                {
                    struct in_addr address;
                    uint32_t msgId = rand();
                    int numDevices = ICRON_requestDevInfo(myCfg, cmd, msgId, TIMEOUT_TWO_SEC, reply);

                    if (numDevices == -1)
                    {
                        printf("Request Device Info failed\n");
                    }
                    else
                    {
                        printf("Request Device Info found %d devices.\n", numDevices);
                        unsigned int i;
                        for (i = 0; i < numDevices; i++)
                        {
                            printf("Device with MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
                                    reply[i].u.replyDevInfo.mac[0],
                                    reply[i].u.replyDevInfo.mac[1],
                                    reply[i].u.replyDevInfo.mac[2],
                                    reply[i].u.replyDevInfo.mac[3],
                                    reply[i].u.replyDevInfo.mac[4],
                                    reply[i].u.replyDevInfo.mac[5]);
                            address.s_addr = reply[i].u.replyDevInfo.ipAddr;
                            printf("  IP = %s\n", inet_ntoa(address));
                            printf("  Network Acquisition Mode = %s\n",
                                    (reply[i].u.replyDevInfo.networkAcquisitionMode == 1) ?
                                    "Static" : "DHCP");
                            printf("  Supported Protocol Version = %d\n",
                                    reply[i].u.replyDevInfo.supportedProtocol);
                            printf("  Vendor = %s\n", reply[i].u.replyDevInfo.vendor);
                            printf("  Product = %s\n", reply[i].u.replyDevInfo.product);
                            printf("  Revision = %s\n", reply[i].u.replyDevInfo.revision);
                        }
                    }
                }
                break;

            case REQUEST_CONFIGURATION_RESPONSE_DATA:
                {
                    uint32_t msgId = rand();
                    int numDevices = ICRON_requestCnfgRespData(myCfg, cmd, msgId, TIMEOUT_TWO_SEC, reply);

                    if (numDevices == -1)
                    {
                        printf("Request Configuration Response Data failed\n");
                    }

                    else
                    {
                        printCnfgRespData(reply);
                    }
                }
                break;

            case PING:
                {
                    uint32_t msgId = rand();
                    int rc = ICRON_pingDevice(myCfg, cmd, msgId, TIMEOUT_ONE_SEC);
                    if (rc == 0)
                    {
                        printf("Device responded with an Acknowledge.\n");
                    }
                    else
                    {
                        printf("Device did not respond!\n");
                    }
                }
                break;

            case REQUEST_EXTENDED_DEVICE_INFO:
                {
                    uint32_t msgId = rand();
                    int numPairedMacs =
                        ICRON_getExtendedDevInfo(myCfg, cmd, msgId, TIMEOUT_ONE_SEC, reply);
                    if (numPairedMacs >= 0)
                    {
                        unsigned int i;
                        printf("Device responded with:\n");
                        printf("  lexOrRex = %s\n",
                                (reply[0].u.replyExtDevInfo.lexOrRex == 1) ?
                                "REX" : "LEX");
                        printf("  Number of Paired devices: %d\n", numPairedMacs);
                        for (i = 0; i < numPairedMacs; i++)
                        {
                            printf("    Paired MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
                                    reply[0].u.replyExtDevInfo.pairedMacs[i][0],
                                    reply[0].u.replyExtDevInfo.pairedMacs[i][1],
                                    reply[0].u.replyExtDevInfo.pairedMacs[i][2],
                                    reply[0].u.replyExtDevInfo.pairedMacs[i][3],
                                    reply[0].u.replyExtDevInfo.pairedMacs[i][4],
                                    reply[0].u.replyExtDevInfo.pairedMacs[i][5]);
                        }
                    }
                    else
                    {
                        printf("Request Extended Device Information failed!\n");
                    }
                }
                break;

            case PAIR_TO_DEVICE:
                {
                    uint32_t msgId = rand();
                    int rc = ICRON_pairToDevice(myCfg, cmd, msgId, TIMEOUT_ONE_SEC);
                    if (rc == 1)
                    {
                        printf("Device paired successfully\n");
                    }
                    else if (rc == 2)
                    {
                        printf("Device pairing failed because either the device is already paired or all vports are already being used.\n");
                    }
                    else
                    {
                        printf("Pair To Device request failed!\n");
                    }
                }
                break;

            case REMOVE_DEVICE_PAIRING:
                {
                    uint32_t msgId = rand();
                    int rc = ICRON_removeDevicePairing(myCfg, cmd, msgId, TIMEOUT_ONE_SEC);
                    if (rc == 1)
                    {
                        printf("Device pairing removed successfully\n");
                    }
                    else if (rc == 2)
                    {
                        printf("Cannot remove pairing since device 1 is not currently paired with device 2.\n");
                    }
                    else
                    {
                        printf("Remove Device Pairing request failed!\n");
                    }
                }
                break;

            case LED_LOCATOR_ON:
                {
                    uint32_t msgId = rand();
                    int rc = SendControlCommand(myCfg, cmd, msgId, TIMEOUT_ONE_SEC, LedLocatorOn_3);
                    if (rc == -1)
                    {
                        printf("Could not locate the device because UDP packet was not received\n");
                    }
                    else
                    {
                        printf("Device located successfully and Led Pattern started\n");
                    }
                }
                break;

            case LED_LOCATOR_OFF:
                {
                    uint32_t msgId = rand();
                    int rc = SendControlCommand(myCfg, cmd, msgId, TIMEOUT_ONE_SEC, LedLocatorOff_3);
                    if (rc == -1)
                    {
                        printf("Could not locate the device because UDP packet was not received\n");
                    }

                    else
                    {
                        printf("Device located successfully and Led Pattern stoppped\n");
                    }
                }
                break;

            case RESET_DEVICE:
                {
                    uint32_t msgId = rand();
                    int rc = SendControlCommand(myCfg, cmd, msgId, TIMEOUT_ONE_SEC, ResetDevice_3);
                    if (rc == -1)
                    {
                        printf("Could not locate the device because UDP packet was not received\n");
                    }
                    else
                    {
                        printf("Device successfully reset\n");
                    }
                }
                break;

            case REQUEST_DEVICE_TOPOLOGY:
                {
                    uint32_t msgId = rand();
                    int numDevices = ICRON_getDevTopology(myCfg, cmd, msgId, TIMEOUT_ONE_SEC, reply);
                    if (numDevices >= 0)
                    {
                        unsigned int i;
                        printf("Get Device Topology returned %d records:\n", numDevices);
                        for (i = 0; i < numDevices; i++)
                        {
                            printf("USB Address: %d\n", reply[i].u.replyDeviceTopology.usbAddr);
                            printf(
                                "  Parent USB Address = %d\n",
                                reply[i].u.replyDeviceTopology.usbParentAddr);
                            printf(
                                "  Port On Parent = %d\n",
                                reply[i].u.replyDeviceTopology.portOnParent);
                            printf("  Is Device A Hub = %s\n",
                                    (reply[i].u.replyDeviceTopology.isHub == 1) ?
                                    "TRUE" : "FALSE");
                            if (reply[i].u.replyDeviceTopology.doesVendorIdExist)
                            {
                                printf(
                                    "  USB Vendor ID = 0x%04X\n",
                                    reply[i].u.replyDeviceTopology.usbVendorId);
                            }
                            if (reply[i].u.replyDeviceTopology.doesProductIdExist)
                            {
                                printf(
                                    "  USB Product ID = 0x%04X\n",
                                    reply[i].u.replyDeviceTopology.usbProductId);
                            }
                        }
                    }
                    else if (numDevices == -2)
                    {
                        printf("Device is a REX. Device Topology is only returned for LEX.\n");
                    }
                    else
                    {
                        printf("Request Device Topology failed!\n");
                    }
                }
                break;

            case USE_DHCP:
                {
                    uint32_t msgId = rand();
                    int rc = ICRON_useDHCP(myCfg, cmd, msgId, TIMEOUT_ONE_SEC);
                    if (rc == 1)
                    {
                        printf("Device set to use DHCP successfully\n");
                    }
                    else if (rc == 0)
                    {
                        printf("Specified MAC Address is not valid for the target IP Address\n");
                    }
                    else
                    {
                        printf("Failed to set device to use DHCP!\n");
                    }
                }
                break;

            case USE_STATIC_IP:
                {
                    uint32_t msgId = rand();
                    int rc = ICRON_useStaticIP(myCfg, cmd, msgId, TIMEOUT_ONE_SEC);

                    if (rc == 1)
                    {
                        printf("Device set to use a static IP successfully\n");
                    }
                    else if (rc == 0)
                    {
                        printf("Specified MAC Address is not valid for the target IP Address\n");
                    }
                    else
                    {
                        printf("Failed to set device to use a static IP!\n");
                    }
                }
                break;

            case USE_FILTERING_STRATEGY:
                {
                    uint32_t msgId = rand();
                    int rc = ICRON_useFilteringStrategy(myCfg, cmd, msgId, TIMEOUT_ONE_SEC);
                    if (rc == 1)
                    {
                        printf("Device has been set to the specified filtering strategy\n");
                    }
                    else if (rc == 0)
                    {
                        printf("Device could not be set to the specified filtering strategy\n");
                    }
                    else
                    {
                        printf("An error occurred when setting the filtering strategy.\n");
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


static void printCnfgRespData (ReplyT *reply)
{
    struct in_addr address;
    unsigned int i = 0;
    int currentDev, macByteNum, numEmptyBytes;

    // Number of paired macs = Total size of Received pairedMacs divided by
    // the size of one mac address which is 6*sizeof(uint8_t)
    int numPairedMacs = ((sizeof(reply[i].u.replyCnfgRespData.pairedMacs))/(6*sizeof(uint8_t)));

    printf("  USB2 high speed = %s\n", (reply[i].u.replyCnfgRespData.highSpeed == 1)?"Enabled":"Disabled");
    printf("  Support MSA = %s\n", (reply[i].u.replyCnfgRespData.msa == 1)?"Enabled":"Disabled");
    // Vhub is also known as Simultaneous Users
    printf("  Simultaneous Users = %s\n", (reply[i].u.replyCnfgRespData.vhub == 1)?"Enabled":"Disabled");

    enum _NETCFG_FilteringStrategy currentFiltering = reply[i].u.replyCnfgRespData.currentFilterStatus;
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

            case _Block_All_Devices_Except_Audio_And_Vendor_Specific:
                printf("  Current Filter Status = %s\n", "Block all but Audio and Vendor Specific devices");
                break;

            default:
                break;
        }

    printf("  IP Acquisition Mode = %s\n", (reply[i].u.replyCnfgRespData.ipAcquisitionMode == 1) ?
           "Static" : "DHCP");

    printf("  Device with MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
           reply[i].u.replyCnfgRespData.mac[0],
           reply[i].u.replyCnfgRespData.mac[1],
           reply[i].u.replyCnfgRespData.mac[2],
           reply[i].u.replyCnfgRespData.mac[3],
           reply[i].u.replyCnfgRespData.mac[4],
           reply[i].u.replyCnfgRespData.mac[5]);

    for (currentDev = 0; currentDev < numPairedMacs; currentDev++)
        {
            for(numEmptyBytes=0, macByteNum= 0; macByteNum<6; macByteNum++)
            {
                if(reply[i].u.replyCnfgRespData.pairedMacs[currentDev][macByteNum]== 0)
                {
                    numEmptyBytes++;
                }
            }
            // If all bytes of the pairedMac are empty, do not print it
            if (numEmptyBytes<5)
            {

                    printf("  Paired MAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
                           reply[i].u.replyCnfgRespData.pairedMacs[currentDev][0],
                           reply[i].u.replyCnfgRespData.pairedMacs[currentDev][1],
                           reply[i].u.replyCnfgRespData.pairedMacs[currentDev][2],
                           reply[i].u.replyCnfgRespData.pairedMacs[currentDev][3],
                           reply[i].u.replyCnfgRespData.pairedMacs[currentDev][4],
                           reply[i].u.replyCnfgRespData.pairedMacs[currentDev][5]);
            }

        }

    printf("  Port Number = %d\n", reply[i].u.replyCnfgRespData.portNumber);

    address.s_addr = reply[i].u.replyCnfgRespData.ipAddr;
    printf("  IP = %s\n", inet_ntoa(address));
    address.s_addr = reply[i].u.replyCnfgRespData.subNetM;
    printf("  Subnet Mask = %s\n", inet_ntoa(address));
    address.s_addr = reply[i].u.replyCnfgRespData.gateway;
    printf("  Default Gateway = %s\n", inet_ntoa(address));
    address.s_addr = reply[i].u.replyCnfgRespData.dhcpServer;
    // If dhcpServer is 255.255.255.255, it means that ip acquisition mode is static
    // and that the dhcp server is not available
    int rc = strcmp("255.255.255.255", inet_ntoa(address));
    if (rc == 0)
    {
        printf("DHCP server is not available\n");
    }
    else
    {
        printf("  DHCP Server = %s\n", inet_ntoa(address));
    }

    printf("  Vhub - number of downstream ports = %d\n", reply[i].u.replyCnfgRespData.numVhubPorts);
    printf("  Vhub - VID = 0x%04x\n", reply[i].u.replyCnfgRespData.vhubVid);
    printf("  Vhub - PID = 0x%04x\n", reply[i].u.replyCnfgRespData.vhubPid);

    printf("  Brand ID = %d\n", reply[i].u.replyCnfgRespData.brandId);
    // If the Brand String is Zero, print 0
    // If the Brand String does not exist, then it defaults to Legacy
    if(reply[i].u.replyCnfgRespData.vendor[0] == 0)
    {
        printf("  Brand String is Empty\n" );
    }
    else
    {
        printf("  Brand String = %s\n", reply[i].u.replyCnfgRespData.vendor);
    }

    printf("  Product = %s\n", reply[i].u.replyCnfgRespData.product);
    printf("  Revision = %s\n", reply[i].u.replyCnfgRespData.revision);

}
