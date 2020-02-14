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
//!   @file  - lexrex_msgs.c
//
//!   @brief - processes messages received over the link from the other CPU
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "toplevel_loc.h"
#ifndef GE_CORE
#include <net_base.h>
#endif

/************************ Defined Constants and Macros ***********************/

// Controls the number of 64 bit cache words read or written during each of the
// task executions of the partial send/receive tasks.
#define CACHE_WORDS_TRANSFERRED_PER_TASK_RUN 8


/******************************** Data Types *********************************/
struct _TOP_IncrementalQReadState
{
    struct XCSR_XICSQueueFrame* qFrame;
    enum
    {
        _TOP_Q_READ_STATE_BEGIN,
        _TOP_Q_READ_STATE_READING,
        _TOP_Q_READ_STATE_READ
    } state;
};


/***************************** Local Variables *******************************/
static void (*lexDownStreamStateChangeMsgHandler)(XCSR_CtrlDownstreamStateChangeMessageT, uint8 vport);
TASKSCH_TaskT _TOP_readCpuRxTaskHandle;

// The _cpuRxQFrame cannot be nested as a type inside the _TOP_IncrementalQReadState type because
// then we are not able to initialize the data array.  Initializing the data array is required in
// order to make it the correct size.  The initialization syntax used is GCC specific.
static struct XCSR_XICSQueueFrame _cpuRxQFrame = {
    .dataSize = 0,
    // NOTE: set in TOP_RxMessageInit() .dataCapacity = XCSR_QUEUE_FRAME_ETHERNET_PACKET_SIZE,
    .data = { [0 ... (XCSR_QUEUE_FRAME_ETHERNET_PACKET_SIZE - 1)] = 0 }
};
static struct _TOP_IncrementalQReadState _TOP_readCpuRxQState = {
    .state =_TOP_Q_READ_STATE_BEGIN,
    .qFrame = &_cpuRxQFrame
};

/************************ Local Function Prototypes **************************/
static void handleXUSBFrameData(
    struct XCSR_XICSQueueFrame* frameData) __attribute__((section(".ftext")));
static void _TOP_readCpuRxQTask(
    TASKSCH_TaskT task, uint32 taskArg) __attribute__((section(".ftext")));
static void _TOP_completedCpuRxFrameRead(
    TASKSCH_TaskT task,
    struct _TOP_IncrementalQReadState* s) __attribute__((section(".ftext")));

/************************** Function Definitions *****************************/

void TOP_RxMessageInit(enum CLM_XUSBLinkMode linkMode)
{
    // Setup our globals
    _cpuRxQFrame.dataCapacity = XCSR_QUEUE_FRAME_ETHERNET_PACKET_SIZE;

    _TOP_readCpuRxTaskHandle = TASKSCH_InitTask(
        &_TOP_readCpuRxQTask, (uint32)&_TOP_readCpuRxQState, FALSE, TASKSCH_CPURX_TASK_PRIORITY);

#ifndef GE_CORE
    if (linkMode == LINK_MODE_MULTI_REX)
    {
        lexDownStreamStateChangeMsgHandler = VHUB_DevicePortMessage;
    }
    else
#endif
    {
        // The Lex doesn't care which VPort, as it is only called in the 1:1
        // use case so it just ignores the 2nd arg and therefore the cast is
        // harmless.
        lexDownStreamStateChangeMsgHandler = CAST(
            &LEX_UlmMessageHandler,
            typeof(void (*)(XCSR_CtrlDownstreamStateChangeMessageT)),
            typeof(void (*)(XCSR_CtrlDownstreamStateChangeMessageT, uint8)));
    }
}

/**
* FUNCTION NAME: TOP_ProcessRxMessage()
*
* @brief  - Process a message received over the link from the other CPU
*
* @return - void
*/
void TOP_ProcessRxMessage(void)
{
    ilog_TOPLEVEL_COMPONENT_0(ILOG_DEBUG, ENTERED_CPU_RX_ISR);
    // Disable the CPU_RX interrupt.  It will be re-enabled before the CPU RX task is disabled.
    XCSR_XUSBDisableInterruptCpuRx();
    TASKSCH_StartTask(_TOP_readCpuRxTaskHandle);

    // This is a fairly uncontrollable event
    // The least significant bytes of the clock ticks is purely random
    RANDOM_AddEntropy(LEON_TimerGetClockTicksLSB());
}

/**
* FUNCTION NAME: _TOP_readCpuRxQTask()
*
* @brief  - Reads CACHE_WORDS_TRANSFERRED_PER_TASK_RUN 64 bit cache words out of the CPU RX queue
*           each execution.  Once the end of frame is found, the higher level handlers are called.
*
* @return - void.
*/
static void _TOP_readCpuRxQTask(TASKSCH_TaskT task, uint32 taskArg)
{
    struct _TOP_IncrementalQReadState* s = (struct _TOP_IncrementalQReadState*)taskArg;
    uint8 phasesExecuted = 0;
    eXCSR_QueueReadStatusT qOperationResult;
    ilog_TOPLEVEL_COMPONENT_1(ILOG_DEBUG, RUNNING_CPU_RX_TASK, s->state);

    // We introduce a loop here so that we execute multiple phases of the task within a single
    // execution.  This allows us to process about twice as many ARP packets per second before
    // overflowing the cache as compared to executing a single phase before returning from the
    // task.
    while(phasesExecuted < 3 && TASKSCH_IsTaskActive(task))
    {
        switch(s->state)
        {
            case _TOP_Q_READ_STATE_BEGIN:
                // Set the data size to zero because it is used to store the number of bytes read.
                s->qFrame->dataSize = 0;
                qOperationResult = XCSR_XICSQueueReadPartialFrameHeader(LEX_REX_SQ_CPU_RX, s->qFrame);
                if(qOperationResult == END_OF_PACKET)
                {
                    s->state = _TOP_Q_READ_STATE_READING;
                }
                else if(qOperationResult == END_OF_FRAME)
                {
                    s->state = _TOP_Q_READ_STATE_READ;
                }
                else if(qOperationResult == CORRUPT_DATA)
                {
                    _TOP_completedCpuRxFrameRead(task, s);
                }
                else
                {
                    iassert_TOPLEVEL_COMPONENT_2(FALSE, CPU_RX_ERR, qOperationResult, __LINE__);
                }
                break;

            case _TOP_Q_READ_STATE_READING:
                qOperationResult = XCSR_XICSQueueReadPartialFrame(
                    LEX_REX_SQ_CPU_RX, s->qFrame, CACHE_WORDS_TRANSFERRED_PER_TASK_RUN);
                if(qOperationResult == END_OF_FRAME)
                {
                    s->state = _TOP_Q_READ_STATE_READ;
                }
                else if(qOperationResult == END_OF_PACKET)
                {
                    // Should not get an end of packet while reading a frame body
                    iassert_TOPLEVEL_COMPONENT_2(
                        qOperationResult != END_OF_FRAME, CPU_RX_ERR, qOperationResult, __LINE__);
                }
                else if(qOperationResult == CORRUPT_DATA || qOperationResult == Q_TOO_LARGE)
                {
                    _TOP_completedCpuRxFrameRead(task, s);
                }
                else
                {
                    // The NO_ERROR case just means we have more data to read in
                    // the next task iteration.
                    iassert_TOPLEVEL_COMPONENT_2(
                        qOperationResult == NO_ERROR, CPU_RX_ERR, qOperationResult, __LINE__);
                }
                break;

            case _TOP_Q_READ_STATE_READ:
                if(s->qFrame->header.one.generic.tagType != ETHERNET)
                {
                    handleXUSBFrameData(s->qFrame);
                }
#ifndef GE_CORE
                else
                {
                    NET_receiveFrameHandler(s->qFrame);
                }
#endif
                _TOP_completedCpuRxFrameRead(task, s);
                break;

            default:
                iassert_TOPLEVEL_COMPONENT_1(FALSE, UNEXPECTED_Q_OPERATION_STATE, s->state);
                break;
        }
        phasesExecuted++;
    }
}

/**
* FUNCTION NAME: _TOP_completedCpuRxFrameRead()
*
* @brief  - This helper function is called when a frame is being read in chunks and a chunk with
*           an end of frame is reached.  The task is stopped and interrupts are enabled if no more
*           frames are available to read.  Otherwise, the task continues and the next frame will
*           begin being processed on the next run of the task.
*
* @return - void.
*/
static void _TOP_completedCpuRxFrameRead(
    TASKSCH_TaskT task, struct _TOP_IncrementalQReadState* s)
{
    s->state = _TOP_Q_READ_STATE_BEGIN;
    if(!XCSR_XICSQueueContainsCompleteFrame(LEX_REX_SQ_CPU_RX))
    {
        XCSR_XUSBEnableInterruptCpuRx();
        TASKSCH_StopTask(task);
    }
}


/**
* FUNCTION NAME: handleXUSBFrameData()
*
* @brief  - Processes data read from the CPU RX data queue.
*
* @return - None.
*/
static void handleXUSBFrameData(struct XCSR_XICSQueueFrame* frameData)
{
    uint8 vport;
    XCSR_CtrlLinkTypeT linkType;
    uint8 genericMsg;
    uint32 msgData;

    vport = frameData->header.one.cpu_cpu.Vport;
    linkType = frameData->header.one.cpu_cpu.ctrlLinkType;
    genericMsg = frameData->header.one.cpu_cpu.msg;
    msgData = frameData->header.one.cpu_cpu.data;
    ilog_TOPLEVEL_COMPONENT_3(ILOG_DEBUG, RECEIVED_SOMETHING_ON_CPU_RXQ, linkType, genericMsg, msgData);

    // Ensure that none of our control link message enums exceed their maximum
    // allowed number of values
    COMPILE_TIME_ASSERT(
           (NUMBER_OF_UNIQUE_CONTROL_LINK_TYPES
                <= MAX_NUM_CTRL_LINK_TYPE_UNIQUE_VALUES)
        && (NUMBER_OF_UNIQUE_CONTROL_LINK_MESSAGES
                <= MAX_NUM_CTRL_LINK_SUBTYPE_UNIQUE_VALUES)
        && (NUMBER_OF_UNIQUE_CONTROL_DOWNSTREAM_STATE_CHANGE_MESSAGES
                <= MAX_NUM_CTRL_LINK_SUBTYPE_UNIQUE_VALUES)
        && (NUMBER_OF_UNIQUE_CONTROL_UPSTREAM_STATE_CHANGE_MESSAGES
                <= MAX_NUM_CTRL_LINK_SUBTYPE_UNIQUE_VALUES)
        && (NUMBER_OF_UNIQUE_CONTROL_SOFTWARE_MESSAGES
                <= MAX_NUM_CTRL_LINK_SUBTYPE_UNIQUE_VALUES)
    );

    if (linkType == LINK_MESSAGE)
    {
        uint64 extraData = frameData->header.two.dword[0] >> 8;
        XCSR_CtrlLinkMessageT linkMessage = genericMsg;

        LINKMGR_ProcessLinkMsg(vport, linkMessage, msgData, extraData);
    }
    else if (linkType == USB_DOWNSTREAM_STATE_CHANGE)
    {
        XCSR_CtrlDownstreamStateChangeMessageT downstreamMsg = genericMsg;
        iassert_TOPLEVEL_COMPONENT_1(GRG_IsDeviceLex(), GOT_MSG_FOR_LEXULM_OR_VHUB_ON_REX, downstreamMsg);
        (*lexDownStreamStateChangeMsgHandler)(downstreamMsg, vport);
    }
    else if (linkType == USB_UPSTREAM_STATE_CHANGE)
    {
        XCSR_CtrlUpstreamStateChangeMessageT upstreamMsg = genericMsg;
        iassert_TOPLEVEL_COMPONENT_1(GRG_IsDeviceRex(), GOT_MSG_FOR_REXULM_ON_LEX, upstreamMsg);
        REXULM_LinkMessageHandler(upstreamMsg);
    }
    else if (linkType == SOFTWARE_MESSAGE_LEX2REX)
    {
        XCSR_CtrlSwMessageT swMsg = genericMsg;
        iassert_TOPLEVEL_COMPONENT_1(GRG_IsDeviceRex(), GOT_MSG_FOR_REXULM_ON_LEX, swMsg);
        REXSCH_SwMessageHandler(swMsg, msgData);
    }
    else
    {
        ilog_TOPLEVEL_COMPONENT_2(ILOG_MAJOR_ERROR, UNKNOWN_LINK_TYPE_MSG, linkType, genericMsg);
    }
}
