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
//!   @file  - netcfg_cmd_impl.c
//
//!   @brief - Contains the implementation of icmd functions for the network
//             based configuration component.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "netcfg_cmd.h"
#include "netcfg_log.h"


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: NETCFG_x()
*
* @brief  - .
*
* @return - void
*/
void NET_icmdAllocateNetworkBuffer(void)
{
    if(!icmdNetBuffer)
    {
        icmdNetBuffer = _NET_allocateBuffer();
        icmdNetBufferSize = 0;
        ilog_NET_COMPONENT_0(ILOG_USER_LOG, NET_ICMD_NETWORK_BUFFER_ALLOCATED);
    }
    else
    {
        ilog_NET_COMPONENT_0(ILOG_USER_LOG, NET_ICMD_NETWORK_BUFFER_PREV_ALLOC);
    }
}
