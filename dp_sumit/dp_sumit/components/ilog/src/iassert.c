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
//!   @file  -  leon_assert.c
//
//!   @brief -  Handles the assert function
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#ifdef ILOG_USE_LEON
#include <leon_cpu.h>
#include <uart.h>
#include <leon_timers.h>
#endif
#include "ilog_loc.h"


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
struct assertInfo_s
{
    uint32_t header;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    uint8_t assertCount; // should initialized to 0
};

/***************************** Local Variables *******************************/
assertHookEndFunctionT pAssertHookEndFunction;
void (*pAssertHookStartFunction)(void);

struct assertInfo_s assertInfo; // in bss so pre-initialized to 0

/************************ Local Function Prototypes **************************/

static void AssertPrintRegisterWindows(uint8_t startWindow, uint8_t endWindow);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: iassert_Init()
*
* @brief  - initialize the assert handler
*
* @return - void
*
* @note   - If this isn't called, asserts will just go into a while(1); loop
*
*/
void iassert_Init
(
    void (*assertHookStartFunctionArg)(void),       // function to call as soon as the assert occurs
    assertHookEndFunctionT assertHookEndFunctionArg // function to call after the assert handling has completed
)
{
    pAssertHookEndFunction = assertHookEndFunctionArg;
    pAssertHookStartFunction = assertHookStartFunctionArg;
}


/**
* FUNCTION NAME: _iassert()
*
* @brief - Assert handler
*
* @param - header       - The log message header (or full log if there are no arguments)
* @param - arg1         - optional arg
* @param - arg2         - optional arg
* @param - arg3         - optional arg
*
* @return - void
*
* @note -
*           The main assert function.  DO NOT CALL DIRECTLY.  Use the iassert_COMPONENT_NAME_? wrapper functions
*/
void _iassert(uint32_t header, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
#ifdef ILOG_USE_LEON
    // Ensure that we won't be interrupted by any interrupts
    LEON_CPUDisableIRQ();

    UART_WaitForTx();   // wait for the UART tx buffer to clear
#endif

    // If the project supplied a hook function, we run it here
    // This would be useful in USB extension to bring down the USB links
    if (pAssertHookStartFunction)
    {
        (*pAssertHookStartFunction)();
    }

    // Save this assert message
    assertInfo.header = header;
    assertInfo.arg1 = arg1;
    assertInfo.arg2 = arg2;
    assertInfo.arg3 = arg3;
    assertInfo.assertCount++;

    // Log this assert message
    _ilog(header, arg1, arg2, arg3);

    // Add all the debugging information we can
#ifdef ILOG_USE_LEON
    {
        uint8_t lastActiveWindow = LEON_CPUGetCurrentRegWindow(LEON_CPUGetPSR());
        uint8_t numOfWin = LEON_CPUGetNumOfRegWindows();

        ilog_ILOG_COMPONENT_3(ILOG_FATAL_ERROR, COPROC_REG_DUMP, LEON_CPUGetTBR(), LEON_CPUGetPSR(), LEON_CPUGetWIM());
        ilog_ILOG_COMPONENT_3(ILOG_FATAL_ERROR, GLOBAL_REG_DUMP, LEON_CPUGetG5(), LEON_CPUGetG6(), LEON_CPUGetG7());
        ilog_ILOG_COMPONENT_1(ILOG_FATAL_ERROR, MISC_REG_DUMP, lastActiveWindow);

        AssertPrintRegisterWindows(0, lastActiveWindow);

        ilog_ILOG_COMPONENT_0(ILOG_FATAL_ERROR, SEPERATOR_LOG);

        AssertPrintRegisterWindows(lastActiveWindow, numOfWin);
    }
#endif

    // call the end function, if possible, else just loop forever
    if (pAssertHookEndFunction)
    {
#ifdef ILOG_USE_LEON
        // wait half a second before resetting.
        // This does a couple of things
        // 1) Allows Tigger to analyze all of the logging data we have just sent.  Tigger probably needs more than 500ms, but at least this is something
        // 2) If we have a USB connection, it gives the host time to determine that we have disconnected, before we reset
        //      If we have an embedded Lex hub, a reset will reset the whole board (because we are the master clock source)
        //      There seems to be an issue with Windows that where a hub resets in the middle of a Port Status request, Windows will disable that hub.
        //      Measuring by CATC in one situation, we need around 120ms, which is about the delay if we dumped a whole stack
        //      So we add another 500ms.
        LEON_TimerValueT origTimerValue = LEON_TimerRead();
        while (LEON_TimerCalcUsecDiff(origTimerValue, LEON_TimerRead()) < 1000 * 500)
            ;

        // Call the hook function with a new stack
        // TODO:  shouldn't this be called first, in case the stack is messed up?  After we process the assert, then call the End Function?
        LEON_CPUInitStackAndCall(pAssertHookEndFunction);
#else
        pAssertHookEndFunction();
#endif
    }
    else
    {
        while (true)
            ;
    }
}

// icmd for checking assert status
void assertStatus(void)
{
    ilog_ILOG_COMPONENT_2(ILOG_USER_LOG, ASSERT_STATUS1, (uint32_t)pAssertHookStartFunction, (uint32_t)pAssertHookEndFunction);
    if (assertInfo.assertCount == 0)
    {
        ilog_ILOG_COMPONENT_0(ILOG_USER_LOG, ASSERT_STATUS2);
    }
    else // an assert has occurred
    {
        ilog_ILOG_COMPONENT_1(ILOG_USER_LOG, ASSERT_STATUS3, assertInfo.assertCount);
        _ilog(assertInfo.header, assertInfo.arg1, assertInfo.arg2, assertInfo.arg3);
    }
}

static void AssertPrintRegisterWindows(uint8_t startWindow, uint8_t endWindow)
{

    for (uint8_t curWin = startWindow; curWin < endWindow; curWin++)
    {
        struct LEON_CPURegs CPURegs;

        LEON_CPUGetRegs(&CPURegs, curWin);
        ilog_ILOG_COMPONENT_3(ILOG_FATAL_ERROR, GEN_REG_DUMP0, curWin,     CPURegs.l0, CPURegs.l1);
        ilog_ILOG_COMPONENT_3(ILOG_FATAL_ERROR, GEN_REG_DUMP1, CPURegs.l2, CPURegs.l3, CPURegs.l4);
        ilog_ILOG_COMPONENT_3(ILOG_FATAL_ERROR, GEN_REG_DUMP2, CPURegs.l5, CPURegs.l6, CPURegs.l7);
        ilog_ILOG_COMPONENT_3(ILOG_FATAL_ERROR, GEN_REG_DUMP3, curWin,     CPURegs.i0, CPURegs.i1);
        ilog_ILOG_COMPONENT_3(ILOG_FATAL_ERROR, GEN_REG_DUMP4, CPURegs.i2, CPURegs.i3, CPURegs.i4);
        ilog_ILOG_COMPONENT_3(ILOG_FATAL_ERROR, GEN_REG_DUMP5, CPURegs.i5, CPURegs.i6, CPURegs.i7);

        UART_WaitForTx();   // make sure we don't overflow the uart when we print all of this out
    }
}
