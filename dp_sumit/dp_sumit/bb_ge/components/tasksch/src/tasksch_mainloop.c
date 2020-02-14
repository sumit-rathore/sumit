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
//!   @file  -  tasksch_mainloop.c
//
//!   @brief -  The main loop of any program using this simple task scheduler
//
//
//!   @note  -  TODO: The 2 schedulers are different by
//                  1) priorities checking
//                  2) allowing new tasks to be allocated
//              These probably should use ifdef TASKSCH_<FEATURE>, instead of
//              project names.
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "tasksch_loc.h"
#ifdef __MSP430__
#include <msp430_traps.h>
#else
#include <leon_traps.h>
#endif

#ifdef GE_PROFILE
#include <leon_timers.h>
#endif
/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

#if defined(GOLDENEARS) || defined(BLACKBIRD)
/**
* FUNCTION NAME: TASKSCH_MainLoop()
*
* @brief  - Main loop of the simple task scheduler.  The next task to run will always be the
*           highest priority task which follows the most recently run task.  This means that tasks
*           at higher priorities can starve any task with a lower priority, but tasks at cannot
*           starve higher or equal priority tasks.
*
* @return - never
*
* @note   - This doesn't support pre-emption
*
*           Interrupts are disabled for all tasks
*/
void TASKSCH_MainLoop(void)
{
    uint8 lastRun = 0;
    while(TRUE)
    {
        const uint8 numOfTasks = _TASKSCH_State.numOfAllocatedTasks;
        enum TASKSCH_taskPriority searchingPriority = _TASKSCH_PRIORITY_HIGHEST;
        uint8 toRun = lastRun + 1;
        boolT searchingForTask = TRUE;
        while(searchingForTask)
        {
            TASKSCH_TaskT task;
            volatile TASKSCH_TaskT vtask;
            if(toRun >= numOfTasks)
            {
                toRun = 0;
            }
            task = &_TASKSCH_State.taskArray[toRun];
            vtask = task;
            if ((vtask->priority <= searchingPriority) && vtask->onRunQueue)
            {
                irqFlagsT oldFlags;
#ifdef TASKSCH_TASKS_ALLOW_INTERRUPTS
                const boolT taskAllowsInterrupts = task->allowInterrupts;
                // This will use a critical section if the task allows interrupts
                if(!taskAllowsInterrupts)
#endif
                {
                    // Enter critical section
                    oldFlags = LEON_LockIrq();
                }
                // *** Possible Critical Section Code ***
                {
                    // Should this task still be run?
                    if(vtask->onRunQueue)
                    {
#ifdef GE_PROFILE
                        LEON_TimerValueT startTime = LEON_TimerRead();
#endif
                        (*(task->task))(task, vtask->taskArg);
#ifdef GE_PROFILE
                        task->timeSpan += LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead());
#endif
                        lastRun = toRun;
                        searchingForTask = FALSE;
                    }
                }
                // *** End Possible Critical Section Code ***
#ifdef TASKSCH_TASKS_ALLOW_INTERRUPTS
                if(!taskAllowsInterrupts)
#endif
                {
                    // Leave critical Section
                    LEON_UnlockIrq(oldFlags);
                }
            }
            else
            {
                if(toRun == lastRun)
                {
                    if(searchingPriority == _TASKSCH_PRIORITY_LOWEST)
                    {
                        searchingForTask = FALSE;
                    }
                    else
                    {
                        // This code assumes that higher enum priority values indicate lower priority
                        // tasks
                        searchingPriority++;
                    }
                }
                toRun++;
            }
        }
    }
}

#else // LG1, Falcon

/**
* FUNCTION NAME: TASKSCH_MainLoop()
*
* @brief  - Main loop of the simple task scheduler
*
* @return - never
*
* @note   - This doesn't support pre-emption
*
*           Interrupts are disabled for all tasks
*/
void TASKSCH_MainLoop(void)
{
    const uint8 numOfTasks = _TASKSCH_State.numOfAllocatedTasks; // Don't keep going back to global memory
    uint8 curTaskNum = 0;

    // Stop TASKSCH_InitTask from initializing any more tasks
    _TASKSCH_State.numOfAllocatedTasks = TASKSCH_MAX_NUM_OF_TASKS;

    while (TRUE)
    {
        TASKSCH_TaskT task = _TASKSCH_State.taskArray + curTaskNum;
        volatile TASKSCH_TaskT vtask = task;
#ifdef TASKSCH_TASKS_ALLOW_INTERRUPTS
        // NOTE: For some uninvestigated reason the code is smaller when this is here,
        // instead of after the if (vtask->onRunQueue)
        boolT taskAllowsInterrupts = task->allowInterrupts;
#endif
        // Should this task be run?
        if (vtask->onRunQueue)
        {
            irqFlagsT oldFlags;
#ifdef TASKSCH_TASKS_ALLOW_INTERRUPTS
            // This will use a critical section if the task allows interrupts
            if (!taskAllowsInterrupts)
#endif
            {
                // Enter critical section
#ifdef __MSP430__
                oldFlags = MSP430_LockIrq();
#else
                oldFlags = LEON_LockIrq();
#endif
            }
            // *** Possible Critical Section Code ***
            {
                // Should this task still be run?
                if (vtask->onRunQueue)
                {
                    (*(task->task))(task, task->taskArg);
                }
            }
            // *** End Possible Critical Section Code ***
#ifdef TASKSCH_TASKS_ALLOW_INTERRUPTS
            if (!taskAllowsInterrupts)
#endif
            {
                // Leave critical Section
#ifdef __MSP430__
                MSP430_UnlockIrq(oldFlags);
#else
                LEON_UnlockIrq(oldFlags);
#endif
            }
        }

        // Get ready for the next task
        curTaskNum++;
        if (curTaskNum >= numOfTasks)
        {
            curTaskNum = 0;
        }
    }
}
#endif // GOLDENEARS
