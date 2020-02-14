///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009, 2010, 2011, 2012
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
//!   @file  -  ilog_main.c
//
//!   @brief -  This file contains all the core ilogging functions
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////


/***************************** Included Headers ******************************/
#include "ilog_loc.h"
#include <ilog.h>
#ifdef ILOG_USE_LEON
//#include <leon_traps.h>
#include <leon_timers.h>
#include <sys_defs.h>
#include <uart.h>
#endif
#ifdef __MSP430__
#include <msp430_uart.h>
#endif

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

// Need to keep track of the logging settings for each component
ilogLevelT component_logging_level[NUMBER_OF_ICOMPONENTS];

static uint32_t timeStampOffset; // Offset the Rex needs to use to get in line with the lex

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _ilog()
*
* @brief - Does the logging
*
* @param - header       - The log message header (or full log if there are no arguments)
* @param - arg1         - optional arg
* @param - arg2         - optional arg
* @param - arg3         - optional arg
*
* @return - void
*
* @note -
*           The main logging function.  DO NOT CALL DIRECTLY.  Use the ilog_COMPONENT_NAME_? wrapper functions
*
*           IMPORTANT!!!! IT IS DOCUMENTED THAT THIS CALLS THE BACKEND OUTPUT FUNCTION WITH 4 BYTE ALIGNMENT AND 4 BYTE MULTIPLE SIZES
*
* @TODO -   remove some magic numbers
*/
void _ilog(uint32_t header, const uint32_t arg1, const uint32_t arg2, const uint32_t arg3)
{
#ifdef BB_PACKETIZE_DISABLED
    uint32_t output_msg[4]; // max length for header + component + code + level + [4 byte arg1] + [4 byte arg2] + [4 byte arg3]
#ifndef ILOG_ASSERT_ON_DROPPED_LOGS
    static
#endif
        bool previousLogPrinted = true; // only use static keyword when it actually matters

#if IENDIAN // big endian
    ilogLevelT level = header & 0xFF;
    //uint8_t code = (header >> 8) & 0xFF; //not needed, but this is where it is
    component_t component = (header >> 16) & 0xFF;
    uint8_t numOfArgs = (header >> 24) & 0x3;
#else // little endian
    ilogLevelT level = (ilogLevelT)(header >> 24);
    //uint8_t code = (header >> 16) & 0xFF; //not needed, but this is where it is
    component_t component = (component_t)((header >> 8) & 0xFF);
    uint8_t numOfArgs = header & 0x3;
#endif
    uint8_t bytesToPrint = 4 + (numOfArgs << 2); // Left shift 2 is multiply by 4

    //Sanity argument check
    iassert_ILOG_COMPONENT_1(component < NUMBER_OF_ICOMPONENTS, INVALID_COMPONENT_ILOG_MAIN, component);
    iassert_ILOG_COMPONENT_1(level < ILOG_NUMBER_OF_LOGGING_LEVELS, INVALID_LEVEL_ILOG_MAIN, level);

    // Check the logging level of this component to see if we need to even log
    if (level < component_logging_level[component])
    {
        return;
    }

    // Copy the args, could be don't cares for the some, but probably faster than checking numOfArgs
    iassert_ILOG_COMPONENT_1(numOfArgs < 4, TOO_MANY_ARGS, numOfArgs);
    output_msg[1] = arg1;
    output_msg[2] = arg2;
    output_msg[3] = arg3;

    {
        // Now done by caller in ilog macro/static line function, so just add the part the caller doesn't know about
        // 8 bit binary header: 1111_<Endian Setting><Previous log printed><number of args upper bit><number of args lower bit>
        if (previousLogPrinted)
        {
#if IENDIAN // big endian
            header |= (0x4 << 24);
#else // little endian
            header |= 0x4;
#endif
        }

        // Basic logging data
        output_msg[0] = header;

        // Output the log message
        do {
#ifdef BUILD_FOR_SIM
            uint8_t i;

            for (i = 0; i < bytesToPrint; i++)
            {
                *(uint8_t *)(0x80000020) = ((uint8_t *)output_msg)[i];
            }
#elif defined(ILOG_USE_LEON)
            previousLogPrinted = UART_AtomicTx((uint8_t *)(output_msg), bytesToPrint);

            // In the case of a lockup, SW will not get to the uart interrupt to transmitt the rest of the uart buffer
            // So we provide this special mode, to ensure that all messages are logged right away
            // This will slow the whole system down, but at least we will see the last message before the lockup
            // This is really useful as we don't have a hardware debugger, to check what state the CPU is stuck in
            if (blockingLogMode)
            {
                UART_WaitForTx();
            }
#elif defined(ILOG_UNDER_TEST_HARNESS)
            previousLogPrinted = ilog_TestHarnessAtomicPrint((uint8_t *)(output_msg), bytesToPrint);

            if (blockingLogMode)
            {
                ilog_TestHarnessWaitForTx();
            }
#elif defined(__MSP430__)
            previousLogPrinted = MSP430_UartAtomicTx((uint8_t *)(output_msg), bytesToPrint);

            // In the case of a lockup, SW will not get to the uart interrupt to transmitt the rest of the uart buffer
            // So we provide this special mode, to ensure that all messages are logged right away
            // This will slow the whole system down, but at least we will see the last message before the lockup
            // This is really useful as we don't have a hardware debugger, to check what state the CPU is stuck in
            if (blockingLogMode)
            {
                MSP430_UartWaitForTx();
            }
#else
#error "No backend function defined for ilog"
#endif

        } while ((level == ILOG_FATAL_ERROR) && (!previousLogPrinted)); // ensure logging occurs when the logging level is fatal
    }

#ifdef ILOG_ASSERT_ON_DROPPED_LOGS
    iassert_ILOG_COMPONENT_0(previousLogPrinted, ILOG_MSG_DROPPED);
#endif


#else // BB_PACKETIZE_DISABLED
    ilogLevelT level = header & 0xFF;
    component_t component = (header >> 16) & 0xFF;
    uint8_t numOfArgs = (header >> 24) & 0x3;


    uint8_t bytesToPrint = 4 + ILOG_TIMESTAMP_SIZE + (numOfArgs << 2); // Left shift 2 is multiply by 4

    // Sanity argument check
    // iassert_ILOG_COMPONENT_1(component < NUMBER_OF_ICOMPONENTS, INVALID_COMPONENT_ILOG_MAIN, component);
    // iassert_ILOG_COMPONENT_1(level < ILOG_NUMBER_OF_LOGGING_LEVELS, INVALID_LEVEL_ILOG_MAIN, level);
    // iassert_ILOG_COMPONENT_1(numOfArgs < 4, TOO_MANY_ARGS, numOfArgs);

    // Check the logging level of this component to see if we need to even log
    if (level >= component_logging_level[component])
    {
        // Copy the args, could be don't cares for some, but probably faster than checking numOfArgs
        const uint32_t output_msg[5] = {header, getIlogTimestamp(), arg1, arg2, arg3};
        // Output the log message
        UART_packetizeSendDataImmediate(UART_PORT_BB, CLIENT_ID_BB_ILOG, NULL, output_msg, bytesToPrint );
    }

#endif // BB_PACKETIZE_DISABLED
}

/**
* FUNCTION NAME: iLog_SetTimeStampOffset()
*
* @brief - Sets the timestamp offset on the Rex so it is more in synch with the Lex
*
* @param - timeStamp    - The timestamp on the other side that we want to synchronize to
*
* @return - void
*
* @note -
*           Only meant to be called on the Rex
*
*/

void iLog_SetTimeStampOffset(uint32_t timeStamp)
{
    uint32_t timerValue = CAST(LEON_TimerRead(), LEON_TimerValueT, uint32_t);

    timeStampOffset = timerValue - timeStamp;

    ilog_ILOG_COMPONENT_2(ILOG_MINOR_EVENT, ILOG_SET_TIMESTAMP, timeStamp, timeStampOffset);
}


uint32_t getIlogTimestamp(void)
{
    uint32_t timerValue = CAST(LEON_TimerRead(), LEON_TimerValueT, uint32_t);
    uint32_t correctedTimeStamp = 0x01000000 - timerValue + timeStampOffset;

    if (correctedTimeStamp > 0x01000000)
    {
        correctedTimeStamp -= 0x01000000;
    }

    return correctedTimeStamp;
}
