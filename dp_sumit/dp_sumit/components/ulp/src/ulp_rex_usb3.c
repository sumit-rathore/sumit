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
// This file contains the implementation of REX ULP (USB Link Protocol) USB3.
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
#include <callback.h>
#include <ulp.h>
#include "ulp_log.h"
#include "ulp_loc.h"
#include <mca.h>

#include <uart.h>
#include <ulp_core_regs.h>
#include <event.h>

// Constants and Macros ###########################################################################
#define REX_LTSSM_TIMER_PERIOD_IN_MS        100

// the amount of time to wait for a device to be connected on USB3 (1 second)
#define REX_DEVICE_CONNECTED_WAIT_TIME   (1*1000)

// - work on this - see if we don't detect 0x61, if we need to do something with the state we are in.  Set up a poll task for LTSSM?
#define REX_LTSSM_TOTAL_TIMEOUT_IN_US   (2*1000*1000)     // should take max 2s for valid link training

// a delay to turn on Rx Terminations after they have been off, so the devices can see it
// note that this also delays the Lex, and is meant to allow the host downstream port to see the Lex
// isn't there anymore.  This time needs to take into account the worst case time for the
// host port (and any devices attached to the Rex) to detect that USB3 is disabled
#define REX_RX_TERMINATION_DELAY        300

//timeout for PLL lock - should only go to a small fraction of this before it is locked
#define PLL_LOCK_TIMEOUT_US     (100)

// IRQ0 ULP interrupts
#define REX_ULP_USB3_IRQ0 \
       (ULP_CORE_IRQ0_RAW_RX_TERMINATION_DET    | \
        ULP_CORE_IRQ0_RAW_IN_HOT_RESET          | \
        ULP_CORE_IRQ0_RAW_COMPLETED_HOT_RESET   | \
        ULP_CORE_IRQ0_RAW_IN_WARM_RESET         | \
        ULP_CORE_IRQ0_RAW_COMPLETED_WARM_RESET  | \
        ULP_CORE_IRQ0_RAW_IN_U3                 | \
        ULP_CORE_IRQ0_RAW_U3_EXIT_INITIATED     | \
        ULP_CORE_IRQ0_RAW_U3_EXIT_COMPLETED)

// IRQ1 ULP interrupts
#define REX_ULP_USB3_IRQ1 \
       (ULP_CORE_IRQ1_RAW_IN_INACTIVE           | \
        ULP_CORE_IRQ1_RAW_IN_RX_DETECT          | \
        ULP_CORE_IRQ1_RAW_IN_POLLING            | \
        ULP_CORE_IRQ1_RAW_IN_U0                 | \
        ULP_CORE_IRQ1_RAW_IN_DISABLED           | \
        ULP_CORE_IRQ1_RAW_IN_COMPLIANCE         | \
        ULP_CORE_IRQ1_RAW_IN_LOOPBACK           | \
        ULP_CORE_IRQ1_RAW_IN_RECOVERY)

// Data Types #####################################################################################
enum RexUlpUsb3State
{
    REX_ULP_USB3_STATE_DISABLED,    // USB3 port state machine disabled

    REX_ULP_USB3_STATE_DELAY_READY, // minimum down time before we say USB3 is ready
    REX_ULP_USB3_STATE_READY,       // USB3 port state machine ready

    REX_ULP_USB3_REX_DEVICE_POLL,   // Waiting for indication on the Rex a device is connected

    REX_ULP_USB3_REX_SETUP,         // Waiting for indication on the Rex USB3 is setup (at U0)
    REX_ULP_USB3_LEX_SETUP,         // Waiting for indication on the Lex USB3 is setup (at U0)
    REX_ULP_USB3_LINK_ACTIVE,       // USB3 link active on Lex and Rex

    REX_ULP_USB3_LINK_FAILED,       // USB3 failed to setup on either the Rex or Lex; waiting for a disable

    REX_ULP_USB3_HOT_RESET_WAIT,    // waiting for Hot Reset to finish
    REX_ULP_USB3_WARM_RESET_WAIT,   // waiting for Warm Reset to finish
    REX_ULP_USB3_IN_STANDBY_MODE,   // in standby mode
};

// TODO add a master timeout event, so if we get stuck we will go back to the idle state
enum RexUlpUsb3Event
{
    REX_ULP_USB3_EVENT_DISABLED,            // disable/exit USB3 link
    REX_ULP_USB3_EVENT_ENABLED,             // enable USB3 link setup

    REX_ULP_USB3_EVENT_HOST_CONNECT,        // Host is connected on the Lex
    REX_ULP_USB3_EVENT_HOST_DISCONNECT,     // Host connection has been removed

    REX_ULP_USB3_EVENT_DEVICE_CONNECTED,    // Device has just connected to the Rex
    REX_ULP_USB3_EVENT_DEVICE_REMOVED,      // Device has been removed from the Rex

    REX_ULP_USB3_EVENT_REX_AT_SS_INACTIVE,  // Rex USB 3 link is in the inactive state
    REX_ULP_USB3_EVENT_REX_AT_U0,           // Rex USB 3 link is setup

    REX_ULP_USB3_EVENT_LEX_AT_U0,           // Lex USB 3 link is setup
    REX_ULP_USB3_EVENT_LEX_FAILED,          // Lex failed USB3 link setup

    REX_ULP_USB3_EVENT_LEX_IN_STANDBY,      // Lex is in standby mode
    REX_ULP_USB3_EVENT_LEX_EXITED_STANDBY,  // Lex exited standby mode
    REX_ULP_USB3_EVENT_WARM_RESET_REQUEST,  // Lex is requesting a warm reset on the Rex
    REX_ULP_USB3_EVENT_WARM_RESET_DONE,     // Warm reset completed on Rex

    REX_ULP_USB3_EVENT_STANDBY_REQUEST,     // Rex is requested to go into Standby (U3)
    REX_ULP_USB3_EVENT_STANDBY_EXIT_REQUEST,   // Rex has exited from standby
    REX_ULP_USB3_EVENT_LEX_IN_HOT_RESET,    // Lex is in Hot Reset
    REX_ULP_USB3_EVENT_HOT_RESET_DONE,      // Hot reset completed on Rex
};


// Static Function Declarations ###################################################################
static void ULP_RexUsb3StateMachineSendEvent(enum RexUlpUsb3Event event)    __attribute__((section(".rexatext")));
static void ULP_RexUsb3StateCallback(void *param1, void *param2)            __attribute__((section(".rexatext")));

static void ULP_RexUsb3StateMachine(enum RexUlpUsb3Event lexHostEvent)      __attribute__((section(".rexatext")));
static void UlpRexUsb3StateDisabled(enum RexUlpUsb3Event event)             __attribute__((section(".rexatext")));
static void UlpRexUsb3StateDelayedReady(enum RexUlpUsb3Event event)         __attribute__((section(".rexatext")));
static void UlpRexUsb3StateReady(enum RexUlpUsb3Event event)                __attribute__((section(".rexatext")));
static void UlpRexUsb3StateDevicePoll(enum RexUlpUsb3Event event)           __attribute__((section(".rexatext")));
static void UlpRexUsb3StateRexSetup(enum RexUlpUsb3Event event)             __attribute__((section(".rexatext")));
static void UlpRexUsb3StateLexSetup(enum RexUlpUsb3Event event)             __attribute__((section(".rexatext")));
static void UlpRexUsb3StateActive(enum RexUlpUsb3Event event)               __attribute__((section(".rexatext")));
static void UlpRexUsb3StateHotResetWait(enum RexUlpUsb3Event event)         __attribute__((section(".rexatext")));
static void UlpRexUsb3StateWarmResetWait(enum RexUlpUsb3Event event)        __attribute__((section(".rexatext")));
static void UlpRexUsb3StateInStandby(enum RexUlpUsb3Event event)            __attribute__((section(".rexatext")));
static void UlpRexUsb3StateFailed(enum RexUlpUsb3Event event)               __attribute__((section(".rexatext")));

static void RexSetupUSB3(void)                                              __attribute__((section(".rexatext")));
static void RexUsb3Disabled(void)                                           __attribute__((section(".rexatext")));
static void RexUsb3Disconnected(void)                                       __attribute__((section(".rexatext")));
static void RexUsb3Failed(void)                                             __attribute__((section(".rexatext")));
static void RexUsb3DeviceRemoved(void)                                      __attribute__((section(".rexatext")));
static void RexUsb3GoToInactive(void)                                       __attribute__((section(".rexatext")));
static void RexUsb3SeriousError(void)                                       __attribute__((section(".rexatext")));
static void RexUsb3TurnOffUsb3(void)                                        __attribute__((section(".rexatext")));

static void RexUsb3ReadyDelayTimeout(void)                                  __attribute__((section(".rexatext")));
static void RexUsb3DeviceConnectTimeout(void)                               __attribute__((section(".rexatext")));
static void RexUsb3StartSetupPolling(void)                                  __attribute__((section(".rexatext")));
static void RexUsb3LtssmPoll(void)                                          __attribute__((section(".rexatext")));
static void RexUsb3SetupHotResetState(void)                                 __attribute__((section(".rexatext")));
static void RexUsb3SetupWarmResetState(void)                                __attribute__((section(".rexatext")));

static uint32_t RexUsb3GetUsb3Status(void)                                  __attribute__((section(".rexatext")));
static void UlpRexUsb3ChannelStatus(enum MCA_ChannelStatus channelStatus)   __attribute__((section(".rexatext")));
static void ULP_RexUsb3OverCurrentIntHandler(void)                          __attribute__((section(".rexatext")));
static void ULP_RexUsb3OverCurrentIntCallback(void *param1, void *param2)   __attribute__((section(".rexatext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct RexUlpUsb3Context
{
    // Lex host port state
    // Lex ULP FSM state.
    enum RexUlpUsb3State ulpFsmState;
    uint8_t controlFlags;               // flags controlling whether this USB controller is enabled/disabled

    // A timer used to track the status of LTSSM register.
    TIMING_TimerHandlerT rexLtssmPollTimer;
    LEON_TimerValueT ltssmStartTime;    // the time we started checking the ltssm states

    // a timer used to delay when we say a device is connected/disconnected
    TIMING_TimerHandlerT deviceConnectedWaitTimer;

    TIMING_TimerHandlerT rexReadyDelayTimer;    // used to enforce a minimum Rx termination down time

    bool lexOnlyResetActive;            // true if the Lex is resetting

} rexUsb3Ulp;

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Initialize the REX ULP module and the ULP finite state machine.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void ULP_RexUsb3Init(bool enabled)
{
    rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_STATE_DISABLED;
    MCA_ChannelInit(MCA_CHANNEL_NUMBER_USB3, UlpRexUsb3ChannelStatus, NULL);

    // setup a timer to poll the USB3 connection every 100ms
    rexUsb3Ulp.rexLtssmPollTimer = TIMING_TimerRegisterHandler(
        RexUsb3LtssmPoll,
        false,
        REX_LTSSM_TIMER_PERIOD_IN_MS);

    // setup a timer to give a timeout to check the device status
    rexUsb3Ulp.deviceConnectedWaitTimer = TIMING_TimerRegisterHandler(
        RexUsb3DeviceConnectTimeout,
        false,
        REX_DEVICE_CONNECTED_WAIT_TIME);

    // setup a timer to delay before we turn on Rx terminations
    rexUsb3Ulp.rexReadyDelayTimer = TIMING_TimerRegisterHandler(
        RexUsb3ReadyDelayTimeout,
        false,
        REX_RX_TERMINATION_DELAY);

    EVENT_Register(ET_USB3_STATUS_CHANGE, RexUsb3GetUsb3Status);

    // Set GPIO over current detection interrupt
    GpioRegisterIrqHandler(GPIO_CONN_USB_OVER_CURRENT, GPIO_IRQ_RISING_OR_FALLING_EDGE, ULP_RexUsb3OverCurrentIntHandler);


    // Strap setting outputs after OutputEnable Low
    // OutputEnable works as in/out mux inside FPGA.
    if(bb_top_getCoreBoardRev() >= BOM_23_00200_A03)    // A03 Board use Clock input
    {                                                   // A01,02 used XTAL input (default strap)
        bb_top_applyUsb3PhyCtrlXtalDisable(true);
        bb_top_applyUsb3PhyCtrlSscDisable(true);
    }

    // only enable USB3 if we are supposed to
    if (enabled)
    {
        ULP_RexUsb3Control(ULP_USB_CONTROL_ENABLE); // USB3 is enabled!
    }
}


//#################################################################################################
// Process the interrupts from the IRQ0 register
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_RexUsb3Irq0(uint32_t rexUlpInts)
{
    if (rexUlpInts & ULP_CORE_IRQ0_RAW_RX_TERMINATION_DET)
    {
        if (UlpHalRxTerminationsPresent() )
        {
            ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_DEVICE_CONNECTED);
        }
        else
        {
            ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_DEVICE_REMOVED);
        }
    }

    if (rexUlpInts & ULP_CORE_IRQ0_RAW_IN_WARM_RESET)
    {
        if (rexUsb3Ulp.lexOnlyResetActive)
        {
            UlpSendCPUCommRexUsb3ResetMessage(ULP_REX_TO_LEX_USB3_RESET_LEX_ONLY_RESET_ACK);
            MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_USB3);
        }
    }

    if (rexUlpInts & ULP_CORE_IRQ0_RAW_COMPLETED_HOT_RESET)
    {
        ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_HOT_RESET_DONE);
    }
    if (rexUlpInts & ULP_CORE_IRQ0_RAW_COMPLETED_WARM_RESET)
    {
        ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_WARM_RESET_DONE);
    }

    // are we in U3?
    if (rexUlpInts &  ULP_CORE_IRQ0_RAW_IN_U3)
    {
    }

    // have we started to exit from U3?
    if (rexUlpInts &  ULP_CORE_IRQ0_RAW_U3_EXIT_INITIATED)
    {
        // we've started exiting from standby - tell the state machine we've exited from standby
        // so it can get the U0 transition
        ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_STANDBY_EXIT_REQUEST);
    }

    // have we exited from U3?
    if (rexUlpInts &  ULP_CORE_IRQ0_RAW_U3_EXIT_COMPLETED)
    {
    }
}


//#################################################################################################
// Process the interrupts from the IRQ1 register
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_RexUsb3Irq1(uint32_t rexUlpInts)
{
    // see if we are in the disabled state
    if (rexUlpInts &  ULP_CORE_IRQ1_RAW_IN_U0)
    {
        if (rexUsb3Ulp.ulpFsmState == REX_ULP_USB3_REX_SETUP)
        {
            ilog_ULP_COMPONENT_1(
                ILOG_MINOR_EVENT,
                ULP_LTSSM_VALID,
                LEON_TimerCalcUsecDiff(rexUsb3Ulp.ltssmStartTime, LEON_TimerRead()));

            rexUsb3Ulp.ltssmStartTime = LEON_TimerRead();
        }
        GpioEnableIrq(GPIO_CONN_USB_OVER_CURRENT);                              // Start Overcurrent detection

        ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_REX_AT_U0);     // host USB3 setup complete
    }

    if (rexUlpInts &  ULP_CORE_IRQ1_RAW_IN_INACTIVE)
    {
        ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_REX_AT_SS_INACTIVE);     // Rex Link is inactive
    }
}


//#################################################################################################
// Sends control operations to the USB3 module.  If the module is enabled, and connected,
// it will run.  Otherwise it will stop
//
// Parameters:
// Return:  true if the module is enabled - false otherwise
// Assumptions:
//      * Always call ULP_RexUsb3Control() before ULP_RexUsb2Control() - Usb3 needs to be
//        enabled first! (device tries USB3 first, and then if that fails, tries USB2)
//#################################################################################################
bool ULP_RexUsb3Control(enum UlpUsbControl controlOperation)
{
    enum UlpUsbControlResult controlResult = ULP_UsbSetControl(&rexUsb3Ulp.controlFlags, controlOperation);

    switch (controlResult)
    {
        case ULP_USB_CONTROL_RESULT_ENABLE:             // this USB module can be enabled
            ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_ENABLED);
            break;

        case ULP_USB_CONTROL_RESULT_DISABLE:            // this USB module can be disabled
        default:
            // This event can detect Lex host disconnect or Link cable disconnect
            GpioDisableIrq(GPIO_CONN_USB_OVER_CURRENT); // Stop Overcurrent detection

            ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_DISABLED);
            break;

        case ULP_USB_CONTROL_RESULT_UNCHANGED:       // no change since the last time this module was called
            break;
    }

    ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_REX_USB3_SET_CONTROL_RESULT, rexUsb3Ulp.controlFlags, controlOperation, controlResult);

    return (ULP_UsbControlEnabled(rexUsb3Ulp.controlFlags));
}


//#################################################################################################
// returns true if USB3 on the Rex is active.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool RexUlpUsb3IsActive(void)
{
    bool active = false;    // ASSUME USB3 is inactive

    // use a switch case so if new states get added they will have to update this
    switch (rexUsb3Ulp.ulpFsmState)
    {
        case REX_ULP_USB3_REX_DEVICE_POLL:  // Waiting for indication on the Rex a device is connected
        case REX_ULP_USB3_REX_SETUP:        // Waiting for the Rex to indicate it has setup USB3
        case REX_ULP_USB3_LEX_SETUP:        // Waiting for the Lex to indicate it has setup USB3
        case REX_ULP_USB3_LINK_ACTIVE:      // USB3 active on Lex and Rex
        case REX_ULP_USB3_HOT_RESET_WAIT:   // waiting for hot reset to finish on the Rex
        case REX_ULP_USB3_WARM_RESET_WAIT:  // waiting for warm reset to finish
        case REX_ULP_USB3_IN_STANDBY_MODE:  // in standby mode
            active = true;  // USB3 is active
            break;

        case REX_ULP_USB3_STATE_DISABLED:   // USB3 state machine disabled
        case REX_ULP_USB3_STATE_DELAY_READY:    // minimum down time before we say USB3 is ready
        case REX_ULP_USB3_STATE_READY:      // USB3 port state machine ready
        case REX_ULP_USB3_LINK_FAILED:      // USB3 failed to setup on either the Rex or Lex
        default:
            break;
    }

    return (active);
}

//#################################################################################################
// handles the USB3 messages CPU messages from the Lex
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_RexUsb3RxCpuMessageHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    iassert_ULP_COMPONENT_1(msgLength == 0, ULP_REX_UNEXPECTED_CPU_MSG_LENGTH, msgLength);
    ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_REX_RCV_LEX_USB3_MSG, subType);

    enum UlpLexSendUsb3Message lexUsbMessage = (enum UlpLexSendUsb3Message)subType;

    // only process this message if the Rex USB3 is enabled!
    if (ULP_RexUsb3Enabled() )
    {
        switch (lexUsbMessage)
        {
            case ULP_LEX_TO_REX_USB3_MODULE_DISABLED:          // USB3 on the /Lex is disabled
                ULP_RexUsb3Control(ULP_USB_CONTROL_PARTNER_SHUTDOWN);  // partner is disabled!
                break;

            case ULP_LEX_TO_REX_USB3_MODULE_ENABLED:          // USB3 on the /Lex is enabled
                ULP_RexUsb3Control(ULP_USB_CONTROL_PARTNER_ENABLED);  // partner is enabled!
                ULP_RexUsb3Control(ULP_USB_CONTROL_PARTNER_READY);      // Lex USB3 is ready!
                break;

            case ULP_LEX_TO_REX_USB3_DISABLED:          // USB 3 disabled on the Lex
                //   ULP_RexUsb3Control(ULP_USB_CONTROL_DISCONNECT);  // treat this as a disconnect
                break;

            case ULP_LEX_TO_REX_USB3_SETUP_SUCCESS:     // USB 3 setup on Lex successful
                ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_LEX_AT_U0);
                break;

            case ULP_LEX_TO_REX_USB3_FAILURE:      // USB 3 failed on the Lex
                ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_LEX_FAILED);
                break;

            case ULP_LEX_TO_REX_USB3_CONNECTED:    // Lex has detected a USB 3 + 2 Host port
                ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_HOST_CONNECT);
                break;

            case ULP_LEX_TO_REX_USB3_DISCONNECTED:       // Lex far end terminations have been removed
                ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_HOST_DISCONNECT);
                break;

            case ULP_LEX_TO_REX_LEX_IN_HOT_RESET:        // Lex is in hot reset
                ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_LEX_IN_HOT_RESET);
                break;

            case ULP_LEX_TO_REX_USB3_WARM_RESET:         // Lex has gone through a USB3 warm reset
                ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_WARM_RESET_REQUEST);
                break;

            case ULP_LEX_TO_REX_LEX_IN_STANDBY:          // Lex is in standby
                ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_LEX_IN_STANDBY);
                break;

            case ULP_LEX_TO_REX_LEX_EXITING_STANDBY:    // Lex is exiting standby
                ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_LEX_EXITED_STANDBY);
                break;

            default:
                break;
        }
    }
    else if ( (lexUsbMessage != ULP_LEX_TO_REX_USB3_MODULE_DISABLED) && (lexUsbMessage != ULP_LEX_TO_REX_USB3_DISABLED) )
    {
        UlpSendCPUCommRexUsb3Message( ULP_REX_TO_LEX_USB3_DISABLED);
        return;
    }

}

//#################################################################################################
// handles the USB3 reset messages CPU messages from the Lex
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_RexUsb3ResetRxCpuMessageHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    iassert_ULP_COMPONENT_1(msgLength == 0, ULP_REX_UNEXPECTED_CPU_MSG_LENGTH, msgLength);
    ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_REX_RCV_LEX_USB3_RESET_MSG, subType);

    enum UlpLexSendUsb3ResetMessage lexUsbMessage = (enum UlpLexSendUsb3ResetMessage)subType;

    // only do the reset if the Rex USB3 is enabled!
    if (!ULP_RexUsb3Enabled() )
    {
        UlpSendCPUCommRexUsb3ResetMessage( ULP_REX_TO_LEX_USB3_RESET_DISABLED);
        return;
    }

    switch (lexUsbMessage)
    {
        case ULP_LEX_TO_REX_USB3_RESET_LEX_ONLY_RESET_REQ:      // To Rex: Lex Only Reset requested
            ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_WARM_RESET_REQUEST);
            rexUsb3Ulp.lexOnlyResetActive = true;
            break;

        case ULP_LEX_TO_REX_USB3_RESET_LEX_ONLY_RESET_MCA_UP:    // To Rex: bring up your MCA channel
            MCA_ChannelLinkUp(MCA_CHANNEL_NUMBER_USB3);
            break;

        case ULP_LEX_TO_REX_USB3_RESET_LEX_ONLY_RESET_DONE:  // To Rex: Lex Only Reset done
            if (rexUsb3Ulp.ulpFsmState == REX_ULP_USB3_LEX_SETUP)
            {
                // tell the Lex we are at U0, waiting...
                UlpSendCPUCommRexUsb3Message(ULP_REX_TO_LEX_USB3_REX_AT_U0);
            }
            rexUsb3Ulp.lexOnlyResetActive = false;
            break;

        case ULP_LEX_TO_REX_USB3_RESET_START_HARD_RESET: // To Rex: Start USB3 reset (bring down system)
            ULP_RexUsb3Control(ULP_USB_CONTROL_RESET_ACTIVE);         // reset is started
            UlpSendCPUCommRexUsb3ResetMessage(ULP_REX_TO_LEX_USB3_RESET_START);
            break;

        case ULP_LEX_TO_REX_USB3_RESET_START_USB3_RESTART:   // To Rex: Start USB3 reset->restart
            UlpSendCPUCommRexUsb3ResetMessage(ULP_REX_TO_LEX_USB3_RESET_START);
            break;

        case ULP_LEX_TO_REX_USB3_RESET_TAKE_DOWN_USB3:   // To Rex: take down USB3
            RexUsb3Disabled();  // take down USB3 on the Rex
            UlpSendCPUCommRexUsb3ResetMessage(ULP_REX_TO_LEX_USB3_RESET_TAKE_DOWN_USB3);
            break;

        case ULP_LEX_TO_REX_USB3_RESET_TAKE_DOWN_MCA:    // To Rex: take down MCA
            ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_LTSSM_VALUE_AFTER_DISABLE, UlpHalGetLTSSM());
//            UlpRexProgramCypressHub();      // start reprogramming the hub
            UlpStatusChange(ULP_STATUS_IN_RESET);   // tell UPP it is disabled
            MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_USB3);
            UlpSendCPUCommRexUsb3ResetMessage(ULP_REX_TO_LEX_USB3_RESET_TAKE_DOWN_MCA);
            break;

        case ULP_LEX_TO_REX_USB3_RESET_ULP_RESET:       // To Rex: reset ULP
            bb_top_ApplyResetUlpCore(true);
            bb_top_applyUsb3PhyCtrlOutputEnable(false);
            UlpSendCPUCommRexUsb3ResetMessage(ULP_REX_TO_LEX_USB3_RESET_ULP_RESET);
            break;

        case ULP_LEX_TO_REX_USB3_RESET_PHY_RESET:       // To Rex: reset PHY
            bb_top_ApplyResetUlpPhy(true);
            bb_top_ApplyResetUlpPhyClkPll(true);
            bb_top_applyUsb3PhyCtrlRst(true);
            bb_top_applyUsb3PhyCtrlStrappingDone(false);
            bb_top_UlpPhyTxClockControl(false);         // disable USB Phy Tx clock
//            bb_topApplyResetXusb(true);
            UlpSendCPUCommRexUsb3ResetMessage(ULP_REX_TO_LEX_USB3_RESET_PHY_RESET);
            break;

        case ULP_LEX_TO_REX_USB3_RESET_ACTIVATE_PHY:    // To Rex: take Phy out of reset
            bb_top_ApplyResetXusb(false);
            bb_top_UlpPhyTxClockControl(true);          // enable USB Phy Tx clock
            bb_top_applyUsb3PhyCtrlRst(false);
            bb_top_applyUsb3PhyCtrlOutputEnable(true);

            UlpStatusChange(ULP_STATUS_ENABLED);                 // enable UPP now, so we can access registers correctly
            bb_top_ApplyResetUlpCore(false);  // now bring the ULP out of reset

            UlpSendCPUCommRexUsb3ResetMessage(ULP_REX_TO_LEX_USB3_RESET_ACTIVATE_PHY);
            break;

        case ULP_LEX_TO_REX_USB3_RESET_ACTIVATE_PHY_2:   // To Rex: take Phy out of reset, stage 2
            bb_top_ApplyResetUlpPhyClkPll(false);
            // this message is sent when the MCA channel link is up
            //            UlpSendCPUCommRexUsb3ResetMessage(ULP_REX_TO_LEX_USB3_RESET_ACTIVATE_PHY_2);
            MCA_ChannelLinkUp(MCA_CHANNEL_NUMBER_USB3);
            break;

        case ULP_LEX_TO_REX_USB3_RESET_ACTIVATE_ULP:     // To Rex: activate ULP
        {
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

            bb_top_ApplyResetUlpPhy(false);
            UlpSendCPUCommRexUsb3ResetMessage(ULP_REX_TO_LEX_USB3_RESET_ACTIVATE_ULP);
        }
        break;

        case ULP_LEX_TO_REX_USB3_RESET_BRING_UP_MCA:     // To Rex: bring up MCA
            MCA_ChannelTxRxSetup(MCA_CHANNEL_NUMBER_USB3);
            UlpSendCPUCommRexUsb3ResetMessage(ULP_REX_TO_LEX_USB3_RESET_BRING_UP_MCA);

            // sent when the MCA Tx, Rx, and link are all up
//            ULP_controlRexInitUSB3();
//            ULP_RexUsb3Control(ULP_USB_CONTROL_RESET_DONE);         // and reset is done
            break;

        default:
            break;
    }
}


//#################################################################################################
// Returns true if USB3 is enabled
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool ULP_RexUsb3Enabled(void)
{
    if (rexUsb3Ulp.controlFlags & ULP_USB_MODULE_LOCAL_ENABLED)
    {
        return (true);  // USB3 is enabled
    }

    return (false);
}


// Static Function Definitions ####################################################################

//#################################################################################################
// Entry point to send an event to the Host port state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_RexUsb3StateMachineSendEvent(enum RexUlpUsb3Event event)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = event;

    CALLBACK_Run(ULP_RexUsb3StateCallback, (void *)eventx, NULL);
}


//#################################################################################################
// Callback wrapper for the Host port state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_RexUsb3StateCallback(void *param1, void *param2)
{
    ULP_RexUsb3StateMachine( (enum RexUlpUsb3Event)param1 );
}


//#################################################################################################
// The entry point for the Rex USB3 state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_RexUsb3StateMachine(enum RexUlpUsb3Event LexUsb3Event)
{
    uint8_t oldState = rexUsb3Ulp.ulpFsmState;

    switch (rexUsb3Ulp.ulpFsmState)
    {
        case REX_ULP_USB3_STATE_DISABLED:    // USB3 state machine disabled
            UlpRexUsb3StateDisabled(LexUsb3Event);
            break;

        case REX_ULP_USB3_STATE_DELAY_READY:    // minimum down time before we say USB3 is ready
            UlpRexUsb3StateDelayedReady(LexUsb3Event);
            break;

        case REX_ULP_USB3_STATE_READY:     // USB3 port state machine ready
            UlpRexUsb3StateReady(LexUsb3Event);
            break;

        case REX_ULP_USB3_REX_DEVICE_POLL:   // Waiting for an indication on the Rex a device is connected
            UlpRexUsb3StateDevicePoll(LexUsb3Event);
            break;

        case REX_ULP_USB3_REX_SETUP:         // Waiting for the Rex to indicate it has setup USB3
            UlpRexUsb3StateRexSetup(LexUsb3Event);
            break;

        case REX_ULP_USB3_LEX_SETUP:         // Waiting for the Lex to indicate it has setup USB3
            UlpRexUsb3StateLexSetup(LexUsb3Event);
            break;

        case REX_ULP_USB3_LINK_ACTIVE:      // USB3 active on Lex and Rex
            UlpRexUsb3StateActive(LexUsb3Event);
            break;

        case REX_ULP_USB3_HOT_RESET_WAIT:   // waiting for hot reset to finish on the Rex
            UlpRexUsb3StateHotResetWait(LexUsb3Event);
            break;

        case REX_ULP_USB3_WARM_RESET_WAIT:   // waiting for warm reset to finish on the Rex
            UlpRexUsb3StateWarmResetWait(LexUsb3Event);
            break;

        case REX_ULP_USB3_IN_STANDBY_MODE:   // in standby mode
            UlpRexUsb3StateInStandby(LexUsb3Event);
            break;

        case REX_ULP_USB3_LINK_FAILED:      // USB3 failed to setup on either the Rex or Lex
            UlpRexUsb3StateFailed(LexUsb3Event);
            break;

        default:
            break;
    }

    if (oldState != rexUsb3Ulp.ulpFsmState)
    {
        EVENT_Trigger(ET_USB3_STATUS_CHANGE, RexUsb3GetUsb3Status());
    }
    ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_REX_USB3_STATE_MSG, oldState, LexUsb3Event, rexUsb3Ulp.ulpFsmState);
}


//#################################################################################################
// Handles events in the REX_ULP_USB3_STATE_DISABLED
//
// Parameters:
//      event                   - REX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpRexUsb3StateDisabled(enum RexUlpUsb3Event event)
{
    switch (event)
    {
        case REX_ULP_USB3_EVENT_ENABLED:        // enable USB3 link setup
            rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_STATE_DELAY_READY;    // minimum down time before we say USB3 is ready
            TIMING_TimerStart(rexUsb3Ulp.rexReadyDelayTimer);   // start the delay timer
           break;

        case REX_ULP_USB3_EVENT_HOST_CONNECT:           // Host is connected on the Lex
        case REX_ULP_USB3_EVENT_HOST_DISCONNECT:        // Host connection has been removed
        case REX_ULP_USB3_EVENT_LEX_AT_U0:      // Lex USB 3 link is setup
        case REX_ULP_USB3_EVENT_LEX_FAILED:     // Lex failed USB3 link setup
        case REX_ULP_USB3_EVENT_LEX_IN_STANDBY: // Lex is in standby mode
        case REX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in Hot Reset
        case REX_ULP_USB3_EVENT_WARM_RESET_REQUEST:     // Lex is requesting a warm reset on the Rex
        case REX_ULP_USB3_EVENT_LEX_EXITED_STANDBY:     // Lex exited standby mode
            UlpSendCPUCommRexUsb3Message(ULP_REX_TO_LEX_USB3_DISABLED);  // tell Lex USB3 is disabled on the Rex
            break;

        case REX_ULP_USB3_EVENT_DISABLED:               // disable/exit USB3 link
        case REX_ULP_USB3_EVENT_DEVICE_CONNECTED:       // Device has just connected to the Rex
        case REX_ULP_USB3_EVENT_DEVICE_REMOVED:         // Device has been removed from the Rex
        case REX_ULP_USB3_EVENT_STANDBY_REQUEST:        // Rex is requested to go into Standby (U3)
        case REX_ULP_USB3_EVENT_STANDBY_EXIT_REQUEST:      // Exit from Standby on the Rex is requested
        case REX_ULP_USB3_EVENT_REX_AT_SS_INACTIVE:     // Rex USB 3 link is in the inactive state
        case REX_ULP_USB3_EVENT_REX_AT_U0:              // Rex USB 3 link is setup
            // TODO: handle these cases?

            // these events don't mean anything in this state
        case REX_ULP_USB3_EVENT_HOT_RESET_DONE:         // Hot reset completed on Rex
        case REX_ULP_USB3_EVENT_WARM_RESET_DONE:        // Warm reset completed on Rex
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_REX_USB3_UNEXPECTED_EVENT,
                event,
                rexUsb3Ulp.ulpFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in the REX_ULP_USB3_STATE_DELAY_READY - Enforces minimum down time for Rx
// terminations.  This is just a placeholder state, until the timer expires and goes to the ready state
//
// Parameters:
//      event                   - REX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpRexUsb3StateDelayedReady(enum RexUlpUsb3Event event)
{
    switch (event)
    {
        case REX_ULP_USB3_EVENT_DISABLED:   // disable/exit USB3 link
            RexUsb3Disabled();              // take down USB3
            break;

        case REX_ULP_USB3_EVENT_HOST_CONNECT:           // Host is connected on the Lex
        case REX_ULP_USB3_EVENT_HOST_DISCONNECT:        // Host connection has been removed
        case REX_ULP_USB3_EVENT_LEX_AT_U0:      // Lex USB 3 link is setup
        case REX_ULP_USB3_EVENT_LEX_FAILED:     // Lex failed USB3 link setup
        case REX_ULP_USB3_EVENT_LEX_IN_STANDBY: // Lex is in standby mode
        case REX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in Hot Reset
        case REX_ULP_USB3_EVENT_WARM_RESET_REQUEST:     // Lex is requesting a warm reset on the Rex
        case REX_ULP_USB3_EVENT_LEX_EXITED_STANDBY:     // Lex exited standby mode

        case REX_ULP_USB3_EVENT_ENABLED:        // enable USB3 link setup
        case REX_ULP_USB3_EVENT_DEVICE_CONNECTED:       // Device has just connected to the Rex
        case REX_ULP_USB3_EVENT_DEVICE_REMOVED:         // Device has been removed from the Rex
        case REX_ULP_USB3_EVENT_STANDBY_REQUEST:        // Rex is requested to go into Standby (U3)
        case REX_ULP_USB3_EVENT_STANDBY_EXIT_REQUEST:      // Exit from Standby on the Rex is requested
        case REX_ULP_USB3_EVENT_REX_AT_SS_INACTIVE:     // Rex USB 3 link is in the inactive state
        case REX_ULP_USB3_EVENT_REX_AT_U0:              // Rex USB 3 link is setup
            // TODO: handle these cases?

            // these events don't mean anything in this state
        case REX_ULP_USB3_EVENT_HOT_RESET_DONE:         // Hot reset completed on Rex
        case REX_ULP_USB3_EVENT_WARM_RESET_DONE:        // Warm reset completed on Rex
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_REX_USB3_UNEXPECTED_EVENT,
                event,
                rexUsb3Ulp.ulpFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in the REX_ULP_USB3_STATE_READY - Rex USB3 is ready, waiting for a host
// connect signal
//
// Parameters:
//      event                   - REX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpRexUsb3StateReady(enum RexUlpUsb3Event event)
{
    switch (event)
    {
        case REX_ULP_USB3_EVENT_DISABLED:   // disable/exit USB3 link
            RexUsb3Disabled();              // take down USB3
            break;

        case REX_ULP_USB3_EVENT_HOST_CONNECT:           // Host is connected on the Lex
            RexSetupUSB3();                             // go setup the ULP HW

            // TODO: do a device prefetch on USB3, and then go to standby until the Lex is available?
            rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_REX_DEVICE_POLL;    // go to the device poll state
            TIMING_TimerStart(rexUsb3Ulp.deviceConnectedWaitTimer);
            break;

        case REX_ULP_USB3_EVENT_HOST_DISCONNECT:        // Host connection has been removed
            UlpSendCPUCommRexUsb3Message(ULP_REX_TO_LEX_USB3_READY);  // tell Lex USB3 is ready on the Rex
            break;

        case REX_ULP_USB3_EVENT_ENABLED:                // enable USB3 link setup
        case REX_ULP_USB3_EVENT_LEX_AT_U0:              // Lex USB 3 link is setup
        case REX_ULP_USB3_EVENT_LEX_FAILED:             // Lex failed USB3 link setup
        case REX_ULP_USB3_EVENT_LEX_IN_STANDBY:         // Lex is in standby mode
        case REX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in Hot Reset
        case REX_ULP_USB3_EVENT_WARM_RESET_REQUEST:     // Lex is requesting a warm reset on the Rex
        case REX_ULP_USB3_EVENT_LEX_EXITED_STANDBY:     // Lex exited standby mode
        case REX_ULP_USB3_EVENT_DEVICE_CONNECTED:       // Device has just connected to the Rex
        case REX_ULP_USB3_EVENT_DEVICE_REMOVED:         // Device has been removed from the Rex
        case REX_ULP_USB3_EVENT_STANDBY_REQUEST:        // Rex is requested to go into Standby (U3)
        case REX_ULP_USB3_EVENT_STANDBY_EXIT_REQUEST:   // Exit from Standby on the Rex is requested
        case REX_ULP_USB3_EVENT_REX_AT_SS_INACTIVE:     // Rex USB 3 link is in the inactive state
        case REX_ULP_USB3_EVENT_REX_AT_U0:              // Rex USB 3 link is setup
            // TODO: handle these cases?

            // these events don't mean anything in this state
        case REX_ULP_USB3_EVENT_HOT_RESET_DONE:         // Hot reset completed on Rex
        case REX_ULP_USB3_EVENT_WARM_RESET_DONE:        // Warm reset completed on Rex
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_REX_USB3_UNEXPECTED_EVENT,
                event,
                rexUsb3Ulp.ulpFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in the REX_ULP_USB3_REX_DEVICE_POLL
//
// Parameters:
//      event                   - REX ULP event
// Return:
// Assumptions:  The device is not present when we enter this state, and we are waiting for
// a device connect event.
//
//#################################################################################################
static void UlpRexUsb3StateDevicePoll(enum RexUlpUsb3Event event)
{
    switch (event)
    {
        case REX_ULP_USB3_EVENT_DISABLED:   // disable/exit USB3 link
            RexUsb3Disabled();              // take down USB3
            break;

        case REX_ULP_USB3_EVENT_HOST_DISCONNECT:        // Host connection has been removed
            RexUsb3Disconnected();
            break;

        case REX_ULP_USB3_EVENT_LEX_FAILED: // Lex failed USB3 link setup - warm reset failure?
            RexUsb3Failed();        // go into the USB3 failure state
            break;

        case REX_ULP_USB3_EVENT_DEVICE_CONNECTED:       // Device has just connected to the Rex
            TIMING_TimerStop(rexUsb3Ulp.deviceConnectedWaitTimer);  // make sure connect timer is off!
            RexUsb3StartSetupPolling();                 // start polling for a valid link to the device
            break;

        case REX_ULP_USB3_EVENT_DEVICE_REMOVED:         // Device has been removed from the Rex
            break;

        case REX_ULP_USB3_EVENT_REX_AT_U0:  // Rex USB 3 link is setup (at U0)
            TIMING_TimerStop(rexUsb3Ulp.deviceConnectedWaitTimer);  // make sure connect timer is off!
            rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_LEX_SETUP;            // wait for the Lex to setup
            UlpSendCPUCommRexUsb3Message(ULP_REX_TO_LEX_USB3_REX_AT_U0);
            break;

        case REX_ULP_USB3_EVENT_REX_AT_SS_INACTIVE:     // Rex USB 3 link is in the inactive state
            RexUsb3GoToInactive();
            break;

        case REX_ULP_USB3_EVENT_WARM_RESET_REQUEST:     // Lex is requesting a warm reset on the Rex
            TIMING_TimerStop(rexUsb3Ulp.deviceConnectedWaitTimer);  // make sure connect timer is off!
            rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_WARM_RESET_WAIT;   // wait for the warm reset to finish
            _ULP_halSetWarmReset();  // generate a warm reset here, wait for it to be done
            break;

        case REX_ULP_USB3_EVENT_HOST_CONNECT:           // Host is connected on the Lex
        case REX_ULP_USB3_EVENT_STANDBY_REQUEST:        // Rex is requested to go into Standby (U3)
        case REX_ULP_USB3_EVENT_STANDBY_EXIT_REQUEST:   // Exit from Standby on the Rex is requested

        case REX_ULP_USB3_EVENT_LEX_AT_U0:              // Lex USB 3 link is setup
        case REX_ULP_USB3_EVENT_LEX_IN_STANDBY:         // Lex is in standby mode
        case REX_ULP_USB3_EVENT_LEX_EXITED_STANDBY:     // Lex exited standby mode
        case REX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in Hot Reset
            // TODO: these events shouldn't be here - assert, abort, or????
//            break;

        // these events don't mean anything in this state
        case REX_ULP_USB3_EVENT_ENABLED:     // enable USB3 link setup
        case REX_ULP_USB3_EVENT_HOT_RESET_DONE:         // Hot reset completed on Rex
        case REX_ULP_USB3_EVENT_WARM_RESET_DONE:        // Warm reset completed on Rex
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_REX_USB3_UNEXPECTED_EVENT,
                event,
                rexUsb3Ulp.ulpFsmState);
            break;
    }
}

//#################################################################################################
// Handles events in the REX_ULP_USB3_REX_SETUP
//
// Parameters:
//      event                   - REX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpRexUsb3StateRexSetup(enum RexUlpUsb3Event event)
{
    switch (event)
    {
        case REX_ULP_USB3_EVENT_DISABLED:   // disable/exit USB3 link
            RexUsb3Disabled();              // take down USB3
            break;

        case REX_ULP_USB3_EVENT_HOST_DISCONNECT:        // Host connection has been removed
            RexUsb3Disconnected();
            break;

        case REX_ULP_USB3_EVENT_LEX_FAILED: // Lex failed USB3 link setup - warm reset failure?
            RexUsb3Failed();        // go into the USB3 failure state
            break;

        case REX_ULP_USB3_EVENT_REX_AT_U0:  // Rex USB 3 link is setup (at U0)
            rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_LEX_SETUP;            // wait for the Lex to setup
            UlpSendCPUCommRexUsb3Message(ULP_REX_TO_LEX_USB3_REX_AT_U0);
            break;

        case REX_ULP_USB3_EVENT_REX_AT_SS_INACTIVE:     // Rex USB 3 link is in the inactive state
            RexUsb3GoToInactive();
            break;

        case REX_ULP_USB3_EVENT_WARM_RESET_REQUEST:     // Lex is requesting a warm reset on the Rex
            RexUsb3SetupWarmResetState();
            break;

        case REX_ULP_USB3_EVENT_DEVICE_REMOVED:         // Device has been removed from the Rex
            UlpSendCPUCommRexUsb3Message(ULP_REX_TO_LEX_USB3_DEVICE_DISCONNECTED);  // tell the Lex the device has been removed

            rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_REX_DEVICE_POLL;          // go back to polling for the device
            TIMING_TimerStart(rexUsb3Ulp.deviceConnectedWaitTimer);
            break;

        case REX_ULP_USB3_EVENT_HOST_CONNECT:           // Host is connected on the Lex
        case REX_ULP_USB3_EVENT_DEVICE_CONNECTED:       // Device has just connected to the Rex
        case REX_ULP_USB3_EVENT_STANDBY_REQUEST:        // Rex is requested to go into Standby (U3)
        case REX_ULP_USB3_EVENT_STANDBY_EXIT_REQUEST:   // Exit from Standby on the Rex is requested

        case REX_ULP_USB3_EVENT_LEX_AT_U0:              // Lex USB 3 link is setup
        case REX_ULP_USB3_EVENT_LEX_IN_STANDBY:         // Lex is in standby mode
        case REX_ULP_USB3_EVENT_LEX_EXITED_STANDBY:     // Lex exited standby mode
        case REX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in Hot Reset
            // TODO: these events shouldn't be here - assert,, abort, or????
//            break;

        // these events don't mean anything in this state
        case REX_ULP_USB3_EVENT_ENABLED:     // enable USB3 link setup
        case REX_ULP_USB3_EVENT_HOT_RESET_DONE:         // Hot reset completed on Rex
        case REX_ULP_USB3_EVENT_WARM_RESET_DONE:        // Warm reset completed on Rex
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_REX_USB3_UNEXPECTED_EVENT,
                event,
                rexUsb3Ulp.ulpFsmState);
            break;
    }
}

//#################################################################################################
// Handles events in the REX_ULP_USB3_LEX_SETUP
//
// Parameters:
//      event                   - REX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpRexUsb3StateLexSetup(enum RexUlpUsb3Event event)
{
    // TODO: add an event/timeout so we can disable if we don't hear back from the Lex setup
    // in a reasonable time.
    switch (event)
    {
        case REX_ULP_USB3_EVENT_DISABLED:   // disable/exit USB3 link
            RexUsb3Disabled();              // take down USB3
            break;

        case REX_ULP_USB3_EVENT_LEX_AT_U0:  // Lex USB 3 link is setup
            rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_LINK_ACTIVE;  // go to the link active state

            ulp_StatMonPhyControl(true);        // turn on phy stats count
            break;

        case REX_ULP_USB3_EVENT_REX_AT_SS_INACTIVE:     // Rex USB 3 link is in the inactive state
            RexUsb3GoToInactive();
            break;

        case REX_ULP_USB3_EVENT_WARM_RESET_REQUEST:     // Lex is requesting a warm reset on the Rex
            RexUsb3SetupWarmResetState();
            break;

        case REX_ULP_USB3_EVENT_LEX_FAILED: // Lex failed USB3 link setup
            RexUsb3Failed();        // go into the USB3 failure state
            break;

        case REX_ULP_USB3_EVENT_DEVICE_REMOVED:         // Device has been removed from the Rex
            RexUsb3DeviceRemoved();
            break;

        case REX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in Hot Reset
            RexUsb3SetupHotResetState();                // go to hot reset as well
            break;


        case REX_ULP_USB3_EVENT_HOST_DISCONNECT:        // Host connection has been removed
            RexUsb3Disconnected();
            break;

        case REX_ULP_USB3_EVENT_REX_AT_U0:              // Rex USB 3 link is setup (caused by recovery cycle, in this case)
        case REX_ULP_USB3_EVENT_DEVICE_CONNECTED:       // Device has just connected to the Rex
        case REX_ULP_USB3_EVENT_HOST_CONNECT:           // Host is connected on the Lex
        case REX_ULP_USB3_EVENT_STANDBY_REQUEST:        // Rex is requested to go into Standby (U3)
        case REX_ULP_USB3_EVENT_LEX_IN_STANDBY:         // Lex is in standby mode
        case REX_ULP_USB3_EVENT_LEX_EXITED_STANDBY:     // Lex exited standby mode
        case REX_ULP_USB3_EVENT_STANDBY_EXIT_REQUEST:      // Exit from Standby on the Rex is requested
        // these events don't mean anything in this state
        case REX_ULP_USB3_EVENT_ENABLED:    // enable USB3 link setup
        case REX_ULP_USB3_EVENT_HOT_RESET_DONE:         // Hot reset completed on Rex
        case REX_ULP_USB3_EVENT_WARM_RESET_DONE:        // Warm reset completed on Rex
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_REX_USB3_UNEXPECTED_EVENT,
                event,
                rexUsb3Ulp.ulpFsmState);
            break;
    }
}

//#################################################################################################
// Handles events in the REX_ULP_USB3_LINK_ACTIVE
//
// Parameters:
//      event                   - REX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpRexUsb3StateActive(enum RexUlpUsb3Event event)
{
    switch (event)
    {
        case REX_ULP_USB3_EVENT_DISABLED:   // disable/exit USB3 link
            RexUsb3Disabled();              // take down USB3
            break;

        case REX_ULP_USB3_EVENT_LEX_FAILED: // Lex failed USB3 link setup - warm reset failure?
            RexUsb3Failed();        // go into the USB3 failure state
            break;

        case REX_ULP_USB3_EVENT_REX_AT_SS_INACTIVE:     // Rex USB 3 link is in the inactive state
            RexUsb3GoToInactive();
            break;

        case REX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in Hot Reset
            RexUsb3SetupHotResetState();                // go to hot reset as well
            break;

        case REX_ULP_USB3_EVENT_WARM_RESET_REQUEST:     // Lex is requesting a warm reset on the Rex
            RexUsb3SetupWarmResetState();
            break;

        case REX_ULP_USB3_EVENT_LEX_IN_STANDBY: // Lex is in standby mode
            rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_IN_STANDBY_MODE;   // go to the standby state
            ulp_StatMonPhyControl(false);        // turn off phy stats count
            _ULP_halControlStandbyMode(true);   // put the Rex into standby mode
            break;

        case REX_ULP_USB3_EVENT_DEVICE_REMOVED:         // Device has been removed from the Rex
            RexUsb3DeviceRemoved();
            break;

        case REX_ULP_USB3_EVENT_REX_AT_U0:  // Rex USB 3 link is setup
            // ignore this - it means the Rex went through recovery
            break;

        case REX_ULP_USB3_EVENT_HOST_DISCONNECT:        // Host connection has been removed
            RexUsb3Disconnected();
            break;

        case REX_ULP_USB3_EVENT_DEVICE_CONNECTED:       // Device has just connected to the Rex
        case REX_ULP_USB3_EVENT_HOST_CONNECT:           // Host is connected on the Lex
        case REX_ULP_USB3_EVENT_STANDBY_REQUEST:        // Rex is requested to go into Standby (U3)
        case REX_ULP_USB3_EVENT_STANDBY_EXIT_REQUEST:      // Exit from Standby on the Rex is requested
        // these events don't mean anything in this state
        case REX_ULP_USB3_EVENT_ENABLED:    // enable USB3 link setup
        case REX_ULP_USB3_EVENT_LEX_AT_U0:  // Lex USB 3 link is setup (should only occur at the Lex setup state
        case REX_ULP_USB3_EVENT_HOT_RESET_DONE:         // Hot reset completed on Rex
        case REX_ULP_USB3_EVENT_WARM_RESET_DONE:        // Warm reset completed on Rex
        case REX_ULP_USB3_EVENT_LEX_EXITED_STANDBY:     // Lex exited standby mode
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_REX_USB3_UNEXPECTED_EVENT,
                event,
                rexUsb3Ulp.ulpFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in the REX_ULP_USB3_HOT_RESET_WAIT state
//
// Parameters:
//      event                   - REX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpRexUsb3StateHotResetWait(enum RexUlpUsb3Event event)
{
    switch (event)
    {
        case REX_ULP_USB3_EVENT_DISABLED:   // disable/exit USB3 link
            RexUsb3Disabled();              // take down USB3
            break;

        case REX_ULP_USB3_EVENT_LEX_FAILED: // Lex failed USB3 link setup - warm reset failure?
            RexUsb3Failed();        // go into the USB3 failure state
            break;

        case REX_ULP_USB3_EVENT_HOT_RESET_DONE:         // Hot reset completed on Rex
           RexUsb3StartSetupPolling();                 // start polling for a valid link to the device
            break;

        case REX_ULP_USB3_EVENT_DEVICE_REMOVED:         // Device has been removed from the Rex
            RexUsb3DeviceRemoved();
            break;

        case REX_ULP_USB3_EVENT_HOST_DISCONNECT:        // Host connection has been removed
            RexUsb3Disconnected();
            break;

        case REX_ULP_USB3_EVENT_WARM_RESET_REQUEST:     // Lex is requesting a warm reset on the Rex
            RexUsb3SetupWarmResetState();
            break;

        // these events don't mean anything in this state
        case REX_ULP_USB3_EVENT_HOST_CONNECT:           // Host is connected on the Lex
        case REX_ULP_USB3_EVENT_DEVICE_CONNECTED:       // Device has just connected to the Rex
        case REX_ULP_USB3_EVENT_STANDBY_REQUEST:        // Rex is requested to go into Standby (U3)
        case REX_ULP_USB3_EVENT_STANDBY_EXIT_REQUEST:      // Exit from Standby on the Rex is requested
        case REX_ULP_USB3_EVENT_REX_AT_SS_INACTIVE:     // Rex USB 3 link is in the inactive state
        case REX_ULP_USB3_EVENT_REX_AT_U0:  // Rex USB 3 link is setup
        case REX_ULP_USB3_EVENT_LEX_AT_U0:  // Lex USB 3 link is setup
        case REX_ULP_USB3_EVENT_LEX_IN_STANDBY: // Lex is in standby mode
        case REX_ULP_USB3_EVENT_LEX_EXITED_STANDBY:     // Lex exited standby mode
        case REX_ULP_USB3_EVENT_ENABLED:    // enable USB3 link setup
        case REX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in Hot Reset
        case REX_ULP_USB3_EVENT_WARM_RESET_DONE:        // Warm reset completed on Rex
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_REX_USB3_UNEXPECTED_EVENT,
                event,
                rexUsb3Ulp.ulpFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in the REX_ULP_USB3_WARM_RESET_WAIT state
//
// Parameters:
//      event                   - REX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpRexUsb3StateWarmResetWait(enum RexUlpUsb3Event event)
{
    switch (event)
    {
        case REX_ULP_USB3_EVENT_DISABLED:   // disable/exit USB3 link
            RexUsb3Disabled();              // take down USB3
            break;

        case REX_ULP_USB3_EVENT_LEX_FAILED: // Lex failed USB3 link setup - warm reset failure?
            RexUsb3Failed();        // go into the USB3 failure state
            break;

        case REX_ULP_USB3_EVENT_WARM_RESET_DONE:        // Warm reset completed on Rex
            RexUsb3StartSetupPolling();                 // start polling for a valid link to the device
            break;

        case REX_ULP_USB3_EVENT_DEVICE_REMOVED:         // Device has been removed from the Rex
            RexUsb3DeviceRemoved();
            break;

        case REX_ULP_USB3_EVENT_HOST_DISCONNECT:        // Host connection has been removed
            RexUsb3Disconnected();
            break;

        case REX_ULP_USB3_EVENT_HOST_CONNECT:           // Host is connected on the Lex
        case REX_ULP_USB3_EVENT_DEVICE_CONNECTED:       // Device has just connected to the Rex
        case REX_ULP_USB3_EVENT_STANDBY_REQUEST:        // Rex is requested to go into Standby (U3)
        case REX_ULP_USB3_EVENT_STANDBY_EXIT_REQUEST:      // Exit from Standby on the Rex is requested
            // these events don't mean anything in this state
        case REX_ULP_USB3_EVENT_REX_AT_SS_INACTIVE:     // Rex USB 3 link is in the inactive state
        case REX_ULP_USB3_EVENT_REX_AT_U0:  // Rex USB 3 link is setup
        case REX_ULP_USB3_EVENT_LEX_AT_U0:  // Lex USB 3 link is setup
        case REX_ULP_USB3_EVENT_LEX_IN_STANDBY: // Lex is in standby mode
        case REX_ULP_USB3_EVENT_LEX_EXITED_STANDBY:     // Lex exited standby mode
        case REX_ULP_USB3_EVENT_ENABLED:    // enable USB3 link setup
        case REX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in Hot Reset
        case REX_ULP_USB3_EVENT_HOT_RESET_DONE:         // Hot reset completed on Rex
        case REX_ULP_USB3_EVENT_WARM_RESET_REQUEST:     // Lex is requesting a warm reset on the Rex
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_REX_USB3_UNEXPECTED_EVENT,
                event,
                rexUsb3Ulp.ulpFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in the REX_ULP_USB3_IN_STANDBY_MODE state
//
// Parameters:
//      event                   - REX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpRexUsb3StateInStandby(enum RexUlpUsb3Event event)
{
    switch (event)
    {
        case REX_ULP_USB3_EVENT_DISABLED:   // disable USB3
            RexUsb3Disabled();              // take down USB3
            break;

        case REX_ULP_USB3_EVENT_LEX_FAILED: // Lex failed USB3 link setup - warm reset failure?
            RexUsb3Failed();        // go into the USB3 failure state
            break;

        case REX_ULP_USB3_EVENT_DEVICE_REMOVED:         // Device has been removed from the Rex
            RexUsb3DeviceRemoved();
            break;

        case REX_ULP_USB3_EVENT_STANDBY_EXIT_REQUEST:   // Exit from Standby on the Rex is requested
            RexUsb3StartSetupPolling();                 // start polling for a valid link to the device
            break;

        case REX_ULP_USB3_EVENT_LEX_EXITED_STANDBY:     // Lex exited standby mode
            _ULP_halControlStandbyMode(false);          // exit standby on the Rex, too
            RexUsb3StartSetupPolling();                 // start polling for a valid link to the device
            break;

        case REX_ULP_USB3_EVENT_REX_AT_SS_INACTIVE:     // Rex USB 3 link is in the inactive state
            RexUsb3GoToInactive();
            break;

        case REX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in Hot Reset
            RexUsb3SetupHotResetState();                // go to hot reset as well
            break;

        case REX_ULP_USB3_EVENT_WARM_RESET_REQUEST:     // Lex is requesting a warm reset on the Rex
            RexUsb3SetupWarmResetState();
            break;

        case REX_ULP_USB3_EVENT_HOST_DISCONNECT:        // Host connection has been removed
            RexUsb3Disconnected();
            break;

        case REX_ULP_USB3_EVENT_HOST_CONNECT:           // Host is connected on the Lex
        case REX_ULP_USB3_EVENT_DEVICE_CONNECTED:       // Device has just connected to the Rex
        case REX_ULP_USB3_EVENT_STANDBY_REQUEST:        // Rex is requested to go into Standby (U3)
       // these events don't mean anything in this state
        case REX_ULP_USB3_EVENT_REX_AT_U0:  // Rex USB 3 is setup
        case REX_ULP_USB3_EVENT_LEX_AT_U0:  // Lex USB 3 link is setup
        case REX_ULP_USB3_EVENT_LEX_IN_STANDBY: // Lex is in standby mode
        case REX_ULP_USB3_EVENT_ENABLED:    // enable USB3
        case REX_ULP_USB3_EVENT_HOT_RESET_DONE:         // Hot reset completed on Rex
        case REX_ULP_USB3_EVENT_WARM_RESET_DONE:        // Warm reset completed on Rex
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_REX_USB3_UNEXPECTED_EVENT,
                event,
                rexUsb3Ulp.ulpFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in the REX_ULP_USB3_LINK_FAILED
//
// Parameters:
//      event                   - REX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpRexUsb3StateFailed(enum RexUlpUsb3Event event)
{
    switch (event)
    {
        case REX_ULP_USB3_EVENT_DISABLED:   // disable USB3
            RexUsb3Disabled();              // take down USB3
            break;

        case REX_ULP_USB3_EVENT_DEVICE_REMOVED:         // Device has been removed from the Rex
            RexUsb3DeviceRemoved();     // TODO: look at a possible reset, since we are in the failure case?
            break;

        case REX_ULP_USB3_EVENT_HOST_DISCONNECT:        // Host connection has been removed
            RexUsb3Disconnected();
            break;

        // these events don't mean anything in this state
        case REX_ULP_USB3_EVENT_LEX_FAILED: // Lex failed USB3 link setup
            break;

        // these events shouldn't occur in this state
        case REX_ULP_USB3_EVENT_HOST_CONNECT:           // Host is connected on the Lex
        case REX_ULP_USB3_EVENT_DEVICE_CONNECTED:       // Device has just connected to the Rex
        case REX_ULP_USB3_EVENT_STANDBY_REQUEST:        // Rex is requested to go into Standby (U3)
        case REX_ULP_USB3_EVENT_STANDBY_EXIT_REQUEST:      // Exit from Standby on the Rex is requested

        // these events don't mean anything in this state
        case REX_ULP_USB3_EVENT_REX_AT_SS_INACTIVE:     // Rex USB 3 link is in the inactive state
        case REX_ULP_USB3_EVENT_REX_AT_U0:  // Rex USB 3 is setup
        case REX_ULP_USB3_EVENT_LEX_AT_U0:  // Lex USB 3 link is setup
        case REX_ULP_USB3_EVENT_LEX_IN_STANDBY: // Lex is in standby mode
        case REX_ULP_USB3_EVENT_LEX_EXITED_STANDBY:     // Lex exited standby mode
        case REX_ULP_USB3_EVENT_ENABLED:    // enable USB3
        case REX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in Hot Reset
        case REX_ULP_USB3_EVENT_HOT_RESET_DONE:         // Hot reset completed on Rex
        case REX_ULP_USB3_EVENT_WARM_RESET_REQUEST:     // Lex is requesting a warm reset on the Rex
        case REX_ULP_USB3_EVENT_WARM_RESET_DONE:        // Warm reset completed on Rex
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_REX_USB3_UNEXPECTED_EVENT,
                event,
                rexUsb3Ulp.ulpFsmState);
            break;
    }
}


//#################################################################################################
// Set up USB3 on the Rex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexSetupUSB3(void)
{
    UlpHalEnableUlpInterrupts(REX_ULP_USB3_IRQ0, REX_ULP_USB3_IRQ1);

    UlpHalControlRxTerminations(true);    // make sure RX terminations are on
    UlpVbusControl(true);                   // make sure vBus is on (after rx terminations is turned on!)
    UlpHalSetRxDetect();                  // go to Rx Detect state
    _ULP_halSetHotResetWait(false);         // make sure we don't wait in hot reset on the Rex

    ulp_StatMonCoreControl(true);  // turn on the ULP stats for the Rex
}

//#################################################################################################
// Tear down USB3 on the Rex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUsb3Disabled(void)
{
    RexUsb3TurnOffUsb3();       // turn off the ULP core state machine, and related stats

    rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_STATE_DISABLED;           // go to the disabled state
    UlpSendCPUCommRexUsb3Message(ULP_REX_TO_LEX_USB3_DISABLED);     // tell the Lex we are disabled
    rexUsb3Ulp.lexOnlyResetActive = false;
}


//#################################################################################################
// USB3 is disconnected on the Rex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUsb3Disconnected(void)
{
    RexUsb3TurnOffUsb3();       // turn off the ULP core state machine, and related stats

    // make sure we go through the ready delay state
    rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_STATE_DELAY_READY;    // minimum down time before we say USB3 is ready
    TIMING_TimerStart(rexUsb3Ulp.rexReadyDelayTimer);           // start the delay timer
}


//#################################################################################################
// Lex USB3 failed - put the Rex into failure state, but don't tell the Lex, because he is
// already in the failure state, and by the time the message gets back to the Lex, he may be in
// a different state, ad the message may push him back to failure
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUsb3Failed(void)
{
    // make sure USB3 is off, so the devices can transition to USB2 if possible
    RexUsb3TurnOffUsb3();

    rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_LINK_FAILED;          // go to the link failure case
}


//#################################################################################################
// USB3 device removed from Rex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUsb3DeviceRemoved(void)
{
   RexUsb3TurnOffUsb3();       // turn off the ULP core state machine, and related stats
   RexSetupUSB3();                             // go setup the ULP HW

   UlpSendCPUCommRexUsb3Message(ULP_REX_TO_LEX_USB3_DEVICE_DISCONNECTED);  // tell the Lex the device has been removed

   rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_REX_DEVICE_POLL;          // go back to polling for the device
   TIMING_TimerStart(rexUsb3Ulp.deviceConnectedWaitTimer);
}


//#################################################################################################
// Rex is in the inactive state - go to that state to reflect that
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUsb3GoToInactive(void)
{
    UlpSendCPUCommRexUsb3Message( ULP_REX_TO_LEX_USB3_REX_AT_INACTIVE); // tell the Lex we are in the inactive state and need a warm reset
}


//#################################################################################################
// Called when we've detected a serious error
// We go into the failure state, and hopefully can recover
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUsb3SeriousError(void)
{
    RexUsb3Failed();
    UlpSendCPUCommRexUsb3Message(ULP_REX_TO_LEX_USB3_FAILURE);  // tell the Lex we failed
}



//#################################################################################################
// Turn off the USB3 state machine on the Rex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUsb3TurnOffUsb3(void)
{
    UlpHalDisableUlpInterrupts( REX_ULP_USB3_IRQ0, REX_ULP_USB3_IRQ1);

    ULP_controlTurnOffUSB3();   // disable the ULP core state machine

    TIMING_TimerStop(rexUsb3Ulp.rexLtssmPollTimer);         // tearing down USB3, make sure timer is off
    TIMING_TimerStop(rexUsb3Ulp.deviceConnectedWaitTimer);  // tearing down USB3, make sure timer is off
    TIMING_TimerStop(rexUsb3Ulp.rexReadyDelayTimer);        // tearing down USB3, make sure timer is off

}


//#################################################################################################
// Ready delay time has elapsed, say the Rex USB3 is ready
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUsb3ReadyDelayTimeout(void)
{
    ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_REX_USB3_DELAY_TIMEOUT, rexUsb3Ulp.ulpFsmState);

    if ( rexUsb3Ulp.ulpFsmState == REX_ULP_USB3_STATE_DELAY_READY )
    {
        // Tell the Lex we are ready, and then wait for the connect message
        UlpSendCPUCommRexUsb3Message(ULP_REX_TO_LEX_USB3_READY);  // tell Lex USB3 is ready on the Rex
        rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_STATE_READY;
   }
}


//#################################################################################################
// Timeout while waiting for a device connect
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUsb3DeviceConnectTimeout(void)
{
    if ( UlpHalRxTerminationsPresent() )
    {
        ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_DEVICE_CONNECTED);
    }
    else
    {
        UlpSendCPUCommRexUsb3Message(ULP_REX_TO_LEX_USB3_DEVICE_DISCONNECTED);  // tell the Lex the device has been removed
    }
}


//#################################################################################################
// USB3 device removed from Rex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUsb3StartSetupPolling(void)
{
    TIMING_TimerStart(rexUsb3Ulp.rexLtssmPollTimer);

    // set the start time here in case we don't get a chance to poll before we get a valid LTSSM
    rexUsb3Ulp.ltssmStartTime = LEON_TimerRead();

    rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_REX_SETUP;    // go to the setup state, wait to see if we link ok
}


//#################################################################################################
// Goes to the hot reset state
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUsb3SetupHotResetState(void)
{
    rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_HOT_RESET_WAIT;   // wait for the hot reset to finish
    _ULP_halSetHotReset();                                  // generate a hot reset, go wait for it to be done
    UlpStatusChange(ULP_STATUS_INBAND_HOT_RESET);           // tell UPP we are doing a hot reset
}

//#################################################################################################
// Goes to the warm reset state
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUsb3SetupWarmResetState(void)
{
    rexUsb3Ulp.ulpFsmState = REX_ULP_USB3_WARM_RESET_WAIT;  // wait for the warm reset to finish
    _ULP_halSetWarmReset();                                 // generate a warm reset here, wait for it to be done
    ulp_StatMonPhyControl(false);                           // turn off phy stats count

    UlpStatusChange(ULP_STATUS_INBAND_WARM_RESET);          // a warm reset is in progress
}

//#################################################################################################
// Polls during Rex Setup, to see when the device is connected, and to make sure we don't wait
// forever for the LTSSM to be valid
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUsb3LtssmPoll(void)
{
    if ( rexUsb3Ulp.ulpFsmState == REX_ULP_USB3_REX_SETUP )
    {
        if (_ULP_IsLtssmValid())
        {
            ULP_RexUsb3StateMachineSendEvent(REX_ULP_USB3_EVENT_REX_AT_U0);     // USB3 setup complete on Rex
        }
        else if (LEON_TimerCalcUsecDiff(rexUsb3Ulp.ltssmStartTime, LEON_TimerRead()) > REX_LTSSM_TOTAL_TIMEOUT_IN_US)
        {
            // Time out and ltssm register value is still invalid - Rex USB3 setup failed!
            ilog_ULP_COMPONENT_1(ILOG_MAJOR_ERROR, ULP_LTSSM_INVALID, UlpHalGetLTSSM());

//            UART_printf("control value 0x%x, config value 0x%x\n", _ULP_GetControlValue(), _ULP_GetConfigurationValue() );
            RexUsb3SeriousError();
        }
        else
        {
            // keep on polling
            TIMING_TimerStart(rexUsb3Ulp.rexLtssmPollTimer);    // keep on polling for U0
        }
    }
}


//#################################################################################################
// Returns the current state of USB3.
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
static uint32_t RexUsb3GetUsb3Status(void)
{
    enum EventUsb3Status status = USB3_IN_RESET;

    // use a switch case so if new states get added they will have to update this
    switch (rexUsb3Ulp.ulpFsmState)
    {
        case REX_ULP_USB3_STATE_DISABLED:   // USB3 state machine disabled
        case REX_ULP_USB3_STATE_DELAY_READY:    // minimum down time before we say USB3 is ready
        case REX_ULP_USB3_STATE_READY:      // USB3 port state machine ready
        case REX_ULP_USB3_REX_DEVICE_POLL:  // Waiting for indication on the Rex a device is connected
        case REX_ULP_USB3_REX_SETUP:        // Waiting for the Rex to indicate it has setup USB3
        case REX_ULP_USB3_LEX_SETUP:        // Waiting for the Lex to indicate it has setup USB3
        case REX_ULP_USB3_HOT_RESET_WAIT:   // waiting for hot reset to finish on the Rex
        case REX_ULP_USB3_WARM_RESET_WAIT:  // waiting for warm reset to finish
        default:
            status = USB3_IN_RESET;         // USB3 is not active
            break;

        case REX_ULP_USB3_LINK_FAILED:      // USB3 failed to setup on either the Rex or Lex
            status = USB3_ERROR;
            break;

        case REX_ULP_USB3_LINK_ACTIVE:      // USB3 active on Lex and Rex
            status = USB3_U0;
            break;

        case REX_ULP_USB3_IN_STANDBY_MODE:  // in standby mode
            status = USB3_U3;
            break;
    }

    return ((uint32_t)status);
}


//#################################################################################################
// Handler for USB3 MCA channel status changes
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
static void UlpRexUsb3ChannelStatus(enum MCA_ChannelStatus channelStatus)
{
    ilog_ULP_COMPONENT_2(ILOG_MINOR_EVENT, ULP_REX_USB3_CHANNEL_STATUS, channelStatus, rexUsb3Ulp.lexOnlyResetActive);

    if (rexUsb3Ulp.lexOnlyResetActive)
    {
        switch (channelStatus)
        {
            case MCA_CHANNEL_STATUS_LINK_ACTIVE:        // channel is linked between Lex and Rex
                MCA_ChannelTxRxSetup(MCA_CHANNEL_NUMBER_USB3);
                break;

            case MCA_CHANNEL_STATUS_CHANNEL_READY:     // channel is linked, and Rx, Tx is setup.  Ready for operation
                break;

            case MCA_CHANNEL_STATUS_CHANNEL_DISABLED:   // channel is disabled
            case MCA_CHANNEL_STATUS_LINK_DOWN:          // channel is down between Lex and Rex, needs to be re-initialized
            default:
                MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_USB3);
                break;
        }
    }
    else
    {
        switch (channelStatus)
        {
            case MCA_CHANNEL_STATUS_LINK_ACTIVE:        // channel is linked between Lex and Rex
                UlpSendCPUCommRexUsb3ResetMessage(ULP_REX_TO_LEX_USB3_RESET_ACTIVATE_PHY_2);
                break;

            case MCA_CHANNEL_STATUS_CHANNEL_READY:     // channel is linked, and Rx, Tx is setup.  Ready for operation
                ULP_RexUsb3Control(ULP_USB_CONTROL_MCA_CHANNEL_RDY);    // signal the channel is available
                ULP_RexUsb3Control(ULP_USB_CONTROL_RESET_DONE);         // and reset is done
                ULP_controlRexInitUSB3();
                break;

            case MCA_CHANNEL_STATUS_CHANNEL_DISABLED:   // channel is disabled
            case MCA_CHANNEL_STATUS_LINK_DOWN:          // channel is down between Lex and Rex, needs to be re-initialized
            default:
                ULP_RexUsb3Control(ULP_USB_CONTROL_MCA_CHANNEL_DOWN); // USB3 MCA channel is down (Rex side)
                break;
        }
    }
}

//#################################################################################################
// USB Overcurrent detect GPIO interrupt handler
//      Overcurrent after link up: 1 Signal drop happen    _____   _____
//                                                              |_|
//                                                               8ms duration low
//      Overcurrent w/o link up: Signal drop happen every 1sec
//                                                         _____   _____   _____   _____
//                                                              |_|     |_|     |_|
//                                                          8ms duration low every 1sec
//
//      This interrupt will be enabled only when Link is up and Lex's host is connected
//      If this interrupt happen by a usb port, the usb port is disabled until link cycle or Lex's host cycle
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_RexUsb3OverCurrentIntHandler(void)
{
    CALLBACK_Run(ULP_RexUsb3OverCurrentIntCallback, NULL, NULL);
}

//#################################################################################################
// Callback for USB over current ISR
//
// Parameters: 
// Return:
// Assumptions:
//#################################################################################################
static void ULP_RexUsb3OverCurrentIntCallback(void *param1, void *param2)
{
    if (!GpioRead(GPIO_CONN_USB_OVER_CURRENT))
    {
        ILOG_istatus(ISTATUS_ULP_OVERCURRENT_DETECTED, 0);
    }
    else
    {
        ILOG_istatus(ISTATUS_ULP_OVERCURRENT_NORMAL, 0);
    }
}

