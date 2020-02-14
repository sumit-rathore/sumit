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

//#################################################################################################
// Module Description
//#################################################################################################
// This file contains the implementation of IDT clock generator 5P49V6913 driver for Blackbird
// project.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// The requirement of the driver is listed below:
//
//  For Infocomm Demo Phase:
//  LEX (LEX_v0.tcs):
//      a. Enable 156.25MHz, LVDS, 1.8V VCCO, no spread-spectrum, CLK 2 Output.
//      b. Do not write the 5P49V6913 in any way that will cause glitch in 25MHz output (note that
//         in the TCS file the VCCO voltage is changed from 3.3 to 1.8, but will need to confirm
//         this is glitchless).
//
//  REX (REX_NO_SSC_v0.tcs):
//      a. Enable 156.25MHz, LVDS, 1.8V VCCO, no spread-spectrum, CLK 2 Output.
//      b. Enable 135.00MHz, LVDS, 1.8V VCCO, no spread-spectrum, CLK 1 Output.
//      c. Provide option to enable/disable 135.00MHz spread-spectrum through Hobbes. SSC when
//         enabled should be 31.5KHz, 0.5%, downspread (REX_SSC_v0.tcs).
//      d. Do not write the 5P49V6913 in any way that will cause glitch in 25MHz output (note that
//         in the TCS file the VCCO voltage is changed from 3.3 to 1.8, but will need to confirm this
//         is glitchless).
//
//  Possible Future Requirements:
//      a. May need access of other 5P49V6913 registers through Hobbes for lab/EMC experiments.
//      b. May need driver support for real-time frequency adjustment of 135.00MHz output through
//         fractional output divider register changes to support feedback loop frequency control.
//         This is only a back-up  strategy at this point.
//
// The IDT clock has a default configuration once it powers up. To meet the requirements, the
// driver is to configure diff registers between the default and the required settings through I2C
// transactions. The provided IDT clock GUI software is able to generate a tcs file that contains the
// entire register settings for a particular configuration. Given four tcs files Default_v0.tcs,
// LEX_v0.tcs, REX_NO_SSC_v0.tcs and REX_SSC_v0.tcs, a python script is created to parse the diff
// registers from those tcs files and output to the i2cd_idt_clk_cfg.c file. The i2cd_idt_clk.c
// file is to write the diff registers to the IDT clock generator through I2C transactions.
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <leon_timers.h>
#include <i2c.h>
#include <bb_core.h>
#include <configuration.h>
#include <idt_clk.h>

#include "idt_clk_loc.h"
#include "idt_clk_log.h"

// Constants and Macros ###########################################################################
// The default I2C device address of the IDT clock is 0xD4 indicated on the data sheet. However, it
// is a 7-bit address, shifting right one bit is required.(A bit confusion here as the low level
// I2C driver in Blackbird is using 7bit address, but it doesn't work with 0XD4. I think IDT data
// sheet meant 8-bit address instead.)
#define IDT_CLK_GENERATOR_I2C_ADDR  0x6A
#define IDT_CLK_GENERATOR_I2C_SPEED (I2C_SPEED_FAST)
#define IDT_CLK_I2C_RETRY_LIMIT     5

// Monitor the PLL lock in register 0x9D.
// Bit 7 is a “lock” bit and bit 1 is a “loss of lock” bit.
// So with the PLL unlocked, bit 7 is “0” and bit 1 is “1”. With the PLL locked, bit 7 is “1” and bit 1 is “0”.
#define IDT_CLK_LOCK_ADDRESS        0x9D
#define IDT_CLK_LOCK_MASK           0x82
#define IDT_CLK_LOCK_VALUE          0x80

// Output Divider 1 Fractional Settings' bit1 is Enable Spectrum
#define IDT_CLK_SSC_ADDRESS         0x25
#define IDT_CLK_SSC_ENABLE          0x46
#define IDT_CLK_SSC_DISABLE         0x44

// Turn off VCO monitoring won’t be able to change VCO bands again
#define IDT_CLK_VCO_MONITOR_ADDRESS 0x1D
#define IDT_CLK_VCO_MONITOR_VALUE   0x6D    // Initial value is 0x6F and clear bit 1

// the amount of delay to wait after setting up the IDT chip before the clocks are stable
// (in microseconds)
#define IDT_CLOCK_STABILIZE_DELAY   (10*1000)  // Maximum 10ms for PLL lock. Check IDT datasheet(NOVEMBER 11, 2016) p19

// Data Types #####################################################################################

enum IdtClkCfgResult
{
    IDT_CLK_CFG_SUCCESS,
    IDT_CLK_NO_CFG,
    IDT_CLK_NO_RESPONSE,
    IDT_CLK_CFG_FAILURE,
    NUMBER_CFG_RESULTS
};

typedef void (*IdtClkInitCompletionHandler)();

// Static Function Declarations ###################################################################
static void I2CD_IdtClkResetCfgRegMap(void);
static void I2CD_IdtClkCfgInitBlocking(enum IdtClkCfgType type);
static void I2CD_Idt6914ClkCfgInitBlocking(enum IdtClkCfgType6914 type);
static void I2CD_IdtClkCheckLock(void);
static void I2CD_IdtReadAsyncHandler(uint8_t * data, uint8_t byteCount)     __attribute__((section(".atext")));
static void I2CD_IdtWriteAsyncHandler(uint8_t * data, uint8_t byteCount)    __attribute__((section(".atext")));
static bool I2CD_IdtRexSSCSupport(void)                                     __attribute__((section(".atext")));

// Global Variables ###############################################################################
extern struct IdtClkRegMap divider1Fraction[4];
extern const struct IdtClkConfiguraton idtConfig[NUMBER_CFG_TYPES];
extern uint8_t idtConfiguration[IDT6914_REG_SIZE];


// Static Variables ###############################################################################
const struct I2cDevice deviceIdtClk =
{
    .deviceAddress = IDT_CLK_GENERATOR_I2C_ADDR,
    .speed = IDT_CLK_GENERATOR_I2C_SPEED,
    .port = I2C_MUX_CORE
};

static struct IdtClkContext
{
    // A pointer to a struct IdtClkRegMap array representing the current IDT clock configuration.
    const struct IdtClkRegMap* currIdtClkCfg;

    // the size of the current configuration array pointed by currIdtClkCfg.
    uint8_t regMapSize;

    // Tracking the index of the configuration array element being used for I2C write transaction.
    uint8_t regMapIndex;

    // A function pointer to store the callback that will be called when the configuration is
    // complete.
    IdtClkInitCompletionHandler overallCompletionHandler;

    // data buffer for general read/ write
    uint8_t address;
    uint8_t dataBuffer[2];

} idtClk;

// Exported Function Definitions ##################################################################
//#################################################################################################
// idtConfigurationSetup : Setting IDT chipset according to feature bits
//
// Parameters:
// Return:
// Assumptions:
//###################K##############################################################################
void idtConfigurationSetup(void)
{
    uint32_t hwFeature = bb_core_getFeatures();
    bool isLex = bb_top_IsDeviceLex();
    bool dpEnabledRex = hwFeature & CORE_FEATURE_DP_SOURCE; // DP Enabled REX (Checked by DP_SOURCE)
    bool usb3Enabled = hwFeature & CORE_FEATURE_ULP;        // USB3 Link Partner present;
    bool sscEnabled = I2CD_IdtRexSSCSupport();              // DP SSC is enabled TODO: need to check if this is required

    if(bb_top_getCoreBoardRev() >= BOM_23_00200_A03)        // A03 board use IDT6914
    {
        enum IdtClkCfgType6914 cfgType = IDT_LEX_REX_USB3_CFG;  // Need 40Mhz clock case (Default P1)

        if(dpEnabledRex)
        {
            if(usb3Enabled)
            {   // If this flash variable is set enable hardaware clk to support SSC in rex
                cfgType = sscEnabled ? IDT_REX_USB3_DP_SSC_CFG : IDT_REX_USB3_DP_CFG;
            }
            else
            {    // If this flash variable is set enable hardaware clk to support SSC in rex
                cfgType = sscEnabled ? IDT_REX_USB2_DP_SSC_CFG : IDT_REX_USB2_DP_CFG;
            }
        }
        else if(!usb3Enabled)                               // USB2 must have DP feature
        {
            cfgType = isLex ? IDT_LEX_USB2_CFG : IDT_REX_USB2_DP_CFG;
        }

        I2CD_Idt6914ClkCfgInitBlocking(cfgType);
        ilog_IDT_CLK_COMPONENT_1(ILOG_MAJOR_EVENT, IDT_CLK_CFG_DONE, cfgType);
    }
    else                                                    // A01, A02 board use IDT6913
    {
        // Below IDT6913 configuration will be removed after HW upgrade
        enum IdtClkCfgType cfgType = dpEnabledRex ? IDT_REX_CFG: IDT_LEX_CFG;
        I2CD_IdtClkCfgInitBlocking(cfgType);
        ilog_IDT_CLK_COMPONENT_1(ILOG_MAJOR_EVENT, IDT_CLK_CFG_DONE, cfgType);

        I2CD_IdtClkResetCfgRegMap();
    }
}

//#################################################################################################
// I2CD_IdtReadRegister
// Parameters: enable / disable SSC
//
// Return:
// Assumptions: It's valid under SSC Enabled profile is set
//#################################################################################################
void IDT_CLK_SscControl(bool enable)
{
    ilog_IDT_CLK_COMPONENT_1(ILOG_MINOR_EVENT, IDT_CLK_DP_SSC_MODE, enable);

    idtClk.dataBuffer[0] = IDT_CLK_SSC_ADDRESS;
    idtClk.dataBuffer[1] = enable ? IDT_CLK_SSC_ENABLE : IDT_CLK_SSC_DISABLE;

    I2C_WriteAsync(&deviceIdtClk, idtClk.dataBuffer, 2, NULL);
}


// Component Scope Function Definitions ###########################################################
//#################################################################################################
// I2CD_IdtReadRegister (Icmd)
// Parameters: read address
//
// Return:
// Assumptions:
//#################################################################################################
void I2CD_IdtReadRegister(uint8_t address)
{
    idtClk.address = address;
    idtClk.dataBuffer[0] = address;

    I2C_WriteReadAsync(
        &deviceIdtClk,
        idtClk.dataBuffer,          // data buffer
        1,                          // Number of bytes to write
        1,                          // Number of bytes to read
        &I2CD_IdtReadAsyncHandler
    );
}

//#################################################################################################
// I2CD_IdtWriteRegister (Icmd)
// Parameters: write address, value to write
//
// Return:
// Assumptions:
//#################################################################################################
void I2CD_IdtWriteRegister(uint8_t address, uint8_t value)
{
    idtClk.address = address;
    idtClk.dataBuffer[0] = address;
    idtClk.dataBuffer[1] = value;

    I2C_WriteReadAsync(
        &deviceIdtClk,
        idtClk.dataBuffer,          // data buffer
        2,                          // Number of bytes to write
        1,                          // Number of bytes to read
        &I2CD_IdtWriteAsyncHandler
    );
}


// Static Function Definitions ####################################################################

//#################################################################################################
// I2CD_IdtReadAsyncHandler
// Parameters: data and byte count
//
// Return:
// Assumptions:
//#################################################################################################
static void I2CD_IdtReadAsyncHandler(uint8_t * data, uint8_t byteCount)
{
    ilog_IDT_CLK_COMPONENT_2(ILOG_USER_LOG, IDT_CLK_GENERAL_READWRITE, idtClk.address, data[0]);
}

//#################################################################################################
// I2CD_IdtWriteAsyncHandler
// Parameters: data and byte count
//
// Return:
// Assumptions:
//#################################################################################################
static void I2CD_IdtWriteAsyncHandler(uint8_t * data, uint8_t byteCount)
{
    ilog_IDT_CLK_COMPONENT_2(ILOG_USER_LOG, IDT_CLK_GENERAL_READWRITE, idtClk.address, data[1]);
}

//#################################################################################################
// Reset configuration register map.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void I2CD_IdtClkResetCfgRegMap(void)
{
    idtClk.currIdtClkCfg = NULL;
    idtClk.regMapSize = 0;
    idtClk.regMapIndex = 0;
}

//#################################################################################################
// Initialize IDT Clock Configuration with blocking.
// Parameters:
//      type    - type of IDT clock configuration.
// Return:
// Assumptions:
//#################################################################################################
static void I2CD_IdtClkCfgInitBlocking(enum IdtClkCfgType type)
{
    idtClk.currIdtClkCfg = idtConfig[type].idtClkCfg;
    idtClk.regMapSize = idtConfig[type].regMapSize;
    idtClk.regMapIndex = 0;
    enum IdtClkCfgResult result = IDT_CLK_CFG_SUCCESS;
    uint8_t retryCount = 0;

    if (idtClk.regMapSize == 0)
    {
        // No need to configure the IDT clk as no diff.
        result = IDT_CLK_NO_CFG;
    }
    else
    {
        while (idtClk.regMapIndex != idtClk.regMapSize)
        {
            ilog_IDT_CLK_COMPONENT_3(
                ILOG_DEBUG,
                IDT_CLK_CONFIGURE_REGISTERS,
                idtClk.regMapIndex,
                (idtClk.currIdtClkCfg[idtClk.regMapIndex]).regOffset,
                (idtClk.currIdtClkCfg[idtClk.regMapIndex]).regValue);

            bool writeSuccess = I2C_WriteBlocking(
                &deviceIdtClk,
                (uint8_t*)(&(idtClk.currIdtClkCfg[idtClk.regMapIndex])),
                sizeof(idtClk.currIdtClkCfg[0]));

            // check for errors and retry
            if (writeSuccess)
            {
                retryCount = 0;
                idtClk.regMapIndex++;
            }
            else if (retryCount < IDT_CLK_I2C_RETRY_LIMIT)
            {
                ilog_IDT_CLK_COMPONENT_2(
                    ILOG_MINOR_ERROR,
                    IDT_CLK_CFG_RETRY,
                    idtClk.regMapIndex,
                    retryCount);

                retryCount++;
            }
            else
            {
                retryCount = 0;
                // Fail to configure the IDT clk generator.
                result = (idtClk.regMapIndex == 0) ?
                    IDT_CLK_NO_RESPONSE : IDT_CLK_CFG_FAILURE;
                break;
            }
        }
    }

    iassert_IDT_CLK_COMPONENT_1(result == IDT_CLK_CFG_SUCCESS, IDT_CLK_CFG_FAIL, result);
}


//#################################################################################################
// Initialize IDT Clock Configuration with blocking.
// Parameters:
//      type    - type of IDT clock configuration.
// Return:
// Assumptions:
//#################################################################################################
static void I2CD_Idt6914ClkCfgInitBlocking(enum IdtClkCfgType6914 type)
{
    uint8_t retryCount = 0;
    enum IdtClkCfgResult result = IDT_CLK_CFG_FAILURE;

    I2CD_IdtClkCfgGenerateRegisters(type);      // Load register values into array

    while((result != IDT_CLK_CFG_SUCCESS) && (retryCount < IDT_CLK_I2C_RETRY_LIMIT))
    {
        bool writeSuccess = I2C_WriteBlocking(
            &deviceIdtClk,
            &idtConfiguration[IDT6914_REG_START_OFFSET],
            ARRAYSIZE(idtConfiguration) - IDT6914_REG_START_OFFSET);

        // check for errors and retry
        if (writeSuccess)
        {
            result = IDT_CLK_CFG_SUCCESS;
        }
        else
        {
            ilog_IDT_CLK_COMPONENT_1(ILOG_MINOR_ERROR, IDT_CLK_CFG_6914_RETRY, retryCount);
            retryCount++;
        }
    }

    iassert_IDT_CLK_COMPONENT_1(result == IDT_CLK_CFG_SUCCESS, IDT_CLK_CFG_FAIL, result);

    I2CD_IdtClkCheckLock();                     // Check IDT6914 Clock's lock status and disable VCO monitoring
}


//#################################################################################################
// I2CD_IdtClkCheckLock
//  Check IDT clock Lock status and turn off VCO monitoring not to change VCO bands again
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void I2CD_IdtClkCheckLock(void)
{
    LEON_TimerValueT startTime;
    bool locked = false;
    uint8_t ReadBuffer;
    uint8_t WriteBuffer[2] = { IDT_CLK_VCO_MONITOR_ADDRESS, IDT_CLK_VCO_MONITOR_VALUE };

    LEON_TimerWaitMicroSec(IDT_CLOCK_STABILIZE_DELAY);              // Wait 10ms to finish VCO calibration

    startTime = LEON_TimerRead();

    // check Lock and turn off VCO monitoring. Wait lock maximum 10ms
    while (!locked && (LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) < IDT_CLOCK_STABILIZE_DELAY ))
    {
        ReadBuffer = IDT_CLK_LOCK_ADDRESS;

        I2C_WriteReadBlocking(                                      // Read 0x9D to check Lock Status
            &deviceIdtClk,
            &ReadBuffer,
            1,                                                      // Write 1byte (address)
            1);                                                     // Read 1byte (value)

        if((ReadBuffer & IDT_CLK_LOCK_MASK) == IDT_CLK_LOCK_VALUE)  // Check Value
        {
            locked = true;
            I2C_WriteBlocking(                                      // Disable VCO monitoring
                &deviceIdtClk,
                WriteBuffer,
                ARRAYSIZE(WriteBuffer));
        }
    }

    iassert_IDT_CLK_COMPONENT_0(locked, IDT_CLK_UNLOCKED);          // If IDT clock is not locked within 10ms, need to check this issue
}

//#################################################################################################
// Get SSC information to setup IDT Clock
//
// Parameters:
// Return:
// Assumptions: This is a precondition check for controlling SSC mode of IDT chipset. This function has to
// be true for DP component to control SSC enable/disable. If this bit is false, ssc will always be
// disabled
//#################################################################################################
static bool I2CD_IdtRexSSCSupport(void)
{
    ConfigIdtClkConfig idtClkConfig;
    bool sscEnabled = false;

    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_IDK_CONFIG, &idtClkConfig))
    {
        ilog_IDT_CLK_COMPONENT_1(ILOG_DEBUG, IDT_CLK_SSC_MODE, idtClkConfig.rexSscSupport);
        sscEnabled = idtClkConfig.rexSscSupport;
    }
    return sscEnabled;
}
