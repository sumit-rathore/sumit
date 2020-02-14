///////////////////////////////////////////////////////////////////////////////////////////////////
//  Icron Technology Corporation - Copyright 2015
//
//  This source file and the information contained in it are confidential and proprietary to Icron
//  Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
//  of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or
//  to any employee of Icron who has not previously obtained written authorization for access from
//  the individual responsible for the source code, will have a significant detrimental effect on
//  Icron and is expressly prohibited.
///////////////////////////////////////////////////////////////////////////////////////////////////
//! @file  -  ibase_version.h
//
//! @brief -  Functions operating on firmware versions
//
//
//! @note  -  This code could is probably a bit too domain specific to really belong in ibase, but
//            there is no better spot for it.
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef IBASE_VERSION_H
#define IBASE_VERSION_H

/*************************************** Included Headers ****************************************/
#include <itypes.h>

/********************************** Defined Constants and Macros *********************************/

/****************************************** Data Types *******************************************/

/************************************ Function Declarations **************************************/

sint8 IBASE_compareFirmwareVersions(
    uint8 v1Major, uint8 v1Minor, uint8 v1Rev, uint8 v2Major, uint8 v2Minor, uint8 v2Rev
    ) __attribute__ ((section (".ftext")));

#endif // IBASE_VERSION_H
