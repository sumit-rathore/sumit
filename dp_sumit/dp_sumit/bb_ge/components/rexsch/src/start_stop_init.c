///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  - start_stop_init.c
//
//!   @brief - Handles starting/stopping/initialization of the Rex Scheduler
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "rexsch_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void _REXSCH_Enable(void) __attribute__((section(".rextext")));

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: REXSCH_Init
*
* @brief  - enables interrupts handlers for this
*
* @return -
*
* @note   -
*
*/

void REXSCH_Init(void)
{
    // Sane defaults
    ilog_SetLevel(ILOG_MAJOR_EVENT, REXSCH_COMPONENT);

   // install rex scheduler interrupt handler
   LEON_InstallIrqHandler(IRQ_XLR0_XRR0, &REXSCH_HandleIrq);
   LEON_EnableIrq(IRQ_XLR0_XRR0);

   REXSCH_ResetState();
}


// Icmd function
void REXSCH_Enable
(
    uint8 usbSpeed
)
{
    _REXSCH_UsbSpeed = CAST(usbSpeed, uint8, enum UsbSpeed);

    _REXSCH_Enable();
}

/**
* FUNCTION NAME: _REXSCH_Enable()
*
* @brief  - Enable Rex Scheduler
*
* @return - none
*
* @note   -
*
*/
static void _REXSCH_Enable(void)
{
   XCSR_XUSBXurmXutmEnable();
   XRR_rexEnable();

   ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_EVENT, ENABLE);
}


boolT REXSCH_LexResumeDone(void)
{
    // Start SOF Synchronization
    XRR_SOFSync();
    return FALSE; // wait until an SOF packet is received
}


void REXSCH_RexResumeDone(void)
{
    // Start Rex Scheduler
    _REXSCH_Enable();
}



boolT REXSCH_LexBusResetDone(void)
{
    // Start SOF Synchronization
    XRR_SOFSync();
    return FALSE; // wait until an SOF packet is received
}


boolT REXSCH_RexBusResetNeg
(
    enum UsbSpeed usbSpeed
)
{
    _REXSCH_UsbSpeed = usbSpeed;
    return FALSE; // This should have no affect on extending bus resets
}


void REXSCH_RexBusResetDone(void)
{
    // Start Rex Scheduler
    _REXSCH_Enable();
}


boolT REXSCH_LexSofReceived(void)
{
    return TRUE; // Stop extending bus reset or resume
}



/**
* FUNCTION NAME: REXSCH_Disable ()
*
* @brief - Disable the REx scheduler
*
* @param - None
*
* @return - None
*
* @note -
*
*
*/
void REXSCH_Disable(void)
{
   XRR_rexDisable();
   XCSR_XUSBXurmXutmDisable();

   ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_EVENT, DISABLE);
   _REXSCH_sofTxCount = 0;
}


/**
* FUNCTION NAME: REXSCH_ResetState()
*
* @brief  - Reset internal state in RexSch
*
* @return - void
*
* @note   - RexSch should be disabled before calling this
*
*           Intended to be called on a bus reset, & not on a suspend
*/
void REXSCH_ResetState(void)
{
   REXMSA_ResetMsaStatus();
}


uint8 REXSCH_getTransmittedSofCount(void)
{
    return _REXSCH_sofTxCount;
}


/**
* FUNCTION NAME: REXSCH_forceSofTx()
*
* @brief  - Ensure SOF packets are transmitted, regardless of synchronization with host
*
* @return - void
*
* @note   - This helps work around buggy devices
*
*/
void REXSCH_forceSofTx(void)
{
    if (_REXSCH_sofTxCount == 0)
    {
        XCSR_XUSBXurmXutmEnable();
        XRR_SOFEnable();
    }
}

/**
* FUNCTION NAME: REXSCH_forceSofTxSetSpd()
*
* @brief  - Ensure SOF packets are transmitted, regardless of synchronization with host
*
* @return - void
*
* @note   - This helps work around buggy devices
*
*/
void REXSCH_forceSofTxSetSpd(enum UsbSpeed spd)
{
    if (_REXSCH_sofTxCount == 0)
    {
        // we need to ensure XURM is enabled because of state machines
        // that depend upon a response from an output packet
        // by keeping the XURM on, an internal timer will reset the state
        // machine preventing a lockup, failure to keep XURM on the state
        // machine timer won't trigger and won't prevent the lockup
        XCSR_XUSBXurmXutmEnable();
        XRR_rexDisable();
        if (spd == USB_SPEED_HIGH)
        {
            XRR_SetSOFGenHighSpeed();
        }
        else if (spd == USB_SPEED_FULL)
        {
            XRR_SetSOFGenFullLowSpeed();
        }
        else
        {
            XRR_SetSOFGenDefaultSpeed();
        }
        XRR_SOFEnable();
    }
}

/**
* FUNCTION NAME: REXSCH_forceSofTxStop()
*
* @brief  - Ensure SOF packets are transmitted, regardless of synchronization with host
*
* @return - void
*
* @note   - This helps work around buggy devices
*
*/
void REXSCH_forceSofTxStop(void)
{
    XRR_SOFDisable();
    REXSCH_Disable(); // same as above but also sets SOFCount to 0
}


