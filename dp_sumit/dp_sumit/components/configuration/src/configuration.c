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
// TODO
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// To create a new configuration variable:
//  1) Create the structure, add it to configuration.h
//  2) Add the structure (as a static variable) in configuration.c; this will be used internally by the
//     module to hold the variable's value
//  3) Update the ConfigurationBuffer union with your new structure; a copy and paste of the structure
//     in configuration.c is ok to do.
//  4) Update the ConfigVariableDefinition table in configuration.c, referencing the structure
//     made in step 2
//  5) Make a default structure in config_defaults.c; add it to the ConfigVarDefaultEntry table
//     (default1_table)
//
//#################################################################################################

// Includes #######################################################################################

#include <ibase.h>
#include <bb_core.h>
#include <configuration.h>

#include "configuration_loc.h"
#include "configuration_log.h"
#ifdef BB_PROFILE
#include <timing_profile.h>
#endif
#include <sys_funcs.h>
#include <uart.h>
#include <event.h>

// Constants and Macros ###########################################################################

#define CONFIG_VAR_ENTRY(name, structure, flags, sources) {flags, name, sources, sizeof(structure), &structure }

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Static Variables ###############################################################################

// Static Methods ###############################################################################
static void calFeatureControl(ConfigBlocksEnable *buffer)                               __attribute__ ((section(".atext")));
static void checkFeatureControlEvent(void)                                              __attribute__ ((section(".atext")));
static void MapAtmelReadtoFeatureEnable(uint8_t *buf, ConfigBlocksEnable *blocksEnabled) __attribute__ ((section(".atext")));
static void Config_CheckFeatureHWSupport(ConfigBlocksEnable *blocksEnabled)             __attribute__ ((section(".atext")));
static bool Config_CheckAFeature(bool featureSW, bool featureHW)                        __attribute__ ((section(".atext")));

// blackbird specific
static ConfigBlocksEnable       featureControl;     // featureEnable && featureMask
static ConfigBlocksEnable       featureMask;        // CONFIG_VARS_BB_FEATURE_MASK - // User's feature selection from Flash
static ConfigBlocksEnable       featureEnable;      // factory feature bits from Atmel
static ConfigBrandName          brandName;          // CONFIG_VAR_BB_BRAND_NAME - the vendor's brand name for this device
static ConfigRandomSeed         randomSeed;         // CONFIG_VAR_BB_PSEUDO_RANDOM_SEED - the seed used for the random number generator
static ConfigDpConfig           dpConfig;           // CONFIG_VAR_BB_DP_CONFIG - miscellaneous configuration bits for DP
static ConfigIdtClkConfig       idtClkConfig;       // CONFIG_VAR_BB_IDK_CONFIG - Rex hardware SSC support
static ConfigUsbConfig          usbConfig;          // CONFIG_VAR_BB_USB_CONFIG - miscellaneous configuration bits for USB 2 & 3
static ConfigUsbExtendedConfig  usbExtendedConfig;  // CONFIG_VAR_BB_USB_EXTENDED_CONFIG - miscellaneous configuration bits for USB 2 & 3, and reserved bits

// network parameters
static ConfigMacAddress         macAddress;         // CONFIG_VAR_NET_MAC_ADDRESS - this device's MAC address
static ConfigIp4Address         devIpAddress;       // CONFIG_VAR_NET_IP_ADDR - IP address of this device.
static ConfigNetPortNumber      devPortNumber;      // CONFIG_VAR_NET_PORT_NUMBER - Port number we are listening on
static ConfigIp4Address         subNetMask;         // CONFIG_VAR_NET_SUBNET_MASK - IP subnet mask
static ConfigIp4Address         defaultGateway;     // CONFIG_VAR_NET_DEFAULT_GATEWAY - IP Default gateway address
static ConfigIp4Address         ipAcquisitionMode;  // CONFIG_VAR_NET_IP_ACQUISITION_MODE - IP address static or dynamic
static ConfigIp4Address         dhcpServerIp;       // CONFIG_VAR_NET_DHCP_SERVER_IP - the DHCP server last used to obtain an IP address.

// GE specific variables
static ConfigGEconfigBits       geConfigBits;       // CONFIG_VAR_GE_CONFIGURATION_BITS - GE's configuration bits
static ConfigGEvhubConfig       USB2vhubSetup;      // CONFIG_VAR_USB2_VHUB_CONFIG - the USB2 vhub configuration

// ULP, Link parameters
static ConfigUlpUsb3ResetParams ulpUsb3ResetParams; // CONFIG_VAR_USB3_RESET_PARAMS
static ConfigFiberParams        fiberParams;        // CONFIG_VAR_FIBER_PARAMS

// static ConfigPhyParams
static ConfigPhyParams          phyParams;          // CONFIG_VAR_PHY_PARAMS

// Temperature threshold parameters
static ConfigTemperatureParams  temperatureParams;  // CONFIG_VAR_TEMPERATURE_PARAMS

// Diagnostic Parameters
static ConfigDiagnosticConfig   diagConfig;

static ConfigurationBuffer      configBuffer;       // storage big enough to hold any variable.

uint32_t featureByte;
// Component scope Variables ######################################################################
// this table sets up the variable for access
const ConfigVariableDefinition configVars[] =
{
    // blackbird specific
    CONFIG_VAR_ENTRY(CONFIG_VARS_BB_FEATURE_MASK,           featureMask,        CONFIG_VAR_NVM_FLASH,           CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VARS_BB_FEATURE_ENABLE,         featureEnable,      0,                              CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VARS_BB_FEATURE_CONTROL,        featureControl,     0,                              CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_BB_BRAND_NAME,              brandName,          0,                              CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_BB_PSEUDO_RANDOM_SEED,      randomSeed,         0,                              CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_BB_DP_CONFIG,               dpConfig,           CONFIG_VAR_NVM_FLASH,           CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_BB_IDK_CONFIG,              idtClkConfig,       CONFIG_VAR_NVM_FLASH,           CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_BB_USB_CONFIG,              usbConfig,          CONFIG_VAR_NVM_FLASH,           CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_BB_USB_EXTENDED_CONFIG,     usbExtendedConfig,  CONFIG_VAR_NVM_FLASH,           CONFIG_VAR_SOURCE_DEFAULTS),
    // network parameters
    CONFIG_VAR_ENTRY(CONFIG_VAR_NET_MAC_ADDRESS,            macAddress,         CONFIG_VAR_NVM_FLASH,           CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_NET_IP_ADDR,                devIpAddress,       0,                              CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_NET_PORT_NUMBER,            devPortNumber,      0,                              CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_NET_SUBNET_MASK,            subNetMask,         0,                              CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_NET_DEFAULT_GATEWAY,        defaultGateway,     0,                              CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_NET_IP_ACQUISITION_MODE,    ipAcquisitionMode,  0,                              CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_NET_DHCP_SERVER_IP,         dhcpServerIp,       0,                              CONFIG_VAR_SOURCE_DEFAULTS),

    // GE specific variables
    CONFIG_VAR_ENTRY(CONFIG_VAR_GE_CONFIGURATION_BITS,      geConfigBits,       CONFIG_VAR_NVM_FLASH,           CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_USB2_VHUB_CONFIG,           USB2vhubSetup,      0,                              CONFIG_VAR_SOURCE_DEFAULTS),

    // ULP, Link parameters
    CONFIG_VAR_ENTRY(CONFIG_VAR_ULP_USB3_RESET_PARAMS,      ulpUsb3ResetParams, CONFIG_VAR_NVM_FLASH,           CONFIG_VAR_SOURCE_DEFAULTS),
    CONFIG_VAR_ENTRY(CONFIG_VAR_FIBER_PARAMS,               fiberParams,        CONFIG_VAR_NVM_FLASH,           CONFIG_VAR_SOURCE_DEFAULTS),

    // PHY Link to contorl parameters : 1G, 2.5G, 5G and 10G, and phy enable
    CONFIG_VAR_ENTRY(CONFIG_VAR_PHY_PARAMS,                 phyParams,          CONFIG_VAR_NVM_FLASH,           CONFIG_VAR_SOURCE_DEFAULTS),

    // Temperature threshold parameters
    CONFIG_VAR_ENTRY(CONFIG_VAR_TEMPERATURE_PARAMS,         temperatureParams,  CONFIG_VAR_NVM_FLASH,           CONFIG_VAR_SOURCE_DEFAULTS),

    // Diagnostic config parameters
    CONFIG_VAR_ENTRY(CONFIG_VAR_DIAG_CONFIG,                diagConfig,         CONFIG_VAR_NVM_FLASH,           CONFIG_VAR_SOURCE_DEFAULTS),
};

const uint8_t configVarsSize = ARRAYSIZE(configVars);

// the status of each variable
uint8_t configVarStatusFlags[ARRAYSIZE(configVars)];
ConfigVariableStatus configVarStatus[ARRAYSIZE(configVars)];

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################
//#################################################################################################
// Initialize the configuration module. Config_ArbitrateSetVar() and Config_ArbitrateGetVar
// are valid after this function returns
//
// Parameters:
// Return:
// Assumptions:  Pin strapping is operational
//#################################################################################################
void Configuration_Init(void)
{
    // need to load ROM defaults first, then pin strapping
    Config_LoadDefaults();

//    config_LoadPinStrapping();

}

//#################################################################################################
// Load the NVM portion of the config variables.  Split into a separate call from Configuration_Init()
// so that configuration can be available before NVM. Wrapper for the internal Load NVM defaults
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void Configuration_LoadNVM(void)
{
    Config_LoadFlash();
}


//#################################################################################################
// IsFeatureEnableNotChanged
//
// Parameters: uint8_t *buf has 32bytes ATMEL encrypt read result
// Return: true (newly read featureEnable is the same with old value)
//         false (newly read featureEnable is different with old value)
// Assumptions:
//#################################################################################################
bool IsFeatureEnableNotChanged(uint8_t *buf)
{
    ConfigBlocksEnable blocksEnabled;
    MapAtmelReadtoFeatureEnable(buf, &blocksEnabled);

    if(!memeq(&blocksEnabled, &featureEnable, sizeof(ConfigBlocksEnable)))
    {
        return false;       // atmel read feature has changed
    }
    else
    {
        return true;        // atmel read feature hasn't changed
    }
}

//#################################################################################################
// Load the ATMEL feature bits and update feature enable & control
//
// Parameters: uint8_t *buf has 32bytes ATMEL encrypt read result
// Return:
//
// Assumptions:
//#################################################################################################
void Config_UpdateFeatures(uint8_t *buf)
{
    ConfigBlocksEnable blocksEnabled;
    MapAtmelReadtoFeatureEnable(buf, &blocksEnabled);
    Config_CheckFeatureHWSupport(&blocksEnabled);

    Config_ArbitrateSetVar(CONFIG_SRC_DEFAULT, CONFIG_VARS_BB_FEATURE_ENABLE, &blocksEnabled);
}


//#################################################################################################
// sets the data associated with the given configuration variable.
//
// Parameters:
// Return: true if successful, false if variable not found, or some error
// Assumptions:
//
//#################################################################################################
bool Config_ArbitrateSetVar(
    const enum ConfigSourceType source,
    const enum ConfigurationVars variableName,
    const void *buffer)
{
    uint8_t index;
    bool result = false;        // assume failure

    uint32_t dataLog;
    memcpy (&dataLog, buffer, sizeof(uint32_t));

    for (index = 0; index < ARRAYSIZE(configVars); index++)
    {
        if (configVars[index].name == variableName)
        {
            // found the variable, now see if we should set the new value
            // TODO: based on the source, see if we should really set this variable
            if ( !memeq(configVars[index].location, buffer, configVars[index].size))
            {
                ilog_CONFIG_COMPONENT_3(ILOG_MINOR_EVENT, CONFIG_SAVE_VARIABLE, source, variableName, dataLog);

                // variable value is different then what we have set - copy over the new value
                memcpy(configVars[index].location, buffer, configVars[index].size);

                // this variable has changed
                configVarStatus[index].source  = source;    // set who changed it
                Config_SaveFlashVar(index);                 // see if it should be saved in NVM

                // check feature control value as feature mask changes
                if( (variableName == CONFIG_VARS_BB_FEATURE_MASK)   ||
                    (variableName == CONFIG_VARS_BB_FEATURE_ENABLE) )
                {
                    checkFeatureControlEvent();
                }
                else
                {
                    ilog_CONFIG_COMPONENT_1(ILOG_MINOR_EVENT, CONFIG_EVENT, variableName);
                    EVENT_Trigger(ET_CONFIGURATION_CHANGE, variableName);
                }
            }
            result = true;
            break; // exit for loop
        }
    }

#ifndef PLUG_TEST
    // Show current featureControl after it is set
    if(variableName == CONFIG_VARS_BB_FEATURE_MASK || variableName == CONFIG_VARS_BB_FEATURE_ENABLE)
    {
        Config_ShowFeatureControlStatus();
    }
#endif //PLUG_TEST
    return(result);
}


//#################################################################################################
// Gets the data associated with the given configuration variable.
//
// Parameters:
// Return: the variable in buffer; true if successful, false if variable not found, or some error
// Assumptions:  buffer is large enough to hold the variable
//
//#################################################################################################
bool Config_ArbitrateGetVar(enum ConfigurationVars variableName, void *buffer)
{
    uint8_t index;
    bool result = false;  // assume failure

    for (index = 0; index < ARRAYSIZE(configVars); index++)
    {
        if (configVars[index].name == variableName)
        {
            // found the variable, now return its value
            memcpy(buffer, configVars[index].location, configVars[index].size);
            result = true;
            break; // exit for loop
        }
    }

    uint32_t dataLog;
    memcpy (&dataLog, buffer, sizeof(uint32_t));
    ilog_CONFIG_COMPONENT_3(ILOG_DEBUG, CONFIG_LOAD_VARIABLE, variableName, dataLog, result);

    return(result);
}

//#################################################################################################
// Gets the data pointer with the given configuration variable
//
// Parameters:
// Return: const pointer to the struct got the given configuration variable
// Note: This function should be used only to return read-only variables.
//#################################################################################################
const void* Config_GetDataPointer(enum ConfigurationVars variableName)
{
    uint8_t index;

    for (index = 0; index < ARRAYSIZE(configVars); index++)
    {
        if (configVars[index].name == variableName)
        {
            // found the variable, now return its value
            return (const void*)configVars[index].location;
        }
    }

    ilog_CONFIG_COMPONENT_0(ILOG_MAJOR_ERROR, CONFIG_GET_DATA_PTR_FAILED);
    return NULL;
}

//#################################################################################################
// returns a buffer that is guaranteed to be big enough for the largest config variable.
// only meant for temporary use - must be released before the thread finishes!
//
// Parameters:
// Return: a pointer to  buffer
// Assumptions: only returns one buffer - don't hold on to it!
//
//#################################################################################################
ConfigurationBuffer * Config_GetBuffer(void)
{
    return (&configBuffer);
}


// Component Scope Function Definitions ###########################################################
//#################################################################################################
// Icmd To check Feature status
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void Configuration_ShowFeaturebits()
{
    ilog_CONFIG_COMPONENT_3(ILOG_USER_LOG, CONFIG_FEATURE_CONTROL, featureControl.USBcontrol, featureControl.DPcontrol, featureControl.MiscControl);
    ilog_CONFIG_COMPONENT_3(ILOG_USER_LOG, CONFIG_FEATURE_ENABLE,  featureEnable.USBcontrol,  featureEnable.DPcontrol,  featureEnable.MiscControl);
    ilog_CONFIG_COMPONENT_3(ILOG_USER_LOG, CONFIG_FEATURE_MASK,    featureMask.USBcontrol,    featureMask.DPcontrol,    featureMask.MiscControl);
}

// Static Function Definitions ####################################################################
//#################################################################################################
// MapAtmelReadtoFeatureEnable
//  Mapping Atmel encrypt read value into ConfigBlocksEnable struct
//
// Parameters: uint8_t *buf has 32bytes ATMEL encrypt read result
//             ConfigBlocksEnable *blocksEnabled where featureEnable will be stored
// Return:
// Assumptions:
//#################################################################################################
static void MapAtmelReadtoFeatureEnable(uint8_t *buf, ConfigBlocksEnable *blocksEnabled)
{
    featureByte = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
    memset(blocksEnabled, 0, sizeof(ConfigBlocksEnable));

    if(featureByte & ATMEL_FEATURE_USB2)
    {
        blocksEnabled->USBcontrol |= BIT_MASK(CONFIG_BLOCK_ENABLE_USB_USB2);
    }
    if(featureByte & ATMEL_FEATURE_USB3)
    {
        blocksEnabled->USBcontrol |= BIT_MASK(CONFIG_BLOCK_ENABLE_USB_USB3);
    }
    if(featureByte & ATMEL_FEATURE_VIDEO)
    {
        blocksEnabled->DPcontrol |= BIT_MASK(CONFIG_BLOCK_ENABLE_DP);
    }
    if(featureByte & ATMEL_FEATURE_LANPORT)
    {
        blocksEnabled->MiscControl |= BIT_MASK(CONFIG_BLOCK_ENABLE_GMII);
    }
    if(featureByte & ATMEL_FEATURE_RS232)
    {
        blocksEnabled->MiscControl |= BIT_MASK(CONFIG_BLOCK_ENABLE_RS232);
    }
}

//#################################################################################################
// check featureControl change & generate event
//
// Parameters: void
// Return:
// Assumptions: Whenever featureEnable, featureMask changes, it should be called
//
//#################################################################################################
static void checkFeatureControlEvent(void)
{
    ConfigBlocksEnable *featureControlBuf =  &(Config_GetBuffer()->featureControl);
    calFeatureControl(featureControlBuf);

    if(!memeq(&featureControl, featureControlBuf, sizeof(ConfigBlocksEnable)))
    {
        //featureControl has changed, update featureControl and generate event
        ilog_CONFIG_COMPONENT_1(ILOG_MINOR_EVENT, CONFIG_EVENT, CONFIG_VARS_BB_FEATURE_CONTROL);

        memcpy(&featureControl, featureControlBuf, sizeof(ConfigBlocksEnable));
        EVENT_Trigger(ET_CONFIGURATION_CHANGE, CONFIG_VARS_BB_FEATURE_CONTROL);
    }
}

//#################################################################################################
// Gets the featureBit data by calculating & operation
//
// Parameters:
// Return: the copied variable in ConfigBlocksEnable buffer
//
//#################################################################################################
static void calFeatureControl(ConfigBlocksEnable *buffer)
{
    uint8_t * pfeatureControl =(uint8_t*)buffer;
    uint8_t * pFeatureMask =(uint8_t*)&featureMask;
    uint8_t * pFeatureEnable =(uint8_t*)&featureEnable;

    for(uint8_t i=0; i< sizeof(ConfigBlocksEnable); i++)
    {
        *(pfeatureControl+i) = *(pFeatureMask+i) & *(pFeatureEnable+i);
    }
}


//#################################################################################################
// Config_CheckFeatureHWSupport
//
// Parameters: ConfigBlocksEnable
// Return:
//
//#################################################################################################
static void Config_CheckFeatureHWSupport(ConfigBlocksEnable *blocksEnabled)
{
    uint32_t hwFeature = bb_core_getFeatures();
    bool checkPass = true;

    union
    {
        uint8_t hwSupportByte;              // To display ilog efficiently, used union structure
        struct
        {
            uint8_t reserved:   5;
            bool usb3Support:   1;
            bool dpSupport:     1;
            bool rs232Support:  1;
        } bf;
    } hwSupport;

    hwSupport.bf.usb3Support = hwFeature & CORE_FEATURE_ULP;
    hwSupport.bf.dpSupport = hwFeature & (CORE_FEATURE_DP_SOURCE | CORE_FEATURE_DP_SINK);
    hwSupport.bf.rs232Support = hwFeature & CORE_FEATURE_RS232_EXT;

    checkPass &= Config_CheckAFeature(blocksEnabled->USBcontrol & BIT_MASK(CONFIG_BLOCK_ENABLE_USB_USB3), hwSupport.bf.usb3Support);
    checkPass &= Config_CheckAFeature(blocksEnabled->DPcontrol & BIT_MASK(CONFIG_BLOCK_ENABLE_DP), hwSupport.bf.dpSupport);
    checkPass &= Config_CheckAFeature(blocksEnabled->MiscControl & BIT_MASK(CONFIG_BLOCK_ENABLE_RS232), hwSupport.bf.rs232Support);

    if(!checkPass)
    {
        ilog_CONFIG_COMPONENT_1(ILOG_MAJOR_ERROR, CONFIG_HW_NOT_SUPPORTED, hwSupport.hwSupportByte);
    // TODO (Temp Blcok)
        // killSystem();
    }
}

//#################################################################################################
// Config_CheckAFeature
//
// Parameters: SW feature, HW feature
// Return: boolean
//
//#################################################################################################
static bool Config_CheckAFeature(bool featureSW, bool featureHW)
{
    return featureSW ? featureHW : true;
}

//#################################################################################################
// Config_CheckAFeature
//
// Parameters: SW feature, HW feature
// Return: boolean
//
//#################################################################################################
uint32_t Config_GetFeatureByte(void)
{
    return featureByte;
}
