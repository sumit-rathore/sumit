//#################################################################################################
// Icron Technology Corporation - Copyright 2016
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef XAUI_LOG_H
#define XAUI_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(XAUI_COMPONENT)
    ILOG_ENTRY(XAUI_TEST, "XAUI code at line = %d\n")
    ILOG_ENTRY(XAUI_FAIL_TX_ALIGN, "RXAUI TX is not aligned before Aquantia release reset\n")

    ILOG_ENTRY(XAUI_STAT_GT0_DISP_ERR,   "STAT:RXAUI rxaui->stats0->gt0_disp_err = 0x%x\n")
    ILOG_ENTRY(XAUI_STAT_GT0_NOT_IN_TBL, "STAT:RXAUI rxaui->stats0->gt0_not_in_table = 0x%x\n")
    ILOG_ENTRY(XAUI_STAT_GT1_DISP_ERR,   "STAT:RXAUI rxaui->stats0->gt1_disp_err = 0x%x\n")
    ILOG_ENTRY(XAUI_STAT_GT1_NOT_IN_TBL, "STAT:RXAUI rxaui->stats0->gt1_not_in_table = 0x%x\n")
    ILOG_ENTRY(XAUI_STAT_MISSING_START,  "STAT:RXAUI rxaui->stats0->missing_start = 0x%x\n")

    ILOG_ENTRY(XAUI_RX_ALIGN, "RXAUI RX align ISR %d\n")
    ILOG_ENTRY(XAUI_RX_TOGGLE_RESET, "RXAUI RX reset toggle\n")

    ILOG_ENTRY(XAUI_RESET_RX_BUFFERS, "RXAUI elasticity buffers reset\n")
ILOG_END(XAUI_COMPONENT, ILOG_DEBUG)

#endif // XAUI_LOG_H
