///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010-2014
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - imain.c
//
//!   @brief - This file contains the main functions for the Golden Ears SW
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "toplevel_loc.h"
#include <storage_Data.h>
#include <rexsch.h>
#include <kc705.h>
#include <eeprom.h>
#include "cypress_hx3_firmware.h"
#include <grg.h>
#include <grg_i2c.h>
#include <grg_led.h>
#include <ulm.h>

#ifdef BB_GE_COMPANION
#include <tasksch.h>
#include <ge_bb_comm.h>
#include <crc.h>
#endif

/************************ Defined Constants and Macros ***********************/
#define CYPRESS_RESET_TIME_IN_US 5000

#ifdef BB_GE_COMPANION
#define GE_STATUS_TO_BB_TIMEOUT_MS          (1000)
#endif
/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
#ifdef BB_GE_COMPANION
static TASKSCH_TaskT checkUartRxTask;
static TIMING_TimerHandlerT statusTimer;
#endif

bool ecoStatus;

/************************** External linker Definitions *****************************/
// Rextext/lextext overlay symbols from linker
extern uint32 __load_start_rextext;
extern uint32 __load_stop_rextext;
extern uint32 __load_start_lextext;
extern uint32 __load_stop_lextext;
extern uint32 __rex_lex_overlay_start;

/************************ Local Function Prototypes **************************/
static void imainPart2(void);
static void imainPart3(enum TOPLEVEL_CypressHx3UpgradeResult result);
static void copyLexRexOverlay(void);
static void logBuildInfo(ilogLevelT level);
#ifndef GE_CORE
static boolT getOwnMACAddress(uint64 *mac);
#endif
#if !defined(GE_CORE) && !defined(BUILD_FOR_SIM)
static void _TOP_i2cInitDone(boolT success);
static void _TOP_killSystemTimerHandler(void);
#ifndef BB_GE_COMPANION
static void _TOP_atmelInitDone(boolT dataAndOtpZonesLocked, boolT configZoneLocked);
static void _TOP_eepromInit(boolT eepromInstalled);
static void _TOP_checkForEeprom(uint8* data, uint8 count);
#endif
#endif
static void assertPreHook(void);
static void assertPostHook(struct assertInfo_s * assertInfo) __attribute__ ((noreturn));
void printString(const char *);
static void getEcoStatus(void);
#ifdef BB_GE_COMPANION
static void checkUartRx(TASKSCH_TaskT task, uint32 arg);
#endif

/************************** Function Definitions *****************************/
/**
* FUNCTION NAME: copyLexRexOverlay()
*
* @brief  - copies Rex or Lex specific code from flash to RAM
*
* @return - void
*
* @note   - This should be called very early in initialization to ensure code is in RAM
*
*/
static void copyLexRexOverlay(void)
{
    uint32 * loadStart;
    uint32 * loadEnd;
    uint32 * dest = &__rex_lex_overlay_start;

    // Calculate addresses of source location in flash
    if (GRG_IsDeviceLex())
    {
        loadStart = &__load_start_lextext;
        loadEnd = &__load_stop_lextext;
    }
    else
    {
        loadStart = &__load_start_rextext;
        loadEnd = &__load_stop_rextext;
    }

    // Copy the data into instruction RAM
    memcpy(dest, loadStart, (uint32)loadEnd - (uint32)loadStart);
}


static void assertPreHook(void)
{
    GRG_GpioSet(GPIO_OUT_LED_ASSERT);
    ULM_preAssertHook();
}



/**
* FUNCTION NAME: assertPostHook()
*
* @brief  - Called after an assert occurs.  The idea is for this function to
*           call into functions in each component that contains useful
*           information to dump on assert.
*
* @return - Does not return.
*/
static void assertPostHook(struct assertInfo_s * assertInfo)
{
    // Ensure everything gets logged
    ilog_setBlockingMode();

    ilog_TOPLEVEL_COMPONENT_1(ILOG_USER_LOG, PENDING_IRQ, LEON_getPendingIrqMask());
    ilog_TOPLEVEL_COMPONENT_1(ILOG_USER_LOG, ENABLED_IRQ, LEON_getEnabledIrqMask());

    logBuildInfo(ILOG_FATAL_ERROR);

    // Message Blackbird about the assert
    sendGeAssertedMsgToBB(assertInfo);

    XCSR_assertHook();
    ULM_assertHook();
    GRG_assertHook();
    CLM_assertHook();

    if(GRG_IsDeviceLex())
    {
        XLR_assertHook();
        SYSCTRLQ_assertHook();
        DTT_assertHook();
#ifndef GE_CORE
        VHUB_assertHook();
#endif
    }
    else // Rex
    {
        XRR_assertHook();
        REXULM_assertHook();
        REXSCH_assertHook();
    }

    LINKMGR_assertHook();

#ifndef GE_CORE
    STORAGE_assertHook();
    ATMEL_assertHook();
    NET_assertHook();
#endif

    TIMING_assertHook();

    // Make sure all the UART data has been flushed before entering the ICMD
    // polling loop, or resetting the chip
    LEON_UartWaitForTx();
#ifdef DEBUG
    ICMD_PollingLoop();
#else
    GRG_ResetChip();
#endif
}

/**
* FUNCTION NAME: imain()
*
* @brief  - The main function for the Golden Ears SW
*
* @return -
*
* @note   -
*
*/
void * imain (void)
{
    uint8 major;
    uint8 minor;
    uint8 debug;
    irqFlagsT flags;

    // Copy Lex specific code or Rex specific code to instruction RAM
    copyLexRexOverlay();

    // Lockout interrupts for the initialization code
    flags = LEON_LockIrq();

    getEcoStatus(); // see if we are running on an ECO chip

    // Configure the uart
    LEON_UartSetBaudRate(115200);
#ifdef BB_GE_COMPANION
    // initialize the UART packetizer
    UART_packetizeInit();
    GEBB_CommInit();
#endif
    // If the length of the version string printed below grows, Expresslink's read software version
    // capability needs to be updated
    printString("Goldenears SW " SOFTWARE_REVISION_STR "\r\n"); // NOTE: Do this first before any ilogs are visible

    ICMD_Init();
    LEON_EnableIrq(IRQ_UART_RX);
    LEON_EnableIrq(IRQ_UART_TX);

    // Uart is good
    // Initialize assert handling
    // broadcast our startup status
    iassert_Init(&assertPreHook, &assertPostHook);
    logBuildInfo(ILOG_MINOR_EVENT);
    
    if(ecoStatus)
    {
        printString("ASIC rev. B detected\r\n");
    }

    ilog_TOPLEVEL_COMPONENT_1(ILOG_USER_LOG, ECO1_FLAG_VALUE, ecoStatus); // ECO flag status
    
    LEON_UartWaitForTx();

    // Setup the timers
    LEON_TimerInit();
    LEON_EnableIrq(IRQ_TIMER2);
    LEON_UartWaitForTx();


    // Initialize the task scheduler
    TASKSCH_Init();

    // GRG initialization
    GRG_Init(&major, &minor, &debug);
    LEON_EnableIrq(IRQ_GRG);
    // note chip register version
    ilog_TOPLEVEL_COMPONENT_3(ILOG_MINOR_EVENT, CHIP_REV, major, minor, debug);
    LEON_UartWaitForTx();

#ifdef BB_GE_COMPANION
    checkUartRxTask = TASKSCH_InitTask(&checkUartRx, 0, false, TASKSCH_PRIORITY_LOW);
    TASKSCH_StartTask(checkUartRxTask);
    statusTimer = TIMING_TimerRegisterHandler(&sendStatsToBB, TRUE, GE_STATUS_TO_BB_TIMEOUT_MS);
    TIMING_TimerStart(statusTimer);
#endif

    // Setup the GPIOs. Note that the pins are mapped differently
    // for the ASIC platform and other platforms.
    if (GRG_GetPlatformID() == GRG_PLATFORMID_ASIC)
    {
        GPIO_LOOKUP_TABLE[GPIO_OUT_LED_ACTIVITY_INDEX]       = GPIO0;
        GPIO_LOOKUP_TABLE[GPIO_OUT_LED_HEART_BEAT_INDEX]     = GPIO1;
        GPIO_LOOKUP_TABLE[GPIO_OUT_REX_VBUS_ENABLE_INDEX]    = GPIO2;
        GPIO_LOOKUP_TABLE[GPIO_OUT_LED_ASSERT_INDEX]         = GPIO3;
        GPIO_LOOKUP_TABLE[GPIO_OUT_LED_HOST_INDEX]           = GPIO4;
        GPIO_LOOKUP_TABLE[GPIO_OUT_USB_HUB_RESET_INDEX]      = GPIO5;
        GPIO_LOOKUP_TABLE[GPIO_OUT_LED_LINK_INDEX]           = GPIO6;
        GPIO_LOOKUP_TABLE[GPIO_OUT_ETH_SHUTDOWN_INDEX]       = GPIO7;
        GPIO_LOOKUP_TABLE[GPIO_OUT_ETH_PHY_RESET_INDEX]      = GPIO8;
        GPIO_LOOKUP_TABLE[GPIO_IN_PAIRING_BUTTON_INDEX]      = GPIO9;
        GPIO_LOOKUP_TABLE[GPIO_IN_SLEW_RATE_INDEX]           = GPIO10;
        GPIO_LOOKUP_TABLE[GPIO_IN_DRIVE_STRENGTH_SEL0_INDEX] = GPIO11;
        GPIO_LOOKUP_TABLE[GPIO_IN_DRIVE_STRENGTH_SEL1_INDEX] = GPIO12;
        GPIO_LOOKUP_TABLE[GPIO_AUX_CLK_CONFIG_INDEX]         = GPIO13;
        GPIO_LOOKUP_TABLE[GPIO_SPARE4_INDEX]                 = GPIO14;
        GPIO_LOOKUP_TABLE[GPIO_CLEI_CLK_CONFIG_INDEX]        = GPIO15;

        const struct gpioInitStates gpioStates[] =
        {
            {GPIO_OUT_LED_ACTIVITY,         OUTPUT_CLEAR},
            {GPIO_OUT_LED_HEART_BEAT,       INPUT_DOWN  },
            // This GPIO is only used on REX, so ideally it would be left as an input on LEX, but
            // the initilization sequence currently doesn't distinguish between LEX and REX.
            {GPIO_OUT_REX_VBUS_ENABLE,      OUTPUT_CLEAR },
            {GPIO_OUT_LED_ASSERT,           INPUT_DOWN  },
            {GPIO_OUT_LED_HOST,             OUTPUT_CLEAR},
            {GPIO_OUT_USB_HUB_RESET,        OUTPUT_CLEAR},
            {GPIO_OUT_LED_LINK,             OUTPUT_CLEAR},
            {GPIO_OUT_ETH_SHUTDOWN,         INPUT_DOWN  },
            {GPIO_OUT_ETH_PHY_RESET,        OUTPUT_CLEAR},
            {GPIO_IN_PAIRING_BUTTON,        INPUT_UP    },
            {GPIO_IN_SLEW_RATE,             INPUT_DOWN  },
            {GPIO_IN_DRIVE_STRENGTH_SEL0,   INPUT_DOWN  },
            {GPIO_IN_DRIVE_STRENGTH_SEL1,   INPUT_DOWN  },
            {GPIO_AUX_CLK_CONFIG,           INPUT_DOWN  },
            {GPIO_SPARE4,                   INPUT_DOWN  },
            {GPIO_CLEI_CLK_CONFIG,          INPUT_DOWN  }
        };
        const uint8 numOfGpioStates = ARRAYSIZE(gpioStates);
        GRG_GpioInit(gpioStates, numOfGpioStates);
    }
    else
    {
        GPIO_LOOKUP_TABLE[GPIO_OUT_LED_ACTIVITY_INDEX]   = GPIO0;
        GPIO_LOOKUP_TABLE[GPIO_OUT_LED_HOST_INDEX]       = GPIO1;
        GPIO_LOOKUP_TABLE[GPIO_OUT_LED_LINK_INDEX]       = GPIO2;
        GPIO_LOOKUP_TABLE[GPIO_OUT_LED_ASSERT_INDEX]     = GPIO3;
        GPIO_LOOKUP_TABLE[GPIO_OUT_LED_HEART_BEAT_INDEX] = GPIO4;
        GPIO_LOOKUP_TABLE[GPIO_OUT_LED_DEBUG_INDEX]      = GPIO5;
        GPIO_LOOKUP_TABLE[GPIO_IN_PAIRING_BUTTON_INDEX]  = GPIO6;
        GPIO_LOOKUP_TABLE[GPIO_IN_ETH_PHY_IRQ_INDEX]     = GPIO7;
        GPIO_LOOKUP_TABLE[GPIO_OUT_ETH_PHY_RESET_INDEX]  = GPIO8;
        GPIO_LOOKUP_TABLE[GPIO_OUT_USB_HUB_RESET_INDEX]  = GPIO9;
        GPIO_LOOKUP_TABLE[GPIO_OUT_USB_PHY_RESET_INDEX]  = GPIO10;
        GPIO_LOOKUP_TABLE[GPIO_SPARE1_INDEX]             = GPIO11;
        GPIO_LOOKUP_TABLE[GPIO_SPARE2_INDEX]             = GPIO12;
        GPIO_LOOKUP_TABLE[GPIO_SPARE3_INDEX]             = GPIO13;
        GPIO_LOOKUP_TABLE[GPIO_SPARE4_INDEX]             = GPIO14;
        GPIO_LOOKUP_TABLE[GPIO_SPARE5_INDEX]             = GPIO15;

        const struct gpioInitStates gpioStates[] =
        {
            {GPIO_OUT_LED_ACTIVITY,     OUTPUT_CLEAR},  // inverted after GRG_GpioInit() for core2300 modules
            {GPIO_OUT_LED_HOST,         OUTPUT_CLEAR},
            {GPIO_OUT_LED_LINK,         OUTPUT_CLEAR},  // inverted after GRG_GpioInit() for core2300 modules
            {GPIO_OUT_LED_ASSERT,       OUTPUT_CLEAR},
            {GPIO_OUT_LED_HEART_BEAT,   OUTPUT_CLEAR},
            {GPIO_OUT_LED_DEBUG,        OUTPUT_CLEAR},
            {GPIO_IN_PAIRING_BUTTON,    INPUT_DOWN  },
            {GPIO_IN_ETH_PHY_IRQ,       INPUT_UP    },
            {GPIO_OUT_ETH_PHY_RESET,    OUTPUT_CLEAR},  // place in reset initially
            {GPIO_OUT_USB_PHY_RESET,    OUTPUT_CLEAR},  // place in reset initially
            {GPIO_OUT_USB_HUB_RESET,    OUTPUT_CLEAR},  // place in reset initially // Rex only, but initialized on both
            {GPIO_SPARE1,               INPUT_DOWN  },
            {GPIO_SPARE2,               INPUT_DOWN  },
            {GPIO_SPARE3,               INPUT_DOWN  },
            {GPIO_SPARE4,               INPUT_DOWN  },
            {GPIO_SPARE5,               INPUT_DOWN  }
        };

        const uint8 numOfGpioStates = ARRAYSIZE(gpioStates);
        GRG_GpioInit(gpioStates, numOfGpioStates);
    }

    if (GRG_IsDeviceSpartan(GRG_GetPlatformID()) && (GRG_GetVariantID() == GRG_VARIANT_SPARTAN6_CORE2300))
    {
        // Core2300 have inverted logic on the LEDs for activity and link
        // Set the default states
        GRG_TurnOffLed(LI_LED_SYSTEM_ACTIVITY);
        GRG_TurnOffLed(LI_LED_SYSTEM_LINK);
    }
    LEON_UartWaitForTx();

    // start the heartbeat LED blinking
    GRG_GpioPulse(GPIO_OUT_LED_HEART_BEAT, GRG_PULSE_SLOW);
    LEON_UartWaitForTx();

#ifdef GE_CORE
    imainPart2(); // TODO: should this be the same as BUILD_FOR_SIM
#elif defined(BUILD_FOR_SIM)
    STORAGE_persistentDataInitialize(&imainPart2, MEMORY_ONLY_STORAGE);
#else
    {
        // Initialize i2c
        enum GRG_PlatformID platform = GRG_GetPlatformID();
        if (platform == GRG_PLATFORMID_KINTEX7_DEV_BOARD)
        {
            KC705_Select(KC705_MUX_FMC_HPC_IIC, &_TOP_i2cInitDone);
        }
        else
        {
            // No special initialization required
            _TOP_i2cInitDone(TRUE); // Arg is for success
        }
    }
#endif

    LEON_UnlockIrq(flags);
    return &TASKSCH_MainLoop;
}


#if !defined(GE_CORE) && !defined(BUILD_FOR_SIM)
/**
* FUNCTION NAME: _TOP_i2cInitDone()
*
* @brief  - A continuation function that is run after i2c is initialized
*
* @return - void
*
* @note   - Kintex board needs to setup i2c MUX.  Other platforms this is a stub
*
*/
static void _TOP_i2cInitDone(boolT success)
{
    iassert_TOPLEVEL_COMPONENT_1(success, I2C_FAILURE, __LINE__);

#ifdef BB_GE_COMPANION
    STORAGE_persistentDataInitialize(&imainPart2, USE_BB_STORAGE);
#else
    const enum GRG_PlatformID platform = GRG_GetPlatformID();

    if (platform == GRG_PLATFORMID_ASIC)
    {
        // The ITC2052 is a point to point products, so there is no firmware requirement on a
        // EEPROM to hold a MAC address.  However an EEPROM may be installed for product tracking.
        // For the ITC2052, we check if an EEPROM is present and use it if it is there, but it is
        // also permitted to run without an EEPROM.
        if (GRG_GetVariantID() == GRG_VARIANT_ASIC_ITC2052)
        {
            uint8* intentionallyNullData = NULL;
            _TOP_checkForEeprom(intentionallyNullData, 0);
        }
        else
        {
            const boolT eepromInstalled = TRUE;
            _TOP_eepromInit(eepromInstalled);
        }
    }
    else
    {
        ATMEL_init(&_TOP_atmelInitDone);
    }
#endif // BB_GE_COMPANION
}

#ifndef BB_GE_COMPANION
/**
* FUNCTION NAME: _TOP_eepromInit()
*
* @brief  - Initialize EEPROM if it is installed
*
* @return - void
*
* @note   -
*
*/
static void _TOP_eepromInit(boolT eepromInstalled)
{
    enum STORAGE_TYPE st = USE_EEPROM_STORAGE;
    if (eepromInstalled)
    {
        const uint8 numEepromPages = 32;
        EEPROM_Init(0, 0, numEepromPages);
    }
    else
    {
        st = MEMORY_ONLY_STORAGE;
    }
    STORAGE_persistentDataInitialize(&imainPart2, st);
}


/**
* FUNCTION NAME: _TOP_checkForEeprom()
*
* @brief  - Check if EEPROM exists by performing radom I2C read operations with 3 reattempts
*
* @return - void
*
* @note   -
*
*/
static void _TOP_checkForEeprom(uint8* data, uint8 count)
{
    static uint8 readAttempt = 0;
    static uint8 i2cTestReadBuffer;
    if (data != NULL)
    {
        ilog_TOPLEVEL_COMPONENT_0(ILOG_MAJOR_EVENT, EEPROM_INSTALLED);
        const boolT eepromInstalled = TRUE;
        _TOP_eepromInit(eepromInstalled);
    }
    else if(readAttempt > 2)
    {
        ilog_TOPLEVEL_COMPONENT_0(ILOG_MAJOR_EVENT, EEPROM_NOT_INSTALLED);
        const boolT eepromInstalled = FALSE;
        _TOP_eepromInit(eepromInstalled);
    }
    else
    {
        readAttempt++;
        const uint8 bus = 0;
        const uint8 i2cAddr = 0x50;
        enum GRG_I2cSpeed speed = GRG_I2C_SPEED_FAST;
        const uint8 readByteCount = 1;
        GRG_I2cReadASync(
            bus,
            i2cAddr,
            speed,
            &i2cTestReadBuffer,
            readByteCount,
            &_TOP_checkForEeprom);
    }
}
#endif // BB_GE_COMPANION
#endif // !defined(GE_CORE) && !defined(BUILD_FOR_SIM)


#if !defined(GE_CORE) && !defined(BUILD_FOR_SIM) && !defined(BB_GE_COMPANION)
/**
* FUNCTION NAME: _TOP_atmelInitDone()
*
* @brief  - A continuation function that is run after ATMEL_Init()
*
* @return - void
*
* @note   -
*
*/
static void _TOP_atmelInitDone
(
    boolT dataAndOtpZonesLocked,
    boolT configZoneLocked
)
{
    enum STORAGE_TYPE storageType;
    const boolT isPeriodic = FALSE;
    const uint32 timeoutInMs = 60000;
    if (dataAndOtpZonesLocked && configZoneLocked)
    {
        TOP_AuthenticateInit();

        storageType = USE_ATMEL_STORAGE;
    }
    else
    {
        // The authentication chip is not locked, so we allow the system to operate normally for a
        // little while before killing the system.
        // No need to permanently store this timer handler because we never use it after this function.
        TIMING_TimerHandlerT unlockedExecutionTimer = TIMING_TimerRegisterHandler(
                &_TOP_killSystemTimerHandler, isPeriodic, timeoutInMs);
        TIMING_TimerStart(unlockedExecutionTimer);
        storageType = MEMORY_ONLY_STORAGE;
    }
    STORAGE_persistentDataInitialize(&imainPart2, storageType);
}
#endif


#if !defined(GE_CORE) && !defined(BUILD_FOR_SIM)
static void _TOP_killSystemTimerHandler(void)
{
    TOP_killSytem(DISABLING_SYS);
}
#endif


/**
* FUNCTION NAME: imainPart2()
*
* @brief  - System initialization is broken into two parts in order to facilitate loading of
*           persistent data from storage.  This is the second part of the initialization which runs
*           after persistent storage has been loaded.
*
* @return - void.
*/
static void imainPart2(void)
{
    // Set pseudo random number generator seed
    RANDOM_Init(); // NOTE: This must be done here, as STORAGE_persistentData must be initialized first

    // The CONFIGURATION_BITS persistent variable is required, but may not
    // always be present.  Initialize the value, but do not save it permanently
    // in persistent storage.
    if (!STORAGE_varExists(CONFIGURATION_BITS))
    {
        // For production this unit must be set, but not in a debug environment
        // Warn the developer with a big fatal error message
        ilog_TOPLEVEL_COMPONENT_0(ILOG_FATAL_ERROR, NO_CFG_VAR_EXISTS);

        STORAGE_varCreate(CONFIGURATION_BITS)->doubleWord =
            (1 << TOPLEVEL_SUPPORT_USB2_HISPEED_OFFSET)                     |
            (1 << TOPLEVEL_SUPPORT_MSA_OFFSET)                              |
            (1 << TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET)                        |
            (0 << TOPLEVEL_USE_ETHERNET_FRAMING_OFFSET)                     |
            (1 << TOPLEVEL_ENET_PHY_MII_SUPPORT_OFFSET)                     |
            (1 << TOPLEVEL_ENET_PHY_GMII_SUPPORT_OFFSET)                    |
            (0 << TOPLEVEL_USE_BCAST_NET_CFG_PROTO_OFFSET)                  |
            (0 << TOPLEVEL_ENABLE_VHUB_OFFSET)                              |
            (0 << TOPLEVEL_ENABLE_DCF_OFFSET)                               |
            (0 << TOPLEVEL_BLOCK_MASS_STORAGE_OFFSET)                       |
            (0 << TOPLEVEL_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET)                |
            (0 << TOPLEVEL_DEPRECATED_REFUSE_PAIRING_WITH_UNBRANDED_OFFSET) |
            (0 << TOPLEVEL_DEPRECATED_REFUSE_PAIRING_WITH_LEGACY_OFFSET)    |
            (0 << TOPLEVEL_ENABLE_LEX_EXTERNAL_CLOCK_OFFSET)                |
            (0 << TOPLEVEL_DISABLE_REX_EXTERNAL_CLOCK_OFFSET)               |
            (0 << TOPLEVEL_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET)      |
            (0 << TOPLEVEL_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET)  |
            (0 << TOPLEVEL_DISABLE_NETWORK_CFG_CHANGES_OFFSET)              |
            (0 << TOPLEVEL_ENABLE_DHCP_OPTION_60_OFFSET);
    }

    // Vhub Configuration Options
    if (!STORAGE_varExists(VHUB_CONFIGURATION))
    {
        union STORAGE_VariableData* storageVar = STORAGE_varCreate(VHUB_CONFIGURATION);
        storageVar->bytes[0] = NUM_OF_VPORTS - 1; // # of downstream ports (not including Vport 0)
        storageVar->halfWords[1] = VHUB_VENDOR_ID;
        storageVar->halfWords[2] = VHUB_PRODUCT_ID;
    }

    // Update the values read out of the storage layer based on other information sources
    TOPLEVEL_configBitsApplyModifications();

    // Update STORAGE vars with variant overrides
    TOPLEVEL_storageVarApplyVariantRestrictions();

    // Output Lex or Rex to uart
    ilog_TOPLEVEL_COMPONENT_0(ILOG_MINOR_EVENT, GRG_IsDeviceLex() ? THIS_IS_LEX : THIS_IS_REX);

    // Log which platform we are on
    switch (GRG_GetPlatformID())
    {
        case GRG_PLATFORMID_KINTEX7_DEV_BOARD:
            ilog_TOPLEVEL_COMPONENT_0(ILOG_MINOR_EVENT, KINTEX_PLATFORM);
            break;
        case GRG_PLATFORMID_SPARTAN6:
            ilog_TOPLEVEL_COMPONENT_0(ILOG_MINOR_EVENT, SPARTAN_PLATFORM);
            switch (GRG_GetVariantID())
            {
                case GRG_VARIANT_SPARTAN6_UON:
                    ilog_TOPLEVEL_COMPONENT_0(ILOG_MINOR_EVENT, SPARTAN_UON_VARIANT);
                    break;

                case GRG_VARIANT_SPARTAN6_CORE2300:
                    ilog_TOPLEVEL_COMPONENT_0(ILOG_MINOR_EVENT, SPARTAN_CORE2300_VARIANT);
                    break;

                default:
                    ilog_TOPLEVEL_COMPONENT_1(ILOG_MINOR_EVENT, UNKNOWN_VARIANT, GRG_GetVariantID());
                    break;
            }
            break;

        case GRG_PLATFORMID_ASIC:
            ilog_TOPLEVEL_COMPONENT_0(ILOG_MINOR_EVENT, ASIC_PLATFORM);
            const enum GRG_VariantID vid = GRG_GetVariantID();
            iassert_TOPLEVEL_COMPONENT_1(
                   vid == GRG_VARIANT_ASIC_ITC1151
                || vid == GRG_VARIANT_ASIC_ITC2051
                || vid == GRG_VARIANT_ASIC_ITC2052
                || vid == GRG_VARIANT_ASIC_ITC2053
                || vid == GRG_VARIANT_ASIC_ITC2054,
                UNSUPPORTED_VID,
                vid);
            ilog_TOPLEVEL_COMPONENT_1(ILOG_MINOR_EVENT, VARIANT_ID, vid);

            // The ITC2053 is only allowed to operate using RGMII and a PHY with a specific
            // identifier.  See phyReadId3() in linkmgr_enet.c for PHY identification.
            if (vid == GRG_VARIANT_ASIC_ITC2053)
            {
                const enum linkType link = GRG_GetLinkType();
                iassert_TOPLEVEL_COMPONENT_1(link == RGMII, ILLEGAL_LINK_TYPE_WITH_2053, link);
            }
            break;

        default:
            ilog_TOPLEVEL_COMPONENT_1(ILOG_MINOR_EVENT, UNKNOWN_PLATFORM, GRG_GetPlatformID());
            break;
    }

    // Initialize the PLL subsystem
    PLL_Init(); //NOTE: sets up external clock (for Rex Hub chip on Icron products)

    boolT uploadingHubFw = FALSE;
#ifndef GE_CORE
    if (GRG_IsDeviceRex())
    {
        // BB Controls hub
#ifndef BB_GE_COMPANION
        // Release hub from reset
        GRG_GpioSet(GPIO_OUT_USB_HUB_RESET);
        if (GRG_GetPlatformID() == GRG_PLATFORMID_ASIC)
        {
            uploadingHubFw = TRUE;
            // Start to measure the time once Cypress Hx3 hub released from reset
            LEON_TimerValueT hubResetTime = LEON_TimerRead();
            // Ensure Cypress Hx3 hub has been out of reset for 5ms
            for (LEON_TimerValueT currTime = LEON_TimerRead();
                 LEON_TimerCalcUsecDiff(hubResetTime, currTime) < CYPRESS_RESET_TIME_IN_US;
                 currTime = LEON_TimerRead())
            {
            }

            TOPLEVEL_tryCypressHx3HubFirmwareUpgrade(&imainPart3);
        }
#endif
    }
#endif

    if (!uploadingHubFw)
    {
        // Tell the completion handler that we we tried to access the hub and it didn't respond
        imainPart3(TOPLEVEL_CYPRESS_HX3_HUB_NOT_RESPONDING);
    }
}


static void imainPart3(enum TOPLEVEL_CypressHx3UpgradeResult result)
{
    if (GRG_IsDeviceRex() && (GRG_GetPlatformID() == GRG_PLATFORMID_ASIC))
    {
        if (result == TOPLEVEL_CYPRESS_HX3_SUCCESS)
        {
            ilog_TOPLEVEL_COMPONENT_0(ILOG_MINOR_EVENT, CYPRESS_HX3_PROGRAMMING_SUCCESS);
        }
        else if (result == TOPLEVEL_CYPRESS_HX3_HUB_NOT_RESPONDING)
        {
            ilog_TOPLEVEL_COMPONENT_0(ILOG_MINOR_EVENT, CYPRESS_HX3_NOT_FOUND);
        }
        else
        {
            iassert_TOPLEVEL_COMPONENT_0(FALSE, CYPRESS_HX3_PROGRAMMING_FAILURE);
        }
    }

    enum CLM_XUSBLinkMode linkMode;
    uint64 ownMACAddr;

#ifndef GE_CORE
    {
        const boolT vhubEnabled =
                ((   STORAGE_varGet(CONFIGURATION_BITS)->doubleWord
                  >> TOPLEVEL_ENABLE_VHUB_OFFSET) & 0x1);
        const boolT l2FramingEnabled =
                ((   STORAGE_varGet(CONFIGURATION_BITS)->doubleWord
                  >> TOPLEVEL_USE_ETHERNET_FRAMING_OFFSET) & 0x1);

        if (l2FramingEnabled)
        {
            linkMode = vhubEnabled ? LINK_MODE_MULTI_REX : LINK_MODE_POINT_TO_POINT;
            if (!getOwnMACAddress(&ownMACAddr))
            {
                // MAC addr doesn't exist in storage but we have layer 2 framing enabled.
                // In this case, HW wants the extender to work for 60 seconds in direct-link mode so
                // that it is still testable.
                ilog_TOPLEVEL_COMPONENT_0(ILOG_WARNING, L2_ENABLED_BUT_NO_MAC_ADDR);
                linkMode = LINK_MODE_DIRECT;
                ownMACAddr = 0;
#if !defined(GE_CORE) && !defined(BUILD_FOR_SIM)
                const boolT isPeriodic = FALSE;
                const uint32 timeoutInMs = 60000;
                TIMING_TimerHandlerT killSystemTimer = TIMING_TimerRegisterHandler(
                    & _TOP_killSystemTimerHandler, isPeriodic, timeoutInMs);
                TIMING_TimerStart(killSystemTimer);
#endif
            }
        }
        else
        {
            ownMACAddr = 0;
            linkMode = LINK_MODE_DIRECT;
        }
        iassert_TOPLEVEL_COMPONENT_0(l2FramingEnabled || !vhubEnabled, INVALID_NETWORK_MODE);
    }
#else
    linkMode = LINK_MODE_DIRECT;
    ownMACAddr = 0;
#endif

    const boolT isASIC = (GRG_GetPlatformID() == GRG_PLATFORMID_ASIC);
    // No PHY_RESET on the ASIC -- the pin is used for configuring slew rate
    if (!isASIC)
    {
        GRG_GpioSet(GPIO_OUT_USB_PHY_RESET);
    }
    // Configure the ULM
    ULM_Init(GRG_IsDeviceLex(), isASIC);

    // output USB speed to the uart
    ilog_TOPLEVEL_COMPONENT_0(
        ILOG_MINOR_EVENT,
        ULM_usb2HighSpeedEnabled() ? USB_2_0_SPEED : USB_1_1_SPEED);

    LEON_UartWaitForTx();

    XCSR_Init(GRG_IsDeviceLex(), linkMode == LINK_MODE_DIRECT);
    LEON_UartWaitForTx();

    if (GRG_IsDeviceLex())
    {
        XLR_Init();
    }
    else
    {
        XRR_Init();
    }
    LEON_UartWaitForTx();

    GRG_GpioSet(GPIO_OUT_ETH_PHY_RESET);
    LINKMGR_Init(linkMode, ownMACAddr);

    // Copy Icron specific code or Crestron specific code to instruction RAM
    // Need to read Flash var 14 to copy xusbcfg_text to IRAM based on type of protocol
    //NETCFG_CopyXusbcfgProtocolOverlay();

#ifndef GE_CORE
    // Networking is not used in direct mode or on variants that don't support it
    if(linkMode != LINK_MODE_DIRECT)
    {
        NET_initialize(ownMACAddr);
        if (GRG_VariantSupportsNetCfg(GRG_GetVariantID()))
        {
            NETCFG_Initialize(linkMode);
        }
    }
#endif


    TOP_RxMessageInit(linkMode);
    ilog_TOPLEVEL_COMPONENT_0(ILOG_MINOR_EVENT, LINK_INIT_COMPLETE);
    LEON_UartWaitForTx();

    // Enable inter CPU communication
    LEON_EnableIrq(IRQ_XCSR1);

    if (GRG_IsDeviceLex())
    {
#ifndef GE_CORE
        if (linkMode == LINK_MODE_MULTI_REX)
        {
            LEX_Init(&VHUB_HostPortMessage, &sendUlmNegotiatedSpeedMsgToBB);

            uint8 firstVPort;
            uint8 lastVPort;
            LINKMGR_getVPortBounds(&firstVPort, &lastVPort);
            const uint8 numVPorts = (lastVPort - firstVPort) + 1;
            const union STORAGE_VariableData* vhubConfig = STORAGE_varGet(VHUB_CONFIGURATION);
            const uint16 vendorId = vhubConfig->halfWords[1];
            const uint16 productId = vhubConfig->halfWords[2];
            VHUB_Init(numVPorts, vendorId, productId);
        }
        else
#endif
        {
            LEX_Init(&LEXULM_SendLexUlmMsgToRex, &sendUlmNegotiatedSpeedMsgToBB);
        }

        DTT_Init();
        DEVMGR_Init(&sendDevConnectionStatusMsgToBB);
        PARSER_init();
        SYSCTRLQ_Init();
    }
    else // Rex
    {
        // Enable the Rexulm
        REXULM_Init(&sendStatsToBB);
    }

    // Log all persistent variables as they exist upon completion of system initialization
    STORAGE_logAllVars(ILOG_MINOR_EVENT);

    ilog_TOPLEVEL_COMPONENT_0(ILOG_MINOR_EVENT, APP_INIT_COMPLETE);

    LEON_UartWaitForTx();

    crcInit();
    sendReadyMsgToBB();
    sendVersionToBB();
}


/**
* FUNCTION NAME: logBuildInfo()
*
* @brief  - logs the build version & time
*
* @return - void
*
* @note   - The defines come from options.h & imain.c_CFLAGS
*
*/
static void logBuildInfo(ilogLevelT level)
{
    ilog_TOPLEVEL_COMPONENT_3(level, SW_VERSION, SOFTWARE_MAJOR_REVISION, SOFTWARE_MINOR_REVISION, SOFTWARE_DEBUG_REVISION);
    ilog_TOPLEVEL_COMPONENT_3(level, BUILD_DATE, MAKE_BUILD_YEAR, MAKE_BUILD_MONTH, MAKE_BUILD_DAY);
    ilog_TOPLEVEL_COMPONENT_3(level, BUILD_TIME, MAKE_BUILD_HOUR, MAKE_BUILD_MINUTE, MAKE_BUILD_SECOND);
#ifdef DEBUG
    ilog_TOPLEVEL_COMPONENT_0(level, DEBUG_BUILD);
#endif
}


/**
* FUNCTION NAME: PrintSwVersion()
*
* @brief  - Print out the current software version and build date and time
*
* @return - void
*
* @note   -
*
*/
void PrintSwVersion(void)
{
    logBuildInfo(ILOG_USER_LOG);
}


/**
* FUNCTION NAME: TOPLEVEL_DEBUG_ASSERT()
*
* @brief  - Make GE assert to test log messages
*
* @return - void
*
* @note   - INVALID_NETWORK_MODE is just sample
*
*/
void TOPLEVEL_DEBUG_ASSERT(uint8 input)
{
    if(input == 0)
        iassert_TOPLEVEL_COMPONENT_3(false, DEBUG_ASSERT, 11,22,33);
    else
        return;
}

/**
 * FUNCTION NAME: TOPLEVEL_printDeviceType
 *
 * @brief  - Prints whether this device is a LEX or a REX.
 *
 * @return - void
 */
void TOPLEVEL_printDeviceType(void)
{
    ilog_TOPLEVEL_COMPONENT_0(
        ILOG_USER_LOG, GRG_IsDeviceLex() ? TOPLEVEL_DEVICE_IS_LEX : TOPLEVEL_DEVICE_IS_REX);
}

/**
 * FUNCTION NAME: TOPLEVEL_getEcoStatus
 *
 * @brief  - Prints the status of ECO Flag, (1 = ECO detected)
 *
 * @return - void
 */
void TOPLEVEL_getEcoStatus(void)
{
    ilog_TOPLEVEL_COMPONENT_1(ILOG_USER_LOG, ECO1_FLAG_VALUE, ecoStatus);
}


#ifndef GE_CORE
/**
* FUNCTION NAME: getOwnMACAddress()
*
* @brief  - Reads this device's MAC address out of persistent storage.
*
* @return - TRUE if the MAC address exists in persistent storage, FALSE otherwise
*
* @note   - The 48 bit MAC address is packed into the lower bits of the 64 bit
*           return value.
*/
static boolT getOwnMACAddress(uint64 *mac)
{
    if (STORAGE_varExists(MAC_ADDR))
    {
        union STORAGE_VariableData* macAddr = STORAGE_varGet(MAC_ADDR);
        // Because the MAC is packed into the first 6 bytes of the data, we need to right shift the
        // data by 16 bits in order to get it into the lowest 48 bits of a uint64.
        *mac = macAddr->doubleWord >> 16;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif


/**
* FUNCTION NAME: TOP_killSytem()
*
* @brief  - Permanently disable the link manager.  This should allow the system
*           to be queried using icmd, but USB extension will not function.
*
* @return -
*/
void TOP_killSytem(TOPLEVEL_COMPONENT_ilogCodeT logMsg)
{
    LINKMGR_disablePhy();
    ilog_TOPLEVEL_COMPONENT_0(ILOG_FATAL_ERROR, logMsg);
}


/**
* FUNCTION NAME: printString()
*
* @brief  - prints a string out the uart
*
* @return - void
*
* @note   -
*
*/
void printString
(
    const char * str    // String to print
)
{
    UART_printf(str);
}

#ifdef BB_GE_COMPANION
static void checkUartRx(TASKSCH_TaskT task, uint32 arg)
{
    UART_ProcessRx();
    TASKSCH_StartTask(task);
}
#endif

/**
* FUNCTION NAME: GetEcoStatus()
*
* @brief  - sets a global variable if the ECO flag is set
*
* @return - void
*
* @note   - needs to be called before everything else, to not effect
*           system operation
*
*/
static void getEcoStatus(void)
{
    GRG_ECOMuxEnable(true);
    ecoStatus = ULM_GetEcoBitState();
    GRG_ECOMuxEnable(false);
}

