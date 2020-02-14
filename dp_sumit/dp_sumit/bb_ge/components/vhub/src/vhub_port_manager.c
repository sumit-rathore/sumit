///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2011, 2012, 2013
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
//!   @file  -  vhub_port_manager.c
//
//!   @brief -  Contains VHUB
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "vhub_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************** Function Definitions *****************************/

/**
 * Reset Port States
 */
void VHUB_InitPortManager(void)
{
    for(uint8 i = 1; i <= VHUB_NUM_PORTS; i++)
    {
        vhub.portInfo[i].state = DISCONNECTED;
    }
}

/**
 * Check whether a VHUB port is currently in use
 *
 * @param port - port number to query (starting at 1)
 * @return - whether port is in use (associated with a vport)
 */
boolT VHUB_IsPortEnabled(uint8 port)
{
    return vhub.portInfo[port].state != DISCONNECTED;
}

/**
 * Connect downstream REX to VHUB port
 *
 * @param vPort - vport of remote rex
 * @param speed - speed at which remote rex is operating
 */
void VHUB_ConnectDev(uint8 vPort, enum UsbSpeed speed)
{
    vhub.portInfo[vPort].speed = speed;
    vhub.portInfo[vPort].state = SUSPENDED;

    if (vhub.portInfo[vPort].usbCh11Status.status & HOST_ENDIAN_TO_USB_16(1 << VHUB_PSTATUS_POWER))
    {
        //persist changes in port descriptor
        VHUB_SET_PORT_STATUS_BIT(vPort, VHUB_PSTATUS_CONNECTION);
        VHUB_SET_PORT_CHANGE_BIT(vPort, VHUB_PCHANGE_CONNECTION);

        //interrupt endpoint
        VHUB_SET_INTEP1_DATA_BIT(vPort); //portX change
        ilog_VHUB_COMPONENT_2(ILOG_MAJOR_EVENT, VHUB_CONNECT_DEV, vPort, speed);
    }
}

/**
 * Disconnect downstream REX from VHUB port
 *
 * @param port - VHUB port to disconnect (should be at least 1)
 */
void VHUB_DisConnectDev(uint8 port)
{
    XUSB_AddressT parent_address = DTT_GetAddressFromVF(&vhub_vf);

    vhub.portInfo[port].state = DISCONNECTED;

    DTT_PortDisconnect(parent_address, port);

    // Clear Port status except Power
    vhub.portInfo[port].usbCh11Status.status = 0;
    VHUB_SET_PORT_STATUS_BIT(port, VHUB_PSTATUS_POWER);

    //change and interrupt endpoint
    VHUB_SET_PORT_CHANGE_BIT(port, VHUB_PCHANGE_CONNECTION);
    VHUB_SET_INTEP1_DATA_BIT(port);
    XCSR_XICSSendMessage(USB_UPSTREAM_STATE_CHANGE, UPSTREAM_DISCONNECT, port);

    ilog_VHUB_COMPONENT_1(ILOG_MAJOR_EVENT, VHUB_DISCONNECT_DEV, port);
}

/**
 * Initiate Suspend on VHUB port
 *
 * @param port - VHUB port to suspend (should be at least 1)
 */
void VHUB_SuspendPort(uint8 port)
{
    if( vhub.portInfo[port].state == OPERATING)
    {
        vhub.portInfo[port].state = SUSPENDED;

        //if enabled, suspend/disable
        if(vhub.portInfo[port].usbCh11Status.status & HOST_ENDIAN_TO_USB_16(1 << VHUB_PSTATUS_ENABLE))
        {
            // suspend
            VHUB_SET_PORT_STATUS_BIT(port, VHUB_PSTATUS_SUSPEND);
            // port suspend change
            VHUB_SET_PORT_CHANGE_BIT(port, VHUB_PCHANGE_SUSPEND);

            // interrupt endpoint
            VHUB_SET_INTEP1_DATA_BIT(port); //portX change
            XCSR_XICSSendMessage(USB_UPSTREAM_STATE_CHANGE, UPSTREAM_SUSPEND, port);

            ilog_VHUB_COMPONENT_1(ILOG_MAJOR_EVENT, VHUB_SUSPEND_DEV, port);
        }

    }
}

/**
 * Finish resume on downstream rex
 *
 * @param port - VHUB port to resume (should be at least 1)
 */
void VHUB_FinishResumePort(uint8 port)
{
    if(vhub.portInfo[port].state == RESUMING)
    {
        vhub.portInfo[port].state = OPERATING;
        // if suspended, resume
        if(vhub.portInfo[port].usbCh11Status.status & HOST_ENDIAN_TO_USB_16(1 << VHUB_PSTATUS_SUSPEND))
        {
            // resume
            VHUB_CLR_PORT_STATUS_BIT(port, VHUB_PSTATUS_SUSPEND);
            // port suspend change
            VHUB_SET_PORT_CHANGE_BIT(port, VHUB_PCHANGE_SUSPEND);

            // interrupt endpoint
            VHUB_SET_INTEP1_DATA_BIT(port); //portX change
            XCSR_XICSSendMessage(USB_UPSTREAM_STATE_CHANGE, UPSTREAM_RESUME_DONE, port);

            ilog_VHUB_COMPONENT_1(ILOG_MAJOR_EVENT, VHUB_RESUME_DEV, port);
        }
    }
}

/**
 * Start resume on downstream rex
 *
 * @param port - VHUB port to resume (should be at least 1)
 */
void VHUB_StartResumePort(uint8 port)
{
    if(vhub.portInfo[port].state == SUSPENDED)
    {
        vhub.portInfo[port].state = RESUMING;
        XCSR_XICSSendMessage(USB_UPSTREAM_STATE_CHANGE, UPSTREAM_RESUME, port);
    }
}

/**
 * Remote wakeup detected, if suspended go to RESUMING
 *
 * @param port
 */
void VHUB_RemoteWakeup(uint8 port)
{
    if(vhub.portInfo[port].state == SUSPENDED)
    {
        vhub.portInfo[port].state = RESUMING;
        vhub.portInResume |= (1 << port);
        TIMING_TimerStart(vhub.portResumeTimerCompleteHandle);
    }
    else
    {
        // TODO
    }
}

