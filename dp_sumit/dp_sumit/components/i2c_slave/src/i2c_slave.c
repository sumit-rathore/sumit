//#################################################################################################
// Icron Technology Corporation - Copyright 2018
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
// i2c_slave driver
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <leon_timers.h>
//#include <tasksch.h>
#include <ififo.h>
#include <module_addresses_regs.h>
#include <interrupts.h>
#include <uart.h>
#include <i2c_slave_regs.h>
#include <i2c_slave.h>
#include "i2c_slave_loc.h"
#include "i2c_slave_log.h"
#include <bb_core.h>
#include <bb_top.h>
// #include <i2cd_switch.h>

// // Constants and Macros ###########################################################################
#define I2C_SLAVE_FIFO_AFT                  (0x16)   // 32 - this value is almost full FIFO LEVELS
#define I2C_SLAVE_FIFO_AET                  (0x0)

// // Data Types #####################################################################################

// // Global Variables ###############################################################################
// // Static Variables ###############################################################################
volatile i2c_slave_s* i2c_slave_registers;

#ifdef USE_I2C_SLAVE
static struct
{
    uint8_t hw_in_fifo_size;
    uint8_t hw_out_fifo_size;
    uint8_t command;
    uint8_t readWriteCount;
    uint8_t incommingBuffer[256];
    bool processingCommand;
	bool transactionComplete;

}i2c_slave_context;


// /************************ Local Function Prototypes **************************/
// IFIFO_CREATE_FIFO_LOCK_UNSAFE(i2c, struct i2c_operation, I2C_OPERATIONS_FIFO_SIZE)


// // Static Function Declarations ###################################################################
static void I2C_Slave_ReadCommand(void)                 __attribute__ ((section(".ftext")));
static void I2C_Slave_WriteFifoBytes(const char *data, uint16_t size)                 __attribute__ ((section(".ftext")));
#endif



// Exported Function Definitions ##################################################################
//#################################################################################################
// I2C_SlaveInit
//
// Parameters: None
// Return: None
// Assumptions:
//
//#################################################################################################
void I2C_SlaveInit()
{
    i2c_slave_registers = (volatile i2c_slave_s*) bb_chip_i2c_slave_s_ADDRESS;
#ifdef USE_I2C_SLAVE
    bb_top_ResetI2CSlave(false);      // FPGA I2C block out of reset

    ilog_I2C_SLAVE_COMPONENT_3(ILOG_DEBUG, I2C_VERSION1_DATA, (uint8_t)i2c_slave_registers->version.bf.major, (uint8_t)i2c_slave_registers->version.bf.minor,(uint8_t)i2c_slave_registers->version.bf.patch);
    // i2c_slave is clocked at processor speed 75MHZ
    // Set I2C address
    i2c_slave_registers->slave_config.bf.slave_addr = (uint32_t)I2C_SLAVE_ADDR;

    // Filters glitches upto 1 + this number
    // glitch is a value between 0-31
    i2c_slave_registers->slave_config.bf.scl_glitch = (uint32_t)3;
    i2c_slave_registers->slave_config.bf.sda_glitch = (uint32_t)3;

    i2c_slave_registers->slave_config.bf.start_clks = (uint32_t)8;
    i2c_slave_registers->slave_config.bf.stop_clks = (uint32_t)8;

    // timeout is a 32 bit value when set to 0 it is disabled
    i2c_slave_registers->timeout.bf.val = 0;

    // Must be set in order to acknowledge the address
    i2c_slave_registers->slave_config.bf.slave_addr_ack = true;

    i2c_slave_registers->slave_config.bf.clock_stretch_en = true;


    // Initialize interrupt
    i2c_slave_registers->irq.s.enable.dw = I2C_SLAVE_IRQ_ENABLE_NEW_COMMAND |
      //                                     I2C_SLAVE_IRQ_ENABLE_OUT_FIFO_AE |
										   I2C_SLAVE_IRQ_ENABLE_DEVICE_BUSY |
                                           I2C_SLAVE_IRQ_ENABLE_IN_FIFO_AF;

    // After enabling interrupts clear any pending interrupts
    i2c_slave_registers->irq.s.pending.dw = i2c_slave_irq_pending_WRITEMASK;

    // Initialize I2C fifo

    i2c_slave_context.hw_out_fifo_size = i2c_slave_registers->out_fifo.bf.fifo_depth;
    i2c_slave_context.hw_in_fifo_size = i2c_slave_registers->in_fifo.bf.fifo_depth;


    i2c_slave_registers->in_fifo.bf.fifo_aft = i2c_slave_context.hw_in_fifo_size - 4;
    i2c_slave_registers->in_fifo.bf.fifo_aet = I2C_SLAVE_FIFO_AET;

    i2c_slave_registers->out_fifo.bf.fifo_aft = i2c_slave_context.hw_out_fifo_size - 4;
    i2c_slave_registers->out_fifo.bf.fifo_aet = I2C_SLAVE_FIFO_AET;

    i2c_slave_context.processingCommand = false;
    i2c_slave_context.transactionComplete = false;

//    ilog_I2C_SLAVE_COMPONENT_1(ILOG_DEBUG, I2C_SLAVE_BUSY, (uint8_t)i2c_slave_registers->status.bf.device_busy);
	i2c_slave_registers->status.bf.device_busy = (uint32_t)0;
//    ilog_I2C_SLAVE_COMPONENT_1(ILOG_DEBUG, I2C_SLAVE_BUSY, (uint8_t)i2c_slave_registers->status.bf.device_busy);
//    I2C_Slave_Test_Init();
    TOPLEVEL_setPollingMask(SECONDARY_INT_I2C_SLAVE_INT_MSK);
#endif
}

//#################################################################################################
// I2C_Slave_InterruptHandler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void I2C_Slave_InterruptHandler(void)
{
#ifdef USE_I2C_SLAVE
    if (i2c_slave_registers->irq.s.pending.bf.device_busy)
    {

		while(i2c_slave_registers->in_fifo.bf.fifo_empty == 0) // Clear out the fifo
		{
			i2c_slave_context.incommingBuffer[i2c_slave_context.readWriteCount] = i2c_slave_registers->rd_data.bf.rd_data;
			i2c_slave_context.readWriteCount++;
		}

		i2c_slave_context.transactionComplete = true;
		i2c_slave_registers->status.bf.device_busy = (uint32_t)0;

		ilog_I2C_SLAVE_COMPONENT_0(ILOG_DEBUG, I2C_TRANSACTION_COMPLETE);
		I2C_Slave_WriteFifoBytes("123456789ABCDEFGHIJK",20);
        i2c_slave_registers->irq.s.pending.bf.device_busy = 1; // Clear the interrupt
    }
#if 0
    if (i2c_slave_registers->irq.s.pending.bf.out_fifo_ae)
    {
        i2c_slave_registers->irq.s.pending.bf.out_fifo_ae = 1;
    }
#endif
    if (i2c_slave_registers->irq.s.pending.bf.in_fifo_af)
    {

		while(i2c_slave_registers->in_fifo.bf.fifo_empty == 0) // Clear out the fifo
		{
			i2c_slave_context.incommingBuffer[i2c_slave_context.readWriteCount] = i2c_slave_registers->rd_data.bf.rd_data;
			i2c_slave_context.readWriteCount++;
		}

        i2c_slave_registers->irq.s.pending.bf.in_fifo_af = 1;
    }

    if (i2c_slave_registers->irq.s.pending.bf.in_fifo_ae)
    {
		while(i2c_slave_registers->in_fifo.bf.fifo_empty == 0) // Clear out the fifo
		{
			i2c_slave_context.incommingBuffer[i2c_slave_context.readWriteCount] = i2c_slave_registers->rd_data.bf.rd_data;
			i2c_slave_context.readWriteCount++;
		}

        i2c_slave_registers->irq.s.pending.bf.in_fifo_ae = 1;
    }

    if (i2c_slave_registers->irq.s.pending.bf.new_command) // A new command occurs when the host writes to this device...
	                                                       // the command is defined in the controller software
    {
        i2c_slave_registers->irq.s.pending.bf.new_command = 1;
        I2C_Slave_ReadCommand();
    }
#endif
}

// Component Scope Function Definitions ###########################################################


// Static Function Definitions ####################################################################
#ifdef USE_I2C_SLAVE
//#################################################################################################
// I2C_Slave_ReadCommand
//    Takes the command from the slave component - this describes the transaction...as it pertains to
//    the software
// Parameters: None
// Return: None
// Assumptions:
//
//#################################################################################################
static void I2C_Slave_ReadCommand(void)
{
    i2c_slave_context.readWriteCount = 0;       // Get new command
    i2c_slave_context.command = i2c_slave_registers->command.dw;
    i2c_slave_context.processingCommand = true;

    ilog_I2C_SLAVE_COMPONENT_1(ILOG_DEBUG, I2C_READ_COMMAND, i2c_slave_context.command);
#if 0
    while(i2c_slave_registers->in_fifo.bf.fifo_empty == 0)
    {
        const uint8_t dataByteRead = i2c_slave_registers->rd_data.bf.rd_data; // Remove the command from the buffer
        ilog_I2C_SLAVE_COMPONENT_2(ILOG_DEBUG, I2C_READ_DATA, dataByteRead, i2c_slave_registers->in_fifo.bf.fifo_depth);

        i2c_slave_context.incommingBuffer[i2c_slave_context.readWriteCount] = dataByteRead;
        i2c_slave_context.readWriteCount++;
    }
#endif
}

static void I2C_Slave_WriteFifoBytes(const char *data, uint16_t size)
{

    int16_t i;

    for (i =0; i < size; i++)
    {
        if(i2c_slave_registers->out_fifo.bf.fifo_full)
            UART_printf("I2C Slave Write FIFO FULL!!!\n");
        else
            i2c_slave_registers->wr_data.bf.wr_data = data[i];
    }
	i2c_slave_registers->status.bf.reply_data_rdy = (uint32_t)1;
}
#endif


// /**
// * FUNCTION NAME: _assertHooki2c()
// *
// * @brief  - Called on an assert, to help debug the assert
// *
// * @return - void
// */
// void _assertHookI2c(void)
// {
//     const uint32_t cr = i2c_registers->control.dw;
//     i2cStatus();
//     ilog_I2C_COMPONENT_1(ILOG_FATAL_ERROR, I2C_CONTROLREG_READ, cr);
// }




