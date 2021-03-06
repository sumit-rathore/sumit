///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2017
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
//!   @file  -  aquantia_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef AQUANTIA_LOG_H
#define AQUANTIA_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/
#define AQUANTIA_NULL_ILOG  0

ILOG_CREATE(AQUANTIA_COMPONENT)
// STAT LOG MESSAGES=========================================================================================================================
    ILOG_ENTRY(AQUANTIA_PMA_RECEIVE_LINK_STATUS, "STAT: Aquantia PMA Receive Link Status (1.1.2): %d\n")
    ILOG_ENTRY(AQUANTIA_PMA_TRANSMIT_FAULT, "STAT: Aquantia PMA Transmit Fault (1.8.B): %d\n")
    ILOG_ENTRY(AQUANTIA_PMA_RECEIVE_FAULT, "STAT: Aquantia PMA Receive Fault (1.8.A): %d\n")
    ILOG_ENTRY(AQUANTIA_PMA_LP_RETRAIN_COUNT, "STAT: Aquantia PMA LP Fast Retrain Count (1.93.F:B): %d\n")
    ILOG_ENTRY(AQUANTIA_PMA_LD_RETRAIN_COUNT, "STAT: Aquantia PMA LD Fast Retrain Count (1.93.A:6): %d\n")
    ILOG_ENTRY(AQUANTIA_PMA_RETRAIN_SIGNAL_TYPE, "STAT: Aquantia PMA Fast Retrain Signal Type (1.93.2:1): %d\n")
    ILOG_ENTRY(AQUANTIA_PMA_NUM_LINK_RECOVERY, "STAT: Aquantia PMA Number Of Link Recovery Since Last AutoNeg (1.E811.F:8): %d\n")
    ILOG_ENTRY(AQUANTIA_PMA_NUM_RFI_RECOVERY, "STAT: Aquantia PMA Number Of RFI Training Link Recovery Since Last AutoNeg (1.E811.7:0): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_TRANSMIT_FAULT, "STAT: Aquantia PCS Transmit Fault (3.8.B): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_RECEIVE_FAULT, "STAT: Aquantia PCS Receive Fault (3.8.A): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_10G_LINK_STATUS, "STAT: Aquantia PCS 10G Receive Link Status (3.20.C): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_10G_HIGH_BER, "STAT: Aquantia PCS 10G BASE-T High BER (3.20.1): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_10G_BLOCK_LOCK, "STAT: Aquantia PCS 10G Block Lock (3.20.0): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_BLOCK_LOCK_LATCHED, "STAT: Aquantia PCS Block Lock Latched (3.21.F): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_HIGH_BER_LATCHED, "STAT: Aquantia PCS 10G BASE-T High BER Latched (3.21.E): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_ERROR_FRAME_COUNT, "STAT: Aquantia PCS LDPC Errored Frame Counter (3.21.D:8): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_ERROR_BLOCK_COUNT, "STAT: Aquantia PCS Errored 65B Block Counter (3.21.7:0): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_SYSTEM_INTERFACE_FAULT, "STAT: Aquantia PCS System Interface Transmit Fault (3.C8F0.0): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_XAUI_INVALID_BLOCK, "STAT: Aquantia PCS XAUI Transmit Invalid 64B Block Detected (3.CC00.0): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_TX_10G_BAD_FRAME_COUNT, "STAT: Aquantia PCS Transmit Vendor 10GBASE-T Bad Frame Counter (3.C823:C822): %d\n")
    // ILOG_ENTRY(AQUANTIA_PCS_CRC8_ERROR_COUNT, "STAT: Aquantia PCS Receive Vendor CRC-8 Error Counter (3.E811:E810): %d\n")
    // ILOG_ENTRY(AQUANTIA_PCS_10G_ERROR_FRAME_COUNT, "STAT: Aquantia PCS Receive Vendor 10G BASE-T Bad Frame Counter (3.E815:E814): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_UNCORRECTED_FRAME_COUNT, "STAT: Aquantia PCS Receive Vendor Uncorrected Frame Counter (3.E821:E820): %d\n")
    // ILOG_ENTRY(AQUANTIA_PCS_LDPC_CORRECTED_F2_ITERATION_COUNT, "STAT: Aquantia PCS LDPC corrected frames 2 iteration Counter (3.E843:E842): %d\n")
    // ILOG_ENTRY(AQUANTIA_PCS_LDPC_CORRECTED_F3_ITERATION_COUNT, "STAT: Aquantia PCS LDPC corrected frames 3 iteration Counter (3.E845:E844): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_LDPC_CORRECTED_F4_ITERATION_COUNT, "STAT: Aquantia PCS LDPC corrected frames 4 iteration Counter (3.E847:E846): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_LDPC_CORRECTED_F5_ITERATION_COUNT, "STAT: Aquantia PCS LDPC corrected frames 5 iteration Counter (3.E849:E848): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_LDPC_CORRECTED_F6_ITERATION_COUNT, "STAT: Aquantia PCS LDPC corrected frames 6 iteration Counter (3.E850): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_LDPC_CORRECTED_F7_ITERATION_COUNT, "STAT: Aquantia PCS LDPC corrected frames 7 iteration Counter (3.E851): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_LDPC_CORRECTED_F8_ITERATION_COUNT, "STAT: Aquantia PCS LDPC corrected frames 8 iteration Counter (3.E852): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_CRC_ERROR, "STAT: Aquantia PCS Rx CRC Frame error (3.EC00.F): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_LDPC_DECODE_FAILURE, "STAT: Aquantia PCS LDPC decode failure (3.EC00.E): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_LOCAL_FAULT_DETECT, "STAT: Aquantia PCS RPL local fault detect (3.EC00.B): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_LOF_DETECT, "STAT: Aquantia PCS RPL LOF detect (3.EC00.A): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_40G_BIP_LOCK, "STAT: Aquantia PCS RPL 40G BIP lock (3.EC00.9): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_INVALID_65B_BLOCK, "STAT: Aquantia PCS Invalid Rx 65B block (3.EC00.8): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_LDPC_ERROR_EXCEEDED, "STAT: Aquantia PCS Rx LDPC consecutive errored frame threshold exceeded (3.EC00.5): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_TX_FAULT, "STAT: Aquantia PHY Transmit Fault (4.8.B): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_RX_FAULT, "STAT: Aquantia PHY Receive Fault (4.8.A): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_XGXS_LANE_ALIGN, "STAT: Aquantia PHY XGXS Lane Alignment Status (4.18.C): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_LANE_SYNC, "STAT: Aquantia PHY Lane Sync (4.18.3:0): %x\n")
    ILOG_ENTRY(AQUANTIA_PHY_TX_BAD_FRAME_COUNT, "STAT: Aquantia PHY XS Transmit (XAUI Rx) Bad Frame Counter (4.C805:C804): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_XAUI_RX_DELETION, "STAT: Aquantia PHY XAUI Rx Sequence Ordered Set Deletion (4.CC01.D): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_RXAUI_LANE_ALIGN_LOCK_B, "STAT: Aquantia PHY RXAUI Lane Alignment Lock Status [1:0] (4.CC01.B): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PHY_RXAUI_LANE_ALIGN_LOCK_A, "STAT: Aquantia PHY RXAUI Lane Alignment Lock Status [1:0] (4.CC01.A): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PHY_TX_RESERVED_XGMII_CHAR, "STAT: Aquantia PHY Reserved XGMII Character Received (4.CC01.9): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PHY_TX_INVALID_XGMII_CHAR, "STAT: Aquantia PHY XAUI Tx Invalid XGMII Character Received (4.CC01.8): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_TX_CODE_VIOLATION_ERR_1, "STAT: Aquantia PHY XAUI Tx Code Violation Error For Lane 0 [3:0] (4.CC01.4): %x\n")
    ILOG_ENTRY(AQUANTIA_PHY_TX_CODE_VIOLATION_ERR_2, "STAT: Aquantia PHY XAUI Tx Code Violation Error For Lane 1 [3:0] (4.CC01.5): %x\n")
    ILOG_ENTRY(AQUANTIA_PHY_TX_CODE_VIOLATION_ERR_3, "STAT: Aquantia PHY XAUI Tx Code Violation Error For Lane 2 [3:0] (4.CC01.6): %x\n")
    ILOG_ENTRY(AQUANTIA_PHY_TX_CODE_VIOLATION_ERR_4, "STAT: Aquantia PHY XAUI Tx Code Violation Error For Lane 3 [3:0] (4.CC01.7): %x\n")
    ILOG_ENTRY(AQUANTIA_PHY_TX_RUN_DISPARITY_ERR_1, "STAT: Aquantia PHY XAUI Tx Running Disparity Error For Lane 0 [3:0] (4.CC01.0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PHY_TX_RUN_DISPARITY_ERR_2, "STAT: Aquantia PHY XAUI Tx Running Disparity Error For Lane 1 [3:0] (4.CC01.1): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PHY_TX_RUN_DISPARITY_ERR_3, "STAT: Aquantia PHY XAUI Tx Running Disparity Error For Lane 2 [3:0] (4.CC01.2): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PHY_TX_RUN_DISPARITY_ERR_4, "STAT: Aquantia PHY XAUI Tx Running Disparity Error For Lane 3 [3:0] (4.CC01.3): 0x%x\n")

    ILOG_ENTRY(AQUANTIA_PHY_RX_BAD_FRAME_COUNT, "STAT: Aquantia PHY XS Receive (XAUI Tx) Rx Bad Frame Counter (4.E805:E804): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_USX_AUTONEGO_NUMBER, "STAT: Aquantia PHY Number of USX Aneg Restarts [F:0] (4.E810.F:0): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_AUTONEGO_STATUS, "STAT: Aquantia PHY System interface Autoneg Status [1:0] (4.E812.F:E): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_RX_LINK_UP, "STAT: Aquantia PHY Rx Link Up (4.E812.D): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_TX_READY, "STAT: Aquantia PHY Tx Ready (4.E812.C): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_RX_RESERVED_XGMII_CHAR, "STAT: Aquantia PHY Reserved XGMII Character Received from PCS (4.EC00.F): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_RX_INVALID_XGMII_CHAR, "STAT: Aquantia PHY Invalid XGMII Character Received from PCS (4.EC00.E): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_RX_LINK_STATUS_MSG, "STAT: Aquantia PHY Link status Message Received from PCS (4.EC00.D): %d\n")
    // ILOG_ENTRY(AQUANTIA_PHY_SYS_RX_LINK_UP, "STAT: Aquantia PHY System Interface Rx Link Up (4.EC01.F): %d\n")
    // ILOG_ENTRY(AQUANTIA_PHY_SYS_RX_LINK_DOWN, "STAT: Aquantia PHY System Interface Rx Link Down (4.EC01.E): %d\n")
    // ILOG_ENTRY(AQUANTIA_PHY_SYS_TX_READY, "STAT: Aquantia PHY System Interface Tx Ready (4.EC01.D): %d\n")
    // ILOG_ENTRY(AQUANTIA_PHY_SYS_TX_NOT_READY, "STAT: Aquantia PHY System Interface Tx Not Ready (4.EC01.C): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_LINK_PULSE_DETECT, "STAT: Aquantia AUTO-NEG Link Pulse Detect (7.CC01.F): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_LINK_CONNECT, "STAT: Aquantia AUTO-NEG Link Connect / Disconnect (7.CC01.0): %d\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_HIGH_TEMP_FAIL, "STAT: Aquantia GLOBAL High Temperature Failure (1E.CC00.E): %d\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_LOW_TEMP_FAIL, "STAT: Aquantia GLOBAL Low Temperature Failure (1E.CC00.D): %d\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_HIGH_TEMP_WARNING, "STAT: Aquantia GLOBAL High Temperature Warning (1E.CC00.C): %d\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_LOW_TEMP_WARNING, "STAT: Aquantia GLOBAL Low Temperature Warning (1E.CC00.B): %d\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_DEVICE_FAULT, "STAT: Aquantia GLOBAL Device Fault (1E.CC00.4): %d\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_FAST_LINK_DROP_FAULT, "STAT: Aquantia GLOBAL Fast Link Drop (1E.CC01.A): %d\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_MDIOD_COMMAND_OVERFLOW, "STAT: Aquantia GLOBAL MDIO Command Handling Overflow (1E.CC01.7): %d\n")
    ILOG_ENTRY(AQUANTIA_DTEXS_RX_LOCAL_FAULT, "STAT: Aquantia DTE XS Receive Local Fault (5.8.10): %d\n")
    ILOG_ENTRY(AQUANTIA_DTEXS_TX_LOCAL_FAULT, "STAT: Aquantia DTE XS Transmit Local Fault (5.8.11): %d\n")

    //not important
    // ILOG_ENTRY(AQUANTIA_PMA_CHANNEL_A_OPERATING_MARGIN,"STAT: Aquantia PMA Channel A Operating Margin (1.85.F:0): %d\n")
    // ILOG_ENTRY(AQUANTIA_PMA_CHANNEL_B_OPERATING_MARGIN,"STAT: Aquantia PMA Channel B Operating Margin (1.86.F:0): %d\n")
    // ILOG_ENTRY(AQUANTIA_PMA_CHANNEL_C_OPERATING_MARGIN,"STAT: Aquantia PMA Channel C Operating Margin (1.87.F:0): %d\n")
    // ILOG_ENTRY(AQUANTIA_PMA_CHANNEL_D_OPERATING_MARGIN,"STAT: Aquantia PMA Channel D Operating Margin (1.88.F:0): %d\n")
    // ILOG_ENTRY(AQUANTIA_PMA_CHANNEL_A_MINIMUM_MARGIN,"STAT: Aquantia PMA Channel A Minimum Operating Margin (1.89.F:0): %d\n")
    // ILOG_ENTRY(AQUANTIA_PMA_CHANNEL_B_MINIMUM_MARGIN,"STAT: Aquantia PMA Channel B Minimum Operating Margin (1.8A.F:0): %d\n")
    // ILOG_ENTRY(AQUANTIA_PMA_CHANNEL_C_MINIMUM_MARGIN,"STAT: Aquantia PMA Channel C Minimum Operating Margin (1.8B.F:0): %d\n")
    // ILOG_ENTRY(AQUANTIA_PMA_CHANNEL_D_MINIMUM_MARGIN,"STAT: Aquantia PMA Channel D Minimum Operating Margin (1.8C.F:0): %d\n")
    ILOG_ENTRY(AQUANTIA_PMA_CHANNEL_A_RECEIVED_SIGNAL_POWER,"STAT: Aquantia PMA Channel A Received Signal Power (1.8D.F:0): %d\n")
    ILOG_ENTRY(AQUANTIA_PMA_CHANNEL_B_RECEIVED_SIGNAL_POWER,"STAT: Aquantia PMA Channel B Received Signal Power (1.8E.F:0): %d\n")
    ILOG_ENTRY(AQUANTIA_PMA_CHANNEL_C_RECEIVED_SIGNAL_POWER,"STAT: Aquantia PMA Channel C Received Signal Power (1.8F.F:0): %d\n")
    ILOG_ENTRY(AQUANTIA_PMA_CHANNEL_D_RECEIVED_SIGNAL_POWER,"STAT: Aquantia PMA Channel D Received Signal Power (1.90.F:0): %d\n")
    ILOG_ENTRY(AQUANTIA_PMA_RECEIVE_CURRENT_LINK_STATUS,"STAT: Aquantia PMA Receive Link Current Status (1.E800.0): %d\n")
    ILOG_ENTRY(AQUANTIA_PMA_FAST_RETRAIN_TIME,"STAT: Aquantia PMA Accumulated Fast Retrain Time (1.E810.F:0): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_FAULT_STATUS,"STAT: Aquantia PCS Fault (3.1.7): %d\n")
    ILOG_ENTRY(AQUANTIA_PCS_RECEIVE_LINK_STATUS,"STAT: Aquantia PCS Receive Link Status (3.1.2): %d\n")
    // ILOG_ENTRY(AQUANTIA_PCS_LDPC_1_ITERATION_CORRECTED_FRAMES,"STAT: Aquantia PCS LDPC corrected frames which converged in 1 iteration (03.E841.F:0): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_TRANSMIT_LINK_ALIGNMENT_STATUS,"STAT: Aquantia PHY XS Transmit Link Alignment Status (4.1.2): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_SERDES_CALS_NUMBER,"STAT: Aquantia PHY Number of Serdes Cals [F:0] (4.C820.F:0): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_SIF_BLOCK_LOCK_TRANSITIONS_1_0_NUMBER,"STAT: Aquantia PHY Number of SIF Block Lock Transtitions 1 - 0 [7:0] (4.C821.F:8): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_SIF_BLOCK_LOCK_TRANSITIONS_0_1_NUMBER,"STAT: Aquantia PHY Number of SIF Block Lock Transtitions 0 - 1 [7:0] (4.C821.7:0): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_SIF_XGS_SWITCH_OVERS_NUMBER,"STAT: Aquantia PHY Number of SIF XGS Switch-overs [F:0] (4.C822.F:0): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_SIGNAL_LOSS_1,"STAT: Aquantia PHY Loss of signal [3:0] (4.CC02.C): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_SIGNAL_LOSS_2,"STAT: Aquantia PHY Loss of signal [3:0] (4.CC02.D): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_SIGNAL_LOSS_3,"STAT: Aquantia PHY Loss of signal [3:0] (4.CC02.E): %d\n")
    ILOG_ENTRY(AQUANTIA_PHY_SIGNAL_LOSS_4,"STAT: Aquantia PHY Loss of signal [3:0] (4.CC02.F): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_PARALLEL_DETECTION_FAULT,"STAT: Aquantia AUTO-NEG Parallel Detection Fault (7.1.9): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_REMOTE_FAULT,"STAT: Aquantia AUTO-NEG Remote Fault (7.1.4): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_LINK_STATUS,"STAT: Aquantia AUTO-NEG Link Status (7.1.2): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_MASTER_SLAVE_CONFIG_FAULT,"STAT: Aquantia AUTO-NEG MASTER-SLAVE Configuration Fault (7.21.F): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_LOCAL_RECEIVER_STATUS,"STAT: Aquantia AUTO-NEG Local Receiver Status (7.21.D): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_REMOTE_RECEIVER_STATUS,"STAT: Aquantia AUTO-NEG Remote Receiver Status (7.21.C): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_RESTARTS_HANDLED,"STAT: Aquantia AUTO-NEG Autonegotiation Restarts Handled [F:0] (7.C813.F:0): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_LINK_PULSE_DETECTED_STATUS,"STAT: Aquantia AUTO-NEG Link Pulse Detected Status (7.C812.F): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_AUTOMATIC_DOWNSHIFT,"STAT: Aquantia AUTO-NEG Automatic Downshift (7.CC00.1): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_CONNECTION_STATE_CHANGE,"STAT: Aquantia AUTO-NEG Connection State Change (7.CC00.0): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_10G_DOWNSHIFT,"STAT: Aquantia AUTO-NEG Downshift From 10G (7.E411.B): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_5G_DOWNSHIFT,"STAT: Aquantia AUTO-NEG Downshift From 5G (7.E411.A): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_2G_DOWNSHIFT,"STAT: Aquantia AUTO-NEG Downshift From 2.5G (7.E411.9): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_1G_DOWNSHIFT,"STAT: Aquantia AUTO-NEG Downshift From 1G (7.E411.8): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_MAX_ADVERTISED_RATE,"STAT: Aquantia AUTO-NEG Max Advertised Rate [3:0] (7.E411.7:4): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_PROTOCOL_ERROR,"STAT: Aquantia AUTO-NEG Autonegotiation Protocol Error (7.EC01.D): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_FLP_IDLE_ERROR,"STAT: Aquantia AUTO-NEG FLP Idle Error (7.EC01.C): %d\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_HIGH_TEMP_FAILURE_STATE,"STAT: Aquantia GLOBAL High Temperature Failure State (1E.C830.E): %d\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_LOW_TEMP_FAILURE_STATE,"STAT: Aquantia GLOBAL Low Temperature Failure State (1E.C830.D): %d\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_HIGH_TEMP_WARNING_STATE,"STAT: Aquantia GLOBAL High Temperature Warning State (1E.C830.C): %d\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_LOW_TEMP_WARNING_STATE,"STAT: Aquantia GLOBAL Low Temperature Warning State (1E.C830.B): %d\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_MESSAGE,"STAT: Aquantia GLOBAL Message [F:0] (1E.C850.F:0): %d\n")

    ILOG_ENTRY(AQUANTIA_TEMP_THRESHOLD,"Aquantia temperature threshold: Warning %d C, Shutdown %d C.\n")
    ILOG_ENTRY(AQUANTIA_TEMP_WARNING,"STAT: Aquantia Temperature warning %d C\n")
    // ILOG_ENTRY(AQUANTIA_TEMP_SHUTDOWN,"Aquantia temperature over its shutdown threshold (%d C). System shut down \n")
    ILOG_ENTRY(AQUANTIA_SET_TEMP_WARNING,"Aquantia warning temperature set to %d C.\n")
    ILOG_ENTRY(AQUANTIA_SET_TEMP_SHUTDOWN,"Aquantia shutdown temperature set to %d C.\n")

    ILOG_ENTRY(AQUANTIA_AUTONEG_CONNECTION_STATE,"STAT: Aquantia AUTO-NEG Connection State[4:0] (7.C810.D:9): %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_ATTEMPTS_SINCE_RESET,"STAT: Aquantia AUTO-NEG Attempts Since Reset (7.C814.F:0): %d\n")

// ICMD log messages =========================================================================================================================
    ILOG_ENTRY(AQUANTIA_PCS_LOOPBACK, "Aquantia PCS loopback(3.0.E): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_10G_SPEED_SELECTION, "Aquantia 10G Speed Selection(3.0.5:2): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TX_SCRAMBLER_DISABLE, "Aquantia Tx Scrambler Disable(3.D800.F): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TX_INJECT_CRC_ERROR, "Aquantia Tx Inject CRC Error(3.D800.E): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TX_INJECT_FRAME_ERROR, "Aquantia Tx Inject Frame Error(3.D800.D): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_RX_ERROR_LDPC_FRAME_ENABLE, "Aquantia Enable Rx LDPC Error Frame(3.E400.0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_RX_LDPC_DECODER_CONTROL, "Aquantia Control Rx LDPC Decoder (3.E400.F): 0x%x\n")

    ILOG_ENTRY(AQUANTIA_XS_LOOPBACK, "Aquantia XS Loopback (4.0.E): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_RECEIVE_TEST_PATTERN_ENABLE, "Enable Aquantia Receive Test Pattern (4.19.2): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PHY_OPERATING_MODE, "Aquantia Operating Phy Mode (4.C441.8:6): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TEST_PATTERN_SELECT, "Aquantia Select Test Pattern (4.19.1:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_LOOPBACK_CONTROL, "Aquantia XS Loopback Control (4.C444.F:B): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_MDI_PACKET_GENERATION, "Aquantia XS MDI Packet Generation (4.C444.5): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_XS_SYSTEM_IF_PACKET_GENERATION, "Aquantia XS I/F Packet Generation (4.C444.2): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_RATE, "Aquantia XS Rate (4.C444.1:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TEST_PATTERN_FORCE_ERROR, "Aquantia Select Test Pattern Force Error (4.D800.F): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TEST_PATTERN_MODE_7_FORCE_ERROR, "Aquantia XS Test Pattern Mode 7 Force Error (4.D800.E): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_XAUI_RX_LOCAL_FAULT_INJECTION, "Aquantia XAUI Rx Local Fault Injection (4.D800.D): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TEST_PATTERN_EXTENDED_SELECT, "Aquantia Test-Pattern Extended Select [1:0] (4.D800.C:B): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TEST_PATTERN_CHECK_ENABLE, "Aquantia Test Pattern Check Enable (4.D800.A): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TEST_PATTERN_CHECK_POINT, "Aquantia Test Pattern Check Point (4.D800.7): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TEST_PATTERN_INSERT_EXTRA_IDLES, "Aquantia Test Pattern Insert Extra Idles [2:0] (4.D801.E:C): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TEST_PATTERN_CHECK_SELECT, "Aquantia Test Pattern Check Select [3:0] (4.D801.B:8): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TEST_PATTERN_CHANNEL_SELECT, "Aquantia Test Pattern Channel Select [3:0] (4.D801.3:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_CHANNEL_0_TEST_PATTERN_ERROR_COUNTER, "Aquantia Channel 0 Test Pattern Error Counter [F:0] (4.D810.F:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_CHANNEL_1_TEST_PATTERN_ERROR_COUNTER, "Aquantia Channel 1 Test Pattern Error Counter [F:0] (4.D811.F:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_CHANNEL_2_TEST_PATTERN_ERROR_COUNTER, "Aquantia Channel 2 Test Pattern Error Counter [F:0] (4.D812.F:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_CHANNEL_3_TEST_PATTERN_ERROR_COUNTER, "Aquantia Channel 3 Test Pattern Error Counter [F:0] (4.D813.F:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TEST_PATTERN_MODE_7_ERROR_COUNTER, "Aquantia Test Pattern Mode 7 Error Counter [F:0] (4.D814.F:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_XAUI_TX_ERROR_INJECTION_LANE_SELECT, "Aquantia XAUI Tx Error Injection Lane Select [2:0] (4.F800.F:D): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_XAUI_TX_INJECT_SYNCHRONIZATION_ERROR, "Aquantia XAUI Tx Inject Synchronization Error (4.F800.C): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_XAUI_TX_INJECT_ALIGNMENT_ERROR, "Aquantia XAUI Tx Inject Alignment Error (4.F800.B): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_XAUI_TX_INJECT_CODE_VIOLATION, "Aquantia XAUI Tx Inject Code Violation (4.F800.A): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_XAUI_TX_10B_VIOLATION_CODEWORD, "Aquantia XAUI Tx 10B Violation Codeword [9:0] (4.F800.9:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PHY_XS_SYSTEM_LOOPBACK_PASS_THROUGH, "Aquantia PHY XS System Loopback Pass Through (4.F802.F): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PHY_XS_SYSTEM_LOOPBACK_ENABLE, "Aquantia PHY XS System Loopback Enable (4.F802.E): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_XAUI_TX_LOCAL_FAULT_INJECTION, "Aquantia XAUI Tx Local Fault Injection (4.F802.D): 0x%x\n")

    ILOG_ENTRY(AQUANTIA_RESTART_AUTONEGOTIATION, "Aquantia Restart Autonegotiation (7.0.9): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_SERDES_START_UP_MODE, "Aquantia SERDES Start-Up Mode [2:0] (7.C410.F:D): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_AUTONEGOTIATION_TIMEOUT, "Aquantia Autonegotiation Timeout [3:0] (7.C411.F:C): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_AUTONEGOTIATION_TIMEOUT_MOD, "Aquantia Autonegotiation Timeout Mod (7.C411.B): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_LINK_PARTNER_1000_BASE_T_FULL_DUPLEX_ABILITY, "Aquantia Link Partner 1000BASE-T Full Duplex Ability (7.E820.F): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_LINK_PARTNER_1000_BASE_T_HALF_DUPLEX_ABILITY, "Aquantia Link Partner 1000BASE-T Half Duplex Ability (7.E820.E): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_LINK_PARTNER_SHORT_REACH, "Aquantia Link Partner Short-Reach (7.E820.D): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_LINK_PARTNER_AQ_RATE_DOWNSHIFT_CAPABILITY, "Aquantia Link Partner AQRate Downshift Capability (7.E820.C): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_LINK_PARTNER_5G, "Aquantia Link Partner 5G (7.E820.B): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_LINK_PARTNER_2G, "Aquantia Link Partner 2.5G (7.E820.A): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_LINK_PARTNER, "Aquantia Link Partner (7.E820.2): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_AUTONEGOTIATION_PROTOCOL_ERROR_STATE, "Aquantia Autonegotiation Protocol Error State (7.E831.D): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_FLP_IDLE_ERROR_STATE, "Aquantia FLP Idle Error State (7.E831.C): 0x%x\n")

    ILOG_ENTRY(AQUANTIA_ENABLE_DIAGNOSTICS, "Aquantia Enable Diagnostics (1E.C400.F): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_HIGH_TEMP_FAILURE_THRESHOLD, "Aquantia High Temp Failure Threshold [F:0] (1E.C421): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_LOW_TEMP_FAILURE_THRESHOLD, "Aquantia Low Temp Failure Threshold [F:0] [F:0] (1E.C422): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_HIGH_TEMP_WARNING_THRESHOLD, "Aquantia High Temp Warning Threshold [F:0] [1:0] (1E.C423): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_LOW_TEMP_WARNING_THRESHOLD, "Aquantia Low Temp Warning Threshold [F:0] (1E.C424): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_DIAGNOSTICS_SELECT, "Aquantia Diagnostics Select (1E.C470.F): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_EXTENDED_MDI_DIAGNOSTICS_SELECT, "Aquantia Extended MDI Diagnostics Select [1:0] (1E.C470.E:D): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_INITIATE_CABLE_DIAGNOSTICS, "Aquantia Initiate Cable Diagnostics (1E.C470.4): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_ENABLE_VDD_POWER_SUPPLY_TUNING, "Aquantia Enable VDD Power Supply Tuning (1E.C472.E): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TUNABLE_EXTERNAL_VDD_POWER_SUPPLY_PRESENT, "Aquantia Tunable External VDD Power Supply Present (1E.C472.6): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_EXTERNAL_VDD_CHANGE_REQUEST, "Aquantia External VDD Change Request [3:0] (1E.C472.5:2): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_ENABLE_5TH_CHANNEL_RFI_CANCELLATION, "Aquantia Enable 5th Channel RFI Cancellation (1E.C472.0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_RATE_TRANSITION_REQUEST, "Aquantia Rate Transition Request [2:0] (1E.C473.A:8): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TRAINING_SNR, "Aquantia Training SNR [7:0] (1E.C473.7:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_GLB_LOOPBACK_CONTROL, "Aquantia Loopback Control [4:0] (1E.C47A.F:B): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_GLB_MDI_PACKET_GENERATION, "Aquantia MDI Packet Generation (1E.C47A.5): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_GLB_SYSTEM_IF_PACKET_GENERATION, "Aquantia Global Provisioning System IF Packet Generation (1E.C47A.3): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_RESERVED_PROVISIONING_RATE, "Aquantia Rate [2:0] (1E.C47A.2:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PAIR_A_STATUS, "Aquantia Pair A Status [2:0] (1E.C800.E:C): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PAIR_B_STATUS, "Aquantia Pair B Status [2:0] (1E.C800.A:8): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PAIR_C_STATUS, "Aquantia Pair C Status [2:0] (1E.C800.6:4): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PAIR_D_STATUS, "Aquantia Pair D Status [2:0] (1E.C800.2:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_STATUS_PAIR_A_REFLECTION_1, "Aquantia Pair A Reflection #1 [7:0] (1E.C801.F:8): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_STATUS_PAIR_A_REFLECTION_2, "Aquantia Pair A Reflection #2 [7:0] (1E.C801.7:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_IMPULSE_RESPONSE_MSW, "Aquantia Impulse Response MSW [F:0] (1E.C802.F:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_STATUS_PAIR_B_REFLECTION_1, "Aquantia Pair B Reflection #1 [7:0] (1E.C803.F:8): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_STATUS_PAIR_B_REFLECTION_2, "Aquantia Pair B Reflection #2 [7:0] (1E.C803.7:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_IMPULSE_RESPONSE_LSW, "Aquantia Impulse Response LSW [F:0] (1E.C804.F:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_STATUS_PAIR_C_REFLECTION_1, "Aquantia Pair C Reflection #1 [7:0] (1E.C805.F:8): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_STATUS_PAIR_C_REFLECTION_2, "Aquantia Pair C Reflection #2 [7:0] (1E.C805.7:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_STATUS_PAIR_D_REFLECTION_1, "Aquantia Pair D Reflection #1 [7:0] (1E.C807.F:8): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_STATUS_PAIR_D_REFLECTION_2, "Aquantia Pair D Reflection #2 [7:0] (1E.C807.7:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PROCESSOR_INTENSIVE_OPERATION_IN_PROGRESS, "Aquantia Processor Intensive MDIO Operation InProgress (1E.C831.F): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_A_REFLECTION_1, "Aquantia Pair A Reflection #1 [2:0] (1E.C880.E:C): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_A_REFLECTION_2, "Aquantia Pair A Reflection #2 [2:0] (1E.C880.A:8): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_A_REFLECTION_3, "Aquantia Pair A Reflection #3 [2:0] (1E.C880.6:4): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_A_REFLECTION_4, "Aquantia Pair A Reflection #4 [2:0] (1E.C880.2:0): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_B_REFLECTION_1, "Aquantia Pair B Reflection #1 [2:0] (1E.C881.E:C): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_B_REFLECTION_2, "Aquantia Pair B Reflection #2 [2:0] (1E.C881.A:8): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_B_REFLECTION_3, "Aquantia Pair B Reflection #3 [2:0] (1E.C881.6:4): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_B_REFLECTION_4, "Aquantia Pair B Reflection #4 [2:0] (1E.C881.2:0): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_C_REFLECTION_1, "Aquantia Pair C Reflection #1 [2:0] (1E.C882.E:C): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_C_REFLECTION_2, "Aquantia Pair C Reflection #2 [2:0] (1E.C882.A:8): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_C_REFLECTION_3, "Aquantia Pair C Reflection #3 [2:0] (1E.C882.6:4): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_C_REFLECTION_4, "Aquantia Pair C Reflection #4 [2:0] (1E.C882.2:0): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_D_REFLECTION_1, "Aquantia Pair D Reflection #1 [2:0] (1E.C883.E:C): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_D_REFLECTION_2, "Aquantia Pair D Reflection #2 [2:0] (1E.C883.A:8): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_D_REFLECTION_3, "Aquantia Pair D Reflection #3 [2:0] (1E.C883.6:4): 0x%x\n")
    // ILOG_ENTRY(AQUANTIA_IMPEDENCE_PAIR_D_REFLECTION_4, "Aquantia Pair D Reflection #4 [2:0] (1E.C883.2:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_CABLE_LENGTH, "Aquantia Cable Length [7:0] (1E.C884.7:0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_GLB_LOOPBACK_STATUS, "Aquantia Loopback Status [4:0] (1E.C888.F:B): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_MDI_PACKET_GENERATION_STATUS, "Aquantia MDI Packet Generation Status (1E.C888.5): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_GLB_SYSTEM_IF_PACKET_GENERATION_STATUS, "Aquantia System I/F Packet Generation Status (1E.C888.3): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_GLOBAL_RESERVED_STATUS_RATE, "Aquantia Rate [2:0] (1E.C888.2:0): 0x%x\n")

    ILOG_ENTRY(AQUANTIA_SHORT_REACH_MODE, "Aquantia Short Reach Mode (1.83.0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TEST_MODE_CONTROL, "Aquantia Test Control Mode (1.84.F:D): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TRANSMITTER_TEST_FREQUENCIES, "Aquantia Transmitter test frequencies (1.84.C:A): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_FAST_RETRAIN_ABILITY, "Aquantia Fast Retrain Ability (1.93.4): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_FAST_RETRAIN_ENABLE, "Aquantia Fast Retrain Enable (1.93.0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_TEST_MODE_RATE, "Aquantia Test Mode rate[1:0] (1.C412.F:E): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_PMA_DIGITAL_SYSTEM_LOOPBACK, "Aquantia Digital System Loopback (1.D800.F): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_EXTERNAL_PHY_LOOPBACK, "Aquantia External Phy Loopback (1.E400.F): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_ENABLE_FAST_RETRAIN, "Enable Aquantia Fast Retrain (1.E400.2): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_FORCE_MDI_CONFIGURATION, "Aquantia Force Mdi Configuration (1.E400.1): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_MDI_CONFIGURATION, "Aquantia Mdi Configuration(1.E400.0): 0x%x\n")
    ILOG_ENTRY(AQUANTIA_GENERAL_RW, "Aquantia Read/Write Result for (%x.%x): 0x%x\n")

// operational log messages =========================================================================================================================
    ILOG_ENTRY(AQUANTIA_ISR_NOT_GEN, "MDIOD not Aquantia generated ISR\n")
    ILOG_ENTRY(AQUANTIA_DBL_READ, "MDIOD Aquantia DblRead reg1 0x%x, reg2 0x%x\n")
    ILOG_ENTRY(AQUANTIA_READ_ASYNC_BITFIELD_NOT_FINISHED, "Requested to do a asynchronous bit field read while other jobs pending\n")
    ILOG_ENTRY(AQUANTIA_PHY_DISABLED, "Aquantia PHY disabled\n")
    ILOG_ENTRY(AQUANTIA_SETUP_STARTED, "Aquantia PHY setup\n")
    ILOG_ENTRY(AQUANTIA_INT_HIGH_TEMP_WARN, "Aquantia PHY high temp warning!\n")
    ILOG_ENTRY(AQUANTIA_INT_HIGH_TEMP_FAIL, "Aquantia PHY high temp failure, shutting down PHY!\n")
    ILOG_ENTRY(AQUANTIA_INIT_COMPLETED, "Aquantia init completed\n")
    ILOG_ENTRY(AQUANTIA_JUNCTION_TEMP, "Aquantia Stats Junction Temperature Read %d C\n")
    ILOG_ENTRY(AQUANTIA_FIRMWARE_VER1, "Aquantia Firmware Version: v%d.%x.%d\n")
    ILOG_ENTRY(AQUANTIA_FIRMWARE_VER2, "Aquantia Firmware Version: ID%d_VER%d\n")

    ILOG_ENTRY(AQUANTIA_DRIVER_STATE_TICK, "Aquantia driver previous state = %d new state %d\n")

    ILOG_ENTRY(AQUANTIA_TX_LINK_UP, "Aquantia TX Link Up checked\n")
    ILOG_ENTRY(AQUANTIA_RXAUI_ALIGNED, "Aquantia RXAUI is aligned\n")
    ILOG_ENTRY(AQUANTIA_RX_LINK_UP, "Aquantia RX Link Up checked\n")
    ILOG_ENTRY(AQUANTIA_MONITOR_REG, "Aquantia Monitor Reg link down triggered %x\n")
    ILOG_ENTRY(AQUANTIA_LINK_TX_READY, "Aquantia Tx Ready Check. Step: %d, Result: %x, Required: %x\n")
    ILOG_ENTRY(AQUANTIA_STABILITY_CHECK, "Aquantia Stability Check. Step: %d, Result: %x, Required: %x\n")
    ILOG_ENTRY(AQUANTIA_LINK_STATUS, "Aquantia CheckLinkStatus New status: 0x%x, raw Value: 0x%x\n")
    ILOG_ENTRY(AQUANTIA_INIT_READ, "Aquantia Initialization Read Step: %d, Read: 0x%x, Required: 0x%x\n")
    ILOG_ENTRY(AQUANTIA_INIT_WRITE, "Aquantia Initialization Write Step: %d\n")
    ILOG_ENTRY(AQUANTIA_INIT_5G, "Aquantia Set to 5G link speed Write Step: %d\n")
    ILOG_ENTRY(AQUANTIA_READ_REG_FAILED, "Aquantia Read Register FAILED! Step: %d, Read: 0x%x, Required: 0x%x\n")
    ILOG_ENTRY(AQUANTIA_READ_CRC8_FAILED, "Aquantia Read CRC 8 FAILED! Step: %d, Read: 0x%x\n")
    ILOG_ENTRY(AQUANTIA_GTX_ERROR_DETECTED, "Aquantia - RXAUI GTx errors detected!\n")
    ILOG_ENTRY(AQUANTIA_POST_STABILITY_CHECK_DONE, "Aquantia - Post stability checks done\n")
    ILOG_ENTRY(AQUANTIA_AUTO_RESET_DETECTED,"Aquantia auto-reset detected, restart link-up sequence\n")

    ILOG_ENTRY(AQUANTIA_STOP_STATS_INPROGRESS,"Aquantia stats stop already in progress!\n")
    ILOG_ENTRY(AQUANTIA_STOP_STAT_NULL_POINTER,"Aquantia stop stats pointer invalid!\n")
    ILOG_ENTRY(AQUANTIA_NORMAL_OPERATION,"Aquantia check normal operation, read value is 0x%x normal operation:%d (1:Normal, 0:Abnormal)\n")
    ILOG_ENTRY(AQUANTIA_ERROR_STATE, "Aquantia link connection rate error status : %d\n")
    ILOG_ENTRY(AQUANTIA_AUTONEG_UNSUPP_SPEED, "Aquantia linked at an unsupported speed\n")

ILOG_END(AQUANTIA_COMPONENT, ILOG_DEBUG) //ILOG_MINOR_EVENT)

#endif // #ifndef AQUANTIA_LOG_H
