///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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

#include "gpio_loc.h"
#include <uart.h>
#include <top_gpio.h>       // Project gpio defines
#include <gpio.h>           // Exposed header for gpio function definitions
#include <bb_top.h>
#include <leon2_regs.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/
typedef void (*GPIO_interruptHandler)(void);

struct gpioIrqHandlerT
{
    enum gpioIrqT type;             // interrupt type (0: rising, 1: falling, 2: rising & falling, 3: high, 4: low)
    GPIO_interruptHandler handler;  // interrupt handler
};

/***************************** Local Variables *******************************/
static struct gpioIrqHandlerT gpioIrqHandler[TOTAL_NUMBER_GPIOS]; //TODO: don't use hard coded number

extern const struct gpioInitStates gpioStatesLex[];
extern const struct gpioInitStates gpioStatesRex[];
extern const uint8_t numOfGpioStatesLex;
extern const uint8_t numOfGpioStatesRex;

/************************ Local Function Prototypes **************************/
static inline uint8_t gpioToUint8(enum gpioT gpio);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: gpioToUint8()
*
* @brief  - A casting function, to try and reduce casting errors
*
* @return - the gpio in uint8_t form
*
* @note   -
*
*/
static inline uint8_t gpioToUint8
(
    enum gpioT gpio // GPIO pin
)
{
    return CAST(gpio, enum gpioT, uint8_t);
}


/**
* FUNCTION NAME: GpioInit()
*
* @brief  - Initializes the GPIO direction register
*
* @return - nothing
*
* @note   -
*
*/
void GpioInit(void)
{
    const struct GpioIntCfgPortT port = {.enable = false, .type = GPIO_IRQ_RISING_EDGE};
    const struct gpioInitStates *pGpioStates =
        bb_top_IsDeviceLex() ? gpioStatesLex : gpioStatesRex;
    uint8_t numOfGpioInitStates =
        bb_top_IsDeviceLex() ? numOfGpioStatesLex : numOfGpioStatesRex;

    uint32_t dirOut = 0;
    uint32_t outputValue = 0;
    uint8_t i;

    bb_top_ResetGpio(false);        // FPGA GPIO block out of reset
    GPIO_halInit();

    // Go through all the gpio States to initialize
    for (i = 0; i < numOfGpioInitStates; i++)
    {
        uint32_t bit = GPIO_MAP(gpioToUint8(pGpioStates[i].gpio));

        // Input is already the default, so lets find the output
        if (pGpioStates[i].type != INPUT)
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
    ilog_GPIO_COMPONENT_2(ILOG_DEBUG, GPIO_INIT, dirOut, outputValue);
    GPIO_dataWrite(outputValue);
    GPIO_dirWrite(dirOut);

    // Disable & initialize interrupt for all ports
    for (i=0; i < 32; i++)
    {
        GPIO_isrCfgWrite(i, &port);
    }
//    LEON_WRITE_BF(LEON2_IRQCTRL2_INT_MASK, _IMASK, SECONDARY_INT_GPIO_CTRL_INT_MSK);
    LEON_EnableIrq2Bits(SECONDARY_INT_GPIO_CTRL_INT_MSK);
}


/**
* FUNCTION NAME: GpioRead()
*
* @brief  - Read a GPIO pin. Note that we need to read both the input and output registers to get
*           the full value
*
* @return - true for set, false for clear
*/
bool GpioRead
(
    enum gpioT pin  // The GPIO pin to read
)
{
    const uint32_t mask = GPIO_MAP(pin);

    // these instructions get the value of all the GPIO pins, input or output
    const uint32_t directionReg = GPIO_dirRead();
    const uint32_t outputValue  =  directionReg & GPIO_dataReadOutput();
    const uint32_t inputValue   = ~directionReg & GPIO_dataRead();
    const uint32_t gpioValue    = outputValue | inputValue;

    const bool gpioPinVal = mask & gpioValue;

    ilog_GPIO_COMPONENT_2(ILOG_DEBUG, GPIO_READ, pin, gpioPinVal);

    return gpioPinVal;
}


/**
* FUNCTION NAME: GpioSet()
*
* @brief  - Sets a gpio pin
*
* @return - void
*
* @note   -
*
*/
void GpioSet
(
    enum gpioT pin  // The GPIO pin to set
)
{
    uint32_t gpioBit = GPIO_MAP(pin);

    // read-modify-write
    GPIO_dataWriteRMW(gpioBit, gpioBit);
    ilog_GPIO_COMPONENT_1(ILOG_DEBUG, GPIO_SET, pin);
}



/**
* FUNCTION NAME: GpioSetMultipleLeds()
*
* @brief  - Sets gpio led pins (overwrite LED area)
*
* @return - void
*
* @note   -
*
*/
void GpioSetMultipleLeds
(
    uint8_t pins                // The LED pins to set
)
{
    const uint32_t ledMask = GPIO_MAP(GPIO_CONN_SYS_STATUS_LED0) |
                        GPIO_MAP(GPIO_CONN_DEBUG_LED0) |
                        GPIO_MAP(GPIO_CONN_LINK_STATUS_LED0) |
                        GPIO_MAP(GPIO_CONN_DEBUG_LED1) |
                        GPIO_MAP(GPIO_CONN_USB2_STATUS_LED0) |
                        GPIO_MAP(GPIO_CONN_USB3_STATUS_LED0) |
                        GPIO_MAP(GPIO_CONN_VIDEO_STATUS_LED0) |
                        GPIO_MAP(GPIO_CONN_DEBUG_LED2);

    // read-modify-write
    GPIO_dataWriteRMW(pins, ledMask);
}


/**
* FUNCTION NAME: GpioClear()
*
* @brief  - Clears a gpio pin
*
* @return - void
*
* @note   -
*
*/
void GpioClear
(
    enum gpioT pin // The gpio pin to clear
)
{
    // read-modify-write
    GPIO_dataWriteRMW(0, GPIO_MAP(pin));
    ilog_GPIO_COMPONENT_1(ILOG_DEBUG, GPIO_CLEAR, pin);
}


/**
* FUNCTION NAME: GpioRegisterIrqHandler()
*
* @brief  - Registering Interrupt Handler for the specified pin
*
* @return - void
*
* @note   - Asserts if pin or interrupt handler are valid.
*
*/
void GpioRegisterIrqHandler
(
    enum gpioT pin,
    enum gpioIrqT type,
    GPIO_interruptHandler handler
)
{
    // include ENUM of Rise/Fall/High/Low for interrupt type
    ilog_GPIO_COMPONENT_1(ILOG_DEBUG, REGISTERING_IRQ, pin);

    iassert_GPIO_COMPONENT_1(pin < TOTAL_NUMBER_GPIOS, INVALID_PIN, pin);
    iassert_GPIO_COMPONENT_2(handler != NULL, IRQ_HANDLER_NOT_SET, pin, __LINE__);

    gpioIrqHandler[pin].type = type;
    gpioIrqHandler[pin].handler = handler;
}


/**
* FUNCTION NAME: GpioDisableIrq()
*
* @brief  - Masks the Interrupt for the specified pin
*
* @return - void
*
* @note   - Asserts if pin is not valid.
*
*/
void GpioDisableIrq
(
    enum gpioT pin
)
{
    const struct GpioIntCfgPortT port = {.enable = false, .type = gpioIrqHandler[pin].type};

    ilog_GPIO_COMPONENT_1(ILOG_DEBUG, DISABLING_IRQ, pin);
    iassert_GPIO_COMPONENT_1(pin < TOTAL_NUMBER_GPIOS, INVALID_PIN, pin);

    GPIO_isrCfgWrite(pin, &port);
}


/**
* FUNCTION NAME: GpioEnableIrq()
*
* @brief  - Enables the interrupt for the specified pin
*
* @return - void
*
* @note   - Asserts if pin or interrupt handler are not valid.
*
*/
void GpioEnableIrq
(
    enum gpioT pin
)
{
    const struct GpioIntCfgPortT port = {.enable = true, .type = gpioIrqHandler[pin].type};

    ilog_GPIO_COMPONENT_1(ILOG_DEBUG, ENABLING_IRQ, pin);

    iassert_GPIO_COMPONENT_1(pin < TOTAL_NUMBER_GPIOS, INVALID_PIN, pin);
    iassert_GPIO_COMPONENT_2(gpioIrqHandler[pin].handler != NULL, IRQ_HANDLER_NOT_SET, pin, __LINE__);

    GPIO_isrCfgWrite(pin, &port);
}

/**
* FUNCTION NAME: isGpioIrqEnabled()
*
* @brief  - Service the interrupt routine for the specified pin.
*
* @return - true or false
*
* @note   - Asserts if the pin is not valid.
*
*/
bool isGpioIrqEnabled
(
    enum gpioT pin
)
{
    struct GpioIntCfgPortT port = {.enable = false, .type = 0};

    iassert_GPIO_COMPONENT_1(pin < TOTAL_NUMBER_GPIOS, INVALID_PIN, pin);
    iassert_GPIO_COMPONENT_2(gpioIrqHandler[pin].handler != NULL, IRQ_HANDLER_NOT_SET, pin, __LINE__);

    GPIO_isrCfgRead(pin, &port);

    return port.enable;
}

//#################################################################################################
// Determine which interrupt fired and call the registered handler, if exists
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void GPIO_irqHandler(void)
{
    uint8_t pin = 0;

    for (pin = 0; pin < TOTAL_NUMBER_GPIOS; pin++)
    {
        if (GPIO_isIrqPending(pin))
        {
            if((gpioIrqHandler[pin].type == GPIO_IRQ_HIGH_LEVEL) || (gpioIrqHandler[pin].type == GPIO_IRQ_LOW_LEVEL))
            {
                // call handler first and erase pending, not to meet the same interrupt
                gpioIrqHandler[pin].handler();
                GPIO_clearIrqPending(pin);
            }
            else
            {
                // call handler after erase pending, not to lose next interrupt
                GPIO_clearIrqPending(pin);
                gpioIrqHandler[pin].handler();
            }
        }
    }
}

