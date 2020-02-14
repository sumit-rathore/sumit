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
//!   @file  -  rex.c
//
//!   @brief -  Rex specific xrr functions
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "xrr_loc.h"
#ifdef XRR_DEBUG_INFINITE_LOOPS
#include <leon_uart.h>
#endif

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

// This enum corresponds to the accepted values for the XRR/Schedule/Mode field.
enum XRR_SchMode
{
    SCHMODE_WRITE             = XRR_XRR_SCHEDULE_MODE_BF_VALUE_SCH_MODE_WRITE,
    SCHMODE_READ              = XRR_XRR_SCHEDULE_MODE_BF_VALUE_SCH_MODE_READ,
    SCHMODE_UPDATE            = XRR_XRR_SCHEDULE_MODE_BF_VALUE_SCH_MODE_UPDATE,
    SCHMODE_UPDATE_NEW_ACTION = XRR_XRR_SCHEDULE_MODE_BF_VALUE_SCH_MODE_UPDATE_NEW_ACTION,
    SCHMODE_UPDATE_NEW_RESP   = XRR_XRR_SCHEDULE_MODE_BF_VALUE_SCH_MODE_UPDATE_NEW_RESP,
    SCHMODE_UPDATE_NEW_MOD    = XRR_XRR_SCHEDULE_MODE_BF_VALUE_SCH_MODE_UPDATE_NEW_MOD,
    SCHMODE_COPY              = XRR_XRR_SCHEDULE_MODE_BF_VALUE_SCH_MODE_COPY
};

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void _XRR_TransferPacket(
    enum XRR_SchMode mode, enum XRR_SchType schType, uint8 destQueue, uint8 srcQueue
    ) __attribute__ ((section(".rextext")));


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: XRR_rexEnable()
*
* @brief  - Enable Rex control
*
* @return - nothing
*
* @note   - Also clears all outstanding interrupts
*
*/
void XRR_rexEnable(void)
{
    uint32 rexCtrlReadValue = XRR_XRR_REXCTRL_READ_REG(XRR_BASE_ADDR);
    uint32 rexCtrlTempValue = rexCtrlReadValue;
    uint32 rexCtrlWriteValue;
    uint32 interruptClearValue = 0;
    uint32 intEnableBits;

    // Create value to write to clear interrupts
    interruptClearValue = XRR_XRR_INTCLR_IRQ0REXDEVRESP_SET_BF(interruptClearValue, 1);
    interruptClearValue = XRR_XRR_INTCLR_IRQ0REXRDFRMHDRASYNC_SET_BF(interruptClearValue, 1);
    interruptClearValue = XRR_XRR_INTCLR_IRQ0REXRDFRMHDRPER_SET_BF(interruptClearValue, 1);
    interruptClearValue = XRR_XRR_INTCLR_IRQ0REXRDFRMHDRMSA_SET_BF(interruptClearValue, 1);
    interruptClearValue = XRR_XRR_INTCLR_IRQ0REXSOF_SET_BF(interruptClearValue, 1);

    // Create value to write to RexCtrl
    rexCtrlTempValue = XRR_XRR_REXCTRL_SOFEN_SET_BF(rexCtrlTempValue, 1);
    rexCtrlTempValue = XRR_XRR_REXCTRL_TXEN_SET_BF(rexCtrlTempValue, 1);
    rexCtrlTempValue = XRR_XRR_REXCTRL_SCHEN_SET_BF(rexCtrlTempValue, 1);
    rexCtrlWriteValue = rexCtrlTempValue;

    // Clear & enable interrupts and enable Rex
    // Enable all interrupts, except SOF (its always enabled)
    XRR_XRR_INTCLR_WRITE_REG(XRR_BASE_ADDR, interruptClearValue);
    intEnableBits = XRR_XRR_INTEN_READ_REG(XRR_BASE_ADDR);
    intEnableBits = XRR_XRR_INTEN_IRQ0REXRDFRMHDRMSA_SET_BF(intEnableBits, 1);
    intEnableBits = XRR_XRR_INTEN_IRQ0REXRDFRMHDRPER_SET_BF(intEnableBits, 1);
    intEnableBits = XRR_XRR_INTEN_IRQ0REXRDFRMHDRASYNC_SET_BF(intEnableBits, 1);
    intEnableBits = XRR_XRR_INTEN_IRQ0REXDEVRESP_SET_BF(intEnableBits, 1);
    XRR_XRR_INTEN_WRITE_REG(XRR_BASE_ADDR, intEnableBits);
    XRR_XRR_REXCTRL_WRITE_REG(XRR_BASE_ADDR, rexCtrlWriteValue);

    ilog_XRR_COMPONENT_2(ILOG_DEBUG, XREX_ENABLE, rexCtrlReadValue, rexCtrlWriteValue);
}


/**
* FUNCTION NAME: XRR_rexDisable()
*
* @brief  - Disable Rex control
*
* @return - nothing
*
* @note   -
*
*/
void XRR_rexDisable(void)
{
    uint32 readValue = XRR_XRR_REXCTRL_READ_REG(XRR_BASE_ADDR);
    uint32 tempValue = readValue;
    uint32 writeValue;
    uint32 intDisableBits;
    uint32 interruptClearValue = 0;

    tempValue = XRR_XRR_REXCTRL_SOFEN_SET_BF(tempValue, 0);
    tempValue = XRR_XRR_REXCTRL_SOFAVGMODE_SET_BF(tempValue, 0);
    tempValue = XRR_XRR_REXCTRL_TXEN_SET_BF(tempValue, 0);
    tempValue = XRR_XRR_REXCTRL_SCHEN_SET_BF(tempValue, 0);
    writeValue = tempValue;

    XRR_XRR_REXCTRL_WRITE_REG(XRR_BASE_ADDR, writeValue);

    // Disable all interrupts, except SOF
    intDisableBits = XRR_XRR_INTEN_READ_REG(XRR_BASE_ADDR);
    intDisableBits = XRR_XRR_INTEN_IRQ0REXRDFRMHDRMSA_SET_BF(intDisableBits, 0);
    intDisableBits = XRR_XRR_INTEN_IRQ0REXRDFRMHDRPER_SET_BF(intDisableBits, 0);
    intDisableBits = XRR_XRR_INTEN_IRQ0REXRDFRMHDRASYNC_SET_BF(intDisableBits, 0);
    intDisableBits = XRR_XRR_INTEN_IRQ0REXDEVRESP_SET_BF(intDisableBits, 0);
    XRR_XRR_INTEN_WRITE_REG(XRR_BASE_ADDR, intDisableBits);

    // Create value to write to clear interrupts
    interruptClearValue = XRR_XRR_INTCLR_IRQ0REXDEVRESP_SET_BF(interruptClearValue, 1);
    interruptClearValue = XRR_XRR_INTCLR_IRQ0REXRDFRMHDRASYNC_SET_BF(interruptClearValue, 1);
    interruptClearValue = XRR_XRR_INTCLR_IRQ0REXRDFRMHDRPER_SET_BF(interruptClearValue, 1);
    interruptClearValue = XRR_XRR_INTCLR_IRQ0REXRDFRMHDRMSA_SET_BF(interruptClearValue, 1);
    interruptClearValue = XRR_XRR_INTCLR_IRQ0REXSOF_SET_BF(interruptClearValue, 1);
    XRR_XRR_INTCLR_WRITE_REG(XRR_BASE_ADDR, interruptClearValue);

    ilog_XRR_COMPONENT_2(ILOG_DEBUG, XREX_DISABLE, readValue, writeValue);
}


/**
* FUNCTION NAME: XRR_SOFSync()
*
* @brief  - Start the SOF synchronization engine
*
* @return - nothing
*
* @note   -
*
*/
void XRR_SOFSync(void)
{
    XRR_XRR_REXCTRL_SOFEN_WRITE_BF(XRR_BASE_ADDR, 1);
}


/**
* FUNCTION NAME: XRR_SOFEnable()
*
* @brief  - Enable SOFs on Rex
*
* @return - nothing
*
* @note   - This runs in a force SOF mode, use XRR_SOFDisable to disable
*
*/
void XRR_SOFEnable(void)
{
    uint32 readValue = XRR_XRR_REXCTRL_READ_REG(XRR_BASE_ADDR);
    uint32 tempValue = readValue;
    uint32 writeValue;

    tempValue = XRR_XRR_REXCTRL_SOFEN_SET_BF(tempValue, 1);
    tempValue = XRR_XRR_REXCTRL_TXEN_SET_BF(tempValue, 1);
    tempValue = XRR_XRR_REXCTRL_SOFAVGMODE_SET_BF(tempValue, 1);
    writeValue = tempValue;

    XRR_XRR_REXCTRL_WRITE_REG(XRR_BASE_ADDR, writeValue);
}

/**
* FUNCTION NAME: XRR_SOFDisable()
*
* @brief  - Disable SOFs on Rex
*
* @return - nothing
*
* @note   -
*
*/
void XRR_SOFDisable(void)
{
    uint32 readValue = XRR_XRR_REXCTRL_READ_REG(XRR_BASE_ADDR);
    uint32 tempValue = readValue;
    uint32 writeValue;

    tempValue = XRR_XRR_REXCTRL_SOFEN_SET_BF(tempValue, 0);
    tempValue = XRR_XRR_REXCTRL_TXEN_SET_BF(tempValue, 0);
    tempValue = XRR_XRR_REXCTRL_SOFAVGMODE_SET_BF(tempValue, 0);
    writeValue = tempValue;

    XRR_XRR_REXCTRL_WRITE_REG(XRR_BASE_ADDR, writeValue);
}

/**
* FUNCTION NAME: XRR_ClearInterruptDeviceResponse
*
* @brief  - Clear the REX Device Response interrupt
*
*/
void XRR_ClearInterruptDeviceResponse(void)
{
    uint32 writeVal = XRR_XRR_INTCLR_IRQ0REXDEVRESP_SET_BF(0, 1);
    XRR_XRR_INTCLR_WRITE_REG(XRR_BASE_ADDR, writeVal);
}

/**
* FUNCTION NAME: XRR_ClearInterruptReadFrameHeaderAsync
*
* @brief  - Clear the REX Async interrupt
*
*/
void XRR_ClearInterruptReadFrameHeaderAsync(void)
{
    uint32 writeVal = XRR_XRR_INTCLR_IRQ0REXRDFRMHDRASYNC_SET_BF(0, 1);
    XRR_XRR_INTCLR_WRITE_REG(XRR_BASE_ADDR, writeVal);
}

/**
* FUNCTION NAME: XRR_ClearInterruptReadFrameHeaderMSA
*
* @brief  - Clear the REX MSA interrupt
*
*/
void XRR_ClearInterruptReadFrameHeaderMSA(void)
{
    uint32 writeVal = XRR_XRR_INTCLR_IRQ0REXRDFRMHDRMSA_SET_BF(0, 1);
    XRR_XRR_INTCLR_WRITE_REG(XRR_BASE_ADDR, writeVal);
}

/**
* FUNCTION NAME: XRR_ClearInterruptReadFrameHeaderPerInt
*
* @brief  - Clear the REX Periodic interrupt
*
*/
void XRR_ClearInterruptReadFrameHeaderPerInt(void)
{
    uint32 writeVal = XRR_XRR_INTCLR_IRQ0REXRDFRMHDRPER_SET_BF(0, 1);
    XRR_XRR_INTCLR_WRITE_REG(XRR_BASE_ADDR, writeVal);
}

/**
* FUNCTION NAME: XRR_ClearInterruptSofInt
*
* @brief  - Clear the REX SOF interrupt
*
*/
void XRR_ClearInterruptSofInt(void)
{
    uint32 writeVal = XRR_XRR_INTCLR_IRQ0REXSOF_SET_BF(0, 1);
   XRR_XRR_INTCLR_WRITE_REG(XRR_BASE_ADDR, writeVal);
}

/**
* FUNCTION NAME: XRR_GetInterrupts
*
* @brief  - Get the interrupt flag register
*
* @return - interrupt flag register
*
* @note   - There may be other fields returned.  Only access by XRR_InterruptBitMaskT functions
*
*/
XRR_InterruptBitMaskT XRR_GetInterrupts(void)
{
    // Ensure the spectareg bits match our enums
    COMPILE_TIME_ASSERT(REX_DEV_RESP            == XRR_XRR_INTFLG_IRQ0REXDEVRESP_BF_SHIFT);
    COMPILE_TIME_ASSERT(REX_RD_FRM_HDR_ASYNC    == XRR_XRR_INTFLG_IRQ0REXRDFRMHDRASYNC_BF_SHIFT);
    COMPILE_TIME_ASSERT(REX_RD_FRM_HDR_PER      == XRR_XRR_INTFLG_IRQ0REXRDFRMHDRPER_BF_SHIFT);
    COMPILE_TIME_ASSERT(REX_RD_FRM_HDR_MSA      == XRR_XRR_INTFLG_IRQ0REXRDFRMHDRMSA_BF_SHIFT);

    return CAST(XRR_XRR_INTFLG_READ_REG(XRR_BASE_ADDR), uint32, XRR_InterruptBitMaskT);
}

/**
* FUNCTION NAME: XRR_UpdateSchPacketNewAction
*
* @brief  - Copy frame header from RdFrmHdr register to SchFrmHdr register
*           i.e. from inbound side to the outbound side, and schedule the packet
*           to the destination queue
*
* @return - nothing
*
* @note   - Everything except action field is copied; function will overwrite
*           the action field.
*
*/
void XRR_UpdateSchPacketNewAction
(
    enum XRR_SchType schType,
    uint8 destQueue,
    uint8 action
)
{
    const enum XRR_SchMode mode = SCHMODE_UPDATE_NEW_ACTION;
    const uint8 srcQueue = 0;

    XRR_XRR_SCHFRMHDR0_ACTION_WRITE_BF(XRR_BASE_ADDR, action);

    _XRR_TransferPacket(mode, schType, destQueue, srcQueue);
}

/**
* FUNCTION NAME: XRR_UpdateSchPacketNewResponse
*
* @brief  - Copy frame header from RdFrmHdr register to SchFrmHdr register
*           i.e. from inbound side to the outbound side, and schedule the packet
*           to the destination queue
*
* @return - nothing
*
* @note   - Everything except reponse field is copied; function will overwrite
*           the response field.
*
*/
void XRR_UpdateSchPacketNewResponse
(
    enum XRR_SchType schType,
    uint8 destQueue,
    uint8 resp
)
{
    const enum XRR_SchMode mode = SCHMODE_UPDATE_NEW_RESP;
    const uint8 srcQueue = 0;

    // set the new response
    XRR_XRR_SCHFRMHDR1_RESPONSE_WRITE_BF(XRR_BASE_ADDR, resp);

    _XRR_TransferPacket(mode, schType, destQueue, srcQueue);
}

/**
* FUNCTION NAME: XRR_UpdateSchPacketNewMod
*
* @brief  - Copy frame header from RdFrmHdr register to SchFrmHdr register
*           i.e. from inbound side to the outbound side, and schedule the packet
*           to the destination queue
*
* @return - nothing
*
* @note   - Everything except modifier field is copied; function will overwrite
*           the modifier field.
*
*/
void XRR_UpdateSchPacketNewMod
(
    enum XRR_SchType schType,
    uint8 destQueue,
    uint8 mod
)
{
    const enum XRR_SchMode mode = SCHMODE_UPDATE_NEW_MOD;
    const uint8 srcQueue = 0;

    // set the new action
    XRR_XRR_SCHFRMHDR0_MODIFIER_WRITE_BF(XRR_BASE_ADDR, mod);

    _XRR_TransferPacket(mode, schType, destQueue, srcQueue);
}

/**
* FUNCTION NAME: XRR_CopyFrameHeader()
*
* @brief  - Copy frame header from the specified source queue to SchFrmHdr register
*           and schedule the packet to the destination queue
*
* @return - nothing
*
* @note   - RTL will read the source queue into the RdFrmHdr register and then
*           copied to the SchFrmHdr register.
*
*/
void XRR_CopyFrameHeader
(
    enum XRR_SchType schType,
    uint8 srcQueue,
    uint8 destQueue
)
{
    const enum XRR_SchMode mode = SCHMODE_COPY;

    _XRR_TransferPacket(mode, schType, destQueue, srcQueue);
}

/**
* FUNCTION NAME: XRR_WriteSchPacket
*
* @brief  - Write frame header to SchFrmHdr register and schedule the packet to destination
*           queue
*
* @return - nothing
*
* @note   - NOTE: please make sure read and write frame headers are stored in separate variables
*           to avoid memory corruption. Before calling this, make sure the count and response
*           variables have been set correctly in the output frame header.
*
*/
void XRR_WriteSchPacket
(
    uint64 frameHeader,
    enum XRR_SchType schType,
    uint8 destQueue
)
{
    const enum XRR_SchMode mode = SCHMODE_WRITE;
    const uint8 srcQueue = 0;
    uint32 reg;

    reg = GET_LSW_FROM_64(frameHeader);
    XRR_XRR_SCHFRMHDR0_WRITE_REG(XRR_BASE_ADDR, reg);

    reg = GET_MSW_FROM_64(frameHeader);
    XRR_XRR_SCHFRMHDR1_WRITE_REG(XRR_BASE_ADDR, reg);

    _XRR_TransferPacket(mode, schType, destQueue, srcQueue);
}

/**
* FUNCTION NAME: XRR_UpdateSchPacket
*
* @brief  - Copy frame header from RdFrmHdr register to SchFrmHdr register
*           i.e. from inbound side to the outbound side, and schedule the packet
*           to the destination queue
*
* @return - nothing
*
* @note   - RTL will not read the source queue into the RdFrmHdr register before
*           copying it to the SchFrmHdr register.
*
*/
void XRR_UpdateSchPacket
(
    enum XRR_SchType schType,
    uint8 destQueue
)
{
    const enum XRR_SchMode mode = SCHMODE_UPDATE;
    const uint8 srcQueue = 0;

    _XRR_TransferPacket(mode, schType, destQueue, srcQueue);
}

/**
* FUNCTION NAME: XRR_ReadQueue
*
* @brief  - Read the frame header for the specified queue to the RdFrmHdr register
* @brief  - Read the input frame header from the read frame header registers
*
* @return - A frame header value intended to be put into an XRR_FrameHeader union.
*/
XRR_FrameHeaderRaw XRR_ReadQueue
(
    uint8 srcQueue
)
{
    const enum XRR_SchMode mode = SCHMODE_READ;
    const enum XRR_SchType schType = SCHTYPE_NONE;
    const uint8 destQueue = 0;

    _XRR_TransferPacket(mode, schType, destQueue, srcQueue);

    return MAKE_U64(
            XRR_XRR_RDFRMHDR1_READ_REG(XRR_BASE_ADDR),
            XRR_XRR_RDFRMHDR0_READ_REG(XRR_BASE_ADDR));
}


/**
* FUNCTION NAME: XRR_GetInputFrameHeader2()
*
* @brief  - Read the second input frame header from the read frame header registers
*
* @return - The second input frame header value intended to be put into an XRR_InputFrameHeader2
*           union.
*/
XRR_InputFrameHeader2Raw XRR_GetInputFrameHeader2(void)
{
    return XRR_XRR_RDFRMHDR2_READ_REG(XRR_BASE_ADDR);
}

/**
* FUNCTION NAME: XRR_GetTransferLen
*
* @brief  - Get the transfer length of the current MSA packet
*
* @return - the transfer length of the current MSA packet
*
* @note   -
*
*/
uint32 XRR_GetTransferLen(void)
{
    return(XRR_XRR_RDFRMHDRMSA_TRANSFERLEN_READ_BF(XRR_BASE_ADDR));
}

/**
* FUNCTION NAME: XRR_ClearInterrupts()
*
* @brief  - Clear Rex interrupts
*
* @return - nothing
*
* @note   - used after clearing the cache or flushing queues
*
*/
void XRR_ClearInterrupts(void)
{
    uint32 ints = 0;

    ints = XRR_XRR_INTCLR_IRQ0REXDEVRESP_SET_BF(ints, 1);
    ints = XRR_XRR_INTCLR_IRQ0REXRDFRMHDRASYNC_SET_BF(ints, 1);
    ints = XRR_XRR_INTCLR_IRQ0REXRDFRMHDRPER_SET_BF(ints, 1);
    ints = XRR_XRR_INTCLR_IRQ0REXRDFRMHDRMSA_SET_BF(ints, 1);
    ints = XRR_XRR_INTCLR_IRQ0REXSOF_SET_BF(ints, 1);
    XRR_XRR_INTCLR_WRITE_REG(XRR_BASE_ADDR, ints);
}


/**
* FUNCTION NAME: XRR_EnableInterruptSOF()
*
* @brief  - Enables the SOF interrupt
*
* @return - void
*
* @note   - Intended for Test Harnesses
*
*/
void XRR_EnableInterruptSOF(void)
{
    XRR_XRR_INTEN_IRQ0REXSOF_WRITE_BF(XRR_BASE_ADDR, 1);
}


/**
* FUNCTION NAME: XRR_DisableInterruptSOF()
*
* @brief  - Disables the SOF interrupt
*
* @return - void
*
* @note   - Intended for Test Harnesses
*
*/
void XRR_DisableInterruptSOF(void)
{
    XRR_XRR_INTEN_IRQ0REXSOF_WRITE_BF(XRR_BASE_ADDR, 0);
}


/**
* FUNCTION NAME: XRR_ClearInterruptSOF()
*
* @brief  - Clears the SOF interrupt
*
* @return - void
*
* @note   - Intended for Test Harnesses
*
*/
void XRR_ClearInterruptSOF(void)
{
    // Write a 1 to the SOF bit position, and a 0 everywhere else
    XRR_XRR_INTCLR_WRITE_REG(XRR_BASE_ADDR, XRR_XRR_INTCLR_IRQ0REXSOF_SET_BF(0, 1));
}


/**
* FUNCTION NAME: XRR_SetSOFGenHighSpeed()
*
* @brief  - set the minimum time between sending out SOFs in full speed
*
* @return - void
*
* @note   - values defined in bug2477
*
*/
void XRR_SetSOFGenHighSpeed(void)
{
    XRR_XRR_SOFCTRL_MINTIME_WRITE_BF(XRR_BASE_ADDR, 7500);
}


/**
* FUNCTION NAME: XRR_SetSOFGenFullLowSpeed()
*
* @brief  - set the minimum time between sending out SOFs in full speed
*
* @return - void
*
* @note   - values defined in bug2477
*
*/
void XRR_SetSOFGenFullLowSpeed(void)
{
    XRR_XRR_SOFCTRL_MINTIME_WRITE_BF(XRR_BASE_ADDR, 60000);
}


/**
* FUNCTION NAME: XRR_SetSOFGenDefaultSpeed()
*
* @brief  - set the minimum time between sending out SOFs in after prefetch
*
* @return - void
*
* @note   - values defined in bug2477
*
*/
void XRR_SetSOFGenDefaultSpeed(void)
{
    XRR_XRR_SOFCTRL_MINTIME_WRITE_BF(XRR_BASE_ADDR, 59960);
}

/**
* FUNCTION NAME: XRR_GetLsbFrameAndUFrame
*
* @brief  -
*
* @return - the current uframe and the least significant bit
*           of the frame
*
* @note   - Bit[3]: least significant bit of the current frame
*           Bit[2:0]: current microframe
*
*/
uint8 XRR_GetLsbFrameAndUFrame(void)
{
    uint8 lsbFrameAndUframe = XRR_XRR_FRAMEINFO_CURUFRAME_READ_BF(XRR_BASE_ADDR);

    return lsbFrameAndUframe;
}

/**
* FUNCTION NAME: XRR_Get_Q_Stat()
*
* @brief  - return the current status of the selected queue
*
* @return - Q status
*
* @note   -
*
*/
uint32 XRR_Get_Q_Stat
(
    uint32 src_q
)
{
    // Update the Schedule-SrcQStat register
    XRR_XRR_SCHEDULE_SRCQUEUE_WRITE_BF(XRR_BASE_ADDR, src_q);

    return XRR_XRR_SCHEDULE_SRCQSTAT_READ_BF(XRR_BASE_ADDR);
}

/**
* FUNCTION NAME: XRR_assertHook
*
* @brief  - Assert hook for XRR that logs REX CTRL, SOF CTRL,
*           and SOF DBG1 registers
*
* @return -
*
* @note   -
*
*/
void XRR_assertHook(void)
{
    ilog_XRR_COMPONENT_2(
            ILOG_FATAL_ERROR,
            XRR_SPECTAREG_READ,
            XRR_BASE_ADDR + XRR_XRR_REXCTRL_REG_OFFSET,
            XRR_XRR_REXCTRL_READ_REG(XRR_BASE_ADDR));

    ilog_XRR_COMPONENT_2(
            ILOG_FATAL_ERROR,
            XRR_SPECTAREG_READ,
            XRR_BASE_ADDR + XRR_XRR_SOFCTRL_REG_OFFSET,
            XRR_XRR_SOFCTRL_READ_REG(XRR_BASE_ADDR));

    ilog_XRR_COMPONENT_2(
            ILOG_FATAL_ERROR,
            XRR_SPECTAREG_READ,
            XRR_BASE_ADDR + XRR_XRR_SOFDBG1_REG_OFFSET,
            XRR_XRR_SOFDBG1_READ_REG(XRR_BASE_ADDR));

    // Process stat registers
    _XRR_ErrorCountCheck();
    _XRR_FlowControlCheck();

    const uint32 xrtDebugReg = XRR_XRR_XRTDEBUG_READ_REG(XRR_BASE_ADDR);
    ilog_XRR_COMPONENT_2(
        ILOG_FATAL_ERROR,
        XRR_XRT_DEBUG_DUMP_1,
        XRR_XRR_XRTDEBUG_RDADDR_GET_BF(xrtDebugReg),
        XRR_XRR_XRTDEBUG_RDENDP_GET_BF(xrtDebugReg));
    ilog_XRR_COMPONENT_3(
        ILOG_FATAL_ERROR,
        XRR_XRT_DEBUG_DUMP_2,
        XRR_XRR_XRTDEBUG_RDACTION_GET_BF(xrtDebugReg),
        XRR_XRR_XRTDEBUG_RDDATAQID_GET_BF(xrtDebugReg),
        XRR_XRR_XRTDEBUG_LASTRESP_GET_BF(xrtDebugReg));
}


/******************************* Local Functions ****************************************/

/**
* FUNCTION NAME: _XRR_TransferPacket
*
* @brief  - Generic function for transferring a queue
*
* @return - void
*
* @note   - srcQueue is only used when copying packet from dynamic queues
*
*/
static void _XRR_TransferPacket
(
    enum XRR_SchMode mode,
    enum XRR_SchType schType,
    uint8 destQueue,
    uint8 srcQueue
)
{
    uint32 data = 0;

    data = XRR_XRR_SCHEDULE_MODE_SET_BF(data, mode);
    data = XRR_XRR_SCHEDULE_SCHTYPE_SET_BF(data, schType);
    data = XRR_XRR_SCHEDULE_DESTQUEUE_SET_BF(data, destQueue);
    data = XRR_XRR_SCHEDULE_SRCQUEUE_SET_BF(data, srcQueue);
    data = XRR_XRR_SCHEDULE_ENABLE_SET_BF(data, 1);

    XRR_XRR_SCHEDULE_WRITE_REG(XRR_BASE_ADDR, data);

    while (XRR_XRR_SCHEDULE_ENABLE_READ_BF(XRR_BASE_ADDR) == 1)
        ;
}

