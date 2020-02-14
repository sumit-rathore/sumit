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
//! @file  -  ibase_version.c
//
//! @brief -  Functions operating on firmware versions
//
//
//! @note  -  This code could is probably a bit too domain specific to really belong in ibase, but
//            there is no better spot for it.
///////////////////////////////////////////////////////////////////////////////////////////////////


/*************************************** Included Headers ****************************************/
#include "ibase_version.h"
#include "ibase.h"

/********************************** Defined Constants and Macros *********************************/

/****************************************** Data Types *******************************************/

/*************************************** Local Variables *****************************************/

/************************************ Function Definitions ***************************************/

/**
* FUNCTION NAME: IBASE_compareFirmwareVersions()
*
* @brief  - Compares two firmware versions.  The function works in the typical way for C
*           comparision functions.
*
* @return - If v1 is newer than v2 then the return value will be positive.  If v1 is older than v2,
*           then the return value will be negative.  If the versions are equal, the return value
*           will be zero.
*/
sint8 IBASE_compareFirmwareVersions(
    uint8 v1Major, uint8 v1Minor, uint8 v1Rev, uint8 v2Major, uint8 v2Minor, uint8 v2Rev)
{
    sint8 cmp = CMP(v1Major, v2Major);

    if (cmp == 0)
    {
        cmp = CMP(v1Minor, v2Minor);
    }

    if (cmp == 0)
    {
        cmp = CMP(v1Rev, v2Rev);
    }

    return cmp;
}
