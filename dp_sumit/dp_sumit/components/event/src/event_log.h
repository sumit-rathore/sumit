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
#ifndef EVENT_LOG_H
#define EVENT_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(EVENT_COMPONENT)
    ILOG_ENTRY(EVENT_INVALID_ACCESS, "Event invalid access at line = %d\n")
    ILOG_ENTRY(EVENT_NULL_HANDLER, "Event subscribe a null handler\n")
    ILOG_ENTRY(EVENT_HANDLER_MAX, "Exceed max number of handlers\n")
    ILOG_ENTRY(EVENT_NULL_CHECK_FUNCTION, "Event check event status function is null, event = %d\n")
    ILOG_ENTRY(EVENT_INVALID_DEQUEUE, "Event invalid dequeue\n")
    ILOG_ENTRY(EVENT_INVALID_POP, "Event invalid stack pop at line = %d\n")
    ILOG_ENTRY(EVENT_INVALID_PUSH, "Event invalid stack push at line = %d\n")
ILOG_END(EVENT_COMPONENT, ILOG_MAJOR_EVENT)

#endif // EVENT_LOG_H
