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
//!   @file  -
//
//!   @brief -
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "topology_loc.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/


/***************************** Local Variables *******************************/
static uint16 msaLogicalAddressAllocatedBitField; // bit 0 is for LA 0, bit x is for LA x

/************************ Local Function Prototypes **************************/
static MSA_AddressT allocateMsaLat(XUSB_AddressT address);
static void setupNewMsa(XUSB_AddressT xusbAddr, MSA_AddressT msaAddr, uint8 epIn, uint8 epOut);
static void __DTT_SendMsaFreedToRex(XUSB_AddressT xusbAddr);

/************************** Function Definitions *****************************/

boolT DTT_IsMsa(XUSB_AddressT xusbAddr)
{
    struct DeviceTopology * node = _DTT_GetDeviceNode(XCSR_getXUSBAddrLogical(xusbAddr));

    return node->msaLA != TOPOLOGY_MSA_LA_UNDEFINED;
}

void _DTT_ConfigureMsa(XUSB_AddressT xusbAddr, uint8 epIn, uint8 epOut)
{
    struct DeviceTopology * pNode = _DTT_GetDeviceNode(XCSR_getXUSBAddrLogical(xusbAddr));

    // Get an MSA Address
    MSA_AddressT msaAddr = XLR_msaGetAddrFromMsaLat(XCSR_getXUSBAddrUsb(xusbAddr));
    if (!XLR_msaAddressGetValid(msaAddr))
    {
        // No valid address found in the MSA LAT.  Try to allocate a new address.
        iassert_TOPOLOGY_COMPONENT_2(
            pNode->msaLA == TOPOLOGY_MSA_LA_UNDEFINED,
            MSA_TOPOLOGY_MISMATCH,
            XCSR_getXUSBAddrUsb(xusbAddr),
            __LINE__);

        msaAddr = allocateMsaLat(xusbAddr);
        if (XLR_msaAddressGetValid(msaAddr))
        {
            pNode->msaLA = XLR_msaAddressGetLA(msaAddr);
        }
    }

    // Only proceed if we have a valid MSA Address
    if (XLR_msaAddressGetValid(msaAddr))
    {
        iassert_TOPOLOGY_COMPONENT_2(
            pNode->msaLA != TOPOLOGY_MSA_LA_UNDEFINED && pNode->msaLA == XLR_msaAddressGetLA(msaAddr),
            MSA_TOPOLOGY_MISMATCH,
            XCSR_getXUSBAddrUsb(xusbAddr),
            __LINE__);

        setupNewMsa(xusbAddr, msaAddr, epIn, epOut);
    }
}

// Note: args are verified to be valid by caller
static void setupNewMsa(XUSB_AddressT xusbAddr, MSA_AddressT msaAddr, uint8 epIn, uint8 epOut)
{
    // Read the pointer table to see if these endpoints are already allocated
    const uint8 msaInPtr = XLR_msaReadPtrTable(msaAddr, epIn);
    const uint8 msaOutPtr = XLR_msaReadPtrTable(msaAddr, epOut);

    // Ensure they are the same, either both allocated to the same value, or both unallocated
    if (msaInPtr != msaOutPtr)
    {
        ilog_TOPOLOGY_COMPONENT_3(ILOG_FATAL_ERROR, MSA_MISCONFIGURED_PTRS1, XLR_msaAddressGetUSB(msaAddr), epIn, msaInPtr);
        iassert_TOPOLOGY_COMPONENT_3(FALSE, MSA_MISCONFIGURED_PTRS2, XLR_msaAddressGetUSB(msaAddr), epOut, msaOutPtr);
    }
    else // the pointers are identical
    {
        if (msaInPtr != 0) //  or msaOutPtr, as they are identical as ensured from above
        {
            // already configured, nothing to do
        }
        else
        {
            // pointers are unallocated
            // Allocate a new pointer and assign it to each
            const uint8 ptr = XLR_msaAllocatePtr(); //0 means unable to allocate new ptr, driver will skip over initial null allocation

            // If we were able to allocate a pointer, configure this mass storage interface
            if (ptr)
            {
                ilog_TOPOLOGY_COMPONENT_3(ILOG_MAJOR_EVENT, NEW_MSA_INTERFACE, XLR_msaAddressGetUSB(msaAddr), epIn, epOut);
                XLR_msaWritePtrTable(msaAddr, epIn, ptr);
                XLR_msaWritePtrTable(msaAddr, epOut, ptr); // NOTE: if epIn == epOut, we are just overwriting the same value
                XLR_msaClearStatusTable(msaAddr, epIn); // We only need to do this for one endpoint, as the pointers are the same
                XCSR_XSSTConfigureMSA(xusbAddr, epIn, epOut);
            }
            else
            {
                ilog_TOPOLOGY_COMPONENT_0(ILOG_USER_LOG, MSA_OUT_OF_PTRS);
            }
        }
    }
}


static MSA_AddressT allocateMsaLat(XUSB_AddressT xusbAddr)
{
    uint8 msaLa;
    MSA_AddressT msaAddr;

    // Initialize the MSA address type
    XLR_msaAddressInit(&msaAddr);

    // Find an unused logical address
    for (msaLa = 1;
        (msaLa < 16) && ((msaLogicalAddressAllocatedBitField >> msaLa) & 1);
        msaLa++)
    { }

    // Did we find a valid address?
    if (msaLa == 16)
    {
        // No available logical addresses
    }
    else
    {
        struct DeviceTopology * pNode = _DTT_GetDeviceNode(XCSR_getXUSBAddrLogical(xusbAddr));

        // we have a valid logical address
        msaLogicalAddressAllocatedBitField |= (1 << msaLa);
        XLR_msaAddressSetLA      (&msaAddr, msaLa);
        XLR_msaAddressSetUSB     (&msaAddr, XCSR_getXUSBAddrUsb(xusbAddr));
        XLR_msaAddressSetValid   (&msaAddr, 1);

        // update the driver
        XLR_msaUpdateAddress(msaAddr);

        // Update the device table
        pNode->msaLA = msaLa;
    }

    return msaAddr;
}


// Must be disabled to call this function
void _DTT_freeMsa(XUSB_AddressT xusbAddr, struct DeviceTopology * pNode, boolT informRex)
{
    if (pNode->msaLA != TOPOLOGY_MSA_LA_UNDEFINED)
    {
        MSA_AddressT msaAddr = XLR_msaGetAddrFromMsaLat(XCSR_getXUSBAddrUsb(xusbAddr));

        // ensure this LA was actually used
        iassert_TOPOLOGY_COMPONENT_3(   msaLogicalAddressAllocatedBitField & (1 << XLR_msaAddressGetLA(msaAddr)),
                                        MSA_FREE_LAT_FAILURE,
                                        XLR_msaAddressGetUSB(msaAddr),
                                        XLR_msaAddressGetLA(msaAddr),
                                        msaLogicalAddressAllocatedBitField);
        // ensure this is valid
        iassert_TOPOLOGY_COMPONENT_2(   XLR_msaAddressGetValid(msaAddr),
                                        MSA_FREE_INVALID,
                                        XLR_msaAddressGetUSB(msaAddr),
                                        XLR_msaAddressGetLA(msaAddr));
        // ensure the HW & SW are in sync
        iassert_TOPOLOGY_COMPONENT_2(   pNode->msaLA == XLR_msaAddressGetLA(msaAddr),
                                        MSA_TOPOLOGY_MISMATCH,
                                        XLR_msaAddressGetUSB(msaAddr),
                                        __LINE__);

        ilog_TOPOLOGY_COMPONENT_2(  ILOG_MAJOR_EVENT,
                MSA_FREE_ADDR,
                pNode->msaLA,
                XCSR_getXUSBAddrUsb(xusbAddr));

        // Conditionally inform the Rex that we're about to free up all MSA
        // information for a device. The Rex should then ensure that all MSA-related resources
        // for the device are freed on its end.
        // This is intended to handle a situation in which a device is disconnected
        // during an MSA transaction and its MsaStatus resource on the Rex is not properly freed.
        // This is exacerbated on Linux, where cycled devices re-enumerate with higher USB
        // addresses, preventing the MsaStatus resource from being reused (as it is looked up
        // by USB address) when the device is reconnected and later freed.
        // See Bug 4131 for details.
        if (informRex && XCSR_XICSGetRexSupportsSwMessages(XCSR_getXUSBAddrVPort(xusbAddr)))
        {
            __DTT_SendMsaFreedToRex(xusbAddr);
        }

        // Iterate over endpoints & free pointer + clear XSST accel bits. Remember each
        // endpoint pair points to same pointer.  Endpoint number 0 is not checked because it can
        // never participate in MSA since it is the control endpoint.
        // TODO should we be clearing the MSA overlay bits (msaEpPair) as well?
        XCSR_XSSTClearEndpointAccelField(xusbAddr, 1, pNode->highEndpoint);
        for (uint8 ep = 1; ep <= pNode->highEndpoint; ep++)
        {
            _DTT_clearMsaPair(msaAddr, ep);
        }

        // clear allocation from our bitfield tracking
        msaLogicalAddressAllocatedBitField =
            msaLogicalAddressAllocatedBitField & ~(1 << XLR_msaAddressGetLA(msaAddr));

        // clear allocation from the hw table
        XLR_msaAddressSetValid(&msaAddr, 0);
        XLR_msaAddressSetLA(&msaAddr, 0);
        XLR_msaUpdateAddress(msaAddr);

        // Clear allocation from topology table
        pNode->msaLA = TOPOLOGY_MSA_LA_UNDEFINED;
    }
}

/**
* FUNCTION NAME: _DTT_clearMsaPair()
*
* @brief  - For a given endpoint with a valid pointer in the MSA pointer table,
*           - Remove the pointer table entries for the endpoint and its MSA pair endpoint
*           - Free the entry from the MSA status table
* @return - nothing
*/
void _DTT_clearMsaPair(MSA_AddressT msaAddr, uint8 ep)
{
    if (XLR_msaAddressGetValid(msaAddr))
    {
        const uint8 ptr = XLR_msaReadPtrTable(msaAddr, ep);
        if (ptr)
        {
            // We have a valid pointer, now remove this pointer from all references endpoints that use it
            // NOTE: There should only ever be 1 or 2 endpoints that use the same pointer, an IN and an OUT
            uint8 ep2;
            for (ep2 = ep; ep2 < 16; ep2++)
            {
                if (XLR_msaReadPtrTable(msaAddr, ep2) == ptr)
                {
                    // This ep2, has the same ptr as ep, lets clear the entry with this ptr
                    // NOTE: this first time through ep == ep2
                    XLR_msaWritePtrTable(msaAddr, ep2, 0);
                }
            }

            // Now free the pointer
            XLR_msaFreePtr(ptr);
        }
    }
}

/**
* FUNCTION NAME: __DTT_SendMsaFreedToRex()
*
* @brief  - Informs the appropriate Rex via a CPU message that we have freed Lex-side MSA
*           resources for a device downstream of it
* @return - nothing
*/
void __DTT_SendMsaFreedToRex
(
    XUSB_AddressT xusbAddr   // XUSB address of the device whose MSA info we've freed
)
{
    // Inform the Rex that we've freed an MSA pair.
    XCSR_XICSSendMessageWithData(
            SOFTWARE_MESSAGE_LEX2REX,
            LEX_FREED_MSA_PAIR,
            XCSR_getXUSBAddrVPort(xusbAddr),
            XCSR_getXUSBAddrUsb(xusbAddr));

    ilog_TOPOLOGY_COMPONENT_2(
            ILOG_DEBUG,
            TOPOLOGY_SENT_MSA_FREED_TO_REX,
            XCSR_getXUSBAddrVPort(xusbAddr),
            XCSR_getXUSBAddrUsb(xusbAddr));
}

/**
* FUNCTION NAME: _DTT_isMsaActive()
*
* @brief  - Returns whether MSA is active (i/oAccel is set) for a
*           given (USB address, endpoint, direction) triple
*
* @return - TRUE if MSA is active for the triple, FALSE otherwise
*/
boolT _DTT_isMsaActive(uint8 usbAddr, uint8 ep, enum EndpointDirection epDir)
{
    const struct XCSR_Xsst xsst = {.sst = XCSR_XSSTRead(usbAddr, ep)};

    return (epDir == EP_DIRECTION_OUT) ?
        (xsst.sstStruct.oAccel != 0) : (xsst.sstStruct.iAccel != 0);
}
