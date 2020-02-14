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
//!   @file  - xlr_msa.c
//
//!   @brief - MSA (Mass Storage Acceleration) routines
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <xlr_msa.h>
#include "xlr_loc.h"

/************************ Defined Constants and Macros ***********************/
#define MSA_READ_ALL    0b000 // RD - MSA_LAT & MSA_MPT & MSA_MST
#define MSA_WRITE_MST   0b001 // WR - MSA_MST
#define MSA_WRITE_MPT   0b010 // WR - MSA_MPT
#define MSA_WRITE_LAT   0b011 // WR - MSA_LAT
#define MSA_RMW_MST     0b100 // RMW - MST_MST

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static uint8 msaReadPtrTable(uint8 usbAddr, uint8 endpoint);
static void readMsaAll(uint8 usbAddr, uint8 ep);

/************************** Function Definitions *****************************/

void _XLR_msaInit(void)
{
    uint8 ptr;
    // request a pointer
    // initially this will return 0, then we never free 0, so 0 can be treated
    // as a null pointer
    ptr = XLR_msaAllocatePtr();
    iassert_XLR_COMPONENT_1(ptr == 0, MSA_INIT_BAD_PTR, ptr);
}

// MSA Lat operations
void msaReadLat(uint8 usbAddr) //icmd
{
    MSA_AddressT msaAddr = XLR_msaGetAddrFromMsaLat(usbAddr);
    ilog_XLR_COMPONENT_3(  ILOG_USER_LOG,
                            MSA_READ_LAT,
                            XLR_msaAddressGetUSB(msaAddr),
                            XLR_msaAddressGetLA(msaAddr),
                            XLR_msaAddressGetValid(msaAddr));
}

void msaWriteLat(uint8 usbAddr, uint8 msaLA, uint8 valid) //icmd + normal
{
    uint32 controlReadValue;

    uint32 controlWriteValue = 0;
    controlWriteValue = XLR_XMST_CONTROL_DEVADDR_SET_BF(controlWriteValue, usbAddr);
    controlWriteValue = XLR_XMST_CONTROL_ACCMODE_SET_BF(controlWriteValue, MSA_WRITE_LAT);
    // XLR_MSA WRLAT : [04:00] = {Enabled[04], LogicalAddress[03:00]}
    controlWriteValue = XLR_XMST_CONTROL_WSTATUS_SET_BF(
        controlWriteValue, ((valid << 4) & 0x10) | (msaLA & 0x1F));
    controlWriteValue = XLR_XMST_CONTROL_GO_SET_BF(controlWriteValue, 1);
    XLR_XMST_CONTROL_WRITE_REG(XLR_BASE_ADDR, controlWriteValue);

    do {
        controlReadValue = XLR_XMST_CONTROL_READ_REG(XLR_BASE_ADDR);
    } while (XLR_XMST_CONTROL_GO_GET_BF(controlReadValue));
}

MSA_AddressT XLR_msaGetAddrFromMsaLat(uint8 usbAddr)
{
    MSA_AddressT msaAddr;
    uint32 mstStatus;

    // Read HW
    readMsaAll(usbAddr, 0);
    mstStatus = XLR_XMST_RSTATUS_READ_REG(XLR_BASE_ADDR);

    // setup return value
    XLR_msaAddressInit     (&msaAddr);
    XLR_msaAddressSetLA    (&msaAddr, XLR_XMST_RSTATUS_LATRDATA_GET_BF(mstStatus));
    XLR_msaAddressSetUSB   (&msaAddr, usbAddr);
    XLR_msaAddressSetValid (&msaAddr, XLR_XMST_RSTATUS_MSAEN_GET_BF(mstStatus));

    return msaAddr;
}

void XLR_msaUpdateAddress(MSA_AddressT msaAddr)
{
    msaWriteLat(XLR_msaAddressGetUSB(msaAddr), XLR_msaAddressGetLA(msaAddr), XLR_msaAddressGetValid(msaAddr));
}

// MSA pointer table operations
static uint8 msaReadPtrTable(uint8 usbAddr, uint8 endpoint)
{
    readMsaAll(usbAddr, endpoint);
    return XLR_XMST_RSTATUS_MPTRDATA_READ_BF(XLR_BASE_ADDR);
}

void msaReadPtrTableICmd(uint8 usbAddr, uint8 endpoint)
{
    uint8 ptr = msaReadPtrTable(usbAddr, endpoint);
    ilog_XLR_COMPONENT_3(  ILOG_USER_LOG,
                            MSA_READ_PTR_TABLE,
                            usbAddr,
                            endpoint,
                            ptr);
    ilog_XLR_COMPONENT_3(  ILOG_USER_LOG,
                            MSA_READ_MST,
                            usbAddr,
                            endpoint,
                            XLR_XMST_RSTATUS_MSTRSTATUS_READ_BF(XLR_BASE_ADDR));
}

uint8 XLR_msaReadPtrTable(MSA_AddressT msaAddr, uint8 endpoint)
{
    return msaReadPtrTable(XLR_msaAddressGetUSB(msaAddr), endpoint);
}

void msaWritePtrTable(uint8 usbAddr, uint8 endpoint, uint8 ptr) // icmd + normal
{
    uint32 controlReadValue;

    uint32 controlWriteValue = 0;
    controlWriteValue = XLR_XMST_CONTROL_DEVADDR_SET_BF(controlWriteValue, usbAddr);
    controlWriteValue = XLR_XMST_CONTROL_DEVENDPT_SET_BF(controlWriteValue, endpoint);
    controlWriteValue = XLR_XMST_CONTROL_ACCMODE_SET_BF(controlWriteValue, MSA_WRITE_MPT);
    // XLT_MSA WR MPT : [03:00] = MIS (MST index)
    controlWriteValue = XLR_XMST_CONTROL_WSTATUS_SET_BF(controlWriteValue, ptr);
    controlWriteValue = XLR_XMST_CONTROL_GO_SET_BF(controlWriteValue, 1);
    XLR_XMST_CONTROL_WRITE_REG(XLR_BASE_ADDR, controlWriteValue);

    do {
        controlReadValue = XLR_XMST_CONTROL_READ_REG(XLR_BASE_ADDR);
    } while (XLR_XMST_CONTROL_GO_GET_BF(controlReadValue));

}

void XLR_msaWritePtrTable(MSA_AddressT msaAddr, uint8 endpoint, uint8 ptr)
{
    msaWritePtrTable(XLR_msaAddressGetUSB(msaAddr), endpoint, ptr);
}

void msaAllocatePtr(void) // icmd
{
    uint8 ptr = XLR_msaAllocatePtr();
    ilog_XLR_COMPONENT_1(ILOG_USER_LOG, MSA_ALLOCATE_PTR, ptr);
}

uint8 XLR_msaAllocatePtr(void) //NOTE: this is called this at initialization to blow through pointer 0
{
    uint32 regReadValue;
    uint32 regWriteValue;

    // Send request for new pointer
    regWriteValue = 0;
    regWriteValue = XLR_XMST_MIDALLO_NEWIDREQ_SET_BF(regWriteValue, 1);
    regWriteValue = XLR_XMST_MIDALLO_GO_SET_BF(regWriteValue, 1);
    XLR_XMST_MIDALLO_WRITE_REG(XLR_BASE_ADDR, regWriteValue);

    // Get new pointer
    do {
        regReadValue = XLR_XMST_MIDALLO_READ_REG(XLR_BASE_ADDR);
    } while (XLR_XMST_MIDALLO_GO_GET_BF(regReadValue) == 1);
    if (XLR_XMST_MIDALLO_IDERR_GET_BF(regReadValue))
    {
        // Error occurred, presumably we are out of pointers
        ilog_XLR_COMPONENT_0(ILOG_WARNING, MSA_ALLOCATE_PTR_FAILED);
        return 0;
    }
    else
    {
        // We have a new pointer
        return XLR_XMST_MIDALLO_NEWID_GET_BF(regReadValue);
    }
}
void XLR_msaFreePtr(uint8 ptr) // icmd + normal
{
    uint32 regReadValue;
    uint32 regWriteValue;

    iassert_XLR_COMPONENT_0(ptr != 0, MSA_FREE_BAD_PTR);

    // Send request for returning pointer
    regWriteValue = 0;
    regWriteValue = XLR_XMST_MIDALLO_RETIDREQ_SET_BF(regWriteValue, 1);
    regWriteValue = XLR_XMST_MIDALLO_RETID_SET_BF(regWriteValue, ptr);
    regWriteValue = XLR_XMST_MIDALLO_GO_SET_BF(regWriteValue, 1);
    XLR_XMST_MIDALLO_WRITE_REG(XLR_BASE_ADDR, regWriteValue);

    // Ensure the operation completes without errors
    do {
        regReadValue = XLR_XMST_MIDALLO_READ_REG(XLR_BASE_ADDR);
    } while (XLR_XMST_MIDALLO_GO_GET_BF(regReadValue) == 1);
    iassert_XLR_COMPONENT_0(XLR_XMST_MIDALLO_IDERR_GET_BF(regReadValue) == 0, MSA_FREE_FAILED);
}

// MSA Status Table operations
void msaClearStatusTable(uint8 usbAddr, uint8 endpoint) // icmd + normal
{
    uint32 controlReadValue;
    uint32 controlWriteValue;

    ilog_XLR_COMPONENT_2(ILOG_DEBUG, MSA_CLR_STS_TABLE, usbAddr, endpoint);
    controlWriteValue = 0;
    controlWriteValue = XLR_XMST_CONTROL_DEVADDR_SET_BF(controlWriteValue, usbAddr);
    controlWriteValue = XLR_XMST_CONTROL_DEVENDPT_SET_BF(controlWriteValue, endpoint);
    controlWriteValue = XLR_XMST_CONTROL_ACCMODE_SET_BF(controlWriteValue, MSA_WRITE_MST);
    // XLR_MSA WRMST : [02:00] = MstStatus[02:00]
    controlWriteValue = XLR_XMST_CONTROL_WSTATUS_SET_BF(controlWriteValue, 0);
    controlWriteValue = XLR_XMST_CONTROL_GO_SET_BF(controlWriteValue, 1);
    XLR_XMST_CONTROL_WRITE_REG(XLR_BASE_ADDR, controlWriteValue);

    do {
        controlReadValue = XLR_XMST_CONTROL_READ_REG(XLR_BASE_ADDR);
    } while (XLR_XMST_CONTROL_GO_GET_BF(controlReadValue));
}

void XLR_msaClearStatusTable(MSA_AddressT msaAddr, uint8 endpoint)
{
    msaClearStatusTable(XLR_msaAddressGetUSB(msaAddr), endpoint);
}

static void readMsaAll(uint8 usbAddr, uint8 ep)
{
    uint32 controlReadValue;
    uint32 controlWriteValue = 0;

    controlWriteValue = XLR_XMST_CONTROL_DEVADDR_SET_BF(controlWriteValue, usbAddr);
    controlWriteValue = XLR_XMST_CONTROL_DEVENDPT_SET_BF(controlWriteValue, ep);
    controlWriteValue = XLR_XMST_CONTROL_ACCMODE_SET_BF(controlWriteValue, MSA_READ_ALL);
    controlWriteValue = XLR_XMST_CONTROL_GO_SET_BF(controlWriteValue, 1);
    XLR_XMST_CONTROL_WRITE_REG(XLR_BASE_ADDR, controlWriteValue);

    do {
        controlReadValue = XLR_XMST_CONTROL_READ_REG(XLR_BASE_ADDR);
    } while (XLR_XMST_CONTROL_GO_GET_BF(controlReadValue));
}
