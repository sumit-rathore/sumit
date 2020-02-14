///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  tasksch_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TASKSCH_LOG_H
#define TASKSCH_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

// Sample:
//ILOG_CREATE(ILOG_COMPONENT)
//    ILOG_ENTRY(INVALID_COMPONENT, "ILOG Received an invalid component\n")
//    ILOG_ENTRY(INVALID_CODE, "ILOG Received an invalid code\n")
//ILOG_END(ILOG_COMPONENT, ILOG_FATAL_ERROR)

ILOG_CREATE(TASKSCH_COMPONENT)
    ILOG_ENTRY(INIT_TASK, "Initializing task 0x%x, with task function 0x%x and with arg 0x%x\n")
    ILOG_ENTRY(INIT_TASK2, "Setting task 0x%x to allowInterrupts %d, and priority %d\n")
    ILOG_ENTRY(TOO_MANY_TASKS, "Too many tasks are being allocated\n")
    ILOG_ENTRY(INVALID_PRIORITY_INIT, "Invalid priority %d in initialization\n")
    ILOG_ENTRY(INVALID_PRIORITY_SETTING, "Invalid priority %d\n")
    ILOG_ENTRY(START, "Starting task 0x%x\n")
    ILOG_ENTRY(STOP, "Stopping task 0x%x\n")
    ILOG_ENTRY(CHANGE_TASK_ARG, "Changing task 0x%x arg to 0x%x\n")
    ILOG_ENTRY(CHANGE_TASK_PRIORITY, "Changing task 0x%x priority to 0x%x\n")
    ILOG_ENTRY(VIEW_TASK1, "Task 0x%x: runs task function 0x%x, with arg 0x%x\n")
    ILOG_ENTRY(VIEW_TASK2, "Task 0x%x: runs at priority %d, with allowInterrupts %d\n")
    ILOG_ENTRY(VIEW_TASK4, "Task 0x%x: is at index %d, & has onRunQueue set to %d\n")
#ifdef GE_PROFILE
    ILOG_ENTRY(VIEW_TASK_TIMESPAN, "Task 0x%x: timespan upper word = %d, lower word = %d, in decimal microseconds\n")
    ILOG_ENTRY(VIEW_TASK_TOTALTIMESPAN, "Total timespan upper word = %d, lower word = %d, in decimal microseconds\n")
#endif
ILOG_END(TASKSCH_COMPONENT, ILOG_WARNING)

#endif // #ifndef TASKSCH_LOG_H


