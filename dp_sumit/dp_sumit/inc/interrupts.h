///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  - interrupts.h
//
//!   @brief - Contains the definitions for all of the interrupts for the project
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

/***************************** Included Headers ******************************/
#include <leon_traps.h>
#ifndef __ASSEMBLER__
#include <itypes.h>
#endif

/************************ Defined Constants and Macros ***********************/

// Blackbird interrupts
#define IRQ_AHB_RESPONSE_ERROR      (LEON_IRQ1)
#define IRQ_UART_RX                 (LEON_IRQ2)
#define IRQ_UART_TX                 (LEON_IRQ3)
#define IRQ_PARALLEL_IO_0           (LEON_IRQ4)
#define IRQ_PARALLEL_IO_1           (LEON_IRQ5)
#define IRQ_PARALLEL_IO_2           (LEON_IRQ6)
#define IRQ_PARALLEL_IO_3           (LEON_IRQ7)
#define IRQ_TIMER1                  (LEON_IRQ8)
#define IRQ_TIMER2                  (LEON_IRQ9)
#define IRQ_SECONDARY_IRQ           (LEON_IRQ10)
#define IRQ_DSU_TRACE_BUFFER        (LEON_IRQ11)
#define IRQ_UNUSED_12               (LEON_IRQ12)
#define IRQ_UNUSED_13               (LEON_IRQ13)
#define IRQ_UNUSED_14               (LEON_IRQ14)
#define IRQ_UNUSED_15               (LEON_IRQ15)

// SECONDARY INTERRUPTS - see FPGA m_blackbird/src/bb_core_cfg_pkg.sv
#define SECONDARY_INT_I2C_INT_MSK                  (1 << 0)
#define SECONDARY_INT_MDIO_INT_MSK                 (1 << 1)
#define SECONDARY_INT_BBTOP_INT_MSK                (1 << 2)
#define SECONDARY_INT_BBCORE_INT_MSK               (1 << 3)
#define SECONDARY_INT_DP_SINK_MAIN_INT_MSK         (1 << 4)
#define SECONDARY_INT_DP_SINK_AUX_HPD_INT_MSK      (1 << 5)
#define SECONDARY_INT_DP_SOURCE_MAIN_INT_MSK       (1 << 6)
#define SECONDARY_INT_DP_SOURCE_AUX_HPD_INT_MSK    (1 << 7)
#define SECONDARY_INT_LINK_LAYER_TX_MSK            (1 << 8)
#define SECONDARY_INT_LINK_LAYER_RX_MSK            (1 << 9)
#define SECONDARY_INT_ULP_INT_CORE_MSK             (1 << 10)
#define SECONDARY_INT_LINK_LAYER3_TX_MSK           (1 << 11)
#define SECONDARY_INT_LINK_LAYER3_RX_MSK           (1 << 12)
#define SECONDARY_INT_ULP_INT_PHY_MSK              (1 << 13)
#define SECONDARY_INT_GE_UART_RX_INT_MSK           (1 << 14)
#define SECONDARY_INT_GE_UART_TX_INT_MSK           (1 << 15)
#define SECONDARY_INT_GPIO_CTRL_INT_MSK            (1 << 16)
#define SECONDARY_INT_I2C_SLAVE_INT_MSK            (1 << 17)
#define SECONDARY_INT_SPI_FLASH_CNTRL_INT_MSK      (1 << 18)
#define SECONDARY_INT_MCA_CORE_INT_MSK             (1 << 19)
#define SECONDARY_INT_MCA_CHANNEL_0_INT_MSK        (1 << 20)
#define SECONDARY_INT_MCA_CHANNEL_1_INT_MSK        (1 << 21)
#define SECONDARY_INT_MCA_CHANNEL_2_INT_MSK        (1 << 22)
#define SECONDARY_INT_MCA_CHANNEL_3_INT_MSK        (1 << 23)
#define SECONDARY_INT_MCA_CHANNEL_4_INT_MSK        (1 << 24)
#define SECONDARY_INT_MCA_CHANNEL_5_INT_MSK        (1 << 25)
#define SECONDARY_INT_XUSB3_INT_MSK                (1 << 26)
#define SECONDARY_INT_UPP_INT_MSK                  (1 << 27)

#define SECONDARY_INT_NUMBER                        (28)    // starts at 0....

/******************************** Data Types *********************************/

/*********************************** API *************************************/
#ifndef __ASSEMBLER__
void BB_interruptSetMdioNotifyHandler(void (*mdioIsrNotifySubscriber)(void));
bool TOPLEVEL_secondaryInterruptHandler(void);
void TOPLEVEL_secondaryPollingHandler(void);

void TOPLEVEL_setPollingMask(uint32_t mask);
void TOPLEVEL_clearPollingMask(uint32_t mask);

#endif

#endif // INTERRUPTS_H

