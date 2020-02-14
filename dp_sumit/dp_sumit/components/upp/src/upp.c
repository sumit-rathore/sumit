///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2018
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  -  upp.c
//
//!   @brief -  Contains the code for the USB Protocol layer Partner (UPP) project:
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################

#include <ibase.h>
#include <bb_top.h>
#include <upp_regs.h>
#include <interrupts.h>
#include <configuration.h>
#include <ulp.h>
#include <upp.h>
#include "upp_loc.h"
#include "upp_log.h"

#include <uart.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################

static void UppUlpSetStatus(enum UlpStatus status);
static void UppUlpProcessStatus(enum UlpStatus status);

// Static Variables ###############################################################################

static struct
{
    bool active;   // TRUE if UPP is active; false otherwise

} uppContext;


// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize the UPP component
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_Init(void)
{
    uppContext.active = false;

    UPP_HalInit();

    ULP_UlpInit(UppUlpSetStatus); // Initialize ULP; startup GE (USB 2.0)

#ifdef BLACKBIRD_ISO
    const bool isLex = bb_top_IsDeviceLex();


    UPP_StatInit();

    if (isLex)
    {
        UPP_LexInit();  
    }
    else
    {
        UPP_RexInit();
    }
    
    if (UppIsIsoEnabled())
    {
        ILOG_istatus(ISO_ENABLED, 0);
    }
    else
    {
        ILOG_istatus(ISO_DISABLED, 0);

    }
#endif
    ilog_UPP_COMPONENT_0(ILOG_MINOR_EVENT, UPP_INIT);
}

//#################################################################################################
// The UPP ISR registered in the LEON's secondary interrupt handlers table.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UPP_UppISR(void)
{
#ifdef BLACKBIRD_ISO
    bb_top_IsDeviceLex() ? UPP_LexUppISR() : UPP_RexUppISR();
#endif
}


// Component Scope Function Definitions ###########################################################

//#################################################################################################
// returns true if UPP is active
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool UppIsIsoEnabled(void)
{
#ifdef BLACKBIRD_ISO
    ConfigUsbExtendedConfig *usbExtendedConfig =  &(Config_GetBuffer()->usbExtendedConfig);
    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_USB_EXTENDED_CONFIG, usbExtendedConfig))
    {
        const bool isoEnabled = usbExtendedConfig->UPPcontrol & (1 << CONFIG_ENABLE_ISO_UPP);

        ilog_UPP_COMPONENT_1(ILOG_MINOR_EVENT, IS_UPP_ISO_ENABLE_STATUS, isoEnabled);

        return(isoEnabled);
    }

    return false;
#else
    return false;
#endif
    }

//#################################################################################################
// returns true if the control transfer part of UPP is active
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool UppControlTransferIsEnabled(void)
{
    ConfigUsbExtendedConfig *usbExtendedConfig =  &(Config_GetBuffer()->usbExtendedConfig);
    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_USB_EXTENDED_CONFIG, usbExtendedConfig))
    {
        const bool uppControlTransferEnable = usbExtendedConfig->UPPcontrol & (1 << CONFIG_ENABLE_CONTROL_TRANSFER);

        ilog_UPP_COMPONENT_1(ILOG_MINOR_EVENT, IS_UPP_CONTROL_TRANSFER_ENABLED, uppControlTransferEnable);

        return(uppControlTransferEnable);
    }

    return true;
}

//#################################################################################################
// Enable UPP
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_EnableIso(void)
{
    // uppBypassed = false;
    ConfigUsbExtendedConfig *usbExtendedConfig =  &(Config_GetBuffer()->usbExtendedConfig);
    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_USB_EXTENDED_CONFIG, usbExtendedConfig))
    {
        usbExtendedConfig->UPPcontrol |= (1 << CONFIG_ENABLE_ISO_UPP) | (1 << CONFIG_ENABLE_CONTROL_TRANSFER);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_USB_EXTENDED_CONFIG, usbExtendedConfig);
    }
    ilog_UPP_COMPONENT_0(ILOG_USER_LOG, UPP_ICMD_ENABLED);
}

//#################################################################################################
// Disable UPP
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_DisableIso(void)
{
    // uppBypassed = true;
    ConfigUsbExtendedConfig *usbExtendedConfig =  &(Config_GetBuffer()->usbExtendedConfig);
    if (Config_ArbitrateGetVar(CONFIG_VAR_BB_USB_EXTENDED_CONFIG, usbExtendedConfig))
    {
        usbExtendedConfig->UPPcontrol &= ~((1 << CONFIG_ENABLE_ISO_UPP) | (1 << CONFIG_ENABLE_CONTROL_TRANSFER));
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_BB_USB_EXTENDED_CONFIG, usbExtendedConfig);
    }
    ilog_UPP_COMPONENT_0(ILOG_USER_LOG, UPP_ICMD_DISABLED);
}


// Static Function Definitions ####################################################################

//#################################################################################################
// The callback when ULP's status changes
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UppUlpSetStatus(enum UlpStatus status)
{
    ilog_UPP_COMPONENT_1(ILOG_MINOR_EVENT, UPP_ULP_STATUS, status);

    switch (status)
    {
        case ULP_STATUS_IN_RESET:          // USB3 is disabled
        case ULP_STATUS_ENABLED:             // At U0
        case ULP_STATUS_INBAND_HOT_RESET:  // a hot reset is in progress
            UppUlpProcessStatus(status);
            break;

        case ULP_STATUS_INBAND_WARM_RESET: // a warm reset is in progress
        case ULP_STATUS_LTSSM_DISABLED:       // went to SS_DISABLED, at least briefly
            UppUlpProcessStatus(ULP_STATUS_INBAND_HOT_RESET);
//            // for these 2 status changes, cycle the UPP component (disable and then enable)
//            // to clear out any erroneous data
//            UppUlpProcessStatus(ULP_STATUS_IN_RESET);
//            UppUlpProcessStatus(ULP_STATUS_ENABLED);
            break;

        default:
            break;
    }
}

//#################################################################################################
// The callback when ULP's status changes
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UppUlpProcessStatus(enum UlpStatus status)
{
    switch (status)
    {
        case ULP_STATUS_IN_RESET:            // USB3 is disabled
            uppContext.active = false;

            if (UppIsIsoEnabled())
            {
                // disable ISO
                TOPLEVEL_clearPollingMask(SECONDARY_INT_UPP_INT_MSK);  // no more UPP interrupts
                Upp_StatMonControl(false);
                UppHalUppEnable(false);

                if (bb_top_IsDeviceLex())
                {
                    UppQueueManagementDisable();                }
                else
                {
                    UPP_RexReinit();
                }
            }
            else
            {
                UppHalUppBypassed(false); // make sure UPP and XUSB3 buffers are reset and bypassed
            }
            break;

        case ULP_STATUS_ENABLED:          // At U0
            if (uppContext.active == false)
            {
                uppContext.active = true;

                if (UppIsIsoEnabled())
                {
                    // enable ISO
                    UppHalUppEnable(true);
                    Upp_StatMonControl(true);

                    if (bb_top_IsDeviceLex())
                    {
                        LexQueueStateSendEventWithNoData(UPP_QUEUE_ENABLE);
                    }
                }
                else
                {
                    UppHalUppBypassed(true);    // make sure UPP and XUSB3 buffers are active and bypassed
                }
            }
            break;

        case ULP_STATUS_INBAND_HOT_RESET:   // a hot reset is in progress
            if (uppContext.active)
            {
                if (UppIsIsoEnabled())
                {
                    UppHalUppRestart();

                    if (bb_top_IsDeviceLex())
                    {
                        // clear out the old information
                        LexQueueStateSendEventWithNoData(UPP_QUEUE_DISABLE);
                        LexQueueStateSendEventWithNoData(UPP_QUEUE_ENABLE);
                    }
                    else
                    {
                        UPP_RexReinit();
                    }
                }
            }
            break;

        // shouldn't get here!
        case ULP_STATUS_INBAND_WARM_RESET: // a warm reset is in progress
        case ULP_STATUS_LTSSM_DISABLED:       // went to SS_DISABLED, at least briefly
        default:
            break;
    }
}

