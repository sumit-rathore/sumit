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
#ifndef FIBER5G_LOG_H
#define FIBER5G_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(FIBER5G_COMPONENT)
    ILOG_ENTRY(LINK_5G_INIT,                    "5G fiber Link Init\n")
    ILOG_ENTRY(LINK_5G_FIBER_LOS_INACTIVE,      "5G fiber Link signal detected\n")
    ILOG_ENTRY(LINK_5G_FIBER_LOS_IRQ_ACTIVE,    "5G fiber LOS IRQ - signal absent\n")
    ILOG_ENTRY(LINK_5G_FIBER_LOS_IRQ_INACTIVE,  "5G fiber LOS IRQ - signal present\n")
    ILOG_ENTRY(LINK_5G_FIBER_SIGNAL_VALID,      "5G fiber Link signal valid\n")
    ILOG_ENTRY(LINK_5G_FIBER_SIGNAL_INVALID,    "5G fiber Link signal too weak\n")
    ILOG_ENTRY(LINK_5G_FIBER_TX_FSM_RESET_START, "5G fiber Link Tx FSM reset start\n")
    ILOG_ENTRY(LINK_5G_FIBER_TX_FSM_RESET_DONE,  "5G fiber Link Tx FSM reset done IRQ\n")
    ILOG_ENTRY(LINK_5G_FIBER_RX_FSM_RESET_START, "5G fiber Link Rx FSM reset start\n")
    ILOG_ENTRY(LINK_5G_FIBER_RX_FSM_RESET_DONE,  "5G fiber Link Rx FSM reset done IRQ: Train time %d microseconds\n")
    ILOG_ENTRY(LINK_5G_FIBER_RX_BUF_UNDERFLOW,   "5G fiber Link Rx elastic buffer underflow IRQ\n")
    ILOG_ENTRY(LINK_5G_FIBER_RX_BUF_OVERFLOW,    "5G fiber Link Rx elastic buffer overflow IRQ\n")
    ILOG_ENTRY(LINK_5G_FIBER_RX_STABILITY,       "5G fiber Link Rx stability check, dispErr %d notInTable %d realign %d\n")
    ILOG_ENTRY(LINK_5G_FIBER_ENABLED,            "5G fiber Link Enabled\n")
    ILOG_ENTRY(LINK_5G_FIBER_DISABLED,           "5G fiber Link Disabled\n")
    ILOG_ENTRY(LINK_5G_SYSTEM_UP,                "5G fiber Link System up detected\n")
    ILOG_ENTRY(LINK_5G_FIBER_SIGNAL_DEBOUNCE,    "5G fiber Link debounce, state:%d losState:%d current state:%d\n")
    ILOG_ENTRY(LINK_5G_FIBER_STATS_RX_DISPARITY,            "STATS:LINKMGR bb_chip->bb_top->sl_5g->gt_disp_err %d\n")
    ILOG_ENTRY(LINK_5G_FIBER_STATS_RX_SYMBOL_NOT_IN_TABLE,  "STATS:LINKMGR bb_chip->bb_top->sl_5g->gt_not_in_table %d\n")
    ILOG_ENTRY(LINK_5G_FIBER_STATS_RX_REALIGN_COUNT,        "STATS:LINKMGR bb_chip->bb_top->sl_5g->gt_realign %d\n")

    ILOG_ENTRY(SFP_FINISAR_STATS_MEAS_TEMP, "*** SFP STATS Temperature %x\n")
    ILOG_ENTRY(SFP_FINISAR_STATS_MEAS_VCC, "*** SFP STATS Vcc %x\n")
    ILOG_ENTRY(SFP_FINISAR_STATS_MEAS_TX_BIAS, "*** SFP STATS TX Bias %x\n")
    ILOG_ENTRY(SFP_FINISAR_STATS_MEAS_TX_POWER, "*** SFP STATS TX Power %x\n")
    ILOG_ENTRY(SFP_FINISAR_STATS_MEAS_RX_POWER, "*** SFP STATS RX Power %x\n")

ILOG_END(FIBER5G_COMPONENT, ILOG_MINOR_EVENT)

#endif // FIBER5G_LOG_H
