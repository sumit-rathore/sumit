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

/******************** Internal Defined Constants and Macros ******************/
#define _LEON_TIMER1_COUNTER_OFFSET         (0x40)
#define _LEON_TIMER1_COUNTER_MASK           (0x3FFFFFFF)
#define _LEON_PSR_PIL_BIT_OFFSET            (8)
#define _LEON_PSR_PIL_MASK                  (0xF << _LEON_PSR_PIL_BIT_OFFSET )

/******************************* Internal API ********************************/
#ifndef __ASSEMBLER__

// exported only for static inlines
// identical to ReadLeonRegister
static inline uint32 _LEON_ReadLeonRegister( uint32 in_nByteOffset )
{
    register volatile uint8* g_pLeonBaseAddr    __asm__("%g7");
    return (*(volatile uint32*)(g_pLeonBaseAddr + in_nByteOffset ));
}

// exported only for static inlines
// identical to ReadLeonRegister
static inline void _LEON_WriteLeonRegister( uint32 in_nByteOffset, uint32 in_nValue )
{
    register volatile uint8* g_pLeonBaseAddr    __asm__("%g7");
    *(volatile uint32*)( g_pLeonBaseAddr + in_nByteOffset ) = in_nValue;
}

#endif //#ifndef __ASSEMBLER__


#endif // _LEON_REG_ACCESS_H


