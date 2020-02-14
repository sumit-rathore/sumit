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
//!   @file  -  vhub_loc.h
//
//!   @brief -  Local file header to the vhub component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef VHUB_LOC_H
#define VHUB_LOC_H

/***************************** Included Headers ******************************/
#include <options.h>
#include <vhub.h>
#include "vhub_log.h"
#include "vhub_cmd.h"
#include <timing_timers.h>        // For port reset timer
#include <ulm.h>
#include <usbdefs.h>
#include <xcsr_xsst.h>
#include <xcsr_xusb.h>
#include <xcsr_xicsq.h>
#include <devmgr.h>
#ifdef GOLDENEARS
#include <xlr.h>
#endif
#include <topology.h>
#include <lex.h>

/************************ Defined Constants and Macros ***********************/
#define VHUB_NUM_ENDPOINTS 1

// Used to access the hub status within vhub.portInfo.usbCh11Status rather than the port status which is what all
// of the other array elements are for
#define VHUB_HUB_STATUS_INDEX 0


// Convenience macros for setting and clearing hub and port status bits

#define _VHUB_SET_GENERIC(_index_, _member_, _bit_) \
    vhub.portInfo[(_index_)].usbCh11Status._member_ = vhub.portInfo[(_index_)].usbCh11Status._member_ | HOST_ENDIAN_TO_USB_16(1 << (_bit_))
#define _VHUB_CLR_GENERIC(_index_, _member_, _bit_) \
    vhub.portInfo[(_index_)].usbCh11Status._member_ = vhub.portInfo[(_index_)].usbCh11Status._member_ & HOST_ENDIAN_TO_USB_16(~(1 << (_bit_)))

#define VHUB_SET_PORT_STATUS_BIT(_port_, _bit_) _VHUB_SET_GENERIC((_port_), status, (_bit_))
#define VHUB_CLR_PORT_STATUS_BIT(_port_, _bit_) _VHUB_CLR_GENERIC((_port_), status, (_bit_))

#define VHUB_SET_PORT_CHANGE_BIT(_port_, _bit_) _VHUB_SET_GENERIC((_port_), change, (_bit_))
#define VHUB_CLR_PORT_CHANGE_BIT(_port_, _bit_) _VHUB_CLR_GENERIC((_port_), change, (_bit_))

#define VHUB_SET_HUB_STATUS_BIT(_bit_)          _VHUB_SET_GENERIC(VHUB_HUB_STATUS_INDEX, status, (_bit_))
#define VHUB_CLR_HUB_STATUS_BIT(_bit_)          _VHUB_CLR_GENERIC(VHUB_HUB_STATUS_INDEX, status, (_bit_))

#define VHUB_SET_HUB_CHANGE_BIT(_bit_)          _VHUB_SET_GENERIC(VHUB_HUB_STATUS_INDEX, change, (_bit_))
#define VHUB_CLR_HUB_CHANGE_BIT(_bit_)          _VHUB_CLR_GENERIC(VHUB_HUB_STATUS_INDEX, change, (_bit_))

// Convenience macros for setting and clearing interrupt 1 data bits
#define VHUB_SET_INTEP1_DATA_BIT(port) vhub.intEp1.hubAndPortStatusChangeMap[port / 8] |= (1 << (port % 8))
#define VHUB_CLR_INTEP1_DATA_BIT(port) vhub.intEp1.hubAndPortStatusChangeMap[port / 8] &= ~(1 << (port % 8))

// USB2.0 spec 11.24.2.6, table 11-19 Hub Status Bits
#define VHUB_HSTATUS_LOCAL_POWER    0   // 1 -> local power supply lost; bus powered
#define VHUB_HSTATUS_OVER_CURRENT   1   // 1 -> Total current draw exceeds maximum

// USB2.0 spec 11.24.2.6, table 11-20 Hub Status Change Bits
#define VHUB_HCHANGE_LOCAL_POWER    0   // 1 -> local power supply lost; bus powered
#define VHUB_HCHANGE_OVER_CURRENT   1   // 1 -> Total current draw exceeds maximum

// USB2.0 spec 11.24.2.7.1, table 11-21, Port Status Bits
#define VHUB_PSTATUS_CONNECTION     0   // 1 -> device is present
#define VHUB_PSTATUS_ENABLE         1   // 1 -> port enabled
#define VHUB_PSTATUS_SUSPEND        2   // 1 -> suspended or resuming
#define VHUB_PSTATUS_OVER_CURRENT   3   // 1 -> over-current condition exists
#define VHUB_PSTATUS_RESET          4   // 1 -> reset signalling asserted
#define VHUB_PSTATUS_POWER          8   // 1 -> port is not in the powered-off state
#define VHUB_PSTATUS_LOW_SPEED      9   // 1 -> low speed device
#define VHUB_PSTATUS_HIGH_SPEED     10  // 1 -> high speed device
#define VHUB_PSTATUS_TEST           11  // 1 -> port is in test mode
#define VHUB_PSTATUS_INDICATOR      12  // 1 -> port indicator displays sw controlled color

// USB2.0 spec 11.24.2.7.2, table 11-22, Port Status Change Bits
#define VHUB_PCHANGE_CONNECTION     0   // 1 -> port connection status changed
#define VHUB_PCHANGE_ENABLE         1   // 1 -> port enable changed
#define VHUB_PCHANGE_SUSPEND        2   // 1 -> port has transitioned out of suspended state
#define VHUB_PCHANGE_OVER_CURRENT   3   // 1 -> over-current indicator has changed
#define VHUB_PCHANGE_RESET          4   // 1 -> reset complete


// The number of ports on the hub
#define VHUB_NUM_PORTS (NUM_OF_VPORTS - 1)

// Macros for working with bit arrays
// This assumes bit 0 is for the hub, and port 1 is bit 1, ... port 7 is bit 7, port 8 is the 2nd byte in bit 0
#define VHUB_BIT_SIZE_IN_BYTES (VHUB_NUM_PORTS / 8 + 1)

/******************************** Data Types *********************************/

// Note: we do not use bit-fields here because the bit order is part of the USB spec and there is
// no guarantee how bit-fields will be packed by the compiler.
union usbCh11StatusStruct
{
    struct {
        uint16 status;
        uint16 change;
    };
    uint32 raw;
};

struct vhubState {
    struct {
        // This type stores information about a specific downstream port in the virtual hub

        // This is for the data to be sent on a portStatus or hubStatus control transaction
        // Array index 0 is the hub's status and the bits are encoded using VHUB_HSTATUS_* and
        // VHUB_HCHANGE_* #defines.  All other array elements correspond to the hub port with the same
        // number and the bits are based on VHUB_PSTATUS_* and VHUB_PCHANGE_* #defines.
        union usbCh11StatusStruct usbCh11Status;

        enum ULM_linkState state;
        enum UsbSpeed speed;
    } portInfo[NUM_OF_VPORTS]; // NOTE: port 0 is the hub itself

    // upstream usb port settings
    enum ULM_linkState linkState;
    enum UsbSpeed linkSpeed;
    boolT remoteWakeupEnabled;
    uint8 currentConfiguration;

    // Which port is currently undergoing a reset
    // 0 means no port is in reset
    // NOTE: Each port is represented by a bit
    // TODO: Increase the size of this var if NUM_OF_VPORTS
    // is greater than 7
    uint8 portInReset;
    // Timer that goes off when a reset is complete
    TIMING_TimerHandlerT portResetTimerCompleteHandle;

    // Which port is currently undergoing a resume
    // 0 means no port is in resume
    // NOTE: Each port is represented by a bit
    // TODO: Increase the size of this var if NUM_OF_VPORTS
    // is greater than 7
    uint8 portInResume;
    // Timer that goes off when a resume is complete
    TIMING_TimerHandlerT portResumeTimerCompleteHandle;

    // Scratch area to be used when replying to control transactions data phase
    union {
        uint8 bytes[2];
        uint16 hword;
    } controlRequestReplyScratchArea;

    struct {
        // local state used for endpoint 1 tracking

        // This represents the data to send to the host on an interrupt endpoint data packet
        //USB 2.0 spec 11.12.4, Figure 11-22, Hub and PortStatusChangeMap, for interrupt endpoint
        uint8 hubAndPortStatusChangeMap[VHUB_BIT_SIZE_IN_BYTES];

        // Is the endpoint halted?  It shouldn't ever be halted
        boolT halted;
    } intEp1;
};

/******************************** Vhub Local Vars ****************************/

//vhub_usb_messaging.c
// NOTE: This can't be inside struct vhubState
// Why, because this struct ends in a flexible array member
// It should be possible to embed it inside struct vhubState, as we know at build time how it is
// GCC gives an error though.  There doesn't seem to be a way around it
extern struct DTT_VF_EndPointHandles vhub_vf;

// vhub_state.c
extern struct vhubState vhub;

/******************************** Internal API *******************************/
//vhub_usb_messaging.c
void VHUB_Reset(void);

//vhub_port_manager.c
void  VHUB_InitPortManager(void);
void  VHUB_ConnectDev(uint8 vPort, enum UsbSpeed speed);
void  VHUB_DisConnectDev(uint8 port);
void  VHUB_PushStatusEndpoint(void);
void  VHUB_SuspendPort(uint8 port);
void  VHUB_FinishResumePort(uint8 port);
void  VHUB_StartResumePort(uint8 port);
boolT VHUB_IsPortEnabled(uint8 port);
void  VHUB_PortWakeup(uint8 port);
void  VHUB_RemoteWakeup(uint8 port);

#endif // VHUB_LOC_H

