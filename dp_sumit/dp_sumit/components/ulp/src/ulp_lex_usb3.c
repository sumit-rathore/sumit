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
// This file contains the implementation of LEX ULP (USB Link Protocol) bringup.
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
#include <ulp_core_regs.h>
#include <event.h>

#include <uart.h>
#include <configuration.h>

// Constants and Macros ###########################################################################
// Some of initial values (which are stored in flash) for LexUlpUsb2Context are defined in configuration.h
#define LEX_LTSSM_TIMER_PERIOD_IN_MS    (100)

// if we are in the inactive state, we should get a warm reset very fast.  However, we have seen that
// on some windows systems, when going through a restart the Lex goes to inactive, and because the PC is
// going through a restart it can stay in the inactive state for seconds
//#define LEX_LTSSM_INACTIVE_TIMEOUT_IN_US    (10*1000*1000) // should take max 10s when in the inactive state
#define LEX_LTSSM_INACTIVE_TIMEOUT_IN_US    (1*1000*1000) // should take max 1s when in the inactive state

// Data Types #####################################################################################
enum LexUlpUsb3State
{
    LEX_ULP_USB3_STATE_DISABLED,    // USB3 port state machine disabled

    LEX_ULP_USB3_STATE_READY,       // USB3 port state machine is ready to connect

    LEX_ULP_USB3_RX_DETECT,         // See if we can detect Rx terminations on the far side
    LEX_ULP_USB3_REX_SETUP,         // Waiting for Rex to indicate it has setup USB3
    LEX_ULP_USB3_LEX_SETUP,         // Waiting for Lex to indicate it has setup USB3
    LEX_ULP_USB3_ACTIVE,            // USB3 active on Lex and Rex

    LEX_ULP_USB3_FAILED,            // USB3 failed on either the Rex or Lex

    LEX_ULP_USB3_REX_HOT_RESET_WAIT,    // waiting for Hot Reset to finish on the Rex
    LEX_ULP_USB3_REX_WARM_RESET_WAIT,   // waiting for warm reset to finish on the Rex

    LEX_ULP_USB3_STANDBY,           // Lex USB3 is in standby (U3)
    LEX_ULP_USB3_STANDBY_HOST_EXIT, // Lex USB3 is exiting standby (Host initiated exit)

    LEX_ULP_USB3_LEX_ONLY_RESET,    // Lex USB3 is in a Lex only reset state
};

// TODO add a master timeout event, so if we get stuck we will go back to the idle state
enum LexUlpUsb3Event
{
    LEX_ULP_USB3_EVENT_DISABLED,                // disable USB3
    LEX_ULP_USB3_EVENT_ENABLED,                 // enable USB3

    LEX_ULP_USB3_EVENT_HOST_CONNECT,            // connect to a host
    LEX_ULP_USB3_EVENT_HOST_DISCONNECT,         // disconnect from the host

    LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED,      // Device disconnected from Rex

    LEX_ULP_USB3_EVENT_REX_AT_U0,               // Rex USB 3 is setup
    LEX_ULP_USB3_EVENT_REX_AT_INACTIVE,         // Rex is in the inactive state
    LEX_ULP_USB3_EVENT_REX_FAILED,              // Rex failed USB3 setup

    LEX_ULP_USB3_EVENT_LEX_AT_U0,               // Lex USB 3 is setup (in the U0 state)
    LEX_ULP_USB3_EVENT_LEX_FAILED,              // Lex failed USB3 setup

    LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE,      // Rx termination change detected!
    LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET,        // Lex is in the Hot Reset state
    LEX_ULP_USB3_EVENT_WARM_RESET,              // Lex went through a warm reset
    LEX_ULP_USB3_EVENT_REX_WARM_RESET_DONE,     // Warm reset done on the Rex

    LEX_ULP_USB3_EVENT_IN_STANDBY,              // Lex is in standby mode
    LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT,        // Lex is exiting standby
};


// Static Function Declarations ###################################################################
static void ULP_LexUsb3StateMachineSendEvent(enum LexUlpUsb3Event event)    __attribute__((section(".lexatext")));
static void ULP_LexUsb3StateCallback(void *param1, void *param2)            __attribute__((section(".lexatext")));

static void ULP_LexUsb3StateMachine(enum LexUlpUsb3Event lexHostEvent)      __attribute__((section(".lexatext")));
static void UlpLexUsb3StateDisabled(enum LexUlpUsb3Event event)             __attribute__((section(".lexatext")));
static void UlpLexUsb3StateReady(enum LexUlpUsb3Event event)                __attribute__((section(".lexatext")));
static void UlpLexUsb3StateRxDetect(enum LexUlpUsb3Event event)             __attribute__((section(".lexatext")));
static void UlpLexUsb3StateRexSetup(enum LexUlpUsb3Event event)             __attribute__((section(".lexatext")));
static void UlpLexUsb3StateLexSetup(enum LexUlpUsb3Event event)             __attribute__((section(".lexatext")));
static void UlpLexUsb3StateActive(enum LexUlpUsb3Event event)               __attribute__((section(".lexatext")));
static void UlpLexUsb3StateWaitRexHotReset(enum LexUlpUsb3Event event)      __attribute__((section(".lexftext")));  // this state needs to be fast
static void UlpLexUsb3StateWaitRexWarmReset(enum LexUlpUsb3Event event)     __attribute__((section(".lexatext")));
static void UlpLexUsb3StateStandby(enum LexUlpUsb3Event event)              __attribute__((section(".lexatext")));
static void UlpLexUsb3StateStandbyHostExited(enum LexUlpUsb3Event event)    __attribute__((section(".lexatext")));
static void UlpLexUsb3StateWaitLexOnlyResetDone(enum LexUlpUsb3Event event) __attribute__((section(".lexatext")));
static void UlpLexUsb3StateFailed(enum LexUlpUsb3Event event)               __attribute__((section(".lexatext")));

static void LexSetupUSB3(void)                                              __attribute__((section(".lexatext")));
static void LexSetupWarmResetWait(void)                                     __attribute__((section(".lexatext")));
static void LexSetupHotResetWait(void)                                      __attribute__((section(".lexftext")));  // Hot reset has tight timing constraints
static void LexUsb3SetDisabledState(void)                                   __attribute__((section(".lexatext")));
static void LexUSB3SetFailedState(void)                                     __attribute__((section(".lexatext")));
static void LexUSB3HandleDisconnectEvent(void)                              __attribute__((section(".lexatext")));
static void LexUsb3TurnOffUsb3(void)                                        __attribute__((section(".lexatext")));
static void LexUsbNoRexDevice(void)                                         __attribute__((section(".lexatext")));
static void LexUsbRxTerminationsRemoved(void)                               __attribute__((section(".lexatext")));
static void LexUsb3FailureRecovery(void)                                    __attribute__((section(".lexatext")));

static void LexCheckLtssm(void)                                             __attribute__((section(".lexatext")));

static uint32_t LexUsb3GetUsb3Status(void)                                  __attribute__((section(".lexatext")));
static void LexUsb3SnoopTimeout(void)                                       __attribute__((section(".lexatext")));
static void LexUsb3WarmResetCountClear(void)                                __attribute__((section(".lexatext")));
static void LexUsb3InactiveCountClear(void)                                 __attribute__((section(".lexatext")));
static void LexUsb3InactiveTimeout(void)                                    __attribute__((section(".lexatext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct LexUlpUsb2Context
{
    // Lex host port state
    // Lex ULP FSM state.
    enum LexUlpUsb3State ulpFsmState;
    uint8_t controlFlags;               // flags controlling whether this USB controller is enabled/disabled
    uint8_t usbSuspendIstatusFlag;

    // A timer used to track the status of LTSSM register.
    TIMING_TimerHandlerT ltssmTimer;
    LEON_TimerValueT ltssmStartTime;    // the time we started checking the ltssm states

    // if we can't decide whether we are USB2 or USB3, default to USB2 after this timeout
    // (but keep on trying to detect the far end terminations)
    TIMING_TimerHandlerT snoopModeTimeout;

    uint8_t failCount;          // number of times we failed to setup USB3 in the current session; cleared when we get to U0
    uint8_t warmResetCount;     // increment for every warm reset cycle we start

    // process the warm reset count every time this timer fires
    TIMING_TimerHandlerT warmResetClearTimeout;

    // if we stay in inactive longer then a certain amount, reset
    TIMING_TimerHandlerT inactiveStuckTimeout;

    TIMING_TimerHandlerT inactiveClearCountTimeout;
    uint8_t inactiveCount;     // increment for every time we go to the inactive state
    bool rexReqInactive;        // true if the Rex requested we go to inactive
    bool standbyExit;           // true if we just exited from standby
    bool LexOnlyResetTriggered; // true if a Lex only reset occurred

    uint32_t lexLtssmTotalTimeoutUs;
    uint8_t lexUsb3MaxFailCount;
    uint8_t lexUsb3WarmResetMaxCount;   // max number of warm reset

} lexUsb3Ulp;

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Initialize the LEX ULP module and the ULP finite state machine.
//
// Parameters:
// Return:
// Assumptions:
//      * USB3 is enabled/disabled only via this function; it cannot be enabled/disabled at run time
//
//#################################################################################################
void ULP_LexUsb3Init(bool enabled)
{
    ConfigUlpUsb3ResetParams *ulpUsb3ResetParams =  &(Config_GetBuffer()->ulpUsb3ResetParams);

    // TODO add an assert if Config_ArbitrateGetVar() returns false
    Config_ArbitrateGetVar(CONFIG_VAR_ULP_USB3_RESET_PARAMS, ulpUsb3ResetParams);

    lexUsb3Ulp.lexUsb3WarmResetMaxCount = ulpUsb3ResetParams->lexUsb3WarmResetMaxCount;
    lexUsb3Ulp.lexLtssmTotalTimeoutUs   = ulpUsb3ResetParams->lexLtssmTotalTimeoutUs;
    lexUsb3Ulp.lexUsb3MaxFailCount      = ulpUsb3ResetParams->lexUsb3MaxFailCount;

    lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_STATE_DISABLED;

    lexUsb3Ulp.ltssmTimer = TIMING_TimerRegisterHandler(
        &LexCheckLtssm,
        true,
        LEX_LTSSM_TIMER_PERIOD_IN_MS);

    // setup timer to poll terminations if we don't get an interrupt
    lexUsb3Ulp.snoopModeTimeout = TIMING_TimerRegisterHandler(
        LexUsb3SnoopTimeout,
        false,
        ulpUsb3ResetParams->lexUsb3SnoopTimeoutMs);

    // setup timer to clear the warm reset counts
    lexUsb3Ulp.warmResetClearTimeout = TIMING_TimerRegisterHandler(
        LexUsb3WarmResetCountClear,
        false,
        ulpUsb3ResetParams->lexUsb3WarmResetTimerPeriodMs);

    // setup timer to check for stuck inactive state (no warm reset from host)
    lexUsb3Ulp.inactiveStuckTimeout = TIMING_TimerRegisterHandler(
        LexUsb3InactiveTimeout,
        false,
        ulpUsb3ResetParams->lexUsb3InactiveStuckTimeoutMs);

    // setup timer to clear the inactive count
    lexUsb3Ulp.inactiveClearCountTimeout = TIMING_TimerRegisterHandler(
        LexUsb3InactiveCountClear,
        false,
        ulpUsb3ResetParams->lexUsb3WarmResetTimerPeriodMs);

    ULP_UlpLexResetUsb3Init();          // initialize the USB3 reset machine

    EVENT_Register(ET_USB3_STATUS_CHANGE, LexUsb3GetUsb3Status);

    // Strap setting outputs after OutputEnable Low
    // OutputEnable works as in/out mux inside FPGA.
    if(bb_top_getCoreBoardRev() >= BOM_23_00200_A03)    // A03 Board use Clock input
    {                                                   // A01,02 used XTAL input (default strap)
        bb_top_applyUsb3PhyCtrlXtalDisable(true);
        bb_top_applyUsb3PhyCtrlSscDisable(true);
    }

    // only turn on the ULP if we are enabled
    if (enabled)
    {
        ULP_LexUsb3Control(ULP_USB_CONTROL_ENABLE); // USB3 is enabled!
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
bool ULP_LexUsb3Enabled(void)
{
    if (lexUsb3Ulp.controlFlags & ULP_USB_MODULE_LOCAL_ENABLED)
    {
        return (true);  // USB3 is enabled
    }

    return (false);
}


//#################################################################################################
// Returns true if USB3 operation is enabled
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool ULP_LexUsb3SystemEnabled(void)
{
    // only allow USB3 operation if both sides allow it
    if ((lexUsb3Ulp.controlFlags & ULP_USB_MODULE_SYSTEM_ENABLED) == ULP_USB_MODULE_SYSTEM_ENABLED)
    {
        return (true);  // USB3 is enabled
    }

    return (false);
}


//#################################################################################################
// Process the interrupts from the IRQ0 register
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_LexUsb3Irq0(uint32_t lexUlpInts)
{
    if (lexUlpInts & ULP_CORE_IRQ0_RAW_RX_TERMINATION_DET)
    {
        ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE);

        // we got an interrupt - we can turn off the timeout timer; not required anymore
        TIMING_TimerStop(lexUsb3Ulp.snoopModeTimeout);
    }

    // see if we have started a hot reset
    if (lexUlpInts & ULP_CORE_IRQ0_RAW_IN_HOT_RESET)
    {
        // tell the Lex there is a Hot Reset starting
        ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET);
    }

    // see if we have completed a hot reset
    if (lexUlpInts & ULP_CORE_IRQ0_RAW_COMPLETED_HOT_RESET)
    {
        _ULP_halSetHotResetWait(true);  // wait the next time we get into a Hot Reset
    }

    // see if we have started a warm reset
    if (lexUlpInts & ULP_CORE_IRQ0_RAW_IN_WARM_RESET)
    {
        // tell the Lex there is a warm reset starting
        ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_WARM_RESET);
    }

    // have we completed a warm reset?
    if (lexUlpInts & ULP_CORE_IRQ0_RAW_COMPLETED_WARM_RESET)
    {
    }

    // have we received a request to go into U1?
    if (lexUlpInts & ULP_CORE_IRQ0_RAW_U1_ENTRY_RECEIVED)
    {
    }

    // have we received a request to go into U2?
    if (lexUlpInts & ULP_CORE_IRQ0_RAW_U2_ENTRY_RECEIVED)
    {
    }

    // have we received a request to go into U3?
    if (lexUlpInts & ULP_CORE_IRQ0_RAW_U3_ENTRY_RECEIVED)
    {
    }

    // are we in U3?
    if (lexUlpInts & ULP_CORE_IRQ0_RAW_IN_U3)
    {
        ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_IN_STANDBY);     // Lex is in standby mode (U3)
    }

    // have we started to exit from U3?
    if (lexUlpInts & ULP_CORE_IRQ0_RAW_U3_EXIT_INITIATED)
    {
        ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT);     // Lex is exiting standby mode (U3)
    }

    // have we exited from U3?
    if (lexUlpInts & ULP_CORE_IRQ0_RAW_U3_EXIT_COMPLETED)
    {
        // this standby cycle finished - next standby cycle, hold in low power until we want to exit
        _ULP_halControlLowPowerWaitMode(true);        
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
void ULP_LexUsb3Irq1(uint32_t lexUlpInts)
{

    // see if we are in U0
    if (lexUlpInts & ULP_CORE_IRQ1_RAW_IN_U0)
    {
        if (lexUsb3Ulp.ulpFsmState == LEX_ULP_USB3_LEX_SETUP)
        {
            ilog_ULP_COMPONENT_1(
                ILOG_MINOR_EVENT,
                ULP_LTSSM_VALID,
                LEON_TimerCalcUsecDiff(lexUsb3Ulp.ltssmStartTime, LEON_TimerRead()));
        }

        if (lexUsb3Ulp.ulpFsmState != LEX_ULP_USB3_ACTIVE)
        {
            // don't send U0 event if we are already active (to support U1/U2 states)
            ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_LEX_AT_U0);     // host USB3 setup complete
        }
    }

    // see if we are in the Loopback state
    if (lexUlpInts & ULP_CORE_IRQ1_RAW_IN_LOOPBACK)
    {
        if (lexUsb3Ulp.ulpFsmState == LEX_ULP_USB3_LEX_SETUP)
        {
            ulp_StatMonPhyControl(true);       // turn on phy stats count if we are in Loopback mode
        }
    }

    if (lexUlpInts & ~ULP_CORE_IRQ1_RAW_IN_INACTIVE)
    {
        // if there is some other state then inactive, make sure the timer is stopped
        // if inactive is also set, the timer will be started again, below
        TIMING_TimerStop(lexUsb3Ulp.inactiveStuckTimeout);
    }

    if (lexUlpInts & ULP_CORE_IRQ1_RAW_IN_INACTIVE)
    {

        ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_LEX_USB_3_IRQ_1, 
                                lexUsb3Ulp.rexReqInactive, lexUsb3Ulp.standbyExit, lexUsb3Ulp.LexOnlyResetTriggered);

        // if we went to the inactive state, increment the count
        // and re-start the clear timer
        lexUsb3Ulp.inactiveCount++;

        // start the stuck timer, so if we are stuck here we will reset
        TIMING_TimerStart(lexUsb3Ulp.inactiveStuckTimeout);

        // make sure we are in the right state to go to reset
        if ( ( (lexUsb3Ulp.rexReqInactive == true) || (lexUsb3Ulp.standbyExit == false) )
               && (UlpLexUsb3Status() == ULP_USB_MODULE_CONNECTED) )
        {
            // reset the Lex
            if (lexUsb3Ulp.LexOnlyResetTriggered == false)
            {
                lexUsb3Ulp.LexOnlyResetTriggered = true;
                lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_LEX_ONLY_RESET;
                LexUsb3TurnOffUsb3();   // turn off the USB3 state machine
                ULP_UlpLexUsb3LexOnlyResetStart();
            }
            else
            {
                LexUsb3FailureRecovery();
            }
        }
        else
        {
            // if we are exiting from standby, clear this flag so we can do another Lex only reset if necessary
            // partial fix for #4852 - BIOS can't wait for a full reset, and will not detect USB2 devices if we
            // try to do one
            lexUsb3Ulp.LexOnlyResetTriggered = false;
        }

        // these flags are no longer needed, once we've handled the inactive state
        lexUsb3Ulp.rexReqInactive = false;
        lexUsb3Ulp.standbyExit = false;
    }
}


//#################################################################################################
// Sends control operations to the USB3 module.  If the module is enabled, and connected,
// it will run.  Otherwise it will stop
//
// Parameters:
// Return:  true if the module is enabled - false otherwise
// Assumptions:
//
//#################################################################################################
bool ULP_LexUsb3Control(enum UlpUsbControl controlOperation)
{
    enum UlpUsbControlResult controlResult = ULP_UsbSetControl(&lexUsb3Ulp.controlFlags, controlOperation);

    switch (controlResult)
    {
        case ULP_USB_CONTROL_RESULT_ENABLE:          // this USB module can be enabled
            ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_ENABLED);
            break;

        case ULP_USB_CONTROL_RESULT_DISABLE:         // this USB module can be disabled
        default:
            ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_DISABLED);
            break;

        case ULP_USB_CONTROL_RESULT_UNCHANGED:       // no change since the last time this module was called
            break;
    }

    ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_LEX_USB3_SET_CONTROL_RESULT, lexUsb3Ulp.controlFlags, controlOperation, controlResult);

    return (ULP_UsbControlEnabled(lexUsb3Ulp.controlFlags));
}

//#################################################################################################
// Returns the current USB3 state machine status
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
enum UlpUsbModuleStatus UlpLexUsb3Status(void)
{
    enum UlpUsbModuleStatus result = ULP_USB_MODULE_DISABLED;

    switch (lexUsb3Ulp.ulpFsmState)
    {
        case LEX_ULP_USB3_STATE_READY:       // USB3 port state machine is ready to connect
        case LEX_ULP_USB3_RX_DETECT:         // See if we can detect Rx terminations on the far side
            result = ULP_USB_MODULE_READY;
            break;

        case LEX_ULP_USB3_FAILED:            // USB3 failed on either the Rex or Lex
            result = ULP_USB_MODULE_DISCONNECTED;
            break;

        case LEX_ULP_USB3_REX_SETUP:         // Waiting for Rex to indicate it has setup USB3
        case LEX_ULP_USB3_LEX_SETUP:         // Waiting for Lex to indicate it has setup USB3
        case LEX_ULP_USB3_ACTIVE:            // USB3 active on Lex and Rex
        case LEX_ULP_USB3_REX_HOT_RESET_WAIT:    // waiting for Hot Reset to finish on the Rex
        case LEX_ULP_USB3_REX_WARM_RESET_WAIT:   // waiting for warm reset to finish on the Rex
        case LEX_ULP_USB3_STANDBY:           // Lex USB3 is in standby (U3)
        case LEX_ULP_USB3_STANDBY_HOST_EXIT: // Lex USB3 is exiting standby (Host initiated exit)
        case LEX_ULP_USB3_LEX_ONLY_RESET:    // Lex USB3 is in a Lex only reset state
            result = ULP_USB_MODULE_CONNECTED;
            break;

        case LEX_ULP_USB3_STATE_DISABLED:    // USB3 port state machine disabled
        default:
            if ( ULP_LexUsb3SystemEnabled() )
            {
                result = ULP_USB_MODULE_DISABLED;
            }
            else
            {
                // USB2 is disabled system wide - say we are shutdown
                result = ULP_USB_MODULE_SHUTDOWN;
            }
            break;
    }

    return (result);
}

//#################################################################################################
// Sends the state machine a connect or disconnect event
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpLexUsb3HostConnect(bool connect)
{
    // tell the USB3 state machine we are connected or not
    ULP_LexUsb3StateMachineSendEvent(
        connect ? LEX_ULP_USB3_EVENT_HOST_CONNECT : LEX_ULP_USB3_EVENT_HOST_DISCONNECT);
}


//#################################################################################################
// handles the USB3 messages CPU messages from the Rex
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_LexUsb3RxCpuMessageHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    iassert_ULP_COMPONENT_1(msgLength == 0, ULP_LEX_UNEXPECTED_CPU_MSG_LENGTH, msgLength);
    ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_LEX_RCV_REX_USB3_MSG, subType);

    enum UlpRexSendUsb3Message rexUsb3Message = (enum UlpRexSendUsb3Message)subType ;

    if (ULP_LexUsb3Enabled())
    {
        switch (rexUsb3Message)
        {
            case ULP_REX_TO_LEX_USB3_MODULE_DISABLED:       // Rex to Lex - Rex USB3 module is disabled
                ULP_LexUsb3Control(ULP_USB_CONTROL_PARTNER_SHUTDOWN);  // partner is disabled!
                break;

            case ULP_REX_TO_LEX_USB3_MODULE_ENABLED:       // Rex to Lex - Rex USB3 module is enabled
                ULP_LexUsb3Control(ULP_USB_CONTROL_PARTNER_ENABLED);  // partner is enabled!
                ULP_UlpLexUsb3ResetStart();                 // part of the link/power up cycle
                UlpLexRexUsbAllowed();
                break;

            case ULP_REX_TO_LEX_USB3_READY:       // Rex to Lex - Rex USB3 is ready, waiting for a connection
                ULP_LexUsb3Control(ULP_USB_CONTROL_PARTNER_READY);  // partner is ready!
                break;

            case ULP_REX_TO_LEX_USB3_DISABLED:       // Rex to Lex - Rex USB3 is currently disabled
                ULP_LexUsb3Control(ULP_USB_CONTROL_PARTNER_DISABLED);  // partner is disabled!
                break;

            case ULP_REX_TO_LEX_USB3_REX_AT_U0:        // USB3 setup on Rex successful
                ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_REX_AT_U0);
                break;

            case ULP_REX_TO_LEX_USB3_FAILURE:      // USB3 setup on Rex failed
                ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_REX_FAILED);
                break;

            case ULP_REX_TO_LEX_USB3_DEVICE_DISCONNECTED:    // Rex to Lex - Device disconnected
                ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED);
                break;

            case ULP_REX_TO_LEX_USB3_REX_AT_INACTIVE:        // the Rex is in the INACTIVE state, and needs a warm reset
                ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_REX_AT_INACTIVE);
                break;

            default:
                break;
        }
    }
    else if ( (rexUsb3Message != ULP_REX_TO_LEX_USB3_MODULE_DISABLED) && (rexUsb3Message != ULP_REX_TO_LEX_USB3_DISABLED) )
    {
        UlpSendCPUCommLexUsb3Message( ULP_LEX_TO_REX_USB3_MODULE_DISABLED);  // tell Rex that USB3 is disabled on the Lex
    }
}

//#################################################################################################
// Starts a Lex only reset
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpLexUsb3LexOnlyResetStart(void)
{
    // reset the Lex
    lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_LEX_ONLY_RESET;
    ULP_UlpLexUsb3LexOnlyResetStart();
}

//#################################################################################################
// Called when the Lex only reset is done
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpLexUsb3LexOnlyResetDone(void)
{
    lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_REX_SETUP;                // wait for response
}

// Static Function Definitions ####################################################################

//#################################################################################################
// Entry point to send an event to the Host port state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_LexUsb3StateMachineSendEvent(enum LexUlpUsb3Event event)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = event;

    CALLBACK_Run(ULP_LexUsb3StateCallback, (void *)eventx, NULL);
}


//#################################################################################################
// Callback wrapper for the Host port state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_LexUsb3StateCallback(void *param1, void *param2)
{
    ULP_LexUsb3StateMachine( (enum LexUlpUsb3Event)param1 );
}


//#################################################################################################
// The entry point for the Host port state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_LexUsb3StateMachine(enum LexUlpUsb3Event LexUsb3Event)
{
    uint8_t oldState = lexUsb3Ulp.ulpFsmState;

    switch (lexUsb3Ulp.ulpFsmState)
    {
        case LEX_ULP_USB3_STATE_DISABLED:    // USB2 state machine disabled
            UlpLexUsb3StateDisabled(LexUsb3Event);
            break;

        case LEX_ULP_USB3_STATE_READY:       // USB3 port state machine is ready to connect
            UlpLexUsb3StateReady(LexUsb3Event);
            break;

        case LEX_ULP_USB3_RX_DETECT:         // See if we can detect Rx terminations on the far side
            UlpLexUsb3StateRxDetect(LexUsb3Event);
            break;

        case LEX_ULP_USB3_REX_SETUP:         // Waiting for the Rex to indicate it has setup USB3
            UlpLexUsb3StateRexSetup(LexUsb3Event);
            break;

        case LEX_ULP_USB3_LEX_SETUP:         // Waiting for the Lex to indicate it has setup USB3
            UlpLexUsb3StateLexSetup(LexUsb3Event);
            break;

        case LEX_ULP_USB3_ACTIVE:            // USB2 active on Lex and Rex
            UlpLexUsb3StateActive(LexUsb3Event);
            break;

        case LEX_ULP_USB3_FAILED:            // USB3 failed to setup on either the Rex or Lex
            UlpLexUsb3StateFailed(LexUsb3Event);
            break;

        case LEX_ULP_USB3_REX_HOT_RESET_WAIT:    // waiting for Hot Reset to finish on the Rex
            UlpLexUsb3StateWaitRexHotReset(LexUsb3Event);
            break;

        case LEX_ULP_USB3_REX_WARM_RESET_WAIT:   // waiting for warm reset to finish on the Rex
            UlpLexUsb3StateWaitRexWarmReset(LexUsb3Event);
            break;

        case LEX_ULP_USB3_STANDBY:            // Lex is in the standby state
            UlpLexUsb3StateStandby(LexUsb3Event);
            break;

        case LEX_ULP_USB3_STANDBY_HOST_EXIT:    // Lex USB3 is exiting standby (Host initiated exit)
            UlpLexUsb3StateStandbyHostExited(LexUsb3Event);
            break;

        case LEX_ULP_USB3_LEX_ONLY_RESET:    // Lex USB3 is in a Lex only reset state
            UlpLexUsb3StateWaitLexOnlyResetDone(LexUsb3Event);
            break;

        default:
            break;
    }

    if (oldState != lexUsb3Ulp.ulpFsmState)
    {
        EVENT_Trigger(ET_USB3_STATUS_CHANGE, LexUsb3GetUsb3Status());
    }

    ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_LEX_USB3_STATE_MSG, oldState, LexUsb3Event, lexUsb3Ulp.ulpFsmState);
}


//#################################################################################################
// Handles events in the LEX_ULP_USB3_STATE_DISABLED
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb3StateDisabled(enum LexUlpUsb3Event event)
{
    switch (event)
    {
        case LEX_ULP_USB3_EVENT_ENABLED:     // enable USB3
            lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_STATE_READY;  // wait until there is a host port for us to connect to
            UlpLexHostSendUsb3Status(ULP_USB_MODULE_READY);     // tell the host port we are ready
            break;

        case LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED:     // Device disconnected from Rex
        case LEX_ULP_USB3_EVENT_REX_AT_U0:              // Rex USB 3 is setup
        case LEX_ULP_USB3_EVENT_REX_FAILED:             // Rex failed USB3 setup
        case LEX_ULP_USB3_EVENT_REX_WARM_RESET_DONE:    // Warm reset done on the Rex
        case LEX_ULP_USB3_EVENT_REX_AT_INACTIVE:         // Rex is in the inactive state
            UlpSendCPUCommLexUsb3Message(ULP_LEX_TO_REX_USB3_DISABLED);  // tell Rex that USB3 is disabled on the Lex
            break;

            // these events don't mean anything in this state
        case LEX_ULP_USB3_EVENT_HOST_CONNECT:           // connect to a host
        case LEX_ULP_USB3_EVENT_HOST_DISCONNECT:        // disconnect from the host
        case LEX_ULP_USB3_EVENT_LEX_AT_U0:      // Lex USB 3 is setup
        case LEX_ULP_USB3_EVENT_LEX_FAILED:     // Lex failed USB3 setup
        case LEX_ULP_USB3_EVENT_DISABLED:       // disable USB3
        case LEX_ULP_USB3_EVENT_WARM_RESET:             // Lex went through a warm reset
        case LEX_ULP_USB3_EVENT_IN_STANDBY:             // Lex is in standby mode (U3)
        case LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT:       // Lex is exiting standby
        case LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in the Hot Reset state
        case LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE:     // Rx termination change detected!

        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_USB3_STATE_READY - waits for a host connect
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb3StateReady(enum LexUlpUsb3Event event)
{
    switch (event)
    {
        case LEX_ULP_USB3_EVENT_DISABLED:       // disable USB3
            LexUsb3SetDisabledState();          // Turn off USB3, go to the disabled state
            break;

        case LEX_ULP_USB3_EVENT_HOST_CONNECT:           // connect to a host
            ULP_controlLexBringUpUSB3();

            lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_RX_DETECT;    // wait until we detect far side terminations

            // start the timer to trigger USB2 if we can't detect the far end terminations
            // (this will also deal with the case where we power up already connected to a USB3 port)
//            ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE);
            TIMING_TimerStart(lexUsb3Ulp.snoopModeTimeout);
            break;

        case LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED:     // Device disconnected from Rex
        case LEX_ULP_USB3_EVENT_REX_AT_U0:              // Rex USB 3 is setup
        case LEX_ULP_USB3_EVENT_REX_FAILED:             // Rex failed USB3 setup
        case LEX_ULP_USB3_EVENT_REX_WARM_RESET_DONE:    // Warm reset done on the Rex
        case LEX_ULP_USB3_EVENT_REX_AT_INACTIVE:         // Rex is in the inactive state
            UlpSendCPUCommLexUsb3Message(ULP_LEX_TO_REX_USB3_DISABLED);  // tell Rex that USB3 is disabled on the Lex
            break;

            // these events don't mean anything in this state
        case LEX_ULP_USB3_EVENT_HOST_DISCONNECT:        // disconnect from the host
        case LEX_ULP_USB3_EVENT_LEX_AT_U0:              // Lex USB 3 is setup
        case LEX_ULP_USB3_EVENT_LEX_FAILED:             // Lex failed USB3 setup
        case LEX_ULP_USB3_EVENT_WARM_RESET:             // Lex went through a warm reset
        case LEX_ULP_USB3_EVENT_IN_STANDBY:             // Lex is in standby mode (U3)
        case LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT:       // Lex is exiting standby
        case LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in the Hot Reset state
        case LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE:     // Rx termination change detected!
        case LEX_ULP_USB3_EVENT_ENABLED:        // enable USB3
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_USB3_RX_DETECT state.  Let the USB3 state machine poll for far end
// Rx terminations.  We will get an event when we are in the polling state
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb3StateRxDetect(enum LexUlpUsb3Event event)
{
    switch (event)
    {
        case LEX_ULP_USB3_EVENT_DISABLED:       // disable USB3
            LexUsb3SetDisabledState();          // Turn off USB3, go to the disabled state
            break;

        case LEX_ULP_USB3_EVENT_HOST_DISCONNECT:        // disconnect from the host
            LexUSB3HandleDisconnectEvent();
            break;

        case LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE:     // Rx termination change detected! (or 1st time timeout)
            if (UlpHalRxTerminationsPresent())
            {
                // terminations present!  This is a USB3 port
                UlpSendCPUCommLexUsb3Message(ULP_LEX_TO_REX_USB3_CONNECTED);    // tell Rex to connect USB3
                lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_REX_SETUP;                // wait for response
            }
            else
            {
                // far end terminations not present, USB2 only port
                ULP_LexHostUsb3Connected(false);    // tell the host state machine that the port is not USB3 capable
                // continue in this state until we detect far end rx terminations, or we are disabled
            }
            break;

            // these events don't mean anything in this state
        case LEX_ULP_USB3_EVENT_HOST_CONNECT:           // connect to a host
        case LEX_ULP_USB3_EVENT_REX_AT_U0:      // Rex USB 3 is setup
        case LEX_ULP_USB3_EVENT_REX_FAILED:     // Rex failed USB3 setup
        case LEX_ULP_USB3_EVENT_REX_AT_INACTIVE:    // Rex is in the inactive state
        case LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED: // Device disconnected from Rex
        case LEX_ULP_USB3_EVENT_LEX_AT_U0:      // Lex USB 3 is setup
        case LEX_ULP_USB3_EVENT_LEX_FAILED:     // Lex failed USB3 setup
        case LEX_ULP_USB3_EVENT_ENABLED:        // enable USB3
        case LEX_ULP_USB3_EVENT_WARM_RESET:     // Lex went through a warm reset
        case LEX_ULP_USB3_EVENT_REX_WARM_RESET_DONE:    // Warm reset done on the Rex
        case LEX_ULP_USB3_EVENT_IN_STANDBY:             // Lex is in standby mode (U3)
        case LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT:       // Lex is exiting standby
        case LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in the Hot Reset state
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_USB3_REX_SETUP - in this state, the host does not see the Lex,
// because the Rx terminations are off
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb3StateRexSetup(enum LexUlpUsb3Event event)
{
    // TODO: add an event/timeout so we keep on telling the Rex we have a USB3 port if
    // we don't hear back from the rex within a certain time (ie the rex is still booting up)
    switch (event)
    {
        case LEX_ULP_USB3_EVENT_DISABLED:       // disable USB3
            LexUsb3SetDisabledState();          // Turn off USB3, go to the disabled state
            break;

        case LEX_ULP_USB3_EVENT_HOST_DISCONNECT:        // disconnect from the host
            LexUSB3HandleDisconnectEvent();
            break;

        case LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED:     // Device disconnected/not present at the Rex
            break;

        case LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE:     // Rx termination change detected!
            LexUsbRxTerminationsRemoved();              // go back to looking for Rx terminations, again
            break;

        case LEX_ULP_USB3_EVENT_REX_AT_U0:      // Rex USB 3 is setup
            ULP_LexHostUsb3Connected(true);     // tell the host state machine that the port is USB3 capable
            LexSetupUSB3();                     // now setup USB3 on the Lex
            lexUsb3Ulp.warmResetCount = 0;
           break;

        case LEX_ULP_USB3_EVENT_REX_FAILED:     // Rex failed USB3 setup
        case LEX_ULP_USB3_EVENT_REX_AT_INACTIVE:    // Rex is in the inactive state
            ilog_ULP_COMPONENT_1(ILOG_MINOR_EVENT, ULP_LEX_USB3_REX_SETUP_ERROR_DETECTED, lexUsb3Ulp.failCount);
            LexUsb3FailureRecovery();
            break;

            // these events don't mean anything in this state
        case LEX_ULP_USB3_EVENT_HOST_CONNECT:           // connect to a host
        case LEX_ULP_USB3_EVENT_LEX_AT_U0:              // Lex USB 3 is setup
        case LEX_ULP_USB3_EVENT_LEX_FAILED:             // Lex failed USB3 setup
        case LEX_ULP_USB3_EVENT_ENABLED:                // enable USB3
        case LEX_ULP_USB3_EVENT_WARM_RESET:             // Lex went through a warm reset
        case LEX_ULP_USB3_EVENT_REX_WARM_RESET_DONE:    // Warm reset done on the Rex
        case LEX_ULP_USB3_EVENT_IN_STANDBY:             // Lex is in standby mode (U3)
        case LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT:       // Lex is exiting standby
        case LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in the Hot Reset state
        default:
            break;
    }
}

//#################################################################################################
// Handles events in the LEX_ULP_USB3_LEX_SETUP
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions: Rex is in U0, Lex just had Rx terminations turned on
//
//#################################################################################################
static void UlpLexUsb3StateLexSetup(enum LexUlpUsb3Event event)
{
    // TODO: add an event/timeout so we keep on telling the Rex we have a USB2 port if
    // we don't hear back from the Rex within a certain time
    switch (event)
    {
        case LEX_ULP_USB3_EVENT_DISABLED:       // disable USB3
            LexUsb3SetDisabledState();          // Turn off USB3, go to the disabled state
            break;

        case LEX_ULP_USB3_EVENT_HOST_DISCONNECT:        // disconnect from the host
            LexUSB3HandleDisconnectEvent();
            break;

        case LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED:     // Device disconnected from Rex
            LexUsbNoRexDevice();
            break;

        case LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE:     // Rx termination change detected!
            if (!UlpHalRxTerminationsPresent())
            {
                LexUsbRxTerminationsRemoved();              // go back to looking for Rx terminations, again
            }
            break;

        case LEX_ULP_USB3_EVENT_LEX_AT_U0:      // Lex USB 3 is setup
            lexUsb3Ulp.usbSuspendIstatusFlag = 0;
            ILOG_istatus(ISTATUS_ULP_LEX_USB3_ACTIVE, 0);
            lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_ACTIVE;               // USB3 is active!
            UlpSendCPUCommLexUsb3Message(ULP_LEX_TO_REX_USB3_SETUP_SUCCESS); // tell Rex USB3 setup succeeded on the Lex
            TIMING_TimerStop(lexUsb3Ulp.ltssmTimer);    // exiting state, no need to poll for a timeout
            lexUsb3Ulp.failCount = 0;                   // at U0 - clear error count
            ulp_StatMonPhyControl(true);                // turn on phy stats count
            UlpLexHostSendUsb3Status(ULP_USB_MODULE_CONNECTED);     // tell the host port we are connected to USB3
            break;

        case LEX_ULP_USB3_EVENT_WARM_RESET:             // Lex went through a warm reset
            LexSetupWarmResetWait();                    // go to the warm reset state, waiting for the Rex to finish
            TIMING_TimerStop(lexUsb3Ulp.ltssmTimer);    // exiting state, no need to poll for a timeout
            break;

        case LEX_ULP_USB3_EVENT_LEX_FAILED:     // Lex failed USB3 setup
        case LEX_ULP_USB3_EVENT_REX_AT_INACTIVE:    // Rex is in the inactive state
            ilog_ULP_COMPONENT_1(ILOG_MINOR_EVENT, ULP_LEX_USB3_LEX_SETUP_ERROR_DETECTED, lexUsb3Ulp.failCount);
            LexUsb3FailureRecovery();
            break;

            // these events don't mean anything in this state
        case LEX_ULP_USB3_EVENT_HOST_CONNECT:           // connect to a host
        case LEX_ULP_USB3_EVENT_REX_FAILED:             // Rex failed USB3 setup
        case LEX_ULP_USB3_EVENT_REX_AT_U0:              // Rex USB 3 is setup
        case LEX_ULP_USB3_EVENT_ENABLED:                // enable USB3
        case LEX_ULP_USB3_EVENT_REX_WARM_RESET_DONE:    // Warm reset done on the Rex
        case LEX_ULP_USB3_EVENT_IN_STANDBY:             // Lex is in standby mode (U3)
        case LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT:       // Lex is exiting standby
        case LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in the Hot Reset state
        default:
            break;
    }
}

//#################################################################################################
// Handles events in the LEX_ULP_USB3_ACTIVE - USB3 is in the U0 state
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions: Lex and Rex are in U0
//
//#################################################################################################
static void UlpLexUsb3StateActive(enum LexUlpUsb3Event event)
{
    switch (event)
    {
        case LEX_ULP_USB3_EVENT_DISABLED:       // disable USB3
            LexUsb3SetDisabledState();          // Turn off USB3, go to the disabled state
            break;

        case LEX_ULP_USB3_EVENT_HOST_DISCONNECT:        // disconnect from the host
            LexUSB3HandleDisconnectEvent();
            break;

        case LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED: // Device disconnected from Rex
            LexUsbNoRexDevice();
            break;

        case LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:   // Lex is in the Hot Reset state
            LexSetupHotResetWait();
            break;

        case LEX_ULP_USB3_EVENT_WARM_RESET:         // Lex went through a warm reset
            LexSetupWarmResetWait();                // go to the warm reset state, waiting for the Rex to finish
            break;

        case LEX_ULP_USB3_EVENT_IN_STANDBY:         // Lex is in standby mode (U3)
            lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_STANDBY;  // go to the standby state
            lexUsb3Ulp.usbSuspendIstatusFlag = 1;
            ILOG_istatus(ISTATUS_ULP_LEX_USB3_SUSPENDED, 0);
            UlpSendCPUCommLexUsb3Message(ULP_LEX_TO_REX_LEX_IN_STANDBY);    // tell the Rex we are in Standby
            ulp_StatMonPhyControl(false);           // turn off phy stats count
            break;

        case LEX_ULP_USB3_EVENT_REX_AT_INACTIVE:    // Rex is in the inactive state
            UlpHalGoToInactive();                 // mirror it on the Lex (should cause a warm reset on the Lex)
            // because of a bug in the Cypress Hub, sometimes an inactive state does not get a warm
            // reset.  Go to the Lex setup state, so if we are stuck at inactive, we will detect it
            lexUsb3Ulp.rexReqInactive = true;   // Rex requested we go to inactive
            LexSetupUSB3();
            break;

        case LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE:     // Rx termination change detected!
            LexUsbRxTerminationsRemoved();              // go back to looking for Rx terminations, again
            break;

            // these events don't mean anything in this state
        case LEX_ULP_USB3_EVENT_HOST_CONNECT:           // connect to a host
        case LEX_ULP_USB3_EVENT_REX_AT_U0:      // Rex USB 3 is setup
        case LEX_ULP_USB3_EVENT_REX_FAILED:     // Rex failed USB3 setup
        case LEX_ULP_USB3_EVENT_LEX_AT_U0:      // Lex USB 3 is setup
        case LEX_ULP_USB3_EVENT_LEX_FAILED:     // Lex failed USB3 setup
            // TODO: assert for these messages (above)?
        case LEX_ULP_USB3_EVENT_ENABLED:                // enable USB3
        case LEX_ULP_USB3_EVENT_REX_WARM_RESET_DONE:    // Warm reset done on the Rex
        case LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT:       // Lex is exiting standby
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_USB3_REX_HOT_RESET_WAIT - waiting for the rex to exit
// hot reset
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb3StateWaitRexHotReset(enum LexUlpUsb3Event event)
{
    switch (event)
    {
        case LEX_ULP_USB3_EVENT_DISABLED:       // disable USB3
            LexUsb3SetDisabledState();          // Turn off USB3, go to the disabled state
            break;

        case LEX_ULP_USB3_EVENT_HOST_DISCONNECT:        // disconnect from the host
            LexUSB3HandleDisconnectEvent();
            break;

        case LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED: // Device disconnected from Rex
            LexUsbNoRexDevice();
            break;

        case LEX_ULP_USB3_EVENT_REX_AT_U0:          // Rex USB 3 is linked
            _ULP_halSetHotResetWait(false);         // exit from hot reset
            LexSetupUSB3();                         // now setup USB3 on the lex
            break;

        case LEX_ULP_USB3_EVENT_REX_FAILED:     // Rex failed USB3 setup
        case LEX_ULP_USB3_EVENT_REX_AT_INACTIVE:    // Rex is in the inactive state
            ilog_ULP_COMPONENT_1(ILOG_MINOR_EVENT, ULP_LEX_USB3_REX_SETUP_ERROR_DETECTED, lexUsb3Ulp.failCount);
            LexUsb3FailureRecovery();
            break;

            // TODO: deal with these cases
        case LEX_ULP_USB3_EVENT_WARM_RESET:     // Lex went through a warm reset
            break;

        case LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE:     // Rx termination change detected!
            LexUsbRxTerminationsRemoved();              // go back to looking for Rx terminations, again
            break;

            // these events don't mean anything in this state
        case LEX_ULP_USB3_EVENT_HOST_CONNECT:           // connect to a host
        case LEX_ULP_USB3_EVENT_LEX_AT_U0:      // Lex USB 3 is setup
        case LEX_ULP_USB3_EVENT_LEX_FAILED:     // Lex failed USB3 setup
        case LEX_ULP_USB3_EVENT_ENABLED:        // enable USB3
        case LEX_ULP_USB3_EVENT_REX_WARM_RESET_DONE:    // Warm reset done on the Rex
        case LEX_ULP_USB3_EVENT_IN_STANDBY:             // Lex is in standby mode (U3)
        case LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT:       // Lex is exiting standby
        case LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in the Hot Reset state
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_USB3_REX_WARM_RESET_WAIT - waiting for the rex to exit
// warm reset
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb3StateWaitRexWarmReset(enum LexUlpUsb3Event event)
{
    switch (event)
    {
        case LEX_ULP_USB3_EVENT_DISABLED:       // disable USB3
            LexUsb3SetDisabledState();          // Turn off USB3, go to the disabled state
            break;

        case LEX_ULP_USB3_EVENT_HOST_DISCONNECT:        // disconnect from the host
            LexUSB3HandleDisconnectEvent();
            break;

        case LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED: // Device disconnected from Rex
            LexUsbNoRexDevice();
            break;

        case LEX_ULP_USB3_EVENT_REX_AT_U0:          // Rex USB 3 is linked
            if (lexUsb3Ulp.warmResetCount == 0)
            {
                TIMING_TimerStart(lexUsb3Ulp.warmResetClearTimeout);    // See how many we get
            }

            lexUsb3Ulp.warmResetCount++;  // keep track of how many times we've gone into warm reset

            if (lexUsb3Ulp.warmResetCount >= lexUsb3Ulp.lexUsb3WarmResetMaxCount)
            {
                LexUsb3FailureRecovery();
            }
            else
            {
                _ULP_halControlWaitInPolling(false);    // no longer wait for polling
                LexSetupUSB3();                         // now setup USB3 on the lex
            }
            break;

        case LEX_ULP_USB3_EVENT_REX_FAILED:     // Rex failed USB3 setup
        case LEX_ULP_USB3_EVENT_REX_AT_INACTIVE:    // Rex is in the inactive state
            ilog_ULP_COMPONENT_1(ILOG_MINOR_EVENT, ULP_LEX_USB3_REX_SETUP_ERROR_DETECTED, lexUsb3Ulp.failCount);
            LexUsb3FailureRecovery();
            break;

        case LEX_ULP_USB3_EVENT_WARM_RESET:     // Lex went through a warm reset
            break;

        case LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE:     // Rx termination change detected!
            LexUsbRxTerminationsRemoved();              // go back to looking for Rx terminations, again
            break;

            // these events don't mean anything in this state
        case LEX_ULP_USB3_EVENT_HOST_CONNECT:           // connect to a host
        case LEX_ULP_USB3_EVENT_LEX_AT_U0:      // Lex USB 3 is setup
        case LEX_ULP_USB3_EVENT_LEX_FAILED:     // Lex failed USB3 setup
        case LEX_ULP_USB3_EVENT_ENABLED:        // enable USB3
        case LEX_ULP_USB3_EVENT_REX_WARM_RESET_DONE:    // Warm reset done on the Rex
        case LEX_ULP_USB3_EVENT_IN_STANDBY:             // Lex is in standby mode (U3)
        case LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT:       // Lex is exiting standby
        case LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in the Hot Reset state
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_USB3_STANDBY state
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb3StateStandby(enum LexUlpUsb3Event event)
{
    switch (event)
    {
        case LEX_ULP_USB3_EVENT_DISABLED:       // disable USB3
            LexUsb3SetDisabledState();          // Turn off USB3, go to the disabled state
            break;

        case LEX_ULP_USB3_EVENT_HOST_DISCONNECT:        // disconnect from the host
            LexUSB3HandleDisconnectEvent();
            break;

        case LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED:      // Device disconnected from Rex
            LexUsbNoRexDevice();
            break;

        case LEX_ULP_USB3_EVENT_REX_AT_U0:      // Rex USB 3 is setup
            _ULP_halControlLowPowerWaitMode(false);  // exit the low power wait mode
            _ULP_halControlStandbyMode(false);  // exit standby on the Lex, too
            LexSetupUSB3();                     // now setup USB3 on the lex

            // the Rex is asking us to leave - on bug 5053, this caused the Lex to get stuck in
            // the inactive state, so go through a reset immediately in this case; don't
            // mark that we get to inactive by exiting standby
            // lexUsb3Ulp.standbyExit = true;      // we are exiting standby
            break;

        case LEX_ULP_USB3_EVENT_REX_FAILED:     // Rex failed USB3 setup
        case LEX_ULP_USB3_EVENT_REX_AT_INACTIVE:    // Rex is in the inactive state
            ilog_ULP_COMPONENT_1(ILOG_MINOR_EVENT, ULP_LEX_USB3_REX_SETUP_ERROR_DETECTED, lexUsb3Ulp.failCount);
            LexUsb3FailureRecovery();
            break;

        case LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT:       // Lex is exiting standby
            lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_STANDBY_HOST_EXIT;  // Host initiated standby exit
            UlpSendCPUCommLexUsb3Message(ULP_LEX_TO_REX_LEX_EXITING_STANDBY);  // tell Rex we are exiting standby
            break;

        case LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in the Hot Reset state
            LexSetupHotResetWait();
            break;

        case LEX_ULP_USB3_EVENT_WARM_RESET:         // Lex went through a warm reset
            LexSetupWarmResetWait();                // go to the warm reset state, waiting for the Rex to finish
            break;

        case LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE:     // Rx termination change detected!
            LexUsbRxTerminationsRemoved();              // go back to looking for Rx terminations, again
            break;

       // these events don't mean anything in this state
        case LEX_ULP_USB3_EVENT_HOST_CONNECT:           // connect to a host
        case LEX_ULP_USB3_EVENT_LEX_AT_U0:      // Lex USB 3 is setup
        case LEX_ULP_USB3_EVENT_LEX_FAILED:     // Lex failed USB3 setup
        case LEX_ULP_USB3_EVENT_ENABLED:        // enable USB3
        case LEX_ULP_USB3_EVENT_REX_WARM_RESET_DONE:    // Warm reset done on the Rex
        case LEX_ULP_USB3_EVENT_IN_STANDBY:             // Lex is in standby mode (U3)
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_USB3_STANDBY_HOST_EXIT state (Host initiated Standby exit)
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb3StateStandbyHostExited(enum LexUlpUsb3Event event)
{
    switch (event)
    {
        case LEX_ULP_USB3_EVENT_DISABLED:       // disable USB3
            LexUsb3SetDisabledState();          // Turn off USB3, go to the disabled state
            break;

        case LEX_ULP_USB3_EVENT_HOST_DISCONNECT:        // disconnect from the host
            LexUSB3HandleDisconnectEvent();
            break;

        case LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED:      // Device disconnected from Rex
            LexUsbNoRexDevice();
            break;

        case LEX_ULP_USB3_EVENT_REX_AT_U0:          // Rex USB 3 is setup
            _ULP_halControlLowPowerWaitMode(false); // exit the low power wait mode
            LexSetupUSB3();                         // now setup USB3 on the lex
            lexUsb3Ulp.standbyExit = true;      // we are exiting standby
            break;

        case LEX_ULP_USB3_EVENT_REX_FAILED:     // Rex failed USB3 setup
        case LEX_ULP_USB3_EVENT_REX_AT_INACTIVE:    // Rex is in the inactive state
            ilog_ULP_COMPONENT_1(ILOG_MINOR_EVENT, ULP_LEX_USB3_REX_SETUP_ERROR_DETECTED, lexUsb3Ulp.failCount);
            LexUsb3FailureRecovery();
            break;

        case LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT:   // Lex is exiting standby
            // we are exiting standby before the Rex - what to do?
            break;

        case LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in the Hot Reset state
            LexSetupHotResetWait();
            break;
        case LEX_ULP_USB3_EVENT_WARM_RESET:         // Lex went through a warm reset
            LexSetupWarmResetWait();                // go to the warm reset state, waiting for the Rex to finish
            break;

        case LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE:     // Rx termination change detected!
            LexUsbRxTerminationsRemoved();              // go back to looking for Rx terminations, again
            break;

       // these events don't mean anything in this state
        case LEX_ULP_USB3_EVENT_HOST_CONNECT:           // connect to a host
        case LEX_ULP_USB3_EVENT_LEX_AT_U0:      // Lex USB 3 is setup
        case LEX_ULP_USB3_EVENT_LEX_FAILED:     // Lex failed USB3 setup
        case LEX_ULP_USB3_EVENT_ENABLED:        // enable USB3
        case LEX_ULP_USB3_EVENT_REX_WARM_RESET_DONE:    // Warm reset done on the Rex
        case LEX_ULP_USB3_EVENT_IN_STANDBY:             // Lex is in standby mode (U3)
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_USB3_LEX_ONLY_RESET - waiting for the lex only reset to finish
// warm reset
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb3StateWaitLexOnlyResetDone(enum LexUlpUsb3Event event)
{
    switch (event)
    {
        case LEX_ULP_USB3_EVENT_DISABLED:       // disable USB3
            LexUsb3SetDisabledState();          // Turn off USB3, go to the disabled state
            break;

        case LEX_ULP_USB3_EVENT_HOST_DISCONNECT:        // disconnect from the host
            LexUsb3FailureRecovery();
            break;

        case LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED: // Device disconnected from Rex
            break;

        case LEX_ULP_USB3_EVENT_REX_FAILED:     // Rex failed USB3 setup
            ilog_ULP_COMPONENT_1(ILOG_MINOR_EVENT, ULP_LEX_USB3_REX_SETUP_ERROR_DETECTED, lexUsb3Ulp.failCount);
            LexUsb3FailureRecovery();
            break;

         case LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE:     // Rx termination change detected!
 //           LexUsbRxTerminationsRemoved();              // go back to looking for Rx terminations, again
            break;

            // these events don't mean anything in this state
         case LEX_ULP_USB3_EVENT_REX_AT_U0:          // Rex USB 3 is linked
        case LEX_ULP_USB3_EVENT_WARM_RESET:     // Lex went through a warm reset
        case LEX_ULP_USB3_EVENT_REX_AT_INACTIVE:    // Rex is in the inactive state
        case LEX_ULP_USB3_EVENT_HOST_CONNECT:           // connect to a host
        case LEX_ULP_USB3_EVENT_LEX_AT_U0:      // Lex USB 3 is setup
        case LEX_ULP_USB3_EVENT_LEX_FAILED:     // Lex failed USB3 setup
        case LEX_ULP_USB3_EVENT_ENABLED:        // enable USB3
        case LEX_ULP_USB3_EVENT_REX_WARM_RESET_DONE:    // Warm reset done on the Rex
        case LEX_ULP_USB3_EVENT_IN_STANDBY:             // Lex is in standby mode (U3)
        case LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT:       // Lex is exiting standby
        case LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in the Hot Reset state
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_USB3_FAILED state (USB3 failed enumeration; will reset once
// there is a disable or disconnect event)
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:  USB 3 is disabled in this state (vBus to USB 3 is off)
//
//#################################################################################################
static void UlpLexUsb3StateFailed(enum LexUlpUsb3Event event)
{
    switch (event)
    {
        case LEX_ULP_USB3_EVENT_DISABLED:       // disable USB3
            LexUsb3SetDisabledState();          // Turn off USB3, go to the disabled state
            break;

        case LEX_ULP_USB3_EVENT_HOST_DISCONNECT:        // disconnect from the host
            LexUSB3HandleDisconnectEvent();
            break;

        case LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE:     // Rx termination change detected!
            LexUsbRxTerminationsRemoved();              // go back to looking for Rx terminations, again
            break;

           // these events don't mean anything in this state
        case LEX_ULP_USB3_EVENT_HOST_CONNECT:           // connect to a host
        case LEX_ULP_USB3_EVENT_REX_DEVICE_REMOVED:     // Device disconnected from Rex
        case LEX_ULP_USB3_EVENT_REX_AT_U0:      // Rex USB 3 is setup
        case LEX_ULP_USB3_EVENT_REX_FAILED:     // Rex failed USB3 setup
        case LEX_ULP_USB3_EVENT_LEX_AT_U0:      // Lex USB 3 is setup
        case LEX_ULP_USB3_EVENT_LEX_FAILED:     // Lex failed USB3 setup
        case LEX_ULP_USB3_EVENT_ENABLED:        // enable USB3
        case LEX_ULP_USB3_EVENT_WARM_RESET:             // Lex went through a warm reset
        case LEX_ULP_USB3_EVENT_REX_WARM_RESET_DONE:    // Warm reset done on the Rex
        case LEX_ULP_USB3_EVENT_IN_STANDBY:             // Lex is in standby mode (U3)
        case LEX_ULP_USB3_EVENT_LEX_STANDBY_EXIT:       // Lex is exiting standby
        case LEX_ULP_USB3_EVENT_LEX_IN_HOT_RESET:       // Lex is in the Hot Reset state
        case LEX_ULP_USB3_EVENT_REX_AT_INACTIVE:        // Rex is in the inactive state
        default:
            break;
    }
}



//#################################################################################################
// LEX sets up USB3, to hopefully go to U0 (active).
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexSetupUSB3(void)
{
    // present RX terminations to host (exit snoop mode!)
    UlpHalControlRxTerminations(true);  // make sure Rx terminations are on
    _ULP_halSetHotResetWait(true);      // make sure we wait in hot reset, while the Rex mirrors it
    lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_LEX_SETUP;        // .. and go  wait for the Lex to setup
    TIMING_TimerStart(lexUsb3Ulp.ltssmTimer);
    lexUsb3Ulp.ltssmStartTime = LEON_TimerRead();

    ulp_StatMonPhyControl(false);        // make sure Phy stats are off until we link

    if (_ULP_IsLtssmValid() && !_ULP_halIsInactivePending())
    {
        // we are already linked - tell the state machine about this event
        ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_LEX_AT_U0);
    }
}


//#################################################################################################
// The Lex is in a hot reset state - tell the Rex, and wait until the Rex is done the hot reset
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexSetupHotResetWait(void)
{
    lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_REX_HOT_RESET_WAIT;  // go to the hot reset wait state
    UlpSendCPUCommLexUsb3Message(ULP_LEX_TO_REX_LEX_IN_HOT_RESET);      // tell the Rex that the Lex is in Hot Reset
    UlpStatusChange(ULP_STATUS_INBAND_HOT_RESET);               // tell UPP we are doing a hot reset
}


//#################################################################################################
// Put the Lex into the warm reset state, tell the Rex to generate a warm reset
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexSetupWarmResetWait(void)
{
    lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_REX_WARM_RESET_WAIT;      // go to the warm reset state
    _ULP_halControlWaitInPolling(true);     // wait in polling until the Rex is done warm reset
    ulp_StatMonPhyControl(false);           // turn off phy stats count
    UlpSendCPUCommLexUsb3Message(ULP_LEX_TO_REX_USB3_WARM_RESET);   // tell the Rex USB3 warm reset was done on the Lex

    UlpStatusChange(ULP_STATUS_INBAND_WARM_RESET); // a warm reset is in progress
}


//#################################################################################################
// Turns off USB3, tells the Rex we are disconnected, and then goes to the ready state, to
// wait for a connect message
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexUSB3HandleDisconnectEvent(void)
{
    ULP_UlpLexUsb3LexOnlyResetStop();
    LexUsb3TurnOffUsb3();   // turn off the USB3 state machine

    lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_STATE_DISABLED;           // go to the disabled state
    UlpSendCPUCommLexUsb3Message(ULP_LEX_TO_REX_USB3_DISCONNECTED); // tell Rex to turn off its USB3
    ULP_LexUsb3Control(ULP_USB_CONTROL_PARTNER_DISABLED);           // partner is disabled!
}

//#################################################################################################
// Tear down USB3 on the Lex - put it into failure mode
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexUSB3SetFailedState(void)
{
    LexUsb3TurnOffUsb3();   // turn off the USB3 state machine

    lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_FAILED;               // go to the failure state for USB3
    UlpSendCPUCommLexUsb3Message(ULP_LEX_TO_REX_USB3_FAILURE);  // tell Rex to turn off its USB3
    UlpLexHostSendUsb3Status(ULP_USB_MODULE_DISCONNECTED);      // tell the host port we are disconnected (failure state)
}


//#################################################################################################
// Tear down USB3 on the Lex - disable it
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexUsb3SetDisabledState(void)
{
    ULP_UlpLexUsb3LexOnlyResetStop();
    LexUsb3TurnOffUsb3();   // turn off the USB3 state machine

    lexUsb3Ulp.LexOnlyResetTriggered = false;
    lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_STATE_DISABLED;       // go to the disabled state
    UlpSendCPUCommLexUsb3Message(ULP_LEX_TO_REX_USB3_DISABLED); // tell Rex to turn off its USB3
    UlpLexHostSendUsb3Status(ULP_USB_MODULE_DISABLED);     // tell the host port we are disabled
}

//#################################################################################################
// Turn off the USb3 state machine on the Lex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexUsb3TurnOffUsb3(void)
{
    UlpHalDisableUlpInterrupts( LEX_ULP_USB3_IRQ0, LEX_ULP_USB3_IRQ1);
    ULP_controlTurnOffUSB3();   // disable the ULP core state machine

    TIMING_TimerStop(lexUsb3Ulp.ltssmTimer);    // tearing down USB3, make sure timer is off
    TIMING_TimerStop(lexUsb3Ulp.snoopModeTimeout);
    TIMING_TimerStop(lexUsb3Ulp.warmResetClearTimeout);    // no need to check for warm resets
    TIMING_TimerStop(lexUsb3Ulp.inactiveStuckTimeout);
    TIMING_TimerStop(lexUsb3Ulp.inactiveClearCountTimeout);
    lexUsb3Ulp.usbSuspendIstatusFlag = 0;
    lexUsb3Ulp.warmResetCount = 0;
    lexUsb3Ulp.inactiveCount  = 0;
    lexUsb3Ulp.rexReqInactive = false;   // request no longer pending
    lexUsb3Ulp.standbyExit = false;      // no longer exiting standby


}

//#################################################################################################
// No device on the Rex - turn off Rx terminations on the Lex side, so the host can see
// there is no device; wait in the Rex setup state until a device is present on the Rex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexUsbNoRexDevice(void)
{
    LexUsb3TurnOffUsb3();   // make sure the USB3 state machine is off
    ULP_controlLexBringUpUSB3();

    // go back to waiting for a device to be connected to the Rex
    lexUsb3Ulp.ulpFsmState = LEX_ULP_USB3_REX_SETUP;
}

//#################################################################################################
// No device on the Rex - turn off Rx terminations on the Lex side, so the host can see
// there is no device
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexUsbRxTerminationsRemoved(void)
{
    // make sure we are in the failed state until we get re-connected
    // (will tell the host we are disconnected)
    LexUSB3SetFailedState();
}


//#################################################################################################
// LEX checks the status of LTSSM register.  Polls every 100ms
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexCheckLtssm(void)
{
    if (lexUsb3Ulp.ulpFsmState == LEX_ULP_USB3_LEX_SETUP)
    {
        const uint8_t currentLtssm = _ULP_GetLtssmMajorState();

        if (currentLtssm == ULP_LTSSM_U3)
        {
            // section 7.5.9.2, of the USB3.1 spec
            // 100 ms (tU3WakeupRetryDelay) after an unsuccessful LFPS handshake and the requirement to exit
            // U3 still exists, then the port shall initiate the U3 wakeup LFPS Handshake signaling
            // to wake up the host
            _ULP_halControlLowPowerWaitMode(false);         // exit the low power wait mode
            _ULP_halControlStandbyMode(false);              // exit standby on the Lex, too
            lexUsb3Ulp.ltssmStartTime = LEON_TimerRead();   // reset the timeout value
        }
        else if ( (currentLtssm == ULP_LTSSM_COMPLIANCE) || (currentLtssm == ULP_LTSSM_LOOPBACK) )
        {
            // if we are in compliance mode (or loopback) on the Lex, stay there
            // (TODO: mirror it to the Rex?  Add a distinct state for compliance mode?)
            lexUsb3Ulp.ltssmStartTime = LEON_TimerRead(); // reset the timeout value
        }
        else if (LEON_TimerCalcUsecDiff(lexUsb3Ulp.ltssmStartTime, LEON_TimerRead()) > lexUsb3Ulp.lexLtssmTotalTimeoutUs)
        {
            // Time out and ltssm register value is still invalid - Host USB3 setup failed!
            ilog_ULP_COMPONENT_1(ILOG_MAJOR_ERROR, ULP_LTSSM_INVALID, UlpHalGetLTSSM());

            // the inactive state has the stuck inactive timer to deal with it
            // ignore if we are stuck in the inactive state
            if (currentLtssm != ULP_LTSSM_INACTIVE)
            {
                ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_LEX_FAILED);
            }

            lexUsb3Ulp.ltssmStartTime = LEON_TimerRead();   // reset the timeout value TEST
        }
        /*
        if (_ULP_IsLtssmValid())
        {
            ilog_ULP_COMPONENT_0(ILOG_MINOR_EVENT, ULP_LEX_LTSSM_VALID);
            ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_LEX_AT_U0);     // host USB3 setup complete
        }
        */
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
static uint32_t LexUsb3GetUsb3Status(void)
{
    enum EventUsb3Status status = USB3_IN_RESET;

    // use a switch case so if new states get added they will have to update this
    switch (lexUsb3Ulp.ulpFsmState)
    {
        case LEX_ULP_USB3_FAILED:               // USB3 failed to setup on either the Rex or Lex
            status = USB3_ERROR;
            break;

        case LEX_ULP_USB3_ACTIVE:               // USB3 active on Lex and Rex
        case LEX_ULP_USB3_REX_SETUP:            // Waiting for the Rex to indicate it has setup USB3
        case LEX_ULP_USB3_LEX_SETUP:            // Waiting for the Lex to indicate it has setup USB3
        case LEX_ULP_USB3_REX_HOT_RESET_WAIT:   // waiting for Hot Reset to finish on the Rex
        case LEX_ULP_USB3_REX_WARM_RESET_WAIT:  // waiting for warm reset to finish on the Rex
        case LEX_ULP_USB3_LEX_ONLY_RESET:    // Lex USB3 is in a Lex only reset state
            status = USB3_U0;
            break;

        case LEX_ULP_USB3_STANDBY:              // Lex is in the standby state
        case LEX_ULP_USB3_STANDBY_HOST_EXIT:    // Lex USB3 is exiting standby (Host initiated exit)
            status = USB3_U3;
            break;

        case LEX_ULP_USB3_STATE_DISABLED:       // USB3 state machine disabled
        case LEX_ULP_USB3_STATE_READY:       // USB3 port state machine is ready to connect
        case LEX_ULP_USB3_RX_DETECT:            // See if we can detect Rx terminations on the far side
        default:
            status = USB3_IN_RESET;             // USB3 is not active
            break;
    }

    return ((uint32_t)status);
}

//#################################################################################################
// Called when we've timed out before receiving a Rx Termination detect interrupt
// Also sets the minimum time when we bring USB3 down before it comes up again.  Shouldn't be
// less then 50ms, because the Rex uses this time to remove and bring back it's Rx terminations
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexUsb3SnoopTimeout(void)
{
    ilog_ULP_COMPONENT_2(ILOG_MINOR_EVENT, ULP_LEX_HOST_SNOOP_TIMEOUT, UlpHalRxTerminationsPresent(), UlpHalGetLTSSM());

    // if we haven't got an interrupt by now, trigger an event so we can see what state the
    // terminations are in
    ULP_LexUsb3StateMachineSendEvent(LEX_ULP_USB3_EVENT_LEX_RX_TERM_CHANGE);
}


//#################################################################################################
// Called after a timeout period has expired, so the warm reset count can be cleared
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexUsb3WarmResetCountClear(void)
{
    ilog_ULP_COMPONENT_1(ILOG_MINOR_EVENT, ULP_LEX_USB3_WARM_RESET_COUNT_CLEARED, lexUsb3Ulp.warmResetCount);

    lexUsb3Ulp.warmResetCount = 0;
}

//#################################################################################################
// Called after a timeout period has expired, so the inactive count can be cleared
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexUsb3InactiveCountClear(void)
{
    ilog_ULP_COMPONENT_1(ILOG_MINOR_EVENT, ULP_LEX_USB3_WARM_RESET_COUNT_CLEARED, lexUsb3Ulp.inactiveCount);

    lexUsb3Ulp.inactiveCount = 0;
}

//#################################################################################################
// Called if we are stuck in a inactive state.  If we are, go to recovery
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexUsb3InactiveTimeout(void)
{
    ilog_ULP_COMPONENT_1(ILOG_MINOR_EVENT, ULP_LEX_USB3_LEX_STUCK_AT_INACTIVE, lexUsb3Ulp.failCount);

    LexUsb3FailureRecovery();
}

//#################################################################################################
// Sends an event to the Lex USB3 state machine depending on the detected state of the far end
// terminations
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexUsb3FailureRecovery(void)
{
    if (lexUsb3Ulp.failCount < lexUsb3Ulp.lexUsb3MaxFailCount)
    {
        lexUsb3Ulp.failCount++;
        lexUsb3Ulp.LexOnlyResetTriggered = false;
        ULP_UlpLexUsb3LexOnlyResetStop();   // stop a lex only reset
        ULP_LexHostUsb3RestartRequest();    // try a USB3 restart
        ilog_ULP_COMPONENT_1(ILOG_MINOR_EVENT, ULP_LEX_USB3_FAILURE_RECOVERY, lexUsb3Ulp.failCount);
    }

    LexUSB3SetFailedState();        // go to the failure state
}
