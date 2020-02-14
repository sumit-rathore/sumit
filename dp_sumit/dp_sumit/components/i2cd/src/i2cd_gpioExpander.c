//#################################################################################################
// Icron Technology Corporation - Copyright 2015
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
// Contains the driver for accessing the TI TCA6424A 24 port I2C GPIO Expander.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// For the sake of code simplicity, this driver reads and writes all 3 bytes of the relevant port
// group at a time.  For example if a client wanted to read GPIO 9 which was previously configured
// as an input, then it is only necessary to issue a 1-byte write-read for INPUT_PORT_1.  Instead,
// this driver issues a 3 byte write-read which means that 2 extra bytes of data are read out of
// the device via the I2C bus.
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <itypes.h>
#include <i2cd_gpioExpander.h>   // Exposed header for gpio expander function definitions
#include <i2c.h>
#include <timing_timers.h>
#include "i2cd_log.h"
#include "i2cd_cmd.h"

// Constants and Macros ###########################################################################
// Any of the commands can have have the auto-increment bit set which affects the behaviour of
// multi-byte reads and writes
#define AUTO_INCREMENT_BIT_POS      7
#define I2CD_GPIO_EXPANDER_ADDR     0x22

// Data Types #####################################################################################
enum Commands
{
    GPIO_EXPANDER_CMD_INPUT_PORT_0,
    GPIO_EXPANDER_CMD_INPUT_PORT_1,
    GPIO_EXPANDER_CMD_INPUT_PORT_2,
    GPIO_EXPANDER_CMD_OUTPUT_PORT_0 = 4,
    GPIO_EXPANDER_CMD_OUTPUT_PORT_1,
    GPIO_EXPANDER_CMD_OUTPUT_PORT_2,
    GPIO_EXPANDER_CMD_POLARITY_INVERSION_PORT_0 = 8,
    GPIO_EXPANDER_CMD_POLARITY_INVERSION_PORT_1,
    GPIO_EXPANDER_CMD_POLARITY_INVERSION_PORT_2,
    GPIO_EXPANDER_CMD_CONFIGURATION_PORT_0 = 12,
    GPIO_EXPANDER_CMD_CONFIGURATION_PORT_1,
    GPIO_EXPANDER_CMD_CONFIGURATION_PORT_2,
};


// Global Variables ###############################################################################

// Static Variables ###############################################################################
const struct I2cDevice i2cDeviceGpioExpander =
{
    .deviceAddress = I2CD_GPIO_EXPANDER_ADDR,
    .speed = I2C_SPEED_SLOW,
    .port = KC705_MUX_FMC_LPC_IIC
};

static const struct gpioI2cInitStates gpioI2cStates[] =
{
    {GPIO_SW1_4        , GPIO_I2C_INPUT_NORMAL},
    {GPIO_SW1_3        , GPIO_I2C_INPUT_NORMAL},
    {GPIO_SW1_2        , GPIO_I2C_INPUT_NORMAL},
    {GPIO_SW1_1        , GPIO_I2C_INPUT_NORMAL},
    {GPIO_D20_LED      , GPIO_I2C_OUTPUT_CLEAR},
    {GPIO_D19_LED      , GPIO_I2C_OUTPUT_CLEAR},
    {GPIO_D18_LED      , GPIO_I2C_OUTPUT_CLEAR},
    {GPIO_D17_LED      , GPIO_I2C_OUTPUT_CLEAR},
    {GPIO_REXN_LEX     , GPIO_I2C_OUTPUT_CLEAR},
    {GPIO_OUT_ENABLE   , GPIO_I2C_INPUT_NORMAL},
    {GPIO_TX_DEEMPH1   , GPIO_I2C_INPUT_NORMAL},
    {GPIO_TX_DEEMPH0   , GPIO_I2C_INPUT_NORMAL},
    {GPIO_TX_MARGIN2   , GPIO_I2C_INPUT_NORMAL},
    {GPIO_TX_MARGIN1   , GPIO_I2C_INPUT_NORMAL},
    {GPIO_TX_MARGIN0   , GPIO_I2C_INPUT_NORMAL},
    {GPIO_TX_SWING     , GPIO_I2C_INPUT_NORMAL},
    {GPIO_GPIO1_RST    , GPIO_I2C_OUTPUT_SET},
    {GPIO_GPIO2_RST    , GPIO_I2C_OUTPUT_CLEAR},
    {GPIO_GPIO3_RST    , GPIO_I2C_OUTPUT_SET},
    {GPIO_VBUS_OC_N    , GPIO_I2C_INPUT_NORMAL},
    {GPIO_SPARE_24     , GPIO_I2C_INPUT_NORMAL},
    {GPIO_ELAS_BUF_MODE, GPIO_I2C_OUTPUT_CLEAR},
    {GPIO_D11_LED      , GPIO_I2C_OUTPUT_CLEAR},
    {GPIO_D12_LED      , GPIO_I2C_OUTPUT_CLEAR}
};

static struct
{
    // Generic buffer to be used to read to and write from hardware.  The size is 4 because the
    // largest amount of data to send is a 1-byte command + 3 bytes of data to cover ports 0, 1 and
    // 2.  The largest read amount will be 3.
    uint8_t transferBuffer[4];
    union
    {
        struct
        {
            void (*doneHandler)(bool success, uint32_t inputValues);
        } input;
        struct
        {
            enum Commands startingCommand;
            uint32_t value;
            uint32_t mask;
            void (*doneHandler)(bool success);
        } outputPolarityOrDirection;
    } operationSpecific;
    union
    {
        // The high level interface is not used until after initialization is complete, so we
        // overlap the data of init and high level interface.
        union
        {
            struct
            {
                void (*callback)(void);
            } clear;
            struct
            {
                void (*callback)(void);
            } set;
            struct
            {
                void (*callback)(bool);
                enum gpioI2cT pin;
            } read;
        } highLevelInterface;
        struct
        {
            const struct gpioI2cInitStates* gpioInitStates;
            uint8_t numOfGpioInitStates;
            void (*completionHandler)(void);
        } init;
    };

} _gpioExpander;

static const uint8_t numOfI2cGpioStates = ARRAYSIZE(gpioI2cStates);
static uint8_t i2cGpioPin = 4;
static bool i2cGpioPinSet = false;
static TIMING_TimerHandlerT I2cLEDTimer;

// Static Function Declarations ###################################################################
static void _I2CD_gpioExpClearCallback(bool success);
static void _I2CD_gpioExpSetCallback(bool success);
static void _I2CD_gpioExpReadCallback(bool success, uint32_t inputValues);
static void _I2CD_gpioExpReadInputsHandler(uint8_t* data, uint8_t byteCount);
static void _I2CD_gpioExpBeginReadModifyWriteOperation(
    enum Commands command, uint32_t value, uint32_t mask, void (*doneHandler)(bool success));
static void _I2CD_gpioExpRmwReadHandler(uint8_t* data, uint8_t byteCount);
static void _I2CD_gpioExpBeginReadModifyWriteOperationBlocking(
    enum Commands command, uint32_t value, uint32_t mask);
static void setI2cGpioPin(void);

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize the GPIO expander and setup the GPIOs according to the initial
// state specification passed in
//
// Parameters:
//      handle              - i2c handle for device
//      interface           - access to i2c functions
//      addrPinHigh         - Is the address selection pin on the GPIO expander chip pulled high
//      gpioInitStates      - initialization states of GPIOs
//      numOfGpioInitStates - number of GPIOs in gpioInitStates
//      initCompleteHandler - callback when configuration completes
// Return:
//              none
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void I2CD_gpioExpInit(void)
{
    _gpioExpander.init.numOfGpioInitStates = numOfI2cGpioStates;
    _gpioExpander.init.gpioInitStates = gpioI2cStates;

    // Set outputs
    {
        uint32_t outputLevels = 0;
        for (uint8_t i = 0; i < numOfI2cGpioStates; i++)
        {
            if (gpioI2cStates[i].type == GPIO_I2C_OUTPUT_SET)
            {
                outputLevels |= (1 << gpioI2cStates[i].gpio);
            }
        }
        _I2CD_gpioExpBeginReadModifyWriteOperationBlocking(
            GPIO_EXPANDER_CMD_OUTPUT_PORT_0, outputLevels, ~0);
    }

    // Set Input Polarities
    {
        uint32_t inputPolarities = 0; // Do not invert any inputs by default
        for (uint8_t i = 0; i < _gpioExpander.init.numOfGpioInitStates; i++)
        {
            if (_gpioExpander.init.gpioInitStates[i].type == GPIO_I2C_INPUT_INVERTED)
            {
                inputPolarities |= (1 << _gpioExpander.init.gpioInitStates[i].gpio);
            }
        }
        _I2CD_gpioExpBeginReadModifyWriteOperationBlocking(
            GPIO_EXPANDER_CMD_POLARITY_INVERSION_PORT_0, inputPolarities, ~0);
    }

    // Set Direction
    {
        uint32_t directions = ~0; // All unspecified GPIOs will be inputs
        for (uint8_t i = 0; i < _gpioExpander.init.numOfGpioInitStates; i++)
        {
            if (    _gpioExpander.init.gpioInitStates[i].type == GPIO_I2C_OUTPUT_CLEAR
                ||  _gpioExpander.init.gpioInitStates[i].type == GPIO_I2C_OUTPUT_SET)
            {
                directions &= (~(1 << _gpioExpander.init.gpioInitStates[i].gpio));
            }
        }
        _I2CD_gpioExpBeginReadModifyWriteOperationBlocking(
            GPIO_EXPANDER_CMD_CONFIGURATION_PORT_0, directions, ~0);
    }
}


//#################################################################################################
// Clears the given output pin and calls the callback when complete.
//
// Parameters:
//      pin                 - gpio pin to clear
//      clearCompleteHandler - callback when operation completes
// Return:
// Assumptions:
//#################################################################################################
void I2CD_gpioExpClearPin(enum gpioI2cT pin, void (*clearCompleteHandler)(void))
{
    const uint32_t value = 0;
    const uint32_t mask = (1 << pin);
    _gpioExpander.highLevelInterface.clear.callback = clearCompleteHandler;
    I2CD_gpioExpSetOutputLevels(value, mask, &_I2CD_gpioExpClearCallback);
}


//#################################################################################################
// Sets the given output pin and calls the callback when complete.
//
// Parameters:
//      pin                 - gpio pin to set
//      setCompleteHandler  - callback when operation completes
// Return:
// Assumptions:
//#################################################################################################
void I2CD_gpioExpSetPin(enum gpioI2cT pin, void (*setCompleteHandler)(void))
{
    const uint32_t value = (1 << pin);
    const uint32_t mask = (1 << pin);
    _gpioExpander.highLevelInterface.set.callback = setCompleteHandler;
    I2CD_gpioExpSetOutputLevels(value, mask, &_I2CD_gpioExpSetCallback);
}


//#################################################################################################
// Reads the given GPIO and calls the given callback function with the read result.  If true is
// passed to the callback, then the GPIO is high.
//
// Parameters:
//      pin                 - gpio pin to read
//      readCompleteHandler - callback when operation completes
// Return:
// Assumptions:
//#################################################################################################
void I2CD_gpioExpReadPin(enum gpioI2cT pin, void (*readCompleteHandler)(bool))
{
    iassert_I2CD_COMPONENT_0(readCompleteHandler != NULL, READ_WITH_NULL_CALLBACK);
    _gpioExpander.highLevelInterface.read.pin = pin;
    _gpioExpander.highLevelInterface.read.callback = readCompleteHandler;
    I2CD_gpioExpReadInputs(&_I2CD_gpioExpReadCallback);
}


//#################################################################################################
// Reads the value of all GPIOs and calls a callback function with the result.  Each bit in the
// value passed to the callback represents a GPIO.  Bit N == GPIO N.  If the bit is 1, then the
// GPIO is high.
//
// Parameters:
//      readDoneHandler     - Callback when operation completes, with success and inputValues as
//                            return parameters.
// Return:
// Assumptions:
//#################################################################################################
void I2CD_gpioExpReadInputs(void (*readDoneHandler)(bool success, uint32_t inputValues))
{
    _gpioExpander.transferBuffer[0] =
        (GPIO_EXPANDER_CMD_INPUT_PORT_0| (1 << AUTO_INCREMENT_BIT_POS));
    const uint8_t writeByteCount = 1;
    const uint8_t readByteCount = 3; // Read Input0,1,2 registers
    _gpioExpander.operationSpecific.input.doneHandler = readDoneHandler;

    I2C_WriteReadAsync(
        &i2cDeviceGpioExpander,
        _gpioExpander.transferBuffer,
        writeByteCount,
        readByteCount,
        _I2CD_gpioExpReadInputsHandler);
}


//#################################################################################################
// Configures whether the output setting of the GPIOs specified by the outputMask is high or low.
// A bit value of 1 means high and 0 means low.  Bit N == GPIO N.  The outputDoneHandler callback
// will be called when the operation has completed.
//
// Parameters:
//      outputValue         - value to be written to pins
//      outputMask          - selects which pins from the outputValue should be written to
//      outputDoneHandler   - function which will be called on completion
// Return:
// Assumptions:
//#################################################################################################
void I2CD_gpioExpSetOutputLevels(
    uint32_t outputValue, uint32_t outputMask, void (*outputDoneHandler)(bool success))
{
    _I2CD_gpioExpBeginReadModifyWriteOperation(
        GPIO_EXPANDER_CMD_OUTPUT_PORT_0, outputValue, outputMask, outputDoneHandler);
}


//#################################################################################################
// Sets the input polarities for the GPIOs matching the polarityMask.  If a GPIO is set with
// a polarity of 1, the value read from the GPIO will be inverted.  Bit N == GPIO N.
//
// Parameters:
//      polarityValue       - polarity (0/1) indicating which pins inverted
//      polarityMask        - selects appropriate pins to write to
//      polarityDoneHandler - function which will be called on completion
// Return:
// Assumptions:
//#################################################################################################
void I2CD_gpioExpSetInputPolarities(
    uint32_t polarityValue, uint32_t polarityMask, void (*polarityDoneHandler)(bool success))
{
    _I2CD_gpioExpBeginReadModifyWriteOperation(
        GPIO_EXPANDER_CMD_POLARITY_INVERSION_PORT_0,
        polarityValue,
        polarityMask,
        polarityDoneHandler);
}


//#################################################################################################
// Sets the direction of the GPIOs matched by the mask to either input (1) or output (0).  Bit N ==
// GPIO N.
//
// Parameters:
//      directionValue      - set pins as either in or out
//      directionMask       - mask to select which pins are altered
//      directionDoneHandler - function which will be called on completion
// Return:
// Assumptions:
//      * GPIOs which are being changed to outputs have already had their level set to the desired
//        output setting.
//#################################################################################################
void I2CD_gpioExpSetIODirections(
    uint32_t directionValue, uint32_t directionMask, void (*directionDoneHandler)(bool success))
{
    _I2CD_gpioExpBeginReadModifyWriteOperation(
        GPIO_EXPANDER_CMD_CONFIGURATION_PORT_0,
        directionValue,
        directionMask,
        directionDoneHandler);
}

//#################################################################################################
// Clears the given output pin, blocking until complete.
//
// Parameters:
//      pin                 - gpio pin to clear
//      clearCompleteHandler - callback when operation completes
// Return:
// Assumptions:
//#################################################################################################
void I2CD_gpioExpClearPinBlocking(enum gpioI2cT pin)
{
    const uint32_t value = 0;
    const uint32_t mask = (1 << pin);
    _I2CD_gpioExpBeginReadModifyWriteOperationBlocking(
        GPIO_EXPANDER_CMD_OUTPUT_PORT_0, value, mask);
}


//#################################################################################################
// Sets the given output pin, blocking until complete
//
// Parameters:
//      pin                 - gpio pin to set
//      setCompleteHandler  - callback when operation completes
// Return:
// Assumptions:
//#################################################################################################
void I2CD_gpioExpSetPinBlocking(enum gpioI2cT pin)
{
    const uint32_t value = (1 << pin);
    const uint32_t mask = (1 << pin);
    _I2CD_gpioExpBeginReadModifyWriteOperationBlocking(
        GPIO_EXPANDER_CMD_OUTPUT_PORT_0, value, mask);
}

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// I2CD_gpioExpTest
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_gpioExpTest(uint32_t patternRepeatPeriod)
{
    // If the timer is non-null, then just reset the pattern repeat period to the new value
    if (I2cLEDTimer != NULL)
    {
        TIMING_TimerResetTimeout(I2cLEDTimer, patternRepeatPeriod);
        TIMING_TimerStart(I2cLEDTimer);
        return;
    }

    // initialize I2C GPIO controller

    // Set low output on GPIOs 7:4
    I2CD_gpioExpSetOutputLevels(0x00, 0xF0, NULL);
    // Set to output on GPIOs 7:4
    I2CD_gpioExpSetIODirections(0x00, 0xF0, NULL);

    // Initialize I2cGPIO pulsing timer
    const bool isPeriodic = true;
    I2cLEDTimer = TIMING_TimerRegisterHandler(&setI2cGpioPin, isPeriodic, patternRepeatPeriod);

    TIMING_TimerStart(I2cLEDTimer);
}

// Static Function Definitions ####################################################################

//#################################################################################################
// Callback that asserts success of the I2C write which initiated the GPIO clear and then calls the
// callback supplied by the caller of I2CD_gpioExpclearPin.
//
// Parameters:
//      success             - was the clear operation successful
// Return:
//              none
// Assumptions:
//#################################################################################################
static void _I2CD_gpioExpClearCallback(bool success)
{
    iassert_I2CD_COMPONENT_0(success, I2C_WRITE_FAILED);
    if (_gpioExpander.highLevelInterface.clear.callback != NULL)
    {
        (*(_gpioExpander.highLevelInterface.clear.callback))();
    }
}


//#################################################################################################
// Callback that asserts success of the I2C write which initiated the GPIO set and then calls the
// callback supplied by the caller of I2CD_gpioExpSetPin.
//
// Parameters:
//      success             - was the set operation successful
// Return:
// Assumptions:
//#################################################################################################
static void _I2CD_gpioExpSetCallback(bool success)
{
    iassert_I2CD_COMPONENT_0(success, I2C_WRITE_FAILED);
    if (_gpioExpander.highLevelInterface.set.callback != NULL)
    {
        (*(_gpioExpander.highLevelInterface.set.callback))();
    }
}


//#################################################################################################
// Callback that asserts success of the I2C write which initiated the GPIO read and then calls the
// callback supplied by the caller of I2CD_gpioExpReadPin.
//
// Parameters:
//      success             - status of init operation
//      inputValues         - values read from pins
// Return:
// Assumptions:
//#################################################################################################
static void _I2CD_gpioExpReadCallback(bool success, uint32_t inputValues)
{
    iassert_I2CD_COMPONENT_0(success, I2C_WRITE_FAILED);
    (*(_gpioExpander.highLevelInterface.read.callback))(
        ((inputValues >> _gpioExpander.highLevelInterface.read.pin) & 0x1) == 1);
}


//#################################################################################################
// Callback from I2CD_gpioExpReadInputs which assembles the data in the read buffer into a single
// uint32_t and checks for success of the read.  This is done to prepare to call the callback
// passed to I2CD_gpioExpReadInputs.
//
// Parameters:
//      data                - pointer to returned data buffer
//      byteCount           - number of bytes read
// Return:
// Assumptions:
//#################################################################################################
static void _I2CD_gpioExpReadInputsHandler(uint8_t* data, uint8_t byteCount)
{
    const bool success = byteCount == 3;
    const uint32_t inputValue = (data[0] | (data[1] << 8) | (data[2] << 16));
    (*(_gpioExpander.operationSpecific.input.doneHandler))(success, inputValue);
}


//#################################################################################################
// Initiates a read-modify-write of 3 adjacent registers specified by the command parameter.  Both
// the mask and value use the least significant 24 bits of the variable to control the 24 pins of
// the switch.
//
// Parameters:
//      command             - GPIO expander command
//      value               - value to be written to the pins
//      mask                - value to select which pins to write to
//      doneHandler         - callback for completed operation
// Return:
// Assumptions:
//#################################################################################################
static void _I2CD_gpioExpBeginReadModifyWriteOperation(
    enum Commands command, uint32_t value, uint32_t mask, void (*doneHandler)(bool success))
{
    _gpioExpander.operationSpecific.outputPolarityOrDirection.startingCommand =
        (command | (1 << AUTO_INCREMENT_BIT_POS));
    _gpioExpander.operationSpecific.outputPolarityOrDirection.value = value;
    _gpioExpander.operationSpecific.outputPolarityOrDirection.mask = mask;
    _gpioExpander.operationSpecific.outputPolarityOrDirection.doneHandler = doneHandler;

    if ((mask & 0x00FFFFFF) == 0x00FFFFFF)
    {
        // If the mask covers all 24 GPIOs, then we don't need to read the existing value as we can
        // just overwrite the value currently in the hardware.  Rather than duplicating the I2C
        // write here, we just fake a callback as if a read had just completed.  The contents of
        // the transferBuffer don't matter as _I2CD_gpioExpreadOutputHandler will overwrite it
        // anyway.
        _I2CD_gpioExpRmwReadHandler(_gpioExpander.transferBuffer, 3);
    }
    else
    {
        _gpioExpander.transferBuffer[0] =
            _gpioExpander.operationSpecific.outputPolarityOrDirection.startingCommand;
        const uint8_t writeByteCount = 1;
        const uint8_t readByteCount = 3; // Read Output0,1,2 registers

        I2C_WriteReadAsync(
            &i2cDeviceGpioExpander,
            _gpioExpander.transferBuffer,
            writeByteCount,
            readByteCount,
            _I2CD_gpioExpRmwReadHandler);
    }
}


//#################################################################################################
// This callback function is called after the read portion of the read-modify-write operation.  It
// deals with the masking and writing of the new value to the registers.
//
// Parameters:
//      data                - buffer of read data
//      byteCount           - bytes of data read
// Return:
// Assumptions:
//#################################################################################################
static void _I2CD_gpioExpRmwReadHandler(uint8_t* data, uint8_t byteCount)
{
    const bool success = byteCount == 3;
    if (!success)
    {
        // If we failed to read the output registers, then we cannot perform the mask and write
        // them, so just callback with failure now.
        (*(_gpioExpander.operationSpecific.outputPolarityOrDirection.doneHandler))(success);
    }
    else
    {
        // The data pointer being passed to us is in fact this modules own transferBuffer, so we
        // need to be very careful about how we overwrite the transfer buffer for use in the
        // upcoming I2C write operation.
        const uint32_t invMask = ~(_gpioExpander.operationSpecific.outputPolarityOrDirection.mask);
        const uint32_t value = _gpioExpander.operationSpecific.outputPolarityOrDirection.value;
        _gpioExpander.transferBuffer[3] = (data[2] & (invMask >> 16)) | (value >> 16);
        _gpioExpander.transferBuffer[2] = (data[1] & (invMask >>  8)) | (value >>  8);
        _gpioExpander.transferBuffer[1] = (data[0] & (invMask >>  0)) | (value >>  0);
        _gpioExpander.transferBuffer[0] =
            _gpioExpander.operationSpecific.outputPolarityOrDirection.startingCommand;
        const uint8_t writeByteCount = 4;

        I2C_WriteAsync(
            &i2cDeviceGpioExpander,
            _gpioExpander.transferBuffer,
            writeByteCount,
            _gpioExpander.operationSpecific.outputPolarityOrDirection.doneHandler);
    }
}


//#################################################################################################
// Initiates a blocking version of a read-modify-write of 3 adjacent registers specified by the
// command parameter.  Both the mask and value use the least significant 24 bits of the variable
// to control the 24 pins of the switch.
//
// Parameters:
//      command             - GPIO expander command
//      value               - value to be written to the pins
//      mask                - value to select which pins to write to
// Return:
// Assumptions:
//#################################################################################################
static void _I2CD_gpioExpBeginReadModifyWriteOperationBlocking(
    enum Commands command, uint32_t value, uint32_t mask)
{
    bool res = true;
    uint8_t transferBuffer[4];

    // not all bi
    if ((mask & 0x00FFFFFF) != 0x00FFFFFF)
    {
        const uint8_t writeByteCount = 1;
        const uint8_t readByteCount = 3; // Read Output0,1,2 registers
        transferBuffer[0] = (command | (1 << AUTO_INCREMENT_BIT_POS));
        res = I2C_WriteReadBlocking(
            &i2cDeviceGpioExpander,
            transferBuffer,
            writeByteCount,
            readByteCount);
    }
    if ((res) || ((mask & 0x00FFFFFF) == 0x00FFFFFF))
    {
        // If the mask covers all 24 GPIOs, then we don't need to read the existing value as we can
        // just overwrite the value currently in the hardware.  Rather than duplicating the I2C
        // write here, we just fake a callback as if a read had just completed.  The contents of
        // the transferBuffer don't matter as _I2CD_gpioExpreadOutputHandler will overwrite it
        // anyway.
        const uint32_t invMask = ~mask;
        transferBuffer[3] = (transferBuffer[2] & (invMask >> 16)) | (value >> 16);
        transferBuffer[2] = (transferBuffer[1] & (invMask >>  8)) | (value >>  8);
        transferBuffer[1] = (transferBuffer[0] & (invMask >>  0)) | (value >>  0);
        transferBuffer[0] = (command | (1 << AUTO_INCREMENT_BIT_POS));
        const uint8_t writeByteCount = 4;
        res = I2C_WriteBlocking(
            &i2cDeviceGpioExpander,
            transferBuffer,
            writeByteCount);
    }
}


//#################################################################################################
// setI2cGpioPin
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void setI2cGpioPin(void)
{
    if (i2cGpioPinSet)
    {
        I2CD_gpioExpSetPin(i2cGpioPin, NULL);
    }
    else
    {
        I2CD_gpioExpClearPin(i2cGpioPin, NULL);
    }
    i2cGpioPin++;
    if (i2cGpioPin == 8)
    {
        i2cGpioPin = 4;
        i2cGpioPinSet = !i2cGpioPinSet;
    }
}