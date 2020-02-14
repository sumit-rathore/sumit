///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2007, 2008
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
//!   @file  -  ibase.h
//
//!   @brief -  common basic macros + ...
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef IBASE_H
#define IBASE_H

/***************************** Included Headers ******************************/
#include <itypes.h>

/************************ Defined Constants and Macros ***********************/

// Converts the value resolved by a macro to a string literal
#define STRINGIFY_MACRO(s) _STRINGIFY_MACRO(s)
#define _STRINGIFY_MACRO(s) #s


// Find the offset within a structure of a member
#define OFFSET_OF(_type_,_member_) ((uint32_t)( &((_type_*)0) -> _member_ ))

// A macro to verify that an argument is of the correct type, returns the argument
#ifdef __MSP430__
// MSP430 compiler doesn't support many GCC extensions
#define VERIFY_TYPE(_arg_, _type_) (_arg_)
#else
// Works by the comparison throwing a compiler error when the types don't match
#define VERIFY_TYPE(_arg_, _type_)  \
    ({  typeof(_arg_) _x_;          \
        _type_ _y_;                 \
        (void)(&_x_ == &_y_);       \
        _arg_;                      \
    })
#endif

// A controlled cast from type old to type new
#define CAST(_var_, _old_type_, _new_type_) \
    ((_new_type_)VERIFY_TYPE(_var_, _old_type_))

// Find out how many elements are in an array
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

// Build time assert, compiler should always optimize away, but fail at build time
// Asserts pass when the expression is true, and fail otherwise
//  Works by the fact that we can't have 2 cases that are the same
//  If expression is false, then 2 cases are both false, so we have a failure
#define COMPILE_TIME_ASSERT(_expr_) \
    do {                            \
        switch (true)               \
        {                           \
            case (false):           \
            case (_expr_):          \
            default:                \
                break;              \
        }                           \
    }  while (false)


// Get the MSW and LSW of a 64 bit value
#define GET_MSW_FROM_64(x) (CAST(((x) >> 32), uint64_t, uint32_t))
#define GET_LSW_FROM_64(x) (CAST(((x) & 0xFFFFFFFF), uint64_t, uint32_t))

// Create a 64bit value
#define MAKE_U64(msw, lsw) ((CAST(msw, uint32_t, uint64_t) << 32) | (CAST(lsw, uint32_t, uint64_t) & 0xFFFFFFFF))

// Get the MSH and LSH from a 32 bit value
#define GET_MSH_FROM_32(x) (CAST(((x) >> 16), uint32_t, uint16_t))
#define GET_LSH_FROM_32(x) (CAST(((x) & 0xFFFF), uint32_t, uint16_t))

// Create a 32bit value
#define MAKE_U32(msh, lsh) ((CAST(msh, uint16_t, uint32_t) << 16) | (CAST(lsh, uint16_t, uint32_t) & 0xFFFF))

// Get the MSB and LSB from a 16 bit value
// NOTE: The operations, seem to do implicit type conversion, so this ends with a cast
#define GET_MSB_FROM_16(x) ((uint8_t)(VERIFY_TYPE(x, uint16_t) >> 8))
#define GET_LSB_FROM_16(x) ((uint8_t)(VERIFY_TYPE(x, uint16_t) & 0xFF))

// Create a 16bit value
// NOTE: The operations, seem to do implicit type conversion, so this ends with a cast
#define MAKE_U16(msb, lsb) ((uint16_t)((CAST(msb, uint8_t, uint16_t) << 8) | (CAST(lsb, uint8_t, uint16_t) & 0xFF)))


// Create mask from a bit width and bit position
// This needs to take care for boundary conditions when the width of the mask fills the type
#define CREATE_MASK(_width_, _position_, _type_) ({                                                                                             \
    typeof(_width_) __width__ = _width_;                                                                                                        \
    typeof(_position_) __position__ = _position_;                                                                                               \
    __width__ == 0 ? 0 : /* If the width is zero return 0x0, else */                                                                            \
        ({                                                                                                                                      \
            _type_ mask0;                                       /* mask at offset 0 */                                                          \
            mask0 =     ((((_type_)1) << (__width__ - 1)) - 1)  /* Produce mask starting at offset that has all the bits but the top bit set */ \
                    |   (((_type_)1) << (__width__ - 1));       /* Or in the top bit */                                                         \
            mask0 << __position__;                              /* Move the mask into position */                                               \
        });                                                                                                                                     \
    })

// set the mask given the bit offset
#define SET_BITMASK(bitoffset) (1 << (bitoffset))

// Macros to reverse the arguments for endian swapping
#define REVERSE_ENDIAN_16(x)    ({ uint16_t y = x; ((y & 0xFF) << 8) | ((y >> 8) & 0xFF); })
#define REVERSE_ENDIAN_32(x)    ({ uint32_t y = x; ((y & 0xFF) << 24) | ((y & 0xFF00) << 8) | ((y >> 8) & 0xFF00) | ((y >> 24) & 0xFF); })

// Unsafe versions of previous macros, these are against coding standard but useful if assigning endian affected values within constant initializers etc.
// NOTE: when using these macros, do NOT pass in statements, such as i++, --i, or all hell will break loose due to multiple evaluations of said statements.
//       this is exactly why it is against coding standard (but sometimes useful)
#define REVERSE_ENDIAN_16_UNSAFE(y)    ( ((y & 0xFF) << 8) | ((y >> 8) & 0xFF) )
#define REVERSE_ENDIAN_32_UNSAFE(y)    ( ((y & 0xFF) << 24) | ((y & 0xFF00) << 8) | ((y >> 8) & 0xFF00) | ((y >> 24) & 0xFF) )

// Define all of the endian macros for this type of endian
#if IENDIAN == 0
// little endian
#define IS_LITTLE_ENDIAN    true
#define IS_BIG_ENDIAN       false

#define BIG_ENDIAN_TO_HOST_16(x)            REVERSE_ENDIAN_16(x)
#define LITTLE_ENDIAN_TO_HOST_16(x)         (x)
#define LITTLE_ENDIAN_TO_HOST_16_UNSAFE(x)  (x)

#define BIG_ENDIAN_TO_HOST_32(x)            REVERSE_ENDIAN_32(x)
#define LITTLE_ENDIAN_TO_HOST_32(x)         (x)
#define LITTLE_ENDIAN_TO_HOST_32_UNSAFE(x)  (x)

#define HOST_ENDIAN_TO_BIG_16(x)            REVERSE_ENDIAN_16(x)
#define HOST_ENDIAN_TO_LITTLE_16(x)         (x)
#define HOST_ENDIAN_TO_LITTLE_16_UNSAFE(x)  (x)

#define HOST_ENDIAN_TO_BIG_32(x)            REVERSE_ENDIAN_32(x)
#define HOST_ENDIAN_TO_LITTLE_32(x)         (x)
#define HOST_ENDIAN_TO_LITTLE_32_UNSAFE(x)  (x)

#elif IENDIAN == 1
//big endian

#define IS_LITTLE_ENDIAN    false
#define IS_BIG_ENDIAN       true

#define BIG_ENDIAN_TO_HOST_16(x)            (x)
#define LITTLE_ENDIAN_TO_HOST_16(x)         REVERSE_ENDIAN_16(x)
#define LITTLE_ENDIAN_TO_HOST_16_UNSAFE(x)  REVERSE_ENDIAN_16_UNSAFE(x)

#define BIG_ENDIAN_TO_HOST_32(x)            (x)
#define LITTLE_ENDIAN_TO_HOST_32(x)         REVERSE_ENDIAN_32(x)
#define LITTLE_ENDIAN_TO_HOST_32_UNSAFE(x)  REVERSE_ENDIAN_32_UNSAFE(x)

#define HOST_ENDIAN_TO_BIG_16(x)            (x)
#define HOST_ENDIAN_TO_LITTLE_16(x)         REVERSE_ENDIAN_16(x)
#define HOST_ENDIAN_TO_LITTLE_16_UNSAFE(x)  REVERSE_ENDIAN_16_UNSAFE(x)

#define HOST_ENDIAN_TO_BIG_32(x)            (x)
#define HOST_ENDIAN_TO_LITTLE_32(x)         REVERSE_ENDIAN_32(x)
#define HOST_ENDIAN_TO_LITTLE_32_UNSAFE(x)  REVERSE_ENDIAN_32_UNSAFE(x)

#else
#error "IENDIAN is not defined"
#endif


// Define the USB to host endian macros
#define HOST_ENDIAN_TO_USB_16(x)    HOST_ENDIAN_TO_LITTLE_16(x)
#define USB_ENDIAN_TO_HOST_16(x)    LITTLE_ENDIAN_TO_HOST_16(x)

#define HOST_ENDIAN_TO_USB_32(x)    HOST_ENDIAN_TO_LITTLE_32(x)
#define USB_ENDIAN_TO_HOST_32(x)    LITTLE_ENDIAN_TO_HOST_32(x)

#define HOST_ENDIAN_TO_USB_16_UNSAFE(x)    HOST_ENDIAN_TO_LITTLE_16_UNSAFE(x)
#define USB_ENDIAN_TO_HOST_16_UNSAFE(x)    LITTLE_ENDIAN_TO_HOST_16_UNSAFE(x)

#define HOST_ENDIAN_TO_USB_32_UNSAFE(x)    HOST_ENDIAN_TO_LITTLE_32_UNSAFE(x)
#define USB_ENDIAN_TO_HOST_32_UNSAFE(x)    LITTLE_ENDIAN_TO_HOST_32_UNSAFE(x)

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

// Gives a negative number if X < Y, 0 if X == Y or a positive number if X > Y
#define CMP(X, Y) (((X) > (Y)) - ((X) < (Y)))

// Gives the number of bytes required to store _size_ bytes of data that are
// aligned to an _align_bytes_ boundary.
#define ALIGNED_SIZE(_size_, _align_bytes_) \
    (_size_ + (_align_bytes_ - 1)) & (~(_align_bytes_ - 1))

#define IS_ALIGNED(x) ((((size_t)x) & (sizeof(size_t) - 1)) == 0)
#define IS_UNALIGNED(x) (!IS_ALIGNED(x))

#define ABSOLUTE_VALUE(a) ((a) < 0 ? -(a) : (a))

// Returns the value y such that lo <= y <= hi and |y-x| is minimized.
#define CLAMP(x, lo, hi) ((x) <= (lo) ? (lo) : (hi) <= (x) ? (hi) : (x))
/******************************** Data Types *********************************/

// a union that allows easy access to the internals of a 64 bit variable
// note that this is endian specific (ie var8[0] is different on a big endian
// versus little endian)
typedef union
{
    uint64_t    var64;
    uint32_t    var32[2];
    uint16_t    var16[4];
    uint8_t     var8[8];
} Var64Access;

// a union that allows easy access to the internals of a 32 bit variable
// note that this is endian specific
typedef union
{
    uint32_t    var32;
    uint16_t    var16[2];
    uint8_t     var8[4];   // big endian: var8[0] is least significant 8 bits on var32
} Var32Access;

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/

// Standard C functions that even the compiler creates calls to (for optimizations)
// Also used by leon startup code to copy code from flash to iram, and clear bss
//      So it should be in flash to be accessible by such early startup code
// NOTE: attribute used is to work around a GCC bug where it doesn't register the dependency on
//       memcpy and memset when it generates the call itself.  This can lead to problems when link
//       time optimization decides that memcpy isn't used.  For more detail, see:
//       http://stackoverflow.com/questions/28708234/how-to-fix-defined-in-discarded-section-linker-error
void * memset(void * dst, int c, size_t n) __attribute__ ((used, section(".ftext")));
void * memcpy(void * dst, const void * src, size_t num) __attribute__ ((used, section(".ftext")));

void * flashmemset(void * dst, int c, size_t n) __attribute__ ((section(".flashcode"), weak));
void * flashmemcpy(void * dst, const void * src, size_t num) __attribute__ ((section(".flashcode"), weak));

bool memeq(const void *src1, const void *src2, size_t n);

// alloca may already be defined by stdlib.h for tests which target the host system rather than the
// embedded device.
#ifndef alloca
// Use GCCs built-in alloca function for dynamic stack allocation
#define alloca(_size_) (__builtin_alloca(_size_))
#endif

#endif //#ifndef IBASE_H

