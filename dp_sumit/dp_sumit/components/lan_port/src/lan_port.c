//#################################################################################################
// Icron Technology Corporation - Copyright 2017
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
// This file contains code for the LAN port on Blackbird
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <configuration.h>      // used for USB enables
#include <event.h>
#include <bb_top.h>
#include <mca.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <timing_profile.h>
#include <cpu_comm.h>
#include <bb_core.h>
#include <interrupts.h>
#include <mdiod_phy_driver.h>
#include <uart.h>
#include <event.h>

#include <lan_port.h>
#include "lan_port_loc.h"
#include "lan_port_log.h"

// Constants and Macros ###########################################################################

#define MII_IPG_MINUS1           (22) // Remco says 100 and 10BaseT are same value for IPG
#define GMII_IPG_MINUS1          (11)

// Data Types #####################################################################################

// structure used to
struct LanPortCpuMessage
{
    uint8_t lanPortState;
    uint8_t deviceSpeed;
};

// Static Function Declarations ###################################################################

static void LanPortCommLinkEventHandler(uint32_t linkUp, uint32_t userContext)              __attribute__((section(".atext")));
static bool LanPortEnabledStatus(void)                                                      __attribute__((section(".atext")));

static void initGmiiDone(void)                                                              __attribute__((section(".atext")));
static void LanPortReceiveCpuMsgHandler(
    uint8_t subType, const uint8_t *msg, uint16_t msgLength)                                __attribute__((section(".atext")));
static void LanPortSendCPUCommMessage(struct LanPortCpuMessage message)                     __attribute__((section(".atext")));
static void xmiiBasicConfigDone(void)                                                       __attribute__((section(".atext")));
static void xmiiDeviceConnectReadHandler(bool device_connected)                             __attribute__((section(".atext")));
static void xmiiSetupGmiiStep2(void)                                                        __attribute__((section(".atext")));
static void xmiiSetupMii(void)                                                              __attribute__((section(".atext")));
static void LanPortSendConnectedMessage(void)                                               __attribute__((section(".atext")));
static void LanPortSendDisconnectedMessage(void)                                            __attribute__((section(".atext")));
static void LanPortConfigureXmii(void)                                                      __attribute__((section(".atext")));
static void LanPortXmiiResetClocks(void)                                                    __attribute__((section(".atext")));
static void LanPortChannelStatus(enum MCA_ChannelStatus channelStatus)                      __attribute__((section(".atext")));
static void LanPortConfigurationEventHandler(uint32_t eventInfo, uint32_t userContext)      __attribute__((section(".atext")));
// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct LanPortContext
{
    // a timer used to assert the reset line for the correct length of time
    enum XmiiPhySpeed speed;    // Speed of the local LAN port
    bool devicesConnected;      // set if both the local and remote devices are connected
    bool lanPortEnabled;        // true if we are enabled
    bool lanPortDisableMsgSent; // message that we are disabled has been sent
} lanPort;

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize the LAN Port
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void LANPORT_LanPortInit(void)
{
    MCA_ChannelInit(MCA_CHANNEL_NUMBER_GMII, LanPortChannelStatus, NULL);
    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_LAN_PORT, LanPortReceiveCpuMsgHandler);

    MDIOD_RegisterPhyDeviceStatusHandler(xmiiDeviceConnectReadHandler);
    EVENT_Subscribe(ET_COMLINK_STATUS_CHANGE, LanPortCommLinkEventHandler, 0);
    EVENT_Subscribe(ET_CONFIGURATION_CHANGE, LanPortConfigurationEventHandler, 0);

    lanPort.lanPortEnabled = LanPortEnabledStatus();
    LanPortCommLinkEventHandler( EVENT_GetEventInfo(ET_COMLINK_STATUS_CHANGE), 0);
}


// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Sends the specified CPU message to the other side, Lex or Rex
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
void LanPortSendCPUCommMessage(struct LanPortCpuMessage message)
{
    ilog_LAN_PORT_COMPONENT_2(ILOG_DEBUG, LAN_PORT_TX_CPU_MSG, message.lanPortState, message.deviceSpeed);
    CPU_COMM_sendMessage(CPU_COMM_TYPE_LAN_PORT, 0, (const uint8_t*)&message, sizeof(message));
}

// Static Function Definitions ####################################################################


//#################################################################################################
// Handler for configuration change events.
//
// Parameters: eventInfo
// Return:
// Assumptions:
//
//#################################################################################################
static void LanPortConfigurationEventHandler(uint32_t eventInfo, uint32_t userContext)
{
    if(eventInfo == CONFIG_VARS_BB_FEATURE_CONTROL)
    {
        lanPort.lanPortEnabled = LanPortEnabledStatus();
        LanPortCommLinkEventHandler( EVENT_GetEventInfo(ET_COMLINK_STATUS_CHANGE), 0);
    }
}

//#################################################################################################
// Handler for communication link up/down events.
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LanPortCommLinkEventHandler(uint32_t linkUp, uint32_t userContext)
{
    if (linkUp && lanPort.lanPortEnabled)
    {
        ilog_LAN_PORT_COMPONENT_0(ILOG_MINOR_EVENT, LAN_PORT_COMLINK_UP_EVENT);

        // Take Enet PHY out of reset and configure
        MDIOD_enetEnable(xmiiBasicConfigDone);
    }
    else    // comlink is down
    {
        ilog_LAN_PORT_COMPONENT_0(ILOG_MINOR_EVENT, LAN_PORT_COMLINK_DOWN_EVENT);

        // ASSUME no devices are connected until we are told otherwise when the link
        // comes back up
        MDIOD_enetDisable(); // place PHY in reset
        lanPort.devicesConnected  = false;
        lanPort.lanPortDisableMsgSent = false;  // send another message if required when the link is up
        lanPort.speed = XMII_INVALID_DEVICE_SPEED;

        // put the LAN (GMII) clocks into reset
        bb_top_applyXmiiRxReset(true);
        bb_top_applyXmiiTxReset(true);
    }
}

//#################################################################################################
// The handler for receiving a CPU message from the other device.
//
// Parameters:
//      msg                     - message data
//      msgLength               - message length
// Return:
// Assumptions:
//      * The message length is always 1 byte.
//#################################################################################################
static void LanPortReceiveCpuMsgHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    struct LanPortCpuMessage *rxmsg = (struct LanPortCpuMessage *)msg;

    iassert_LAN_PORT_COMPONENT_1(msgLength == 2, LAN_PORT_UNEXPECTED_CPU_MSG_LENGTH, msgLength);
    ilog_LAN_PORT_COMPONENT_2(ILOG_DEBUG, LAN_PORT_RCV_CPU_MSG, rxmsg->lanPortState, rxmsg->deviceSpeed);

    if (lanPort.lanPortEnabled == false)
    {
        if (lanPort.lanPortDisableMsgSent == false)
        {
            // we are not enabled - tell the other side we don't have a device connected
            // but only send it once per link cycle, to avoid spamming
            LanPortSendDisconnectedMessage();
            lanPort.lanPortDisableMsgSent = true;
        }
        return;
    }

    enum LanPortCpuCommMessage remoteMessage = (enum LanPortCpuCommMessage) (rxmsg->lanPortState);
    enum XmiiPhySpeed remoteSpeed = rxmsg->deviceSpeed;

    switch(remoteMessage)
    {
        case LAN_PORT_CPU_COMM_DEVICE_CONNECTED:        // device connected to the remote's Lan Port
            // is there a device connected on our side?  Is the Ethernet Phy still in reset?
            if ( MDIOD_isPhyDeviceConnected() )
            {
                // device connected on our side, too.  Only act if we haven't previously connected
                // we don't connect unless the speeds match
                if (lanPort.devicesConnected == false)
                {
                    // Determine minimum speed between local and remote
                    // Force PHY advertisement to that minimum if we're greater than
                    // remote speed
                    // This process will take down the PHY-PHY link, when it comes back up, LinkUp
                    // event, we will check the local speed again and re-assign it then, and then
                    // continue with the Xmii initialization
                    // Disconnect "ISR" should not be called - phyLinkStateUp is set to down by
                    // setPhySpeed function, preventing the triggering of the disconnect
                    if (lanPort.speed > remoteSpeed)
                    {
                        MDIOD_setPhySpeed(remoteSpeed);
                    }
                    else if (lanPort.speed == remoteSpeed)
                    {
                        lanPort.devicesConnected = true;
                        LanPortConfigureXmii();
                    }
                    else // Remote speed greater than local speed - resend our status and speed
                    {
                        // Resending local speed allows the remote to determine its speed is
                        // greater than this speed and it will lower the speed on its own lanPort
                        LanPortSendConnectedMessage();
                    }
                }
            }
            else
            {
                // no device - make sure we are in the removed state
                LanPortSendDisconnectedMessage();
            }
            break;

        case LAN_PORT_CPU_COMM_DEVICE_REMOVED:      // device disconnected from the remote's Lan Port
        default:
            if (lanPort.devicesConnected)
            {
                // Remote device is disconnected, and given the PHY DRIVER is polling, the
                // transmission of data will have stopped by the time this CPU message is
                // received
                // Take down our MCA so we stop transmitting to the remote,
                // then signal to the remote it is safe to bring down its own MCA
                lanPort.devicesConnected = false;
                lanPort.speed = XMII_INVALID_DEVICE_SPEED;
                MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_GMII);
                LanPortXmiiResetClocks();
                LanPortSendDisconnectedMessage();
                MDIOD_setPhySpeed(XMII_1000MBPS);
            }
            break;
    }
}


//#################################################################################################
// Basic PHY MDIO config completed, await PhyDriver to call
// xmiiDeviceConnectReadHandler to handle device connect/disconnect
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void xmiiBasicConfigDone(void)
{
}

//#################################################################################################
// Device Connected handler - called whenever the PHY to PHY Link State changes (down or up)
// PHY link up (true) means a device is connected, and down (false) means device disconnected
// Call the appropriate function to handle the situation
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void xmiiDeviceConnectReadHandler(bool device_connected)
{
    enum XmiiPhySpeed speed = MDIOD_GetPhySpeed();  // get the speed of this connection
    ilog_LAN_PORT_COMPONENT_2(ILOG_MINOR_EVENT, LAN_PORT_DEVICE_STATE_CHANGE, device_connected, speed);

    if (lanPort.lanPortEnabled == false)
    {
        return;
    }

    // Only configure xmii if currently not configured already
    if (device_connected)
    {
        if (speed != XMII_INVALID_DEVICE_SPEED)
        {
            // Send message of device speed
            lanPort.speed = speed;
            LanPortSendConnectedMessage();
        }
    }
    else if ((!device_connected) && (lanPort.devicesConnected))
    {
        // If local device disconnected, send message for remote to take down the MCA first
        // Remote will then send a disconnected message and that's when the local MCA is taken down
        LanPortSendDisconnectedMessage();
    }
    else // do nothing
    {
    }
}


//#################################################################################################
// Setup GMII part 2
// Set GTX CLK, XMII<->PTP, tri-states are off, GTX CLK enabled
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void xmiiSetupGmiiStep2(void)
{
    bb_top_xmiiTxClkDetect(false); // GTX CLK
    bb_core_xmiiCtrl(XMII_CTRL_MODE_GMII, GMII_IPG_MINUS1);
    bb_top_gmiiCtrlSetTristates(false); // Turn off all tristates
    bb_top_xmiiGtxClkEn(true);

    initGmiiDone();
}


//#################################################################################################
// Setup MII
// Set TX CLK, XMII<->PTP, tri-states are off, GTX CLK disabled
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void xmiiSetupMii(void)
{
    bb_top_xmiiTxClkDetect(true); // TX CLK
    bb_core_xmiiCtrl(XMII_CTRL_MODE_MII, MII_IPG_MINUS1);
    bb_top_miiCtrlSetTristates(false); // Turn off most tri-states (TXD 7:4 are tri-stated)
    bb_top_xmiiGtxClkEn(false);

    initGmiiDone();
}


//#################################################################################################
// PHY Configuration and XMII Setup Completed
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void initGmiiDone(void)
{
    if (lanPort.speed == XMII_1000MBPS)
    {
        ilog_LAN_PORT_COMPONENT_0(ILOG_DEBUG, LAN_PORT_PHY_INIT_COMPLETED_1G);
    }
    else if (lanPort.speed == XMII_100MBPS)
    {
        ilog_LAN_PORT_COMPONENT_0(ILOG_DEBUG, LAN_PORT_PHY_INIT_COMPLETED_100M);
    }

    // take the LAN clocks out of reset
    bb_top_applyXmiiTxReset(false);
    bb_top_applyXmiiRxReset(false);

    // now start bringing up the MCA channel
    MCA_ChannelLinkUp(MCA_CHANNEL_NUMBER_GMII);

    // tell the other side our our current connection status
    // at this point they have already negotiated the lower rate
    // this just tells them to set their clocks and bring up their side of the MCA channel
    LanPortSendConnectedMessage();
}


//#################################################################################################
// Get the enabled status for the LAN port Returns true if enabled, false if not
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static bool LanPortEnabledStatus(void)
{
    bool lanPortEnable = false;

    ConfigBlocksEnable *blocksEnabled =  &(Config_GetBuffer()->featureControl);
    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_CONTROL, blocksEnabled))
    {
        lanPortEnable = blocksEnabled->MiscControl & (1 << CONFIG_BLOCK_ENABLE_GMII);
    }

    return (lanPortEnable);
}

//#################################################################################################
// Send connected message to remote
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LanPortSendConnectedMessage(void)
{
    struct LanPortCpuMessage message = {0};

    message.lanPortState  = LAN_PORT_CPU_COMM_DEVICE_CONNECTED;
    message.deviceSpeed   = lanPort.speed;

    LanPortSendCPUCommMessage(message);
}


//#################################################################################################
// Send disconnected message to remote
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LanPortSendDisconnectedMessage(void)
{
    // tell the other side we are disconnected
    const struct LanPortCpuMessage message = {LAN_PORT_CPU_COMM_DEVICE_REMOVED, XMII_INVALID_DEVICE_SPEED};

    LanPortSendCPUCommMessage(message);
}


//#################################################################################################
// Handles the setup of the XMII based upon speed
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LanPortConfigureXmii(void)
{
    switch(lanPort.speed)
    {
        case XMII_10MBPS:
        case XMII_100MBPS:
            xmiiSetupMii(); // setup is the same for 10 or 100Mbps
            break;

        case XMII_1000MBPS:
            MDIOD_enetEnable125MHzPhyClk(&xmiiSetupGmiiStep2);
            break;

        case XMII_INVALID_DEVICE_SPEED:
        default:
            break;
    }
}


//#################################################################################################
// Reset the XMII Tx and RX clocks
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void LanPortXmiiResetClocks(void)
{
    bb_top_applyXmiiRxReset(true);
    bb_top_applyXmiiTxReset(true);
}

//#################################################################################################
// Handler for LanPort channel status changes
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
static void LanPortChannelStatus(enum MCA_ChannelStatus channelStatus)
{
    ilog_LAN_PORT_COMPONENT_1(ILOG_DEBUG, LAN_PORT_MCA_CHANNEL_STATUS, channelStatus);

    switch (channelStatus)
    {
        case MCA_CHANNEL_STATUS_LINK_ACTIVE:        // channel is linked between Lex and Rex
            MCA_ChannelTxRxSetup(MCA_CHANNEL_NUMBER_GMII);    // now setup Tx and Rx
            break;

        case MCA_CHANNEL_STATUS_CHANNEL_READY:     // channel is linked, and Rx, Tx is setup.  Ready for operation
           break;

        case MCA_CHANNEL_STATUS_CHANNEL_DISABLED:   // channel is disabled
        case MCA_CHANNEL_STATUS_LINK_DOWN:          // channel is down between Lex and Rex, needs to be re-initialized
        default:
            break;
    }
}


