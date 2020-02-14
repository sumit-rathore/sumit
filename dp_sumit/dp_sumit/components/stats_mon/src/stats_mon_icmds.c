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
//
// Statistics module iCMD support
//
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>

#include <stats_mon.h>
#include "stats_mon_loc.h"
#include "stats_mon_log.h"


// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// ICMD to print all non zero stats
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void STATSMON_PrintStatsCmd(void)
{
    ilog_STATS_MON_COMPONENT_0(ILOG_USER_LOG, STATS_MON_GROUP_PRINT_STATS);

    STATSMON_PrintStats();
}


//#################################################################################################
// Icmd Disable the LAN port
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void STATSMON_ClearAllStatsCmd(void)
{
}


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################

