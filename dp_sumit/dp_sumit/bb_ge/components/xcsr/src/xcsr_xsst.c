///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
//!   @file  -  xcsr_xsst.c
//
//!   @brief -  System status table drivers as part of the XCSR
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "xcsr_loc.h"
#include <xlr.h> // For releasing retry buffers
#include <storage_Data.h>

/************************ Defined Constants and Macros ***********************/

// Access modes for the XSST
#define READ_ALL                0b0000  // Read the LAT, SST LSW and SST MSW
#define READ_ALL_COND           0b0001  // Read the LAT, SST LSW and SST MSW - Conditional Read
#define WRITE_LAT               0b0010  // Write the LAT
#define WRITE_SST_LSW           0b0011  // Write the LSW of the SST
#define WRITE_SST_MSW           0b0100  // Write the MSW of the SST
#define WRITE_SST_LSW_MSW       0b0101  // Write both the LSW and MSW of the SST
#define WRITE_SST_LSW_MSW_COND  0b0110  // Write both the LSW and MSW of the SST - Conditional Write
#define RMW_LAT                 0b0111  // Do a read-modify-write of the LAT
#define RMW_SST_LSW             0b1000  // Do a read-modify-write of the LSW of the SST
#define RMW_SST_MSW             0b1001  // Do a read-modify-write of the MSW of the SST
#define RMW_SST_LSW_MSW         0b1010  // Do a read-modify-write of both the LSW and MSW of the SST
/* 0b1011 - Reserved */
/* 0b1100 - Reserved */
/* 0b1101 - Reserved */
/* 0b1110 - Reserved */
/* 0b1111 - Reserved */


// Read XSST definitions
#define READ_LAT_DATA       XCSR_XSST_SSTSTATUS2_READ_REG
#define READ_XSST_DATA_MSW  XCSR_XSST_SSTSTATUS1_READ_REG
#define READ_XSST_DATA_LSW  XCSR_XSST_SSTSTATUS0_READ_REG
// Scratch register definitions
#define WRITE_LAT_DATA       XCSR_XUSB_SCRATCH0_WRITE_REG
#define WRITE_XSST_DATA_LSW  XCSR_XUSB_SCRATCH0_WRITE_REG
#define WRITE_XSST_DATA_MSW  XCSR_XUSB_SCRATCH1_WRITE_REG
#define WRITE_LAT_MASK       XCSR_XUSB_SCRATCH2_WRITE_REG
#define WRITE_XSST_MASK_LSW  XCSR_XUSB_SCRATCH2_WRITE_REG
#define WRITE_XSST_MASK_MSW  XCSR_XUSB_SCRATCH3_WRITE_REG


// Values to be used in the i/oFwd2Cpu fields of the XSST
enum _XCSR_XSST_Fwd2CpuType
{
    XCSR_XSST_FWD2CPU_NO_FORWARDING,
    XCSR_XSST_FWD2CPU_DNS_TO_Q,
    XCSR_XSST_FWD2CPU_DNS_TO_Q_UPS_DELAYED,
    XCSR_XSST_FWD2CPU_DNS_TO_Q_UPS_TO_Q
};

/**
* FUNCTION NAME: _XSST_RMW()
*
* @brief  - A macro to help with XSST Read-Modify-Writes
*
* @return - old value pre-modify and writeback, when getValArg is set, otherwise returns 0
*
* @note   - This is to simplify to the caller when to use which access mode
*
*           Setting the mask to 0, will perform a read
*           Setting the mask to ~0ULL will perform a write, unless getValArg is set
*           Otherwise do a read-modify-write
*
*           This won't work with the LAT
*
*/
#if 1 // Not using the macro, as the compiler is having difficulties determining when values are known at compile time
#define _XSST_RMW(usbAddressArg, endPointArg, mask64arg, val64arg, getValArg)               \
    ((void)getValArg, XCSR_XSSTWriteMask(usbAddressArg, endPointArg, val64arg, mask64arg))
#else
#define _XSST_RMW(usbAddressArg, endPointArg, mask64arg, val64arg, getValArg)               \
({                                                                                          \
    const uint8 _usbAddress = usbAddressArg;                                                \
    const uint8 _endPoint = endPointArg;                                                    \
    const uint64 _mask = mask64arg;                                                         \
    const uint64 _val = val64arg;                                                           \
    const boolT getVal = getValArg;                                                         \
    const uint32 maskLsw = _mask & 0xFFFFFFFF;                                              \
    const uint32 valLsw = _val & 0xFFFFFFFF;                                                \
    const uint32 maskMsw = _mask >> 32;                                                     \
    const uint32 valMsw = _val >> 32;                                                       \
                                                                                            \
    uint32 controlVal = 0UL;                                                                \
    uint32 oldValueLsw = 0;                                                                 \
    uint32 oldValueMsw = 0;                                                                 \
                                                                                            \
    /* Ensure the mask is known at compile time, otherwise why call this helper function */ \
    COMPILE_TIME_ASSERT(__builtin_constant_p(mask64arg));                                   \
                                                                                            \
    /* Ensure no bits outside of the XSST are going to be set */                            \
    COMPILE_TIME_ASSERT((~CREATE_MASK(XSST_RECORD_WIDTH, 0, uint64) & _mask) == 0);         \
                                                                                            \
    controlVal = XCSR_XSST_CONTROL_DEVADDR_SET_BF(controlVal, _usbAddress);                 \
    controlVal = XCSR_XSST_CONTROL_DEVENDPT_SET_BF(controlVal, _endPoint);                  \
    controlVal = XCSR_XSST_CONTROL_GO_SET_BF(controlVal, 1);                                \
                                                                                            \
                                                                                            \
    if (maskMsw == 0UL)                                                                     \
    {                                                                                       \
        if (maskLsw == 0UL)                                                                 \
        {                                                                                   \
            /* read  */                                                                     \
            /* It would be useful to check if getVal is set, but a compile time check */    \
            /* will fail here, as this code will be compiled when this path is not taken */ \
            controlVal = XCSR_XSST_CONTROL_ACCMODE_SET_BF(controlVal, READ_ALL);            \
        }                                                                                   \
        else if ((maskLsw == ~0UL) && !getVal)                                              \
        {                                                                                   \
            /* write LSW */                                                                 \
            WRITE_XSST_DATA_LSW(XCSR_BASE_ADDR, valLsw);                                    \
            controlVal = XCSR_XSST_CONTROL_ACCMODE_SET_BF(controlVal, WRITE_SST_LSW);       \
        }                                                                                   \
        else                                                                                \
        {                                                                                   \
            /* RMW of LSW */                                                                \
            WRITE_XSST_MASK_LSW(XCSR_BASE_ADDR, maskLsw);                                   \
            WRITE_XSST_DATA_LSW(XCSR_BASE_ADDR, valLsw);                                    \
            controlVal = XCSR_XSST_CONTROL_ACCMODE_SET_BF(controlVal, RMW_SST_LSW);         \
        }                                                                                   \
    }                                                                                       \
    else if (maskLsw == 0UL)                                                                \
    {                                                                                       \
        if ((maskMsw == ~0UL) && !getVal)                                                   \
        {                                                                                   \
            /* write MSW */                                                                 \
            WRITE_XSST_DATA_MSW(XCSR_BASE_ADDR, valMsw);                                    \
            controlVal = XCSR_XSST_CONTROL_ACCMODE_SET_BF(controlVal, WRITE_SST_MSW);       \
        }                                                                                   \
        else                                                                                \
        {                                                                                   \
            /* RMW of MSW */                                                                \
            WRITE_XSST_MASK_MSW(XCSR_BASE_ADDR, maskMsw);                                   \
            WRITE_XSST_DATA_MSW(XCSR_BASE_ADDR, valMsw);                                    \
            controlVal = XCSR_XSST_CONTROL_ACCMODE_SET_BF(controlVal, RMW_SST_MSW);         \
        }                                                                                   \
    }                                                                                       \
    else if ((maskMsw == ~0UL) && (maskLsw == ~0UL) && !getVal)                             \
    {                                                                                       \
        /* Full Write */                                                                    \
        WRITE_XSST_DATA_MSW(XCSR_BASE_ADDR, valMsw);                                        \
        WRITE_XSST_DATA_LSW(XCSR_BASE_ADDR, valLsw);                                        \
        controlVal = XCSR_XSST_CONTROL_ACCMODE_SET_BF(controlVal, WRITE_SST_LSW_MSW);       \
    }                                                                                       \
    else                                                                                    \
    {                                                                                       \
        /* Full RMW */                                                                      \
        WRITE_XSST_MASK_MSW(XCSR_BASE_ADDR, maskMsw);                                       \
        WRITE_XSST_DATA_MSW(XCSR_BASE_ADDR, valMsw);                                        \
        WRITE_XSST_MASK_LSW(XCSR_BASE_ADDR, maskLsw);                                       \
        WRITE_XSST_DATA_LSW(XCSR_BASE_ADDR, valLsw);                                        \
        controlVal = XCSR_XSST_CONTROL_ACCMODE_SET_BF(controlVal, RMW_SST_LSW_MSW);         \
    }                                                                                       \
                                                                                            \
    /* Perform operation */                                                                 \
    writeControlAndBlock(controlVal);                                                       \
                                                                                            \
    if (getVal)                                                                             \
    {                                                                                       \
        /* read back the value */                                                           \
        oldValueLsw = READ_XSST_DATA_LSW(XCSR_BASE_ADDR);                                   \
        oldValueMsw = READ_XSST_DATA_MSW(XCSR_BASE_ADDR);                                   \
    }                                                                                       \
                                                                                            \
    MAKE_U64(oldValueMsw, oldValueLsw); /* return from expression */                        \
})
#endif

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static uint32 writeControlAndBlock(uint32 controlWriteValue) __attribute__ ((section (".ftext"), noinline));
static void XSSTBitFieldSanityCheck(void);
static void LATBitFieldSanityCheck(void);


/************************** Function Definitions *****************************/
/**
* FUNCTION NAME: writeControlAndBlock()
*
* @brief  - Write to the control register and block until the go bit is cleared
*
* @return - The value read from the control register
*
* @note   - This is an excellent candidate to be called as a sibling tail call
*
*/
static uint32 writeControlAndBlock
(
    uint32 controlWriteValue // Value to write to the control register
)
{
    uint32 controlReadValue;

    XCSR_XSST_CONTROL_WRITE_REG(XCSR_BASE_ADDR, controlWriteValue);

    do {
        controlReadValue = XCSR_XSST_CONTROL_READ_REG(XCSR_BASE_ADDR);
    } while (XCSR_XSST_CONTROL_GO_GET_BF(controlReadValue));

    return controlReadValue;
}



/**
* FUNCTION NAME: XCSR_XSSTInit()
*
* @brief  - Initialize the system status table by ensuring it is clear
*
* @return - nothing
*
* @note   -
*
*/
void XCSR_XSSTInit(void)
{
    // Check sanity of XSST & LAT bitfields
    XSSTBitFieldSanityCheck();
    LATBitFieldSanityCheck();

    // ensure that the xsst module power on reset has finished.
    while (0 == XCSR_XSST_CONTROL_INITDONE_READ_BF(XCSR_BASE_ADDR))
        ;
}



/**
* FUNCTION NAME: XSSTBitFieldSanityCheck()
*
* @brief  - Contains compile time checks to ensure the generated defines are in sync with the bitfields
*
* @return - void
*
* @note   - This uses a local helper macro to make life easier
*
*           The structure is defined with widths from the generated defines,
*           so here we are verifying that the positions are correct.
*
*           This check should really be a compile time assert, but GCC can't seem to figure it out at compile time
*           Even so when the assert won't trigger, GCC optimizes this function away
*/
static void XSSTBitFieldSanityCheck(void)
{
    // define helper macro
#define _XSST_CHECK_BIT(_struct_, bitfield, offset) do                                          \
    {                                                                                           \
        /* Local structures for comparisons */                                                  \
        struct XCSR_Xsst defs;                                                                  \
        struct XCSR_Xsst bits;                                                                  \
        \
        /* Clear structures */                                                                  \
        defs.sst = 0;                                                                           \
        bits.sst = 0;                                                                           \
        \
        /* Set bits that should be identical in both */                                         \
        defs.sst = 1ULL << (offset);                                                            \
        bits._struct_.bitfield = 1;                                                             \
        \
        /* Verify that the bits were identical */                                               \
        iassert_XCSR_COMPONENT_1(defs.sst == bits.sst, XSST_BITFIELD_CHECK_FAILURE, __LINE__);  \
    } while (FALSE)

    // Check each bitfield in the main XSST bitfield structure
    // Upper 32 bits
    // XUSB RTL control only
    _XSST_CHECK_BIT(sstStruct,  iBlk,           XSST_IBLK_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oBlk,           XSST_OBLK_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  iErrCnt,        XSST_IERR_CNT_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oErrCnt,        XSST_OERR_CNT_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  iFwd2Cpu,       XSST_IFWD2CPU_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oFwd2Cpu,       XSST_OFWD2CPU_OFFSET);
    // Shared CPU and XUSB control
    _XSST_CHECK_BIT(sstStruct,  ovrLay,         XSST_OVRLAY_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  iSplit,         XSST_ISPLIT_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oSplit,         XSST_OSPLIT_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  iDcf,           XSST_IDCF_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oDcf,           XSST_ODCF_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  iEpTypeSel,     XSST_IEPTYPESEL_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oEpTypeSel,     XSST_OEPTYPESEL_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  rtyUsage,       XSST_RTRY_USAGE_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  iAltClrRsp,     XSST_IFWD2CPUGATE_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oAltClrRsp,     XSST_OFWD2CPUGATE_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  iClr,           XSST_ICLR_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oClr,           XSST_OCLR_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  iHstAcs,        XSST_IHST_ACS_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oHstAcs,        XSST_OHST_ACS_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  iDet,           XSST_IDET_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oDet,           XSST_ODET_OFFSET);
    // Next 32 bits
    // Queue management
    _XSST_CHECK_BIT(sstStruct,  iQid,           XSST_IQID_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oQid,           XSST_OQID_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  iNtfyCnt,       XSST_INTFYCNT_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oNtfyCnt,       XSST_ONTFYCNT_OFFSET);
    // CPU access only
    _XSST_CHECK_BIT(sstStruct,  iAccel,         XSST_IACCELMODE_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oAccel,         XSST_OACCELMODE_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  iEpType,        XSST_IEPTYPE_OFFSET);
    _XSST_CHECK_BIT(sstStruct,  oEpType,        XSST_OEPTYPE_OFFSET);

    // Check each bitfield in the control endpoint overlay field
    _XSST_CHECK_BIT(ovrLay.ctrlEndPointStruct, ctrlRspNullD1,   XSST_CTRL_RSP_NULLD1_OFFSET);
    _XSST_CHECK_BIT(ovrLay.ctrlEndPointStruct, ctrlRspNullD0,   XSST_CTRL_RSP_NULLD0_OFFSET);
    _XSST_CHECK_BIT(ovrLay.ctrlEndPointStruct, ctrlRspStall,    XSST_CTRL_RSP_STALL_OFFSET);
    _XSST_CHECK_BIT(ovrLay.ctrlEndPointStruct, setupRspPndg,    XSST_SETUP_RSP_PNDG_OFFSET);
    _XSST_CHECK_BIT(ovrLay.ctrlEndPointStruct, setupRspTgl,     XSST_SETUP_RSP_TGL_OFFSET);
    _XSST_CHECK_BIT(ovrLay.ctrlEndPointStruct, insysOvrd,       XSST_INSYSOVRD_OFFSET);
    _XSST_CHECK_BIT(ovrLay.ctrlEndPointStruct, bci,             XSST_BCI_OFFSET);
    _XSST_CHECK_BIT(ovrLay.ctrlEndPointStruct, bco,             XSST_BCO_OFFSET);

    // Check each bitfield in the bulk MSA endpoint overlay field
    _XSST_CHECK_BIT(ovrLay.oBulkMsaEndPointStruct, msaEpPair, XSST_OMSA_EP_PAIR_OFFSET);
    _XSST_CHECK_BIT(ovrLay.iBulkMsaEndPointStruct, msaEpPair, XSST_IMSA_EP_PAIR_OFFSET);

    // Check each bitfield in the interrupt endpoint overlay field
    _XSST_CHECK_BIT(ovrLay.iIntEndPointStruct,  iPerBufState,   XSST_INT_IPERBUFSTATE_OFFSET);
    _XSST_CHECK_BIT(ovrLay.iIntEndPointStruct,  iPerBufLimit,   XSST_INT_IPERBUFLMT_OFFSET);
    _XSST_CHECK_BIT(ovrLay.iIntEndPointStruct,  copyToCpu,      XSST_INT_COPY_TO_CPU_OFFSET);
    _XSST_CHECK_BIT(ovrLay.iIntEndPointStruct,  fullHalfRate,   XSST_INT_OPTNAK_FULHLFRTE_OFFSET);

    // Check each bitfield in the isochronous endpoint overlay field
    _XSST_CHECK_BIT(ovrLay.iIsoEndPointStruct, iPerBufState, XSST_ISO_IPERBUFSTATE_OFFSET);
    _XSST_CHECK_BIT(ovrLay.iIsoEndPointStruct, iPerBufLimit, XSST_ISO_IPERBUFLMT_OFFSET);

#undef _XSST_CHECK_BIT
}

/**
* FUNCTION NAME: LATBitFieldSanityCheck()
*
* @brief  - Contains compile time checks to ensure the 2 generated defines are in sync with each other
*
* @return - void
*
* @note   - 1) Ensure the read bits from Spectareg, match the write bits from xsst_fw.h
*           2) Ensure the LAT bitfield struct matches the defines in xsst_fw.h
*
*/
static void LATBitFieldSanityCheck(void)
{
    iassert_XCSR_COMPONENT_1(XCSR_XSST_SSTSTATUS2_VPORT_BF_MASK == CREATE_MASK(XSST_SW_VPORT_WIDTH, XSST_LAT_VPORT_OFFSET, uint32), LAT_BITFIELD_CHECK_FAILURE, __LINE__);
    iassert_XCSR_COMPONENT_1(XCSR_XSST_SSTSTATUS2_VFNEN_BF_MASK == CREATE_MASK(XSST_LAT_VFEN_WIDTH,  XSST_LAT_VFEN_OFFSET,  uint32), LAT_BITFIELD_CHECK_FAILURE, __LINE__);
    iassert_XCSR_COMPONENT_1(XCSR_XSST_SSTSTATUS2_SPLIT_BF_MASK == CREATE_MASK(XSST_LAT_SPLIT_WIDTH, XSST_LAT_SPLIT_OFFSET, uint32), LAT_BITFIELD_CHECK_FAILURE, __LINE__);
    iassert_XCSR_COMPONENT_1(XCSR_XSST_SSTSTATUS2_INSYS_BF_MASK == CREATE_MASK(XSST_LAT_INSYS_WIDTH, XSST_LAT_INSYS_OFFSET, uint32), LAT_BITFIELD_CHECK_FAILURE, __LINE__);
    iassert_XCSR_COMPONENT_1(XCSR_XSST_SSTSTATUS2_LAVAL_BF_MASK == CREATE_MASK(XSST_LAT_LVAL_WIDTH,  XSST_LAT_LVAL_OFFSET,  uint32), LAT_BITFIELD_CHECK_FAILURE, __LINE__);
    iassert_XCSR_COMPONENT_1(XCSR_XSST_SSTSTATUS2_LADDR_BF_MASK == CREATE_MASK(XSST_LAT_LADDR_WIDTH, XSST_LAT_LADDR_OFFSET, uint32), LAT_BITFIELD_CHECK_FAILURE, __LINE__);

    // define helper macro
#define _LAT_CHECK_BIT(bitfield, offset) do                                                     \
    {                                                                                           \
        /* Local structures for comparisons */                                                  \
        struct XCSR_Lat defs;                                                                   \
        struct XCSR_Lat bits;                                                                   \
                                                                                                \
        /* Clear structures */                                                                  \
        defs.lat = 0;                                                                           \
        bits.lat = 0;                                                                           \
                                                                                                \
        /* Set bits that should be identical in both */                                         \
        defs.lat = 1ULL << (offset);                                                            \
        bits.latStruct.bitfield = 1;                                                            \
                                                                                                \
        /* Verify that the bits were identical */                                               \
        iassert_XCSR_COMPONENT_1(defs.lat == bits.lat, LAT_BITFIELD_CHECK_FAILURE, __LINE__);   \
    } while (FALSE)

    // Check each bitfield in the bitfield structure
    _LAT_CHECK_BIT(vport, XSST_LAT_VPORT_OFFSET);
    _LAT_CHECK_BIT(vfen, XSST_LAT_VFEN_OFFSET);
    _LAT_CHECK_BIT(split, XSST_LAT_SPLIT_OFFSET);
    _LAT_CHECK_BIT(inSys, XSST_LAT_INSYS_OFFSET);
    _LAT_CHECK_BIT(logicalAddressValid, XSST_LAT_LVAL_OFFSET);
    _LAT_CHECK_BIT(logicalAddress, XSST_LAT_LADDR_OFFSET);

#undef _LAT_CHECK_BIT
}


/**
* FUNCTION NAME: XCSR_XSSTUpdateAddress()
*
* @brief  - Writes to the logical address table. The logical address is entered
*           into the table at usb address and the split is set accordingly. Also
*           ensures the address is in-sys.
*
* @return - nothing
*
* @note   - logical address table does usb->logical
*
* @TODO   - Could this be a straight write, instead of a R-M-W
*
*/
void XCSR_XSSTUpdateAddress
(
    XUSB_AddressT address,  // Address of the device to write the usb address to
    boolT split  // Set split bit
)
{
    uint32 tempValue = 0;
    struct XCSR_Lat value;
    struct XCSR_Lat mask;


    ilog_XCSR_COMPONENT_3(ILOG_MAJOR_EVENT, UPDATE_ADDRESS, XCSR_getXUSBAddrUsb(address), XCSR_getXUSBAddrLogical(address), XCSR_getXUSBAddrVPort(address));

    // Write the mask
    mask.lat = 0;
    mask.latStruct.split = ~0;
    mask.latStruct.logicalAddress = ~0;
    mask.latStruct.logicalAddressValid = ~0;
    mask.latStruct.inSys = ~0;
    mask.latStruct.vport = ~0;
    mask.latStruct.vfen = ~0;
    WRITE_LAT_MASK(XCSR_BASE_ADDR, mask.lat);

    // Write the data
    value.lat = 0;
    value.latStruct.split = split;
    value.latStruct.logicalAddress = XCSR_getXUSBAddrLogical(address);
    value.latStruct.logicalAddressValid = 1;
    value.latStruct.inSys = XCSR_getXUSBAddrInSys(address);
    value.latStruct.vport = XCSR_getXUSBAddrVPort(address);
    value.latStruct.vfen = (XCSR_getXUSBAddrVirtFuncPointer(address) != NULL) ? ~0 : 0;
    WRITE_LAT_DATA(XCSR_BASE_ADDR, value.lat);

    tempValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(tempValue, XCSR_getXUSBAddrUsb(address));
    tempValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(tempValue, RMW_LAT);
    tempValue = XCSR_XSST_CONTROL_GO_SET_BF(tempValue, 1);
    writeControlAndBlock(tempValue);
}


/**
* FUNCTION NAME: XCSR_XSSTClearInsys()
*
* @brief  - Clears the in-sys bit for the given XUSB address
*
* @return - nothing
*
* @note   - logical address table does usb->logical
*
*/
void XCSR_XSSTClearInsys
(
    XUSB_AddressT address  // Address of the device to clear in-sys
)
{
    uint32 tempValue = 0;
    struct XCSR_Lat mask;

    mask.lat = 0;
    mask.latStruct.inSys = ~0;
    WRITE_LAT_MASK(XCSR_BASE_ADDR, mask.lat);
    WRITE_LAT_DATA(XCSR_BASE_ADDR, 0);

    tempValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(tempValue, XCSR_getXUSBAddrUsb(address));
    tempValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(tempValue, RMW_LAT);
    tempValue = XCSR_XSST_CONTROL_GO_SET_BF(tempValue, 1);
    writeControlAndBlock(tempValue);
}


/**
* FUNCTION NAME: XCSR_XSSTClearLAT()
*
* @brief  - Writes to the logical address table. The logical address entry is
*           cleared at usb address and clears all the bits.
*
* @return - nothing
*
* @note   - logical address table does usb->logical
*
*/
void XCSR_XSSTClearLAT
(
    XUSB_AddressT address  // Address of the device to clear
)
{
    uint32 tempValue = 0;

    WRITE_LAT_DATA(XCSR_BASE_ADDR, 0);

    tempValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(tempValue, XCSR_getXUSBAddrUsb(address));
    tempValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(tempValue, WRITE_LAT);
    tempValue = XCSR_XSST_CONTROL_GO_SET_BF(tempValue, 1);
    writeControlAndBlock(tempValue);
}


/**
* FUNCTION NAME: XCSR_XSSTGetXUSBAddrFromLAT()
*
* @brief  - Gets an XUSB_AddressT from reading the Logical Address Table
*
* @return - nothing
*
* @note   -
*
*/
XUSB_AddressT XCSR_XSSTGetXUSBAddrFromLAT
(
    uint8 usbAddr   // Address of the device to read
)
{
    uint32 tempValue = 0;
    XUSB_AddressT address;
    struct XCSR_Lat readLat;

    iassert_XCSR_COMPONENT_1(usbAddr <= MAX_USB_ADDRESS, XCSR_READ_LOOKUP_TABLE_INVALID_ARG_ERROR_LOG, __LINE__);

    tempValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(tempValue, usbAddr);
    tempValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(tempValue, READ_ALL);
    tempValue = XCSR_XSST_CONTROL_GO_SET_BF(tempValue, 1);
    writeControlAndBlock(tempValue);

    readLat.lat = READ_LAT_DATA(XCSR_BASE_ADDR);

    XCSR_initXUSBAddr(&address);
    XCSR_setXUSBAddrUsb(&address, usbAddr);
    XCSR_setXUSBAddrValid(&address, readLat.latStruct.logicalAddressValid);
    XCSR_setXUSBAddrLogical(&address, readLat.latStruct.logicalAddress);
    XCSR_setXUSBAddrInSys(&address, readLat.latStruct.inSys);
    XCSR_setXUSBAddrVPort(&address, readLat.latStruct.vport);

    return address;
}


/**
* FUNCTION NAME: XCSR_XSSTReadLogicalAddressTable()
*
* @brief  - Read the entire logical address table
*
* @return - Logical address table
*
* @note   - LAT is only 13 bits
*
*/
uint32 XCSR_XSSTReadLogicalAddressTable
(
    uint8 usbAddress  // Address of the device to read
)
{
    uint32 tempValue = 0;

    iassert_XCSR_COMPONENT_1(usbAddress <= MAX_USB_ADDRESS, XCSR_READ_LOOKUP_TABLE_INVALID_ARG_ERROR_LOG, __LINE__);

    tempValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(tempValue, usbAddress);
    tempValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(tempValue, READ_ALL);
    tempValue = XCSR_XSST_CONTROL_GO_SET_BF(tempValue, 1);
    writeControlAndBlock(tempValue);

    return READ_LAT_DATA(XCSR_BASE_ADDR);
}


/**
* FUNCTION NAME: XCSR_XSSTRead()
*
* @brief  - Read the XSST for the given address, endpoint
*
* @return - nothing
*
* @note   -
*
*/
uint64 XCSR_XSSTRead
(
    uint8 usbAddress,  //Address to read from
    uint8 endPoint  //Endpoint to read from
)
{
    return _XSST_RMW(usbAddress, endPoint, 0, 0, TRUE);
}


/**
* FUNCTION NAME: XCSR_XSSTReadConditional()
*
* @brief  - Read the XSST for the given address, endpoint
*
* @return - read value
*
* @note   - conditional read must be followed by a conditional write, conditional read sets a flag on the XSST
*           entry and any access to that entry (read or write) will clear the flag
*
*/
uint64 XCSR_XSSTReadConditional
(
    XUSB_AddressT address,  //Address to read from
    uint8 endPoint  //Endpoint to read from
)
{
    uint32 tempValue = 0;
    uint32 readValueLSW;
    uint32 readValueMSW;

    iassert_XCSR_COMPONENT_2(endPoint <= MAX_ENDPOINTS, XCSR_INVALID_ENDPOINT_NUMBER, endPoint, __LINE__);

    tempValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(tempValue, XCSR_getXUSBAddrUsb(address));
    tempValue = XCSR_XSST_CONTROL_DEVENDPT_SET_BF(tempValue, endPoint);
    tempValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(tempValue, READ_ALL_COND);
    tempValue = XCSR_XSST_CONTROL_GO_SET_BF(tempValue, 1);
    writeControlAndBlock(tempValue);

    readValueLSW = READ_XSST_DATA_LSW(XCSR_BASE_ADDR);
    readValueMSW = READ_XSST_DATA_MSW(XCSR_BASE_ADDR);

    return MAKE_U64(readValueMSW, readValueLSW);
}


/**
* FUNCTION NAME: XCSR_XSSTWriteConditional()
*
* @brief  - Conditionally write to the XSST for the given address, endpoint
*
* @return - TRUE if write completed successfully, FALSE otherwise
*
* @note   - conditional write must be preceded by a conditional read, conditional read sets a flag on the XSST
*           entry and any access to that entry (read or write) will clear the flag; on a conditional write if
*           the flag is cleared the write will fail
*
*/
boolT XCSR_XSSTWriteConditional
(
    XUSB_AddressT address, //Address to write to
    uint8 endPoint,        //Endpoint to write to
    uint64 writeValue      //xsst status to write
)
{
    uint32 tempValue = 0;
    boolT writeFailed = 0;
    const uint32 writeValueLSW = writeValue & 0xFFFFFFFF;
    const uint32 writeValueMSW = writeValue >> 32;

    iassert_XCSR_COMPONENT_2(endPoint <= MAX_ENDPOINTS, XCSR_INVALID_ENDPOINT_NUMBER, endPoint, __LINE__);

    // Set the value to write
    WRITE_XSST_DATA_LSW(XCSR_BASE_ADDR, writeValueLSW);
    WRITE_XSST_DATA_MSW(XCSR_BASE_ADDR, writeValueMSW);

    tempValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(tempValue, XCSR_getXUSBAddrUsb(address));
    tempValue = XCSR_XSST_CONTROL_DEVENDPT_SET_BF(tempValue, endPoint);
    tempValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(tempValue, WRITE_SST_LSW_MSW_COND);
    tempValue = XCSR_XSST_CONTROL_GO_SET_BF(tempValue, 1);
    tempValue = writeControlAndBlock(tempValue);

    writeFailed = XCSR_XSST_CONTROL_WRFAIL_GET_BF(tempValue);
    if(writeFailed)
    {
        ilog_XCSR_COMPONENT_3(
            ILOG_MINOR_EVENT, XSST_CONDITIONAL_WRITE_FAILURE,
            XCSR_getXUSBAddrLogical(address), XCSR_getXUSBAddrUsb(address), endPoint);
    }
    return !writeFailed;
}


/**
* FUNCTION NAME: XCSR_XSSTWriteEndpoint()
*
* @brief  - Write to the XSST for the given address, endpoint
*
* @return - old XSST value
*
* @note   - writing to the XSST is always a read-mod-write
*           Never ever touch the setup response toggle bit!
*
*           TODO: should this just return the old epType?
*/
uint64 XCSR_XSSTWriteEndpoint
(
    XUSB_AddressT address,     // xusb address of the device to write to
    uint8 endpointNumber,      // endpoint number to write to
    uint8 endpointType,        // endpoint type to write
    enum EndpointDirection endpointDirection, // endpoint direction
    boolT isHub,               // is this device a hub?
    boolT isSplit,             // is this device operating as a split device (ie USB1.1 over USB2.0)
    boolT isOtherDirectionMSA, // Set if the other direction of this endpoint is part of an MSA pair
    boolT preserveMSA,         // Set if we are allowed to write MSA info for this endpoint
    boolT blockAccess          // Block access to this endpoint using i/oDcf
)
{
    struct XCSR_Xsst value;
    struct XCSR_Xsst mask;

    iassert_XCSR_COMPONENT_2(
        ((endpointNumber != 0) && (endpointNumber <= MAX_ENDPOINTS)),
        XCSR_INVALID_ENDPOINT_NUMBER, endpointNumber, __LINE__);

    // Setup the endpoint entry
    mask.sst = 0;
    value.sst = 0;

    if (endpointDirection == EP_DIRECTION_IN)
    {
        value.sstStruct.iEpType = endpointType;
        mask.sstStruct.iEpType = ~0;
        value.sstStruct.iAltClrRsp = 0;  // alternate response bit is always cleared initially to send a NAK
        mask.sstStruct.iAltClrRsp = ~0;
        mask.sstStruct.iAccel = ~0;      // have the mask clear the whole accel field, or change below if MSA
        value.sstStruct.iDcf = blockAccess ? 1 : 0;
        mask.sstStruct.iDcf = ~0;        // Ensure this endpoint is active
    }
    else
    {
        value.sstStruct.oEpType = endpointType;
        mask.sstStruct.oEpType = ~0;
        value.sstStruct.oAltClrRsp = 0;  // alternate response bit is always cleared initially to send a NAK
        mask.sstStruct.oAltClrRsp = ~0;
        mask.sstStruct.oAccel = ~0;      // have the mask clear the whole accel field, or change below if MSA
        value.sstStruct.oDcf = blockAccess ? 1 : 0;
        mask.sstStruct.oDcf = ~0;        // Ensure this endpoint is active
    }

    // Set the Type select for splits to be from the split packet
    if (isSplit)
    {
        if (endpointDirection == EP_DIRECTION_IN)
        {
            mask.sstStruct.iEpTypeSel = ~0;
            value.sstStruct.iEpTypeSel = 1;
        }
        else
        {
            mask.sstStruct.oEpTypeSel = ~0;
            value.sstStruct.oEpTypeSel = 1;
        }
    }


    switch (endpointType)
    {
        case EP_TYPE_ISO:
            break;

        case EP_TYPE_BULK:
            if (preserveMSA)
            {
                mask.sstStruct.ovrLay = 0;  // don't modify the MSA settings
                mask.sstStruct.iAccel = 0;  // don't modify the MSA settings
                mask.sstStruct.oAccel = 0;  // don't modify the MSA settings
            }
            break;

        case EP_TYPE_INT:
            if (endpointDirection == EP_DIRECTION_IN)
            {
                mask.ovrLay.iIntEndPointStruct.fullHalfRate = ~0;
                // Only set fullHalfRate (aka optimized NAK) if
                // (i)  The endpoint is not on a hub, (TODO we currently do this for all hubs,
                //      but it should only be necessary for full-speed hubs) and
                // (ii) The endpoint (which has an interrupt IN) does not have an
                //      MSA-paired bulk OUT.
                // See section 4.3.4 of "U:\Projects\GoldenEars\90-Documents\0-00381 ExtremeUSB
                // Transactions\90-00381-B07.pdf" for an explanation of (i).

                // The reason for (ii) is related to Bug 4143. Essentially, there is an overlap
                // between the interrupt IN and bulk OUT overlays in the XSST. Writing to the
                // msaEpPair bitfield of the bulk OUT overlay clobbers the iPerBufLimit and
                // iPerBufState bitfields in the interrupt IN overlay. Since it is too late
                // to change this in RTL for the Goldenears ASIC, we work around this issue by
                // setting the fullHalfRate bitfield in the interrupt IN ovelay to zero. This
                // causes RTL to ignore the iPerBuf{Limit,State} fields for this endpoint,
                // and thus prevents any erroneous behavior that would result from writing
                // invalid values to the iPerBuf{Limit,State} fields.
                // This workaround has not been tested against many devices, since devices that
                // have an MSA-paired bulk OUT and interrupt IN on the same endpoint seem to be
                // rare. If this workaround does end up causing problems, we could always disable
                // MSA for this kind of endpoint instead.
                value.ovrLay.iIntEndPointStruct.fullHalfRate = !isHub && !isOtherDirectionMSA;

                if (!isHub && isOtherDirectionMSA)
                {
                    ilog_XCSR_COMPONENT_2(
                        ILOG_WARNING,
                        XCSR_MSA_OUT_INT_IN_DISABLED_OPTIMIZED_NAK,
                        endpointNumber,
                        XCSR_getXUSBAddrUsb(address));
                }
            }
            break;

        case EP_TYPE_CTRL:
        default:
            iassert_XCSR_COMPONENT_2(FALSE, XCSR_INVALID_ENDPOINT_TYPE, endpointType, __LINE__);
    }

    ilog_XCSR_COMPONENT_3(ILOG_DEBUG, WRITING_ENDPOINT_DATA1, XCSR_getXUSBAddrLogical(address), endpointNumber, endpointType);
    ilog_XCSR_COMPONENT_2(ILOG_DEBUG, WRITING_ENDPOINT_DATA2, (value.sst >> 32), value.sst);
    ilog_XCSR_COMPONENT_2(ILOG_DEBUG, WRITING_ENDPOINT_DATA3, (mask.sst >> 32), mask.sst);

    return XCSR_XSSTWriteMask(XCSR_getXUSBAddrUsb(address), endpointNumber, value.sst, mask.sst);
}


void XCSR_XSSTConfigureMSA(XUSB_AddressT address, uint8 inEndpoint, uint8 outEndpoint)
{
    XSSTConfigureMSA(XCSR_getXUSBAddrUsb(address), inEndpoint, outEndpoint);
}

void XSSTConfigureMSA(uint8 usbAddr, uint8 inEndpoint, uint8 outEndpoint)
{
    struct XCSR_Xsst inMask;
    struct XCSR_Xsst outMask;
    struct XCSR_Xsst inEp;
    struct XCSR_Xsst outEp;

    inMask.sst = 0;
    outMask.sst = 0;
    inEp.sst = 0;
    outEp.sst = 0;

    inMask.sstStruct.iAccel = ~0;
    outMask.sstStruct.oAccel = ~0;
    inEp.sstStruct.iAccel = 1;
    outEp.sstStruct.oAccel = 1;

    inMask.ovrLay.iBulkMsaEndPointStruct.msaEpPair = ~0;
    outMask.ovrLay.oBulkMsaEndPointStruct.msaEpPair = ~0;
    inEp.ovrLay.iBulkMsaEndPointStruct.msaEpPair = outEndpoint;
    outEp.ovrLay.oBulkMsaEndPointStruct.msaEpPair = inEndpoint;

    XCSR_XSSTWriteMask(usbAddr, inEndpoint, inEp.sst, inMask.sst);
    XCSR_XSSTWriteMask(usbAddr, outEndpoint, outEp.sst, outMask.sst);
}

void XCSR_XSSTClearOverlayAndAccel(XUSB_AddressT address, uint8 endpoint, enum EndpointDirection direction)
{
    struct XCSR_Xsst mask;
    struct XCSR_Xsst ep;

    mask.sst = 0;
    ep.sst = 0;

    if (direction == EP_DIRECTION_IN)
    {
        mask.sstStruct.iAccel = ~0;
        ep.sstStruct.iAccel = 0;
        mask.ovrLay.iBulkMsaEndPointStruct.msaEpPair = ~0;
        ep.ovrLay.iBulkMsaEndPointStruct.msaEpPair = 0;
    }
    else
    {
        mask.sstStruct.oAccel = ~0;
        ep.sstStruct.oAccel = 0;
        mask.ovrLay.oBulkMsaEndPointStruct.msaEpPair = ~0;
        ep.ovrLay.oBulkMsaEndPointStruct.msaEpPair = 0;
    }

    XCSR_XSSTWriteMask(XCSR_getXUSBAddrUsb(address), endpoint, ep.sst, mask.sst);
}


/**
* FUNCTION NAME: XCSR_XSSTClearEndpointAccelField()
*
* @brief  - Clear the endpoint acceleration field
*
* @return - nothing
*
* @note   -
*
*/
void XCSR_XSSTClearEndpointAccelField(XUSB_AddressT address, uint8 LowEndpointNumber, uint8 HighEndpointNumber)
{
    struct XCSR_Xsst mask;
    uint32 controlRegValue;
    uint8 endpointNumber;

    iassert_XCSR_COMPONENT_2(HighEndpointNumber <= MAX_ENDPOINTS, XCSR_INVALID_ENDPOINT_NUMBER, HighEndpointNumber, __LINE__);


    // Set the write mask
    mask.sst = 0;
    mask.sstStruct.iAccel = ~0;
    mask.sstStruct.oAccel = ~0;
    WRITE_XSST_MASK_LSW(XCSR_BASE_ADDR, GET_LSW_FROM_64(mask.sst));
    WRITE_XSST_MASK_MSW(XCSR_BASE_ADDR, GET_MSW_FROM_64(mask.sst));

    // Set & write the status to clear everything but what is masked out
    WRITE_XSST_DATA_LSW(XCSR_BASE_ADDR, 0);
    WRITE_XSST_DATA_MSW(XCSR_BASE_ADDR, 0);


    controlRegValue = 0;
    controlRegValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(controlRegValue, XCSR_getXUSBAddrUsb(address));
    controlRegValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(controlRegValue, RMW_SST_LSW_MSW);
    controlRegValue = XCSR_XSST_CONTROL_GO_SET_BF(controlRegValue, 1);

    for (endpointNumber=LowEndpointNumber; endpointNumber <= HighEndpointNumber; endpointNumber++)
    {
        controlRegValue = XCSR_XSST_CONTROL_DEVENDPT_SET_BF(controlRegValue, endpointNumber);
        writeControlAndBlock(controlRegValue);
    }
}


/**
* FUNCTION NAME: XCSR_XSSTResetEndpoints()
*
* @brief  - Reset the XSST entry for the given address, endpoint. Clear everything except endpoint,
*           endpoint extension, and the Alternate Response.
*
* @return - void
*
* @note   - When clearing the XSST if there was a valid Qid then the queue must be flushed and deallocated
*
*           When this is called, MSA is completely disabled, so we can clean up the MSA fields
*/
void XCSR_XSSTResetEndpoints
(
    XUSB_AddressT address,   //Address to clear the XSST
    uint8 LowEndpointNumber, //Lowest endpoint to clear
    uint8 HighEndpointNumber //Highest endpoint to clear
)
{
    struct XCSR_Xsst oldXsst;
    struct XCSR_Xsst mask;
    uint32 writeControlRegValue;
    uint8 endpointNumber;

    iassert_XCSR_COMPONENT_2(HighEndpointNumber <= MAX_ENDPOINTS, XCSR_INVALID_ENDPOINT_NUMBER, HighEndpointNumber, __LINE__);

    // Set the write mask
    mask.sst = ~0;
    mask.sstStruct.iEpType = 0;
    mask.sstStruct.oEpType = 0;
    mask.sstStruct.iAltClrRsp = 0;
    mask.sstStruct.oAltClrRsp = 0;
    mask.sstStruct.iAccel = 0;
    mask.sstStruct.oAccel = 0;
    mask.sstStruct.iDcf = 0;
    mask.sstStruct.oDcf = 0;

    // Set & write the status to clear everything but what is masked out
    WRITE_XSST_DATA_LSW(XCSR_BASE_ADDR, 0);
    WRITE_XSST_DATA_MSW(XCSR_BASE_ADDR, 0);

    writeControlRegValue = 0;
    writeControlRegValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(writeControlRegValue, XCSR_getXUSBAddrUsb(address));
    writeControlRegValue = XCSR_XSST_CONTROL_GO_SET_BF(writeControlRegValue, 1);

    for (endpointNumber=LowEndpointNumber; endpointNumber <= HighEndpointNumber; endpointNumber++)
    {
        struct XCSR_Xsst readValue;

        // Set the endpoint # for this iteration
        writeControlRegValue = XCSR_XSST_CONTROL_DEVENDPT_SET_BF(writeControlRegValue, endpointNumber);

        // Check the epType
        writeControlRegValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(writeControlRegValue, READ_ALL);
        // Tell the HW to do the XSST read
        writeControlAndBlock(writeControlRegValue);
        readValue.sst = MAKE_U64(READ_XSST_DATA_MSW(XCSR_BASE_ADDR), READ_XSST_DATA_LSW(XCSR_BASE_ADDR));
        // if type is control clear the whole overlay
        // if type is MSA clear the whole overlay
        // if type is IN interrupt, preserve the overlay
        // else overlay is unused, and should be cleared

        mask.sstStruct.ovrLay = ~0; // Clear the overlay, default setting
        if (readValue.sstStruct.iEpType == EP_TYPE_ISO)
        {
            mask.ovrLay.iIntEndPointStruct.iPerBufLimit = 0;     // Preserve the overlay
            mask.ovrLay.iIntEndPointStruct.iPerBufState = 0;     // Preserve the overlay
        }
        else if (readValue.sstStruct.iEpType == EP_TYPE_INT)
        {
            mask.ovrLay.iIntEndPointStruct.iPerBufLimit = 0;     // Preserve the overlay
            mask.ovrLay.iIntEndPointStruct.iPerBufState = 0;     // Preserve the overlay
            mask.ovrLay.iIntEndPointStruct.copyToCpu = 0;        // Preserve the overlay
            mask.ovrLay.iIntEndPointStruct.fullHalfRate = 0;     // Preserve the overlay
        }

        // Write the mask
        WRITE_XSST_MASK_LSW(XCSR_BASE_ADDR, GET_LSW_FROM_64(mask.sst));
        WRITE_XSST_MASK_MSW(XCSR_BASE_ADDR, GET_MSW_FROM_64(mask.sst));

        writeControlRegValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(writeControlRegValue, RMW_SST_LSW_MSW);
        // Tell the HW to do the XSST R-M-W
        writeControlAndBlock(writeControlRegValue);

        oldXsst.sst = MAKE_U64(READ_XSST_DATA_MSW(XCSR_BASE_ADDR), READ_XSST_DATA_LSW(XCSR_BASE_ADDR));

        // Check to see if a retry buffer needs to be released
        if (oldXsst.sstStruct.rtyUsage)
        {
            ilog_XCSR_COMPONENT_2(ILOG_MAJOR_EVENT, RELEASE_RETRY,
                   XCSR_getXUSBAddrUsb(address), endpointNumber);
            XLR_releaseRetryBuffer(oldXsst.sstStruct.rtyUsage);
        }

        // Check if there was a valid Q prior to nuking the XSST
        if (oldXsst.sstStruct.iQid != 0)
        {
            // Must flush and return the qid after clearing XSST
            XCSR_XICSQueueFlushAndDeallocate(oldXsst.sstStruct.iQid);
        }
        if (oldXsst.sstStruct.oQid != 0)
        {
            // Must flush and return the qid after clearing XSST
            XCSR_XICSQueueFlushAndDeallocate(oldXsst.sstStruct.oQid);
        }
    }
}


/**
* FUNCTION NAME: XCSR_XSSTClearBCO()
*
* @brief  - Clear the BCO flag in the XSST for the given USB address
*
* @return - nothing
*
* @note   -
*
*/
void XCSR_XSSTClearBCO
(
    XUSB_AddressT address  // address to clear BCO for
)
{
    uint32 tempValue = 0;
    struct XCSR_Xsst mask;

    ilog_XCSR_COMPONENT_3(ILOG_MINOR_EVENT, CLR_BCO, XCSR_getXUSBAddrUsb(address), XCSR_getXUSBAddrLogical(address), XCSR_getXUSBAddrValid(address) << 1 | XCSR_getXUSBAddrInSys(address));

    // Set the mask for BCO
    mask.sst = 0;
    mask.ovrLay.ctrlEndPointStruct.bco = ~0;
    WRITE_XSST_MASK_MSW(XCSR_BASE_ADDR, GET_MSW_FROM_64(mask.sst));
    WRITE_XSST_MASK_LSW(XCSR_BASE_ADDR, GET_LSW_FROM_64(mask.sst));

    // Set the BCO flag value
    WRITE_XSST_DATA_MSW(XCSR_BASE_ADDR, 0);
    WRITE_XSST_DATA_LSW(XCSR_BASE_ADDR, 0);

    // Setup control register for a read-mod-write to the given address for its control endpoint
    tempValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(tempValue, XCSR_getXUSBAddrUsb(address));
    //tempValue = XCSR_XSST_CONTROL_DEVENDPT_SET_BF(tempValue, 0); // not needed as it's already 0
    tempValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(tempValue, RMW_SST_LSW_MSW);
    tempValue = XCSR_XSST_CONTROL_GO_SET_BF(tempValue, 1);
    writeControlAndBlock(tempValue);
}


/**
* FUNCTION NAME: XCSR_XSSTClearBCI()
*
* @brief  - Clear the BCI flag in the XSST for the given USB address
*
* @return - nothing
*
* @note   -
*
*/
void XCSR_XSSTClearBCI
(
    XUSB_AddressT address  // address to clear BCI for
)
{
    uint32 tempValue = 0;
    struct XCSR_Xsst mask;

    ilog_XCSR_COMPONENT_3(ILOG_MINOR_EVENT, CLR_BCI, XCSR_getXUSBAddrUsb(address), XCSR_getXUSBAddrLogical(address), XCSR_getXUSBAddrValid(address) << 1 | XCSR_getXUSBAddrInSys(address));

    // Set the mask for BCI
    mask.sst = 0;
    mask.ovrLay.ctrlEndPointStruct.bci = ~0;
    WRITE_XSST_MASK_MSW(XCSR_BASE_ADDR, GET_MSW_FROM_64(mask.sst));
    WRITE_XSST_MASK_LSW(XCSR_BASE_ADDR, GET_MSW_FROM_64(mask.sst));

    // Set the BCI flag value
    WRITE_XSST_DATA_MSW(XCSR_BASE_ADDR, 0);
    WRITE_XSST_DATA_LSW(XCSR_BASE_ADDR, 0);

    // Setup control register for a read-mod-write to the given address for its control endpoint
    tempValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(tempValue, XCSR_getXUSBAddrUsb(address));
    //tempValue = XCSR_XSST_CONTROL_DEVENDPT_SET_BF(tempValue, 0); // not needed as it's already 0
    tempValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(tempValue, RMW_SST_LSW_MSW);
    tempValue = XCSR_XSST_CONTROL_GO_SET_BF(tempValue, 1);
    writeControlAndBlock(tempValue);
}


/**
* FUNCTION NAME: XCSR_XSSTWriteMask()
*
* @brief  - Generic XSST write. Writes the given value to the XSST for the given address, endpoint
*
* @return - nothing
*
* @note   - The generic XSST write function is only to be used in special circumstances such as UI cmd,
*           or bug fix that cannot be implemented with other existing functions
*
*/
uint64 XCSR_XSSTWriteMask
(
    uint8 usbAddress,  //address to update the xsst
    uint8 endPoint,  //endpoint number to update
    uint64 writeValue,  //xsst status to write
    uint64 writeMask  //write mask
)
{
    uint32 tempValue = 0;
    const uint32 writeValueLSW = writeValue & 0xFFFFFFFF;
    const uint32 writeValueMSW = writeValue >> 32;
    const uint32 writeMaskLSW = writeMask & 0xFFFFFFFF;
    const uint32 writeMaskMSW = writeMask >> 32;
    uint32 oldValueLSW;
    uint32 oldValueMSW;

    iassert_XCSR_COMPONENT_2(endPoint <= MAX_ENDPOINTS, XCSR_INVALID_ENDPOINT_NUMBER, endPoint, __LINE__);

    // Setup the write mask
    WRITE_XSST_MASK_LSW(XCSR_BASE_ADDR, writeMaskLSW);
    WRITE_XSST_MASK_MSW(XCSR_BASE_ADDR, writeMaskMSW);

    // Set the value to write
    WRITE_XSST_DATA_LSW(XCSR_BASE_ADDR, writeValueLSW);
    WRITE_XSST_DATA_MSW(XCSR_BASE_ADDR, writeValueMSW);

    tempValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(tempValue, usbAddress);
    tempValue = XCSR_XSST_CONTROL_DEVENDPT_SET_BF(tempValue, endPoint);
    tempValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(tempValue, RMW_SST_LSW_MSW);
    tempValue = XCSR_XSST_CONTROL_GO_SET_BF(tempValue, 1);
    writeControlAndBlock(tempValue);

    oldValueLSW = READ_XSST_DATA_LSW(XCSR_BASE_ADDR);
    oldValueMSW = READ_XSST_DATA_MSW(XCSR_BASE_ADDR);
    ilog_XCSR_COMPONENT_3(ILOG_DEBUG, XCSR_XSST_WRITE_MASK_LSW, writeValueLSW, writeMaskLSW, oldValueLSW);
    ilog_XCSR_COMPONENT_3(ILOG_DEBUG, XCSR_XSST_WRITE_MASK_MSW, writeValueMSW, writeMaskMSW, oldValueMSW);
    return ((uint64)oldValueMSW << 32) + (uint64)oldValueLSW;
}

/**
* FUNCTION NAME: XCSR_XSSTWriteLAT()
*
* @brief  - Write to the logical address table
*
* @return - void
*
* @note   - Used by the Rex Scheduler
*
*/
void XCSR_XSSTWriteLAT(
    uint8 usbAddress,
    uint32 value
)
{
    uint32 tempValue = 0;

    WRITE_LAT_DATA(XCSR_BASE_ADDR, value);

    tempValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(tempValue, usbAddress);
    tempValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(tempValue, WRITE_LAT);
    tempValue = XCSR_XSST_CONTROL_GO_SET_BF(tempValue, 1);
    writeControlAndBlock(tempValue);
}


/**
* FUNCTION NAME: icmdXSSTWriteLAT()
*
* @brief  - Writes to the LAT
*
* @return - void
*
* @note   - This is an icmd function
*
*/
void icmdXSSTWriteLat
(
    uint8 usbAddress,   //
    uint8 endPoint,     //
    uint32 value        //
)
{
    uint32 controlVal = 0;

    ilog_XCSR_COMPONENT_3(ILOG_USER_LOG, LOG_XSST_WRITE_LAT, usbAddress, endPoint, value);

    controlVal = XCSR_XSST_CONTROL_DEVADDR_SET_BF(controlVal, usbAddress);
    controlVal = XCSR_XSST_CONTROL_DEVENDPT_SET_BF(controlVal, endPoint);
    controlVal = XCSR_XSST_CONTROL_ACCMODE_SET_BF(controlVal, WRITE_LAT);
    controlVal = XCSR_XSST_CONTROL_GO_SET_BF(controlVal, 1);

    WRITE_LAT_DATA(XCSR_BASE_ADDR, value);

    writeControlAndBlock(controlVal);
}


/**
* FUNCTION NAME: icmdXSSTWriteSST()
*
* @brief  - Writes the XSST
*
* @return - void
*
* @note   - This is an icmd function
*
*/
void icmdXSSTWriteSST
(
    uint8 usbAddress,   //
    uint8 endPoint,     //
    uint32 valueMSW,    //
    uint32 valueLSW     //
)
{
    uint32 controlVal = 0;

    ilog_XCSR_COMPONENT_3(ILOG_USER_LOG, LOG_XSST_WRITE_SST, (usbAddress << 16) | endPoint, valueMSW, valueLSW);

    controlVal = XCSR_XSST_CONTROL_DEVADDR_SET_BF(controlVal, usbAddress);
    controlVal = XCSR_XSST_CONTROL_DEVENDPT_SET_BF(controlVal, endPoint);
    controlVal = XCSR_XSST_CONTROL_ACCMODE_SET_BF(controlVal, WRITE_SST_LSW_MSW);
    controlVal = XCSR_XSST_CONTROL_GO_SET_BF(controlVal, 1);

    WRITE_XSST_DATA_MSW(XCSR_BASE_ADDR, valueMSW);
    WRITE_XSST_DATA_LSW(XCSR_BASE_ADDR, valueLSW);

    writeControlAndBlock(controlVal);
}

/**
* FUNCTION NAME: icmdXSSTReadModifyWriteSST()
*
* @brief  - Read-Modify-Write
*
* @return - void
*
* @note   - This is an icmd function
*
*/

void icmdXSSTReadModifyWriteSST
(
    uint8 usbAddress,   //
    uint8 endPoint,     //
    uint32 valueMSW,    //
    uint32 valueLSW,    //
    uint32 maskMSW,     //
    uint32 maskLSW      //
)
{
    uint64 oldValue = XCSR_XSSTWriteMask(usbAddress, endPoint, MAKE_U64(valueMSW, valueLSW), MAKE_U64(maskMSW, maskLSW));

    ilog_XCSR_COMPONENT_3(ILOG_USER_LOG, XCSR_XSST_WRITE_MASK_LSW, valueLSW, maskLSW, GET_LSW_FROM_64(oldValue));
    ilog_XCSR_COMPONENT_3(ILOG_USER_LOG, XCSR_XSST_WRITE_MASK_MSW, valueMSW, maskMSW, GET_MSW_FROM_64(oldValue));
}

/**
* FUNCTION NAME: icmdXSSTReadAll()
*
* @brief  - Accesses the XSST
*
* @return - void
*
* @note   - This is an icmd function
*
*/
void icmdXSSTReadAll
(
    uint8 usbAddress,   //
    uint8 endPoint      //
)
{
    uint32 valReadMSW;
    uint32 valReadLSW;
    uint32 latRead;
    uint32 controlVal = 0;

    controlVal = XCSR_XSST_CONTROL_DEVADDR_SET_BF(controlVal, usbAddress);
    controlVal = XCSR_XSST_CONTROL_DEVENDPT_SET_BF(controlVal, endPoint);
    controlVal = XCSR_XSST_CONTROL_ACCMODE_SET_BF(controlVal, READ_ALL);
    controlVal = XCSR_XSST_CONTROL_GO_SET_BF(controlVal, 1);

    writeControlAndBlock(controlVal);

    valReadMSW = READ_XSST_DATA_MSW(XCSR_BASE_ADDR);
    valReadLSW = READ_XSST_DATA_LSW(XCSR_BASE_ADDR);
    latRead = READ_LAT_DATA(XCSR_BASE_ADDR);

    ilog_XCSR_COMPONENT_3(ILOG_USER_LOG, LOG_XSST_READ, (usbAddress << 16) | endPoint, valReadMSW, valReadLSW);
    ilog_XCSR_COMPONENT_3(ILOG_USER_LOG, LOG_LAT_READ, usbAddress, endPoint, latRead);
}

void XCSR_XSSTAddr0SetInsys(void)
{
    // Perform RMW of LAT to only set inSys by having it in the mask and the value
    uint32 tempValue = 0;
    struct XCSR_Lat latValWithInSysSet;
    latValWithInSysSet.lat = 0;
    latValWithInSysSet.latStruct.inSys = 1;

    WRITE_LAT_MASK(XCSR_BASE_ADDR, latValWithInSysSet.lat);
    WRITE_LAT_DATA(XCSR_BASE_ADDR, latValWithInSysSet.lat);

    tempValue = XCSR_XSST_CONTROL_DEVADDR_SET_BF(tempValue, 0);
    tempValue = XCSR_XSST_CONTROL_ACCMODE_SET_BF(tempValue, RMW_LAT);
    tempValue = XCSR_XSST_CONTROL_GO_SET_BF(tempValue, 1);
    writeControlAndBlock(tempValue);
}

