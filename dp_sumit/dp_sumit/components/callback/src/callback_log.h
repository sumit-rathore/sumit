//#################################################################################################
// Icron Technology Corporation - Copyright 2016
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef CALLBACK_LOG_H
#define CALLBACK_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(CALLBACK_COMPONENT)
    ILOG_ENTRY(CALLBACK_INVALID_TYPE, "Callback invalid type %d callback[index].type %d at line = %d\n")
    ILOG_ENTRY(CALLBACK_INVALID_HANDLE, "Callback invalid handle, tokens mismatched, at line = %d\n")
    ILOG_ENTRY(CALLBACK_INVALID_REMOVE, "Callback invalid remove at line = %d type %d index %d\n")
    ILOG_ENTRY(CALLBACK_FREE_LIST_EMPTY, "Callback free stack is empty\n")
    ILOG_ENTRY(CALLBACK_SINGLE_RUN_CANCELED, "New single run callback command canceled\n")
ILOG_END(CALLBACK_COMPONENT, ILOG_MAJOR_EVENT)

#endif // CALLBACK_LOG_H
