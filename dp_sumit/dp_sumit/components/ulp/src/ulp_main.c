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
// This file contains common ULP (USB Link Partner) code for the Lex and Rex.
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
#include <interrupts.h>
#include <ulp.h>
#include "ulp_log.h"
#include "ulp_loc.h"
#include <configuration.h>      // used for USB enables

#include <uart.h>

// Constants and Macros ###########################################################################

// when this mask is set, the USB module can start
#define ULP_USB_ALLOWED_MASK            \
    (ULP_USB_MODULE_LOCAL_ENABLED         |   \
     ULP_USB_MODULE_PARTNER_ENABLED |   \
     ULP_USB_MODULE_PARTNER_READY   |   \
     ULP_USB_MODULE_SYSTEM_UP       |   \
     ULP_USB_MODULE_CHANNEL_UP      |   \
     ULP_USB_MODULE_RESET_DONE )

// Exported Function Definitions ##################################################################

// Component Variables ############################################################################

// Static Variables ###############################################################################

UlpStatusCallback updateFunction;   // Callback to use when reporting status changes

// Global Variables ###############################################################################

//#################################################################################################
// Initialize the LEX ULP module and the ULP finite state machine.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void ULP_UlpInit(UlpStatusCallback statusFunction)
{
    const bool isLex = bb_top_IsDeviceLex();

    ilog_ULP_COMPONENT_0(ILOG_DEBUG, ULP_INIT_CALLED);

    updateFunction = statusFunction;

    _ULP_HalInit();

    ULP_StatInit();

    if (isLex)
    {
        ULP_LexInit();
    }
    else
    {
        ULP_RexInit();
    }

    ULP_GeControlInit();    // initialize common GE code for Lex/Rex
}


//#################################################################################################
// The ULP ISR registered in the LEON's secondary interrupt handlers table.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ULP_UlpISR(void)
{
    bb_top_IsDeviceLex() ? ULP_LexUlpISR() : ULP_RexUlpISR();
}

//#################################################################################################
// The XUSB ISR registered in the LEON's secondary interrupt handlers table.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ULP_Xusb3ISR(void)
{
//    bb_top_IsDeviceLex() ? ULP_LexUlpISR() : ULP_RexUlpISR();
}



// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Sets the given control byte with the result of the control operation.  Returns TRUE if
// the usb module is enabled.  Common between the Lex and Rex
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
enum UlpUsbControlResult ULP_UsbSetControl(uint8_t *usbControl, enum UlpUsbControl controlOperation)
{
    uint8_t control = *usbControl;

    enum UlpUsbControlResult result = ULP_USB_CONTROL_RESULT_UNCHANGED;  // ASSUME there was no change in USB operation

    enum UlpUsbControlResult oldResult =
        ULP_UsbControlEnabled(control ) ? ULP_USB_CONTROL_RESULT_ENABLE : ULP_USB_CONTROL_RESULT_DISABLE;

    switch (controlOperation)
    {
        case ULP_USB_CONTROL_DISABLE:    // disable this USB control
            control &= ~ULP_USB_MODULE_LOCAL_ENABLED;
            break;

        case ULP_USB_CONTROL_ENABLE:     // enable this USB control
            control |= ULP_USB_MODULE_LOCAL_ENABLED;
            break;

        case ULP_USB_CONTROL_PARTNER_SHUTDOWN:    // the other side is disabled
            control &= ~(ULP_USB_MODULE_PARTNER_ENABLED | ULP_USB_MODULE_PARTNER_READY);
            break;

        case ULP_USB_CONTROL_PARTNER_ENABLED:     // the other side is enabled
            control |= ULP_USB_MODULE_PARTNER_ENABLED;
            break;

        case ULP_USB_CONTROL_PARTNER_READY:      // the other side is ready, waiting for a connection
            control |= ULP_USB_MODULE_PARTNER_READY;
            break;

        case ULP_USB_CONTROL_PARTNER_DISABLED:   // the other side is disabled
            control &= ~ULP_USB_MODULE_PARTNER_READY;
            break;

        case ULP_USB_CONTROL_SYSTEM_UP:          // system is up and ready
            control |= ULP_USB_MODULE_SYSTEM_UP;
            break;

        case ULP_USB_CONTROL_SYSTEM_DOWN:
            // system is down, not connected (we will get a connect message when we come back up)
            control &= ~ULP_USB_MODULE_SYSTEM_UP;
            break;

        case ULP_USB_CONTROL_MCA_CHANNEL_RDY:  // MCA channel active
            control |= ULP_USB_MODULE_CHANNEL_UP;
            break;

        case ULP_USB_CONTROL_MCA_CHANNEL_DOWN:   // MCA channel not available
            control &= ~(ULP_USB_MODULE_CHANNEL_UP);
            break;

        case ULP_USB_CONTROL_RESET_ACTIVE:       // reset in progress
            control &= ~ULP_USB_MODULE_CHANNEL_UP;
            control &= ~ULP_USB_MODULE_RESET_DONE;
            break;

        case ULP_USB_CONTROL_RESET_DONE:         // reset done
            control |=  ULP_USB_MODULE_RESET_DONE;
            break;

        default:
            break;
    }

    if (*usbControl != control)
    {
        *usbControl = control;

        result = ULP_UsbControlEnabled(control ) ? ULP_USB_CONTROL_RESULT_ENABLE : ULP_USB_CONTROL_RESULT_DISABLE;

        if (result == oldResult)
        {
            result = ULP_USB_CONTROL_RESULT_UNCHANGED;
        }
    }

    return (result);
}

bool ULP_UsbControlEnabled(const uint8_t usbControl)
{
    if ( (usbControl & ULP_USB_ALLOWED_MASK) == ULP_USB_ALLOWED_MASK)
    {
        return (true);
    }

    return (false);
}

//#################################################################################################
// Called whenever ULP's status changes
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UlpStatusChange(enum UlpStatus status)
{
    // a callback is used here to avoid a circular dependency between UPP and ULP
    updateFunction(status);

}

//#################################################################################################
// Returns whether the USB2 system is enabled
//
// Parameters:
// Return: true if enabled, false otherwise
// Notes: Until both sides have received the other side's current status, this function
//        may return FALSE incorrectly
//
//#################################################################################################
bool ULP_Usb2SystemEnabled(void)
{
    return(bb_top_IsDeviceLex() ? ULP_LexUsb2SystemEnabled() : ULP_RexUsb2SystemEnabled());
}


//#################################################################################################
// NVM based initialization
// Parameters:
// Return:
// Assumptions: Reads a byte from address 0xC0B00000
//#################################################################################################
UsbPortsEnabled ulp_GetUsbEnables(void)
{
    UsbPortsEnabled usbEnabled = {0};

    ConfigBlocksEnable *blocksEnabled =  &(Config_GetBuffer()->featureControl);
    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_CONTROL, blocksEnabled))
    {
        usbEnabled.usb2Enabled = blocksEnabled->USBcontrol  & (1 << CONFIG_BLOCK_ENABLE_USB_USB2);
        usbEnabled.usb3Enabled = blocksEnabled->USBcontrol  & (1 << CONFIG_BLOCK_ENABLE_USB_USB3);
    }

    return (usbEnabled);  // usually I don't return a structure, but it's only 2 bytes
}

//#################################################################################################
// NVM based initialization
// Parameters:
// Return:
// Assumptions: Reads a byte from address 0xC0B00000
//#################################################################################################
bool ulp_Usb3ResetOnDisconnect(void)
{
    bool result = false;
    ConfigUsbConfig *usbConfig =  &(Config_GetBuffer()->usbConfig);
    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_USB_CONFIG, usbConfig))
    {
        result = (usbConfig->usb3Config & ULP_BIT_MASK(CONFIG_USB3_RESET_USB_ON_DISCONNECT));
    }

    return (result);  // true if we should reset on disconnect
}

