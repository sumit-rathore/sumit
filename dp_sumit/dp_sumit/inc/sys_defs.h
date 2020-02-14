///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2017
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
//!   @file  - sysdefs.h
//
//!   @brief - Contains definitions used by the event and packetize modules
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef SYS_DEFS_H
#define SYS_DEFS_H

/***************************** Included Headers ******************************/

/************************ Defined Constants and Macros ***********************/

// Packetize channel definitions

// the channel Id's for GE
enum ChannelIdGE
{
    CLIENT_ID_GE_ILOG,              // Ilog, Icmd for GE
    CLIENT_ID_GE_ICMD,              // CH 1, FW updates for GE
    CLIENT_ID_GE_STORAGE_VARS,      // storage var data for GE (BB = server, GE = client)
    CLIENT_ID_GE_CONFIG,            // CH 1, Configuration for GE
    CLIENT_ID_GE_STATUS,            // CH 1, Status from GE
    CLIENT_ID_GE_PRINTF,            // GE's debug Printf output
    CLIENT_ID_GE_USB_CTRL,          // Used to send USB messages to GE, such as Enable ULM on GE REX
    CLIENT_ID_GE_REX_DEV_STATUS,    // GE REX Device status - connected/disconnected

    CLIENT_ID_GE_COMMANDS = 0xC0,   // GE command channel
    CLIENT_ID_GE_INFO = 0xC1,       // GE command status channel
    CLIENT_ID_GE_PROGRAM_DATA = 0xC2,   // GE program data channel
};

#define MAX_NUM_GE_CLIENT_IDS 11

// the channel Id's for Blackbird
enum ChannelIdBB
{
    CLIENT_ID_BB_ISTATUS = 0x40,        // Customer viewable ilogs

    CLIENT_ID_BB_ILOG = 0x80,           // iLogs from Blackbird
    CLIENT_ID_BB_ICMD,                  // iCmds to Blackbird
    CLIENT_ID_BB_PRINTF,                // Printf from Blackbird

    CLIENT_ID_BB_GE_ILOG = 0xB0,        // iLogs from GE
    CLIENT_ID_BB_GE_ICMD,               // iCmds to GE
    CLIENT_ID_BB_GE_PRINTF,             // Printf from GE

    CLIENT_ID_BB_COMMANDS = 0xC0,       // BB command channel
    CLIENT_ID_BB_INFO = 0xC1,           // BB command status channel
    CLIENT_ID_BB_PROGRAM_DATA = 0xC2,   // BB program data channel

};

#define MAX_NUM_BB_CLIENT_IDS 10

#define MAX_NUM_CLIENT_IDS (MAX_NUM_BB_CLIENT_IDS + MAX_NUM_GE_CLIENT_IDS)

enum Usb2Status
{
    // ULM LinkState for GE
    USB2_IN_RESET,       // Off
    USB2_DISCONNECTED,   // Off
    USB2_OPERATING,      // On
    USB2_SUSPENDING,     // On
    USB2_SUSPENDED,      // Pulse
    USB2_RESUMING,       // On
    USB2_BUS_RESETTING,  // On
    // New status for signaling error
    USB2_ERROR,          // Pulse
    // New status for GE in reset
};

enum EventUsb3Status
{
    USB3_IN_RESET,      // Off
    USB3_U0,            // On
    USB3_U1,            // On
    USB3_U2,            // On
    USB3_U3,            // Pulse
    USB3_ERROR,         // Pulse
};

enum LinkStatus
{
    LINK_IN_RESET,      // Off
    LINK_OPERATING,     // On
    LINK_ERROR,         // 1Hz pulse with status
};

enum VideoStatus
{
    VIDEO_IN_RESET,     // Off
    VIDEO_OPERATING,    // On
    VIDEO_ERROR,        // Pulse
    VIDEO_TRAINING_UP,  // Blink
};

#define RS232_MODULE_MAX_SUPPORTED_BAUD_RATE    (115200)

/******************************** Data Types *********************************/

/*********************************** API *************************************/

#endif // SYS_DEFS_H

