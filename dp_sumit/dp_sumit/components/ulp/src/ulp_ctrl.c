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
// This file contains the implementation of LEX ULP bringup.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// The current design expects a VBus detection on the fly to enable USB3 and USB2. The normal
// sequence of ULP events lists as follow:
//      * LEX detects VBus enable from vbus_det interrupt.
//      * LEX sends a VBus detected CPU message to REX.
//      * REX sets up USB3. Once it is done, notify LEX through a CPU message.
//      * LEX sets up USB3 and send REX a CPU message to enable GE at REX.
//      * REX brings GE out of reset and send LEX a CPU message.
//      * LEX enables GE.
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <cpu_comm.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <gpio.h>
#include <interrupts.h>
#include <ulp.h>
#include "ulp_log.h"
#include "ulp_loc.h"
#include <mca.h>
#include <uart.h>
#include <i2cd_cypress_hx3.h>

// Constants and Macros ###########################################################################
#define CYPRESS_RESET_TIME_IN_US (10000)

// no spec on how long to wait after deasserting the Phy reset, use 100 microseconds
#define ULP_PHY_RESET_DEASSERT_DELAY            (100)

// section 5.3.1.4 Power up sequence of the TUSB1310A USB phy chip, says to
// wait around 300 microseconds after bringing the Phy PLL out of reset
// wait 500uS just to be sure
#define ULP_PHY_STATUS_TO_PLL_DEASSERT_DELAY    (500)

//timeout for PLL lock - should only go to a small fraction of this before it is locked
#define PLL_LOCK_TIMEOUT_US     (5000)

// Data Types #####################################################################################

// Static Function Declarations ###################################################################
//static void ULP_controlBringUpUlp(void)             __attribute__((section(".atext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################


//#################################################################################################
// Controls vBus, independent of whether this is a Lex or Rex
//
// Parameters:
//      enable                  - true to turn vBus on
// Return:
// Assumptions:
//
//#################################################################################################
void UlpVbusControl(bool enable)
{
    if (bb_top_IsDeviceLex())
    {
        UlpPhyVbusControl(enable);
    }
    else
    {
        UlpRexSetVbus(enable);
    }
}


//#################################################################################################
// Controls the Phy vBus reading
//
// Parameters:
//      enable                  - true to turn vBus on
// Return:
// Assumptions:
//
//#################################################################################################
void UlpPhyVbusControl(bool enable)
{
        bb_top_applyUsb3PhyCtrlVbusPresent(enable);
        ilog_ULP_COMPONENT_0(ILOG_DEBUG, (enable ? ULP_CTRL_PHY_USB3_VBUS_ON : ULP_CTRL_PHY_USB3_VBUS_OFF));
}


//#################################################################################################
// Initialize USB 3 on the Rex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ULP_controlRexInitUSB3(void)
{
    UlpVbusControl(false);                      // make sure vBus is off
    UlpHalSetSSdisable();                       // go to disable

    UlpHalSetupUlpCoreGuards();                 // make sure PTP guards are setup
    UlpHalControlCreditHpTimer(true);           //

    _ULP_halControlAutoRxTerminations(false);   // make sure Rx terminations won't be turned off automatically in some states
    UlpHalControlRxTerminations(false);         // make sure RX terminations are off

    _ULP_halUsb3SetLowPowerStates(ULP_U3_POWER_STATE);
    _ULP_halUsb3SetLowPowerStates(ULP_SS_DISABLED_POWER_STATE);

    _ULP_halUsb3ConfigInternalLoopback(true);   // use internal loopback, until the Phy can support it
    UlpHalSplitDp();                            // sets values for split_dp_wait and split_dp_mode registers
    UlpHalRxPartnerFifo();                      // Sets values for new buffer rx_partner_fifo

    UlpPendingHpTimerCtrl(true);               // Sets value for pending_hp_timer_en

    TOPLEVEL_setPollingMask(SECONDARY_INT_ULP_INT_CORE_MSK);  // link is up, allow polled interrupts
}


//#################################################################################################
// Initialize USB 3 on the Lex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ULP_controlLexInitUSB3(void)
{
    // before bringing ULP out of reset, make sure vBus is controlled by software
    bb_top_applyUsb3PhyCtrlVbusSwCtrl(true);    // put Vbus under SW control
    UlpVbusControl(false);                      // vBus is not present, initially!

    _ULP_halUsb3SetUpstreamPort(true);          // set port to upstream (Lex),

    UlpHalSetSSdisable();                       // go to disable

    UlpHalSetupUlpCoreGuards();                 // make sure PTP guards are setup
    UlpHalControlCreditHpTimer(true);           //

    _ULP_halControlAutoRxTerminations(false);   // make sure Rx terminations won't be turned off automatically in some states
    UlpHalControlRxTerminations(false);       // make sure RX terminations are off

    // just enable standby; U1, U2 are not supported at this time
//    _ULP_halUsb3SetLowPowerStates(ULP_U1_POWER_STATE);
//    _ULP_halUsb3SetLowPowerStates(ULP_U2_POWER_STATE);
    _ULP_halUsb3SetLowPowerStates(ULP_U3_POWER_STATE);
    _ULP_halUsb3SetLowPowerStates(ULP_SS_DISABLED_POWER_STATE);
    _ULP_halControlLowPowerWaitMode(true);      // wait to exit standby

    _ULP_halUsb3ConfigInternalLoopback(true);   // use internal loopback, until the Phy can support it
    _ULP_halControlWaitInPolling(false);        // make sure we aren't waiting in polling
    _ULP_halSetHotResetWait(true);              // wait the next time we get into Hot Reset
    UlpHalSplitDp();                            // sets values for split_dp_wait and split_dp_mode registers
    UlpHalRxPartnerFifo();                      // Sets values for new buffer rx_partner_fifo

    UlpPendingHpTimerCtrl(true);               // Sets value for pending_hp_timer_en

    TOPLEVEL_setPollingMask(SECONDARY_INT_ULP_INT_CORE_MSK);  // link is up, allow polled interrupts
}


//#################################################################################################
// Bring up USB 3 on the Lex
//
// Parameters:
// Return:
// Assumptions: Lex should be in the SS_DISABLED state
//#################################################################################################
void ULP_controlLexBringUpUSB3(void)
{
    UlpHalSetRxDetect();  // go to Rx Detect state
    _ULP_halControlLowPowerWaitMode(true);     // wait in U3 for the Rex to get to U0

    // enable interrupts - can only be done once ULP has been brought out of reset
    UlpHalEnableUlpInterrupts( LEX_ULP_USB3_IRQ0, LEX_ULP_USB3_IRQ1);

    UlpVbusControl(true);                   // vBus is present!

    // Ok, turn on ULP stats
    ulp_StatMonCoreControl(true);
}

//#################################################################################################
// Puts the ULP core state machine into the disabled state on either the Lex or Rex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ULP_controlTurnOffUSB3(void)
{
    // turn everything off, and force us to the disabled state
    UlpHalControlRxTerminations(false);         // go back into snoop mode
    UlpVbusControl(false);                      // vBus is not present until the lex/rex is enabled again
    UlpHalSetSSdisable();                       // go to the disabled state

    UlpStatusChange(ULP_STATUS_LTSSM_DISABLED);   // Tell UPP we've gone to the disabled state

    // these settings are mainly for the Lex; on the Rex, they should already be turned off
    _ULP_halControlWaitInPolling(false);        // no longer wait in polling, when coming out of a warm reset
    _ULP_halSetHotResetWait(false);             // make sure we no longer wait in hot reset
    _ULP_halControlLowPowerWaitMode(false);     // exit the low power (U3) wait mode

    ulp_StatMonCoreControl(false);              // turn off the ULP core stats
    ulp_StatMonPhyControl(false);               // turn off phy stats count
}


// Static Function Definitions ####################################################################






