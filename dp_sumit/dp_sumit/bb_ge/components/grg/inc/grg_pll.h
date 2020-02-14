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
//!   @file  -  grg_pll.h
//
//!   @brief -  PLL driver functions as part of the GRG component
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GRG_PLL_H
#define GRG_PLL_H

/***************************** Included Headers ******************************/
#include <itypes.h>

/************************ Defined Constants and Macros ***********************/
#define GRG_PLL_MEASUREMENT_FAIL ((sint16)(-1))

/******************************** Data Types *********************************/
enum GRG_PllSelect
{
    // NOTE: Values match HW for PLL Measurement
    GRG_PllSelectCrmPhyClk = 0, // After PLL, lock needs to be estabilished before freq. test
    GRG_PllSelectCtmPhyClk = 1, // After PLL, lock needs to be estabilished before freq. test
    GRG_PllSelectCrmRefClk = 2, // Reference to PLL, IE input clock
    GRG_PllSelectCtmRefClk = 3, // Reference to PLL, IE input clock (output of mux clock selection)
    GRG_PllSelectClmClk1   = 4, // Input pin of FPGA

    // TODO: Should these be broken up into 2 different enums???
    // The following selections can't be measured, and values are arbitrary
    GRG_PllSelectClmPhyClk,     // Output of CLM PLL, which is derived from system clock,
                                // input to mux selector for CTM PLL
    GRG_PllSelectCtmRefClk0,    // Input pin I_CLM_TX_CLK0.  Used by GMII/TBI transmit reference clock for CTM PLL 
                                // input to mux selector for CTM PLL
    GRG_PllSelectCtmRefClk1,    // Input pin I_CLM_TX_CLK1.  Used by MII transmit reference clock for CTM PLL
};

typedef sint16 GRG_PllMeasurementT;

/*********************************** API *************************************/

void GRG_PllRegisterIrqHandler(enum GRG_PllSelect, void (*handler)(void));
void GRG_PllEnableIrq(enum GRG_PllSelect);
void GRG_PllDisableIrq(enum GRG_PllSelect);

boolT GRG_PllIsLocked(enum GRG_PllSelect);

// (De-)Activating clocks
void GRG_PllCtmSelectSource(enum GRG_PllSelect);
void GRG_PllResetClmClock(void);
void GRG_PllResetCrmClock(void);
void GRG_PllResetCtmClock(void);
void GRG_PllActivateClmClock(void);
void GRG_PllActivateCrmClock(void);
void GRG_PllActivateCtmClock(void);

// Read/Write implementation specific registers
// IE these register behaviour changes on Spartan/Virtex/Asic
uint32 GRG_PllReadConfig(void);
uint32 GRG_PllReadConfig2(void);
uint32 GRG_PllReadConfig3(void);
uint32 GRG_PllReadConfig4(void);
void GRG_PllSetConfig(uint32);
void GRG_PllSetConfig2(uint32);
void GRG_PllSetConfig3(uint32);
void GRG_PllSetConfig4(uint32);

// Operations for measuring clocks
GRG_PllMeasurementT GRG_PllMeasureFreq(enum GRG_PllSelect pllSelection);
uint16 GRG_MeasurePllInMHz(enum GRG_PllSelect pllSelection);
boolT GRG_PllMeasureIs125Mhz(GRG_PllMeasurementT);
boolT GRG_PllMeasureIs25Mhz(GRG_PllMeasurementT);

#endif // GRG_PLL_H

