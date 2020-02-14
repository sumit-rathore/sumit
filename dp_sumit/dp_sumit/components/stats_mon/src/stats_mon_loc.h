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
#ifndef STATS_MON_LOC_H
#define STATS_MON_LOC_H

// Includes #######################################################################################
#include <stats_mon.h>
#include <timing_timers.h>
#include <callback.h>

// Constants and Macros ###########################################################################

// maximum number of groups that can be registered.
#define STATS_MON_MAX_STATS_GROUPS          33      // Only MCA takes 3x6(ch) + 3(core) = 21

#define STATS_MON_GROUP_NOT_FOUND           0xFF    // return code if we can't find the given handle

#define STATS_MON_GROUP_CURRENT_STAT_IDLE   0xFF    // currentStat: nextStat value when this group is idle

#define STATS_MON_GROUP_FLAG_ENABLED        0x01    // groupFlags: group is enabled

// Data Types #####################################################################################

typedef struct
{
    const StatRegistration  *statsGroup;    // pointer to the parameters for this group
    CallbackHandleT         groupGetStat;   // system callback handle used to get the next stat

    uint8_t                 statTickSlot;   // how many ticks to go before this group is read
    uint8_t                 currentStat;    // the current stat in this group to process, STATS_MON_GROUP_CURRENT_STAT_IDLE if done
    uint8_t                 groupFlags;     // additional controls for this group

    StatDisableCallback     disabledCallback;   // callback after a stat shows last stats and disabled
} StatMonGroup;

struct StatisticsContext
{
    uint8_t                 groupsRegistered;   // number of stats groups registered
    uint8_t                 nextGroup;          // scheduler index for searching next group

    TIMING_TimerHandlerT    statsTickTimer;

    StatMonGroup            group[STATS_MON_MAX_STATS_GROUPS];  // the groups of stats we need to process
};

// Function Declarations ##########################################################################

uint32_t StatsMonGetStatValue(StatMonGroup * const group, const uint8_t statIndex);
uint32_t StatsMonSetStatValue(StatMonGroup * const group, const uint8_t statIndex, uint32_t newValue);

void STATSMON_PrintStats(void)                                  __attribute__ ((section(".atext")));

// Component Level Variables #######################################################################

extern const StatRegistration UlpCore8Stats;

#endif // STATS_MON_LOC_H
