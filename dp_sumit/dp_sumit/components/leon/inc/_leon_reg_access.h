///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -  _leon_reg_access.h
//
//!   @brief -  An internal file to the leon component exposed in /inc/ for static inlines
//
//!   @note  -  This is shared with assembly
//
///////////////////////////////////////////////////////////////////////////////
#ifndef _LEON_REG_ACCESS_H
#define _LEON_REG_ACCESS_H

/***************************** Included Headers ******************************/
#ifndef __ASSEMBLER__
#include <itypes.h>
#endif
#include <leon_mem_map.h>
#include <options.h>

/******************** Internal Defined Constants and Macros ******************/
// 32bit reads/writes
#define LEON_READ_REG(_register_) (_LEON_ReadLeonRegister(_register_ ## _OFFSET))
#define LEON_WRITE_REG(_register_, _value_) (_LEON_WriteLeonRegister(_register_ ## _OFFSET, _value_))

// bitmask bitfield, shift by bitfield offset
#define LEON_GET_BF(_value_, _reg_, _bitfield_) ( \
        ((_value_) & (_reg_ ## _bitfield_ ## _MASK)) \
        >> (_reg_ ## _bitfield_ ## _OFFSET))
// clear bitfield with mask, reverse shift value, apply mask, OR in bitfield
#define LEON_SET_BF(_value_, _reg_, _bitfield_, _setvalue_) ( \
        ((_value_) & ~(_reg_ ## _bitfield_ ## _MASK)) \
        | (((_setvalue_) << (_reg_ ## _bitfield_ ## _OFFSET)) \
            & (_reg_ ## _bitfield_ ## _MASK)))

// required because single bit bitfields do not have _MASK or _OFFSET
// 
#define LEON_GET_BIT(_value_, _reg_, _bitfield_) ( \
        (((_value_) & (_reg_ ## _bitfield_)) > (0)) ? (1) : (0))
// if value 0, clear bit, else set bit
#define LEON_SET_BIT(_value_, _reg_, _bitfield_, _setvalue_) ( \
        ((_setvalue_) > (0)) ? \
        ((_value_) | (_reg_ ## _bitfield_)) : \
        ((_value_) & ~(_reg_ ## _bitfield_)))

// read full register - extract bitfield
#define LEON_READ_BF(_register_, _bitfield_) (LEON_GET_BF(LEON_READ_REG(_register_),_register_, _bitfield_))
// read full register - set bitfield of that full read - write to register
#define LEON_WRITE_BF(_register_, _bitfield_, _value_) \
    (LEON_WRITE_REG(_register_, \
        LEON_SET_BF(LEON_READ_REG(_register_), _register_, _bitfield_, _value_)))

#define LEON_READ_BIT(_register_, _bitfield_) (LEON_GET_BIT(LEON_READ_REG(_register_),_register_, _bitfield_))
// read full register - set bitfield of that full read - write to register
#define LEON_WRITE_BIT(_register_, _bitfield_, _value_) \
    (LEON_WRITE_REG(_register_, \
        LEON_SET_BIT(LEON_READ_REG(_register_), _register_, _bitfield_, _value_)))

/******************************* Internal API ********************************/
#ifndef __ASSEMBLER__

// exported only for static inlines
// identical to ReadLeonRegister
static inline uint32_t _LEON_ReadLeonRegister( uint32_t in_nByteOffset )
{
    register volatile uint8_t* g_pLeonBaseAddr    __asm__("%g7");
    return (*(volatile uint32_t*)(g_pLeonBaseAddr + in_nByteOffset ));
}

// exported only for static inlines
// identical to ReadLeonRegister
static inline void _LEON_WriteLeonRegister( uint32_t in_nByteOffset, uint32_t in_nValue )
{
    register volatile uint8_t* g_pLeonBaseAddr    __asm__("%g7");
    *(volatile uint32_t*)( g_pLeonBaseAddr + in_nByteOffset ) = in_nValue;
}

void LEON_resetIrqs(void)                           __attribute__((section(".atext")));

#endif //#ifndef __ASSEMBLER__

#endif // _LEON_REG_ACCESS_H


