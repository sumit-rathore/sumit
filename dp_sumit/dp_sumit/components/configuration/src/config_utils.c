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
//
// Miscellaneous configuration functions
//
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################

#include <ibase.h>
#include <configuration.h>
#include "configuration_log.h"
#include "configuration_loc.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// Shows the current stored feature configuration
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void Config_ShowFeatureControlStatus(void)
{
    ConfigBlocksEnable *blocksEnabled =  &(Config_GetBuffer()->featureControl);
    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_CONTROL, blocksEnabled))
    {
        const bool usb2enable = blocksEnabled->USBcontrol   & (1 << CONFIG_BLOCK_ENABLE_USB_USB2);
        const bool usb3enable = blocksEnabled->USBcontrol   & (1 << CONFIG_BLOCK_ENABLE_USB_USB3);
        const bool dpEnable   = blocksEnabled->DPcontrol    & (1 << CONFIG_BLOCK_ENABLE_DP);
        const bool lanEnable  = blocksEnabled->MiscControl  & (1 << CONFIG_BLOCK_ENABLE_GMII);
        const bool rs232Enable = blocksEnabled->MiscControl & (1 << CONFIG_BLOCK_ENABLE_RS232);

        ilog_CONFIG_COMPONENT_0(ILOG_USER_LOG, usb2enable  ? NVM_CONFIG_STATUS_USB2_ON    : NVM_CONFIG_STATUS_USB2_OFF);
        ilog_CONFIG_COMPONENT_0(ILOG_USER_LOG, usb3enable  ? NVM_CONFIG_STATUS_USB3_ON    : NVM_CONFIG_STATUS_USB3_OFF);
        ilog_CONFIG_COMPONENT_0(ILOG_USER_LOG, dpEnable    ? NVM_CONFIG_STATUS_DP_ON      : NVM_CONFIG_STATUS_DP_OFF);
        ilog_CONFIG_COMPONENT_0(ILOG_USER_LOG, lanEnable   ? NVM_CONFIG_STATUS_LANPORT_ON : NVM_CONFIG_STATUS_LANPORT_OFF);
        ilog_CONFIG_COMPONENT_0(ILOG_USER_LOG, rs232Enable ? NVM_CONFIG_STATUS_RS232_ON   : NVM_CONFIG_STATUS_RS232_OFF);
    }

    // print out the ISO enables as well
    ConfigUsbExtendedConfig *usbExtendedConfig =  &(Config_GetBuffer()->usbExtendedConfig);
    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_USB_EXTENDED_CONFIG, usbExtendedConfig))
    {
        const bool isoEnabled               = usbExtendedConfig->UPPcontrol & (1 << CONFIG_ENABLE_ISO_UPP);
        const bool uppControlTransferEnable = usbExtendedConfig->UPPcontrol & (1 << CONFIG_ENABLE_CONTROL_TRANSFER);

        ilog_CONFIG_COMPONENT_0(ILOG_USER_LOG, isoEnabled  ?               NVM_CONFIG_STATUS_ISO_ON      : NVM_CONFIG_STATUS_ISO_OFF);
        ilog_CONFIG_COMPONENT_0(ILOG_USER_LOG, uppControlTransferEnable  ? NVM_CONFIG_STATUS_ISO_CTRL_ON : NVM_CONFIG_STATUS_ISO_CTRL_OFF);
    }
}

// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################

