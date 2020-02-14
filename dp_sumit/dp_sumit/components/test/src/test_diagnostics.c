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
// ICmd function definitions for the DP_HPD component.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <configuration.h>
#include "test_log.h"
#include <leon_timers.h>
#include <timing_timers.h>
#include <test_diagnostics.h>
#include "aux_api.h"


// Constants and Macros ###########################################################################
#define ISOLATE_ENABLED     1
#define ISOLATE_DISABLED    0
#define DIAGNOSTIC_TIMEOUT  1000
// Data Types #####################################################################################

// Static Function Declarations ###################################################################
static void TestDiagnosticTimerHandler(void);
// Global Variables ###############################################################################
static bool enableDiagnostic;
static enum DiagErrorState errorState;
TIMING_TimerHandlerT diagnosticTimer;

// Static Variables ###############################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Sets the error state of the diagnostic in ROM
//
// Parameters:  Error state
// Return:
// Assumptions:
//
//#################################################################################################
void TEST_DiagnosticInit(void)
{
    errorState = DIAG_NOT_TESTED;
    enableDiagnostic = false;
    diagnosticTimer = TIMING_TimerRegisterHandler(TestDiagnosticTimerHandler, false, DIAGNOSTIC_TIMEOUT);
}

//#################################################################################################
// Sets the error state of the diagnostic in ROM
//
// Parameters:  Error state
// Return:
// Assumptions:
//
//#################################################################################################
void TEST_SetErrorState(enum ComponentCode newComponentCode, enum DiagErrorState newErrorState)
{
    // Set flash variable when error code is reported

    switch(newComponentCode)
    {
        case DIAG_DP:
            if (newErrorState != DIAG_NO_ERROR)
            {
                errorState |= newErrorState;
            }
            else
            {
                errorState = newErrorState;
            }
            break;

        case DIAG_RSVD1:
        case DIAG_RSVD2:
        case DIAG_RSVD3:
        case DIAG_NUM_COMPONENT:
        default:
            break;
    }
}

//#################################################################################################
// Get the Diagnostic state of the diagnostic from ROM
//
// Parameters:
// Return: Error State
// Assumptions:
//
//#################################################################################################
bool TEST_GetDiagState(void)
{
    return enableDiagnostic;
}

//#################################################################################################
// Get the Error state of the diagnostic from ROM
//
// Parameters:
// Return: Error State
// Assumptions:
//
//#################################################################################################
enum DiagErrorState TEST_GetErrorState(void)
{
    return errorState;
}

//#################################################################################################
// Get the error state of the diagnostic from ROM
//
// Parameters:
// Return: Error State
// Assumptions:
//
//#################################################################################################
void TEST_EnableDiagnostic(void)
{
    enableDiagnostic = true;
    errorState =  DIAG_NO_HPD;

#if !defined BB_ISO && !defined BB_USB
    AUX_StartDiagnostic();
#endif

    TIMING_TimerStart(diagnosticTimer);
}

//#################################################################################################
// Function for printing variables for TEST component
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void TEST_PrintTestVariables(void)
{
    ilog_TEST_COMPONENT_3(ILOG_MAJOR_EVENT, TEST_PRINT_STATUS, TEST_GetDiagState(), TEST_GetErrorState(),
        TEST_GetDiagState() ? ISOLATE_ENABLED : ISOLATE_DISABLED);
}

// Static Function Definitions ###################################################################
static void TestDiagnosticTimerHandler(void)
{
    TEST_PrintTestVariables();
}
