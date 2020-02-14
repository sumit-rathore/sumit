///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011, 2012
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
//!   @file  -  flash_data_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef FLASH_DATA_LOG_H
#define FLASH_DATA_LOG_H

/***************************** Included Headers ******************************/
#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(FLASH_DATA_COMPONENT)
    ILOG_ENTRY(FLASH_INIT, "Flash driver initialization\n")
    ILOG_ENTRY(MULTIPLE_ACTIVE_SECTIONS, "Multiple sections of flash were marked active\n")
    ILOG_ENTRY(INIT_SECTION1_MARKED_AS_ACTIVE, "Initialization: Marking section1 as the active section\n")
    ILOG_ENTRY(INIT_FOUND_GARBAGE_HEADER, "Initialization found a garbage flash header in section %d\n")
    ILOG_ENTRY(UNEXPECTED_HEADER_WHEN_SETTING, "Unexpectedly read header %d, when going to write %d\n")
    ILOG_ENTRY(WRITE_VAR_SIZE_TOO_BIG, "Error Writing variable %d.  Size %d is too big\n")
    ILOG_ENTRY(WRITE_VAR, "Writing variable %d, with size %d\n")
    ILOG_ENTRY(WRITE_VAR_IS_REPLACEMENT, "Wrote variable %d, & now erasing previous setting\n")
    ILOG_ENTRY(ERASE_VAR, "Erasing var %d\n")
    ILOG_ENTRY(ERASE_VAR_FOR_VAR_THAT_DOESNT_EXIST, "Trying to erase var %d, but it doesn't exist in flash\n")
    ILOG_ENTRY(READ_VAR, "Read variable %d\n")
    ILOG_ENTRY(CANT_READ_VAR, "Tried to read variable %d, but it doesn't exist\n")
    ILOG_ENTRY(UNABLE_TO_FIND_ACTIVE_DATA_SECTION, "Unable to find active data section\n")
    ILOG_ENTRY(FLASH_VAR_TOO_BIG, "Flash variable %d is too big at %d bytes to be read\n")
ILOG_END(FLASH_DATA_COMPONENT, ILOG_DEBUG)

#endif // #ifndef FLASH_DATA_LOG_H

