///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2011, 2012, 2013
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
//!   @file  -  gpio.c
//
//!   @brief -  Contains the driver for accessing the GPIO
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <ibase.h>
#include <itypes.h>

#include <timing_timers.h>
#include <leon_traps.h>

#include "grg_loc.h"

#include <gpio.h>       // Project gpio defines
#include <grg_gpio.h>   // Exposed header for gpio function definitions
#include <grg_led.h>

/************************ Defined Constants and Macros ***********************/
#define NUM_OF_GPIO 16 // NOTE: Spectareg doesn't define this, so it is done here


/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static uint8 fastPulseCount = 0;
static uint16 gpioSlowPulseMask = 0;
static uint16 gpioFastPulseMask = 0;

/************************ Local Function Prototypes **************************/
static void pulseGPIOs(void) __attribute__((section(".ftext")));
static inline uint8 gpioToUint8(enum gpioT gpio);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: gpioToUint8()
*
* @brief  - A casting function, to try and reduce casting errors
*
* @return - the gpio in uint8 form
*
* @note   -
*
*/
static inline uint8 gpioToUint8
(
    enum gpioT gpio // GPIO pin
)
{
    return CAST(gpio, enum gpioT, uint8);
}


/**
* FUNCTION NAME: pulseGPIOs()
*
* @brief  - A timer callback function that pulses all of the GPIOs
*
* @return - void
*/
static void pulseGPIOs(void)
{
    uint32 readValue;
    uint32 writeValue;
    uint16 pulseMask = 0;

    fastPulseCount++;
    pulseMask = gpioFastPulseMask;
    // The slow pulse happens once every five fast pulses.
    if(fastPulseCount >= 5)
    {
        pulseMask |= gpioSlowPulseMask;
        fastPulseCount = 0;
    }

    // read
    readValue = GRG_GPIO_OUT_READ_REG(GRG_BASE_ADDR);

    // modify
    writeValue = readValue ^ pulseMask;

    // log
    ilog_GRG_COMPONENT_3(ILOG_DEBUG, GPIO_PULSE_TIMER, pulseMask, readValue, writeValue);

    // write
    GRG_GPIO_OUT_WRITE_REG(GRG_BASE_ADDR, writeValue);
}


/**
* FUNCTION NAME: GRG_GpioInit()
*
* @brief  - Initializes the GPIO direction register
*
* @return - nothing
*
* @note   -
*
*/
void GRG_GpioInit
(
    const struct gpioInitStates * pGpioStates,
    uint8 numOfGpioInitStates
)
{
    enum GRG_VariantID grgVariantID = GRG_GetVariantID();
    // On the Core2300 and ASIC variants which are used in core module designs, we output a
    // toggling signal on the activity LED so that customers migrating from the Core2100 platform
    // can identify whether they are using a Core2300 or Core2100.
    switch (grgVariantID)
    {
        case GRG_VARIANT_SPARTAN6_CORE2300:
        case GRG_VARIANT_ASIC_ITC1151:
        case GRG_VARIANT_ASIC_ITC2052:
        case GRG_VARIANT_ASIC_ITC2051:
        case GRG_VARIANT_ASIC_ITC2054:
            {
                const uint32 activityLedBit = (1 << gpioToUint8(GPIO_OUT_LED_ACTIVITY));
                // Configure the activity LED as an output
                GRG_GPIO_DIR_WRITE_REG(GRG_BASE_ADDR, activityLedBit);

                for (uint8 i = 0; i <= 30; i++)
                {
                    GRG_GPIO_OUT_WRITE_REG(GRG_BASE_ADDR, (i & 0x1) ? activityLedBit : 0);
                    LEON_TimerWaitMicroSec(5000);
                }
            }
            break;
        case GRG_VARIANT_SPARTAN6_UON:
        case GRG_VARIANT_ASIC_ITC2053:      // currently unsupprted:
            break;
        default:
            iassert_GRG_COMPONENT_1(FALSE, UNSUPPORTED_VARIANT_ID, grgVariantID);
            break;
    }

    TIMING_TimerHandlerT LEDTimer;
    uint32 dirOut = 0;
    uint32 outputValue = 0;
    uint8 i;

    // Initialize GPIO pulsing timer
    LEDTimer = TIMING_TimerRegisterHandler(&pulseGPIOs, TRUE, GRG_GPIO_PULSE_RATE);
    TIMING_TimerStart(LEDTimer);

    // Register LED locator pattern timer
    GRG_LedRegisterLocatorPatternTimer();

    // Go through all the gpio States to initialize
    for (i = 0; i < numOfGpioInitStates; i++)
    {
        uint32 bit = 1 << gpioToUint8(pGpioStates[i].gpio);

        // Input is already the default, so lets find the output
        if ((pGpioStates[i].type == OUTPUT_SET) || (pGpioStates[i].type == OUTPUT_CLEAR))
        {
            dirOut |= bit;

            // Set all pins that need to be set.  Clear is the default
            if (pGpioStates[i].type == OUTPUT_SET)
            {
                outputValue |= bit;
            }
        }
    }

    // Actually modify the registers
    ilog_GRG_COMPONENT_2(ILOG_DEBUG, GPIO_INIT, dirOut, outputValue);
    GRG_GPIO_OUT_WRITE_REG(GRG_BASE_ADDR, outputValue);
    GRG_GPIO_DIR_WRITE_REG(GRG_BASE_ADDR, dirOut);

    // Calculate the bitfield patterns for pull-ups and drive strength (ASIC only).
    // Also set up the CLM TX drive strength and slew rate (ASIC only).
    if (GRG_GetPlatformID() == GRG_PLATFORMID_ASIC)
    {
        uint32 gpioPull = 0;        // This makes all pins float by default
        uint32 driveStrength = 0;   // This sets all outputs to 4mA by default

        for (i = 0; i < numOfGpioInitStates; i++)
        {
            const uint8 ioNumber = gpioToUint8(pGpioStates[i].gpio);

            // Set drive strength to 16mA for all GPIOs
            driveStrength |= (GPIO_16mA << (ioNumber * GPIO_DRIVE_STRENGTH_BITFIELD_WIDTH));

            enum gpioPull pullDir =
                (pGpioStates[i].type == INPUT_UP) ?   GPIO_PULL_UP :
                (pGpioStates[i].type == INPUT_DOWN) ? GPIO_PULL_DOWN :
                                                      GPIO_NO_PULL;
            gpioPull |= (pullDir << (ioNumber * GPIO_PULL_BITFIELD_WIDTH));
        }

        GRG_IOCFG_GPIODRIVE_WRITE_REG(GRG_BASE_ADDR, driveStrength);
        GRG_IOCFG_GPIOPULL_WRITE_REG(GRG_BASE_ADDR, gpioPull);

        // Set up CLM TX drive strength and slew rate, which are controlled
        // by input GPIOs on the ASIC.
        const enum _GRG_ClmTxDriveStrengthGPIO driveStrengthRead = GRG_CLM_TX_DRIVE_STR_GPIO_8MA; // Blackbird specific

        const enum _GRG_ClmTxDriveStrengthIOCFG driveStrengthWrite =
            driveStrengthRead == GRG_CLM_TX_DRIVE_STR_GPIO_8MA  ? GRG_CLM_TX_DRIVE_STR_IOCFG_8MA  :
            driveStrengthRead == GRG_CLM_TX_DRIVE_STR_GPIO_12MA ? GRG_CLM_TX_DRIVE_STR_IOCFG_12MA :
            driveStrengthRead == GRG_CLM_TX_DRIVE_STR_GPIO_16MA ? GRG_CLM_TX_DRIVE_STR_IOCFG_16MA :
                                                                  GRG_CLM_TX_DRIVE_STR_IOCFG_INVALID;

        ilog_GRG_COMPONENT_2(
            ILOG_MAJOR_EVENT,
            DRIVE_STRENGTH,
            driveStrengthRead,
            driveStrengthWrite);

        iassert_GRG_COMPONENT_1(
                driveStrengthWrite != GRG_CLM_TX_DRIVE_STR_IOCFG_INVALID,
                INVALID_CLMTX_DRIVE_STR,
                driveStrengthRead);

        uint32 clmTxRegister = GRG_IOCFG_CLMTX_READ_REG(GRG_BASE_ADDR);
        clmTxRegister = GRG_IOCFG_CLMTX_DRIVE_SET_BF(
                clmTxRegister,
                  driveStrengthWrite << 0
                | driveStrengthWrite << 2
                | driveStrengthWrite << 4
                | driveStrengthWrite << 6
                | driveStrengthWrite << 8
                | driveStrengthWrite << 10
                | driveStrengthWrite << 12);

        // GRG_CLM_TX_SLEW_SLOW will only work in GMII mode. For RGMII, slew rate has to be GRG_CLM_TX_SLEW_FAST
        const enum _GRG_ClmTxSlewRate slewRate = GRG_CLM_TX_SLEW_SLOW ;  // Blackbird specific
        clmTxRegister = GRG_IOCFG_CLMTX_SLEW_SET_BF(
                clmTxRegister,
                  slewRate << 0
                | slewRate << 1
                | slewRate << 2
                | slewRate << 3
                | slewRate << 4
                | slewRate << 5
                | slewRate << 6);

        GRG_IOCFG_CLMTX_WRITE_REG(GRG_BASE_ADDR, clmTxRegister);
    }
}


/**
* FUNCTION NAME: GRG_GpioRead()
*
* @brief  - Read a GPIO pin
*
* @return - TRUE for set, FALSE otherwise
*/
boolT GRG_GpioRead
(
    enum gpioT pin  // The GPIO pin to read
)
{
    const uint8 gpioPinNum = gpioToUint8(pin);
    const uint32 tmp = GRG_GPIO_IN_READ_REG(GRG_BASE_ADDR);
    const boolT gpioVal = ((tmp >> gpioPinNum) & 0x1);

    ilog_GRG_COMPONENT_2(ILOG_DEBUG, GPIO_READ, gpioPinNum, gpioVal);

    return gpioVal;
}



/**
* FUNCTION NAME: GRG_GpioSet()
*
* @brief  - Sets a gpio pin
*
* @return - void
*
* @note   -
*
*/
void GRG_GpioSet
(
    enum gpioT pin  // The GPIO pin to set
)
{
    uint32 gpioBit = 1 << gpioToUint8(pin);
    uint32 tmp;

    // read-modify-write
    tmp = GRG_GPIO_OUT_READ_REG(GRG_BASE_ADDR);
    GRG_GPIO_OUT_WRITE_REG(GRG_BASE_ADDR, tmp | gpioBit);

    // take this pin out of the pulse masks
    gpioSlowPulseMask &= ~gpioBit;
    gpioFastPulseMask &= ~gpioBit;

    ilog_GRG_COMPONENT_1(ILOG_DEBUG, GPIO_SET, pin);
}


/**
* FUNCTION NAME: GRG_GpioClear()
*
* @brief  - Clears a gpio pin
*
* @return - void
*
* @note   -
*
*/
void GRG_GpioClear
(
    enum gpioT pin // The gpio pin to clear
)
{
    uint32 gpioBitClearMask = ~(1 << gpioToUint8(pin));
    uint32 tmp;

    // read-modify-write
    tmp = GRG_GPIO_OUT_READ_REG(GRG_BASE_ADDR);
    GRG_GPIO_OUT_WRITE_REG(GRG_BASE_ADDR, tmp & gpioBitClearMask);

    // take this pin out of the pulse masks
    gpioSlowPulseMask &= gpioBitClearMask;
    gpioFastPulseMask &= gpioBitClearMask;

    ilog_GRG_COMPONENT_1(ILOG_DEBUG, GPIO_CLEAR, pin);
}


/**
* FUNCTION NAME: GRG_GpioPulse()
*
* @brief  - Pulse a GPIO pin
*
* @return - void
*
* @note   - This is intended for LEDs.  All LEDs blink together.
*/
void GRG_GpioPulse
(
    enum gpioT pin, // The gpio pin to pulse
    enum GRG_PulseRate rate
)
{
    uint32 gpioBit = 1 << gpioToUint8(pin);
    uint16* pulseMask;
    uint16* otherPulseMask;

    if(rate == GRG_PULSE_SLOW)
    {
        pulseMask = &gpioSlowPulseMask;
        otherPulseMask = &gpioFastPulseMask;
    }
    else
    {
        pulseMask = &gpioFastPulseMask;
        otherPulseMask = &gpioSlowPulseMask;
    }

    // add to the pulse mask
    *pulseMask |= gpioBit;
    // remove from the other pulse mask
    *otherPulseMask &= (~gpioBit);

    ilog_GRG_COMPONENT_1(ILOG_DEBUG, GPIO_PULSE, pin);
}

/**
* FUNCTION NAME: GRG_GpioRegisterIrqHandler()
*
* @brief  - Registering Interrupt Handler for the specified pin
*
* @return - void
*
* @note   - Asserts if pin or interrupt handler are valid.
*
*/
void GRG_GpioRegisterIrqHandler
(
    enum gpioT pin,
    void (*handler)(void)
)
{
    // Sanity checks
    iassert_GRG_COMPONENT_1(pin < NUM_OF_GPIO, INVALID_PIN, pin);
    COMPILE_TIME_ASSERT(GRG_GRG_INTFLG_GPIO0_BF_SHIFT == 0);
    COMPILE_TIME_ASSERT(GRG_GRG_INTFLG_GPIO15_BF_SHIFT == 15);

    _GRG_IrqRegisterHandler(pin, handler);
}

/**
* FUNCTION NAME: GRG_GpioDisableIrq()
*
* @brief  - Masks the Interrupt for the specified pin
*
* @return - void
*
* @note   - Asserts if pin is not valid.
*
*/
void GRG_GpioDisableIrq
(
    enum gpioT pin
)
{
    // Sanity checks
    iassert_GRG_COMPONENT_1(pin < NUM_OF_GPIO, INVALID_PIN, pin);
    COMPILE_TIME_ASSERT(GRG_GRG_INTFLG_GPIO0_BF_SHIFT == 0);
    COMPILE_TIME_ASSERT(GRG_GRG_INTFLG_GPIO15_BF_SHIFT == 15);

    _GRG_IrqDisable(pin);
}

/**
* FUNCTION NAME: GRG_GpioEnableIrq()
*
* @brief  - Enables the interrupt for the specified pin
*
* @return - void
*
* @note   - Asserts if pin or interrupt handler are not valid.
*
*/
void GRG_GpioEnableIrq
(
    enum gpioT pin
)
{
    // Sanity checks
    iassert_GRG_COMPONENT_1(pin < NUM_OF_GPIO, INVALID_PIN, pin);
    COMPILE_TIME_ASSERT(GRG_GRG_INTFLG_GPIO0_BF_SHIFT == 0);
    COMPILE_TIME_ASSERT(GRG_GRG_INTFLG_GPIO15_BF_SHIFT == 15);

    _GRG_IrqEnable(pin);
}


void GRG_GpioFastPulse(enum gpioT pin)
{
    GRG_GpioPulse(pin, GRG_PULSE_FAST);
}


void GRG_GpioSlowPulse(enum gpioT pin)
{
    GRG_GpioPulse(pin, GRG_PULSE_SLOW);
}
