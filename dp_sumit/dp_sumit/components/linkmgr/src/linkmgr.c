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
// This file contains the implementation of Link Manager.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// Link Manager looks at things from a PHY level, not a Lex-Rex communication level
// * Intended use is a notification system for the Lex-Rex communication layer
// * PHY_enable/disable are external events (imain/ICMDs)
// * PHY_restart is an external event
// * PHY_synced is an external event (XAUI)
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <bb_core.h>
#include <bb_chip_regs.h>
#include <xaui.h>
#include <mac.h>
#include <mca.h>
#include <timing_timers.h>
#include <gpio.h>
#include <callback.h>
#include <state_machine.h>
#include "linkmgr_log.h"
#include "linkmgr_loc.h"
#include <aquantia.h>
#include <aquantia_stat.h>
#include <configuration.h>
#include <event.h>
#include <fiber5g.h>
#include <led.h>

#include <uart.h>       // for UART_printf()

// Constants and Macros ###########################################################################

// debounce time used to switch between states (in milliseconds)
#define PHY_LINK_DEBOUNCE_TIMEOUT                   (125)

// Data Types #####################################################################################
enum PhyState
{
    PHY_STATE_DISABLED,             // PHY is held in reset and not enabled
    PHY_STATE_DELAYED_START,        // Delay after coming out of the disabled state
    PHY_STATE_WAIT_PHY_LINK_UP,     // Waiting for Phy link Up indication
    PHY_STATE_LINK_WAIT_REMOTE,     // Waiting for the Remote side to be available
    PHY_STATE_LINK_OPERATIONAL,     // Link is validated and operational
    PHY_STATE_PENDING_DISABLE,      // waiting for disable to finish
    PHY_STATE_PENDING_ENABLE,       // waiting for disable to finish, then enable the link manager again
    PHY_STATE_ERROR,                // PHY error state
    PHY_NUM_STATES
};

// TODO add a master timeout event, so if we get stuck we will go back to the idle state
enum PhyEvent
{
    PHY_EVENT_ENTER = UTILSM_EVENT_ENTER,   // 0
    PHY_EVENT_EXIT = UTILSM_EVENT_EXIT,     // 1
    PHY_EVENT_DISABLE,                      // 2 Phy shutdown request
    PHY_EVENT_ENABLE,                       // 3 Externally triggered, start Phy bringup process
    PHY_EVENT_RDY_FOR_SHUTDOWN,             // 4 Phy is ready to be shut down
    PHY_EVENT_DEBOUNCE,                     // 5 A debounce timeout occurred in this state
    PHY_EVENT_PHY_LINK_UP,                  // 6 Physical link (Catx or Fiber) is up
    PHY_EVENT_PHY_LINK_DOWN,                // 7 Physical link (Catx or Fiber) is down
    PHY_EVENT_LINK_REMOTE_OK,               // 8 MAC link layer is ok
    PHY_EVENT_FALLBACK_SPEED                // 9 Linked at unsupported speed
};

struct LinkMgrContext
{
    struct UtilSmInfo stateMachineInfo;             // LinkMgr PHY state.
    TIMING_TimerHandlerT  linkDebounceTimer;        // Link debounce timer
    enum LinkStatus linkStatus;                     // Stores the current link status of PHY
    bool shutdownLink;                              // another layer to control the Link manager
} ;

// Static Function Declarations ###################################################################
static void LinkMgrStateMachineSendEvent(enum PhyEvent event)           __attribute__ ((section(".atext")));
static void LinkMgrStateCallback(void *param1, void *param2)            __attribute__ ((section(".atext")));

// Phy manager state machine
static enum PhyState _LINKMGR_phyDisabledHandler(enum PhyEvent event, enum PhyState currentState)            __attribute__ ((section(".atext")));
static enum PhyState _LINKMGR_phyDelayedStart(enum PhyEvent event, enum PhyState currentState)               __attribute__ ((section(".atext")));
static enum PhyState _LINKMGR_phyWaitLinkUp(enum PhyEvent event, enum PhyState currentState)                 __attribute__ ((section(".atext")));
static enum PhyState _LINKMGR_phyWaitRemoteHandler(enum PhyEvent event, enum PhyState currentState)          __attribute__ ((section(".atext")));
static enum PhyState _LINKMGR_phyLinkOperational(enum PhyEvent event, enum PhyState currentState)            __attribute__ ((section(".atext")));
static enum PhyState _LINKMGR_phyPendingDisableHandler(enum PhyEvent event, enum PhyState currentState)      __attribute__ ((section(".atext")));
static enum PhyState _LINKMGR_phyPendingEnableHandler(enum PhyEvent event, enum PhyState currentState)       __attribute__ ((section(".atext")));
static enum PhyState _LINKMGR_phyErrorHandler(enum PhyEvent event, enum PhyState currentState)               __attribute__ ((section(".atext")));
static enum PhyState _LINKMGR_phyCommonHandler(enum PhyEvent event, enum PhyState currentState)              __attribute__ ((section(".atext")));

static void LinkMgrSignalReadyForShutdown(void)                         __attribute__ ((section(".atext")));

static void _LINKMGR_DebounceTimerHandler(void)                         __attribute__ ((section(".atext")));

static bool LinkMgrPhyEnableStatus(void)                                __attribute__ ((section(".atext")));
static enum ConfigBlockLinkSpeed LinkMgrGetPhyLinkSpeed(void)           __attribute__ ((section(".atext")));
static void LinkMgrPhyComlinkControl(bool enable)                       __attribute__ ((section(".atext")));
static void LinkMgrPhyBringUpLink(void)                                 __attribute__ ((section(".atext")));
static void LinkMgrPhyTakedownLink(void)                                __attribute__ ((section(".atext")));

// interface to the low level HW Phy
static void LINKMGR_HwPhyInit(void)                                     __attribute__ ((section(".atext")));
static void LINKMGR_HwPhyStart(void)                                    __attribute__ ((section(".atext")));
static void LINKMGR_HwPhyStop(void)                                     __attribute__ ((section(".atext")));
static void LINKMGR_HwPhyLinkStatus(enum LinkStatus)                    __attribute__ ((section(".atext")));

static void LINKMGR_MacLinkRxStatus(bool linkStatus)                    __attribute__ ((section(".atext")));

static uint32_t LinkMgrGetPhyLinkUpStatus(void);

// Global Variables ###############################################################################

// Static Variables ###############################################################################

static const EventHandler phyStateTable[PHY_NUM_STATES] = 
{
    [PHY_STATE_DISABLED]                = _LINKMGR_phyDisabledHandler,
    [PHY_STATE_DELAYED_START]           = _LINKMGR_phyDelayedStart,
    [PHY_STATE_WAIT_PHY_LINK_UP]        = _LINKMGR_phyWaitLinkUp,
    [PHY_STATE_LINK_WAIT_REMOTE]        = _LINKMGR_phyWaitRemoteHandler,
    [PHY_STATE_LINK_OPERATIONAL]        = _LINKMGR_phyLinkOperational,
    [PHY_STATE_PENDING_DISABLE]         = _LINKMGR_phyPendingDisableHandler,
    [PHY_STATE_PENDING_ENABLE]          = _LINKMGR_phyPendingEnableHandler,
    [PHY_STATE_ERROR]                   = _LINKMGR_phyErrorHandler,
};

static struct LinkMgrContext linkMgr = 
{
    .stateMachineInfo.stateHandlers = phyStateTable,
    .stateMachineInfo.logInfo.info.reserved = 0,
    .stateMachineInfo.logInfo.info.logLevel = (uint8_t)ILOG_MAJOR_EVENT,
    .stateMachineInfo.logInfo.info.ilogComponent = LINKMGR_COMPONENT,
    .stateMachineInfo.logInfo.info.ilogId = LINK_MGR_STATE_TRANSITION
};
// Exported Function Definitions ##################################################################

//#################################################################################################
// Register the generic timer and set the initial state.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void LINKMGR_init(void )
{
    linkMgr.linkDebounceTimer = TIMING_TimerRegisterHandler(
        _LINKMGR_DebounceTimerHandler,
        false,
        PHY_LINK_DEBOUNCE_TIMEOUT);

    LINKMGR_HwPhyInit(); // init the lower level Phy modules

    // note that care must be taken with this event, that the subscribers only receive
    // notifications when the event changes
    EVENT_Register(ET_PHYLINK_STATUS_CHANGE, LinkMgrGetPhyLinkUpStatus);

    LinkMgrStateMachineSendEvent(PHY_EVENT_ENTER);

    // setup the commlink
    LINKMGR_ComlinkInit();
}


//#################################################################################################
// External input to generate PHY Enable event.
//
// Parameters:
//      * enable        - if true, enable, if false, disable the PHY
// Return:
// Assumptions:
//#################################################################################################
void LINKMGR_phyShutdown(void)
{
    linkMgr.shutdownLink = true;
    LINKMGR_phyEnable(false);
}

//#################################################################################################
// External input to generate PHY Enable event.
//
// Parameters:
//      * enable        - if true, enable, if false, disable the PHY
// Return:
// Assumptions:
//#################################################################################################
void LINKMGR_phyEnable(bool enable)
{
    if (enable && LinkMgrPhyEnableStatus() && (linkMgr.shutdownLink == false))
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, PHY_ENABLED);
        LinkMgrStateMachineSendEvent(PHY_EVENT_ENABLE);
    }
    else
    {
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, PHY_DISABLED);
        LinkMgrStateMachineSendEvent(PHY_EVENT_DISABLE);
    }
}


//#################################################################################################
// Link down error detected by the upper layers - restart the link
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void LINKMGR_phyLinkDownErrorDetected(void)
{
    ilog_LINKMGR_COMPONENT_0(ILOG_MINOR_ERROR, LINK_HW_PHY_RESTART);
    LINKMGR_phyEnable(false);
    LINKMGR_phyEnable(true);
}


//#################################################################################################
// Comlink has detected that the system is up
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void LINKMGR_SystemUpDetected(void)
{
    switch(bb_core_getLinkMode())
    {
        case CORE_LINK_MODE_ONE_LANE_SFP_PLUS:
            Link_SL_5G_SystemUp();
            break;

        case CORE_LINK_MODE_AQUANTIA:
        default:
            break;
    }
}


//#################################################################################################
// External input to generate PHY Enable event.
//
// Parameters:
//      * enable        - if true, enable, if false, disable the PHY
// Return:
// Assumptions:
//#################################################################################################
void LINKMGR_phyEnableIcmd(uint8_t val)
{
    ConfigPhyParams *phyParams =  &(Config_GetBuffer()->phyParams);

    if (Config_ArbitrateGetVar(CONFIG_VAR_PHY_PARAMS, phyParams))
    {
        const uint8_t enablePhyBit = 1 << CONFIG_BLOCK_ENABLE_PHY;
        phyParams->phyConfig = (val != 0) ? (phyParams->phyConfig |  enablePhyBit) :
                                                  (phyParams->phyConfig & ~enablePhyBit);

        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_PHY_PARAMS, phyParams);
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, (val != 0) ? PHY_ENABLED : PHY_DISABLED);
    }

    LINKMGR_phyEnable(val); // also do this in real time
}

//#################################################################################################
// External command to toggle the phy enable state.  Useful for debugging phy up/down sequences
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void LINKMGR_phyToggleIcmd(void)
{
    LINKMGR_phyEnableIcmd(!LinkMgrPhyEnableStatus());
}


// Component Scope Function Definitions ###########################################################

//#################################################################################################
// External input to generate COMLINK Enable event.
//
// Parameters:
//      * enable        - if true, enable, if false, disable the PHY
// Return:
// Assumptions:
//#################################################################################################
void LINKMGR_comLinkEnableIcmd(uint8_t value)
{
    LinkMgrPhyComlinkControl(value);
}

//#################################################################################################
// Icmd Set link to 5G
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void LinkMgrSetLinkTo5GIcmd(void)
{
    ConfigPhyParams *phyParams =  &(Config_GetBuffer()->phyParams);

    if (Config_ArbitrateGetVar(CONFIG_VAR_PHY_PARAMS, phyParams))
    {
        phyParams->phyConfig |= (1 << CONFIG_BLOCK_PHY_LINK_SPEED_BIT_1);
        phyParams->phyConfig &= ~(1 << CONFIG_BLOCK_PHY_LINK_SPEED_BIT_0);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_PHY_PARAMS, phyParams);
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, NVM_CONFIG_STATUS_LINK_TO_5G);
    }
}


//#################################################################################################
// Icmd Set link to 10G
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void LinkMgrSetLinkTo10GIcmd(void)
{
    ConfigPhyParams *phyParams =  &(Config_GetBuffer()->phyParams);

    if (Config_ArbitrateGetVar(CONFIG_VAR_PHY_PARAMS, phyParams))
    {
        phyParams->phyConfig |= (1 << CONFIG_BLOCK_PHY_LINK_SPEED_BIT_1);
        phyParams->phyConfig |= (1 << CONFIG_BLOCK_PHY_LINK_SPEED_BIT_0);
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VAR_PHY_PARAMS, phyParams);
        ilog_LINKMGR_COMPONENT_0(ILOG_USER_LOG, NVM_CONFIG_STATUS_LINK_TO_10G);
    }
}


// Static Function Definitions ####################################################################

//#################################################################################################
// Entry point to send an event to the link manager state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LinkMgrStateMachineSendEvent(enum PhyEvent event)
{
    // this just deals with cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
    uint32_t eventx = event;

    CALLBACK_Run(LinkMgrStateCallback, (void *)eventx, NULL);
}

//#################################################################################################
// Callback wrapper for the link manager state machine
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LinkMgrStateCallback(void *param1, void *param2)
{
    UTILSM_PostEvent(&linkMgr.stateMachineInfo,
                    (uint32_t)param1,
                     param2);
}

//#################################################################################################
// Handles events in PHY_STATE_DISABLED
//
// Parameters:
//      event                   - PHY event
// Return:
// Assumptions:
//
//#################################################################################################
static enum PhyState _LINKMGR_phyDisabledHandler(enum PhyEvent event, enum PhyState currentState)
{
    enum PhyState newState = (enum PhyState)currentState;
    if (event == PHY_EVENT_ENTER)
    {
        if (linkMgr.stateMachineInfo.prevState != PHY_STATE_DELAYED_START)
        {
            LinkMgrPhyTakedownLink();

            // indicate Phy link down if we haven't already done so
            if (linkMgr.linkStatus != LINK_IN_RESET)
            {
                linkMgr.linkStatus = LINK_IN_RESET;
                EVENT_Trigger(ET_PHYLINK_STATUS_CHANGE, LINK_IN_RESET );    // signal link down
            }
        }
    }
    else if (event == PHY_EVENT_ENABLE)
    {
        newState = PHY_STATE_DELAYED_START;
    }

    return newState;
}


//#################################################################################################
// Handles events in PHY_STATE_DELAYED_START - wait PHY_LINK_DEBOUNCE_TIMEOUT before starting
// (if we are in the middle of a link restart, this delay gives the rest of the system a chance
//  to properly shut down before we start up again)
//
// Parameters:
//      event                   - PHY event
// Return:
// Assumptions:
//
//#################################################################################################
static enum PhyState _LINKMGR_phyDelayedStart(enum PhyEvent event, enum PhyState currentState)
{
    enum PhyState newState = (enum PhyState)currentState;
    if (event == PHY_EVENT_ENTER)
    {
        TIMING_TimerStart(linkMgr.linkDebounceTimer);
    }
    else if (event == PHY_EVENT_EXIT)
    {
        TIMING_TimerStop(linkMgr.linkDebounceTimer);        // stop the debounce timer, if running
    }
    else if (event == PHY_EVENT_DISABLE)
    {
        newState = PHY_STATE_DISABLED;       // we are disabled
    }
    else if (event == PHY_EVENT_DEBOUNCE)
    {
        newState = PHY_STATE_WAIT_PHY_LINK_UP;
    }

    return newState;
}


//#################################################################################################
// Handles events in PHY_STATE_WAIT_PHY_LINK_UP
//
// Parameters:
//      event                   - PHY event
// Return:
// Assumptions:
//
//#################################################################################################
static enum PhyState _LINKMGR_phyWaitLinkUp(enum PhyEvent event, enum PhyState currentState)
{
    enum PhyState newState = (enum PhyState)currentState;
    if (event == PHY_EVENT_ENTER)
    {
        LINKMGR_HwPhyStart();   // Run Aquantia reset timer ensuring reset low(120ms)/high(120ms)
    }
    else if (event == PHY_EVENT_PHY_LINK_UP)
    {
        newState = PHY_STATE_LINK_WAIT_REMOTE;
    }
    else if (event == PHY_EVENT_FALLBACK_SPEED)
    {
        newState = PHY_STATE_ERROR;
    }
    else
    {
        newState =  _LINKMGR_phyCommonHandler(event, currentState);
    }

    return newState;
}


//#################################################################################################
// Handles events in PHY_STATE_LINK_WAIT_REMOTE - Phy is in the link up state, just waiting for 
// the MAC layer to say everything is ok
//
// Parameters:
//      event                   - PHY event
// Return:
// Assumptions:
//
//#################################################################################################
static enum PhyState _LINKMGR_phyWaitRemoteHandler(enum PhyEvent event, enum PhyState currentState)
{
    enum PhyState newState = (enum PhyState)currentState;
    if (event == PHY_EVENT_ENTER)
    {
        MAC_RemoteFaultEnable(false);    // Rx is stable, turn off remote fault signaling
    }
    else if (event == PHY_EVENT_PHY_LINK_DOWN)
    {
        LINKMGR_phyLinkDownErrorDetected(); // bad error - do a link restart
    }
    else if (event == PHY_EVENT_LINK_REMOTE_OK)
    {
        newState = PHY_STATE_LINK_OPERATIONAL;
    }
    else if (event == PHY_EVENT_FALLBACK_SPEED)
    {
        newState = PHY_STATE_ERROR;
    }
    else
    {
        newState =  _LINKMGR_phyCommonHandler(event, currentState);
    }

    return newState;
}


//#################################################################################################
// Handles events in PHY_STATE_LINK_OPERATIONAL
//
// Parameters:
//      event                   - PHY event
// Return:
// Assumptions:
//#################################################################################################
static enum PhyState _LINKMGR_phyLinkOperational(enum PhyEvent event, enum PhyState currentState)
{
    enum PhyState newState = (enum PhyState)currentState;
    if (event == PHY_EVENT_ENTER)
    {
        LinkMgrPhyBringUpLink();            // link is up - go to the link up state
    }
    else if (event == PHY_EVENT_DISABLE)
    {
        LINKMGR_ComlinkEnable(false);  // we are disabled; make sure the comlink is, too

        linkMgr.linkStatus = LINK_IN_RESET;
        EVENT_Trigger(ET_PHYLINK_STATUS_CHANGE, LINK_IN_RESET );    // signal link down

        if(bb_core_getLinkMode() == CORE_LINK_MODE_ONE_LANE_SFP_PLUS)
        {
            newState = PHY_STATE_DISABLED;
        }
        else
        {
            MAC_RemoteFaultEnable(true);    // turn on remote fault signaling we are down

            // we want to get the final status of Aquantia before we shut it down
            MDIOD_AquantiaStopStatsMonitor(LinkMgrSignalReadyForShutdown);

            // wait for Aquantia to finish
            newState = PHY_STATE_PENDING_DISABLE;
        }
    }
    else if (event == PHY_EVENT_PHY_LINK_DOWN)
    {
        LINKMGR_phyLinkDownErrorDetected(); // bad error - do a link restart
    }
    else if (event == PHY_EVENT_FALLBACK_SPEED)
    {
        newState = PHY_STATE_ERROR;
    }
    else
    {
        newState =  _LINKMGR_phyCommonHandler(event, currentState);
    }

    return newState;
}


//#################################################################################################
// Handles events in PHY_STATE_PENDING_DISABLE
//
// Parameters:
//      event                   - PHY event
// Return:
// Assumptions:
//
//#################################################################################################
static enum PhyState _LINKMGR_phyPendingDisableHandler(enum PhyEvent event, enum PhyState currentState)
{
    enum PhyState newState = (enum PhyState)currentState;
    if (event == PHY_EVENT_ENABLE)
    {
        newState = PHY_STATE_PENDING_ENABLE;
    }
    else if (event == PHY_EVENT_RDY_FOR_SHUTDOWN)
    {
        newState = PHY_STATE_DISABLED;
    }
    else if (event == PHY_EVENT_DISABLE)
    {
        // block this event from being processed by the common handler; we are already disabled
    }
    else
    {
        newState =  _LINKMGR_phyCommonHandler(event, currentState);
    }

    return newState;
}


//#################################################################################################
// Handles events in PHY_STATE_PENDING_ENABLE
//
// Parameters:
//      event                   - PHY event
// Return:
// Assumptions:
//
//#################################################################################################
static enum PhyState _LINKMGR_phyPendingEnableHandler(enum PhyEvent event, enum PhyState currentState)
{
    enum PhyState newState = (enum PhyState)currentState;
    if (event == PHY_EVENT_DISABLE)
    {
        newState = PHY_STATE_PENDING_DISABLE;
    }
    else if (event == PHY_EVENT_RDY_FOR_SHUTDOWN)
    {
        newState = PHY_STATE_DISABLED;
        LinkMgrStateMachineSendEvent(PHY_EVENT_ENABLE);
    }
    else
    {
        newState =  _LINKMGR_phyCommonHandler(event, currentState);
    }

    return newState;
}


//#################################################################################################
// Handles events in PHY_STATE_ERROR
//
// Parameters:
//      event                   - PHY event
// Return:
// Assumptions:
//
//#################################################################################################
static enum PhyState _LINKMGR_phyErrorHandler(enum PhyEvent event, enum PhyState currentState)
{
    enum PhyState newState = (enum PhyState)currentState;
    if (event == PHY_EVENT_ENTER)
    {
        linkMgr.linkStatus = LINK_ERROR;
        EVENT_Trigger(ET_PHYLINK_STATUS_CHANGE, LINK_ERROR);
    }
    else
    {
        newState =  _LINKMGR_phyCommonHandler(event, currentState);
    }

    return newState;
}


//#################################################################################################
// Common handler for PHY events
//
// Parameters:
//      event                   - PHY event
// Return:
// Assumptions:
//
//#################################################################################################
static enum PhyState _LINKMGR_phyCommonHandler(enum PhyEvent event, enum PhyState currentState)
{
    enum PhyState newState = currentState;

    switch(event)
    {
        case PHY_EVENT_ENTER:
        case PHY_EVENT_EXIT:
            break;

        case PHY_EVENT_DISABLE:
            newState = PHY_STATE_DISABLED;
            break;

        case PHY_EVENT_FALLBACK_SPEED:
        case PHY_EVENT_ENABLE:   
        case PHY_EVENT_RDY_FOR_SHUTDOWN:
        case PHY_EVENT_DEBOUNCE:
        case PHY_EVENT_PHY_LINK_UP:
        case PHY_EVENT_PHY_LINK_DOWN:
        case PHY_EVENT_LINK_REMOTE_OK:

        default:
            ilog_LINKMGR_COMPONENT_2(ILOG_MINOR_ERROR, LINK_MGR_INVALID_EVENT, event, currentState);
            break;
    }
    return newState;
}


//#################################################################################################
// Initializes the low level HW Phy
//
// Parameters:
// Return:
// Assumptions:  Only called once, at system startup
//
//#################################################################################################
static void LINKMGR_HwPhyInit(void)
{
//    UART_printf("LINKMGR_HwPhyInit %d\n", bb_core_getLinkMode());
    switch(bb_core_getLinkMode())
    {
        case CORE_LINK_MODE_ONE_LANE_SFP_PLUS:
            Link_SL_5G_Init(LINKMGR_HwPhyLinkStatus, LINKMGR_phyLinkDownErrorDetected);
            break;

        case CORE_LINK_MODE_AQUANTIA:
            MDIOD_aquantiaPhyInit(LINKMGR_HwPhyLinkStatus, LINKMGR_phyLinkDownErrorDetected);
            break;

        default:
            break;
    }

    bb_top_LinkClockEnable(true);   // turn on the link PLL, wait for it to lock
}


//#################################################################################################
// Tells the Low level HW Phy to start operation
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LINKMGR_HwPhyStart(void)
{
    ilog_LINKMGR_COMPONENT_1(ILOG_MINOR_EVENT, LINK_HW_PHY_START, bb_core_getLinkMode());

    // before we start the PHY, make sure the Rx path is enabled
    MCA_EnableRx();
    MCA_EnableTx();
    MAC_EnableLayer3Rx();
    MAC_EnableRx(LINKMGR_MacLinkRxStatus);

    MAC_EnableTx();                 // MAC Tx is active for Local and Remote Fault signaling
    MAC_RemoteFaultEnable(true);    // turn on remote fault signaling until Rx is stable

    XAUI_EnableRx();  // enable Rxaui and SL 5G stability stats

    switch(bb_core_getLinkMode())
    {
        case CORE_LINK_MODE_ONE_LANE_SFP_PLUS:
            Link_SL_5G_Control(true);      // turn on the 5G, wait for link up
            break;

        case CORE_LINK_MODE_AQUANTIA:
            // Initialize Aquantia next
            MDIOD_aquantiaPhySetup(LinkMgrGetPhyLinkSpeed());   // get the Aquantia Phy running...
            break;

        default:
            break;
    }
}


//#################################################################################################
// Tells the low level Phy to go into a stopped state
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LINKMGR_HwPhyStop(void)
{
    switch(bb_core_getLinkMode())
    {
        case CORE_LINK_MODE_ONE_LANE_SFP_PLUS:
            Link_SL_5G_Control(false);      // turn off the 5G
            break;

        case CORE_LINK_MODE_AQUANTIA:
            MDIOD_aquantiaPhyDisable(); // make sure Aquantia is in reset
           break;

        default:
            break;
    }

    XAUI_DisableRx();
}

//#################################################################################################
// External input to signal that the HW is linked or not
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LINKMGR_HwPhyLinkStatus(enum LinkStatus linkStatus)
{
    ilog_LINKMGR_COMPONENT_1(ILOG_MINOR_EVENT, LINK_HW_PHY_LINK_STATUS, linkStatus);
    switch(linkStatus)
    {
        case LINK_IN_RESET:
            LinkMgrStateMachineSendEvent(PHY_EVENT_PHY_LINK_DOWN);
            break;

        case LINK_OPERATING:
            LinkMgrStateMachineSendEvent(PHY_EVENT_PHY_LINK_UP);
            break;

        case LINK_ERROR:
            LinkMgrStateMachineSendEvent(PHY_EVENT_FALLBACK_SPEED);
            break;

        default:
            (linkMgr.stateMachineInfo.currentState == PHY_STATE_LINK_OPERATIONAL) ? 
                LinkMgrStateMachineSendEvent(PHY_EVENT_PHY_LINK_UP) : LinkMgrStateMachineSendEvent(PHY_EVENT_PHY_LINK_DOWN);
            break;
    }
}


//#################################################################################################
// External input to signal that the link layer Rx is ok (true) or receiving faults (false)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LINKMGR_MacLinkRxStatus(bool linkStatus)
{
    ilog_LINKMGR_COMPONENT_1(ILOG_MINOR_EVENT, LINK_HW_PHY_MAC_LINK_RX_STATUS, linkStatus);
    if (linkStatus)
    {
        LinkMgrStateMachineSendEvent(PHY_EVENT_LINK_REMOTE_OK);
    }
}


//#################################################################################################
// Get the enabled status for the Phy manager. Returns true if enabled, false if not
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static bool LinkMgrPhyEnableStatus(void)
{
    ConfigPhyParams *phyParams =  &(Config_GetBuffer()->phyParams);

    if (Config_ArbitrateGetVar(CONFIG_VAR_PHY_PARAMS, phyParams))
    {
        const uint8_t linkType = phyParams->phyConfig;
        const bool phyEnable   = linkType & (1 << CONFIG_BLOCK_ENABLE_PHY);

        return (phyEnable);
    }

    return (false); // if we can't find the variable, ASSUME we are disabled
}

//#################################################################################################
// Get the configured link speed for the Phy
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static enum ConfigBlockLinkSpeed LinkMgrGetPhyLinkSpeed(void)
{
    ConfigPhyParams *phyParams =  &(Config_GetBuffer()->phyParams);

    if (Config_ArbitrateGetVar(CONFIG_VAR_PHY_PARAMS, phyParams))
    {
        const enum ConfigBlockLinkSpeed linkSpeed =
            phyParams->phyConfig & ((1 << CONFIG_BLOCK_PHY_LINK_SPEED_BIT_0) | (1 << CONFIG_BLOCK_PHY_LINK_SPEED_BIT_1));

        return (linkSpeed);
    }
    else
    {
        return (CONFIG_BLOCK_LINK_SPEED_10G);
    }
}


//#################################################################################################
// Phy link is up - bring up the dependent components
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LinkMgrPhyBringUpLink(void)
{
    // the MCA modules can't use the Phy link up event because we can't guarantee the order
    // they will be called with events.  We call them inline here to ensure the correct order
    MAC_EnableLayer3Tx();
    MAC_StartStatsMonitor();
    linkMgr.linkStatus = LINK_OPERATING;
    EVENT_Trigger(ET_PHYLINK_STATUS_CHANGE, LINK_OPERATING ); // signal phy link up!
}


//#################################################################################################
// Phy link is down - make sure all dependent components are down, too
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LinkMgrPhyTakedownLink(void)
{
    // disable the modules in the reverse order from when they were enabled
    MAC_StopStatsMonitor();             // MAC layer is in reset - stop collecting stats

    MAC_DisableLayer3Tx();
    MAC_DisableTx();

    LINKMGR_HwPhyStop();    // turn off the low level Phy

    MAC_DisableRx();
    MAC_DisableLayer3Rx();
    MCA_DisableTx();
    MCA_DisableRx();
}


//#################################################################################################
// Returns whether the Phy link is up or not
//
// Parameters:
// Return:  true if the Phy link is up, false otherwise
// Assumptions:
//
//#################################################################################################
static uint32_t LinkMgrGetPhyLinkUpStatus(void)
{
    return (linkMgr.linkStatus);
}

//#################################################################################################
// Phy is ready to be shut down
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LinkMgrSignalReadyForShutdown(void)
{
    LinkMgrStateMachineSendEvent(PHY_EVENT_RDY_FOR_SHUTDOWN);
}

//#################################################################################################
// Debounce timeout occurred
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _LINKMGR_DebounceTimerHandler(void)
{
    LinkMgrStateMachineSendEvent(PHY_EVENT_DEBOUNCE);
}

//#################################################################################################
// Enables/Disables the Comlink
//
// Parameters: enable - true to enable the comlink, false to turn it off
// Return:
// Assumptions:
//
//#################################################################################################
static void LinkMgrPhyComlinkControl(bool enable)
{
    LINKMGR_ComlinkEnable(enable);
}

