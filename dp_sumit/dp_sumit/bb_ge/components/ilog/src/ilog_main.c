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

#ifdef ILOG_USE_LEON
#ifdef BLACKBIRD
#include <uart.h>
#else
#include <leon_uart.h>
#include <leon_traps.h>
#endif // BLACKBIRD
#endif // ILOG_USE_LEON
#ifdef __MSP430__
#include <msp430_uart.h>
#endif

#ifdef BB_GE_COMPANION
#include <ge_bb_comm.h>
#endif

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

// Need to keep track of the logging settings for each component
ilogLevelT component_logging_level[NUMBER_OF_ICOMPONENTS];

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
void _ilog(uint32 header, uint32 arg1, uint32 arg2, uint32 arg3)
{
    uint32 output_msg[4]; // max length for header + component + code + level + [4 byte arg1] + [4 byte arg2] + [4 byte arg3]
#ifndef ILOG_ASSERT_ON_DROPPED_LOGS
    static
#endif
        boolT previousLogPrinted = TRUE; // only use static keyword when it actually matters

#if IENDIAN // big endian
    ilogLevelT level = header & 0xFF;
    //uint8 code = (header >> 8) & 0xFF; //not needed, but this is where it is
    component_t component = (header >> 16) & 0xFF;
    uint8 numOfArgs = (header >> 24) & 0x3;
#else // little endian
    ilogLevelT level = (ilogLevelT)(header >> 24);
    //uint8 code = (header >> 16) & 0xFF; //not needed, but this is where it is
    component_t component = (component_t)((header >> 8) & 0xFF);
    uint8 numOfArgs = header & 0x3;
#endif
    uint8 bytesToPrint = 4 + (numOfArgs << 2); // Left shift 2 is multiply by 4

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
            uint8 i;

            for (i = 0; i < bytesToPrint; i++)
            {
                *(uint8 *)(0x80000020) = ((uint8 *)output_msg)[i];
            }
#elif defined(ILOG_USE_LEON)
#ifdef BLACKBIRD
            previousLogPrinted = UART_AtomicTx((uint8 *)(output_msg), bytesToPrint);
#else
#ifdef BB_GE_COMPANION
            previousLogPrinted = GEBB_iLogSend(output_msg, bytesToPrint);
#else
            previousLogPrinted = LEON_UartAtomicTx((uint8 *)(output_msg), bytesToPrint);
#endif
#endif
            // In the case of a lockup, SW will not get to the uart interrupt to transmitt the rest of the uart buffer
            // So we provide this special mode, to ensure that all messages are logged right away
            // This will slow the whole system down, but at least we will see the last message before the lockup
            // This is really useful as we don't have a hardware debugger, to check what state the CPU is stuck in
            if (blockingLogMode)
            {
#ifdef BLACKBIRD
                UART_WaitForTx();
#else
                LEON_UartWaitForTx();
#endif
            }
#elif defined(ILOG_UNDER_TEST_HARNESS)
            previousLogPrinted = ilog_TestHarnessAtomicPrint((uint8 *)(output_msg), bytesToPrint);

            if (blockingLogMode)
            {
                ilog_TestHarnessWaitForTx();
            }
#elif defined(__MSP430__)
            previousLogPrinted = MSP430_UartAtomicTx((uint8 *)(output_msg), bytesToPrint);

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
}


