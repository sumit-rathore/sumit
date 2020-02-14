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
#ifndef BB_TOP_LOG_H
#define BB_TOP_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################
ILOG_CREATE(TEST_COMPONENT)
    // These logs should be maintain index.
    // added at the bottom when you add more, Don't change order of this log !!!!!!!!!!!
    ILOG_ENTRY(TEST_PRINT_STATUS, "\nEnable status    :%d\nError State      :%d\nIsolated State   :%d\n")
    ILOG_ENTRY(TEST_SET_ERROR_STATE, "Diagnostic component : %d with Error code : %d\n")
    ILOG_ENTRY(TEST_PRINT_TEST_STATUS, "Test Status : %d\n")
    ILOG_ENTRY(TEST_FLASH_GOLDEN_PROTECT, "Golden Protected %d\n")
    ILOG_ENTRY(TEST_DP_OC_READ, "DP Over current status : %d\n")
ILOG_END(TEST_COMPONENT, ILOG_MINOR_EVENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // BB_TOP_LOG_H
