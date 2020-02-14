//#################################################################################################
// Icron Technology Corporation - Copyright 2019
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
//
//!   @file  -  i2c.c
//
//!   @brief -  Driver for I2C
//
//!   @note  -  The I2C HW share a register interface, so there can only
//              be 1 transaction at a time, regardless of type or bus.
//
//              This driver works with only asynchronous requests, and queues
//              them up in an ififo.  All code runs with interrupts disabled,
//              and should operate very quickly.
//
//              The ififo queues up all requests.  When the system is ready to
//              process a request, it starts the idle task, which will pops the
//              first request off the queue, and operates on it.  Then it marks
//              the global i2c.opInProgress, as active to prevent a new
//              operation from starting
//
//              Normal call flow:
//              * An I2C public API function is called
//              * The public API function creates a union i2c_operation,
//                and calls the generic submitAsyncOperation()
//              * submitAsyncOperation() does the following
//                  * places the operation on the ififo queue
//                  * checks if the system is idle, if so start the idle task
//                    otherwise the operation will be started when HW is free
//              * i2cIdleTask() will
//      `           1) pops the request off the ififo
//                  2) marks i2c.opInProgress as true
//                  2) start i2c operations, then stop the idle task
//                  3) run i2c wake ops, then call finalizeAsyncOperation()
//              * i2c operations may enable almost full/empty irq to read/write
//                the hw buffer, then disable and wait for the completion irq
//              * I2C done irq, calls finalizeAsyncOperation()
//              * finalizeAsyncOperation()
//                  * marks i2c.opInProgress as false
//                  * calls the completion handler
//                  * checks if another tasks is queued up, and starts/stops
//                    the idle task as needed
//
//              I2C operations are only for the Spartan platform
//
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <leon_timers.h>
#include <callback.h>
#include <timing_timers.h>
#include <ififo.h>
#include <i2c_master_regs.h>
#include <bb_core.h>
#include <bb_top.h>
#include <bb_top_a7.h>
#include <module_addresses_regs.h>
#include <interrupts.h>
#include <i2c.h>

#include "i2c_loc.h"
#include "i2c_log.h"

#include <uart.h>       // For debugging

// Constants and Macros ###########################################################################
#define I2CD_SWITCH_DEVICE_ADDRESS      (0x74)      // (0x70 | (a2Set<<2) | (a1Set<<1) | a0Set)
#define I2C_OPERATIONS_FIFO_SIZE        5

#define I2C_WAKE_TIME_DRIVE_SDA_LOW     (60)        // in microseconds
#define I2C_WAKE_TIME_IDLE              (3)         // in milliseconds!


// Data Types #####################################################################################
enum i2cBusOperation    // Used to define i2c operation and i2c_master.control.opmode setting value
{
    I2C_READ                = 0,
    I2C_WRITE               = 1,
    I2C_WRITE_READ          = 2,        // First write device register address, then read it.
    I2C_WRITE_READ_BLOCK    = 3,        // Do a write, followed by a read. The read size depends on the value of the first byte read.
    I2C_WAKE                = 4,        // Perform a long pulse on the SDA line, to wake up sleeping devices
};

enum i2cWakeState
{
    I2C_WAKE_DRIVE_NONE,                // no wake state is active
    I2C_WAKE_DRIVE_SDA_LOW_STATE,       // drive SDA low for 60 microseconds
    I2C_WAKE_IDLE_TIME_STATE            // After a device has being woken up, there needs to be 2.5 ms of idle time
};

enum i2cWakeOperation
{
    I2C_WAKE_STOP = 0,
    I2C_WAKE_START = 1
};

struct I2cTask                          // struct when i2c module stores a request to fifo
{
    const struct I2cDevice *device;     // I2C device information
    uint8_t *ptrToData;                 // read/write data pointer - shouldn't be in stack
    void   *notifyCompletionHandler;

    enum i2cBusOperation operation;     // What this task is doing
    uint8_t byteCountWrite;             // Number of bytes to write
    uint8_t byteCountRead;              // Number of bytes to read
};


// Global Variables ###############################################################################

// Static Variables ###############################################################################
const struct I2cDevice devicePc9548 =           // I2C Mux switch device info
{
    .deviceAddress = I2CD_SWITCH_DEVICE_ADDRESS,
    .speed = I2C_SPEED_SLOW,
    .port = I2C_MUX_CORE
};

struct {
    struct I2cTask curOp;
    LEON_TimerValueT i2cWakeTimer;              // to check wakeup sda low 60us time
    TIMING_TimerHandlerT idleWakeTimer;         // to ensure I2C_WAKE_TIME_IDLE after finishied wakeup
#ifdef I2C_USE_TIME_MARKERS
    LEON_TimerValueT lastTimeMarker;        // For debugging log time stamps
#endif
    bool opInProgress;
    enum i2cWakeState wakeState;
    uint8_t writeBytesProcessed;                // Number of writing bytes processed
    uint8_t readBytesProcessed;                 // Number of reading bytes processed
    enum I2cMuxPort currentPort;                // I2C port currently using
    uint8_t muxPortBuffer;                      // I2C switch communication buffer
} i2cContext;

volatile i2c_master_s* i2c_registers;

uint16_t I2cSpeedPrescaler[NUM_OF_I2C_SPEEDS];
uint8_t i2c_hw_fifo_size;
static void (*i2c_blockCallback)(void);         // callback which runs in blocking while loop

IFIFO_CREATE_FIFO_LOCK_UNSAFE(i2c, struct I2cTask, I2C_OPERATIONS_FIFO_SIZE)

// Static Function Declarations ###################################################################
static void i2cIdleTask(void *param1, void *param2);
static void i2cWakeDelay(void *param1, void *param2);
static void i2cIdleDelay( void );
static void i2cSetPortDone(bool success);
static void i2cStart(void);
static void i2cStartInterface(void);
static void i2cWakeOp(enum i2cWakeOperation);
static void submitAsyncOperation(const struct I2cTask *newOp);
static void finalizeAsyncOperation(uint32_t arg1, uint32_t arg2);
static void i2cMarkTime(I2C_COMPONENT_ilogCodeT);
static bool i2cReadHwFifo(void);
static uint32_t i2cFreqToPrescalerConv(uint32_t freq)               __attribute__((section(".atext")));
static void i2cConfigBlocking(
    const struct I2cDevice *device,
    enum i2cBusOperation op,
    uint8_t readByteCount)                                          __attribute__((section(".atext")));
static void I2C_InitPort(void);
static void I2C_SetPortBlock(enum I2cMuxPort port);
static bool I2C_SetPortAsync(enum I2cMuxPort port, void (*selectDone)(bool));
static void i2cStoreSpeedFreq(uint32_t idx, uint32_t freq);

static void I2C_pC9548SelectPorts(uint8_t portSettings, void (*selectDone)(bool success));
static void I2C_pC9548SelectPortsBlocking(uint8_t portSettings);

// Exported Function Definitions ##################################################################
//#################################################################################################
// I2C_init: Initialization function
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2C_init(void (*callback)(void))
{
    i2c_registers = (volatile i2c_master_s*) bb_chip_i2c_master_s_ADDRESS;
    i2c_blockCallback = callback;

    bb_top_ResetI2CMaster(false);      // FPGA I2C block out of reset
    bb_top_setupI2c();

#ifdef I2C_USE_TIME_MARKERS
    i2cContext.lastTimeMarker = LEON_TimerRead();
#endif

    // Setup the prescaler conversion array
    i2cStoreSpeedFreq(I2C_SPEED_SLOW, I2C_SPEED_100KHZ);
    i2cStoreSpeedFreq(I2C_SPEED_FAST, I2C_SPEED_400KHZ);
    i2cStoreSpeedFreq(I2C_SPEED_FAST_PLUS, I2C_SPEED_1MHZ);

    // Set default current port. This is because program bb can't initialize bb_top and switch IC
    I2C_InitPort();

    // Initialize I2C fifo
    i2c_registers->fifo.bf.fifo_aft = I2C_FIFO_AFT;
    i2c_registers->fifo.bf.fifo_aet = I2C_FIFO_AET;
    i2c_hw_fifo_size = i2c_registers->fifo.bf.fifo_depth;

    i2cContext.idleWakeTimer = TIMING_TimerRegisterHandler(i2cIdleDelay, false, I2C_WAKE_TIME_IDLE);

    // Clear pending interrupts
    i2c_registers->irq.s.pending.dw = i2c_master_irq_pending_WRITEMASK;

    // Clear clear bit (bit5 of I2C control, The clear bit must be asserted for at least 2 PCLK cycles)
    i2c_registers->control.bf.clear = 0;

    // Enable the done interrupt
    i2c_registers->irq.s.enable.bf.done = 1;

    TOPLEVEL_setPollingMask(SECONDARY_INT_I2C_INT_MSK);
}

//#################################################################################################
// I2C_WriteAsync: Write to an i2c device
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2C_WriteAsync( const struct I2cDevice *device,    // I2C device info
                    uint8_t *prtToData,                 // Write data/ buffer
                    uint8_t byteCountWrite,             // Number of /bytes to write
                    void (*notifyWriteCompleteHandler)(bool success))
{
    const struct I2cTask newOp =
    {
        .device = device,
        .ptrToData = prtToData,
        .notifyCompletionHandler = notifyWriteCompleteHandler,
        .operation = I2C_WRITE,
        .byteCountWrite = byteCountWrite,
        .byteCountRead = 0
    };

    submitAsyncOperation(&newOp);
}

//#################################################################################################
// I2C_ReadAsync: Read from an i2c device
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2C_ReadAsync(  const struct I2cDevice *device,     // I2C device info
                    uint8_t *prtToData,                 // Read data/ buffer
                    uint8_t byteCountRead,              // Number of /bytes to read
                    void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount))
{
    const struct I2cTask newOp =
    {
        .device = device,
        .ptrToData = prtToData,
        .notifyCompletionHandler = notifyReadCompleteHandler,
        .operation = I2C_READ,
        .byteCountWrite = 0,
        .byteCountRead = byteCountRead
    };

    submitAsyncOperation(&newOp);
}

//#################################################################################################
// I2C_WriteReadAsync: Start an i2c transaction that is a START-WRITE-START-READ-STOP
//
// Parameters:
// Return:
// Assumptions: After write message has gone (HW fifo is empty), RTL starts read it
//#################################################################################################
void I2C_WriteReadAsync( const struct I2cDevice *device, // I2C device info
                        uint8_t *prtToData,             // Write/Read uses the same buffer
                        uint8_t byteCountWrite,         // Number of /bytes to write
                        uint8_t byteCountRead,          // Number of /bytes to read
                        void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount))
{
    const struct I2cTask newOp =
    {
        .device = device,
        .ptrToData = prtToData,
        .notifyCompletionHandler = notifyReadCompleteHandler,
        .operation = I2C_WRITE_READ,
        .byteCountWrite = byteCountWrite,
        .byteCountRead = byteCountRead
    };

    submitAsyncOperation(&newOp);
}

//#################################################################################################
// I2C_WriteReadBlockAsync: Start an i2c transaction that is a START-WRITE-START-READ-STOP
//                         read size is the first byte read
//
// Parameters:
// Return:
// Assumptions: After write message has gone (HW fifo is empty), RTL starts read it
//              When RTL reads the first byte, it stores the value into read_byte_count
//              byteCountRead represents read buffer size, not the number of bytes to read
//#################################################################################################
void I2C_WriteReadBlockAsync( const struct I2cDevice *device,// I2C device info
                            uint8_t *prtToData,             // Write/Read uses the same buffer
                            uint8_t byteCountWrite,         // Number of /bytes to write
                            uint8_t byteCountRead,          // Maximum buffer size of read byte
                            void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount))
{
    const struct I2cTask newOp =
    {
        .device = device,
        .ptrToData = prtToData,
        .notifyCompletionHandler = notifyReadCompleteHandler,
        .operation = I2C_WRITE_READ_BLOCK,
        .byteCountWrite = byteCountWrite,
        .byteCountRead = byteCountRead
    };

    submitAsyncOperation(&newOp);
}

//#################################################################################################
// I2C_Wake: Perform an i2c wake operation
//
// Parameters:
// Return:
// Assumptions: This is part of the public API of this module.
//              I2c wake is describe in the Atmel ATSH204 documentation
//#################################################################################################
void I2C_Wake( const struct I2cDevice *device, void (*notifyWakeCompleteHandler)(void))
{
    const struct I2cTask newOp =
    {
        .device = device,
        .ptrToData = NULL,
        .notifyCompletionHandler = notifyWakeCompleteHandler,
        .operation = I2C_WAKE,
        .byteCountWrite = 0,
        .byteCountRead = 0
    };

    submitAsyncOperation(&newOp);
}

//#################################################################################################
// Perform blocking write
//
// Parameters:  device - I2C device info
//              data - buffer of data to write
//              byteCountWrite - number of bytes to write
// Return:
//              true if success
// Assumptions:
//#################################################################################################
bool I2C_WriteBlocking( const struct I2cDevice *device,
                        uint8_t *data,
                        uint8_t byteCountWrite)
{
    // If this system is idle, start this operation,
    // otherwise we need to wait until the current operation(s) complete
    if (i2cContext.opInProgress)
    {
        ilog_I2C_COMPONENT_3(ILOG_MAJOR_EVENT, I2C_BLOCKING_OP_IN_PROGRESS,
                                (uint32_t)device, (uint32_t)i2cContext.curOp.device, __LINE__);
        return false;
    }

    I2C_SetPortBlock(device->port);

    // disable IRQ's
    uint32_t intMsk = i2c_registers->irq.s.enable.dw;
    i2c_registers->irq.s.enable.dw = 0;
    i2c_registers->irq.s.pending.dw = (~0);

    // for writes we fill fifo first then configure and issue go then keep filling until done
    uint8_t bytesProcessed = 0;
    bool configured = false;

    do
    {
        // Write the HW fifo until full or until all byteCount
        while (bytesProcessed < byteCountWrite &&
               i2c_registers->fifo.bf.fifo_full == 0)
        {
            i2c_registers->wr_data.bf.wr_data = data[bytesProcessed];
            bytesProcessed++;
        }
        // keep this loop tight so we don't let the fifo to empty
        // if it does, a stop condition will be sent which we don't want
        // ONLY configure ONCE
        if (!configured)
        {
            i2cConfigBlocking(device, I2C_WRITE, 0);
            configured = true;
        }
    } while (bytesProcessed < byteCountWrite);

    LEON_TimerValueT timeoutStart = LEON_TimerRead();
    while (i2c_registers->control.bf.go != 0)
    {
        iassert_I2C_COMPONENT_1 (
            (500000 > LEON_TimerCalcUsecDiff(timeoutStart, LEON_TimerRead())),
            I2C_WRITE_GO_TIMEOUT, i2c_registers->control.dw);

        if(i2c_blockCallback != NULL)
        {
            i2c_blockCallback();
        }
    }
    i2c_registers->irq.s.pending.dw = i2c_master_irq_enable_RESETMASK;
    i2c_registers->irq.s.enable.dw = intMsk;

    if (i2c_registers->control.bf.error)
    {
        ilog_I2C_COMPONENT_1(ILOG_MAJOR_EVENT, I2C_BLOCKING_ERROR, __LINE__);
        i2c_registers->control.bf.clear = 1;
        i2c_registers->control.bf.clear = 0;
        return false;
    }
    else
    {
        return true;
    }
}

//#################################################################################################
// Perform blocking read
//
// Parameters:  device - I2C device info
//              data - buffer of data to store read values
//              byteCountRead - number of bytes to read
// Return:
//              true if success
// Assumptions:
//#################################################################################################
bool I2C_ReadBlocking(  const struct I2cDevice *device,
                        uint8_t *data,
                        uint8_t byteCountRead)
{
    // If this system is idle, start this operation,
    // otherwise we need to wait until the current operation(s) complete
    if (i2cContext.opInProgress)
    {
        ilog_I2C_COMPONENT_3(ILOG_MAJOR_EVENT, I2C_BLOCKING_OP_IN_PROGRESS,
                                (uint32_t)device, (uint32_t)i2cContext.curOp.device, __LINE__);
        return false;
    }

    I2C_SetPortBlock(device->port);

    // disable IRQ's
    uint32_t intMsk = i2c_registers->irq.s.enable.dw;
    i2c_registers->irq.s.enable.dw = 0;
    i2c_registers->irq.s.pending.dw = (~0);

    i2cConfigBlocking(device, I2C_READ, byteCountRead);

    uint8_t bytesProcessed = 0;
    LEON_TimerValueT timeoutStart = LEON_TimerRead();

    do
    {
        // wait until fifo almost full or done bit completed
        while ((i2c_registers->fifo.bf.fifo_af == 0) && (i2c_registers->control.bf.go == 1))
        {
            iassert_I2C_COMPONENT_0 (
                (500000 > LEON_TimerCalcUsecDiff(timeoutStart, LEON_TimerRead())),
                I2C_READ_GO_TIMEOUT);

            if(i2c_blockCallback != NULL)
            {
                i2c_blockCallback();
            }
        }
        while (i2c_registers->fifo.bf.fifo_empty == 0)
        {
            const uint8_t dataByteRead = i2c_registers->rd_data.bf.rd_data;
            iassert_I2C_COMPONENT_1(bytesProcessed < byteCountRead, I2C_READ_TOO_MANY_BYTES, byteCountRead);
            data[bytesProcessed] = dataByteRead;
            bytesProcessed++;
        }
    } while (bytesProcessed < byteCountRead);

    i2c_registers->irq.s.pending.dw = i2c_master_irq_pending_RESETMASK;
    i2c_registers->irq.s.enable.dw = intMsk;

    if (i2c_registers->control.bf.error)
    {
        ilog_I2C_COMPONENT_1(ILOG_MAJOR_EVENT, I2C_BLOCKING_ERROR, __LINE__);
        i2c_registers->control.bf.clear = 1;
        i2c_registers->control.bf.clear = 0;
        return false;
    }
    else
    {
        return true;
    }
}

//#################################################################################################
// Perform blocking write-read
//
// Parameters:  device - I2C device info
//              data - buffer of data to write/read
//              byteCountWrite - number of bytes to write
//              byteCountRead - number of bytes to read
// Return:
//              true if success
// Assumptions:
//#################################################################################################
bool I2C_WriteReadBlocking( const struct I2cDevice *device,
                            uint8_t *data,
                            uint8_t byteCountWrite,
                            uint8_t byteCountRead)
{
    // If this system is idle, start this operation,
    // otherwise we need to wait until the current operation(s) complete
    if (i2cContext.opInProgress)
    {
        ilog_I2C_COMPONENT_3(ILOG_MAJOR_EVENT, I2C_BLOCKING_OP_IN_PROGRESS,
                                (uint32_t)device, (uint32_t)i2cContext.curOp.device, __LINE__);
        return false;
    }

    I2C_SetPortBlock(device->port);

    // disable IRQ's
    uint32_t intMsk = i2c_registers->irq.s.enable.dw;
    i2c_registers->irq.s.enable.dw = 0;
    i2c_registers->irq.s.pending.dw = (~0);

    uint8_t bytesProcessed = 0;
    // fill in write fifo first
    while (bytesProcessed < byteCountWrite &&
           i2c_registers->fifo.bf.fifo_full == 0)
    {
        i2c_registers->wr_data.bf.wr_data = data[bytesProcessed];
        bytesProcessed++;
    }

    // configure and go
    i2cConfigBlocking(device, I2C_WRITE_READ, byteCountRead);

    // poll like read
    bytesProcessed = 0;
    LEON_TimerValueT timeoutStart = LEON_TimerRead();

    do
    {
        // wait until fifo almost full or done bit completed
        // TODO: Time out - assert
        while ((i2c_registers->fifo.bf.fifo_af == 0) && (i2c_registers->control.bf.go == 1))
        {
            iassert_I2C_COMPONENT_0 (
                (500000 > LEON_TimerCalcUsecDiff(timeoutStart, LEON_TimerRead())),
                I2C_WRITE_READ_GO_TIMEOUT);

            if(i2c_blockCallback != NULL)
            {
                i2c_blockCallback();
            }
        }
        while (i2c_registers->fifo.bf.fifo_empty == 0)
        {
            const uint8_t dataByteRead = i2c_registers->rd_data.bf.rd_data;
            iassert_I2C_COMPONENT_2(bytesProcessed < byteCountRead, I2C_READ_TOO_MANY_BYTES, __LINE__, byteCountRead);
            data[bytesProcessed] = dataByteRead;
            bytesProcessed++;
        }
    } while (bytesProcessed < byteCountRead);

    i2c_registers->irq.s.pending.dw = i2c_master_irq_pending_RESETMASK;
    i2c_registers->irq.s.enable.dw = intMsk;

    if (i2c_registers->control.bf.error)
    {
        ilog_I2C_COMPONENT_1(ILOG_MAJOR_EVENT, I2C_BLOCKING_ERROR, __LINE__);
        i2c_registers->control.bf.clear = 1;
        i2c_registers->control.bf.clear = 0;
        return false;
    }
    else
    {
        return true;
    }
}

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// I2C_AlmostEmptyIrq: ISR when the HW i2c fifo is almost empty
//
// Parameters:
// Return:
// Assumptions: We are in an i2c write operation, write more data!
//#################################################################################################
void I2C_AlmostEmptyIrq(void)
{
    iassert_I2C_COMPONENT_3(
        i2cContext.opInProgress &&
        ((i2cContext.curOp.operation == I2C_WRITE) ||
        (i2cContext.curOp.operation == I2C_WRITE_READ) ||
        (i2cContext.curOp.operation == I2C_WRITE_READ_BLOCK)),
        I2C_INVALID_TASK_STATE, __LINE__,
        i2cContext.curOp.device->deviceAddress, i2cContext.curOp.operation);

    uint8_t *data = i2cContext.curOp.ptrToData;

    // Write the HW fifo
    while ((i2cContext.writeBytesProcessed < i2cContext.curOp.byteCountWrite) &&
           (i2c_registers->fifo.bf.fifo_full == 0))
    {
        i2c_registers->wr_data.bf.wr_data = data[i2cContext.writeBytesProcessed];
        i2cContext.writeBytesProcessed++;
    }

    // Check if the almost emptyIrq should be disabled
    if (i2cContext.writeBytesProcessed >= i2cContext.curOp.byteCountWrite)
    {
        i2c_registers->irq.s.enable.bf.fifo_ae = 0;
    }

    // Clear IRQ
    i2c_registers->irq.s.pending.bf.fifo_ae = 1;
}

//#################################################################################################
// I2C_AlmostFullIrq: ISR when the HW i2c fifo is almost full
//
// Parameters:
// Return:
// Assumptions: We are in an i2c read operation, read more data!
//#################################################################################################
void I2C_AlmostFullIrq(void)
{
    uint8_t blockSize;

    // Ensure there is something to do, otherwise why did we get the irq
    iassert_I2C_COMPONENT_3( i2cContext.opInProgress &&
                            ((i2cContext.curOp.operation == I2C_READ) ||
                            (i2cContext.curOp.operation == I2C_WRITE_READ) ||
                            (i2cContext.curOp.operation == I2C_WRITE_READ_BLOCK)),
                            I2C_INVALID_TASK_STATE, __LINE__,
                            i2cContext.curOp.device->deviceAddress, i2cContext.curOp.operation);

    // read all the data, and ensure we are not done, we should only complete in the done IRQ
    if (i2cReadHwFifo())
    {
        ifail_I2C_COMPONENT_3( I2C_INVALID_TASK_STATE, __LINE__,
                        i2cContext.curOp.device->deviceAddress, i2cContext.curOp.operation);
    }

    if (i2cContext.curOp.operation == I2C_WRITE_READ_BLOCK)
    {
        // The first byte read back is the size of the i2c read block
        blockSize = i2cContext.curOp.ptrToData[0];
    }
    else
    {
        blockSize = i2cContext.curOp.byteCountRead;
    }

    // If the hardware fifo is large enough to store all of the bytes remaining to be read,
    // then we can safely disable the almost full interrupt.
    if ((blockSize - i2cContext.readBytesProcessed) <= i2c_hw_fifo_size)
    {
        i2c_registers->irq.s.enable.bf.fifo_af = 0;
    }

    // Clear IRQ
    i2c_registers->irq.s.pending.bf.fifo_af = 1;
}

//#################################################################################################
// I2C_DoneIrq: ISR when the HW I2C operation has completed
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2C_DoneIrq(void)
{
    // Clear interrupt
    i2c_registers->irq.s.pending.bf.done = 1;
    const i2c_master_control controlReg = {.dw = i2c_registers->control.dw};
    const i2c_master_fifo fifoReg = {.dw = i2c_registers->fifo.dw};

    // Ensure there is something to do, otherwise why did we get the irq
    iassert_I2C_COMPONENT_3( i2cContext.opInProgress && (controlReg.bf.go == 0),
                            I2C_INVALID_TASK_STATE, __LINE__,
                            i2cContext.curOp.device->deviceAddress, i2cContext.curOp.operation);

    // check for errors
    if (controlReg.bf.error)
    {
        ilog_I2C_COMPONENT_1(ILOG_MINOR_ERROR, I2C_TRN_ERROR, fifoReg.dw);

        // clear i2c fifo
        i2c_registers->control.bf.clear = 1;
        i2c_registers->control.bf.clear = 0;    // bit needs to be explicitly cleared to exit

        finalizeAsyncOperation(0, 0);
    }
    else // No errors
    {
        if (i2cContext.curOp.operation == I2C_WRITE)
        {
            ilog_I2C_COMPONENT_0(ILOG_DEBUG, I2C_WRITE_FINISH);

            // Validate state of system
            // NOTE: There is a small chance, that the system is overloaded with high priority
            //       interrupts.  In this case SW may not be able to stop the i2c fifo from
            //       underflowing in time.  This will cause an assert, and in production the system
            //       will reset.
            iassert_I2C_COMPONENT_3( (fifoReg.bf.fifo_empty == 1) &&
                            (i2cContext.writeBytesProcessed == i2cContext.curOp.byteCountWrite),
                            I2C_INVALID_TASK_STATE, __LINE__,
                            i2cContext.curOp.device->deviceAddress, i2cContext.curOp.operation);

            finalizeAsyncOperation(true, 0); // argument is for Success
        }
        else // read operation, or writeRead, or writeReadBlock
        {
            ilog_I2C_COMPONENT_0(ILOG_DEBUG, I2C_READ_FINISH);

            // Read all data, and ensure it was all read
            if(!i2cReadHwFifo())
            {
                ifail_I2C_COMPONENT_3( I2C_INVALID_TASK_STATE, __LINE__,
                                       i2cContext.curOp.device->deviceAddress, i2cContext.curOp.operation);
            }

            finalizeAsyncOperation((uint32_t)(i2cContext.curOp.ptrToData), i2cContext.readBytesProcessed);
        }
    }
}

//#################################################################################################
// i2cStatus: ICommand for checking current status
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void i2cStatus(void)
{
    // Check the I2C state of where it is in processing Async operations
    if (i2cContext.opInProgress)
    {
        ilog_I2C_COMPONENT_3( ILOG_USER_LOG, I2C_OPERATIONS, i2c_fifoSpaceUsed() + 1,
            i2cContext.curOp.device->deviceAddress, i2cContext.curOp.operation);
    }
    else if (i2c_fifoEmpty())
    {
        ilog_I2C_COMPONENT_0(ILOG_USER_LOG, I2C_NO_OPERATIONS);
    }
    else
    {
        ilog_I2C_COMPONENT_1(ILOG_USER_LOG, I2C_OPERATIONS_QUEUED, i2c_fifoSpaceUsed());
    }
}


// Static Function Definitions ####################################################################
//#################################################################################################
// i2cIdleTask: See description at the top of the file
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void i2cIdleTask(void *param1, void *param2)
{
    if ( !i2cContext.opInProgress && !i2c_fifoEmpty() )
    {
        iassert_I2C_COMPONENT_3(
            (i2c_registers->fifo.bf.fifo_empty != 0) && (i2c_registers->control.bf.go != 1),
            I2C_INVALID_TASK_STATE, __LINE__,
            i2cContext.curOp.device->deviceAddress, i2cContext.curOp.operation);

        struct I2cTask peekTask = i2c_fifoPeekRead();

        if(I2C_SetPortAsync(peekTask.device->port, i2cSetPortDone))
        {
            i2cStart();
        }
    }
}

//#################################################################################################
// i2cSetPortDone: Call back handler after setting i2c mux switch
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void i2cSetPortDone(bool success)
{
    iassert_I2C_COMPONENT_3( success, I2C_INVALID_TASK_STATE, __LINE__,
                    i2cContext.curOp.device->deviceAddress, i2cContext.curOp.operation);

    i2cStart();
}

//#################################################################################################
// i2cStart: Start I2C Wakeup or communication interface
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void i2cStart(void)
{
    i2cContext.opInProgress = true;
    i2cContext.curOp = i2c_fifoRead();

    if (i2cContext.curOp.operation == I2C_WAKE)
    {
        i2cContext.wakeState = I2C_WAKE_DRIVE_SDA_LOW_STATE;
        i2cWakeOp(I2C_WAKE_START);
        i2cMarkTime(TIME_MARKER_I2C_WAKE_START);
        CALLBACK_Run(i2cWakeDelay, NULL, NULL);
    }
    else
    {
        i2cStartInterface();
    }
}

//#################################################################################################
// i2cWakeOp: Process an i2c Wake operation
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void i2cWakeOp(enum i2cWakeOperation wakeOp)  // Command to process
{
    i2c_master_control controlRegVal =
        {.bf = {.force_sda_low = (wakeOp == I2C_WAKE_START) ? 1 : 0}};

    controlRegVal.bf.clock_stretch_en = 1;

    ilog_I2C_COMPONENT_1(ILOG_DEBUG, I2C_DO_WAKE_OP, wakeOp);

    i2c_registers->control.dw = controlRegVal.dw;
    i2cContext.i2cWakeTimer = LEON_TimerRead();
}

//#################################################################################################
// i2cWakeDelay: Enforces a wake delay after wakeup signal
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void i2cWakeDelay(void *param1, void *param2)
{
    iassert_I2C_COMPONENT_3(
        ((i2cContext.curOp.operation == I2C_WAKE) && (i2cContext.wakeState == I2C_WAKE_DRIVE_SDA_LOW_STATE)),
        I2C_INVALID_TASK_STATE, __LINE__,
        i2cContext.curOp.device->deviceAddress, i2cContext.curOp.operation);

    if (LEON_TimerCalcUsecDiff(i2cContext.i2cWakeTimer, LEON_TimerRead()) > I2C_WAKE_TIME_DRIVE_SDA_LOW)
    {
        // wake complete, start the idle
        i2cContext.wakeState = I2C_WAKE_IDLE_TIME_STATE;
        i2cWakeOp(I2C_WAKE_STOP);
        i2cMarkTime(TIME_MARKER_I2C_WAKE_STOP);
        TIMING_TimerStart(i2cContext.idleWakeTimer);
    }
    else
    {
        CALLBACK_Run(i2cWakeDelay, NULL, NULL);
    }
}

//#################################################################################################
// i2cIdleDelay: Wakeup is done after the wake up signal followed by a minimum idle time
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void i2cIdleDelay( void )
{
    iassert_I2C_COMPONENT_3(
        ((i2cContext.curOp.operation == I2C_WAKE) && (i2cContext.wakeState == I2C_WAKE_IDLE_TIME_STATE)),
        I2C_INVALID_TASK_STATE, __LINE__,
        i2cContext.curOp.device->deviceAddress, i2cContext.curOp.operation);

    // wake idle complete
    i2cContext.wakeState = I2C_WAKE_DRIVE_NONE;
    ilog_I2C_COMPONENT_0(ILOG_DEBUG, I2C_WAKE_COMPLETE);
    finalizeAsyncOperation(0, 0);
}

//#################################################################################################
// i2cStartInterface: Start an i2c operation
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void i2cStartInterface(void)
{
    i2c_master_control controlReg = {.dw = i2c_registers->control.dw};
    i2c_master_irq_enable intMaskReg = {.dw = i2c_registers->irq.s.enable.dw};

    controlReg.bf.slave_addr = i2cContext.curOp.device->deviceAddress;
    i2c_registers->prescaler.bf.prescaler = I2cSpeedPrescaler[i2cContext.curOp.device->speed];

    intMaskReg.bf.fifo_af = i2cContext.curOp.byteCountRead > i2c_hw_fifo_size;      // Read more than hw fifo size case
    intMaskReg.bf.fifo_ae = i2cContext.curOp.byteCountWrite > i2c_hw_fifo_size;     // Write more than hw fifo size case

    i2cContext.writeBytesProcessed = 0;
    i2cContext.readBytesProcessed = 0;
    controlReg.bf.opmode = i2cContext.curOp.operation;

    if (i2cContext.curOp.operation == I2C_READ)
    {
        ilog_I2C_COMPONENT_1(ILOG_DEBUG, I2C_START_READ, i2cContext.curOp.byteCountRead);

        controlReg.bf.read_byte_count = i2cContext.curOp.byteCountRead;
    }
    else // write operation, or writeRead, or writeReadBlock operation
    {
        uint8_t bytesToWriteNow = MIN(i2cContext.curOp.byteCountWrite, i2c_hw_fifo_size);
        uint8_t* data = i2cContext.curOp.ptrToData;
        i2cContext.writeBytesProcessed = bytesToWriteNow;

        if (i2cContext.curOp.operation == I2C_WRITE)
        {
            ilog_I2C_COMPONENT_0(ILOG_DEBUG, I2C_START_WRITE);
        }
        else if (i2cContext.curOp.operation == I2C_WRITE_READ)
        {
            ilog_I2C_COMPONENT_2(ILOG_DEBUG, I2C_START_WRITE_READ,
                i2cContext.curOp.byteCountWrite, i2cContext.curOp.byteCountRead);

            controlReg.bf.read_byte_count = i2cContext.curOp.byteCountRead;
        }
        else if (i2cContext.curOp.operation == I2C_WRITE_READ_BLOCK)
        {
            ilog_I2C_COMPONENT_2(ILOG_DEBUG, I2C_START_WRITE_READ_BLOCK,
                i2cContext.curOp.byteCountWrite, i2cContext.curOp.byteCountRead);

            intMaskReg.bf.fifo_af = 1;          // Set almost full irq because we don't know reply data amount
        }
        else
        {
            ifail_I2C_COMPONENT_3( I2C_INVALID_TASK_STATE, __LINE__,
                                    i2cContext.curOp.device->deviceAddress, i2cContext.curOp.operation);
        }

        for (uint8_t i = 0; i < bytesToWriteNow; i++)
        {
            i2c_registers->wr_data.bf.wr_data = data[i];
        }
    }

    // Kick off the transaction
    i2c_registers->irq.s.enable.dw = intMaskReg.dw;
    i2c_registers->control.dw = controlReg.dw;
    i2c_registers->control.bf.go = 1;

    i2cMarkTime(TIME_MARKER_I2C_START);
}

//#################################################################################################
// i2cReadHwFifo: Helper function to read all data out of HW i2c fifo
//
// Parameters:
// Return:      true if all data has been read, false otherwise
// Assumptions:
//#################################################################################################
static bool i2cReadHwFifo(void)
{
    uint8_t* data = i2cContext.curOp.ptrToData;
    uint8_t byteCount = i2cContext.curOp.byteCountRead;     // byteCount: number of data read or maximum buffer size

    // Read the HW fifo
    while (i2c_registers->fifo.bf.fifo_empty == 0)
    {
        const uint8_t dataByteRead = i2c_registers->rd_data.bf.rd_data;

        if (i2cContext.readBytesProcessed < byteCount)
        {
            data[i2cContext.readBytesProcessed] = dataByteRead;
        }
        else
        {   // Read more than what we want to read or size of buffer
            ifail_I2C_COMPONENT_2(I2C_READ_TOO_MANY_BYTES, __LINE__, byteCount);
        }
        i2cContext.readBytesProcessed++;
    }

    if (i2cContext.curOp.operation == I2C_WRITE_READ_BLOCK)
    {
        byteCount = data[0];                                // update real data length
    }

    return (i2cContext.readBytesProcessed >= byteCount);
}

//#################################################################################################
// finalizeAsyncOperation: Completion handler on all i2c operations
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void finalizeAsyncOperation
(
    uint32_t arg1,    // Args to pass to completion handler
    uint32_t arg2     // Format varies depending on transaction type: i2c/read/write/etc
)
{
    i2c_registers->irq.s.enable.bf.fifo_af = 0;     // ensure interrupts for i2c almost full/empty are disabled
    i2c_registers->irq.s.enable.bf.fifo_ae = 0;

    i2cContext.opInProgress = false;                // Mark the operation as complete

    CALLBACK_Run(i2cIdleTask, NULL, NULL);          // New operations are kicked off in the idle task

    if (i2cContext.curOp.notifyCompletionHandler)
    {
        CALLBACK_Run(i2cContext.curOp.notifyCompletionHandler, (void *)arg1, (void *)arg2);
    }

    i2cMarkTime(TIME_MARKER_FINALIZE_OP);
}

//#################################################################################################
// submitAsyncOperation: Submit an operation to the ififo of outstanding operations
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void submitAsyncOperation(const struct I2cTask *newOp)
{
    iassert_I2C_COMPONENT_0(!i2c_fifoFull(), I2C_FIFO_OVER_FLOW);

    i2c_fifoWrite(*newOp);              // Add operation to our work queue

    CALLBACK_Run(i2cIdleTask, NULL, NULL);

    i2cMarkTime(TIME_MARKER_SUBMIT_OP);
}

//#################################################################################################
// I2C_InitPort: Initialize I2C port
//
// Parameters:
// Return:
// Assumptions: We can't reset bb_top and Switch chipset when load program_bb
//#################################################################################################
static void I2C_InitPort(void)
{
    i2cContext.currentPort = I2C_MUX_CORE;
    bb_top_SetRtlI2cMuxPort(I2C_MASTER_CORE);
    I2C_pC9548SelectPortsBlocking(0);
}

//#################################################################################################
// I2C_SetPortBlock: Blocking I2C port control
//
// Parameters: enum I2cMuxPort
// Return:
// Assumptions:
//#################################################################################################
static void I2C_SetPortBlock(enum I2cMuxPort port)
{
    if (port != i2cContext.currentPort)         // Check if current port needs to be changed
    {
        if(port >= I2C_MUX_NONE)                // Doesn't need TI i2c mux case: Core or Motherboard
        {
            enum I2cPortSel portSel = (port - I2C_MUX_NONE) == 0 ? I2C_MASTER_CORE : I2C_MASTER_MOTHERBOARD;
            bb_top_SetRtlI2cMuxPort(portSel);
        }
        else
        {
            bb_top_SetRtlI2cMuxPort(I2C_MASTER_CORE);
            I2C_pC9548SelectPortsBlocking(1 << port);
        }

        i2cContext.currentPort = port;
    }
}

//#################################################################################################
// I2C_SetPortAsync: I2C port control
//
// Parameters: enum I2cMuxPort
// Return:     true: set finished, false: set is progressing due to switch control
// Assumptions:
//#################################################################################################
static bool I2C_SetPortAsync(enum I2cMuxPort port, void (*selectDone)(bool success))
{
    bool switchDone = true;

    if (port != i2cContext.currentPort)         // Check if current port needs to be changed
    {
        if(port >= I2C_MUX_NONE)                // Doesn't need TI i2c mux case: Core or Motherboard
        {
            enum I2cPortSel portSel = (port - I2C_MUX_NONE) == 0 ? I2C_MASTER_CORE : I2C_MASTER_MOTHERBOARD;
            bb_top_SetRtlI2cMuxPort(portSel);
        }
        else
        {
            bb_top_SetRtlI2cMuxPort(I2C_MASTER_CORE);
            I2C_pC9548SelectPorts(1 << port, selectDone);
            switchDone = false;
        }

        i2cContext.currentPort = port;
    }

    return switchDone;
}

//#################################################################################################
// Configure control registers for blocking operation
//
// Parameters:
//              handle - handle with the address and speed
//              op - selecting the i2c operation (read, write, etc...)
// Return:
// Assumptions:
//              Write will call this after filling fully or partly, the fifo wr_data
//              Read will call this before polling on the data avail in read fifo rd_data
//#################################################################################################
static void i2cConfigBlocking(
    const struct I2cDevice *device,
    enum i2cBusOperation op,
    uint8_t readByteCount)
{
    i2c_master_control controlReg = {.dw = i2c_registers->control.dw};

    controlReg.bf.slave_addr = device->deviceAddress;
    i2c_registers->prescaler.bf.prescaler = I2cSpeedPrescaler[device->speed];

    switch (op)
    {
        case I2C_READ:
        case I2C_WRITE_READ:
            controlReg.bf.read_byte_count = readByteCount;
            break;
        case I2C_WRITE:
        case I2C_WRITE_READ_BLOCK:
            break;
        case I2C_WAKE:
        default:
            ifail_I2C_COMPONENT_3( I2C_INVALID_TASK_STATE, __LINE__,
                                    i2cContext.curOp.device->deviceAddress, i2cContext.curOp.operation);
            break;
    }

    controlReg.bf.opmode = op;

    // Kick off the transaction
    i2c_registers->control.dw = controlReg.dw;
    i2c_registers->control.bf.go = 1;
}

//#################################################################################################
// i2cStoreSpeedFreq
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void i2cStoreSpeedFreq(uint32_t idx, uint32_t freq)
{
    if (idx < NUM_OF_I2C_SPEEDS)
    {
        I2cSpeedPrescaler[idx] = i2cFreqToPrescalerConv(freq);
    }
}

//#################################################################################################
// i2cFreqToPrescalerConv
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static uint32_t i2cFreqToPrescalerConv(uint32_t freq)
{
    uint32_t res = 0;
    if (freq > 0)
    {
        res = bb_core_getCpuClkFrequency() / (freq * 4);
    }
    return res;
}

//#################################################################################################
// i2cMarkTime : Print out a timer marker message.
//               Useful for determining how much time passes between events
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
#ifdef I2C_USE_TIME_MARKERS
static void i2cMarkTime(I2C_COMPONENT_ilogCodeT msg)
{
    LEON_TimerValueT currTime = LEON_TimerRead();
    ilog_I2C_COMPONENT_1(ILOG_MINOR_EVENT, msg, LEON_TimerCalcUsecDiff(i2cContext.lastTimeMarker, currTime));
    i2cContext.lastTimeMarker = currTime;
}
#else
static inline void i2cMarkTime(I2C_COMPONENT_ilogCodeT msg) { }
#endif


//#################################################################################################
// Selects the ports of the I2C switch specified by applying the supplied portMask to the supplied
// portSettings.
//
// Parameters:
//      portSettings        - A bit vector specifying which ports to turn on (1) and off (0).
//      selectDone          - Callback function to be called when the new port configuration has
//                            been applied.
// Return:
// Assumptions: This doesn't use I2C fifo. It must run before we start new 'real' i2c communication
//
//#################################################################################################
static void I2C_pC9548SelectPorts(uint8_t portSettings, void (*selectDone)(bool success))
{
    iassert_I2C_COMPONENT_0(!i2cContext.opInProgress, I2C_CONTROL_SWITCH_ERROR);
    ilog_I2C_COMPONENT_1(ILOG_DEBUG, I2C_SWITCH_SELECT_PORTS, portSettings);

    i2cContext.muxPortBuffer = portSettings;
    memset(&i2cContext.curOp, 0, sizeof(i2cContext.curOp));

    i2cContext.curOp.device = &devicePc9548;
    i2cContext.curOp.ptrToData = &i2cContext.muxPortBuffer;
    i2cContext.curOp.notifyCompletionHandler = selectDone;
    i2cContext.curOp.operation = I2C_WRITE;
    i2cContext.curOp.byteCountWrite = 1;

    i2cContext.opInProgress = true;
    i2cStartInterface();
}

//#################################################################################################
// Selects the ports of the I2C switch specified by applying the supplied portMask to the supplied
// portSettings.
//
// Parameters:
//      portSettings        - A bit vector specifying which ports to turn on (1) and off (0).
//
// Return:
//#################################################################################################
static void I2C_pC9548SelectPortsBlocking(uint8_t portSettings)
{
    ilog_I2C_COMPONENT_1(ILOG_DEBUG, I2C_SWITCH_SELECT_PORTS, portSettings);

    I2C_WriteBlocking(&devicePc9548, &portSettings, 1);
}