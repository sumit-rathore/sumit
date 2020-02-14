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
//!   @file  -  init.c
//
//!   @brief -  initialization functions for the xrr component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "xrr_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void _XRR_InputFrameHeaderBitFieldSanityCheck(void);
static void _XRR_InputFrameHeader2BitFieldSanityCheck(void);
static void _XRR_OutputFrameHeaderBitFieldSanityCheck(void);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: XRR_Init()
*
* @brief  - Initialisation code for the XRR
*
* @return - nothing
*
* @note   -
*
*/
void XRR_Init(void)
{
    uint32 intEnableBits = 0;

    // Check ID register
    iassert_XRR_COMPONENT_0(XRR_XRR_ID_ID_READ_BF(XRR_BASE_ADDR) == XRR_XRR_ID_ID_BF_RESET, INVALID_ID);

    // Check CVS register
    {
        uint32 revRegValue = XRR_XRR_REV_READ_REG(XRR_BASE_ADDR);
        iassert_XRR_COMPONENT_2(XRR_XRR_REV_CVSMAJOR_GET_BF(revRegValue) == XRR_XRR_REV_CVSMAJOR_BF_RESET,
                                INVALID_CVS_MAJOR,
                                XRR_XRR_REV_CVSMAJOR_GET_BF(revRegValue),
                                XRR_XRR_REV_CVSMAJOR_BF_RESET);
        iassert_XRR_COMPONENT_2(XRR_XRR_REV_CVSMINOR_GET_BF(revRegValue) == XRR_XRR_REV_CVSMINOR_BF_RESET,
                                INVALID_CVS_MINOR,
                                XRR_XRR_REV_CVSMINOR_GET_BF(revRegValue),
                                XRR_XRR_REV_CVSMINOR_BF_RESET);
    }

    // Make sure that all of the enum XRR_SchType values match the values from the spectareg header.
    COMPILE_TIME_ASSERT(SCHTYPE_NONE == XRR_XRR_SCHEDULE_SCHTYPE_BF_VALUE_SCHTYPE_NONE);
    COMPILE_TIME_ASSERT(SCHTYPE_ASYNC == XRR_XRR_SCHEDULE_SCHTYPE_BF_VALUE_SCHTYPE_ASYNC);
    COMPILE_TIME_ASSERT(SCHTYPE_PER == XRR_XRR_SCHEDULE_SCHTYPE_BF_VALUE_SCHTYPE_PER);
    COMPILE_TIME_ASSERT(SCHTYPE_MSA == XRR_XRR_SCHEDULE_SCHTYPE_BF_VALUE_SCHTYPE_MSA);
    COMPILE_TIME_ASSERT(SCHTYPE_MSA_RETRY == XRR_XRR_SCHEDULE_SCHTYPE_BF_VALUE_SCHTYPE_MSA_RETRY);
    COMPILE_TIME_ASSERT(SCHTYPE_UPS_ASYNC == XRR_XRR_SCHEDULE_SCHTYPE_BF_VALUE_SCHTYPE_UPS);
    COMPILE_TIME_ASSERT(SCHTYPE_LOCAL == XRR_XRR_SCHEDULE_SCHTYPE_BF_VALUE_SCHTYPE_LOCAL);
    // The spectareg #define was missing, but should be included in the next FPGA release.
    //COMPILE_TIME_ASSERT(SCHTYPE_UPS_ACC == XRR_XRR_SCHEDULE_SCHTYPE_BF_VALUE_SCHTYPE_UPS_ACC);

    // check the sanity of frame header bitfields
    _XRR_InputFrameHeaderBitFieldSanityCheck();
    _XRR_InputFrameHeader2BitFieldSanityCheck();
    _XRR_OutputFrameHeaderBitFieldSanityCheck();

    // Note: The interrupt handler is in the RexScheduler

    // Enable all rex ints
    intEnableBits = XRR_XRR_INTEN_IRQ0REXSOF_SET_BF(intEnableBits, 1);

    // Set nak limit to 256*256 = 65536
    // Each unit represents 256 NAKs, hence the value of 256
    XRR_XRR_TXCTRL_MSANAKLIMIT_WRITE_BF(XRR_BASE_ADDR, 0x0100);

    // Enable the interrupts
    XRR_XRR_INTEN_WRITE_REG(XRR_BASE_ADDR, intEnableBits);

    // Initialize stats timers
    {
        TIMING_TimerHandlerT flowControlTimerHandle;
        TIMING_TimerHandlerT errorCountTimerHandle;

        // Create the periodic 1000 millisecond timers for checking the stats
        errorCountTimerHandle = TIMING_TimerRegisterHandler(&_XRR_ErrorCountCheck, TRUE, 1000);
        TIMING_TimerStart(errorCountTimerHandle);

        flowControlTimerHandle = TIMING_TimerRegisterHandler(&_XRR_FlowControlCheck, TRUE, 1000);
        TIMING_TimerStart(flowControlTimerHandle);
    }
}


/**
* FUNCTION NAME: _XRR_InputFrameHeaderBitFieldSanityCheck()
*
* @brief  - Contains runtime time checks to ensure the generated defines are in sync with the bitfields
*
* @return - void
*
* @note   - This uses a local helper macro to make life easier
*
*           The structure is defined with widths from the generated defines,
*           so here we are verifying that the positions are correct.
*
*           This check should really be a compile time assert, but GCC can't seem to figure it out at compile time
*           Even so when the assert won't trigger, GCC optimizes this function away
*/
static void _XRR_InputFrameHeaderBitFieldSanityCheck(void)
{
    // define helper macro
#define _XRR_CHECK_BIT(_struct_, bitfield, offset) do                                           \
    {                                                                                           \
        /* Local structures for comparisons */                                                  \
        union XRR_FrameHeader defs;                                                             \
        union XRR_FrameHeader bits;                                                             \
                                                                                                \
        /* Clear structures */                                                                  \
        defs.raw = 0;                                                                           \
        bits.raw = 0;                                                                           \
                                                                                                \
        /* Set bits that should be identical in both */                                         \
        defs.raw = 1ULL << (offset);                                                            \
        bits._struct_.bitfield = 1;                                                             \
                                                                                                \
        /* Verify that the bits were identical */                                               \
        iassert_XRR_COMPONENT_1(defs.raw == bits.raw,                                           \
                XRR_BITFIELD_CHECK_FAILURE, __LINE__);                                          \
    } while (FALSE)

    // LSW
    _XRR_CHECK_BIT(generic, addr, XRR_XRR_RDFRMHDR0_ADDR_BF_SHIFT);
    _XRR_CHECK_BIT(generic, endp, XRR_XRR_RDFRMHDR0_ENDP_BF_SHIFT);
    _XRR_CHECK_BIT(generic, endpType, XRR_XRR_RDFRMHDR0_ENDPTYPE_BF_SHIFT);
    _XRR_CHECK_BIT(generic, action, XRR_XRR_RDFRMHDR0_ACTION_BF_SHIFT);
    _XRR_CHECK_BIT(generic, modifier, XRR_XRR_RDFRMHDR0_MODIFIER_BF_SHIFT);
    _XRR_CHECK_BIT(generic, dataQid, XRR_XRR_RDFRMHDR0_DATAQID_BF_SHIFT);
    _XRR_CHECK_BIT(generic, toggle, XRR_XRR_RDFRMHDR0_TOGGLE_BF_SHIFT);
    _XRR_CHECK_BIT(input, schType, XRR_XRR_RDFRMHDR0_SCHTYPE_BF_SHIFT);
    _XRR_CHECK_BIT(input, response, XRR_XRR_RDFRMHDR0_RESPONSE_BF_SHIFT);

    // MSW
    _XRR_CHECK_BIT(generic, hubPort, XRR_XRR_RDFRMHDR1_HUBPORT_BF_SHIFT + 32);
    _XRR_CHECK_BIT(generic, hubAddr, XRR_XRR_RDFRMHDR1_HUBADDR_BF_SHIFT + 32);
    _XRR_CHECK_BIT(generic, splitSE, XRR_XRR_RDFRMHDR1_SPLITSE_BF_SHIFT + 32);
    _XRR_CHECK_BIT(input, error, XRR_XRR_RDFRMHDR1_ERROR_BF_SHIFT + 32);
    _XRR_CHECK_BIT(input, rxUFrame, XRR_XRR_RDFRMHDR1_RXUFRAME_BF_SHIFT + 32);
    _XRR_CHECK_BIT(input, rxFrame, XRR_XRR_RDFRMHDR1_RXFRAME_BF_SHIFT + 32);

#undef _XRR_CHECK_BIT
}

/**
* FUNCTION NAME: _XRR_InputFrameHeader2BitFieldSanityCheck()
*
* @brief  - Contains runtime time checks to ensure the generated defines are in sync with the bitfields
*
* @return - void
*
* @note   - This uses a local helper macro to make life easier
*
*           The structure is defined with widths from the generated defines,
*           so here we are verifying that the positions are correct.
*
*           This check should really be a compile time assert, but GCC can't seem to figure it out at compile time
*           Even so when the assert won't trigger, GCC optimizes this function away
*/
static void _XRR_InputFrameHeader2BitFieldSanityCheck(void)
{
    // define helper macro
#define _XRR_CHECK_BIT(bitfield, offset) do                                                     \
    {                                                                                           \
        /* Local structures for comparisons */                                                  \
        union XRR_InputFrameHeader2 defs;                                                       \
        union XRR_InputFrameHeader2 bits;                                                       \
                                                                                                \
        /* Clear structures */                                                                  \
        defs.frameHeader = 0;                                                                   \
        bits.frameHeader = 0;                                                                   \
                                                                                                \
        /* Set bits that should be identical in both */                                         \
        defs.frameHeader = 1UL << (offset);                                                     \
        bits.frameHeaderStruct.bitfield = 1;                                                    \
                                                                                                \
        /* Verify that the bits were identical */                                               \
        iassert_XRR_COMPONENT_1(defs.frameHeader == bits.frameHeader,                           \
                XRR_BITFIELD_CHECK_FAILURE, __LINE__);                                          \
    } while (FALSE)

    _XRR_CHECK_BIT(accel, XRR_XRR_RDFRMHDR2_ACCEL_BF_SHIFT);
    _XRR_CHECK_BIT(count, XRR_XRR_RDFRMHDR2_COUNT_BF_SHIFT);
    _XRR_CHECK_BIT(cbwDir, XRR_XRR_RDFRMHDR2_CBWDIR_BF_SHIFT);

#undef _XRR_CHECK_BIT
}

/**
* FUNCTION NAME: _XRR_OutputFrameHeaderBitFieldSanityCheck()
*
* @brief  - Contains runtime time checks to ensure the generated defines are in sync with the bitfields
*
* @return - void
*
* @note   - This uses a local helper macro to make life easier
*
*           The structure is defined with widths from the generated defines,
*           so here we are verifying that the positions are correct.
*
*           This check should really be a compile time assert, but GCC can't seem to figure it out at compile time
*           Even so when the assert won't trigger, GCC optimizes this function away
*/
static void _XRR_OutputFrameHeaderBitFieldSanityCheck(void)
{
    // define helper macro
#define _XRR_CHECK_BIT(_struct_, bitfield, offset) do                                           \
    {                                                                                           \
        /* Local structures for comparisons */                                                  \
        union XRR_FrameHeader defs;                                                             \
        union XRR_FrameHeader bits;                                                             \
                                                                                                \
        /* Clear structures */                                                                  \
        defs.raw = 0;                                                                           \
        bits.raw = 0;                                                                           \
                                                                                                \
        /* Set bits that should be identical in both */                                         \
        defs.raw = 1ULL << (offset);                                                            \
        bits._struct_.bitfield = 1;                                                             \
                                                                                                \
        /* Verify that the bits were identical */                                               \
        iassert_XRR_COMPONENT_1(defs.raw == bits.raw,                                           \
                XRR_BITFIELD_CHECK_FAILURE, __LINE__);                                          \
    } while (FALSE)

    // LSW
    _XRR_CHECK_BIT(generic, addr, XRR_XRR_SCHFRMHDR0_ADDR_BF_SHIFT);
    _XRR_CHECK_BIT(generic, endp, XRR_XRR_SCHFRMHDR0_ENDP_BF_SHIFT);
    _XRR_CHECK_BIT(generic, endpType, XRR_XRR_SCHFRMHDR0_ENDPTYPE_BF_SHIFT);
    _XRR_CHECK_BIT(generic, action, XRR_XRR_SCHFRMHDR0_ACTION_BF_SHIFT);
    _XRR_CHECK_BIT(generic, modifier, XRR_XRR_SCHFRMHDR0_MODIFIER_BF_SHIFT);
    _XRR_CHECK_BIT(generic, dataQid, XRR_XRR_SCHFRMHDR0_DATAQID_BF_SHIFT);
    _XRR_CHECK_BIT(generic, toggle, XRR_XRR_SCHFRMHDR0_TOGGLE_BF_SHIFT);

    // MSW
    _XRR_CHECK_BIT(generic, hubPort, XRR_XRR_SCHFRMHDR1_HUBPORT_BF_SHIFT + 32);
    _XRR_CHECK_BIT(generic, hubAddr, XRR_XRR_SCHFRMHDR1_HUBADDR_BF_SHIFT + 32);
    _XRR_CHECK_BIT(generic, splitSE, XRR_XRR_SCHFRMHDR1_SPLITSE_BF_SHIFT + 32);
    _XRR_CHECK_BIT(output, response, XRR_XRR_SCHFRMHDR1_RESPONSE_BF_SHIFT + 32);
    _XRR_CHECK_BIT(output, count, XRR_XRR_SCHFRMHDR1_COUNT_BF_SHIFT + 32);

#undef _XRR_CHECK_BIT
}

