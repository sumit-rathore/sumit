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
#include <mca.h>
#include "ulp_loc.h"

#include <ulp_core_regs.h>

#include <uart.h>

// Constants and Macros ###########################################################################

// Vbus debounce interval
#define LEX_VBUS_DEBOUNCE_INTERVAL          (20)    // interval between debounce checks, in ms

// number of consecutive times to check before we consider it stable
// see section 4.1.3, 4.2.1, and section 5, timing of the BC1.2 final spec
// it gives TVBUS_REAPP of at least 100ms.
#define LEX_DEBOUNCE_OK_COUNT               (25)

#define LEX_ULP_HOST_SETUP_USB2_ACKED       0x01        // Lex USB 2 has acknowledged
#define LEX_ULP_HOST_SETUP_USB3_ACKED       0x01        // Lex USB 3 has acknowledged

#define LEX_ULP_HOST_USB_READY              (LEX_ULP_HOST_SETUP_USB2_ACKED | LEX_ULP_HOST_SETUP_USB3_ACKED)

#define LEX_ULP_HOST_USB3_EVENT_OFFSET      4       // offset of USB3 events from USB2 - see UlpLexHostSendUsb3Status()

// Data Types #####################################################################################
enum LexUlpHostState
{
    LEX_ULP_HOST_STATE_DISABLED,            // host port state machine disabled

    LEX_ULP_HOST_STATE_WAIT_USB,            // enabled, waiting for the USB state machines to be ready
    LEX_ULP_HOST_STATE_HOST_NOT_CONNECTED,  // enabled, not connected to a host port
    LEX_ULP_HOST_STATE_SNOOP_HOST_PORT,     // connected, snooping host to see what interface it supports

    LEX_ULP_HOST_STATE_HOST_USB2,           // Lex is connected to a USB2 port (no USB3!)
    LEX_ULP_HOST_STATE_HOST_USB3,           // Lex is connected to a USB3 port (no USB2!)
    LEX_ULP_HOST_STATE_HOST_USB32,          // Lex is connected to a USB3 and USB2 port
};

// TODO add a master timeout event, so if we get stuck we will go back to the idle state
enum LexUlpHostEvent
{
    LEX_ULP_HOST_EVENT_DISABLED,            // Host port state machine disabled
    LEX_ULP_HOST_EVENT_ENABLED,             // Host port state machine enabled

    LEX_ULP_HOST_EVENT_HOST_CONNECTED,      // Host port connected to Lex
    LEX_ULP_HOST_EVENT_HOST_REMOVED,        // Host port disconnected from Lex

    LEX_ULP_HOST_EVENT_SNOOP_USB3,          // Host port snoop complete - we are connected to USB3
    LEX_ULP_HOST_EVENT_USB3_NO_RX_TERMINATIONS,    // USB3 Rx terminations removed

    LEX_ULP_HOST_EVENT_USB2_READY,          // USB2 module ready to go
    LEX_ULP_HOST_EVENT_USB2_CONNECTED,      // USB2 module connected, running
    LEX_ULP_HOST_EVENT_USB2_DISCONNECTED,   // USB2 module disconnected, stopped
    LEX_ULP_HOST_EVENT_USB2_DISABLED,       // USB2 module is inactive

    // order is important here - the 4 USB3 events must come after the USB2 events. See UlpLexHostSendUsb3Status()
    LEX_ULP_HOST_EVENT_USB3_READY,          // USB3 module ready to go
    LEX_ULP_HOST_EVENT_USB3_CONNECTED,      // USB3 module connected, running
    LEX_ULP_HOST_EVENT_USB3_DISCONNECTED,   // USB3 module disconnected, stopped
    LEX_ULP_HOST_EVENT_USB3_DISABLED,       // USB3 module is inactive

    LEX_ULP_HOST_EVENT_HOST_SOFT_CYCLE,     // Host port software restart requested
    LEX_ULP_HOST_EVENT_USB3_RESTART,        // USB3 restart requested - only valid when connected to USB3
    LEX_ULP_HOST_EVENT_USB3_RESET_DONE,     // USB3 reset done - only valid when in the USB2 connected state
};


// Static Function Declarations ###################################################################
static void LexDebounceVbus(void)                                           __attribute__((section(".lexatext")));

static void ULP_LexHostStateMachineSendEvent(enum LexUlpHostEvent event)    __attribute__((section(".lexatext")));
static void ULP_LexHostStateCallback(void *param1, void *param2)            __attribute__((section(".lexatext")));

static void ULP_LexHostStateMachine(enum LexUlpHostEvent lexHostEvent)      __attribute__((section(".lexatext")));

static void UlpLexHostStateDisabled(enum LexUlpHostEvent event)             __attribute__((section(".lexatext")));
static void UlpLexHostStateWaitUsb(enum LexUlpHostEvent event)              __attribute__((section(".lexatext")));
static void UlpLexHostStateNoHostPort(enum LexUlpHostEvent event)           __attribute__((section(".lexatext")));
static void UlpLexHostStateSnoop(enum LexUlpHostEvent event)                __attribute__((section(".lexatext")));
static void UlpLexHostStateUsb2OnlyConnected(enum LexUlpHostEvent event)    __attribute__((section(".lexatext")));
static void UlpLexHostStateUsb3OnlyConnected(enum LexUlpHostEvent event)    __attribute__((section(".lexatext")));
static void UlpLexHostStateUsb321Connected(enum LexUlpHostEvent event)      __attribute__((section(".lexatext")));

static void LexVbusDebounceControl(bool enable)                             __attribute__((section(".lexatext")));
static void UlpLexHostCheckUsbStatus(enum UlpUsbModuleStatus usbStatus)     __attribute__((section(".lexatext")));
static enum LexUlpHostEvent UlpLexHostConvertUsbStatus(enum UlpUsbModuleStatus usbStatus)   __attribute__((section(".lexatext")));
static void UlpLexHostUsb2Selected(void)                                    __attribute__((section(".lexatext")));
static void LexUSB3Restarted(void)                                          __attribute__((section(".lexatext")));
static void LexHostSoftCycle(void)                                          __attribute__((section(".lexatext")));
static void LexHostRemoved(void)                                            __attribute__((section(".lexatext")));
static void LexHostDisabled(void)                                           __attribute__((section(".lexatext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct LexUlpHostContext
{
    // Lex host port state
    // Lex ULP FSM state.
    enum LexUlpHostState ulpFsmState;

    // a timer used to debounce the VBus state
    TIMING_TimerHandlerT vBusDebounceTimer;
    bool vBusDetect;            // true if vBus detected; false otherwise
    uint8_t vBusDebounceCount;  // the number of consecutive vBus readings that are the same

} lexHostUlp;

// Exported Function Definitions ##################################################################


//#################################################################################################
// Handles LEX VBus detect interrupt.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_LexVbusDetectIntHandler(void)
{
    bool currentVbusState = _ULP_isVbusDetSet();

    lexHostUlp.vBusDebounceCount = 0;
    lexHostUlp.vBusDetect = currentVbusState;
    if (currentVbusState == 0)
    {
        // immediately indicate a vbus down event to the host
        ULP_LexHostStateMachineSendEvent(LEX_ULP_HOST_EVENT_HOST_REMOVED);
    }

    // vBus detected, start debouncing (circuitry only gives a ~5ms time)
    LexVbusDebounceControl(true);

    ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_LEX_HOST_VBUS_INTERRUPT, _ULP_isVbusDetSet());
}

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Initialize the LEX host submodule
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void ULP_LexHostInit(void)
{
   lexHostUlp.vBusDebounceTimer = TIMING_TimerRegisterHandler(
        LexDebounceVbus,
        true,
        LEX_VBUS_DEBOUNCE_INTERVAL);

    lexHostUlp.ulpFsmState = LEX_ULP_HOST_STATE_DISABLED;

    // Enable Top's Vbus ISR
    bb_top_irqUsbVbusDetect(true);
}


//#################################################################################################
// Turns the Lex Host machine on or off. Note that turning the host machine off will disable USB
// operation completely
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_LexHostControl(bool enable)
{
    if (enable)
    {
        ULP_LexHostStateMachineSendEvent(LEX_ULP_HOST_EVENT_ENABLED);
    }
    else
    {
        ULP_LexHostStateMachineSendEvent(LEX_ULP_HOST_EVENT_DISABLED);
    }
}


//#################################################################################################
// Tells the host state machine whether it is connected to a USB2 or USB3 port
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_LexHostUsb3Connected(bool usb3Present)
{
    // send a USB3 or USB2 snoop event, based on whether USB3 is present or not
    ULP_LexHostStateMachineSendEvent(
        usb3Present ? LEX_ULP_HOST_EVENT_SNOOP_USB3 : LEX_ULP_HOST_EVENT_USB3_NO_RX_TERMINATIONS);
}


//#################################################################################################
// Sends a USB3 restart event to the host state machine
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_LexHostUsb3RestartRequest(void)
{
    // send a USB3 restart request
    ULP_LexHostStateMachineSendEvent(LEX_ULP_HOST_EVENT_USB3_RESTART);
}

//#################################################################################################
// Tells the Host state machine that the USB3 reset is done
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_LexHostUsb3ResetDone(void)
{
    // send a USB3 restart request
    ULP_LexHostStateMachineSendEvent(LEX_ULP_HOST_EVENT_USB3_RESET_DONE);
}


//#################################################################################################
// Sends a host restart event to the host state machine
// Used to simulate a disconnect/reconnect on the host port
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_LexHostCycleRequest(void)
{
    // Tell the host state machine to disconnect and re-connect
    ULP_LexHostStateMachineSendEvent(LEX_ULP_HOST_EVENT_HOST_SOFT_CYCLE);
}


//#################################################################################################
// Sends the host state machine the current state of the USB2 state machine
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpLexHostSendUsb2Status(enum UlpUsbModuleStatus usb2Status)
{
    ULP_LexHostStateMachineSendEvent(UlpLexHostConvertUsbStatus(usb2Status));
}


//#################################################################################################
// Sends the host state machine the current state of the USB3 state machine
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpLexHostSendUsb3Status(enum UlpUsbModuleStatus usb3Status)
{
    // note the dirty hack to convert these into USB3 events.  Done only to save code space
    ULP_LexHostStateMachineSendEvent(UlpLexHostConvertUsbStatus(usb3Status) + LEX_ULP_HOST_USB3_EVENT_OFFSET);
}


// Static Function Definitions ####################################################################

//#################################################################################################
// Entry point to send an event to the Host port state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_LexHostStateMachineSendEvent(enum LexUlpHostEvent event)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = event;

    CALLBACK_Run(ULP_LexHostStateCallback, (void *)eventx, NULL);
}

//#################################################################################################
// Callback wrapper for the Host port state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_LexHostStateCallback(void *param1, void *param2)
{
    ULP_LexHostStateMachine( (enum LexUlpHostEvent)param1 );
}


//#################################################################################################
// The entry point for the Host port state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ULP_LexHostStateMachine(enum LexUlpHostEvent lexHostEvent)
{
    uint8_t oldState = lexHostUlp.ulpFsmState;

    switch (lexHostUlp.ulpFsmState)
    {
        case LEX_ULP_HOST_STATE_DISABLED:           // host port state machine disabled
            UlpLexHostStateDisabled(lexHostEvent);
            break;

        case LEX_ULP_HOST_STATE_WAIT_USB:            // enabled, waiting for the USB state machines to be ready
            UlpLexHostStateWaitUsb(lexHostEvent);
            break;

        case LEX_ULP_HOST_STATE_HOST_NOT_CONNECTED:  // not connected to a host port
            UlpLexHostStateNoHostPort(lexHostEvent);
            break;

        case LEX_ULP_HOST_STATE_SNOOP_HOST_PORT:    // snooping host to see what interface it supports
            UlpLexHostStateSnoop(lexHostEvent);
            break;

        case LEX_ULP_HOST_STATE_HOST_USB32:          // Lex is connected to a USB3 and USB2 port
            UlpLexHostStateUsb321Connected(lexHostEvent);
            break;

        case LEX_ULP_HOST_STATE_HOST_USB2:          // Lex is connected to a USB2 only port (no USB3!)
            UlpLexHostStateUsb2OnlyConnected(lexHostEvent);
            break;

        case LEX_ULP_HOST_STATE_HOST_USB3:          // Lex is connected to a USB3 only port (no USB2!)
            UlpLexHostStateUsb3OnlyConnected(lexHostEvent);
            break;

        default:
            break;
    }

    ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_LEX_HOST_STATE_MSG, oldState, lexHostEvent, lexHostUlp.ulpFsmState);
}


//#################################################################################################
// Handles events in the LEX_ULP_HOST_STATE_DISABLED
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexHostStateDisabled(enum LexUlpHostEvent event)
{
    switch (event)
    {
        case LEX_ULP_HOST_EVENT_ENABLED:             // Host port state machine enabled
            ULP_LexUsb3Control(ULP_USB_CONTROL_SYSTEM_UP);
            ULP_LexUsb2Control(ULP_USB_CONTROL_SYSTEM_UP);

            // wait for the USB state machines to come up
            lexHostUlp.ulpFsmState = LEX_ULP_HOST_STATE_WAIT_USB;
            break;

        // these events don't do anything when we are disabled
        case LEX_ULP_HOST_EVENT_USB2_READY:          // USB2 module ready to go
        case LEX_ULP_HOST_EVENT_USB2_CONNECTED:      // USB2 module connected, running
        case LEX_ULP_HOST_EVENT_USB2_DISCONNECTED:   // USB2 module disconnected, stopped
        case LEX_ULP_HOST_EVENT_USB2_DISABLED:       // USB2 module is inactive

        case LEX_ULP_HOST_EVENT_USB3_READY:          // USB3 module ready to go
        case LEX_ULP_HOST_EVENT_USB3_CONNECTED:      // USB3 module connected, running
        case LEX_ULP_HOST_EVENT_USB3_DISCONNECTED:   // USB3 module disconnected, stopped
        case LEX_ULP_HOST_EVENT_USB3_DISABLED:       // USB3 module is inactive

        case LEX_ULP_HOST_EVENT_HOST_CONNECTED:     // Host port connected to Lex
        case LEX_ULP_HOST_EVENT_HOST_REMOVED:       // Host port disconnected from Lex
        case LEX_ULP_HOST_EVENT_HOST_SOFT_CYCLE:    // Host port software restart requested
        case LEX_ULP_HOST_EVENT_SNOOP_USB3:         // Host port snoop complete - we are connected to USB3
        case LEX_ULP_HOST_EVENT_USB3_RESTART:       // USB3 restart requested - only valid when connected to USB3
        case LEX_ULP_HOST_EVENT_USB3_RESET_DONE:    // USB3 reset done - only valid when in the USB2 connected state
        case LEX_ULP_HOST_EVENT_USB3_NO_RX_TERMINATIONS:   // USB3 Rx terminations removed
        case LEX_ULP_HOST_EVENT_DISABLED:           // Host port state machine disabled
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_HOST_STATE_WAIT_USB state.  Waits for the USB modules to be ready
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexHostStateWaitUsb(enum LexUlpHostEvent event)
{
    switch (event)
    {
        case LEX_ULP_HOST_EVENT_DISABLED:           // Host port state machine disabled
            LexHostDisabled();
            break;

        case LEX_ULP_HOST_EVENT_USB2_READY:          // USB2 module ready to go
            UlpLexHostCheckUsbStatus(UlpLexUsb3Status());   // see if USB3 is ready
            break;

        case LEX_ULP_HOST_EVENT_USB3_READY:          // USB3 module ready to go
            UlpLexHostCheckUsbStatus(UlpLexUsb2Status());   // see if USB2 is ready
            break;

            // these events don't do anything in this state
        case LEX_ULP_HOST_EVENT_HOST_CONNECTED:     // Host port connected to Lex

        case LEX_ULP_HOST_EVENT_USB2_CONNECTED:      // USB2 module connected, running
        case LEX_ULP_HOST_EVENT_USB2_DISCONNECTED:   // USB2 module disconnected, stopped
        case LEX_ULP_HOST_EVENT_USB2_DISABLED:       // USB2 module is inactive

        case LEX_ULP_HOST_EVENT_USB3_CONNECTED:      // USB3 module connected, running
        case LEX_ULP_HOST_EVENT_USB3_DISCONNECTED:   // USB3 module disconnected, stopped
        case LEX_ULP_HOST_EVENT_USB3_DISABLED:       // USB3 module is inactive

        case LEX_ULP_HOST_EVENT_SNOOP_USB3:         // Host port snoop complete - we are connected to USB3
        case LEX_ULP_HOST_EVENT_USB3_RESTART:       // USB3 restart requested - only valid when connected to USB3
        case LEX_ULP_HOST_EVENT_USB3_RESET_DONE:    // USB3 reset done - only valid when in the USB2 connected state
        case LEX_ULP_HOST_EVENT_USB3_NO_RX_TERMINATIONS:   // USB3 Rx terminations removed
        case LEX_ULP_HOST_EVENT_HOST_REMOVED:       // Host port disconnected from Lex
        case LEX_ULP_HOST_EVENT_HOST_SOFT_CYCLE:    // Host port software restart requested
        case LEX_ULP_HOST_EVENT_ENABLED:            // Host port state machine enabled
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_HOST_STATE_HOST_NOT_CONNECTED
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexHostStateNoHostPort(enum LexUlpHostEvent event)
{
    switch (event)
    {
        case LEX_ULP_HOST_EVENT_HOST_CONNECTED:     // Host port connected to Lex
            ILOG_istatus(ISTATUS_ULP_LEX_HOST_CONNECTED, 0);

            if (UlpLexUsb3Status() == ULP_USB_MODULE_READY)
            {
                UlpLexUsb3HostConnect(true); // tell USB3 to start snooping
                lexHostUlp.ulpFsmState = LEX_ULP_HOST_STATE_SNOOP_HOST_PORT;
            }
            else
            {
                UlpLexHostUsb2Selected();   // USB3 not ready, fall back to USB2
            }
            break;

        case LEX_ULP_HOST_EVENT_DISABLED:           // Host port state machine disabled
            LexHostDisabled();
            break;

        case LEX_ULP_HOST_EVENT_USB2_DISABLED:       // USB2 module is inactive
        case LEX_ULP_HOST_EVENT_USB3_DISABLED:       // USB3 module is inactive
            // wait for the USB state machines to come up
            lexHostUlp.ulpFsmState = LEX_ULP_HOST_STATE_WAIT_USB;

            // need to wait for the USB modules to come up before looking at vBus
            LexVbusDebounceControl(false);
            break;

        // these events don't do anything in this state
        case LEX_ULP_HOST_EVENT_USB2_READY:          // USB2 module ready to go
        case LEX_ULP_HOST_EVENT_USB2_CONNECTED:      // USB2 module connected, running
        case LEX_ULP_HOST_EVENT_USB2_DISCONNECTED:   // USB2 module disconnected, stopped

        case LEX_ULP_HOST_EVENT_USB3_READY:          // USB3 module ready to go
        case LEX_ULP_HOST_EVENT_USB3_CONNECTED:      // USB3 module connected, running
        case LEX_ULP_HOST_EVENT_USB3_DISCONNECTED:   // USB3 module disconnected, stopped

        case LEX_ULP_HOST_EVENT_SNOOP_USB3:         // Host port snoop complete - we are connected to USB3
        case LEX_ULP_HOST_EVENT_USB3_RESTART:       // USB3 restart requested - only valid when connected to USB3
        case LEX_ULP_HOST_EVENT_USB3_RESET_DONE:    // USB3 reset done - only valid when in the USB2 connected state
        case LEX_ULP_HOST_EVENT_USB3_NO_RX_TERMINATIONS:   // USB3 Rx terminations removed
        case LEX_ULP_HOST_EVENT_HOST_REMOVED:       // Host port disconnected from Lex
        case LEX_ULP_HOST_EVENT_HOST_SOFT_CYCLE:    // Host port software restart requested
        case LEX_ULP_HOST_EVENT_ENABLED:            // Host port state machine enabled
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_HOST_STATE_SNOOP_HOST_PORT
//
// Parameters:
//      event                   - LEX ULP event
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpLexHostStateSnoop(enum LexUlpHostEvent event)
{
    switch (event)
    {
        case LEX_ULP_HOST_EVENT_HOST_REMOVED:        // Host port disconnected from Lex
            LexHostRemoved();
            break;

        case LEX_ULP_HOST_EVENT_DISABLED:            // Host port state machine disabled
            LexHostDisabled();
            break;

        case LEX_ULP_HOST_EVENT_SNOOP_USB3:          // Host port snoop complete - we are connected to USB3
            lexHostUlp.ulpFsmState = LEX_ULP_HOST_STATE_HOST_USB3;      // we are connected to USB3

            ILOG_istatus(ISTATUS_ULP_LEX_HOST_USB3, 0); // host port supports USB 3 only
            break;

        case LEX_ULP_HOST_EVENT_USB3_NO_RX_TERMINATIONS:   // USB3 Rx terminations not present - USB2 port
            UlpLexHostUsb2Selected();
            break;

        case LEX_ULP_HOST_EVENT_USB3_DISCONNECTED:   // USB3 module disconnected, stopped
        case LEX_ULP_HOST_EVENT_HOST_SOFT_CYCLE:     // Host port software restart requested
            LexHostSoftCycle();
            break;

        case LEX_ULP_HOST_EVENT_USB2_READY:          // USB2 module ready to go
        case LEX_ULP_HOST_EVENT_USB2_CONNECTED:      // USB2 module connected, running
        case LEX_ULP_HOST_EVENT_USB2_DISCONNECTED:   // USB2 module disconnected, stopped
        case LEX_ULP_HOST_EVENT_USB2_DISABLED:       // USB2 module is inactive

        case LEX_ULP_HOST_EVENT_USB3_READY:          // USB3 module ready to go
        case LEX_ULP_HOST_EVENT_USB3_CONNECTED:      // USB3 module connected, running
        case LEX_ULP_HOST_EVENT_USB3_DISABLED:       // USB3 module is inactive

        case LEX_ULP_HOST_EVENT_HOST_CONNECTED:     // Host port connected to Lex
        case LEX_ULP_HOST_EVENT_ENABLED:            // Host port state machine enabled
        case LEX_ULP_HOST_EVENT_USB3_RESTART:       // USB3 restart requested - only valid when connected to USB3
        case LEX_ULP_HOST_EVENT_USB3_RESET_DONE:    // USB3 reset done - only valid when in the USB2 connected state
        default:
            break;

    }
}


//#################################################################################################
// Handles events in the LEX_ULP_HOST_STATE_HOST_USB32, LEX_ULP_HOST_STATE_HOST_USB2
//
// Parameters:
//      event                   - LEX host ULP event
// Return:
//
// Assumptions:
//
//#################################################################################################
static void UlpLexHostStateUsb2OnlyConnected(enum LexUlpHostEvent event)
{
    switch (event)
    {
        // any cases where USB2 has an error or is removed, treat it as a host removal
        case LEX_ULP_HOST_EVENT_USB2_DISCONNECTED:   // USB2 module disconnected, stopped
        case LEX_ULP_HOST_EVENT_HOST_REMOVED:        // Host port disconnected from Lex
            LexHostRemoved();
            break;

        case LEX_ULP_HOST_EVENT_DISABLED:            // Host port state machine disabled
            LexHostDisabled();
            break;

        case LEX_ULP_HOST_EVENT_USB3_READY:         // USB3 module ready to go
            UlpLexUsb3HostConnect(true);            // tell USB3 we are connected to a host port
            break;

        case LEX_ULP_HOST_EVENT_USB3_CONNECTED:      // USB3 module connected, running
            lexHostUlp.ulpFsmState = LEX_ULP_HOST_STATE_HOST_USB32;  // go to the USB3 state
            ILOG_istatus(ISTATUS_ULP_LEX_HOST_USB321, 0); // host port has USB3/2, now
            break;

        case LEX_ULP_HOST_EVENT_HOST_SOFT_CYCLE:    // Host port software restart requested
            LexHostSoftCycle();
            break;

           // these events don't do anything in this state
        case LEX_ULP_HOST_EVENT_USB2_READY:          // USB2 module ready to go
        case LEX_ULP_HOST_EVENT_USB2_CONNECTED:      // USB2 module connected, running
        case LEX_ULP_HOST_EVENT_USB2_DISABLED:       // USB2 module is inactive

        case LEX_ULP_HOST_EVENT_USB3_DISCONNECTED:   // USB3 module disconnected, stopped
        case LEX_ULP_HOST_EVENT_USB3_DISABLED:       // USB3 module is inactive
        case LEX_ULP_HOST_EVENT_USB3_RESET_DONE:    // USB3 reset done - only valid when in the USB2 connected state

        case LEX_ULP_HOST_EVENT_HOST_CONNECTED:     // Host port connected to Lex
        case LEX_ULP_HOST_EVENT_ENABLED:            // Host port state machine enabled
        case LEX_ULP_HOST_EVENT_USB3_RESTART:       // USB3 restart requested - only valid when connected to USB3
        case LEX_ULP_HOST_EVENT_SNOOP_USB3:          //  terminations are present - we are connected to USB3!
        case LEX_ULP_HOST_EVENT_USB3_NO_RX_TERMINATIONS:   // USB3 Rx terminations removed
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_HOST_STATE_HOST_USB3 state.  USB2 is not available
//
// Parameters:
//      event                   - LEX host ULP event
// Return:
//
// Assumptions:
//
//#################################################################################################
static void UlpLexHostStateUsb3OnlyConnected(enum LexUlpHostEvent event)
{
    switch (event)
    {
        // any cases where USB3 has an error or is removed, treat it as a host removal
        case LEX_ULP_HOST_EVENT_USB3_DISCONNECTED:          // USB3 module disconnected, stopped
        case LEX_ULP_HOST_EVENT_USB3_DISABLED:              // USB3 module is inactive
        case LEX_ULP_HOST_EVENT_HOST_REMOVED:               // Host port disconnected from Lex
            LexHostRemoved();
            break;

        case LEX_ULP_HOST_EVENT_USB3_NO_RX_TERMINATIONS:    // USB3 Rx terminations removed
            if (UlpLexUsb2Status() == ULP_USB_MODULE_READY)
            {
                UlpLexUsb2HostConnect(true);            // tell USB2 to connect
            }
            else
            {
                // no USB2, and no USB3 - go back to waiting to find out what we have
                lexHostUlp.ulpFsmState = LEX_ULP_HOST_STATE_SNOOP_HOST_PORT;
            }
            break;

        case LEX_ULP_HOST_EVENT_DISABLED:           // Host port state machine disabled
            LexHostDisabled();
            break;

        case LEX_ULP_HOST_EVENT_USB3_RESTART:       // USB3 restart requested - only valid when connected to USB3
            LexUSB3Restarted();
            break;

        case LEX_ULP_HOST_EVENT_HOST_SOFT_CYCLE:    // Host port software restart requested
            LexHostSoftCycle();
            break;

        case LEX_ULP_HOST_EVENT_USB2_READY:         // USB2 module ready to go
        case LEX_ULP_HOST_EVENT_USB3_CONNECTED:     // USB3 module connected, running
            UlpLexUsb2HostConnect(true);            // tell USB2 to connect
            break;

        case LEX_ULP_HOST_EVENT_USB2_CONNECTED:      // USB2 module connected, running
            lexHostUlp.ulpFsmState = LEX_ULP_HOST_STATE_HOST_USB32;  // go to the USB321 state
            ILOG_istatus(ISTATUS_ULP_LEX_HOST_USB321, 0); // host port has USB3, now
            break;

        // these events don't do anything in this state
        case LEX_ULP_HOST_EVENT_USB2_DISCONNECTED:   // USB2 module disconnected, stopped
        case LEX_ULP_HOST_EVENT_USB2_DISABLED:       // USB2 module is inactive

        case LEX_ULP_HOST_EVENT_USB3_READY:          // USB3 module ready to go

        case LEX_ULP_HOST_EVENT_SNOOP_USB3:         // Host port snoop complete - we are connected to USB3
        case LEX_ULP_HOST_EVENT_HOST_CONNECTED:     // Host port connected to Lex
        case LEX_ULP_HOST_EVENT_ENABLED:            // Host port state machine enabled
        case LEX_ULP_HOST_EVENT_USB3_RESET_DONE:    // USB3 reset done - only valid when in the USB2 connected state
        default:
            break;
    }
}


//#################################################################################################
// Handles events in the LEX_ULP_HOST_STATE_HOST_USB32, LEX_ULP_HOST_STATE_HOST_USB2
//
// Parameters:
//      event                   - LEX host ULP event
// Return:
//
// Assumptions:
//
//#################################################################################################
static void UlpLexHostStateUsb321Connected(enum LexUlpHostEvent event)
{
    switch (event)
    {
        case LEX_ULP_HOST_EVENT_HOST_REMOVED:       // Host port disconnected from Lex
            LexHostRemoved();
            break;

        case LEX_ULP_HOST_EVENT_DISABLED:           // Host port state machine disabled
            LexHostDisabled();
            break;

        case LEX_ULP_HOST_EVENT_USB3_CONNECTED:     // USB3 module connected, running
            UlpLexUsb2HostConnect(true);            // tell USB2 to connect
            break;

        case LEX_ULP_HOST_EVENT_USB3_DISABLED:       // USB3 module is inactive
            UlpLexHostUsb2Selected();
            break;

        // go to USB2 if either of these events occur
        case LEX_ULP_HOST_EVENT_USB3_NO_RX_TERMINATIONS:   // USB3 Rx terminations removed
        case LEX_ULP_HOST_EVENT_USB3_DISCONNECTED:   // USB3 module disconnected, stopped
            UlpLexHostUsb2Selected();
            UlpLexUsb3HostConnect(false);
            ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_LEX_HOST_USB3_REMOVED);
            break;

        case LEX_ULP_HOST_EVENT_USB3_RESTART:       // USB3 restart requested - only valid when connected to USB3
            LexUSB3Restarted();
            break;

        case LEX_ULP_HOST_EVENT_HOST_SOFT_CYCLE:    // Host port software restart requested
            LexHostSoftCycle();
            break;

        case LEX_ULP_HOST_EVENT_USB2_DISCONNECTED:   // USB2 module disconnected, stopped
            lexHostUlp.ulpFsmState = LEX_ULP_HOST_STATE_HOST_USB3;      // we are connected to USB3 only
            ILOG_istatus(ISTATUS_ULP_LEX_HOST_USB3, 0); // host port supports USB 3 only
            UlpLexUsb2HostConnect(false);            // tell USB2 to disconnect
            break;

        // these events don't do anything in this state
        case LEX_ULP_HOST_EVENT_USB2_READY:          // USB2 module ready to go
        case LEX_ULP_HOST_EVENT_USB2_CONNECTED:      // USB2 module connected, running
        case LEX_ULP_HOST_EVENT_USB2_DISABLED:       // USB2 module is inactive

        case LEX_ULP_HOST_EVENT_USB3_READY:          // USB3 module ready to go

        case LEX_ULP_HOST_EVENT_SNOOP_USB3:         // Host port snoop complete - we are connected to USB3
        case LEX_ULP_HOST_EVENT_HOST_CONNECTED:     // Host port connected to Lex
        case LEX_ULP_HOST_EVENT_ENABLED:            // Host port state machine enabled
        case LEX_ULP_HOST_EVENT_USB3_RESET_DONE:    // USB3 reset done - only valid when in the USB2 connected state
        default:
            break;
    }
}

//#################################################################################################
// Turn vBus debouncing on or off
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexVbusDebounceControl(bool enable)
{
    lexHostUlp.vBusDebounceCount = 0;

    if (enable)
    {
        TIMING_TimerStart(lexHostUlp.vBusDebounceTimer);
    }
    else
    {
        TIMING_TimerStop(lexHostUlp.vBusDebounceTimer);
    }
}

//#################################################################################################
// Debounce a change in vBus state; won't exit until it is stable (hopefully)
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexDebounceVbus(void)
{
    bool currentVbusState = _ULP_isVbusDetSet();

    if (currentVbusState != lexHostUlp.vBusDetect)
    {
        // change of vBus state, restart debouncing
        lexHostUlp.vBusDetect = currentVbusState;
        lexHostUlp.vBusDebounceCount = 0;

        // immediately signal if there is no vBus
        if (currentVbusState == 0)
        {
            ULP_LexHostStateMachineSendEvent(LEX_ULP_HOST_EVENT_HOST_REMOVED);
        }
    }
    else
    {
        lexHostUlp.vBusDebounceCount++;
        if (lexHostUlp.vBusDebounceCount > LEX_DEBOUNCE_OK_COUNT)
        {
            ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_LEX_HOST_VBUS_DETECT, currentVbusState);

            LexVbusDebounceControl(false);  // debouncing done, turn it off

            if (currentVbusState == 1)
            {
                ULP_LexHostStateMachineSendEvent(LEX_ULP_HOST_EVENT_HOST_CONNECTED);
            }
            else
            {
                ULP_LexHostStateMachineSendEvent(LEX_ULP_HOST_EVENT_HOST_REMOVED);
            }
        }
    }
}


//#################################################################################################
// Lex has been disconnected from the host
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexHostSoftCycle(void)
{
    lexHostUlp.vBusDebounceCount = 0;
 
    UlpLexUsb3HostConnect(false);
    UlpLexUsb2HostConnect(false);

    // we disconnected USB2 and USB3 - wait until they tell us they are ready
    lexHostUlp.ulpFsmState = LEX_ULP_HOST_STATE_WAIT_USB;

    ILOG_istatus(ISTATUS_ULP_LEX_HOST_REMOVED, 0);
}


//#################################################################################################
// USB3 restart has been requested
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexUSB3Restarted(void)
{
    // the only difference between a soft cycle and a host removed is the hard reset we
    // do in the case of a actual host removed event
    LexHostSoftCycle();

    ULP_UlpLexUsb3ResetStart();     // disconnected - go through a reset cycle
}

//#################################################################################################
// Lex has been disconnected from the host
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexHostRemoved(void)
{
    // the only difference between a soft cycle and a host removed is the hard reset we
    // do in the case of an actual host removed event
    LexHostSoftCycle();

    if (ulp_Usb3ResetOnDisconnect())    // if we can do a reset on host disconnect...
    {
        ULP_UlpLexUsb3ResetStart();     // disconnected - go through a reset cycle

        // do a GE restart, too
        UlpGeControl(false);
        UlpGeControl(true);
    }
}


//#################################################################################################
// Lex has been disconnected from the host
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpLexHostCheckUsbStatus(enum UlpUsbModuleStatus usbStatus)
{
    if ((usbStatus == ULP_USB_MODULE_READY) || (usbStatus == ULP_USB_MODULE_SHUTDOWN))
    {
        // USB modules are ready
        // assume no host is connected - the vbus debounce timer will set it straight
        lexHostUlp.ulpFsmState = LEX_ULP_HOST_STATE_HOST_NOT_CONNECTED;

        // get the initial state of the host port
        LexVbusDebounceControl(true);
    }
}


//#################################################################################################
// Goes to a USB2 only state
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpLexHostUsb2Selected(void)
{
    UlpLexUsb2HostConnect(true);            // tell USB2 to connect
    lexHostUlp.ulpFsmState = LEX_ULP_HOST_STATE_HOST_USB2;          // we are connected to USB2 only

    ILOG_istatus(ISTATUS_ULP_LEX_HOST_USB2, 0); // host port supports USB 2 only
}


//#################################################################################################
// Lex has been disconnected from the host
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static enum LexUlpHostEvent UlpLexHostConvertUsbStatus(enum UlpUsbModuleStatus usbStatus)
{
    enum LexUlpHostEvent result = LEX_ULP_HOST_EVENT_USB2_DISABLED;

    switch (usbStatus)
    {
        case ULP_USB_MODULE_READY:           // module is active
            result = LEX_ULP_HOST_EVENT_USB2_READY;
            break;

        case ULP_USB_MODULE_CONNECTED:       // module is connected, running
            result = LEX_ULP_HOST_EVENT_USB2_CONNECTED;
            break;

        case ULP_USB_MODULE_DISCONNECTED:    // module is disconnected, stopped
            result = LEX_ULP_HOST_EVENT_USB2_DISCONNECTED;
            break;

        case ULP_USB_MODULE_DISABLED:        // module is inactive
        case ULP_USB_MODULE_SHUTDOWN:        // module is shutdown - disabled by user or the other side
        default:
            result = LEX_ULP_HOST_EVENT_USB2_DISABLED;
            break;
    }

    return (result);
}


//#################################################################################################
// Helper code for when the host has been disabled
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexHostDisabled(void)
{
    LexHostRemoved();

    ULP_LexUsb3Control(ULP_USB_CONTROL_SYSTEM_DOWN);    // system disabled
    ULP_LexUsb2Control(ULP_USB_CONTROL_SYSTEM_DOWN);

    lexHostUlp.ulpFsmState = LEX_ULP_HOST_STATE_DISABLED;
}




