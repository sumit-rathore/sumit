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
//!   @file  -  xcsr_xsst.h
//
//!   @brief -  System status table drivers part of the XCSR
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XCSR_XSST_H
#define XCSR_XSST_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <ibase.h>
#include <xsst_fw.h>        // file generated by RTL team
#include <leon_mem_map.h>   // Needed by XUSB_addressT to convert packed pointers to DRAM pointers
#include <usbdefs.h>

/************************ Defined Constants and Macros ***********************/

// Endpoint definitions
#define MAX_ENDPOINTS 15


#define XSST_SW_VPORT_WIDTH (XSST_LAT_VPORT_WIDTH)


/******************************** Data Types *********************************/
struct XCSR_Xsst
{
    union
    {
        uint64 sst;
        struct //       bit             width                   description
        {
            // Upper 32 bits
            // XUSB RTL control only
            uint64      iBlk:           XSST_IBLK_WIDTH;        // In blocking behaviour
            uint64      oBlk:           XSST_OBLK_WIDTH;        // Out blocking behaviour
            uint64      iErrCnt:        XSST_IERR_CNT_WIDTH;    // Three strikes counter
            uint64      oErrCnt:        XSST_OERR_CNT_WIDTH;    // Three strikes counter
            uint64      iFwd2Cpu:       XSST_IFWD2CPU_WIDTH;
            uint64      oFwd2Cpu:       XSST_OFWD2CPU_WIDTH;
            // Shared CPU and XUSB control
            uint64      ovrLay:         XSST_OVRLAY_WIDTH;      // Changes dependent on different conditions of the endpoint
            uint64      iSplit:         XSST_ISPLIT_WIDTH;      // Set when split transaction is in flight
            uint64      oSplit:         XSST_OSPLIT_WIDTH;      // Set when split transaction is in flight
            uint64      iDcf:           XSST_IDCF_WIDTH;        // Device Class Filtering.  When set only do synthetic responses (IE NAK or ISO NULL DATA0)
            uint64      oDcf:           XSST_ODCF_WIDTH;        // Device Class Filtering.  When set only do synthetic responses (IE NAK or ISO NULL DATA0)
            uint64      iEpTypeSel:     XSST_IEPTYPESEL_WIDTH;  // Extract IN direction endpoint type from SPLIT packet instead of XSST
            uint64      oEpTypeSel:     XSST_OEPTYPESEL_WIDTH;  // Extract OUT direction endpoint type from SPLIT packet instead of XSST
            uint64      rtyUsage:       XSST_RTRY_USAGE_WIDTH;  // Indicates if this endpoint has data in the 2 retry buffers, bit1=RetryBuffer1, bit0=RetryBuffer0
            uint64      iAltClrRsp:     XSST_IFWD2CPUGATE_WIDTH;    // Chooses alternate response to use for a particular epType if iClr bits are set
            uint64      oAltClrRsp:     XSST_OFWD2CPUGATE_WIDTH;    // Chooses alternate response to use for a particular epType if oClr bits are set
            uint64      iClr:           XSST_ICLR_WIDTH;        // Causes LEX to apply stuck endpoint clearing rules to IN behaviour
            uint64      oClr:           XSST_OCLR_WIDTH;        // Causes LEX to apply stuck endpoint clearing rules to OUT behaviour
            uint64      iHstAcs:        XSST_IHST_ACS_WIDTH;    // Host access bit
            uint64      oHstAcs:        XSST_OHST_ACS_WIDTH;    // Host access bit
            // Next 32 bits
            uint64      iDet:           XSST_IDET_WIDTH;        // Processor stuck endpoint detection in the IN direction
            uint64      oDet:           XSST_ODET_WIDTH;        // Processor stuck endpoint detection in the OUT direction
            // Queue Management
            uint64      iQid:           XSST_IQID_WIDTH;        // Qid of the in response for the in endpoint
            uint64      oQid:           XSST_OQID_WIDTH;        // Qid of the out response for the out endpoint
            uint64      iNtfyCnt:       XSST_INTFYCNT_WIDTH;    // Incremented when XCRM posts a response to LEX, decremented when LEX returns a response to the host.
            uint64      oNtfyCnt:       XSST_ONTFYCNT_WIDTH;    // Incremented when XCRM posts a response to LEX, decremented when LEX returns a response to the host.
            // CPU access only
            uint64      iAccel:         XSST_IACCELMODE_WIDTH;  // Indicates the acceleration asociated with the OUT epType
            uint64      oAccel:         XSST_OACCELMODE_WIDTH;  // Indicates the acceleration asociated with the IN epType
            uint64      iEpType:        XSST_IEPTYPE_WIDTH;     // Endpoint type classification of IN packet types
            uint64      oEpType:        XSST_OEPTYPE_WIDTH;     // Endpoint type classification for the SETUP/OUT/PING packet types
        } sstStruct;
        union
        {
            struct
            {
                uint64      unused1:        (64 - XSST_CTRL_RSP_NULLD1_WIDTH - XSST_CTRL_RSP_NULLD1_OFFSET);
                uint64      ctrlRspNullD1:  XSST_CTRL_RSP_NULLD1_WIDTH;
                uint64      ctrlRspNullD0:  XSST_CTRL_RSP_NULLD0_WIDTH;
                uint64      ctrlRspStall:   XSST_CTRL_RSP_STALL_WIDTH;
                uint64      setupRspPndg:   XSST_SETUP_RSP_PNDG_WIDTH;
                uint64      setupRspTgl:    XSST_SETUP_RSP_TGL_WIDTH;
                uint64      insysOvrd:      XSST_INSYSOVRD_WIDTH;
                uint64      bci:            XSST_BCI_WIDTH;
                uint64      bco:            XSST_BCO_WIDTH;
                uint64      unused0:        (64 - XSST_OVRLAY_OFFSET);
            } ctrlEndPointStruct;
            struct
            {
                uint64      unused1:        (64 - (XSST_OMSA_EP_PAIR_OFFSET + XSST_OMSA_EP_PAIR_WIDTH));
                uint64      msaEpPair:      XSST_OMSA_EP_PAIR_WIDTH;
                uint64      unused0:        XSST_OMSA_EP_PAIR_OFFSET;
            } oBulkMsaEndPointStruct;
            struct
            {
                uint64      unused1:        (64 - (XSST_IMSA_EP_PAIR_OFFSET + XSST_IMSA_EP_PAIR_WIDTH));
                uint64      msaEpPair:      XSST_IMSA_EP_PAIR_WIDTH;
                uint64      unused0:        XSST_IMSA_EP_PAIR_OFFSET;
            } iBulkMsaEndPointStruct;
            struct
            {
                uint64      unused1:        (64 - XSST_INT_IPERBUFLMT_WIDTH - XSST_INT_IPERBUFLMT_OFFSET);
                uint64      iPerBufLimit:   XSST_INT_IPERBUFLMT_WIDTH;      // Periodic IN transaction buffer limit (INT and ISO)
                uint64      iPerBufState:   XSST_INT_IPERBUFSTATE_WIDTH;    // In periodic buffer state
                uint64      copyToCpu:      XSST_INT_COPY_TO_CPU_WIDTH;
                uint64      fullHalfRate:   XSST_INT_OPTNAK_FULHLFRTE_WIDTH;
                uint64      unused0:        XSST_INT_OPTNAK_FULHLFRTE_OFFSET;
            } iIntEndPointStruct;
            struct
            {
                uint64      unused1:        (64 - XSST_ISO_IPERBUFLMT_WIDTH - XSST_ISO_IPERBUFLMT_OFFSET);
                uint64      iPerBufLimit:   XSST_ISO_IPERBUFLMT_WIDTH;      // Periodic IN transaction buffer limit (INT and ISO)
                uint64      iPerBufState:   XSST_ISO_IPERBUFSTATE_WIDTH;    // In periodic buffer state
                uint64      unused0:        XSST_ISO_IPERBUFSTATE_OFFSET;
            } iIsoEndPointStruct;
        } ovrLay;
    };
};


struct XCSR_Lat
{
    union
    {
        uint32 lat;
        struct //       bit                     width                       description
        {
            uint32      unused:                 (32 - XSST_SW_VPORT_WIDTH   // Unused bits that HW doesn't even assign as reserved
                                                 - XSST_LAT_VFEN_WIDTH
                                                 - XSST_LAT_SPLIT_WIDTH
                                                 - XSST_LAT_INSYS_WIDTH
                                                 - XSST_LAT_LVAL_WIDTH
                                                 - XSST_LAT_LADDR_WIDTH);
            uint32      vport:                  XSST_SW_VPORT_WIDTH;        // Virtual port ID
            uint32      vfen:                   XSST_LAT_VFEN_WIDTH;        // Virtual function enabled indicator
            uint32      split:                  XSST_LAT_SPLIT_WIDTH;       // Split device indicator // NOTE: this only gets set on SetAddress, USB:0 will never be marked as split
            uint32      inSys:                  XSST_LAT_INSYS_WIDTH;       // In system
            uint32      logicalAddressValid:    XSST_LAT_LVAL_WIDTH;        // Logical address valid
            uint32      logicalAddress:         XSST_LAT_LADDR_WIDTH;       // Logical Address, assigned by the topology
        } latStruct;
    };
};


// XUSB_AddressT -- this is an abstract value with packed bitfields
typedef struct XUSB_Address * XUSB_AddressT;

#define logicalOffset   (0)
#define logicalWidth    (XSST_LAT_LADDR_WIDTH) // xsst_fw.h -- currently set to 5
#define vportOffset     (logicalOffset + logicalWidth)
#define vportWidth      (XSST_SW_VPORT_WIDTH) // xsst_fw.h -- currently set to 3
#define usbValidOffset  (vportOffset + vportWidth)
#define usbValidWidth   (1)
#define USBOffset       (usbValidOffset + usbValidWidth)
#define USBWidth        (7)   // From USB spec
#define virtFuncOffset  (USBOffset + USBWidth)
#define virtFuncWidth   (LEON_PACKED_DRAM_POINTER_BITS)
// inSys is packed at the 31st bit rather than the 30th to allow the compiler to do < 0 checks.
#define inSysOffset     (31)
#define inSysWidth      (1)

// Ensure this fits into the abstract type.  The "-1" is for the inSys bit which is placed in the
// 31st bit position.
static inline void XCSR_confirmXUSBAddrSize(void)
{
    COMPILE_TIME_ASSERT((virtFuncOffset + virtFuncWidth) <= ((sizeof(XUSB_AddressT) * 8) - 1));
}

static inline uint8 XCSR_getXUSBAddrVPort(XUSB_AddressT arg)
{
    return ((uint32)arg >> vportOffset) & ((1 << vportWidth) - 1);
}
static inline uint8 XCSR_getXUSBAddrUsb(XUSB_AddressT arg)
{
    return ((uint32)arg >> USBOffset) & ((1 << USBWidth) - 1);
}
static inline uint8 XCSR_getXUSBAddrValid(XUSB_AddressT arg)
{
    return ((uint32)arg >> usbValidOffset) & ((1 << usbValidWidth) - 1);
}
static inline uint8 XCSR_getXUSBAddrInSys(XUSB_AddressT arg)
{
    return ((uint32)arg >> inSysOffset) & ((1 << inSysWidth) - 1);
}
static inline uint8 XCSR_getXUSBAddrLogical(XUSB_AddressT arg)
{
    return ((uint32)arg >> logicalOffset) & ((1 << logicalWidth) - 1);
}
static inline void * XCSR_getXUSBAddrVirtFuncPointer(XUSB_AddressT arg)
{
    return LEON_unpackDRamPointer(((uint32)arg >> virtFuncOffset) & ((1 << virtFuncWidth) - 1));
}

static inline void XCSR_setXUSBAddrVPort(XUSB_AddressT* arg, uint8 arg2)
{
    uint32 mask = ((1 << vportWidth) - 1);
    uint32 posMask = mask << vportOffset;
    *arg = (XUSB_AddressT)((*(uint32 *)arg & ~posMask) | ((arg2 & mask) << vportOffset));
}
static inline void XCSR_setXUSBAddrUsb(XUSB_AddressT* arg, uint8 arg2) __attribute__((always_inline));
static inline void XCSR_setXUSBAddrUsb(XUSB_AddressT* arg, uint8 arg2)
{
    uint32 mask = ((1 << USBWidth) - 1);
    uint32 posMask = mask << USBOffset;
    *arg = (XUSB_AddressT)((*(uint32 *)arg & ~posMask) | ((arg2 & mask) << USBOffset));
}
static inline void XCSR_setXUSBAddrValid(XUSB_AddressT* arg, uint8 arg2)
{
    uint32 mask = ((1 << usbValidWidth) - 1);
    uint32 posMask = mask << usbValidOffset;
    *arg = (XUSB_AddressT)((*(uint32 *)arg & ~posMask) | ((arg2 & mask) << usbValidOffset));
}
static inline void XCSR_setXUSBAddrInSys(XUSB_AddressT* arg, uint8 arg2)
{
    uint32 mask = ((1 << inSysWidth) - 1);
    uint32 posMask = mask << inSysOffset;
    *arg = (XUSB_AddressT)((*(uint32 *)arg & ~posMask) | ((arg2 & mask) << inSysOffset));
}
static inline void XCSR_setXUSBAddrLogical(XUSB_AddressT* arg, uint8 arg2)
{
    uint32 mask = ((1 << logicalWidth) - 1);
    uint32 posMask = mask << logicalOffset;
    *arg = (XUSB_AddressT)((*(uint32 *)arg & ~posMask) | ((arg2 & mask) << logicalOffset));
}
// TODO: could add an assert to the next line to ensure that this pointer is in DRAM memory
static inline void XCSR_setXUSBAddrVirtFuncPointer(XUSB_AddressT* arg, void* arg2)
{
    uint32 mask = ((1 << virtFuncWidth) - 1);
    uint32 posMask = mask << virtFuncOffset;
    *arg = (XUSB_AddressT)((*(uint32 *)arg & ~posMask) | (LEON_packDRamPointer(arg2) << virtFuncOffset));
}

static inline void XCSR_initXUSBAddr(XUSB_AddressT* arg)
{
    *arg = (XUSB_AddressT)0;
}

/*********************************** API *************************************/
void XCSR_XSSTInit(void);

void XCSR_XSSTUpdateAddress(XUSB_AddressT address, boolT split);

void XCSR_XSSTAddr0SetInsys(void);
void XCSR_XSSTClearInsys(XUSB_AddressT address);
void XCSR_XSSTClearLAT(XUSB_AddressT address);
static inline void XCSR_XSSTIntentToClearLAT(XUSB_AddressT arg) { } // Not used in GE

XUSB_AddressT XCSR_XSSTGetXUSBAddrFromLAT(uint8 usbAddr) __attribute__ ((section (".lextext")));
uint32 XCSR_XSSTReadLogicalAddressTable(uint8 usbAddress);

uint64 XCSR_XSSTRead(uint8 usbAddress, uint8 endPoint) __attribute__ ((section (".lextext")));
uint64 XCSR_XSSTReadConditional(XUSB_AddressT address, uint8 endPoint);
boolT XCSR_XSSTWriteConditional(XUSB_AddressT address, uint8 endPoint, uint64 writeValue);

uint64 XCSR_XSSTWriteEndpoint(XUSB_AddressT address, uint8 endpointNumber, uint8 endpointType,
    enum EndpointDirection endpointDirection, boolT isHub, boolT isSplit, boolT isOtherDirectionMSA, boolT preserveMSA, boolT blockAccess);
void XCSR_XSSTConfigureMSA(XUSB_AddressT address, uint8 inEndpoint, uint8 outEndpoint);
void XCSR_XSSTClearOverlayAndAccel(XUSB_AddressT address, uint8 endpoint, enum EndpointDirection direction);
static inline void XCSR_XSSTResetEndpoint(XUSB_AddressT address, uint8 endpointNumber);
void XCSR_XSSTResetEndpoints(XUSB_AddressT address, uint8 LowEndpointNumber, uint8 HighEndpointNumber);
void XCSR_XSSTClearEndpointAccelField(XUSB_AddressT address, uint8 LowEndpointNumber, uint8 HighEndpointNumber);

void XCSR_XSSTClearBCO(XUSB_AddressT address) __attribute__ ((section (".lextext")));
void XCSR_XSSTClearBCI(XUSB_AddressT address) __attribute__ ((section (".lextext")));

uint64 XCSR_XSSTWriteMask(uint8 usbAddress, uint8 endPoint, uint64 writeValue, uint64 writeMask) __attribute__ ((section (".lextext")));

void XCSR_XSSTWriteLAT(uint8 usbAddress, uint32 value) __attribute__ ((section (".lextext")));

void XCSR_LATShowEntry(uint8 usbAddress);
void XCSR_XSSTShowEntry(XUSB_AddressT address);

void XCSR_ShowXUSBAddress(const XUSB_AddressT address);

/************************ Static inline definitions **************************/
static inline void XCSR_XSSTResetEndpoint(XUSB_AddressT address, uint8 endpointNumber)
{ XCSR_XSSTResetEndpoints(address, endpointNumber, endpointNumber); }

#endif // XCSR_XSST_H

