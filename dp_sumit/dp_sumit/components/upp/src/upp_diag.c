///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2018
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
//!   @file  -  upp.c
//
//!   @brief -  Contains the code for the USB Protocol layer Partner (UPP) project:
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################

#include <ibase.h>
#include <callback.h>
#include <usbdefs.h>
#include "upp_log.h"
#include "upp_loc.h"

#include <uart.h>

// Constants and Macros ###########################################################################

#define ENDPOINT_ATTRIBUTE_TYPE_MASK 0x3
#define UPP_DIAG_MAX_BUFFERS_STORED 2000

// Data Types #####################################################################################

struct UppDiagBufferInfo
{
    uint16_t WriteIndex;
    bool bufferOverflow;
    struct UPPBufferDiag buffer[UPP_DIAG_MAX_BUFFERS_STORED];

    uint16_t ReadIndex;
};

// Static Function Declarations ###################################################################

static void UppDiagPrintBuffer(void *param1, void *param2);

// Static Variables ###############################################################################

static struct
{
    struct UppDiagBufferInfo buffersRead;

} uppDiagCtx;

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// re-init Diag info
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_DiagReinit(void)
{
    // reset all diagnostic structures
    // done printing out
    memset(&uppDiagCtx.buffersRead, 0, sizeof(uppDiagCtx.buffersRead));
}

//#################################################################################################
// To print configuration descriptor
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UppDiagStoreBufferInfo(struct UPPBufferDiag *bufferInfo)
{
    uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.WriteIndex++] = *bufferInfo;

// comment this out if we only want to store the first X number of entries
    if (uppDiagCtx.buffersRead.WriteIndex == uppDiagCtx.buffersRead.ReadIndex)
    {
        uppDiagCtx.buffersRead.ReadIndex = uppDiagCtx.buffersRead.WriteIndex++;
    }
    
    if (uppDiagCtx.buffersRead.WriteIndex >= UPP_DIAG_MAX_BUFFERS_STORED)
    {

        // uncomment if you only want to store the first X number of entries
//      uppDiagCtx.buffersRead.WriteIndex = UPP_DIAG_MAX_BUFFERS_STORED-1; // stay at max

        // comment this out if we only want to store the first X number of entries
        uppDiagCtx.buffersRead.WriteIndex = 0; // wrap at 0
        uppDiagCtx.buffersRead.ReadIndex  = 1;

        uppDiagCtx.buffersRead.bufferOverflow = 1;
    }
}

//#################################################################################################
// To print configuration descriptor
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UppDiagPrintBufferInfo(void)
{
    if (CALLBACK_RunSingle(UppDiagPrintBuffer, NULL, NULL))
    {
        uppDiagCtx.buffersRead.ReadIndex = 0;
    }
}

//#################################################################################################
// To print configuration descriptor
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UppDiagConfigurationDescriptor(struct ConfigDescriptor *configDesc)
{
    ilog_UPP_COMPONENT_2(ILOG_MINOR_EVENT, UPP_PARSE_CONFIG_DESCRIPTOR_INFO,
        configDesc->bConfigurationVal,
        configDesc->bNumInterfaces);
}

//#################################################################################################
// To print Interface descriptor
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UppDiagInterfaceDescriptor(struct InterfaceDescriptor *interfaceDesc)
{
    ilog_UPP_COMPONENT_3(ILOG_MINOR_EVENT, UPP_INTERFACE_DESCRIPTOR_INFO,
        interfaceDesc->bInterfaceNumber,
        interfaceDesc->bAlternateSetting,
        interfaceDesc->bNumEndpoints);
}

//#################################################################################################
// To print endpoint descriptor
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void UppDiagEndpointDescriptor(struct EndpointDescriptor *endpointDesc)
{
    uint8_t transfer_type = endpointDesc->bmAttributes & ENDPOINT_ATTRIBUTE_TYPE_MASK;

    ilog_UPP_COMPONENT_0(ILOG_DEBUG, transfer_type == 0 ? UPP_ENDPOINT_ATTRIBUTE_CONTROL:(transfer_type == 0x1 ? UPP_ENDPOINT_ATTRIBUTE_ISO:
                    (transfer_type == 0x2 ? UPP_ENDPOINT_ATTRIBUTE_BULK:UPP_ENDPOINT_ATTRIBUTE_INTERRUPT)));

    //  ilog_UPP_COMPONENT_1(ILOG_DEBUG, UPP_ENDPOINT_DESC_TYPE, endpointDesc->bmAttributes);
    ilog_UPP_COMPONENT_1(ILOG_DEBUG, UPP_ENDPOINT_ADDRESS, endpointDesc->bEndpointAddress);
}
// Static Function Definitions ####################################################################

//#################################################################################################
// To print configuration descriptor
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void UppDiagPrintBuffer(void *param1, void *param2)
{
    if (uppDiagCtx.buffersRead.ReadIndex >= UPP_DIAG_MAX_BUFFERS_STORED)
    {
        uppDiagCtx.buffersRead.ReadIndex = 0;
    }

    if (uppDiagCtx.buffersRead.ReadIndex != uppDiagCtx.buffersRead.WriteIndex)
    {
        if (uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].fifo)
        {
            UART_printf(
                "D2H device:%d type %d defer %d bytes read %d timestamp %x CRC %x\n",
                uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].deviceAddress,
                uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].type,
                uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].deferred,
                uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].numberOfReads << 2,
                uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].packetTimeTag,
                uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].crc16);
        }
        else
        {
            UART_printf(
                "H2D device:%d type %d setup %d status %d bytes read %d timestamp %x CRC %x\n",
                uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].deviceAddress,
                uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].type,
                uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].setupPacket,
                uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].deferred,
                uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].numberOfReads << 2,
                uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].packetTimeTag,
                uppDiagCtx.buffersRead.buffer[uppDiagCtx.buffersRead.ReadIndex].crc16);
        }

        UART_WaitForTx();
        uppDiagCtx.buffersRead.ReadIndex++;

        CALLBACK_RunSingle(UppDiagPrintBuffer, NULL, NULL);
    }
    else
    {
        UART_printf("Done printing out!\n");
        // done printing out
        UPP_DiagReinit();
    }
}
