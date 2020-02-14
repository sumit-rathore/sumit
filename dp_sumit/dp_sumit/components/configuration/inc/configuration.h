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
//
//
//
//#################################################################################################
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// Includes #######################################################################################
#include <itypes.h>

// Constants and Macros ###########################################################################

// brand name size is 30 bytes for compatibility with GE
#define CONFIG_BRAND_NAME_SIZE  30

// these defines have to do with Manufacturing
#define CONFIG_SERIAL_NUMBER_SIZE   64
#define CONFIG_MODEL_STRING_SIZE    32

// these bit defines are used by the hwFeaturesSupported variable
// they define what hardware support the product has
#define CONFIG_HW_FEATURE_USB2          0       // hardware support for USB 2 exists
#define CONFIG_HW_FEATURE_USB3          1       // hardware support for USB 3 exists
#define CONFIG_HW_FEATURE_DP_VIDEO      2       // hardware support for DP video exists

// these bit defines set what soft features are enabled
// USB features
#define CONFIG_BLOCK_ENABLE_USB_USB2    0       // USB 2 enabled
#define CONFIG_BLOCK_ENABLE_USB_USB3    1       // USB 3 enabled

// DP features
#define CONFIG_BLOCK_ENABLE_DP              0       // Main DP block enabled
#define CONFIG_BLOCK_ENABLE_DP_ISOLATED_REX    2       // Dp isolated REX enabled
#define CONFIG_BLOCK_ENABLE_DP_ISOLATED_LEX    3       // Dp isolated LEX enabled

// MISC features
#define CONFIG_BLOCK_ENABLE_GMII            0       // Enable/Disable LAN port GE
#define CONFIG_BLOCK_ENABLE_RS232           1       // Enable/Disable RS232

// PHY Link Control
#define CONFIG_BLOCK_PHY_LINK_SPEED_BIT_0   0       // Control PHY Link Speed
#define CONFIG_BLOCK_PHY_LINK_SPEED_BIT_1   1       // Control PHY Link Speed
#define CONFIG_BLOCK_ENABLE_PHY             7       // ENABLE/Disable Phy

// TEMPEARTURE Control
#define CONFIG_BLOCK_TEMPERATURE_FPGA_WARN_2      FPGA_SW_WARN_TEMPERATURE_2         // FPGA Temperature warning value
#define CONFIG_BLOCK_TEMPERATURE_FPGA_WARN_3      FPGA_SW_WARN_TEMPERATURE_3         // FPGA Temperature warning value

#define CONFIG_BLOCK_TEMPERATURE_FPGA_CUT_2       FPGA_SW_CUT_TEMPERATURE_2         // FPGA Temperature shutdown value
#define CONFIG_BLOCK_TEMPERATURE_FPGA_CUT_3       FPGA_SW_CUT_TEMPERATURE_3         // FPGA Temperature shutdown value
#define CONFIG_BLOCK_TEMPERATURE_AQUANTIA_WARN    AQUANTIA_SW_WARN_TEMPERATURE    // Aquantia Temperature warning value
#define CONFIG_BLOCK_TEMPERATURE_AQUANTIA_CUT     AQUANTIA_SW_CUT_TEMPERATURE     // Aquantia Temperature shutdown value

// bits used in ConfigUsbConfig
#define CONFIG_USB3_RESET_USB_ON_DISCONNECT     0       // reset USB3 ULP when disconnected

// bits used in ConfigUsbExtendedConfig, for upp
#define CONFIG_ENABLE_ISO_UPP                   0
#define CONFIG_ENABLE_CONTROL_TRANSFER          1
// ULP USB Reset Parameters Initial values
// Sumit TODO: temporarily increasing for DP
#define DEFAULT_LEX_LTSSM_TOTAL_TIMEOUT_IN_US           (2*1000*1000) // should take max 1s to get to U0 from RxDetect valid

// give 250ms for snoop mode, otherwise go on to USB2 - see section 7.5.3.6, which specifies 8 cycles
// put the Rex into the disabled state, but let the Lex keep on scanning, in case USB3 comes back
// eventually (host restart, sleep or shutdown cycles)
#define DEFAULT_LEX_USB3_SNOOP_TIMEOUT                  250
#define DEFAULT_LEX_USB3_WARM_RESET_TIMER_PERIOD_IN_MS  (8*1000)
#define DEFAULT_LEX_USB3_WARM_RESET_MAX_COUNT           2

// one log showed that when we went to inactive using the goto inactive feature,
// the warm reset occurred just before 500ms.  Using 750 milliseconds as a failure point
#define DEFAULT_LEX_USB3_INACTIVE_STUCK_TIMEOUT_IN_MS   750

// maximum number of times we fail to setup USB3 before we really go to the failure state
#define DEFAULT_LEX_USB3_MAX_FAIL_COUNT                 3

// Fiber Parameters Initial values
#define DEFAULT_SFP_RX_POWER_TIMEOUT_MS                 20
#define DEFAULT_SFP_RX_POWER_THRESHOLD_UPPER            2000
#define DEFAULT_SFP_RX_POWER_THRESHOLD_LOWER            1300
#define DEFAULT_SFP_RX_POWER_DEBOUNCE_COUNT             2

// Initial value for reserved space
#define DEFAULT_RESERVED                                0

// atmel feature bits definition
#define ATMEL_FEATURE_USB2              1 << 0
#define ATMEL_FEATURE_USB3              1 << 1
#define ATMEL_FEATURE_VIDEO             1 << 2
#define ATMEL_FEATURE_NETWORK           1 << 3
#define ATMEL_FEATURE_HDCP              1 << 4
#define ATMEL_FEATURE_COMPRESSION       1 << 5
#define ATMEL_FEATURE_RS232             1 << 6
#define ATMEL_FEATURE_LANPORT           1 << 7

// Data Types #####################################################################################

// these enum definitions set the variable name values, and should not be changed to keep backwards compatibility,
// especially with variables stored in NVM.  Gaps have been added so some variables can be added without changing
// the position of older variables
enum ConfigurationVars
{
    CONFIG_VARS_INVALID_VAR = 0,            // This variable is guaranteed not to exist

    CONFIG_VARS_BB_FEATURE_MASK = 20,       // user's setting of what blocks (USB3, USB2, DP) are enabled or disabled
    CONFIG_VARS_BB_FEATURE_ENABLE,          // product's setting of what blocks (USB3, USB2, DP) are enabled or disabled
    CONFIG_VARS_BB_FEATURE_CONTROL,         // control of feature : feature Mask & feature control
    CONFIG_VARS_BB_MODEL_INFO,              // the model and perhaps serial number of this unit
    CONFIG_VAR_BB_PSEUDO_RANDOM_SEED,       // For the random # generator component
    CONFIG_VAR_BB_BRAND_NAME,               // the brand name for this device
    CONFIG_VAR_BB_DP_CONFIG,                // Persistent settings for DP configuration
    CONFIG_VAR_BB_IDK_CONFIG,               // Persistent settings for Rex SSC support
    CONFIG_VAR_BB_USB_CONFIG,               // Persistent settings for USB configuration
    CONFIG_VAR_BB_USB_EXTENDED_CONFIG,      // Extended settings for USB configuration
    // GE, USB 2 configuration
    CONFIG_VAR_GE_CONFIGURATION_BITS = 40,  // GE's configuration bits
//    CONFIG_VAR_USB2_DCF,                    // USB 2.0 Device class filtering (DCF) for this device pair TODO: bit enable of each USB 2 device class
    CONFIG_VAR_USB2_VHUB_CONFIG,            // Options for configuring USB2 Virtual Hub on GE

    CONFIG_VAR_GE_PAIRED_MAC,               // the MAC address of the device we are paired with

    // ethernet configuration variables
    CONFIG_VAR_NET_MAC_ADDRESS = 80,        // the MAC address of this unit
    CONFIG_VAR_NET_IP_ADDR,                 // IP address of this device.
    CONFIG_VAR_NET_PORT_NUMBER,             // the UDP port it will listen to for requests
    CONFIG_VAR_NET_SUBNET_MASK,             // subnet mask.
    CONFIG_VAR_NET_DEFAULT_GATEWAY,         // default gateway.
    CONFIG_VAR_NET_IP_ACQUISITION_MODE,     // Static or Dynamic IP address allocation
    CONFIG_VAR_NET_DHCP_SERVER_IP,          // the DHCP server last used to obtain an IP address.

    CONFIG_VAR_ULP_USB3_RESET_PARAMS,       // usb3 reset paramaters
    CONFIG_VAR_FIBER_PARAMS,                // Fiber power parameters
    CONFIG_VAR_PHY_PARAMS,                  // Link configuration (speed, phy enable)

    CONFIG_VAR_TEMPERATURE_PARAMS,          // Temperature threshold setup for FPGA, Aquantia
    CONFIG_VAR_DIAG_CONFIG,                 // System Diagnostic for Production line
    CONFIG_VAR_MAX_VARS                     // the highest number (+1) of configuration variables we have
};

enum ConfigSourceType
{
    CONFIG_SRC_DEFAULT,         // setting is a default value
    CONFIG_SRC_PIN_STRAP,       // setting derived from HW strapping pins

    CONFIG_SRC_NVM_FLASH,       // setting was from non-volatile memory (FLASH)
    CONFIG_SRC_NVM_EEPROM,      // setting was from non-volatile memory (EEPROM)

    CONFIG_SRC_UART,            // received from the UART
    CONFIG_SRC_I2C_SLAVE,       // received from the I2C slave port
    CONFIG_SRC_NETWORK,         // received from Ethernet, UDP protocol

    CONFIG_SRC_SAVE,            // setting should be saved

    CONFIG_SRC_NUMBER_OF_SOURCES    // used to keep track of how many sources we have
};

enum ConfigBlockLinkSpeed
{
    CONFIG_BLOCK_LINK_SPEED_1G = 0,
    CONFIG_BLOCK_LINK_SPEED_2_5G,
    CONFIG_BLOCK_LINK_SPEED_5G,
    CONFIG_BLOCK_LINK_SPEED_10G
};

enum ConfigSscMode
{
    CONFIG_SSC_DISABLE   = 0,  // SSC is disabled always not depending on the DPCD reg
    CONFIG_SSC_ENABLE    = 1,  // SSC is enabled always not depending on the DPCD reg
    CONFIG_SSC_PASSED_ON = 2   // SSC state is as stated by the DPCD reg
};

typedef struct _ConfigDateTime
{
    uint16_t    year;           // 0 - 65535 AD
    uint8_t     month;          // 1-12 (Jan-Dec)
    uint8_t     day;            // 1-31

    uint8_t     hours;          // 0-23
    uint8_t     seconds;        // 0-59

} ConfigDateTime;

typedef struct _ConfigManufacturingInfo
{
    // the model string, set by Badger
    char model[CONFIG_MODEL_STRING_SIZE+1];  // +1 for space for terminating zero

    // the serial number set by Badger can be a variable string.  Right now (Sept 14, 2016), it's max is 29 bytes.
    // Setting it to 64 in the hopes it will never be bigger then that
    char serialNumber[CONFIG_SERIAL_NUMBER_SIZE+1];

    ConfigDateTime dateManufactured;  // the timestamp when this unit was built

} ConfigManufacturingInfo;

typedef struct _ConfigHwFeaturesSupported
{
    // a bit mask that lists all the features that this product is capable of
    // ie has hardware support for
    uint64 hwFeaturesSupported;

} ConfigHwFeaturesSupported;

// the vendor's brand name for this device
typedef struct _ConfigBrandName
{
    char brandName[CONFIG_BRAND_NAME_SIZE+1];  // +1 for space for terminating zero

} ConfigBrandName;

// device's 6 byte Mac address, in big endian format
typedef struct _ConfigMacAddress
{
    union
    {
        uint8_t macAddress[6];
        uint64_t macVar;
    };

} ConfigMacAddress;

// the IP4 address structure used by various variables
typedef struct _ConfigIP4address
{
    union
    {
        uint8_t  ip4Address[4];
        uint32_t ip4var;         // to be compatible with in_addr struct
    };

} ConfigIp4Address;

// the network port number we are listening on
// wrapped in a struct to be the same as all the other variables
typedef struct _ConfigNetPortNumber
{
    uint16_t ipPortNumber;

} ConfigNetPortNumber;

// the seed used for the random number generator
// wrapped in a struct to be the same as all the other variables
typedef struct _ConfigRandomSeed
{
    uint32_t seed;

} ConfigRandomSeed;

typedef struct _ConfigDiagnosticConfig
{
    bool        diagStatus;
    uint8_t     testStatus;
    uint8_t     rsvd1ErrorCode;     // Reserved for future error reporting components
    uint8_t     rsvd2ErrorCode;     // Reserved for future error reporting components
    uint8_t     rsvd3ErrorCode;     // Reserved for future error reporting components
    uint8_t     rsvd4ErrorCode;     // Reserved for future error reporting components
} ConfigDiagnosticConfig;

typedef struct _ConfigDpConfig
{
    bool                enableIsolate;        // Disable isolated REX/Lex by default
    bool                disableYCbCr;         // Disable yCbCr222 support from second block of Edid
    bool                newAluCalculation;    // Enable new ALU calculation
    bool                noReadMccs;           // read mccs
    bool                noSendAudio;          // Disable audio if set, Default is audio disabled
    bool                enableLcBwIsolation;  // Enable independent BW/LC link training on LEX and REX
    uint8_t             lexSscAdvertiseMode;  // State of Lex SSC mode
    uint8_t             rexSscAdvertiseMode;  // State of Rex SSC mode
    uint8_t             voltageSwing;         // VS 255 by default
    uint8_t             preEmphasis;          // PE 255 by default
    uint8_t             laneCount;            // Lanecount to be advertise to host, 0 by default disabling use of flash value
    uint8_t             bandwidth;            // Bandwidth to be advertise to host, 0 by default disabling use of flash value
    uint8_t             edidType;             // Pass REX's edid by default
    uint8_t             bpcMode;              // Do not modify bpc by default
    uint8_t             powerDownTime;        // Timer value before setting power down state to monitor
    uint8_t             compressionRatio;     // Sets the compression ratio
#ifdef PLUG_TEST
    uint8_t             enableAuxTraffic;     // Enable AUX traffic over UART
#endif // PLUG_TEST
} ConfigDpConfig;

// miscellaneous configuration bits for USB
typedef struct
{
    uint8_t     usb3Config;     // USB3 settings
    uint8_t     usb2Config;     // USB2 settings

} ConfigUsbConfig;

// miscellaneous configuration bits for USB
typedef struct
{
    uint8_t     UPPcontrol;
    uint8_t     reserved1;
    uint8_t     reserved2;
    uint8_t     reserved3;

    uint32_t    reserved4;

} ConfigUsbExtendedConfig;

// wrapped in a struct to be the same as all the other variables
typedef struct _ConfigIdtClk
{
    bool rexSscSupport;          // Enable SSC support

} ConfigIdtClkConfig;

typedef struct
{
    uint8_t     USBcontrol;     // control for USB 2 and 3
    uint8_t     DPcontrol;      // control for DP
    uint8_t     MiscControl;    // control for Miscellaneous features (Lan Port, RS232)

} ConfigBlocksEnable;

typedef struct
{
    uint8_t     phyConfig;      // control for Link to 1G, 2.5G, 5G and 10G, and phy enable

} ConfigPhyParams;

typedef struct
{
    uint8_t     fpgaWarnThreshold_2;      // control for FPGA temperature warning -2
    uint8_t     fpgaWarnThreshold_3;      // control for FPGA temperature warning -3
    uint8_t     fpgaShutThreshold_2;      // control for FPGA temperature shutdown for raven or -2
    uint8_t     fpgaShutThreshold_3;      // control for FPGA temperature shutdown for maverick or -3
    uint8_t     aquantiaWarnThreshold;  // control for Aquantia temperature warning
    uint8_t     aquantiaShutThreshold;  // control for Aquantia temperature shutdown
    uint16_t    buffer;                 // for additional temperature check in the future

} ConfigTemperatureParams;

// configuration bits passed on to GE.
// Note that some of these bits are generated from other variables
typedef struct _ConfigGEconfigBits
{
    // only 53 bits are available - see storage_vars.h in GE
    uint64_t    geConfigBits;

} ConfigGEconfigBits;

typedef struct _ConfigGEvhubConfig
{
    uint8_t     numVhubPorts;   // should be 0 for Blackbird?
    uint16_t    vhubVid;        // vHub vendor ID
    uint16_t    vhubPid;        // vHub product ID

} ConfigGEvhubConfig;

typedef struct _ConfigUlpUsb3ResetParams
{
    uint32_t    lexLtssmTotalTimeoutUs;     // max time to get to U0 from RxDetect valid
    uint16_t    lexUsb3SnoopTimeoutMs;      // timeout for snoop mode, otherwise go on to USB2 - see section 7.5.3.6, which specifies 8 cycles
// put the Rex into the disabled state, but let the Lex keep on scanning, in case USB3 comes back
// eventually (host restart, sleep or shutdown cycles)
    uint16_t    lexUsb3WarmResetTimerPeriodMs;
    uint8_t     lexUsb3WarmResetMaxCount;
    uint8_t     lexUsb3MaxFailCount;        // maximum number of times we fail to setup USB3 before we really go to the failure state
    uint16_t    lexUsb3InactiveStuckTimeoutMs;
    uint32_t    reserved0;                  // reserve flash space for future use
} ConfigUlpUsb3ResetParams;

typedef struct _ConfigFiberParams
{
    uint8_t     sfpRxPowerTimeoutMs;
    uint8_t     sfpRxPowerDebounceCount;
    uint16_t    sfpRxPowerThresholdUpper;
    uint16_t    sfpRxPowerThresholdLower;
    uint32_t    reserved0;                  // reserve flash space for future use
    uint32_t    reserved1;                  // reserve flash space for future use
} ConfigFiberParams;



// this union is meant to set a buffer big enough to hold the largest variable.  For consistency,
// when adding a variable, add it individually, and in this union, even if it is not
// the biggest variable

typedef union
{
    // blackbird specific
    ConfigBlocksEnable          featureControl;     // CONFIG_VARS_BB_FEATURE_CONTROL - // control of what blocks (USB3, USB2, DP) are enabled or disabled
    ConfigBrandName             brandName;          // CONFIG_VAR_BB_BRAND_NAME - the vendor's brand name for this device
    ConfigRandomSeed            randomSeed;         // CONFIG_VAR_BB_PSEUDO_RANDOM_SEED - the seed used for the random number generator
    ConfigDpConfig              dpConfig;           // CONFIG_VAR_BB_DP_CONFIG - miscellaneous configuration bits DP
    ConfigUsbConfig             usbConfig;          // CONFIG_VAR_BB_USB_CONFIG - miscellaneous configuration bits for USB 2 & 3
    ConfigUsbExtendedConfig     usbExtendedConfig;  // CONFIG_VAR_BB_USB_EXTENDED_CONFIG - miscellaneous configuration bits for USB 2 & 3, and reserved bits
    // network parameters
    ConfigMacAddress            macAddress;         // CONFIG_VAR_NET_MAC_ADDRESS - this device's MAC address
    ConfigIp4Address            devIpAddress;       // CONFIG_VAR_NET_IP_ADDR - IP address of this device.
    ConfigNetPortNumber         devPortNumber;      // CONFIG_VAR_NET_PORT_NUMBER - Port number we are listening on
    ConfigIp4Address            subNetMask;         // CONFIG_VAR_NET_SUBNET_MASK - IP subnet mask
    ConfigIp4Address            defaultGateway;     // CONFIG_VAR_NET_DEFAULT_GATEWAY - IP Default gateway address
    ConfigIp4Address            ipAcquisitionMode;  // CONFIG_VAR_NET_IP_ACQUISITION_MODE - IP address static or dynamic
    ConfigIp4Address            dhcpServerIp;       // CONFIG_VAR_NET_DHCP_SERVER_IP - the DHCP server last used to obtain an IP address.

    // GE specific variables
    ConfigGEconfigBits          geConfigBits;       // CONFIG_VAR_GE_CONFIGURATION_BITS - GE's configuration bits
    ConfigGEvhubConfig          USB2vhubSetup;      // CONFIG_VAR_USB2_VHUB_CONFIG - the USB2 vhub configuration

    // ULP, Link parameters
    ConfigUlpUsb3ResetParams    ulpUsb3ResetParams; // CONFIG_VAR_USB3_RESET_PARAMS
    ConfigFiberParams           fiberParams;        // CONFIG_VAR_FIBER_PARAMS

    // PHY Link to contorl parameters : 1G, 2.5G, 5G and 10G, and phy enable
    ConfigPhyParams             phyParams;          // CONFIG_VAR_PHY_PARAMS

    ConfigTemperatureParams     temperatureParams;  // CONFIG_VAR_TEMPERATURE_PARAMS

    ConfigDiagnosticConfig      diagConfig;

    uint8_t                     data[1];            // pointer to access this buffer
} ConfigurationBuffer;


// Function Declarations ##########################################################################
void Configuration_Init(void);
void Configuration_LoadNVM(void)  __attribute__ ((section(".atext")));

ConfigurationBuffer * Config_GetBuffer(void);


bool Config_ArbitrateSetVar(
    const enum ConfigSourceType source,
    const enum ConfigurationVars variableName,
    const void *buffer);

bool Config_ArbitrateGetVar(enum ConfigurationVars variableName, void *buffer);
const void* Config_GetDataPointer(enum ConfigurationVars variableName);

// miscellaneous functions
void Config_ShowFeatureControlStatus(void)      __attribute__ ((section(".atext")));
void Config_UpdateFeatures(uint8_t *buf)        __attribute__ ((section(".atext")));
bool IsFeatureEnableNotChanged(uint8_t *buf)    __attribute__ ((section(".atext")));
void Config_EnableFeature(void)                 __attribute__ ((section(".atext")));
void Config_DisableFeature(void)                __attribute__ ((section(".atext")));
bool Config_IsDefaultFeatureEnable(void)        __attribute__ ((section(".atext")));

uint32_t Config_GetFeatureByte(void);

#endif // CONFIGURATION_H
