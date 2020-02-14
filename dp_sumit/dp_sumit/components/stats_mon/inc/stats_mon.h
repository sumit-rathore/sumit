//#################################################################################################
// Icron TechnologyUSB Corporation - Copyright 2017
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef STATISTICS_H
#define STATISTICS_H

// Includes #######################################################################################
#include <itypes.h>

// Constants and Macros ###########################################################################



#define STATISTIC_INTERVAL_TICK     100     // stats tick interval, in milliseconds

#define STATMON_FPGA_READ_PARAM(fpgaAddress, flags, ilogComponent, ilogCode)      \
    { { flags, ilogComponent, ilogCode, 0 }, fpgaAddress }

#define STATMON_FPGA_READ_PARAM_MISC(fpgaAddress, flags, ilogComponent, ilogCode, misc)      \
    { { flags, ilogComponent, ilogCode, misc }, fpgaAddress }

#define STATMON_I2C_SFP_READ_PARAM(i2cRegAddress, flags, ilogComponent, ilogCode)      \
    { { flags, ilogComponent, ilogCode, 0 }, i2cRegAddress }

#define STATMON_AQUANTIA_READ_PARAM(regAddress1, regAddress2, devType, flags, ilogComponent, ilogCode, bitMask, bitOffset) \
    { { flags, ilogComponent, ilogCode, 0 }, regAddress1, regAddress2, devType, bitMask, bitOffset }

#define STATMON_XADC_READ_PARAM(regAddress, flags, ilogComponent, ilogCode) \
    { { flags, ilogComponent, ilogCode, 0 }, regAddress }

#define STATMON_REGISTRATION_INIT(statSize, statEntries, statDataArea, StatReadFunc, statClearFunc, statPollInterval)      \
    { statSize, ARRAYSIZE(statEntries), statPollInterval, STATISTIC_FPGA_REG, (StatsGetParam *)statEntries, StatReadFunc, statClearFunc, statDataArea, {0, 0} }

#define STATMON_REGISTRATION_INIT_OFFSET_INDEX(statSize, statEntries, statDataArea, StatReadFunc, statClearFunc, statPollInterval, index, offset)      \
    { statSize, ARRAYSIZE(statEntries), statPollInterval, STATISTIC_FPGA_REG, (StatsGetParam *)statEntries, StatReadFunc, statClearFunc, statDataArea, {index, offset} }

#define STATMON_REGISTRATION_INIT_AQUANTIA(statSize, statEntries, statDataArea, StatReadFunc, statClearFunc, statPollInterval)      \
    { statSize, ARRAYSIZE(statEntries), statPollInterval, STATISTIC_AQUANTIA_REG, (StatsGetParam *)statEntries, StatReadFunc, statClearFunc, statDataArea, {0, 0} }

#define STATMON_REGISTRATION_INIT_SFP(statSize, statEntries, statDataArea, StatReadFunc, statClearFunc, statPollInterval)      \
    { statSize, ARRAYSIZE(statEntries), statPollInterval, STATISTIC_SFP, (StatsGetParam *)statEntries, StatReadFunc, statClearFunc, statDataArea, {0, 0} }

#define STATMON_REGISTRATION_INIT_XADC(statSize, statEntries, statDataArea, StatReadFunc, statClearFunc, statPollInterval)      \
    { statSize, ARRAYSIZE(statEntries), statPollInterval, STATISTIC_XADC, (StatsGetParam *)statEntries, StatReadFunc, statClearFunc, statDataArea, {0, 0} }

// Stats_mon flag bits definition
#define STATMON_PARAM_FLAG_RELATIVE_VALUE       (1 << 0) // this param is relative, always cleared after a read
#define STATMON_PARAM_FLAG_CHANGE               (1 << 1) // this param should always be changing between reads - print if no change!
#define STATMON_PARAM_FLAG_SET                  (1 << 2) // this param print only if it's one
#define STATMON_PARAM_FLAG_CLR                  (1 << 3) // this param print only if it's zero

#define STATMON_PARAM_ISTATUS_DISPLAY           (1 << 5) // Bit5 indicates it shows ISTATUS
#define STATMON_PARAM_MAJOR_DISPLAY             (1 << 6) // Bit6 indicates it shows major log (display in red)
#define STATMON_PARAM_INDEX_DISPLAY             (1 << 7) // Highest bit indicate using MISC data of params

// Data Types #####################################################################################
typedef void (*StatDisableCallback)();          // Callback function when a stat group is disabled

enum StatisticType
{
    STATISTIC_FPGA_REG,             // this stat is from a FPGA register
    STATISTIC_DRP_REG,              // this stat is from an indirect DRP read
    STATISTIC_AQUANTIA_REG,         // this stat is from an aquantia read
    STATISTIC_SFP,                  //
    STATISTIC_XADC,                 // this stat is from a XADC read
};

//
enum StatisticDataSize // MUST equal number of bytes, see STATSMON_ClearStatData()
{
    STATISTIC_DATA_SIZE_8_BITS  = 1,    // statistic fits into 1 byte
    STATISTIC_DATA_SIZE_16_BITS = 2,    // statistic fits into 2 bytes
    STATISTIC_DATA_SIZE_32_BITS = 4,    // statistic fits into 4 bytes
    STATISTIC_DATA_SIZE_64_BITS = 8,    // statistic fits into 8 bytes
};

// common stat_mon parameter struct for every monitoring module
// Heads up! this sturct should be located at the beginning of each group's param struct.
typedef struct
{
   uint8_t     flags;
   uint8_t     ilogComponent;       // the component this stat came from
   uint8_t     ilogCode;            // the ilog(ISTATUS) code for this stat, referenced to the component
   uint8_t     misc;                // miscellaneous variable (can be used for displaying index, etc)
} StatsCommonParams;

// stat parameter for reading FPGA component
typedef struct
{
    StatsCommonParams commonParam;  // Need to be located at the beginning
    uint32_t    fpgaAddress;        // the address in the FPGA this stat is at
} StatFpgaReg;

// stat parameter for i2cSfp
typedef struct
{
    StatsCommonParams commonParam;  // Need to be located at the beginning
    uint8_t     i2cRegAddress;      // Device register to read - offset within device
} StatI2cSfpReg;

// stat parameter for Aquantia
typedef struct
{
    StatsCommonParams commonParam;  // Need to be located at the beginning
    uint16_t    regAddress1;        // register address to read
    uint16_t    regAddress2;        // register address to read if we need to read two registers to get one value
    uint8_t     devType;            // register region
    uint16_t    bitMask;            // bits field to be monitored (works only for regAddress1)
    uint8_t     bitOffset;          // bits field location from bit0 (works only for regAddress1)
} StatAquantiaReg;

// stat parameter for XADC
typedef struct
{
    StatsCommonParams commonParam;  // Need to be located at the beginning
    uint16_t    drpAddr;            // register address to read
} StatXAdcReg;

// this union defines the params needed to get a statistic
typedef union
{
    StatFpgaReg         FpgaRegRead;            // get a stat directly from one of the FPGA registers
    StatI2cSfpReg       I2cSfpRegRead;          // get stat over I2C
//    StatDrpreg          drpRegAdddress;       // Get a stat indirectly, through a DRP read
    StatAquantiaReg     AquantiaRegRead;        // get a stat from the aquantia chip
    StatXAdcReg         XAdcRegRead;            // get a stat from ADC
} StatsGetParam;

typedef struct
{
    uint16_t                    index;          // when a group shares same base register address, it's used to calculate the group's register address
    uint16_t                    offset;         // when a group uses index, it indicates address offset for calulating register address
} StatIndexOffset;

// function to call with the value of the stat, once it is retrieved
typedef void     (*StatValueCallback)(void *statsContext, uint32_t statValue);

// function to call to get the value of the given stat - when the value is retrieved the callback is called,
// with the given stats context
// returns the updated value
typedef uint32_t (*StatGetFunctionPtr)(const StatIndexOffset *pIndexOffset, StatsGetParam *paramAddress, uint32_t currentValue, StatValueCallback statCallback, void * const context);

// callback registered with the stats monitor to clear the group's stats
typedef void     (*StatsClearFunctionPtr)(void * const statMonContext);


typedef struct
{
    uint8_t                     statSize;           // the size of these stats (1, 2, or 4 bytes)
    uint8_t                     numberOfStats;      // number of stats we are registering
    uint8_t                     scanInterval;       // how long per each scan, in 100ms ticks, 0 is disabled
    uint8_t                     statType;           // info on which type of stat this is

    StatsGetParam               *statLookupParams;  // an array of stats to get
    StatGetFunctionPtr          getStatFunction;    // the function to call to get these stats
    StatsClearFunctionPtr       clearAllStats;      // function to call to clear the stats (clears HW stat registers too)
    void                        *statData;          // the place to store the stat data

    StatIndexOffset             indexOffset;
} StatRegistration;


// Function Declarations ##########################################################################

void STATSMON_Init(void)                                                                __attribute__ ((section(".atext")));
void STATSMON_RegisterStatgroup(const StatRegistration *stats)                          __attribute__ ((section(".atext")));
void STATSMON_StatgroupControl(const StatRegistration *stats, bool enable)              __attribute__ ((section(".atext")));
void STATSMON_StatgroupDisable(const StatRegistration *stats, StatDisableCallback callback) __attribute__ ((section(".atext")));

uint32_t STATSMON_FpgaRegisterRead(
    const StatIndexOffset *pIndexOffset,
    StatsGetParam *paramInfo,
    uint32_t regValue,
    StatValueCallback statCallback,
    void *context);     // left in IRAM - may be called a bunch of times

void STATSMON_ClearStatData( void * const statMonContext)                       __attribute__ ((section(".atext")));

//void STAT_PrintStatistics(StatRegistration *stats)                               __attribute__ ((section(".atext")));

#endif // STATISTICS_H
