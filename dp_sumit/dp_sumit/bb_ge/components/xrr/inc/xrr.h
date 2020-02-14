///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
//!   @file  -  xrr.h
//
//!   @brief -  XRR driver code
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XRR_H
#define XRR_H

/***************************** Included Headers ******************************/
#include <ibase.h>

/************************ Defined Constants and Macros ***********************/

// This enum corresponds to the accepted values for the XRR/Schedule/SchType field.  The enum
// exists so that function type signatures can more clearly express which values are acceptable for
// schType rather than taking a uint8 parameter based on the
// XRR_XRR_SCHEDULE_SCHTYPE_BF_VALUE_SCHTYPE_* defines from xrr_mmreg_macro.h.
// NOTE: If the enum changes, update the COMPILE_TIME_ASSERT calls in xrr/src/init.c.
enum XRR_SchType
{
    SCHTYPE_NONE,
    SCHTYPE_ASYNC,
    SCHTYPE_PER,
    SCHTYPE_MSA,
    SCHTYPE_MSA_RETRY,
    SCHTYPE_UPS_ASYNC,
    SCHTYPE_LOCAL,
    SCHTYPE_UPS_ACC
};


/******************************** Data Types *********************************/

// This typedef exists as a way to express a function return value or a parameter in a way that
// explains to the reader that the value should be put into a XRR_FrameHeader union.  The union is
// not passed around directly because GCC 4.7.1 generates less efficient code if you do that.
typedef uint64 XRR_FrameHeaderRaw;
// Structure that encapsulates RdFrmHdr0-1 and SchFrmHdr0-1 registers
union XRR_FrameHeader
{
    XRR_FrameHeaderRaw raw;

    // Fields common between RdFrmHdr0-1 and SchFrmHdr0-1 registers
    struct {
        uint64 reserved2:   16;
        uint64 splitSE:     2;
        uint64 hubAddr:     7;
        uint64 hubPort:     7;

        uint64 reserved1:   7;
        uint64 toggle:      1;
        uint64 dataQid:     7;
        uint64 modifier:    2;
        uint64 action:      2;
        uint64 endpType:    2;
        uint64 endp:        4;
        uint64 addr:        7;
    } generic;

    // Field specific to RdFrmHdr0-1 registers
    struct {
        uint64 reserved:    1;
        uint64 rxFrame:     11;
        uint64 rxUFrame:    3;
        uint64 error:       1;
        uint64 generic2:    16;

        uint64 response:    4;
        uint64 schType:     3;
        uint64 generic1:    25;
    } input;

    // Field specific to SchFrmHdr0-1 registers
    struct {
        uint64 reserved2:   9;
        uint64 count:       3;
        uint64 response:    4;
        uint64 generic2:    16;

        uint64 reserved1:   7;
        uint64 generic1:    25;
    } output;
};


// This typedef exists as a way to express a function return value or a parameter in a way that
// explains to the reader that the value should be put into a XRR_InputFrameHeader2 union.  The
// union is not passed around directly because GCC 4.7.1 generates less efficient code if you do
// that.
typedef uint32 XRR_InputFrameHeader2Raw;
// Structure that encapsulates RdFrmHdr2 register
// hence it is named InputFrameHeader2
union XRR_InputFrameHeader2
{
    XRR_InputFrameHeader2Raw frameHeader;
    struct
    {
        uint32 reserved:    25;
        uint32 cbwDir:      1;
        // number of packets sent downstream per frame
        uint32 count:       3;
        uint32 accel:       3;
    } frameHeaderStruct;
};

// This matches the Spectareg hw macros!!!!
// If this changes, ensure it matches checks in rex.c:XRR_GetInterrupts()
enum XRR_InterruptBit
{
    REX_DEV_RESP,
    REX_RD_FRM_HDR_ASYNC,
    REX_RD_FRM_HDR_PER,
    REX_RD_FRM_HDR_MSA,
    REX_SOF
        // NOTE: There are more bits in the RTL.  Not (yet) used by SW
};

// Abstract type
struct _XRR_InterruptBitMask;
typedef struct _XRR_InterruptBitMask * XRR_InterruptBitMaskT;

/*********************************** API *************************************/

// Init
void XRR_Init(void);

void XRR_assertHook(void);

// Irq functions
static inline boolT XRR_CheckInterruptBit(XRR_InterruptBitMaskT mask, enum XRR_InterruptBit bit);
XRR_InterruptBitMaskT XRR_GetInterrupts(void) __attribute__ ((section(".rextext")));
void XRR_ClearInterrupts(void) __attribute__ ((section(".rextext")));

// REX Control
void XRR_rexEnable(void);
void XRR_rexDisable(void)    __attribute__((section(".rextext")));
void XRR_SOFSync(void);

void XRR_SOFEnable(void);
void XRR_SOFDisable(void);

// SOF interrupt handling.  Really only for test harnesses
void XRR_EnableInterruptSOF(void) __attribute__ ((section(".rextext")));
void XRR_DisableInterruptSOF(void) __attribute__ ((section(".rextext")));
void XRR_ClearInterruptSOF(void) __attribute__ ((section(".rextext")));

void XRR_ClearInterruptDeviceResponse(void) __attribute__ ((section(".rextext")));
void XRR_ClearInterruptReadFrameHeaderAsync(void) __attribute__ ((section(".rextext")));
void XRR_ClearInterruptReadFrameHeaderMSA(void) __attribute__ ((section(".rextext")));
void XRR_ClearInterruptReadFrameHeaderPerInt(void) __attribute__ ((section(".rextext")));
void XRR_ClearInterruptSofInt(void) __attribute__ ((section(".rextext")));

// SOF Speed Setting
void XRR_SetSOFGenHighSpeed(void) __attribute__ ((section(".rextext")));
void XRR_SetSOFGenFullLowSpeed(void) __attribute__ ((section(".rextext")));
void XRR_SetSOFGenDefaultSpeed(void) __attribute__ ((section(".rextext")));

// Packet Scheduling
void XRR_UpdateSchPacket(
    enum XRR_SchType schType, uint8 dest_queue) __attribute__ ((section(".rextext")));
void XRR_UpdateSchPacketNewAction(
    enum XRR_SchType schType, uint8 dest_queue, uint8 action
    ) __attribute__ ((section(".rextext")));
void XRR_UpdateSchPacketNewResponse(
    enum XRR_SchType schType, uint8 dest_queue, uint8 resp) __attribute__ ((section(".rextext")));
void XRR_UpdateSchPacketNewMod(
    enum XRR_SchType schType, uint8 dest_queue, uint8 mod) __attribute__ ((section(".rextext")));
void XRR_CopyFrameHeader(
    enum XRR_SchType schType, uint8 src_queue, uint8 dest_queue
    ) __attribute__ ((section(".rextext")));
void XRR_WriteSchPacket(
    uint64 frameHeader, enum XRR_SchType schType, uint8 dest_queue
    ) __attribute__ ((section(".rextext")));
XRR_FrameHeaderRaw XRR_ReadQueue(uint8 src_queue) __attribute__ ((section(".rextext")));

// Miscellaneous
uint32 XRR_GetTransferLen(void) __attribute__ ((section(".rextext")));
uint8 XRR_GetLsbFrameAndUFrame(void) __attribute__ ((section(".rextext")));
uint32 XRR_Get_Q_Stat(uint32 src_q) __attribute__ ((section(".rextext")));

// Frame Headers
XRR_InputFrameHeader2Raw XRR_GetInputFrameHeader2(void) __attribute__ ((section(".rextext")));

/************************ Static Inline Definitions **************************/

static inline boolT XRR_CheckInterruptBit(XRR_InterruptBitMaskT mask, enum XRR_InterruptBit bit)
{
    return (0 != (( CAST(mask,XRR_InterruptBitMaskT, uint32))
                    & (1 << CAST(bit, enum XRR_InterruptBit, uint32))));
}

#endif // XRR_H

