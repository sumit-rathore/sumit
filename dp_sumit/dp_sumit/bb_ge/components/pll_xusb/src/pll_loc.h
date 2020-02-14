///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2014
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
//!   @file  -  pll_loc.h
//
//!   @brief -  Internal header file for the PLL component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef PLL_LOC_H_
#define PLL_LOC_H_

/***************************** Included Headers ******************************/
#include <itypes.h> // for boolT

/************************ Defined Constants and Macros ***********************/
#define PLL_MAX_ACTIVE_TIME_W_NO_LOCK 100000 // 100 milliseconds

// Common to all platforms
#define _PLL_CFG2_EXT_CLK_OFFSET 28
#define _PLL_CFG2_EXT_CLK_MASK  0x10000000

// For the ASIC PLL, output freq = N/M * input freq - we need
// to double input freq due to hardware phase shift needs
#define _PLL_ASIC_NS_VALUE  (4)
#define _PLL_ASIC_MS_VALUE  (2)

// The CLM PLL uses different values for NS and MS
#define _PLL_ASIC_CLM_NS_VALUE (25)
#define _PLL_ASIC_CLM_MS_VALUE (6)

// For CLM PLL setup using the ASIC - set the post divider to divide by 2
#define _PLL_CLM_POST_DIVIDER_VALUE 0x01

// TODO: Currently Spartan only bits, intended for other platforms later
#define _PLL_CFG2_CLM_DIV_OFFSET    20
#define _PLL_CFG2_CLM_DIV_MASK      0x0FF00000   // NOTE: program desired (value - 1) // TODO: make default 16-1 == 15
#define _PLL_CFG2_CLM_MUL_OFFSET    12
#define _PLL_CFG2_CLM_MUL_MASK      0x000FF000   // NOTE: program desired (value - 1) // TODO: make default 20-1 == 19
#define _PLL_CFG2_CLM_DONE_OFFSET   9
#define _PLL_CFG2_CLM_DONE_MASK     0x00000200
#define _PLL_CFG2_CLM_GO_OFFSET     8
#define _PLL_CFG2_CLM_GO_MASK       0x00000100
#define _PLL_CFG2_CLM_PHASE_OFFSET  4
#define _PLL_CFG2_CLM_PHASE_MASK    0x00000030

// Common to all non-ASIC platforms
#define _PLL_CFG2_CXM_DONE_OFFSET   3
#define _PLL_CFG2_CXM_DONE_MASK     0x00000008
#define _PLL_CFG2_CXM_GO_OFFSET     2
#define _PLL_CFG2_CXM_GO_MASK       0x00000004
#define _PLL_CFG2_CXM_RANGE_OFFSET  1
#define _PLL_CFG2_CXM_RANGE_MASK    0x00000002
#define _PLL_CFG2_CXM_SEL_OFFSET    0
#define _PLL_CFG2_CXM_SEL_MASK      0x00000001

// ASIC only bits, not used by any FPGA based platform

// Overall PLL configuration - GRG_PllConfig fields
#define _PLL_CFG_CTM_RESET_OFFSET       20
#define _PLL_CFG_CTM_RESET_MASK         0x00100000
#define _PLL_CFG_CTM_CLOCK_SEL_OFFSET   18
#define _PLL_CFG_CTM_CLOCK_SEL_MASK     0x000C0000
#define _PLL_CFG_CTM_LOCK_EVENT_OFFSET  17
#define _PLL_CFG_CTM_LOCK_EVENT_MASK    0x00020000
#define _PLL_CFG_CTM_LOCK_OFFSET        16
#define _PLL_CFG_CTM_LOCK_MASK          0x00010000
#define _PLL_CFG_CRM_RESET_OFFSET       12
#define _PLL_CFG_CRM_RESET_MASK         0x00001000
#define _PLL_CFG_CRM_LOCK_EVENT_OFFSET  9
#define _PLL_CFG_CRM_LOCK_EVENT_MASK    0x00000200
#define _PLL_CFG_CRM_LOCK_OFFSET        8
#define _PLL_CFG_CRM_LOCK_MASK          0x00000100
#define _PLL_CFG_CLM_RESET_OFFSET       4
#define _PLL_CFG_CLM_RESET_MASK         0x00000010
#define _PLL_CFG_CLM_LOCK_EVENT_OFFSET  1
#define _PLL_CFG_CLM_LOCK_EVENT_MASK    0x00000002
#define _PLL_CFG_CLM_LOCK_OFFSET        0
#define _PLL_CFG_CLM_LOCK_MASK          0x00000001

// CTM fields - GRG_PllConfig2 register
#define _PLL_CFG2_CLM_TX_PHASE_SEL_OFFSET   20
#define _PLL_CFG2_CLM_TX_PHASE_SEL_MASK     0x00300000
#define _PLL_CFG2_CTM_90_270_SEL_OFFSET     19
#define _PLL_CFG2_CTM_90_270_SEL_MASK       0x00080000
#define _PLL_CFG2_CTM_0_180_SEL_OFFSET      18
#define _PLL_CFG2_CTM_0_180_SEL_MASK        0x00040000
#define _PLL_CFG2_CTM_90_RESET_SEL_OFFSET   17
#define _PLL_CFG2_CTM_90_RESET_SEL_MASK     0x00020000
#define _PLL_CFG2_CTM_0_RESET_SEL_OFFSET    16
#define _PLL_CFG2_CTM_0_RESET_SEL_MASK      0x00010000
#define _PLL_CFG2_CTM_LOCK_SEL_OFFSET       15
#define _PLL_CFG2_CTM_LOCK_SEL_MASK         0x00008000
#define _PLL_CFG2_CTM_BW_SW_OFFSET          14
#define _PLL_CFG2_CTM_BW_SW_MASK            0x00004000
#define _PLL_CFG2_CTM_RANGE_OFFSET          12
#define _PLL_CFG2_CTM_RANGE_MASK            0x00003000
#define _PLL_CFG2_CTM_NS_OFFSET             6
#define _PLL_CFG2_CTM_NS_MASK               0x00000FC0
#define _PLL_CFG2_CTM_MS_OFFSET             0
#define _PLL_CFG2_CTM_MS_MASK               0x0000003F

// CRM fields - GRG_PllConfig3 register
#define _PLL_CFG3_CRM_REF_CLOCK_SEL_OFFSET  22
#define _PLL_CFG3_CRM_REF_CLOCK_SEL_MASK    0x00400000
#define _PLL_CFG3_CRM_NEG_EDGE_SEL_OFFSET   21
#define _PLL_CFG3_CRM_NEG_EDGE_SEL_MASK     0x00200000
#define _PLL_CFG3_CRM_POS_EDGE_SEL_OFFSET   20
#define _PLL_CFG3_CRM_POS_EDGE_SEL_MASK     0x00100000
#define _PLL_CFG3_CRM_90_270_SEL_OFFSET     19
#define _PLL_CFG3_CRM_90_270_SEL_MASK       0x00080000
#define _PLL_CFG3_CRM_0_180_SEL_OFFSET      18
#define _PLL_CFG3_CRM_0_180_SEL_MASK        0x00040000
#define _PLL_CFG3_CRM_90_RESET_SEL_OFFSET   17
#define _PLL_CFG3_CRM_90_RESET_SEL_MASK     0x00020000
#define _PLL_CFG3_CRM_0_RESET_SEL_OFFSET    16
#define _PLL_CFG3_CRM_0_RESET_SEL_MASK      0x00010000
#define _PLL_CFG3_CRM_LOCK_SEL_OFFSET       15
#define _PLL_CFG3_CRM_LOCK_SEL_MASK         0x00008000
#define _PLL_CFG3_CRM_BW_SW_OFFSET          14
#define _PLL_CFG3_CRM_BW_SW_MASK            0x00004000
#define _PLL_CFG3_CRM_RANGE_OFFSET          12
#define _PLL_CFG3_CRM_RANGE_MASK            0x00003000
#define _PLL_CFG3_CRM_NS_OFFSET             6
#define _PLL_CFG3_CRM_NS_MASK               0x00000FC0
#define _PLL_CFG3_CRM_MS_OFFSET             0
#define _PLL_CFG3_CRM_MS_MASK               0x0000003F

// CLM fields - GRG_PllConfig4 register
#define _PLL_CFG4_CLM_REF1_OUT_OFFSET       24
#define _PLL_CFG4_CLM_REF1_OUT_MASK         0x07000000
#define _PLL_CFG4_CLM_REF0_OUT_OFFSET       20
#define _PLL_CFG4_CLM_REF0_OUT_MASK         0x00700000
#define _PLL_CFG4_CLM_POST_DIVIDER_OFFSET   16
#define _PLL_CFG4_CLM_POST_DIVIDER_MASK     0x00030000
#define _PLL_CFG4_CLM_LOCK_SEL_OFFSET       15
#define _PLL_CFG4_CLM_LOCK_SEL_MASK         0x00008000
#define _PLL_CFG4_CLM_BW_SW_OFFSET          14
#define _PLL_CFG4_CLM_BW_SW_MASK            0x00004000
#define _PLL_CFG4_CLM_RANGE_OFFSET          12
#define _PLL_CFG4_CLM_RANGE_MASK            0x00003000
#define _PLL_CFG4_CLM_NS_OFFSET             6
#define _PLL_CFG4_CLM_NS_MASK               0x00000FC0
#define _PLL_CFG4_CLM_MS_OFFSET             0
#define _PLL_CFG4_CLM_MS_MASK               0x0000003F

// Mux definitions for the generated output clocks
#define _PLL1_SYSTEM_CLOCK_24MHZ (1)

/******************************** Data Types *********************************/

enum _PLL_RangeFpga {
    // NOTE: values match hardware bits
    _PLL_RANGE_FPGA_25MHZ_TO_60MHZ = 0,
    _PLL_RANGE_FPGA_60MHZ_TO_125MHZ = 1
};

enum _PLL_RangeAsic {
    // NOTE: values match hardware bits
    _PLL_RANGE_ASIC_25MHZ_TO_50MHZ = 0,
    _PLL_RANGE_ASIC_50MHZ_TO_100MHZ = 1,
    _PLL_RANGE_ASIC_100MHZ_TO_200MHZ = 2,
    _PLL_RANGE_ASIC_200MHZ_TO_400MHZ = 3
};

// These match the required hardware bits
enum _PLL_AsicInputSource
{
    _PLL_CTM_IREF_CLK1 = 0,
    _PLL_CTM_24MHz_CLK = 1,
    _PLL_CTM_IREF_CLK0 = 2,
    _PLL_CTM_CLM_CLK = 3
};

enum _PLL_FpgaPhaseSelect
{
    // NOTE: values match hardware bits
    _PLL_FPGA_0_DEGREES = 0,
    _PLL_FPGA_180_DEGREES = 1,
    _PLL_FPGA_270_DEGREES = 2,
    _PLL_FPGA_90_DEGREES = 3
};

// These match the required hardware bits
enum _PLL_AsicPhaseSelect
{
    _PLL_ASIC_NO_OUTPUT = 0,
    _PLL_ASIC_0_DEGREES = 1,
    _PLL_ASIC_90_DEGREES = 2,
    _PLL_ASIC_180_DEGREES = 3
};

// These match the hardware bits in the PllConfig4 register
enum _PLL_Ref0ClockOut
{
    _REF0_NO_CLOCK = 0,
    _REF0_CLM_POST_DIV_CLOCK = 1,
    _REF0_CTM_OUTPUT_CLOCK = 2,
    _REF0_CRM_OUTPUT_CLOCK = 3,
    _REF0_CLM_OUTPUT_CLOCK = 4,
    _REF0_CRM_OUT_DIV_CLOCK = 5,
    _REF0_CTM_LOCK_SIGNAL = 6,
    _REF0_CRM_LOCK_SIGNAL = 7
};

// These match the hardware bits in the PllConfig4 register
enum _PLL_Ref1ClockOut
{
    _REF1_NO_CLOCK = 0,
    _REF1_SYS_CLOCK_24MHz = 1,
    _REF1_SYS_CLOCK_12MHz = 2,
    _REF1_USB_PHY_CLOCK = 3,
    _REF1_CLM_POST_DIV_CLOCK = 4,
    _REF1_CTM_INPUT_CLOCK = 5,
    _REF1_CTM_OUT_DIV_CLOCK = 6,
    _REF1_CRM_OUT_DIV_CLOCK = 7
};


struct _PLL_state
{
    void (*lossHandler)(void);
    LEON_TimerValueT activateTime;
};

/*********************************** API *************************************/



#endif /* PLL_LOC_H_ */
