///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011
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
//!   @file  -  header.c
//
//!   @brief -  Handles all access to the flash section headers
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "flash_data_loc.h"

/************************ Defined Constants and Macros ***********************/
#define __BLANK_FLASH       0b11111111
#define __ACTIVE_FLASH      0b11111110
#define __NOT_ACTIVE_FLASH  0b11111100

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: _FLASH_readHeader()
*
* @brief  - Read a flash section header
*
* @return - state of the flash section
*
* @note   -
*
*/
enum flash_header _FLASH_readHeader
(
    enum flash_section section  // Which flash section to read
)
{
    // Read flash header
    uint8 header = *_FLASH_getSectionAddress(section);

    // Determine return value
    if (header == __BLANK_FLASH)            // Blank
    {
        return FLASH_HEADER_BLANK;
    }
    else if (header == __ACTIVE_FLASH)      // Active bit set
    {
        return FLASH_HEADER_ACTIVE;
    }
    else if (header == __NOT_ACTIVE_FLASH)  // Active bit & Not-Active bit set
    {
        return FLASH_HEADER_NOT_ACTIVE;
    }
    else
    {
        return FLASH_GARBAGE;
    }
}


/**
* FUNCTION NAME: _FLASH_setHeader()
*
* @brief  - Set the header of a flash section
*
* @return - void
*
* @note   - This does basic consistency checks and will assert on error
*
*/
void _FLASH_setHeader
(
    enum flash_section section,     // section of flash
    enum flash_header header        // header to write
)
{
    // Read flash oldHeader
    enum flash_header oldHeader = _FLASH_readHeader(section);
    uint8 writeValue;

    // Determine if new value is valid
    if (oldHeader == FLASH_HEADER_BLANK)
    {
        iassert_FLASH_DATA_COMPONENT_2(header == FLASH_HEADER_ACTIVE, UNEXPECTED_HEADER_WHEN_SETTING, oldHeader, header);
        writeValue = 0b11111110;
    }
    else if (oldHeader == FLASH_HEADER_ACTIVE)
    {
        iassert_FLASH_DATA_COMPONENT_2(header == FLASH_HEADER_NOT_ACTIVE, UNEXPECTED_HEADER_WHEN_SETTING, oldHeader, header);
        writeValue = 0b11111100;
    }
    else
    {
        iassert_FLASH_DATA_COMPONENT_2(FALSE, UNEXPECTED_HEADER_WHEN_SETTING, oldHeader, header);
    }

    _FLASHRAW_writeByte(_FLASH_getSectionAddress(section), writeValue);
}


enum flash_section _FLASH_getActiveDataSection(void)
{
    if (_FLASH_readHeader(SECTION1) == FLASH_HEADER_ACTIVE)
    {
        return SECTION1;
    }
    else if (_FLASH_readHeader(SECTION2) == FLASH_HEADER_ACTIVE)
    {
        return SECTION2;
    }
    else
    {
        iassert_FLASH_DATA_COMPONENT_0(FALSE, UNABLE_TO_FIND_ACTIVE_DATA_SECTION);
        __builtin_unreachable();
    }
}

