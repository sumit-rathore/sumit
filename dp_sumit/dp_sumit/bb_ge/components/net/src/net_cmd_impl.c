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
//!   @file  - net_cmd_impl.c
//
//!   @brief - Contains the implementation of icmd functions for the networking
//             component.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "net_cmd.h"
#include "net_base_loc.h"
#include "net_log.h"
#include "net_ipv4_loc.h"
#include <storage_Data.h>


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: NET_icmdSetIPv4Configuration()
*
* @brief  - Sets the IP address, netmask and default gateway of this device.
*
* @return - void
*
* @note   - This function makes use of the corresponding "view" function for
*           convenience.
*/
void NET_icmdSetIPv4Configuration(
    uint32 ipAddress, uint32 subnetMask, uint32 defaultGateway)
{
    NET_ipv4SetIPAddress(ipAddress);
    NET_ipv4SetSubnetMask(subnetMask);
    NET_ipv4SetDefaultGateway(defaultGateway);
    NET_icmdViewIPv4Configuration();
}

/**
* FUNCTION NAME: NET_icmdViewIPv4Configuration()
*
* @brief  - Prints the IP address, netmask and default gateway of this device.
*
* @return - void
*/
void NET_icmdViewIPv4Configuration(void)
{
    ilog_NET_COMPONENT_3(
        ILOG_USER_LOG,
        NET_ICMD_SHOW_IP_CONFIGURATION,
        NET_ipv4GetIPAddress(),
        NET_ipv4GetSubnetMask(),
        NET_ipv4GetDefaultGateway());
}
