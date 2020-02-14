///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009, 2010, 2011, 2012, 2013
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
//!   @file  -  grg_pll.c
//
//!   @brief -  PLL driver functions as part of the GRG component
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "grg_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static uint8 getPllIrqNum(enum GRG_PllSelect);

/************************** Function Definitions *****************************/

static uint8 getPllIrqNum(enum GRG_PllSelect pll)
{
    // Interrupt flags are identical between ASIC and FPGA, so there is no
    // need to differentiate
    uint8 irq;

    // Sanity
    COMPILE_TIME_ASSERT(GRG_GRG_INTFLG_CLMPLLLOCK_BF_SHIFT == GRG_GRG_INTMSK_CLMPLLLOCK_BF_SHIFT);
    COMPILE_TIME_ASSERT(GRG_GRG_INTFLG_CRMPLLLOCK_BF_SHIFT == GRG_GRG_INTMSK_CRMPLLLOCK_BF_SHIFT);
    COMPILE_TIME_ASSERT(GRG_GRG_INTFLG_CTMPLLLOCK_BF_SHIFT == GRG_GRG_INTMSK_CTMPLLLOCK_BF_SHIFT);

    switch (pll)
    {
        case GRG_PllSelectClmPhyClk:
            irq = GRG_GRG_INTFLG_CLMPLLLOCK_BF_SHIFT;
            break;
        case GRG_PllSelectCtmPhyClk:
            irq = GRG_GRG_INTFLG_CTMPLLLOCK_BF_SHIFT;
            break;
        case GRG_PllSelectCrmPhyClk:
            irq = GRG_GRG_INTFLG_CRMPLLLOCK_BF_SHIFT;
            break;
        default:
            iassert_GRG_COMPONENT_1(FALSE, INVALID_PLL, pll);
            break;
    }

    return irq;
}

/**
* FUNCTION NAME: GRG_PllRegisterIrqHandler()
*
* @brief  -
*
* @return - void
*
* @note   -
*
*/
void GRG_PllRegisterIrqHandler
(
    enum GRG_PllSelect pll,
    void (*handler)(void)
)
{
    _GRG_IrqRegisterHandler(getPllIrqNum(pll), handler);
}

void GRG_PllEnableIrq(enum GRG_PllSelect pll)
{
    _GRG_IrqEnable(getPllIrqNum(pll));
}


void GRG_PllDisableIrq(enum GRG_PllSelect pll)
{
    _GRG_IrqDisable(getPllIrqNum(pll));
}


boolT GRG_PllIsLocked(enum GRG_PllSelect pll)
{
    const uint32 reg = GRG_PLL_PLLCONFIG_READ_REG(GRG_BASE_ADDR);
    boolT locked;

    // Note that the lock bits are identical between the ASIC and
    // the FPGA, so there is no need to differentiate
    switch (pll)
    {
        case GRG_PllSelectClmPhyClk:
            locked = GRG_PLL_PLLCONFIG_CLMLOCK_GET_BF(reg);
            break;
        case GRG_PllSelectCtmPhyClk:
            locked = GRG_PLL_PLLCONFIG_CTMLOCK_GET_BF(reg);
            break;
        case GRG_PllSelectCrmPhyClk:
            locked = GRG_PLL_PLLCONFIG_CRMLOCK_GET_BF(reg);
            break;
        default:
            iassert_GRG_COMPONENT_1(FALSE, INVALID_PLL, pll);
            break;
    }

    return locked;
}

void GRG_PllCtmSelectSource(enum GRG_PllSelect pllInputSel)
{
    uint32 reg = GRG_PLL_PLLCONFIG_READ_REG(GRG_BASE_ADDR);

    // Note that the clock select bits are identical between the ASIC
    // and the FPGA, so there is no need to differentiate
    switch (pllInputSel)
    {
        case GRG_PllSelectClmPhyClk:
            reg = GRG_PLL_PLLCONFIG_CTMCLKSEL_SET_BF(reg, 3);
            break;

        case GRG_PllSelectCtmRefClk0:
            reg = GRG_PLL_PLLCONFIG_CTMCLKSEL_SET_BF(reg, 2);
            break;

        case GRG_PllSelectCtmRefClk1:
            reg = GRG_PLL_PLLCONFIG_CTMCLKSEL_SET_BF(reg, 0); // NOTE: both 0 & 1 would work
            break;

        default:
            iassert_GRG_COMPONENT_1(FALSE, INVALID_PLL, pllInputSel);
            break;
    }
    GRG_PLL_PLLCONFIG_WRITE_REG(GRG_BASE_ADDR, reg);
}


void GRG_PllResetClmClock(void)
{
    uint32 reg = GRG_PLL_PLLCONFIG_READ_REG(GRG_BASE_ADDR);
    reg = GRG_PLL_PLLCONFIG_CLMRESET_SET_BF(reg, 1);
    GRG_PLL_PLLCONFIG_WRITE_REG(GRG_BASE_ADDR, reg);
}

void GRG_PllResetCrmClock(void)
{
    uint32 reg = GRG_PLL_PLLCONFIG_READ_REG(GRG_BASE_ADDR);
    reg = GRG_PLL_PLLCONFIG_CRMRESET_SET_BF(reg, 1);
    GRG_PLL_PLLCONFIG_WRITE_REG(GRG_BASE_ADDR, reg);
}
void GRG_PllResetCtmClock(void)
{
    uint32 reg = GRG_PLL_PLLCONFIG_READ_REG(GRG_BASE_ADDR);
    reg = GRG_PLL_PLLCONFIG_CTMRESET_SET_BF(reg, 1);
    GRG_PLL_PLLCONFIG_WRITE_REG(GRG_BASE_ADDR, reg);
}


void GRG_PllActivateClmClock(void)
{
    uint32 reg = GRG_PLL_PLLCONFIG_READ_REG(GRG_BASE_ADDR);
    reg = GRG_PLL_PLLCONFIG_CLMRESET_SET_BF(reg, 0);
    GRG_PLL_PLLCONFIG_WRITE_REG(GRG_BASE_ADDR, reg);
}

void GRG_PllActivateCrmClock(void)
{
    uint32 reg = GRG_PLL_PLLCONFIG_READ_REG(GRG_BASE_ADDR);
    reg = GRG_PLL_PLLCONFIG_CRMRESET_SET_BF(reg, 0);
    GRG_PLL_PLLCONFIG_WRITE_REG(GRG_BASE_ADDR, reg);
}
void GRG_PllActivateCtmClock(void)
{
    uint32 reg = GRG_PLL_PLLCONFIG_READ_REG(GRG_BASE_ADDR);
    reg = GRG_PLL_PLLCONFIG_CTMRESET_SET_BF(reg, 0);
    GRG_PLL_PLLCONFIG_WRITE_REG(GRG_BASE_ADDR, reg);
}


void _GRG_assertHookPll(void)
{
#if 0 // TODO
    const GRG_PllMeasurementT clmClock = GRG_PllMeasureFreq(GRG_PllSelectClm);
    const GRG_PllMeasurementT ctmClock = GRG_PllMeasureFreq(GRG_PllSelectCtm);
    const GRG_PllMeasurementT crmClock = GRG_PllMeasureFreq(GRG_PllSelectCrm);
#endif

    ilog_GRG_COMPONENT_2(ILOG_FATAL_ERROR,
            GRG_SPECTAREG_READ,
            GRG_BASE_ADDR + GRG_PLL_PLLCONFIG_REG_OFFSET,
            GRG_PLL_PLLCONFIG_READ_REG(GRG_BASE_ADDR));
    ilog_GRG_COMPONENT_2(ILOG_FATAL_ERROR,
            GRG_SPECTAREG_READ,
            GRG_BASE_ADDR + GRG_PLL_PLLCONFIG2_REG_OFFSET,
            GRG_PLL_PLLCONFIG2_READ_REG(GRG_BASE_ADDR));
    ilog_GRG_COMPONENT_2(ILOG_FATAL_ERROR,
            GRG_SPECTAREG_READ,
            GRG_BASE_ADDR + GRG_PLL_PLLCONFIG3_REG_OFFSET,
            GRG_PLL_PLLCONFIG3_READ_REG(GRG_BASE_ADDR));

#if 0 // TODO
    ilog_GRG_COMPONENT_1(ILOG_FATAL_ERROR, FREQ_MEASURE_CLM, clmClock);
    ilog_GRG_COMPONENT_1(ILOG_FATAL_ERROR, FREQ_MEASURE_CTM, ctmClock);
    ilog_GRG_COMPONENT_1(ILOG_FATAL_ERROR, FREQ_MEASURE_CRM, crmClock);
#endif
}


uint32 GRG_PllReadConfig(void)
{
    uint32 value = GRG_PLL_PLLCONFIG_READ_REG(GRG_BASE_ADDR);
    return value;
}

uint32 GRG_PllReadConfig2(void)
{
    uint32 value = GRG_PLL_PLLCONFIG2_READ_REG(GRG_BASE_ADDR);
    ilog_GRG_COMPONENT_1(ILOG_DEBUG, PLL2_READ, value);
    return value;
}


uint32 GRG_PllReadConfig3(void)
{
    return GRG_PLL_PLLCONFIG3_READ_REG(GRG_BASE_ADDR);
}

uint32 GRG_PllReadConfig4(void)
{
    return GRG_PLL_PLLCONFIG4_READ_REG(GRG_BASE_ADDR);
}

void GRG_PllSetConfig(uint32 value)
{
    GRG_PLL_PLLCONFIG_WRITE_REG(GRG_BASE_ADDR, value);
}

void GRG_PllSetConfig2(uint32 value)
{
    ilog_GRG_COMPONENT_1(ILOG_DEBUG, PLL2_WRITE, value);
    GRG_PLL_PLLCONFIG2_WRITE_REG(GRG_BASE_ADDR, value);
}

void GRG_PllSetConfig3(uint32 value)
{
    GRG_PLL_PLLCONFIG3_WRITE_REG(GRG_BASE_ADDR, value);
}

void GRG_PllSetConfig4(uint32 value)
{
    GRG_PLL_PLLCONFIG4_WRITE_REG(GRG_BASE_ADDR, value);
}



