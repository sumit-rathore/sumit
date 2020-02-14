///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  -  toplevel_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XADC_LOG_H
#define XADC_LOG_H

/***************************** Included Headers ******************************/
#include <project_components.h>
#include <ilog.h>

/************************ Defined Constants and Macros ***********************/
ILOG_CREATE(XADC_COMPONENT)
    ILOG_ENTRY(XADC_READ_TEMP,"XADC Read Temp %d.%.2d C\n")
    ILOG_ENTRY(XADC_SET_TEMP_WARNING_2,"FPGA warning temperature for -2 board set to %d C.\n")
    ILOG_ENTRY(XADC_SET_TEMP_WARNING_3,"FPGA warning temperature for -3 board set to %d C.\n") 
    ILOG_ENTRY(XADC_SET_TEMP_SHUTDOWN_2,"FPGA shutdown temperature for -2 board set to %d C.\n")
    ILOG_ENTRY(XADC_SET_TEMP_SHUTDOWN_3,"FPGA shutdown temperature for -3 board set to %d C.\n")
    ILOG_ENTRY(XADC_FPGA_THRESHOLD,"FPGA temperature threshold: Warning %d C, Shutdown %d C.\n")
    ILOG_ENTRY(XADC_READ_VCC_INT,"XADC Read VCC INT %d.%.3d V\n")
    ILOG_ENTRY(XADC_READ_VCC_AUX,"XADC Read VCC AUX %d.%.3d V\n")
    ILOG_ENTRY(XADC_READ_VCC_BRAM,"XADC Read VCC BRAM %d.%.3d V\n")
ILOG_END(XADC_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef XADC_LOG_H

