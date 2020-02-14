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

#ifdef PLATFORM_A7
#include <bb_top.h>
#include <bb_top_a7.h>
#endif

// Constants and Macros ###########################################################################
#define SYSTEM_RESET_DELAY_IN_US   (2*1000*1000)    // 2 seconds

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// Process subcommands for SystemCommand
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void CMD_processCommandSystemSubcommand(const void* data)
{
    CommandResponseAck cmdResp = { .commandAccepted = 1 };
    const uint8_t cmd = *((uint8_t*)data);
    const uint8_t subcmd = *((uint8_t*)(data) + 1);
    // Analyze subcommand - second byte
    switch (subcmd)
    {
        case CMD_SYSTEM_RESET:
            CMD_commandSendResponseAck(&cmdResp);
            UART_WaitForTx();
            // Cobs can't change baud rates fast enough
            // So we wait before resetting, so Cobs will receive our startup logs
            // So we wait 0.2s before actually resetting
            LEON_TimerWaitMicroSec(SYSTEM_RESET_DELAY_IN_US);
            //bb_top_systemReset();
            // We need to know which FPGA was loaded via ICAP so we know the correct region we're
            // in
            // We need to know if ASIC or FPGA so we know what the offset should be if we're not
            // Fallback -- always setting to 0 is slower than rebooting the fpga we're running on
            // That is - try current if we're current rather than jump to Fallback and then jump to
            // current
            // Protect the Golden area:
            bb_top_reloadFpga();
            break;
        case CMD_SYSTEM_DEVICE_INFO:
            CMD_commandSendResponseAck(&cmdResp);
#ifdef BB_PROGRAM_BB
            CMD_sendSoftwareVersion(INFO_MESSAGE_SECONDARY_ID_VERSION_PROGRAM_BB);
            CMD_hardwareInfo(INFO_MESSAGE_SECONDARY_ID_VERSION_HARDWARE);
#else
            CMD_sendSoftwareVersion(INFO_MESSAGE_SECONDARY_ID_VERSION_FIRMWARE);
            CMD_hardwareInfo(INFO_MESSAGE_SECONDARY_ID_VERSION_HARDWARE);
#endif     
            break;
        default:
            ilog_COMMAND_COMPONENT_2(ILOG_MINOR_ERROR, CMD_SYSTEM_INVALID, cmd, subcmd);
            break;
    }
}


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################



