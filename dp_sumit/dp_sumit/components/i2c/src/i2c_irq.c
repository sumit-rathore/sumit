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
//!   @file  - bgrg_irq.c
//
//!   @brief - contains the entry point for I2C interrupts
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "i2c_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

void I2c_InterruptHandler(void)
{

    const uint32_t status = i2c_registers->irq.s.pending.dw & i2c_registers->irq.s.enable.dw;

    // Write 1 to clear each of the interrupts in the status int_status register

    if ((status & I2C_MASTER_IRQ_ENABLE_FIFO_AE) == I2C_MASTER_IRQ_ENABLE_FIFO_AE)
    {
        i2c_registers->irq.s.pending.bf.fifo_ae = 1;
        I2C_AlmostEmptyIrq();
    }

    if ((status & I2C_MASTER_IRQ_ENABLE_FIFO_AF) == I2C_MASTER_IRQ_ENABLE_FIFO_AF)
    {
        i2c_registers->irq.s.pending.bf.fifo_af = 1;
        I2C_AlmostFullIrq();
    }

    if ((status & I2C_MASTER_IRQ_ENABLE_DONE) == I2C_MASTER_IRQ_ENABLE_DONE)
    {
        i2c_registers->irq.s.pending.bf.done = 1;
        I2C_DoneIrq();
    }
}

