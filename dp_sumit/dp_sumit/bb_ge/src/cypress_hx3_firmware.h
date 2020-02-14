///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2015
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - cypress_hx3_firmware.h
//
//!   @brief - Uploads a custom firmware image to a Cypress HX3 hub if it is found on the I2C bus.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef CYPRESS_HX3_FIRMWARE_H
#define CYPRESS_HX3_FIRMWARE_H


/***************************** Included Headers ******************************/

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

enum TOPLEVEL_CypressHx3UpgradeResult
{
    TOPLEVEL_CYPRESS_HX3_SUCCESS,
    TOPLEVEL_CYPRESS_HX3_HUB_NOT_RESPONDING,
    TOPLEVEL_CYPRESS_HX3_TRANSMISSION_FAILURE
};


/*********************************** API *************************************/

void TOPLEVEL_tryCypressHx3HubFirmwareUpgrade(void (*completionHandler)(enum TOPLEVEL_CypressHx3UpgradeResult));

#endif // CYPRESS_HX3_FIMRWARE_H
