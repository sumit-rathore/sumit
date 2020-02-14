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
//!   @file  -  data.c
//
//!   @brief -  Handles the data level access to flash
//
//
//!   @note  - TODO: create a local function:
//                  uint8 * parseFlash(boolT (*processVar)(uint8 * flashVarLocation));
//                  that can be used for all flash parsing
//                  processVar returns TRUE if parsing should continue, FALSE to stop
//                  return value is last flash location, either
//                      last var processed, end of flash, first empty flash address
//
//              TODO: Add macros for fetch size, active state, name, data location, etc
//
//
//  each variable should be stored as
//  byte 1) header with the following bitfields:
//      0-6:    size:       Allows variables up to 127 bytes in size. Stored as bit inverse, so 0x7F indicates size 0, or unused var
//      7:      active:     Written to 0 once this variable is no longer used
//  byte 2) Variable name:  enum of all the variables we support
//                          ie enum { MAC_ADDR, STATIC_IP, GATEWAY_IP, NETMASK_IP, USE_DHCP, DST_IP, etc , UNUSED_FLASH=0xFF}
//  byte 3 - ?) Variable data.  Anywhere from 1 byte to 127 bytes in size
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "flash_data_loc.h"
#include <flash_data.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static uint8 * findVarStart(enum storage_varName);
static uint8 * findNextVarStartAddr(uint8 sizeRequired);

/************************** Function Definitions *****************************/

static uint8 * findNextVarStartAddr(uint8 sizeRequired)
{
    uint8 * addr;
    uint8 * endAddr;
    enum flash_section section;

    // Get flash section address
    section = _FLASH_getActiveDataSection();
    addr = _FLASH_getSectionAddress(section);
    endAddr = _FLASH_getSectionEndAddress(section);

    // skip over flash section header
    addr++;

    // Start scanning variables until the variable is found
    while (TRUE)
    {
        uint8 flashVarHeader = addr[0];
        uint8 flashVarSize = (~flashVarHeader & 0x7F);

        if (flashVarHeader == 0xFF)
        {
            // We have reached the end of the data section

            // Ensure there is enough room for this new variable
            return ((endAddr - addr) >= sizeRequired) ? addr : NULL;
        }

        // Increment our address to the next variable
        addr += flashVarSize + 2;

        // Check if the end of the section is reached
        if (endAddr < addr)
        {
            return NULL;
        }
    }
}

static uint8 * findVarStart(enum storage_varName varToFind)
{
    uint8 * addr;
    uint8 * endAddr;
    enum flash_section section;

    // Get flash section address
    section = _FLASH_getActiveDataSection();
    addr = _FLASH_getSectionAddress(section);
    endAddr = _FLASH_getSectionEndAddress(section);

    // skip over flash section header
    addr++;

    // Start scanning variables until the variable is found
    while (TRUE)
    {
        uint8 flashVarHeader = addr[0];
        uint8 flashVarName = addr[1];
        uint8 flashVarSize = (~flashVarHeader & 0x7F);

        if (flashVarHeader == 0xFF)
        {
            // Variable doesn't exist in flash, we have reached the end of the
            // data section
            return NULL;
        }

        if ((flashVarName == varToFind) && (flashVarHeader & 0x80)) // Search variable name && active bit
        {
            // Found flash variable
            return addr;
        }

        // Increment our address to the next variable
        addr += flashVarSize + 2;

        // Check if the end of the section is reached
        if (endAddr < addr)
        {
            return NULL;
        }
    }
}

boolT _FLASH_needDefragmenting(enum flash_section section) { /* TODO */ return FALSE; }
void _FLASH_switchActive(enum flash_section oldActiveSection, enum flash_section newActiveSection) { /* TODO */ }

boolT FLASH_writeVar(struct flash_var * varToWrite)    // return TRUE on success; FALSE otherwise (perhaps flash is full, or there is already a write in progress)
{
    uint8 * oldAddr = findVarStart(varToWrite->var);
    uint8 * newAddr = findNextVarStartAddr(varToWrite->size + 2);   // add 2 for the header

    iassert_FLASH_DATA_COMPONENT_2( varToWrite->size < ((uint8)(0x80)),                         // condition
                                    WRITE_VAR_SIZE_TOO_BIG, varToWrite->var, varToWrite->size); // log + args


    ilog_FLASH_DATA_COMPONENT_2(ILOG_MAJOR_EVENT, WRITE_VAR, varToWrite->var, varToWrite->size);

    // is the new variable, identical to what is already on flash
    if (    oldAddr                                     // Check to see if old var exists
        &&  ((~(*oldAddr) & 0x7F) == varToWrite->size)) // Check to see if sizes match
    {
        uint8 i;
        uint8 * oldData = oldAddr + 2;  // skip over header
        boolT sameData = TRUE;
        for (i = 0; ((i < varToWrite->size) && sameData); i++)
        {
            sameData = (oldData[i] == varToWrite->data[i]);
        }

        if (sameData)
        {
            // The data was exactly the same
            return TRUE;
        }
    }

    // Was the flash data section full?
    if (newAddr == NULL)
    {
        return FALSE;
    }
    else
    {
        uint8 i;

        // write header (size)
        _FLASHRAW_writeByte(newAddr, 0x80 | ~varToWrite->size);  // ensure top bit set to be active & write inverse of size
        newAddr++;

        // write varName
        _FLASHRAW_writeByte(newAddr, varToWrite->var);  // ensure top bit set to be active & write inverse of size
        newAddr++;

        // write varData
        for (i = 0; i < varToWrite->size; i++)
        {
            _FLASHRAW_writeByte(newAddr + i, varToWrite->data[i]);  // ensure top bit set to be active & write inverse of size
        }

        // erase old var
        if (oldAddr)
        {
            ilog_FLASH_DATA_COMPONENT_1(ILOG_MINOR_EVENT, WRITE_VAR_IS_REPLACEMENT, varToWrite->var);
            _FLASHRAW_writeByte(oldAddr, *oldAddr & 0x7F);  // clear top bit
        }

        return TRUE;
    }
}


void FLASH_eraseVar(enum storage_varName var)
{
    uint8 * addr = findVarStart(var);

    if (addr)
    {
        // Erasing variable
        ilog_FLASH_DATA_COMPONENT_1(ILOG_MAJOR_EVENT, ERASE_VAR, var);
        _FLASHRAW_writeByte(addr, *addr & 0x7F);  // clear top bit
    }
    else
    {
        // hmm, variable never existed
        ilog_FLASH_DATA_COMPONENT_1(ILOG_WARNING, ERASE_VAR_FOR_VAR_THAT_DOESNT_EXIST, var);
    }
}

boolT FLASH_readVar(struct flash_var * varToRead, enum storage_varName var) // return TRUE if Variable found and read, FALSE otherwise
{
    uint8 * addr = findVarStart(var);

    if (addr)
    {
        uint8 size;
        ilog_FLASH_DATA_COMPONENT_1(ILOG_MINOR_EVENT, READ_VAR, var);
        size = (~*addr & 0x7F);
        // Ensure size isn't over max
        iassert_FLASH_DATA_COMPONENT_2(size <= FLASHDATA_MAX_VAR_SIZE, FLASH_VAR_TOO_BIG, var, size);
        varToRead->size = size;
        varToRead->var = var;
        memcpy(&varToRead->data[0], addr + 2, size);
        return TRUE;
    }
    else
    {
        ilog_FLASH_DATA_COMPONENT_1(ILOG_MAJOR_EVENT, CANT_READ_VAR, var);
        return FALSE;
    }
}
