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
//!   @file  -  tasksch.h
//
//!   @brief -  API for the task scheduler
//
//
//!   @note  -  This is a simple task scheduler with no pre-emption
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TASKSCH_H
#define TASKSCH_H

/***************************** Included Headers ******************************/
#include <itypes.h> // For boolT

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

// WARNING: Make sure to update the special values _TASKSCH_PRIORITY_HIGHEST/LOWEST if a new
// highest or lowest priority is introduced.  Do not add more priority levels than required because
// the complexity of iterating over all of the tasks is numTasks * numPriorities.
enum TASKSCH_taskPriority
{
    _TASKSCH_PRIORITY_HIGHEST,
    TASKSCH_PRIORITY_CRITICAL = _TASKSCH_PRIORITY_HIGHEST,
    TASKSCH_PRIORITY_HIGH,
    TASKSCH_PRIORITY_NORMAL,
    TASKSCH_PRIORITY_LOW,
    _TASKSCH_PRIORITY_LOWEST = TASKSCH_PRIORITY_LOW
};

struct _TASKSCH_Task;
typedef struct _TASKSCH_Task * TASKSCH_TaskT;

/*********************************** API *************************************/

void TASKSCH_Init(void);

// If allowInterrupts is set, this allows possible race conditions between start/stop task and when
// a task is kicked off.  If allowInterrupts is set, it should be expected that the task could
// still be called after it is stopped.
TASKSCH_TaskT TASKSCH_InitTask(
    void (*task)(TASKSCH_TaskT, uint32 taskArg),
    uint32 taskArg,
    boolT allowInterrupts,
    enum TASKSCH_taskPriority);

void TASKSCH_StartTask(TASKSCH_TaskT) __attribute__ ((section(".ftext")));
void TASKSCH_StopTask(TASKSCH_TaskT) __attribute__ ((section(".ftext")));
void TASKSCH_ChangeTaskArg(TASKSCH_TaskT, uint32 newTaskArg) __attribute__ ((section(".ftext")));
uint32 TASKSCH_GetTaskArg(TASKSCH_TaskT) __attribute__ ((section(".ftext")));
void TASKSCH_ChangeTaskPriority(
    TASKSCH_TaskT, enum TASKSCH_taskPriority) __attribute__ ((section(".ftext")));
void TASKSCH_MainLoop(void) __attribute__ ((section(".ftext"), noreturn));
boolT TASKSCH_IsTaskActive(TASKSCH_TaskT) __attribute__ ((section(".ftext")));

#endif // TASKSCH_H

