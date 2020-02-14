/**********************************************************************
 *
 * Filename:    crc.h
 *
 * Description: A header file describing the various CRC standards.
 *
 * Notes:
 *
 *
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/
#ifndef CRC_h
#define CRC_h

#include <itypes.h>

/*
 * Select the CRC standard from the list that follows.
 */
// ICRON change: setting CRC16
#define CRC32

// CRC16 for Atmel chip
typedef unsigned short  crc16;

#define CRC_NAME_16            "CRC16-16"
#define POLYNOMIAL_16          0x8005
#define INITIAL_REMAINDER_16   0x0000
#define FINAL_XOR_VALUE_16     0x0000
#define REFLECT_DATA_16        true
// ICRON change:  changing REFLECT_REMAINDER to false
#define REFLECT_REMAINDER_16   false
#define CHECK_VALUE_16         0xBB3D

#if defined(CRC8)

typedef unsigned char  crc;

#define CRC_NAME           "CRC-8/WCDMA"
#define POLYNOMIAL          0x9B
#define INITIAL_REMAINDER   0x00
#define FINAL_XOR_VALUE     0x00
#define REFLECT_DATA        true
// ICRON change:  changing REFLECT_REMAINDER to false
#define REFLECT_REMAINDER   false
#define CHECK_VALUE         0x25

#elif defined(CRC_CCITT)

typedef unsigned short  crc;

#define CRC_NAME           "CRC-CCITT"
#define POLYNOMIAL          0x1021
#define INITIAL_REMAINDER   0xFFFF
#define FINAL_XOR_VALUE     0x0000
#define REFLECT_DATA        false
#define REFLECT_REMAINDER   false
#define CHECK_VALUE         0x29B1

#elif defined(CRC16)

typedef unsigned short  crc;

#define CRC_NAME            "CRC-16"
#define POLYNOMIAL          0x8005
#define INITIAL_REMAINDER   0x0000
#define FINAL_XOR_VALUE     0x0000
#define REFLECT_DATA        true
// ICRON change:  changing REFLECT_REMAINDER to false
#define REFLECT_REMAINDER   false
#define CHECK_VALUE         0xBB3D

#elif defined(CRC32)

typedef uint32_t  crc32;

#define CRC_NAME            "CRC-32"
#define POLYNOMIAL          0x04C11DB7
#define INITIAL_REMAINDER   0xFFFFFFFF
#define FINAL_XOR_VALUE     0xFFFFFFFF
// ICRON change:  changing REFLECT_REMAINDER to false
#define REFLECT_DATA        true
#define REFLECT_REMAINDER   false
#define CHECK_VALUE         0xCBF43926

#else

#error "One of CRC8, CRC_CCITT, CRC16, or CRC32 must be #define'd."

#endif

typedef uint64_t  crc64;

#define CRC64_NAME            "ECMA-182CRC-64"
#define POLYNOMIAL64          0x42F0E1EBA9EA3692ULL
#define INITIAL_REMAINDER64   0xFFFFFFFFFFFFFFFFULL
#define FINAL_XOR_VALUE64     0xFFFFFFFFFFFFFFFFULL



void  crcInit(void);
void  crc16Init(void);
crc32   crcSlow(unsigned char const message[], int nBytes);
crc32   crcFast(unsigned char const message[], int nBytes);

crc64 crcFastInit(void);
crc64 crcFastBlock(unsigned char const message[], int nBytes, crc64 crcValue);
crc64 crcFastFinalize(crc64 crcValue);

crc16 crc16Slow(unsigned char const message[], uint32_t nBytes);
crc16 crc16Fast(unsigned char const message[], uint32_t nBytes);


#endif /* CRC_h */
