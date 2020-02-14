//#################################################################################################
// Icron Technology Corporation - Copyright 2015 - 2016
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef GE_BB_COMM_H
#define GE_BB_COMM_H

// Includes #######################################################################################
#include <itypes.h>
#include <leon_uart.h>
#include <iassert.h>
// Constants and Macros ###########################################################################

 // do it as a macro to save stack space
#define GEBB_iLogSend(msg, len) \
    UART_packetizeSendDataImmediate(CLIENT_ID_GE_ILOG, NULL, msg, len)

#define GE_STATUS_REX_DSP_STATE_OFFSET          (8)
#define GE_STATUS_REX_USP_STATE_OFFSET          (12)
#define GE_STATUS_REX_USB_DEV_SPEED_OFFSET      (16)
#define GE_STATUS_REX_USB_OP_SPEED_OFFSET       (20)
#define GE_STATUS_REX_USB_SPEED_MASK            (0xF)
#define GE_STATUS_LEX_VPORT_XUSB_STATE_OFFSET   (8)
#define GE_STATUS_VERSION_MAJOR_OFFSET          (16)
#define GE_STATUS_VERSION_MINOR_OFFSET          (8)
#define GE_STATUS_VERSION_DEBUG_OFFSET          (0)
#define GE_BB_STATUS_TYPE_OFFSET                (24)

// This enum definition should be matched with bb_ge_comm.c
enum GeToBbStatusMessageType
{
    GE_BB_STATUS_TYPE_UPDATE_LED,
    GE_BB_STATUS_TYPE_DEVICE_CONNECTION,
    GE_BB_STATUS_TYPE_ULM_NEG_SPEED,
    GE_BB_STATUS_TYPE_GE_ASSERTED,
    GE_BB_STATUS_TYPE_GE_VERSION,
    GE_BB_STATUS_TYPE_GE_CRC,
    GE_BB_STATUS_TYPE_GE_BITSTUFF_ERROR,        // Message for Bitstuff Error ISTATUS
    GE_BB_STATUS_TYPE_GE_CRC_ERROR,             // Message for CRC Error ISTATUS
    GE_BB_STATUS_TYPE_READY = 0xFF
};

// do it as a macro to save stack space
//#define GEBB_iLogSend(msg, len) ( (uint32_t)(msg) * (len) )

// Data Types #####################################################################################

// Function Declarations ##########################################################################

void GEBB_CommInit(void);

// iCmd client functions
void GEBB_COMM_icmdRxHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t size, uint8_t responseId);
// For rex to enable ULM and initialize HW
void GEBB_COMM_GeControlHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t size, uint8_t responseId) __attribute__((section(".ftext")));

void sendStatsToBB(void);
void sendReadyMsgToBB(void);
void sendVersionToBB(void);
void sendGeAssertedMsgToBB(struct assertInfo_s * assertInfo);
void sendDevConnectionStatusMsgToBB(uint8_t port, bool connected);
void sendUlmNegotiatedSpeedMsgToBB(uint8_t ulmNegSpeed);
void sendGEIstatusToBB(uint8_t istatusIndex, uint32_t istatusValue);
#endif // GE_BB_COMM_H

