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
#include "command.h"
#include <leon_uart.h>
#include <options.h>
#include <leon_timers.h>
#include "command_loc.h"
#include <flash_raw.h>
#include <grg.h>
#include <crc.h>

// Constants and Macros ###########################################################################
#define START_PROGRAM_TIMER_PERIOD_IN_MS (5000)
#define STATUS_UPDATE_TIMER_PERIOD_IN_MS (3000)
#define FLASH_BLOCK_SIZE (65536)

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
    enum ProgramStates currState;
    LEON_TimerValueT genericTimer;
    uint32_t blocksToErase;
    uint32_t eraseOffset;
    uint32_t pgmAddr;
    uint32_t imageSize;
    bool writeSuccess;
    bool timerEnabled;
    uint32_t timeout;
};

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static ProgramStartCommand pgmStartCmd;
static struct ProgrammingInfo pgmInfo;

// Static Function Declarations ###################################################################
static void _CMD_resetState(void);

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
    // determine address based on region number
    pgmInfo.pgmAddr = FLASHWRITER_BASE_ADDRESS;

    switch (pgmStartCmd.regionNumber)
    {
        case PROGRAM_REGION_FLASHWRITER: // IRAM region used by bootrom
            break;
        case PROGRAM_REGION_FPGA_GOLDEN: // Golden FPGA and FW image
            break;
        case PROGRAM_REGION_FW_GOLDEN: // Golden FW image
            break;
        case PROGRAM_REGION_FPGA_CURRENT: // Current FPGA and FW image
            break;
        case PROGRAM_REGION_FW_CURRENT: // Current FW image
            break;
        default:
        break;
    }
    pgmInfo.currState = PGM_STATE_IDLE;
    pgmInfo.writeSuccess = false;
    pgmInfo.timerEnabled = false;
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

//    UART_printf("Sz %x\n", pgmStartCmd.imageSize);
//    UART_printf("Addr %x\n", pgmInfo.pgmAddr);

    if (pgmStartCmd.imageSize == 0)
    {
        pgmResp.commandAccepted = 0;
    }

    if (pgmResp.commandAccepted > 0)
    {
        pgmInfo.blocksToErase = pgmStartCmd.imageSize >> 16; // 64kB blocks -- upper 16 bits
        // handle fraction
        if ((pgmStartCmd.imageSize & 0xFFFF) > 0) // 64kB blocks so look for lower 16 bits
        {
            pgmInfo.blocksToErase++;
        }

        // Start a timer - if we timeout then we didn't get an erase cmd
        pgmInfo.genericTimer = LEON_TimerRead();
        pgmInfo.timerEnabled = true;
        pgmInfo.imageSize = pgmStartCmd.imageSize;
        pgmInfo.timeout = START_PROGRAM_TIMER_PERIOD_IN_MS;
        pgmInfo.currState = PGM_STATE_PROGRAM_START;
        pgmInfo.writeSuccess = true;
        pgmResp.blocksToErase = pgmInfo.blocksToErase;
    }
    CMD_commandSendResponse(&pgmResp);
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
    // Erase flash region
    // TODO Replace with a non-blocking version (ie: poll on the done)
    // Break the region into blocks, but the size only needs to be 1 byte for the cmd
    CommandResponse pgmResp = {.commandAccepted = 0, .blocksToErase = 0};
    if (pgmInfo.blocksToErase > 0)
    {
        // NOTE: eraseGeneric has been modified that while
        // waiting on the In Progress bit it polls the UART by calling
        // UART_TX_ISR function
        FLASHRAW_eraseGeneric(
            (pgmInfo.pgmAddr + pgmInfo.eraseOffset), 1);

        if (!CMD_verifyErase(
                (pgmInfo.pgmAddr + pgmInfo.eraseOffset), FLASH_BLOCK_SIZE))
        {
            pgmResp.commandAccepted = 0;
        }
        else
        {
            pgmResp.commandAccepted = 1;
        }
        pgmInfo.blocksToErase--;
        pgmInfo.eraseOffset+= FLASH_BLOCK_SIZE;
    }
    pgmResp.blocksToErase = pgmInfo.blocksToErase;
    CMD_commandSendResponse(&pgmResp);
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
    uint32_t _size = (size == 0) ? (uint32_t)256 : (uint32_t)size;
    pgmInfo.currState = PGM_STATE_PROGRAM_DATA;
    bool lastPkt = false;
    uint32_t _sizeU = _size;
    ProgramDataResponse pgmResp = { .commandAccepted = 0};
    // Round up to nearest 16th
    // NOTE: Very strange GE SFI/Driver issue that I cannot figure out:
    // Write 3, 8, 16, 32, 48, 64, etc...and it works!
    // Write with size of 24, 60 or some other number that isn't a multiple of
    // 16 or but not less than 16 and it doesn't work! The flash is written with garbage
    // Only once did the 24 bytes write but all the bytes after were 0
    // Most of the time it would destroy the flash's data -- even if we write the full
    // GE FW image, if the last paylaod isn't a multiple of 16 it will mess up even address
    // 0x3000_0000 which is the start of flash for GE
    // Keep this workaround in until we find out of this is a GE SFI issue.
    if ((_size & 0xF) > 0)
    {
        _size &= 0xFFF0;
        _size += 0x10;
        lastPkt = true;
    }

    // using ProgramInfo struct, write to flash
    // NOTE: _write has been modified that while
    // waiting on the In Progress bit it polls the UART by calling
    // UART_TX_ISR function
    FLASHRAW_write((uint8_t*)pgmInfo.pgmAddr, (uint8_t*)data, _size);

    // Verify write - start with assumption of writeSuccess
    // if any fails, we set to false but never reset to true until next
    // start_program command
#if 0
    if(!CMD_verifyWrite((uint8_t*)(pgmInfo.pgmAddr), (uint8_t*)data, _size))
    {
        pgmInfo.writeSuccess = false;
    }

    if (!pgmInfo.writeSuccess)
    {
        UART_printf("verify failed\n");
    }
#endif
    // Update our address and size counters
    if (lastPkt)
    {
        _size = _sizeU;
    }
    pgmInfo.pgmAddr += _size;
    pgmInfo.imageSize -= _size;
    pgmResp.commandAccepted = 1;
//    UART_printf("GE: is %x\n",pgmStartCmd.imageSize);

    // Ensure we still have more data to write
    if (pgmInfo.imageSize > 0)
    {
        CMD_programDataSendResponse(&pgmResp);
    }
    else // last packet sent
    {
//        UART_printf("Ge pgm done\n");
        pgmInfo.timerEnabled = false;
        uint32_t crcCheck =
            crcFast((uint8_t*)FLASHWRITER_BASE_ADDRESS, pgmStartCmd.imageSize);
        if (pgmStartCmd.imageCrc != crcCheck)
        {
            pgmInfo.writeSuccess = false;
        }
        if (pgmInfo.writeSuccess)
        {
            pgmResp.commandAccepted = 1;
        }
        else
        {
            pgmResp.commandAccepted = 0;
        }
        CMD_programDataSendResponse(&pgmResp);
    }
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
bool CMD_verifyWrite(uint8_t* src1, uint8_t* src2, uint32_t n)
{
    uint8_t* ptr = src1;
    uint8_t* buffPtr = src2;
    uint32_t bytesChecked = 0;
    bool result = true;
//    UART_printf("Vaddr %x, %x\n", ptr, buffPtr);
    for (bytesChecked = 0; (bytesChecked < n) && result; bytesChecked ++)
    {
        if (*buffPtr != *ptr)
        {
            result = false;
            UART_printf("VFaddr %x, %x\n", ptr, buffPtr);
            UART_printf("VFval %x, %x\n", *((uint32_t*)ptr), *((uint32_t*)buffPtr));
        }
        else
        {
            ptr++;
            buffPtr++;
        }

    }
#if 0
    // failed on 32-bit boundary -check by byte
    if (!result)
    {
        result = true;
        uint8_t* s1 = (uint8_t*)ptr;
        uint8_t* s2 = (uint8_t*)buffPtr;
        for (uint8_t i = 0; (i < 4) && result; i++)
        {
            if (*s1 != *s2)
            {
                result = false;
                UART_printf("verify %x, %x\n", s1, s2);
            }
            else
            {
                s1++;
                s2++;
            }
        }
    }
#endif
    return result;
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
void CMD_timerHandler(void)
{
    // We are not using Timer2 so we'll check value here
    // If greater, execute code in switch case, then we disable ourselves
    if (!pgmInfo.timerEnabled)
    {
        return;
    }
    if (LEON_TimerCalcUsecDiff(pgmInfo.genericTimer, LEON_TimerRead()) < pgmInfo.timeout)
    {
        return;
    }
    // do something depending on the currState
    switch(pgmInfo.currState)
    {
        case PGM_STATE_IDLE:
            break;
        case PGM_STATE_PROGRAM_START:
            _CMD_resetState();
            break;
        case PGM_STATE_ERASE_REGION:
            break;
        case PGM_STATE_PROGRAM_DATA:
            UART_printf("Not done %x\n", pgmInfo.imageSize);
            break;
        default:
            break;
    }
    pgmInfo.timerEnabled = false;
}




