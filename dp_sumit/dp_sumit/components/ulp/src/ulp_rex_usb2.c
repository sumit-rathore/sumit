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
#include <bb_ge_comm.h>

#include <uart.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
enum RexUlpUsb2State
{
    REX_ULP_USB2_STATE_DISABLED,    // USB2 port state machine disabled
    REX_ULP_USB2_STATE_ACTIVE,      // USB2 active on Lex and Rex
    REX_ULP_USB2_STATE_FAILED,      // USB2 failed on the Rex (or Lex)
};

// TODO add a master timeout event, so if we get stuck we will go back to the idle state
enum RexUlpUsb2Event
{
    REX_ULP_USB2_EVENT_DISABLED,    // disable USB2
    REX_ULP_USB2_EVENT_ENABLED,     // enable USB2

    REX_ULP_USB2_EVENT_LEX_FAILED,  // USB2 failed on the Lex
    REX_ULP_USB2_EVENT_REX_FAILED,  // USB2 failed on the Rex
};

// Static Function Declarations ###################################################################
static void ULP_RexUsb2StateMachineSendEvent(enum RexUlpUsb2Event event)    __attribute__((section(".rexatext")));
static void ULP_RexUsb2StateCallback(void *param1, void *param2)            __attribute__((section(".rexatext")));

static void ULP_RexUsb2StateMachine(enum RexUlpUsb2Event lexHostEvent)      __attribute__((section(".rexatext")));

static void UlpRexUsb2StateDisabled(enum RexUlpUsb2Event event)             __attribute__((section(".rexatext")));
static void UlpRexUsb2StateActive(enum RexUlpUsb2Event event)               __attribute__((section(".rexatext")));
static void UlpRexUsb2StateFailed(enum RexUlpUsb2Event event)               __attribute__((section(".rexatext")));

static void UlpRexUsb2SetFailedState(void)                                  __attribute__((section(".rexatext")));
static void UlpRexUsb2SetDisabledState(void)                                __attribute__((section(".rexatext")));


// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct RexUlpUsb2Context
{
    // Lex host port state
    // Lex ULP FSM state.
    enum RexUlpUsb2State ulpFsmState;   // the state this USB controller is in

    uint8_t controlFlags;               // flags controlling whether this USB controller is enabled/disabled

} rexUsb2Ulp;

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
void ULP_RexUsb2Init(bool enabled)
{
    rexUsb2Ulp.ulpFsmState = REX_ULP_USB2_STATE_DISABLED;

    if (enabled)
    {
        ULP_RexUsb2Control(ULP_USB_CONTROL_ENABLE);
    }
}


//#################################################################################################
// Sends control operations to the USB2 module.  If the module is enabled, and connected,
// it will run.  Otherwise it will stop
//
// Parameters:
// Return:
// Assumptions:
//      * Always call ULP_RexUsb3Control() before ULP_RexUsb2Control() - Usb3 needs to be
//        enabled first! (device tries USB3 first, and then if that fails, tries USB2)
//#################################################################################################
bool ULP_RexUsb2Control(enum UlpUsbControl controlOperation)
{
    enum UlpUsbControlResult controlResult = ULP_UsbSetControl(&rexUsb2Ulp.controlFlags, controlOperation);

    switch (controlResult)
    {
        case ULP_USB_CONTROL_RESULT_ENABLE:          // this USB module can be enabled
            ULP_RexUsb2StateMachineSendEvent(REX_ULP_USB2_EVENT_ENABLED);
            break;

        case ULP_USB_CONTROL_RESULT_DISABLE:         // this USB module can be disabled
        default:
            ULP_RexUsb2StateMachineSendEvent(REX_ULP_USB2_EVENT_DISABLED);
            break;

        case ULP_USB_CONTROL_RESULT_UNCHANGED:       // no change since the last time this module was called
            break;
    }

    ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_REX_USB2_SET_CONTROL_RESULT, rexUsb2Ulp.controlFlags, controlOperation, controlResult);

    return (ULP_UsbControlEnabled(rexUsb2Ulp.controlFlags));
}


//#################################################################################################
// returns true if USB2 on the Rex is active.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool RexUlpUsb2IsActive(void)
{
    bool active = false;    // ASSUME USB2 is inactive

    switch (rexUsb2Ulp.ulpFsmState)
    {
        case REX_ULP_USB2_STATE_ACTIVE:     // USB2 active on Lex and Rex
            active = true;  // USB2 is active
            break;

        case REX_ULP_USB2_STATE_FAILED:     // USB2 failed on the Rex (or Lex)
        case REX_ULP_USB2_STATE_DISABLED:   // USB2 state machine disabled
        default:
            break;
    }

    return (active);
}


//#################################################################################################
// handles the USB2 messages CPU messages from the Lex
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_RexUsb2RxCpuMessageHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    iassert_ULP_COMPONENT_1(msgLength == 0, ULP_REX_UNEXPECTED_CPU_MSG_LENGTH, msgLength);
    ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_REX_RCV_LEX_USB2_MSG, subType);

    enum UlpLexSendUsb2Message lexUsbMessage = (enum UlpLexSendUsb2Message)subType;

    // only process this message if the Rex USB2 is enabled!
    if (ULP_RexUsb2Enabled() )
    {
        switch (lexUsbMessage)
        {
            case ULP_LEX_TO_REX_USB2_MODULE_DISABLED:                   // USB2 on the /Lex is disabled
                ULP_RexUsb2Control(ULP_USB_CONTROL_PARTNER_SHUTDOWN);   // Lex USB2 is disabled!
                break;

            case ULP_LEX_TO_REX_USB2_MODULE_ENABLED:                    // USB2 on the Lex is enabled
                ULP_RexUsb2Control(ULP_USB_CONTROL_PARTNER_ENABLED);    // Lex USB2 is enabled!
                break;

            case ULP_LEX_TO_REX_USB2_CONNECTED:                         // Lex has detected a USB 2 Host port
                ULP_RexUsb2Control(ULP_USB_CONTROL_PARTNER_READY);      // Lex USB2 is ready!
                break;

            case ULP_LEX_TO_REX_USB2_DISCONNECTED:       // Host port is disconnected from the Lex
            case ULP_LEX_TO_REX_USB2_DISABLED:                          // USB 2 disabled on the Lex
                ULP_RexUsb2Control(ULP_USB_CONTROL_PARTNER_DISABLED);   // Lex USB2 is disconnected
                break;

            case ULP_LEX_TO_REX_USB2_FAILED:             // USB 2 state machine has failed on the Lex
                ULP_RexUsb2StateMachineSendEvent(REX_ULP_USB2_EVENT_LEX_FAILED);
                break;

            default:
                break;
        }
    }
    else if ( (lexUsbMessage != ULP_LEX_TO_REX_USB2_MODULE_DISABLED) && (lexUsbMessage != ULP_LEX_TO_REX_USB2_DISABLED) )
    {
        UlpSendCPUCommRexUsb2Message( ULP_REX_TO_LEX_USB2_MODULE_DISABLED);
        return;
    }
}


//#################################################################################################
// Sends the state machine a failed event, when something is wrong with GE
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpRexUsb2Failed(void)
{
    // send a Lex failed event
    ULP_RexUsb2StateMachineSendEvent( REX_ULP_USB2_EVENT_REX_FAILED );
}


//#################################################################################################
// Returns true if USB2 is enabled
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool ULP_RexUsb2Enabled(void)
{
    if (rexUsb2Ulp.controlFlags & ULP_USB_MODULE_LOCAL_ENABLED)
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
bool ULP_RexUsb2SystemEnabled(void)
{
    if ((rexUsb2Ulp.controlFlags & ULP_USB_MODULE_SYSTEM_ENABLED) == ULP_USB_MODULE_SYSTEM_ENABLED)
    {
        return (true);  // USB2 is enabled
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
static void ULP_RexUsb2StateMachineSendEvent(enum RexUlpUsb2Event event)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = event;

    CALLBACK_Run(ULP_RexUsb2StateCallback, (void *)eventx, NULL);
}

//#################################################################################################
// Callback wrapper for the Host port state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_RexUsb2StateCallback(void *param1, void *param2)
{
    ULP_RexUsb2StateMachine( (enum RexUlpUsb2Event)param1 );
}


//#################################################################################################
// The entry point for the Host port state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_RexUsb2StateMachine(enum RexUlpUsb2Event rexUsb2Event)
{
    uint8_t oldState = rexUsb2Ulp.ulpFsmState;

    switch (rexUsb2Ulp.ulpFsmState)
    {
        case REX_ULP_USB2_STATE_DISABLED:    // USB2 state machine disabled
            UlpRexUsb2StateDisabled(rexUsb2Event);
            break;

       case REX_ULP_USB2_STATE_ACTIVE:            // USB2 active on Lex and Rex
            UlpRexUsb2StateActive(rexUsb2Event);
            break;

       case REX_ULP_USB2_STATE_FAILED:     // USB2 failed on the Rex (or Lex)
           UlpRexUsb2StateFailed(rexUsb2Event);
           break;

        default:
            break;
    }

    ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_REX_USB2_STATE_MSG, oldState, rexUsb2Event, rexUsb2Ulp.ulpFsmState);
}


//#################################################################################################
// Handles events in the REX_ULP_USB2_STATE_DISABLED
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpRexUsb2StateDisabled(enum RexUlpUsb2Event event)
{
    switch (event)
    {
        case REX_ULP_USB2_EVENT_ENABLED:     // enable USB2
            rexUsb2Ulp.ulpFsmState = REX_ULP_USB2_STATE_ACTIVE;   // go to the active state
            UlpVbusControl(true);       // make sure vBus is on
            break;

        case REX_ULP_USB2_EVENT_LEX_FAILED:  // USB2 failed on the Lex
            UlpSendCPUCommRexUsb2Message(ULP_REX_TO_LEX_USB2_DISABLED);
            break;


       // these events don't mean anything in this state
        case REX_ULP_USB2_EVENT_REX_FAILED:  // USB2 failed on the Rex
        case REX_ULP_USB2_EVENT_DISABLED:    // disable USB2
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the REX_ULP_USB2_LEX_SETUP
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpRexUsb2StateActive(enum RexUlpUsb2Event event)
{
    switch (event)
    {
        case REX_ULP_USB2_EVENT_DISABLED:    // disable USB2
            UlpRexUsb2SetDisabledState();
            break;

        case REX_ULP_USB2_EVENT_REX_FAILED:  // USB2 failed on the Rex
            UlpSendCPUCommRexUsb2Message(ULP_REX_TO_LEX_USB2_FAILED);
            UlpRexUsb2SetFailedState();
            break;

        case REX_ULP_USB2_EVENT_LEX_FAILED:  // USB2 failed on the Rex
            UlpRexUsb2SetFailedState();
            break;

        // these events don't mean anything in this state
        case REX_ULP_USB2_EVENT_ENABLED:     // enable USB2
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the REX_ULP_USB2_LEX_SETUP
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpRexUsb2StateFailed(enum RexUlpUsb2Event event)
{
    switch (event)
    {
        case REX_ULP_USB2_EVENT_DISABLED:    // disable USB2
            UlpRexUsb2SetDisabledState();
            break;

            // these events don't mean anything in this state
        case REX_ULP_USB2_EVENT_LEX_FAILED:  // USB2 failed on the Lex
        case REX_ULP_USB2_EVENT_REX_FAILED:  // USB2 failed on the Rex
        case REX_ULP_USB2_EVENT_ENABLED:     // enable USB2
        default:
            break;
    }
}

//#################################################################################################
// Tear down USB3 on the Lex - disable it
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpRexUsb2SetFailedState(void)
{
    rexUsb2Ulp.ulpFsmState = REX_ULP_USB2_STATE_FAILED;      // USB2 failed on the Rex (or Lex)
    UlpVbusControl(false);
}


//#################################################################################################
// Tear down USB3 on the Lex - disable it
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpRexUsb2SetDisabledState(void)
{
    rexUsb2Ulp.ulpFsmState = REX_ULP_USB2_STATE_DISABLED;   // go to the disabled state
    UlpVbusControl(false);
    UlpSendCPUCommRexUsb2Message(ULP_REX_TO_LEX_USB2_DISABLED);
}


