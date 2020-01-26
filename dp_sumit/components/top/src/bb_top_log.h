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
#ifndef BB_TOP_LOG_H
#define BB_TOP_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################
ILOG_CREATE(TOP_COMPONENT)

    ILOG_ENTRY(BB_TOP_DRP_READ_INVALID_MASK, "Tried to read from more than one DRP bus at a time: drpEnMask = 0x%x\n")
    ILOG_ENTRY(BB_TOP_BAD_MUTEX_TOKEN, "Wrong IMutex token at line %d\n")
    ILOG_ENTRY(BB_TOP_DP_DRP_WRITE, "Wrote DRP address 0x%x with drpEnMask = 0x%x and write data = 0x%x\n")
    ILOG_ENTRY(BB_TOP_DP_DRP_READ, "Read DRP address 0x%x with drpEnMask = 0x%x: read data = 0x%x\n")
    ILOG_ENTRY(BB_TOP_DP_DRP_RMW1, "RMWed DRP address 0x%x with drpEnMask = 0x%x, rmwMask = %d\n")
    ILOG_ENTRY(BB_TOP_DP_DRP_RMW2, " and write data = 0x%x\n")
    ILOG_ENTRY(BB_TOP_DP_INVALID_DRP_READ,
               "Tried to read from more than one DRP bus at a time: drpEnMask = 0x%x\n")
    ILOG_ENTRY(BB_TOP_DP_INVALID_BANDWIDTH, "Invalid bandwidth setting %d at line %d\n")
    ILOG_ENTRY(BB_TOP_DP_INVALID_LANE_COUNT, "Invalid lane count setting %d at line %d\n")
    ILOG_ENTRY(BB_TOP_DP_TRANSCEIVER_CONFIG_VALUE_ERROR,
               "DP transceiver value error: bad value = 0x%x at line %d\n")
    ILOG_ENTRY(BB_TOP_DP_INVALID_MMCM_OUTPUT_FREQ,
               "Invalid MMCM output frequency: bw = %d\n")
    ILOG_ENTRY(BB_TOP_DP_INVALID_MMCM_OUTPUT_ENCODING,
               "Invalid MMCM output encoding: bw = %d, laneWidth = %d\n")
    ILOG_ENTRY(BB_TOP_DP_GTX_RESET_TOO_SLOW, "GTX took too long to come out of reset!\n")
    ILOG_ENTRY(BB_TOP_DP_GTP_RESET_TOO_SLOW, "GTP took too long to come out of reset! Waited for %d us\n")
    ILOG_ENTRY(BB_TOP_DP_SET_VOLTAGE_SWING, "Writing txDiffCtrl with 0x%x -> 0x%x\n")
    ILOG_ENTRY(BB_TOP_DP_SET_PREEMPHASIS, "Writing txPostCursor with 0x%x -> 0x%x\n")
    ILOG_ENTRY(BB_TOP_DP_PRECHARGE, "Pre-charging main link with txInhibit = %d, laneCount = %d\n")
    ILOG_ENTRY(BB_TOP_DP_INITIALIZING_DP_TRANSCEIVERS, "Initializing DP transceivers: bw = %d, lc = %d\n")
    ILOG_ENTRY(BB_TOP_DBGXX, "### DBG 0x%x, 0x%x\n")
    ILOG_ENTRY(BB_TOP_HOLD_GE_RESET, "Placing GE in reset mode\n")
    ILOG_ENTRY(BB_TOP_GE_RUN, "Placing GE in run mode\n")
    ILOG_ENTRY(BB_TOP_GE_BOOTLOADER, "Placing GE in bootloader mode\n")
    ILOG_ENTRY(BB_TOP_GE_NULL_RUN_WATCHDOG_CALLBACK, "GE Run Wathdog happen without Callback\n")
    ILOG_ENTRY(BB_TOP_GE_NULL_RESET_WATCHDOG_CALLBACK, "GE Reset Wathdog happen without Callback\n")
    ILOG_ENTRY(BB_TOP_GE_RUN_AGAIN, "Set GE Run mode again while it's alreay in Run mode\n")
    ILOG_ENTRY(BB_TOP_DRP_WRITE_WAIT_TIMEOUT, "DRP Write wait for busses idle timeout\n")
    ILOG_ENTRY(BB_TOP_LINK_LOCK_WAIT_TIMEOUT, "Link PLL lock wait timeout\n")
    ILOG_ENTRY(BB_TOP_GTP_OUT_OF_RESET, "GTP is out of reset\n")
    ILOG_ENTRY(BB_TOP_DP_FRQ, "DP Freq took %d us to measure count = %d\n")
    ILOG_ENTRY(BB_TOP_DP_MMCM_LOCK, "DP MMCM Lock done took %d us to lock\n")
    ILOG_ENTRY(BB_TOP_DP_FRQ_TIMEOUT, "DP Freq measure time out\n")
    ILOG_ENTRY(BB_TOP_SSC_DETECTION, "***** SSC Detection = %d *****\n")

    ILOG_ENTRY(BB_TOP_READ_ICAP, "Read Reg = %x Val = %x\n")
    ILOG_ENTRY(BB_TOP_WRITE_ICAP, "Write Reg = %x Val = %x\n")
    ILOG_ENTRY(BB_TOP_READ_USER_REG, "User Reg = %x\n")
    ILOG_ENTRY(BB_TOP_WRITE_USER_REG, "Write User Reg = %x\n")
    ILOG_ENTRY(BB_TOP_FRQ_RUNNING, " Frequency detection still running\n")
    ILOG_ENTRY(BB_TOP_DRP_WRITE_TIMER_OVER, "DRP Set Timerover. drp_en_mask = 0x%x\n")
    ILOG_ENTRY(BB_TOP_DRP_READ_TIME_OVER, "DRP Read Timerover. Line at: %d\n")
    ILOG_ENTRY(BB_TOP_DP_SOURCE_RST, "DP Source Reset :%d (1:Apply reset, 0: Clear reset)\n")
    ILOG_ENTRY(BB_TOP_DP_NO_SYMBOL_LOCK, "No symbol lock. align status: 0x%x\n")
    ILOG_ENTRY(BB_TOP_CORE_TYPE, "The core type is -%d\n")


ILOG_END(TOP_COMPONENT, ILOG_MINOR_EVENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // BB_TOP_LOG_H
