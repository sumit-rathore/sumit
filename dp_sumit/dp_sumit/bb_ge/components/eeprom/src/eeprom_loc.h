///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2014
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
//!   @file  - eeprom_loc.h
//
//!   @brief - Local info for the AT24CXX EEPROM chips.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef EEPROM_LOC_H
#define EEPROM_LOC_H

/***************************** Included Headers ******************************/
#include <stddef.h>
#include <timing_timers.h>
#include "eeprom_log.h"

/***************************** Defined constants *****************************/
// This is functionally a multiply by 16, as there are 16 bytes per page
#define EEPROM_AT24CXX_PAGE_SHIFT (4)
// Right shifting by this amount places the A10, A9 and A8 bits in the proper
// spot in the I2C address byte
#define EEPROM_AT24CXX_SHIFT_ADDRESS_BITS (8)
#define EEPROM_AT24CXX_A10_TO_A8_MASK (0x07)
#define EEPROM_AT24CXX_ADDRESS_LENGTH (1)


/******************************** Data Types *********************************/

// This struct is contained solely within this module and should not be accessed
// by any client.
struct EEPROM_at24cxxState
{
    // Size of the EEPROM in pages
    uint8 numPages;

    uint8 i2cAddr;

    // I2C bus number that the EEPROM is on
    uint8 i2cBus;

    // Marked as TRUE when there is an operation in progress
    boolT busy;

    TIMING_TimerHandlerT writeRetryWaitTimer;
    TIMING_TimerHandlerT readRetryWaitTimer;

    // Have we seen a NAKed read/write (presumably because we didn't wait long enough after
    // starting a write cycle) and waited for the chip to be ready for us to retry the read/write?
    boolT waitTimerExpired;

    // Page number of the pending read/write
    uint8 activePageNum;

    // Context to be passed to the callback when the current operation completes
    void* callbackData;

    union
    {
        struct
        {
            // Function that is called when the read operation completes
            void (*completionFunction)(void* callbackData, boolT success, uint8* pageData, uint8 byteCount);

            // Pointer to the user supplied buffer to read data into.  The user should have
            // supplied a buffer that is at least GRG_CAT24CXX_PAGE_SIZE in length.
            uint8* pageBuffer;
        } read;
        struct
        {
            // Function that is called when the write operation completes
            void (*completionFunction)(void* callbackData, boolT success);

            // Make the buffer big enough to contain an address and a page of data.
            uint8 pageBuffer[EEPROM_PAGE_SIZE+EEPROM_AT24CXX_ADDRESS_LENGTH];
        } write;
    };
};


/*********************************** API *************************************/

#endif // EEPROM_LOC_H
