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
// Implementations of functions related to RS232 pipe.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################


// Includes #######################################################################################
#include <options.h>
#include <bb_core.h>
#include <bb_top.h>
#include <event.h>
#include <mca.h>
#include <leon_timers.h>
#include <cpu_comm.h>
#include <configuration.h>
#include "rs232_log.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// CPU messages sent from one device to the other
enum Rs232CpuCommMessage
{
    RS232_CPU_COMM_DEVICE_CONNECTED,    // device connected to the remote's RS232 port
    RS232_CPU_COMM_DEVICE_REMOVED,      // device disconnected from the remote's RS232 port
};

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static struct
{
    bool rs232LinkUp;   // used to signal when both RS232 ports are configured and operational
    bool rs232Enabled;  // used to keep state of the commanded operation - even after link down
} rs232Context;

// Static Function Declarations ###################################################################
static void RS232_enableConfigureHW(bool enable)                                __attribute__((section(".atext")));
static void RS232_LinkEventHandler(uint32_t linkUp, uint32_t userContext)       __attribute__((section(".atext")));
static void RS232_SendCPUCommMessage(uint8_t message)                           __attribute__((section(".atext")));
static void RS232_ReceiveCpuMsgHandler(
    uint8_t subType, const uint8_t *msg, uint16_t msgLength)                    __attribute__((section(".atext")));
static void RS232ChannelStatus(enum MCA_ChannelStatus channelStatus)            __attribute__((section(".atext")));
static void RS232ConfigurationEventHandler(uint32_t eventInfo, uint32_t userContext) __attribute__((section(".atext")));
static bool RS232EnabledStatus(void)                                            __attribute__((section(".atext")));

// Exported Function Definitions ##################################################################
//#################################################################################################
// Initialize the RS232 chip and configure the supporting FPGA registers, and register to LINK UP
// event for handling the MCA bring up and take down
//
// Parameters:
// Return:
// Assumptions:
// * We only enable event subcription for BoxProducts, not core products!
//#################################################################################################
void RS232_Init(void)
{
    MCA_ChannelInit(MCA_CHANNEL_NUMBER_RS232, RS232ChannelStatus, NULL);
    CPU_COMM_RegisterHandler(CPU_COMM_TYPE_RS232, RS232_ReceiveCpuMsgHandler);

    EVENT_Subscribe(ET_COMLINK_STATUS_CHANGE, RS232_LinkEventHandler, 0);
    EVENT_Subscribe(ET_CONFIGURATION_CHANGE, RS232ConfigurationEventHandler, 0);

    rs232Context.rs232Enabled = RS232EnabledStatus(); //added this because rs232 was missing an
                                                    //actual configuration event

    // TODO find some way of knowing Core Vs BoxProduct - this is BoxProduct
}


// Component Scope Function Definitions ###########################################################
//#################################################################################################
// User input - turns on or off the RS232 port
//
// Parameters:
//      enable  - enable or disable the RS232 port
// Return:
// Assumptions:
// * Intended for core variant, to be called from I2C Slave, after receiving customer commands
// * The bring-up sequence (assuming both units enabled):
// *    Local uint sends connected message to remote
// *    Remote takes module out of reset, configures the RS232 chip, sets the clock scale down then
// *        enables it, configures the MCA
// *    Sends device connected to local device
// *    Local device takes module out of reset, configures the RS232 chip, sets the clock scale
// *        then enables it, configures the MCA
// * The take-down sequence:
// *    Local unit stops transmitting
// *    Local unit sends disconnected message to remote
// *    Remote stops transmitting, disables MCA, disables the RS232 chip, places module in reset
// *    Remote sends disconnected message to local unit
// *    Local unit stops transmitting (code reuse), disables MCA, disables RS232 chip, places
// *        module in reset
//#################################################################################################
void RS232_ControlInput(bool enable)
{
    rs232Context.rs232Enabled = enable;
    if (enable)
    {
        // Message the other end the command to bring up the device
        RS232_SendCPUCommMessage(RS232_CPU_COMM_DEVICE_CONNECTED); // Notify remote to bringup MCA and enable the port - it will message local and do the same when message is received
    }
    else
    {
        // Stop transmitting to the remote but do not turn off local MCA
        bb_core_rs232_configure(false);
        RS232_SendCPUCommMessage(RS232_CPU_COMM_DEVICE_REMOVED); // Notify remote to bringup MCA and enable the port - it will message local and do the same when message is received
    }
}

//#################################################################################################
// ICMD Enable/Disable the RS232 port
// Parameters:
//      enable      - enable or disable the RS232 port, same as ControlInput
// Return:
// Assumptions: After modify feature_mask, it'll generate configuration event
//              and RS232ConfigurationEventHandler will be called
//#################################################################################################
void RS232_enable(uint8_t enable)
{
    ConfigBlocksEnable *blocksEnabled =  &(Config_GetBuffer()->featureControl);

    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled))
    {
        if(enable)
        {
            blocksEnabled->MiscControl |= (1 << CONFIG_BLOCK_ENABLE_RS232);
        }
        else
        {
            blocksEnabled->MiscControl &= ~(1 << CONFIG_BLOCK_ENABLE_RS232);
        }
        Config_ArbitrateSetVar(CONFIG_SRC_UART, CONFIG_VARS_BB_FEATURE_MASK, blocksEnabled);
    }
}


// Static Function Definitions ####################################################################
//#################################################################################################
// Wrapper for sequence of bringup and takedown of the RS232 HW
//
// Parameters:
//      enable      - to determine whether bringup or takedown RS232 HW
// Return:
// Assumptions:
//
//#################################################################################################
static void RS232_enableConfigureHW(bool enable)
{
    if (enable)
    {
        // Prep the chip
        bb_top_rs232ChipConfigure(true);

        // Configure ScaleDown and Set Enable
        bb_core_rs232_configure(true);

        // now start bringing up the MCA channel
        MCA_ChannelLinkUp(MCA_CHANNEL_NUMBER_RS232);
    }
    else
    {
        // Stop transmitting into MCA
        bb_core_rs232_configure(false);

        // disable MCA channel for RS232
        MCA_ChannelLinkDn(MCA_CHANNEL_NUMBER_RS232);

        // Disable the chip
        bb_top_rs232ChipConfigure(false);

        // put module in reset
        bb_top_rs232Reset(true);
    }
}


//#################################################################################################
// Handler for communication link up/down events, wraps the control function
//
// Parameters:
// Return:
// Assumptions:
// * The bring-up sequence (assuming both units enabled):
// *    Local uint sends connected message to remote
// *    Remote takes module out of reset, configures the RS232 chip, sets the clock scale down then
// *        enables it, configures the MCA
// *    Sends device connected to local device
// *    Local device takes module out of reset, configures the RS232 chip, sets the clock scale
// *        then enables it, configures the MCA
// * The take-down sequence:
// *    Local unit stops transmitting (code reuse), disables MCA, disables RS232 chip, places
// *        module in reset
// * It is assumed the remote will see the link down event and thus perform the same actions
//#################################################################################################
static void RS232_LinkEventHandler(uint32_t linkUp, uint32_t userContext)
{
    // only bring up if we were enabled to begin with
    if (linkUp && rs232Context.rs232Enabled)
    {
        // Message the other end the command to bring up the device
        RS232_SendCPUCommMessage(RS232_CPU_COMM_DEVICE_CONNECTED); // Notify remote to bringup MCA and enable the port - it will message local and do the same when message is received
    }
    else
    {
        // If the link is down - take down channel and reset
        RS232_enableConfigureHW(false);
        rs232Context.rs232LinkUp = false;
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
static void RS232_SendCPUCommMessage(uint8_t message)
{
    CPU_COMM_sendSubType(CPU_COMM_TYPE_RS232, message);
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
static void RS232_ReceiveCpuMsgHandler(uint8_t subType, const uint8_t *msg, uint16_t msgLength)
{
    iassert_RS232_COMPONENT_1(msgLength == 0, RS232_UNEXPECTED_CPU_MSG_LENGTH, msgLength);
    ilog_RS232_COMPONENT_1(ILOG_DEBUG, RS232_RCV_CPU_MSG, subType);
    enum Rs232CpuCommMessage remoteMessage = (enum Rs232CpuCommMessage)subType;

    switch(remoteMessage)
    {
        case RS232_CPU_COMM_DEVICE_CONNECTED:    // Remote bringup commanded
            if (!rs232Context.rs232LinkUp && rs232Context.rs232Enabled)
            {
                RS232_enableConfigureHW(true);
                rs232Context.rs232LinkUp = true;
                // Message the other end the command to bring up the device
                RS232_SendCPUCommMessage(RS232_CPU_COMM_DEVICE_CONNECTED);
            }
            else if (!rs232Context.rs232Enabled)
            {
                // Not enabled - ensure remote goes down until local is enabled
                RS232_SendCPUCommMessage(RS232_CPU_COMM_DEVICE_REMOVED);
            }
            // Do nothing if receiving connected message after local already enabled and configured
            break;

        case RS232_CPU_COMM_DEVICE_REMOVED:      // Remote takedown commanded
            if (rs232Context.rs232LinkUp)
            {
                RS232_enableConfigureHW(false);
                rs232Context.rs232LinkUp = false;
                RS232_SendCPUCommMessage(RS232_CPU_COMM_DEVICE_REMOVED); // Notify remote takedown complete so it can perform its own takedown
            }
            break;

        default:
            break;
    }
}


//#################################################################################################
// Handler for RS232 channel status changes
//
// Parameters:
// Return:
//
// Assumptions:
//#################################################################################################
static void RS232ChannelStatus(enum MCA_ChannelStatus channelStatus)
{
    ilog_RS232_COMPONENT_1(ILOG_DEBUG, RS232_MCA_CHANNEL_STATUS, channelStatus);

    switch (channelStatus)
    {
        case MCA_CHANNEL_STATUS_LINK_ACTIVE:        // channel is linked between Lex and Rex
            MCA_ChannelTxRxSetup(MCA_CHANNEL_NUMBER_RS232);    // now setup Tx and Rx
            // take module out of reset
            bb_top_rs232Reset(false);
            break;

        case MCA_CHANNEL_STATUS_CHANNEL_READY:      // channel is linked, and Rx, Tx is setup.  Ready for operation
           break;

        case MCA_CHANNEL_STATUS_CHANNEL_DISABLED:   // channel is disabled
        case MCA_CHANNEL_STATUS_LINK_DOWN:          // channel is down between Lex and Rex, needs to be re-initialized
        default:
            break;
    }
}


//#################################################################################################
// Handler for configuration change events.
//
// Parameters: eventInfo
// Return:
// Assumptions:
//
//#################################################################################################
static void RS232ConfigurationEventHandler(uint32_t eventInfo, uint32_t userContext)
{
    if(eventInfo == CONFIG_VARS_BB_FEATURE_CONTROL)
    {
        rs232Context.rs232Enabled = RS232EnabledStatus();
        RS232_LinkEventHandler( EVENT_GetEventInfo(ET_COMLINK_STATUS_CHANGE), 0);
    }
}


//#################################################################################################
// Get the enabled status for the LAN port Returns true if enabled, false if not
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static bool RS232EnabledStatus(void)
{
    bool rs232Enable = false;

    ConfigBlocksEnable *blocksEnabled =  &(Config_GetBuffer()->featureControl);
    if (Config_ArbitrateGetVar(CONFIG_VARS_BB_FEATURE_CONTROL, blocksEnabled))
    {
        rs232Enable = blocksEnabled->MiscControl & (1 << CONFIG_BLOCK_ENABLE_RS232);
    }

    return (rs232Enable);
}
