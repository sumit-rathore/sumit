//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################

//#################################################################################################
// Module Description
//#################################################################################################
// The is basically a generic programming handler intended to work with
// UART, I2C, and Ethernet.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <itypes.h>
#include "command.h"
#include <leon_uart.h>
#include <options.h>
#include "command_loc.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static uint8_t currCmdRespID;
static bool _eraseReceived;
static uint32_t cmdRespSent;
static uint32_t pgmDataRespSent;

// Static Function Declarations ###################################################################
//typedef void (*UartPacketRxHandlerT)(enum PacketRxStatus rxStatus, const void* data, const uint16_t size, uint8_t responseID);
static void _CMD_commandHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseID);

// Exported Function Definitions ##################################################################

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void CMD_Init(void)
{
    CMD_programInit();
    UART_packetizeRegisterClient(
        CLIENT_ID_GE_COMMANDS,
        &_CMD_commandHandler);

    UART_packetizeRegisterClient(
        CLIENT_ID_GE_PROGRAM_DATA,
        &CMD_programDataHandler);
    pgmDataRespSent = 0;
    cmdRespSent = 0;

}


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
static void _CMD_commandHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseID)
{
    const uint8_t cmd = *((uint8_t*)data);
    const uint8_t subcmd = *((uint8_t*)(data) + 1);
    currCmdRespID = responseID;

    // Analyze command - first byte
    if (cmd == CMD_PROGRAM)
    {
        // Analyze subcommand - second byte
        switch (subcmd)
        {
            case CMD_START_PROGRAM:
                CMD_startProgram(data);
                _eraseReceived = false;
                break;
            case CMD_ERASE_REGION:
                CMD_eraseRegion();
                _eraseReceived = true;
                break;
            default:
                break;
        }
    }
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void CMD_commandSendResponse(CommandResponse * pgmResp)
{
    cmdRespSent++;
    pgmResp->fill1 = (uint8_t)cmdRespSent;
    UART_packetizeSendDataImmediate(
        CLIENT_ID_GE_COMMANDS,
        NULL,
        pgmResp,
        sizeof(CommandResponse));
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void CMD_programDataSendResponse(ProgramDataResponse *pgmResp)
{
    pgmDataRespSent++;
    UART_packetizeSendDataImmediate(
        CLIENT_ID_GE_PROGRAM_DATA,
        NULL,
        pgmResp,
        sizeof(ProgramDataResponse));
//    UART_printf("cPDS %x\n", pgmDataRespSent);
//    UART_printf("cRS %x\n", cmdRespSent);
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void CMD_sendSoftwareVersion(void)
{
    struct GeSoftwareVersion geSwVer;
    memset(&geSwVer, 0, sizeof(struct GeSoftwareVersion));
    geSwVer.hdr.msgId = INFO_MESSAGE_ID_VERSION;
    geSwVer.hdr.secMsgId = INFO_MESSAGE_SECONDARY_ID_VERSION_FIRMWARE;
    geSwVer.majorRevision = SOFTWARE_MAJOR_REVISION;
    geSwVer.minorRevision = SOFTWARE_MINOR_REVISION;
    geSwVer.debugRevision = SOFTWARE_DEBUG_REVISION;
    UART_packetizeSendDataImmediate(
        CLIENT_ID_GE_INFO,
        NULL,
        &geSwVer,
        sizeof(struct GeSoftwareVersion));
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
bool CMD_programEraseReceived(void)
{
    return _eraseReceived;
}



