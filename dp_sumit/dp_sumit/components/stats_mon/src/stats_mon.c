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

//#################################################################################################
// Module Description
//#################################################################################################
// This file contains statistic helper code for the Lex and Rex.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//
//
//#################################################################################################

// Includes #######################################################################################

#include <ibase.h>
#include <leon_timers.h>
#include <callback.h>

#include <stats_mon.h>
#include "stats_mon_loc.h"
#include "stats_mon_log.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################
static void StatsMonTickHandler(void)                                                       __attribute__ ((section(".atext")));
uint8_t StatsMonFindStatsGroup(const StatRegistration *stats)                               __attribute__ ((section(".atext")));
static void StatsMonPrintValue(const StatIndexOffset *pIndexOffset, const StatsCommonParams *commonParam, const uint32_t statValue) __attribute__ ((section(".atext")));
static void StatsMonGroupGetStatValue(void * const context, void *param);
static void StatsMonGroupStatValueCallback(void * const context, const uint32_t statValue);
static StatsGetParam* getParamsPtr(StatMonGroup * const group, uint8_t currentStat);
static bool StatsMonNeedDisplay(uint8_t flags, uint32_t currentStatValue, const uint32_t newStatValue);
static void StatsScheduler(void);
static bool StatsRun(uint8_t startGroup, uint8_t endGroup);
// Static Variables ###############################################################################

// Component Variables ############################################################################
struct StatisticsContext statsContext;

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize the stats monitor module
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void STATSMON_Init(void)
{
    // setup the poll timer to give us the monitoring ticks to read the stats
    statsContext.statsTickTimer = TIMING_TimerRegisterHandler(
        StatsMonTickHandler,
        true,
        STATISTIC_INTERVAL_TICK);

    TIMING_TimerStart(statsContext.statsTickTimer);
}

//#################################################################################################
// Register a stats monitor group to be scanned by the stats module
//
// Parameters:
// Return:
// Assumptions:
//     * Only called once per group
//
//#################################################################################################
void STATSMON_RegisterStatgroup(const StatRegistration *stats)
{
    ilog_STATS_MON_COMPONENT_3(ILOG_USER_LOG, STATS_MON_GROUP_REGISTERED,
        statsContext.groupsRegistered, STATS_MON_MAX_STATS_GROUPS, (uint32_t)stats->numberOfStats);

    // make sure we haven't exceeded the number of groups we've allocated
    iassert_STATS_MON_COMPONENT_0(
        statsContext.groupsRegistered < STATS_MON_MAX_STATS_GROUPS, STATS_MON_MAX_REGISTERED);

    // ok, add this group into the ones we want to scan
    StatMonGroup * const group = &(statsContext.group[statsContext.groupsRegistered]);

    group->statsGroup = stats;

    // try to make sure that each stat is offset from the rest, to try to spread out CPU loading
    group->statTickSlot = stats->scanInterval + statsContext.groupsRegistered;

    // this group is idle, until the interval expires
    group->currentStat = STATS_MON_GROUP_CURRENT_STAT_IDLE;

    // allocate the callback for this group
    group->groupGetStat =
        CALLBACK_Allocate(
            StatsMonGroupGetStatValue,
            &(statsContext.group[statsContext.groupsRegistered]),
            NULL);

    statsContext.groupsRegistered++;  // update the index for the next group
}

//#################################################################################################
// Enables or disables the stats group specified.  Clears the group's statistic data on an enable
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void STATSMON_StatgroupControl(const StatRegistration *stats, bool enable)
{
    uint8_t groupIndex = StatsMonFindStatsGroup(stats);
    StatMonGroup * const group = &(statsContext.group[groupIndex]);

    if (enable)
    {
        group->disabledCallback = NULL;

        // only enable the group if it isn't already enabled, to allow
        // calling this multiple times with no ill effects
        if((group->groupFlags & STATS_MON_GROUP_FLAG_ENABLED) == 0)
        {
            group->groupFlags |= STATS_MON_GROUP_FLAG_ENABLED;

            // try to make sure that each stat is offset from the rest, to try to spread out CPU loading
            group->statTickSlot = stats->scanInterval + groupIndex;

            if (group->statsGroup->clearAllStats != NULL)
            {
                // clear all the statistics data we have on this group
                group->statsGroup->clearAllStats(group);
            }
        }
    }
    else
    {
        STATSMON_StatgroupDisable(stats, NULL);
    }
}

//#################################################################################################
// Disables the stats group specified.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void STATSMON_StatgroupDisable(const StatRegistration *stats, StatDisableCallback callback)
{
    // don't clear the statistics data here, so the user can still look at the old data
    // while the subsystem is down

    uint8_t groupIndex = StatsMonFindStatsGroup(stats);
    StatMonGroup * const group = &(statsContext.group[groupIndex]);

    group->disabledCallback = callback;

    if(group->disabledCallback != NULL)
    {
        if(group->groupFlags & STATS_MON_GROUP_FLAG_ENABLED)
        {
            group->statTickSlot = 0;
            StatsScheduler();
        }
        else
        {
            callback();     // Disabled stat can't run last stat, directly call the callback
        }
    }
    else
    {
        // if callback is exist, flag will be cleared when it calls callback
        group->groupFlags &= ~STATS_MON_GROUP_FLAG_ENABLED;
    }
}

/*
//#################################################################################################
// Enables or disables the stats group specified
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void STATSMON_ReadStat(const StatRegistration *stats, uint8_t statIndex)
{
    uint8_t groupIndex = StatsMonFindStatsGroup(stats);

    if (enable)
    {
        statsContext.group[groupIndex].groupFlags |= STATS_MON_GROUP_FLAG_ENABLED;
    }
    else
    {
        statsContext.group[groupIndex].groupFlags &= ~STATS_MON_GROUP_FLAG_ENABLED;
    }
}
*/

//#################################################################################################
// Function to read a standard FPGA register
//
// Parameters:
// Return:
// Assumptions:
//     * this function is only meant for those FPGA registers where a simple read will suffice
//
//#################################################################################################
uint32_t STATSMON_FpgaRegisterRead(
    const StatIndexOffset *pIndexOffset,
    StatsGetParam *paramInfo,
    uint32_t regValue,
    StatValueCallback statCallback,
    void *context)
{
    StatFpgaReg *fpgaStatParams = (StatFpgaReg *)paramInfo;

    uint32_t currentRegValue = *(uint32_t *)(fpgaStatParams->fpgaAddress + pIndexOffset->index * pIndexOffset->offset);

    // update the FPGA stat value
    if (paramInfo->FpgaRegRead.commonParam.flags & STATMON_PARAM_FLAG_RELATIVE_VALUE)
    {
        // add in the relative change to the current value we have
        regValue += currentRegValue;

        if (currentRegValue != 0)
        {
            ilog_STATS_MON_COMPONENT_2(ILOG_DEBUG, STATS_MON_PRINT_REG_VALUE, fpgaStatParams->fpgaAddress, currentRegValue);
        }
    }
    else
    {
        // just get the absolute value
        regValue = currentRegValue;
    }

    if (statCallback != NULL)
    {
        // call the given callback with the value
        statCallback(context, regValue);
    }

    return (regValue);  // return the latest value
}

//#################################################################################################
// Function to read a standard FPGA register
//
// Parameters:
// Return:
// Assumptions:
//     * this function is only meant for those FPGA registers where a simple read will suffice
//
//#################################################################################################
void STATSMON_ClearStatData( void * const statMonContext)
{
    StatMonGroup * const group = statMonContext;

    uint32_t dataSize = group->statsGroup->statSize * group->statsGroup->numberOfStats;

    memset(group->statsGroup->statData, 0, dataSize);

    // now read the registers to clear them (only for FPGA registers)
    if (group->statsGroup->statType == STATISTIC_FPGA_REG)
    {
        StatFpgaReg *statFpgaReg = (StatFpgaReg *)(group->statsGroup->statLookupParams);

        for (int regNum = 0; regNum < group->statsGroup->numberOfStats; regNum++)
        {
            *(uint32_t volatile *)(statFpgaReg[regNum].fpgaAddress);
        }
    }
}

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Find the given stats group in our list
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
uint8_t StatsMonFindStatsGroup(const StatRegistration *stats)
{
    for (int i = 0; i < statsContext.groupsRegistered; i++)
    {
        if (statsContext.group[i].statsGroup == stats)
        {
            return (i);  // return the index belonging to this stats group
        }
    }

    ifail_STATS_MON_COMPONENT_1(STATS_MON_GROUP_NOT_REGISTERED, (uint32_t)stats);

    // shouldn't get here because of the ifail, but just to make the compiler happy
    return (STATS_MON_GROUP_NOT_FOUND);
}

//#################################################################################################
// Gets the current stat value from the data array given to us by the client
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
uint32_t StatsMonGetStatValue(StatMonGroup * const group, const uint8_t statIndex)
{
    uint32_t currentValue = 0;

    if (statIndex < group->statsGroup->numberOfStats)
    {
        switch(group->statsGroup->statSize)
        {
            case STATISTIC_DATA_SIZE_8_BITS:    // statistic fits into 1 byte
                currentValue = ((uint8_t *)(group->statsGroup->statData))[statIndex];
                break;

            case STATISTIC_DATA_SIZE_16_BITS:    // statistic fits into 2 bytes
                currentValue = ((uint16_t *)(group->statsGroup->statData))[statIndex];
                break;

            case STATISTIC_DATA_SIZE_32_BITS:    // statistic fits into 4 bytes
                currentValue = ((uint32_t *)(group->statsGroup->statData))[statIndex];
                break;

            case STATISTIC_DATA_SIZE_64_BITS:    // statistic fits into 8 bytes
            default:
                break;
        }
    }
    else
    {

    }

    return (currentValue);
}

//#################################################################################################
// Gets the current stat value from the data array given to us by the client
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
uint32_t StatsMonSetStatValue(StatMonGroup * const group, const uint8_t statIndex, uint32_t newValue)
{
    if (statIndex < group->statsGroup->numberOfStats)
    {
        switch(group->statsGroup->statSize)
        {
            case STATISTIC_DATA_SIZE_8_BITS:    // statistic fits into 1 byte
                newValue &= UINT8_MAX;
                ((uint8_t *)(group->statsGroup->statData))[statIndex] = newValue;
                break;

            case STATISTIC_DATA_SIZE_16_BITS:    // statistic fits into 2 bytes
                newValue &= UINT16_MAX;
                ((uint16_t *)(group->statsGroup->statData))[statIndex] = newValue;
                break;

            case STATISTIC_DATA_SIZE_32_BITS:    // statistic fits into 4 bytes
                newValue &= UINT32_MAX;
                ((uint32_t *)(group->statsGroup->statData))[statIndex] = newValue;
                break;

            case STATISTIC_DATA_SIZE_64_BITS:    // statistic fits into 8 bytes
            default:
                break;
        }
    }
    else
    {

    }

    return (newValue);
}

//#################################################################################################
// ICMD to print all non zero stats
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void STATSMON_PrintStats(void)
{
    for (int groupIndex = 0; groupIndex < statsContext.groupsRegistered; groupIndex++)
    {
        StatMonGroup * const group = &(statsContext.group[groupIndex]);
        const int maxGroupStats = group->statsGroup->numberOfStats;

        for (int statIndex = 0; statIndex < maxGroupStats; statIndex++)
        {
            const uint32_t currentValue = StatsMonGetStatValue(group, statIndex);
            const StatsCommonParams *commonParamInfo = (StatsCommonParams *)getParamsPtr(group, statIndex);

            // print out this stat's value
            if (currentValue != 0)
            {
                StatsMonPrintValue(&group->statsGroup->indexOffset, commonParamInfo, currentValue);
            }
        }
    }
}


// Static Function Definitions ####################################################################

//#################################################################################################
// Handles a timer tick event - scans all stats groups, and processes those that are ready
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void StatsMonTickHandler(void)
{
    bool runScheduler = false;

    for (int i = 0; i < statsContext.groupsRegistered; i++)
    {
        if (statsContext.group[i].groupFlags & STATS_MON_GROUP_FLAG_ENABLED)
        {
            if (statsContext.group[i].statTickSlot != 0)
            {
                statsContext.group[i].statTickSlot--;
                if(statsContext.group[i].statTickSlot == 0)     // If a stat is ready to run, ask scheduler
                {
                    runScheduler = true;                        // Don't call scheduler here. just set a flag and decrease other group's tick as well
                                                                // To prevent group 0 from having higher priority when tick timing is the same
                }
            }
        }
    }

    if(runScheduler)
    {
        StatsScheduler();
    }
}


//#################################################################################################
// StatsScheduler
//  run stats one by one
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void StatsScheduler(void)
{
    uint8_t statProcessing = 0;

    for (int i = 0; i < statsContext.groupsRegistered; i++)
    {
        if ((statsContext.group[i].groupFlags & STATS_MON_GROUP_FLAG_ENABLED)
            && (statsContext.group[i].currentStat != STATS_MON_GROUP_CURRENT_STAT_IDLE))
        {
            statProcessing++;
        }
    }

    iassert_STATS_MON_COMPONENT_1(statProcessing <= 1, STATS_MON_MULTIPLE_STAT, statProcessing);

    // If there's no running stat, start runinning from next stat
    if(statProcessing == 0)
    {
        if(!(StatsRun(statsContext.nextGroup, statsContext.groupsRegistered)))
        {
            StatsRun(0, statsContext.nextGroup);
        }
    }
}


//#################################################################################################
// StatsRun
//  search enabled & idle stat group and run it
//
// Parameters: startGroup index, end Group index
// Return:
// Assumptions:
//
//#################################################################################################
static bool StatsRun(uint8_t startGroup, uint8_t endGroup)
{
    bool runStat = false;

    for (uint8_t i = startGroup; i < endGroup; i++)
    {
        if (statsContext.group[i].groupFlags & STATS_MON_GROUP_FLAG_ENABLED)
        {
            if (statsContext.group[i].statTickSlot == 0)
            {
                // update the statistics for this group, if it isn't already in progress
                if (statsContext.group[i].currentStat == STATS_MON_GROUP_CURRENT_STAT_IDLE)
                {
                    statsContext.group[i].currentStat = 0;  // ok, start with the first stat
                    statsContext.nextGroup = i+1;           // update next stat to search

                    runStat = true;
                    CALLBACK_Schedule(statsContext.group[i].groupGetStat);
                    break;                                  // Do not search next stat
                }
                else
                {
                    ilog_STATS_MON_COMPONENT_1(ILOG_MINOR_EVENT, STATS_MON_GROUP_STILL_PROCESSING, i);
                }
            }
        }
    }

    return runStat;
}


//#################################################################################################
// Return parameter pointer for a group
//
// Parameters:  group (current monitoring group), currentStat (index of stat for the group)
//              getCommonParam (choose common param or entire param)
//
// Return:      address of param
// Assumptions:
//
//#################################################################################################
static StatsGetParam* getParamsPtr(StatMonGroup * const group, uint8_t currentStat)
{
    StatsGetParam *paramsPtr = NULL;

    switch(group->statsGroup->statType)
    {
        case STATISTIC_FPGA_REG:
        {
            StatFpgaReg *pFpgaParam = (StatFpgaReg *)(group->statsGroup->statLookupParams);
            paramsPtr = (StatsGetParam*)&(pFpgaParam[currentStat]);
            break;
        }
        case STATISTIC_DRP_REG:
            break;
        case STATISTIC_AQUANTIA_REG:
        {
            StatAquantiaReg *pAquantiaParam = (StatAquantiaReg *)(group->statsGroup->statLookupParams);
            paramsPtr = (StatsGetParam*)&(pAquantiaParam[currentStat]);
            break;
        }
        case STATISTIC_SFP:
        {
            StatI2cSfpReg *pi2cSfpParam = (StatI2cSfpReg *)(group->statsGroup->statLookupParams);
            paramsPtr = (StatsGetParam*)&(pi2cSfpParam[currentStat]);
            break;
        }
        case STATISTIC_XADC:
        {
            StatXAdcReg *pXAdcParam = (StatXAdcReg *)(group->statsGroup->statLookupParams);
            paramsPtr = (StatsGetParam*)&(pXAdcParam[currentStat]);
            break;
        }
        default:
            break;
    }

    iassert_STATS_MON_COMPONENT_0(paramsPtr != NULL, STATS_MON_NO_PARAM);
    return paramsPtr;
}

//#################################################################################################
// System callback that will get the current stat
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void StatsMonGroupGetStatValue(void *context, void *param)
{
    StatMonGroup * const group = context;

    // get this stat's value
    group->statsGroup->getStatFunction(
        &group->statsGroup->indexOffset,
        getParamsPtr(group, group->currentStat),
        StatsMonGetStatValue(group, group->currentStat),
        StatsMonGroupStatValueCallback,
        group);
}


//#################################################################################################
// Called when there is a new value for the group's current stat.  Will schedule another system
// callback to get the next value if we aren't done.
//
// Parameters:
// Return:  The truncated value
// Assumptions:
//
//#################################################################################################
static void StatsMonGroupStatValueCallback(void * const context, const uint32_t newStatValue)
{
    bool    groupStatDone = false;
    StatMonGroup * const group = context;
    uint32_t currentStatValue =StatsMonGetStatValue(group, group->currentStat);

    if (group->groupFlags & STATS_MON_GROUP_FLAG_ENABLED)
    {
        const StatsCommonParams *commonParamInfo = (StatsCommonParams *)getParamsPtr(group, group->currentStat);
        uint8_t flags = commonParamInfo->flags;

        if(StatsMonNeedDisplay(flags, currentStatValue, newStatValue))
        {
            // print out the new value...
            StatsMonPrintValue(&group->statsGroup->indexOffset, commonParamInfo, newStatValue);

            // ... and save it
            StatsMonSetStatValue(group, group->currentStat, newStatValue);
        }

        group->currentStat++;   // go on to the next stat

        if (group->currentStat < group->statsGroup->numberOfStats)
        {
            // not done processing stats, schedule another read
            CALLBACK_Schedule(group->groupGetStat);
        }
        else
        {
            // this is called when it disabled stat with checking the last stat
            if(group->disabledCallback != NULL)
            {
                group->groupFlags &= ~STATS_MON_GROUP_FLAG_ENABLED;
                group->disabledCallback();
                group->disabledCallback = NULL;
            }

            groupStatDone = true;
        }
    }
    else
    {
        groupStatDone = true;
    }

    if(groupStatDone)
    {
        // this group is disabled - go back to idle
        group->currentStat = STATS_MON_GROUP_CURRENT_STAT_IDLE;
        group->statTickSlot = group->statsGroup->scanInterval;
        StatsScheduler();
    }
}

//#################################################################################################
// Decide to Display stat
//      STATMON_PARAM_FLAG_SET or STATMON_PARAM_FLAG_CLR has a higher priority to check value 1 or 0
//      If those bits are not set, then check if it's changed or not
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static bool StatsMonNeedDisplay(uint8_t flags, uint32_t currentStatValue, const uint32_t newStatValue)
{
    bool needDisplay = false;

    // If STATMON_PARAM_FLAG_SET or STATMON_PARAM_FLAG_CLR is set, don't check the other case
    if(flags & (STATMON_PARAM_FLAG_SET | STATMON_PARAM_FLAG_CLR))
    {
        // if STATMON_PARAM_FLAG_SET or STATMON_PARAM_FLAG_CLR is set and new value is 1 or 0, print out the value
        needDisplay = ( ((flags & STATMON_PARAM_FLAG_SET) && (newStatValue == 1))
                        || ((flags & STATMON_PARAM_FLAG_CLR) && (newStatValue == 0)) );
    }
    else
    {
        // this complicated if basically checks if the variable should change (STATMON_PARAM_FLAG_CHANGE == 1)
        // and it doesn't change, print out the value, or,
        // if the variable shouldn't change (STATMON_PARAM_FLAG_CHANGE == 0) and it does change, print out the value
//        needDisplay = !( (flags & STATMON_PARAM_FLAG_CHANGE) ^ (newStatValue == currentStatValue));
        needDisplay = (    !(flags & STATMON_PARAM_FLAG_CHANGE) && (newStatValue != currentStatValue))
                       || ( (flags & STATMON_PARAM_FLAG_CHANGE) && (newStatValue == currentStatValue));
    }

    return needDisplay;
}

//#################################################################################################
// Called when there is a new value for the group's current stat.  Will schedule another system
// callback to get the next value if we aren't done.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void StatsMonPrintValue(const StatIndexOffset *pIndexOffset, const StatsCommonParams *commonParam, const uint32_t statValue)
{
    uint8_t ilogComponent = commonParam->ilogComponent;
    uint8_t ilogCode = commonParam->ilogCode;
    uint8_t flags = commonParam->flags;
    uint8_t index = pIndexOffset->index;

    if(flags & STATMON_PARAM_ISTATUS_DISPLAY)
    {
        // ISTAUS display
        ILOG_istatus(ilogCode, 1, statValue);
    }
    else
    {
        // ILOG display
        if(flags & STATMON_PARAM_INDEX_DISPLAY)
        {
            const uint32_t ilogHeader = _ilog_header(
                flags & STATMON_PARAM_MAJOR_DISPLAY ? ILOG_MINOR_ERROR : ILOG_MINOR_EVENT,
                ilogComponent,
                ilogCode,
                2);

            _ilog(ilogHeader, index, statValue, 0);
        }
        else
        {
            const uint32_t ilogHeader = _ilog_header(
                flags & STATMON_PARAM_MAJOR_DISPLAY ? ILOG_MINOR_ERROR : ILOG_MINOR_EVENT,
                ilogComponent,
                ilogCode,
                1);

            _ilog(ilogHeader, statValue, 0, 0);
        }
    }

}

