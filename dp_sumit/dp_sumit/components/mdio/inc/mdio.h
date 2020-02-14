///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009, 2010, 2012
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
//!   @file  -  mdio_mdio.h
//
//!   @brief -  drivers for reading/writing/what-have-you via the MDIO interface
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MDIO_H
#define MDIO_H

/***************************** Included Headers ******************************/
#include <ibase.h>


/************************ Defined Constants and Macros ***********************/


/******************************** Data Types *********************************/
typedef void (*NotifyReadCompleteHandler)(uint16_t data);       // callback for mdio async read operation
typedef void (*NotifyWriteCompleteHandler)(void);               // callback for mdio async write operation

enum MDIO_DEVTYPE
{
    MDIO_DEVTYPE_PMD_PMA   = 0x1,       // Device type PMA
    MDIO_DEVTYPE_WIS       = 0x2,
    MDIO_DEVTYPE_PCS       = 0x3,       // Device type PCS
    MDIO_DEVTYPE_PHY_XS    = 0x4,       // Device type PHY XS
    MDIO_DEVTYPE_DTE_XS    = 0x5,       // Device type DTE XS: Created when xaui->control->type_sel = 0x02 (check function: XAUI_Control)
    MDIO_DEVTYPE_AUTO_NEGO = 0x7,       // Device type Auto Negotiation
    MDIO_DEVTYPE_GLOBAL    = 0x1E,      // Device type Global
};

/*********************************** API *************************************/
void _MdioInit(void (*callback)(void)) __attribute__((section(".ftext")));

void Mdio_InterruptHandler(void) __attribute__((section(".ftext")));

void MdioWriteASync(
    uint8_t device,
    uint8_t address,
    uint16_t data,
    NotifyWriteCompleteHandler writeCompleteHandler,
    uint8_t muxPort);

void MdioReadASync(
    uint8_t device,
    uint8_t address,
    NotifyReadCompleteHandler readCompleteHandler,
    uint8_t muxPort) __attribute__((section(".ftext")));

void MdioIndirectWriteASync
(
    uint8_t device,
    enum MDIO_DEVTYPE devType,
    uint16_t address,
    uint16_t data,
    NotifyWriteCompleteHandler writeCompleteHandler,
    uint8_t muxPort) __attribute__((section(".ftext")));

void MdioIndirectReadASync
(
    uint8_t device,
    enum MDIO_DEVTYPE devType,
    uint16_t address,
    NotifyReadCompleteHandler readCompleteHandler,
    uint8_t muxPort) __attribute__((section(".ftext")));

uint16_t MdioReadSync(
    uint8_t device,
    uint8_t address,
    uint8_t muxPort) __attribute__((section(".ftext")));

void MdioWriteSync(
    uint8_t device,
    uint8_t address,
    uint16_t data,
    uint8_t muxPort) __attribute__((section(".ftext")));

uint16_t MdioIndirectReadSync
(
    uint8_t device, // Address of MDIO attached device
    enum MDIO_DEVTYPE devType,
    uint16_t address, // Register address to read from
    uint8_t muxPort
) __attribute__((section(".ftext")));

uint16_t MdioIndirectWriteSync
(
    uint8_t device, // Address of MDIO attached device
    enum MDIO_DEVTYPE devType,
    uint16_t address, // Register address to read from
    uint16_t data,
    uint8_t muxPort
) __attribute__((section(".ftext")));

void MdioIndirectWriteSyncBitField
(
    uint8_t device, // Address of MDIO attached device
    enum MDIO_DEVTYPE devType, // MDIO Devices have different types on same physical device:
    uint16_t address, // Register address for Device Type to write to
    uint16_t data,      // data to be written
    uint8_t muxPort,    // MDIO port direction
    uint16_t mask,      // Bits to be controlled
    uint8_t offset      // Bits location from bit0
) __attribute__((section(".ftext")));


#endif // MDIO_H

