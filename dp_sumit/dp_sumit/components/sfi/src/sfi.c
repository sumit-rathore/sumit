///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009
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
//!   @file  -  sfi.c
//
//!   @brief -  Handles the low level access to the flash chip
//
//
//!   @note  -  Every SFI flash chip has slightly different instruction sets,
//              when using this API take note of the flash chip target
//
//              ENDIAN, WTF is happening on the bus with the data bytes
//              Documentation is a little self-conflicting: Talk to Keith
//              Endian is messed up in HW, read and write are opposing endian
//
//              It is assumed that only one user would operate on the flash,
//              so no locking is done at this level.  Caller is responsible
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <sfi.h>
#include <ibase.h>
#include <bb_core.h>
#include <bb_top.h>
#include <uart.h>
#include <sys_defs.h>
#include <stdarg.h>
#include <spi_flash_ctrl_regs.h>
#include <options.h>
#include <module_addresses_regs.h>

/************************ Defined Constants and Macros ***********************/
//#define SFI_DBG
#define SFI_LANE_MASK (0xF) // 1 for single lane, 3 for dual, F for Quad instructions


/******************************** Data Types *********************************/
// Writes, Erases, GetStatus are APB bus commands
// Reads of memory are through AHB
// BasicCfg replaces Instruction register - where cmds are sent
enum SfiTransactionStates
{
    SFI_STATE_IDLE,
    SFI_STATE_ERASE_SECTOR,
    SFI_STATE_PROGRAM
};

enum SfiTransactionEvents
{
    SFI_EVENT_OPERATION_START_NEW,  // API
    SFI_EVENT_OPERATION_SUSPENDED,  // Handler checks and transaction needs to resume
    SFI_EVENT_OPERATION_RESUME,     // Handler sets
    SFI_EVENT_OPERATION_COMPLETED   // Handler sets
};

// Dummy clock cycles for matching CPU frequency
// Below are cpu frequencies and the index represents the number of dummy clock cycles required for
// the named operation (Fast Read and Quad Fast Read)
// Currently not used as it didn't work as expected
// Theory is to use the index as number of clock cycles for AHB reads - speeding things up a little
// given the default is 10 for QUAD, however it doesn't work using reduced dummy clock cycles
uint32_t SfiDummyClockCyclesFastRead[] =
{
    90000000,
    100000000,
    108000000
};

uint32_t SfiDummyClockCyclesQuadFastRead[] =
{
    43000000,
    60000000,
    75000000,
    90000000,
    100000000,
    105000000,
    108000000
};

volatile spi_flash_ctrl_s* sfi_registers;

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

static uint32_t SFI_FifoWordFill(const uint32_t *buffer, uint32_t size);
static uint32_t SFI_FifoByteFill(const uint8_t *buffer, uint32_t size);
static void SFI_sendEnterExit4ByteAddressMode(bool enter);
static void SFI_sendWriteEnable(void);


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: LEON_SFISendWrite()
*
* @brief  - Does one page program to the chip
*
* @return - the number of bytes written
*
* @note   - 1: Flash WINBOND W25Q32FV programming page max size is 256B
*              if less than that is written, the WR_EN instruction must be sent
*              again
*           2: If greater than 256B is attempted, flash chip will overwrite
*              the first bytes; circular buffer behaviour
*           3: Once the SFI HW fifo is full and we issue the TR_EN_BUSY via
*              the control register, we must re-issue the WR_EN instruction
*
*/
uint32_t LEON_SFISendWrite
(
    const uint8_t * buf,          //the data to write
    const uint32_t bufSize,       //the number of bytes in buf
    const bool setGoBit,          // set GO bit to start transaction
    const bool waitOnGoBit        // poll on go bit until transaction completed
)
{
    uint32_t bytesRemaining = bufSize;

    // Write until fifo full, by polling fifo_full, or all bytes written
    // then issue control write and return
    // don't block polling busy for transaction to complete
    // split out actual writing portion into separate function

    if (bufSize > 0) // erase uses this function too - don't write to wr_data in erase case
    {
        // see if we can do word writes
        if ((bytesRemaining >= 4) &&  (((uint32_t)buf & 0x3) == 0))
        {
            bytesRemaining -= SFI_FifoWordFill((const uint32_t *)buf, bytesRemaining);
        }
        else // Handle non-word aligned by packing into register but using byte pointer
        {
            bytesRemaining -= SFI_FifoByteFill(buf, bytesRemaining);
        }

    } // bufSize > 0
    // enable suspend on read operation
    sfi_registers->mm.s.suspend_cfg.bf.enable = 1;

    // start writing the data
    if (setGoBit)
    {
        sfi_registers->control.bf.go = 1;
    }

    // Go bit is cleared when a transaction is complete
    // If we keep up with the FIFO, we don't need to poll on the Go bit until we've sent everything
    // wait for go bit to be cleared
    while((bytesRemaining == 0) && (sfi_registers->control.bf.go == 1))
    {;}

    return (bufSize - bytesRemaining);
}


/**
* FUNCTION NAME: LEON_SFISendInstructionData()
*
* @brief  - send instruction & wait for completion
*
* @return - void
*
* @note   - eg, Write Status Register
*
*           It blocks until the operation is complete
*
*/
// TODO Add argument for suspend enable?
void LEON_SFISendInstructionData
(
    uint32_t data          //the data to write
)
{
    // Write the data into the buffer
    sfi_registers->wr_data.bf.val = data;

    //start writing the data
    sfi_registers->control.bf.go = 1;

    // wait for go bit to be cleared
    while(sfi_registers->control.bf.go == 1)
    {;}
}
/**
* FUNCTION NAME: LEON_SFISendInstruction()
*
* @brief  - send instruction & wait for completion
*
* @return - void
*
* @note   - eg, Write enable, erase
*
*           It blocks until the operation is complete
*
*/
void LEON_SFISendInstruction (void)
{
    sfi_registers->control.bf.go = 1;
    while(sfi_registers->control.bf.go == 1)
    {;}
}


/**
* FUNCTION NAME: LEON_SFISendReadStatus()
*
* @brief  - send instruction, wait for completion, and grab data to return
*
* @return - The data read
*
* @note   - It blocks until the operation is complete
*
*/
uint32_t LEON_SFISendReadStatus(void)
{
    //Odd the old flash writer would clear the data register here
    sfi_registers->control.bf.go = 1;
    while(sfi_registers->control.bf.go == 1)
    {;}

    return sfi_registers->rd_data.bf.val;
}


/**
* FUNCTION NAME: LEON_SFIInit()
*
* @brief  - initialize struct
*
* @return - void
*
*/
void LEON_SFIInit (bool isCurrentImage, bool isAsic)
{
    sfi_registers = (volatile spi_flash_ctrl_s*) bb_chip_spi_flash_ctrl_s_ADDRESS;
    bb_top_ResetSpiFlash(false);          // FPGA SPI Flash block out of reset

    // Need to know if ASIC or FPGA as the offset is different
    // Need to set our APB bus offset so Storage Vars can write to the correct region
    // ProgramBB will nullify this write because it manages address regions itself
    // AHB bus reads will also need to be set, even though the ROM does this
    if (isCurrentImage)
    {
        uint32_t offset = 0;
        if (isAsic)
        {
            offset = FLASH_BIN_ASIC_CURRENT_TABLE_ADDRESS - FLASH_BIN_ASIC_GOLDEN_TABLE_ADDRESS;
        }
        else
        {
            offset = FLASH_BIN_FPGA_CURRENT_TABLE_ADDRESS - FLASH_BIN_FPGA_GOLDEN_TABLE_ADDRESS;
        }
        // APB writes/reads
        sfi_registers->addr_offset.bf.val = offset;
        // AHB (CPU) reads
        sfi_registers->mm.s.addr_offset.bf.val = offset;
    }

    SFI_ResetFlash();

    // Configure for suspend operation - leave disabled, only enable on program or erase
    SFI_configureSuspendOnReadOperation();

    // TODO Configure AHB section for reads
    SFI_configureAhbReadControl();

}


//#################################################################################################
// Configure instruciton parameters
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void SFI_configureInstructionMaskGap(uint16_t gap, uint8_t dataMask, uint8_t addrMask, uint8_t cmdMask)
{
    sfi_registers->basic_cfg.bf.tr_gap_p3 = gap;
    sfi_registers->basic_cfg.bf.io_mask_data = dataMask & SFI_LANE_MASK;
    sfi_registers->basic_cfg.bf.io_mask_addr = addrMask & SFI_LANE_MASK;
    sfi_registers->basic_cfg.bf.io_mask_cmd  = cmdMask  & SFI_LANE_MASK;
}


//#################################################################################################
// Configure instruction control
//
// Parameters:
// Return:
// Assumptions:
// * Go is never set here!
// * NumBytes is 0 if we want to put more than the fifo size (32B)
// * SPI module in DD will cap if we don't keep filling fifo regardless of num_bytes (if > 32)
// * Remco recommends only using num_bytes if < 32bytes
// * If Num_bytes = 0 then SPI module assumes data length is multiple of 4-bytes
//#################################################################################################
void SFI_configureInstructionControl(
    uint8_t addrMode,
    uint8_t readWriteData,
    uint8_t numDummyClks,
    uint8_t numDataBytes,
    uint8_t command,
    uint32_t flashOffset)
{
    sfi_registers->control.bf.write = readWriteData;
    sfi_registers->control.bf.cmd = command;
    sfi_registers->control.bf.addr_mode = addrMode;
    sfi_registers->control.bf.num_dummy_clks = numDummyClks;
    sfi_registers->control.bf.num_bytes = numDataBytes;
    sfi_registers->addr.bf.val = (flashOffset & SPI_FLASH_CTRL_ADDR_VAL_MASK);
}


//#################################################################################################
// Configure Suspend operation
//
// Parameters:
// Return:
// Assumptions:
// * Enable will be set when we are programming or erasing, FPGA will clear it when read complete
// * This is a setup - when a read comes, it will send the suspend comand
// * When we get our callback to check on the status of the erase/program we must first check if we
// were suspended - then we check if the read has completed -- read status
// When read is complete, we set the enable again and send a resume command so the program/erase
// can continue
//#################################################################################################
void SFI_configureSuspendOnReadOperation(void)
{
    // TODO parameterize this, as this driver shouldn't know the details - when we have another
    // chip to worry about - hardcoding here to make parameterization easier in future
    sfi_registers->mm.s.suspend_cfg.bf.tr_gap_p3 = 3;
    sfi_registers->mm.s.suspend_cfg.bf.cmd = 0x75;
    sfi_registers->mm.s.suspend_cfg.bf.io_mask = 0x1;
}



//#################################################################################################
// Any transactions in flight - only applicable to APB transactions, not AHB (reads)
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool SFI_isTransactionInFlight(void)
{
    return (sfi_registers->control.bf.go == 1);
}


//#################################################################################################
// Configure for AHB reads - not through SFI like Read Status Register
//
// Parameters:
// Return:
// Assumptions:
// * Only needs to be called once so any attempt by CPU to read Flash will follow this
// configuration
//#################################################################################################
void SFI_configureAhbReadControl(void)
{
    // TODO parameterize this, as this driver shouldn't know the details - when we have another
    // chip to worry about - hardcoding here to make parameterization easier in future

    // 10 dummy clock cycles - see MT25QL256ABA data sheeet, command definitions (table 19)
    // F - tr gap p3 setting - lower setting (3) didn't work - put to max for compatibility
    // FF1 - see command definitions table - all address, data lanes are available, only D0 for commands
    // EC - 4 byte quad input/output fast read
    sfi_registers->mm.s.basic_cfg.bf.addr_mode = SFI_ADDR_MODE_4BYTE; // address mode
    sfi_registers->mm.s.basic_cfg.bf.num_dummy_clks = 10; // Do not change - complicated
    sfi_registers->mm.s.basic_cfg.bf.tr_gap_p3 = 0xF;
    sfi_registers->mm.s.basic_cfg.bf.io_mask_data = 0xF; // dataMask;
    sfi_registers->mm.s.basic_cfg.bf.io_mask_addr = 0xF; // addrMask;
    sfi_registers->mm.s.basic_cfg.bf.io_mask_cmd = 1; // cmdMask;
    sfi_registers->mm.s.basic_cfg.bf.read_cmd = 0xEC; // fast quad read supporting 3 or 4 byte access
}


//#################################################################################################
// Get Suspend Enable value - SW sets (and can clear) and FPGA clears
// This bit is used to signal to FPGA that if set - issue a SUSPEND if there is a Program or Erase
// in progress at the time of the read
// Once that Program or Erase is completed, this must be cleared by SW
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool SFI_isSuspendEnableSet(void)
{
    // enable is set by SW when initiating a Program or Erase
    // If cleared, FPGA cleared it  or SW cleared it when Program/Erase completed
    // State machine will determine if FPGA or SW cleared it
    return (sfi_registers->mm.s.suspend_cfg.bf.enable == 1);
}


//#################################################################################################
// If no reads took place when Program or Erase was in progress, this bit will remain set, SW
// should clear it
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void SFI_clearSuspendEnable(void)
{
    // enable is set by SW when initiating a Program or Erase
    // If cleared, FPGA cleared it  or SW cleared it when Program/Erase completed
    // State machine will determine if FPGA or SW cleared it
    sfi_registers->mm.s.suspend_cfg.bf.enable = 0;
}

/*
 * The functions below are not used, only as reference. The scenario for thier use, updating and
 * slightly improving timing on reads by reducing the number of dummy cycles, brings with it a risk
 * of impeding the FPGA loading.
 * Scenario:
 * 0) Flash configured for lower clock cycles
 * 1) SPI DD HW configured the same
 * 2) Now operating with volatile changes
 * 3) System crashes or a ProgB (NOT Power cycle) happens
 * 4) Volatile configuration remains and FPGA can't load
 * 5) The Flash chip requires a power cycle - since no reset is available and it will remain in the
 * altered mode and not match SPI DD HW operation or default FPGA loading configuration
 */


//#################################################################################################
// Horrible look-up range-checking table function for returning dummy cycles for reads based on CPU
// frequency
// NOT USED as it currently used, as the values in the table do not work in reality
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint8_t SFI_getAhbReadDummyClockCycles(bool isQuadFastRead)
{
    uint32_t cpuFreq = bb_core_getCpuClkFrequency();
    uint32_t * p = SfiDummyClockCyclesFastRead;
    uint8_t dummyCycles = 1;
    uint8_t tableCount = sizeof(SfiDummyClockCyclesFastRead) / sizeof(SfiDummyClockCyclesFastRead[0]);

    if (isQuadFastRead)
    {
        tableCount = sizeof(SfiDummyClockCyclesQuadFastRead) / sizeof(SfiDummyClockCyclesQuadFastRead[0]);
        p = SfiDummyClockCyclesQuadFastRead;
    }
    if (cpuFreq <= *p)
    {
        return dummyCycles;
    }

    for (dummyCycles = 1; dummyCycles < tableCount; dummyCycles++)
    {
        if ( (*p < cpuFreq) && (cpuFreq <= *(p+1)) )
        {
            return dummyCycles;
        }
    }

    // Cpu > tabled valued? return 10.
    dummyCycles = 10;
    return dummyCycles;
}

//#################################################################################################
// Configure and Erase a Sector
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
void SFI_eraseSector(uint32_t sectorAddress)
{

    // Enter 4-byte address mode
    SFI_sendEnterExit4ByteAddressMode(true);
    SFI_sendWriteEnable();

    SFI_configureInstructionMaskGap(
        1, // number of cycles after chip select deasserted before new one can start (+3 automatically) -- reads 20ns (this can be 0), nonReads 50ns (this should be 1)
        0, // data lane mask
        1, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_4BYTE, // 3 or 4 byte address mode
        SFI_DATA_WRITE, // data read or write
        0, // current flash requires 0 dummy clocks on program cmd
        0, // length of data
        SFI_BLOCK_ERASE_64, // command
        (uint32_t)((uint8_t *)sectorAddress)// address in flash: code uses first block of flash
    ); // address

    // Remove CPU addressing (MSB) of address
    LEON_SFISendWrite(
        NULL, // data source
        0, // bytes to write
        true, // set GO bit after loading FIFO
        true); // wait on GO bit to be cleared - indicating transaction completed

}

//#################################################################################################
// Configure and for a page write to flash
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
void SFI_setupPageWrite(uint32_t address, uint32_t numBytes)
{

    // Enter 4-byte address mode
    SFI_sendEnterExit4ByteAddressMode(true);

    SFI_sendWriteEnable();

    // Configure for SFI PAGE PROGRAM
    SFI_configureInstructionMaskGap(
        1,  // number of cycles after chip select de-asserted before it can be re-asserted 20ns for reads, 50ns for non-read (FPGA has 3 already, so set to 0 for read, 1 for non-read)
        1,  // data lane mask
        1,  // addr lane mask,
        1); // cmd lane mask
    // If datasize is 0 then the fifo needs to be constantly filled
    // Once the fifo is empty, the transaction is considered completed
    // Also, if datasize is 0, words are written -- SFISendWrite will handle
    // this and when bytesLeftToWrite is < 4, it will return and force this function
    // to start a new transaction for the remaining bytes
    SFI_configureInstructionControl(
        SFI_ADDR_MODE_4BYTE, // 3 or 4 byte address mode
        SFI_DATA_WRITE, // data read or write
        0, // current flash requires 0 dummy clocks on program cmd
        (numBytes >= 4 ? 0 : numBytes), // length of data - write words where possible, if less than a word, write as separate transaction
        SFI_PAGE_PROG, // command
        address); // offset in flash to write to address
}

//#################################################################################################
// Wrapper to Exit 4-Byte Address Mode and clear suspend enable
//
// Parameters: 
// Return:
// Assumptions:
// * Values set are specific to the Flash chip and the command issued
// * This instruction is single lane and our chip doesn't support QUAD mode
//#################################################################################################
void SFI_clearSuspendGoto3ByteAddress(void)
{
    SFI_clearSuspendEnable();
    SFI_sendEnterExit4ByteAddressMode(false);
}

//#################################################################################################
// Wrapper to configure and issue Enter/Exit 4-Byte Address Mode
//
// Parameters: enter - true for 32 bit address mode, false for 24 bit
// Return:
// Assumptions:
// * Values set are specific to the Flash chip and the command issued
// * This instruction is single lane and our chip doesn't support QUAD mode
//#################################################################################################
static void SFI_sendEnterExit4ByteAddressMode(bool enter)
{
    SFI_sendWriteEnable();
    // Configure for WRite ENable
    SFI_configureInstructionMaskGap(
        1,  // number of cycles after chip select deasserted before can be asserted, 20ns for read, 50ns for non-read (0 for read, 1 for non-read, fpga adds 3 clock cycles)
        0,  // data lane mask
        0,  // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_WRITE,
        0, // current flash requires 0 dummy clocks on program cmd
        0,// date bytes to write/read
        enter ? SFI_4BYTE_ADDR_MODE_ENTER : SFI_4BYTE_ADDR_MODE_EXIT, // command
        0); // address

    // Enable the device for writing
    LEON_SFISendInstruction(); // sets go bit and waits on it

}


//#################################################################################################
// Configure and read from Volatile Configuration Register
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
uint8_t SFI_ReadU8Register(uint8_t cmdValue)
{
    // Configure for ReadStatus register (not generic read)
    SFI_configureInstructionMaskGap(
        0, // number of cycles after cmd to deassert signal line
        2, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        0, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_READ,
        0, // current flash requires 0 dummy clocks on program cmd
        1,// date bytes to write/read
        cmdValue, // command
        0); // address

    // Set and poll on Go and return data
    return LEON_SFISendReadStatus();

}

//#################################################################################################
// Configure and read from Volatile Configuration Register
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
uint16_t SFI_ReadU16Register(uint8_t cmdValue)
{
    // Configure for ReadStatus register (not generic read)
    SFI_configureInstructionMaskGap(
        0, // number of cycles after cmd to deassert signal line
        2, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        0, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_READ,
        0, // current flash requires 0 dummy clocks on program cmd
        2,// date bytes to write/read
        cmdValue, // command
        0); // address

    // Set and poll on Go and return data
    return LEON_SFISendReadStatus();

}

//#################################################################################################
// Configure and read from Volatile Configuration Register
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
uint32_t SFI_ReadU32Register(uint8_t cmdValue)
{
    // Configure for ReadStatus register (not generic read)
    SFI_configureInstructionMaskGap(
        0, // number of cycles after cmd to deassert signal line
        2, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        0, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_READ,
        0, // current flash requires 0 dummy clocks on program cmd
        4,// data bytes to write/read
        cmdValue, // command
        0); // address

    // Set and poll on Go and return data
    return LEON_SFISendReadStatus();

}

//#################################################################################################
// Configure and read from Volatile Configuration Register
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
uint32_t SFI_ReadDeviceId(void)
{
    // Configure for ReadStatus register (not generic read)
    SFI_configureInstructionMaskGap(
        0, // number of cycles after cmd to deassert signal line
        2, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        0, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_READ,
        0, // current flash requires 0 dummy clocks on program cmd
        4,// date bytes to write/read
        0x9F, // command
        0); // address

    // Set and poll on Go and return data
    return LEON_SFISendReadStatus();

}


//#################################################################################################
// Configure and read from Volatile Configuration Register
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
uint8_t SFI_readVolatileCfgRegister(void)
{
    // Configure for ReadStatus register (not generic read)
    SFI_configureInstructionMaskGap(
        0, // number of cycles after cmd to deassert signal line
        2, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        0, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_READ,
        0, // current flash requires 0 dummy clocks on program cmd
        1,// date bytes to write/read
        0x85, // command
        0); // address

    // Set and poll on Go and return data
    return LEON_SFISendReadStatus();

}

//#################################################################################################
// Configure for reading of the first status register
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
void SFI_writeVolatileCfgRegister(uint8_t val)
{
    // Configure for ReadStatus register (not generic read)
    SFI_configureInstructionMaskGap(
        0, // number of cycles after cmd to deassert signal line
        1, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        0, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_WRITE,
        0, // current flash requires 0 dummy clocks on program cmd
        1,// date bytes to write/read
        0x81, // command
        0); // address

    LEON_SFISendWrite((uint8_t*)(&val), 1, true, true);
}

//#################################################################################################
// Set MMU Address Offset - This applies to AHB reads
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void SFI_setMmuAddressOffset(uint32_t val)
{
    sfi_registers->mm.s.addr_offset.bf.val = val;
}

//#################################################################################################
// Set MMU Address Offset - This applies to AHB reads
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void SFI_setApbAddressOffset(uint32_t val)
{
    sfi_registers->addr_offset.bf.val = val;
}

//#################################################################################################
// Set MMU Address Offset - This applies to AHB reads
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static uint32_t SFI_FifoWordFill(const uint32_t *buffer, uint32_t size)
{
    uint32_t bytesWritten = 0;

    // send the data to SFI's FIFO, until we have less than 4 byte or the FIFO is full
    while ( (size >= 4) && (sfi_registers->out_fifo.bf.fifo_full == 0) )
    {
        sfi_registers->wr_data.bf.val = *buffer++; // write the data!
        bytesWritten +=4;
        size -= 4;
    }

    return (bytesWritten);
}

//#################################################################################################
// Set MMU Address Offset - This applies to AHB reads
//
// Parameters:
// Return:
// Assumptions: This code should run as fast as possible.
//              HW fifo keep sending data out to flash. It will stop if the HW fifo is empty
//#################################################################################################
static uint32_t SFI_FifoByteFill(const uint8_t *buffer, uint32_t size)
{
    uint32_t bytesWritten = 0;

    // Check if we set bytes number (fixed number writing) or not (finish writing when fifo is empty)
    if(sfi_registers->control.bf.num_bytes != 0)
    {
        // Fixed number writing. end byte boundary will be cared by HW
        while ( (size > bytesWritten) && (sfi_registers->out_fifo.bf.fifo_full == 0) )
        {
            uint32_t writeValue = 0;

            for (uint32_t byteCount = 0; (size > bytesWritten) && (byteCount < 4);
                    byteCount++, bytesWritten++)
            {
                writeValue  = writeValue << 8;
                writeValue |= buffer[bytesWritten];
            }
            sfi_registers->wr_data.bf.val = writeValue;
        }
    }
    else
    {
        // Non fixed number writing. HW don't know if the end boundary aligned with 4
        // Therefore, we write word amount and write remainer in a separate write
        while ( (size >= 4) && (sfi_registers->out_fifo.bf.fifo_full == 0) )
        {
            sfi_registers->wr_data.bf.val = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
            size-=4;
            bytesWritten += 4;
            buffer+=4;
        }
    }

    return (bytesWritten);
}

//#################################################################################################
// Configure and read from password location
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
void SFI_readPassword(uint8_t *data)
{
    uint32_t readVal;
    // Configure for ReadStatus register (not generic read)
    SFI_configureInstructionMaskGap(
        0, // number of cycles after cmd to deassert signal line
        2, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        0, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_READ,
        0, // current flash requires 0 dummy clocks on program cmd
        8,// date bytes to write/read
        SFI_READ_PASSWORD, // command
        0); // address

    // Set and poll on Go and return data
    readVal = LEON_SFISendReadStatus();
    *(uint32_t *)data = readVal;
    *(uint32_t *)(data + 4) = sfi_registers->rd_data.bf.val;

}

//#################################################################################################
// Configure and write from password location
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
void SFI_writePassword(uint8_t *data)
{
    // Configure for ReadStatus register (not generic read)
    SFI_configureInstructionMaskGap(
        0, // number of cycles after cmd to deassert signal line
        2, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        0, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_WRITE,
        0, // current flash requires 0 dummy clocks on program cmd
        8,// date bytes to write/read
        SFI_WRITE_PASSWORD, // command
        0); // address

    // Set and poll on Go and return data
    LEON_SFISendWrite(data, 8, true, true);

}
//#################################################################################################
// Wrapper to configure and issue Write Enable
//
// Parameters:
// Return:
// Assumptions:
// * Values set are specific to the Flash chip and the command issued
// * This instruction is single lane and our chip doesn't support QUAD mode
//#################################################################################################
static void SFI_sendWriteEnable(void)
{
    // Configure for write enable
    SFI_configureInstructionMaskGap(
        1,  // number of cycles after chip select de-asserted before can be asserted, 20ns for read, 50ns for non-read (0 for read, 1 for non-read, fpga adds 3 clock cycles)
        0,  // data lane mask
        0,  // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_WRITE,
        0, // current flash requires 0 dummy clocks on program cmd
        0,// date bytes to write/read
        SFI_WREN, // command
        0); // address

    // Enable the device for writing
    LEON_SFISendInstruction(); // sets go bit and waits on it

}
//#################################################################################################
// Wrapper to configure and issue ReadStatusFlags
//
// Parameters:
// Return: StatusFlags
// Assumptions:
// * Values set are specific to the Flash chip and the command issued
// * This instruction is single lane and our chip doesn't support QUAD mode
//#################################################################################################
uint8_t SFI_readStatusFlags(void)
{
    // Configure for readStatusFlags
    SFI_configureInstructionMaskGap(
        0,  // number of cycles after chip select de-asserted before can be asserted, 20ns for read, 50ns for non-read (0 for read, 1 for non-read, fpga adds 3 clock cycles)
        2,  // data lane mask
        0,  // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_READ,
        0, // current flash requires 0 dummy clocks on program cmd
        1,// date bytes to write/read
        SFI_READ_FLAG_STATUS, // command
        0); // address

    // Enable the device for writing
    return LEON_SFISendReadStatus(); // sets go bit and waits on it

}

//#################################################################################################
// Wrapper to configure and issue ReadStatusFlags
//
// Parameters:
// Return: StatusFlags
// Assumptions:
// * Values set are specific to the Flash chip and the command issued
// * This instruction is single lane and our chip doesn't support QUAD mode
//#################################################################################################
uint8_t SFI_readStatusRegister(void)
{
    // Configure for readStatusReg
    SFI_configureInstructionMaskGap(
        0,  // number of cycles after chip select de-asserted before can be asserted, 20ns for read, 50ns for non-read (0 for read, 1 for non-read, fpga adds 3 clock cycles)
        2,  // data lane mask
        0,  // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_READ,
        0, // current flash requires 0 dummy clocks on program cmd
        1,// date bytes to write/read
        SFI_RDSR, // command
        0); // address

    // Enable the device for writing
    return LEON_SFISendReadStatus(); // sets go bit and waits on it

}

//#################################################################################################
// Wrapper to configure and issue clearStatusFlags
//
// Parameters:
// Return: StatusFlags
// Assumptions:
// * Values set are specific to the Flash chip and the command issued
// * This instruction is single lane and our chip doesn't support QUAD mode
//#################################################################################################
void SFI_clearStatusFlags(void)
{
    // Configure for readStatusReg
    SFI_configureInstructionMaskGap(
        0,  // number of cycles after chip select de-asserted before can be asserted, 20ns for read, 50ns for non-read (0 for read, 1 for non-read, fpga adds 3 clock cycles)
        0,  // data lane mask
        0,  // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_READ,
        0, // current flash requires 0 dummy clocks on program cmd
        0,// date bytes to write/read
        SFI_CLEAR_FLAG_STATUS, // command
        0); // address

    
    LEON_SFISendInstruction(); // Issue the command

}


//#################################################################################################
// Reset the flash device back to a known condition
//
// Parameters:
// Return: none
// Assumptions:
// * Values set are specific to the Flash chip and the command issued
// * This instruction is single lane and our chip doesn't support QUAD mode
//#################################################################################################
void SFI_ResetFlash(void)
{
    // Configure for readStatusReg
    SFI_configureInstructionMaskGap(
        0,  // number of cycles after chip select de-asserted before can be asserted, 20ns for read, 50ns for non-read (0 for read, 1 for non-read, fpga adds 3 clock cycles)
        0,  // data lane mask
        0,  // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_WRITE,
        0, // current flash requires 0 dummy clocks on program cmd
        0,// date bytes to write/read
        SFI_RESET_ENABLE, // command
        0); // address

    LEON_SFISendInstruction(); // Issue the command

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_WRITE,
        0, // current flash requires 0 dummy clocks on program cmd
        0,// date bytes to write/read
        SFI_RESET_FLASH, // command
        0); // address

    LEON_SFISendInstruction(); // Issue the command
}



//#################################################################################################
// Write volatileLockBits location
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
void SFI_writeVolatileLockBits(uint8_t lockBits, uint32_t address)
{
    SFI_sendWriteEnable();
    // Configure for ReadStatus register (not generic read)
    SFI_configureInstructionMaskGap(
        0, // number of cycles after cmd to deassert signal line
        1, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        1, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_WRITE,
        0, // current flash requires 0 dummy clocks on program cmd
        1,// date bytes to write/read
        SFI_WRITE_VOLATILE_LOCK, // command
        address); // address

    // Set and poll on Go and return data
    LEON_SFISendWrite(&lockBits, 1, true, true);

}

//#################################################################################################
// Read volatileLockBits location
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
uint32_t SFI_ReadVolatileLockBits(uint32_t address)
{
    // Configure for ReadStatus register (not generic read)
    SFI_configureInstructionMaskGap(
        0, // number of cycles after cmd to deassert signal line
        2, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        1, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_READ,
        0, // current flash requires 0 dummy clocks on program cmd
        2,// date bytes to write/read
        SFI_READ_VOLATILE_LOCK, // command
        address); // address

    // Set and poll on Go and return data
    return LEON_SFISendReadStatus();

}

//#################################################################################################
// Write sector protection location
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
void SFI_writeSectorProtectionBits(uint16_t lockBits)
{
    SFI_sendWriteEnable();

    // Configure for ReadStatus register (not generic read)
    SFI_configureInstructionMaskGap(
        0, // number of cycles after cmd to deassert signal line
        1, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        0, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_WRITE,
        0, // current flash requires 0 dummy clocks on program cmd
        2,// date bytes to write/read
        SFI_PROG_SECTOR_PROTECT, // command
        0); // address

    // Set and poll on Go and return data
    LEON_SFISendWrite((uint8_t*)(&lockBits), 2, true, true);

}


//#################################################################################################
// Write nonvolatileLockBits location
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
void SFI_writeNonVolatileLockBits(uint32_t address)
{
    SFI_sendWriteEnable();

    // Configure for ReadStatus register (not generic read)
    SFI_configureInstructionMaskGap(
        0, // number of cycles after cmd to deassert signal line
        0, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        1, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_4BYTE,
        SFI_DATA_WRITE,
        0, // current flash requires 0 dummy clocks on program cmd
        0,// date bytes to write/read
        SFI_WRITE_NON_VOLATILE_LOCK, // command
        address); // address

    // Set and poll on Go and return data
    LEON_SFISendInstruction(); // sets go bit and waits on it
//    LEON_SFISendWrite((uint8_t*)(&lockBits), 2, true, true);

}

//#################################################################################################
// Read nonvolatileLockBits location
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
uint32_t SFI_ReadNonVolatileLockBits(uint32_t address)
{
    // Configure for ReadStatus register (not generic read)
    SFI_configureInstructionMaskGap(
        0, // number of cycles after cmd to deassert signal line
        2, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        1, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_4BYTE,
        SFI_DATA_READ,
        0, // current flash requires 0 dummy clocks on program cmd
        2,// date bytes to write/read
        SFI_READ_NON_VOLATILE_LOCK, // command
        address); // address

    // Set and poll on Go and return data
    return LEON_SFISendReadStatus();

}

//#################################################################################################
// Erase nonvolatileLockBits location
//
// Parameters:
// Return:
// Assumptions:
// * This instruction is configured for single lane and our chip doesn't support QUAD mode
//#################################################################################################
bool SFI_EraseNonVolatileLockBits(void)
{
    SFI_sendWriteEnable();

    SFI_configureInstructionMaskGap(
        1, // number of cycles after cmd to deassert signal line
        0, // data lane mask - read status comes in on DQ1, but cmd goes out on DQ0
        0, // addr lane mask,
        1); // cmd lane mask

    SFI_configureInstructionControl(
        SFI_ADDR_MODE_3BYTE,
        SFI_DATA_WRITE,
        0, // current flash requires 0 dummy clocks on program cmd
        0,// date bytes to write/read
        SFI_ERASE_NON_VOLATILE_LOCK, // command
        0); // address

    LEON_SFISendInstruction();
    return true;

}

