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
//!   @file  - kc705_si5326.c
//
//!   @brief - Setup the jitter attenuation chip on KC705.
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "kc705_loc.h"
#include <ibase.h>
#ifdef BLACKBIRD
#include <bgrg_i2c.h>
#else
#include <grg_i2c.h>
#endif

/************************ Defined Constants and Macros ***********************/
#define PLL_SI5326_DEJITTER_CHIP_ADDRESS        (0x68)
#ifdef BLACKBIRD
#define PLL_SI5326_DEJITTER_CHIP_SPEED          BGRG_I2C_SPEED_SLOW
#else
#define PLL_SI5326_DEJITTER_CHIP_SPEED          GRG_I2C_SPEED_SLOW
#endif

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static void (*_SetupDeJitterCompletionHandler)(void);

// Variable to keep track of DeJitterInit array index
static uint8 currentRegIndex = 0;

static const struct { uint8 reg; uint8 data; } deJitterInit[] =
{
    { 0x00, 0x14 },    { 0x01, 0xE4 },    { 0x02, 0xA2 },
    { 0x03, 0x15 },    { 0x04, 0x92 },    { 0x05, 0xED },
    { 0x06, 0x2D },    { 0x07, 0x2A },    { 0x08, 0x00 },
    { 0x09, 0xC0 },    { 0x0a, 0x08 },    { 0x0b, 0x42 },

    { 0x13, 0x29 },    { 0x14, 0x3E },    { 0x15, 0xFF },
    { 0x16, 0xDF },    { 0x17, 0x1F },    { 0x18, 0x3F },
    { 0x19, 0x40 },

    { 0x1F, 0x00 },    { 0x20, 0x00 },    { 0x21, 0x05 },
    { 0x22, 0x00 },    { 0x23, 0x00 },    { 0x24, 0x05 },

    { 0x28, 0xA0 },    { 0x29, 0x01 },    { 0x2A, 0x3B },
    { 0x2B, 0x00 },    { 0x2C, 0x00 },    { 0x2D, 0x4E },
    { 0x2E, 0x00 },    { 0x2F, 0x00 },    { 0x30, 0x4E },

    { 0x37, 0x00 },

    { 0x83, 0x1F },    { 0x84, 0x02 },

    { 0x89, 0x01 },    { 0x8A, 0x0F },    { 0x8B, 0xFF },

    { 0x8E, 0x00 },    { 0x8F, 0x00 },

    { 0x88, 0x40 }
};

/************************ Local Function Prototypes **************************/
static void _KC705_SetupDeJitterChip2(boolT success);
static void _KC705_SetupDeJitterChipDone(boolT success);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: KC705_SetupDeJitterChip
*
* @brief  - Setup De Jitter chip
*
* @return - void
*
* @note   -
*
*/
void KC705_SetupDeJitterChip(void (*notifyWriteCompleteHandler)(void))
{
    _SetupDeJitterCompletionHandler = notifyWriteCompleteHandler;

    KC705_Select(KC705_MUX_SI5326, &_KC705_SetupDeJitterChip2);
}

/**
* FUNCTION NAME: _KC705_SetupDeJitterChip2
*
* @brief  - Set de-jitter chip to 1 ppm
*
* @return - void
*
* @note   -
*
*/

static void _KC705_SetupDeJitterChip2(boolT success)
{
    uint8 bus = 0;
    uint8 device = PLL_SI5326_DEJITTER_CHIP_ADDRESS;
    void (*notifyWriteCompleteHandler)(boolT success);

    iassert_KC705_COMPONENT_1((success == TRUE), DE_JITTER_WRITE_FAILED, __LINE__);

    if (currentRegIndex  >= (ARRAYSIZE(deJitterInit)) - 1)
    {
        ilog_KC705_COMPONENT_0(ILOG_MAJOR_EVENT, DEJITTER_CHIP_CONFIGURED);
        notifyWriteCompleteHandler = &_KC705_SetupDeJitterChipDone;
    }
    else
    {
        notifyWriteCompleteHandler = &_KC705_SetupDeJitterChip2;
    }

#ifdef BLACKBIRD
    BGRG_I2cWriteASync
#else
    GRG_I2cWriteASync
#endif
                      (bus,
                       device,
                       PLL_SI5326_DEJITTER_CHIP_SPEED,
                       (uint8 *)&deJitterInit[currentRegIndex],
                       sizeof(deJitterInit[currentRegIndex]),
                       notifyWriteCompleteHandler);

    currentRegIndex++;
}

/**
* FUNCTION NAME: _KC705_SetupDeJitterChipDone
*
* @brief  - Verify the last I2C write and then call the completion handler
*
* @return - void
*
* @note   -
*
*/
static void _KC705_SetupDeJitterChipDone(boolT success)
{
    iassert_KC705_COMPONENT_1((success == TRUE), DE_JITTER_WRITE_FAILED, __LINE__);
    
    (*_SetupDeJitterCompletionHandler)();
}

