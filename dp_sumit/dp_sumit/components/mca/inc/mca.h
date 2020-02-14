//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef MCA_H
#define MCA_H

// Includes #######################################################################################
#include <ibase.h>

// Constants and Macros ###########################################################################


// Data Types #####################################################################################

// NOTE: enum values correspond to actual channel numbers
enum MCA_ChannelNumber
{
    MCA_CHANNEL_NUMBER_CPU_COMM,    // comm channel between Lex and Rex
    MCA_CHANNEL_NUMBER_DP,          // DP MCA channel
    MCA_CHANNEL_NUMBER_USB3,        // USB3 MCA channel
    MCA_CHANNEL_NUMBER_GE,          // USB2 MCA channel
    MCA_CHANNEL_NUMBER_GMII,        // LanPort MCA channel
    MCA_CHANNEL_NUMBER_RS232,       // RS232 channel

    NUM_OF_MCA_CHANNEL_NUMBER       // must be the last on the list - number of MCA channels in use
};

// This enum maps directly to a HW bit (mca_tx/rx client packet/stream)
// Do not modify the enum ordering
enum MCA_ClientCommType
{
    MCA_CLIENT_STREAM,
    MCA_CLIENT_PACKET,

    NUM_OF_MCA_CLIENT_COMM_TYPE
};

enum MCA_ChannelStatus
{
    MCA_CHANNEL_STATUS_CHANNEL_DISABLED,    // channel is disabled
    MCA_CHANNEL_STATUS_LINK_ACTIVE,         // channel is linked between Lex and Rex
    MCA_CHANNEL_STATUS_LINK_DOWN,           // channel is down between Lex and Rex, needs to be re-initialized
    MCA_CHANNEL_STATUS_CHANNEL_READY,       // channel is linked, and Rx, Tx is setup.  Ready for operation

//    MCA_CHANNEL_STATUS_NUM       // must be the last on the list - number of MCA channel status defined
};

enum MCA_ChannelError
{
    MCA_CHANNEL_ERROR_RX_FIFO_OVERFLOW,     // channel rx fifo overflow
    MCA_CHANNEL_ERROR_RX_GRD_MAX_ERROR,     // channel rx guard max error
    MCA_CHANNEL_ERROR_TX_FIFO_FULL_ERR,   // Channel Tx fifo full error
};

// callback executed when the link is established between Lex and Rex sides of a channel
typedef void (*McaChannelStatusChangeCallback)(enum MCA_ChannelStatus);

// callback executed when it detects the channel's error
typedef void (*McaChannelErrorCallback)(enum MCA_ChannelError);


// Function Declarations ##########################################################################
// TODO should pass in channel count (num elements in the AddressArrays)
void MCA_Init( void)                                                __attribute__((section(".atext")));

// void MCA_Enable(void)                                               __attribute__((section(".atext")));
void MCA_EnableRx(void)                                             __attribute__((section(".atext")));
void MCA_EnableTx(void)                                             __attribute__((section(".atext")));
void MCA_DisableTx(void)                                            __attribute__((section(".atext")));
void MCA_DisableRx(void)                                            __attribute__((section(".atext")));

void MCA_ChannelInit(
    enum MCA_ChannelNumber channelNumber,
    McaChannelStatusChangeCallback statusCallback,
    McaChannelErrorCallback errorCallback)                          __attribute__((section(".atext")));

void MCA_ChannelTxRxSetup(enum MCA_ChannelNumber channelNumber)     __attribute__((section(".atext")));


// called from within an interrupt
enum MCA_ChannelStatus MCA_GetChannelStatus(enum MCA_ChannelNumber channelNumber);

void MCA_CoreIrq(void);
void MCA_Channel0Irq(void);
void MCA_Channel1Irq(void);
void MCA_Channel2Irq(void);
void MCA_Channel3Irq(void);
void MCA_Channel4Irq(void);
void MCA_Channel5Irq(void);
void MCA_ChannelDropTxPacket(enum MCA_ChannelNumber channelNumber, bool drop)   __attribute__((section(".atext")));
bool MCA_ChannelLinkUp(enum MCA_ChannelNumber channelNumber)                    __attribute__((section(".atext")));
bool MCA_ChannelLinkDn(enum MCA_ChannelNumber channelNumber)                    __attribute__((section(".atext")));
void MCA_ChannelLinkControl(enum MCA_ChannelNumber channelNumber, bool enable)  __attribute__((section(".ftext")));
#endif // MCA_H
