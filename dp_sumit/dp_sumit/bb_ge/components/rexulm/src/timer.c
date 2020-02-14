///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010-2012
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
//!   @file  -  timer.c
//
//!   @brief -  handles the rexulm callback timer
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "rexulm_loc.h"
#ifdef GOLDENEARS
#include <storage_Data.h>
#endif

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: _REXULM_ConnectDebounceTimerHandler()
*
* @brief  - Callback timer function for the 100ms after receiving the connect ISR
*
* @return - void
*
* @note   - The 100ms is to allow the device to power up. Defined by USB Spec
*
*/
#ifndef REXULM_NO_PREFETCH_DEVICE_SPEED
void _REXULM_ConnectDebounceTimerHandler(void)
{
    boolT usbHsSupported;
    _REXULM_MarkTime(TIME_MARKER_TIMER_PREFETCH);

    // This can only happen when the timer from debouncing a connect has occured
    iassert_REXULM_COMPONENT_0(rex.downstreamPort == DISCONNECTED, DEVICE_NOT_DISCONNECTED_WHEN_TIMER_EXP);

    ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, DEVICE_CONNECTED_AND_NOW_PREFETCHING_SPEED);

    rex.devSpeed = USB_SPEED_INVALID;
    rex.opSpeed = USB_SPEED_INVALID;
    rex.downstreamPort = BUS_RESETTING;
    rex.upstreamPort = DISCONNECTED;
    rex.everEnumerated = FALSE;
    rex.needsReset = FALSE;
    // negotiate Speed (Auto)
    // generate bus reset (REX)
    usbHsSupported =
#ifdef GOLDENEARS
        ULM_usb2HighSpeedEnabled();
#else // LG1 case
        GRG_IsHSJumperSelected();
#endif
        ULM_GenerateRexUsbReset(usbHsSupported ? USB_SPEED_INVALID : USB_SPEED_FULL);

#ifdef GOLDENEARS
    ULM_EnableSuspendDetect();
#endif

    _REXULM_UpdateSystemState();
}
#endif


/**
* FUNCTION NAME: _REXULM_ConnectUsbPortTimerHandler()
*
* @brief  - Timer function that enables the downstream USB port
*
* @return - void
*
* @note   - This is called once we are sure enough time has passed to drain the
*           downstream VBus to properly reset devices
*
*           On GE there is a VBusEnable pin to the RexHub
*           This is not active until this function is called
*           Specifically ULM_ConnectRexUsbPort() is called
*
*           On LG1 VBus is always hardwired, so this will have little effect
*
*/
void _REXULM_ConnectUsbPortTimerHandler(void)
{
    _REXULM_MarkTime(TIME_MARKER_TIMER_CONNECT);

//    _REXULM_flushQueues(FALSE);
    ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, ENABLING_USB_PORT);

    // Start the USB
    GRG_GpioSet(GPIO_OUT_REX_VBUS_ENABLE);
    ULM_ConnectRexUsbPort();
}

/**
* FUNCTION NAME: _REXULM_lg1_Hub_DownstreamPortPowerDischargeTimer
*
* @brief  - LG1 only: Timer function to allow enough time to drain the downstream ports
*           of the Rex hub, before initiating link to the LEX.
*
* @return - void
*
* @note   - This is called once we are sure enough time has passed to drain the
*           downstream port power to properly reset devices
*
*/
#if defined(LIONSGATE) && !defined(REXULM_NO_PREFETCH_DEVICE_SPEED)
void _REXULM_lg1_Hub_DownstreamPortPowerDischargeTimer(void)
{
    rex.lg1HubDownstreamPortPowerDischargeComplete = TRUE;
    if (rex.lexLinkUp)
    {
        _REXULM_SendHostDeviceConnect();
    }
    ilog_REXULM_COMPONENT_0(ILOG_MAJOR_EVENT, EXPIRE_REX_DFP_DISCHARGE_TIMER);
}
#endif


/**
* FUNCTION NAME: _REXULM_UlmDisconnectDisableTimerHandler()
*
* @brief  - Timer function that disables the ULM in event of ULM disconnect
*
* @return - void
*
* @note   - This is called once we are sure enough time has passed to allow EOP
*           to be transmitted to the downstream USB devices from the ULM before
*           disabling the ULM
*
*/
void _REXULM_DevConnDebounceTimerHandler(void)
{
    // check we haven't changed state
    if (ULM_CheckRexConnect())
    {
        _REXULM_UlmConnect();
    }
    else
    {
        _REXULM_UlmDisconnect();
    }
}


