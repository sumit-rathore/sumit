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
//!   @file  -  sfi.h
//
//!   @brief -
//
//
//!   @note  -  These API assume only a single thread is accessing flash, so no
//              locking is done.  If multiple threads are accessing flash, the
//              caller is responsible for implementing a locking mechanism
//
///////////////////////////////////////////////////////////////////////////////
#ifndef SFI_H
#define SFI_H

/***************************** Included Headers ******************************/
#include <itypes.h>

/************************ Defined Constants and Macros ***********************/
#define SFI_CONTROL_FIFO_SIZE 4        //the fifo can hold 4 words or 16 bytes at a time

// Serial flash instructions
// IMPORTANT!!! DO NOT ADD A READ COMMAND (0x03).  Certain Icron qualified flash
// chips process the read command at less than 60MHz, and it won't work with
// 60MHz USB designs.  Icron uses 60MHz as this is 1/8 of USB2.0 480Mbps
// Flash instructions
#define SFI_WREN                0x06
#define SFI_RDSR                0x05
#define SFI_FAST_READ           0x0B
#define SFI_PAGE_PROG           0x02
#define SFI_CHIP_ERASE_S        0xC7
#define SFI_WRITE_STATUS_REG_1  0x01
#define SFI_WRITE_STATUS_REG_2  0x31
#define SFI_WRITE_STATUS_REG_3  0x11
#define SFI_READ_STATUS_REG_1   0x05
#define SFI_READ_STATUS_REG_2   0x35
#define SFI_READ_STATUS_REG_3   0x15
#define SFI_BLOCK_ERASE_64      0xD8
#define SFI_WRITE_STATUS_LEN    0x01
#define SFI_4BYTE_ADDR_MODE_ENTER 0xB7
#define SFI_4BYTE_ADDR_MODE_EXIT  0xE9

#define SFI_BLOCK_ERASE_4       0xD8
#define SFI_READ_FLAG_STATUS    0x70
#define SFI_CLEAR_FLAG_STATUS    0x50

#define SFI_PROG_BUSY_FLAG       0x80
#define SFI_ERASE_SUSPENDED_FLAG 0x40
#define SFI_ERASE_FAILURE_FLAG 0x20
#define SFI_PROG_SUSPENDED_FLAG  0x04
#define SFI_4BYTE_ADDR_FLAG      0x01

#define SFI_READ_SECTOR_PROTECT 0x2D
#define SFI_PROG_SECTOR_PROTECT 0x2C

#define SFI_READ_VOLATILE_LOCK  0xE8
#define SFI_WRITE_VOLATILE_LOCK 0xE5

#define SFI_READ_NON_VOLATILE_LOCK  0xE2
#define SFI_WRITE_NON_VOLATILE_LOCK 0xE3
#define SFI_ERASE_NON_VOLATILE_LOCK 0xE4

#define SFI_READ_GLOBAL_FREEZE  0xA7
#define SFI_WRITE_GLOBAL_FREEZE 0xA6

#define SFI_READ_PASSWORD       0x27
#define SFI_WRITE_PASSWORD      0x28
#define SFI_UNLOCK_PASSWORD     0x29

#define SFI_RESET_ENABLE        0x66
#define SFI_RESET_FLASH         0x99


#define SFI_CRC_RUN             0x9B

#define SFI_4B_BLOCK_ERASE_64   0xDC
#define SFI_4B_BLOCK_ERASE_4    0x21
#define SFI_4B_PAGE_PROG        0x12

#define SFI_4B_READ_VOLATILE_LOCK  0xE0
#define SFI_4B_WRITE_VOLATILE_LOCK 0xE1

#define SFI_SECTOR_PROTECT_BIT   1
#define SFI_SECTOR_LOCK_DOWN_BIT 2
#define SFI_NONVOLATILE_PROTECT_PASSWORD_DISABLED 0xFFFD
#define SFI_NONVOLATILE_PROTECT_DISABLED 2

// Flash general definitions
#define SFI_BUSY_MASK               (1 << 0)    // Write & Erase in progress
#define SFI_PROG_FAILURE_FLAG       0x10    // Micron only
#define SFI_FAIL_ON_PROT_FLAG       0x02    // Micron only
#define SFI_FLASH_SECTOR_SIZE       0x10000

/******************************** Data Types *********************************/
typedef enum {
    LEON_FLASH_1_BYTE_DATA = 0x00,
    LEON_FLASH_2_BYTE_DATA = 0x08,
    LEON_FLASH_3_BYTE_DATA = 0x10,
    LEON_FLASH_4_BYTE_DATA = 0x18
} LEON_FlashDataLengthT;

enum SfiAddrMode
{
    SFI_ADDR_MODE_3BYTE, // 24-bit addressing into flash
    SFI_ADDR_MODE_4BYTE  // 32-bit addressing into flash
};

enum SfiDataReadWrite
{
    SFI_DATA_READ,
    SFI_DATA_WRITE
};

/*********************************** API *************************************/

// send instruction & wait for completion
//eg, Write enable, erase
void LEON_SFISendInstruction(void) __attribute__ ((section(".ftext")));

// send instruction, wait for completion, and grab data to return
//eg, read status status
uint32_t LEON_SFISendReadStatus(void) __attribute__ ((section(".ftext")));

// send instruction
//eg, send read fast read command
void LEON_SFISendReadInstruction(uint8_t SFIInstruction, LEON_FlashDataLengthT) __attribute__ ((section(".ftext")));

//sends instruction and data (with write mask)
uint32_t LEON_SFISendWrite
(
    const uint8_t * buf,          //the data to write
    const uint32_t bufSize,       //the number of bytes in buf
    const bool setGoBit,          // set GO bit to start transaction
    const bool waitOnGoBit        // poll on go bit until transaction completed
) __attribute__ ((section(".ftext")));

// sends instruction and data (with instruction mask)
void LEON_SFISendInstructionData(uint32_t data) __attribute__ ((section(".ftext")));

void LEON_SFIInit (bool isCurrentImage, bool isAsic);
uint8_t SFI_readStatusRegister(void);
uint8_t SFI_readStatusFlags(void);
void SFI_clearStatusFlags(void);

void SFI_eraseSector(uint32_t sectorAddress);
void SFI_clearSuspendGoto3ByteAddress(void);
void SFI_setupPageWrite(uint32_t address, uint32_t numBytes);


void SFI_configureInstructionMaskGap(uint16_t gap, uint8_t dataMask, uint8_t addrMask, uint8_t cmdMask);
void SFI_configureInstructionControl(
    uint8_t addrMode,
    uint8_t readWriteData,
    uint8_t numDummyClks,
    uint8_t numDataBytes,
    uint8_t command,
    uint32_t flashOffset);

void SFI_configureSuspendOnReadOperation(void);
bool SFI_isTransactionInFlight(void);
void SFI_configureAhbReadControl(void);
bool SFI_isSuspendEnableSet(void);
void SFI_clearSuspendEnable(void);
uint8_t SFI_getAhbReadDummyClockCycles(bool isQuadFastRead);
uint8_t SFI_readVolatileCfgRegister(void);

uint8_t  SFI_ReadU8Register(uint8_t cmdValue);
uint16_t SFI_ReadU16Register(uint8_t cmdValue);
uint32_t SFI_ReadU32Register(uint8_t cmdValue);

uint32_t SFI_ReadDeviceId(void);
void SFI_ResetFlash(void);
void SFI_writeVolatileCfgRegister(uint8_t val);
void SFI_setMmuAddressOffset(uint32_t val);
void SFI_setApbAddressOffset(uint32_t val);

void SFI_readPassword(uint8_t *data);
void SFI_writePassword(uint8_t *data);

void SFI_writeVolatileLockBits(uint8_t lockBits, uint32_t address);
uint32_t SFI_ReadVolatileLockBits(uint32_t address);

bool SFI_EraseNonVolatileLockBits(void);
uint32_t SFI_ReadNonVolatileLockBits(uint32_t address);
void SFI_writeNonVolatileLockBits(uint32_t address);


#endif // SFI_H




