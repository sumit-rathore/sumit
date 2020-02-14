///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012, 2013
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - atmel_packets_i2c.c
//
//!   @brief -
//
//!   @note  - Normal chip state machine event flow, & for recovery read below
//              Submit operation from sleep state:
//                  ATMEL_ASLEEP
//                  -> ATMEL_CREATE_WRITE_CRC
//                  -> ATMEL_WAKING_UP
//                  -> ATMEL_AWAKE      // pass through state
//                  -> ATMEL_WRITE
//                          on failure do recovery sequence
//                  -> ATMEL_PROCESSING
//                  -> ATMEL_READ
//                          on failure do recovery sequence
//                  -> ATMEL_VERIFY_READ_CRC
//                          on failure do recovery sequence
//                  -> ATMEL_AWAKE      // Followed by sending a sleep command
//                  -> ATMEL_ASLEEP
//
//              There isn't actually any need to track state, other than busy
//              or not, and which CRC (read/write) is being calculated.  The
//              remaining states are for assert checks within event handlers.
//              Busy means not asleep
//
//              This chip tends to fail sometimes.  Failures are usually the
//              chip seeming to reset, which results in the chip in its sleep
//              state.  If an operation fails the following happens
//              1) send a wakeup: it might already be awake
//              2) put the chip to sleep: now it is determinstically asleep
//              3) wake up the chip, and restart the whole i2c transaction
//                  -> ATMEL_WAKING_UP
//                  -> ATMEL_AWAKE      // Followed by sending a sleep command
//                  -> ATMEL_ASLEEP
//                  -> ATMEL_WAKING_UP  // jump to normal flow above
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "atmel_crypto_loc.h"

/************************ Defined Constants and Macros ***********************/

#if 0
#define _ATMEL_GO_INACTIVE_CMD (0x02) // idle command, TempKey and RNG seed register are retained
#else
#define _ATMEL_GO_INACTIVE_CMD (0x01) // sleep command, all volatile state is reset
#endif

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

static struct
{
    uint8 writeBuffer[85];  // Size taken from max size in documentation, then +1 for i2c write address
                            // [0] contains the write address (0x03) for basically all Atmel operations
                            // [1] contains the transaction size (not including [0])
                            // [2] contains the opcode
                            // [3] contains the param1
                            // [4] contains the param2 lsb
                            // [5] contains the param2 msb
                            // following data bytes are the optional data paramaters, followed by the CRC

    uint8 readBuffer[35];   // [0] contains the transaction size, up to 32 data bytes, followed by 2 byte CRC

    enum { ATMEL_ASLEEP, ATMEL_WAKING_UP, ATMEL_CREATE_WRITE_CRC, ATMEL_AWAKE, ATMEL_WRITE, ATMEL_PROCESSING, ATMEL_READ, ATMEL_VERIFY_READ_CRC} chipState;
    uint8 readReqSize;
    uint8 simpleCmd;
    TIMING_TimerHandlerT executionTimer;
    TASKSCH_TaskT crcTask;
    void (*completionHandler)(uint8 * data, uint8 dataSize, void * userPtr);
    void * userPtr;
    enum GRG_I2cSpeed speed;
    uint8 retryCount;

    boolT doneFirstCmd; // Since the ATSHA204 has no reset line, this is to ensure the 1st cmd doesn't interrupt a cmd from before the reset
    boolT isInitializing; // Used to determine which logging messages are used for the recovery sequence
} atmel_i2c __attribute__((aligned(4)));

/************************ Local Function Prototypes **************************/

static void atmel_crc(TASKSCH_TaskT, uint32 taskArg);
static void atmel_start_wakeup(
#ifdef _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
        const uint32 crcTime
#else
        void
#endif
    ) __attribute__((noinline, section(".ftext")));
static void atmel_wokeup(void) __attribute__((section(".ftext")));
static void atmel_writeDone(boolT success) __attribute__((section(".ftext")));
static void atmel_executionTimeElapsed(void) __attribute__((section(".ftext")));
static void atmel_readDone(uint8 * data, uint8 byteCount) __attribute__((section(".ftext")));
static void atmel_idleDone(boolT success) __attribute__((section(".ftext")));
static void atmel_i2cComplete(const boolT crc_match, uint16 crc
#ifdef _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
    , const uint32 crcTime
#endif
    ) __attribute__((noinline, section(".ftext")));

static void atmel_assertHelper(uint32 line) __attribute__((noinline, noreturn));
static void atmel_debugDumpState(void);

// Recovery functions
static void atmel_startRecovery(void) __attribute__((section(".ftext")));
static void atmel_recoveryWokeup(void) __attribute__((section(".ftext")));
static void atmel_recoveryInactive(boolT success) __attribute__((section(".ftext")));

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _ATMEL_i2cInit()
*
* @brief  - Initialization function
*
* @return - void
*
* @note   - sets up default state, and registers timer and task
*
*/
void _ATMEL_i2cInit(void)
{
    atmel_i2c.executionTimer = TIMING_TimerRegisterHandler(
        &atmel_executionTimeElapsed, FALSE, 1); // Dummy timeout, as it set later

    atmel_i2c.crcTask = TASKSCH_InitTask(
        &atmel_crc, // void (*task)(TASKSCH_TaskT, uint32 taskArg),
        0,          //uint32 taskArg,
        TRUE,       //boolT allowInterrupts,
        TASKSCH_ATMEL_TASK_PRIORITY);

    atmel_i2c.chipState = ATMEL_ASLEEP;
    atmel_i2c.speed = ATMEL_CRYPTO_I2C_SPEED;
    atmel_i2c.isInitializing = TRUE;
}


/**
* FUNCTION NAME: atmel_setSpeed()
*
* @brief  - icmd to set the i2c speed
*
* @return - void
*
* @note   -
*
*/
void atmel_setSpeed
(
    uint8 newSpeed
)
{
    ilog_ATMEL_CRYPTO_COMPONENT_2(ILOG_USER_LOG, SET_SPEED, atmel_i2c.speed, newSpeed);
    atmel_i2c.speed = newSpeed;
}


/**
* FUNCTION NAME: atmel_assertHelper()
*
* @brief  - An assert helper
*
* @return - never
*
* @note   - Runs outside of IRAM, intended to help save IRAM
*
*/
static void atmel_assertHelper(uint32 line)
{
    iassert_ATMEL_CRYPTO_COMPONENT_1(FALSE, INVALID_COND, line);
    __builtin_unreachable();
}


/**
* FUNCTION NAME: __ATMEL_submitI2cOperation()
*
* @brief  - Submit an i2c operation to the Atmel chip
*
* @return - TRUE if operation was submitted, FALSE otherwise
*
* @note   - See the top of the file for the process of a complete i2c operation
*
*/
boolT __ATMEL_submitI2cOperation
(
    uint32 bufStart, // Prepoulated for first 4 bytes of the buffer to transmitt
    uint16 param2,
    uint8 writeDataSize,
    const uint8 * writeData,
    uint8 readDataSize,
    void (*completionHandler)(uint8 * data, uint8 dataSize, void * userPtr),
    void * userPtr,
    uint32 operationExecutionTime
)
{
    if (atmel_i2c.chipState != ATMEL_ASLEEP)
    {
        // System is busy, try again later
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_DEBUG, ATMEL_SUBMIT_I2C_SYS_BUSY);
        return FALSE;
    }

    ilog_ATMEL_CRYPTO_COMPONENT_3(
        ILOG_DEBUG, ATMEL_SUBMIT_I2C_1, atmel_i2c.writeBuffer[2], atmel_i2c.writeBuffer[3], param2);
    ilog_ATMEL_CRYPTO_COMPONENT_3(
        ILOG_DEBUG, ATMEL_SUBMIT_I2C_2, writeDataSize, readDataSize, (uint32)completionHandler);


    // setup operation
    atmel_i2c.retryCount = 0;
    TIMING_TimerResetTimeout(atmel_i2c.executionTimer, operationExecutionTime);
    atmel_i2c.completionHandler = completionHandler;
    atmel_i2c.userPtr = userPtr;
    atmel_i2c.readReqSize = readDataSize;
    *CAST((&atmel_i2c.writeBuffer[0]), uint8 *, uint32 *) = bufStart; // This populates entries 0 to 3 of the buffer
    if (atmel_i2c.writeBuffer[1] >= sizeof(atmel_i2c.writeBuffer))
    {
        atmel_assertHelper(__LINE__);
    }
    atmel_i2c.writeBuffer[4] = param2 & 0xFF; //LSB
    atmel_i2c.writeBuffer[5] = param2 >> 8; // MSB
    if (writeDataSize != 0)
    {
        if (writeData == NULL)
        {
            atmel_assertHelper(__LINE__);
        }
        memcpy(&atmel_i2c.writeBuffer[6], writeData, writeDataSize);
    }

    TASKSCH_StartTask(atmel_i2c.crcTask);
    atmel_i2c.chipState = ATMEL_CREATE_WRITE_CRC;

    return TRUE;
}


/**
* FUNCTION NAME: atmel_crc()
*
* @brief  - Idle task to calculate a CRC for an i2c packet
*
* @return - void
*
* @note   - Used to both verify incoming packets, and create CRC for outgoing packets
*
*           See the top of the file for the process of a complete i2c operation
*
*           INTERRUPTS ARE ENABLED!!!!
*/
static void atmel_crc(TASKSCH_TaskT task, uint32 taskArg)
{
    uint16 crc;
    uint8 crc_msb;
    uint8 crc_lsb;
    uint8 packetSize;
    uint8 * buf;
#ifdef _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
    LEON_TimerValueT startTime;
    uint32 crcTime;
#endif


    // Find initial start location, and packet size
    if (atmel_i2c.chipState == ATMEL_CREATE_WRITE_CRC)
    {
        buf = &atmel_i2c.writeBuffer[1]; // skip over the reg address
    }
    else if (atmel_i2c.chipState == ATMEL_VERIFY_READ_CRC)
    {
        buf = &atmel_i2c.readBuffer[0];
    }
    else
    {
        atmel_assertHelper(__LINE__);
    }
    packetSize = buf[0];


    // Calculate CRC
#ifdef _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
    startTime = LEON_TimerRead();
#endif
    crc = CRC_crc16(buf, packetSize - 2); // don't include 2 byte CRC at the end
    crc_msb = crc >> 8;
    crc_lsb = crc & 0xFF;
#ifdef _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
    crcTime = LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead());
#endif


    // Now that CRC calculation is done, go to the next step
    if (atmel_i2c.chipState == ATMEL_CREATE_WRITE_CRC)
    {
        // Set CRC
        buf[packetSize - 2] = crc_lsb;
        buf[packetSize - 1] = crc_msb;

        atmel_start_wakeup(
#ifdef _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
            crcTime
#endif
            );
    }
    else
    {
        // Check CRC
        const boolT crc_match = (
                (crc_lsb == buf[packetSize - 2])
            &&  (crc_msb == buf[packetSize - 1]));

        atmel_i2cComplete(
            crc_match,
            crc
#ifdef _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
            , crcTime
#endif
            );
    }
}


/**
* FUNCTION NAME: atmel_start_wakeup()
*
* @brief  - Packet is now ready to send to the chip, so start the wakeup process
*
* @return - void
*
* @note   - Interrupts are enabled on entry
*
*           See the top of the file for the process of a complete i2c operation
*/
static void atmel_start_wakeup
(
#ifdef _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
        const uint32 crcTime    // To help debug how long operations take
#else
        void
#endif
)
{
    // NOTE: Interrupts are enabled!!!!
    // So lets disabled them, so we can call something
    irqFlagsT flags = LEON_LockIrq();

#ifdef _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
    ilog_ATMEL_CRYPTO_COMPONENT_1(ILOG_MINOR_EVENT, CRC_WRITE_PACKET_DONE, crcTime);
#endif

    // We're done calculating CRC values
    TASKSCH_StopTask(atmel_i2c.crcTask);

    // Wakeup the chip
    atmel_i2c.chipState = ATMEL_WAKING_UP;
    ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_DEBUG, SENDING_ATMEL_I2C_WAKEUP);
    if (atmel_i2c.doneFirstCmd)
    {
        GRG_I2cWake(ATMEL_CRYPTO_I2C_BUS, &atmel_wokeup);
    }
    else
    {
        // 1st time through, we should jump straight to atmel_startRecovery()
        // This is in case the atmel chip is still processing a command from before a reset
        atmel_startRecovery();
        atmel_i2c.doneFirstCmd = TRUE;
    }

    LEON_UnlockIrq(flags);
}


/**
* FUNCTION NAME: atmel_wokeup()
*
* @brief  - Continuation function called after the Atmel chip has woken up.  It will send a i2c write
*
* @return - void
*
* @note   - This sends the command to the Atmel chip to process
*
*           See the top of the file for the process of a complete i2c operation
*/
static void atmel_wokeup(void)
{
    if (atmel_i2c.chipState != ATMEL_WAKING_UP)
    {
        atmel_assertHelper(__LINE__);
    }
    atmel_i2c.chipState = ATMEL_AWAKE; // NOTE: pass through state, this is going to change in 2 lines

    // Kick off write
    atmel_i2c.chipState = ATMEL_WRITE;
    GRG_I2cWriteASync(  ATMEL_CRYPTO_I2C_BUS,
                        ATMEL_CRYPTO_I2C_DEVICE,
                        atmel_i2c.speed,
                        atmel_i2c.writeBuffer,
                        atmel_i2c.writeBuffer[1] + 1,
                        &atmel_writeDone);
}


/**
* FUNCTION NAME: atmel_writeDone()
*
* @brief  - Continuation function called after the Atmel chip send command write is done
*
* @return - void
*
* @note   - This will start a delay timer, waiting for the Atmel chip to process the command
*
*           See the top of the file for the process of a complete i2c operation
*/
static void atmel_writeDone
(
    boolT success // was the write command a success, or was there an i2c failure?
)
{
    if (atmel_i2c.chipState != ATMEL_WRITE)
    {
        atmel_assertHelper(__LINE__);
    }

    if (!success)
    {
        // Failure, start the recovery process
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_MAJOR_ERROR, WRITE_FAILED);
        atmel_startRecovery();
    }
    else
    {
        // Success. The command was sent

        // Wait for Amtel chip to execute the command we just sent
        TIMING_TimerStart(atmel_i2c.executionTimer);
        atmel_i2c.chipState = ATMEL_PROCESSING;
    }
}


/**
* FUNCTION NAME: atmel_executionTimeElapsed()
*
* @brief  - Timer function indicating the Atmel chip is done processing the command
*
* @return - void
*
* @note   - This kicks off the read of the Atmel chip, to get the command result
*
*           See the top of the file for the process of a complete i2c operation
*/
static void atmel_executionTimeElapsed(void)
{
    const uint8 i2cBytesToRead = atmel_i2c.readReqSize + 3; // count + packet + crc-16

    if (atmel_i2c.chipState != ATMEL_PROCESSING)
    {
        atmel_assertHelper(__LINE__);
    }
    iassert_ATMEL_CRYPTO_COMPONENT_1(i2cBytesToRead <= sizeof(atmel_i2c.readBuffer), INVALID_LARGE_READ_REQUEST, i2cBytesToRead);

    // Kick off a read
    atmel_i2c.chipState = ATMEL_READ;
    GRG_I2cReadASync(   ATMEL_CRYPTO_I2C_BUS,
        ATMEL_CRYPTO_I2C_DEVICE,
        atmel_i2c.speed,
        atmel_i2c.readBuffer, i2cBytesToRead,
        &atmel_readDone);
}


/**
* FUNCTION NAME: atmel_readDone()
*
* @brief  - Continuation function called after the Atmel chip send read is done
*
* @return - void
*
* @note   - This will start the operation to put the Atmel chip back to sleep
*
*           See the top of the file for the process of a complete i2c operation
*/
static void atmel_readDone(uint8 * data, uint8 byteCount)
{
    if (atmel_i2c.chipState != ATMEL_READ)
    {
        atmel_assertHelper(__LINE__);
    }
    if (byteCount > sizeof(atmel_i2c.readBuffer))
    {
        atmel_assertHelper(__LINE__);
    }

    if (data == NULL)
    {
        // Failure, start the recovery process
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_MAJOR_ERROR, READ_FAILED);
        atmel_startRecovery();
    }
    else
    {
        // I2C read was successful

        if (data != &atmel_i2c.readBuffer[0])
        {
            atmel_assertHelper(__LINE__);
        }

        // Ensure we read the whole packet
        iassert_ATMEL_CRYPTO_COMPONENT_2(byteCount >= data[0], SHORT_PACKET, byteCount, data[0]);

        if (byteCount > data[0])
        {
            // Extra bytes ?? The chip could have returned valid packet with a 1 byte status code
            ilog_ATMEL_CRYPTO_COMPONENT_2(ILOG_WARNING, LONG_PACKET, byteCount, data[0]);

            // TODO: In bug 4046 byteCount is 7, but data[0] is 4
            // This is really bad.
            // Presumably we have just reset, and this packet was for an older command, from before the reset
            // Throwing in some debug to help diagnose
            atmel_debugDumpState();

            byteCount = data[0];
        }

        // This is an invalid short packet
        iassert_ATMEL_CRYPTO_COMPONENT_1(byteCount >= 4, INVALID_SHORT_PACKET, byteCount);

        // Allow a new transaction to start
        atmel_i2c.chipState = ATMEL_AWAKE;

        // put the chip into an inactive mode
        atmel_i2c.simpleCmd = _ATMEL_GO_INACTIVE_CMD;
        GRG_I2cWriteASync(  ATMEL_CRYPTO_I2C_BUS,
                ATMEL_CRYPTO_I2C_DEVICE,
                atmel_i2c.speed,
                &atmel_i2c.simpleCmd,
                1,
                &atmel_idleDone);
    }
}


/**
* FUNCTION NAME: atmel_idleDone()
*
* @brief  - Continuation function called after the Atmel chip has accepted the command to go to sleep
*
* @return - void
*
* @note   - This will start the idle task to verify the CRC received during the read phase
*
*           See the top of the file for the process of a complete i2c operation
*/
static void atmel_idleDone(boolT success)
{
    iassert_ATMEL_CRYPTO_COMPONENT_0(success, IDLE_FAILED);
    if (atmel_i2c.chipState != ATMEL_AWAKE)
    {
        atmel_assertHelper(__LINE__);
    }

    atmel_i2c.chipState = ATMEL_VERIFY_READ_CRC;
    TASKSCH_StartTask(atmel_i2c.crcTask);
}


/**
* FUNCTION NAME: atmel_i2cComplete()
*
* @brief  - This function is called when the incoming packet CRC has been checked
*
* @return - void
*
* @note   - Interrupts are enabled on entry
*
*           This will call back the completion handler from the __ATMEL_submitI2cOperation() caller
*
*           This concludes an i2c operation of the Atmel chip, and higher level code can now submit a new request
*
*           See the top of the file for the process of a complete i2c operation
*/
static void atmel_i2cComplete
(
    const boolT crc_match,
    uint16 crc
#ifdef _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
    , const uint32 crcTime
#endif
)
{
    const uint8 packetSize = atmel_i2c.readBuffer[0];
    // NOTE: Interrupts are enabled!!!!
    // So lets disabled them, so we can call something
    irqFlagsT flags = LEON_LockIrq();

#ifdef _ATMEL_MEASURE_MATH_FUNCTIONS_TIME
    ilog_ATMEL_CRYPTO_COMPONENT_1(ILOG_MINOR_EVENT, CRC_READ_PACKET_DONE, crcTime);
#endif

    // We're done calculating CRC values
    TASKSCH_StopTask(atmel_i2c.crcTask);

    atmel_i2c.chipState = ATMEL_ASLEEP;

    // check CRC
    if (!crc_match)
    {
        // Failure, start the recovery process
        ilog_ATMEL_CRYPTO_COMPONENT_2(
                ILOG_MAJOR_ERROR,
            CRC_FAIL,
            crc,
            (atmel_i2c.readBuffer[packetSize - 2])
            + (atmel_i2c.readBuffer[packetSize - 1] << 8));
        atmel_startRecovery();
    }
    else
    {
        // crc matches

        // note: return value skips over byte count, and doesn't include CRC16
        (*atmel_i2c.completionHandler)(&atmel_i2c.readBuffer[1], packetSize - 3, atmel_i2c.userPtr);
    }
    LEON_UnlockIrq(flags);
}


/**
* FUNCTION NAME: ATMEL_assertHook()
*
* @brief  - Assert helper function to log the internal state of this component
*
* @return - void
*
* @note   -
*
*/
void ATMEL_assertHook(void)
{
    atmel_debugDumpState();
}

static void atmel_debugDumpState(void)
{
    const uint32 writeBufStart = 0
        + (atmel_i2c.writeBuffer[0] << 24)
        + (atmel_i2c.writeBuffer[1] << 16)
        + (atmel_i2c.writeBuffer[2] << 8)
        + (atmel_i2c.writeBuffer[3] << 0);
    const uint32 readBufStart = 0
        + (atmel_i2c.readBuffer[0] << 24)
        + (atmel_i2c.readBuffer[1] << 16)
        + (atmel_i2c.readBuffer[2] << 8)
        + (atmel_i2c.readBuffer[3] << 0);

    ilog_ATMEL_CRYPTO_COMPONENT_3(ILOG_FATAL_ERROR, CHIP_STATE1,
            atmel_i2c.chipState, (uint32)atmel_i2c.completionHandler, (uint32)atmel_i2c.userPtr);
    ilog_ATMEL_CRYPTO_COMPONENT_3(ILOG_FATAL_ERROR, CHIP_STATE2,
            atmel_i2c.readReqSize, atmel_i2c.simpleCmd, writeBufStart);
    ilog_ATMEL_CRYPTO_COMPONENT_1(ILOG_FATAL_ERROR, CHIP_STATE3,
            readBufStart);
}


/**
* FUNCTION NAME: atmel_startRecovery()
*
* @brief  - starts an i2c recovery process.  Called after an i2c failure
*
* @return - void
*
* @note   - This can come from any state that accesses the Atmel chip
*           The only thing that is known is that the writeBuffer is ready for the chip
*
*           See the top of the file for the process of a complete i2c operation
*/
static void atmel_startRecovery(void)
{
    iassert_ATMEL_CRYPTO_COMPONENT_0(atmel_i2c.retryCount < ATMEL_I2C_RETRY_LIMIT, MAX_I2C_RETRY_EXCEEDED);
    atmel_i2c.retryCount++;
    // If this is the first pass through, we log a different message - logging initialization
    // as an error is confusing
    if(atmel_i2c.isInitializing == TRUE)
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_MAJOR_EVENT, ATMEL_INIT_STEP1);
    }
    else
    {
        ilog_ATMEL_CRYPTO_COMPONENT_1(ILOG_MAJOR_ERROR, I2C_RECOVERY_START, atmel_i2c.retryCount);
    }
    GRG_I2cWake(ATMEL_CRYPTO_I2C_BUS, &atmel_recoveryWokeup);
}


/**
* FUNCTION NAME: atmel_recoveryWokeup()
*
* @brief  - Continuation function after an initial wakeup process in recovery mode
*
* @return - void
*
* @note   - The chip really should be awake after this step, but it isn't known
*           what the internal state is
*
*           See the top of the file for the process of a complete i2c operation
*/
static void atmel_recoveryWokeup(void)
{
    // If this is the first pass through, we log a different message - logging initialization
    // as an error is confusing
    if (atmel_i2c.isInitializing == TRUE)
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_MAJOR_EVENT, ATMEL_INIT_STEP2);
    }
    else
    {
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_MAJOR_ERROR, I2C_RECOVERY_WOKEUP);
    }
    // put the chip into an inactive mode
    atmel_i2c.simpleCmd = _ATMEL_GO_INACTIVE_CMD;
    GRG_I2cWriteASync(  ATMEL_CRYPTO_I2C_BUS,
                        ATMEL_CRYPTO_I2C_DEVICE,
                        atmel_i2c.speed,
                        &atmel_i2c.simpleCmd,
                        1,
                        &atmel_recoveryInactive);
}


/**
* FUNCTION NAME: atmel_recoveryInactive()
*
* @brief  - Continuation function after the chip has been put to sleep
*
* @return - void
*
* @note   - This far into the recovery, the chip is almost guaranteed to be in sync
*           If the previous operation failed, then the chip was already asleep
*
*           See the top of the file for the process of a complete i2c operation
*/
static void atmel_recoveryInactive
(
    boolT success   // IE did the Atmel chip ACK the i2c transaction
)
{
    // If this is the first pass through, we log a different message - logging initialization
    // as an error is confusing
    if(atmel_i2c.isInitializing == TRUE)
    {
        ilog_ATMEL_CRYPTO_COMPONENT_1(ILOG_MAJOR_EVENT, ATMEL_INIT_STEP3, success);
    }
    else
    {
        ilog_ATMEL_CRYPTO_COMPONENT_1(ILOG_MAJOR_ERROR, I2C_RECOVERY_DONE, success);
    }
    // At this point, any retries are due to unexpected behaviour and can be logged as an error
    atmel_i2c.isInitializing = FALSE;

    if (success)
    {
        // now wake up chip and go back to normal processing
        atmel_i2c.chipState = ATMEL_WAKING_UP;
        ilog_ATMEL_CRYPTO_COMPONENT_0(ILOG_DEBUG, SENDING_ATMEL_I2C_WAKEUP);
        GRG_I2cWake(ATMEL_CRYPTO_I2C_BUS, &atmel_wokeup);
    }
    else
    {
        atmel_startRecovery();
    }
}

