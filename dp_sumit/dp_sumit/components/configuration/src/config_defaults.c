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
// TODO
//#################################################################################################

// Includes #######################################################################################

#include <ibase.h>
#include <configuration.h>

#include "configuration_log.h"
#include "configuration_loc.h"
#ifdef BB_PROFILE
#include <timing_profile.h>
#endif

// Constants and Macros ###########################################################################
#define CONFIG_DEFAULT_BRAND_NAME       "Test BlackBird"
#define CONFIG_DEFAULT_PORT_NUMBER      0x17f9
#define CONFIG_DEFAULT_RANDOM_SEED      0xdeadbeefUL

// taken from GE storage_vars.h
enum ConfigBitOffsets
{
                                                              // ASIC   FPGA            ASIC Variant Restriction
    TOPLEVEL_SUPPORT_USB2_HISPEED_OFFSET                     = 0, // H      Core:H/Box:S    Y
    TOPLEVEL_SUPPORT_MSA_OFFSET                              = 1, // S      S               N
    TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET                        = 2, // S      S               Y
    TOPLEVEL_USE_ETHERNET_FRAMING_OFFSET                     = 3, // H      Core:H&&S/Box:S Y
    TOPLEVEL_ENET_PHY_MII_SUPPORT_OFFSET                     = 4, // C(1)   S               N
    TOPLEVEL_ENET_PHY_GMII_SUPPORT_OFFSET                    = 5, // C(1)   S               N
    TOPLEVEL_USE_BCAST_NET_CFG_PROTO_OFFSET                  = 6, // S      S               N
    TOPLEVEL_ENABLE_VHUB_OFFSET                              = 7, // S      S               Y
    TOPLEVEL_ENABLE_DCF_OFFSET                               = 8, // S      S               Y
    TOPLEVEL_BLOCK_MASS_STORAGE_OFFSET                       = 9, // S      S               Y
    TOPLEVEL_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET                = 10,// S      S               Y
    TOPLEVEL_DEPRECATED_REFUSE_PAIRING_WITH_UNBRANDED_OFFSET = 11,// C(0)   C(0)            N
    TOPLEVEL_DEPRECATED_REFUSE_PAIRING_WITH_LEGACY_OFFSET    = 12,// C(0)   C(0)            N
    TOPLEVEL_ENABLE_LEX_EXTERNAL_CLOCK_OFFSET                = 13,// H      S               N
    TOPLEVEL_DISABLE_REX_EXTERNAL_CLOCK_OFFSET               = 14,// H      S               N
    TOPLEVEL_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET      = 15,// S      S               Y
    TOPLEVEL_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET  = 16,// S      S               Y
    TOPLEVEL_DISABLE_NETWORK_CFG_CHANGES_OFFSET              = 17,// S      S               Y
    TOPLEVEL_ENABLE_DHCP_OPTION_60_OFFSET                    = 18,// S      S               Y

    TOPLEVEL_NUM_CONFIG_BITS
};

// VHub options - taken from GE options.h
// ------------
#define NUM_OF_VPORTS   1
#define VHUB_VENDOR_ID  (0x089d) //TODO: remove for branded products
#define VHUB_PRODUCT_ID (0x0001)

// Data Types #####################################################################################

typedef struct _ConfigVarDefaultEntry
{
    enum ConfigurationVars  variable;       // the variable we want to set to a non zero value
    const void              *defaultValue;  // where the default value is

} ConfigVarDefaultEntry;


// Static Function Declarations ###################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

const ConfigBlocksEnable        dflt1_featureMask =      // CONFIG_VARS_BB_FEATURE_MASK - // control of what blocks (USB3, USB2, DP) are enabled or disabled
{
    BIT_MASK(CONFIG_BLOCK_ENABLE_USB_USB2) | BIT_MASK(CONFIG_BLOCK_ENABLE_USB_USB3),    // USBcontrol  - USB2,3 enables
    BIT_MASK(CONFIG_BLOCK_ENABLE_DP),                                                   // DPcontrol   - DP enables
    BIT_MASK(CONFIG_BLOCK_ENABLE_GMII) | BIT_MASK(CONFIG_BLOCK_ENABLE_RS232)            // MiscControl - LAN port, RS232 enables
};

const ConfigBlocksEnable        dflt1_featureEnable =       // CONFIG_VARS_BB_FEATURE_ENABLE - // control of what blocks (USB3, USB2, DP) are enabled or disabled
{
    0,      // USBcontrol  - USB2,3 disable
    0,      // DPcontrol   - DP disable
    0       // MiscControl   - Lan port, RS232 disable
};

const ConfigPhyParams           dflt1_phyConfig =           // CONFIG_VAR_PHY_PARAMS
{
    BIT_MASK(CONFIG_BLOCK_PHY_LINK_SPEED_BIT_0) |                                           // PhyControl - Phy link speed
    BIT_MASK(CONFIG_BLOCK_PHY_LINK_SPEED_BIT_1) |                                           // PhyControl - Phy link speed
    BIT_MASK(CONFIG_BLOCK_ENABLE_PHY)
};

const ConfigTemperatureParams   dflt1_temperatureConfig =   // CONFIG_VAR_TEMPERATURE_PARAMS
{
    CONFIG_BLOCK_TEMPERATURE_FPGA_WARN_2,
    CONFIG_BLOCK_TEMPERATURE_FPGA_WARN_3,
    CONFIG_BLOCK_TEMPERATURE_FPGA_CUT_2,
    CONFIG_BLOCK_TEMPERATURE_FPGA_CUT_3,
    CONFIG_BLOCK_TEMPERATURE_AQUANTIA_WARN,
    CONFIG_BLOCK_TEMPERATURE_AQUANTIA_CUT,
    0       // buffer for future
};

const ConfigBrandName           dflt1_brandName =           // CONFIG_VAR_BB_BRAND_NAME - the vendor's brand name for this device
{
    CONFIG_DEFAULT_BRAND_NAME
};

const ConfigRandomSeed          dflt1_randomSeed =          // CONFIG_VAR_BB_PSEUDO_RANDOM_SEED - the seed used for the random number generator
{
    CONFIG_DEFAULT_RANDOM_SEED
};

const ConfigDiagnosticConfig    dflt1_diagConfig=
{
    false,      // Diagnostic Enable status
    0,          // Test Status for Production script
    0,          // DP Error Code
    0,          // Diagnostic Error Code Reserved for future use
    0,          // Diagnostic Error Code Reserved for future use
    0,          // Diagnostic Error Code Reserved for future use
};

const ConfigDpConfig            dflt1_dpConfig =            // CONFIG_VAR_BB_DP_CONFIG - miscellaneous configuration bits for DP
{
    false,      // Disable isolated REX/Lex by default
    false,      // Disable yCbCr support from second block of Edid
    true,       // Disable new ALU calcuration
    true,       // read mccs
    false,      // Disable Audio by default
    false,      // Match Lex and Rex BW/LC by default
    2,          // Lex - Pass monitor's value by default
    2,          // Rex - Pass monitor's value by default
    255,        // VS 255 by default
    255,        // PE 255 by default
    0,          // LC - Pass the monitors value by default
    0,          // BW - Pass the monitors value by default
    0,          // Pass REX's edid by default
    0,          // Do not modify bpc by default
    0,          // Timer value is 0 means no waiting before setting power down
    4,          // Sets the compression ratio 4:1 as default
#ifdef PLUG_TEST
    0,          // Disable Aux Traffic logs by default
#endif // PLUG_TEST
};

const ConfigIdtClkConfig        dflt1_idtClk =              // CONFIG_VAR_BB_IDK_CONFIG - Rex hardware SSC support
{
    true        // Rex SSC support is enabled by default
};

const ConfigUsbConfig           dflt1_usbConfig =           // CONFIG_VAR_BB_USB_CONFIG - miscellaneous configuration bits for USB 2 & 3
{
    BIT_MASK(CONFIG_USB3_RESET_USB_ON_DISCONNECT),          // default to USB3 resetting when disconnected
    0
};

const ConfigUsbExtendedConfig   dflt1_usbExtendedConfig =           // CONFIG_VAR_BB_USB_EXTENDED_CONFIG - miscellaneous configuration bits for USB 2 & 3
{
#ifdef BB_ISO
    .UPPcontrol = BIT_MASK(CONFIG_ENABLE_ISO_UPP) | BIT_MASK(CONFIG_ENABLE_CONTROL_TRANSFER)
#else
    .UPPcontrol = 0
#endif
};

// network parameters
const ConfigMacAddress          dflt1_MacAddress =          // CONFIG_VAR_NET_MAC_ADDRESS - this device's MAC address
{
    { { 0x02, 0xB1, 0x13, 0x00, 0x80, 0x58 } }              // use 02-B1-13... for point to point devices (00-B1-13... for network devices)
};

const ConfigNetPortNumber       dflt1_devPortNumber =       // CONFIG_VAR_NET_PORT_NUMBER - Port number we are listening on
{
    CONFIG_DEFAULT_PORT_NUMBER
};

// GE specific variables
const ConfigGEconfigBits        dflt1_geConfigBits =        // CONFIG_VAR_GE_CONFIGURATION_BITS - GE's configuration bits
{
    BIT_MASK(TOPLEVEL_SUPPORT_USB2_HISPEED_OFFSET)  |
    BIT_MASK(TOPLEVEL_SUPPORT_MSA_OFFSET)           |
    BIT_MASK(TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET)     |
    BIT_MASK(TOPLEVEL_ENET_PHY_MII_SUPPORT_OFFSET)  |
    BIT_MASK(TOPLEVEL_ENET_PHY_GMII_SUPPORT_OFFSET)
};

const ConfigGEvhubConfig        dflt1_USB2vhubSetup =       // CONFIG_VAR_USB2_VHUB_CONFIG - the USB2 vhub configuration
{
    NUM_OF_VPORTS, VHUB_VENDOR_ID, VHUB_PRODUCT_ID
};

const ConfigUlpUsb3ResetParams  dflt1_ulpUsb3ResetParams =  // CONFIG_VAR_USB3_RESET_PARAMS
{
    DEFAULT_LEX_LTSSM_TOTAL_TIMEOUT_IN_US,                  // lexLtssmTotalTimeoutUs
    DEFAULT_LEX_USB3_SNOOP_TIMEOUT,                         // lexUsb3SnoopTimeoutMs
    DEFAULT_LEX_USB3_WARM_RESET_TIMER_PERIOD_IN_MS,         // lexUsb3WarmResetTimerPeriodMs
    DEFAULT_LEX_USB3_WARM_RESET_MAX_COUNT,                  // lexUsb3WarmResetMaxCount
    DEFAULT_LEX_USB3_MAX_FAIL_COUNT,                        // lexUsb3MaxFailCount
    DEFAULT_LEX_USB3_INACTIVE_STUCK_TIMEOUT_IN_MS,          // lexUsb3InactiveStuckTimeoutMs
    DEFAULT_RESERVED
};

const ConfigFiberParams  dflt1_fiberParams =                // CONFIG_VAR_FIBER_PARAMS
{
    DEFAULT_SFP_RX_POWER_TIMEOUT_MS,                        // sfpRxPowerTimeoutMs
    DEFAULT_SFP_RX_POWER_DEBOUNCE_COUNT,                    // sfpRxPowerDebounceCount
    DEFAULT_SFP_RX_POWER_THRESHOLD_UPPER,                   // sfpRxPowerThresholdUpper
    DEFAULT_SFP_RX_POWER_THRESHOLD_LOWER,                   // sfpRxPowerThresholdLower
    DEFAULT_RESERVED,
    DEFAULT_RESERVED
};

// variable table for default initialization
// (only those variables that aren't zero)
const ConfigVarDefaultEntry default1_table[] =
{
    { CONFIG_VARS_BB_FEATURE_MASK,      &dflt1_featureMask },
    { CONFIG_VARS_BB_FEATURE_ENABLE,    &dflt1_featureEnable },
    { CONFIG_VARS_BB_FEATURE_CONTROL,   &dflt1_featureEnable },
    { CONFIG_VAR_BB_BRAND_NAME,         &dflt1_brandName },
    { CONFIG_VAR_BB_PSEUDO_RANDOM_SEED, &dflt1_randomSeed },
    { CONFIG_VAR_BB_DP_CONFIG,          &dflt1_dpConfig },
    { CONFIG_VAR_BB_IDK_CONFIG,         &dflt1_idtClk},
    { CONFIG_VAR_BB_USB_CONFIG,         &dflt1_usbConfig },
    { CONFIG_VAR_BB_USB_EXTENDED_CONFIG,&dflt1_usbExtendedConfig },
    { CONFIG_VAR_NET_MAC_ADDRESS,       &dflt1_MacAddress },
    { CONFIG_VAR_NET_PORT_NUMBER,       &dflt1_devPortNumber },
    { CONFIG_VAR_GE_CONFIGURATION_BITS, &dflt1_geConfigBits },
    { CONFIG_VAR_USB2_VHUB_CONFIG,      &dflt1_USB2vhubSetup },
    { CONFIG_VAR_ULP_USB3_RESET_PARAMS, &dflt1_ulpUsb3ResetParams },
    { CONFIG_VAR_FIBER_PARAMS,          &dflt1_fiberParams },
    { CONFIG_VAR_PHY_PARAMS,            &dflt1_phyConfig },
    { CONFIG_VAR_TEMPERATURE_PARAMS,    &dflt1_temperatureConfig },
};

// Exported Function Definitions ##################################################################
//#################################################################################################
// set all enable for feature
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void Config_EnableFeature()
{
    Config_ArbitrateSetVar(
        CONFIG_SRC_DEFAULT,
        CONFIG_VARS_BB_FEATURE_CONTROL,
        &dflt1_featureMask);        // default for featureMask enables all feature
}

//#################################################################################################
// set all disable for feature
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void Config_DisableFeature()
{
    Config_ArbitrateSetVar(
        CONFIG_SRC_DEFAULT,
        CONFIG_VARS_BB_FEATURE_CONTROL,
        &dflt1_featureEnable);      // default for featureEnable disables all feature
}

//#################################################################################################
// Check default for featureEnable
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool Config_IsDefaultFeatureEnable()
{
    ConfigBlocksEnable featureEnable;

    if(Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_ENABLE, &featureEnable))
    {
        if(memeq(&dflt1_featureEnable, &featureEnable, sizeof(ConfigBlocksEnable)))
        {
            return true;
        }
    }
    return false;
}

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// load the default values for the configuration variables
//
// Parameters:
// Return:
// Assumptions:  NVM access may not be operational
//#################################################################################################
void Config_LoadDefaults(void)
{
    uint8_t index;

    for (index = 0; index < ARRAYSIZE(default1_table); index++)
    {
        Config_ArbitrateSetVar(
            CONFIG_SRC_DEFAULT,
            default1_table[index].variable,
            default1_table[index].defaultValue);
    }
}

// Static Function Definitions ####################################################################

