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

#include <uart.h>

// Constants and Macros ###########################################################################


// Data Types #####################################################################################
enum LexUlpUsb2State
{
    LEX_ULP_USB2_STATE_DISABLED,    // USB2 port state machine disabled
    LEX_ULP_USB2_STATE_READY,       // USB2 port state machine is ready to connect
    LEX_ULP_USB2_ACTIVE,            // USB2 active on Lex and Rex
    LEX_ULP_USB2_STATE_FAILED,      // USB2 failure
};

// TODO add a master timeout event, so if we get stuck we will go back to the idle state
enum LexUlpUsb2Event
{
    LEX_ULP_USB2_EVENT_DISABLED,        // disable USB2
    LEX_ULP_USB2_EVENT_ENABLED,         // enable USB2

    LEX_ULP_USB2_EVENT_HOST_CONNECT,    // connect to a host
    LEX_ULP_USB2_EVENT_HOST_DISCONNECT, // disconnect from the host

    LEX_ULP_USB2_EVENT_REX_FAILED,      // Rex USB 2 has failed
    LEX_ULP_USB2_EVENT_REX_DISABLED,    // Rex USB 2 is disabled

    LEX_ULP_USB2_EVENT_LEX_FAILED,      // Lex USB 2 has failed
};

// Static Function Declarations ###################################################################
static void ULP_LexUsb2StateMachineSendEvent(enum LexUlpUsb2Event event)    __attribute__((section(".lexatext")));
static void ULP_LexUsb2StateCallback(void *param1, void *param2)            __attribute__((section(".lexatext")));

static void ULP_LexUsb2StateMachine(enum LexUlpUsb2Event lexHostEvent)      __attribute__((section(".lexatext")));
static void UlpLexUsb2StateDisabled(enum LexUlpUsb2Event event)             __attribute__((section(".lexatext")));
static void UlpLexUsb2StateReady(enum LexUlpUsb2Event event)                __attribute__((section(".lexatext")));
static void UlpLexUsb2StateActive(enum LexUlpUsb2Event event)               __attribute__((section(".lexatext")));
static void UlpLexUsb2StateFailed(enum LexUlpUsb2Event event)               __attribute__((section(".lexatext")));

static void UlpLexUsb2Disconnected(void)                                    __attribute__((section(".lexatext")));
static void UlpLexUsb2Disable(void)                                         __attribute__((section(".lexatext")));
static void UlpLexUsb2FailHandler(void)                                     __attribute__((section(".lexatext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct LexUlpUsb2Context
{
    // Lex host port state
    // Lex ULP FSM state.
    enum LexUlpUsb2State ulpFsmState;   // the state this USB controller is in

    uint8_t controlFlags;               // flags controlling whether this USB controller is enabled/disabled

} lexUsb2Ulp;

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Initialize the LEX ULP module and the ULP finite state machine.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void ULP_LexUsb2Init(bool enabled)
{
    lexUsb2Ulp.ulpFsmState = LEX_ULP_USB2_STATE_DISABLED;

    if (enabled)
    {
        ULP_LexUsb2Control(ULP_USB_CONTROL_ENABLE);
    }
}


//#################################################################################################
// Sends control operations to the USB2 module.  If the module is enabled, and connected,
// it will run.  Otherwise it will stop
//
// Parameters:
// Return:
// Assumptions:
//      - USB2 can only be enabled or disabled.  To enable it, a number of conditions have to be
//        met.  If any of these conditions are not met, the system should be disabled
//#################################################################################################
bool ULP_LexUsb2Control(enum UlpUsbControl controlOperation)
{
    enum UlpUsbControlResult controlResult = ULP_UsbSetControl(&lexUsb2Ulp.controlFlags, controlOperation);

    switch (controlResult)
    {
        case ULP_USB_CONTROL_RESULT_ENABLE:          // this USB module can be enabled
            ULP_LexUsb2StateMachineSendEvent(LEX_ULP_USB2_EVENT_ENABLED);
            break;

        case ULP_USB_CONTROL_RESULT_DISABLE:         // this USB module can be disabled
        default:
            ULP_LexUsb2StateMachineSendEvent(LEX_ULP_USB2_EVENT_DISABLED);
            break;

        case ULP_USB_CONTROL_RESULT_UNCHANGED:       // no change since the last time this module was called
            break;
    }

    ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_LEX_USB2_SET_CONTROL_RESULT, lexUsb2Ulp.controlFlags, controlOperation, controlResult);

    return (ULP_UsbControlEnabled(lexUsb2Ulp.controlFlags));
}

//#################################################################################################
// Returns true if USB2 is enabled
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool ULP_LexUsb2LocalEnabled(void)
{
    if (lexUsb2Ulp.controlFlags & ULP_USB_MODULE_LOCAL_ENABLED)
    {
        return (true);  // USB2 is enabled
    }

    return (false);
}


//#################################################################################################
// Returns true if USB2 is enabled on the entire system
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool ULP_LexUsb2SystemEnabled(void)
{
    if ((lexUsb2Ulp.controlFlags & ULP_USB_MODULE_SYSTEM_ENABLED) == ULP_USB_MODULE_SYSTEM_ENABLED)
    {
        return (true);  // USB2 is enabled
    }

    return (false);
}


//#################################################################################################
// handles the USB3 messages CPU messages from the Rex
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_LexUsb2RxCpuMessageHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    iassert_ULP_COMPONENT_1(msgLength == 0, ULP_LEX_UNEXPECTED_CPU_MSG_LENGTH, msgLength);
    ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_LEX_RCV_REX_USB2_MSG, subType);

    enum UlpRexSendUsb2Message rexUsb2Message = (enum UlpRexSendUsb2Message)subType;

    if (ULP_LexUsb2LocalEnabled())
    {
        switch (rexUsb2Message)
        {
            case ULP_REX_TO_LEX_USB2_MODULE_DISABLED:       // Rex to Lex - Rex USB2 module is disabled
                ULP_LexUsb2Control(ULP_USB_CONTROL_PARTNER_SHUTDOWN);  // partner is disabled!
                UlpGeControl(false);  // make sure GE is in reset
                break;

            case ULP_REX_TO_LEX_USB2_MODULE_ENABLED:        // Rex to Lex - Rex USB2 module is enabled
                ULP_LexUsb2Control(ULP_USB_CONTROL_PARTNER_ENABLED);  // partner is enabled!
                ULP_LexUsb2Control(ULP_USB_CONTROL_PARTNER_READY);      // Rex USB2 is ready!
                UlpLexRexUsbAllowed();
                UlpGeControl(true);  // start bringing up GE
                break;

            case ULP_REX_TO_LEX_USB2_FAILED:             // Rex to Lex - Rex USB2 has failed
                ULP_LexUsb2StateMachineSendEvent(LEX_ULP_USB2_EVENT_REX_FAILED);
                break;

            case ULP_REX_TO_LEX_USB2_DISABLED:       // Rex to Lex - Rex USB2 is currently disabled
                ULP_LexUsb2StateMachineSendEvent(LEX_ULP_USB2_EVENT_REX_DISABLED);
                break;

            default:
                break;
        }
    }
    else if ( (rexUsb2Message != ULP_REX_TO_LEX_USB2_MODULE_DISABLED) && (rexUsb2Message != ULP_REX_TO_LEX_USB2_DISABLED))
    {
        UlpSendCPUCommLexUsb2Message(ULP_LEX_TO_REX_USB2_MODULE_DISABLED);  // tell Rex that USB2 is disabled on the Lex
    }
}

//#################################################################################################
// Returns the current USB2 state machine status
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
enum UlpUsbModuleStatus UlpLexUsb2Status(void)
{
    enum UlpUsbModuleStatus result = ULP_USB_MODULE_DISABLED;

    switch (lexUsb2Ulp.ulpFsmState)
    {
        case LEX_ULP_USB2_STATE_READY:       // USB2 port state machine is ready to connect
            result = ULP_USB_MODULE_READY;
            break;

        case LEX_ULP_USB2_STATE_FAILED:      // USB2 failure
            result = ULP_USB_MODULE_DISCONNECTED;
            break;

        case LEX_ULP_USB2_ACTIVE:            // USB2 active on Lex and Rex
            result = ULP_USB_MODULE_CONNECTED;
            break;

        case LEX_ULP_USB2_STATE_DISABLED:    // USB2 port state machine disabled
        default:
            if ( ULP_LexUsb2SystemEnabled() )
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
void UlpLexUsb2HostConnect(bool connect)
{
    // send a connect or disconnect event
    ULP_LexUsb2StateMachineSendEvent(
        connect ? LEX_ULP_USB2_EVENT_HOST_CONNECT : LEX_ULP_USB2_EVENT_HOST_DISCONNECT);
}


//#################################################################################################
// Sends the state machine a failed event, when something is wrong with GE
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpLexUsb2Failed(void)
{
    // send a Lex failed event
    ULP_LexUsb2StateMachineSendEvent( LEX_ULP_USB2_EVENT_LEX_FAILED );
}

// Static Function Definitions ####################################################################

//#################################################################################################
// Entry point to send an event to the USB2 state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_LexUsb2StateMachineSendEvent(enum LexUlpUsb2Event event)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = event;

    CALLBACK_Run(ULP_LexUsb2StateCallback, (void *)eventx, NULL);
}

//#################################################################################################
// Callback wrapper for the USB2 state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_LexUsb2StateCallback(void *param1, void *param2)
{
    ULP_LexUsb2StateMachine( (enum LexUlpUsb2Event)param1 );
}


//#################################################################################################
// The entry point for the USB2 state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_LexUsb2StateMachine(enum LexUlpUsb2Event lexUsb2Event)
{
    uint8_t oldState = lexUsb2Ulp.ulpFsmState;

    switch (lexUsb2Ulp.ulpFsmState)
    {
        case LEX_ULP_USB2_STATE_DISABLED:    // USB2 state machine disabled
            UlpLexUsb2StateDisabled(lexUsb2Event);
            break;

        case LEX_ULP_USB2_STATE_READY:       // USB2 port state machine is ready to connect
            UlpLexUsb2StateReady(lexUsb2Event);
             break;

        case LEX_ULP_USB2_ACTIVE:            // USB2 active on Lex and Rex
            UlpLexUsb2StateActive(lexUsb2Event);
            break;

        case LEX_ULP_USB2_STATE_FAILED:      // USB2 failure
            UlpLexUsb2StateFailed(lexUsb2Event);
             break;

        default:
            break;
    }

    ilog_ULP_COMPONENT_3(ILOG_MINOR_EVENT, ULP_LEX_USB2_STATE_MSG, oldState, lexUsb2Event, lexUsb2Ulp.ulpFsmState);
}


//#################################################################################################
// Handles events in the LEX_ULP_USB2_STATE_DISABLED
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb2StateDisabled(enum LexUlpUsb2Event event)
{
    switch (event)
    {
        case LEX_ULP_USB2_EVENT_ENABLED:     // enable USB2
            lexUsb2Ulp.ulpFsmState = LEX_ULP_USB2_STATE_READY;  // USB2 port state machine is ready to connect
            UlpLexHostSendUsb2Status(ULP_USB_MODULE_READY);     // tell the host port we are ready
            break;

        // these events don't mean anything in this state
        case LEX_ULP_USB2_EVENT_HOST_CONNECT:    // connect to a host
        case LEX_ULP_USB2_EVENT_HOST_DISCONNECT: // disconnect from the host
        case LEX_ULP_USB2_EVENT_REX_FAILED:      // Rex USB 2 has failed
        case LEX_ULP_USB2_EVENT_LEX_FAILED:      // Lex USB 2 has failed
        case LEX_ULP_USB2_EVENT_DISABLED:       // disable USB2
        case LEX_ULP_USB2_EVENT_REX_DISABLED:   // Rex USB 2 is disabled
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_LEX_USB2_UNEXPECTED_EVENT,
                event,
                lexUsb2Ulp.ulpFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_USB2_STATE_READY - GE module is out of reset, and is waiting for
// a host connect event
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb2StateReady(enum LexUlpUsb2Event event)
{
    switch (event)
    {
        case LEX_ULP_USB2_EVENT_DISABLED:       // disable USB2
            UlpLexUsb2Disable();                // go to the disabled state
            break;

        case LEX_ULP_USB2_EVENT_HOST_CONNECT:    // connect to a host
            _ULP_halControlGeVbus(true);                            // activate the Lex GE Vbus
            lexUsb2Ulp.ulpFsmState = LEX_ULP_USB2_ACTIVE;           // USB2 is active!
            UlpLexHostSendUsb2Status(ULP_USB_MODULE_CONNECTED);     // tell the host port we are connected
            UlpSendCPUCommLexUsb2Message(ULP_LEX_TO_REX_USB2_CONNECTED);  // tell the Rex we are connected
            break;

        // these events don't mean anything in this state
        case LEX_ULP_USB2_EVENT_REX_FAILED:      // Rex USB 2 has failed
        case LEX_ULP_USB2_EVENT_LEX_FAILED:      // Lex USB 2 has failed
        case LEX_ULP_USB2_EVENT_HOST_DISCONNECT: // disconnect from the host
        case LEX_ULP_USB2_EVENT_ENABLED:     // enable USB2
        case LEX_ULP_USB2_EVENT_REX_DISABLED:   // Rex USB 2 is disabled
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_LEX_USB2_UNEXPECTED_EVENT,
                event,
                lexUsb2Ulp.ulpFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_USB2_REX_SETUP
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb2StateActive(enum LexUlpUsb2Event event)
{
    switch (event)
    {
        case LEX_ULP_USB2_EVENT_DISABLED:       // disable USB2
        case LEX_ULP_USB2_EVENT_REX_DISABLED:   // Rex USB 2 is disabled
            UlpLexUsb2Disable();                // go to the disabled state
            break;

        case LEX_ULP_USB2_EVENT_HOST_DISCONNECT: // disconnect from the host
            UlpLexUsb2Disconnected();
            break;

        case LEX_ULP_USB2_EVENT_REX_FAILED:     // Rex USB 2 has failed
            UlpLexUsb2FailHandler();
            break;

        case LEX_ULP_USB2_EVENT_LEX_FAILED:     // Lex USB 2 has failed
            UlpLexUsb2FailHandler();
            UlpSendCPUCommLexUsb2Message(ULP_LEX_TO_REX_USB2_FAILED);  // tell Rex USB2 failed on the Lex
            break;

           // these events don't mean anything in this state
        case LEX_ULP_USB2_EVENT_HOST_CONNECT:   // connected to a host
        case LEX_ULP_USB2_EVENT_ENABLED:        // enable USB2
        default:
            ilog_ULP_COMPONENT_2(ILOG_MINOR_ERROR, ULP_LEX_USB2_UNEXPECTED_EVENT, event, lexUsb2Ulp.ulpFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_USB2_STATE_FAILED
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb2StateFailed(enum LexUlpUsb2Event event)
{
    switch (event)
    {
        case LEX_ULP_USB2_EVENT_DISABLED:       // disable USB2
            UlpLexUsb2Disable();                // go to the disabled state
            break;

        case LEX_ULP_USB2_EVENT_HOST_DISCONNECT: // disconnect from the host
            // cycle GE to bring it out of the failure case
            UlpGeControl(false);
            UlpGeControl(true);

            // go to the disabled state, and wait for the GE reset to finish
            lexUsb2Ulp.ulpFsmState = LEX_ULP_USB2_STATE_DISABLED;
            break;

        // these events don't mean anything in this state
        case LEX_ULP_USB2_EVENT_ENABLED:     // enable USB2
        case LEX_ULP_USB2_EVENT_HOST_CONNECT:   // connect to a host
        case LEX_ULP_USB2_EVENT_REX_FAILED:     // Rex USB 2 has failed
        case LEX_ULP_USB2_EVENT_LEX_FAILED:     // Lex USB 2 has failed
        case LEX_ULP_USB2_EVENT_REX_DISABLED:   // Rex USB 2 is disabled - we want to stay in the failure state
        default:
            ilog_ULP_COMPONENT_2(
                ILOG_MINOR_ERROR,
                ULP_LEX_USB2_UNEXPECTED_EVENT,
                event,
                lexUsb2Ulp.ulpFsmState);
            break;
    }
}


//#################################################################################################
// Disconnects USB2 on the Lex
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb2Disconnected(void)
{
    _ULP_halControlGeVbus(false);                           // turn the Lex GE Vbus off
    UlpSendCPUCommLexUsb2Message(ULP_LEX_TO_REX_USB2_DISCONNECTED);  // tell Rex USB2 is disconnected
    lexUsb2Ulp.ulpFsmState = LEX_ULP_USB2_STATE_READY;   // go to the ready, disconnected state
    UlpLexHostSendUsb2Status(ULP_USB_MODULE_READY);     // tell the host port we are ready
}

//#################################################################################################
// Goes to the USB2 failure state on the Lex
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb2FailHandler(void)
{
    lexUsb2Ulp.ulpFsmState = LEX_ULP_USB2_STATE_FAILED;     // go to the failed state
    _ULP_halControlGeVbus(false);                           // turn the Lex GE Vbus off
    UlpLexHostSendUsb2Status(ULP_USB_MODULE_DISCONNECTED);  // tell the host port we are disconnected
}

//#################################################################################################
// Disables USB2 on the Lex
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexUsb2Disable(void)
{
    lexUsb2Ulp.ulpFsmState = LEX_ULP_USB2_STATE_DISABLED;   // go to the disabled state
    _ULP_halControlGeVbus(false);                           // turn the Lex GE Vbus off
    UlpSendCPUCommLexUsb2Message(ULP_LEX_TO_REX_USB2_DISABLED);  // tell Rex to turn off its USB
}



