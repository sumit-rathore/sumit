///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
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
//!   @file  - packed_pointers.c
//
//!   @brief -
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <leon_mem_map.h>
#include <ibase.h>

/************************ Defined Constants and Macros ***********************/

// 0x0.... is a flash ptr
// 0x1.... is a flash ptr
// 0x2.... is an IRAM ptr
// 0x3.... is a DRAM ptr
#define FLASH_MASK                      0x1FFFF
#define RAM_MASK                        0x0FFFF
#define INDICATOR_MASK                  0x30000
#define RAM_INDICATOR_MASK              0x20000
#define IRAM_INDICATOR                  0x20000
#define DRAM_INDICATOR                  0x30000
#define DRAM_NOT_IRAM_INDICATOR_MASK    0x10000

#define DRAM_MASK                       0x3FFF


/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/



uint32 LEON_packPointer(void * arg)
{
    if (!arg)
    {
        return 0;
    }
    else
    {
        uint32 ptr = CAST(arg, void *, uint32);

        if (ptr & 0x10000000) // flash
        {
            return ptr & FLASH_MASK;
        }
        else if (ptr & 0x01000000) // DRAM
        {
            return (ptr & RAM_MASK) | DRAM_INDICATOR;
        }
        else //IRAM
        {
            return (ptr & RAM_MASK) | IRAM_INDICATOR;
        }
    }
}


void * LEON_unpackPointer(uint32 ptr)
{
    uint32 ret = 0;

    if (ptr)
    {
        if (ptr & RAM_INDICATOR_MASK)
        {
            if (ptr & DRAM_NOT_IRAM_INDICATOR_MASK)
            {
                ret = (ptr & RAM_MASK) | LEON_DRAM_ADDR;
            }
            else // iram
            {
                ret = (ptr & RAM_MASK) | LEON_IRAM_ADDR;
            }
        }
        else // flash
        {
                ret = ptr | SERIAL_FLASH_BASE_ADDR;
        }
    }

    return CAST(ret, uint32, void *);
}

uint32 LEON_packWordPointer(void * arg)
{
    return LEON_packPointer(arg) >> 2;
}


void * LEON_unpackWordPointer(uint32 arg)
{
    return LEON_unpackPointer(arg << 2);
}



uint32 LEON_packDRamPointer(void * arg)
{
    return CAST(arg, void *, uint32) & DRAM_MASK;
}


void * LEON_unpackDRamPointer(uint32 arg)
{
    if (arg)
        return CAST(arg | LEON_DRAM_ADDR, uint32, void *);
    else
        return NULL;
}



uint32 LEON_packWordDRamPointer(void * arg)
{
    return LEON_packDRamPointer(arg) >> 2;
}


void * LEON_unpackWordDRamPointer(uint32 arg)
{
    return LEON_unpackDRamPointer(arg << 2);
}


