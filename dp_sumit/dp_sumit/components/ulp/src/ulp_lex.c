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
// This file contains the implementation of LEX ULP (USB Link Protocol)
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
#include <i2cd_cypress_hx3.h>
#include <event.h>
#include <mca.h>
#include <interrupts.h>
#include <ulp.h>
#include "ulp_log.h"
#include "ulp_loc.h"
#include <configuration.h>
#include <uart.h>   // only needed for UART_printf()

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// control flag definitions
// note that usbControl is only 8 bits and will need to be expanded if we have more then 8 flags
#define LEX_ULP_LEX_ENABLED                 0x01    // set when the host module is enabled
#define LEX_ULP_LEX_COMLINK                 0x02    // set when the system's comm link is up
#define LEX_ULP_LEX_HUB_READY               0x04    // set when the hub on the Lex is ready
#define LEX_ULP_LEX_PARTNER_READY           0x08    // set when the device we are paired with is ready

#define LEX_CONTROL_FLAG_REX_DISABLED       0x40    // Rex is disabled
#define LEX_CONTROL_FLAG_RX_REX_READY       0x80    // Rex is ready - message sent

// when this mask is set, the lex is ready, and is just waiting for the rex to check in
#define LEX_USB_LEX_READY_MASK    \
    (LEX_ULP_LEX_ENABLED        | \
     LEX_ULP_LEX_COMLINK        | \
     LEX_ULP_LEX_HUB_READY )

// send out an 'I am alive' message once every 500ms until we hear back from the Rex
#define LEX_ALIVE_INTERVAL      500

enum LexUlpState
{
    LEX_ULP_COMM_LINK_DN, // No Rex-Lex communication link
    LEX_ULP_NO_HOST_PORT_CONNECTED,         // not connected to a host port
    LEX_ULP_SNOOP_HOST_PORT_NO_REX_DEVICE,  // snooping host without rex device
    LEX_ULP_LEX_USB3_CONNECTED,             // connected to a USB3, USB2 capable port
    LEX_ULP_LEX_USB2_CONNECTED,             // connected to a USB 2 only port (no USB 3)
    LEX_ULP_REX_DEVICE_USB3_SETUP,          // waiting for USB 3 setup to complete on Rex
    LEX_ULP_LEX_USB3_SETUP,                 // waiting for USB 3 setup to complete on the Lex
    LEX_ULP_REX_DEVICE_USB2_SETUP,          // waiting for USB2 to be enabled on Rex
    LEX_ULP_USB_LINK_ESTABLISHED,           // USB Link successfully established between Lex and Rex
};

// TODO add a master timeout event, so if we get stuck we will go back to the idle state
enum LexUlpEvent
{
    LEX_ULP_EVENT_HOST_CONNECTED,       // Host port connected to Lex
    LEX_ULP_EVENT_HOST_REMOVED,         // Host port disconnected from Lex
    LEX_ULP_LEX_PORT_SNOOP_COMPLETE,    // Host port snoop complete
    LEX_ULP_EVENT_REX_DEVICE_REMOVED,   // Rex Device removed
    LEX_ULP_EVENT_REX_USB3_SETUP,       // Rex USB3 setup complete
    LEX_ULP_EVENT_REX_USB3_FAILED,      // Rex USB3 setup failed
    LEX_ULP_EVENT_HOST_USB3_SETUP,      // Host USB3 setup complete
    LEX_ULP_EVENT_HOST_USB3_FAILED,     // Host USB3 setup failed
    LEX_ULP_EVENT_REX_USB2_SETUP,       // Rex USB2 setup complete
    LEX_ULP_EVENT_COMM_LINK_UP,         // Blackbird link active
    LEX_ULP_EVENT_COMM_LINK_DOWN,       // Blackbird link unavailable
};

typedef void (*LexUlpEventHandlerFn)(enum LexUlpEvent event, const uint8_t* context);

// Static Function Declarations ###################################################################
static void LexUlpReceiveCpuMsgHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength);

static void LexCommLinkEventHandler(uint32_t linkUp, uint32_t userContext)              __attribute__((section(".lexatext")));
static void LexConfigurationEventHandler(uint32_t eventInfo, uint32_t userContext)      __attribute__((section(".lexatext")));

static void LexIamAliveTimeout(void)                                                    __attribute__((section(".lexatext")));
static void LexUlpUpdateSystem(void)                                                    __attribute__((section(".lexatext")));
static void LexCypressHubInitComplete(void)                                             __attribute__((section(".lexatext")));
static void LexLinkUpMsgToRex(void)                                                     __attribute__((section(".lexatext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct LexUlpContext
{
    uint8_t controlFlags;           // miscellaneous flags for operations

    // A timer used to periodically send out the 'Lex is alive' message
    TIMING_TimerHandlerT lexAliveTimer;

} lexUlp;

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize the LEX ULP module and the ULP finite state machine.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void ULP_LexInit(void)
{
    lexUlp.lexAliveTimer = TIMING_TimerRegisterHandler( LexIamAliveTimeout, true, LEX_ALIVE_INTERVAL);

    UsbPortsEnabled usbLexControl = ulp_GetUsbEnables();  // get the enable bits for USB2, USB3

    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_ULP_USB,        LexUlpReceiveCpuMsgHandler);             // used for Generic USB messages between Lex/Rex
    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_ULP_USB2,       ULP_LexUsb2RxCpuMessageHandler);         // used for USB 2 messages between Lex/Rex
    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_ULP_USB3,       ULP_LexUsb3RxCpuMessageHandler);         // used for USB 3 messages between Lex/Rex
    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_ULP_USB3_RESET, ULP_LexUsb3ResetRxCpuMessageHandler);    // used for USB 3 reset specific messages between Lex/Rex

    ULP_LexHostInit();  // initialize the host state machine
    ULP_LexUsb2Init(usbLexControl.usb2Enabled);  // initialize the USB2 state machine
    ULP_LexUsb3Init(usbLexControl.usb3Enabled);  // initialize the USB3 state machine

    EVENT_Subscribe(ET_COMLINK_STATUS_CHANGE, LexCommLinkEventHandler, 0);
    EVENT_Subscribe(ET_CONFIGURATION_CHANGE, LexConfigurationEventHandler, 0);

    I2CD_CypressHx3HubInit(LexCypressHubInitComplete);

    // if USB2 or USB3 is enabled, then the USB module is enabled
    if (usbLexControl.usb2Enabled || usbLexControl.usb3Enabled)
    {
        // go do the hub programming cycle
        UlpLexProgramCypressHub();
    }
    else
    {
        // if the USB module is disabled, treat it like a link down
        LexCommLinkEventHandler( false, 0);
    }

    ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_LEX_INITIALIZATION);
}

//#################################################################################################
// The main ISR for the LEX's ULP state machine.
//
// Parameters:
// Return:
// Assumptions:
//      * Currently it only services VBus detection interrupt.
//#################################################################################################
void ULP_LexUlpISR(void)
{
    uint32_t lexUlpIntsIrq0 = UlpHalGetIrq0Interrupts();
    uint32_t lexUlpIntsIrq1 = UlpHalGetIrq1Interrupts();

    ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_LEX_ISR, lexUlpIntsIrq0, lexUlpIntsIrq1, UlpHalGetLTSSM());

    if (lexUlpIntsIrq0)
    {
        ULP_LexUsb3Irq0(lexUlpIntsIrq0);  // handle the USB3 specific interrupts
    }

    if (lexUlpIntsIrq1)
    {
        ULP_LexUsb3Irq1(lexUlpIntsIrq1);  // handle the USB3 specific interrupts
    }
}


// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Turns the Lex Host machine on or off. Note that turning the host machine off will disable USB
// operation completely
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void ULP_LexUsbControl(enum UlpLexUsbControl operation)
{
    uint16_t oldControlFlags = lexUlp.controlFlags;  // to see if things had really changed

    switch (operation)
    {
        case ULP_USB_LEX_ENABLE:         // enable the host port
            lexUlp.controlFlags |=  LEX_ULP_LEX_ENABLED;
            break;

        case ULP_USB_LEX_DISABLE:        // disable the host port
            lexUlp.controlFlags &= ~LEX_ULP_LEX_ENABLED;
            break;

        case ULP_USB_LEX_COMLINK_UP:     // comm link is active
            lexUlp.controlFlags |=  LEX_ULP_LEX_COMLINK;
            break;

        case ULP_USB_LEX_COMLINK_DOWN:   // comm link is inactive, assume the other device is not ready as well!
            lexUlp.controlFlags &= ~(LEX_ULP_LEX_COMLINK | LEX_ULP_LEX_PARTNER_READY | LEX_CONTROL_FLAG_RX_REX_READY | LEX_CONTROL_FLAG_REX_DISABLED );
            TIMING_TimerStop(lexUlp.lexAliveTimer);
            break;

        // after a comm link up event, the paired device might take some time to be available
        // these two messages allow one side to synchronize with the other side
        case ULP_USB_LEX_DEVICE_READY:       // The other paired device is ready (synchronization message)
            lexUlp.controlFlags |=  LEX_ULP_LEX_PARTNER_READY;
            break;

        case ULP_USB_LEX_DEVICE_BUSY: // The other paired device is busy
            lexUlp.controlFlags &= ~LEX_ULP_LEX_PARTNER_READY;  // partner is definitely not ready (shouldn't matter, though)
            break;

        case ULP_USB_LEX_DEVICE_UNAVAILABLE: // The other paired device is disabled
            lexUlp.controlFlags &= ~LEX_ULP_LEX_PARTNER_READY;  // partner is definitely not ready (shouldn't matter, though)
            lexUlp.controlFlags |=  LEX_CONTROL_FLAG_REX_DISABLED;
            TIMING_TimerStop(lexUlp.lexAliveTimer); // next comm link up we get we will try again
            break;

        case ULP_USB_LEX_I_AM_ALIVE:         // sent out whenever the Lex is back from a link down and hasn't heard from the Rex
            break;  // nothing to do - if we are up, we'll send a message to the Rex, below

        case ULP_USB_LEX_HUB_READY:      // Hub has been initialized
            lexUlp.controlFlags |=  LEX_ULP_LEX_HUB_READY;
            break;

        case ULP_USB_LEX_HUB_DOWN:      // Hub is not ready
            lexUlp.controlFlags &=  ~LEX_ULP_LEX_HUB_READY;
            break;

        default:
            break;
    }

    if (oldControlFlags != lexUlp.controlFlags)
    {
        LexUlpUpdateSystem();

        // if we were ready, and now we are not, tell the other side
        if ( ((oldControlFlags & LEX_USB_LEX_READY_MASK) == LEX_USB_LEX_READY_MASK) &&
             ((lexUlp.controlFlags & LEX_USB_LEX_READY_MASK) != LEX_USB_LEX_READY_MASK) )
        {
            UlpSendCPUCommLexUsbMessage(ULP_LEX_TO_REX_LEX_BUSY);  // tell the Rex we are busy
        }
    }

    ilog_ULP_COMPONENT_2(ILOG_DEBUG, ULP_LEX_SET_CONTROL_RESULT, lexUlp.controlFlags, operation);
}

//#################################################################################################
// gets the status of the Lex comlink status - true if active, false if down
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool UlpLexComlinkActive(void)
{
  return (lexUlp.controlFlags & LEX_ULP_LEX_COMLINK);
}


//#################################################################################################
// Gets the status of the Rex- true if disabled, false if active
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool UlpLexRexUsbDisabled(void)
{
  return (lexUlp.controlFlags & LEX_CONTROL_FLAG_REX_DISABLED);
}

//#################################################################################################
// Either Rex USB2 or USB3 is enabled - start the I am alive timer if the Rex hasn't
// already responded
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpLexRexUsbAllowed(void)
{
    // USB is allowed on the Rex - if we haven't heard from it yet, start pinging
    if ( !(lexUlp.controlFlags & LEX_ULP_LEX_PARTNER_READY)     &&
         !(lexUlp.controlFlags & LEX_CONTROL_FLAG_REX_DISABLED) &&
          (lexUlp.controlFlags & LEX_ULP_LEX_ENABLED))
    {
        TIMING_TimerStart(lexUlp.lexAliveTimer);
    }
}


//#################################################################################################
// Sends the specified CPU message to the Rex
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
void UlpSendCPUCommLexUsbMessage(enum UlpLexSendUsbMessage lexMessage)
{
    CPU_COMM_sendSubType(CPU_COMM_TYPE_ULP_USB, lexMessage);
}


//#################################################################################################
// Sends the specified CPU message to the Rex
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
void UlpSendCPUCommLexUsb2Message(enum UlpLexSendUsb2Message lexMessage)
{
    CPU_COMM_sendSubType(CPU_COMM_TYPE_ULP_USB2, lexMessage);
}


//#################################################################################################
// Sends the specified CPU message to the Rex
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
void UlpSendCPUCommLexUsb3Message(enum UlpLexSendUsb3Message lexMessage)
{
    CPU_COMM_sendSubType(CPU_COMM_TYPE_ULP_USB3, lexMessage);
}


//#################################################################################################
// Sends the specified CPU message to the Rex
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
void UlpSendCPUCommLexUsb3ResetMessage(enum UlpLexSendUsb3ResetMessage lexMessage)
{
    CPU_COMM_sendSubType(CPU_COMM_TYPE_ULP_USB3_RESET, lexMessage);
}

//#################################################################################################
// Starts the programming of the Cypress hub
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UlpLexProgramCypressHub(void)
{
    // ok - starting to program the hub - make sure it is marked as down
    ULP_LexUsbControl(ULP_USB_LEX_HUB_DOWN);

    // go do the hub programming cycle
    I2CD_CypressHubStartProgramming();
}

// Static Function Definitions ####################################################################

//#################################################################################################
// The handler for LEX receiving a CPU message from the REX.
//
// Parameters:
//      msg                     - message data
//      msgLength               - message length
// Return:
// Assumptions:
//      * The message length is always 1 byte.
//#################################################################################################
static void LexUlpReceiveCpuMsgHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    iassert_ULP_COMPONENT_1(msgLength == 0, ULP_LEX_UNEXPECTED_CPU_MSG_LENGTH, msgLength);
    ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_LEX_RCV_REX_USB_MSG, subType);

    if (UlpLexComlinkActive())
    {
        enum UlpRexSendUsbMessage rexUsbMessage = (enum UlpRexSendUsbMessage)subType;

        switch (rexUsbMessage)
        {
            case ULP_REX_TO_LEX_REX_USB_MODULE_DISABLED:     // Rex to Lex - Rex USB module is currently disabled
                ULP_LexUsbControl(ULP_USB_LEX_DEVICE_UNAVAILABLE);
                break;

            case ULP_REX_TO_LEX_REX_COMLINK_UP:              // Rex Com link is up
                LexLinkUpMsgToRex();
                break;

            case ULP_REX_TO_LEX_REX_USB_READY:               // Rex is ready to start USB operations
                ULP_LexUsbControl(ULP_USB_LEX_DEVICE_READY);
                break;

            case ULP_REX_TO_LEX_REX_BUSY:                    // Rex is busy setting up USB
                ULP_LexUsbControl(ULP_USB_LEX_DEVICE_BUSY);
                break;

            default:
                break;
        }
    }
}

//#################################################################################################
// LEX Configuration Event handler.
//
// Parameters: eventInfo
// Return:
// Assumptions:
//
//#################################################################################################
static void LexConfigurationEventHandler(uint32_t eventInfo, uint32_t userContext)
{
    if(eventInfo == CONFIG_VARS_BB_FEATURE_CONTROL)
    {
        UsbPortsEnabled usbPortsEnabled = ulp_GetUsbEnables();
        ULP_LexUsb2Control(usbPortsEnabled.usb2Enabled ? ULP_USB_CONTROL_ENABLE : ULP_USB_CONTROL_DISABLE);
        ULP_LexUsb3Control(usbPortsEnabled.usb3Enabled ? ULP_USB_CONTROL_ENABLE : ULP_USB_CONTROL_DISABLE);
    }
}

//#################################################################################################
// Handler for communication link up/down interrupts.
//
// Parameters:
// Return:
// Assumptions: Intended to be registered as a callback to be executed when the link status changes
//
//#################################################################################################
static void LexCommLinkEventHandler(uint32_t linkUp, uint32_t userContext)
{
    if (EVENT_GetEventInfo(ET_COMLINK_STATUS_CHANGE))
    {
        ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_LEX_COMLINK_UP_EVENT);

        if (ULP_LexUsb2LocalEnabled() || ULP_LexUsb3Enabled())
        {
            // link up - go through a reset cycle (will also reset the Rex's USB3)
            // will also do this on init, because we will see the link up event
            ULP_LexUsbControl(ULP_USB_LEX_ENABLE);  // enable the USB system

            ULP_LexUsbControl(ULP_USB_LEX_COMLINK_UP);
        }
    }
    else
    {
        ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_LEX_COMLINK_DOWN_EVENT);

        ULP_LexUsbControl(ULP_USB_LEX_COMLINK_DOWN);
        ULP_LexUsbControl(ULP_USB_LEX_DISABLE);  // disable the USB system

        ULP_LexUsb2Control(ULP_USB_CONTROL_PARTNER_SHUTDOWN);
        ULP_LexUsb3Control(ULP_USB_CONTROL_PARTNER_SHUTDOWN);

        TOPLEVEL_clearPollingMask(SECONDARY_INT_ULP_INT_CORE_MSK);  // no polled interrupts; link is down

        UlpGeControl(false);                                // Com link down - put GE into reset
        bb_top_ApplyResetXusb(true);
        ULP_UlpLexUsb3ResetStop();                          // wait until the link is up before going through a reset cycle
    }
}

//#################################################################################################
// Called periodically from a timer once the Com Link is up, until we have heard from the Rex
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexIamAliveTimeout(void)
{
    LexUlpUpdateSystem();   // sends a message to the Rex if we are ready
}


//#################################################################################################
// Called to see if the control flags indicate we are ready
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexUlpUpdateSystem(void)
{
    bool hostEnable = false;

    // see if the we (lex) is ready to go
    if ((lexUlp.controlFlags & LEX_USB_LEX_READY_MASK) == LEX_USB_LEX_READY_MASK)
    {
        // we are ready - if the rex has checked in, the system is ready to go!
        if (lexUlp.controlFlags & LEX_ULP_LEX_PARTNER_READY)
        {
            hostEnable = true;
            if (!(lexUlp.controlFlags & LEX_CONTROL_FLAG_RX_REX_READY))
            {
                UlpSendCPUCommLexUsbMessage(ULP_LEX_TO_REX_LEX_USB_READY);  // tell the Rex we are ready
                lexUlp.controlFlags |= LEX_CONTROL_FLAG_RX_REX_READY;   // but only once...
                TIMING_TimerStop(lexUlp.lexAliveTimer);
           }
        }
        else
        {
            UlpSendCPUCommLexUsbMessage(ULP_LEX_TO_REX_LEX_USB_READY);  // tell the Rex we are ready
        }
    }

    // turn on or off the Host port state machine based on the state of the system
    ULP_LexHostControl(hostEnable);

    ilog_ULP_COMPONENT_2(ILOG_DEBUG, ULP_LEX_SYSTEM_UPDATE_RESULT, lexUlp.controlFlags, hostEnable);
}

//#################################################################################################
// Callback for when the Lex Hub is successfully initialized/programmed
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LexCypressHubInitComplete(void)
{
    // ok - hub is up - mark it as ready
    ULP_LexUsbControl(ULP_USB_LEX_HUB_READY);
}


//#################################################################################################
// Lex send linkup message to Rex
//
// Parameters:
// Return:
// Assumptions: Lex sends information after getting Rex's request
//
//#################################################################################################
static void LexLinkUpMsgToRex(void)
{
    if (ULP_LexUsb2LocalEnabled())
    {
        UlpSendCPUCommLexUsb2Message(ULP_LEX_TO_REX_USB2_MODULE_ENABLED);   // USB2 is enabled on this side
    }
    else
    {
        UlpSendCPUCommLexUsb2Message(ULP_LEX_TO_REX_USB2_MODULE_DISABLED);   // USB2 is disabled on this side
    }

    if (ULP_LexUsb3Enabled())
    {
        UlpSendCPUCommLexUsb3Message(ULP_LEX_TO_REX_USB3_MODULE_ENABLED);   // USB3 is enabled on this side
    }
    else
    {
        UlpSendCPUCommLexUsb3Message(ULP_LEX_TO_REX_USB3_MODULE_DISABLED);  // USB3 is disabled on this side
    }

    UlpSendCPUCommLexUsbMessage(ULP_LEX_TO_REX_LEX_COMLINK_UP);
}
