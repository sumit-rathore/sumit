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

ILOG_CREATE(STORAGE_COMPONENT)
    ILOG_ENTRY(STORAGE_VAR_TOO_BIG, "Storage variable %d is too big at %d bytes to be read\n")
    ILOG_ENTRY(FLASH_ACCESS_INVALID_VARIABLE, "Tried to access flash variable %d, but there are only %d variables\n")
    ILOG_ENTRY(STORAGE_GET_VAR_NOT_EXIST, "Tried to get a persistent variable %d that does not exist\n")
    ILOG_ENTRY(STORAGE_CREATE_VAR_ALREADY_EXISTS, "Tried to create a persistent variable %d that already exists\n")
    ILOG_ENTRY(FLASH_REMOVE_VAR_NOT_EXISTS, "Tried to remove a persistent variable %d that doesn't exist\n")
    ILOG_ENTRY(STORAGE_ICMD_READ_VAR, "Variable %d has MSW=0x%x and LSW=0x%x.\n")
    ILOG_ENTRY(STORAGE_ICMD_WRITE_VAR, "Wrote variable %d with MSW=0x%x and LSW=0x%x.\n")
    ILOG_ENTRY(STORAGE_ICMD_DUMP_VARS, "Dumping all persistent variables:\n")
    ILOG_ENTRY(STORAGE_SAVING_VAR, "Saving persistent var %d\n")
    ILOG_ENTRY(STORAGE_ASSERT_HOOK_TITLE,         "Persistent data assert hook:\n")
    ILOG_ENTRY(STORAGE_ASSERT_HOOK_PENDING_WRITE, "  Pending write to variable %d\n")
    ILOG_ENTRY(STORAGE_INVALID_STORAGE_BACKEND, "The storage backend value of %d is invalid.\n")
    ILOG_ENTRY(READ_VAR_FAIL, "Unable to read var %d\n")
    ILOG_ENTRY(WROTE_VAR_FAIL, "Unable to write var %d\n")
    ILOG_ENTRY(CANT_READ_VAR, "Tried to read variable %d, but it doesn't exist\n")
    ILOG_ENTRY(ERASE_VAR, "Erasing variable %d\n")
    ILOG_ENTRY(ERASE_VAR_FOR_VAR_THAT_DOESNT_EXIST, "Trying to erase variable %d, but it doesn't exist in storage\n")
    ILOG_ENTRY(DEPRECATED_EEPROM_READ_FAIL, "Could not read eeprom page %d\n")
    ILOG_ENTRY(DEPRECATED_EEPROM_WRITE_FAIL, "Could not write to I2C EEPROM chip\n")
    ILOG_ENTRY(EEPROM_PAGE_READ_FAILED, "EEPROM read failed while reading page %d\n")
    ILOG_ENTRY(EEPROM_INVALID_READ_SIZE, "EEPROM page read size %d is invalid while reading page %d\n")
    ILOG_ENTRY(EEPROM_WRITE_BACKUP_FAILED, "Failed to write to EEPROM backup page for variable %d\n")
    ILOG_ENTRY(EEPROM_WRITE_PRIMARY_FAILED, "Failed to write to EEPROM primary page for variable %d\n")
    ILOG_ENTRY(EEPROM_PRIMARY_PAGE_DATA_CORRUPTED, "Primary location for variable %d is corrupted, but backup is intact. Restoring from backup.\n")
    ILOG_ENTRY(EEPROM_PRIMARY_PAGE_BAD_CRC, "Primary location has a bad CRC, and backup is for a different variable. Probable corruption of variable %d.\n")
    ILOG_ENTRY(EEPROM_BACKUP_PAGE_BAD_CRC, "Primary and backup both failed CRC check for variable %d. Assuming primary is valid.\n")
    ILOG_ENTRY(EEPROM_RESTORE_BACKUP_FAILED, "Write to EEPROM backup page %d failed for restoring primary page %d\n")
    ILOG_ENTRY(EEPROM_RESTORE_PRRIMARY_FAILED, "Restore EEPROM primary page %d failed\n")
ILOG_END(STORAGE_COMPONENT, ILOG_DEBUG)

#endif // #ifndef FLASH_DATA_LOG_H

