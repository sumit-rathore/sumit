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
#ifndef GE_PROGRAM_LOG_H
#define GE_PROGRAM_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(GE_PROGRAM_COMPONENT)
    ILOG_ENTRY(GE_PROGRAM_RCV_EVENT, "In state %d, received event %d\n")
    ILOG_ENTRY(GE_PROGRAM_COMPLETED, "Ge programming completed, restart GE\n")
    ILOG_ENTRY(GE_PGM_BLK, "Programming GE blk %d of %d sent\n")
    ILOG_ENTRY(GE_PGM_ADDR, "Programming GE addr 0x%x\n")
    ILOG_ENTRY(GE_PGM_PKT_END, "Programming GE cannot send - reached end\n")
    ILOG_ENTRY(GE_PGM_START, "GE Automatic download start \n")
    ILOG_ENTRY(GE_PGM_RESP_FAIL, "GE ProgramStart rejected\n")
    ILOG_ENTRY(GE_PGM_ERASE_BLOCK_FAIL, "GE Program Erasing block 0x%x failed\n")
    ILOG_ENTRY(GE_PGM_SIZE_DIFF, "GE Program InfoMsg size 0x%x, expected 0x%x\n")
    ILOG_ENTRY(GE_PGM_NAK, "GE Program NAK\n")
ILOG_END(GE_PROGRAM_COMPONENT, ILOG_MINOR_EVENT)
//ILOG_END(GE_PROGRAM_COMPONENT, ILOG_DEBUG)

#endif // GE_PROGRAM_LOG_H
