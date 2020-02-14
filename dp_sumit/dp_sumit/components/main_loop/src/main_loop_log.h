//#################################################################################################
// Icron Technology Corporation - Copyright 2018
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef MAIN_LOOP_LOG_H
#define MAIN_LOOP_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(MAIN_LOOP_COMPONENT)
    ILOG_ENTRY(MAX_TIME, "Max task time: %d us\n")
    ILOG_ENTRY(MAX_TIME_DETAIL, "poll:%d, event and callback:%d\n")
    ILOG_ENTRY(RESET_STAT, "Reset Max task time\n")
ILOG_END(MAIN_LOOP_COMPONENT, ILOG_DEBUG)

#endif // ULP_LOG_H
