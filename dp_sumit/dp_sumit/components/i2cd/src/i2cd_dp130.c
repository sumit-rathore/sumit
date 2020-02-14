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
// This module provides the I2C interface to the TI DP130 Redriver
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// Each function will provide a functional interface to the registers
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <timing_profile.h>
#include <timing_timers.h>
#include <i2c.h>
#include <i2cd_dp130.h>
#include <i2cd_dp130api.h>

#include "i2cd_log.h"
#include "i2cd_cmd.h"

// Constants and Macros ###########################################################################

#define I2CD_DP130_DEVICE_ADDRESS           (0x2E)
// The DP130 chip only supports up to 100 kHz
#define I2CD_DP130_DEVICE_SPEED             I2C_SPEED_SLOW
// Register Addresses, first byte of data sent to DP130
#define I2CD_DP130_AUTO_PWR_DN_FRC_DN       (0x01)
#define I2CD_DP130_SQUELCH                  (0x03)
#define I2CD_DP130_EN_LNK_TRN_AUX_SRC       (0x04)

#define I2CD_DP130_EQ_I2C_ENABLE            (0x05)
#define I2CD_DP130_AEQ_LN0_L0L1_SET         (0x05)
#define I2CD_DP130_AEQ_LN0_L2L3_SET         (0x06)
#define I2CD_DP130_AEQ_LN1_L0L1_SET         (0x07)
#define I2CD_DP130_AEQ_LN1_L2L3_SET         (0x08)
#define I2CD_DP130_AEQ_LN2_L0L1_SET         (0x09)
#define I2CD_DP130_AEQ_LN2_L2L3_SET         (0x0A)
#define I2CD_DP130_AEQ_LN3_L0L1_SET         (0x0B)
#define I2CD_DP130_AEQ_LN3_L2L3_SET         (0x0C)

#define I2CD_DP130_BOOST_DP_TMDS_VOD_VPRE   (0x15)
#define I2CD_DP130_HPD_CAD_TEST_MODE        (0x17)

#define I2CD_DP130_I2C_DPCD_RST             (0x1B)

#define I2CD_DP130_DPCD_ADDR_HIGH           (0x1C) // 3:0 High, 0x1D 15:8, 0x1E 7:0
#define I2CD_DP130_DPCD_ADDR_MID            (0x1D)
#define I2CD_DP130_DPCD_ADDR_LOW            (0x1E)
#define I2CD_DP130_DPCD_DATA                (0x1F)

#define I2CD_DP130_DEV_ID_REV               (0x20)

#define I2CD_DP130_TRANS_ELEM_MAX_CNT       (64)

#define SIZEOF_RAW(type) sizeof(((type *)0)->raw)

#define EQ_I2C_ENABLE_MASK                  (1 << 7)
#define I2CD_DP130_I2C_RETRY_LIMIT          (5)

#define DP130_WAIT_TIME                     (400)     // DP130 400ms wait after out of reset

// Data Types #####################################################################################
enum Dp130TransactionState
{
    DP130_TRANSACTION_STATE_INACTIVE,
    DP130_TRANSACTION_STATE_READ,
    DP130_TRANSACTION_STATE_WRITE
};

// Global Variables ###############################################################################

// Static Variables ###############################################################################
const struct I2cDevice i2cDeviceDp130 =
{
    .deviceAddress = I2CD_DP130_DEVICE_ADDRESS,
    .speed = I2CD_DP130_DEVICE_SPEED,
    .port = I2C_MUX_MOTHERBOARD
};

static struct
{
    enum Dp130TransactionState state;

    TIMING_TimerHandlerT dp130Timer;        // DP130 operation wait timer to check 400ms after out of reset
    void (*initCallback)(bool);             // DP130 Initialize completion callback
    bool initSuccess;                       // DP130 initialize success
    union
    {
        struct
        {
            void (*writeCompletionHandler)(bool);
        } write;
        struct
        {
            void (*readCompletionHandler)(uint8_t* data, uint8_t byteCount);
        } read;
    };
    uint8_t dataBuffer[9];
} dp130Ctx __attribute__((section(".rexbss")));

// Static Function Declarations ###################################################################
#ifdef DP130_CUSTOM_FUNCTIONS_NEEDED
static void performRead(const uint8_t byteCount) __attribute__((section(".rexftext")));
static void _I2CD_dp130ReadCompleteHandler(uint8_t* data, uint8_t byteCount) __attribute__((section(".rexftext")));
#endif  // DP130_CUSTOM_FUNCTIONS_NEEDED

static void _I2CD_dp130WriteCompleteHandler(bool success)                   __attribute__((section(".rexftext")));
static void performWrite(const uint8_t byteCount)                           __attribute__((section(".rexftext")));
static void _I2CD_dp130ResetTransactionData(void)                           __attribute__((section(".rexftext")));
static void I2CD_dp130GeneralReadHandler(uint8_t* data, uint8_t byteCount)  __attribute__((section(".rexatext")));
static void DP130ResetWaitTimerHandler(void)                                __attribute__((section(".rexatext")));
static void I2CD_dp130SetEqEnable(void)                                     __attribute__((section(".rexatext")));
static void I2CD_dp130SetEqEnableHandler(bool)                              __attribute__((section(".rexatext")));

// Exported Function Definitions ##################################################################

//#################################################################################################
// Setup local struct with I2C handle and interface pointer for I2C access
//
// Parameters:
// Return:
// Assumptions:
//      * This function will work with a localized static variable to track operation state.
//      * Rather than have a series of functions as callbacks for all the
//      * writes, use this function update the state variable, permitting state
//      * machine state change operation
//#################################################################################################
void I2CD_dp130Init(void)
{
    _I2CD_dp130ResetTransactionData();
    // According to DP130 data sheet (sn75dp130, ch11) device will not be available
    // for operation until after 400ms after a valid reset
    // As per the reference design, no I2C transactions with the DP130 will be performed
    // (at this time)

    dp130Ctx.dp130Timer = TIMING_TimerRegisterHandler(
                                DP130ResetWaitTimerHandler, false, DP130_WAIT_TIME);

    I2CD_dp130Enable(NULL);
}


//#################################################################################################
// Disable dp130 (clear port and stop init timer)
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_dp130Disable(void)
{
    bb_top_ApplyEnableDp130(false);
    bb_top_ApplyResetDp130(true);
    TIMING_TimerStop(dp130Ctx.dp130Timer);
}


//#################################################################################################
// Enable dp130 (set port and start init timer)
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_dp130Enable(void (*callback)(bool))
{
    dp130Ctx.initCallback = callback;

    bb_top_ApplyResetDp130(false);
    bb_top_ApplyEnableDp130(true);
    TIMING_TimerStart(dp130Ctx.dp130Timer);
}

//#################################################################################################
// Return init success information
//
// Parameters:
// Return: true(init success), false(init fail or dp130 doesn't exist)
// Assumptions:
//      * This function assumes chip has been reset or powered up
//#################################################################################################
bool I2CD_dp130InitSuccess(void)
{
    return dp130Ctx.initSuccess;
}


//#################################################################################################
// Set dp130 eq enable
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_dp130GeneralRead(uint8_t readAddr)
{
    dp130Ctx.state = DP130_TRANSACTION_STATE_READ;
    dp130Ctx.dataBuffer[0] = readAddr;

    I2C_WriteReadAsync(  &i2cDeviceDp130,
                        dp130Ctx.dataBuffer,
                        1,                              // Number of write byte
                        1,                              // Number of read byte
                        I2CD_dp130GeneralReadHandler);
}

#ifdef DP130_CUSTOM_FUNCTIONS_NEEDED

//#################################################################################################
// Set function for DP130 Auto Shutdown and Force Power Down
//
// Parameters:
//      reg                 - auto shutdown and force power down
//      writeCompleteHandler - callback for write completion
// Return:
// Assumptions:
//#################################################################################################
void I2CD_setDp130AutoShutDnFrcPwrDn(
    const union I2cdDp130AutoShutDnFrcPwrDn* reg, void (*writeCompleteHandler)(void))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.write.writeCompletionHandler = writeCompleteHandler;
    // load data buffer with address and data to write
    dp130Ctx.dataBuffer[0] = I2CD_DP130_AUTO_PWR_DN_FRC_DN;
    memcpy(&(dp130Ctx.dataBuffer[1]), &(reg->raw), sizeof(reg->raw));

    // kick off I2C write
    performWrite(sizeof(reg->raw) + 1);
}

//#################################################################################################
// Get function for DP130 Auto Shutdown and Force Power Down
//
// Parameters:
//      readCompleteHandler - callback function for handler read response
// Return:
// Assumptions:
//      * Calling function provides callBack function, driver provides memory space that's
//      * alive during callBack life, once callback returns to driver, buffer is reused
//#################################################################################################
void I2CD_getDp130AutoShutDnFrcPwrDn(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.read.readCompletionHandler = readCompleteHandler;

    dp130Ctx.dataBuffer[0] = I2CD_DP130_AUTO_PWR_DN_FRC_DN;

    performRead(SIZEOF_RAW(union I2cdDp130AutoShutDnFrcPwrDn));
}

//#################################################################################################
// Set function for DP130 Squelch values
//
// Parameters:
//      reg                 - squelch values
//      writeCompleteHandler - callback for write completion
// Return:
// Assumptions:
//#################################################################################################
void I2CD_setDp130Squelch(const union I2cdDp130Squelch* reg, void (*writeCompleteHandler)(void))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.write.writeCompletionHandler = writeCompleteHandler;
    // load data buffer with address and data to write
    dp130Ctx.dataBuffer[0] = I2CD_DP130_SQUELCH;
    memcpy(&(dp130Ctx.dataBuffer[1]), &(reg->raw), sizeof(reg->raw));

    // kick off I2C write
    performWrite(sizeof(reg->raw) + 1);
}

//#################################################################################################
// Get function for DP130 Squelch values
//
// Parameters:
//      readCompleteHandler - callback function for handler read response
// Return:
// Assumptions:
//      * Calling function provides callBack function, driver provides memory space that's
//      * alive during callBack life, once callback returns to driver, buffer is reused
//#################################################################################################
void I2CD_getDp130Squelch(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.read.readCompletionHandler = readCompleteHandler;

    dp130Ctx.dataBuffer[0] = I2CD_DP130_SQUELCH;

    performRead(SIZEOF_RAW(union I2cdDp130Squelch));
}

//#################################################################################################
// Set function for DP130 Enable Link Training and Aux Source
//
// Parameters:
//      reg                 - enable link training, set aux source
// Return:
// Assumptions:
//#################################################################################################
void I2CD_setDp130EnLinkTrainingAuxSrc(
    const union I2cdDp130EnLinkTrainingAuxSrc* reg, void (*writeCompleteHandler)(void))
{
    iassert_I2CD_COMPONENT_1(
        (dp130ApiCtx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130ApiCtx.write.writeCompletionHandler = writeCompleteHandler;
    // load data buffer with address and data to write
    dp130ApiCtx.dataBuffer[0] = I2CD_DP130_EN_LNK_TRN_AUX_SRC;
    memcpy(&(dp130ApiCtx.dataBuffer[1]), &(reg->raw), sizeof(reg->raw));

    // kick off I2C write
    performWrite(sizeof(reg->raw) + 1);
}

//#################################################################################################
// Get function for DP130 Enable Link Training and Aux Source
//
// Parameters:
//      readCompleteHandler - callback function for handler read response
// Return:
// Assumptions:
//      * Calling function provides callBack function, driver provides memory space that's
//      * alive during callBack life, once callback returns to driver, buffer is reused
//#################################################################################################
void I2CD_getDp130EnLinkTrainingAuxSrc(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.dataBuffer[0] = I2CD_DP130_EN_LNK_TRN_AUX_SRC;

    dp130Ctx.read.readCompletionHandler = readCompleteHandler;
    performRead(SIZEOF_RAW(union I2cdDp130EnLinkTrainingAuxSrc));
}

//#################################################################################################
// Set function for DP130 AEQ Levels for all lanes
//
// Parameters:
//      reg                 - lanes and their respective levels for aeq
//      completionHandler   - callback for i2c write
// Return:
// Assumptions:
//#################################################################################################
void I2CD_setDp130AllLanesLevels(
    const union I2cdDp130AllLanesLevels* reg, void (*writeCompleteHandler)(void))
{
    iassert_I2CD_COMPONENT_1(
        (dp130ApiCtx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);
    dp130ApiCtx.write.writeCompletionHandler = writeCompleteHandler;

    dp130ApiCtx.dataBuffer[0] = I2CD_DP130_EQ_I2C_ENABLE;
    memcpy(&(dp130ApiCtx.dataBuffer[1]), &(reg->raw), sizeof(reg->raw));

    performWrite(sizeof(reg->raw) + 1);
}

//#################################################################################################
// Get function for DP130 AEQ levels for all lanes
//
// Parameters:
//      readCompleteHandler - callback function for handler read response
// Return:
// Assumptions:
//      * Calling function provides callBack function, driver provides memory space that's
//      * alive during callBack life, once callback returns to driver, buffer is reused
//#################################################################################################
void I2CD_getDp130AllLanesLevels(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);
    dp130Ctx.read.readCompletionHandler = readCompleteHandler;

    dp130Ctx.dataBuffer[0] = I2CD_DP130_EQ_I2C_ENABLE;

    performRead(SIZEOF_RAW(union I2cdDp130AllLanesLevels));
}

//#################################################################################################
// Set function for DP130 Boost, DP TMDS, VOD VPRE
//
// Parameters:
//      reg                 - pre-emphasis settings
//      writeCompleteHandler - callback for write completion
// Return:
// Assumptions:
//#################################################################################################
void I2CD_setDp130BoostDpTmdsVodVpre(
    const union I2cdDp130BoostDpTmdsVodVpre* reg, void (*writeCompleteHandler)(void))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.write.writeCompletionHandler = writeCompleteHandler;
    // load data buffer with address and data to write
    dp130Ctx.dataBuffer[0] = I2CD_DP130_BOOST_DP_TMDS_VOD_VPRE;
    memcpy(&(dp130Ctx.dataBuffer[1]), &(reg->raw), sizeof(reg->raw));

    // kick off I2C write
    performWrite(sizeof(reg->raw) + 1);
}

//#################################################################################################
// Get function for DP130 Boost, DP TMDS, VOD VPRE
//
// Parameters:
//      readCompleteHandler - callback function for handler read response
// Return:
// Assumptions:
//      * Calling function provides callBack function, driver provides memory space that's
//      * alive during callBack life, once callback returns to driver, buffer is reused
//#################################################################################################
void I2CD_getDp130BoostDpTmdsVodVpre(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.dataBuffer[0] = I2CD_DP130_BOOST_DP_TMDS_VOD_VPRE;

    dp130Ctx.read.readCompletionHandler = readCompleteHandler;
    performRead(SIZEOF_RAW(union I2cdDp130BoostDpTmdsVodVpre));
}

//#################################################################################################
// Set function for DP130 HPD, CAD, Tst Mode
//
// Parameters:
//      reg                 - HPD, CAD, Tst Mode
//      writeCompleteHandler - callback for write completion
// Return:
// Assumptions:
//#################################################################################################
void I2CD_setDp130HpdCadTstMode(
    const union I2cdDp130HpdCadTstMode* reg, void (*writeCompleteHandler)(void))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.write.writeCompletionHandler = writeCompleteHandler;
    // load data buffer with address and data to write
    dp130Ctx.dataBuffer[0] = I2CD_DP130_HPD_CAD_TEST_MODE;
    memcpy(&(dp130Ctx.dataBuffer[1]), &(reg->raw), sizeof(reg->raw));

    // kick off I2C write
    performWrite(sizeof(reg->raw) + 1);
}

//#################################################################################################
// Get function for DP130 HPD, CAD, Tst Mode
//
// Parameters:
//      readCompleteHandler - callback function for handler read response
// Return:
// Assumptions:
//      * Calling function provides callBack function, driver provides memory space that's
//      * alive during callBack life, once callback returns to driver, buffer is reused
//#################################################################################################
void I2CD_getDp130HpdCadTstMode(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.dataBuffer[0] = I2CD_DP130_HPD_CAD_TEST_MODE;

    dp130Ctx.read.readCompletionHandler = readCompleteHandler;
    performRead(SIZEOF_RAW(union I2cdDp130HpdCadTstMode));
}

//#################################################################################################
// Set function for DP130 I2C Soft Reset, DPCD Reset
//
// Parameters:
//      reg                 - reset i2c, reset dpcd
//      writeCompleteHandler - callback for write completion
// Return:
// Assumptions:
//#################################################################################################
void I2CD_setDp130I2cSoftRstDpcdRst(
    const union I2cdDp130I2cSoftRstDpcdRst* reg, void (*writeCompleteHandler)(void))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.write.writeCompletionHandler = writeCompleteHandler;
    // load data buffer with address and data to write
    dp130Ctx.dataBuffer[0] = I2CD_DP130_I2C_DPCD_RST;
    memcpy(&(dp130Ctx.dataBuffer[1]), &(reg->raw), sizeof(reg->raw));

    // kick off I2C write
    performWrite(sizeof(reg->raw) + 1);
}

//#################################################################################################
// Get function for DP130 I2C Soft Reset, DPCD Reset
//
// Parameters:
//      readCompleteHandler - callback function for handler read response
// Return:
// Assumptions:
//      * Calling function provides callBack function, driver provides memory space that's
//      * alive during callBack life, once callback returns to driver, buffer is reused
//#################################################################################################
void I2CD_getDp130I2cSoftRstDpcdRst(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.dataBuffer[0] = I2CD_DP130_I2C_DPCD_RST;

    dp130Ctx.read.readCompletionHandler = readCompleteHandler;
    performRead(SIZEOF_RAW(union I2cdDp130I2cSoftRstDpcdRst));
}

//#################################################################################################
// Set function for DP130 DPCD Address
//
// Parameters:
//      reg                 - dpcd address
//      writeCompleteHandler - callback for write completion
// Return:
// Assumptions:
//#################################################################################################
void I2CD_setDp130DpcdAddr(const union I2cdDp130DpcdAddr* reg, void (*writeCompleteHandler)(void))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.write.writeCompletionHandler = writeCompleteHandler;
    // load data buffer with address and data to write
    dp130Ctx.dataBuffer[0] = I2CD_DP130_DPCD_ADDR_HIGH;
    memcpy(&(dp130Ctx.dataBuffer[1]), &(reg->raw), sizeof(reg->raw));

    // kick off I2C write
    performWrite(sizeof(reg->raw) + 1);
}

//#################################################################################################
// Get function for DP130 DPCD Address
//
// Parameters:
//      readCompleteHandler - callback function for handler read response
// Return:
// Assumptions:
//      * Calling function provides callBack function, driver provides memory space that's
//      * alive during callBack life, once callback returns to driver, buffer is reused
//#################################################################################################
void I2CD_getDp130DpcdAddr(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.dataBuffer[0] = I2CD_DP130_DPCD_ADDR_HIGH;

    dp130Ctx.read.readCompletionHandler = readCompleteHandler;
    performRead(SIZEOF_RAW(union I2cdDp130DpcdAddr));
}

//#################################################################################################
// Set function for DP130 DPCD data
//
// Parameters:
//      reg                 - dpcd data
//      writeCompleteHandler - callback for write completion
// Return:
// Assumptions:
//#################################################################################################
void I2CD_setDp130DpcdData(const union I2cdDp130DpcdData* reg, void (*writeCompleteHandler)(void))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.write.writeCompletionHandler = writeCompleteHandler;
    // load data buffer with address and data to write
    dp130Ctx.dataBuffer[0] = I2CD_DP130_DPCD_DATA;
    memcpy(&(dp130Ctx.dataBuffer[1]), &(reg->raw), sizeof(reg->raw));

    // kick off I2C write
    performWrite(sizeof(reg->raw) + 1);
}

//#################################################################################################
// Get function for DP130 DPCD data
//
// Parameters:
//      readCompleteHandler - callback function for handler read response
// Return:
// Assumptions:
//      * Calling function provides callBack function, driver provides memory space that's
//      * alive during callBack life, once callback returns to driver, buffer is reused
//#################################################################################################
void I2CD_getDp130DpcdData(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.dataBuffer[0] = I2CD_DP130_DPCD_DATA;

    dp130Ctx.read.readCompletionHandler = readCompleteHandler;
    performRead(SIZEOF_RAW(union I2cdDp130DpcdData));
}

//#################################################################################################
// Set function for DP130 Device ID
//
// Parameters:
//      reg                 - device ID
//      writeCompleteHandler - callback for write completion
// Return:
// Assumptions:
//#################################################################################################
void I2CD_setDp130DevIdRevBitInvert(
    const union I2cdDp130DevIdRevBitInvert* reg, void (*writeCompleteHandler)(void))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.write.writeCompletionHandler = writeCompleteHandler;
    // load data buffer with address and data to write
    dp130Ctx.dataBuffer[0] = I2CD_DP130_DEV_ID_REV;
    memcpy(&(dp130Ctx.dataBuffer[1]), &(reg->raw), sizeof(reg->raw));

    // kick off I2C write
    performWrite(sizeof(reg->raw) + 1);
}

//#################################################################################################
// Get function for DP130 Device ID
//
// Parameters:
//      readCompleteHandler - callback function for handler read response
// Return:
// Assumptions:
//      * Calling function provides callBack function, driver provides memory space that's
//      * alive during callBack life, once callback returns to driver, buffer is reused
//#################################################################################################
void I2CD_getDp130DevIdRevBitInvert(
    void (*readCompleteHandler)(uint8_t* data, uint8_t byteCount))
{
    iassert_I2CD_COMPONENT_1(
        (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
        DP130_TRANS_IN_PROGRESS,
        __LINE__);

    dp130Ctx.dataBuffer[0] = I2CD_DP130_DEV_ID_REV;

    dp130Ctx.read.readCompletionHandler = readCompleteHandler;
    performRead(SIZEOF_RAW(union I2cdDp130DevIdRevBitInvert));
}


// Component Scope Function Definitions ###########################################################


// Static Function Definitions ####################################################################

//#################################################################################################
// Reads from the transaction queue and executes an I2C read or write
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void performRead(const uint8_t byteCount)
{
    dp130Ctx.state = DP130_TRANSACTION_STATE_READ;
    const uint8_t writeByteCount = 1;

    I2C_WriteReadAsync(
        &i2cDeviceDp130,
        dp130Ctx.dataBuffer,
        writeByteCount,
        byteCount,
        &_I2CD_dp130ReadCompleteHandler);
}


//#################################################################################################
// I2C read callback
//
// Parameters:
//      success             - data and bytecount, should be 1 byte read
// Return:
// Assumptions:
//      * Calling function of read transaction should supply a buffer matching the size of the
//      * struct they are reading to avoid data corruption
//#################################################################################################
static void _I2CD_dp130ReadCompleteHandler(uint8_t* data, uint8_t byteCount)
{
    iassert_I2CD_COMPONENT_0(
        ((data != NULL) && (byteCount > 0)),
        DP130_READ_FAILED);

    void (* cb)(uint8_t*, uint8_t) = dp130Ctx.read.readCompletionHandler;
    _I2CD_dp130ResetTransactionData();
    (*cb)(data, byteCount);
}

#endif // DP130_CUSTOM_FUNCTIONS_NEEDED

//#################################################################################################
// Resets the transaction tracking variables to their default state.  This function should be
// called each time a transaction completes.
//
// Parameters:
// Return:
// Assumptions:
//      * There is no active I2C transaction with the DP130 chip.
//#################################################################################################
static void _I2CD_dp130ResetTransactionData(void)
{
    dp130Ctx.state = DP130_TRANSACTION_STATE_INACTIVE;
}

//#################################################################################################
// DP130 genaral read function
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void I2CD_dp130GeneralReadHandler(uint8_t* data, uint8_t byteCount)
{
    _I2CD_dp130ResetTransactionData();
    ilog_I2CD_COMPONENT_1(ILOG_USER_LOG, DP130_GENERAL_READ, data[0]);
}


//#################################################################################################
// Reads from the transaction queue and executes an I2C read or write
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void performWrite(const uint8_t byteCount)
{
    dp130Ctx.state = DP130_TRANSACTION_STATE_WRITE;
    I2C_WriteAsync(
        &i2cDeviceDp130,
        dp130Ctx.dataBuffer,
        byteCount,
        _I2CD_dp130WriteCompleteHandler);
}

//#################################################################################################
// I2C write callback
//
// Parameters:
//      success             - status of last write transaction
// Return:
// Assumptions:
//#################################################################################################
static void _I2CD_dp130WriteCompleteHandler(bool success)
{
    if(!success)
    {
        ilog_I2CD_COMPONENT_0(ILOG_FATAL_ERROR, DP130_WRITE_FAILED);
    }

    if(dp130Ctx.write.writeCompletionHandler)
    {
        dp130Ctx.write.writeCompletionHandler(success);
    }
    _I2CD_dp130ResetTransactionData();
}

//#################################################################################################
// DP130 400ms reset wait timer
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void DP130ResetWaitTimerHandler(void)
{
    I2CD_dp130SetEqEnable();
}

//#################################################################################################
// Set dp130 eq enable
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void I2CD_dp130SetEqEnable(void)
{
    iassert_I2CD_COMPONENT_1(
    (dp130Ctx.state == DP130_TRANSACTION_STATE_INACTIVE),
    DP130_TRANS_IN_PROGRESS,
    __LINE__);

    dp130Ctx.write.writeCompletionHandler = I2CD_dp130SetEqEnableHandler;

    // load data buffer with address and data to write
    dp130Ctx.dataBuffer[0] = I2CD_DP130_EQ_I2C_ENABLE;
    dp130Ctx.dataBuffer[1] = EQ_I2C_ENABLE_MASK;

    performWrite(2);        // Write Address and value : 2byte
}


//#################################################################################################
// dp130 eq write async event handler
// calls init callback after writing done
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void I2CD_dp130SetEqEnableHandler(bool success)
{
    //Eq setting is the only initial setting for dp130
    dp130Ctx.initSuccess = success;

    if(dp130Ctx.initCallback)
    {
        dp130Ctx.initCallback(success);
    }
}
