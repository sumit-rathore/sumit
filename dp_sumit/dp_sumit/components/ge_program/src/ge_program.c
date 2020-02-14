//#################################################################################################
// Icron Technology Corporation - Copyright 2016
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
// This file contains the implementation of Link Manager.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// Link Manager looks at things from a PHY level, not a Lex-Rex communication level
// * Intended use is a notification system for the Lex-Rex communication layer
// * PHY_enable/disable are external events (imain/ICMDs)
// * PHY_restart is an external event
// * PHY_synced is an external event (XAUI)
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <bb_top_ge.h>
#include <cpu_comm.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <gpio.h>
#include <bb_core.h>
#include "ge_program_log.h"
#include <sys_defs.h>
#include <uart.h>
#include <led.h>
#include <bb_ge_comm.h>
#include <ge_program.h>
#include <timing_profile.h>
#include <command.h>
#include <crc.h>
#include <configuration.h>

// Constants and Macros ###########################################################################
#define RX_NAK                                      (21)
#define RX_ACK                                      (6)

#define GE_FW_PAYLOAD_SIZE                          (256)

// Data Types #####################################################################################
enum GeProgramState
{
    GE_PGM_STATE_RESET, // GE is held in reset
    GE_PGM_STATE_WAIT_FOR_BOOTLOADER_RUN, // Waiting for Xmodem to send NAKs
    GE_PGM_STATE_BOOTLOADER_RUNNING, // Bootloader is running based on receipt of sequence
    GE_PGM_STATE_TRANSFERRING_FLSHWTR, // Xmodem transferring flash writer into IRAM
    GE_PGM_STATE_WAIT_FOR_FLSHWTR_RUN, // Last block sent, ACK ignored, wait for Flshwtr boot seq
    GE_PGM_STATE_FLSHWTR_RUNNING, // Receipt sequence of FlashWriter's welcome message
    GE_PGM_STATE_CMD_PROGRAM_START, // ProgramStart sent to flashwriter
    GE_PGM_STATE_CMD_ERASE, // Erasing flash
    GE_PGM_STATE_PROGRAM_DATA, // Sending main FW image
    // Start GE and test it works? Or let BB full system bringup determine that?
};

enum GeProgramEvent
{
    GE_PGM_EVENT_RESET_GE, // Trigger GE into reset
    // ROM Events
    GE_PGM_EVENT_ENTER_BOOTLOADER_MODE, // Event to set the boot pins to program and de-assert reset
    GE_PGM_EVENT_RX_BOOTLOADER_RUN_SEQ, // Bootloader welcome sequence received
    GE_PGM_EVENT_RX_NAK, // Received NAK - used to signal waiting (ex: waiting for image)
    GE_PGM_EVENT_RX_ACK, // Received ACK - signals Xmodem packet transfer was successful
    GE_PGM_EVENT_SENT_EOT, // Last block sent, including EOT
    GE_PGM_EVENT_RX_FLSHWTR_RUN_SEQ, // Flashwriter welcome sequence received
    // Flashwriter Command Events - pass to main FSM since commands are tied to states
    GE_PGM_EVENT_CMD_RESP_ACCEPTED, // Flashwriter accepted program start - move to next state
    GE_PGM_EVENT_CMD_RESP_FAILED, // Flashwriter rejected program start - reset?
};

// Static Function Declarations ###################################################################

static void _GE_PGM_geStateMachine(enum GeProgramEvent event) __attribute__((section(".atext")));
static void _GE_PGM_geResetHandler(enum GeProgramEvent event) __attribute__((section(".atext")));
static void _GE_PGM_geBootloaderRunningHandler(enum GeProgramEvent event) __attribute__((section(".atext")));
static void _GE_PGM_geWaitForBootloaderRunHandler(enum GeProgramEvent event) __attribute__((section(".atext")));
static void _GE_PGM_geTransferringFlshWtrHandler(enum GeProgramEvent event) __attribute__((section(".atext")));
static void _GE_PGM_geWaitForFlshWtrRunHandler(enum GeProgramEvent event) __attribute__((section(".atext")));
static void _GE_PGM_geFlshWtrRunningHandler(enum GeProgramEvent event) __attribute__((section(".atext")));
static void _GE_PGM_geCommandProgramStart(enum GeProgramEvent event) __attribute__((section(".atext")));
static void _GE_PGM_geCommandErase(enum GeProgramEvent event) __attribute__((section(".atext")));
static void _GE_PGM_geProgramData(enum GeProgramEvent event) __attribute__((section(".atext")));

static void _GE_PGM_commandResponseHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseID);

static void _GE_PGM_commandStatusHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseID);

static void _GE_PGM_programDataResponseHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseID);

static void _GE_PGM_commandSendProgramStart(void);
static void _GE_PGM_commandSendErase(void);
static void _GE_PGM_commandSendProgramData(void);
static void _GE_PGM_xmodemSend(uint8_t blkNum, uint8_t* pkt);
static void _GE_PGM_sendGeFlashWriter(bool repeat, bool reset);
static void _GE_PROGRAM_geSentEOT(void);
static void _GE_PGM_commandSendProgramDataNAK(void);

// Global Variables ###############################################################################
static uint32_t* flash_bin_table_ptr;

// Static Variables ###############################################################################
struct GePgmContext
{
    // LinkMgr PHY FSM state.
    enum GeProgramState gePgmFsmState;

    // Address of flash we're reading from
    uint32_t srcAddr;

    // Size we have left to write in bytes
    uint32_t bytesToWrite;
    uint32_t blocksToEraseOrProgram;
};

static const uint8_t romBootSequence[] =
{"Goldenears 01.02.00 Bootloader version 1.3"};
static uint8_t romBootSequenceIdx;
    // Buffer for writing firmware
static uint8_t geFwPayload[GE_FW_PAYLOAD_SIZE];
static struct GePgmContext gePgm;
static uint8_t geFlshWtrBuff[128];

// Store to send with Device Info messages
// reuse for erase and programData
static uint16_t totalBlocks;

// Exported Function Definitions ##################################################################

//#################################################################################################
// Register the generic timer and set the initial state.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void GE_PROGRAM_init(void)
{
    flash_bin_table_ptr = (uint32_t*)FLASH_BIN_TABLE_ADDRESS;

    // Need Channel CLIENT_ID_GE_COMMANDS for the ACK from flashwriter
    // do not need anything else except maybe CLIENT_ID_GE_CMD_STATUS
    UART_packetizeRegisterClient(
        UART_PORT_GE,
        CLIENT_ID_GE_COMMANDS,
        &_GE_PGM_commandResponseHandler);

    UART_packetizeRegisterClient(
        UART_PORT_GE,
        CLIENT_ID_GE_INFO,
        &_GE_PGM_commandStatusHandler);

    UART_packetizeRegisterClient(
        UART_PORT_GE,
        CLIENT_ID_GE_PROGRAM_DATA,
        &_GE_PGM_programDataResponseHandler);

    gePgm.gePgmFsmState = GE_PGM_STATE_RESET;
}


//#################################################################################################
// External input to generate EnterBootloader event.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void GE_PROGRAM_geEnterBootloaderMode(void)
{
    UART_packetizeEnableGE(false);
    
    romBootSequenceIdx = 0;
    _GE_PGM_geStateMachine(GE_PGM_EVENT_ENTER_BOOTLOADER_MODE);
}


//#################################################################################################
// External input to generate EnterBootloader event.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void GE_PROGRAM_geEnterReset(void)
{
    UART_packetizeEnableGE(false);
    _GE_PGM_geStateMachine(GE_PGM_EVENT_RESET_GE);
    UART_clearGeRx();
    UART_clearGeTx();
}


//#################################################################################################
// Process Rx Byte - Intended for non-packetized, GE ROM interface.
// This function is always one step ahead of the main state machine - it generates key state
// transitions
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void _GE_PGM_processRxByte(uint8_t byte)
{
    switch (gePgm.gePgmFsmState)
    {
        case GE_PGM_STATE_RESET: // GE is held in reset
            break;

        case GE_PGM_STATE_BOOTLOADER_RUNNING: // Bootloader is running based on receipt of sequence
            // Expecting NAK from bootloader
            // Once we load flashwriter, it will send/receive everything in packetized format
            if (RX_NAK == byte)
            {
                _GE_PGM_geStateMachine(GE_PGM_EVENT_RX_NAK);
            }
            if (RX_ACK == byte)
            {
                _GE_PGM_geStateMachine(GE_PGM_EVENT_RX_ACK);
            }
            break;

        case GE_PGM_STATE_WAIT_FOR_BOOTLOADER_RUN: // Waiting for Xmodem to send NAKs
            // Expecting bootloader start sequence
            if (romBootSequence[romBootSequenceIdx] == byte)
            {
                if (romBootSequenceIdx < (sizeof (romBootSequence) - 2))
                {
                    romBootSequenceIdx++;
                }
                else
                {
                    // ROM is running
                    _GE_PGM_geStateMachine(GE_PGM_EVENT_RX_BOOTLOADER_RUN_SEQ);
                }
            }
            break;

        case GE_PGM_STATE_TRANSFERRING_FLSHWTR: // Xmodem transferring flash writer into IRAM
            if (RX_NAK == byte)
            {
                _GE_PGM_geStateMachine(GE_PGM_EVENT_RX_NAK);
            }
            if (RX_ACK == byte)
            {
                _GE_PGM_geStateMachine(GE_PGM_EVENT_RX_ACK);
            }
            break;

        case GE_PGM_STATE_WAIT_FOR_FLSHWTR_RUN: // Last block sent, ACK ignored, wait for Flshwtr boot seq

        case GE_PGM_STATE_FLSHWTR_RUNNING: // Receipt sequence of FlashWriter's welcome message

        case GE_PGM_STATE_CMD_PROGRAM_START: // ProgramStart sent to flashwriter

        case GE_PGM_STATE_CMD_ERASE: // Erasing flash

        case GE_PGM_STATE_PROGRAM_DATA: // Sending main FW image

            break;

        default:
            break;
    }
}

//#################################################################################################
// Check Program BB was run by GE automatic update
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void GE_PGM_checkAutoDownload()
{
    if(bb_core_getProgramBbOperation() && SPR_PROGRAM_BB_SET_FOR_GE_DOWNLOAD)         // Check programBB run by automatic download
    {
        ilog_GE_PROGRAM_COMPONENT_0(ILOG_MAJOR_EVENT, GE_PGM_START);
        CMD_setPgmStart(true);
        GE_PROGRAM_geEnterReset();      // place GE in reset

        // start bootROM of GE ASIC - GE_PROGRAM wil automatically
        // load flashwriter and transfer firmware
        GE_PROGRAM_geEnterBootloaderMode();
    }
}

// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################
//#################################################################################################
// Internal input to generate EOT event.
// Used by XModem to transfer flashWriter to GE ROM
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _GE_PROGRAM_geSentEOT(void)
{
    _GE_PGM_geStateMachine(GE_PGM_EVENT_SENT_EOT);
}



//#################################################################################################
// The PHY state machine.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _GE_PGM_geStateMachine(enum GeProgramEvent event)
{
    ilog_GE_PROGRAM_COMPONENT_2(ILOG_DEBUG, GE_PROGRAM_RCV_EVENT, gePgm.gePgmFsmState, event);

    switch (gePgm.gePgmFsmState)
    {
        case GE_PGM_STATE_RESET: // GE is held in reset
            _GE_PGM_geResetHandler(event);
            break;

        case GE_PGM_STATE_WAIT_FOR_BOOTLOADER_RUN: // Waiting for Xmodem to send NAKs
            _GE_PGM_geWaitForBootloaderRunHandler(event);
            break;

        case GE_PGM_STATE_BOOTLOADER_RUNNING: // Bootloader is running based on receipt of sequence
            _GE_PGM_geBootloaderRunningHandler(event);
            break;

        case GE_PGM_STATE_TRANSFERRING_FLSHWTR: // Xmodem transferring flash writer into IRAM
            _GE_PGM_geTransferringFlshWtrHandler(event);
            break;

        case GE_PGM_STATE_WAIT_FOR_FLSHWTR_RUN: // Last block sent, ACK ignored, wait for Flshwtr boot seq
            _GE_PGM_geWaitForFlshWtrRunHandler(event);
            break;

        case GE_PGM_STATE_FLSHWTR_RUNNING: // Receipt sequence of FlashWriter's welcome message
            _GE_PGM_geFlshWtrRunningHandler(event);
            break;

        case GE_PGM_STATE_CMD_PROGRAM_START: // ProgramStart sent to flashwriter
            _GE_PGM_geCommandProgramStart(event);
            break;

        case GE_PGM_STATE_CMD_ERASE: // Erasing flash
            _GE_PGM_geCommandErase(event);
            break;

        case GE_PGM_STATE_PROGRAM_DATA: // Sending main FW image
            _GE_PGM_geProgramData(event);
            break;

        default:
            break;
    }
}


//#################################################################################################
// Handles events in GE_PGM_STATE_RESET - set boot select pins and de-assert the reset
//
// Parameters:
//      event                   - Ge Programming Event
// Return:
// Assumptions:
//  *   GE is already in reset
//#################################################################################################
static void _GE_PGM_geResetHandler(enum GeProgramEvent event)
{
    switch (event)
    {
        case GE_PGM_EVENT_RESET_GE: // Trigger GE into reset
            bb_top_SetGEToResetMode();
            break;
        case GE_PGM_EVENT_RX_NAK: // Received NAK - used to signal waiting (ex: waiting for image)
        case GE_PGM_EVENT_RX_ACK: // Received ACK - signals Xmodem packet transfer was successful
            break;
        case GE_PGM_EVENT_RX_BOOTLOADER_RUN_SEQ: // Bootloader welcome sequence received
            break;
        case GE_PGM_EVENT_SENT_EOT: // Last block sent, including EOT
        case GE_PGM_EVENT_RX_FLSHWTR_RUN_SEQ: // Flashwriter welcome sequence received
            break;
        case GE_PGM_EVENT_ENTER_BOOTLOADER_MODE: // Event to set the boot pins to program and de-assert reset
            _GE_PGM_sendGeFlashWriter(false, true); // reset blk number - no transfer when reset set
            bb_top_SetGEToBootloaderMode();
            gePgm.gePgmFsmState = GE_PGM_STATE_WAIT_FOR_BOOTLOADER_RUN;
            break;
        case GE_PGM_EVENT_CMD_RESP_ACCEPTED: // Flashwriter accepted program start - move to next state
        case GE_PGM_EVENT_CMD_RESP_FAILED: // Flashwriter rejected program start - reset?
            break;
        default:
            break;
    }
}


//#################################################################################################
// Handles events in GE_PGM_STATE_WAIT_FOR_FLSHWTR_NAK
//
// Parameters:
//      event                   - PHY event
// Return:
// Assumptions:
//
//#################################################################################################
static void _GE_PGM_geWaitForBootloaderRunHandler(enum GeProgramEvent event)
{
    switch (event)
    {
        case GE_PGM_EVENT_RESET_GE: // Trigger GE into reset
            bb_top_SetGEToResetMode();
            gePgm.gePgmFsmState = GE_PGM_STATE_RESET;
            break;
        case GE_PGM_EVENT_RX_NAK: // Received NAK - used to signal waiting (ex: waiting for image)
        case GE_PGM_EVENT_RX_ACK: // Received ACK - signals Xmodem packet transfer was successful
            break;
        case GE_PGM_EVENT_RX_BOOTLOADER_RUN_SEQ: // Bootloader welcome sequence received
            _GE_PGM_sendGeFlashWriter(false, true); // reset blk number - no transfer when reset set
            UART_clearGeTx();
            gePgm.gePgmFsmState = GE_PGM_STATE_BOOTLOADER_RUNNING;
            break;
        case GE_PGM_EVENT_SENT_EOT: // Last block sent, including EOT
        case GE_PGM_EVENT_RX_FLSHWTR_RUN_SEQ: // Flashwriter welcome sequence received
        case GE_PGM_EVENT_ENTER_BOOTLOADER_MODE: // Event to set the boot pins to program and de-assert reset
        case GE_PGM_EVENT_CMD_RESP_ACCEPTED: // Flashwriter accepted program start - move to next state
        case GE_PGM_EVENT_CMD_RESP_FAILED: // Flashwriter rejected program start - reset?
            break;
        default:
            break;
    }
}


//#################################################################################################
// Handles events in GE_PGM_STATE_BOOTLOADER_RUNNING
//
// Parameters:
//      event                   - Ge Programming event
// Return:
// Assumptions:
//
//#################################################################################################
static void _GE_PGM_geBootloaderRunningHandler(enum GeProgramEvent event)
{
    switch (event)
    {
        case GE_PGM_EVENT_RESET_GE: // Trigger GE into reset
            bb_top_SetGEToResetMode();
            gePgm.gePgmFsmState = GE_PGM_STATE_RESET;
            break;
        case GE_PGM_EVENT_RX_NAK: // Received NAK - used to signal waiting (ex: waiting for image)
            // NAKs in this state mean we're ready to start transferring flash writer
            gePgm.gePgmFsmState = GE_PGM_STATE_TRANSFERRING_FLSHWTR;
            _GE_PGM_sendGeFlashWriter(false, false); // transfer first packet
            break;
        case GE_PGM_EVENT_RX_ACK: // Received ACK - signals Xmodem packet transfer was successful
        case GE_PGM_EVENT_RX_BOOTLOADER_RUN_SEQ: // Bootloader welcome sequence received
        case GE_PGM_EVENT_SENT_EOT: // Last block sent, including EOT
        case GE_PGM_EVENT_RX_FLSHWTR_RUN_SEQ: // Flashwriter welcome sequence received
        case GE_PGM_EVENT_ENTER_BOOTLOADER_MODE: // Event to set the boot pins to program and de-assert reset
        case GE_PGM_EVENT_CMD_RESP_ACCEPTED: // Flashwriter accepted program start - move to next state
        case GE_PGM_EVENT_CMD_RESP_FAILED: // Flashwriter rejected program start - reset?
            break;
        default:
            break;
    }
}


//#################################################################################################
// Handles events in GE_PGM_STATE_TRANSFERRING_FLSHWTR
//
// Parameters:
//      event                   - PHY event
// Return:
// Assumptions:
//
//#################################################################################################
static void _GE_PGM_geTransferringFlshWtrHandler(enum GeProgramEvent event)
{
    switch (event)
    {
        case GE_PGM_EVENT_RESET_GE: // Trigger GE into reset
            bb_top_SetGEToResetMode();
            gePgm.gePgmFsmState = GE_PGM_STATE_RESET;
            break;
        case GE_PGM_EVENT_RX_NAK: // Received NAK - used to signal waiting (ex: waiting for image)
            // Packet wasn't accepted by Xmodem - retry
            ilog_GE_PROGRAM_COMPONENT_0(ILOG_USER_LOG, GE_PGM_NAK);
            _GE_PGM_sendGeFlashWriter(true, false); // resend packet
            break;
        case GE_PGM_EVENT_RX_ACK: // Received ACK - signals Xmodem packet transfer was successful
            // Packet was accepted by Xmodem - send another
            _GE_PGM_sendGeFlashWriter(false, false); // transfer next packet
            // if last packet was successfully transferred, including EOT, we should see the
            // FlshWtr boot sequence come in as an event
            break;
        case GE_PGM_EVENT_RX_BOOTLOADER_RUN_SEQ: // Bootloader welcome sequence received
            _GE_PGM_sendGeFlashWriter(false, true); // reset blk number - no transfer when reset set
            UART_clearGeTx();
            gePgm.gePgmFsmState = GE_PGM_STATE_BOOTLOADER_RUNNING;
            break;
        case GE_PGM_EVENT_SENT_EOT: // Last block sent, including EOT
            UART_packetizeEnableGE(true);
            gePgm.gePgmFsmState = GE_PGM_STATE_WAIT_FOR_FLSHWTR_RUN;
            break;
        case GE_PGM_EVENT_RX_FLSHWTR_RUN_SEQ: // Flashwriter welcome sequence received
        case GE_PGM_EVENT_ENTER_BOOTLOADER_MODE: // Event to set the boot pins to program and de-assert reset
        case GE_PGM_EVENT_CMD_RESP_ACCEPTED: // Flashwriter accepted program start - move to next state
        case GE_PGM_EVENT_CMD_RESP_FAILED: // Flashwriter rejected program start - reset?
            break;
        default:
            break;
    }
}


//#################################################################################################
// Handles events in GE_PGM_STATE_WAIT_FOR_FLSHWTR_RUN
//
// Parameters:
//      event                   - Ge Programming event
// Return:
// Assumptions:
//
//#################################################################################################
static void _GE_PGM_geWaitForFlshWtrRunHandler(enum GeProgramEvent event)
{
    switch (event)
    {
        case GE_PGM_EVENT_RESET_GE: // Trigger GE into reset
            bb_top_SetGEToResetMode();
            gePgm.gePgmFsmState = GE_PGM_STATE_RESET;
            break;
        case GE_PGM_EVENT_RX_NAK: // Received NAK - used to signal waiting (ex: waiting for image)
        case GE_PGM_EVENT_RX_ACK: // Received ACK - signals Xmodem packet transfer was successful
            break;
        case GE_PGM_EVENT_RX_BOOTLOADER_RUN_SEQ: // Bootloader welcome sequence received
            _GE_PGM_sendGeFlashWriter(false, true); // reset blk number - no transfer when reset set
            UART_clearGeTx();
            gePgm.gePgmFsmState = GE_PGM_STATE_BOOTLOADER_RUNNING;
            break;
        case GE_PGM_EVENT_SENT_EOT: // Last block sent, including EOT
            break;
        case GE_PGM_EVENT_RX_FLSHWTR_RUN_SEQ: // Flashwriter welcome sequence received
            gePgm.gePgmFsmState = GE_PGM_STATE_FLSHWTR_RUNNING;
            break;
        case GE_PGM_EVENT_ENTER_BOOTLOADER_MODE: // Event to set the boot pins to program and de-assert reset
        case GE_PGM_EVENT_CMD_RESP_ACCEPTED: // Flashwriter accepted program start - move to next state
        case GE_PGM_EVENT_CMD_RESP_FAILED: // Flashwriter rejected program start - reset?
            break;
        default:
            break;
    }
}


//#################################################################################################
// Handles events in GE_PGM_STATE_FLSHWTR_RUNNING
//
// Parameters:
//      event                   - Ge Programming event
// Return:
// Assumptions:
//
//#################################################################################################
static void _GE_PGM_geFlshWtrRunningHandler(enum GeProgramEvent event)
{
    switch (event)
    {
        case GE_PGM_EVENT_RESET_GE: // Trigger GE into reset
            bb_top_SetGEToResetMode();
            gePgm.gePgmFsmState = GE_PGM_STATE_RESET;
            break;
        case GE_PGM_EVENT_RX_NAK: // Received NAK - used to signal waiting (ex: waiting for image)
        case GE_PGM_EVENT_RX_ACK: // Received ACK - signals Xmodem packet transfer was successful
            break;
        case GE_PGM_EVENT_RX_BOOTLOADER_RUN_SEQ: // Bootloader welcome sequence received
        case GE_PGM_EVENT_SENT_EOT: // Last block sent, including EOT
        case GE_PGM_EVENT_RX_FLSHWTR_RUN_SEQ: // Flashwriter welcome sequence received
            // Flashwriter will continuously send this message until we send commands
            _GE_PGM_commandSendProgramStart();
            gePgm.gePgmFsmState = GE_PGM_STATE_CMD_PROGRAM_START;
            break;
        case GE_PGM_EVENT_ENTER_BOOTLOADER_MODE: // Event to set the boot pins to program and de-assert reset
        case GE_PGM_EVENT_CMD_RESP_ACCEPTED: // Flashwriter accepted program start - move to next state
        case GE_PGM_EVENT_CMD_RESP_FAILED: // Flashwriter rejected program start - reset?
            break;
        default:
            break;
    }
}


//#################################################################################################
// Handles events in CMD_PROGRAM_START
//
// Parameters:
//      event                   - GeProgramming event
// Return:
// Assumptions:
//
//#################################################################################################
static void _GE_PGM_geCommandProgramStart(enum GeProgramEvent event)
{
    switch (event)
    {
        case GE_PGM_EVENT_RESET_GE: // Trigger GE into reset
            bb_top_SetGEToResetMode();
            gePgm.gePgmFsmState = GE_PGM_STATE_RESET;
            break;
        case GE_PGM_EVENT_RX_NAK: // Received NAK - used to signal waiting (ex: waiting for image)
        case GE_PGM_EVENT_RX_ACK: // Received ACK - signals Xmodem packet transfer was successful
            break;
        case GE_PGM_EVENT_RX_BOOTLOADER_RUN_SEQ: // Bootloader welcome sequence received
        case GE_PGM_EVENT_SENT_EOT: // Last block sent, including EOT
        case GE_PGM_EVENT_RX_FLSHWTR_RUN_SEQ: // Flashwriter welcome sequence received
        case GE_PGM_EVENT_ENTER_BOOTLOADER_MODE: // Event to set the boot pins to program and de-assert reset
        case GE_PGM_EVENT_CMD_RESP_ACCEPTED: // Flashwriter accepted program start - move to next state
            _GE_PGM_commandSendErase();
            // Notify external (hobbes or customer code)
            // Use math to print out counting up blocks in status
            // We use decrementing blocks in logic
            // Store this here, after acceptance of StartProgram CMD response
            // The StartPgmResp contains the total blocks to erease
            totalBlocks = gePgm.blocksToEraseOrProgram;
            CMD_sendEraseBlockStatus(
                (totalBlocks - gePgm.blocksToEraseOrProgram + 1),
                PROGRAM_REGION_FLASHWRITER,
                totalBlocks,
                INFO_MESSAGE_ID_STATUS_MODULE0);
            gePgm.gePgmFsmState = GE_PGM_STATE_CMD_ERASE;
            break;
        case GE_PGM_EVENT_CMD_RESP_FAILED: // Flashwriter rejected program start - reset?
            ilog_GE_PROGRAM_COMPONENT_0(ILOG_MAJOR_ERROR, GE_PGM_RESP_FAIL);
            break;
        default:
            break;
    }
}


//#################################################################################################
// Handles events in CMD_ERASE
//
// Parameters:
//      event                   - GeProgramming event
// Return:
// Assumptions:
//
//#################################################################################################
static void _GE_PGM_geCommandErase(enum GeProgramEvent event)
{
    switch (event)
    {
        case GE_PGM_EVENT_RESET_GE: // Trigger GE into reset
            bb_top_SetGEToResetMode();
            gePgm.gePgmFsmState = GE_PGM_STATE_RESET;
            break;
        case GE_PGM_EVENT_RX_NAK: // Received NAK - used to signal waiting (ex: waiting for image)
        case GE_PGM_EVENT_RX_ACK: // Received ACK - signals Xmodem packet transfer was successful
            break;
        case GE_PGM_EVENT_RX_BOOTLOADER_RUN_SEQ: // Bootloader welcome sequence received
        case GE_PGM_EVENT_SENT_EOT: // Last block sent, including EOT
        case GE_PGM_EVENT_RX_FLSHWTR_RUN_SEQ: // Flashwriter welcome sequence received
        case GE_PGM_EVENT_ENTER_BOOTLOADER_MODE: // Event to set the boot pins to program and de-assert reset
        case GE_PGM_EVENT_CMD_RESP_ACCEPTED: // Flashwriter accepted program start - move to next state
            // Decrement our eraseBlocks copy
            if(gePgm.blocksToEraseOrProgram > 0)
            {
                _GE_PGM_commandSendErase();

                // Notify external (hobbes or customer code)
                // Use math to print out counting up blocks in status
                // We use decrementing blocks in logic
                CMD_sendEraseBlockStatus(
                    (totalBlocks - gePgm.blocksToEraseOrProgram + 1),
                    PROGRAM_REGION_FLASHWRITER,
                    totalBlocks,
                    INFO_MESSAGE_ID_STATUS_MODULE0);

            }
            else  // Start sending program data
            {
                // Record total blocks to write before sending first programData
                totalBlocks = (flash_bin_table_ptr[FLASH_BIN_TABLE_GE_FW_SIZE] >> 16);
                if ((flash_bin_table_ptr[FLASH_BIN_TABLE_GE_FW_SIZE] & 0xFFFF) > 0)
                {
                    totalBlocks++; // handle fraction.
                }
                gePgm.blocksToEraseOrProgram = 1;
                _GE_PGM_commandSendProgramData(); // send first block of firmware
                gePgm.gePgmFsmState = GE_PGM_STATE_PROGRAM_DATA;
//                UART_InterruptHandlerTxGe();
            }
            break;
        case GE_PGM_EVENT_CMD_RESP_FAILED: // Flashwriter rejected program start - reset?
            ilog_GE_PROGRAM_COMPONENT_1(ILOG_MAJOR_ERROR, GE_PGM_ERASE_BLOCK_FAIL, gePgm.blocksToEraseOrProgram);
            break;
        default:
            break;
    }
}


//#################################################################################################
// Handles events in PROGRAM_DATA
//
// Parameters:
//      event                   - GeProgramming event
// Return:
// Assumptions:
//
//#################################################################################################
static void _GE_PGM_geProgramData(enum GeProgramEvent event)
{
    switch (event)
    {
        case GE_PGM_EVENT_RESET_GE: // Trigger GE into reset
            bb_top_SetGEToResetMode();
            gePgm.gePgmFsmState = GE_PGM_STATE_RESET;
            break;
        case GE_PGM_EVENT_RX_NAK: // Received NAK - used to signal waiting (ex: waiting for image)
        case GE_PGM_EVENT_RX_ACK: // Received ACK - signals Xmodem packet transfer was successful
            break;
        case GE_PGM_EVENT_RX_BOOTLOADER_RUN_SEQ: // Bootloader welcome sequence received
        case GE_PGM_EVENT_SENT_EOT: // Last block sent, including EOT
            break;
        case GE_PGM_EVENT_RX_FLSHWTR_RUN_SEQ: // Flashwriter welcome sequence received
        case GE_PGM_EVENT_ENTER_BOOTLOADER_MODE: // Event to set the boot pins to program and de-assert reset
        case GE_PGM_EVENT_CMD_RESP_ACCEPTED: // Flashwriter accepted program start - move to next state
            _GE_PGM_commandSendProgramData(); // send next block of firmware
            break;
        case GE_PGM_EVENT_CMD_RESP_FAILED: // Flashwriter rejected program start - reset?
            _GE_PGM_commandSendProgramDataNAK();
            break;
        default:
            break;
    }
}


//#################################################################################################
// Handles the command response to the commands we send
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _GE_PGM_commandResponseHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseID)
{
    CommandResponse pgmResp = {.commandAccepted = 0, .blocksToErase = 0};

    memcpy(&pgmResp, (uint32_t*)data, sizeof(CommandResponse));

    uint8_t resp;
    // Analyze command response - first byte
    if (pgmResp.commandAccepted > 0)
    {
        resp = GE_PGM_EVENT_CMD_RESP_ACCEPTED;
        gePgm.blocksToEraseOrProgram = pgmResp.blocksToErase;
    }
    else
    {
        resp = GE_PGM_EVENT_CMD_RESP_FAILED;
        // TODO ilog? reset or state machine? signal COMMAND we failed?
    }
    _GE_PGM_geStateMachine(resp);
}


//#################################################################################################
// Handles the command response to the commands we send
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _GE_PGM_programDataResponseHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseID)
{
    ProgramDataResponse pgmResp = {.commandAccepted = 0};
    memcpy(&pgmResp, (uint32_t*)data, sizeof(ProgramDataResponse));

    uint8_t resp = GE_PGM_EVENT_CMD_RESP_FAILED;
    // Analyze command response - first byte
    if (pgmResp.commandAccepted > 0)
    {
        resp = GE_PGM_EVENT_CMD_RESP_ACCEPTED;
    }
    else
    {
        resp = GE_PGM_EVENT_CMD_RESP_FAILED;
        // TODO ilog? reset or state machine? signal COMMAND we failed?
    }
    _GE_PGM_geStateMachine(resp);
}


//#################################################################################################
// Handles the command response to the commands we send
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _GE_PGM_commandStatusHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseID)
{

    if (size == sizeof(struct GeSoftwareVersion))
    {
        struct GeSoftwareVersion geSwVer;
        memcpy(&geSwVer, data, sizeof(struct GeSoftwareVersion));
        if ((geSwVer.hdr.msgId == INFO_MESSAGE_ID_VERSION) &&
            (geSwVer.hdr.secMsgId == INFO_MESSAGE_SECONDARY_ID_VERSION_FIRMWARE))
        {
            _GE_PGM_geStateMachine(GE_PGM_EVENT_RX_FLSHWTR_RUN_SEQ);
        }
    }
    else
    {
        ilog_GE_PROGRAM_COMPONENT_2(ILOG_MINOR_ERROR, GE_PGM_SIZE_DIFF, size, sizeof(struct GeSoftwareVersion));
    }
}


//#################################################################################################
// Send program start command
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _GE_PGM_commandSendProgramStart(void)
{
    gePgm.bytesToWrite = flash_bin_table_ptr[FLASH_BIN_TABLE_GE_FW_SIZE];
    gePgm.srcAddr = flash_bin_table_ptr[FLASH_BIN_TABLE_GE_FW_START];

    // GE ASIC so we erase the default that GE's FlashWriter knows to erase
    // This may need to change if we every use a GE FPGA, as the FPGA's regions will be need to be
    // upgradable as well
    ProgramStartCommand pgmStartCmd = {.command = 1, .subcommand = 1, .imageSize = gePgm.bytesToWrite};
    pgmStartCmd.command = 1;
    pgmStartCmd.subcommand = 1;
    pgmStartCmd.imageSize = gePgm.bytesToWrite;
    pgmStartCmd.imageCrc = crcFast((uint8_t*)gePgm.srcAddr, gePgm.bytesToWrite);

    UART_packetizeSendDataImmediate(
        UART_PORT_GE,
        CLIENT_ID_GE_COMMANDS,
        NULL,
        &pgmStartCmd,
        sizeof(ProgramStartCommand));
}


//#################################################################################################
// Send erase command
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _GE_PGM_commandSendErase(void)
{
    // GE ASIC so we erase the default that GE's FlashWriter knows to erase
    // This may need to change if we every use a GE FPGA, as the FPGA's regions will be need to be
    // upgradable as well
    ProgramEraseRegionCommand pgmEraseCmd = {.command = 1, .subcommand = 2};

    UART_packetizeSendDataImmediate(
        UART_PORT_GE,
        CLIENT_ID_GE_COMMANDS,
        NULL,
        &pgmEraseCmd,
        sizeof(ProgramEraseRegionCommand));
}


//#################################################################################################
// Send program data
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _GE_PGM_commandSendProgramDataNAK(void)
{
    CommandResponseAck cmdResp = { .commandAccepted = 0};
    CMD_commandSendResponseAck(&cmdResp);
}


//#################################################################################################
// Send program data
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _GE_PGM_commandSendProgramData(void)
{
    // 0 for 256 bytes to be transferred
    uint16_t maxSize = 256;
    uint16_t payloadSize = (gePgm.bytesToWrite > maxSize) ? maxSize : gePgm.bytesToWrite;

    if (gePgm.bytesToWrite == 0)    // GE program finished ?
    {
        ilog_GE_PROGRAM_COMPONENT_0(ILOG_MAJOR_EVENT, GE_PROGRAM_COMPLETED);
        UART_WaitForTx();

        bb_top_systemReset();
        return;
    }
    // clear payload - set to erased flash all F's
    memset(geFwPayload, 0, sizeof(geFwPayload));

    // read payload from flash
    memcpy(geFwPayload, (uint32_t*)(gePgm.srcAddr), payloadSize);

    // send packet
    UART_packetizeSendDataImmediate(
        UART_PORT_GE,
        CLIENT_ID_GE_PROGRAM_DATA,
        NULL,
        geFwPayload,
        payloadSize);

    if ((gePgm.srcAddr == (flash_bin_table_ptr[FLASH_BIN_TABLE_GE_FW_START] + 0x10000)) ||
        (gePgm.bytesToWrite == flash_bin_table_ptr[FLASH_BIN_TABLE_GE_FW_SIZE])) // 64kB blocks
    {
        // Notify external (hobbes or customer code)
        CMD_sendProgramBlockStatus(
            gePgm.blocksToEraseOrProgram,
            PROGRAM_REGION_FLASHWRITER,
            totalBlocks,
            INFO_MESSAGE_ID_STATUS_MODULE0);
        gePgm.blocksToEraseOrProgram++;
    }

    gePgm.bytesToWrite -= payloadSize;
    gePgm.srcAddr += payloadSize;
}


//#################################################################################################
// perform Xmodem transfers over UART of GE's flashwriter
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _GE_PGM_sendGeFlashWriter(bool repeat, bool reset)
{
    static uint8_t blkNum = 0;
    uint32_t blkCount = flash_bin_table_ptr[FLASH_BIN_TABLE_GE_FLSHWTR_SIZE] / 128 + 1;

    // handle padding case - always set unused bytes in pkt to 0xFF
    memset(geFlshWtrBuff, 0xFF, 128);

    if (blkNum > 0 && repeat)
    {
        blkNum--;
    }
    if (reset)
    {
        UART_clearGeTx();
        blkNum = 0;
    }


    uint32_t* addr = (uint32_t*)(blkNum * 128 + flash_bin_table_ptr[FLASH_BIN_TABLE_GE_FLSHWTR_START]);
    ilog_GE_PROGRAM_COMPONENT_1(ILOG_DEBUG, GE_PGM_ADDR, (uint32_t)addr);

    memcpy(geFlshWtrBuff, addr, 128);
    if ((blkNum < blkCount) && !reset)
    {
        // add 1 because protocol blk num begins at 1, but we need blk num for address offset
        // so we count from zero but add 1 for the protocol to be happy
        _GE_PGM_xmodemSend(blkNum + 1, geFlshWtrBuff);
        ilog_GE_PROGRAM_COMPONENT_2(ILOG_DEBUG, GE_PGM_BLK, blkNum, blkCount - 1);
        blkNum++;
    }
    else if (blkNum == blkCount) // last blk of FlshWtr sent, now send EOT
    {
        uint8_t eot = 4;
        UART_ByteTxByCh(UART_PORT_GE, eot);
        _GE_PROGRAM_geSentEOT();
        blkNum++; // ensure only one EOT sent
    }
    else
    {
        if (!reset)
        {
            ilog_GE_PROGRAM_COMPONENT_0(ILOG_MINOR_EVENT, GE_PGM_PKT_END);
        }
    }
    // read 128B from flash
    // wrap it in xmodem packet
    // send out UART
    // repeat - padd on last packet
}


//#################################################################################################
// Send pkt to UART one byte at a time
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _GE_PGM_xmodemSend(uint8_t blkNum, uint8_t* pkt)
{
    uint16_t bytesSent = 0;
    uint8_t pktWrapper[4];
    uint8_t chkSum = 0;
    uint8_t i = 0;

    for (i = 0; i < 128; i++)
    {
        chkSum += pkt[i];
    }

    pktWrapper[0] = 1; // SOH
    pktWrapper[1] = blkNum; // packet number
    pktWrapper[2] = (255 - blkNum);
    pktWrapper[3] = chkSum;

    for (bytesSent = 0; bytesSent < 132; bytesSent++)
    {
        if (bytesSent < 3) // SOH, PKTNUM, INVPKTNUM
        {
            UART_ByteTxByCh(UART_PORT_GE, pktWrapper[bytesSent]);
        }
        else if (bytesSent == 131) // CHKSUM
        {
            UART_ByteTxByCh(UART_PORT_GE, pktWrapper[3]);
        }
        else // PAYLOAD - between header and chksum
        {
            UART_ByteTxByCh(UART_PORT_GE, pkt[bytesSent-3]);
        }
    }
}


