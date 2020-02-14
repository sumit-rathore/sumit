///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2007, 2008
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
//!   @file  -  queue_test.c
//
//!   @brief -  contains the test harness for writing to a queue and reading back
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#include <leon_uart.h>
#include "queue_test_log.h"
#include <xcsr_xicsq.h>


void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint8 qid;
    uint32 testData = 0xDEADBEEF;
    struct XCSR_XICSQueueFrame* testFrame;
    struct XCSR_XICSQueueFrame* readFrame;
    eXCSR_QueueReadStatusT returnVal;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, testFrame);
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, readFrame);

     // Configure the uart
    LEON_UartSetBaudRate115200();

    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, QUEUE_TEST_STARTUP);
    LEON_UartWaitForTx();

    // Allocate a queue
    qid = XCSR_XICSQQueueAllocate(QT_DEFAULT);
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, ALLOCATE_Q, qid);
    LEON_UartWaitForTx();

    testFrame->dataSize = 0;
    testFrame->header.one.word[0] = testData;
    XCSR_XICSWriteFrame(qid, testFrame);
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, WRITE_TO_Q, qid);
    LEON_UartWaitForTx();

    // Now read out what was written into the queue
    readFrame->header.one.word[0] = 0;
    returnVal = XCSR_XICSQueueReadFrame(qid, readFrame);
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, RETURN_VALUE, returnVal);
    LEON_UartWaitForTx();
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_MAJOR_EVENT, READ_FROM_Q, qid, readFrame->header.one.word[0]);
    LEON_UartWaitForTx();

    iassert_TEST_HARNESS_COMPONENT_2(readFrame->header.one.word[0] == readFrame->header.one.word[0],
                                     DATA_ERROR, readFrame->header.one.word[0], testFrame->header.one.word[0]);

    //TODO: this test harness can be expanded, test all the return codes from the read queue function, add more data etc
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, QUEUE_TEST_FINISHED);

    // That's it, but there is no way to finish, so lets loop forever
    while (TRUE)
    ;
}

