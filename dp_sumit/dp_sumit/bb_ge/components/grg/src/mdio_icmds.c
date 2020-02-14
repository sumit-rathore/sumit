///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
//!   @file  -  mdio_icmds.c
//
//!   @brief -  This file contains the functions for icmd
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <grg_mdio.h>
#include "grg_loc.h"


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void icmdMdioReadHelper(uint16 data);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: icmdMdioWrite()
*
* @brief  - write a value to the requested Mdio attached device's register
*
* @return - void
*
* @note   -
*
*/
void icmdMdioWrite
(
    uint8 device,  // The address of the MDIO connected device to write to
    uint8 address, // The address of the register on the device to write to
    uint16 data    // The data to write
)
{
    ilog_GRG_COMPONENT_3(ILOG_USER_LOG, MDIO_ICMD_WRITE, device, address, data);
    GRG_MdioWriteASync(device, address, data, NULL);
}


/**
* FUNCTION NAME: icmdMdioRead()
*
* @brief  - read a value from the requested Mdio attached device's register
*
* @return - void
*
* @note   -
*
*/
void icmdMdioRead
(
    uint8 device,  // Address of MDIO attached device
    uint8 address  // Register address to read from
)
{
    ilog_GRG_COMPONENT_2(ILOG_USER_LOG, MDIO_ICMD_READ_START, device, address);
    GRG_MdioReadASync(device, address, &icmdMdioReadHelper);
}
static void icmdMdioReadHelper(uint16 data)
{
    ilog_GRG_COMPONENT_1(ILOG_USER_LOG, MDIO_ICMD_READ_DONE, data);
}

/**
* FUNCTION NAME: icmdMdioWriteSync()
*
* @brief  - write a value to the requested Mdio attached device's register
*
* @return - void
*
* @note   -
*
*/
void icmdMdioWriteSync
(
    uint8 device,  // The address of the MDIO connected device to write to
    uint8 address, // The address of the register on the device to write to
    uint16 data    // The data to write
)
{
    GRG_MdioWriteSync(device, address, data);
    ilog_GRG_COMPONENT_3(ILOG_USER_LOG, MDIO_SYNC_WRITE, data, device, address);
}


/**
* FUNCTION NAME: icmdMdioReadSync()
*
* @brief  - read a value from the requested Mdio attached device's register
*
* @return - void
*
* @note   -
*
*/
void icmdMdioReadSync
(
    uint8 device,  // Address of MDIO attached device
    uint8 address  // Register address to read from
)
{
    uint16 readVal = GRG_MdioReadSync(device, address);
    ilog_GRG_COMPONENT_3(ILOG_USER_LOG, MDIO_SYNC_READ, readVal, device, address);
}
