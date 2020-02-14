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
//!   @file  - xlr_msa.h
//
//!   @brief - MSA (Mass Storage Acceleration) routines
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XLR_MSA_H
#define XLR_MSA_H


/***************************** Included Headers ******************************/
#include <itypes.h>


/************************ Defined Constants and Macros ***********************/
// Internal definitions of the abstract type MSA_AddressT
#define MSA_LA_OFFSET       (0)
#define MSA_LA_WIDTH        (4)
#define MSA_USB_OFFSET      (MSA_LA_OFFSET + MSA_LA_WIDTH )
#define MSA_USB_WIDTH       (7)
#define MSA_VALID_OFFSET    (MSA_USB_OFFSET + MSA_USB_WIDTH)
#define MSA_VALID_WIDTH     (1)


/******************************** Data Types *********************************/
// abstract type for MSA_AddressT
struct MSA_Address;
typedef struct MSA_Address * MSA_AddressT;


/*********************************** API *************************************/
// MSA address operations
static inline void  XLR_msaAddressInit     (MSA_AddressT *) __attribute__ ((always_inline));
static inline void  XLR_msaAddressSetLA    (MSA_AddressT *, uint8) __attribute__ ((always_inline));
static inline void  XLR_msaAddressSetUSB   (MSA_AddressT *, uint8) __attribute__ ((always_inline));
static inline void  XLR_msaAddressSetValid (MSA_AddressT *, uint8) __attribute__ ((always_inline));
static inline uint8 XLR_msaAddressGetLA    (MSA_AddressT) __attribute__ ((always_inline));
static inline uint8 XLR_msaAddressGetUSB   (MSA_AddressT) __attribute__ ((always_inline));
static inline uint8 XLR_msaAddressGetValid (MSA_AddressT) __attribute__ ((always_inline));

// MSA Lat operations
MSA_AddressT    XLR_msaGetAddrFromMsaLat(uint8 usbAddr);
void            XLR_msaUpdateAddress(MSA_AddressT address);

// MSA pointer table operations
uint8           XLR_msaReadPtrTable(MSA_AddressT, uint8 endpoint);
void            XLR_msaWritePtrTable(MSA_AddressT, uint8 endpoint, uint8 ptr);
uint8           XLR_msaAllocatePtr(void);
void            XLR_msaFreePtr(uint8 ptr);

// MSA Status Table operations
void            XLR_msaClearStatusTable(MSA_AddressT, uint8 endpoint);

/************************ Static inline definitions **************************/
static inline void XLR_msaAddressInit(MSA_AddressT * arg) { *arg = (MSA_AddressT)0; }
static inline void XLR_msaAddressSetLA(MSA_AddressT * arg, uint8 arg2)
{
    uint32 mask = ((1 << MSA_LA_WIDTH) - 1);
    uint32 posMask = mask << MSA_LA_OFFSET;
    *arg = (MSA_AddressT)((*(uint32 *)arg & ~posMask) | ((arg2 & mask) << MSA_LA_OFFSET));
}
static inline void XLR_msaAddressSetUSB(MSA_AddressT * arg, uint8 arg2)
{
    uint32 mask = ((1 << MSA_USB_WIDTH) - 1);
    uint32 posMask = mask << MSA_USB_OFFSET;
    *arg = (MSA_AddressT)((*(uint32 *)arg & ~posMask) | ((arg2 & mask) << MSA_USB_OFFSET));
}
static inline void XLR_msaAddressSetValid(MSA_AddressT * arg, uint8 arg2)
{
    uint32 mask = ((1 << MSA_VALID_WIDTH) - 1);
    uint32 posMask = mask << MSA_VALID_OFFSET;
    *arg = (MSA_AddressT)((*(uint32 *)arg & ~posMask) | ((arg2 & mask) << MSA_VALID_OFFSET));
}
static inline uint8 XLR_msaAddressGetLA(MSA_AddressT arg)
{
    return ((uint32)arg >> MSA_LA_OFFSET) & ((1 << MSA_LA_WIDTH) - 1);
}
static inline uint8 XLR_msaAddressGetUSB(MSA_AddressT arg)
{
    return ((uint32)arg >> MSA_USB_OFFSET) & ((1 << MSA_USB_WIDTH) - 1);
}
static inline uint8 XLR_msaAddressGetValid(MSA_AddressT arg)
{
    return ((uint32)arg >> MSA_VALID_OFFSET) & ((1 << MSA_VALID_WIDTH) - 1);
}


#endif // XLR_MSA_H
