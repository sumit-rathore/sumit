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
//!   @file  -  testmode.c
//
//!   @brief -  Handles USB Test Mode transactions
//
//
//!   @note  -  The code will never return from this file.
//              The USB spec requires a power cycle to get out of a test mode
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "sys_ctrl_q_loc.h"
#include <ulm.h>
#include <icmd.h>
#include <leon_timers.h>
#include <leon_uart.h>
#include <xlr.h>
#include <grg.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _SYSCTRLQ_HandleTestModeResponse()
*
* @brief  - Starts up the USB test mode
*
* @return - never
*
* @note   -
*
*/
void _SYSCTRLQ_HandleTestModeResponse
(
    enum expectedUpstreamPacket transactionType // Which test mode to run
)
{
    uint8 qid;

    LEON_TimerWaitMicroSec(2000); //block for 2 ms, to wait for ULM to finish transmitting response (probably only need microseconds)

    ULM_SetTestMode();

    switch (transactionType)
    {
        case eTest_J:
            ULM_GenJ();
            break;

        case eTest_K:
            ULM_GenK();
            break;

        case eTest_SE0_NAK:
            XLR_NakAllRequests();
            XLR_lexEnable();
            break;

        case eTest_Packet:
            // allocate Q
            qid = XCSR_XICSQQueueAllocate(QT_DEFAULT);

            // set the QID tracker
            XLR_SetQueueTrakerQid(qid);

            // write in response new dynamic q
            XCSR_QueueWriteTestModeFrame(qid);

            // and go
            XLR_StartSendingTestModePacket();
            break;

        default:
            iassert_SYS_CTRL_Q_COMPONENT_1(FALSE, TEST_MODE_HANDLING_INVALID, transactionType);
            break;
    }

    ilog_SYS_CTRL_Q_COMPONENT_0(ILOG_WARNING, STARTING_TEST);
    // USB traffic is dead.  Just go enter a debug polling loop
    ICMD_PollingLoop();
}

