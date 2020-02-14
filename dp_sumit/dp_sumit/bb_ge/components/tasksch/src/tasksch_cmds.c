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
//!   @file  -  tasksch_cmds.c
//
//!   @brief -  Contains icmds
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "tasksch_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: viewTasks()
*
* @brief  - icmd function to display all of the tasks
*
* @return - void
*/
void viewTasks(void)
{
    for (uint8 i = 0; i < TASKSCH_MAX_NUM_OF_TASKS; i++)
    {
        TASKSCH_TaskT task = &_TASKSCH_State.taskArray[i];
        if (task->task)
        {
            ilog_TASKSCH_COMPONENT_3(
                ILOG_USER_LOG, VIEW_TASK1, (uint32)task, (uint32)task->task, task->taskArg);
            ilog_TASKSCH_COMPONENT_3(
                ILOG_USER_LOG, VIEW_TASK2, (uint32)task, task->priority, task->allowInterrupts);
            ilog_TASKSCH_COMPONENT_3(
                ILOG_USER_LOG, VIEW_TASK4, (uint32)task, i, task->onRunQueue);
        }
    }
}


#ifdef GE_PROFILE
/**
* FUNCTION NAME: _TASKSCH_viewTaskTimespan()
*
* @brief  - icmd function to display timespan of each task ran in the task scheduler
*
* @return - void
*
* @note   -
*
*/
void _TASKSCH_viewTaskTimespan(void)
{
    TASKSCH_TaskT task;
    uint64 totalTimespan = 0;
    for (uint8 i = 0; i < _TASKSCH_State.numOfAllocatedTasks; i++)
    {
        task = &_TASKSCH_State.taskArray[i];
        if (task->task != NULL)
        {
            ilog_TASKSCH_COMPONENT_3(
                ILOG_USER_LOG, VIEW_TASK_TIMESPAN, (uint32)(task->task), task->timeSpan >> 32, task->timeSpan & 0xffffffff);
            totalTimespan += task->timeSpan;
        }
    }
    ilog_TASKSCH_COMPONENT_2(ILOG_USER_LOG, VIEW_TASK_TOTALTIMESPAN, totalTimespan >> 32, totalTimespan & 0xffffffff);
}
#endif
