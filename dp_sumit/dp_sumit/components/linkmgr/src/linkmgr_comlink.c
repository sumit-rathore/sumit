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
// This file contains the implementation of COM LINK portion of the Link Manager.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// COM LINK of the Link Manager looks at things from a Lex-Rex link level
// * Operational only when the PHY link is up
// * Responsible for the bring up and take down of the CPU comm MCA channel
//
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <cpu_comm.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <bb_top.h>
#include <bb_core.h>
#include <mca.h>
#include <callback.h>
#include <event.h>
#include <random.h>
#include "linkmgr_log.h"
#include "linkmgr_loc.h"

#include <uart.h>   // for debug (UART_printf() )

// Constants and Macros ###########################################################################
#define COM_LINK_DEBOUNCE_PERIOD_IN_MS      (500)       // minimum time the link is down before coming back up again
#define COM_LINK_RESTART_LINK_PERIOD_IN_MS  (4 * 1000)  // Wait time before restarting the link
                                                        // Fiber reset takes about 2.5sec

// Data Types #####################################################################################
enum ComLinkState
{
    COM_LINK_STATE_LINK_DISABLED,               // PHY LINK Down state or initial state
    COM_LINK_STATE_LINK_DOWN_DELAYED_START,     // Wait a bit before starting to enforce minimum down time
    COM_LINK_STATE_LINK_DOWN_WAIT_FOR_CHANNEL,  // Wait for the channel to be linked before going on
    COM_LINK_STATE_LINK_UP_DELAYED,             // Wait a bit before declaring the link up
    COM_LINK_STATE_LINK_UP_WAIT_FOR_READY,      // Wait for the ready message from the other side
    COM_LINK_STATE_LINK_UP,                     // Lex-Rex link established
};

enum ComLinkEvent
{
    COM_LINK_EVENT_DISABLE,             // Disable the comm link channel
    COM_LINK_EVENT_ENABLE,              // Enable the comm link channel - Phy link is up
    COM_LINK_EVENT_CHANNEL_UP,          // Comm channel is linked with the other side
    COM_LINK_EVENT_CHANNEL_DOWN,        // Comm channel is down
    COM_LINK_EVENT_READY_MSG,           // Ready message received from the other side
    COM_LINK_EVENT_DEBOUNCE,            // debouce timeout
};

// Static Function Declarations ###################################################################

static void ComLinkSendCPUCommMessage(const uint8_t message)                    __attribute__((section(".atext")));
static void ComlinkReceiveCpuMsgHandler(
    uint8_t subType, const uint8_t *msg, uint16_t msgLength)                    __attribute__((section(".atext")));

static uint32_t ComlinkGetComlinkState(void);

static void ComLinkStateMachineSendEvent(enum ComLinkEvent event)               __attribute__((section(".atext")));
static void ComLinkStateCallback(void *param1, void *param2)                    __attribute__((section(".atext")));

// these states aren't time critical
static void ComLinkStateMachine(enum ComLinkEvent event)                        __attribute__((section(".atext")));
static void ComLinkDisabledHandler(enum ComLinkEvent event)                     __attribute__((section(".atext")));
static void ComLinkDelayedStart(enum ComLinkEvent event)                        __attribute__((section(".atext")));
static void ComLinkDownWaitForChannel(enum ComLinkEvent event)                  __attribute__((section(".atext")));
static void ComLinkDelayedLinkUp(enum ComLinkEvent event)                       __attribute__((section(".atext")));
static void ComLinkWaitForReady(enum ComLinkEvent event)                        __attribute__((section(".atext")));
static void ComLinkUpHandler(enum ComLinkEvent event)                           __attribute__((section(".atext")));

static void ComlinkDebounceTimeout(void)                                        __attribute__((section(".atext")));
static void ComLinkLinkDownTimeout(void)                                        __attribute__((section(".atext")));
static void ComLinkSetStatus(bool linkUp)                                       __attribute__((section(".atext")));
static void ComLinkDisableLink(void)                                            __attribute__((section(".atext")));

static void CommlinkPhyLinkStatusHandler(uint32_t PhyLinkUp, uint32_t userContext)  __attribute__((section(".atext")));
static void ComlinkMcaChannelStatusHandler(enum MCA_ChannelStatus channelStatus)    __attribute__((section(".atext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct ComLinkMgrContext
{
    // LinkMgr COM LINK FSM state.
    enum ComLinkState comLinkFsmState;

    // A timer used to enforce a delay before coming back up again
    TIMING_TimerHandlerT DebounceDelayTimer;

    // A timer used to reset link up sequence
    TIMING_TimerHandlerT LinkRestartTimer;

    bool comLinkUp;     // true if the comlink is active, false if it is down

    bool geVerified;    // true if GE's version checked

} comLinkContext;

// Exported Function Definitions ##################################################################

//#################################################################################################
// Register the generic timer and set the initial state.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void LINKMGR_ComlinkInit(void)
{
    CPU_COMM_Init(LINKMGR_phyLinkDownErrorDetected);

    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_COM_LINK, ComlinkReceiveCpuMsgHandler);

    // setup a timer to enforce a delay when bringing up the link
    // this delay is added whenever comlink is enabled
    // (in case it was disabled and then enabled right away), or when the link goes down,
    // so the other side can see the link is down
    // the delay is meant to avoid the situation where the link bounces between up and down
    // too quickly
    comLinkContext.DebounceDelayTimer = TIMING_TimerRegisterHandler(
        ComlinkDebounceTimeout,
        false,
        COM_LINK_DEBOUNCE_PERIOD_IN_MS);

    // timer used to detect that the link has been down for too long
    comLinkContext.LinkRestartTimer = TIMING_TimerRegisterHandler(
        ComLinkLinkDownTimeout,
        false,
        COM_LINK_RESTART_LINK_PERIOD_IN_MS);

    comLinkContext.comLinkFsmState = COM_LINK_STATE_LINK_DISABLED;

    // note that care must be taken with this event, that the subscribers only receive
    // notifications when the event changes
    EVENT_Register(ET_COMLINK_STATUS_CHANGE, ComlinkGetComlinkState);

    // make sure we get Phy link status changes
    EVENT_Subscribe(ET_PHYLINK_STATUS_CHANGE, CommlinkPhyLinkStatusHandler, 0);

    MCA_ChannelInit(MCA_CHANNEL_NUMBER_CPU_COMM, ComlinkMcaChannelStatusHandler, NULL);
}


//#################################################################################################
// External input to generate COM LINK Enable event.
//
// Parameters:
//      * enable        - if true, bring up, if false take down the COM LINK
// Return:
// Assumptions:
//#################################################################################################
void LINKMGR_ComlinkEnable(bool enable)
{
    if (enable)
    {
        ComLinkStateMachineSendEvent(COM_LINK_EVENT_ENABLE);
    }
    else
    {
        ComLinkStateMachineSendEvent(COM_LINK_EVENT_DISABLE);
    }
}


//#################################################################################################
// Tells GE version and CRC are verfied so that BB can start link
//
//  Wake-up GE and getting GE's information is related with its' reset and boot pin
//  Not to mingle wakeup procedure, it suspends GE's wakeup procedure until GE verification finish
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void LINKMGR_setGeVerified()
{
    comLinkContext.geVerified = true;

    if(comLinkContext.comLinkFsmState == COM_LINK_STATE_LINK_UP)
    {
        ComLinkSetStatus(true);
    }
}


// Component Scope Function Definitions ###########################################################

//#################################################################################################
// External input to generate a cpu comm channel packet
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void LINKMGR_comLinkTxPacketIcmd(uint8_t value)
{
    ComLinkSendCPUCommMessage(COM_LINK_MSG_TEST_PACKET);
}

// Static Function Definitions ####################################################################

//#################################################################################################
// Entry point to send an event to the cpu com link state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ComLinkStateMachineSendEvent(enum ComLinkEvent event)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = event;

    CALLBACK_Run(ComLinkStateCallback, (void *)eventx, NULL);
}

//#################################################################################################
// Callback wrapper for the link manager state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ComLinkStateCallback(void *param1, void *param2)
{
    ComLinkStateMachine( (enum ComLinkEvent)param1 );
}


//#################################################################################################
// Debounce timer handler - used to make sure we don't bounce in and out of certain states
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ComlinkDebounceTimeout(void)
{
    ComLinkStateMachineSendEvent(COM_LINK_EVENT_DEBOUNCE);
}


//#################################################################################################
// _LINKMGR_REX_LinkDownTimeout
//  To check comlink up time and do reset if it can't bring up.
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ComLinkLinkDownTimeout(void)
{
    // link has been down too long - tell the link manager about it
    ilog_LINKMGR_COMPONENT_0(ILOG_MAJOR_ERROR, COMLINK_LINKUP_TIMEOUT);
//    LINKMGR_phyLinkDownErrorDetected();
}


//#################################################################################################
// The COM LINK state machine.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void ComLinkStateMachine(enum ComLinkEvent event)
{
    enum ComLinkState oldState = comLinkContext.comLinkFsmState;

    switch (comLinkContext.comLinkFsmState)
    {
        case COM_LINK_STATE_LINK_DISABLED:   // PHY LINK Down or initial state
            ComLinkDisabledHandler(event);
            break;

        case COM_LINK_STATE_LINK_DOWN_DELAYED_START:    // Wait a bit before starting to enforce minimum down time
            ComLinkDelayedStart(event);
            break;

        case COM_LINK_STATE_LINK_DOWN_WAIT_FOR_CHANNEL: // Wait for the channel to be linked before going on
            ComLinkDownWaitForChannel(event);
            break;

        case COM_LINK_STATE_LINK_UP_DELAYED:            // Wait a bit before declaring the link up
            ComLinkDelayedLinkUp(event);
            break;

        case COM_LINK_STATE_LINK_UP_WAIT_FOR_READY:      // Wait for the ready message from the other side
            ComLinkWaitForReady(event);
            break;

        case COM_LINK_STATE_LINK_UP: // Lex-Rex link established
            ComLinkUpHandler(event);
            break;

        default:
            break;
    }

    ilog_LINKMGR_COMPONENT_3(ILOG_USER_LOG, COMLINK_RCV_EVENT, oldState, event, comLinkContext.comLinkFsmState);
}


//#################################################################################################
// Handles events in COM_LINK_STATE_LINK_DISABLED
//
// Parameters:
//      event                   - COM LINK event
// Return:
// Assumptions:
//
//#################################################################################################
static void ComLinkDisabledHandler(enum ComLinkEvent event)
{
    switch (event)
    {
        case COM_LINK_EVENT_ENABLE: // Triggered by PHY_EVENT_LINK_UP from LINKMGR
            comLinkContext.comLinkFsmState = COM_LINK_STATE_LINK_DOWN_DELAYED_START;

            // ensure the link is down for a minimum time before starting to contact the other side
            TIMING_TimerStart(comLinkContext.DebounceDelayTimer);
            break;

        // these events don't mean anything, in this state
        case COM_LINK_EVENT_CHANNEL_UP:     // Comm channel is linked with the Rex
        case COM_LINK_EVENT_CHANNEL_DOWN:   // Comm channel is down
        case COM_LINK_EVENT_DISABLE:        // Disable the comm link channel
        case COM_LINK_EVENT_READY_MSG:      // Ready message received from the other side
        case COM_LINK_EVENT_DEBOUNCE:       // keep the link down for a minimum time
        default:
            ilog_LINKMGR_COMPONENT_2(
                ILOG_MINOR_ERROR,
                COMLINK_STATE_EVENT_INVALID,
                event,
                comLinkContext.comLinkFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in COM_LINK_STATE_LINK_DOWN_DELAYED_START
//
// Parameters:
//      event                   - COM LINK event
// Return:
// Assumptions:
//
//#################################################################################################
static void ComLinkDelayedStart(enum ComLinkEvent event)
{
    switch (event)
    {
        case COM_LINK_EVENT_DISABLE:    // Disable the comm link channel
            ComLinkDisableLink();       // bring down the comlink
            break;

        // link down for a minimum time, start bringing up the channel
        case COM_LINK_EVENT_DEBOUNCE:
            comLinkContext.comLinkFsmState = COM_LINK_STATE_LINK_DOWN_WAIT_FOR_CHANNEL;
            TIMING_TimerStart(comLinkContext.LinkRestartTimer);  // timeout after a bit if the link is still not up

            MCA_ChannelLinkUp(MCA_CHANNEL_NUMBER_CPU_COMM);
           break;

        // these events don't mean anything, in this state
        case COM_LINK_EVENT_CHANNEL_UP:     // Comm channel is linked with the Rex
        case COM_LINK_EVENT_CHANNEL_DOWN:   // Comm channel is down
        case COM_LINK_EVENT_ENABLE:         // Triggered by PHY_EVENT_LINK_UP from LINKMGR
        case COM_LINK_EVENT_READY_MSG:      // Ready message received from the other side
        default:
            ilog_LINKMGR_COMPONENT_2(
                ILOG_MINOR_ERROR,
                COMLINK_STATE_EVENT_INVALID,
                event,
                comLinkContext.comLinkFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in COM_LINK_STATE_LINK_DOWN_WAIT_FOR_CHANNEL (waiting for the MCA channel)
//
// Parameters:
//      event                   - COM LINK event
// Return:
// Assumptions:
//
//#################################################################################################
static void ComLinkDownWaitForChannel(enum ComLinkEvent event)
{
    switch (event)
    {
        case COM_LINK_EVENT_DISABLE:    // Disable the comm link channel
            ComLinkDisableLink();       // bring down the comlink
            break;

            // link down for a minimum time, start trying to contact the Rex
        case COM_LINK_EVENT_CHANNEL_UP:     // Comm channel is linked with the Rex
            // Note: this interrupt seems to trigger even when the MCA is in reset, from power up
            bb_core_setCoreIrqEn(BB_CORE_IRQ_PENDING_IRQ_MCA_RX_CPU_SRDY);

            // give a bit of debounce time before declaring the link up
            TIMING_TimerStart(comLinkContext.DebounceDelayTimer);

            comLinkContext.comLinkFsmState = COM_LINK_STATE_LINK_UP_DELAYED;

            LINKMGR_SystemUpDetected();     // indicate system up
            break;

            // these events don't mean anything, in this state
        case COM_LINK_EVENT_CHANNEL_DOWN:   // Comm channel is down
        case COM_LINK_EVENT_ENABLE:         // Triggered by PHY_EVENT_LINK_UP from LINKMGR
        case COM_LINK_EVENT_DEBOUNCE:       // keep the link down for a minimum time
        case COM_LINK_EVENT_READY_MSG:      // Ready message received from the other side
        default:
            ilog_LINKMGR_COMPONENT_2(
                ILOG_MINOR_ERROR,
                COMLINK_STATE_EVENT_INVALID,
                event,
                comLinkContext.comLinkFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in COM_LINK_STATE_LINK_UP_DELAYED - pauses a bit to make sure the link is stable,
// and flushes any old CPU messages, from any subsystems
//
// Parameters:
//      event                   - COM LINK event
// Return:
// Assumptions:
//
//#################################################################################################
static void ComLinkDelayedLinkUp(enum ComLinkEvent event)
{
    switch (event)
    {
        case COM_LINK_EVENT_DISABLE:    // Disable the comm link channel
            ComLinkDisableLink();       // bring down the comlink
            break;

        case COM_LINK_EVENT_CHANNEL_DOWN:   // Comm channel is down
            ComLinkSetStatus(false);
            ilog_LINKMGR_COMPONENT_0(ILOG_MINOR_EVENT, COMLINK_CH_DOWN_DELAYED_LINKUP);
            break;

        case COM_LINK_EVENT_DEBOUNCE:       // keep the link down for a minimum time
            // On the Lex, when the time expires, we send a message to the Rex to say
            // we are ready.  On the Rex, we ignore the timeout, and wait for the Lex
            // to tell us he is ready
            if (bb_top_IsDeviceLex() )
            {
                TIMING_TimerStart(comLinkContext.DebounceDelayTimer);
                ComLinkSendCPUCommMessage(COM_LINK_MSG_LEX_ALIVE);
            }

            comLinkContext.comLinkFsmState = COM_LINK_STATE_LINK_UP_WAIT_FOR_READY;
            break;

        // these events don't mean anything, in this state
        case COM_LINK_EVENT_ENABLE:         // Triggered by PHY_EVENT_LINK_UP from LINKMGR
        case COM_LINK_EVENT_CHANNEL_UP:     // Comm channel is linked with the Rex
        case COM_LINK_EVENT_READY_MSG:      // Ready message received from the other side
        default:
            ilog_LINKMGR_COMPONENT_2(
                ILOG_MINOR_ERROR,
                COMLINK_STATE_EVENT_INVALID,
                event,
                comLinkContext.comLinkFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in COM_LINK_STATE_LINK_UP_WAIT_FOR_READY
// Used to confirm the other side is up and running
//
// Parameters:
//      event                   - COM LINK event
// Return:
// Assumptions:
//
//#################################################################################################
static void ComLinkWaitForReady(enum ComLinkEvent event)
{
    switch (event)
    {
        case COM_LINK_EVENT_DISABLE:    // Disable the comm link channel
            ComLinkDisableLink();       // bring down the comlink
            break;

        case COM_LINK_EVENT_CHANNEL_DOWN:   // Comm channel is down
            ComLinkSetStatus(false);
            ilog_LINKMGR_COMPONENT_0(ILOG_MINOR_EVENT, COMLINK_CH_DOWN_DELAYED_LINKUP);
            break;

        case COM_LINK_EVENT_DEBOUNCE:       // keep the link down for a minimum time
            // On the Lex, when the time expires, we send a message to the Rex to say
            // we are ready.  On the Rex, we ignore the timeout, and wait for the Lex
            // to tell us he is ready
            if (bb_top_IsDeviceLex() )
            {
                TIMING_TimerStart(comLinkContext.DebounceDelayTimer);
                ComLinkSendCPUCommMessage(COM_LINK_MSG_LEX_ALIVE);
            }
            break;

        case COM_LINK_EVENT_READY_MSG:      // Ready message received from the other side
            ComLinkSetStatus(true);         // signal that the link is up!
            comLinkContext.comLinkFsmState = COM_LINK_STATE_LINK_UP;
            TIMING_TimerStop(comLinkContext.LinkRestartTimer);   // stop Comlink wait timer

            if (!bb_top_IsDeviceLex() )
            {
                ComLinkSendCPUCommMessage(COM_LINK_MSG_REX_ALIVE);
            }
            break;

            // these events don't mean anything, in this state
        case COM_LINK_EVENT_ENABLE:         // Triggered by PHY_EVENT_LINK_UP from LINKMGR
        case COM_LINK_EVENT_CHANNEL_UP:     // Comm channel is linked with the Rex
        default:
            ilog_LINKMGR_COMPONENT_2(
                ILOG_MINOR_ERROR,
                COMLINK_STATE_EVENT_INVALID,
                event,
                comLinkContext.comLinkFsmState);
            break;
    }
}


//#################################################################################################
// Handles events in COM_LINK_STATE_LINK_UP
//
// Parameters:
//      event                   - COM LINK event
// Return:
// Assumptions:
//
//#################################################################################################
static void ComLinkUpHandler(enum ComLinkEvent event)
{
    switch (event)
    {
        case COM_LINK_EVENT_DISABLE:    // Disable the comm link channel
            ComLinkDisableLink();       // bring down the comlink
            break;

        case COM_LINK_EVENT_CHANNEL_DOWN:   // Comm channel is down
            ComLinkSetStatus(false);
            ilog_LINKMGR_COMPONENT_0(ILOG_MINOR_EVENT, COMLINK_CH_DOWN_LINKUP);
            break;

        // these events don't mean anything, in this state
        case COM_LINK_EVENT_ENABLE:         // Triggered by PHY_EVENT_LINK_UP from LINKMGR
        case COM_LINK_EVENT_DEBOUNCE:       // keep the link down for a minimum time
        case COM_LINK_EVENT_CHANNEL_UP:     // Comm channel is linked with the Rex
        case COM_LINK_EVENT_READY_MSG:      // Ready message received from the other side
        default:
            ilog_LINKMGR_COMPONENT_2(
                ILOG_MINOR_ERROR,
                COMLINK_STATE_EVENT_INVALID,
                event,
                comLinkContext.comLinkFsmState);
            break;
    }
}


//#################################################################################################
// Sends the specified CPU message to the other side, Lex or Rex
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
static void ComLinkSendCPUCommMessage(const uint8_t message)
{
    CPU_COMM_sendSubType(CPU_COMM_TYPE_COM_LINK, message);
}

//#################################################################################################
// The handler for receiving a CPU message from the other unit.
//
// Parameters:
//      msg                     - message data
//      msgLength               - message length
// Return:
// Assumptions:
//      * The message length is always 1 byte.
//#################################################################################################
static void ComlinkReceiveCpuMsgHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    // iassert_ULP_COMPONENT_1(msgLength == 0, ULP_LEX_UNEXPECTED_CPU_MSG_LENGTH, msgLength);

    enum ComLinkMsg rcvMessage = (enum ComLinkMsg)subType;

    ilog_LINKMGR_COMPONENT_2(ILOG_USER_LOG, COMLINK_MSG_RCVD, comLinkContext.comLinkFsmState, rcvMessage);

    switch (rcvMessage)
    {
        case COM_LINK_MSG_REX_ALIVE:
        case COM_LINK_MSG_LEX_ALIVE:
            ComLinkStateMachineSendEvent(COM_LINK_EVENT_READY_MSG);
            break;

        case COM_LINK_MSG_LEX_REQUEST_RESTART_TO_REX:
            break;

        case COM_LINK_MSG_TEST_PACKET:
        default:
            break;
    }

}


//#################################################################################################
// Handler for Phy Link status changes
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
static void CommlinkPhyLinkStatusHandler(uint32_t PhyLinkStatus, uint32_t userContext)
{
    // turn the ComLink on or off based on the status of the PhyLink
    LINKMGR_ComlinkEnable(PhyLinkStatus == LINK_OPERATING);
}


//#################################################################################################
// Handler for CPU comm channel status changes
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
static void ComlinkMcaChannelStatusHandler(enum MCA_ChannelStatus channelStatus)
{
    ilog_LINKMGR_COMPONENT_1(ILOG_MINOR_EVENT, COMLINK_MCA_CH_STATUS, channelStatus);

    switch (channelStatus)
    {
        case MCA_CHANNEL_STATUS_CHANNEL_DISABLED:   // channel is disabled
        case MCA_CHANNEL_STATUS_LINK_DOWN:          // channel is down between Lex and Rex, needs to be re-initialized
            LINKMGR_phyLinkDownErrorDetected();
//            ComLinkStateMachineSendEvent(COM_LINK_EVENT_CHANNEL_DOWN);
            break;

        case MCA_CHANNEL_STATUS_LINK_ACTIVE:        // channel is linked between Lex and Rex
            MCA_ChannelTxRxSetup(MCA_CHANNEL_NUMBER_CPU_COMM);  // setup Tx and Rx
            break;

         case MCA_CHANNEL_STATUS_CHANNEL_READY:     // channel is linked, and Rx, Tx is setup.  Ready for operation
            iLog_SetTimeStampOffset(0);            // reset the time stamp to zero, to synch time stamps with the Rex
            ComLinkStateMachineSendEvent(COM_LINK_EVENT_CHANNEL_UP);
            break;

        default:
            break;
    }
}


//#################################################################################################
// Disable the Comlink driver
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
static void ComLinkDisableLink(void)
{
    // (will also set state to COM_LINK_STATE_LINK_DOWN_DELAYED_START,
    // but we will set it correctly to COM_LINK_STATE_LINK_DISABLED, below)
    ComLinkSetStatus(false);

    comLinkContext.comLinkFsmState = COM_LINK_STATE_LINK_DISABLED;
    TIMING_TimerStop(comLinkContext.DebounceDelayTimer);
    TIMING_TimerStop(comLinkContext.LinkRestartTimer);
}


//#################################################################################################
// Signal the link is down
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
static void ComLinkSetStatus(bool linkUp)
{
    if (linkUp == false)
    {
        // link is down
        TIMING_TimerStart(comLinkContext.DebounceDelayTimer);
        comLinkContext.comLinkFsmState = COM_LINK_STATE_LINK_DOWN_DELAYED_START;

        MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_CPU_COMM);    // make sure the MCA channel is disabled
        bb_core_setCoreIrqDis(BB_CORE_IRQ_PENDING_IRQ_MCA_RX_CPU_SRDY);     // and the interrupt is turned off

    }

    if (comLinkContext.comLinkUp != linkUp      // sanity check - only indicate link change if it really did
        && comLinkContext.geVerified)           // Is ge version verified ?
    {
        if(linkUp)
        {
            if(bb_core_getLinkMode() == CORE_LINK_MODE_AQUANTIA)
            {
            //     if(bb_core_getLinkRate() == CORE_LINK_RATE_10GBPS)
            //     {
            //         ILOG_istatus(ISTATUS_10G_COMLINK_STATE_UP, 0);
            //     }
            //     else if(bb_core_getLinkRate() == CORE_LINK_RATE_5GBPS)
            //     {
            //         ILOG_istatus(ISTATUS_5G_COMLINK_STATE_UP, 0);
            //     }
            }
            else if(bb_core_getLinkMode() == CORE_LINK_MODE_ONE_LANE_SFP_PLUS)
            {
                ILOG_istatus(ISTATUS_FIBRE_COMLINK_STATE_UP, 0);
            }
        } 
        else
        {
            ILOG_istatus(ISTATUS_COMLINK_STATE_DN, 0);
        }
        // update the new link status, and tell the rest of the system
        comLinkContext.comLinkUp = linkUp;
        EVENT_Trigger(ET_COMLINK_STATUS_CHANGE, (uint32_t)comLinkContext.comLinkUp);
    }
}

//#################################################################################################
// Tells whether the link is up or down based on the most recent information available.
//
// Parameters:
// Return:
//      true if the link is up or false if it is down.
// Assumptions:
//#################################################################################################
static uint32_t ComlinkGetComlinkState(void)
{
    return comLinkContext.comLinkUp && (EVENT_GetEventInfo(ET_PHYLINK_STATUS_CHANGE) == LINK_OPERATING);
}


