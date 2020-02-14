///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009, 2014
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
//!   @file  -  eeprom_log.h
//
//!   @brief -  The eeprom driver logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef EEPROM_LOG_H
#define EEPROM_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(EEPROM_COMPONENT)
    ILOG_ENTRY(EEPROM_BUSY, "There is already an EEPROM access in progress.\n")
    ILOG_ENTRY(EEPROM_ADDRESS_EXCEEDS_CAPACITY, "An attempt was made to access EEPROM page %d which exceeds capacity %d.\n")
    ILOG_ENTRY(ICMD_EEPROM_READ_FAILED, "Failed reading external eeprom\n")
    ILOG_ENTRY(ICMD_EEPROM_WRITE_FAILED, "Failed writing to external eeprom\n")
    ILOG_ENTRY(ICMD_EEPROM_WRITE_SUCCESSFUL, "Write to EEPROM is complete\n")
    ILOG_ENTRY(ICMD_EEPROM_WRONG_READ_LENGTH, "Expected %d bytes, read %d bytes instead\n")
    ILOG_ENTRY(ICMD_PAGE_VALUES, "Byte %d: Value: 0x%x\n")
    ILOG_ENTRY(ICMD_WORD_VALUES, "Word %d: Value: 0x%x\n")
    ILOG_ENTRY(ICMD_INVALID_PAGE, "Page %d is outside the valid range of 0 to %d\n")
    ILOG_ENTRY(EEPROM_INITIALIZATION_STARTING, "Beginning EEPROM initialization\n")
    ILOG_ENTRY(EEPROM_READ_SUBMIT, "Submitting an EEPROM read of page %d with pageData pointing at 0x%x\n")
    ILOG_ENTRY(EEPROM_WRITE_SUBMIT, "Submitting an EEPROM write of page %d with pageData pointing at 0x%x\n")
ILOG_END(EEPROM_COMPONENT, ILOG_MINOR_EVENT)

#endif // EEPROM_LOG_H
