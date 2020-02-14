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
#ifndef IDT_CLK_LOG_H
#define IDT_CLK_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################
ILOG_CREATE(IDT_CLK_COMPONENT)
    ILOG_ENTRY(IDT_CLK_CFG_RETRY, "IDT clk NAKed at index = %d, retryCount = %d\n")
    ILOG_ENTRY(IDT_CLK_CFG_6914_RETRY, "IDT6914 clk NAKed retryCount = %d\n")
    ILOG_ENTRY(IDT_CLK_CONFIGURE_REGISTERS, "IDT clk is being configured at index = %d, regOffset = 0x%x, value = 0x%x\n")
    ILOG_ENTRY(IDT_CLK_CFG_DONE, "IDT clk configuration done, type=%d\n")
    ILOG_ENTRY(IDT_CLK_CFG_FAIL, "IDT clk configuration fail, result=%d\n")
    ILOG_ENTRY(IDT_CLK_UNLOCKED, "IDT clk is not locked\n")
    ILOG_ENTRY(IDT_CLK_GENERAL_READWRITE, "IDT General read and write for register (0x%x) result: 0x%x\n")
    ILOG_ENTRY(IDT_CLK_INVALID_TYPE, "IDT6914 Invalid Type: %d\n")
    ILOG_ENTRY(IDT_CLK_SSC_MODE, "Rex SSC Mode = %d\n")
    ILOG_ENTRY(IDT_CLK_DP_SSC_MODE, "Rex DP SSC Mode = %d\n")

ILOG_END(IDT_CLK_COMPONENT, ILOG_MINOR_EVENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // IDT_CLK_LOG_H
