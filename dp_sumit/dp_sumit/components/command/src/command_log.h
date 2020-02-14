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
#ifndef COMMAND_LOG_H
#define COMMAND_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(COMMAND_COMPONENT)
    ILOG_ENTRY(CMD_PROGRAM_ERASE_BLOCK, "Erasing block %d of %d\n")
    ILOG_ENTRY(CMD_PROGRAM_ERASE_BLOCK_FAILED, "ERROR: Erasing block %d of %d failed\n")
    ILOG_ENTRY(CMD_PROGRAM_WRITE_FAILED, "ERROR: Write failed!\n")
    ILOG_ENTRY(CMD_PROGRAM_WRITE_SUCCESS, "Write success!\n")
    ILOG_ENTRY(CMD_PROGRAM_ADDRESS, ">>>   Programming to address 0%x\n")
    ILOG_ENTRY(CMD_SYSTEM_INVALID, "CMD Rx invalid Cmd: 0x%x, SubCmd: 0x%x\n")
    ILOG_ENTRY(CMD_RUN_PROGRAMBB, "### Rx RunPgmBB ### Start Addr: %x, Size: %d\n")
    ILOG_ENTRY(VERIFY_ERASE_FAILED, "Verify Erase Failed: Erased addr %x has data %x\n")
    ILOG_ENTRY(INVALID_RX_CMD, "CMD_processCommandProgramSubcommand: CMD Rx invalid, Cmd %x, SubCmd %x\n")
    ILOG_ENTRY(CMD_PROGRAM_BB_CALL, "Load and run PROGRAM_BB After 300ms\n")
ILOG_END(COMMAND_COMPONENT, ILOG_MAJOR_EVENT)
#endif // COMMAND_LOG_H
