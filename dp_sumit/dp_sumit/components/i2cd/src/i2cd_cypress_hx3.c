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
// This file implements a driver for Cypress HX3 hub, which uploads a custom firmware image to a
// Cypress HX3 hub if it is found on the I2C bus.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################


//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <i2c.h>
#include <gpio.h>
#include <i2cd_cypress_hx3.h>

#include "i2cd_log.h"

#include <uart.h>

// Constants and Macros ###########################################################################
#define CYPRESS_HX3_I2C_ADDR 0x60
#define CYPRESS_HUB_HX3_RTL_MUX_PORT 2
//#define CYPRESS_HX3_I2C_SPEED (I2C_SPEED_SLOW)
#define CYPRESS_HX3_I2C_SPEED (I2C_SPEED_FAST)

#define REGULAR_TRANSFER_DATA_LEN 16

#define CYPRESS_HUB_WRITE_RETRIES (3)
#define CYPRESS_HUB_OUT_OF_RESET_WAIT_IN_MS     (10)

// maximum number of times will will try to program the hub before asserting
#define CYPRESS_HUB_MAX_PROGRAMMING_RETRIES     5


// Data Types #####################################################################################

typedef void (*CypressHx3HubInitCompletionHandler)(void);

enum CypressHubProgramState
{
    CYPRESS_HUB_PROGRAM_INIT,           // programming of the hub has not started, yet
    CYPRESS_HUB_PROGRAM_RESET,          // Hub is being reset
    CYPRESS_HUB_PROGRAM_IN_PROGRESS,    // Hub is being programmed
    CYPRESS_HUB_PROGRAM_DONE,           // Hub programming is done
};

// Static Function Declarations ###################################################################
const struct I2cDevice i2cDeviceCypress =
{
    .deviceAddress = CYPRESS_HX3_I2C_ADDR,
    .speed = CYPRESS_HX3_I2C_SPEED,
    .port = I2C_MUX_MOTHERBOARD
};

static void I2cd_upgradeCypressHx3HubFirmware(void)             __attribute__((section(".atext")));
static void CypressHubTimerHandler(void)                        __attribute__((section(".atext")));
static void I2cd_CypressWriteCompleteHandler(bool success)      __attribute__((section(".atext")));
static void CypressHubReset(bool reset)                         __attribute__((section(".atext")));

// Global Variables ###############################################################################
// From i2cd_cypress_hx3_firmware.c:
extern const uint8_t __attribute__((section(".flashrodata"))) cypressHx3FirmwareRex[];
extern const uint8_t __attribute__((section(".flashrodata"))) cypressHx3FirmwareLex[];

extern const uint32_t __attribute__((section(".flashrodata"))) cypressHx3FwLenRex;
extern const uint32_t __attribute__((section(".flashrodata"))) cypressHx3FwLenLex;

// Static Variables ###############################################################################
static struct
{
    uint16_t firmwareByteOffset;    // number of bytes programmed so far
    uint8_t writeRetryCount;        // number of times we've retried writes

    // These variables are different depending on whether it is a Lex or Rex
    uint16_t cypressHx3FwLen;       // the length of the firmware we want to transfer to the hub
    uint8_t const *cypressHx3Firmware;    // pointer to the firmware image we want to transfer

    // Number of Cypress HX3 FW bytes to be transferred in the actual I2C transaction. This value
    // does not include the two firmware offset bytes.
    uint8_t actualTransferDataLen;

    // Small writeable buffer for transferring chunks of the firmware image.
    uint8_t transferBuffer[2 +  REGULAR_TRANSFER_DATA_LEN];

    enum CypressHx3UpgradeResult upgradeStatus;     // current fw programming status

    // A timer used to wait for a period of time after bringing Cypress Hub out of reset.
    TIMING_TimerHandlerT cypressTimer;
    uint8_t cypressRetryCounter;        // keeps track of how many times we've tried to program the hub
    enum CypressHubProgramState hubProgramState;

    // Function pointer to store the callback that will be called when the upgrade is finished.
    CypressHx3HubInitCompletionHandler overallCompletionHandler;

} hx3;

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize Cypress HX3 Hub.
// Parameters:
//              i2cInterface - a function pointer for the device driver to access low level I2C
//                             operation.
//              i2cHandle    - a representation of the complete path to access the I2C slave device
//                             through the I2C switch on the selected bus.
// Return:
// Assumptions: Should only be called once, at system initialization
//#################################################################################################
void I2CD_CypressHx3HubInit( void (*completionHandler)(void) )
{
    if (bb_top_IsDeviceLex())
    {
        // programming the Lex Hub
        hx3.cypressHx3Firmware = cypressHx3FirmwareLex;
        hx3.cypressHx3FwLen    = cypressHx3FwLenLex;
    }
    else
    {
        // programming the Rex Hub
        hx3.cypressHx3Firmware = cypressHx3FirmwareRex;
        hx3.cypressHx3FwLen    = cypressHx3FwLenRex;
    }

    hx3.overallCompletionHandler = completionHandler;

    hx3.cypressTimer = TIMING_TimerRegisterHandler(
        CypressHubTimerHandler,
        true,
        CYPRESS_HUB_OUT_OF_RESET_WAIT_IN_MS);
}


//#################################################################################################
// Starts the hub programming process
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_CypressHubStartProgramming(void)
{
    // make sure we aren't already in progress
    if ( (hx3.hubProgramState == CYPRESS_HUB_PROGRAM_DONE) ||
         (hx3.hubProgramState == CYPRESS_HUB_PROGRAM_INIT) ) // CYPRESS_HUB_PROGRAM_INIT for the first time case
    {
        hx3.cypressRetryCounter = 0;
        hx3.hubProgramState = CYPRESS_HUB_PROGRAM_INIT;
        TIMING_TimerStart(hx3.cypressTimer);
    }
    else
    {
        ilog_I2CD_COMPONENT_0(ILOG_MINOR_EVENT, CYPRESS_FIRMWARE_UPGRADE_IN_PROGRESS);
    }
}


// Component Scope Function Definitions ###########################################################


// Static Function Definitions ####################################################################

//#################################################################################################
// Handles Cypress Hub timer when it expires.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void CypressHubTimerHandler(void)
{
    ilog_I2CD_COMPONENT_1(ILOG_MINOR_EVENT, CYPRESS_HUB_TIMER_STATE, hx3.hubProgramState);

    switch(hx3.hubProgramState)
    {
        case CYPRESS_HUB_PROGRAM_INIT:   // programming of the hub has not started, yet
        default:
            // Rex Cypress hub init
            hx3.hubProgramState = CYPRESS_HUB_PROGRAM_RESET;
            CypressHubReset(true);  // make sure the hub is in reset
            break;

        case CYPRESS_HUB_PROGRAM_RESET:  // Hub is in reset
            hx3.hubProgramState = CYPRESS_HUB_PROGRAM_IN_PROGRESS;  // go on to the programming state
            CypressHubReset(false);  // take the hub out of reset
            break;

        case CYPRESS_HUB_PROGRAM_IN_PROGRESS:     // Hub is being programmed
            TIMING_TimerStop(hx3.cypressTimer);

            // ok, we've waited long enough for the hub to boot up - start downloading the hub image
            ilog_I2CD_COMPONENT_1(ILOG_MINOR_EVENT, CYPRESS_UPGRADING_FIRMWARE, hx3.cypressHx3FwLen);
            hx3.writeRetryCount = 0;    // make sure our write retry counter is 0, initially
            hx3.firmwareByteOffset = 0; // make sure we are at the beginning (if we failed the first time)
            I2cd_upgradeCypressHx3HubFirmware();
            break;

        case CYPRESS_HUB_PROGRAM_DONE:     // Hub is programmed, and running!
            break;
    }
}

//#################################################################################################
// Upgrade Cypress HX3 Hub firmware by filling the transfer buffer and kicking off I2C write
// asynchronous operation.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void I2cd_upgradeCypressHx3HubFirmware(void)
{
    // Offset is in little endian format as informed by Cypress in the email
    // TODO: In cypress I2C log file, they seem using big endian. We tried both address formats
    // and both work. Why???
    hx3.transferBuffer[0] = (hx3.firmwareByteOffset & 0xFF);
    hx3.transferBuffer[1] = (hx3.firmwareByteOffset >> 8) & 0xFF;

    hx3.actualTransferDataLen =
        MIN(REGULAR_TRANSFER_DATA_LEN, hx3.cypressHx3FwLen - hx3.firmwareByteOffset);

    if (hx3.actualTransferDataLen != 0)
    {
        memcpy(
            &hx3.transferBuffer[2],
            &hx3.cypressHx3Firmware[hx3.firmwareByteOffset],
            hx3.actualTransferDataLen);

        I2C_WriteAsync(
            &i2cDeviceCypress,
            hx3.transferBuffer,
            hx3.actualTransferDataLen + 2,
            I2cd_CypressWriteCompleteHandler
        );
    }
    else
    {
        // programming successfully done!
        ilog_I2CD_COMPONENT_0( ILOG_MINOR_EVENT, CYPRESS_HUB_PROGRAMMING_SUCCESS);
        hx3.hubProgramState = CYPRESS_HUB_PROGRAM_DONE;
        hx3.overallCompletionHandler();
    }
}

//#################################################################################################
// Completion handler for each I2C write cycle.
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void I2cd_CypressWriteCompleteHandler(bool success)
{
    if (success)
    {
        hx3.writeRetryCount = 0;
        hx3.firmwareByteOffset += hx3.actualTransferDataLen;
        I2cd_upgradeCypressHx3HubFirmware();    // go program the next block
    }
    else if (hx3.writeRetryCount < CYPRESS_HUB_WRITE_RETRIES)
    {
        ilog_I2CD_COMPONENT_2(
            ILOG_MINOR_ERROR,
            CYPRESS_HX3_RETRY,
            hx3.firmwareByteOffset,
            hx3.writeRetryCount);

        hx3.writeRetryCount++;
        I2cd_upgradeCypressHx3HubFirmware();    // retry this block
    }
    else if ((hx3.firmwareByteOffset == 0) && (hx3.cypressRetryCounter == 0))
    {
        // if we failed on the 1st transfer, assume the hub is not there
        ilog_I2CD_COMPONENT_0( ILOG_MINOR_ERROR, CYPRESS_HUB_NOT_FOUND);
        hx3.hubProgramState = CYPRESS_HUB_PROGRAM_DONE;
        hx3.overallCompletionHandler();
    }
    else
    {
        // this programming attempt failed - try again (if we haven't tried too many times already!)
        iassert_I2CD_COMPONENT_0(
            (hx3.cypressRetryCounter < CYPRESS_HUB_MAX_PROGRAMMING_RETRIES),
            CYPRESS_HUB_FAILED_PROGRAMMING);

        // retry the cypress hub programming, from the start
        TIMING_TimerStart(hx3.cypressTimer);
        hx3.hubProgramState = CYPRESS_HUB_PROGRAM_INIT;
        hx3.cypressRetryCounter++;
    }
}



//#################################################################################################
// Puts the hub into reset or not
//
// Parameters: active - false to take the hub out of reset, true to put it into reset
// Return:
// Assumptions:
//
//#################################################################################################
static void CypressHubReset(bool reset)
{
    // TODO: put this in bb_top?
    if (reset)
    {
        // put the hub into reset
        GpioClear(GPIO_CONN_HUB_RST_B_A);
    }
    else
    {
        // take the hub out of reset - will need to be programmed with the firmware image
        // after a hub boot up delay of at least 5ms
        GpioSet(GPIO_CONN_HUB_RST_B_A);
    }
}


