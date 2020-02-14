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
// This file contains the implementation of REX ULP bringup.
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
#include <bb_ge_comm.h>
#include <mca.h>
#include <i2cd_cypress_hx3.h>
#include <event.h>
#include <interrupts.h>
#include <ulp.h>
#include "ulp_loc.h"
#include "ulp_log.h"
#include <configuration.h>

// Constants and Macros ###########################################################################
// control flag definitions
// note that usbControl is only 8 bits and will need to be expanded if we have more then 8 flags
#define REX_ULP_REX_ENABLED                     0x01    // set when the host module is enabled
#define REX_ULP_REX_COMLINK                     0x02    // set when the system's comm link is up
#define REX_ULP_REX_VBUS_READY                  0x10    // set when the Rex's Vbus is ready to be set high
#define REX_ULP_REX_HUB_READY                   0x20    // set when the Rex's Hub has been initialized
#define REX_ULP_REX_PARTNER_READY               0x40    // set when the device we are paired with is ready

#define REX_CONTROL_FLAG_LEX_DISABLED           0x80    // Lex USB is disabled

// when this mask is set, the Rex USB module can start (and with it, the rest of the USB system)
#define REX_USB_REX_ALLOWED_MASK      \
    (REX_ULP_REX_ENABLED            | \
     REX_ULP_REX_COMLINK            | \
     REX_ULP_REX_VBUS_READY         | \
     REX_ULP_REX_HUB_READY)

// minimum time vBus is off is 500ms, so the devices can properly see vBus go away
#define REX_ULP_MINIMUM_VBUS_OFF_TIME   500

// delivering Rex's Comlink up to ask Lex's USB information every 50ms, it needs to be stop after getting the information
#define REX_ULP_REX_COMPLINKUP_SEND     50

// Data Types #####################################################################################

// Static Function Declarations ###################################################################
static void RexUlpCpuMsgEventHandler(
    uint8_t subType, const uint8_t *msg, uint16_t msgLength)                                __attribute__((section(".rexftext")));

static void RexCypressHubInitComplete(void)                                                 __attribute__((section(".rexatext")));
static void RexCommLinkEventHandler(uint32_t linkUp, uint32_t userContext)                  __attribute__((section(".rexatext")));
static void RexConfigurationEventHandler(uint32_t eventInfo, uint32_t userContext)          __attribute__((section(".rexatext")));

static void RexUlpVbusDelayTimeout(void)                                                    __attribute__((section(".rexatext")));
static void UlpRexUsb2DevDisconnectHandler(void)                                            __attribute__((section(".rexatext")));
static void UlpRexUsb2DevConnectHandler(void)                                               __attribute__((section(".rexatext")));
static void RexUlpComLinkSendTimeout(void)                                                  __attribute__((section(".rexatext")));
static void RexLinkUpMsgToLex(void)                                                         __attribute__((section(".rexatext")));

// Global Variables ###############################################################################
// Static Variables ###############################################################################
static struct RexUlpContext
{
    uint8_t controlFlags;           // miscellaneous flags for operations

    struct
    {
        uint8_t vBusState : 1;          // true = vBus On, false = vBus off
        uint8_t vBusDelayActive : 1;    // delay active when turning vBus off

    } flags;

    // a timer used to delay vBus, so it stays off a minimum time
    TIMING_TimerHandlerT vBusWaitTimer;

    // a timer used to inform Rex side comlink Up and ask for Lex to send USB message
    TIMING_TimerHandlerT rexComLinkUpTimer;

} rexUlp;


// Exported Function Definitions ##################################################################
//#################################################################################################
// Initialize the REX ULP module and the ULP finite state machine.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void ULP_RexInit(void)
{
    UsbPortsEnabled usbRexControl = ulp_GetUsbEnables();  // get the enable bits for USB2, USB3

    // setup a timer enforce a minimum vBus off time on the Rex
    rexUlp.vBusWaitTimer = TIMING_TimerRegisterHandler(
        RexUlpVbusDelayTimeout,
        false,
        REX_ULP_MINIMUM_VBUS_OFF_TIME);

    // setup a timer ask for Lex to send USB message
    rexUlp.rexComLinkUpTimer = TIMING_TimerRegisterHandler(
        RexUlpComLinkSendTimeout,
        true,                // periodic timer, needs stop
        REX_ULP_REX_COMPLINKUP_SEND);

    ULP_RexUsbControl(ULP_USB_REX_VBUS_READY);  // initially, vBus is ready
    UlpVbusControl(false);               // make sure vBus is off, initially

    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_ULP_USB,        RexUlpCpuMsgEventHandler);               // used for Generic USB messages between Lex/Rex
    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_ULP_USB2,       ULP_RexUsb2RxCpuMessageHandler);         // used for USB 2 messages between Lex/Rex
    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_ULP_USB3,       ULP_RexUsb3RxCpuMessageHandler);         // used for USB 3 messages between Lex/Rex
    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_ULP_USB3_RESET, ULP_RexUsb3ResetRxCpuMessageHandler);    // used for USB 3 reset specific messages between Lex/Rex

    BBGE_COMM_registerHandler(BBGE_COMM_MSG_HNDLR_GE_REX_DEV_DISCONN, &UlpRexUsb2DevDisconnectHandler);
    BBGE_COMM_registerHandler(BBGE_COMM_MSG_HNDLR_GE_REX_DEV_CONN,    &UlpRexUsb2DevConnectHandler);

    ULP_RexUsb2Init(usbRexControl.usb2Enabled);  // initialize the USB2 state machine
    ULP_RexUsb3Init(usbRexControl.usb3Enabled);  // initialize the USB3 state machine

    EVENT_Subscribe(ET_COMLINK_STATUS_CHANGE, RexCommLinkEventHandler, 0);
    EVENT_Subscribe(ET_CONFIGURATION_CHANGE, RexConfigurationEventHandler, 0);

    I2CD_CypressHx3HubInit(RexCypressHubInitComplete);

    // if USB2 or USB3 is enabled, then the USB module is enabled
    if (usbRexControl.usb2Enabled || usbRexControl.usb3Enabled)
    {
        // go do the hub programming cycle
        UlpRexProgramCypressHub();
    }
}


//#################################################################################################
// The main ISR for the REX's ULP state machine.
//
// Parameters:
// Return:
// Assumptions:
//      * Currently it does not service any interrupt.
//#################################################################################################
void ULP_RexUlpISR(void)
{
    uint32_t rexUlpIntsIrq0 = UlpHalGetIrq0Interrupts();
    uint32_t rexUlpIntsIrq1 = UlpHalGetIrq1Interrupts();

    ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_REX_ISR, rexUlpIntsIrq0, rexUlpIntsIrq1, UlpHalGetLTSSM());

    if (rexUlpIntsIrq0)
    {
        ULP_RexUsb3Irq0(rexUlpIntsIrq0);  // handle the USB3 specific interrupts
    }

    if (rexUlpIntsIrq1)
    {
        ULP_RexUsb3Irq1(rexUlpIntsIrq1);  // handle the USB3 specific interrupts
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
void ULP_RexUsbControl(enum UlpRexUsbControl operation)
{
    enum UlpUsbControl usbControl = ULP_USB_CONTROL_SYSTEM_DOWN;  // ASSUME the system is down
    uint16_t oldControlFlags = rexUlp.controlFlags;  // to see if things had really changed

    switch (operation)
    {
        case ULP_USB_REX_ENABLE:         // enable USB on the Rex
            rexUlp.controlFlags |=  REX_ULP_REX_ENABLED;
            break;

        case ULP_USB_REX_DISABLE:        // disable USB on the Rex
            rexUlp.controlFlags &= ~REX_ULP_REX_ENABLED;
            break;

        case ULP_USB_REX_COMLINK_UP:     // comm link is active
            rexUlp.controlFlags |=  REX_ULP_REX_COMLINK;
            break;

        case ULP_USB_REX_COMLINK_DOWN:   // comm link is inactive, assume the other device is not ready as well!
            rexUlp.controlFlags &= ~(REX_ULP_REX_COMLINK | REX_ULP_REX_PARTNER_READY | REX_CONTROL_FLAG_LEX_DISABLED);
            break;

       // after a comm link up event, the paired device might take some time to be available
        // ULP_USB_REX_DEVICE_READY and ULP_USB_REX_DEVICE_BUSY allow one side to synchronize with the other side
        case ULP_USB_REX_DEVICE_READY:       // The other paired device is ready (synchronization message)
            rexUlp.controlFlags |=  REX_ULP_REX_PARTNER_READY;
            break;
        case ULP_USB_REX_DEVICE_BUSY:        // The other paired device is busy (synchronization message)
            rexUlp.controlFlags &= ~REX_ULP_REX_PARTNER_READY;
            break;

       case ULP_USB_REX_DEVICE_UNAVAILABLE: // The other paired device is disabled
            rexUlp.controlFlags &= ~REX_ULP_REX_PARTNER_READY;
            rexUlp.controlFlags |= REX_CONTROL_FLAG_LEX_DISABLED;
            break;

       case ULP_USB_REX_VBUS_READY:      // Rex Vbus ready to be put high
           rexUlp.controlFlags |=  REX_ULP_REX_VBUS_READY;
           break;

       case ULP_USB_REX_VBUS_DOWN:       // Rex Vbus in a minimum low cycle
           rexUlp.controlFlags &=  ~REX_ULP_REX_VBUS_READY;
            break;

        case ULP_USB_REX_HUB_READY:      // Hub has been initialized
            rexUlp.controlFlags |=  REX_ULP_REX_HUB_READY;
            break;

        case ULP_USB_REX_HUB_DOWN:      // Hub is not ready
            rexUlp.controlFlags &=  ~REX_ULP_REX_HUB_READY;
            break;

        default:
            break;
    }

    // turn on or off the USB state machines based on the state of the system
    if ((rexUlp.controlFlags & REX_USB_REX_ALLOWED_MASK) == REX_USB_REX_ALLOWED_MASK)
    {
        UlpSendCPUCommRexUsbMessage(ULP_REX_TO_LEX_REX_USB_READY);  // tell the Lex we are ready

        if (rexUlp.controlFlags & REX_ULP_REX_PARTNER_READY)
        {
            // our partner is ready - turn on the system
            usbControl = ULP_USB_CONTROL_SYSTEM_UP;
        }
    }

    ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_REX_SET_CONTROL_RESULT, rexUlp.controlFlags, operation, (usbControl == ULP_USB_CONTROL_SYSTEM_UP ? 1 : 0) );

    if (oldControlFlags != rexUlp.controlFlags)
    {
        // update the USB state machines with the current system status (up or down)
        ULP_RexUsb3Control(usbControl);
        ULP_RexUsb2Control(usbControl);

        // if we were ready, and now we are not, tell the other side
        if ( ((oldControlFlags & REX_USB_REX_ALLOWED_MASK) == REX_USB_REX_ALLOWED_MASK) &&
             ((rexUlp.controlFlags & REX_USB_REX_ALLOWED_MASK) != REX_USB_REX_ALLOWED_MASK) )
        {
            UlpSendCPUCommRexUsbMessage(ULP_REX_TO_LEX_REX_BUSY);  // tell the Lex we are busy
        }
    }
}

//#################################################################################################
// Sends the specified CPU message to the Lex
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
void UlpSendCPUCommRexUsbMessage(enum UlpRexSendUsbMessage rexMessage)
{
    CPU_COMM_sendSubType(CPU_COMM_TYPE_ULP_USB, rexMessage);
}


//#################################################################################################
// Sends the specified CPU message to the Lex
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
void UlpSendCPUCommRexUsb2Message(enum UlpRexSendUsb2Message rexMessage)
{
    CPU_COMM_sendSubType(CPU_COMM_TYPE_ULP_USB2, rexMessage);
}


//#################################################################################################
// Sends the specified CPU message to the Lex
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
void UlpSendCPUCommRexUsb3Message(enum UlpRexSendUsb3Message rexMessage)
{
    CPU_COMM_sendSubType(CPU_COMM_TYPE_ULP_USB3, rexMessage);
}


//#################################################################################################
// Sends the specified CPU message to the Lex
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
void UlpSendCPUCommRexUsb3ResetMessage(enum UlpRexSendUsb3ResetMessage rexMessage)
{
    CPU_COMM_sendSubType(CPU_COMM_TYPE_ULP_USB3_RESET, rexMessage);
}


//#################################################################################################
// Starts the programming of the Cypress hub
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UlpRexProgramCypressHub(void)
{
    // ok - starting to program the hub - make sure it is marked as down
    ULP_RexUsbControl(ULP_USB_REX_HUB_DOWN);

    // go do the hub programming cycle
    I2CD_CypressHubStartProgramming();
}

//#################################################################################################
// Turns on or off the vBus for USB2 & USB3 on the Rex
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpRexSetVbus(bool enable)
{
    // On the Rex, vBus is active if we say it is, or if USB2 or USB3 are active
    bool vBusRequest = enable || RexUlpUsb2IsActive() || RexUlpUsb3IsActive();

    if (rexUlp.flags.vBusDelayActive == false)
    {
        UlpHalRexVbusActive(vBusRequest);

        // if we are turning vBus off, wait a bit before we turn it back on again
        if ( rexUlp.flags.vBusState && (vBusRequest == false) )
        {
            rexUlp.flags.vBusDelayActive = true;
            TIMING_TimerStart(rexUlp.vBusWaitTimer);
            ULP_RexUsbControl(ULP_USB_REX_VBUS_DOWN);
        }
    }

    rexUlp.flags.vBusState = vBusRequest;
}


// Static Function Definitions ####################################################################

//#################################################################################################
// vBus delay timeout has expired - set the current value for vBus
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUlpVbusDelayTimeout(void)
{
    rexUlp.flags.vBusDelayActive = false;
    UlpRexSetVbus(rexUlp.flags.vBusState);
    ULP_RexUsbControl(ULP_USB_REX_VBUS_READY);
}


//#################################################################################################
// Rex Comlink send timeout has expired
//      request Lex's USB information once again
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexUlpComLinkSendTimeout(void)
{
    UlpSendCPUCommRexUsbMessage(ULP_REX_TO_LEX_REX_COMLINK_UP);
}


//#################################################################################################
// The handler for REX receiving a CPU message from the LEX.
//
// Parameters:
//      msg                     - message data
//      msgLength               - message length
// Return:
// Assumptions:
//      * The message length is always 1 byte.
//#################################################################################################
static void RexUlpCpuMsgEventHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    iassert_ULP_COMPONENT_1(msgLength == 0, ULP_REX_UNEXPECTED_CPU_MSG_LENGTH, msgLength);
    ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_REX_RCV_LEX_USB_MSG, subType);

    enum UlpLexSendUsbMessage lexUsbMessage = (enum UlpLexSendUsbMessage)subType;

    if ( (rexUlp.controlFlags & REX_ULP_REX_ENABLED) == 0)
    {
        UlpSendCPUCommRexUsbMessage(ULP_REX_TO_LEX_REX_USB_MODULE_DISABLED);  // tell the Lex we are not available
        return;
    }

    switch (lexUsbMessage)
    {
        case ULP_LEX_TO_REX_LEX_COMLINK_UP:
            TIMING_TimerStop(rexUlp.rexComLinkUpTimer);
            RexLinkUpMsgToLex();
            break;

        case ULP_LEX_TO_REX_LEX_USB_READY:     // Lex is ready to start USB operations
            // mark that our partner is ready
            ULP_RexUsbControl(ULP_USB_REX_DEVICE_READY);
            break;

        case ULP_LEX_TO_REX_LEX_BUSY:           // Lex is busy setting up USB
            break;

        default:
            break;
    }
 }


//#################################################################################################
// The ULP handler for the event of GE REX device disconnect.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpRexUsb2DevDisconnectHandler(void)
{
    ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_REX_GE_REX_DEV_DISCONN);
//    ULP_RexStateMachine(REX_ULP_EVENT_DEV_DISCONN_USB2);
}


//#################################################################################################
// The ULP handler for the event of GE REX device disconnect.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpRexUsb2DevConnectHandler(void)
{
    ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_REX_GE_REX_DEV_CONN);
//    ULP_RexStateMachine(REX_ULP_EVENT_DEV_CONN_USB2);
}



//#################################################################################################
// Rex Basic Mode Init.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void RexCypressHubInitComplete(void)
{
    // ok - hub is up - mark it as ready
    ULP_RexUsbControl(ULP_USB_REX_HUB_READY);
}


//#################################################################################################
// REX Configuration Event handler.
//
// Parameters: eventInfo
// Return:
// Assumptions:
//
//#################################################################################################
static void RexConfigurationEventHandler(uint32_t eventInfo, uint32_t userContext)
{
    if(eventInfo == CONFIG_VARS_BB_FEATURE_CONTROL)
    {
        UsbPortsEnabled usbPortsEnabled = ulp_GetUsbEnables();

        ULP_RexUsb2Control(usbPortsEnabled.usb2Enabled ? ULP_USB_CONTROL_ENABLE : ULP_USB_CONTROL_DISABLE);
        ULP_RexUsb3Control(usbPortsEnabled.usb3Enabled ? ULP_USB_CONTROL_ENABLE : ULP_USB_CONTROL_DISABLE);
    }
}

//#################################################################################################
// Handler for communication link up/down interrupts.
//
// Parameters:
// Return:
// Assumptions: Intended to be registered as a callback to be executed in the top-level ISR
//              for the specified interrupt.
//#################################################################################################
static void RexCommLinkEventHandler(uint32_t linkUp, uint32_t userContext)
{
    if (linkUp)
    {
        // if USB2 or USB3 is enabled, then the USB module is enabled
        if (ULP_RexUsb2Enabled() || ULP_RexUsb3Enabled())
        {
            UlpSendCPUCommRexUsbMessage(ULP_REX_TO_LEX_REX_COMLINK_UP);     // Ask for Lex to send USB message first before Rex sends to Lex
            TIMING_TimerStart(rexUlp.rexComLinkUpTimer);                    // Start timer to keep asking to Lex until it gets Lex info or link is down

            ULP_RexUsbControl(ULP_USB_REX_COMLINK_UP);
            ULP_RexUsbControl(ULP_USB_REX_ENABLE);                      // enable the USB system
        }
        else
        {
            ULP_RexUsbControl(ULP_USB_REX_DISABLE);                     // disable the USB system
        }

        ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_REX_COMLINK_UP_EVENT);
    }
    else
    {
        ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_REX_COMLINK_DOWN_EVENT);

        TIMING_TimerStop(rexUlp.rexComLinkUpTimer);

        // com channel down!
        ULP_RexUsbControl(ULP_USB_REX_COMLINK_DOWN);
        ULP_RexUsb3Control(ULP_USB_CONTROL_PARTNER_SHUTDOWN);
        ULP_RexUsb2Control(ULP_USB_CONTROL_PARTNER_SHUTDOWN);

        TOPLEVEL_clearPollingMask(SECONDARY_INT_ULP_INT_CORE_MSK);      // no polled interrupts; link is down
        UlpGeControl(false);                                            // Com link down - put GE into reset
        bb_top_ApplyResetXusb(true);
    }
}


//#################################################################################################
// Rex send linkup message to Lex
//
// Parameters:
// Return:
// Assumptions: Rex sends information after getting Lex link up info
//
//#################################################################################################
static void RexLinkUpMsgToLex(void)
{
    if (ULP_RexUsb2Enabled())
    {
        UlpSendCPUCommRexUsb2Message(ULP_REX_TO_LEX_USB2_MODULE_ENABLED);    // USB2 is enabled on this side
    }
    else
    {
        UlpSendCPUCommRexUsb2Message(ULP_REX_TO_LEX_USB2_MODULE_DISABLED);   // USB2 is disabled on this side
    }

    if (ULP_RexUsb3Enabled())
    {
        UlpSendCPUCommRexUsb3Message(ULP_REX_TO_LEX_USB3_MODULE_ENABLED);    // USB3 is enabled on this side
    }
    else
    {
        UlpSendCPUCommRexUsb3Message(ULP_REX_TO_LEX_USB3_MODULE_DISABLED);   // USB3 is disabled on this side
    }

    // if USB2 and USB3 is disabled, then the USB module is unavailable
    if (!ULP_RexUsb2Enabled() && !ULP_RexUsb3Enabled())
    {
        UlpSendCPUCommRexUsbMessage(ULP_REX_TO_LEX_REX_USB_MODULE_DISABLED);
    }
}

