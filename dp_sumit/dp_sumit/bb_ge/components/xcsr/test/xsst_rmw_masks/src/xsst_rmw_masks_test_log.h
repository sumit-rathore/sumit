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
//!   @file  -  xsst_rmw_masks_test_log.h
//
//!   @brief -  This file contains the logging definitions for this module
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XSST_RMW_MASKS_TEST_LOG_H
#define XSST_RMW_MASKS_TEST_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(TEST_HARNESS_COMPONENT)
    ILOG_ENTRY(GOING_TO_DO_A_FULL_RMW_MASK_IS, "going to do a full RMW, mask is 0x%x 0x%x\n")
    ILOG_ENTRY(GOING_TO_DO_A_FULL_RMW_VAL_IS, "going to do a full RMW, val is 0x%x 0x%x\n")
    ILOG_ENTRY(DID_A_FULL_RMW_RETVAL_IS, "did a full rmw, got back 0x%x 0x%x\n")
    ILOG_ENTRY(GOING_TO_DO_A_MSW_RMW_MASK_IS, "going to do a msw RMW, val is 0x%x 0x%x\n")
    ILOG_ENTRY(GOING_TO_DO_A_MSW_RMW_VAL_IS, "going to do a msw RMW, val is 0x%x 0x%x\n")
    ILOG_ENTRY(DID_A_MSW_RMW_RETVAL_IS, "did a msw rmw, got back 0x%x 0x%x\n")
ILOG_END(TEST_HARNESS_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef XSST_RMW_MASKS_TEST_LOG_H


