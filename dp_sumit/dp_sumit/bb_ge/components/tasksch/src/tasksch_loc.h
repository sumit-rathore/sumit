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
//!   @file  -  tasksch_loc.h
//
//!   @brief -  Local header file for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TASKSCH_LOC_H
#define TASKSCH_LOC_H

/***************************** Included Headers ******************************/
#include <tasksch.h>
#include <options.h>
#include "tasksch_log.h"
#include "tasksch_cmd.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
struct _TASKSCH_Task
{
    void (*task)(TASKSCH_TaskT, uint32);    // used if non-null
    uint32 taskArg;
    enum TASKSCH_taskPriority priority;     // currently ignored
    boolT allowInterrupts;                  // currently all tasks run with interrupts disabled
    boolT onRunQueue;
#ifdef GE_PROFILE
    uint64 timeSpan;
#endif
};

struct _TASKSCH_StateStruct
{
    struct _TASKSCH_Task taskArray[TASKSCH_MAX_NUM_OF_TASKS];
    uint32 numOfAllocatedTasks;
};

/**************************** Global Variables *******************************/
extern struct _TASKSCH_StateStruct _TASKSCH_State;


/*********************************** API *************************************/

#endif // TASKSCH_LOC_H

