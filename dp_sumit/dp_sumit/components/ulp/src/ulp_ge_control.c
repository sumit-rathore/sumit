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
// This file contains the code for bringing up and shutting down GE
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <bb_top_ge.h>
#include <cpu_comm.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <mca.h>
#include <event.h>
#include <led.h>
#include <bb_ge_comm.h>
#include <ulp.h>
#include "ulp_log.h"
#include "ulp_loc.h"

#include <uart.h>

// Constants and Macros ###########################################################################

#define ULP_GE_CONTROL_MINIMUM_RESET_TIME   20      // set the minimum reset time to 20ms

// Data Types #####################################################################################
enum UlpGeUsbControlStates
{
    ULP_GE_CONTROL_DISABLED,            // GE is disabled, ready to be enabled
    ULP_GE_CONTROL_DISABLE_IN_PROGRESS, // GE disabled, waiting for minimum reset time to pass
    ULP_GE_CONTROL_ENABLE_PENDING,      // enable request pending, waiting for GE to finish disabling
    ULP_GE_CONTROL_GE_ENABLED,                // GE enabled
};

// Static Function Declarations ###################################################################
static void UlpGeReceiveCpuMsgHandler(
    uint8_t subType, const uint8_t *msg, uint16_t msgLength)                            __attribute__ ((section(".atext")));
static void UlpGeResetDone(void)                                                        __attribute__ ((section(".atext")));
static void UlpGeChannelStatus(enum MCA_ChannelStatus channelStatus)                    __attribute__ ((section(".atext")));
static void GeMinimumResetTimeout(void)                                                 __attribute__ ((section(".atext")));
static void UlpSendCPUCommGeControlMessage(enum UlpGeControlMessage geControlMessage)   __attribute__ ((section(".atext")));
static void GeFailureHandler()                                                          __attribute__ ((section(".atext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct GeUlpControlContext
{
    TIMING_TimerHandlerT GeMinimumResetTimer;   // enforces minimum reset time

    enum UlpGeUsbControlStates controlStates;   // used to indicate the state we are in

} geControl;

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize the LEX ULP module and the ULP finite state machine.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void ULP_GeControlInit(void)
{
    MCA_ChannelInit(MCA_CHANNEL_NUMBER_GE, UlpGeChannelStatus, NULL);

    BBGE_COMM_registerHandler(BBGE_COMM_MSG_HNDLR_GE_READY, UlpGeResetDone);
    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_ULP_GE_CONTROL, UlpGeReceiveCpuMsgHandler); // used to receive status updates from the other side

    geControl.GeMinimumResetTimer = TIMING_TimerRegisterHandler(
        GeMinimumResetTimeout,
        false,
        ULP_GE_CONTROL_MINIMUM_RESET_TIME);
}


// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Sets up or bring down the Lex/Rex GE link
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UlpGeControl(bool enable)
{
    enum UlpGeUsbControlStates oldCcontrolState = geControl.controlStates;

    // only turn GE on if the link is up, and it is enabled
    if (enable && EVENT_GetEventInfo(ET_COMLINK_STATUS_CHANGE))
    {
        if (ULP_Usb2SystemEnabled())
        {
            if (geControl.controlStates == ULP_GE_CONTROL_DISABLED)
            {
                bb_top_SetGEToResetMode();              // make sure GE is in reset, until the link is up

                bb_top_ControlGeDataPhy(true);          // enable Phy link between GE and Blackbird

                MCA_ChannelLinkUp(MCA_CHANNEL_NUMBER_GE);    // start bringing the link up

                // tell the other side we are enabled
                UlpSendCPUCommGeControlMessage(ULP_GE_CONTROL_MSG_GE_ENABLED);

                geControl.controlStates = ULP_GE_CONTROL_GE_ENABLED;
            }
            else if (geControl.controlStates == ULP_GE_CONTROL_DISABLE_IN_PROGRESS)
            {
                geControl.controlStates = ULP_GE_CONTROL_ENABLE_PENDING;
            }
        }
        else
        {
            // tell the other side we are disabled
            UlpSendCPUCommGeControlMessage(ULP_GE_CONTROL_MSG_GE_DISABLED);
        }
    }
    else
    {
        if ( (geControl.controlStates != ULP_GE_CONTROL_DISABLED) &&
             (geControl.controlStates != ULP_GE_CONTROL_DISABLE_IN_PROGRESS) )
        {
            TIMING_TimerStart(geControl.GeMinimumResetTimer);
            geControl.controlStates = ULP_GE_CONTROL_DISABLE_IN_PROGRESS;

            bb_top_SetGEToResetMode();          // Put GE into reset

            MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_GE);    // make sure the MCA channel is disabled

            bb_top_ControlGeDataPhy(false); // disable Phy link between GE and Blackbird

            // tell the other side we are disabled
            UlpSendCPUCommGeControlMessage(ULP_GE_CONTROL_MSG_GE_DISABLED);

            if ( bb_top_IsDeviceLex() )
            {
                ULP_LexUsb2Control(ULP_USB_CONTROL_RESET_ACTIVE);    // GE is in reset
            }
            else
            {
                ULP_RexUsb2Control(ULP_USB_CONTROL_RESET_ACTIVE);    // GE is in reset
            }
        }
        else if (geControl.controlStates == ULP_GE_CONTROL_ENABLE_PENDING)
        {
            geControl.controlStates = ULP_GE_CONTROL_DISABLE_IN_PROGRESS;
        }
    }
    ilog_ULP_COMPONENT_3(ILOG_DEBUG, ULP_GE_CONTROL_STATES, oldCcontrolState, geControl.controlStates, enable);
}

// Static Function Definitions ####################################################################

//#################################################################################################
// handles the CPU messages from the other side
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void UlpGeReceiveCpuMsgHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    iassert_ULP_COMPONENT_1(msgLength == 0, ULP_LEX_UNEXPECTED_CPU_MSG_LENGTH, msgLength);
    ilog_ULP_COMPONENT_1(ILOG_DEBUG, ULP_GE_CONTROL_RCV_CPU_MSG, subType);

    enum UlpGeControlMessage geControlMessage = (enum UlpGeControlMessage)subType;

    switch (geControlMessage)
    {
        case ULP_GE_CONTROL_MSG_GE_ENABLED:         // GE enabled on the far side
            UlpGeControl(true);                 // enable GE on this side, too
            break;

        case ULP_GE_CONTROL_MSG_GE_DISABLED:        // GE disabled on the far side
            // there is a possible bug here, where if Rex really wants to be disabled, this will
            // actually cause it to be enabled, instead

            // this if catches the case where we disable and then enable GE, to do a reset cycle
            // it only does the disable if an enable is not pending
            if (geControl.controlStates != ULP_GE_CONTROL_ENABLE_PENDING)
            {
                UlpGeControl(false);                // disable GE on this side, too
            }
            break;

        case ULP_GE_CONTROL_MSG_START_GE:           // Rex to Lex - Start GE on your side (GE running on Rex)
            bb_top_SetGEToRunMode(GeFailureHandler, GeFailureHandler);      // remove reset from GE
            break;

        default:
            break;
    }
}

//#################################################################################################
// Called when GE is up and running
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UlpGeResetDone(void)
{
    if ( bb_top_IsDeviceLex() )
    {
        ULP_LexUsb2Control(ULP_USB_CONTROL_RESET_DONE); // GE is done resetting (Lex side)
    }
    else
    {
        ULP_RexUsb2Control(ULP_USB_CONTROL_RESET_DONE); // GE is done resetting (Rex side)
        UlpSendCPUCommGeControlMessage(ULP_GE_CONTROL_MSG_START_GE);  // tell Lex to start GE
    }
}

//#################################################################################################
// Handler for USB2 (GE) channel status changes
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
static void UlpGeChannelStatus(enum MCA_ChannelStatus channelStatus)
{
    ilog_ULP_COMPONENT_1(ILOG_MINOR_EVENT, ULP_GE_CHANNEL_STATUS, channelStatus);

    switch (channelStatus)
    {
        case MCA_CHANNEL_STATUS_LINK_ACTIVE:        // channel is linked between Lex and Rex
            MCA_ChannelTxRxSetup(MCA_CHANNEL_NUMBER_GE);    // now setup Tx and Rx
            break;

        case MCA_CHANNEL_STATUS_CHANNEL_READY:      // channel is linked, and Rx, Tx is setup.  Ready for operation
            if ( bb_top_IsDeviceLex())
            {
                ULP_LexUsb2Control(ULP_USB_CONTROL_MCA_CHANNEL_RDY); // GE MCA channel is ready
            }
            else
            {
                // for the Rex, take GE out of reset
                bb_top_SetGEToRunMode(GeFailureHandler, GeFailureHandler);      // remove reset from GE
                ULP_RexUsb2Control(ULP_USB_CONTROL_MCA_CHANNEL_RDY); // GE MCA channel is ready
            }
            break;

        case MCA_CHANNEL_STATUS_CHANNEL_DISABLED:   // channel is disabled
            if (geControl.controlStates == ULP_GE_CONTROL_ENABLE_PENDING)
            {
                break;                              // Enable Pending status, Ignore disable event and Wait GeMinimumResetTimeout
            }
            // Fall through
        case MCA_CHANNEL_STATUS_LINK_DOWN:          // channel is down between Lex and Rex, needs to be re-initialized
        default:
            // something is wrong with the GE channel - put GE in reset (will reset the link, too)
            UlpGeControl(false);
            if ( bb_top_IsDeviceLex() )
            {
                ULP_LexUsb2Control(ULP_USB_CONTROL_MCA_CHANNEL_DOWN); // GE MCA channel is down (Lex side)
            }
            else
            {
                ULP_RexUsb2Control(ULP_USB_CONTROL_MCA_CHANNEL_DOWN); // GE MCA channel is down (Rex side)
            }
            break;
    }
}

//#################################################################################################
// Called when GE is up and running
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void GeMinimumResetTimeout(void)
{
    if (geControl.controlStates == ULP_GE_CONTROL_ENABLE_PENDING)
    {
        geControl.controlStates = ULP_GE_CONTROL_DISABLED;
        UlpGeControl(true);  // ok to enable GE, now
    }
    else
    {
        geControl.controlStates = ULP_GE_CONTROL_DISABLED;
    }
}

//#################################################################################################
// Sends the specified CPU message to the other side
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
static void UlpSendCPUCommGeControlMessage(enum UlpGeControlMessage geControlMessage)
{
    CPU_COMM_sendSubType(CPU_COMM_TYPE_ULP_GE_CONTROL, geControlMessage);
}


//#################################################################################################
// Called when a GE failure has been detected
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
static void GeFailureHandler()
{
    ilog_ULP_COMPONENT_0(ILOG_FATAL_ERROR, ULP_GE_FAILURE_HANDLER);

    if ( bb_top_IsDeviceLex() )
    {
        UlpLexUsb2Failed();
    }
    else
    {
        UlpRexUsb2Failed();
    }
}
