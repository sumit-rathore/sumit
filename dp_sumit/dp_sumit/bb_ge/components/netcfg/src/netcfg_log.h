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
//!   @file  - netcfg_log.h
//
//!   @brief - The log messages for the network based device configuration
//             component.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NETCFG_LOG_H
#define NETCFG_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(NETCFG_COMPONENT)
    ILOG_ENTRY(NETCFG_COULD_NOT_BIND_TO_PORT, "Could not bind the network based configuration listener to port %d.\n")
    ILOG_ENTRY(DEPRECATED_NETCFG_FLASH_VAR_WRONG_SIZE, "The flash variable read was %d bytes instead of %d.\n")
    ILOG_ENTRY(NETCFG_READ_INVALID_VPORT_PAIRING, "Tried to read the MAC address pairing for the invalid vport %d.\n")
    ILOG_ENTRY(DEPRECATED_NETCFG_FAILED_TO_WRITE_FLASH, "Failed to write flash variable %d.\n")
    ILOG_ENTRY(DEPRECATED_NETCFG_PRODUCT_VARIANT_NOT_DEFINED, "Product variant is not set in persistent data storage\n")
    ILOG_ENTRY(XUSBCFG_ICRON, "XUSB Configuration Protocol is Icron\n")
    ILOG_ENTRY(XUSBCFG_CRESTRON, "XUSB Configuration Protocol is Crestron\n")
    ILOG_ENTRY(NETCFG_RECEIVED_REQUEST, "Received netcfg command %d\n")
    ILOG_ENTRY(UDPpacket,"UDP transmission packet handled good and sender IP = %d\n")
   ILOG_END(NETCFG_COMPONENT, ILOG_DEBUG)

#endif // #ifndef NETCFG_LOG_H
