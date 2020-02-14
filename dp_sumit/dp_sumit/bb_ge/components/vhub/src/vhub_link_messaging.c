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
//!   @file  -  vhub_link_messaging.c
//
//!   @brief -
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "vhub_loc.h"

/***************************** Local Variables *******************************/

/************************** Function Definitions *****************************/

/**
 * Handle XUSB messages from the host via the LEXULM
 *
 * @param msg - HOST message
 *
 */
void VHUB_HostPortMessage
(
    XCSR_CtrlUpstreamStateChangeMessageT msg
)
{
    switch (msg)
    {
        case UPSTREAM_BUS_RESET_HS:
            vhub.linkSpeed = USB_SPEED_HIGH;
            ilog_VHUB_COMPONENT_0(ILOG_MAJOR_EVENT, HOST_CONNECT_HS);
            // fall through
        case UPSTREAM_BUS_RESET_FS:
            if (msg == UPSTREAM_BUS_RESET_FS)
            {
                vhub.linkSpeed = USB_SPEED_FULL;
                ilog_VHUB_COMPONENT_0(ILOG_MAJOR_EVENT, HOST_CONNECT_FS);
            }
            vhub.linkState = BUS_RESETTING;
            vhub.currentConfiguration = 0;
            VHUB_Reset();
            DTT_HostResetVirtualFunction(&vhub_vf);
            break;

        case UPSTREAM_BUS_RESET_DONE:
            iassert_VHUB_COMPONENT_3(vhub.linkState == BUS_RESETTING, UNEXPECTED_EVENT_IN_STATE, msg, vhub.linkState, __LINE__);
            vhub.linkState = OPERATING;
            DTT_EnableAddress0();
            break;

        case UPSTREAM_SUSPEND:
            if (vhub.linkState == DISCONNECTED)
            {
                // On a connect the first event is a suspend, before a reset is done
                // So this event is just ignored
            }
            else
            {
                iassert_VHUB_COMPONENT_3(vhub.linkState == OPERATING, UNEXPECTED_EVENT_IN_STATE, msg, vhub.linkState, __LINE__);
                vhub.linkState = SUSPENDED;
                ilog_VHUB_COMPONENT_0(ILOG_MAJOR_EVENT, HOST_SUSPEND);
                for(uint8 i = 1; i <= VHUB_NUM_PORTS; i++)
                {
                    if(VHUB_IsPortEnabled(i))
                    {
                        VHUB_SuspendPort(i);
                    }
                }
            }
            break;

        case UPSTREAM_RESUME:
            iassert_VHUB_COMPONENT_3(vhub.linkState == SUSPENDED, UNEXPECTED_EVENT_IN_STATE, msg, vhub.linkState, __LINE__);
            vhub.linkState = RESUMING;
            ilog_VHUB_COMPONENT_0(ILOG_MAJOR_EVENT, HOST_RESUME);
            for(uint8 i = 1; i <= VHUB_NUM_PORTS; i++)
            {
                if(VHUB_IsPortEnabled(i))
                {
                    VHUB_StartResumePort(i);
                }
            }
            break;

        case UPSTREAM_RESUME_DONE:
            iassert_VHUB_COMPONENT_3(vhub.linkState == RESUMING, UNEXPECTED_EVENT_IN_STATE, msg, vhub.linkState, __LINE__);
            vhub.linkState = OPERATING;
            for(uint8 i = 1; i <= VHUB_NUM_PORTS; i++)
            {
                if(VHUB_IsPortEnabled(i))
                {
                    VHUB_FinishResumePort(i);
                }
            }
            break;

        case UPSTREAM_DISCONNECT:
            vhub.linkState = DISCONNECTED;
            ilog_VHUB_COMPONENT_0(ILOG_MAJOR_EVENT, HOST_DISCONNECT_MSG);
            // If the upstream port is disconnected, then reset VHUB
            vhub.currentConfiguration = 0;
            VHUB_Reset();

            // Let the LexULM know we are ready to connect again
            LEX_UlmMessageHandler(DOWNSTREAM_CONNECT_HS);
            break;

        case UPSTREAM_BUS_RESET_LS: // Hubs only work in HS and FS
        default:
            iassert_VHUB_COMPONENT_1(FALSE, HOST_OTHER, msg);
            break;
    }
}


/**
 * Handle XUSB messages from downstream REX vports
 *
 * @param msg - REX msg
 * @param vport - vport associated with REX
 */
void VHUB_DevicePortMessage
(
    XCSR_CtrlDownstreamStateChangeMessageT msg, // The message from the Rex
    uint8 vport                                 // The Vport of the Rex the message came from
)
{
    enum UsbSpeed speed;
    switch (msg)
    {
        case DOWNSTREAM_CONNECT_HS:
            speed = USB_SPEED_HIGH;
            goto deviceConnect; // --------------------\ goto on this line
        case DOWNSTREAM_CONNECT_FS:                 // |
            speed = USB_SPEED_FULL;                 // |
            goto deviceConnect; // --------------------* goto on this line
        case DOWNSTREAM_CONNECT_LS:                 // |
            speed = USB_SPEED_LOW;                  // |
deviceConnect: // <------------------------------------/
            ilog_VHUB_COMPONENT_1(ILOG_MAJOR_EVENT, VHUB_DEVICE_CONNECT, vport);
            VHUB_ConnectDev(vport, speed);
            if((vhub.linkState == SUSPENDED) && (vhub.remoteWakeupEnabled))
            {
                vhub.linkState = RESUMING;
                LEX_UlmMessageHandler(DOWNSTREAM_REMOTE_WAKEUP);
            }
            break;

        case DOWNSTREAM_DISCONNECT:
            ilog_VHUB_COMPONENT_1(ILOG_MAJOR_EVENT, VHUB_DEVICE_DISCONNECT, vport);
            VHUB_DisConnectDev(vport);
            if ((vhub.linkState == SUSPENDED) && (vhub.remoteWakeupEnabled))
            {
                vhub.linkState = RESUMING;
                LEX_UlmMessageHandler(DOWNSTREAM_REMOTE_WAKEUP);
            }
            break;

        case DOWNSTREAM_REMOTE_WAKEUP:
            ilog_VHUB_COMPONENT_1(ILOG_MAJOR_EVENT, VHUB_DEVICE_WAKEUP, vport);
            if (vhub.linkState == OPERATING)
            {
                VHUB_RemoteWakeup(vport);
            }
            else
            {
                // Hubs are required to propogate resume signalling from downstream devices even if
                // the hub itself does not have remote wakeup enabled.
                if  (vhub.linkState == SUSPENDED)
                {
                    vhub.linkState = RESUMING;
                    LEX_UlmMessageHandler(DOWNSTREAM_REMOTE_WAKEUP);
                    VHUB_RemoteWakeup(vport);
                }
                else if (vhub.linkState == RESUMING)
                {
                    VHUB_RemoteWakeup(vport);
                }
            }
            break;
        default:
            ilog_VHUB_COMPONENT_1(ILOG_MAJOR_ERROR, VHUB_UNKNOWN_MSG_FROM_DOWNSTREAM, msg);
            break;
    }
}

