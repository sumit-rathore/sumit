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
#include <_leon_reg_access.h>

/************************ Defined Constants and Macros ***********************/

#define FLASH_MASK                      0x1FFFF
#define RAM_MASK                        0x0FFFF
#define AHBRAM_MASK                     0x1FFFF
#define DRAM_MASK                       0x0FFFF

// NOTE: The width of 16 allows for regions of up to 64K in size.  Larger regions can be supported
// by encoding multiple types within the same region to divide it into smaller pieces.
#define PACKED_DATA_MASK                0x0000FFFF
#define PACKED_DATA_OFFSET              0
#define PACKED_DATA_WIDTH               16

#define PACKED_TYPE_MASK                0x00070000
#define PACKED_TYPE_OFFSET              PACKED_DATA_WIDTH
#define PACKED_TYPE_WIDTH               3


/******************************** Data Types *********************************/

// NOTE: ensure that all LEON_MemoryRegion values fit within REGION_WIDTH bits.
enum LEON_PointerType
{
    PTR_NULL,
    PTR_IRAM,
    PTR_DRAM,
    PTR_AHBRAM_LOW,
    PTR_AHBRAM_HIGH,
    PTR_FLASH_LOW,
    PTR_FLASH_HIGH
};


/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/
