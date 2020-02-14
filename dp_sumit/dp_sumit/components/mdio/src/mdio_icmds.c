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
#include "mdio_loc.h"


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static void icmdMdioReadHelper(uint16_t data);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: icmdMdioWrite()
*
* @brief  - write a value to the requested Mdio attached device's register
*
* @return - void
*/
void icmdMdioWrite
(
    uint8_t device,  // The address of the MDIO connected device to write to
    uint8_t address, // The address of the register on the device to write to
    uint16_t data,    // The data to write
    uint8_t muxPort
) __attribute__((section(".atext")));
void icmdMdioWrite
(
    uint8_t device,  // The address of the MDIO connected device to write to
    uint8_t address, // The address of the register on the device to write to
    uint16_t data,    // The data to write
    uint8_t muxPort
)
{
    ilog_MDIO_COMPONENT_3(ILOG_USER_LOG, MDIO_ICMD_WRITE, device, address, data);
    MdioWriteASync(device, address, data, NULL, muxPort);
}


void icmdMdioIndirectWrite
(
    uint8_t device, // Phy address, 5 bits
    uint8_t devType, // 5 bits: PMD/PMA, WIS, PCS, PHY XS, DTE XS
    uint16_t address,
    uint16_t data,
    uint8_t muxPort
) __attribute__((section(".atext")));

void icmdMdioIndirectWrite
(
    uint8_t device, // Phy address, 5 bits
    uint8_t devType, // 5 bits: PMD/PMA, WIS, PCS, PHY XS, DTE XS
    uint16_t address,
    uint16_t data,
    uint8_t muxPort
)
{
    ilog_MDIO_COMPONENT_2(ILOG_USER_LOG, MDIO_INDIRECT_ICMD_WRITE, device, devType);
    ilog_MDIO_COMPONENT_2(ILOG_USER_LOG, MDIO_INDIRECT_ICMD_WRITE2, address, data);
    MdioIndirectWriteASync(device, devType, address, data, NULL, muxPort);
}


/**
* FUNCTION NAME: icmdMdioRead()
*
* @brief  - read a value from the requested Mdio attached device's register
*
* @return - void
*/
void icmdMdioRead
(
    uint8_t device,  // Address of MDIO attached device
    uint8_t address,  // Register address to read from
    uint8_t muxPort
) __attribute__((section(".atext")));
void icmdMdioRead
(
    uint8_t device,  // Address of MDIO attached device
    uint8_t address,  // Register address to read from
    uint8_t muxPort
)
{
    ilog_MDIO_COMPONENT_2(ILOG_USER_LOG, MDIO_ICMD_READ_START, device, address);
    MdioReadASync(device, address, &icmdMdioReadHelper, muxPort);
}


void icmdMdioIndirectRead
(
    uint8_t device, // Phy address, 5 bits
    uint8_t devType, // 5 bits: PMD/PMA, WIS, PCS, PHY XS, DTE XS
    uint16_t address,
    uint8_t muxPort
) __attribute__((section(".atext")));
void icmdMdioIndirectRead
(
    uint8_t device, // Phy address, 5 bits
    uint8_t devType, // 5 bits: PMD/PMA, WIS, PCS, PHY XS, DTE XS
    uint16_t address,
    uint8_t muxPort
)
{
    ilog_MDIO_COMPONENT_3(ILOG_USER_LOG, MDIO_INDIRECT_ICMD_READ_START, device, devType, address);
    MdioIndirectReadASync (device, devType, address, &icmdMdioReadHelper, muxPort);
}


static void icmdMdioReadHelper(uint16_t data)
{
    ilog_MDIO_COMPONENT_1(ILOG_USER_LOG, MDIO_ICMD_READ_DONE, data);
}

