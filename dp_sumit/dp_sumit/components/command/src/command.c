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
#include <command.h>
#include <sys_defs.h>
#include <uart.h>
#include <timing_timers.h>
#include "command_loc.h"
#include "command_log.h"
#include <ge_program.h>
#include <leon_mem_map.h>
#include <leon_cpu.h>
#include <bb_core.h>
#include <linkmgr.h>
#include <leon_timers.h>
#include <sys_funcs.h>
#include <bb_top.h>
#include <bb_top_ge.h>
#include <configuration.h>
#include <i2cd_dp130.h>
#include <i2cd_dp159api.h>

// Constants and Macros ###########################################################################
#define RUN_PROGRAM_BB_DELAY_IN_MS  (300) // 300ms delay

// Data Types #####################################################################################

// Global Variables ###############################################################################
extern const uint32_t chip_version;
extern const uint32_t chip_date;
extern const uint32_t chip_time;
extern const uint32_t romRev;
struct
{
    bool bbDownload;            // Indicating BB FW download case to increase Baudrate
} commandContext;
struct HardwareInfo hwInfo;     //structure to get hardware info
// Static Variables ###############################################################################
static uint8_t currCmdRespID;
static uint32_t* flash_bin_table_ptr;

// Static Function Declarations ###################################################################
//typedef void (*UartPacketRxHandlerT)(enum PacketRxStatus rxStatus, const void* data, const uint16_t size, uint8_t responseID);
static void _CMD_commandHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseID);
static bool flashNeedsProtection = false;

#ifndef BB_PROGRAM_BB
static void CMD_runProgramBB(void) __attribute__((section(".atext")));
#endif
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
        UART_PORT_BB,
        CLIENT_ID_BB_COMMANDS,
        &_CMD_commandHandler);

    UART_packetizeRegisterClient(
        UART_PORT_BB,
        CLIENT_ID_BB_PROGRAM_DATA,
        &CMD_programDataHandler);

    flash_bin_table_ptr = (uint32_t*)FLASH_BIN_TABLE_ADDRESS;
    CMD_clearFlashReProtect();
}

//#################################################################################################
// setup for flash reprotection upon system reset
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void CMD_setForFlashReProtect(void)
{
    flashNeedsProtection = true;
}

//#################################################################################################
// check for flash reprotection upon system reset
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
bool CMD_checkReProtectFlash(void)
{
    return(flashNeedsProtection);
}
//#################################################################################################
// disable flash reprotection upon system reset
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void CMD_clearFlashReProtect(void)
{
    flashNeedsProtection = false;
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void CMD_commandSendResponseAck(CommandResponseAck * pgmResp)
{
    UART_packetizeSendResponseImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_COMMANDS,
        currCmdRespID,
        pgmResp,
        sizeof(CommandResponseAck));
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void CMD_commandSendResponse(CommandResponse * pgmResp)
{
    UART_packetizeSendResponseImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_COMMANDS,
        currCmdRespID,
        pgmResp,
        sizeof(CommandResponse));
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void CMD_programDataSendResponse(ProgramDataResponse *pgmResp, uint8_t responseID)
{
    UART_packetizeSendResponseImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_PROGRAM_DATA,
        responseID,
        pgmResp,
        sizeof(ProgramDataResponse));
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void CMD_loadAndRunProgramBB(bool bbDownload)
{
    CommandResponseAck cmdResp = { .commandAccepted = 1 };

    ilog_COMMAND_COMPONENT_2(ILOG_MAJOR_EVENT, CMD_RUN_PROGRAMBB,
        flash_bin_table_ptr[FLASH_BIN_TABLE_PGM_BB_START],
        flash_bin_table_ptr[FLASH_BIN_TABLE_PGM_BB_SIZE]);

#ifdef BB_PROGRAM_BB // If programBB already running, send out DevInfo I'mAlive messages
    CMD_commandSendResponseAck(&cmdResp);
    UART_WaitForTx();
    CMD_setPgmStart(false);
#else
    commandContext.bbDownload = bbDownload;

    LINKMGR_phyShutdown();      // make sure the Phy is down - we need to focus on programming
    bb_top_SetGEToResetMode();  // prevent GE from sending MSG BB while downloading

    ilog_COMMAND_COMPONENT_0(ILOG_MAJOR_EVENT, CMD_PROGRAM_BB_CALL);

    // Send out Ack to Cobs as Cobs is still in 115200 baud rate and after receiving Ack, Cobs will
    // switch to 460800 baud rate. In this way, Cobs won't miss any ilogs from main FW and
    // ProgramBB due to baud rate mismatch.
    CMD_commandSendResponseAck(&cmdResp);
    UART_WaitForTx();

    // The delay ensures link down and ready to download before running program BB
    // It is also for Cobs to have enough time to switch to 460800 baud rate so that it can capture ilogs from ProgramBB imain
    if(IsSystemUnderAssert())       // system under assert can't run timer. directly call program bb
    {
        LEON_TimerWaitMicroSec(RUN_PROGRAM_BB_DELAY_IN_MS * 1000);
        CMD_runProgramBB();
    }
    else
    {
        TIMING_TimerHandlerT programBBRunTimer = TIMING_TimerRegisterHandler(CMD_runProgramBB, false, RUN_PROGRAM_BB_DELAY_IN_MS);
        TIMING_TimerStart(programBBRunTimer);
    }
#endif
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:.
//#################################################################################################
bool CMD_sendProgramBlockStatus(
    uint8_t blockNumber,
    uint8_t regionNumber,
    uint8_t totalBlocks,
    uint8_t msgId)
{
    if ((msgId != INFO_MESSAGE_ID_STATUS_MODULE0) &&
        (msgId != INFO_MESSAGE_ID_STATUS_MODULE1))
    {
        return false;
    }

    struct ProgramBlock pgmBlk;
    memset(&pgmBlk, 0, sizeof(struct ProgramBlock));

    pgmBlk.hdr.msgId = msgId;
    pgmBlk.hdr.secMsgId = INFO_MESSAGE_SECONDARY_ID_STATUS_PROGRAM_BLOCK;
    pgmBlk.blockNumber = blockNumber;
    pgmBlk.regionNumber = regionNumber;
    pgmBlk.totalBlocks = totalBlocks;

    UART_packetizeSendDataImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_INFO,
        NULL,
        &pgmBlk,
        sizeof(struct ProgramBlock));
    return true;
}


//#################################################################################################
//
// Parameters:
// Return:
//      valid or not
// Assumptions:
//      * This function assumes message iD will be either MODULE0 or 1
//#################################################################################################
bool CMD_sendEraseBlockStatus(
    uint8_t blockNumber,
    uint8_t regionNumber,
    uint8_t totalBlocks,
    uint8_t msgId)
{
    if ((msgId != INFO_MESSAGE_ID_STATUS_MODULE0) &&
        (msgId != INFO_MESSAGE_ID_STATUS_MODULE1))
    {
        return false;
    }

    struct EraseBlock ersBlk;
    memset(&ersBlk, 0, sizeof(struct EraseBlock));

    ersBlk.hdr.msgId = msgId;
    ersBlk.hdr.secMsgId = INFO_MESSAGE_SECONDARY_ID_STATUS_ERASE_BLOCK;
    ersBlk.blockNumber = blockNumber;
    ersBlk.regionNumber = regionNumber;
    ersBlk.totalBlocks = totalBlocks;

    UART_packetizeSendDataImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_INFO,
        NULL,
        &ersBlk,
        sizeof(struct EraseBlock));
    return true;
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void CMD_sendSoftwareVersion(uint8_t secMsgId)
{
    struct SoftwareVersion swVer = {0};
    swVer.hdr.msgId = INFO_MESSAGE_ID_VERSION;
    swVer.hdr.secMsgId = secMsgId;
    swVer.fpga.majorRevision = (uint16_t)(chip_version >> 16);
    swVer.fpga.minorRevision = (uint8_t)((chip_version >> 8) & 0xFF);
    swVer.fpga.debugRevision = (uint8_t)(chip_version & 0xFF);
    swVer.fpga.buildYear = (uint16_t)(chip_date >> 16);
    swVer.fpga.buildMonth = (uint8_t)((chip_date >> 8) & 0xFF);
    swVer.fpga.buildDay = (uint8_t)(chip_date & 0xFF);
    swVer.fpga.buildHour = (uint8_t)((chip_time >> 16) & 0xFF);
    swVer.fpga.buildMinute = (uint8_t)((chip_time >> 8) & 0xFF);
    swVer.fpga.buildSecond = (uint8_t)(chip_time & 0xFF);
    swVer.fpga.fallbackImage = (uint8_t)bb_top_IsFpgaGoldenImage() << 4 |
        (uint8_t)bb_top_a7_isFpgaFallback();
    swVer.romMajorRevision = (uint16_t)(romRev & 0xFFFF);

#ifdef BB_PROGRAM_BB
    swVer.majorRevision = PGMBB_MAJOR_REVISION;
    swVer.minorRevision = PGMBB_MINOR_REVISION;
    swVer.debugRevision = PGMBB_DEBUG_REVISION;
#else // main BB firmware
    swVer.majorRevision = SOFTWARE_MAJOR_REVISION;
    swVer.minorRevision = SOFTWARE_MINOR_REVISION;
    swVer.debugRevision = SOFTWARE_DEBUG_REVISION;
#endif
    swVer.buildYear = MAKE_BUILD_YEAR;
    swVer.buildMonth = MAKE_BUILD_MONTH;
    swVer.buildDay = MAKE_BUILD_DAY;
    swVer.buildHour = MAKE_BUILD_HOUR;
    swVer.buildMinute = MAKE_BUILD_MINUTE;
    swVer.buildSecond = MAKE_BUILD_SECOND;

    swVer.rex_lex_n = bb_core_isRex() ? 1 : 0;

    UART_packetizeSendDataImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_INFO,
        NULL,
        &swVer,
        sizeof(struct SoftwareVersion));
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void CMD_programGE(void)
{
#ifdef BB_PROGRAM_BB
    GE_PROGRAM_geEnterReset(); // place GE in reset
    // start bootROM of GE ASIC - GE_PROGRAM wil automatically
    // load flashwriter and transfer firmware
    GE_PROGRAM_geEnterBootloaderMode();
#else
    bb_core_setProgramBbOperation(SPR_PROGRAM_BB_SET_FOR_GE_DOWNLOAD);

    CMD_loadAndRunProgramBB(false);     // run program_bb for GE automatic download
#endif
}

// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void _CMD_commandHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseID)
{
    const uint8_t cmd = *((uint8_t*)data);
    currCmdRespID = responseID;

    // Analyze command - first byte
    switch (cmd)
    {
        case CMD_PROGRAM:
            CMD_processCommandProgramSubcommand(data);
            break;
        case CMD_SYSTEM:
            CMD_processCommandSystemSubcommand(data);
            break;
        case CMD_RS232:
#ifndef BB_PROGRAM_BB
            CMD_processCommandRs232Subcommand(data);
#endif
            break;
        default:
            break;
    }
}

#ifndef BB_PROGRAM_BB
//#################################################################################################
// CMD_runProgramBB
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void CMD_runProgramBB(void)
{
    if(commandContext.bbDownload)               // In case of BB FW download, increase Baudrate to high speed
    {
        UART_setBaudrate(UART_PORT_BB, LEON_UART_BAUD_460800);
    }

    LEON_CPUDisableIRQ();
    void (*runBin)(void);
    flashmemcpy((uint32_t*)LEON_IRAM_ADDR,
        (uint32_t*)flash_bin_table_ptr[FLASH_BIN_TABLE_PGM_BB_START],
        (uint32_t)flash_bin_table_ptr[FLASH_BIN_TABLE_PGM_BB_SIZE]);
    runBin = (void (*)(void))(LEON_IRAM_ADDR);  // cast as function pointer
    runBin();
}
#endif

//#################################################################################################
// CMD_hardwareInfo
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void CMD_hardwareInfo(uint8_t secMsgId)
{
    hwInfo.hdr.secMsgId = secMsgId;
    hwInfo.hdr.msgId = INFO_MESSAGE_ID_VERSION;
    hwInfo.platform = bb_top_IsASIC() << 4 | bb_core_isRex();
    hwInfo.features = Config_GetFeatureByte();
    hwInfo.platformId = CMD_GetPlatformId();

    UART_packetizeSendDataImmediate(
    UART_PORT_BB,
    CLIENT_ID_BB_INFO,
    NULL,
    &hwInfo,
    sizeof(struct HardwareInfo));
}


//#################################################################################################
// Platform ID Generate
//
// Parameters:
// Return: 0: Not defined (for backward compatibility)
//         1: Raven FPGA
//         2: Maverick DP FPGA
//
// Assumptions: This function should be called after passing 500ms from start-up
//              dp130 needs 400ms after releasing reset
//#################################################################################################
uint8_t CMD_GetPlatformId(void)
{
    uint8_t platformId = 0;

    if(!bb_top_IsASIC())
    {
        if(bb_core_isRex())
        {
            platformId = I2CD_dp130InitSuccess() ? 2 : 1;

        }
        else
        {
            platformId = I2CD_dp159InitSuccess() ? 2 : 1;
        }
    }
    return platformId;
}
