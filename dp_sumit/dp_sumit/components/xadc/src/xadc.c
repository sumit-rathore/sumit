//#################################################################################################
// Icron Technology Corporation - Copyright <YEAR_X>, <YEAR_X+N>
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
// The XADC module reads into the FPGA (only, not ASIC) XILINX register to read temperatures and
// voltages and setting alarms for temperatures and voltages
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// The XADC registers are interfaced via DRP protocol
// *    Notes..
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <bb_top_dp.h>
#include <bb_top_regs.h>
#include <stats_mon.h>
#include <configuration.h>
#include <led.h>
#include <xadc.h>
#include <sys_funcs.h>
#include "xadc_cmd.h"
#include "xadc_log.h"
#include <bb_core.h>
// Constants and Macros ###########################################################################
// Refer to UG480 Xilinx Ref Doc, Chapter 2
// ADC values are 12b in length, however they are MSB justified, so bitshift by 4 to the right
// during calculations. Note that we scale these values up in order to retain the fractional parts.
// This is the original code which used fractional valued constants and resulted in
// a bunch of soft-FP code being pulled into our firmware:
//
// #define GET_TEMP_DEG_C(ADC_CODE) ((((ADC_CODE >> 4) * 503.975 / 4096) - 273.15) * 100)
// #define GET_VOLTAGE(ADC_CODE) (((ADC_CODE >> 4) * 3.000 / 4096) * 1000)
//
// These are the integer approximated versions:
// Max absolute error w.r.t. the soft-FP verion = 0.993774 * 0.01 C, at adc_code = 368.
#define GET_TEMP_DEG_C(adc_code) ((50398 * ((adc_code) >> 4) / 4096) - 27315)
// Max absolute error w.r.t. the soft-FP verion = 0.998047 mV, at adc_code = 7056.
#define GET_VOLTAGE(adc_code) (3000 * ((adc_code) >> 4) / 4096)

// Status Register Addresses
#define XADC_SR_ADDR_TEMPERATURE        (0x00) // MSB justified, 12 MSBits are the data from the ADC
#define XADC_SR_ADDR_VCCINT             (0x01) // MSB justified
#define XADC_SR_ADDR_VCCAUX             (0x02) // MSB justified
#define XADC_SR_ADDR_VP_VN              (0x03) // MSB justified
#define XADC_SR_ADDR_VREFP              (0x04) // MSB justified
#define XADC_SR_ADDR_VREFN              (0x05)
#define XADC_SR_ADDR_VCCBRAM            (0x06) // MSB justified, 12 MSBits
#define XADC_SR_ADDR_SUPPLY_A_OFFSET    (0x08) // Calibration coeff for supply sensor offset using ADC A
#define XADC_SR_ADDR_ADC_A_OFFSET       (0x09) // Calibration coeff for ADC A offset
#define XADC_SR_ADDR_ADC_A_GAIN         (0x0A) // Calibration coeff for ADC A gain error
#define XADC_SR_ADDR_VCCPINT            (0x0D) // MSB justified, 12 MSBits
#define XADC_SR_ADDR_VCCPAUX            (0x0E) // MSB justified, 12 MSBits
#define XADC_SR_ADDR_VCCO_DDR           (0x0F) // MSB justified, 12 MSBits
#define XADC_SR_ADDR_SUPPLY_B_OFFSET    (0x30) // Calibration coeff for supply sensor offset using ADC B
#define XADC_SR_ADDR_ADC_B_OFFSET       (0x31) // Calibration coeff for ADC B offset
#define XADC_SR_ADDR_ADC_B_GAIN         (0x32) // Calibration coeff for ADC B gain error
#define XADC_SR_ADDR_FLAG               (0x3F) // General status flag


#define XADC_STATS_POLL_INTERVAL        30

// Display when it changes with accumulating value
#define XADC_STAT_REG_CHANGE_REL_ISTATUS( \
        regAddress, \
        ilogCode) \
        STATMON_XADC_READ_PARAM( \
            regAddress, \
            STATMON_PARAM_FLAG_RELATIVE_VALUE | STATMON_PARAM_ISTATUS_DISPLAY, \
            XADC_COMPONENT, \
            ilogCode) \

// Data Types #####################################################################################
struct DrpAddressValuePair
{
    uint16_t addr;
    uint16_t val;
};

// Static Function Declarations ###################################################################
static uint32_t XAdcReadStatRegister(
    const StatIndexOffset *pIndexOffset,
    StatsGetParam *paramInfo,
    uint32_t regValue,
    StatValueCallback statCallback,
    void *context)                          __attribute__((section(".atext")));
static uint16_t XADC_readVccInt(void)       __attribute__((section(".atext")));
static uint16_t XADC_readVccAux(void)       __attribute__((section(".atext")));
static uint16_t XADC_readVccBram(void)      __attribute__((section(".atext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static const struct DrpAddressValuePair xadcInit[] = {
    {0x40, 0x9000}, // averaging of 16 selected for external channels
    {0x41, 0x2ef0}, // contiuous sequence mode, disable unused alarms, enable calibration
    {0x42, 0x0400}, // set DCLK divides
    {0x48, 0x4701}, // CHSEL1 - enable temp, VCCINT, VCCAUX, VCCBRAM and calibration
    {0x49, 0x000f}, // CHSEL2 - enable aux analog channels 0 - 3
    {0x4A, 0x0000}, // SEQAVG1 disabled
    {0x4B, 0x0000}, // SEQAVG2 disabled
    {0x4C, 0x0000}, // SEQINMODE0
    {0x4D, 0x0000}, // SEQINMODE1
    {0x4E, 0x0000}, // SEQACQ0
    {0x4F, 0x0000}, // SEQACQ1
    {0x50, 0xb5ed}, // temp upper alarm trigger 85C
    {0x51, 0x5999}, // Vccint upper alarm limit 1.0V
    {0x52, 0xA147}, // Vccaux upper alarm limit 1.89V
    {0x53, 0x0000}, // OT upper alarm limit 125C using automatic shutdown
    {0x54, 0xa93a}, // Temp lower alarm reset 60C
    {0x55, 0x5111}, // Vccint lower alarm limit 0.95V
    {0x56, 0x91eb}, // Vccaux lower alarm limit 1.71V
    {0x57, 0xae4e}, // OT lower alarm reset 70C
    {0x58, 0x5999}  // Vccbram upser alarm limit 1.05V
};

static const StatXAdcReg XAdcStatU8Regs[] =                     // Array of monitoring stats for 8bits data
{
    XADC_STAT_REG_CHANGE_REL_ISTATUS(XADC_SR_ADDR_TEMPERATURE, ISTATUS_FPGA_TEMP_WARNING),
};

static uint8_t XAdcStatU8data[ARRAYSIZE(XAdcStatU8Regs)];       // Stat value storage array

static const StatRegistration XAdc8Registration = STATMON_REGISTRATION_INIT_XADC(
        STATISTIC_DATA_SIZE_8_BITS,
        XAdcStatU8Regs,
        XAdcStatU8data,
        XAdcReadStatRegister,
        STATSMON_ClearStatData,
        XADC_STATS_POLL_INTERVAL);

static struct {
    uint8_t tempThresholdWarning;           // warning level display threshold
    uint8_t tempThresholdShutdown;          // shutdown level threshold
    uint8_t oldTemperature;                 // backup temperature not to show log
    bool    tempWarning;                    // indicating temp warning set to release led state only once
} xadcContext;

static const bb_top_drp_drp_en_mask readMask = { .bf.xadc = 1 };

// Exported Function Definitions ##################################################################
//#################################################################################################
// Perform a series of writes based upon Xilinx UG480 Chapter 6 Application Guidelines Example
// to configure the XADC for temperature and voltage monitoring
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void XADC_init(void)
{
    // Initialize the XADC hardware.
    for (uint8_t writeCount = 0; writeCount < ARRAYSIZE(xadcInit); writeCount++)
    {
        bb_top_dpDrpWrite(
            xadcInit[writeCount].addr,
            xadcInit[writeCount].val,
            BB_TOP_DRP_DRP_EN_MASK_XADC_MASK);
    }

    // Load threshold value from backup configuration
    ConfigTemperatureParams *temperatureParams =  &(Config_GetBuffer()->temperatureParams);
    if (Config_ArbitrateGetVar(CONFIG_VAR_TEMPERATURE_PARAMS, temperatureParams))
    {
        // update threshold context not to read everytime
        if (bb_top_isDash3())
        {
            xadcContext.tempThresholdWarning = temperatureParams->fpgaWarnThreshold_3;
            xadcContext.tempThresholdShutdown = temperatureParams->fpgaShutThreshold_3;
        }
        else
        {
            xadcContext.tempThresholdWarning = temperatureParams->fpgaWarnThreshold_2;
            xadcContext.tempThresholdShutdown = temperatureParams->fpgaShutThreshold_2;
        }
        
        ilog_XADC_COMPONENT_2(ILOG_MINOR_EVENT, XADC_FPGA_THRESHOLD, xadcContext.tempThresholdWarning, xadcContext.tempThresholdShutdown);
    }

    // Register stat and run
    STATSMON_RegisterStatgroup(&XAdc8Registration);
    STATSMON_StatgroupControl(&XAdc8Registration, true);
}


//#################################################################################################
// Read current temperature and convert the ADC value using UG480 Chapter 2 temperature sensor
// equation Temp(C) = ADC_CODE * 503.975/4096 - 273.15
//
// Parameters:
// Return:
//      temperature - signed 16-bit value -- x100 so we can include two decimale places
// Assumptions:
//#################################################################################################
int16_t XADC_readTemperature(void)
{
    const uint16_t adc_code = bb_top_drpRead(
        XADC_SR_ADDR_TEMPERATURE, readMask);
    return GET_TEMP_DEG_C(adc_code);
}


//#################################################################################################
// XADC_getVoltageIcmd
// Parameters:
// Return:
// Assumptions: Reads voltage level
//#################################################################################################
void XADC_getVoltageIcmd(void)
{
    {
        const uint16_t volt = XADC_readVccInt();
        ilog_XADC_COMPONENT_2(ILOG_USER_LOG, XADC_READ_VCC_INT, volt / 1000, volt % 1000);
    }

    {
        const uint16_t volt = XADC_readVccAux();
        ilog_XADC_COMPONENT_2(ILOG_USER_LOG, XADC_READ_VCC_AUX, volt / 1000, volt % 1000);
    }

    {
        const uint16_t volt = XADC_readVccBram();
        ilog_XADC_COMPONENT_2(ILOG_USER_LOG, XADC_READ_VCC_BRAM, volt / 1000, volt % 1000);
    }
}


//#################################################################################################
// XADC_getFpgaTempIcmd
// Parameters:
// Return:
// Assumptions: Reads FPGA temperature
//#################################################################################################
void XADC_getFpgaTempIcmd(void)
{
    const int16_t temp = XADC_readTemperature();
    // Temp is x100 so convert to show decimal
    ilog_XADC_COMPONENT_2(ILOG_USER_LOG, XADC_READ_TEMP, temp / 100, temp % 100);
}


//#################################################################################################
// Icmd to update FPGA temerature warning value for -2
//
// Parameters:
//      * val        new warning threshold temperature
// Return:
// Assumptions:
//#################################################################################################
void XADC_setFpgaWarningTemperature_2(uint8_t val)
{
    ConfigTemperatureParams *temperatureParams =  &(Config_GetBuffer()->temperatureParams);

    if (Config_ArbitrateGetVar(CONFIG_VAR_TEMPERATURE_PARAMS, temperatureParams))
    {
        temperatureParams->fpgaWarnThreshold_2 = val;
        xadcContext.tempThresholdWarning = val;         // update warning context variable

        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_TEMPERATURE_PARAMS, temperatureParams);
        ilog_XADC_COMPONENT_1(ILOG_USER_LOG, XADC_SET_TEMP_WARNING_2, val);
    }
}

//#################################################################################################
// Icmd to update FPGA temerature warning value for -3
//
// Parameters:
//      * val        new warning threshold temperature
// Return:
// Assumptions:
//#################################################################################################
void XADC_setFpgaWarningTemperature_3(uint8_t val)
{
    ConfigTemperatureParams *temperatureParams =  &(Config_GetBuffer()->temperatureParams);

    if (Config_ArbitrateGetVar(CONFIG_VAR_TEMPERATURE_PARAMS, temperatureParams))
    {
        temperatureParams->fpgaWarnThreshold_3 = val;
        xadcContext.tempThresholdWarning = val;         // update warning context variable

        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_TEMPERATURE_PARAMS, temperatureParams);
        ilog_XADC_COMPONENT_1(ILOG_USER_LOG, XADC_SET_TEMP_WARNING_3, val);
    }
}

//#################################################################################################
// Icmd to update FPGA temerature shutdown value for -2
//
// Parameters:
//      * val        new shutdown threshold temperature
// Return:
// Assumptions:
//#################################################################################################
void XADC_setFpgaShutdownTemperature_2(uint8_t val)
{
    ConfigTemperatureParams *temperatureParams = &(Config_GetBuffer()->temperatureParams);

    if (Config_ArbitrateGetVar(CONFIG_VAR_TEMPERATURE_PARAMS, temperatureParams))
    {
        temperatureParams->fpgaShutThreshold_2 = val;
        xadcContext.tempThresholdShutdown = val;        // update shutdown context variable

        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_TEMPERATURE_PARAMS, temperatureParams);
        ilog_XADC_COMPONENT_1(ILOG_USER_LOG, XADC_SET_TEMP_SHUTDOWN_2, val);
    }
}

//#################################################################################################
// Icmd to update FPGA temerature shutdown value for -3
//
// Parameters:
//      * val        new shutdown threshold temperature
// Return:
// Assumptions:
//#################################################################################################
void XADC_setFpgaShutdownTemperature_3(uint8_t val)
{
    ConfigTemperatureParams *temperatureParams = &(Config_GetBuffer()->temperatureParams);

    if (Config_ArbitrateGetVar(CONFIG_VAR_TEMPERATURE_PARAMS, temperatureParams))
    {
        temperatureParams->fpgaShutThreshold_3 = val;
        xadcContext.tempThresholdShutdown = val;        // update shutdown context variable

        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_TEMPERATURE_PARAMS, temperatureParams);
        ilog_XADC_COMPONENT_1(ILOG_USER_LOG, XADC_SET_TEMP_SHUTDOWN_3, val);
    }
}

// Component Scope Function Definitions ###########################################################
// Static Function Definitions ####################################################################
//#################################################################################################
// Read ADC stat through drp
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static uint32_t XAdcReadStatRegister(
    const StatIndexOffset *pIndexOffset,
    StatsGetParam *paramInfo,
    uint32_t regValue,
    StatValueCallback statCallback,
    void *context)
{
    uint16_t drpAddr = paramInfo->XAdcRegRead.drpAddr;
    uint32_t retValue = 0;

    const uint16_t adc_code = bb_top_drpRead(drpAddr, readMask);

    if(drpAddr == XADC_SR_ADDR_TEMPERATURE) 
    {
        retValue = GET_TEMP_DEG_C(adc_code) / 100;

        if(retValue >= xadcContext.tempThresholdShutdown)
        {
            ILOG_istatus(ISTATUS_FPGA_TEMP_FAULT, 1, retValue);
            LED_SetLedState(LS_TEMP_FAULT, true);       // set LED fault

            killSystem();

            // stop temperature monitoring
            STATSMON_StatgroupControl(&XAdc8Registration, false);
            return 0;
        }
        else if(retValue < xadcContext.tempThresholdWarning)
        {
            retValue = xadcContext.oldTemperature;      // For below Threshold, keeps old value not to show log
            LED_SetLedState(LS_TEMP_WARN_FPGA, false);
        }
        else                                            // Warning status
        {
            LED_SetLedState(LS_TEMP_WARN_FPGA, true);
        }
    }

    xadcContext.oldTemperature = retValue;              // Backup the last value
    statCallback(context, retValue);
    return 0;
}


//#################################################################################################
// Read current VCC INT voltage and convert the ADC value using UG480 Chapter 2 power supply sensor
// equation Voltage (V) = ADC_CODE * 3V / 4096
//
// Parameters:
// Return:
//      voltage - unsigned 16-bit value -- x100 so we can include two decimale places
// Assumptions:
//#################################################################################################
static uint16_t XADC_readVccInt(void)
{
    const uint16_t adc_code = bb_top_drpRead(
        XADC_SR_ADDR_VCCINT, readMask);
    return GET_VOLTAGE(adc_code);
}


//#################################################################################################
// Read current VCC AUX voltage and convert the ADC value using UG480 Chapter 2 power supply sensor
// equation Voltage (V) = ADC_CODE * 3V / 4096
//
// Parameters:
// Return:
//      voltage - unsigned 16-bit value -- x100 so we can include two decimale places
// Assumptions:
//#################################################################################################
static uint16_t XADC_readVccAux(void)
{
    const uint16_t adc_code = bb_top_drpRead(
        XADC_SR_ADDR_VCCAUX, readMask);
    return GET_VOLTAGE(adc_code);
}


//#################################################################################################
// Read current VCC BRAM voltage and convert the ADC value using UG480 Chapter 2 power supply sensor
// equation Voltage (V) = ADC_CODE * 3V / 4096
//
// Parameters:
// Return:
//      voltage - unsigned 16-bit value -- x100 so we can include two decimale places
// Assumptions:
//#################################################################################################
static uint16_t XADC_readVccBram(void)
{
    const uint16_t adc_code = bb_top_drpRead(
        XADC_SR_ADDR_VCCBRAM, readMask);
    return GET_VOLTAGE(adc_code);
}


