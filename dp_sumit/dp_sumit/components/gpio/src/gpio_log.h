///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009, 2012
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
//!   @file  -  gpio_log.h
//
//!   @brief -  The general purpose driver logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GPIO_LOG_H
#define GPIO_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(GPIO_COMPONENT)
    // General logging messages
    ILOG_ENTRY(GPIO_INIT, "Initializing GPIO, direction reg 0x%x, output bits 0x%x\n")
    ILOG_ENTRY(GPIO_READ, "GPIO_READ pin %d, bool val 0x%x\n")
    ILOG_ENTRY(GPIO_SET, "GPIO_SET pin %d\n")
    ILOG_ENTRY(GPIO_CLEAR, "GPIO_CLEAR pin %d\n")
    ILOG_ENTRY(GPIO_PULSE, "GPIO_PULSE pin %d\n")
    ILOG_ENTRY(FREQ_MEASURE, "Measuring PLL %d, XUSB is %d, CXM is %d\n")
    ILOG_ENTRY(INVALID_PIN, "Invalid Pin number %d\n")

    // IRQ messages
    ILOG_ENTRY(BGRG_IRQ_LOG, "Interrupts 0x%x triggered\n")
    ILOG_ENTRY(BGRG_IRQ_UNSERVICED, "Interrupts 0x%x were unserviced\n")
    ILOG_ENTRY(BGRG_GPIO7_LOG, "Interrupt GPIO7 triggered\n")
    ILOG_ENTRY(IRQ_HANDLER_NOT_SET, "Interrupt Handler is not set for pin %d in line %d\n")
    ILOG_ENTRY(REGISTERING_IRQ, "Registering Interrrupt handler for pin %d\n")
    ILOG_ENTRY(SERVICING_IRQ, "Servicing Interrupt for pin %d\n")
    ILOG_ENTRY(DISABLING_IRQ, "Disabling Interrupt for pin %d\n")
    ILOG_ENTRY(ENABLING_IRQ, "Enabling Interrupt for pin %d\n")

    // new messages
    ILOG_ENTRY(BGRG_HW_DEFAULTS, "Power-on harware defaults from GPIOs are: 0x%x\n")
    ILOG_ENTRY(BGRG_INT_HANDLER_PRI, "BGRG Interrupt Handler Primary Reg: 0x%x\n")
    ILOG_ENTRY(BGRG_INT_HANDLER_SEC, "BGRG Interrupt Handler Secondary Reg: 0x%x\n")
    ILOG_ENTRY(BGRG_INT_HANDLER_PRI_POST, "BGRG Interrupt Handler POST Primary Reg: 0x%x\n")
    ILOG_ENTRY(BGRG_INT_HANDLER_SEC_POST, "BGRG Interrupt Handler POST Secondary Reg: 0x%x\n")
    ILOG_ENTRY(GPIO_IRQ_NOT_AVAIL, "GPIO has no more available interrupts for assignment\n")
ILOG_END(GPIO_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef GPIO_LOG_H

