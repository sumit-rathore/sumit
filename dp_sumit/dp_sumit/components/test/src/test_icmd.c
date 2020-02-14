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
#include <flash_raw.h>
#include <gpio.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// ICMD for saving System Diagnostic Flash variable
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void TEST_SystemDiagnosticIcmd(void)
{
    TEST_EnableDiagnostic();
}

//#################################################################################################
// ICMD for protecting flash
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void TEST_ProtectFlashIcmd(void)
{
    FLASHRAW_GoldenProtect();
}

//#################################################################################################
// ICMD for reading the flash
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void TEST_ReadFlashProtectIcmd(void)
{
    ilog_TEST_COMPONENT_1(ILOG_USER_LOG, TEST_FLASH_GOLDEN_PROTECT, FLASHRAW_ReadGoldenProtect());
}

//#################################################################################################
// ICMD to set Test status bit in flash
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void TEST_SetTestStatusFlashVariableIcmd(uint8_t newTestStatus)
{
    // Set flash variable when error code is reported
    ConfigDiagnosticConfig *diagConfig =  &(Config_GetBuffer()->diagConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_DIAG_CONFIG, diagConfig))
    {
        diagConfig->testStatus = newTestStatus;
        if(Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_DIAG_CONFIG, diagConfig))
        {
            ilog_TEST_COMPONENT_1(ILOG_MAJOR_EVENT, TEST_PRINT_TEST_STATUS, diagConfig->testStatus);
        }
    }
}

//#################################################################################################
// ICMD to read Test status bit from flash
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void TEST_GetTestStatusFlashVariableIcmd(void)
{
    // Set flash variable when error code is reported
    ConfigDiagnosticConfig *diagConfig =  &(Config_GetBuffer()->diagConfig);

    if (Config_ArbitrateGetVar(CONFIG_VAR_DIAG_CONFIG, diagConfig))
    {
        ilog_TEST_COMPONENT_1(ILOG_MAJOR_EVENT, TEST_PRINT_TEST_STATUS, diagConfig->testStatus);
    }
}

//#################################################################################################
// Function for printing variables for TEST component
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void TEST_GetFpgaOCStatusIcmd(void)
{
    bool pinStatus = GpioRead(GPIO_CONN_DP_OVER_CURRENT);
    ilog_TEST_COMPONENT_1(ILOG_MAJOR_EVENT, TEST_DP_OC_READ, !pinStatus);
    ILOG_istatus(ISTATUS_DP_OVER_CURRENT_WARNING, 0);
}
