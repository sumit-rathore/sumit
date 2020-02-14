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
//!   @file  - netcfg_cmd.h
//
//!   @brief - This file contains the icmd information for this component
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NETCFG_CMD_H
#define NETCFG_CMD_H

/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

ICMD_FUNCTIONS_CREATE(NETCFG_COMPONENT)
    //ICMD_FUNCTIONS_ENTRY(NETCFG_f, "Command description", commandReturnType)
ICMD_FUNCTIONS_END(NETCFG_COMPONENT)

#endif // NETCFG_CMD_H
