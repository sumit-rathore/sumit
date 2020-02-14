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
//!   @file  -  icmd.c
//
//!   @brief -  The icmd parser
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/

#include <itypes.h>
#include <ibase.h>
#include <project_components.h>
#include <icmd.h>
#include "icmd_log.h"
#ifdef __MSP430__
#include <msp430_uart.h>
#else
#include <leon_uart.h>
#endif

#ifdef USE_OPTIONS_H
#include <options.h>
#endif


#ifdef ICMD_USE_NON_ACTIVITY_TIMER
#include <timing_timers.h>
#ifdef __MSP430__
#include <msp430_timers.h>
#else
#include <leon_timers.h>
#endif
#endif

/************************ Defined Constants and Macros ***********************/
#if IENDIAN // big endian
#define ICMD_HEADER 0x98
#define ICMD_HEADER_MASK 0xF8
#else // Little endian
#define ICMD_HEADER 0xA8
#define ICMD_HEADER_MASK 0xF8
#endif

/***************************** Local Variables *******************************/

// Define all icmd entries for each component as a weak variable
#define COMPONENT_PARSER_PREFIX
#define COMPONENT_PARSER(x) extern void (* const icmd_ ## x[])() __attribute__ ((weak));
#define COMPONENT_PARSER_POSTFIX
#include <project_components.h>
#undef COMPONENT_PARSER_PREFIX
#undef COMPONENT_PARSER
#undef COMPONENT_PARSER_POSTFIX

// Create an array of the icmd entries for each component
#define COMPONENT_PARSER_PREFIX static const void * const icmd_callbacks[] = {
#define COMPONENT_PARSER(x) &icmd_ ## x,
#define COMPONENT_PARSER_POSTFIX };
#include <project_components.h>
#undef COMPONENT_PARSER_PREFIX
#undef COMPONENT_PARSER
#undef COMPONENT_PARSER_POSTFIX

static struct {
    uint32 args[8];
#ifdef ICMD_USE_NON_ACTIVITY_TIMER
    TIMING_TimerHandlerT inactivityTimer;
#endif
#ifdef ICMD_USE_JUNK_TIMER
    TIMING_TimerHandlerT junkTimer;
#endif
    uint8 curOffset;
} icmd;


static uint32_t iCmdArgs[6];

static uint8_t iCmdResponseId;  // the response ID to use in the command response
/************************** Function Declarations ****************************/

static void ICMD_ProcessByte(uint8)  __attribute__ ((noinline));
static void ICMD_ProcessByte_Starter(uint8) __attribute__ ((section (".ftext")));

static void ICMD_PacketizeHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t length, uint8_t responseId) __attribute__ ((section (".atext")));
static void ICMD_ProcessIcmd (const uint8_t *data, const uint16_t size) __attribute__ ((section (".atext")));

#ifdef ICMD_USE_NON_ACTIVITY_TIMER
static void _icmd_timeout(void) __attribute__ ((section (".ftext")));
#endif
#ifdef ICMD_USE_JUNK_TIMER
static void _icmd_enableProcessing(void) __attribute__ ((section (".ftext")));
#endif

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _icmd_timeout()
*
* @brief  - local function to call when our timer expires
*
* @return - void
*
* @note   - This just resets the current icmd message
*           Placed into .ftext to not delay the rex scheduler on garbage data
*
*/
#ifdef ICMD_USE_NON_ACTIVITY_TIMER
static void _icmd_timeout(void)
{
    // It has being more than ICMD_USE_NON_ACTIVITY_TIMER milliseconds since the last character.  This is probably a new message
    icmd.curOffset = 0;
    ilog_ICMD_COMPONENT_0(ILOG_MAJOR_ERROR, ICMD_TIMEOUT);
}
#endif

// timer function that is run after timer expiration after receiving junk
// helps filter UART noise when plugging/unplugging cables
#ifdef ICMD_USE_JUNK_TIMER
static void _icmd_enableProcessing(void)
{
    // Nothing to do.  Just take note that the timer is no longer running
    ilog_ICMD_COMPONENT_0(ILOG_MAJOR_EVENT, JUNK_TIMER_EXPIRED_REENABLING_PROCESSING);
}
#endif

/**
* FUNCTION NAME: ICMD_ProcessByte_Starter()
*
* @brief  - The icmd parser to process the 1st byte of incoming requests
*
* @return - void
*
* @note   - The end of this function is intended to have a sibling tail call
*
*           This in IRAM to always process the 1st byte quickly, in case it is a bogus message, and we won't hold up the system
*/
void ICMD_ProcessByte_StarterPublic (uint8 rxByte)
{
    ICMD_ProcessByte_Starter(rxByte);
}

static void ICMD_ProcessByte_Starter
(
    uint8 rxByte    // Byte received for icmd
)
{
#ifdef ICMD_USE_JUNK_TIMER
    if (TIMING_TimerEnabled(icmd.junkTimer))
    {
        // Cable is getting plugged/unplugged.  Filtering out junk for a bit of time
        return;
    }
#endif

    if ((icmd.curOffset == 0) && ((rxByte & ICMD_HEADER_MASK) != ICMD_HEADER))
    {
        // Not an icmd header.
        ilog_ICMD_COMPONENT_1(ILOG_MAJOR_ERROR, INVALID_HEADER, rxByte);
#ifdef ICMD_USE_JUNK_TIMER
        TIMING_TimerStart(icmd.junkTimer);
#endif
    }
    else
    {
        ICMD_ProcessByte(rxByte);
    }
}

/**
* FUNCTION NAME: ICMD_ProcessByte()
*
* @brief  - The icmd parser to process incoming requests and call the function handler
*
* @return - void
*
* @note   - The end of this function is intended to have a sibling tail call
*         - This doesn't work with GCC 4.4.3.  More compiler optimization is required for this
*
*         - Processing is started with ICMD_ProcessByte_Starter
*/
static void ICMD_ProcessByte
(
    uint8 rxByte    // Byte received for icmd
)
{
    uint8 *incoming_msg;
    uint8 numOfArgs;
    uint32 * pArgs = icmd.args;

    // basic sanity check
    iassert_ICMD_COMPONENT_2((icmd.curOffset < sizeof(icmd.args)), CUROFFSET_CORRUPTED, icmd.curOffset, __LINE__);
#ifdef ICMD_USE_NON_ACTIVITY_TIMER
    TIMING_TimerStart(icmd.inactivityTimer);
#endif

    incoming_msg = CAST(pArgs, uint32 *, uint8 *);
    incoming_msg[icmd.curOffset] = rxByte;

    switch (icmd.curOffset)
    {
        case 0: //Header
            icmd.curOffset++;    // Header check is done at entry to function
            break;

        case 1: //Component #
            icmd.curOffset++;
            break;

        case 2: //Function #
            icmd.curOffset = 4;
            break;

        case 3: // Can't happen, we skip this value
            iassert_ICMD_COMPONENT_2(FALSE, CUROFFSET_CORRUPTED, icmd.curOffset, __LINE__);
            break;

        default: // Args
            icmd.curOffset++;
            break;

    }

    numOfArgs = incoming_msg[0] & 0x7;

    // Check for a complete icmd
#ifdef __MSP430__ // 16 bit machine
    if (icmd.curOffset == ((numOfArgs * 2) + 4))
#else // 32 bit machine
    if (icmd.curOffset == ((numOfArgs * 4) + 4))
#endif
    {
        uint8 componentNumber = incoming_msg[1];
        uint8 functionNumber = incoming_msg[2];

        // Reset our state for the next message
        icmd.curOffset = 0;

        // Ensure the command is valid
        if (componentNumber >= NUMBER_OF_ICOMPONENTS)
        {
            // Invalid component.
            ilog_ICMD_COMPONENT_1(ILOG_MAJOR_ERROR, INVALID_COMPONENT, componentNumber);
        }
        else if (numOfArgs > 6)
        {
            // too many args
            ilog_ICMD_COMPONENT_1(ILOG_MAJOR_ERROR, TOO_MANY_ARGS, numOfArgs);
        }
        else
        {   // Call the icmd function handler

            void (*pFunction)(uint32, uint32, uint32, uint32, uint32, uint32);
            const typeof(pFunction) * functionArray = icmd_callbacks[componentNumber];

            ilog_ICMD_COMPONENT_3(ILOG_DEBUG, RECVD_ICMD, componentNumber, functionNumber, numOfArgs);

            if (functionArray == NULL)
            {   // No valid function array for this component
                ilog_ICMD_COMPONENT_1(ILOG_MAJOR_ERROR, NO_ICMD_FCN_PTR_ARRAY, componentNumber);
            }
            else
            {   // We are good to go!!! call the function & stop the timer
                pFunction = functionArray[functionNumber];
#ifdef ICMD_USE_NON_ACTIVITY_TIMER
                TIMING_TimerStop(icmd.inactivityTimer);
#endif

                ilog_ICMD_COMPONENT_3(ILOG_DEBUG, CALLING_HANDLER, (uint32)pFunction, icmd.args[1], icmd.args[2]);

                (*pFunction)(icmd.args[1], icmd.args[2], icmd.args[3], icmd.args[4], icmd.args[5], icmd.args[6]);
                UART_packetizeRegisterClient( CLIENT_ID_GE_ICMD, &ICMD_PacketizeHandler);
            }
        }
    }
}


/**
* FUNCTION NAME: ICMD_ProcessIcmd()
*
* @brief  - Parses and calls the received iCmd
*
* @return - void
*
* @note
*
*/
static void ICMD_ProcessIcmd (const uint8_t *data, const uint16_t size)
{
    // get the miscellaneous info
    uint8_t numberOfArgs    = data[0] & 0x7;
    uint8_t componentNumber = data[1];
    uint8_t functionNumber  = data[2];

    // Ensure the command is valid
    if (componentNumber >= NUMBER_OF_ICOMPONENTS)
    {
        // Invalid component.
        ilog_ICMD_COMPONENT_1(ILOG_MAJOR_ERROR, INVALID_COMPONENT, componentNumber);
    }
    else if ( (numberOfArgs > 6) || (((size-3) >> 2) < numberOfArgs))
    {
        // too many args
        ilog_ICMD_COMPONENT_2(ILOG_MAJOR_ERROR, TOO_MANY_ARGS, numberOfArgs, size);
    }
    else
    {   // Call the icmd function handler

        void (*pFunction)(uint32, uint32, uint32, uint32, uint32, uint32);
        const typeof(pFunction) * functionArray = icmd_callbacks[componentNumber];

        ilog_ICMD_COMPONENT_3(ILOG_DEBUG, RECVD_ICMD, componentNumber, functionNumber, numberOfArgs);

        if (functionArray == NULL)
        {   // No valid function array for this component
            ilog_ICMD_COMPONENT_1(ILOG_MAJOR_ERROR, NO_ICMD_FCN_PTR_ARRAY, componentNumber);
        }
        else
        {   // We are good to go!!! call the function & stop the timer
            pFunction = functionArray[functionNumber];

            // copy over the arguments
            memset(iCmdArgs, 0, sizeof(iCmdArgs));      // make sure any unused arguments are zeroed out
            memcpy(iCmdArgs, &data[3], size-3);
            ilog_ICMD_COMPONENT_3(ILOG_DEBUG, CALLING_HANDLER, (uint32)pFunction, iCmdArgs[1], iCmdArgs[2]);

            (*pFunction)(iCmdArgs[0], iCmdArgs[1], iCmdArgs[2], iCmdArgs[3], iCmdArgs[4], iCmdArgs[5]);
        }
    }
}

/**
* FUNCTION NAME: ICMD_PollingLoop()
*
* @brief  - An endless polling loop that processes icmd messages
*
* @return - never
*
* @note   - This is intended to be called at the end of an assert for debugging
*
*           Interrupts are assumed to be disabled when icmds are run, so they
*           should be disabled when this command is run
*/
void ICMD_PollingLoop(void)
{
    uint8 rxByte;
#ifdef ICMD_USE_NON_ACTIVITY_TIMER
#ifdef __MSP430__
    MSP430_TimerValueT loopLastValidCharTimeVal = MSP430_TimerRead();
#else
    LEON_TimerValueT loopLastValidCharTimeVal = LEON_TimerRead();
#endif
#endif

    // Loop forever reading the uart & processing icmd's
    while (TRUE)
    {
        if  (
#ifdef __MSP430__
                MSP430_UartRx(&rxByte)
#else
                LEON_UartRx(&rxByte)
#endif
            )
        {
            // Byte received: process & restart timer
            ICMD_ProcessByte_Starter(rxByte);
#ifdef ICMD_USE_NON_ACTIVITY_TIMER
            loopLastValidCharTimeVal =
#ifdef __MSP430__
                MSP430_TimerRead();
#else
                LEON_TimerRead();
#endif
        }
        else if (   (icmd.curOffset)
#ifdef __MSP430__
                &&  (MSP430_TimerCalcUsecDiff(loopLastValidCharTimeVal, MSP430_TimerRead()) > (1000 * ICMD_USE_NON_ACTIVITY_TIMER))
#else
                &&  (LEON_TimerCalcUsecDiff(loopLastValidCharTimeVal, LEON_TimerRead()) > (1000 * ICMD_USE_NON_ACTIVITY_TIMER))
#endif
                )
        {
            // No byte was received and timer has expired
            _icmd_timeout();
#endif
        }
        else
        {
            // No new uart bytes, no timer running on an existing icmd
            // Lets ensure that the uart has time to output all of its messages
#ifdef __MSP430__
            MSP430_UartPollingModeDoWork();
#else
            LEON_UartPollingModeDoWork();
#endif
        }
    }
}

/**
* FUNCTION NAME: ICMD_Init()
*
* @brief  - Initialize the icmd component
*
* @return - void
*
* @note   - Doesn't enable UART interrupt, but registers uart RX handler
*           This is because the UART interrupt may or may not be shared
*           So we let the higher level initialization code determine when to enable the interrupt
*/
void ICMD_Init(void)
{
#ifdef __MSP430__
    MSP430_UartSetRxHandler(ICMD_ProcessByte_Starter);
#else
    LEON_UartSetRxHandler(ICMD_ProcessByte_Starter);
#endif

    //icmd.curOffset = 0; // bss code is all pre-initialized to zero
#ifdef ICMD_USE_NON_ACTIVITY_TIMER
    icmd.inactivityTimer = TIMING_TimerRegisterHandler(&_icmd_timeout, FALSE, ICMD_USE_NON_ACTIVITY_TIMER);
#endif
#ifdef ICMD_USE_JUNK_TIMER
    icmd.junkTimer = TIMING_TimerRegisterHandler(&_icmd_enableProcessing, FALSE, ICMD_USE_JUNK_TIMER);
#endif
}

uint8_t ICMD_GetResponseID(void)
{
    return (iCmdResponseId);
}

/**
* FUNCTION NAME: ICMD_PacketizeHandler()
*
* @brief  - Receive handler for iCmds received over the BB ext comm port
*
* @return - void
*
* @note
*
*
*/
static void ICMD_PacketizeHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t length, uint8_t responseId)
{
    // save the response ID - it will stay until the next iCmd is processed, which could be the very next call
    // so if you need to use it after your function returns, you'll need to save a copy of it.
    // Note that multiple calls to the same iCmd could have different response ID's
    iCmdResponseId = responseId;
    ICMD_ProcessIcmd(data, length);
}

