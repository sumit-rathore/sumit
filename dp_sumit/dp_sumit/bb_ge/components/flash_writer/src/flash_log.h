///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  -  flash_log.h
//
//!   @brief -  This file contains the logging definitions for the flash module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef FLASH_LOG_H
#define FLASH_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>


/************************ Defined Constants and Macros ***********************/

#define ERASE "Erasing serial flash memory...\n"
#define ERASE_COMPLETE "Serial Flash Erase Complete\n"
#define ERASE_CHECK "Checking serial flash memory erase...\n"
#define ERASE_CHECK_FAIL "Erase failed @ 0x%x, read 0x%x\n"
#define ERASE_CHECK_DONE "Done checking erase\n"
#define ANNOUNCE "Flash Writer running\n"
#define FAILED "Xmodem transfer failed\n"
#define PROGRAM_CHECK_ERROR "Error verifying flash, expected value :0x%x, read value: 0x%x, address: 0x%x\n"
#define ERASE_FAILED_ABORT "Erasing flash failed, aborting\n"
#define PROG_COMP_WAIT_FOR_USER_RESET "Flash programing complete.  Waiting for user to reset ...\n"
#define ERASE_CODE_BLOCK "Erasing serial flash code block memory...\n"
#define PROG_COMP_SEND_CR_LF_TO_RESET "Press enter (CR-LF) to reset ...\n"

#endif // #ifndef FLASH_LOG_H

