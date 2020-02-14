///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  -  mdio.c
//
//!   @brief -  Driver for MDIO
//
//!   @note  -  The MDIO &  HW share a register interface, so there can only
//              be 1 transaction at a time, regardless of type or bus.
//
//              This driver works with only asynchronous requests, and queues
//              them up in an ififo.  All code runs with interrupts disabled,
//              and should operate very quickly.
//
//              The ififo queues up all requests.  When the system is ready to
//              process a request, it starts the idle task, which will pops the
//              first request off the queue, and operates on it.  Then it marks
//              the global mdio.opInProgress, as active to prevent a new
//              operation from starting
//
//              Normal call flow:
//              * An  or MDIO public API function is called
//              * The public API function creates a struct MDIO_Transaction,
//                and calls the generic submitASyncOperation()
//              * submitASyncOperation() does the following
//                  * places the operation on the ififo queue
//                  * checks if the system is idle, if so start the idle task
//                    otherwise the operation will be started when HW is free
//              * mdioIdleTask() will
//      `           1) pops the request off the ififo
//                  2) marks mdio.opInProgress as true
//                  2) start  and mdio operations, then stop the idle task
//                  3) run  wake ops, then call finalizeASyncOperation()
//              *  operations may enable almost full/empty irq to read/write
//                the hw buffer, then disable and wait for the completion irq
//              * MDIO &  done irq, calls finalizeASyncOperation()
//              * finalizeASyncOperation()
//                  * marks mdio.opInProgress as false
//                  * calls the completion handler
//                  * checks if another tasks is queued up, and starts/stops
//                    the idle task as needed
//
//               operations are only for the Spartan platform
//              MDIO operations are for the Spartan & Virtex platforms
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <ibase.h>
#include <leon_timers.h>
#include <callback.h>
#include <ififo.h>
#include <bb_core.h>
#include <bb_top.h>
#include <module_addresses_regs.h>
#include <leon_mem_map.h>
#include <interrupts.h>
#include <mdio.h>
#include "mdio_loc.h"

/************************ Defined Constants and Macros ***********************/
#define MDIO_OPERATIONS_FIFO_SIZE   8   // *NOTE* MDIO indirect uses 2 buffers

#define MDIO_MDC_FREQ               (2000000)
#define MDIO_HOLD_CLK               (0x5)
#define MDIO_SETUP_CLK              (0x4)
#define MDIO_POLL_TIMEOUT           (500000)

/******************************** Data Types *********************************/

enum mdioOperation
{
    MDIO_OP_WRITE                       = 1,
    MDIO_OP_READ                        = 2,
    MDIO_OP_INDIRECT_ADDRESS            = 0,
    MDIO_OP_INDIRECT_WRITE              = 1,
    MDIO_OP_INDIRECT_POST_READ_INC_ADDR = 2,
    MDIO_OP_INDIRECT_READ               = 3
};

enum MDIO_StartOfFrame
{
    INDIRECT_ADDRESS,
    DIRECT_ADDRESS
};


struct MDIO_Transaction
{
    uint8_t deviceAddress;          // 5 bit MDIO addr
    uint8_t op;                     // use enum mdioOperation
    uint8_t indirectMode;           // Set if indirect mode is used
    uint8_t regAddress;             // Internal device register addr

    uint16_t writeData;             // data to write for write operation
    uint8_t muxPort;                // mux port number - must be powers of 2

    void *notifyCompletionHandler;  // pointer to callback function
};

/***************************** Local Variables *******************************/

static struct {
    struct MDIO_Transaction curOp;

    bool opInProgress;

    volatile mdio_master_s* mdio_registers;
    void (*callback)(void);    // callback which runs in blocking while loop

#ifdef MDIO_USE_TIME_MARKERS
    LEON_TimerValueT lastTimeMarker;        // For debugging log time stamps
#endif

} mdioContext;

/************************ Local Function Prototypes **************************/

// Call the ififo macro to create the local function prototypes
IFIFO_CREATE_FIFO_LOCK_UNSAFE(mdio, struct MDIO_Transaction, MDIO_OPERATIONS_FIFO_SIZE)

static void mdioIdleTask(void *param1, void *param2)                __attribute__ ((section(".ftext")));
static void mdioStart(struct MDIO_Transaction *op)                    __attribute__ ((section(".ftext")));
static void submitASyncOperation(struct MDIO_Transaction *newOp, void* notifyHandler)  __attribute__ ((section(".ftext")));
static void mdioMarkTime(MDIO_COMPONENT_ilogCodeT)                  __attribute__ ((section(".ftext")));

static void mdioAssertInvalidState(uint32_t line)                   __attribute__ ((noinline, noreturn));
static uint32_t calcHalfMDCPeriod(uint32_t freq)                    __attribute__ ((section(".ftext")));

static void MdioNotifyReadHandler(void *param1, void *param2);
static void MdioNotifyWriteHandler(void *param1, void *param2);

// Exported Function Definitions ##################################################################

/**
* FUNCTION NAME: _MdioInit()
*
* @brief  - Initialization function
*
* @return - void
*/
void _MdioInit(void (*callback)(void))
{
    mdioContext.mdio_registers = (volatile mdio_master_s*) bb_chip_mdio_master_s_ADDRESS;
    mdioContext.callback = callback;

    bb_top_ResetMdioMaster(false);      // FPGA MDIO block out of reset

#ifdef MDIO_USE_TIME_MARKERS
    mdioContext.lastTimeMarker = LEON_TimerRead();
#endif

    // Enable the done interrupt
    mdioContext.mdio_registers->irq.s.enable.bf.done = 1;

    const uint8_t halfMDCPeriodClocks = calcHalfMDCPeriod(MDIO_MDC_FREQ);
    mdioContext.mdio_registers->timing.bf.half_mdc_clks = halfMDCPeriodClocks;
    // hold_clks is relevant to the case where the master is driving the data line.  It represents
    // the number of CPU clock cycles after the rising edge of the MDIO clock before the data
    // transition occurs.  By spec, this value must be > 10ns.  We choose a value of 1/2 the MDIO
    // clock period because that gives the slave a long time to sample the data after the clock
    // edge.
    mdioContext.mdio_registers->timing.bf.hold_clks = halfMDCPeriodClocks;
    // setup_clks controls when data is sampled for a read.  The value represents the number of CPU
    // clock cycles before the MDIO clock rising edge.  The value can be at most MDIO_PERIOD -
    // 300ns and must be > 0ns.  This is because the MDIO spec allows the slave to drive data
    // between 0 and 300ns after the rising edge of the MDIO clock.  A valud of (MDIO_PERIOD - 300)
    // / 2 gives the optimal position.
    // setup_clks = ((1 / MDIO_MDC_FREQ) - (3 / 10000000)) * LEON_CPU_CLK_FREQ
    mdioContext.mdio_registers->timing.bf.setup_clks =
        (bb_core_getCpuClkFrequency() / MDIO_MDC_FREQ) - (3 * bb_core_getCpuClkFrequency() / 10000000);

#ifdef PLATFORM_A7
    bb_top_TriStateMdioMdc(false);
#endif

    TOPLEVEL_setPollingMask(SECONDARY_INT_MDIO_INT_MSK);
}


/**
* FUNCTION NAME: Mdio_InterruptHandler()
*
* @brief  - ISR when the HW  or MDIO operation has completed
*
* @return - void
*/
void Mdio_InterruptHandler(void)
{
    if (mdioContext.mdio_registers->irq.s.pending.bf.done == 1)
    {
        // write to clear
        mdioContext.mdio_registers->irq.s.pending.bf.done = 1;

        // Ensure there is something to do, otherwise why did we get the irq
        if (!mdioContext.opInProgress)
        {
            mdioAssertInvalidState(__LINE__);
        }

        if (mdioContext.mdio_registers->control.bf.go == 1)
        {
            mdioAssertInvalidState(__LINE__);
        }

        // MDIO operation has completed
        const uint32_t data = mdioContext.mdio_registers->rd_data.bf.rd_data;
        ilog_MDIO_COMPONENT_1(ILOG_DEBUG, MDIO_FINISH, data);

        void *callback = mdioContext.curOp.notifyCompletionHandler;

        if (callback != NULL)
        {
            if ( (mdioContext.curOp.op == MDIO_OP_READ) || (mdioContext.curOp.op == MDIO_OP_INDIRECT_READ) )
            {
                CALLBACK_Run(MdioNotifyReadHandler, callback, (void *)data);
            }
            else
            {
                CALLBACK_Run(MdioNotifyWriteHandler, callback, (void *)data);
            }
        }

        mdioMarkTime(TIME_MARKER_FINALIZE_OP);

        // Mark the operation as complete
        mdioContext.opInProgress = false;

        // New operations are kicked off in the idle task
        CALLBACK_Run(mdioIdleTask, NULL, NULL);
    }
}


/**
* FUNCTION NAME: _assertHookmdio()
*
* @brief  - Called on an assert, to help debug the assert
*
* @return - void
*/
void _assertHookMdio(void)
{
    mdioStatus();
    ilog_MDIO_COMPONENT_1(ILOG_FATAL_ERROR, MDIO_CONTROLREG_READ, mdioContext.mdio_registers->control.dw);
}


/**
* FUNCTION NAME: mdioStatus()
*
* @brief  - ICommand for checking current status
*
* @return - void
*
* @note   - Also called on the assert hook to help debug
*/
void mdioStatus(void)
{
    // Check the MDIO state of where it is in processing ASync operations
    if (mdioContext.opInProgress)
    {
        ilog_MDIO_COMPONENT_3(
                ILOG_USER_LOG,
                MDIO_OPERATIONS,
                mdio_fifoSpaceUsed() + 1,
                (uint32_t)mdioContext.curOp.notifyCompletionHandler,
                (uint32_t)mdioContext.curOp.regAddress);
   }
    else if (mdio_fifoEmpty())
    {
        ilog_MDIO_COMPONENT_0(ILOG_USER_LOG, MDIO_NO_OPERATIONS);
    }
    else
    {
        ilog_MDIO_COMPONENT_1(ILOG_USER_LOG, MDIO_OPERATIONS_QUEUED, mdio_fifoSpaceUsed());
    }
}



/**
* FUNCTION NAME: MdioWriteASync()
*
* @brief  - Write to an MDIO device
*
* @return - void
*
* @note   - This is part of the public API of this module
*/
void MdioWriteASync
(
    uint8_t device,                                     // MDIO device address
    uint8_t address,                                    // MDIO register address
    uint16_t data,                                      // Data to write
    NotifyWriteCompleteHandler writeCompleteHandler,    // completion handler
    uint8_t muxPort
)
{
    struct MDIO_Transaction newOp = { 0 };
    newOp.deviceAddress = device;
    newOp.op = MDIO_OP_WRITE;
    newOp.regAddress = address;
    newOp.writeData = data;
    newOp.muxPort = muxPort;

    // Go
    submitASyncOperation(&newOp, writeCompleteHandler);
}


/**
* FUNCTION NAME: MdioReadASync()
*
* @brief  - Read from an MDIO device
*
* @return - void
*
* @note   - This is part of the public API of this module
*/
void MdioReadASync
(
    uint8_t device,                                     // MDIO device address
    uint8_t address,                                    // MDIO register address
    NotifyReadCompleteHandler readCompleteHandler,      // completion handler
    uint8_t muxPort
)
{
    struct MDIO_Transaction newOp = { 0 };
    newOp.deviceAddress = device;
    newOp.op = MDIO_OP_READ;
    newOp.regAddress = address;
    newOp.muxPort = muxPort;

    // Go
    submitASyncOperation(&newOp, readCompleteHandler);
}


void MdioIndirectWriteASync
(
    uint8_t device, // Phy address, 5 bits
    enum MDIO_DEVTYPE devType, // 5 bits: PMD/PMA, WIS, PCS, PHY XS, DTE XS
    uint16_t address,
    uint16_t data,
    NotifyWriteCompleteHandler writeCompleteHandler,    // completion handler
    uint8_t muxPort
)
{
    // Create new operation for issuing an address command
    struct MDIO_Transaction newOp = { 0 };
    newOp.deviceAddress = device;
    newOp.op = MDIO_OP_INDIRECT_ADDRESS;
    newOp.indirectMode = 1;
    newOp.regAddress = devType;
    newOp.writeData = address;
    newOp.muxPort = muxPort;

    // Go
    submitASyncOperation(&newOp, NULL);

    // Modify operation for the write command
    newOp.op = MDIO_OP_INDIRECT_WRITE;
    newOp.writeData = data;

    // Go
    submitASyncOperation(&newOp, writeCompleteHandler);
}


void MdioIndirectReadASync
(
    uint8_t device, // Phy address, 5 bits
    enum MDIO_DEVTYPE devType, // 5 bits: PMD/PMA, WIS, PCS, PHY XS, DTE XS
    uint16_t address,
    NotifyReadCompleteHandler readCompleteHandler,  // completion handler
    uint8_t muxPort
)
{
    // Create new operation for issuing an address command
    struct MDIO_Transaction newOp = { 0 };
    newOp.deviceAddress = device;
    newOp.op = MDIO_OP_INDIRECT_ADDRESS;
    newOp.indirectMode = 1;
    newOp.regAddress = devType;
    newOp.writeData = address;
    newOp.muxPort = muxPort;

    // Go
    submitASyncOperation(&newOp, NULL);

    // Modify operation for the write command
    newOp.op = MDIO_OP_INDIRECT_READ;

    // Go
    submitASyncOperation(&newOp, readCompleteHandler);
}


/**
* FUNCTION NAME: MdioReadSync()
*
* @brief  - Synchonously read a value from the requested MDIO attached device's register
*
* @return - the data read from the MDIO device
*/
uint16_t MdioReadSync
(
    uint8_t device, // Address of MDIO attached device
    uint8_t address, // Register address to read from
    uint8_t muxPort
)
{
    // We require the FIFO to be empty for sync writes so we don't
    // clobber other pending operations
    mdioContext.mdio_registers->irq.s.enable.bf.done = 0;

    iassert_MDIO_COMPONENT_0(mdio_fifoEmpty(), SYNC_MDIO_READ_FIFO_NOT_EMPTY);

    if (muxPort != MDIO_MUX_NOT_PRESENT)
    {
        bb_top_SetMdioSlave(muxPort);
    }
    // Select the device
    mdioContext.mdio_registers->control.bf.phyad = device;

    // Write the register address
    mdioContext.mdio_registers->control.bf.regad = address;

    mdioContext.mdio_registers->control.bf.op = MDIO_OP_READ;

    // Set access mode to read
    mdioContext.mdio_registers->control.bf.st = DIRECT_ADDRESS;

    // Kick off the transaction
    mdioContext.mdio_registers->control.bf.go = 1;

    // Spin while we wait for the read to complete
    LEON_TimerValueT timeoutStart = LEON_TimerRead();
    while (mdioContext.mdio_registers->control.bf.go != 0)
    {
        iassert_MDIO_COMPONENT_0 (
            (MDIO_POLL_TIMEOUT > LEON_TimerCalcUsecDiff(timeoutStart, LEON_TimerRead())),
            MDIO_SYNC_READ_TIMEOUT);

        if(mdioContext.callback != NULL)
        {
            mdioContext.callback();
        }
    }

    // Read out the data
    const uint16_t data = mdioContext.mdio_registers->rd_data.bf.rd_data;

    // Clear the interrupt flag that was set when the read completed
    mdioContext.mdio_registers->irq.s.pending.bf.done = 1;
    mdioContext.mdio_registers->irq.s.enable.bf.done = 1;
    return data;
}


/**
* FUNCTION NAME: MdioWriteSync()
*
* @brief  - Synchronously write a value to the requested MDIO attached device's register
*
* @return - void
*/
void MdioWriteSync
(
    uint8_t device,  // The address of the MDIO connected device to write to
    uint8_t address, // The address of the register on the device to write to
    uint16_t data,    // The data to write
    uint8_t muxPort
)
{
    mdioContext.mdio_registers->irq.s.enable.bf.done = 0;
    // We require the FIFO to be empty for sync writes so we don't
    // clobber other pending operations
    iassert_MDIO_COMPONENT_0(mdio_fifoEmpty(), SYNC_MDIO_WRITE_FIFO_NOT_EMPTY);

    // We require the FIFO to be empty for sync writes so we don't
    // clobber other pending operations
    if (muxPort != MDIO_MUX_NOT_PRESENT)
    {
        bb_top_SetMdioSlave(muxPort);
    }

    // Select the device
    mdioContext.mdio_registers->control.bf.phyad = device;

    // Write the register address
    mdioContext.mdio_registers->control.bf.regad = address;

    mdioContext.mdio_registers->control.bf.op = MDIO_OP_WRITE;

    // Set access mode to write
    mdioContext.mdio_registers->control.bf.st = DIRECT_ADDRESS;

    mdioContext.mdio_registers->wr_data.bf.wr_data = data;

    // Kick off the transaction
    mdioContext.mdio_registers->control.bf.go = 1;

    // Spin while we wait for the write to complete
    LEON_TimerValueT timeoutStart = LEON_TimerRead();
    while (mdioContext.mdio_registers->control.bf.go != 0)
    {
        iassert_MDIO_COMPONENT_0 (
            (MDIO_POLL_TIMEOUT > LEON_TimerCalcUsecDiff(timeoutStart, LEON_TimerRead())),
            MDIO_SYNC_WRITE_TIMEOUT);

        if(mdioContext.callback != NULL)
        {
            mdioContext.callback();
        }
    }

    // Clear the interrupt flag that was set when the read completed
    mdioContext.mdio_registers->irq.s.pending.bf.done = 1;
    mdioContext.mdio_registers->irq.s.enable.bf.done = 1;
}


//#################################################################################################
// Perform indirect synchronous read operation
//
// Parameters:
//              device - address of MDIO device
//              devType - indirect address
//              address - register to read from
//              muxPort - RTL MUX port number device is on
// Return:
// Assumptions:
//#################################################################################################
uint16_t MdioIndirectReadSync
(
    uint8_t device, // Address of MDIO attached device
    enum MDIO_DEVTYPE devType, // MDIO Devices have different types on same physical device:
    // PMD/PMA, WIS, PCS, PHY XS, DTE XS, and vendor defined
    uint16_t address, // Register address to read from
    uint8_t muxPort
)
{
    mdioContext.mdio_registers->irq.s.enable.bf.done = 0;
    // We require the FIFO to be empty for sync writes so we don't
    // clobber other pending operations
    iassert_MDIO_COMPONENT_0(mdio_fifoEmpty(), SYNC_MDIO_READ_FIFO_NOT_EMPTY);

    if (muxPort != MDIO_MUX_NOT_PRESENT)
    {
        bb_top_SetMdioSlave(muxPort);
    }
    // Select the device
    mdioContext.mdio_registers->control.bf.phyad = device;

    // Write the Device Type - MDIO device will interpret this as PMD/PMA, PCS, PHY XS etc...
    mdioContext.mdio_registers->control.bf.regad = devType;

    mdioContext.mdio_registers->control.bf.op = MDIO_OP_INDIRECT_ADDRESS;

    // Set access mode to send address via write buffer
    mdioContext.mdio_registers->control.bf.st = INDIRECT_ADDRESS;

    // Write the address in the Device Type we wish to read
    mdioContext.mdio_registers->wr_data.bf.wr_data = address;

    // Kick off the transaction
    mdioContext.mdio_registers->control.bf.go = 1;

    // Spin while we wait for the read to complete
    LEON_TimerValueT timeoutStart = LEON_TimerRead();
    while (mdioContext.mdio_registers->control.bf.go != 0)
    {
        iassert_MDIO_COMPONENT_0 (
            (MDIO_POLL_TIMEOUT > LEON_TimerCalcUsecDiff(timeoutStart, LEON_TimerRead())),
            MDIO_INDIRECT_SYNC_READ_ADDRESS_TIMEOUT);

        if(mdioContext.callback != NULL)
        {
            mdioContext.callback();
        }
    }

    // Now change operation to actual read
    mdioContext.mdio_registers->control.bf.op = MDIO_OP_INDIRECT_READ;

    // Kick off the transaction
    mdioContext.mdio_registers->control.bf.go = 1;

    // Spin while we wait for the read to complete
    timeoutStart = LEON_TimerRead();
    while (mdioContext.mdio_registers->control.bf.go != 0)
    {
        iassert_MDIO_COMPONENT_0 (
            (MDIO_POLL_TIMEOUT > LEON_TimerCalcUsecDiff(timeoutStart, LEON_TimerRead())),
            MDIO_INDIRECT_SYNC_READ_DATA_TIMEOUT);

        if(mdioContext.callback != NULL)
        {
            mdioContext.callback();
        }
    }

    // Read out the data
    const uint16_t data = mdioContext.mdio_registers->rd_data.bf.rd_data;

    // Clear the interrupt flag that was set when the read completed
    mdioContext.mdio_registers->irq.s.pending.bf.done = 1;
    mdioContext.mdio_registers->irq.s.enable.bf.done = 1;
    return data;
}


//#################################################################################################
// Perform indirect synchronous write operation
//
// Parameters:
//              device - address of MDIO device
//              devType - indirect address
//              address - register to write to
//              data - data to write to said address
//              muxPort - RTL MUX port number device is on
// Return:
// Assumptions:
//#################################################################################################
uint16_t MdioIndirectWriteSync
(
    uint8_t device, // Address of MDIO attached device
    enum MDIO_DEVTYPE devType, // MDIO Devices have different types on same physical device:
    //PMD/PMA, WIS, PCS, PHY XS, DTE XS, and vendor defined
    uint16_t address, // Register address for Device Type to write to
    uint16_t data,
    uint8_t muxPort
)
{
    mdioContext.mdio_registers->irq.s.enable.bf.done = 0;
    // We require the FIFO to be empty for sync writes so we don't
    // clobber other pending operations
    iassert_MDIO_COMPONENT_0(mdio_fifoEmpty(), SYNC_MDIO_READ_FIFO_NOT_EMPTY);

    if (muxPort != MDIO_MUX_NOT_PRESENT)
    {
        bb_top_SetMdioSlave(muxPort);
    }
    // Select the device
    mdioContext.mdio_registers->control.bf.phyad = device;

    // Write the Device Type - MDIO device will interpret this as PMD/PMA, PCS, PHY XS etc..
    mdioContext.mdio_registers->control.bf.regad = devType;

    mdioContext.mdio_registers->control.bf.op = MDIO_OP_INDIRECT_ADDRESS;

    // Set access mode to send address via write buffer
    mdioContext.mdio_registers->control.bf.st = INDIRECT_ADDRESS;

    // Write the address in the Device Type we wish to write to
    mdioContext.mdio_registers->wr_data.bf.wr_data = address;

    // Kick off the transaction
    mdioContext.mdio_registers->control.bf.go = 1;

    // Spin while we wait for the write to complete
    LEON_TimerValueT timeoutStart = LEON_TimerRead();
    while (mdioContext.mdio_registers->control.bf.go != 0)
    {
        iassert_MDIO_COMPONENT_0 (
            (MDIO_POLL_TIMEOUT > LEON_TimerCalcUsecDiff(timeoutStart, LEON_TimerRead())),
            MDIO_INDIRECT_SYNC_WRITE_ADDRESS_TIMEOUT);

        if(mdioContext.callback != NULL)
        {
            mdioContext.callback();
        }
    }

    // Now change operation to actual write
    mdioContext.mdio_registers->control.bf.op = MDIO_OP_INDIRECT_WRITE;

    // Write the data
    mdioContext.mdio_registers->wr_data.bf.wr_data = data;

    // Kick off the transaction
    mdioContext.mdio_registers->control.bf.go = 1;

    // Spin while we wait for the read to complete
    timeoutStart = LEON_TimerRead();
    while (mdioContext.mdio_registers->control.bf.go != 0)
    {
        iassert_MDIO_COMPONENT_0 (
            (MDIO_POLL_TIMEOUT > LEON_TimerCalcUsecDiff(timeoutStart, LEON_TimerRead())),
            MDIO_INDIRECT_SYNC_WRITE_DATA_TIMEOUT);

        if(mdioContext.callback != NULL)
        {
            mdioContext.callback();
        }
    }

    // Clear the interrupt flag that was set when the read completed
    mdioContext.mdio_registers->irq.s.pending.bf.done = 1;
    mdioContext.mdio_registers->irq.s.enable.bf.done = 1;
    return data;
}


//#################################################################################################
// Perform indirect synchronous write bit field operation
//
// Parameters:
//              device - address of MDIO device
//              devType - indirect address
//              address - register to write to
//              data - data to write to said address
//              muxPort - RTL MUX port number device is on
//              mask - bits to be written
//              offset - shift left offset of value which will be written
// Return:
// Assumptions: It's blocking syncronous function. Use it only for icommand
//#################################################################################################
void MdioIndirectWriteSyncBitField
(
    uint8_t device, // Address of MDIO attached device
    enum MDIO_DEVTYPE devType, // MDIO Devices have different types on same physical device:
    //PMD/PMA, WIS, PCS, PHY XS, DTE XS, and vendor defined
    uint16_t address, // Register address for Device Type to write to
    uint16_t data,
    uint8_t muxPort,
    uint16_t mask,
    uint8_t offset
)
{
    uint16_t currentValue = MdioIndirectReadSync(device, devType, address, muxPort);    // read current value first
    uint16_t writtenValue = ( currentValue & ~mask ) | (data << offset);                // manipulate written value
    MdioIndirectWriteSync(device, devType, address, writtenValue, muxPort);
}


// Static Function Definitions ####################################################################
static uint32_t calcHalfMDCPeriod(uint32_t freq)
{
    return bb_core_getCpuClkFrequency() / (freq * 2);
}


/**
* FUNCTION NAME: mdioMarkTime()
*
* @brief  - Print out a timer marker message.  Useful for determining how much time passes between
*           events.
*
* @return - void
*
* @note   - When this feature was added it increased code size by 284 bytes
*/
#ifdef MDIO_USE_TIME_MARKERS
static void mdioMarkTime(MDIO_COMPONENT_ilogCodeT msg)
{
    LEON_TimerValueT currTime = LEON_TimerRead();
    ilog_MDIO_COMPONENT_1(
        ILOG_MINOR_EVENT, msg, LEON_TimerCalcUsecDiff(mdioContext.lastTimeMarker, currTime));
    mdioContext.lastTimeMarker = currTime;
}
#else
static inline void mdioMarkTime(MDIO_COMPONENT_ilogCodeT msg) { }
#endif

/**
* FUNCTION NAME: mdioAssertInvalidState()
*
* @brief  - Helper function to save IRAM on asserts
*
* @return - never
*/
static void mdioAssertInvalidState(uint32_t line)
{
    ifail_MDIO_COMPONENT_3(
        MDIO_INVALID_TASK_STATE,
        line,
        (uint32_t)mdioContext.curOp.notifyCompletionHandler,
        (uint32_t)mdioContext.curOp.regAddress);
    __builtin_unreachable();
}


/**
* FUNCTION NAME: mdioStart()
*
* @brief  - Start a new MDIO transaction
*
* @return - void
*/
static void mdioStart( struct MDIO_Transaction *op )
{
    mdioContext.mdio_registers->control.dw = 0;
    mdioContext.mdio_registers->control.bf.phyad = op->deviceAddress;
    mdioContext.mdio_registers->control.bf.regad = op->regAddress;
    mdioContext.mdio_registers->control.bf.op = op->op;
    mdioContext.mdio_registers->control.bf.st = op->indirectMode ? INDIRECT_ADDRESS : DIRECT_ADDRESS;
    ilog_MDIO_COMPONENT_1(ILOG_DEBUG, MDIO_CONTROLREG_READ, mdioContext.mdio_registers->control.dw);
    // set mux
    // Normally MuxPort would be 0xFF but we're restricted to 4 bits (6 bits avail)
    // so we'll keep 0xF as our "no mux" option
    if (op->muxPort != MDIO_MUX_NOT_PRESENT)
    {
        bb_top_SetMdioSlave(op->muxPort);
    }

    if (op->indirectMode)
    {
        switch (op->op)
        {
            case MDIO_OP_INDIRECT_ADDRESS:
                ilog_MDIO_COMPONENT_3(
                    ILOG_DEBUG,
                    MDIO_START_ADDRESS,
                    op->deviceAddress,
                    op->regAddress,
                    op->writeData);
                mdioContext.mdio_registers->wr_data.bf.wr_data = op->writeData;
                break;

            case MDIO_OP_INDIRECT_WRITE:
                ilog_MDIO_COMPONENT_3(
                    ILOG_DEBUG,
                    MDIO_START_WRITE,
                    op->deviceAddress,
                    op->regAddress,
                    op->writeData);
                mdioContext.mdio_registers->wr_data.bf.wr_data = op->writeData;
                break;

            case MDIO_OP_INDIRECT_READ:
                ilog_MDIO_COMPONENT_2(
                    ILOG_DEBUG, MDIO_START_READ, op->deviceAddress, op->regAddress);
                break;

            case MDIO_OP_INDIRECT_POST_READ_INC_ADDR:
            default:
                mdioAssertInvalidState(__LINE__);
                break;
        }
    }
    else // direct mode
    {

        if (op->op == MDIO_OP_READ)
        {
            ilog_MDIO_COMPONENT_2(
                ILOG_DEBUG, MDIO_START_READ, op->deviceAddress, op->regAddress);
        }
        else if (op->op == MDIO_OP_WRITE)
        {
            ilog_MDIO_COMPONENT_3(
                ILOG_DEBUG,
                MDIO_START_WRITE,
                op->deviceAddress,
                op->regAddress,
                op->writeData);
            mdioContext.mdio_registers->wr_data.bf.wr_data = op->writeData;
        }
        else
        {
            mdioAssertInvalidState(__LINE__);
        }
    }

    // Kick off the transaction
    mdioContext.mdio_registers->control.bf.go = 1;
    ilog_MDIO_COMPONENT_1(ILOG_DEBUG, MDIO_CONTROLREG_READ, mdioContext.mdio_registers->control.dw);

    mdioMarkTime(TIME_MARKER_MDIO_START);
}


/**
* FUNCTION NAME: mdioIdleTask()
*
* @brief  - The main idle task
*
* @return - void
*
* @note   - See description at the top of the file
*/
static void mdioIdleTask( void * param1, void * param2)
{
    if ( (mdioContext.opInProgress == false) && !mdio_fifoEmpty() )
    {
        if (mdioContext.mdio_registers->control.bf.go == 1)
        {
            mdioAssertInvalidState(__LINE__);
        }

        mdioContext.opInProgress = true;

        mdioContext.curOp = mdio_fifoRead();

        // new MDIO transaction
        mdioStart(&mdioContext.curOp);
    }
}

/**
* FUNCTION NAME: submitASyncOperation()
*
* @brief  - Submit an operation to the ififo of outstanding operations
*
* @return - void
*
* @note   - See description at the top of the file
*/
static void submitASyncOperation
(
    struct MDIO_Transaction *newOp,
    void* notifyHandler     // completion handler
)
{
    newOp->notifyCompletionHandler = notifyHandler;

    // Add operation to our work queue
    iassert_MDIO_COMPONENT_0(!mdio_fifoFull(), MDIO_FIFO_OVER_FLOW);
    mdio_fifoWrite(*newOp);

    CALLBACK_Run(mdioIdleTask, NULL, NULL);

    mdioMarkTime(TIME_MARKER_SUBMIT_OP);
}

/**
* FUNCTION NAME: MdioNotifyReadHandler()
*
* @brief  - calls the registered read callback function when the operation is complete
*
* @return - void
*
* @note   - See description at the top of the file
*/
static void MdioNotifyReadHandler(void *param1, void *param2)
{
    NotifyReadCompleteHandler notifyHandler = param1;
    uint32_t data = (uint32_t)param2;
    notifyHandler( (uint16_t)data);
}

/**
* FUNCTION NAME: MdioNotifyWriteHandler()
*
* @brief  - calls the registered write callback function when the operation is complete
*
* @return - void
*
* @note   - See description at the top of the file
*/
static void MdioNotifyWriteHandler(void *param1, void *param2)
{
    NotifyWriteCompleteHandler notifyHandler = param1;
    notifyHandler();
}
