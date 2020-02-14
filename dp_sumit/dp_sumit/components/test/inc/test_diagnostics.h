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
#ifndef TEST_H
#define TEST_H

// Includes #######################################################################################
#include <ibase.h>
#include <leon_timers.h>
#include "configuration.h"
// Constants and Macros ###########################################################################

// Data Types #####################################################################################
enum DiagErrorState
{
    // The error states are bit masked. Each bit identifies as an error state

    DIAG_NO_ERROR               = 0x0,    // 0
    DIAG_NO_HPD                 = (1<<0), // 1
    DIAG_LC_BW_NOT_HIGH         = (1<<1), // 2
    DIAG_LT_FAIL                = (1<<2), // 4
    DIAG_LT_NOT_HIGH_SETTING    = (1<<3), // 8
    DIAG_8b10b_ERROR            = (1<<4), // 16
    DIAG_NOT_640_480            = (1<<5), // 32
    // Not tested
    DIAG_NOT_TESTED             = 0xff,   // 255
};

enum ComponentCode
{
    DIAG_DP = 0,        // Component code for DP
    DIAG_RSVD1,         // Reserved component code for future
    DIAG_RSVD2,         // Reserved component code for future
    DIAG_RSVD3,         // Reserved component code for future
    DIAG_NUM_COMPONENT  // Reserved component code for future
};

// Function Declarations ##########################################################################
void TEST_SetErrorState(enum ComponentCode newComponentCode, enum DiagErrorState newErrorState);
bool TEST_GetDiagState(void);
void TEST_EnableDiagnostic(void);
enum DiagErrorState TEST_GetErrorState(void);
void TEST_PrintTestVariables(void);
void TEST_DiagnosticInit(void);
void TEST_ReadFlashProtectIcmd(void);
void TEST_GetTestStatusFlashVariableIcmd(void);
void TEST_GetFpgaOCStatusIcmd(void);

#endif // AUX_H
