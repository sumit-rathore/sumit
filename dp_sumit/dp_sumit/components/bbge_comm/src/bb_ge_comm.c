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
#include <sys_defs.h>

#include <timing_timers.h>

#include <bb_ge_comm.h>
#include <bb_core.h>
#include <uart.h>
#include <led.h>
#include <bb_top.h>
#include <bb_top_ge.h>
#include <leon_timers.h>
#include <ge_program.h>
#include <event.h>

#include "bb_ge_comm_loc.h"     // component level definitions
#include "bb_ge_comm_log.h"     // iLog definitions

#include <configuration.h>
#include <command.h>
#include <linkmgr.h>

// Constants and Macros ###########################################################################
#define GE_STATUS_LINK_STATE_OFFSET             (0)
#define GE_STATUS_LINK_STATE_MASK               (0xFF)
#define GE_STATUS_REX_DSP_STATE_OFFSET          (8)
#define GE_STATUS_REX_USP_STATE_OFFSET          (12)
#define GE_STATUS_REX_PORT_STATE_MASK           (0xF)
#define GE_STATUS_REX_USB_DEV_SPEED_OFFSET      (16)
#define GE_STATUS_REX_USB_OP_SPEED_OFFSET       (20)
#define GE_STATUS_REX_USB_SPEED_MASK            (0xF)
#define GE_STATUS_LEX_VPORT_XUSB_STATE_OFFSET   (8)
#define GE_STATUS_LEX_VPORT_XUSB_STATE_MASK     (0xF)
#define GE_STATUS_LEX_DEVICE_CONNECTION_MASK    (0x1)
#define GE_STATUS_LEX_ULM_NEG_SPEED_MASK        (0x3)
#define GE_STATUS_VERSION_MAJOR_OFFSET          (16)
#define GE_STATUS_VERSION_MINOR_OFFSET          (8)
#define GE_STATUS_VERSION_DEBUG_OFFSET          (0)
#define GE_STATUS_VERSION_MAJOR_MASK            (0xFF << GE_STATUS_VERSION_MAJOR_OFFSET)
#define GE_STATUS_VERSION_MINOR_MASK            (0xFF << GE_STATUS_VERSION_MINOR_OFFSET)
#define GE_STATUS_VERSION_DEBUG_MASK            (0xFF << GE_STATUS_VERSION_DEBUG_OFFSET)

// Data Types #####################################################################################
enum GeUlmLinkState
{
    DISCONNECTED = 1,
    OPERATING,
    SUSPENDING,
    SUSPENDED,
    RESUMING,
    BUS_RESETTING
};

// This enum definition should be matched with ge_bb_comm.h
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

enum UsbSpeed
{
    USB_SPEED_HIGH       = 0,    // These values are set to the match the HW registers
    USB_SPEED_FULL       = 1,
    USB_SPEED_LOW        = 2,
    USB_SPEED_INVALID    = 3
};

enum RequestToGe
{
    DISABLE_ULM         = 0,
    ENABLE_ULM          = 1,
    REQUEST_CRC         = 2
};
// Global Variables ###############################################################################

// Static Variables ###############################################################################
static enum GeUlmLinkState ulmLinkState;
static struct GESWVersion
{
    uint32_t crc;
    uint8_t geSWMajorRevision;
    uint8_t geSWMinorRevision;
    uint8_t geSWDebugRevision;
    uint8_t fill;
} geSWVersion;

#ifdef PLATFORM_A7
static struct BB_GE_COMM_Context
{
    uint32_t geCurrentDeviceStatus;
    uint32_t geCurrentUlmNegSpeed;

    bool geVersionVerified;                     // To prevent check geVersion whenever GE reset
} bbGeContext;
#endif

static void (*msgHandlers[BBGE_COMM_MSG_HNDLR_NUM]) (void);

// TODO See if we can somehow read from the LD file the actual location

// Static Function Declarations ###################################################################
static void BBGE_COMM_versionCrcCheckHandler(void)              __attribute__ ((section(".atext")));
static void BBGE_COMM_SendGEIstatus(enum GeUlmLinkState state)  __attribute__ ((section(".atext")));
#ifndef BB_PROGRAM_BB
static uint32_t BBGE_COMM_CheckUsb2EventStatus(void)            __attribute__ ((section(".atext")));
#endif
// Exported Function Definitions ##################################################################

//#################################################################################################
// Register handler and client ID and port
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void BBGE_COMM_init(void)
{
    // Register clients for the BB <-> GE port
    UART_packetizeRegisterClient( UART_PORT_GE, CLIENT_ID_GE_ILOG,           BBGE_COMM_ilogRxHandler);
    UART_packetizeRegisterClient( UART_PORT_GE, CLIENT_ID_GE_ICMD,           BBGE_COMM_icmdRxHandler);
    UART_packetizeRegisterClient( UART_PORT_GE, CLIENT_ID_GE_PRINTF,         BBGE_COMM_gePrintfHandler);
    UART_packetizeRegisterClient( UART_PORT_GE, CLIENT_ID_GE_STATUS,         BBGE_COMM_geStatusHandler);
    UART_packetizeRegisterClient( UART_PORT_GE, CLIENT_ID_GE_STORAGE_VARS,   GEBB_COMM_StorageRxHandler);
    UART_packetizeRegisterClient( UART_PORT_GE, CLIENT_ID_GE_REX_DEV_STATUS, BBGE_COMM_geRexDevStatusHandler);

    // Register clients for the BB <-> external world port
    UART_packetizeRegisterClient( UART_PORT_BB, CLIENT_ID_BB_GE_ICMD, BBGE_COMM_geIcmdHandler);
#ifndef BB_PROGRAM_BB
    EVENT_Register(ET_USB2_STATUS_CHANGE, BBGE_COMM_CheckUsb2EventStatus);
#endif
    bbGeContext.geCurrentDeviceStatus = 0;
}

//#################################################################################################
// BBGE_RunGEVerify
//  Reset GE and check version & CRC
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void BBGE_RunGEVerify(void)
{
    bb_top_SetGEToRunMode(BBGE_COMM_versionCrcCheckHandler, NULL);
}

//#################################################################################################
// Allows users to supply list of handlers so we don't have to manage dependencies amongst
// submodules
//
// Parameters:
// Return:
// Assumptions:
// * Initialization already completed
//#################################################################################################
void BBGE_COMM_registerHandler(enum BbGeCommMsgHandler idx, void (*msgHandler)(void))
{
    msgHandlers[idx] = msgHandler;
}


// Component Scope Function Definitions ###########################################################
void BBGE_COMM_gePrintfHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t length, uint8_t responseId)
{
    UART_printf("GE DBG: ");
#ifndef BB_PACKETIZE_DISABLED
    //UART_printf((char*)data);
    UART_packetizeSendDataImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_PRINTF,
        NULL,
        data,
        length);
#else
    uint16_t bytesSent = 0;
    uint8_t* bytes = (uint8_t*)data;
    for (bytesSent = 0; bytesSent < length; bytesSent++)
    {
        UART_ByteTxByCh(UART_PORT_BB, bytes[bytesSent]);
    }
#endif // BB_PACKETIZE_DISABLED
}

void BBGE_COMM_geStatusHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t length, uint8_t responseId)
{
    uint32_t *geStatus = (uint32_t*)data;

//    UART_printf("GE Status: %x\n", geStatus[0]);

// Status Structure
// 7:0 link state
//  -- REX --
//  USB States
//  11:8 - Downstream Port State
//  15:12 - Upstream Port State
//  19:16 - USB Device Speed
//  23:20 - USB Operating Speed

// GE LED operation
// SYSTEM LINK LED
//      ACTIVE -- On
//      anything else -- Pulse
//
// -- LEX --
// SYSTEM HOST LED
//      SUSPENDED -- Pulse
//      RESUME_DONE -- On
//      BUS_RESETTING -- On
//      DISCONNECTED -- Off
//      RESET_DONE -- No Change
//
// -- REX --
// SYSTEM HOST LED
//      USP DISCONNECTED -- Off
//      DSP SUSPENDED -- Pulse
//      DSP anything else -- On
//
    enum GeToBbStatusMessageType geStatusType = geStatus[0] >> 24;
    switch(geStatusType)
    {
        case GE_BB_STATUS_TYPE_READY:
            // GE ready status message
            if (msgHandlers[BBGE_COMM_MSG_HNDLR_GE_READY] != NULL)
            {
                (*msgHandlers[BBGE_COMM_MSG_HNDLR_GE_READY]) ();
            }
            break;

        case GE_BB_STATUS_TYPE_UPDATE_LED:
            // LED message comming from GE every second.
            // Use this for GE alive signal and refresh Watchdog Timer
            bb_top_StartGEWatchdogRunningTimer();


            enum GeUlmLinkState currUlmLinkState = 1;
            // Update LEX LEDs
            if (bb_top_IsDeviceLex())
            {
                currUlmLinkState = (geStatus[0] >> GE_STATUS_LEX_VPORT_XUSB_STATE_OFFSET)
                                & GE_STATUS_LEX_VPORT_XUSB_STATE_MASK;
                currUlmLinkState++; // USB2_IN_RESET for Usb2Status is 0, so inc by 1
            }
            else // Update REX LEDs
            {
                currUlmLinkState = (geStatus[0] >> GE_STATUS_REX_DSP_STATE_OFFSET)
                                & GE_STATUS_REX_PORT_STATE_MASK;
                currUlmLinkState++; // USB2_IN_RESET for Usb2Status is 0, so inc by 1
                // Check if GE has a device connected
                if (currUlmLinkState != DISCONNECTED)
                {
                    // TODO: Set a state? Send a message to BB LEX for host connection completion?
                }
            }

            // Check Status change and send event & istatus
            if (currUlmLinkState != ulmLinkState)
            {
                ulmLinkState = currUlmLinkState;
                EVENT_Trigger(ET_USB2_STATUS_CHANGE, ulmLinkState);
                BBGE_COMM_SendGEIstatus(ulmLinkState);
            }

            break;

        case GE_BB_STATUS_TYPE_DEVICE_CONNECTION:
            if (geStatus[0] != bbGeContext.geCurrentDeviceStatus)
            {
                if (geStatus[0] & GE_STATUS_LEX_DEVICE_CONNECTION_MASK)
                {
                    ILOG_istatus(ISTATUS_ULP_LEX_USB2_DEV_CONN, 0);
                }
                else
                {
                    ILOG_istatus(ISTATUS_ULP_LEX_USB2_DEV_DISCONN, 0);
                }
            }
            bbGeContext.geCurrentDeviceStatus = geStatus[0];
            break;

        case GE_BB_STATUS_TYPE_ULM_NEG_SPEED:
            if (geStatus[0] != bbGeContext.geCurrentUlmNegSpeed)
            {
                uint8_t speed = (uint8_t)(geStatus[0] & GE_STATUS_LEX_ULM_NEG_SPEED_MASK);
                switch(speed)
                {
                    case USB_SPEED_HIGH:
                        ILOG_istatus(ISTATUS_ULP_LEX_USB2_ULM_NEG_SPEED_HIGH, 0);
                        break;
                    case USB_SPEED_FULL:
                        ILOG_istatus(ISTATUS_ULP_LEX_USB2_ULM_NEG_SPEED_FULL, 0);
                        break;
                    case USB_SPEED_LOW:
                        ILOG_istatus(ISTATUS_ULP_LEX_USB2_ULM_NEG_SPEED_LOW, 0);
                    case USB_SPEED_INVALID:
                    default:
                        break;
                }
            }
            bbGeContext.geCurrentUlmNegSpeed = geStatus[0];
            break;

        case GE_BB_STATUS_TYPE_GE_ASSERTED:
            ilog_BBGE_COMM_COMPONENT_0(ILOG_MAJOR_EVENT, BBGE_COMM_GE_ASSERTED);

            for(uint16_t args = 1; args < (length / sizeof(geStatus[0])); args++)
            {
                ilog_BBGE_COMM_COMPONENT_2(ILOG_MAJOR_EVENT, BBGE_COMM_GE_ASSERT_INFO, args-1, geStatus[args]);
            }
            break;

        case GE_BB_STATUS_TYPE_GE_VERSION:
        {
            // Check first GE version response only to verify version and CRC for automatic download
            if(!bbGeContext.geVersionVerified)
            {
                uint32_t geMsg[2];
                const uint32_t* flash_bin_table_ptr = (uint32_t*)FLASH_BIN_TABLE_ADDRESS;
                const uint32_t flashGESizeValue = flash_bin_table_ptr[FLASH_BIN_TABLE_GE_FW_SIZE];

                geSWVersion.geSWMajorRevision = (geStatus[0] & GE_STATUS_VERSION_MAJOR_MASK) >> GE_STATUS_VERSION_MAJOR_OFFSET;
                geSWVersion.geSWMinorRevision = (geStatus[0] & GE_STATUS_VERSION_MINOR_MASK) >> GE_STATUS_VERSION_MINOR_OFFSET;
                geSWVersion.geSWDebugRevision = (geStatus[0] & GE_STATUS_VERSION_DEBUG_MASK) >> GE_STATUS_VERSION_DEBUG_OFFSET;

                ilog_BBGE_COMM_COMPONENT_3(ILOG_MINOR_EVENT, BBGE_COMM_GE_VERSION,
                    geSWVersion.geSWMajorRevision, geSWVersion.geSWMinorRevision, geSWVersion.geSWDebugRevision);

                geMsg[0] = REQUEST_CRC;
                geMsg[1] = flashGESizeValue;

                // after got GE's running version, request GE's CRC
                UART_packetizeSendDataImmediate(
                    UART_PORT_GE,
                    CLIENT_ID_GE_USB_CTRL,
                    NULL,
                    geMsg,
                    sizeof(geMsg));
            }
            else
            {
                // Got GE's second boot message
                // Does not request CRC, and start GE running watchdog & stop GE reset watchdog
                bb_top_StopGEWatchdogResetTimer();
                bb_top_StartGEWatchdogRunningTimer();
            }
            break;
        }

        case GE_BB_STATUS_TYPE_GE_CRC:
            geSWVersion.crc = geStatus[1];
            ilog_BBGE_COMM_COMPONENT_1(ILOG_MAJOR_EVENT, BBGE_COMM_GE_CRC, geSWVersion.crc);

            // 1st Boot Got all information, doesn't need to wait GE Watchdog and call it directly
            // GE Watchdog will be useful in case of GE program is broken and can't respond
            bb_top_StopGEWatchdogResetTimer();
            BBGE_COMM_versionCrcCheckHandler();
            break;

        case GE_BB_STATUS_TYPE_GE_BITSTUFF_ERROR:
            ILOG_istatus(ISTATUS_ULP_USB2_BITSTUFF_ERROR, 0);
            break;

        case GE_BB_STATUS_TYPE_GE_CRC_ERROR:
            ILOG_istatus(ISTATUS_ULP_USB2_CRC_ERROR, 0);
            break;

        default:
            break;
    }
}


//#################################################################################################
// Sends message to GE to enable ULM
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void BBGE_COMM_sendGeMsgRexEnableULM(void)
{
#ifdef PLATFORM_A7
    uint32_t geMsg[1];
    geMsg[0] = ENABLE_ULM;
    UART_packetizeSendDataImmediate(
        UART_PORT_GE,
        CLIENT_ID_GE_USB_CTRL,
        NULL,
        geMsg,
        sizeof(geMsg));
#endif
}



//#################################################################################################
// Update BB REX on GE REX Device connect/disconnect status
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void BBGE_COMM_geRexDevStatusHandler(enum PacketRxStatus rxStatus, const void* data, const uint16_t length, uint8_t responseId)
{
    uint8_t* bytes = (uint8_t*)data;
    if (bytes == 0) // disconnect
    {
        if (msgHandlers[BBGE_COMM_MSG_HNDLR_GE_REX_DEV_DISCONN] != NULL)
        {
            (*msgHandlers[BBGE_COMM_MSG_HNDLR_GE_REX_DEV_DISCONN])();
        }
    }
    else // connect
    {
        if (msgHandlers[BBGE_COMM_MSG_HNDLR_GE_REX_DEV_CONN] != NULL)
        {
            (*msgHandlers[BBGE_COMM_MSG_HNDLR_GE_REX_DEV_CONN])();
        }
    }
}

// Static Function Definitions ####################################################################
//#################################################################################################
// GE Version check timer to decide GE version and run emergency upgrade for GE
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void BBGE_COMM_versionCrcCheckHandler(void)
{
    const uint32_t* flash_bin_table_ptr = (uint32_t*)FLASH_BIN_TABLE_ADDRESS;
    const uint32_t flashGEVersionValue = flash_bin_table_ptr[FLASH_BIN_TABLE_GE_FW_VERSION];
    const uint32_t flashGECrcValue = flash_bin_table_ptr[FLASH_BIN_TABLE_GE_FW_CRC];

    // Load bin table's GE firmware information
    const struct GESWVersion geFlashVersion = {
        .geSWMajorRevision = (flashGEVersionValue & FLASH_BIN_TABLE_GE_FW_VERSION_MAJOR_MASK) >> FLASH_BIN_TABLE_GE_FW_VERSION_MAJOR_OFFET,
        .geSWMinorRevision = (flashGEVersionValue & FLASH_BIN_TABLE_GE_FW_VERSION_MINOR_MASK) >> FLASH_BIN_TABLE_GE_FW_VERSION_MINOR_OFFET,
        .geSWDebugRevision = (flashGEVersionValue & FLASH_BIN_TABLE_GE_FW_VERSION_DEBUG_MASK) >> FLASH_BIN_TABLE_GE_FW_VERSION_DEBUG_OFFET,
        .crc = flashGECrcValue,
        .fill = 0
    };

    if(!memeq(&geFlashVersion, &geSWVersion, sizeof(struct GESWVersion)))
    {
        ilog_BBGE_COMM_COMPONENT_2(ILOG_MAJOR_ERROR, BBGE_COMM_GE_VERSION_MISMATCH,
            geFlashVersion.geSWMajorRevision << GE_STATUS_VERSION_MAJOR_OFFSET
                | geFlashVersion.geSWMinorRevision << GE_STATUS_VERSION_MINOR_OFFSET
                | geFlashVersion.geSWDebugRevision << GE_STATUS_VERSION_DEBUG_OFFSET,
            geFlashVersion.crc
        );

        bb_core_setProgramBbOperation(SPR_PROGRAM_BB_SET_FOR_GE_DOWNLOAD);

        CMD_loadAndRunProgramBB(false);     // run program_bb for GE automatic download
    }
    else
    {
        ilog_BBGE_COMM_COMPONENT_2(ILOG_MAJOR_EVENT, BBGE_COMM_GE_VERSION_MATCH,
            geFlashVersion.geSWMajorRevision << GE_STATUS_VERSION_MAJOR_OFFSET
                | geFlashVersion.geSWMinorRevision << GE_STATUS_VERSION_MINOR_OFFSET
                | geFlashVersion.geSWDebugRevision << GE_STATUS_VERSION_DEBUG_OFFSET,
            geFlashVersion.crc
        );

        bbGeContext.geVersionVerified = true;           // Indicate GE version's verification not to ask CRC and reset GE again

        // initialize GE to prepare normal start
        bb_top_SetGEToResetMode();

        LINKMGR_setGeVerified();
    }
}

//#################################################################################################
// BBGE_COMM_SendGEIstatus
//  Send GE's ISTATUS message when GE status has changed
// Parameters: GeUlmLinkState
// Return:
// Assumptions:
//#################################################################################################
static void BBGE_COMM_SendGEIstatus(enum GeUlmLinkState state)
{
    switch (state)
    {
        case BUS_RESETTING:
            ILOG_istatus(ISTATUS_ULP_LEX_USB2_BUS_RESET, 0);
            break;
        case SUSPENDED:
            ILOG_istatus(ISTATUS_ULP_LEX_USB2_SUSPENDED, 0);
            break;
        case OPERATING:
        case SUSPENDING:
        case RESUMING:
        case DISCONNECTED:
        default:
            break;
    }
}

#ifndef BB_PROGRAM_BB
//#################################################################################################
// GE Version check timer to decide GE version and run emergency upgrade for GE
//  ET_USB2_STATUS_CHANGE
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static uint32_t BBGE_COMM_CheckUsb2EventStatus(void)
{
    if(bb_top_isGEResetOn())                // Check GE reset, it can't get by communication
    {
        ulmLinkState = USB2_IN_RESET;
    }
    return ulmLinkState;
}
#endif

