///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2011, 2012, 2013
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
//!   @file  -  pll.h
//
//!   @brief -  External header file for the PLL component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef PLL_H
#define PLL_H

/***************************** Included Headers ******************************/
#include <itypes.h> // for boolT

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/

void PLL_Init(void);

// Cfg the PLL source
void PLL_CfgMII(boolT isDDR);
void PLL_CfgGMII(boolT isRGMII);
void PLL_CfgTbi(boolT isRTBI); //TODO: hardcoded 125MHz
void PLL_CfgClei(void);//TODO: hardcoded 125MHz

// link clocks, crm and ctm
void PLL_crmRegisterIrqHandler(void (*handler)(void));
void PLL_ctmRegisterIrqHandler(void (*handler)(void));
boolT PLL_crmCheckLockAndEnableIrq(void);
boolT PLL_ctmCheckLockAndEnableIrq(void);
void PLL_DisableLinkClocks(void); // ctm & crm

#endif // PLL_H

