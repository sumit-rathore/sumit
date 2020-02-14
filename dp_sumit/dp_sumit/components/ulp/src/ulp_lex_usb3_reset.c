//#################################################################################################
// Icron Technology Corporation - Copyright 2016W
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
// This file contains the Lex USB3 reset state machine
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//
//  Does a thorough reset of all of the USB3 components, synchronizing with the Rex
//
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <cpu_comm.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <callback.h>
#include <event.h>
#include <ulp.h>
#include "ulp_log.h"
#include <mca.h>
#include "ulp_loc.h"

#include <ulp_core_regs.h>

#include <uart.h>

// Constants and Macros ###########################################################################

#define ULP_LEX_RESET_STEP_INTERVAL      10  // give 10ms for each reset step

#define ULP_LEX_ONLY_RESET_STEP_INTERVAL    10  // give 30ms for each reset step
#define ULP_LEX_ONLY_RESET_SYNCH_DELAY      400 // give 400ms for the Host to detect that we aren't there

// give 300ms before kicking the Rex again to start resetting USB3
#define ULP_LEX_RESET_REX_START         300

// USB3 reset should take ~100ms, so it should be done in 1 second
#define ULP_LEX_RESET_MASTER_TIMEOUT    1000

//timeout for PLL lock - should only go to a small fraction of this before it is locked
#define PLL_LOCK_TIMEOUT_US     (500)


// Data Types #####################################################################################
enum UlpResetLexState
{
    ULP_LEX_RESET_INACTIVE,                     // reset is inactive
    ULP_LEX_RESET_WAIT_REX_START,               // wait for the Rex to start

    ULP_LEX_RESET_TAKE_DOWN_USB3_WAIT_REX,      // Wait for the Rex to take down USB3
    ULP_LEX_RESET_TAKE_DOWN_USB3_PAUSE,         // pause before going on to the next state

    ULP_LEX_RESET_TAKE_DOWN_MCA_WAIT_REX,       // Wait for the Rex to take down its MCA channels
    ULP_LEX_RESET_TAKE_DOWN_MCA_PAUSE,          // pause before going on to the next state

    ULP_LEX_RESET_APPLY_ULP_RESET_WAIT_REX,     // Wait for the Rex to reset its ULP
    ULP_LEX_RESET_APPLY_ULP_RESET_PAUSE,        // pause before going on to the next state

    ULP_LEX_RESET_APPLY_PHY_RESET_WAIT_REX,     // Wait for the Rex to reset its USB3 Phy
    ULP_LEX_RESET_APPLY_PHY_RESET_PAUSE,        // pause before going on to the next state

    ULP_LEX_RESET_BRING_UP_USB_PHY_WAIT_REX,    // Wait for the Rex to Bring up its USB3 Phy
    ULP_LEX_RESET_BRING_UP_USB_PHY_PAUSE,       // pause before going on to the next state

    ULP_LEX_RESET_BRING_UP_USB_PHY_2_WAIT_REX,  // Wait for the Rex to Bring up its USB3 Phy (stage 2)
    ULP_LEX_RESET_BRING_UP_USB_PHY_2_PAUSE,     // pause before going on to the next state

    ULP_LEX_RESET_BRING_UP_ULP_WAIT_REX,        // Wait for the Rex to Bring up its ULP
    ULP_LEX_RESET_BRING_UP_ULP_PAUSE,           // pause before going on to the next state

    ULP_LEX_RESET_BRING_UP_MCA_WAIT_REX,        // Wait for the Rex to bring up its MCA channels
    ULP_LEX_RESET_BRING_UP_MCA_PAUSE,           // pause before going on to the next state
};

enum UlpResetLexEvent
{
    ULP_LEX_RESET_EVENT_START_RESET,                // start the reset cycle
    ULP_LEX_RESET_EVENT_STOP_RESET,                 // stop the reset cycle

    ULP_LEX_RESET_EVENT_PAUSE_COMPLETE,             // pause between set up stages complete

    ULP_LEX_RESET_EVENT_REX_START,                  // Rex has started reset
    ULP_LEX_RESET_EVENT_REX_TOOK_DOWN_USB3,         // Rex has taken down USB3
    ULP_LEX_RESET_EVENT_REX_APPLIED_ULP_RESET,      // Rex has put its ULP into reset
    ULP_LEX_RESET_EVENT_REX_TOOK_DOWN_MCA,          // Rex has taken down its MCA channels
    ULP_LEX_RESET_EVENT_REX_APPLIED_PHY_RESET,      // Rex has put its USB3 Phy into reset
    ULP_LEX_RESET_EVENT_REX_BROUGHT_UP_PHY,         // Rex has brought its Phy out of reset
    ULP_LEX_RESET_EVENT_REX_PHY_STAGE_2,            // Rex has brought its Phy out of reset, stage 2
    ULP_LEX_RESET_EVENT_REX_BROUGHT_UP_ULP,         // Rex has taken its ULP out of reset
    ULP_LEX_RESET_EVENT_REX_BROUGHT_UP_MCA,         // Rex has brought its MCA channels out of reset

    ULP_LEX_RESET_EVENT_MASTER_TIMEOUT,             // Reset taking too long - timeout
};

enum UlpResetLexOnlyState
{
    ULP_LEX_ONLY_RESET_START,               // start the Lex only reset cycle
    ULP_LEX_ONLY_RESET_TAKE_DOWN_MCA,      // take the MCA channel down, next
    ULP_LEX_ONLY_RESET_TAKE_DOWN_USB3,      // put the Lex ULP core into reset
    ULP_LEX_ONLY_RESET_TAKE_DOWN_USB3_PHY,  // put the Lex ULP PHY into reset
    ULP_LEX_ONLY_RESET_BRING_UP_USB3_PHY,   // bring the USB3 Phy out of reset
    ULP_LEX_ONLY_RESET_BRING_UP_USB3_PHY_START_PLL,     // Start PHY PLL
    ULP_LEX_ONLY_RESET_BRING_UP_USB3_PHY_WAIT_PLL_LOCK, // Pause for PLL lock
    ULP_LEX_ONLY_RESET_DONE,                    // Reset finished
    ULP_LEX_ONLY_RESET_ERROR,                   // Reset error state reached

};

// Static Function Declarations ###################################################################
static void UlpLexUsb3ResetStepTimeout(void)                                                    __attribute__((section(".lexatext")));
static void UlpLexUsb3ResetRexStartTimeout(void)                                                __attribute__((section(".lexatext")));
//static void UlpLexUsb3ResetMasterTimeout(void)                                                  __attribute__((section(".lexatext")));

static void UlpLexUsb3ResetStateMachineSendEvent(enum UlpResetLexEvent event)                   __attribute__((section(".lexatext")));
static void UlpLexUsb3ResetStateCallback(void *param1, void *param2)                            __attribute__((section(".lexatext")));

static void UlpLexUsb3ResetStateMachine(enum UlpResetLexEvent lexUsb3ResetEvent)                __attribute__((section(".lexatext")));
static void UlpLexUsb3ResetPauseStates(enum UlpResetLexEvent lexUsb3ResetEvent)                 __attribute__((section(".lexatext")));
static void UlpLexUsb3ResetRexWaitStates(enum UlpResetLexEvent lexUsb3ResetEvent)               __attribute__((section(".lexatext")));

static void UlpLexUsb3ChannelStatus(enum MCA_ChannelStatus channelStatus)                       __attribute__((section(".lexatext")));

static void UlpLexUsb3ResetLexOnly(void)                                                        __attribute__((section(".lexatext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct UlpResetLexUsb3
{
    enum UlpResetLexState ulpFsmState;  // current ULP Lex reset state
//    uint8_t controlFlags;               // flags controlling whether this state machine is enabled/disabled

    TIMING_TimerHandlerT resetStepTimer;        // timer used to pace the reset steps
    TIMING_TimerHandlerT resetStartRexTimer;    // timer to poll Rex until it starts reset

    bool resetActive;                           // set if reset is in progress
    bool lexOnlyResetActive;                    // set if a Lex only reset is active

    enum UlpResetLexOnlyState   lexOnlyResetState;  // the state we are in while a Lex only reset is in progress
    TIMING_TimerHandlerT        lexResetOnlyTimer;  // timer to step through resetting the Lex

} ulpLexUsb3Reset;

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Setup the USB3 reset state machine
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_UlpLexResetUsb3Init(void)
{
    MCA_ChannelInit(MCA_CHANNEL_NUMBER_USB3, UlpLexUsb3ChannelStatus, NULL);

    ulpLexUsb3Reset.resetStepTimer = TIMING_TimerRegisterHandler(
       UlpLexUsb3ResetStepTimeout,
        false,
        ULP_LEX_RESET_STEP_INTERVAL);

    ulpLexUsb3Reset.resetStartRexTimer = TIMING_TimerRegisterHandler(
        UlpLexUsb3ResetRexStartTimeout,
        false,
        ULP_LEX_RESET_REX_START);

    ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_INACTIVE;
    ulpLexUsb3Reset.resetActive = false;

    ulpLexUsb3Reset.lexResetOnlyTimer = TIMING_TimerRegisterHandler(
        UlpLexUsb3ResetLexOnly,
        false,
        ULP_LEX_ONLY_RESET_STEP_INTERVAL);


}


//#################################################################################################
// Start the USB3 reset state machine
//
// Parameters:
// Return:
// Notes:
//
//#################################################################################################
void ULP_UlpLexUsb3ResetStart(void)
{
    if ( ULP_LexUsb3SystemEnabled() &&
        (ulpLexUsb3Reset.resetActive == false) &&
        EVENT_GetEventInfo(ET_COMLINK_STATUS_CHANGE) )
    {
        ULP_UlpLexUsb3LexOnlyResetStop();   // make sure the Lex only reset is stopped!
        UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_START_RESET);
        ULP_LexUsb3Control(ULP_USB_CONTROL_RESET_ACTIVE);         // reset is started
    }
}

//#################################################################################################
// Stop the USB3 reset state machine
//
// Parameters:
// Return:
// Notes:
//
//#################################################################################################
void ULP_UlpLexUsb3ResetStop(void)
{
    UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_STOP_RESET);
}

//#################################################################################################
// Start the USB3 Lex only reset state machine
//
// Parameters:
// Return:
// Notes:
//
//#################################################################################################
void ULP_UlpLexUsb3LexOnlyResetStart(void)
{
    ulpLexUsb3Reset.lexOnlyResetState = ULP_LEX_ONLY_RESET_START;

    // make sure the time interval is set correctly
    TIMING_TimerResetTimeout(ulpLexUsb3Reset.lexResetOnlyTimer, ULP_LEX_ONLY_RESET_STEP_INTERVAL);
    ulpLexUsb3Reset.lexOnlyResetActive = true;

    // tell the Rex that a Lex only reset is starting
    UlpSendCPUCommLexUsb3ResetMessage(ULP_LEX_TO_REX_USB3_RESET_LEX_ONLY_RESET_REQ);
}


//#################################################################################################
// Stop the USB3 Lex only reset state machine
//
// Parameters:
// Return:
// Notes:
//
//#################################################################################################
void ULP_UlpLexUsb3LexOnlyResetStop(void)
{
    TIMING_TimerStop(ulpLexUsb3Reset.lexResetOnlyTimer);  // make sure the timer is stopped
    ulpLexUsb3Reset.lexOnlyResetState = ULP_LEX_ONLY_RESET_ERROR;
    ulpLexUsb3Reset.lexOnlyResetActive = false;

    // tell the Rex that the Lex only reset is done
    UlpSendCPUCommLexUsb3ResetMessage(ULP_LEX_TO_REX_USB3_RESET_LEX_ONLY_RESET_DONE);
}

//#################################################################################################
// handles the USB3 messages CPU messages from the Rex
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_LexUsb3ResetRxCpuMessageHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    iassert_ULP_COMPONENT_1(msgLength == 0, ULP_LEX_UNEXPECTED_CPU_MSG_LENGTH, msgLength);
    ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_LEX_RCV_REX_USB3_RESET_MSG, subType);

    enum UlpRexSendUsb3ResetMessage rexUsb3ResetMessage = (enum UlpRexSendUsb3ResetMessage)subType;

    switch (rexUsb3ResetMessage)
    {
        case ULP_REX_TO_LEX_USB3_RESET_DISABLED:     // To Lex: Rex USB3 reset not allowed (USB3 disabled)
            UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_STOP_RESET);
           break;

        case ULP_REX_TO_LEX_USB3_RESET_LEX_ONLY_RESET_ACK:   // To Lex: Lex only reset acknowledged
            UlpLexUsb3ResetLexOnly();                        // start a Lex only reset
            break;

        case ULP_REX_TO_LEX_USB3_RESET_START:            // To Lex: USB3 reset started
            UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_REX_START);
            break;

        case ULP_REX_TO_LEX_USB3_RESET_TAKE_DOWN_USB3:   // To Lex: USB3 taken down
            UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_REX_TOOK_DOWN_USB3);
            break;

        case ULP_REX_TO_LEX_USB3_RESET_TAKE_DOWN_MCA:    // To Lex: MCA USB3 channel shut down
            UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_REX_TOOK_DOWN_MCA);
            break;

        case ULP_REX_TO_LEX_USB3_RESET_ULP_RESET:        // To Lex: ULP reset
            UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_REX_APPLIED_ULP_RESET);
            break;

        case ULP_REX_TO_LEX_USB3_RESET_PHY_RESET:        // To Lex: PHY reset
            UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_REX_APPLIED_PHY_RESET);
            break;

        case ULP_REX_TO_LEX_USB3_RESET_ACTIVATE_PHY:     // To Lex: Phy taken out of reset
            UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_REX_BROUGHT_UP_PHY);
            break;

        case ULP_REX_TO_LEX_USB3_RESET_ACTIVATE_PHY_2:   // To Lex: Phy reset, stage 2
            UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_REX_PHY_STAGE_2);
            break;

        case ULP_REX_TO_LEX_USB3_RESET_ACTIVATE_ULP:     // To Lex: activate ULP
            UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_REX_BROUGHT_UP_ULP);
            break;

        case ULP_REX_TO_LEX_USB3_RESET_BRING_UP_MCA:     // To Lex: bring up MCA
            UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_REX_BROUGHT_UP_MCA);
            break;

        default:
            break;
    }
}


// Static Function Definitions ####################################################################

//#################################################################################################
// Entry point to send an event to the Host port state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpLexUsb3ResetStateMachineSendEvent(enum UlpResetLexEvent event)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = event;

    CALLBACK_Run(UlpLexUsb3ResetStateCallback, (void *)eventx, NULL);
}

//#################################################################################################
// Callback wrapper for the Host port state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpLexUsb3ResetStateCallback(void *param1, void *param2)
{
    UlpLexUsb3ResetStateMachine( (enum UlpResetLexEvent)param1 );
}


//#################################################################################################
// The entry point for the USB3 Reset state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpLexUsb3ResetStateMachine(enum UlpResetLexEvent lexUsb3ResetEvent)
{
    uint8_t oldState = ulpLexUsb3Reset.ulpFsmState;

    switch (lexUsb3ResetEvent)
    {
        case ULP_LEX_RESET_EVENT_START_RESET:                // start the reset cycle
        {
            enum UlpLexSendUsb3ResetMessage lexMessage = ULP_LEX_TO_REX_USB3_RESET_START_HARD_RESET;

            UlpSendCPUCommLexUsb3ResetMessage(lexMessage);    // tell the Rex to start USB3 reset
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_WAIT_REX_START;
            TIMING_TimerStart(ulpLexUsb3Reset.resetStartRexTimer);  // continue sending messages until we hear back from the Rex
        }
           break;

        case ULP_LEX_RESET_EVENT_STOP_RESET:                 // stop the reset cycle
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_INACTIVE;
            ulpLexUsb3Reset.resetActive = false;
            TIMING_TimerStop(ulpLexUsb3Reset.resetStartRexTimer);  // ok, stop pestering the Rex - reset has stopped
            ULP_LexUsb3Control(ULP_USB_CONTROL_RESET_DONE);
           break;

        case ULP_LEX_RESET_EVENT_REX_START:                  // Rex has started reset
        case ULP_LEX_RESET_EVENT_PAUSE_COMPLETE:             // pause between reset stages is complete
            UlpLexUsb3ResetPauseStates(lexUsb3ResetEvent);
            break;

        case ULP_LEX_RESET_EVENT_REX_TOOK_DOWN_USB3:         // Rex has taken down USB3
        case ULP_LEX_RESET_EVENT_REX_APPLIED_ULP_RESET:      // Rex has put its ULP into reset
        case ULP_LEX_RESET_EVENT_REX_APPLIED_PHY_RESET:      // Rex has put its USB3 Phy into reset
        case ULP_LEX_RESET_EVENT_REX_TOOK_DOWN_MCA:          // Rex has taken down its MCA channels
        case ULP_LEX_RESET_EVENT_REX_BROUGHT_UP_PHY:         // Rex has brought its Phy out of reset
        case ULP_LEX_RESET_EVENT_REX_PHY_STAGE_2:            // Rex has brought its Phy out of reset, stage 2
        case ULP_LEX_RESET_EVENT_REX_BROUGHT_UP_ULP:         // Rex has taken its ULP out of reset
        case ULP_LEX_RESET_EVENT_REX_BROUGHT_UP_MCA:         // Rex has brought its MCA channels out of reset
            UlpLexUsb3ResetRexWaitStates(lexUsb3ResetEvent);
            break;

        case ULP_LEX_RESET_EVENT_MASTER_TIMEOUT:             // Reset taking too long - timeout

        default:
            break;
    }

    ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_LEX_USB3_RESET_STATE_MSG, oldState, lexUsb3ResetEvent, ulpLexUsb3Reset.ulpFsmState);
}


//#################################################################################################
// Called after a pause has been completed, and we are ready to go on to the next state
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpLexUsb3ResetPauseStates(enum UlpResetLexEvent lexUsb3ResetEvent)
{
    switch (ulpLexUsb3Reset.ulpFsmState)
    {
        case ULP_LEX_RESET_WAIT_REX_START:                  // Rex has started reset
            ULP_controlTurnOffUSB3();       // turn off the ULP core state machine, and related stats on the Lex

            ulpLexUsb3Reset.resetActive = true;    // reset is active
            // tell the Rex to start taking down USB3
            UlpSendCPUCommLexUsb3ResetMessage(ULP_LEX_TO_REX_USB3_RESET_TAKE_DOWN_USB3);
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_TAKE_DOWN_USB3_WAIT_REX;

            TIMING_TimerStop(ulpLexUsb3Reset.resetStartRexTimer);  // ok, stop pestering the Rex - reset has started
            break;

        case ULP_LEX_RESET_TAKE_DOWN_USB3_PAUSE:         // Wait after take down of USB3
            // tell the Rex to start taking down the MCA channel - there should be no more traffic, now
            UlpSendCPUCommLexUsb3ResetMessage(ULP_LEX_TO_REX_USB3_RESET_TAKE_DOWN_MCA);
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_TAKE_DOWN_MCA_WAIT_REX;

            ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_LTSSM_VALUE_AFTER_DISABLE, UlpHalGetLTSSM());
            UlpLexProgramCypressHub();              // start reprogramming the hub
            UlpStatusChange(ULP_STATUS_IN_RESET);   // tell UPP it is disabled
            MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_USB3);
            break;

        case ULP_LEX_RESET_TAKE_DOWN_MCA_PAUSE:          // Wait after taking down the USB3 MCA channel
            // tell the Rex to start taking down ULP
            UlpSendCPUCommLexUsb3ResetMessage(ULP_LEX_TO_REX_USB3_RESET_ULP_RESET);
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_APPLY_ULP_RESET_WAIT_REX;

            bb_top_ApplyResetUlpCore(true);              // ULP core in reset
            bb_top_applyUsb3PhyCtrlOutputEnable(false);  // turn off the Phy outputs
            break;

        case ULP_LEX_RESET_APPLY_ULP_RESET_PAUSE:        // Wait after ULP reset
            // tell the Rex to start taking down the Phy
            UlpSendCPUCommLexUsb3ResetMessage(ULP_LEX_TO_REX_USB3_RESET_PHY_RESET);
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_APPLY_PHY_RESET_WAIT_REX;

            bb_top_ApplyResetUlpPhy(true);              // ULP Phy in reset
            bb_top_ApplyResetUlpPhyClkPll(true);
            bb_top_applyUsb3PhyCtrlRst(true);
            bb_top_applyUsb3PhyCtrlStrappingDone(false);
            bb_top_UlpPhyTxClockControl(false);         // disable USB Phy Tx clock
            break;

        case ULP_LEX_RESET_APPLY_PHY_RESET_PAUSE:        // Wait after USB3 Phy reset
            // ok - USB3 Phy, ULP, and MCA is in reset - start bringing things up
            bb_top_ApplyResetXusb(false);

            bb_top_UlpPhyTxClockControl(true);         // enable USB Phy Tx clock
            bb_top_applyUsb3PhyCtrlRst(false);
            bb_top_applyUsb3PhyCtrlStrappingDone(true);
            bb_top_applyUsb3PhyCtrlOutputEnable(true);

            UlpStatusChange(ULP_STATUS_ENABLED);                 // enable UPP now, so we can access registers correctly
            bb_top_ApplyResetUlpCore(false);  // now bring the ULP core out of reset

            UlpSendCPUCommLexUsb3ResetMessage(ULP_LEX_TO_REX_USB3_RESET_ACTIVATE_PHY);
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_BRING_UP_USB_PHY_WAIT_REX;
            break;

        case ULP_LEX_RESET_BRING_UP_USB_PHY_PAUSE:       // Wait after activating the USB Phy
            // stage 2 of bringing up the Phy
            UlpSendCPUCommLexUsb3ResetMessage(ULP_LEX_TO_REX_USB3_RESET_ACTIVATE_PHY_2);
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_BRING_UP_USB_PHY_2_WAIT_REX;

            bb_top_ApplyResetUlpPhyClkPll(false);
            MCA_ChannelLinkUp(MCA_CHANNEL_NUMBER_USB3);
           break;

        case ULP_LEX_RESET_BRING_UP_USB_PHY_2_PAUSE:     // Wait after bringing up the USB3 Phy (stage 2)
            // bring up ULP
            UlpSendCPUCommLexUsb3ResetMessage(ULP_LEX_TO_REX_USB3_RESET_ACTIVATE_ULP);
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_BRING_UP_ULP_WAIT_REX;

            // wait for PLL lock
            LEON_TimerValueT time = LEON_TimerRead();
            while ( !bb_top_isUlpPhyClkLocked())
            {
                if (LEON_TimerCalcUsecDiff(time, LEON_TimerRead()) > PLL_LOCK_TIMEOUT_US)
                {
                    ilog_ULP_COMPONENT_0(ILOG_USER_LOG, ULP_ULP_CORE_PLL_LOCK_FAIL);
                    break;
                }
            }
            ilog_ULP_COMPONENT_1(ILOG_USER_LOG, ULP_CORE_RESET_CALLED, LEON_TimerCalcUsecDiff(time, LEON_TimerRead()));

            bb_top_ApplyResetUlpPhy(false);         // bring ULP Phy out of reset
            break;


        case ULP_LEX_RESET_BRING_UP_ULP_PAUSE:           // Wait after bringing up the ULP
            // ULP is out of reset - turn on the MCA
            UlpSendCPUCommLexUsb3ResetMessage(ULP_LEX_TO_REX_USB3_RESET_BRING_UP_MCA);
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_BRING_UP_MCA_WAIT_REX;
            break;

        case ULP_LEX_RESET_BRING_UP_MCA_PAUSE:           // Wait after bringing up the USB3 MCA channels
            MCA_ChannelTxRxSetup(MCA_CHANNEL_NUMBER_USB3);

            // this is set when the channel is fully operational, Rx, Tx, and Link
//            ULP_LexUsb3Control(ULP_USB_CONTROL_RESET_DONE);
//            ULP_controlLexBringUpUSB3();    // setup ULP for Lex
            break;


        case ULP_LEX_RESET_INACTIVE:                     // reset is inactive
        case ULP_LEX_RESET_TAKE_DOWN_USB3_WAIT_REX:      // Wait for the Rex to take down USB3
        case ULP_LEX_RESET_TAKE_DOWN_MCA_WAIT_REX:       // Wait for the Rex to take down its MCA channels
        case ULP_LEX_RESET_APPLY_ULP_RESET_WAIT_REX:     // Wait for the Rex to reset its ULP
        case ULP_LEX_RESET_APPLY_PHY_RESET_WAIT_REX:     // Wait for the Rex to reset its USB3 Phy
        case ULP_LEX_RESET_BRING_UP_USB_PHY_WAIT_REX:    // Wait for the Rex to Bring up its USB3 Phy
        case ULP_LEX_RESET_BRING_UP_USB_PHY_2_WAIT_REX:  // Wait for the Rex to Bring up its USB3 Phy (stage 2)
        case ULP_LEX_RESET_BRING_UP_ULP_WAIT_REX:        // Wait for the Rex to Bring up its ULP
        case ULP_LEX_RESET_BRING_UP_MCA_WAIT_REX:        // Wait for the Rex to bring up its MCA channels
        default:
            // we can get extra starts because we might have sent extra starts to the Rex
            if (lexUsb3ResetEvent != ULP_LEX_RESET_EVENT_REX_START)
            {
                ilog_ULP_COMPONENT_2(ILOG_USER_LOG, ULP_LEX_USB3_INVALID_RESET_STATE_MSG, ulpLexUsb3Reset.ulpFsmState, lexUsb3ResetEvent);
            }
            break;
    }
}


//#################################################################################################
// Verifies the correct event from the given state, and goes on to the next state, after a pause
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb3ResetRexWaitStates(enum UlpResetLexEvent lexUsb3ResetEvent)
{
    enum UlpResetLexEvent expectedEvent = ULP_LEX_RESET_EVENT_START_RESET; // just set to a invalid value

    switch (ulpLexUsb3Reset.ulpFsmState)
    {
        case ULP_LEX_RESET_TAKE_DOWN_USB3_WAIT_REX:      // Wait for the Rex to take down USB3
        case ULP_LEX_RESET_TAKE_DOWN_USB3_PAUSE:         // Wait after take down of USB3 - might get some extra ULP_LEX_RESET_EVENT_REX_START events here
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_TAKE_DOWN_USB3_PAUSE;
            expectedEvent = ULP_LEX_RESET_EVENT_REX_TOOK_DOWN_USB3;
            break;

        case ULP_LEX_RESET_TAKE_DOWN_MCA_WAIT_REX:       // Wait for the Rex to take down its MCA channels
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_TAKE_DOWN_MCA_PAUSE;
            expectedEvent = ULP_LEX_RESET_EVENT_REX_TOOK_DOWN_MCA;
            break;

        case ULP_LEX_RESET_APPLY_ULP_RESET_WAIT_REX:     // Wait for the Rex to reset its ULP
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_APPLY_ULP_RESET_PAUSE;
            expectedEvent = ULP_LEX_RESET_EVENT_REX_APPLIED_ULP_RESET;
            break;

        case ULP_LEX_RESET_APPLY_PHY_RESET_WAIT_REX:     // Wait for the Rex to reset its USB3 Phy
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_APPLY_PHY_RESET_PAUSE;
            expectedEvent = ULP_LEX_RESET_EVENT_REX_APPLIED_PHY_RESET;
            break;

        case ULP_LEX_RESET_BRING_UP_USB_PHY_WAIT_REX:    // Wait for the Rex to Bring up its USB3 Phy
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_BRING_UP_USB_PHY_PAUSE;
            expectedEvent = ULP_LEX_RESET_EVENT_REX_BROUGHT_UP_PHY;
            break;

        case ULP_LEX_RESET_BRING_UP_USB_PHY_2_WAIT_REX:  // Wait for the Rex to Bring up its USB3 Phy (stage 2)
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_BRING_UP_USB_PHY_2_PAUSE;
            expectedEvent = ULP_LEX_RESET_EVENT_REX_PHY_STAGE_2;
            break;

        case ULP_LEX_RESET_BRING_UP_ULP_WAIT_REX:        // Wait for the Rex to Bring up its ULP
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_BRING_UP_ULP_PAUSE;
            expectedEvent = ULP_LEX_RESET_EVENT_REX_BROUGHT_UP_ULP;
            break;

        case ULP_LEX_RESET_BRING_UP_MCA_WAIT_REX:        // Wait for the Rex to bring up its MCA channels
            ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_BRING_UP_MCA_PAUSE;
            expectedEvent = ULP_LEX_RESET_EVENT_REX_BROUGHT_UP_MCA;
            break;

        case ULP_LEX_RESET_INACTIVE:                     // reset is inactive
        case ULP_LEX_RESET_WAIT_REX_START:               // wait for the Rex to start
        case ULP_LEX_RESET_TAKE_DOWN_MCA_PAUSE:          // Wait after taking down the USB3 MCA channel
        case ULP_LEX_RESET_APPLY_ULP_RESET_PAUSE:        // Wait after ULP reset
        case ULP_LEX_RESET_APPLY_PHY_RESET_PAUSE:        // Wait after USB3 Phy reset
        case ULP_LEX_RESET_BRING_UP_USB_PHY_PAUSE:       // Wait after activating the USB Phy
        case ULP_LEX_RESET_BRING_UP_USB_PHY_2_PAUSE:     // Wait after bringing up the USB3 Phy (stage 2)
        case ULP_LEX_RESET_BRING_UP_ULP_PAUSE:           // Wait after Bring up its ULP
        case ULP_LEX_RESET_BRING_UP_MCA_PAUSE:           // Wait after bringing up the USB3 MCA channels
            break;

        default:
            break;
    }

    iassert_ULP_COMPONENT_2(expectedEvent == lexUsb3ResetEvent, ULP_LEX_RCV_UNEXPECTED_USB3_RESET_MSG, expectedEvent, lexUsb3ResetEvent);
    TIMING_TimerStart(ulpLexUsb3Reset.resetStepTimer);  // wait a bit before going on
}


//#################################################################################################
// LEX checks the status of LTSSM register.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpLexUsb3ResetStepTimeout(void)
{
    UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_PAUSE_COMPLETE);
}

//#################################################################################################
// Polls to see if the Rex is enabled; if so, will ping the Rex for reset until it responds
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpLexUsb3ResetRexStartTimeout(void)
{
    ULP_UlpLexUsb3ResetStart();   // keep on trying until we hear from the Rex
}

//#################################################################################################
// LEX checks the status of LTSSM register.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
/*
static void UlpLexUsb3ResetMasterTimeout(void)
{
    UlpLexUsb3ResetStateMachineSendEvent(ULP_LEX_RESET_EVENT_PAUSE_COMPLETE);
}
*/

//#################################################################################################
// Handler for USB3 MCA channel status changes
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
static void UlpLexUsb3ChannelStatus(enum MCA_ChannelStatus channelStatus)
{
    ilog_ULP_COMPONENT_2(ILOG_MINOR_EVENT, ULP_LEX_USB3_CHANNEL_STATUS, channelStatus, ulpLexUsb3Reset.lexOnlyResetActive);

    if (ulpLexUsb3Reset.lexOnlyResetActive)
    {
        switch (channelStatus)
        {
            case MCA_CHANNEL_STATUS_LINK_ACTIVE:        // channel is linked between Lex and Rex
                MCA_ChannelTxRxSetup(MCA_CHANNEL_NUMBER_USB3);
                break;

            case MCA_CHANNEL_STATUS_CHANNEL_READY:      // channel is linked, and Rx, Tx is setup.  Ready for operation
                // ULP is out of reset - turn on the MCA
                ULP_controlLexInitUSB3();               // initialize ULP for Lex
                ULP_controlLexBringUpUSB3();
                TIMING_TimerStart(ulpLexUsb3Reset.lexResetOnlyTimer);  // wait a bit before going on
                break;

            case MCA_CHANNEL_STATUS_CHANNEL_DISABLED:   // channel is disabled
            case MCA_CHANNEL_STATUS_LINK_DOWN:          // channel is down between Lex and Rex, needs to be re-initialized
            default:
                break;
        }
    }
    else
    {
        switch (channelStatus)
        {
            case MCA_CHANNEL_STATUS_LINK_ACTIVE:        // channel is linked between Lex and Rex
                // both the Rex and Lex gets this event, at almost the same time
                // - the Rex sends us a message when the link is active
                break;

            case MCA_CHANNEL_STATUS_CHANNEL_READY:      // channel is linked, and Rx, Tx is setup.  Ready for operation
                ULP_LexUsb3Control(ULP_USB_CONTROL_MCA_CHANNEL_RDY);    // signal the channel is available
                ULP_LexUsb3Control(ULP_USB_CONTROL_RESET_DONE);         // and reset is done
                ULP_LexHostUsb3ResetDone();                             // tell the host state machine, too

                ULP_controlLexInitUSB3();               // initialize ULP for Lex
                ulpLexUsb3Reset.ulpFsmState = ULP_LEX_RESET_INACTIVE;   // done reset
                ulpLexUsb3Reset.resetActive = false;

            ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_LTSSM_VALUE_AFTER_DISABLE, UlpHalGetLTSSM());
                break;

            case MCA_CHANNEL_STATUS_CHANNEL_DISABLED:   // channel is disabled
            case MCA_CHANNEL_STATUS_LINK_DOWN:          // channel is down between Lex and Rex, needs to be re-initialized
            default:
                ULP_LexUsb3Control(ULP_USB_CONTROL_MCA_CHANNEL_DOWN); // USB3 MCA channel is down (Lex side)
                ULP_UlpLexUsb3ResetStart();   // disconnected - go through a reset cycle (if one isn't already in progress)
                break;
        }
    }
}



//#################################################################################################
// Lex only reset - used for those states when reseting the REx as well will cause issues
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpLexUsb3ResetLexOnly(void)
{
    bool restartTimer = true;

    if (ulpLexUsb3Reset.lexOnlyResetActive)
    {
        switch (ulpLexUsb3Reset.lexOnlyResetState)
        {
            case ULP_LEX_ONLY_RESET_START:               // start the Lex only reset cycle
                ULP_controlTurnOffUSB3();       // turn off the ULP core state machine, and related stats on the Lex
                break;

            case ULP_LEX_ONLY_RESET_TAKE_DOWN_MCA:      // take the MCA channel down, next
                UlpStatusChange(ULP_STATUS_IN_RESET);   // tell UPP it is disabled
                MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_USB3);
                break;

            case ULP_LEX_ONLY_RESET_TAKE_DOWN_USB3:      // put the Lex ULP core into reset
                bb_top_ApplyResetUlpCore(true);             // ULP core in reset
                bb_top_applyUsb3PhyCtrlOutputEnable(false); // turn off the Phy outputs
                break;

            case ULP_LEX_ONLY_RESET_TAKE_DOWN_USB3_PHY:  // put the Lex ULP PHY into reset
                bb_top_ApplyResetUlpPhy(true);              // ULP Phy in reset
                bb_top_ApplyResetUlpPhyClkPll(true);
                bb_top_applyUsb3PhyCtrlRst(true);
                bb_top_applyUsb3PhyCtrlStrappingDone(false);
                bb_top_UlpPhyTxClockControl(false);         // disable USB Phy Tx clock
                TIMING_TimerResetTimeout(ulpLexUsb3Reset.lexResetOnlyTimer, ULP_LEX_ONLY_RESET_SYNCH_DELAY);
                break;

            case ULP_LEX_ONLY_RESET_BRING_UP_USB3_PHY:      // bring the USB3 Phy out of reset
                bb_top_UlpPhyTxClockControl(true);          // enable USB Phy Tx clock
                bb_top_applyUsb3PhyCtrlRst(false);
                bb_top_applyUsb3PhyCtrlStrappingDone(true);
                bb_top_applyUsb3PhyCtrlOutputEnable(true);

                UlpStatusChange(ULP_STATUS_ENABLED);                 // enable UPP now, so we can access registers correctly
                bb_top_ApplyResetUlpCore(false);  // now bring the ULP core out of reset
                TIMING_TimerResetTimeout(ulpLexUsb3Reset.lexResetOnlyTimer, ULP_LEX_ONLY_RESET_STEP_INTERVAL);
                break;

            case ULP_LEX_ONLY_RESET_BRING_UP_USB3_PHY_START_PLL:   // Start PHY PLL
                // stage 2 of bringing up the Phy
                bb_top_ApplyResetUlpPhyClkPll(false);
                break;

            case ULP_LEX_ONLY_RESET_BRING_UP_USB3_PHY_WAIT_PLL_LOCK: // Pause for PLL lock
                iassert_ULP_COMPONENT_0(bb_top_isUlpPhyClkLocked(), ULP_LEX_USB3_RESET_PLL_NOT_LOCKED);

                bb_top_ApplyResetUlpPhy(false);         // bring ULP Phy out of reset

                MCA_ChannelLinkUp(MCA_CHANNEL_NUMBER_USB3);
                UlpSendCPUCommLexUsb3ResetMessage(ULP_LEX_TO_REX_USB3_RESET_LEX_ONLY_RESET_MCA_UP);    // To Rex: bring up your MCA channel
                restartTimer = false;                 // don't start the timer
                break;

            case ULP_LEX_ONLY_RESET_DONE:                // Reset complete
                UlpLexUsb3LexOnlyResetDone();
                ulpLexUsb3Reset.lexOnlyResetActive = false;

                // tell the Rex that the Lex only reset is done
                UlpSendCPUCommLexUsb3ResetMessage(ULP_LEX_TO_REX_USB3_RESET_LEX_ONLY_RESET_DONE);

                // fall through and stop going on to the next state
            case ULP_LEX_ONLY_RESET_ERROR:
            default:
                // make sure the state gets set to a known value
                ulpLexUsb3Reset.lexOnlyResetState = ULP_LEX_ONLY_RESET_ERROR;
                restartTimer = false;                 // don't start the timer
                break;

        }

        ilog_ULP_COMPONENT_1(ILOG_MINOR_EVENT, ULP_LEX_USB3_RESET_STATE, ulpLexUsb3Reset.lexOnlyResetState);

        if (restartTimer)
        {
            TIMING_TimerStart(ulpLexUsb3Reset.lexResetOnlyTimer);  // wait a bit before going on
        }

        ulpLexUsb3Reset.lexOnlyResetState++;    // go on to the next state
    }
}



