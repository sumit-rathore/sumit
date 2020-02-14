///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2011, 2012
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
//!   @file  -  init.c
//
//!   @brief -  Initialization routine for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "flash_data_loc.h"
#include <flash_data.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static enum flash_header cleanFlash(enum flash_header header, enum flash_section section);
static void processPossibleSectionSwap(enum flash_section activeSection, enum flash_section inactiveSection);

/************************** Function Definitions *****************************/

void FLASH_init(void) //garbage collect flash.  Only place where a flash sector can be erased. Does bouncing between data section 1 and data section 2
{
    enum flash_header sec1 = _FLASH_readHeader(SECTION1);
    enum flash_header sec2 = _FLASH_readHeader(SECTION2);

    ilog_FLASH_DATA_COMPONENT_0(ILOG_DEBUG, FLASH_INIT);

    // Sanity check of the flash data max size
    COMPILE_TIME_ASSERT(FLASHDATA_MAX_VAR_SIZE < 128);
    COMPILE_TIME_ASSERT(FLASHDATA_MAX_VAR_SIZE >= 16);

    // Erase any sections that need erasing
    sec1 = cleanFlash(sec1, SECTION1);
    sec2 = cleanFlash(sec2, SECTION2);

    // At this point there only 4 valid statesi:
    // Both blank
    // section 1 active, 2 blank
    // section 2 blank, 1 active
    // both active
    if (    (sec1 == FLASH_HEADER_BLANK)
        &&  (sec2 == FLASH_HEADER_BLANK))
    {
        // Both sections are blank
        // Set section 1 as active
        ilog_FLASH_DATA_COMPONENT_0(ILOG_WARNING, INIT_SECTION1_MARKED_AS_ACTIVE);
        _FLASH_setHeader(SECTION1, FLASH_HEADER_ACTIVE);
    }
    else if (   (sec1 == FLASH_HEADER_ACTIVE)
            &&  (sec2 == FLASH_HEADER_ACTIVE))
    {
        //TODO: error both are active
        //Do we assert?  A unit in the field could turn into a brick!
        //We could count valid variables, and choose an active section
        //  (assuming valid section has between 1 & FLASH_NUMBER_OF_VARIABLE_NAMES)
        // Assert during development, but change later
        iassert_FLASH_DATA_COMPONENT_0(FALSE, MULTIPLE_ACTIVE_SECTIONS);
    }
    else
    {
        // one section is active and one is blank
        processPossibleSectionSwap( (sec1 == FLASH_HEADER_ACTIVE) ? sec1 : sec2,
                                    (sec1 == FLASH_HEADER_ACTIVE) ? sec2 : sec1);
    }
}

static enum flash_header cleanFlash(enum flash_header header, enum flash_section section)
{
    // Erase any sections that need erasing
    switch (header)
    {
        case FLASH_HEADER_BLANK:
        case FLASH_HEADER_ACTIVE:
            // No erasing needed
            break;

        case FLASH_GARBAGE:
            ilog_FLASH_DATA_COMPONENT_1(ILOG_MAJOR_ERROR, INIT_FOUND_GARBAGE_HEADER, section);
        case FLASH_HEADER_NOT_ACTIVE:
        default:
            _FLASHRAW_erase(section);
            header = FLASH_HEADER_BLANK;
            break;
    }

    return header;
}

static void processPossibleSectionSwap(enum flash_section activeSection, enum flash_section inactiveSection)
{
    if (_FLASH_needDefragmenting(activeSection))
    {
        _FLASH_switchActive(/*old*/ activeSection, /*new*/ inactiveSection);
    }
}

