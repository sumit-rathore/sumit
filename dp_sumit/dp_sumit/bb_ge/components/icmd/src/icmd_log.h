///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -  icmd_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ICMD_LOG_H
#define ICMD_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(ICMD_COMPONENT)
    ILOG_ENTRY(INVALID_HEADER, "Received 0x%x, when expecting an icmd header\n")
    ILOG_ENTRY(INVALID_COMPONENT, "Received %d, when expecting a component\n")
    ILOG_ENTRY(TOO_MANY_ARGS, "Too many args %d in icmd request\n")
    ILOG_ENTRY(CUROFFSET_CORRUPTED, "curOffset with value %d, is corrupt at line %d\n")
    ILOG_ENTRY(CALLING_HANDLER, "Calling function 0x%x, with 1st two args 0x%x, 0x%x\n")
    ILOG_ENTRY(RECVD_ICMD, "Received icmd for component %d, function %d, with %d arguments\n")
    ILOG_ENTRY(NO_ICMD_FCN_PTR_ARRAY, "No icmd function pointer array found for component %d\n")
    ILOG_ENTRY(ICMD_TIMEOUT, "icmd message timed out. Resetting receive buffer\n")
    ILOG_ENTRY(BASE_READ_MEM, "Read address 0x%x: value 0x%x\n")
    ILOG_ENTRY(BASE_WRITE_MEM, "Wrote address 0x%x: value 0x%x\n")
    ILOG_ENTRY(DEPRECATED_ICMD_TIMER_REGISTER_FAILURE, "Unable to register icmd timer\n")
    ILOG_ENTRY(ICMD_TIMER_NOT_REGISTERED, "ICmd timer is not register, did no one call ICMD_Init()?\n")
    ILOG_ENTRY(BASE_READ_MODIFY_WRITE, "Read-Modify-Write address 0x%x: Read value 0x%x, Write value 0x%x\n")
    ILOG_ENTRY(BASE_READ_MODIFY_WRITE_CONFLICT_MASKS, "Read-Modify-Write for address 0x%x has conflicting bits in set bitfields 0x%x and clear bitfields0x%x\n")
    ILOG_ENTRY(BASE_MODIFY_BITFIELD, "Modify bitfield for address 0x%x starting at position %i for width %i bits\n")
    ILOG_ENTRY(MODIFY_BITFIELD_RESULT,"Write value: 0x%x, Read value: 0x%x, New value 0x%x\n")
    ILOG_ENTRY(DUMP_MEMORY_ADDR_INVALID, "Address 0x%x is not a valid 32bit memory location\n")
    ILOG_ENTRY(DEPRECATED_ICMD, "This iCommand is no longer available\n")
    ILOG_ENTRY(BASE_READ_MEM16, "Read address 0x%x: value 0x%x\n")
    ILOG_ENTRY(BASE_WRITE_MEM16, "Wrote address 0x%x: value 0x%x\n")
    ILOG_ENTRY(BASE_READ_MODIFY_WRITE16, "Read-Modify-Write address 0x%x: Read value 0x%x, Write value 0x%x\n")
    ILOG_ENTRY(BASE_READ_MODIFY_WRITE_CONFLICT_MASKS16, "Read-Modify-Write for address 0x%x has conflicting bits in set bitfields 0x%x and clear bitfields0x%x\n")
    ILOG_ENTRY(JUNK_TIMER_EXPIRED_REENABLING_PROCESSING, "Junk timer expired.  Re-enabling processing of icmds\n")
ILOG_END(ICMD_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef ICMD_LOG_H


