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
//!   @file  -  state.c
//
//!   @brief -  keeps track of all state for the rexulm component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "rexulm_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Gloval Variable *******************************/

// NOTE: this is declared extern in rexulm_loc.h
struct rexulmState rex __attribute__ ((section (".rexbss")));

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: _REXULM_UpdateSystemState()
*
* @brief  - Update any state related information
*
* @return - void
*
* @note   - If anything needs to be set (ie LED's) based on the current state,
*           this function will handle it
*
*           Intended to be a tail call of any rexulm event handler
*
*/
void _REXULM_UpdateSystemState(void)
{
    // If the downstream is no longer suspending, stop looking for SOF packets
    if (rex.downstreamPort != SUSPENDING)
    {
        TASKSCH_StopTask(rex.suspendingIdleTask);
    }

    // If the downstream is no longer bus resetting, stop looking for SOF packets
    if (rex.downstreamPort != BUS_RESETTING)
    {
        TASKSCH_StopTask(rex.busResettingIdleTask);
    }

    // check to see if any host messages set the state of the host
    // but we aren't connected.  This happens when our disconnect message
    // passes the host message on the wire.
    if ((rex.downstreamPort == DISCONNECTED) || (!rex.lexLinkUp))
    {
        rex.upstreamPort = DISCONNECTED;
    }

    // If the upstream is no longer operating, stop looking for SOF packets
    if (rex.upstreamPort != OPERATING)
    {
        TASKSCH_StopTask(rex.sofIdleTask);
        rex.sofSynced = FALSE;
    }

    // Set system state (ie LED's)
    if (rex.upstreamPort == DISCONNECTED)
    {
#ifdef GOLDENEARS
        GRG_TurnOffLed(LI_LED_SYSTEM_HOST);
        GRG_TurnOffLed(LI_LED_SYSTEM_ACTIVITY);
#else
        if (rex.lexLinkUp)
        {
            // The Rex-Lex link is up, which indicates the Host LED is under control of the Rex ULM
            GRG_GpioClear(GPIO_OUT_LED_HOST);
        }
        else
        {
            // If the Link Manager is using the Host LED (ie to blink for version mismatches)
            // don't change the setting of the Host LED
        }
        GRG_GpioClear(GPIO_OUT_LED_ACTIVITY);
#endif
    }
    else
    {
        if (rex.downstreamPort == SUSPENDED)
        {
#ifdef GOLDENEARS
            GRG_ToggleLed(LI_LED_SYSTEM_HOST, LPR_SLOW);

#else
            GRG_GpioPulse(GPIO_OUT_LED_HOST);
#endif
        }
        else
        {
            // NOTE:    This is different between the Lex & Rex for a resume
            //          On the Lex a resume has the LED_HOST still pulsing
            //          On the Rex a resume has the LED_HOST active
            //          This allows tech support to diagnose an infinite resume
            GRG_TurnOnLed(LI_LED_SYSTEM_HOST);
        }

        if (rex.downstreamPort == OPERATING)
        {
#ifdef GOLDENEARS
            GRG_ToggleLed(LI_LED_SYSTEM_ACTIVITY, LPR_SLOW);
#else
            GRG_GpioPulse(GPIO_OUT_LED_ACTIVITY);
#endif
        }
        else
        {
#ifdef GOLDENEARS
            GRG_TurnOffLed(LI_LED_SYSTEM_ACTIVITY);
#else
            GRG_GpioClear(GPIO_OUT_LED_ACTIVITY);
#endif
        }
    }

    // Log the current state if the current logging level is high enough
    // NOTE: We do the check here, so in the normal case (no logging),
    //       there is no need to _REXULM_logCurrentState in IRAM
    if (ilog_GetLevel(REXULM_COMPONENT) <= ILOG_MINOR_EVENT)
    {
        _REXULM_logCurrentState(ILOG_MINOR_EVENT);
    }
}


/**
* FUNCTION NAME: _REXULM_logCurrentState()
*
* @brief  - logs the current state
*
* @return - void
*
* @note   - intended to be a tail call of its caller
*/
void _REXULM_logCurrentState
(
    ilogLevelT logLevel   // The ilog level to log at
)
{
    // Log our current state
    ilog_REXULM_COMPONENT_0(logLevel, rex.lexLinkUp ? LEX_REX_LINK_UP : LEX_REX_LINK_DOWN);
    ilog_REXULM_COMPONENT_2(logLevel, REX_SPEED, rex.devSpeed, rex.opSpeed);
    ilog_REXULM_COMPONENT_3(logLevel, REX_DEV_HOST_STATE, rex.downstreamPort, rex.upstreamPort, rex.everEnumerated);
#if defined(LIONSGATE) && !defined(REXULM_NO_PREFETCH_DEVICE_SPEED)
    ilog_REXULM_COMPONENT_1(logLevel, REX_PORTDISCHARGECOMPLETE, rex.lg1HubDownstreamPortPowerDischargeComplete);
#endif
}


/**
* FUNCTION NAME: REXULM_getUpstreamPortState()
*
* @brief  - returns uint8 of upstream port state
*
* @return - void
*
* @note   - 
*/
uint8 REXULM_getUpstreamPortState(void)
{
    return rex.upstreamPort;
}


/**
* FUNCTION NAME: REXULM_getDownstreamPortState()
*
* @brief  - returns uint8 of downstream port state
*
* @return - void
*
* @note   - 
*/
uint8 REXULM_getDownstreamPortState(void)
{
    return rex.downstreamPort;
}


/**
* FUNCTION NAME: REXULM_getUsbOpSpeed()
*
* @brief  - returns uint8 of operating speed
*
* @return - void
*
* @note   - 
*/
uint8 REXULM_getUsbOpSpeed(void)
{
    return rex.opSpeed;
}


/**
* FUNCTION NAME: REXULM_getUsbDevSpeed()
*
* @brief  - returns uint8 of device speed
*
* @return - void
*
* @note   - 
*/
uint8 REXULM_getUsbDevSpeed(void)
{
    return rex.devSpeed;
}




