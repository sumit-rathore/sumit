///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013
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
//!   @file  -  bgrg_cmdresp.h
//
//!   @brief -  This file contains the icmdresp information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef I2C_CMDRESP_H
#define I2C_CMDRESP_H

/***************************** Included Headers ******************************/
#include <icmdresp.h>

/************************ Defined Constants and Macros ***********************/

// macro use
//ICMDRESP_START( <component name from project_components.h> )
//  ICMDRESP_ENTRY( <name>, <icmd name>, <ilog name>, <ilog arg number>)
//  ICMDRESP_ENTRY( <name>, <icmd name>, <ilog name>, <ilog arg number>)
//  ICMDRESP_ENTRY( <name>, <icmd name>, <ilog name>, <ilog arg number>)
//ICMDRESP_END( <component name from project_components.h> )


ICMDRESP_START(I2C_COMPONENT)
ICMDRESP_END(I2C_COMPONENT)

#endif // I2C_CMDRESP_H
