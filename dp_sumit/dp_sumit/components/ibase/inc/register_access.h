///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013
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
//!   @file  -  register_access.h
//
//!   @brief -  macros used for reading and writing registers
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef REGISTER_ACCESS_H
#define REGISTER_ACCESS_H

// Macros to help get and set bitfields from registers
// the defines _prefix_ ## _MASK, and _prefix_ ## _OFFSET, must already exist
#define GET_BF(_prefix_, _reg_) (((_reg_) & (_prefix_ ## _MASK)) >> (_prefix_ ## _OFFSET))
#define SET_BF(_prefix_, _reg_, _value_) ((((_value_) << (_prefix_ ## _OFFSET)) & (_prefix_ ## _MASK)) | ((_reg_) & ~(_prefix_ ## _MASK)))

// Macros to help get/set/clr a single bit in a register value
#define GET_BIT(_mask_, _reg_) (((_reg_) & (_mask_)) ? 1 : 0)
#define SET_BIT(_mask_, _reg_) ((_reg_) | (_mask_))
#define CLR_BIT(_mask_, _reg_) ((_reg_) & ~(_mask_))

#endif //#ifndef REGISTER_ACCESS_H
