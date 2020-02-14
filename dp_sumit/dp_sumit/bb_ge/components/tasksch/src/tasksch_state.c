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
//!   @file  -  tasksch_state.c
//
//!   @brief -  Handles all the state changing associated with tasks
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "tasksch_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Global Variables *******************************/
// This variable is defined extern in the local header file
struct _TASKSCH_StateStruct _TASKSCH_State;


/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: TASKSCH_InitTask()
*
* @brief  - Initialize a new task
*
* @return - the new task handler
*/
TASKSCH_TaskT TASKSCH_InitTask
(
    void (*task)(TASKSCH_TaskT, uint32),
    uint32 taskArg,
    boolT allowInterrupts,
    enum TASKSCH_taskPriority priority
)
{
    TASKSCH_TaskT newTask;
    uint32 taskNum = _TASKSCH_State.numOfAllocatedTasks;

    iassert_TASKSCH_COMPONENT_0(taskNum < TASKSCH_MAX_NUM_OF_TASKS, TOO_MANY_TASKS);
    iassert_TASKSCH_COMPONENT_1(
        priority >= _TASKSCH_PRIORITY_HIGHEST && priority <= _TASKSCH_PRIORITY_LOWEST,
        INVALID_PRIORITY_INIT,
        priority);

    _TASKSCH_State.numOfAllocatedTasks++;

    newTask = &_TASKSCH_State.taskArray[taskNum];

    ilog_TASKSCH_COMPONENT_3(ILOG_MINOR_EVENT, INIT_TASK, (uint32)newTask, (uint32)task, taskArg);
    ilog_TASKSCH_COMPONENT_3(
        ILOG_MINOR_EVENT, INIT_TASK2, (uint32)newTask, allowInterrupts, priority);

    newTask->task = task;
    newTask->taskArg = taskArg;
    newTask->allowInterrupts = allowInterrupts;
    newTask->priority = priority;
    newTask->onRunQueue = FALSE;

    return newTask;
}


/**
* FUNCTION NAME: TASKSCH_StartTask()
*
* @brief  - Start a task
*
* @return - void
*/
void TASKSCH_StartTask
(
    TASKSCH_TaskT task
)
{
    ilog_TASKSCH_COMPONENT_1(ILOG_MINOR_EVENT, START, (uint32)task);
    task->onRunQueue = TRUE;
}


/**
* FUNCTION NAME: TASKSCH_StopTask()
*
* @brief  - Stop a task
*
* @return - void
*/
void TASKSCH_StopTask
(
    TASKSCH_TaskT task
)
{
    ilog_TASKSCH_COMPONENT_1(ILOG_MINOR_EVENT, STOP, (uint32)task);
    task->onRunQueue = FALSE;
}


/**
* FUNCTION NAME: TASKSCH_ChangeTaskArg()
*
* @brief  - Change the argument to a task
*
* @return - void
*/
void TASKSCH_ChangeTaskArg
(
    TASKSCH_TaskT task,
    uint32 newTaskArg
)
{
    ilog_TASKSCH_COMPONENT_2(ILOG_MINOR_EVENT, CHANGE_TASK_ARG, (uint32)task, newTaskArg);
    task->taskArg = newTaskArg;
}


/**
* FUNCTION NAME: TASKSCH_GetTaskArg()
*
* @brief  - Retrieve the task argument from the given task.
*
* @return - The task argument.
*/
uint32 TASKSCH_GetTaskArg(TASKSCH_TaskT task)
{
    return task->taskArg;
}


/**
* FUNCTION NAME: TASKSCH_ChangeTaskPriority()
*
* @brief  - Change the priority of a task.
*
* @return - void
*/
void TASKSCH_ChangeTaskPriority
(
    TASKSCH_TaskT task,
    enum TASKSCH_taskPriority priority
)
{
    ilog_TASKSCH_COMPONENT_2(ILOG_MINOR_EVENT, CHANGE_TASK_PRIORITY, (uint32)task, priority);
    iassert_TASKSCH_COMPONENT_1(
        priority >= _TASKSCH_PRIORITY_HIGHEST && priority <= _TASKSCH_PRIORITY_LOWEST,
        INVALID_PRIORITY_SETTING,
        priority);
    task->priority = priority;
}


/**
* FUNCTION NAME: TASKSCH_IsTaskActive()
*
* @brief  - Checks if a task is active.
*
* @return - TRUE if task is running or FALSE if not running.
*/
boolT TASKSCH_IsTaskActive
(
    TASKSCH_TaskT task
)
{
    return task->onRunQueue;
}

