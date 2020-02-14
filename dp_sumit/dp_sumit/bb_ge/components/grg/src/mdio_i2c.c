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
//!   @file  -  mdio_i2c.c
//
//!   @brief -  Driver for both MDIO and I2C
//
//!   @note  -  The MDIO & I2C HW share a register interface, so there can only
//              be 1 transaction at a time, regardless of type or bus.
//
//              This driver works with only asynchronous requests, and queues
//              them up in an ififo.  All code runs with interrupts disabled,
//              and should operate very quickly.
//
//              The ififo queues up all requests.  When the system is ready to
//              process a request, it starts the idle task, which will pop the
//              first request off the queue, and operates on it.  Then it marks
//              the global mdioI2c.opState as in progress to prevent a new
//              operation from starting.
//
//              Normal call flow:
//              * An I2C or MDIO public API function is called
//              * The public API function creates a union mdio_i2c_operation,
//                and calls the generic submitASyncOperation()
//              * submitASyncOperation() does the following
//                  * places the operation on the ififo queue
//                  * checks if the system is idle, if so start the idle task
//                    otherwise the operation will be started when HW is free
//              * mdioI2cIdleTask() will
//      `           1) pops the request off the ififo
//                  2) marks mdioI2c.opState as in progress
//                  2) start i2c and mdio operations, then stop the idle task
//                  3) run i2c wake ops, then call finalizeASyncOperation()
//              * i2c operations may enable almost full/empty irq to read/write
//                the hw buffer, then disable and wait for the completion irq
//              * MDIO & I2C done irq, calls finalizeASyncOperation()
//              * finalizeASyncOperation()
//                  * marks mdioI2c.opState as finalize
//                  * starts the idle task
//              * mdioI2cIdleTask() will
//                  * check if another tasks is queued up, or if a callback
//                    exists, then start/stop the idle task as needed
//                  * call the completion handler
//
//              I2C operations are only for the Spartan platform
//              MDIO operations are for the Spartan & Virtex platforms
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "grg_loc.h"
#include <tasksch.h>
#include <ififo.h>

/************************ Defined Constants and Macros ***********************/
#define I2C_HW_FIFO_SIZE 32
#define I2C_WAKE_TIME_DRIVE_SDA_LOW (60)
#define I2C_WAKE_TIME_IDLE          (3000)


/******************************** Data Types *********************************/

enum mdioI2cBus
{
    MDIO_BUS,   // Ethernet Phy
    I2C_BUS_0,  // Atmel Authentication Chip
    I2C_BUS_1   // Rex USB Hub
};

enum mdioI2cBusOperation
{
    MDIO_I2C_READ,
    MDIO_I2C_WRITE,
    I2C_WAKE,               // Perform a long pulse on the SDA line, to wake up sleeping devices
    I2C_WRITE_READ,         // Do a write, followed by a read.  For SMB reads.  IE first write device register address, then read it
    I2C_WRITE_READ_BLOCK,   // Do a write, followed by a read.  For SMB block reads. The read size depends on the value of the first byte read
    I2C_WRITE_READ_BLOCK_IN_PROGRESS // operation has started
};

enum i2cWakeState
{
    I2C_WAKE_DRIVE_SDA_LOW_STATE,   // drive SDA low for 60 microseconds
    I2C_WAKE_IDLE_TIME_STATE        // After a device has being woken up, there needs to be 2.5 ms of idle time
};

enum i2cWakeOperation
{
    I2C_WAKE_STOP = 0,      // Matches GRG_I2CMDIO_CONTROL_FORCESDAN_SET_BF() settings
    I2C_WAKE_START = 1      // Matches GRG_I2CMDIO_CONTROL_FORCESDAN_SET_BF() settings
};

union mdio_i2c_operation
{
    uint64 raw;
    struct {
        union {
            uint32 raw;
            struct {
                // NOTE: the order of fields is picked for smallest code size after compiler optimizations
                uint32 notifyCompletionHandler:LEON_PACKED_POINTER_BITS;// pointer to callback function
                uint32 deviceAddress:7;                                 // 7bit i2c addr or 5 bit mdio addr
                uint32 busOperation:3;                                  // use enum mdioI2cBusOperation
                uint32 bus:2;                                           // use enum mdioI2cBus
                uint32 i2cSpeed_i2cWakeState:2;                         // i2c specific:
                                                                        // use enum GRG_I2cSpeed for read/write
                                                                        // use i2cWakeState for i2c wake operations
                                                                        // ignored for MDIO
            };
        } header;
        union {
            uint32 i2cMdioRaw;
            struct {
                uint32 padding:11;                                      // unused
                uint32 regAddress:5;                                    // Internal device register addr
                uint32 writeData:16;                                    // data to write for write operation
            } mdio;
            struct {
                uint32 byteCount:7;                                     // Bytes to read/write
                uint32 bytesProcessed:7;                                // how many bytes have been read/written from/to HW FIFO
                uint32 ptrToData:LEON_PACKED_POINTER_BITS;              // pointer to data for read/write
            } i2c;
            struct {
                // The strategy here is no "almost empty/full" flags are needed
                // As only 0-31 bytes are allowed in either read or write direction
                // This keeps the code simpler
                uint32 padding1:3;                                      // unused
                uint32 byteCountRead:5;                                 // Bytes to read
                uint32 padding2:1;                                      // unused
                uint32 byteCountWrite:5;                                // Bytes to write
                uint32 ptrToData:LEON_PACKED_POINTER_BITS;              // pointer to data for read/write
            } i2cWriteRead;
            struct {
                // The strategy here is to do the write phase with the almost full fifo irq triggering
                // This ensures that the read phase and the write phase can be treated distinctly different
                // NOTE: The amount of read bytes is not yet known
                uint32 dataBufSize:6;                                   // Size of the data buffer
                uint32 padding:5;                                       // unused
                uint32 byteCountWrite:3;                                // Bytes to write
                uint32 ptrToData:LEON_PACKED_POINTER_BITS;              // pointer to data for read/write
            } i2cWriteReadBlock;
            struct {
                // The strategy here is to do the write phase with the almost full fifo irq triggering
                // This ensures that the read phase and the write phase can be treated distinctly different
                // NOTE: The amount of read bytes is not yet known, but will be the first byte read back
                uint32 dataBufSize:6;                                   // Size of the data buffer
                uint32 bytesProcessed:8;                                // how many bytes have been read from HW FIFO
                uint32 ptrToData:LEON_PACKED_POINTER_BITS;              // pointer to data for read/write
            } i2cWriteReadBlockInProgress;
            LEON_TimerValueT i2cWakeTimer;
        };
    };
};


/***************************** Local Variables *******************************/
struct {
    union mdio_i2c_operation curOp;
    TASKSCH_TaskT task;
#ifdef GRG_MDIO_I2C_USE_TIME_MARKERS
    LEON_TimerValueT lastTimeMarker;        // For debugging log time stamps
#endif
    enum { MDIO_I2C_OP_IN_PROGRESS, MDIO_I2C_OP_IDLE, MDIO_I2C_OP_FINALIZE }
        opState;
    uint32 finalizeArg1;
    uint32 finalizeArg2;
} mdioI2c;

/************************ Local Function Prototypes **************************/
// Call the ififo macro to create the local function prototypes
IFIFO_CREATE_FIFO_LOCK_UNSAFE(mdio_i2c, uint64, GRG_MDIO_I2C_OPERATIONS_FIFO_SIZE)
// Should create
// static inline boolT mdio_i2c_fifoEmpty(void);
// static inline boolT mdio_i2c_fifoFull(void);
// static inline uint32 mdio_i2c_fifoSpaceAvail(void);
// static inline uint32 mdio_i2c_fifoSpaceUsed(void);
// static inline void mdio_i2c_fifoWrite(uint64 newElement);
// static inline uint64 mdio_i2c_fifoRead(void);
// static inline uint64 mdio_i2c_fifoPeekRead(void);
// static inline uint64 * mdio_i2c_fifoPeekReadPtr(void);

static void mdioI2cIdleTask(TASKSCH_TaskT, uint32)      __attribute__ ((section(".ftext")));
static void mdioStart(uint64 rawOperation)              __attribute__ ((section(".ftext")));
static void i2cStart(void)                              __attribute__ ((section(".ftext")));
static void i2cWakeOp(enum i2cWakeOperation)            __attribute__ ((section(".ftext")));
static void submitASyncOperation(uint64 op, void * notifyHandler) __attribute__ ((section(".ftext")));
static void finalizeASyncOperation()                    __attribute__ ((section(".ftext"), noinline)); //NOTE: optional args
static void mdioI2cMarkTime(GRG_COMPONENT_ilogCodeT)    __attribute__ ((section(".ftext")));
static boolT i2cReadHwFifo(void)                        __attribute__ ((section(".ftext")));

static void mdioI2cAssertInvalidState(uint32 line)      __attribute__ ((noinline, noreturn));

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: mdioI2cMarkTime()
*
* @brief  - Print out a timer marker message.  Useful for determining how much time passes between events
*
* @return - void
*
* @note   - When this feature was added it increased code size by 284 bytes
*
*/
#ifdef GRG_MDIO_I2C_USE_TIME_MARKERS
static void mdioI2cMarkTime(GRG_COMPONENT_ilogCodeT msg)
{
    LEON_TimerValueT currTime = LEON_TimerRead();
    ilog_GRG_COMPONENT_1(ILOG_MINOR_EVENT, msg, LEON_TimerCalcUsecDiff(mdioI2c.lastTimeMarker, currTime));
    mdioI2c.lastTimeMarker = currTime;
}
#else
static inline void mdioI2cMarkTime(GRG_COMPONENT_ilogCodeT msg) { }
#endif


/**
* FUNCTION NAME: _GRG_MdioI2cInit()
*
* @brief  - Initialization function
*
* @return - void
*
* @note   -
*
*/
void _GRG_MdioI2cInit(void)
{
    mdioI2c.task = TASKSCH_InitTask(&mdioI2cIdleTask, 0, FALSE, TASKSCH_MDIO_I2C_TASK_PRIORITY);
#ifdef GRG_MDIO_I2C_USE_TIME_MARKERS
    mdioI2c.lastTimeMarker = LEON_TimerRead();
#endif

    // Set idle state
    mdioI2c.opState = MDIO_I2C_OP_IDLE;

    // Enable the done interrupt
    GRG_GRG_INTMSK_I2CMDIODONE_WRITE_BF(GRG_BASE_ADDR, 1);
}


/**
* FUNCTION NAME: _GRG_assertHookMdioI2c()
*
* @brief  - Called on an assert, to help debug the assert
*
* @return - void
*
* @note   -
*
*/
void _GRG_assertHookMdioI2c(void)
{
    mdioI2cStatus();

    // dump the rest of the fifo
    while (!mdio_i2c_fifoEmpty())
    {
        const union mdio_i2c_operation op = { .raw = mdio_i2c_fifoRead() };
        ilog_GRG_COMPONENT_2(ILOG_FATAL_ERROR, MDIO_I2C_QUEUED_OPERATION, op.header.raw, op.i2cMdioRaw);
    }

    ilog_GRG_COMPONENT_1(ILOG_FATAL_ERROR, I2C_MDIO_CONTROLREG_READ, GRG_I2CMDIO_CONTROL_READ_REG(GRG_BASE_ADDR));
}


/**
* FUNCTION NAME: mdioI2cStatus()
*
* @brief  - ICommand for checking current status
*
* @return - void
*
* @note   - Also called on the assert hook to help debug
*
*/
void mdioI2cStatus(void)
{
    // Check the MDIO state of where it is in processing ASync operations
    switch (mdioI2c.opState)
    {
        case MDIO_I2C_OP_IN_PROGRESS:
            ilog_GRG_COMPONENT_3(
                    ILOG_USER_LOG,
                    MDIO_I2C_OPERATIONS,
                    mdio_i2c_fifoSpaceUsed() + 1,
                    mdioI2c.curOp.header.raw,
                    mdioI2c.curOp.i2cMdioRaw);
            break;

        case MDIO_I2C_OP_FINALIZE:
            ilog_GRG_COMPONENT_3(
                    ILOG_USER_LOG,
                    MDIO_I2C_OPERATIONS_FINALIZE,
                    mdio_i2c_fifoSpaceUsed() + 1,
                    mdioI2c.finalizeArg1,
                    mdioI2c.finalizeArg2);
            break;

        case MDIO_I2C_OP_IDLE:
            if (mdio_i2c_fifoEmpty())
            {
                ilog_GRG_COMPONENT_0(ILOG_USER_LOG, MDIO_I2C_NO_OPERATIONS);
            }
            else
            {
                ilog_GRG_COMPONENT_1(ILOG_USER_LOG, MDIO_I2C_OPERATIONS_QUEUED, mdio_i2c_fifoSpaceUsed());
            }
            break;

        default:
            mdioI2cAssertInvalidState(__LINE__);
            break;
    }
}


/**
* FUNCTION NAME: mdioI2cAssertInvalidState()
*
* @brief  - Helper function to save IRAM on asserts
*
* @return - never
*
* @note   -
*
*/
static void mdioI2cAssertInvalidState(uint32 line)
{
    iassert_GRG_COMPONENT_3(
        FALSE,
        MDIO_I2C_INVALID_TASK_STATE,
        line,
        mdioI2c.curOp.header.raw,
        mdioI2c.curOp.i2cMdioRaw);
    __builtin_unreachable();
}

/**
* FUNCTION NAME: mdioStart()
*
* @brief  - Start a new MDIO transaction
*
* @return - void
*
* @note   -
*
*/
static void mdioStart
(
    uint64 rawOperation // actual type is union mdio_i2c_operation
                        // This is the operation to start
)
{
    uint32 controlReg;
    union mdio_i2c_operation op;
    op.raw = rawOperation;

    controlReg = 0;
    controlReg = GRG_I2CMDIO_CONTROL_PHYADDR_SET_BF(controlReg, op.header.deviceAddress);
    controlReg = GRG_I2CMDIO_CONTROL_RDBYTESREGADDR_SET_BF(controlReg, op.mdio.regAddress);
    // default is zero: controlReg = GRG_I2CMDIO_CONTROL_INDIRECTMODE_SET_BF(controlReg, 0);
    // default is zero: controlReg = GRG_I2CMDIO_CONTROL_I2CMDIOSEL_SET_BF(controlReg, 0);
    if (op.header.busOperation == MDIO_I2C_READ)
    {
        ilog_GRG_COMPONENT_2(ILOG_DEBUG, MDIO_START_READ, op.header.deviceAddress, op.mdio.regAddress);
        controlReg = GRG_I2CMDIO_CONTROL_ACCESSMODE_SET_BF(controlReg, 2);
    }
    else // write operation
    {
        ilog_GRG_COMPONENT_3(ILOG_DEBUG, MDIO_START_WRITE, op.header.deviceAddress, op.mdio.regAddress, op.mdio.writeData);
        controlReg = GRG_I2CMDIO_CONTROL_ACCESSMODE_SET_BF(controlReg, 1);

        // Ensure the control register is in i2c mode, before writing to WRITE_REG
        GRG_I2CMDIO_CONTROL_WRITE_REG(GRG_BASE_ADDR, controlReg);
        GRG_I2CMDIO_WDATA_WRITE_REG(GRG_BASE_ADDR, op.mdio.writeData);
    }

    // Kick off the transaction
    controlReg = GRG_I2CMDIO_CONTROL_GO_SET_BF(controlReg, 1);
    GRG_I2CMDIO_CONTROL_WRITE_REG(GRG_BASE_ADDR, controlReg);
    TASKSCH_StopTask(mdioI2c.task); // We are interrupt driven for mdio
    mdioI2cMarkTime(TIME_MARKER_MDIO_START);
}


/**
* FUNCTION NAME: i2cStart()
*
* @brief  - Start an i2c operation
*
* @return - void
*
* @note   -
*
*/
static void i2cStart(void)
{
    uint32 controlReg;
    uint32 irqMask;

    irqMask = GRG_GRG_INTMSK_READ_REG(GRG_BASE_ADDR);

    controlReg = 0;

    controlReg = GRG_I2CMDIO_CONTROL_PHYADDR_SET_BF(controlReg, mdioI2c.curOp.header.deviceAddress);
    controlReg = GRG_I2CMDIO_CONTROL_SPEEDSEL_SET_BF(controlReg, mdioI2c.curOp.header.i2cSpeed_i2cWakeState);
    controlReg = GRG_I2CMDIO_CONTROL_I2CMDIOSEL_SET_BF(controlReg, 1);
    controlReg = GRG_I2CMDIO_CONTROL_PORTSEL_SET_BF(
            controlReg,
            (mdioI2c.curOp.header.bus == I2C_BUS_0 ? 0 : 1));

    if (mdioI2c.curOp.header.busOperation == MDIO_I2C_READ)
    {
        const boolT opLargerThanHwFifo = mdioI2c.curOp.i2c.byteCount > I2C_HW_FIFO_SIZE;

        ilog_GRG_COMPONENT_1(ILOG_DEBUG, I2C_START_READ, mdioI2c.curOp.i2c.byteCount);

        controlReg = GRG_I2CMDIO_CONTROL_ACCESSMODE_SET_BF(controlReg, 0);
        controlReg = GRG_I2CMDIO_CONTROL_RDBYTESREGADDR_SET_BF(controlReg, mdioI2c.curOp.i2c.byteCount);
        irqMask = GRG_GRG_INTMSK_I2CFIFOALMOSTFULL_SET_BF(irqMask, opLargerThanHwFifo);
        irqMask = GRG_GRG_INTMSK_I2CFIFOALMOSTEMPTY_SET_BF(irqMask, 0);
    }
    else // write operation, or writeRead, or writeReadBlock operation
    {
        uint8 bytesToWriteNow;
        uint8 i;
        uint8 * data;

        if (mdioI2c.curOp.header.busOperation == MDIO_I2C_WRITE)
        {
            const boolT opLargerThanHwFifo = mdioI2c.curOp.i2c.byteCount > I2C_HW_FIFO_SIZE;

            ilog_GRG_COMPONENT_0(ILOG_DEBUG, I2C_START_WRITE);

            irqMask = GRG_GRG_INTMSK_I2CFIFOALMOSTEMPTY_SET_BF(irqMask, opLargerThanHwFifo);
            bytesToWriteNow = MIN(mdioI2c.curOp.i2c.byteCount, I2C_HW_FIFO_SIZE);
            mdioI2c.curOp.i2c.bytesProcessed = bytesToWriteNow;
            data = LEON_unpackPointer(mdioI2c.curOp.i2c.ptrToData);
            controlReg = GRG_I2CMDIO_CONTROL_ACCESSMODE_SET_BF(controlReg, 1);
        }
        else if (mdioI2c.curOp.header.busOperation == I2C_WRITE_READ)
        {
            ilog_GRG_COMPONENT_2(ILOG_DEBUG, I2C_START_WRITE_READ, mdioI2c.curOp.i2cWriteRead.byteCountWrite, mdioI2c.curOp.i2cWriteRead.byteCountRead);
            controlReg = GRG_I2CMDIO_CONTROL_RDBYTESREGADDR_SET_BF(controlReg, mdioI2c.curOp.i2cWriteRead.byteCountRead);

            bytesToWriteNow = mdioI2c.curOp.i2cWriteRead.byteCountWrite;
            data = LEON_unpackPointer(mdioI2c.curOp.i2cWriteRead.ptrToData);
            controlReg = GRG_I2CMDIO_CONTROL_ACCESSMODE_SET_BF(controlReg, 2);
        }
        else if (mdioI2c.curOp.header.busOperation == I2C_WRITE_READ_BLOCK)
        {
            const uint32 ptrToDataPacked = mdioI2c.curOp.i2cWriteReadBlock.ptrToData;
            const uint32 dataBufSize = mdioI2c.curOp.i2cWriteReadBlock.dataBufSize;
            ilog_GRG_COMPONENT_2(ILOG_DEBUG, I2C_START_WRITE_READ_BLOCK, mdioI2c.curOp.i2cWriteReadBlock.byteCountWrite, dataBufSize);

            bytesToWriteNow = mdioI2c.curOp.i2cWriteReadBlock.byteCountWrite;
            data = LEON_unpackPointer(ptrToDataPacked);
            irqMask = GRG_GRG_INTMSK_I2CFIFOALMOSTFULL_SET_BF(irqMask, 1);
            controlReg = GRG_I2CMDIO_CONTROL_ACCESSMODE_SET_BF(controlReg, 3);

            // Move to next state of the bus operation
            mdioI2c.curOp.header.busOperation = I2C_WRITE_READ_BLOCK_IN_PROGRESS;
            mdioI2c.curOp.i2cWriteReadBlockInProgress.dataBufSize = dataBufSize;
            mdioI2c.curOp.i2cWriteReadBlockInProgress.bytesProcessed = 0;
            mdioI2c.curOp.i2cWriteReadBlockInProgress.ptrToData = ptrToDataPacked;
        }
        else
        {
            mdioI2cAssertInvalidState(__LINE__);
        }

        // Ensure we are in i2c mode, before touch WRITE_REG
        GRG_I2CMDIO_CONTROL_WRITE_REG(GRG_BASE_ADDR, controlReg);

        for (i = 0; i < bytesToWriteNow; i++)
        {
            GRG_I2CMDIO_WDATA_WRITE_REG(GRG_BASE_ADDR, data[i]);
        }
    }

    // Kick off the transaction
    controlReg = GRG_I2CMDIO_CONTROL_GO_SET_BF(controlReg, 1);
    GRG_GRG_INTMSK_WRITE_REG(GRG_BASE_ADDR, irqMask);
    GRG_I2CMDIO_CONTROL_WRITE_REG(GRG_BASE_ADDR, controlReg);
    TASKSCH_StopTask(mdioI2c.task); // We are interrupt driven for i2c
    mdioI2cMarkTime(TIME_MARKER_I2C_START);
}


/**
* FUNCTION NAME: i2cWakeOp()
*
* @brief  - Process an i2c Wake operation
*
* @return - void
*
* @note   -
*
*/
static void i2cWakeOp
(
    enum i2cWakeOperation wakeOp  // Command to process
)
{
    uint32 controlReg = 0;

    ilog_GRG_COMPONENT_1(ILOG_DEBUG, I2C_DO_WAKE_OP, wakeOp);

    controlReg = GRG_I2CMDIO_CONTROL_I2CMDIOSEL_SET_BF(controlReg, 1);
    controlReg = GRG_I2CMDIO_CONTROL_PORTSEL_SET_BF(
            controlReg,
            (mdioI2c.curOp.header.bus == I2C_BUS_0) ? 0 : 1);
    controlReg = GRG_I2CMDIO_CONTROL_FORCESDAN_SET_BF(controlReg, wakeOp);
    GRG_I2CMDIO_CONTROL_WRITE_REG(GRG_BASE_ADDR, controlReg);
    mdioI2c.curOp.i2cWakeTimer = LEON_TimerRead();
}


/**
* FUNCTION NAME: mdioI2cIdleTask()
*
* @brief  - The main idle task
*
* @return - void
*
* @note   - See description at the top of the file
*
*/
static void mdioI2cIdleTask
(
    TASKSCH_TaskT task,
    uint32 taskArg      // unused
)
{
    uint32 controlReg = GRG_I2CMDIO_CONTROL_READ_REG(GRG_BASE_ADDR);

    if (    !GRG_I2CMDIO_CONTROL_FIFOEMPTY_GET_BF(controlReg)
         || (GRG_I2CMDIO_CONTROL_GO_GET_BF(controlReg) != 0))
    {
        mdioI2cAssertInvalidState(__LINE__);
    }

    if (mdioI2c.opState == MDIO_I2C_OP_IDLE)
    {
        // Nothing going on, start a new operation
        if (mdio_i2c_fifoEmpty())
        {
            mdioI2cAssertInvalidState(__LINE__);
        }
        mdioI2c.curOp.raw = mdio_i2c_fifoRead();
        mdioI2c.opState = MDIO_I2C_OP_IN_PROGRESS;

        if (mdioI2c.curOp.header.bus == MDIO_BUS)
        {
            // new MDIO transaction
            mdioStart(mdioI2c.curOp.raw);
        }
        else // I2C
        {
            if (mdioI2c.curOp.header.busOperation == I2C_WAKE)
            {
                mdioI2c.curOp.header.i2cSpeed_i2cWakeState = I2C_WAKE_DRIVE_SDA_LOW_STATE;
                i2cWakeOp(I2C_WAKE_START);
                mdioI2cMarkTime(TIME_MARKER_I2C_WAKE_START);
            }
            else // I2C read/write
            {
                i2cStart();
            }
        }
    }
    else if (mdioI2c.opState == MDIO_I2C_OP_IN_PROGRESS)
    {
        // Operation in progress

        // First check if this is wake operation, which is on a timer
        if (mdioI2c.curOp.header.busOperation == I2C_WAKE)
        {
            if (mdioI2c.curOp.header.i2cSpeed_i2cWakeState == I2C_WAKE_DRIVE_SDA_LOW_STATE)
            {
                if (LEON_TimerCalcUsecDiff(mdioI2c.curOp.i2cWakeTimer, LEON_TimerRead()) > I2C_WAKE_TIME_DRIVE_SDA_LOW)
                {
                    // wake complete, start the idle
                    mdioI2c.curOp.header.i2cSpeed_i2cWakeState = I2C_WAKE_IDLE_TIME_STATE;
                    i2cWakeOp(I2C_WAKE_STOP);
                    mdioI2cMarkTime(TIME_MARKER_I2C_WAKE_STOP);
                }
            }
            else if (mdioI2c.curOp.header.i2cSpeed_i2cWakeState == I2C_WAKE_IDLE_TIME_STATE)
            {
                if (LEON_TimerCalcUsecDiff(mdioI2c.curOp.i2cWakeTimer, LEON_TimerRead()) > I2C_WAKE_TIME_IDLE)
                {
                    // wake idle complete
                    ilog_GRG_COMPONENT_0(ILOG_DEBUG, I2C_WAKE_COMPLETE);
                    finalizeASyncOperation();
                }
            }
            else
            {
                mdioI2cAssertInvalidState(__LINE__);
            }
        }
        else
        {
            mdioI2cAssertInvalidState(__LINE__);
        }
    }
    else if (mdioI2c.opState == MDIO_I2C_OP_FINALIZE)
    {
        // Operation is being finalized
        void (*callback)(uint32, uint32);
        mdioI2c.opState = MDIO_I2C_OP_IDLE;

        if (mdio_i2c_fifoEmpty())
        {
            // No more operations to run.  Stop the idle task from running
            TASKSCH_StopTask(mdioI2c.task);
        }

        // The callback is done in the idle task to reduce stack depth
        callback = LEON_unpackPointer(mdioI2c.curOp.header.notifyCompletionHandler);
        if (callback)
        {
            (*callback)(mdioI2c.finalizeArg1, mdioI2c.finalizeArg2);
        }
    }
    else
    {
        mdioI2cAssertInvalidState(__LINE__);
    }
}


/**
* FUNCTION NAME: _GRG_i2cAlmostEmptyIrq()
*
* @brief  - ISR when the HW i2c fifo is almost empty
*
* @return - void
*
* @note   -
*
*/
void _GRG_i2cAlmostEmptyIrq(void)
{
    // We are in an i2c write operation, write more data!
    uint8 * data;

    // Ensure there is something to do, otherwise why did we get the irq
    // Also ensure this is an i2c write
    if (    (mdioI2c.opState != MDIO_I2C_OP_IN_PROGRESS)
        ||  (mdioI2c.curOp.header.bus == MDIO_BUS)
        ||  (mdioI2c.curOp.header.busOperation != MDIO_I2C_WRITE))
    {
        mdioI2cAssertInvalidState(__LINE__);
    }

    data = LEON_unpackPointer(mdioI2c.curOp.i2c.ptrToData);

    // Write the HW fifo
    while ( (mdioI2c.curOp.i2c.bytesProcessed < mdioI2c.curOp.i2c.byteCount)
        &&  !GRG_I2CMDIO_CONTROL_FIFOFULL_READ_BF(GRG_BASE_ADDR))
    {
        GRG_I2CMDIO_WDATA_WRITE_REG(GRG_BASE_ADDR, data[mdioI2c.curOp.i2c.bytesProcessed]);
        mdioI2c.curOp.i2c.bytesProcessed++;
    }

    // Check if the almost emptyIrq should be disabled
    if (mdioI2c.curOp.i2c.bytesProcessed >= mdioI2c.curOp.i2c.byteCount)
    {
        GRG_GRG_INTMSK_I2CFIFOALMOSTEMPTY_WRITE_BF(GRG_BASE_ADDR, 0);
    }

    // Clear IRQ
    GRG_GRG_INTFLG_WRITE_REG(GRG_BASE_ADDR, GRG_GRG_INTFLG_I2CFIFOALMOSTEMPTY_BF_MASK);
}


/**
* FUNCTION NAME: i2cReadHwFifo()
*
* @brief  - Helper function to read all data out of HW i2c fifo
*
* @return - TRUE if all data has been read, FALSE otherwise
*
* @note   -
*
*/
static boolT i2cReadHwFifo(void)
{
    uint8 * data;
    uint8 byteCount = 0; // Compiler thinks it's used uninitialized, even though it isn't
    uint8 bytesProcessed;

    // Find operation parameters
    if (mdioI2c.curOp.header.busOperation == I2C_WRITE_READ)
    {
        data = LEON_unpackPointer(mdioI2c.curOp.i2cWriteRead.ptrToData);
        byteCount = mdioI2c.curOp.i2cWriteRead.byteCountRead;
        bytesProcessed = 0;
    }
    else if (mdioI2c.curOp.header.busOperation == MDIO_I2C_READ)
    {
        data = LEON_unpackPointer(mdioI2c.curOp.i2c.ptrToData);
        byteCount = mdioI2c.curOp.i2c.byteCount;
        bytesProcessed = mdioI2c.curOp.i2c.bytesProcessed;
    }
    else if (mdioI2c.curOp.header.busOperation == I2C_WRITE_READ_BLOCK_IN_PROGRESS)
    {
        data = LEON_unpackPointer(mdioI2c.curOp.i2cWriteReadBlockInProgress.ptrToData);
        bytesProcessed = mdioI2c.curOp.i2cWriteReadBlockInProgress.bytesProcessed;
    }
    else
    {
        mdioI2cAssertInvalidState(__LINE__);
    }


    // Read the HW fifo
    while (!GRG_I2CMDIO_CONTROL_FIFOEMPTY_READ_BF(GRG_BASE_ADDR))
    {
        const uint8 dataByteRead = GRG_I2CMDIO_RDATA_READ_REG(GRG_BASE_ADDR);
        if (mdioI2c.curOp.header.busOperation == I2C_WRITE_READ_BLOCK_IN_PROGRESS)
        {
            const uint32 dataBufSize = mdioI2c.curOp.i2cWriteReadBlockInProgress.dataBufSize;
            if (bytesProcessed < dataBufSize)
            {
                data[bytesProcessed] = dataByteRead;
            }
            byteCount = data[0];
        }
        else
        {
            iassert_GRG_COMPONENT_1(bytesProcessed < byteCount, I2C_READ_TOO_MANY_BYTES, byteCount);
            data[bytesProcessed] = dataByteRead;
        }
        bytesProcessed++;
    }


    // Do operation structure updates
    if (mdioI2c.curOp.header.busOperation == I2C_WRITE_READ)
    {
        // Nothing to update in the operation
    }
    else if (mdioI2c.curOp.header.busOperation == MDIO_I2C_READ)
    {
        mdioI2c.curOp.i2c.bytesProcessed = bytesProcessed;
    }
    else if (mdioI2c.curOp.header.busOperation == I2C_WRITE_READ_BLOCK_IN_PROGRESS)
    {
        mdioI2c.curOp.i2cWriteReadBlockInProgress.bytesProcessed = bytesProcessed;
    }
    else
    {
        mdioI2cAssertInvalidState(__LINE__);
    }

    return (bytesProcessed >= byteCount);
}


/**
* FUNCTION NAME: _GRG_i2cAlmostFullIrq()
*
* @brief  - ISR when the HW i2c fifo is almost full
*
* @return - void
*
* @note   -
*
*/
void _GRG_i2cAlmostFullIrq(void)
{
    // We are in an i2c read operation, read more data!
    uint32 blockSize;
    uint32 bytesProcessed;

    // Ensure there is something to do, otherwise why did we get the irq
    if (    (mdioI2c.opState != MDIO_I2C_OP_IN_PROGRESS)
        ||  (mdioI2c.curOp.header.bus == MDIO_BUS)
        ||  (    (mdioI2c.curOp.header.busOperation != MDIO_I2C_READ)
              && (mdioI2c.curOp.header.busOperation != I2C_WRITE_READ_BLOCK_IN_PROGRESS)))
    {
        mdioI2cAssertInvalidState(__LINE__);
    }

    // read all the data, and ensure we are not done, we should only complete in the done IRQ
    if (i2cReadHwFifo())
    {
        mdioI2cAssertInvalidState(__LINE__);
    }

    if (mdioI2c.curOp.header.busOperation == MDIO_I2C_READ)
    {
        blockSize = mdioI2c.curOp.i2c.byteCount;
        bytesProcessed = mdioI2c.curOp.i2c.bytesProcessed;
    }
    else // (mdioI2c.curOp.header.busOperation == I2C_WRITE_READ_BLOCK_IN_PROGRESS)
    {
        // The first byte read back is the size of the i2c read block
        blockSize = *(uint8 *)LEON_unpackPointer(mdioI2c.curOp.i2cWriteReadBlockInProgress.ptrToData);
        bytesProcessed = mdioI2c.curOp.i2cWriteReadBlockInProgress.bytesProcessed;
    }

    // NOTE: all vars are unsigned, so doing a comparison before possiblity of subtraction resulting in negative
    if ((blockSize > I2C_HW_FIFO_SIZE) && ((blockSize - I2C_HW_FIFO_SIZE) > bytesProcessed))
    {
        // leave the almostEmpty Irq enabled, there is lots more data coming
    }
    else
    {
        // disable the almostFull Irq.  There are I2C_HW_FIFO_SIZE bytes or less remaining
        GRG_GRG_INTMSK_I2CFIFOALMOSTFULL_WRITE_BF(GRG_BASE_ADDR, 0);
    }

    // Clear IRQ
    GRG_GRG_INTFLG_WRITE_REG(GRG_BASE_ADDR, GRG_GRG_INTFLG_I2CFIFOALMOSTFULL_BF_MASK);
}


/**
* FUNCTION NAME: _GRG_mdioI2cDoneIrq()
*
* @brief  - ISR when the HW I2C or MDIO operation has completed
*
* @return - void
*
* @note   - See description at the top of the file
*
*/
void _GRG_mdioI2cDoneIrq(void)
{
    // done
    uint32 controlReg;

    // Clear interrupt
    GRG_GRG_INTFLG_WRITE_REG(GRG_BASE_ADDR, GRG_GRG_INTFLG_I2CMDIODONE_BF_MASK);

    // Ensure there is something to do, otherwise why did we get the irq
    if (mdioI2c.opState != MDIO_I2C_OP_IN_PROGRESS)
    {
        mdioI2cAssertInvalidState(__LINE__);
    }

    controlReg = GRG_I2CMDIO_CONTROL_READ_REG(GRG_BASE_ADDR);
    if (GRG_I2CMDIO_CONTROL_GO_GET_BF(controlReg) != 0)
    {
        mdioI2cAssertInvalidState(__LINE__);
    }

    // Check if this was I2C or MDIO?
    if (mdioI2c.curOp.header.bus == MDIO_BUS)
    {
        // MDIO operation has completed
        const uint32 data = GRG_I2CMDIO_RDATA_READ_REG(GRG_BASE_ADDR);
        ilog_GRG_COMPONENT_1(ILOG_DEBUG, MDIO_FINISH, data);
        if (!GRG_I2CMDIO_CONTROL_FIFOEMPTY_GET_BF(controlReg))
        {
            mdioI2cAssertInvalidState(__LINE__);
        }
        finalizeASyncOperation(data);
    }
    else
    {
        // I2C operation has completed

        // check for errors
        if (GRG_I2CMDIO_CONTROL_TRNERR_GET_BF(controlReg))
        {
#ifdef GRG_I2C_ASSERT_ON_FAILURE
            iassert_GRG_COMPONENT_0(FALSE, I2C_TRN_ERROR);
#else
            uint32 controlRegI2c = GRG_I2CMDIO_CONTROL_I2CMDIOSEL_SET_BF(0, 1);
            uint32 SetClearcontrolReg = GRG_I2CMDIO_CONTROL_CLEAR_SET_BF(controlRegI2c ,1);
            // Indicates an I2C NAK. This is not necessarily an error (TODO: rename to I2C_NAK?)
            ilog_GRG_COMPONENT_0(ILOG_MINOR_EVENT, I2C_TRN_ERROR);

            // clear HW Fifo
            GRG_I2CMDIO_CONTROL_WRITE_REG(GRG_BASE_ADDR, SetClearcontrolReg);   // set clear
            GRG_I2CMDIO_CONTROL_WRITE_REG(GRG_BASE_ADDR, controlRegI2c);        // clear clear
            finalizeASyncOperation(0, 0); // This works for both read and write to signal an error
#endif
        }
        else // No errors
        {
            if (mdioI2c.curOp.header.busOperation == MDIO_I2C_WRITE)
            {
                ilog_GRG_COMPONENT_0(ILOG_DEBUG, I2C_WRITE_FINISH);

                // Validate state of system
                // NOTE: There is a small chance, that the system is overloaded with high priority interrupts
                //       In this case SW may not be able to stop the i2c fifo from underflowing in time
                //       This will cause an assert, and in production the system will reset
                if (mdioI2c.curOp.i2c.bytesProcessed != mdioI2c.curOp.i2c.byteCount)
                {
                    mdioI2cAssertInvalidState(__LINE__);
                }
                if (!GRG_I2CMDIO_CONTROL_FIFOEMPTY_GET_BF(controlReg))
                {
                    mdioI2cAssertInvalidState(__LINE__);
                }

                finalizeASyncOperation(TRUE); // argument is for Success
            }
            else // read operation, or writeRead, or writeReadBlock
            {
                ilog_GRG_COMPONENT_0(ILOG_DEBUG, I2C_READ_FINISH);
                if (GRG_I2CMDIO_CONTROL_FIFOEMPTY_GET_BF(controlReg))
                {
                    mdioI2cAssertInvalidState(__LINE__);
                }

                // Read all data, and ensure it was all read
                if (!i2cReadHwFifo())
                {
                    mdioI2cAssertInvalidState(__LINE__);
                }

                if (mdioI2c.curOp.header.busOperation == I2C_WRITE_READ)
                {
                    finalizeASyncOperation(LEON_unpackPointer(mdioI2c.curOp.i2cWriteRead.ptrToData), mdioI2c.curOp.i2cWriteRead.byteCountRead);
                }
                else if (mdioI2c.curOp.header.busOperation == MDIO_I2C_READ)
                {
                    finalizeASyncOperation(LEON_unpackPointer(mdioI2c.curOp.i2c.ptrToData), mdioI2c.curOp.i2c.bytesProcessed);
                }
                else if (mdioI2c.curOp.header.busOperation == I2C_WRITE_READ_BLOCK_IN_PROGRESS)
                {
                    finalizeASyncOperation(
                        LEON_unpackPointer(mdioI2c.curOp.i2cWriteReadBlockInProgress.ptrToData),
                        mdioI2c.curOp.i2cWriteReadBlockInProgress.dataBufSize);
                }
                else
                {
                    mdioI2cAssertInvalidState(__LINE__);
                }
            }
        }
    }
}


/**
* FUNCTION NAME: finalizeASyncOperation()
*
* @brief  - Completion handler on all mdio/i2c operations
*
* @return - void
*
* @note   - See description at the top of the file
*
*/
static void finalizeASyncOperation
(
    uint32 arg1,    // Args to pass to completion handler
    uint32 arg2     // Format varies depending on transaction type: i2c/mdio/read/write/etc
)
{
#ifndef GRG_I2C_ASSERT_ON_FAILURE
    uint32 irqFlag;
#endif
    uint32 irqMask;

    // ensure interrupts for i2c almost full/empty are disabled
    irqMask = GRG_GRG_INTMSK_READ_REG(GRG_BASE_ADDR);
    irqMask = GRG_GRG_INTMSK_I2CFIFOALMOSTFULL_SET_BF(irqMask, 0);
    irqMask = GRG_GRG_INTMSK_I2CFIFOALMOSTEMPTY_SET_BF(irqMask, 0);
    GRG_GRG_INTMSK_WRITE_REG(GRG_BASE_ADDR, irqMask);
#ifndef GRG_I2C_ASSERT_ON_FAILURE
    // ensure interrupts for i2c are not currently flagged (can happen on a failed transaction)
    irqFlag = 0;
    irqFlag = GRG_GRG_INTFLG_I2CFIFOALMOSTEMPTY_SET_BF(irqFlag, 1);
    irqFlag = GRG_GRG_INTFLG_I2CFIFOALMOSTFULL_SET_BF(irqFlag, 1);
    GRG_GRG_INTFLG_WRITE_REG(GRG_BASE_ADDR, irqFlag);
#endif

    // Mark the operation as complete
    mdioI2c.opState = MDIO_I2C_OP_FINALIZE;
    mdioI2c.finalizeArg1 = arg1;
    mdioI2c.finalizeArg2 = arg2;

    // The callback is called from the idle task to avoid overflowing the
    // stack.
    TASKSCH_StartTask(mdioI2c.task);

    mdioI2cMarkTime(TIME_MARKER_FINALIZE_OP);
}


/**
* FUNCTION NAME: submitASyncOperation()
*
* @brief  - Submit an operation to the ififo of outstanding operations
*
* @return - void
*
* @note   - See description at the top of the file
*
*/
static void submitASyncOperation
(
    uint64 op,              // actual type is union mdio_i2c_operation
                            // This is the operation to be submitted

    void * notifyHandler    // completion handler
)
{
    union mdio_i2c_operation newOp;
    newOp.raw = op;
    newOp.header.notifyCompletionHandler = LEON_packPointer(notifyHandler);

    // Add operation to our work queue
    iassert_GRG_COMPONENT_0(!mdio_i2c_fifoFull(), MDIO_I2C_FIFO_OVER_FLOW);
    mdio_i2c_fifoWrite(newOp.raw);

    // If this system is idle, start this operation,
    // otherwise we need to wait until the current operation(s) complete
    if (mdioI2c.opState == MDIO_I2C_OP_IDLE)
    {
        TASKSCH_StartTask(mdioI2c.task);
    }
    mdioI2cMarkTime(TIME_MARKER_SUBMIT_OP);
}


/**
* FUNCTION NAME: GRG_MdioWriteASync()
*
* @brief  - Write to an MDIO device
*
* @return - void
*
* @note   - This is part of the public API of this module
*
*/
void GRG_MdioWriteASync
(
    uint8 device,                               // MDIO device address
    uint8 address,                              // MDIO register address
    uint16 data,                                // Data to write
    void (*notifyWriteCompleteHandler)(void)    // completion handler
)
{
    // Create new operation
    union mdio_i2c_operation newOp;
    newOp.raw = 0;
    newOp.header.bus = MDIO_BUS;
    newOp.header.busOperation = MDIO_I2C_WRITE;
    newOp.header.deviceAddress = device;
    newOp.mdio.regAddress = address;
    newOp.mdio.writeData = data;

    // Go
    submitASyncOperation(newOp.raw, notifyWriteCompleteHandler);
}


/**
* FUNCTION NAME: GRG_MdioReadASync()
*
* @brief  - Read from an MDIO device
*
* @return - void
*
* @note   - This is part of the public API of this module
*
*/
void GRG_MdioReadASync
(
    uint8 device,                                   // MDIO device address
    uint8 address,                                  // MDIO register address
    void (*notifyReadCompleteHandler)(uint16 data)  // completion handler
)
{
    // Create new operation
    union mdio_i2c_operation newOp;
    newOp.raw = 0;
    newOp.header.bus = MDIO_BUS;
    newOp.header.busOperation = MDIO_I2C_READ;
    newOp.header.deviceAddress = device;
    newOp.mdio.regAddress = address;

    // Go
    submitASyncOperation(newOp.raw, notifyReadCompleteHandler);
}


/**
* FUNCTION NAME: GRG_I2cWriteASync()
*
* @brief  - Write to an i2c device
*
* @return - void
*
* @note   - This is part of the public API of this module
*
*/
void GRG_I2cWriteASync
(
    uint8 bus,
    uint8 device,
    enum GRG_I2cSpeed speed,
    uint8 * data,
    uint8 byteCount,
    void (*notifyWriteCompleteHandler)(boolT success)
)
{
    // Create new operation
    union mdio_i2c_operation newOp;
    newOp.raw = 0;
    newOp.header.bus = ((bus == 0) ? I2C_BUS_0 : I2C_BUS_1);
    newOp.header.busOperation = MDIO_I2C_WRITE;
    newOp.header.deviceAddress = device;
    newOp.header.i2cSpeed_i2cWakeState = speed; //TODO: assert this is valid
    newOp.i2c.byteCount = byteCount; //TODO: assert this is valid
    newOp.i2c.ptrToData = LEON_packPointer(data);

    // Go
    submitASyncOperation(newOp.raw, notifyWriteCompleteHandler);
}


/**
* FUNCTION NAME: GRG_I2cReadASync()
*
* @brief  - Read from an i2c device
*
* @return - void
*
* @note   - This is part of the public API of this module
*
*/
void GRG_I2cReadASync
(
    uint8 bus, uint8 device,                            // I2C chip
    enum GRG_I2cSpeed speed,                            // Speed
    uint8 * data, uint8 byteCount,                      // Read data
    void (*notifyReadCompleteHandler)(uint8 * data, uint8 byteCount)
        // On failure, data* will be NULL, and byteCount will be 0
        // Note: byteCount may be less than requested
        //       as the device can end the transfer early
)
{
    // Create new operation
    union mdio_i2c_operation newOp;
    newOp.raw = 0;
    newOp.header.bus = ((bus == 0) ? I2C_BUS_0 : I2C_BUS_1);
    newOp.header.busOperation = MDIO_I2C_READ;
    newOp.header.deviceAddress = device;
    newOp.header.i2cSpeed_i2cWakeState = speed; //TODO: assert this is valid
    newOp.i2c.byteCount = byteCount; //TODO: assert this is valid
    newOp.i2c.ptrToData = LEON_packPointer(data);

    // Go
    submitASyncOperation(newOp.raw, notifyReadCompleteHandler);
}


/**
* FUNCTION NAME: GRG_I2cWriteReadASync()
*
* @brief  - Start an i2c transaction that is a START-WRITE-START-READ-STOP
*
* @return - void
*
* @note   - For SMB read, where first a internal register is written to the chip, then a read request is made
*
*/
void GRG_I2cWriteReadASync
(
    uint8 bus, uint8 device,                            // I2C chip
    enum GRG_I2cSpeed speed,                            // Speed
    uint8 * data, uint8 writeByteCount, uint8 readByteCount, // Uses *data as buffer to write out, then re-uses as read buffer
    void (*notifyReadCompleteHandler)(uint8 * data, uint8 byteCount)
        // On failure, data* will be NULL, and byteCount will be 0
        // Note: byteCount may be less than requested
        //       as the device can end the transfer early
)
{
   // Create new operation
    union mdio_i2c_operation newOp;
    newOp.raw = 0;
    newOp.header.bus = ((bus == 0) ? I2C_BUS_0 : I2C_BUS_1);
    newOp.header.busOperation = I2C_WRITE_READ;
    newOp.header.deviceAddress = device;
    newOp.header.i2cSpeed_i2cWakeState = speed; //TODO: assert this is valid
    newOp.i2cWriteRead.byteCountRead = readByteCount; //TODO: assert this is valid
    newOp.i2cWriteRead.byteCountWrite = writeByteCount; //TODO: assert this is valid
    newOp.i2cWriteRead.ptrToData = LEON_packPointer(data);

    // Go
    submitASyncOperation(newOp.raw, notifyReadCompleteHandler);
}


/**
* FUNCTION NAME: GRG_I2cWriteReadBlockASync()
*
* @brief  - Start an i2c transaction that is a START-WRITE-START-READ-STOP & read size is the first byte read
*
* @return - void
*
* @note   - For SMB read, where first a internal register is written to the chip, then a read request is made
*
*           The read is of a smb block format, where the 1st byte is the number of bytes to be read, followed by the data bytes
*/
void GRG_I2cWriteReadBlockASync
(
    uint8 bus, uint8 device,                            // I2C chip
    enum GRG_I2cSpeed speed,                            // Speed
    uint8 * data,                                       // Uses *data as buffer to write out, then re-uses as read buffer
    uint8 writeByteCount,                               // number of bytes to write (for smb this should be 1)
    uint8 readByteCount,                                // Size of data buffer.  Actual count is from i2c block read
    void (*notifyReadCompleteHandler)(uint8 * data, uint8 byteCount)
        // On failure, data* will be NULL, and byteCount will be 0
        // Note: byteCount may be less than requested
        //       as the device can end the transfer early
)
{
    // Create new operation
    union mdio_i2c_operation newOp;
    newOp.raw = 0;
    newOp.header.bus = ((bus == 0) ? I2C_BUS_0 : I2C_BUS_1);
    newOp.header.busOperation = I2C_WRITE_READ_BLOCK;
    newOp.header.deviceAddress = device;
    newOp.header.i2cSpeed_i2cWakeState = speed; //TODO: assert this is valid
    newOp.i2cWriteReadBlock.dataBufSize = readByteCount; //TODO: assert this is valid
    newOp.i2cWriteReadBlock.byteCountWrite = writeByteCount; //TODO: assert this is valid
    newOp.i2cWriteReadBlock.ptrToData = LEON_packPointer(data);

    // Go
    submitASyncOperation(newOp.raw, notifyReadCompleteHandler);
}


/**
* FUNCTION NAME: GRG_I2cWake()
*
* @brief  - Perform an i2c wake operation
*
* @return - void
*
* @note   - This is part of the public API of this module
*
*           i2c wake is describe in the Atmel ATSH204 documentation
*/
void GRG_I2cWake
(
    uint8 bus,                              // i2c bus to perform wake on
    void (*notifyWakeCompleteHandler)(void) // completion handler
)
{
    // Create new operation
    union mdio_i2c_operation newOp;
    newOp.raw = 0;
    newOp.header.bus = ((bus == 0) ? I2C_BUS_0 : I2C_BUS_1);
    newOp.header.busOperation = I2C_WAKE;

    // Go
    submitASyncOperation(newOp.raw, notifyWakeCompleteHandler);
}

/**
* FUNCTION NAME: GRG_MdioWriteSync()
*
* @brief  - Synchronously write a value to the requested MDIO attached device's register
*
* @return - void
*
* @note   -
*
*/
void GRG_MdioWriteSync
(
    uint8 device,  // The address of the MDIO connected device to write to
    uint8 address, // The address of the register on the device to write to
    uint16 data    // The data to write
)
{
    // We require the I2C job FIFO to be empty for sync writes so we don't
    // clobber other pending operations
    iassert_GRG_COMPONENT_0(mdio_i2c_fifoEmpty(), SYNC_MDIO_WRITE_FIFO_NOT_EMPTY);

    uint32 controlReg = 0;

    // Select the device
    controlReg = GRG_I2CMDIO_CONTROL_PHYADDR_SET_BF(controlReg, device);

    // Write the register address
    controlReg = GRG_I2CMDIO_CONTROL_RDBYTESREGADDR_SET_BF(controlReg, address);

    // Set access mode to write
    controlReg = GRG_I2CMDIO_CONTROL_ACCESSMODE_SET_BF(controlReg, 1);

    // Ensure the control register is in i2c mode, before writing to WRITE_REG
    GRG_I2CMDIO_CONTROL_WRITE_REG(GRG_BASE_ADDR, controlReg);
    GRG_I2CMDIO_WDATA_WRITE_REG(GRG_BASE_ADDR, data);

    // Kick off the transaction
    controlReg = GRG_I2CMDIO_CONTROL_GO_SET_BF(controlReg, 1);
    GRG_I2CMDIO_CONTROL_WRITE_REG(GRG_BASE_ADDR, controlReg);

    // Spin while we wait for the write to complete
    do {
        controlReg = GRG_I2CMDIO_CONTROL_READ_REG(GRG_BASE_ADDR);
    } while (GRG_I2CMDIO_CONTROL_GO_GET_BF(controlReg) != 0);

    // Clear the interrupt flag that was set when the write completed
    GRG_GRG_INTFLG_WRITE_REG(GRG_BASE_ADDR, GRG_GRG_INTFLG_I2CMDIODONE_BF_MASK);
}

/**
* FUNCTION NAME: GRG_MdioReadSync()
*
* @brief  - Synchonously read a value from the requested MDIO attached device's register
*
* @return - the data read from the MDIO device
*
* @note   -
*
*/
uint16 GRG_MdioReadSync
(
    uint8 device, // Address of MDIO attached device
    uint8 address // Register address to read from
)
{
    // We require the I2C job FIFO to be empty for sync writes so we don't
    // clobber other pending operations
    iassert_GRG_COMPONENT_0(mdio_i2c_fifoEmpty(), SYNC_MDIO_READ_FIFO_NOT_EMPTY);

    uint32 controlReg = 0;

    // Select the device
    controlReg = GRG_I2CMDIO_CONTROL_PHYADDR_SET_BF(controlReg, device);

    // Write the register address
    controlReg = GRG_I2CMDIO_CONTROL_RDBYTESREGADDR_SET_BF(controlReg, address);

    // Set access mode to read
    controlReg = GRG_I2CMDIO_CONTROL_ACCESSMODE_SET_BF(controlReg, 2);

    // Kick off the transaction
    controlReg = GRG_I2CMDIO_CONTROL_GO_SET_BF(controlReg, 1);
    GRG_I2CMDIO_CONTROL_WRITE_REG(GRG_BASE_ADDR, controlReg);

    // Spin while we wait for the read to complete
    do {
        controlReg = GRG_I2CMDIO_CONTROL_READ_REG(GRG_BASE_ADDR);
    } while (GRG_I2CMDIO_CONTROL_GO_GET_BF(controlReg) != 0);

    // Read out the data
    const uint32 data = GRG_I2CMDIO_RDATA_READ_REG(GRG_BASE_ADDR);

    // Clear the interrupt flag that was set when the read completed
    GRG_GRG_INTFLG_WRITE_REG(GRG_BASE_ADDR, GRG_GRG_INTFLG_I2CMDIODONE_BF_MASK);

    return data;
}

