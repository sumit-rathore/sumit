//#################################################################################################
// Icron Technology Corporation - Copyright 2015
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
// Configuration of jitter chip
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <ulp.h>
#include "ulp_log.h"
#include "ulp_loc.h"
#include <configuration.h>


// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// ICMD Enable USB2
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ULP_enableUsb2(void)
{
    ConfigBlocksEnable *blocksEnabled =  &(Config_GetBuffer()->featureControl);
    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled))
    {
        blocksEnabled->USBcontrol |= (1 << CONFIG_BLOCK_ENABLE_USB_USB2);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled);
    }
}


//#################################################################################################
// Icmd Disable USB2
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ULP_disableUsb2(void)
{
    ConfigBlocksEnable *blocksEnabled =  &(Config_GetBuffer()->featureControl);
    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled))
    {
        blocksEnabled->USBcontrol &= ~(1 << CONFIG_BLOCK_ENABLE_USB_USB2);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled);
    }
}

//#################################################################################################
// ICMD Enable USB3
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ULP_enableUsb3(void)
{
    ConfigBlocksEnable *blocksEnabled =  &(Config_GetBuffer()->featureControl);
    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled))
    {
        blocksEnabled->USBcontrol |= (1 << CONFIG_BLOCK_ENABLE_USB_USB3);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled);
    }
}


//#################################################################################################
// Icmd Disable USB2
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ULP_disableUsb3(void)
{
    ConfigBlocksEnable *blocksEnabled =  &(Config_GetBuffer()->featureControl);
    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled))
    {
        blocksEnabled->USBcontrol &= ~(1 << CONFIG_BLOCK_ENABLE_USB_USB3);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled);
    }
}

//#################################################################################################
// ICMD Enable USB3
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ULP_enableUsb3ResetOnDisconnect(void)
{
    ConfigUsbConfig *usbConfig =  &(Config_GetBuffer()->usbConfig);
    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_USB_CONFIG, usbConfig))
    {
        usbConfig->usb3Config |= ULP_BIT_MASK(CONFIG_USB3_RESET_USB_ON_DISCONNECT);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_USB_CONFIG, usbConfig);
    }
}


//#################################################################################################
// Icmd Disable USB2
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void ULP_disableUsb3ResetOnDisconnect(void)
{
    ConfigUsbConfig *usbConfig =  &(Config_GetBuffer()->usbConfig);
    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_USB_CONFIG, usbConfig))
    {
        usbConfig->usb3Config &= ~ULP_BIT_MASK(CONFIG_USB3_RESET_USB_ON_DISCONNECT);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_USB_CONFIG, usbConfig);
    }
}


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################

