///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -  pll_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef PLL_LOG_H
#define PLL_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(PLL_COMPONENT)
    ILOG_ENTRY(TBI_125, "TBI 125MHz\n")
    ILOG_ENTRY(GMII_125, "GMII 125MHz\n")
    ILOG_ENTRY(MII_25, "MII 25MHz\n")
    ILOG_ENTRY(RGMII_25, "RGMII 25MHz\n")
    ILOG_ENTRY(RGMII_125, "RGMII 125MHz\n")
    ILOG_ENTRY(NO_IRQ_HANDLER, "No IRQ handler at line %d\n")
    ILOG_ENTRY(RMII_50, "RMII 50MHz\n")
    ILOG_ENTRY(CLEI_LOG, "CLEI\n")
    ILOG_ENTRY(NO_LOCK_AT_LINE, "Unable to lock PLL %d at line %d\n")
    ILOG_ENTRY(EXTERNAL_CLK_ENABLED, "External clock enabled\n")
    ILOG_ENTRY(UNHANDLED_CLK_RANGE, "Unhandled ASIC clock range %d\n")
    ILOG_ENTRY(CLEI_UNSUPPORTED_ON_KINTEX, "The CLEI link type is unsupported on the Kintex platform\n")
    ILOG_ENTRY(RGMII25_UNSUPPORTED_ON_KINTEX, "The RGMII25 link type is unsupported on the Kintex platform\n")
    ILOG_ENTRY(CLEI_CTM_INPUT_CLK, "The CLEI operating clock is %d\n")
ILOG_END(PLL_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef PLL_LOG_H

