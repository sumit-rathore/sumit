///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2011, 2012
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
//!   @file  -  xsstmon_ge.c
//
//!   @brief -  monitors various periodic system features such as BCO and queues
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "topology_loc.h"
#include <xlr.h>
#include <storage_Data.h>


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/


/************************ Local Function Prototypes **************************/

static void CheckEndpoint(
    const struct XCSR_Xsst *pOriginalStatus, struct XCSR_Xsst *pUpdatedStatus,
    XUSB_AddressT address, uint8 endptNum, boolT isHub);

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: DTT_bug1418_SuspendNotify()
*
* @brief  - LEX Ulm has gone into suspend
*
* @return - nothing
*
* @note   - not used for GE
*
*/
void DTT_bug1418_SuspendNotify(void)
{
}


/**
* FUNCTION NAME: DTT_bug1418_NotSuspendNotify()
*
* @brief  - LEX Ulm has left suspend
*
* @return - nothing
*
* @note   - not used for GE
*
*/
void DTT_bug1418_NotSuspendNotify(void)
{
}


/**
* FUNCTION NAME: _DTT_XSSTMonitorProcess()
*
* @brief  - Check the in/out blocking bits in the XSST and take appropriate actions
*
* @return - nothing
*/
void _DTT_XSSTMonitorProcess(XUSB_AddressT address, uint8 endpoint, const boolT isHub)
{
    struct XCSR_Xsst updatedXsst;
    struct XCSR_Xsst originalXsst;

    if (XCSR_getXUSBAddrVirtFuncPointer(address) != NULL)
    {
        // Virtual function
        // Don't monitor
        return;
    }

    // Read the XSST
    originalXsst.sst = XCSR_XSSTReadConditional(address, endpoint);

    // Process the XSST
    updatedXsst.sst = originalXsst.sst;
    CheckEndpoint(&originalXsst, &updatedXsst, address, endpoint, isHub);

    // Do the update
    if(updatedXsst.sst != originalXsst.sst)
    {
        const boolT writeSuccess =
            XCSR_XSSTWriteConditional(address, endpoint, updatedXsst.sst);
        if(writeSuccess)
        {
            // If the write succeeded, we want to deallocate queues that were
            // previously associated with the XSST.

            // Check to see if a retry buffer got cleared
            // NOTE: It is invalid for SW to set a bit, so the XOR works by
            //       checking for only 1 the fields set (original or new)
            //       as in this case it must be original set, new cleared
            const uint8 retryBufClearedMap =
                originalXsst.sstStruct.rtyUsage ^ updatedXsst.sstStruct.rtyUsage;
            if (retryBufClearedMap)
            {
                ilog_TOPOLOGY_COMPONENT_2(
                    ILOG_MAJOR_EVENT, RELEASE_RETRY, XCSR_getXUSBAddrUsb(address), endpoint);
                XLR_releaseRetryBuffer(retryBufClearedMap);
            }

            // Clean up IN queue
            if(updatedXsst.sstStruct.iQid != originalXsst.sstStruct.iQid)
            {
                ilog_TOPOLOGY_COMPONENT_2(
                    ILOG_MAJOR_EVENT, FLUSH_IN_QUEUE, XCSR_getXUSBAddrLogical(address), endpoint);
                XCSR_XICSQueueFlushAndDeallocate(originalXsst.sstStruct.iQid);
            }

            // Clean up OUT queue
            if(updatedXsst.sstStruct.oQid != originalXsst.sstStruct.oQid)
            {
                ilog_TOPOLOGY_COMPONENT_2(
                    ILOG_MAJOR_EVENT, FLUSH_OUT_QUEUE, XCSR_getXUSBAddrLogical(address), endpoint);
                XCSR_XICSQueueFlushAndDeallocate(originalXsst.sstStruct.oQid);
            }
        }
    }
}


/**
* FUNCTION NAME: CheckEndpoint()
*
* @brief  - Apply blocking algorithms based on endpoint type
*
* @return - nothing
*
* @note   - Control IN blocking:          Does not use three strikes, nor alternate response,
*                                         behaves essentially like control out blocking
*           Iso IN blocking:              If IN detect is still set, flush Qid
*           Bulk and Int IN/OUT blocking: Bulk and int are treated the same, they use three strikes
*                                         and the alternate response bit is cleared on the 3rd
*                                         strike
*
*           Also checks BCI/BCO and logs based on the setting.  This won't check control endpoints
*           when BCO/BCI is set.
*/
static void CheckEndpoint(
    const struct XCSR_Xsst *originalStatus,
    struct XCSR_Xsst *updatedStatus,
    XUSB_AddressT address,
    uint8 endptNum,
    boolT isHub)
{
    uint8 logicalAddr = XCSR_getXUSBAddrLogical(address);

    // Extra sanity checks for the control endpoint
    if (endptNum == 0)
    {
        const boolT bciBcoSet = originalStatus->ovrLay.ctrlEndPointStruct.bci
                            &&  originalStatus->ovrLay.ctrlEndPointStruct.bco;

        // According to the lex software manual from Terry, software should take corrective action
        // if both BCO and BCI are set.
        iassert_TOPOLOGY_COMPONENT_1(!bciBcoSet, BCI_BCO_SET, logicalAddr);
    }

    // Using a retry buffer should be very rare.  If one is use, log
    if (originalStatus->sstStruct.rtyUsage)
    {
        ilog_TOPOLOGY_COMPONENT_3(
            ILOG_WARNING,
            TOPOLOGY_SHOW_XSST_RETRY_USAGE,
            XCSR_getXUSBAddrUsb(address),
            endptNum,
            originalStatus->sstStruct.rtyUsage);
    }

    // Check IN Endpoint

    // SW XSST Monitor is only managaing non-MSA endpoints.  Mass storage devices like HDD, or DVD
    // drive can take up to a few minutes to start up and these devices will always be reported as
    // "stuck" based on our algorithm.
    if (originalStatus->sstStruct.iAccel == 0) // only process non-accel
    {
        // Check if host has accessed the IN endpoint since iHstAcs was set.
        if (originalStatus->sstStruct.iHstAcs == 1)
        { // Endpoint not accessed by host
            boolT isHubInterruptInEndpoint = (isHub && (originalStatus->sstStruct.iEpType == EP_TYPE_INT));

            if(isHubInterruptInEndpoint)
            {
                // For hub endpoints, the host will always keep accessing the endpoint except
                // during suspend-resume cycles. If host issues a suspend when the hub is in the
                // middle of an interrupt IN transaction, we still want to forward the hub
                // transaction to the host after resume instead of flushing the queue.  isHub
                // condition will prevent hub queues from getting cleared.
            }
            else
            {
                updatedStatus->sstStruct.iQid = 0;
                updatedStatus->sstStruct.iNtfyCnt = 0;
                updatedStatus->sstStruct.iBlk = 0;
                updatedStatus->sstStruct.iDet = 0;
                updatedStatus->sstStruct.iClr = 0;
                updatedStatus->sstStruct.iHstAcs = 0;
                updatedStatus->sstStruct.rtyUsage = 0;
            }
        }
        else
        { // Endpoint accessed by host
            // Check whether valid data has been sent from the device
            if (originalStatus->sstStruct.iDet == 1)
            { // No valid data sent from the device
                // There is a small chance of the following case:
                // 1. In the previous iteration, we set Det and HstAcs.
                // 2. After that the host accesses the device once which clear HstAcs but
                //    since we still don't have a response from the device, we leave Det set.
                // 3. After this, the host stops accessing the endpoint.
                // 4. LEX receives a response from the REX.
                // 5. At this point, since the host is no longer accessing the device and the
                //    next transaction will be a new transaction instead of continuation of the
                //    previous transaction, we will set HstAcs, which will cause the endpoint
                //    to be cleared and its queues flushed in the next iteration, instead of
                //    asserting that the endpoint is stuck.

                if (originalStatus->sstStruct.iQid != 0)
                {
                    // Check for a host orphaned endpoint
                    updatedStatus->sstStruct.iHstAcs = 1;
                }
                else
                {
                    _DTT_showDeviceXSST(logicalAddr); // To assist in debugging
                    ilog_TOPOLOGY_COMPONENT_2(
                        ILOG_MAJOR_ERROR, INPUT_STUCK_ENDPOINT, logicalAddr, endptNum);
                    updatedStatus->sstStruct.iClr = 1;

                }
            }
            else
            { // Valid data sent from the device
                if(originalStatus->sstStruct.iBlk || (originalStatus->sstStruct.iEpType == EP_TYPE_ISO))
                {
                    // Setting Det and HstAcs will mark this endpoint to be monitored in the next
                    // iteration.  If Blk is set, it means that we are in the middle of a
                    // transaction and we need to monitor the endpoint.  For ISO IN endpoints,
                    // since these endpoints are periodic, Blk is only set for a short period
                    // (between IN and synthetic Null Data0 response) and we can assume that the
                    // host will issue periodic requests.

                    if (originalStatus->sstStruct.rtyUsage == 0)
                    {
                        // Only set iDet if the retry buffers are unused, otherwise iDet is cleared,
                        // and it will falsely look stuck
                        updatedStatus->sstStruct.iDet = 1;
                    }
                    updatedStatus->sstStruct.iHstAcs = 1;
                }
            }
        }
    }

    // Check OUT Endpoint

    // Check endpoints only if Accel bit is not set
    if (originalStatus->sstStruct.oAccel == 0)
    {
        if (originalStatus->sstStruct.oEpType == EP_TYPE_ISO)
        {
            // Do nothing. ISO out just flows straight to Rex
        }
        else
        {
            if (originalStatus->sstStruct.oHstAcs == 1) // Check if host is accessing the endpoint
            { // Endpoint not accessed by host
                updatedStatus->sstStruct.oQid = 0;
                updatedStatus->sstStruct.oNtfyCnt = 0;
                updatedStatus->sstStruct.oBlk = 0;
                updatedStatus->sstStruct.oDet = 0;
                updatedStatus->sstStruct.oClr = 0;
                updatedStatus->sstStruct.oHstAcs = 0;
            }
            else
            { // Endpoint accessed by host
                // Check whether valid data has been sent from the device
                if (originalStatus->sstStruct.oDet == 1)
                { // No valid data received from the device
                    // There is a small chance of the following case:
                    // 1. In the previous iteration, we set Det and HstAcs.
                    // 2. After that the host accesses the device once which clear HstAcs but since
                    //    we still don't have a response from the device, we leave Det set.
                    // 3. After this, the host stops accessing the endpoint.
                    // 4. LEX receives a response from the REX.
                    // 5. At this point, since the host is no longer accessing the device and the
                    //    next transaction will be a new transaction instead of continuation of the
                    //    previous transaction, we will set HstAcs, which will cause the endpoint
                    //    to be cleared in the next iteration, instead of asserting that the
                    //    endpoint is stuck.
                    if (originalStatus->sstStruct.oQid != 0)
                    {
                        // Check for a host orphaned endpoint
                        updatedStatus->sstStruct.oHstAcs = 1;
                    }
                    else
                    {
                        _DTT_showDeviceXSST(logicalAddr); // To assist in debugging
                        ilog_TOPOLOGY_COMPONENT_2(
                            ILOG_MAJOR_ERROR, OUTPUT_STUCK_ENDPOINT, logicalAddr, endptNum);
                        updatedStatus->sstStruct.oClr = 1;
                    }
                }
                else
                { // Valid data received from the device
                    if(originalStatus->sstStruct.oBlk)
                    {
                        // Setting Det and HstAcs will mark this endpoint to be monitored in the
                        // next iteration.  If Blk is set, it means that we are in the middle of a
                        // transaction and we need to monitor the endpoint.
                        updatedStatus->sstStruct.oDet = 1;
                        updatedStatus->sstStruct.oHstAcs = 1;
                    }
                }
            }
        }
    }
}

