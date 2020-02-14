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

//#################################################################################################
// Module Description
//#################################################################################################
// This file contains the implementation of ULP hardware abstraction layer.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <ulp.h>
#include <ulp_core_regs.h>
#include <ulp_phy_regs.h>
#include <xusb3_regs.h>
#include <cpu_comm.h>
#include <bb_top.h>
#include <bb_top_ge.h>

#include "ulp_loc.h"
#include "ulp_log.h"
#include <uart.h>
#include <bb_core.h>
#include <module_addresses_regs.h>

// Constants and Macros ###########################################################################

#define ULP_PHY_TX_MARGIN_MASK (ULP_PHY_PIPE_CTRL_TX_MARGIN_MASK >> ULP_PHY_PIPE_CTRL_TX_MARGIN_OFFSET)

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static volatile ulp_core_s * const ulpCorePtr = (volatile ulp_core_s*) bb_chip_ulp_core_s_ADDRESS;
static volatile ulp_phy_s  * const ulpPhyPtr = (volatile ulp_phy_s*) bb_chip_ulp_phy_s_ADDRESS;
static volatile xusb3_s    * const xusb3Ptr = (volatile xusb3_s*) bb_chip_xusb3_s_ADDRESS;

// Static Function Declarations ###################################################################
// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// Initialize the LEX ULP hardware abstraction layer.
//
// Parameters:
//      ulpCoreLexBaseAddr      - LEX ULP core base address
//      ulpLexIntEn             - LEX interrupts to enable
// Return:
// Assumptions:
//#################################################################################################
void _ULP_HalInit(void)
{
}


//#################################################################################################
// Get the pending ULP interrupts from irq0, and then clears the interrupts
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint32_t UlpHalGetIrq0Interrupts(void)
{
    // these are status interrupts - no processing needs to be done to clear them except
    // acknowledging them, in case there are more
    uint32_t pendingInterrupts = (ulpCorePtr->irq0.s.pending.dw & ulpCorePtr->irq0.s.enable.dw); // get the pending interrupts
    ulpCorePtr->irq0.s.pending.dw = pendingInterrupts;          // clear them

    return (pendingInterrupts);
}


//#################################################################################################
// Get the pending ULP interrupts from irq1, and then clears the interrupts
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint32_t UlpHalGetIrq1Interrupts(void)
{
    // these are status interrupts - no processing needs to be done to clear them except
    // acknowledging them, in case there are more
    uint32_t pendingInterrupts = (ulpCorePtr->irq1.s.pending.dw & ulpCorePtr->irq1.s.enable.dw); // get the pending interrupts
    ulpCorePtr->irq1.s.pending.dw = pendingInterrupts;          // clear them

    return (pendingInterrupts);
}


//#################################################################################################
// Enable ULP interrupts
//
// Parameters:
//      intsToEnable            - interrupt(s) to enable
// Return:
// Assumptions:
//#################################################################################################
void UlpHalEnableUlpInterrupts(uint32_t ulpIrq0ToEnable, uint32_t ulpIrq1ToEnable)
{
    ulpCorePtr->irq0.s.enable.dw |= ulpIrq0ToEnable;
    ulpCorePtr->irq1.s.enable.dw |= ulpIrq1ToEnable;


    ilog_ULP_COMPONENT_2(ILOG_DEBUG, ULP_HAL_SET_IRQ_ENABLE, ulpIrq0ToEnable, ulpIrq1ToEnable);
}


//#################################################################################################
// Disable ULP interrupts
//
// Parameters:
//      intsToDisable            - interrupt(s) to disable
// Return:
// Assumptions:
//#################################################################################################
void UlpHalDisableUlpInterrupts(uint32_t ulpIrq0ToDisable, uint32_t ulpIrq1ToDisable)
{
    ulpCorePtr->irq0.s.enable.dw &= ~ulpIrq0ToDisable;      // disable to IRQ0 interrupts
    ulpCorePtr->irq0.s.pending.dw = ulpIrq0ToDisable;       // clear any pending ones

    ulpCorePtr->irq1.s.enable.dw &= ~ulpIrq1ToDisable;      // disable any IRQ1 interrupts
    ulpCorePtr->irq1.s.pending.dw = ulpIrq1ToDisable;       // clear any pending interrupts

    ilog_ULP_COMPONENT_2(ILOG_DEBUG, ULP_HAL_SET_IRQ_DISABLE, ulpIrq0ToDisable, ulpIrq1ToDisable);
}


//#################################################################################################
// Configure LEX ULP core register to set SS disable.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpHalSetSSdisable(void)
{
    ulpCorePtr->control.bf.go2_ss_disabled = 1;
    ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_HAL_SET_SS_DISABLE);
}


//#################################################################################################
// Configure ULP core register to go to inactive
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpHalGoToInactive(void)
{
    ulpCorePtr->control.bf.go2_ss_inactive = 1;
    ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_HAL_GO_TO_INACTIVE);
}

//#################################################################################################
// See if we are still waiting to go inactive
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool _ULP_halIsInactivePending(void)
{
    return(ulpCorePtr->control.bf.go2_ss_inactive);

}
//#################################################################################################
// Configure LEX ULP core register to set Rx detect
//
// Parameters:
// Return:
// Assumptions:
//      * Note - this is only guaranteed to work if we were is the SS_DISABLED state; it may or
//               may not work in other states
//#################################################################################################
void UlpHalSetRxDetect(void)
{
    ulpCorePtr->control.bf.go2_rx_detect = 1;
    ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_HAL_SET_RX_DETECT);
}


//#################################################################################################
// Configure LEX ULP core register to go to hot reset
//
// Parameters:
// Return:
// Assumptions:
//      * Note - this is only guaranteed to work if we are in the U0 state;
//
//#################################################################################################
void _ULP_halSetHotReset(void)
{
    ulpCorePtr->control.bf.go2_hot_reset = 1;   // go to hot reset...
    ulpCorePtr->control.bf.go2_recovery = 1;    // ...from recovery (can't go to hot reset directly from U0)
    ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_HAL_SET_HOT_RESET);
}


//#################################################################################################
// Configure ULP core register to wait in hot reset (or not)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void _ULP_halSetHotResetWait(bool enabled)
{
    ulpCorePtr->control.bf.wait_in_hot_reset = enabled;
    ilog_ULP_COMPONENT_0(ILOG_DEBUG, enabled ? ULP_HAL_SET_HOT_RESET_WAIT_ON : ULP_HAL_SET_HOT_RESET_WAIT_OFF);
}


//#################################################################################################
// Configure ULP core register to go to warm reset
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void _ULP_halSetWarmReset(void)
{
    ulpCorePtr->control.bf.gen_warm_reset = 1;
    ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_HAL_SET_WARM_RESET);
}


//#################################################################################################
// Configure Lex/Rex ULP core register to set the receive terminations
//
// Note: On the Rex, vBus must be turned on AFTER Rx Terminations are set.  If vBus is set on
//       while Rx Terminations are off, the device will assume it is connecting to a USB2 port
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpHalControlRxTerminations(bool enabled)
{
    if (enabled)
    {
        // RX terminations are presented to host/device
        ulpCorePtr->control.bf.rx_termination_en = 1;
        ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_HAL_RX_TERM_ON);
   }
    else
    {
        ulpCorePtr->control.bf.rx_termination_en = 0;
        ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_HAL_RX_TERM_OFF);
    }
}

//#################################################################################################
// Configure LEX ULP core register to enable/disable auto hardware clearing of rx termination
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void _ULP_halControlAutoRxTerminations(bool enabled)
{
    if (enabled)
    {
        // RX terminations are presented to host/device
        ulpCorePtr->control.bf.allow_rx_term_en_hw_clr = 1;
        ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_HAL_RX_AUTO_TERM_ON);
   }
    else
    {
        ulpCorePtr->control.bf.allow_rx_term_en_hw_clr = 0;
        ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_HAL_RX_AUTO_TERM_OFF);
    }
}

//#################################################################################################
// Configure ULP core register to wait in polling or not
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void _ULP_halControlWaitInPolling(bool enable)
{
    ulpCorePtr->control.bf.wait_in_polling_lfps = enable;
    ilog_ULP_COMPONENT_0(ILOG_DEBUG, enable ? ULP_HAL_WAIT_IN_POLLING_ON : ULP_HAL_WAIT_IN_POLLING_OFF);
}


//#################################################################################################
// Set the Tx margin on the Phy controller
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UlpHalSetTxMargin(uint8_t txMargin)
{
    ulpPhyPtr->pipe_ctrl.bf.tx_margin = txMargin & ULP_PHY_TX_MARGIN_MASK;
}


//#################################################################################################
// enables/disables the Credit HP timer
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UlpHalControlCreditHpTimer(bool enableCreditTimer)
{
    ulpCorePtr->configuration.bf.credit_hp_timer_en = enableCreditTimer;
}



//#################################################################################################
// Configure ULP core register to set whether the port is upstream (Lex) or downstream (Rex)
//
// Parameters: setToUpstream - true will set port to upstream, false will set to downstream
// Return:
// Assumptions:
//
//#################################################################################################
void _ULP_halUsb3SetUpstreamPort(bool setToUpstream)
{
    xusb3Ptr->configuration.bf.upstream_port = setToUpstream;
}

//#################################################################################################
// Configure ULP core register to set whether the port is upstream (Lex) or downstream (Rex)
//
// Parameters: setToUpstream - true will set port to upstream, false will set to downstream
// Return:
// Assumptions:
//
//#################################################################################################
void _ULP_halUsb3ConfigInternalLoopback(bool enableLoopback)
{
    ulpCorePtr->configuration.bf.internal_loopback_en = enableLoopback;
}

//#################################################################################################
// Configure ULP core register to enable U1, U2, or U3 states
//
// Parameters: setToUpstream - true will set port to upstream, false will set to downstream
// Return:
// Assumptions:
//
//#################################################################################################
void _ULP_halUsb3SetLowPowerStates(enum Ulp_UxStateNames enableUxState)
{
    switch (enableUxState)
    {
        case ULP_U1_POWER_STATE:        // allow the U1 power state
            ulpCorePtr->configuration.bf.allow_u1 = 1;
            break;

        case ULP_U2_POWER_STATE:        // allow the U2 power state
            ulpCorePtr->configuration.bf.allow_u2 = 1;
            break;

        case ULP_U3_POWER_STATE:        // allow the U3 power state
            ulpCorePtr->configuration.bf.quasi_u3_mode = 1;
            break;

        case ULP_SS_DISABLED_POWER_STATE:    // when in SS_DISABLED, go to a low power state (P2 rather then P3)
            ulpCorePtr->configuration.bf.quasi_ss_disabled_mode = 1;
            break;

        default:
            break;
    }
}


//#################################################################################################
// Enters/Exits Standby mode (U3) based on the given argument
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void _ULP_halControlStandbyMode(bool enabled)
{
    if (enabled)
    {
        // enter suspend (U3) mode
        ulpCorePtr->control.bf.go_u3 = 1;   // cleared automatically after entering U3
        ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_HAL_ENTER_STANDBY);
   }
    else
    {
        // exit suspend (U3) mode
        ulpCorePtr->control.bf.exit_low_power = 1;
        ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_HAL_EXIT_STANDBY);
    }
}

//#################################################################################################
// Sets low power mode wait
//
// When set, allows the software to hold the port in its exit handshaking (either initiated by
// link partner or self initiated) to come out U3 following Section 7.2.4.2.7.\n\nNote the limit
// time based on Table 6-30 in Section 6.9.2. A 20ms for U3 (t13-t10)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void _ULP_halControlLowPowerWaitMode(bool enabled)
{
    ulpCorePtr->control.bf.wait_in_u3 = enabled;
}


//#################################################################################################
// Get the value of LTSSM register, major state only (minor state masked off)
//
// Parameters:
// Return:
//      ulpCorePtr->ctrl_sts.s.ltssm.dw)      - value of LTSM register
// Assumptions:
//#################################################################################################
uint8_t _ULP_GetLtssmMajorState(void)
{
    return (UlpHalGetLTSSM() & ULP_LTSSM_MAJOR_STATE_MASK);
}


//#################################################################################################
// Get the value of LTSSM register.
//
// Parameters:
// Return:
//      ulpCorePtr->ctrl_sts.s.ltssm.dw)      - value of LTSM register
// Assumptions:
//#################################################################################################
uint8_t UlpHalGetLTSSM(void)
{
    return (ulpCorePtr->ltssm_state.dw);
}

//#################################################################################################
// Get the value of the ULP control register.
//
// Parameters:
// Return:
//      ulpCorePtr->ctrl_sts.s.ltssm.dw)      - value of LTSM register
// Assumptions:
//#################################################################################################
uint32_t _ULP_GetControlValue(void)
{
    return (ulpCorePtr->control.dw);
}

//#################################################################################################
// Get the value of the ULP configuration register.
//
// Parameters:
// Return:
//      ulpCorePtr->ctrl_sts.s.ltssm.dw)      - value of LTSM register
// Assumptions:
//#################################################################################################
uint32_t _ULP_GetConfigurationValue(void)
{
    return (ulpCorePtr->configuration.dw);
}


//#################################################################################################
// Check if the value of LTSSM register is valid.
//
// Parameters:
// Return:
// Assumptions:
//      * The valid value is 0x61 for the current implementation.
//
//#################################################################################################
bool _ULP_IsLtssmValid(void)
{
    return (UlpHalGetLTSSM() == 0x61);
}



//#################################################################################################
// Get the value of LTSSM register.
//
// Parameters:
// Return:
//      ulpCorePtr->ctrl_sts.s.ltssm.dw)      - value of LTSM register
// Assumptions:
//#################################################################################################
bool _ULP_isVbusDetSet(void)
{
    return bb_top_isIrqUsbVbusDetectRawSet();
}


//#################################################################################################
// Gets the current status of the USB3 Rx terminations
//
// Parameters:
// Return: TRUE if Rx terminations present, false otherwise
//
// Assumptions:
//#################################################################################################
bool UlpHalRxTerminationsPresent(void)
{
    return (ulpCorePtr->irq0.s.raw.bf.rx_termination_det == 1);
}



//#################################################################################################
// Sets vBus active or not on the Rex
//
// Parameters: active - false to turn vBus off, true to turn it on
// Return:
// Assumptions:
//
//#################################################################################################
void UlpHalRexVbusActive(bool active)
{
    // TODO: put this in bb_top?
    if (active)
    {
        GpioSet(GPIO_CONN_VBUS_DETECT_EN_A);
    }
    else
    {
        GpioClear(GPIO_CONN_VBUS_DETECT_EN_A);
    }

    ilog_ULP_COMPONENT_0(ILOG_DEBUG, (active ? ULP_CTRL_USB3_VBUS_ON : ULP_CTRL_USB3_VBUS_OFF));

}

//#################################################################################################
// Sets the vbUs input to Ge on the Lex high (active) or low
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void _ULP_halControlGeVbus(bool active)
{
    bb_top_ApplyGEVbusDetect(active);
    ilog_ULP_COMPONENT_0(ILOG_DEBUG, (active ? ULP_HAL_GE_VBUS_ON : ULP_HAL_GE_VBUS_OFF));
}

//#################################################################################################
// sets the rd2clr_config for ulp phy registers so it will automatically clear the stat when read
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void _ULP_phyHalSetReadClearStats(void)
{
    ulpPhyPtr->stats0.s.rd2clr_config.dw   = ulp_phy_stats0_rd2clr_config_WRITEMASK;
    ulpPhyPtr->stats1.s.rd2clr_config.dw   = ulp_phy_stats1_rd2clr_config_WRITEMASK;
}


//#################################################################################################
// sets the rd2clr_config for ULP core registers so it will automatically clear the stat when read
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void _ULP_coreHalSetReadClearStats(void)
{
    ulpCorePtr->stats0.s.rd2clr_config.dw                                   = ulp_core_stats0_rd2clr_config_WRITEMASK;
    xusb3Ptr->ptp_guard_2core.s.stats0.s.rd2clr_config.dw                   = xusb3_ptp_guard_2core_stats0_rd2clr_config_WRITEMASK;
    xusb3Ptr->buff_2ulp.s.stats0.s.rd2clr_config.dw                         = xusb3_buff_2ulp_stats0_rd2clr_config_WRITEMASK;
    xusb3Ptr->buff_2upp.s.stats0.s.rd2clr_config.dw                         = xusb3_buff_2upp_stats0_rd2clr_config_WRITEMASK;
    xusb3Ptr->stats0.s.rd2clr_config.dw                                     = xusb3_stats0_rd2clr_config_WRITEMASK;
    ulpCorePtr->ptp_guard_2phy.s.stats0.s.rd2clr_config.dw                  = ulp_core_ptp_guard_2phy_stats0_rd2clr_config_WRITEMASK;
    ulpCorePtr->ptp_guard_2core.s.stats0.s.rd2clr_config.dw                 = ulp_core_ptp_guard_2core_stats0_rd2clr_config_WRITEMASK;
    xusb3Ptr->rx_partner_fifo.s.write_engine.s.stats0.s.rd2clr_config.dw    = xusb3_rx_partner_fifo_write_engine_stats0_rd2clr_config_WRITEMASK;
    xusb3Ptr->rx_partner_fifo.s.read_engine.s.stats0.s.rd2clr_config.dw     = xusb3_rx_partner_fifo_read_engine_stats0_rd2clr_config_WRITEMASK;
}

//#################################################################################################
// sets the split_dp_wait and split_dp_mode registers if splitting is needed
// both LEX and REX need to be programmed to be in the same mode, i.e., the split_dp_mode must be
// the same. However, the split_dp_wait can be different
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpHalSplitDp(void)
{
    // Require minimum 10 mcups (Max payload = 1024) and each mcup = 16 cycles for potential 8 channels
    xusb3Ptr->configuration.bf.split_dp_wait_time = 0xFFFF; // Max value
    xusb3Ptr->configuration.bf.split_dp_mode = 0x1;
}

//#################################################################################################
// Sets up the ULP core guards with the recommended values
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpHalSetupUlpCoreGuards(void)
{
    // Require minimum 10 mcups (Max payload = 1024) and each mcup = 16 cycles for potential 8 channels

    // PTP cycles with back pressure (drdy=1'b0) are excluded from the count
    ulpCorePtr->ptp_guard_2phy.s.config0.s.max_cycles.bf.max_cycle_mode = 1;

    // Sets the maximum number of clock cycles the PTP packet can last between its sop and eop taking into account the max_cycle_mode
    // Theoretically, max cycles should be 263, but the lowest we could go without errors is 264.
    // The suggested value (from Mohsen) is 265
    ulpCorePtr->ptp_guard_2phy.s.config0.s.max_cycles.bf.max_cycles = 265;

    // settings given by Mohsen in e-mail dated 10/15/2019
    ulpCorePtr->ptp_guard_2core.s.config0.s.max_cycles.bf.max_cycle_mode = 1;
    ulpCorePtr->ptp_guard_2core.s.config0.s.max_cycles.bf.max_cycles = 300;

    ilog_ULP_COMPONENT_2(ILOG_MINOR_EVENT, ULP_MAX_CYCLE,
        ulpCorePtr->ptp_guard_2phy.s.config0.s.max_cycles.bf.max_cycle_mode,
        ulpCorePtr->ptp_guard_2phy.s.config0.s.max_cycles.bf.max_cycles);

}

//#################################################################################################
// sets the values for new buffer rx_partner_fifo
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpHalRxPartnerFifo(void)
{
    xusb3Ptr->rx_partner_fifo.s.write_engine.s.config0.s.mode.bf.pkt_strm         = 0x0; // 0 = streaming
    xusb3Ptr->rx_partner_fifo.s.write_engine.s.config0.s.mode.bf.flw_ctrl_en      = 0x1;    // flow control enabled
    xusb3Ptr->rx_partner_fifo.s.write_engine.s.config0.s.mode.bf.drp_on_pkt_err   = 0x0;
    xusb3Ptr->rx_partner_fifo.s.write_engine.s.config0.s.mode.bf.drp_on_sop       = 0x0;
    xusb3Ptr->rx_partner_fifo.s.write_engine.s.config0.s.clear.bf.wclr            = 0x0;
    xusb3Ptr->rx_partner_fifo.s.read_engine.s.config0.s.clear.bf.rclr             = 0x0;

    if(bb_core_getLinkMode() == CORE_LINK_MODE_AQUANTIA)
    {
        // email from Mohsen Oct 2,2019
        if(bb_top_IsDeviceLex())
        {
            xusb3Ptr->rx_partner_fifo.s.write_engine.s.config0.s.notify_limit.bf.ntfy_lmt = 648;
        }
        else    // REX
        {
            xusb3Ptr->rx_partner_fifo.s.write_engine.s.config0.s.notify_limit.bf.ntfy_lmt = 500;
        }
    }
    else if(bb_core_getLinkMode() == CORE_LINK_MODE_ONE_LANE_SFP_PLUS)
    {
        xusb3Ptr->rx_partner_fifo.s.write_engine.s.config0.s.notify_limit.bf.ntfy_lmt = 412; //
    }

#if 0
    // TODO: this numbers need to be confirmed by Mohsen
    if(bb_core_getLinkMode() == CORE_LINK_MODE_AQUANTIA)
    {
        if(bb_top_IsDeviceLex())
        {
            xusb3Ptr->rx_partner_fifo.s.write_engine.s.config0.s.notify_limit.bf.ntfy_lmt = 256;
        }
        else    // REX
        {
            xusb3Ptr->rx_partner_fifo.s.write_engine.s.config0.s.notify_limit.bf.ntfy_lmt = 640;
        }
    }
    else if(bb_core_getLinkMode() == CORE_LINK_MODE_ONE_LANE_SFP_PLUS)
    {
        xusb3Ptr->rx_partner_fifo.s.write_engine.s.config0.s.notify_limit.bf.ntfy_lmt = 600;
    }
#endif
}

//#################################################################################################
// Control function to enable/disable pending_hp_timer
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpPendingHpTimerCtrl(bool enable)
{
    ulpCorePtr->configuration.bf.pending_hp_timer_en  = enable;
}

// Static Function Definitions ####################################################################


