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
// Configuration of jitter chip
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################


// Includes #######################################################################################
#include <ilog.h>
#include <ibase.h>
#include <i2c.h>
#include <i2cd_si5326.h>

#include "i2cd_log.h"
#include "i2cd_si5326cfg.h"

// Constants and Macros ###########################################################################
#define PLL_SI5326_DEJITTER_CHIP_ADDRESS                (0x68)
#define PLL_SI5326_DEJITTER_CHIP_SPEED                  I2C_SPEED_SLOW
#define PLL_SI5326_DEJITTER_CHIP_ICAL_ADDRESS           (0x88)
#define PLL_SI5326_DEJITTER_CHIP_ICAL_MASK              (0x40)
#define PLL_SI5326_DEJITTER_CHIP_ICAL_OFFSET            (0x6)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_LOS_FLG_ADDRESS    (0x83)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_LOS_MSK            (0x07)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_MSK            (0x0E)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_LOS2_MSK           (0x04)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_LOS2_OFFSET        (0x02)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_LOS1_MSK           (0x02)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_LOS1_OFFSET        (0x01)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_LOSX_MSK           (0x01)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_LOSX_OFFSET        (0x00)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_FOS2_MSK       (0x08)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_FOS2_OFFSET    (0x03)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_FOS1_MSK       (0x04)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_FOS1_OFFSET    (0x02)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_LOL_MSK        (0x02)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_LOL_OFFSET     (0x01)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_LOS_CLEARED        (0x18)
#define PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_CLEARED        (0x00)

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct
{
    void (*SetupDeJitterCompletionHandler)(void);
    void (*IrqDeJitterCompletionHandler)(void);
    // Variable to keep track of DeJitterInit array index
    uint8_t currentRegIndex;
    // Data buffer for I2C reads and writes.
    // **WARNING**: this buffer must be sized according the the largest read or write that this
    //              module does.
    uint8_t transactionBuffer[3];
} _SI5326;

// Static Function Declarations ###################################################################
const struct I2cDevice i2cDeviceSi5326 =
{
    .deviceAddress = PLL_SI5326_DEJITTER_CHIP_ADDRESS,
    .speed = PLL_SI5326_DEJITTER_CHIP_SPEED,
    .port = KC705_MUX_SI5326
};

static void _I2CD_deJitterChipIrqHandlerReadFlags(uint8_t* data, uint8_t byteCount);
static void _I2CD_deJitterChipIrqHandlerClearFlags(bool success);

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Setup dejitter chip
//
// Parameters:
//      handle              - i2c handle for device through mux
//      interface           - access to i2c functions
//      notifyWriteCompleteHandler - call back when configuration completes
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void I2CD_setupDeJitterChip(void (*notifyInterruptClearWriteCompleteHandler)(void))
{
    // Once we enable interrupts, after configuring this device, it will trigger so we setup our
    // interrupt handle here
    if (notifyInterruptClearWriteCompleteHandler != NULL)
    {
        _SI5326.IrqDeJitterCompletionHandler = notifyInterruptClearWriteCompleteHandler;
    }

    // Write configuration to the chip
    for (   _SI5326.currentRegIndex = 0;
            _SI5326.currentRegIndex < getDeJitterInitLength();
            (_SI5326.currentRegIndex)++ )
    {
        I2C_WriteBlocking(
            &i2cDeviceSi5326,
            (uint8_t *)&deJitterInit[_SI5326.currentRegIndex],
            sizeof(deJitterInit[_SI5326.currentRegIndex]));
    }

    // check for chip initialzation done bit set
    {
        uint8_t readCount = 0;
        const uint8_t writeByteCount = 1;
        const uint8_t readByteCount = 1;
        uint8_t transactionBuffer[1];
        do
        {
            transactionBuffer[0] = PLL_SI5326_DEJITTER_CHIP_ICAL_ADDRESS;

            I2C_WriteReadBlocking(
                &i2cDeviceSi5326,
                transactionBuffer,
                writeByteCount,
                readByteCount);
            readCount++;
        }
        while ( (readCount < 100) &&
                (   (transactionBuffer[0] & PLL_SI5326_DEJITTER_CHIP_ICAL_MASK)
                >>  PLL_SI5326_DEJITTER_CHIP_ICAL_OFFSET) != 0 );

        // Arbitrarily choose 100 times as the maximum number of times to poll.
        iassert_I2CD_COMPONENT_1(readCount < 100, DEJITTER_CHIP_FAILED_CALIBRATION, readCount);
    }
}


//#################################################################################################
// Issue a writeRead to the deJitter chip interrupt registers, 131 and 132
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void I2CD_deJitterIrqHandler(void)
{
    // writeRead 2 bytes from 131
    const uint8_t writeByteCount = 1;
    const uint8_t readByteCount = 2;

    _SI5326.transactionBuffer[0] = PLL_SI5326_DEJITTER_CHIP_IRQ_LOS_FLG_ADDRESS;

    I2C_WriteReadAsync(
        &i2cDeviceSi5326,
        _SI5326.transactionBuffer,
        writeByteCount,
        readByteCount,
        _I2CD_deJitterChipIrqHandlerReadFlags);
}


//#################################################################################################
// Checks read flag register values then issues write to clear all flags
//
// Parameters:
//      data                - pointer to returned data buffer
//      byteCount           - number of bytes read back
// Return:
// Assumptions:
//#################################################################################################
static void _I2CD_deJitterChipIrqHandlerReadFlags(uint8_t* data, uint8_t byteCount)
{
    _SI5326.transactionBuffer[0] = PLL_SI5326_DEJITTER_CHIP_IRQ_LOS_FLG_ADDRESS;
    _SI5326.transactionBuffer[1] = PLL_SI5326_DEJITTER_CHIP_IRQ_LOS_CLEARED;
    _SI5326.transactionBuffer[2] = PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_CLEARED;

    iassert_I2CD_COMPONENT_0((data != NULL && byteCount == 2), DEJITTER_CHIP_FAILED_IRQ_READ);

    {
        const uint8_t writeByteCount = 3;
        // Register 131 LOS2, LOS1, LOSX FLGs
        uint8_t val = data[0] & PLL_SI5326_DEJITTER_CHIP_IRQ_LOS_MSK;

        if ( ((val & PLL_SI5326_DEJITTER_CHIP_IRQ_LOS2_MSK) >>
            PLL_SI5326_DEJITTER_CHIP_IRQ_LOS2_OFFSET) == 0x1)
        {
            ilog_I2CD_COMPONENT_0(ILOG_MAJOR_EVENT, DEJITTER_CHIP_IRQ_LOS2);
        }
        if ( ((val & PLL_SI5326_DEJITTER_CHIP_IRQ_LOS1_MSK) >>
            PLL_SI5326_DEJITTER_CHIP_IRQ_LOS1_OFFSET) == 0x1)
        {
            ilog_I2CD_COMPONENT_0(ILOG_MAJOR_EVENT, DEJITTER_CHIP_IRQ_LOS1);
        }
        if ( ((val & PLL_SI5326_DEJITTER_CHIP_IRQ_LOSX_MSK) >>
            PLL_SI5326_DEJITTER_CHIP_IRQ_LOSX_OFFSET) == 0x1)
        {
            ilog_I2CD_COMPONENT_0(ILOG_MAJOR_EVENT, DEJITTER_CHIP_IRQ_LOSX);
        }

        // Register 132 FOS2, FOS1, LOL FLGs
        val = data[1] & PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_MSK;

        if ( ((val & PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_FOS2_MSK) >>
            PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_FOS2_OFFSET) == 0x1)
        {
            ilog_I2CD_COMPONENT_0(ILOG_MAJOR_EVENT, DEJITTER_CHIP_IRQ_FOS2);
        }
        if ( ((val & PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_FOS1_MSK) >>
            PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_FOS1_OFFSET) == 0x1)
        {
            ilog_I2CD_COMPONENT_0(ILOG_MAJOR_EVENT, DEJITTER_CHIP_IRQ_FOS1);
        }
        if ( ((val & PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_LOL_MSK) >>
            PLL_SI5326_DEJITTER_CHIP_IRQ_FLG_LOL_OFFSET) == 0x1)
        {
            ilog_I2CD_COMPONENT_0(ILOG_MAJOR_EVENT, DEJITTER_CHIP_IRQ_LOL);
        }

        I2C_WriteAsync(
            &i2cDeviceSi5326,
            _SI5326.transactionBuffer,
            writeByteCount,
            _I2CD_deJitterChipIrqHandlerClearFlags);
    }
}


//#################################################################################################
// Call dejitter intterupt completion handler, which should re-enable int_alm
//
// Parameters:
//      success             - indicates if setup passed or failed writing to dejitter for writing
//                            to calibration
// Return:
// Assumptions:
//#################################################################################################
static void _I2CD_deJitterChipIrqHandlerClearFlags(bool success)
{
    iassert_I2CD_COMPONENT_0(success, DEJITTER_CHIP_FAILED_IRQ_WRITE);
    (*(_SI5326.IrqDeJitterCompletionHandler))();
}

// Static Function Definitions ####################################################################

