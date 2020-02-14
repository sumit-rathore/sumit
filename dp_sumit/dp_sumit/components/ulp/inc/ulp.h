//#################################################################################################
// Icron TechnologyUSB Corporation - Copyright 2017
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef ULP_H
#define ULP_H

// Includes #######################################################################################
#include <ibase.h>

// Constants and Macros ###########################################################################

enum UlpStatus
{
    ULP_STATUS_IN_RESET,       // USB3 is disabled
    ULP_STATUS_ENABLED,          // USB3 is out of module reset
    ULP_STATUS_INBAND_HOT_RESET,   // a hot reset is in progress
    ULP_STATUS_INBAND_WARM_RESET,  // a warm reset is in progress
    ULP_STATUS_LTSSM_DISABLED,        // went to SS_DISABLED, at least briefly
};


// Data Types #####################################################################################
// Callback to use when reporting the ULP status changes
typedef void (*UlpStatusCallback)(enum UlpStatus status);

// enables separation of USB2 and USB3 for testing
enum UsbOptions
{
    USB_OPTION_USB2_ONLY,       // GE only
    USB_OPTION_USB3_ONLY,       // USB3 only, don't even take GE out of reset
    USB_OPTION_USB2_AND_USB3,   // USB3 first then USB2
};

// structure used to return whether USB2 and USB3 are on
typedef struct
{
    bool usb2Enabled;       // true if USB2 is enabled
    bool usb3Enabled;       // true if USB3 is enabled
} UsbPortsEnabled;

// Function Declarations ##########################################################################
void ULP_UlpInit(UlpStatusCallback updateFunction) __attribute__ ((section(".atext")));
void ULP_UlpISR(void);
void ULP_Xusb3ISR(void);
void ULP_LexUlpISR(void);
void ULP_RexUlpISR(void);
void ULP_LexVbusDetectIntHandler(void);
void ULP_rexShutdownUsb3(enum UsbOptions option) __attribute__ ((section(".atext")));
UsbPortsEnabled ulp_GetUsbEnables(void) __attribute__((section(".atext")));

#endif // ULP_H
