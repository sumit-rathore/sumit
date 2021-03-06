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
#ifndef ULP_LOG_H
#define ULP_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Function Declarations ##########################################################################
ILOG_CREATE(ULP_COMPONENT)
    ILOG_ENTRY(ULP_LEX_INITIALIZATION, "Lex ULP initialization\n")
    ILOG_ENTRY(ULP_LEX_ISR, "Got a Lex ULP ISR, Irq0: 0x%08x Irq1: 0x%02x LTSSM: 0x%02x\n")
    ILOG_ENTRY(ULP_LEX_HOST_VBUS_INTERRUPT, "Lex Host VBus interrupt, %d\n")
    ILOG_ENTRY(ULP_LEX_HOST_VBUS_DETECT, "Lex VBus detected %d\n")
    ILOG_ENTRY(ULP_LEX_HOST_RX_DETECT, "Lex Rx detected %d\n")
    ILOG_ENTRY(ULP_LEX_HOST_STATE_MSG, "Lex host state machine received a message, old state = %d, Event = %d new state = %d\n")
    ILOG_ENTRY(ULP_LEX_HOST_USB3, "** Host port supports USB3/2 **\n")
    ILOG_ENTRY(ULP_LEX_HOST_USB2, "** Host port supports USB2 **\n")
    ILOG_ENTRY(ULP_LEX_HOST_USB3_REMOVED, "USB3 removed\n")
    ILOG_ENTRY(ULP_LEX_HOST_SNOOP_TIMEOUT, "Host port Snoop mode timeout RxDetect = %d ltssm = 0x%x\n")
    ILOG_ENTRY(ULP_LEX_USB2_ENABLED_NO_CONNECTION, "Lex USB2 state machine enabled but not connected!\n")
    ILOG_ENTRY(ULP_LEX_USB2_STATE_MSG, "Lex USB2 state machine received a message, old state = %d, Event = %d new state = %d\n")
    ILOG_ENTRY(ULP_LEX_USB2_UNEXPECTED_EVENT, "Lex USB2 state machine received an unexpected event, Event = %d state = %d\n")
    ILOG_ENTRY(ULP_LEX_USB3_STATE_MSG, "Lex USB3 state machine received a message, old state = %d, Event = %d new state = %d\n")
    ILOG_ENTRY(ULP_LEX_USB3_UNEXPECTED_EVENT, "Lex USB3 state machine received an unexpected event, Event = %d state = %d\n")
    ILOG_ENTRY(ULP_LEX_USB3_RESET_STATE_MSG, "Lex USB3 reset state machine received a message, old state = %d, Event = %d new state = %d\n")
    ILOG_ENTRY(ULP_LEX_USB3_INVALID_RESET_STATE_MSG, "Lex USB3 reset state machine in a bad state for this event, state = %d Event = %d \n")
    ILOG_ENTRY(ULP_LEX_USB3_RESET_PLL_NOT_LOCKED, "Lex USB3 reset PLL not locked!\n")
    ILOG_ENTRY(ULP_LEX_USB3_LEX_SETUP_ERROR_DETECTED, "Lex USB3 setup failure; failure count %d\n")
    ILOG_ENTRY(ULP_LEX_USB3_REX_SETUP_ERROR_DETECTED, "Rex USB3 setup failure reported; failure count %d\n")
    ILOG_ENTRY(ULP_LEX_USB3_LEX_STUCK_AT_INACTIVE, "Lex USB3 - Lex stuck at inactive; failure count %d\n")
    ILOG_ENTRY(ULP_LEX_USB3_WARM_RESET_COUNT_CLEARED, "Lex USB3 - Warm reset count %d cleared\n")
    ILOG_ENTRY(ULP_LEX_USB3_FAILURE_RECOVERY, "Lex USB3 - Lex recovery; failure count %d\n")
    ILOG_ENTRY(ULP_LEX_UNEXPECTED_CPU_MSG_LENGTH, "Unexpected length of CPU message received by Lex, msgLength = %d\n")
    ILOG_ENTRY(ULP_LEX_COMLINK_UP_EVENT, "Lex received a comlink event, link is ** UP **\n")
    ILOG_ENTRY(ULP_LEX_COMLINK_DOWN_EVENT, "Lex received a comlink event, link is ** DOWN **\n")
    ILOG_ENTRY(ULP_LEX_RCV_REX_USB_MSG, "Lex received a Rex USB CPU message, msg = %d\n")
    ILOG_ENTRY(ULP_LEX_RCV_REX_USB2_MSG, "Lex received a Rex USB2 CPU message, msg = %d\n")
    ILOG_ENTRY(ULP_LEX_RCV_REX_USB3_MSG, "Lex received a Rex USB3 CPU message, msg = %d\n")
    ILOG_ENTRY(ULP_LEX_RCV_REX_USB3_RESET_MSG, "Lex received a Rex USB3 reset CPU message, msg = %d\n")
    ILOG_ENTRY(ULP_LEX_RCV_UNEXPECTED_USB3_RESET_MSG, "Lex USB3 reset: expected %d, but received %d message from Rex \n")
    ILOG_ENTRY(ULP_LTSSM_VALID, "LTSSM register value is valid time %d microseconds\n")
    ILOG_ENTRY(ULP_LTSSM_INVALID, "LTSSM register value is invalid, lttssm = 0x%x\n")
    ILOG_ENTRY(ULP_CTRL_USB3_VBUS_ON,  "vBus is ON\n")
    ILOG_ENTRY(ULP_CTRL_USB3_VBUS_OFF, "vBus is OFF\n")
    ILOG_ENTRY(ULP_CTRL_PHY_USB3_VBUS_ON,  "PHY vBus is ON\n")
    ILOG_ENTRY(ULP_CTRL_PHY_USB3_VBUS_OFF, "PHY vBus is OFF\n")
    ILOG_ENTRY(ULP_REX_ISR, "Got a Rex ULP ISR, Irq0: 0x%08x Irq1: 0x%02x LTSSM: 0x%02x\n")
    ILOG_ENTRY(ULP_REX_USB2_STATE_MSG, "Rex USB2 state machine received a message, old state = %d, Event = %d, new state = %d\n")
    ILOG_ENTRY(ULP_REX_USB3_STATE_MSG, "Rex USB3 state machine received a message, old state = %d, Event = %d, new state = %d\n")
    ILOG_ENTRY(ULP_REX_USB3_DELAY_TIMEOUT, "Rex USB3 delayed ready timeout rexUlpFsmState = %d\n")
    ILOG_ENTRY(ULP_REX_USB3_UNEXPECTED_EVENT, "Rex USB3 state machine received an unexpected event, Event = %d state = %d\n")
    ILOG_ENTRY(ULP_REX_UNEXPECTED_CPU_MSG_LENGTH, "Unexpected length of CPU message received by Rex, msgLength = %d\n")
    ILOG_ENTRY(ULP_REX_NO_RX_DETECT_YET,    "USB3 Rex - no Rx terminations detected\n")
    ILOG_ENTRY(ULP_REX_RCV_LEX_USB_MSG, "Rex received a Lex USB CPU message, msg = %d\n")
    ILOG_ENTRY(ULP_REX_RCV_LEX_USB2_MSG, "Rex received a Lex USB2 CPU message, msg = %d\n")
    ILOG_ENTRY(ULP_REX_RCV_LEX_USB3_MSG, "Rex received a Lex USB3 CPU message, msg = %d\n")
    ILOG_ENTRY(ULP_REX_RCV_LEX_USB3_RESET_MSG, "Rex received a Lex USB3 Reset CPU message, msg = %d\n")
    ILOG_ENTRY(ULP_REX_GE_REX_DEV_DISCONN, "REX - GE REX device disconnect\n")
    ILOG_ENTRY(ULP_REX_GE_REX_DEV_CONN, "REX - GE REX device connect\n")
    ILOG_ENTRY(ULP_ULP_CORE_PLL_LOCK_FAIL, "Ulp Core failed to lock\n")
    ILOG_ENTRY(ULP_REX_COMLINK_UP_EVENT, "Rex received a comlink event, link is ** UP **\n")
    ILOG_ENTRY(ULP_REX_COMLINK_DOWN_EVENT, "Rex received a comlink event, link is ** DOWN **\n")
    ILOG_ENTRY(ULP_LEX_SET_CONTROL_RESULT, "ULP_LexUsbControl() controlflags 0x%x Operation %d\n")
    ILOG_ENTRY(ULP_LEX_SYSTEM_UPDATE_RESULT, "LexUlpUpdateSystem() controlflags 0x%x hostEnable  %d\n")
    ILOG_ENTRY(ULP_REX_SET_CONTROL_RESULT, "ULP_RexUsbControl controlflags 0x%x Operation %d result %d\n")
    ILOG_ENTRY(ULP_REX_USB2_SET_CONTROL_RESULT, "ULP_RexUsb2Control flags 0x%x Operation %d result %d\n")
    ILOG_ENTRY(ULP_LEX_USB2_SET_CONTROL_RESULT, "ULP_LexUsb2Control flags 0x%x Operation %d result %d\n")
    ILOG_ENTRY(ULP_REX_USB3_SET_CONTROL_RESULT, "ULP_RexUsb3Control flags 0x%x Operation %d result %d\n")
    ILOG_ENTRY(ULP_LEX_USB3_SET_CONTROL_RESULT, "ULP_LexUsb3Control flags 0x%x Operation %d result %d\n")
    ILOG_ENTRY(ULP_HAL_SET_IRQ_ENABLE, "USB3 IRQ enabled irq0:0x%x irq1: 0x%x\n")
    ILOG_ENTRY(ULP_HAL_SET_IRQ_DISABLE, "USB3 IRQ disabled irq0:0x%x irq1: 0x%x\n")
    ILOG_ENTRY(ULP_HAL_SET_SS_DISABLE, "USB3 SS_DISABLED set\n")
    ILOG_ENTRY(ULP_HAL_GO_TO_INACTIVE, "USB3 Go to Inactive set\n")
    ILOG_ENTRY(ULP_HAL_SET_RX_DETECT,  "USB3 Rx.Detect set\n")
    ILOG_ENTRY(ULP_HAL_SET_HOT_RESET_WAIT_ON,  "USB3 Hot Reset wait ON\n")
    ILOG_ENTRY(ULP_HAL_SET_HOT_RESET_WAIT_OFF, "USB3 Hot Reset wait OFF\n")
    ILOG_ENTRY(ULP_HAL_SET_HOT_RESET,  "USB3 generating Hot Reset\n")
    ILOG_ENTRY(ULP_HAL_SET_WARM_RESET, "USB3 generating warm reset\n")
    ILOG_ENTRY(ULP_HAL_WAIT_IN_POLLING_ON,  "USB3 Wait in the polling state ON\n")
    ILOG_ENTRY(ULP_HAL_WAIT_IN_POLLING_OFF, "USB3 Wait in the polling state OFF\n")
    ILOG_ENTRY(ULP_HAL_RX_AUTO_TERM_ON,   "USB3 Auto Rx terminations on\n")
    ILOG_ENTRY(ULP_HAL_RX_AUTO_TERM_OFF,  "USB3 Auto Rx terminations off\n")
    ILOG_ENTRY(ULP_HAL_RX_TERM_ON,   "USB3 Rx terminations on\n")
    ILOG_ENTRY(ULP_HAL_RX_TERM_OFF,  "USB3 Rx terminations off\n")
    ILOG_ENTRY(ULP_HAL_GE_VBUS_ON,   "USB2 Lex GE vBus on\n")
    ILOG_ENTRY(ULP_HAL_GE_VBUS_OFF,  "USB2 Lex GE vBus off\n")
    ILOG_ENTRY(ULP_HAL_ENTER_STANDBY,  "USB3 Entering Standby\n")
    ILOG_ENTRY(ULP_HAL_EXIT_STANDBY,   "USB3 Exiting Standby\n")
    ILOG_ENTRY(ULP_HAL_REX_VBUS_ON,  "Rex vBus on\n")
    ILOG_ENTRY(ULP_HAL_REX_VBUS_OFF, "Rex vBus off\n")
    ILOG_ENTRY(ULP_INIT_CALLED, "ULP initialization called\n")
    ILOG_ENTRY(ULP_CORE_RESET_CALLED, "ULP core reset triggered PLL lock took %d microseconds\n")
    ILOG_ENTRY(ULP_USB3_TAKEDOWN_CALLED, "ULP_controlTakedownUSB3() called\n")
    ILOG_ENTRY(ULP_GE_CONTROL_RCV_CPU_MSG, "GE Control received a CPU message, msg = %d\n")
    ILOG_ENTRY(ULP_RESET_STATE, "ULP Reset State machine, state = 0x%x\n")
    ILOG_ENTRY(ULP_LTSSM_VALUE_AFTER_DISABLE, "LTSSM register value after vBus disable= 0x%x\n")
    ILOG_ENTRY(ULP_CORE_STATS_SENT_LRTY, "STAT:ULP bb_chip->ulp_core->stats0->snt_lrty: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_SENT_LBAD, "STAT:ULP bb_chip->ulp_core->stats0->snt_lbad: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_RCVD_LRTY, "STAT:ULP bb_chip->ulp_core->stats0->rcvd_lrty: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_RCVD_LBAD, "STAT:ULP bb_chip->ulp_core->stats0->rcvd_lbad: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_TX_LFPS_CNT_IN_ERR, "STAT:ULP bb_chip->ulp_core->stats0->tx_lfps_cnt_in_err: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_RX_LFPS_CNT_IN_ERR, "STAT:ULP bb_chip->ulp_core->stats0->rx_lfps_cnt_in_err: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_TX_FRAMER_PTP_VIOLATED, "STAT:ULP bb_chip->ulp_core->stats0->tx_framer_ptp_violated: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_LINK_TRAINING_PTP_VIOLATED, "STAT:ULP bb_chip->ulp_core->stats0->link_training_ptp_violated: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_LINK_CMD_PTP_VIOLATED, "USTAT:ULP bb_chip->ulp_core->stats0->link_command_ptp_violated: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_LMP_PTP_VIOLATED, "STAT:ULP bb_chip->ulp_core->stats0->lmp_ptp_violated: %d\n")
    ILOG_ENTRY(XUSB3_STATS_LINK_PARTNER_PTP_VIOLATED, "STAT:ULP bb_chip->xusb3->stats0->link_partner_ptp_violated: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_LINK_CMD_RX_EVENT_FIFO_OVERFLOW, "STAT:ULP bb_chip->ulp_core->stats0->link_command_rx_event_fifo_overflow: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_LINK_CMD_RX_EVENT_FIFO_UNDERFLOW, "STAT:ULP bb_chip->ulp_core->stats0->link_command_rx_event_fifo_underflow: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_LMP_RX_EVENT_FIFO_OVERFLOW, "STAT:ULP bb_chip->ulp_core->stats0->lmp_rx_event_fifo_overflow: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_LMP_RX_EVENT_FIFO_UNDERFLOW, "STAT:ULP bb_chip->ulp_core->stats0->lmp_rx_event_fifo_underflow: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_REMOTE_RX_HDR_BUFF_CRDT_IN, "STAT:ULP bb_chip->ulp_core->stats0->remote_rx_hdr_buff_crdt_cnt_in_err: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_TO_PHY_OUT_SOP, "ULP CORE STATS PTP SOP to PHY path count %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_TO_PHY_OUT_EOP, "ULP CORE STATS PTP EOP to PHY path count %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_FROM_PHY_IN_SOP, "ULP CORE STATS PTP SOP from PHY path count %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_FROM_PHY_IN_EOP, "ULP CORE STATS PTP EOP from PHY path count %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_TO_LINK_OUT_SOP, "ULP CORE STATS PTP SOP to ethernet link path count %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_TO_LINK_OUT_EOP, "ULP CORE STATS PTP EOP to ethernet link path count %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_FROM_LINK_IN_SOP, "ULP CORE STATS PTP SOP from ethernet link path count %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_FROM_LINK_IN_EOP, "ULP CORE STATS PTP EOP from ethernet link path count %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_GO2_U0_FROM_RECOVERY, "STAT:ULP bb_chip->ulp_core->stats0->go2_u0_from_recovery: %d\n")
    ILOG_ENTRY(ULP_CORE_STATS_RX_GO_RECOVERY, "STAT:ULP bb_chip->ulp_core->stats0->rx_go_recovery: %d\n")
    ILOG_ENTRY(ULP_PHY_STATS0_RX_ELASTIC_BUFF_OVERFLOW, "USTAT:ULP bb_chip->ulp_phy->stats0->rx_elastic_buff_overflow: %d\n")
    ILOG_ENTRY(ULP_PHY_STATS0_RX_ELASTIC_BUFF_UNDERFLOW, "STAT:ULP bb_chip->ulp_phy->stats0->rx_elastic_buff_underflow: %d\n")
    ILOG_ENTRY(ULP_PHY_STATS0_RX_DISPARITY_ERR, "STAT:ULP bb_chip->ulp_phy->stats0->rx_disparity_err: %d\n")
    ILOG_ENTRY(ULP_PHY_STATS1_SKIP_INSERT_IN_ERR, "STAT:ULP bb_chip->ulp_phy->stats1->skp_insert_in_err: %d\n")
    ILOG_ENTRY(ULP_PHY_STATS1_DPP_ABORT, "STAT:ULP bb_chip->ulp_phy->stats1->dpp_abort: %d\n")
    ILOG_ENTRY(ULP_PHY_STATS1_RX_FRAMER_PTP_VIOLATED, "STAT:ULP bb_chip->ulp_phy->stats1->rx_framer_ptp_violated: %d\n")
    ILOG_ENTRY(XUSB3_STATS_UNKNOWN_PKT_DRP,                      "STAT:ULP bb_chip->xusb3->stats0->unknown_pkt_drp: %d\n")
    ILOG_ENTRY(XUSB3_STATS_DWN_STREAM_BUSY_DRP,                  "STAT:ULP bb_chip->xusb3->stats0->dwn_stream_busy_drp: %d\n")
    ILOG_ENTRY(XUSB3_STATS_DROP_LONE_DPP,                        "STAT:ULP bb_chip->xusb3->stats0->drop_lone_dpp: %d\n")
    ILOG_ENTRY(XUSB3_STATS_RCVD_LONE_DPH,                        "STAT:ULP bb_chip->xusb3->stats0->rcvd_lone_dph: %d\n")
    ILOG_ENTRY(XUSB3_STATS_TIMEDOUT_2JOIN_DPP,                   "STAT:ULP bb_chip->xusb3->stats0->timedout_2join_dpp: %d\n")
    ILOG_ENTRY(XUSB3_RX_PFIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR, "STAT:ULP bb_chip->xusb3->rx_pfifo->write_engine->stats0->pkt_max_byte_cnt_err: %d\n")
    ILOG_ENTRY(XUSB3_RX_PFIFO_WRENG_STATS0_FIFO_FULL_ERR,        "STAT:ULP bb_chip->xusb3->rx_pfifo->write_engine->stats0->fifo_full_err: %d\n")
    ILOG_ENTRY(XUSB3_RX_PFIFO_WRENG_STATS0_PKT_ERR,              "STAT:ULP bb_chip->xusb3->rx_pfifo->write_engine->stats0->pkt_err: %d\n")
    ILOG_ENTRY(XUSB3_RX_PFIFO_WRENG_STATS0_PKT_SOP_ERR,          "STAT:ULP bb_chip->xusb3->rx_pfifo->write_engine->stats0->pkt_sop_err: %d\n")
    ILOG_ENTRY(XUSB3_RX_PFIFO_WRENG_STATS0_DRP_PKT_RD,           "STAT:ULP bb_chip->xusb3->rx_pfifo->write_engine->stats0->drp_pkt_rd: %d\n")
    ILOG_ENTRY(XUSB3_RX_PFIFO_WRENG_STATS0_DRP_PKT_WR,           "STAT:ULP bb_chip->xusb3->rx_pfifo->write_engine->stats0->drp_pkt_wr: %d\n")
    ILOG_ENTRY(XUSB3_RX_PFIFO_RDENG_STATS0_DRP_PKT,              "STAT:ULP bb_chip->xusb3->rx_pfifo->read_engine->stats0->drp_pkt: %d\n")
    ILOG_ENTRY(XUSB3_PTPGUARD_2CORE_MISSING_SOP_ERR,             "STAT:ULP bb_chip->xusb3->ptp_guard_2core->stats0->missing_sop_err: %d\n")
    ILOG_ENTRY(XUSB3_PTPGUARD_2CORE_MISSING_EOP_ERR,             "STAT:ULP bb_chip->xusb3->ptp_guard_2core->stats0->missing_eop_err: %d\n")
    ILOG_ENTRY(XUSB3_PTPGUARD_2CORE_MAX_CYCLE_ERR,               "STAT:ULP bb_chip->xusb3->ptp_guard_2core->stats0->max_cycle_err: %d\n")
    ILOG_ENTRY(ULP_CORE_PTPGUARD_2PHY_MISSING_SOP_ERR,              "STAT:ULP bb_chip->ulp_core->ptp_guard_2phy->stats0->missing_sop_err: %d\n")
    ILOG_ENTRY(ULP_CORE_PTPGUARD_2PHY_MISSING_EOP_ERR,              "STAT:ULP bb_chip->ulp_core->ptp_guard_2phy->stats0->missing_eop_err: %d\n")
    ILOG_ENTRY(ULP_CORE_PTPGUARD_2PHY_MAX_CYCLE_ERR,                "STAT:ULP bb_chip->ulp_core->ptp_guard_2phy->stats0->max_cycle_err: %d\n")
    ILOG_ENTRY(ULP_GE_CONTROL_STATES,                               "geControl.controlStates old: %d new:%d enable:%d\n")
    ILOG_ENTRY(ULP_GE_CHANNEL_STATUS,                               "UlpGeChannelStatus() %d\n")
    ILOG_ENTRY(ULP_GE_FAILURE_HANDLER,                              "GeFailureHandler: Running Watchdog Occur !!!!\n")
    ILOG_ENTRY(ULP_LEX_USB3_CHANNEL_STATUS,                         "UlpLexUsb3ChannelStatus() %d lexOnlyResetActive %d\n")
    ILOG_ENTRY(ULP_LEX_USB3_RESET_STATE,                            "UlpLexUsb3ResetLexOnly() lexOnlyResetState %d\n")
    ILOG_ENTRY(ULP_REX_USB3_CHANNEL_STATUS,                         "UlpRexUsb3ChannelStatus() %d RexOnlyResetActive %d\n")
    ILOG_ENTRY(ULP_LEX_USB_3_IRQ_1,                                 "rexReqInactive %d standbyExit %d LexOnlyResetTriggered %d\n")
    ILOG_ENTRY(ULP_MAX_CYCLE,                                       "max_cycle_mode %d max_cycles %d\n")

#ifndef PLUG_TEST
ILOG_END(ULP_COMPONENT, ILOG_DEBUG)
#else
ILOG_END(ULP_COMPONENT, ILOG_FATAL_ERROR)
#endif //PLUG_TEST

/* ILOG_ENTRY(ULP_UNHANDLED_INTERRUPT, "Unhandled ULP interrupt, Irq0 = 0x%x Irq1 = 0x%x\n") */
/*     ILOG_ENTRY(ULP_CORE_STATS_CHANGED, "ULP CORE STATUS Error, stats changed\n")

 */
#endif // ULP_LOG_H
