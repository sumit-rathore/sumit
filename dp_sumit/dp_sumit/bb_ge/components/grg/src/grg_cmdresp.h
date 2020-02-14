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
//!   @file  -  grg_cmdresp.h
//
//!   @brief -  This file contains the icmdresp information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef GRG_CMDRESP_H
#define GRG_CMDRESP_H

/***************************** Included Headers ******************************/
#include <icmdresp.h>

/************************ Defined Constants and Macros ***********************/

// macro use
//ICMDRESP_START( <component name from project_components.h> )
//  ICMDRESP_ENTRY( <name>, <icmd name>, <ilog name>, <ilog arg number>)
//  ICMDRESP_ENTRY( <name>, <icmd name>, <ilog name>, <(ilog arg number, ilog arg number 2)>)
//  ICMDRESP_ENTRY( <name>, <icmd name>, <ilog name>, <(ilog arg number, ilog arg 2, ilog arg 3)>)
//ICMDRESP_END( <component name from project_components.h> )


ICMDRESP_START(GRG_COMPONENT)
    ICMDRESP_ENTRY(divide16bit, icmdDivide, INT16_DIVIDE, (0,1))
    ICMDRESP_ENTRY(multiply16bit, icmdMultiply, INT16_MULTIPLY, 0)
ICMDRESP_END(GRG_COMPONENT)

#endif // GRG_CMDRESP_H
