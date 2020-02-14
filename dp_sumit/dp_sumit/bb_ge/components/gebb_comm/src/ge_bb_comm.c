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

//#################################################################################################
// Module Description
//#################################################################################################
//  * Packets received are stored in recvPkt
//  *   Process packet for SOH, EOT and payload length matching header - set validity
//  *   Process packet for client and notify client of packet and validity of packet
//  * Packet to send - form packet, copy payload upto MAX payload size
//  *   If bytes to transfer larger than payload, set static info and wait for callback
//  *   to continue processing the packet -- TODO: How to handle buffer? Where store it? Pointer?
//  *   Need to establish signal to client caller to notify buffer has been copied and client can
//  *   reuse their buffer
//
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// *
//#################################################################################################


// Includes #######################################################################################
#include <ibase.h>
#include <itypes.h>

#include <timing_timers.h>
#include "ge_bb_comm_log.h"

#include <ge_bb_comm.h>
#include <leon_uart.h>
#include <rexulm.h>
#include <grg.h>

#include <crc.h>
#include <linkmgr.h>
#include <crc.h>
#include <lex.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################
enum RequestToGe
{
    DISABLE_ULM         = 0,
    ENABLE_ULM          = 1,
    REQUEST_CRC         = 2
};
// Global Variables ###############################################################################

// Static Variables ###############################################################################

// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initializes the BB<->GE comm link
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void GEBB_CommInit(void)
{
    // register iCmd client
    UART_packetizeRegisterClient(CLIENT_ID_GE_ICMD,
                                 &GEBB_COMM_icmdRxHandler);
    // register GE control client
    UART_packetizeRegisterClient(CLIENT_ID_GE_USB_CTRL,
                                 &GEBB_COMM_GeControlHandler);
}

//#################################################################################################
// Applies to REX only, enables the ULM USB HW to be initialized
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void GEBB_COMM_GeControlHandler(
    enum PacketRxStatus rxStatus,
    const void* data,
    const uint16_t size,
    uint8_t responseId)
{
    uint32_t *packet = (uint32_t*)data;

    switch(packet[0])
    {
        case DISABLE_ULM:
            if (!GRG_IsDeviceLex())
            {
                REXULM_disableUsb();
            }
            break;

        case ENABLE_ULM:
            if (!GRG_IsDeviceLex())
            {
                REXULM_enableUsb();
            }
            break;

        case REQUEST_CRC:
        {
            uint32_t crcCheck = crcFast((uint8_t*)SERIAL_FLASH_BASE_ADDR, packet[1]);

            uint32 statusPayload[2];
            statusPayload[0] = GE_BB_STATUS_TYPE_GE_CRC << GE_BB_STATUS_TYPE_OFFSET;
            statusPayload[1] = crcCheck;

            UART_packetizeSendDataImmediate(
                CLIENT_ID_GE_STATUS,
                NULL,
                &statusPayload[0],
                sizeof(statusPayload));
            break;
        }
        default:
            break;
    }
}

// Component Scope Function Definitions ###########################################################
void sendStatsToBB(void)
{
    uint32 statusPayload = GE_BB_STATUS_TYPE_UPDATE_LED << GE_BB_STATUS_TYPE_OFFSET;
    uint16 statusPayloadSize = sizeof(statusPayload);

    // getLinkStatu - uint32, 4-bits per vport
    // eg: 3:0 - vport 0, 7:4 - vport 1, etc...
    // convert to single bit, if vport is 0101 or state of 5, set bit
    uint8 linkStatePacked = 0;
    uint32 linkState = LINKMGR_getLinkState();
    int bit = 0;
    for (bit = 0; bit < 8; bit++)
    {
        if ((linkState >> (4*bit)) & 0xF)
        {
            linkStatePacked |= 1 << bit;
        }
    }
    statusPayload |= linkStatePacked;
    if (!GRG_IsDeviceLex())
    {
        statusPayload |= (REXULM_getDownstreamPortState() << GE_STATUS_REX_DSP_STATE_OFFSET);
        statusPayload |= (REXULM_getUpstreamPortState() << GE_STATUS_REX_USP_STATE_OFFSET);
        statusPayload |= ((REXULM_getUsbDevSpeed() & GE_STATUS_REX_USB_SPEED_MASK) <<
                          GE_STATUS_REX_USB_DEV_SPEED_OFFSET);
        statusPayload |= ((REXULM_getUsbOpSpeed() & GE_STATUS_REX_USB_SPEED_MASK) <<
                          GE_STATUS_REX_USB_OP_SPEED_OFFSET);
    }
    else
    {
        statusPayload |= (LEXULM_getLexUlmLinkState() << GE_STATUS_LEX_VPORT_XUSB_STATE_OFFSET);
    }

    UART_packetizeSendDataImmediate(
        CLIENT_ID_GE_STATUS,
        NULL,
        &statusPayload,
        statusPayloadSize);
}

void sendReadyMsgToBB(void)
{
    uint32 statusPayload = GE_BB_STATUS_TYPE_READY << GE_BB_STATUS_TYPE_OFFSET;
    uint16 statusPayloadSize = sizeof(statusPayload);
    // send status message of all 1's
    statusPayload = 0xffffffff;
    UART_packetizeSendDataImmediate(
        CLIENT_ID_GE_STATUS,
        NULL,
        &statusPayload,
        statusPayloadSize);
}

void sendVersionToBB(void)
{
    uint32 statusPayload = GE_BB_STATUS_TYPE_GE_VERSION << GE_BB_STATUS_TYPE_OFFSET
        | SOFTWARE_MAJOR_REVISION << GE_STATUS_VERSION_MAJOR_OFFSET
        | SOFTWARE_MINOR_REVISION << GE_STATUS_VERSION_MINOR_OFFSET
        | SOFTWARE_DEBUG_REVISION << GE_STATUS_VERSION_DEBUG_OFFSET;

    uint16 statusPayloadSize = sizeof(statusPayload);
    UART_packetizeSendDataImmediate(
        CLIENT_ID_GE_STATUS,
        NULL,
        &statusPayload,
        statusPayloadSize);
}

void sendGeAssertedMsgToBB(struct assertInfo_s * assertInfo)
{
    uint32 statusPayload[5];
    statusPayload[0] = GE_BB_STATUS_TYPE_GE_ASSERTED << GE_BB_STATUS_TYPE_OFFSET;
    statusPayload[1] = assertInfo->header;
    statusPayload[2] = assertInfo->arg1;
    statusPayload[3] = assertInfo->arg2;
    statusPayload[4] = assertInfo->arg3;
    uint16 statusPayloadSize = sizeof(statusPayload);

    UART_packetizeSendDataImmediate(
        CLIENT_ID_GE_STATUS,
        NULL,
        &statusPayload[0],
        statusPayloadSize);
    LEON_UartWaitForTx();
}

void sendDevConnectionStatusMsgToBB(uint8_t port, bool connected)
{
    uint32 statusPayload = GE_BB_STATUS_TYPE_DEVICE_CONNECTION << GE_BB_STATUS_TYPE_OFFSET;
    uint16 statusPayloadSize = sizeof(statusPayload);
    statusPayload |= connected;
    statusPayload |= port << 8;
    UART_packetizeSendDataImmediate(
        CLIENT_ID_GE_STATUS,
        NULL,
        &statusPayload,
        statusPayloadSize);
}

void sendUlmNegotiatedSpeedMsgToBB(uint8_t ulmNegSpeed)
{
    uint32 statusPayload = GE_BB_STATUS_TYPE_ULM_NEG_SPEED << GE_BB_STATUS_TYPE_OFFSET;
    uint16 statusPayloadSize = sizeof(statusPayload);
    statusPayload |= ulmNegSpeed;

    UART_packetizeSendDataImmediate(
        CLIENT_ID_GE_STATUS,
        NULL,
        &statusPayload,
        statusPayloadSize);
}

// This is for showing ISTATUS onto XComm
void sendGEIstatusToBB(enum GeToBbStatusMessageType msgType, uint32_t istatusValue)
{
    uint32 statusPayload[2];
    statusPayload[0] = (msgType << GE_BB_STATUS_TYPE_OFFSET);
    statusPayload[1] = istatusValue;

    uint16 statusPayloadSize = sizeof(statusPayload);

    UART_packetizeSendDataImmediate(
        CLIENT_ID_GE_STATUS,
        NULL,
        &statusPayload,
        statusPayloadSize);
}

// Static Function Definitions ####################################################################

//#################################################################################################
// Send pkt to UART one byte at a time
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################


