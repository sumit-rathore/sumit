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
// Implementation of Program.bin from a UART perspective.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <itypes.h>
#include <command.h>
#include <uart.h>
#include <options.h>
#include <timing_timers.h>
#include "command_loc.h"
#include "command_log.h"
#include <ge_program.h>
#include <linkmgr.h>
#include <flash_raw.h>
#include <bb_top.h>
#include <crc.h>
#include <leon_timers.h>
#include <sfi.h>

#ifdef PLATFORM_A7
#include <bb_top.h>
#include <bb_top_a7.h>
#endif

// Constants and Macros ###########################################################################
#define STATUS_UPDATE_TIMER_PERIOD_IN_MS (2500)
#define FLASH_BLOCK_SIZE (65536)
#define MAX_PAYLOAD_SIZE (256)

// Data Types #####################################################################################
enum ProgramStates
{
    PGM_STATE_IDLE, // nothing happening
    PGM_STATE_PROGRAM_START, // kick off timer and wait
    PGM_STATE_ERASE_REGION, // erase regions then wait
    PGM_STATE_PROGRAM_DATA // write data to destination
};

struct ProgrammingInfo
{
    TIMING_TimerHandlerT programWatchdogTimer;      // To detect error and change to be ready for new command
    uint32_t pgmAddr;
    uint32_t blocksToErase;
    uint32_t eraseOffset;
    uint32_t crcPgmAddr;
    uint32_t crcImageSize;
    uint32_t mmuOffset;
    enum ProgramStates currState;
    bool programStartReceived;
};

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static ProgramStartCommand pgmStartCmd;
static struct ProgrammingInfo pgmInfo;

// Static Function Declarations ###################################################################
static void _CMD_resetState(void);
static void _CMD_watchdogTimerHandler(void);

#ifdef BB_PROGRAM_BB
static void eraseFinishHandler(void);
#endif
// Exported Function Definitions ##################################################################

//#################################################################################################
// Register our timer
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void CMD_programInit(void)
{
    pgmInfo.programWatchdogTimer = TIMING_TimerRegisterHandler(
        &_CMD_watchdogTimerHandler,
        false,
        STATUS_UPDATE_TIMER_PERIOD_IN_MS);

//TODO Move this code into startProgram and have it parse the command and set the params
    // determine address based on region number
    pgmInfo.currState = PGM_STATE_IDLE;
}


//#################################################################################################
// Enter programming mode, start a timer -- timeout goes back to idle
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void CMD_startProgram(const void* data)
{
    CommandResponse pgmResp = {.commandAccepted = 1, .blocksToErase = 0};

    // map data to ProgramStartCommand struct
    memcpy(&pgmStartCmd, data, sizeof(ProgramStartCommand));

    if (pgmStartCmd.imageSize == 0)
    {
        pgmResp.commandAccepted = 0;
    }

    if (pgmResp.commandAccepted > 0)
    {
        // For programBB we must ensure the APB bus offset is 0 so writes are not automatically
        // given an offset, same for the reads which are through AHB
        // MainFW will use the offsets, however ProgramBB uses the absolute address values below
        // based on RegionNumber, for both writes and reads
        SFI_setApbAddressOffset(0);
        pgmInfo.mmuOffset = 0;
        SFI_setMmuAddressOffset(0);

        switch (pgmStartCmd.regionNumber)
        {
            case PROGRAM_REGION_FLASHWRITER: // IRAM region used by bootrom
                pgmInfo.pgmAddr = 0; // The ROM has its own handler - this isn't used by anything
                break;
            case PROGRAM_REGION_FPGA_GOLDEN: // Golden FPGA and FW image
                FLASHRAW_Unprotect();
                CMD_setForFlashReProtect();
                pgmInfo.pgmAddr = FPGA_GOLDEN_FLASH_START_ADDRESS;
                break;
            case PROGRAM_REGION_FW_GOLDEN: // Golden FW image
                FLASHRAW_Unprotect();
                CMD_setForFlashReProtect();
                pgmInfo.pgmAddr = FLASH_BIN_FPGA_GOLDEN_TABLE_ADDRESS;
                break;
            case PROGRAM_REGION_FPGA_CURRENT: // Current FPGA and FW image
                pgmInfo.mmuOffset = FPGA_CURRENT_FLASH_START_ADDRESS - FPGA_GOLDEN_FLASH_START_ADDRESS;
                pgmInfo.pgmAddr = FPGA_CURRENT_FLASH_START_ADDRESS;
                break;
            case PROGRAM_REGION_FW_CURRENT: // Current FW image
                // ASIC or FPGA
                pgmInfo.mmuOffset = FPGA_CURRENT_FLASH_START_ADDRESS - FPGA_GOLDEN_FLASH_START_ADDRESS;
                pgmInfo.pgmAddr =
                    bb_top_IsASIC() ?
                    FLASH_BIN_ASIC_CURRENT_TABLE_ADDRESS : FLASH_BIN_FPGA_CURRENT_TABLE_ADDRESS;
                break;
            default:
            break;
        }
        // Keep this printf for now until QA has tested Multiboot/Fallback
        // It is handy to know what region you are programming
        ilog_COMMAND_COMPONENT_1(ILOG_MINOR_EVENT, CMD_PROGRAM_ADDRESS, pgmInfo.pgmAddr);

        pgmInfo.blocksToErase = pgmStartCmd.imageSize >> 16; // 64kblocks upper 16 bits
        if ((pgmStartCmd.imageSize & 0xFFFF) > 0)
        {
            pgmInfo.blocksToErase++;
        }

        // Start a timer - if we timeout then we didn't get an erase cmd
        TIMING_TimerStart(pgmInfo.programWatchdogTimer);
        pgmInfo.currState = PGM_STATE_PROGRAM_START;
        pgmResp.blocksToErase = pgmInfo.blocksToErase;
        pgmInfo.programStartReceived = true;

        // store these for CRC
        pgmInfo.crcImageSize = pgmStartCmd.imageSize;
        pgmInfo.crcPgmAddr = pgmInfo.pgmAddr;
        pgmInfo.eraseOffset = 0;
    }
    UART_WaitForTx();
    CMD_commandSendResponse(&pgmResp);
    UART_WaitForTx();
}

//#################################################################################################
// Enter programming mode, start a timer -- timeout goes back to idle
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void CMD_eraseRegion(void)
{
    pgmInfo.currState = PGM_STATE_ERASE_REGION;
    TIMING_TimerStart(pgmInfo.programWatchdogTimer);

    // Erase flash region
    // TODO Replace with a non-blocking version (ie: poll on the done)
    // Break the region into blocks, but the size only needs to be 1 byte

    if (pgmInfo.blocksToErase > 0)
    {
        // NOTE: eraseGeneric has been modified that while
        // waiting on the In Progress bit it polls the UART by calling
        // UART_TX_ISR function
#ifdef BB_PROGRAM_BB
        FLASHRAW_eraseGenericAsync((pgmInfo.pgmAddr + pgmInfo.eraseOffset), 1, eraseFinishHandler);
#endif
    }
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
void CMD_programDataHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseID)
{
    uint16_t rxPacketSize = (size == 0) ? MAX_PAYLOAD_SIZE : size;

    ProgramDataResponse pgmResp = { .commandAccepted = 0};
    // reject if we've already written the image
    if (pgmStartCmd.imageSize == 0)
    {
        CMD_programDataSendResponse(&pgmResp, responseID);
        UART_WaitForTx();
        return;
    }

    pgmInfo.currState = PGM_STATE_PROGRAM_DATA;

    // using ProgramInfo struct, write to flash
    // NOTE: _write has been modified that while
    // waiting on the In Progress bit it polls the UART by calling
    // UART_TX_ISR function
    FLASHRAW_write((uint8_t*)pgmInfo.pgmAddr, (uint8_t*)data, rxPacketSize);

#ifdef DEBUG_PROGRAMMING
    // verify
    {
        uint32_t * pF = (uint32_t*)(pgmInfo.pgmAddr);
        uint32_t * pB = (uint32_t*)(data);
        for (uint16_t i = 0; (i < rxPacketSize) && (pF < (uint32_t*)((uint32_t)pgmInfo.pgmAddr + rxPacketSize)); i++)
        {
            if (*pB != *pF)
            {
                UART_printf("buffer-flash mismatch at %x, buff %x, flash %x\n", pF, *pB, *pF);
            }
            pB++;
            pF++;
        }
    }
#endif // DEBUG_PROGRAMMING

    if(pgmStartCmd.imageSize >= rxPacketSize)
    {
        pgmInfo.pgmAddr += rxPacketSize;
        pgmStartCmd.imageSize -= rxPacketSize;
    }
    else
    {
        pgmStartCmd.imageSize = 0;
    }

    // Ensure we still have more data to write
    if (pgmStartCmd.imageSize > 0)
    {
        pgmResp.commandAccepted = 1;
        TIMING_TimerStart(pgmInfo.programWatchdogTimer);
    }
    else // last packet sent
    {
        TIMING_TimerStop(pgmInfo.programWatchdogTimer);
        uint32_t crc = crcFast((uint8_t*)pgmInfo.crcPgmAddr, pgmInfo.crcImageSize);

        if (pgmStartCmd.imageCrc == crc)
        {
            if(CMD_checkReProtectFlash())
            {
                FLASHRAW_GoldenProtect();
            }
            ilog_COMMAND_COMPONENT_0(ILOG_MAJOR_EVENT, CMD_PROGRAM_WRITE_SUCCESS);
            pgmResp.commandAccepted = 1;
        }
        else
        {
            ilog_COMMAND_COMPONENT_0(ILOG_MAJOR_EVENT, CMD_PROGRAM_WRITE_FAILED);
            pgmResp.commandAccepted = 0;
            _CMD_watchdogTimerHandler();        // Failure Reset UART port and start sending programBB heart beat signal

#ifdef DEBUG_PROGRAMMING
            // DEBUG
            {
                volatile uint32_t * p = (uint32_t*)((pgmInfo.crcPgmAddr + pgmInfo.crcImageSize - 30) & ~0x3);
                for (int i = 0; i < 12; i++)
                {
                    UART_printf("Addr %x, val %x\n", p, *p);
                    p++;
                }
                // check overlap
                p = (uint32_t*)(pgmInfo.crcPgmAddr + 0xf0);
                for (int i = 0; i < 8; i++)
                {
                    UART_printf("Addr %x, val %x\n", p, *p);
                    p++;
                }
                // check overlap
                p = (uint32_t*)(pgmInfo.crcPgmAddr + 0x1f0);
                for (int i = 0; i < 8; i++)
                {
                    UART_printf("Addr %x, val %x\n", p, *p);
                    p++;
                }

                p = (uint32_t*)(0xC0A00200);
                for (int i = 0; i < 5; i++)
                {
                    UART_printf("Addr %x, val %x\n", p, *p);
                    p++;
                }

                UART_printf("ImgSize %x\n", pgmInfo.crcImageSize);
                UART_printf("ImgCrc %x, calc %x\n", pgmStartCmd.imageCrc, crc);
                UART_WaitForTx();
            }
#endif // DEBUG_PROGRAMMING
        }
    }

    CMD_programDataSendResponse(&pgmResp, responseID);
    UART_WaitForTx();
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
bool CMD_verifyErase(uint32_t addr, uint32_t size)
{
    uint32_t* ptr = (uint32_t*)addr;
    uint32_t bytesChecked = 0;
    bool result = true;
    for (bytesChecked = 0; bytesChecked < size; bytesChecked += 4)
    {
        if (0xFFFFFFFF != *ptr)
        {
            result = false;
            uint32_t address = (uint32_t) ptr;
            ilog_COMMAND_COMPONENT_2(ILOG_FATAL_ERROR, VERIFY_ERASE_FAILED, address, *ptr);
            bytesChecked = size;
        }
        else
        {
            ptr++;
        }
    }
    return result;
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
bool CMD_verifyWrite(uint32_t* buff, uint32_t addr, uint32_t size)
{
    uint32_t* ptr = (uint32_t*)addr;
    uint32_t* buffPtr = buff;
    uint32_t bytesChecked = 0;
    bool result = true;
    for (bytesChecked = 0; bytesChecked < size; bytesChecked += 4)
    {
        if (*buffPtr != *ptr)
        {
            result = false;
                break;
            bytesChecked = size;
        }
        else
        {
            ptr++;
            buffPtr++;
        }
    }
    return result;
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool CMD_receivedProgramStart(void)
{
    return pgmInfo.programStartReceived;
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void CMD_setPgmStart(bool set)
{
    pgmInfo.programStartReceived = set;
}


//#################################################################################################
// Process subcommands for ProgramCommand
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void CMD_processCommandProgramSubcommand(const void* data)
{
    const uint8_t cmd = *((uint8_t*)data);
    const uint8_t subcmd = *((uint8_t*)(data) + 1);
    // Analyze subcommand - second byte
    switch (subcmd)
    {
        case CMD_START_PROGRAM:
            CMD_startProgram(data);
            break;
        case CMD_ERASE_REGION:
            CMD_eraseRegion();
            break;
        case CMD_PROGRAM_GE:
            CMD_setPgmStart(true);
#ifdef BB_PROGRAM_BB
            // Block download GE from cobs, BB will automatically download GE
            CommandResponseAck cmdResp = { .commandAccepted = 1};
            CMD_commandSendResponseAck(&cmdResp);
            UART_WaitForTx();
#endif
            break;
        case CMD_PROGRAM_BB:
            CMD_loadAndRunProgramBB(true);      // run program_bb for BB FW
            break;

        default:
            ilog_COMMAND_COMPONENT_2(ILOG_MAJOR_EVENT, INVALID_RX_CMD, cmd, subcmd);
            break;
    }
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
static void _CMD_resetState(void)
{
    memset(&pgmStartCmd, 0, sizeof(ProgramStartCommand));
    pgmInfo.currState = PGM_STATE_IDLE;
}


//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once during system startup.
//#################################################################################################
static void _CMD_watchdogTimerHandler(void)
{
    // do something depending on the currState
    switch(pgmInfo.currState)
    {
        case PGM_STATE_PROGRAM_START:
        case PGM_STATE_ERASE_REGION:
        case PGM_STATE_PROGRAM_DATA:
            _CMD_resetState();
            pgmInfo.programStartReceived = false;
            UART_packetizeResetBB();
            break;
        case PGM_STATE_IDLE:
        default:
            break;
    }
}


#ifdef BB_PROGRAM_BB
//#################################################################################################
// eraseFinishHandler
//  sendback message after finishing erase block
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void eraseFinishHandler(void)
{
    CommandResponse pgmResp = {.commandAccepted = 1, .blocksToErase = 0};

    if (!CMD_verifyErase((pgmInfo.pgmAddr + pgmInfo.eraseOffset), FLASH_BLOCK_SIZE) )
    {
        pgmResp.commandAccepted = 0;
    }

    pgmInfo.blocksToErase--;
    pgmInfo.eraseOffset+= FLASH_BLOCK_SIZE;

    pgmResp.blocksToErase = pgmInfo.blocksToErase;
    UART_WaitForTx();
    CMD_commandSendResponse(&pgmResp);
    UART_WaitForTx();
}
#endif
