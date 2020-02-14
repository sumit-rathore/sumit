//#################################################################################################
// Icron Technology Corporation - Copyright 2017
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef STATS_MON_LOG_H
#define STATS_MON_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################

ILOG_CREATE(STATS_MON_COMPONENT)
    ILOG_ENTRY(STATS_MON_MAX_REGISTERED,            "Max number of stat groups registered for monitoring\n")
    ILOG_ENTRY(STATS_MON_GROUP_REGISTERED,          "Stat group registered %d of %d, number of stats %d\n")
    ILOG_ENTRY(STATS_MON_GROUP_STILL_PROCESSING,    "Stat group %d still being processed when interval expired\n")
    ILOG_ENTRY(STATS_MON_GROUP_NOT_REGISTERED,      "Stats group not registered %08x!?\n")
    ILOG_ENTRY(STATS_MON_GROUP_PRINT_STATS,         "****Printing non-zero statistics: ****\n")
    ILOG_ENTRY(STATS_MON_PRINT_REG_VALUE,           "Reg address %08x value %x\n")
    ILOG_ENTRY(STATS_MON_NO_PARAM,                  "Group has no parameters! \n")
    ILOG_ENTRY(STATS_MON_MULTIPLE_STAT,             "Multiple Stats are running at the same time: %d \n")
ILOG_END(STATS_MON_COMPONENT, ILOG_MINOR_EVENT)

#endif // STATS_MON_LOG_H
