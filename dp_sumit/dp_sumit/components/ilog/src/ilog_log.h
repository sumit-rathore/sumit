///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009, 2010
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
//!   @file  -  ilog_log.h
//
//!   @brief -  The ilog logs
//
//
//!   @note  -  Add new logs with extreme care.  We don't want recursive logging
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ILOG_LOG_H
#define ILOG_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(ILOG_COMPONENT)
    ILOG_ENTRY(INVALID_COMPONENT, "ILOG Received an invalid component %d from line %d\n")
    ILOG_ENTRY(INVALID_LEVEL, "ILOG Received an invalid logging level %d from line %d\n")
    ILOG_ENTRY(TOO_MANY_ARGS, "ILOG Received too many args, %d to be exact\n")
    ILOG_ENTRY(GET_LEVEL, "Component %d is at logging level %d\n")
    ILOG_ENTRY(COPROC_REG_DUMP, "Sparc reg dump:           TBR = 0x%x, PSR = 0x%x, WIM = 0x%x\n")
    ILOG_ENTRY(GLOBAL_REG_DUMP, "Sparc reg dump:           %%g5 = 0x%x, %%g6 = 0x%x, %%g7 = 0x%x\n")
    ILOG_ENTRY(MISC_REG_DUMP,   "Sparc reg dump:           current window %d\n\n")
    ILOG_ENTRY(SEPERATOR_LOG,   "\n\n---------- Active windows follow ----------\n\n")

    ILOG_ENTRY(GEN_REG_DUMP0,   "Sparc reg dump for win %2d:%%l0 = 0x%x, %%l1 = 0x%x\n")
    ILOG_ENTRY(GEN_REG_DUMP1,   "Sparc reg dump:           %%l2 = 0x%x, %%l3 = 0x%x, %%l4 = 0x%x\n")
    ILOG_ENTRY(GEN_REG_DUMP2,   "Sparc reg dump:           %%l5 = 0x%x, %%l6 = 0x%x, %%l7 = 0x%x\n")
    ILOG_ENTRY(GEN_REG_DUMP3,   "Sparc reg dump for win %2d:%%i0 = 0x%x, %%i1 = 0x%x\n")
    ILOG_ENTRY(GEN_REG_DUMP4,   "Sparc reg dump:           %%i2 = 0x%x, %%i3 = 0x%x, %%i4 = 0x%x\n")
    ILOG_ENTRY(GEN_REG_DUMP5,   "Sparc reg dump:           %%i5 = 0x%x, %%i6 = 0x%x, %%i7 = 0x%x\n")
    ILOG_ENTRY(SET_BLOCKING_MODE, "Setting ilog blocking mode\n")
    ILOG_ENTRY(CLEAR_BLOCKING_MODE, "Clearing ilog blocking mode\n")
    ILOG_ENTRY(ILOG_SET_TIMESTAMP, "Setting timestamp to value %d, offset needed %d\n")
    ILOG_ENTRY(TEST_ASSERT, "Icmd run for testing asserts.  Args are %d, %d, %d\n")
    ILOG_ENTRY(LOG_LEVEL_CHANGED, "Changing logging level for component %d to level %d\n")
    ILOG_ENTRY(INVALID_COMPONENT_ILOG_MAIN, "ILOG main logging function received an invalid component %d\n")
    ILOG_ENTRY(INVALID_LEVEL_ILOG_MAIN, "ILOG main logging function received an invalid logging level %d\n")
    ILOG_ENTRY(ILOG_MSG_DROPPED, "ILOG message dropped\n")

    ILOG_ENTRY(ASSERT_STATUS1, "Assert status: preAssertHookFunction is 0x%x, postAssertHookFunction is 0x%x\n")
    ILOG_ENTRY(ASSERT_STATUS2, "Assert status: no asserts have occured\n")
    ILOG_ENTRY(ASSERT_STATUS3, "Assert status: %d asserts have occured, last assert message was:\n")
ILOG_END(ILOG_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef ILOG_LOG_H

