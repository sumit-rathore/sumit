///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  -
//
//!   @brief -
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "topology_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
union _taskArg {
    uint32 raw;
    struct {
        uint8 unused0;
        uint8 unused1;
        uint8 logicalAddr;
        uint8 endpoint;
    };
};

/***************************** Local Variables *******************************/
static TIMING_TimerHandlerT XSSTBlockingTimer;
static TASKSCH_TaskT XSSTBlockingTask;

/************************ Local Function Prototypes **************************/
static void _DTT_XSSTMonXsstBlockingMonitor(void);
static void _DTT_XSSTMonXsstBlockingMonitorTask(TASKSCH_TaskT, uint32 taskArg);

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: _DTT_XSSTMonInit()
*
* @brief  - Initialise the various system monitor timers
*
* @return - nothing
*
*/
void _DTT_XSSTMonInit(void)
{
    // The timer kicks off the tasks
    XSSTBlockingTimer = TIMING_TimerRegisterHandler(
        _DTT_XSSTMonXsstBlockingMonitor, TRUE, CFG_IOBLKING_TIMER_INTERVAL);
    TIMING_TimerStart(XSSTBlockingTimer);

    // The task actually processes the XSST
    XSSTBlockingTask = TASKSCH_InitTask(
        &_DTT_XSSTMonXsstBlockingMonitorTask,
        0, //uint32 taskArg, See union _taskArg for details
        FALSE, //boolT allowInterrupts,
        TASKSCH_TOPOLOGY_XSST_MONITOR_PRIORITY);
}



/**
* FUNCTION NAME: _DTT_XSSTMonStop()
*
* @brief  - Stops the XSST monitor timer so that the XSST monitor will not do
*           its work.
*
* @return - nothing
*
* @note   - icmd
*/
void _DTT_XSSTMonStop(void)
{
    if (GRG_IsDeviceLex())
    {
        TIMING_TimerStop(XSSTBlockingTimer);
        ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, STOP_XSST);
    }
    else
    {
        ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, XSSTMON_LEX_SPECIFIC_ICMD);
    }
}


/**
* FUNCTION NAME: _DTT_XSSTMonStart()
*
* @brief  - Starts the XSST monitor timer to periodically free unused queues.
*
* @return - nothing
*
* @note   - icmd
*/
void _DTT_XSSTMonStart(void)
{
    if (GRG_IsDeviceLex())
    {
        TIMING_TimerStart(XSSTBlockingTimer);
        ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, START_XSST);
    }
    else
    {
        ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, XSSTMON_LEX_SPECIFIC_ICMD);
    }
}


static void _DTT_XSSTMonXsstBlockingMonitor(void)
{
    uint8 logicalAddr;
    union _taskArg taskArg; 

    if (TASKSCH_IsTaskActive(XSSTBlockingTask))
    {
        ilog_TOPOLOGY_COMPONENT_0(ILOG_WARNING, XSST_MONITOR_STILL_PROCESSING);
        return;
    }

    // Update Logical address and ensure that the logical address is sane
    taskArg.raw = TASKSCH_GetTaskArg(XSSTBlockingTask);
    logicalAddr = taskArg.logicalAddr;
    logicalAddr++;
    if (__builtin_popcount(MAX_USB_DEVICES) == 1)
    {
        const uint8 logicalMask = MAX_USB_DEVICES - 1;
        logicalAddr &= logicalMask;
    }
    else if (logicalAddr >= MAX_USB_DEVICES)
    {
        logicalAddr = 0;
    }

    // Update the argument to restart at endpoint 0 for the new address
    taskArg.raw = 0;
    taskArg.logicalAddr = logicalAddr;
    TASKSCH_ChangeTaskArg(XSSTBlockingTask, taskArg.raw);

    // kick off the task
    TASKSCH_StartTask(XSSTBlockingTask);
}

static void _DTT_XSSTMonXsstBlockingMonitorTask
(
    TASKSCH_TaskT task,
    uint32 taskArg      // See union _taskArg for details
)
{
    union _taskArg arg;
    XUSB_AddressT address;
    struct DeviceTopology *deviceNode;

    // build the argument union
    arg.raw = taskArg;

    // Check if this address is even in use
    address = _DTT_GetAddressFromLogical(arg.logicalAddr);
    if (!XCSR_getXUSBAddrInSys(address))
    {
        // Stop the task, no work to be done
        TASKSCH_StopTask(task);
        return;
    }

    // Check if the last endpoint has been processed
    deviceNode = _DTT_GetDeviceNode(arg.logicalAddr);
    if (arg.endpoint > deviceNode->highEndpoint)
    {
        // Stop the task, no work to be done
        TASKSCH_StopTask(task);
        return;
    }

    // There is work to be done
    // Call LG1/GE specific code
    _DTT_XSSTMonitorProcess(address, arg.endpoint, deviceNode->isHub);

    // Update endpoint for the next iteration
    arg.endpoint++;
    TASKSCH_ChangeTaskArg(task, arg.raw);
}

