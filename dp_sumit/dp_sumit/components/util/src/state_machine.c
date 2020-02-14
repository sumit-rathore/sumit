//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################

//#################################################################################################
// Module Description
//#################################################################################################
// Implementations of state machine functions common to the Lex and Rex subsystems.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <uart.h> // for debug (UART_printf() )
#include <state_machine.h>
#include "util_log.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################
static void UTILSM_PrintLog(const union LogInfo* logInfo, uint8_t arg1, uint8_t arg2, uint8_t arg3);
// Static Variables ###############################################################################

// Component Variables ############################################################################

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void UTILSM_PrintLog(const union LogInfo* logInfo, uint8_t arg1, uint8_t arg2, uint8_t arg3)
{
    if (logInfo->info.logLevel != ILOG_DEBUG)
    {
        const uint32_t ilogHeader = _ilog_header(
            logInfo->info.logLevel,
            logInfo->info.ilogComponent,
            logInfo->info.ilogId,
            3);

        _ilog(ilogHeader, arg1, arg2, arg3);
    }
}

//#################################################################################################
// state machine controller
//
// Parameters:
// Return:
// Assumptions: This is the *only* function that should ever manipulate state
//              UTILSM_EVENT_ENTER or main event handler can change state but UTILSM_EVENT_EXIT can't
//#################################################################################################
void UTILSM_PostEvent(
    struct UtilSmInfo *smState,
    uint8_t event,
    const void *eventData)
{
    smState->event = event;
    smState->eventData = eventData;
    uint8_t currentState = smState->currentState;
    uint8_t nextState = smState->stateHandlers[ currentState](event,  currentState);
    bool stateTransition = currentState != nextState;

    UTILSM_PrintLog(&smState->logInfo, currentState, nextState, event);

    if (stateTransition)
    {
        // exit the current state, next state is delivered as state parameter for exit event
        smState->stateHandlers[currentState](UTILSM_EVENT_EXIT, nextState);

        while (stateTransition)
        {
            // enter the new state
            smState->currentState = nextState;    // update the current state
            smState->prevState    = currentState; // update the previous state
            uint8_t afterEnterState = smState->stateHandlers[nextState](UTILSM_EVENT_ENTER, nextState);

            UTILSM_PrintLog(&smState->logInfo, nextState, afterEnterState, UTILSM_EVENT_ENTER);

            if(afterEnterState != nextState)
            {
                // we've switched states again!
                currentState = nextState;
                nextState = afterEnterState;
                event = UTILSM_EVENT_ENTER;
            }
            else
            {
                stateTransition = false;
            }
        }
    }

}
