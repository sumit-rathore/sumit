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
// RS232 Command Interface Module - processing received commands for RS232.
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
#include "command_loc.h"
#include "command_log.h"
#include <bb_top.h>
#include <rs232.h>

#ifdef PLATFORM_A7
#include <bb_top.h>
#include <bb_top_a7.h>
#endif

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// Process subcommands for RS232Command
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void CMD_processCommandRs232Subcommand(const void* data)
{
    CommandResponseAck cmdResp = { .commandAccepted = 1 };
    Rs232ControlCommand rs232ControlCommand = {0};
    memcpy(&rs232ControlCommand, data, sizeof (Rs232ControlCommand));

    rs232ControlCommand.command = *((uint8_t*)data);
    rs232ControlCommand.subcommand = *((uint8_t*)(data) + 1);

    // Analyze subcommand - second byte
    switch (rs232ControlCommand.subcommand)
    {
        case CMD_RS232_CONTROL:
            CMD_commandSendResponseAck(&cmdResp);
            UART_WaitForTx();
            RS232_ControlInput(rs232ControlCommand.enable_disable_b);
            break;

        default:
            break;
    }
}


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################



