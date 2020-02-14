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
// Hardware driver for the MAC TX and RX blocks.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <options.h>
#include <ilog.h>
#include <stats_mon.h>
#include <timing_timers.h>
#include <i2c.h>
#include <configuration.h>
#include <fiber5g.h>

#include "fiber5g_log.h"
#include "fiber5g_cmd.h"

// #include <uart.h>

// Constants and Macros ###########################################################################
#define I2CD_SFP_FINISAR_DEVICE_ADDRESS             (0x51)
#define I2CD_SFP_FINISAR_DEVICE_SPEED               I2C_SPEED_SLOW
#define I2CD_SFP_FINISAR_DEVICE_SW_PORT             (0xFF) // Not through switch
#define I2CD_SFP_FINISAR_DEVICE_RTLMUX_PORT         (1) // Motherboard

// Register Addresses, first byte of data sent to SFP_FINISAR
#define I2CD_SFP_FINISAR_MEAS_TEMP                  (96)
#define I2CD_SFP_FINISAR_MEAS_VCC                   (98)
#define I2CD_SFP_FINISAR_MEAS_TX_BIAS               (100)
#define I2CD_SFP_FINISAR_MEAS_TX_POWER              (102)
#define I2CD_SFP_FINISAR_MEAS_RX_POWER              (104)

#define SFP_STAT_VALUE_TOLERANCE        (40) // Range in 4% for variance between reads - if greater difference than print value
#define SFP_STATS_POLL_INTERVAL         (1000/STATISTIC_INTERVAL_TICK)        // poll the MAC stats every second

// Relative, should not change -- if changes - print
#define I2CD_FINISAR_STAT_I2C_REG_NOCHNG_REL( \
        i2cdRegAddress, \
        ilogCode)     \
        STATMON_I2C_SFP_READ_PARAM( \
            i2cdRegAddress, \
            STATMON_PARAM_FLAG_RELATIVE_VALUE, \
            FIBER5G_COMPONENT, \
            ilogCode)

// Relative, should change -- if no changes - print
#define I2CD_FINISAR_STAT_I2C_REG_CHANGE_REL( \
        i2cdRegAddress, \
        ilogCode)     \
        STATMON_I2C_SFP_READ_PARAM( \
            i2cdRegAddress, \
            STATMON_PARAM_FLAG_CHANGE | STATMON_PARAM_FLAG_RELATIVE_VALUE, \
            FIBER5G_COMPONENT, \
            ilogCode)

// Absolute, should not change -- if changes - print
#define I2CD_FINISAR_STAT_I2C_REG_NOCHNG_ABS( \
        i2cdRegAddress, \
        ilogCode)     \
        STATMON_I2C_SFP_READ_PARAM( \
            i2cdRegAddress, \
            0, \
            FIBER5G_COMPONENT, \
            ilogCode)

// Aboslute, should change -- if no changes - print
#define I2CD_FINISAR_STAT_I2C_REG_CHANGE_ABS( \
        i2cdRegAddress, \
        ilogCode)     \
        STATMON_I2C_SFP_READ_PARAM( \
            i2cdRegAddress, \
            STATMON_PARAM_FLAG_CHANGE, \
            FIBER5G_COMPONENT, \
            ilogCode)

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
const struct I2cDevice i2cDeviceSfpFinisar =
{
    .deviceAddress = I2CD_SFP_FINISAR_DEVICE_ADDRESS,
    .speed = I2CD_SFP_FINISAR_DEVICE_SPEED,
    .port = I2C_MUX_MOTHERBOARD
};

#if 0
static const StatI2cSfpReg I2cStatU8Regs[] =
{
//    MAC_STAT_FPGA_REG_NOCHNG_REL( bb_chip_link_layer_rx_out_fsm_rollover_ADDRESS,              LL_RX_MAC_STATS0_OUT_FSM_ROLLOVER_COUNT),
};

static uint8_t I2cStatStatU8data[ARRAYSIZE(I2cStatU8Regs)];
#endif

static const StatI2cSfpReg I2cStatU16Regs[] =
{
    I2CD_FINISAR_STAT_I2C_REG_NOCHNG_ABS(I2CD_SFP_FINISAR_MEAS_TEMP, SFP_FINISAR_STATS_MEAS_TEMP),
    I2CD_FINISAR_STAT_I2C_REG_NOCHNG_ABS(I2CD_SFP_FINISAR_MEAS_VCC, SFP_FINISAR_STATS_MEAS_VCC),
    I2CD_FINISAR_STAT_I2C_REG_NOCHNG_ABS(I2CD_SFP_FINISAR_MEAS_TX_BIAS, SFP_FINISAR_STATS_MEAS_TX_BIAS),
    I2CD_FINISAR_STAT_I2C_REG_NOCHNG_ABS(I2CD_SFP_FINISAR_MEAS_TX_POWER, SFP_FINISAR_STATS_MEAS_TX_POWER),
    I2CD_FINISAR_STAT_I2C_REG_NOCHNG_ABS(I2CD_SFP_FINISAR_MEAS_RX_POWER, SFP_FINISAR_STATS_MEAS_RX_POWER),
};

static uint16_t I2cStatU16data[ARRAYSIZE(I2cStatU16Regs)];

#if 0
static const StatI2cSfpReg I2cStatU32Regs[] =
{
//    MAC_STAT_FPGA_REG_NOCHNG_REL( bb_chip_link_layer_rx_pause_frame_ADDRESS,                   LL_RX_MAC_STATS0_PAUSE_FRAME_COUNT),
};

static uint32_t I2cStatStatU32data[ARRAYSIZE(I2cStatU32Regs)];

const StatRegistration I2c8bitStats = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_8_BITS,
        MacStatU8Regs,
        MacStatStatU8data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MAC_STATS_POLL_INTERVAL);
#endif

const StatRegistration I2c16bitsStats = STATMON_REGISTRATION_INIT_SFP(
        STATISTIC_DATA_SIZE_16_BITS,
        I2cStatU16Regs,
        I2cStatU16data,
        I2CD_sfpFinisarRegRead,
        STATSMON_ClearStatData,
        SFP_STATS_POLL_INTERVAL);

#if 0
const StatRegistration I2c32bitsStats = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_32_BITS,
        MacStatU32Regs,
        MacStatStatU32data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        MAC_STATS_POLL_INTERVAL);
#endif

struct StatsCallback
{
    StatsGetParam * paramInfo;
    StatValueCallback statCallback;
    void * context;
    uint32_t regValue;
};

// TODO Handle different calls issued without waiting for I2C transactions to finish
static struct StatsCallback statsCallbacks;

static struct
{
    void (*rxPowerReadCallback)(bool);
    TIMING_TimerHandlerT sfpRxPowerTimer;   // timer used to poll the rx power, to see if it is valid
    uint16_t sfpRxPowerThresholdUpper;  // if the power level is above this level, rx is valid
    uint16_t sfpRxPowerThresholdLower;  // if the power level is below this level, rx is invalid
    uint16_t sfpRxPower;                // the last read rx power level
    uint8_t dataBuffer[2];
    uint8_t powerPollData[2];           // data buffer used when polling Rx power
    uint8_t sfpRxPowerPollingPeriod;    // period used when looking at the Rx power
    uint8_t debounceCounter;            // used to debounce the rx power when it becomes valid
    uint8_t sfpRxPowerDebounceCount;    // if debounceCounter over this count, above threshold becomes true
    bool sfpRxPowerAboveThreshold;      // true if Rx power is valid, false if not
} sfpFinisarContext;


// Static Function Declarations ###################################################################
static void I2CD_sfpFinisarStatsReadCallback(uint8_t* data, uint8_t byteCount);
static void I2CD_sfpFinisarRxPowerReadCallback(uint8_t* data, uint8_t byteCount);
static void I2CD_sfpFinisarRxPowerTimerDone(void);

// Exported Function Definitions ##################################################################

//#################################################################################################
// Setup local struct with I2C handle and interface pointer for I2C access
//
// Parameters:
//      handle              - i2c handle for device through mux
//      interface           - access to i2c functions
//      notifyWriteCompleteHandler - call back when configuration completes
// Return:
// Assumptions:
//      * This function will work with a localized static variable to track operation state.
//      * Rather than have a series of functions as callbacks for all the
//      * writes, use this function update the state variable, permitting state
//      * machine state change operation
//#################################################################################################
void I2CD_sfpFinisarInit(void)
{
//    STATSMON_RegisterStatgroup(&I2c8bitStats);
    STATSMON_RegisterStatgroup(&I2c16bitsStats);
//    STATSMON_RegisterStatgroup(&I2c32bitsStats);
}


//#################################################################################################
//  Initialize Rx signal polling
//
// Parameters:
// Return:
// Assumptions:  Call only once, at startup
//
//#################################################################################################
void I2CD_sfpFinisarRxPowerPollingInit(
    void (*callback)(bool aboveThreshold))
{
    ConfigFiberParams *fiberParams = &(Config_GetBuffer()->fiberParams);

    if(Config_ArbitrateGetVar(CONFIG_VAR_FIBER_PARAMS, fiberParams))
    {
    sfpFinisarContext.rxPowerReadCallback = callback;
        sfpFinisarContext.sfpRxPowerThresholdLower = fiberParams->sfpRxPowerThresholdLower;
        sfpFinisarContext.sfpRxPowerThresholdUpper = fiberParams->sfpRxPowerThresholdUpper;
        sfpFinisarContext.sfpRxPowerPollingPeriod = fiberParams->sfpRxPowerTimeoutMs;
        sfpFinisarContext.sfpRxPowerDebounceCount = fiberParams->sfpRxPowerDebounceCount;

    sfpFinisarContext.sfpRxPowerTimer = TIMING_TimerRegisterHandler(
        &I2CD_sfpFinisarRxPowerTimerDone,
        true,
        sfpFinisarContext.sfpRxPowerPollingPeriod);
    }
}


//#################################################################################################
// Enable RxPower polling
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_sfpFinisarRxPowerPollingEnable(void)
{
//    UART_printf("I2CD_sfpFinisarRxPowerPollingEnable\n");

    sfpFinisarContext.sfpRxPower = 0;
    sfpFinisarContext.sfpRxPowerAboveThreshold = false; // assume the signal is invalid to start
    sfpFinisarContext.debounceCounter = 0;
    TIMING_TimerStart(sfpFinisarContext.sfpRxPowerTimer);
    I2CD_sfpFinisarStartStatsMonitor();
}


//#################################################################################################
// Disable RxPower polling
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_sfpFinisarRxPowerPollingDisable(void)
{
//    UART_printf("I2CD_sfpFinisarRxPowerPollingDisable\n");
    TIMING_TimerStop(sfpFinisarContext.sfpRxPowerTimer);
    I2CD_sftFinisarStopStatsMonitor();
}


//#################################################################################################
//  Returns TRUE if the detected signal level is valid
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool I2CD_sfpFinisarRxPowerAboveThreshold(void)
{
    return sfpFinisarContext.sfpRxPowerAboveThreshold;
}


//#################################################################################################
// Start the sfp stats monitor timer, which will periodically check error stats and log them
// when they change.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_sfpFinisarStartStatsMonitor(void)
{
//    STATSMON_StatgroupControl(&I2c8bitStats,   true);
    STATSMON_StatgroupControl(&I2c16bitsStats, true);
//    STATSMON_StatgroupControl(&I2c32bitsStats, true);
}


//#################################################################################################
// Stop the sfp stats monitor timer.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_sftFinisarStopStatsMonitor(void)
{
//    STATSMON_StatgroupControl(&I2c8bitStats,   false);
    STATSMON_StatgroupControl(&I2c16bitsStats, false);
//    STATSMON_StatgroupControl(&I2c32bitsStats, false);
}


//#################################################################################################
// Get Data for stats monitor - I2C callback will call stats monitor with updated data.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint32_t I2CD_sfpFinisarRegRead(
    const StatIndexOffset *pIndexOffset,
    StatsGetParam *paramInfo,
    uint32_t regValue,
    StatValueCallback statCallback,
    void *context)
{
    // store args for I2C callback to use
    statsCallbacks.paramInfo = paramInfo;
    statsCallbacks.statCallback = statCallback;
    statsCallbacks.context = context;
    statsCallbacks.regValue = regValue;
    sfpFinisarContext.dataBuffer[0] = paramInfo->I2cSfpRegRead.i2cRegAddress;
    const uint8_t writeByteCount = 1;

    I2C_WriteReadAsync(
        &i2cDeviceSfpFinisar,
        sfpFinisarContext.dataBuffer,
        writeByteCount,
        2,
        I2CD_sfpFinisarStatsReadCallback
    );

    return(0);  // no value available at this time
}


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################

//#################################################################################################
// Send Data to stats monitor.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void I2CD_sfpFinisarStatsReadCallback(uint8_t* data, uint8_t byteCount)
{
    if (data != NULL)
    {
        // update the FPGA stat value
        if (statsCallbacks.paramInfo->I2cSfpRegRead.commonParam.flags & STATMON_PARAM_FLAG_RELATIVE_VALUE)
        {
            // add in the relative change to the current value we have
            statsCallbacks.regValue += (data[0] << 8) | data[1];
        }
        else
        {
            // just get the absolute value
            // data read MSB then LSB
            // only update if the delta is bigger than the given tolerance
            uint32_t val = (data[0] << 8) | data[1];
            int32_t delta = (int16_t)(val - statsCallbacks.regValue);
            delta = ABSOLUTE_VALUE(delta);
            delta = delta << 10; // multiply by 1024
            if (val != 0)  // avoid divide by zero error
            {
                uint32_t percentDiff = delta/val;
                if (percentDiff > SFP_STAT_VALUE_TOLERANCE)
                {
                    statsCallbacks.regValue = val;
                }
            }
            else
            {
                statsCallbacks.regValue = val;
            }
        }
    }

    // call the given callback with the value
    statsCallbacks.statCallback(
        statsCallbacks.context,
        statsCallbacks.regValue);
}


//#################################################################################################
// Start the sfp stats monitor timer, which will periodically check error stats and log them
// when they change.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_icmdSfpFinisarStartStatsMonitor(void)
{
//    STATSMON_StatgroupControl(&I2c8bitStats,   true);
    STATSMON_StatgroupControl(&I2c16bitsStats, true);
//    STATSMON_StatgroupControl(&I2c32bitsStats, true);
}


//#################################################################################################
// Stop the sfp stats monitor timer.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_icmdSftFinisarStopStatsMonitor(void)
{
//    STATSMON_StatgroupControl(&I2c8bitStats,   false);
    STATSMON_StatgroupControl(&I2c16bitsStats, false);
//    STATSMON_StatgroupControl(&I2c32bitsStats, false);
}


//#################################################################################################
// Sets threshold level for RxPower.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_icmdSfpFinisarSetRxPowerThresholds(uint16_t thresholdLower, uint16_t thresholdUpper)
{
    sfpFinisarContext.sfpRxPowerThresholdLower = thresholdLower;
    sfpFinisarContext.sfpRxPowerThresholdUpper = thresholdUpper;
}


//#################################################################################################
// Sets RxPower polling period.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_icmdSfpFinisarSetRxPowerPollingPeriod(uint32_t pollingPeriod)
{
    sfpFinisarContext.sfpRxPowerPollingPeriod = pollingPeriod;
}


//#################################################################################################
// Handle RxPower read.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void I2CD_sfpFinisarRxPowerReadCallback(uint8_t* data, uint8_t byteCount)
{
    bool oldRxPower = sfpFinisarContext.sfpRxPowerAboveThreshold;

    if (data != NULL)
    {
        sfpFinisarContext.sfpRxPower = *((uint16_t *) data);

        // perform hysteresis check
        //       ________________
        //      /         /
        // ____/_________/
        // if !Abv & RxPwr >= Upper --- Abv = True
        // if Abv & RxPwr <= Lower --- Abv = False
        //
        if ((sfpFinisarContext.sfpRxPower >= sfpFinisarContext.sfpRxPowerThresholdUpper)
            && !sfpFinisarContext.sfpRxPowerAboveThreshold)
        {
            if (sfpFinisarContext.debounceCounter > sfpFinisarContext.sfpRxPowerDebounceCount)
            {
                // Power threshold is valid!  Valid signal detected
                sfpFinisarContext.sfpRxPowerAboveThreshold = true;
            }
            else
            {
                sfpFinisarContext.debounceCounter++;
            }
        }
        else if (sfpFinisarContext.sfpRxPower <= sfpFinisarContext.sfpRxPowerThresholdLower)
        {
            sfpFinisarContext.sfpRxPowerAboveThreshold = false;
            sfpFinisarContext.debounceCounter = 0;
        }

        // only call the callback if the status has changed
        if (oldRxPower != sfpFinisarContext.sfpRxPowerAboveThreshold)
        {
//            UART_printf("I2CD_sfpFinisarRxPowerReadCallback %d value %d\n",
//                sfpFinisarContext.sfpRxPowerAboveThreshold,
//                sfpFinisarContext.sfpRxPower);

            if (sfpFinisarContext.rxPowerReadCallback != NULL)
            {
                (*sfpFinisarContext.rxPowerReadCallback)
                    (sfpFinisarContext.sfpRxPowerAboveThreshold);
            }
        }
    }
    else
    {
//        UART_printf("I2CD_sfpFinisarRxPowerReadCallback() data is null!\n");
    }
}

//#################################################################################################
// Handle Timeout - issue read if still enabled.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void I2CD_sfpFinisarRxPowerTimerDone(void)
{
    sfpFinisarContext.powerPollData[0] = I2CD_SFP_FINISAR_MEAS_RX_POWER;
    const uint8_t writeByteCount = 1;       // only need to write one byte of data to the device

    I2C_WriteReadAsync(
        &i2cDeviceSfpFinisar,
        sfpFinisarContext.powerPollData,
        writeByteCount,
        2,
        I2CD_sfpFinisarRxPowerReadCallback
    );
}



