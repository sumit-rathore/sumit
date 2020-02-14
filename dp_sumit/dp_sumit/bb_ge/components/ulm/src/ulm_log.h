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
//!   @file  -  ulm_log.h
//
//!   @brief -  The ULM driver logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef ULM_LOG_H
#define ULM_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(ULM_COMPONENT)
    ILOG_ENTRY(ULM_SPEED_INVALID_ERROR_LOG, "Speed is invalid\n")
    ILOG_ENTRY(ULM_INVALID_REV, "Invalid ULM revision\n")
    ILOG_ENTRY(ULM_INVALID_CVS_REV, "Invalid ULM CVS revision\n")
    ILOG_ENTRY(ULM_INIT_LEX, "LEX - ULM Init\n")
    ILOG_ENTRY(ULM_INIT_REX, "REX - ULM Init\n")
    ILOG_ENTRY(ULM_INTERRUPT, "ULM interrupt triggered by 0x%x\n")
    ILOG_ENTRY(CTRL_BITS_LEFT_SET, "Ctrl bits were left set. ctrlReg is 0x%x\n")
    ILOG_ENTRY(WRITING_CTRL_REG, "Writing control reg as 0x%x\n")
    ILOG_ENTRY(ULM_SPECTAREG_READ, "Read ULM Register: 0x%x, Value: 0x%x\n")
    ILOG_ENTRY(ULM_DISCON_USBPORT_DC, "ULM_DISCON - ULM OFF\n")
    ILOG_ENTRY(ULM_DISCON_USBPORT_CON, "ULM_DISCON - ULM ON\n")
ILOG_END(ULM_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef ULM_LOG_H

