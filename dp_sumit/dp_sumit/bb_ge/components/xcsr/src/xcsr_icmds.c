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
//!   @file  -  xcsr_icmds.c
//
//!   @brief -  This file contains the functions for icmd
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "xcsr_loc.h"
#include <xcsr_xicsq.h>
#include <leon_uart.h>


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static inline void DecodeXsstEntry(struct XCSR_Xsst *pDev, boolT isSplit);
static void DecodeLatEntry(struct XCSR_Lat *pDev);


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: icmdReadQueueStats()
*
* @brief  - get the queue statistics for the given qid
*
* @return - void
*
* @note   -
*
*/
void icmdReadQueueStats
(
    uint8 qid  // Qid to get statistics for
)
{
    struct XCSR_QueueStats qStats;

    if (qid < MAX_QIDS)
    {
        XCSR_XICSQueueGetStats(qid, &qStats);

        ilog_XCSR_COMPONENT_3(ILOG_USER_LOG, XICSQ_ICMD_GET_Q_STATS1, qid, qStats.frameCount, qStats.wordCount);
        if (qStats.emptyStatus == 0)
        {
            ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, XICSQ_ICMD_GET_Q_STATS3, qStats.emptyStatus);
        }
        else
        {
            ilog_XCSR_COMPONENT_3(ILOG_USER_LOG, XICSQ_ICMD_GET_Q_STATS2, qStats.emptyStatus, qStats.data0, qStats.data1);
        }
    }
    else
    {
        ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, ICMD_INVALID_QID, qid);
    }
}


/**
* FUNCTION NAME: icmdReadCacheStats()
*
* @brief  - get statistics for the entire cache
*
* @return - void
*
* @note   -
*
*/
void icmdReadCacheStats()
{
    struct XCSR_QueueStats qStats;
    uint8 qid;
    uint32 cache_status_reg;

    for (qid=1; qid < MAX_QIDS; qid++)
    {
        XCSR_XICSQueueGetStats(qid, &qStats);

        if (qStats.emptyStatus != 0)
        {
            ilog_XCSR_COMPONENT_3(ILOG_USER_LOG, XICSQ_ICMD_GET_Q_STATS1, qid, qStats.frameCount, qStats.wordCount);
            ilog_XCSR_COMPONENT_3(ILOG_USER_LOG, XICSQ_ICMD_GET_Q_STATS2, qStats.emptyStatus, qStats.data0, qStats.data1);
        }
        LEON_UartWaitForTx();
    }
    cache_status_reg = XCSR_XICS_STATUS_READ_REG(XCSR_BASE_ADDR);
    ilog_XCSR_COMPONENT_2(ILOG_USER_LOG, ICMD_CACHE_STATS_QID_SID_FREE_COUNT, XCSR_XICS_STATUS_QIDFREECNT_GET_BF(cache_status_reg),
                          XCSR_XICS_STATUS_SIDFREECNT_GET_BF(cache_status_reg));
}


/**
* FUNCTION NAME: icmdReadQueueFrame()
*
* @brief  - read a frame out of qid
*
* @return - void
*
* @note   -
*
*/
void icmdReadQueueFrame
(
    uint8 qid  // Qid to read from
)
{
    if (qid < MAX_QIDS)
    {
        eXCSR_QueueReadStatusT retVal;
        uint8 i = 0;
        uint8 j = 0;
        struct XCSR_XICSQueueFrame* frameData;
        XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frameData);

        retVal = XCSR_XICSQueueReadFrame(qid, frameData);

        if (retVal != END_OF_FRAME)
        {
            ilog_XCSR_COMPONENT_2(ILOG_USER_LOG, ICMD_READ_FRAME_SHOW_ERROR, qid, retVal);
        }
        else
        {
            ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, ICMD_READ_FRAME_SHOW_Q, qid);
        }

        ilog_XCSR_COMPONENT_2(ILOG_USER_LOG, ICMD_READ_FRAME_SHOW_HEADER, frameData->header.one.word[0], frameData->header.one.word[1]);
        ilog_XCSR_COMPONENT_2(ILOG_USER_LOG, ICMD_READ_FRAME_SHOW_HEADER2, frameData->header.two.word[0], frameData->header.two.word[1]);

        while (i < frameData->dataSize)
        {
            ilog_XCSR_COMPONENT_2(
                ILOG_USER_LOG, ICMD_READ_FRAME_SHOW_DATA,
                ((uint32*)frameData->data)[j], ((uint32*)frameData->data)[j+1]);
            i = i + 8;
            j = j + 2;
            LEON_UartWaitForTx();
        }
    }
    else
    {
        ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, ICMD_INVALID_QID, qid);
    }
}


/**
* FUNCTION NAME: icmdWriteQueueFrame()
*
* @brief  - write a frame to a dynamic queue
*
* @return - void
*
* @note   - writes arbitrary data for the specified number of bytes into a dynamic queue
*
*/
void icmdWriteQueueFrame
(
    uint8 qFrameType,  // The type of frame to write: generic, cpu2cpu, downstream, upstream, other (not really a frame header)
    uint8 numBytes  // Number of bytes to write
)
{
    uint8 qid;
    uint8 i = 0;
    struct XCSR_XICSQueueFrame* frameData;
    // Randomly assigning some values
    XUSB_AddressT address;
    uint8 ep = 7;
    uint8 mod = 0;
    enum XCSR_XUSBAction action = XUSB_IN;
    enum XCSR_Queue queue = 64;
    enum EndpointTransferType epType = EP_TYPE_BULK;
    boolT toggle = 0;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frameData);

    XCSR_initXUSBAddr(&address);
    XCSR_setXUSBAddrUsb(&address, 113);

    ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, ICMD_WRITING_QUEUE, numBytes);

    // Get a dynamic queue
    qid = XCSR_XICSQQueueAllocate(QT_DEFAULT);

    // Build the frame header
    switch (qFrameType)
    {
        case 0: //generic
        {
            frameData->header.one.generic.tagType = 0;
            frameData->header.one.generic.Vport = 2;
            frameData->header.one.generic.frmStruct = 0; // Single frame header, no data
            frameData->header.one.generic.retryMode = 1;
            frameData->header.one.generic.retryEn = 0;
            frameData->header.one.generic.dataQid = 66; // Some random value
            break;
        }
        case 1: //cpu2cpu
        {
            frameData->header.one.cpu_cpu.tagType = 0;
            frameData->header.one.cpu_cpu.Vport = 2;
            frameData->header.one.cpu_cpu.frmStruct = 0;
            frameData->header.one.cpu_cpu.retryMode = 1;
            frameData->header.one.cpu_cpu.retryEn = 0;
            frameData->header.one.cpu_cpu.dataQid = 66;
            frameData->header.one.cpu_cpu.cmdSeq = 0;
            frameData->header.one.cpu_cpu.ctrlLinkType = 1;
            frameData->header.one.cpu_cpu.msg = 2;
            break;
        }
        case 2: //downstream
        {
            XCSR_XICSBuildDownstreamFrameHeader(frameData, XCSR_getXUSBAddrUsb(address), ep, mod, action, queue, epType, toggle, qid);
            break;
        }
        case 3: //upstream
        {
            XCSR_XICSBuildUpstreamFrameHeader(frameData, address, ep, mod, action, queue, epType, toggle, qid);
            break;
        }
        default:
        {
            frameData->header.one.word[0] = 0xdeadbeef;
            frameData->header.one.word[1] = 0x01234567;
            frameData->dataSize = numBytes;
            break;
        }
    }

    //create some data
    while ((i < numBytes) && (i < 72))
    {
        frameData->data[i] = i + 15;
        i++;
    }

    frameData->dataSize = numBytes;

    XCSR_XICSDumpFrame(frameData, ILOG_USER_LOG);
    XCSR_XICSWriteFrame(qid, frameData);

    icmdReadQueueStats(qid);
}


/**
* FUNCTION NAME: XCSR_LATShowEntry()
*
* @brief  - Print out the LAT entry for the given address
*
* @return - nothing
*
* @note   - called by topology component icmds
*
*/
void XCSR_LATShowEntry
(
    uint8 usbAddress  //address to print out LAT entry for
)
{
    struct XCSR_Lat lat;

    lat.lat = XCSR_XSSTReadLogicalAddressTable(usbAddress);
    DecodeLatEntry(&lat);
}


/**
* FUNCTION NAME: XCSR_ShowXUSBAddress
*
* @brief  - Extracts each field from an XUSB address and logs it.
*
* @return - nothing
*
* @note   -
*/
void XCSR_ShowXUSBAddress(
    const XUSB_AddressT address // The XUSB address to display in the log
)
{
    ilog_XCSR_COMPONENT_3(
        ILOG_USER_LOG, XCSR_SHOW_XUSB_ADDRESS_1,
        XCSR_getXUSBAddrLogical(address),
        XCSR_getXUSBAddrUsb(address),
        XCSR_getXUSBAddrVPort(address));
    ilog_XCSR_COMPONENT_3(
        ILOG_USER_LOG, XCSR_SHOW_XUSB_ADDRESS_2,
        XCSR_getXUSBAddrValid(address),
        XCSR_getXUSBAddrInSys(address),
        (uint32)XCSR_getXUSBAddrVirtFuncPointer(address));
}


/**
* FUNCTION NAME: DecodeLatEntry()
*
* @brief  - Decode and print out the given LAT entry into more human readable form
*
* @return - nothing
*
* @note   -
*
*/
static void DecodeLatEntry
(
    struct XCSR_Lat *pDev  // Logical address table data read from HW
)
{
    ilog_XCSR_COMPONENT_3(ILOG_USER_LOG, DECODE_LAT_LA_INSYS_VAL, pDev->latStruct.logicalAddress, pDev->latStruct.inSys, pDev->latStruct.logicalAddressValid);
    ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_LAT_VPORT, pDev->latStruct.vport);
    if (pDev->latStruct.split == 1)
    {
        ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_LAT_SPLIT);
    }
    if (pDev->latStruct.vfen == 1)
    {
        ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_LAT_VFEN);
    }
}


/**
* FUNCTION NAME: XCSR_XSSTShowEntry()
*
* @brief  - Print out the XSST entry for the given address
*
* @return - nothing
*
* @note   - called by topology component icmds
*
*/
void XCSR_XSSTShowEntry
(
    XUSB_AddressT address  //address to print out XSST entry for
)
{
    struct XCSR_Xsst dev;
    uint8 endpoint = 0;
    boolT xsstEmpty = TRUE;
    uint8 usbAddr = XCSR_getXUSBAddrUsb(address);
    boolT isSplit;
    struct XCSR_Lat lat;
    lat.lat = XCSR_XSSTReadLogicalAddressTable(usbAddr);

    isSplit = lat.latStruct.split;
    dev.sst = XCSR_XSSTRead(usbAddr, endpoint);

    if (!XCSR_getXUSBAddrInSys(address))
    {
        // Device is outside the ExtremeUSB topology
        ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, SHOW_XSST_NOT_IN_SYS, XCSR_getXUSBAddrUsb(address));
    }
    else
    {
        ilog_XCSR_COMPONENT_2(ILOG_USER_LOG, SHOW_XSST_IN_SYS, XCSR_getXUSBAddrUsb(address), XCSR_getXUSBAddrLogical(address));

        for (endpoint=0; endpoint<16; endpoint++) //Loop through all endpoints
        {
            dev.sst = XCSR_XSSTRead(XCSR_getXUSBAddrUsb(address), endpoint);
            // only print endpoints that have entries
            // Assume that endpoint 0 is the only Control endpoint
            if ((endpoint == 0) || (dev.sst != 0))
            {
                ilog_XCSR_COMPONENT_3(ILOG_USER_LOG, SHOW_XSST_ENDPOINT_ENTRY, XCSR_getXUSBAddrUsb(address), endpoint, GET_MSW_FROM_64(dev.sst));
                ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, SHOW_XSST_ENDPOINT_ENTRY2, GET_LSW_FROM_64(dev.sst));
                DecodeXsstEntry(&dev, isSplit);
                xsstEmpty = FALSE;
            }
            LEON_UartPollingModeDoWork();
        }
    }

    if (xsstEmpty)
    {
        ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, SHOW_XSST_ALL_ZEROES);
    }
}


/**
* FUNCTION NAME: DecodeXsstEntry()
*
* @brief  - Decode and print out the given XSST entry into more human readable form
*
* @return - nothing
*
* @note   -
*
*/
static inline void DecodeXsstEntry
(
    struct XCSR_Xsst *pDev,  // XSST data read from HW
    boolT isSplit            // Split device
)
{
    boolT validInputEp = FALSE;
    boolT validOutputEp = FALSE;

    // Print all non-control endpoints
    if (pDev->sstStruct.iEpType != EP_TYPE_CTRL)
    {
        validInputEp = TRUE;
    }

    if (pDev->sstStruct.oEpType != EP_TYPE_CTRL)
    {
        validOutputEp = TRUE;
    }

    // Print XSST entry if endpoint is Default Control Endpoint
    if ((pDev->sstStruct.iEpType == EP_TYPE_CTRL) &&
        (pDev->sstStruct.oEpType == EP_TYPE_CTRL))
    {
        validInputEp = TRUE;
        validOutputEp = TRUE;
    }

    // Print IN bits
    if (validInputEp)
    {
        // Print Upper 32 bits
        if (pDev->sstStruct.iBlk == 1)
        {
            ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_IN_BLOCKING);
        }
        if (pDev->sstStruct.iErrCnt != 0)
        {
            ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_XSST_IN_ERRORS, pDev->sstStruct.iErrCnt);
        }
        if (pDev->sstStruct.iHstAcs == 1)
        {
            ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_IHOST_ACCESS);
        }
        if (pDev->sstStruct.iClr == 1)
        {
            ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_IN_CLEAR);
        }
        if (pDev->sstStruct.iDet == 1)
        {
            ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_IN_DETECT);
        }

        // Print Lower 32 bits
        if (pDev->sstStruct.iQid != 0)
        {
            ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_XSST_IN_QID, pDev->sstStruct.iQid);
        }
        if (pDev->sstStruct.iAltClrRsp == 1)
        {
            ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_ALTERNATE_RESPONSE);
        }
        if (pDev->sstStruct.iAccel == 1)
        {
            ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_ACCELERATION);
        }

        switch (pDev->sstStruct.iEpType) // Endpoint type
        {
            case EP_TYPE_CTRL:
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_ENDPOINT_CTRL_IN);
                break;
            case EP_TYPE_ISO:
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_ENDPOINT_ISO_IN);
                break;
            case EP_TYPE_BULK:
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_ENDPOINT_BULK_IN);
                break;
            case EP_TYPE_INT:
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_ENDPOINT_INT_IN);
                break;
            default:
                ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_XSST_INVALID_ENDPOINT_TYPE, pDev->sstStruct.iEpType);
                break;
        }
    }

    // Print OUT bits
    if (validOutputEp)
    {
        // Print Upper 32 bits
        if (pDev->sstStruct.oBlk == 1)
        {
            ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_OUT_BLOCKING);
        }
        if (pDev->sstStruct.oErrCnt != 0)
        {
            ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_XSST_OUT_ERRORS, pDev->sstStruct.oErrCnt);
        }
        if (pDev->sstStruct.oHstAcs == 1)
        {
            ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_OHOST_ACCESS);
        }
        if (pDev->sstStruct.oClr == 1)
        {
            ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_OUT_CLEAR);
        }
        if (pDev->sstStruct.oDet == 1)
        {
            ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_OUT_DETECT);
        }

        // Print Lower 32 bits
        if (pDev->sstStruct.oQid != 0)
        {
            ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_XSST_OUT_QID, pDev->sstStruct.oQid);
        }
        if (pDev->sstStruct.iNtfyCnt != 0)
        {
            ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_XSST_INOTIFY_COUNT, pDev->sstStruct.iNtfyCnt);
        }
        if (pDev->sstStruct.oNtfyCnt != 0)
        {
            ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_XSST_ONOTIFY_COUNT, pDev->sstStruct.oNtfyCnt);
        }
        if (pDev->sstStruct.oAltClrRsp == 1)
        {
            ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_ALTERNATE_RESPONSE);
        }
        if (pDev->sstStruct.oAccel == 1)
        {
            ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_ACCELERATION);
        }

        switch (pDev->sstStruct.oEpType) // Endpoint type
        {
            case EP_TYPE_CTRL:
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_ENDPOINT_CTRL_OUT);
                break;
            case EP_TYPE_ISO:
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_ENDPOINT_ISO_OUT);
                break;
            case EP_TYPE_BULK:
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_ENDPOINT_BULK_OUT);
                break;
            case EP_TYPE_INT:
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_ENDPOINT_INT_OUT);
                break;
            default:
                ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_XSST_INVALID_ENDPOINT_TYPE, pDev->sstStruct.oEpType);
                break;
        }
    }

    if (pDev->sstStruct.rtyUsage != 0)
    {
        ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_XSST_RETRY_USAGE, pDev->sstStruct.rtyUsage);
    }

    // Print Overlay bits
    if (pDev->sstStruct.ovrLay != 0)
    {
        // Print Control Overlay bits
        if ((pDev->sstStruct.iEpType == EP_TYPE_CTRL) &&
                (pDev->sstStruct.oEpType == EP_TYPE_CTRL))
        {
            if (pDev->ovrLay.ctrlEndPointStruct.bco != 0)
            {
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_BCO);
            }
            if (pDev->ovrLay.ctrlEndPointStruct.bci != 0)
            {
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_BCI);
            }
            if (pDev->ovrLay.ctrlEndPointStruct.insysOvrd != 0)
            {
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_EP_INSYS_OVERRIDE);
            }
            if (pDev->ovrLay.ctrlEndPointStruct.setupRspTgl != 0)
            {
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_SETUP_RESP_TOGGLE);
            }
            if (pDev->ovrLay.ctrlEndPointStruct.setupRspPndg != 0)
            {
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_SETUP_RESP_PENDING);
            }
            if (pDev->ovrLay.ctrlEndPointStruct.ctrlRspStall != 0)
            {
                ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_SETUP_RESP_STALL);
            }
        }
        else
        {
            // Overlay fields depend on direction and type because of which
            // the two switch statements were not combined

            // Print non-control Overlay IN bits
            switch (pDev->sstStruct.iEpType) // Endpoint type
            {
                case EP_TYPE_CTRL:
                    break;
                case EP_TYPE_ISO:
                    break;
                case EP_TYPE_BULK:
                    if (pDev->ovrLay.iBulkMsaEndPointStruct.msaEpPair != 0)
                    {
                        ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_XSST_MSAPAIREP,
                                pDev->ovrLay.iBulkMsaEndPointStruct.msaEpPair);
                    }
                    break;
                case EP_TYPE_INT:
                    if (pDev->ovrLay.iIntEndPointStruct.fullHalfRate != 0)
                    {
                        ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_INT_FULLHALFRATE);
                    }
                    if (pDev->ovrLay.iIntEndPointStruct.copyToCpu != 0)
                    {
                        ilog_XCSR_COMPONENT_0(ILOG_USER_LOG, DECODE_XSST_INT_COPYTOCPU);
                    }
                    break;
                default:
                    ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_XSST_INVALID_ENDPOINT_TYPE, pDev->sstStruct.iEpType);
                    break;
            }

            // Print non-control Overlay OUT bits
            switch (pDev->sstStruct.oEpType) // Endpoint type
            {
                case EP_TYPE_CTRL:
                    break;
                case EP_TYPE_ISO:
                    break;
                case EP_TYPE_BULK:
                    if (pDev->ovrLay.oBulkMsaEndPointStruct.msaEpPair != 0)
                    {
                        ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_XSST_MSAPAIREP,
                                pDev->ovrLay.oBulkMsaEndPointStruct.msaEpPair);
                    }
                    break;
                case EP_TYPE_INT:
                    break;
                default:
                    ilog_XCSR_COMPONENT_1(ILOG_USER_LOG, DECODE_XSST_INVALID_ENDPOINT_TYPE, pDev->sstStruct.oEpType);
                    break;
            }

        }
    }
    
}//end DecodeXsstEntry


// Send a message over the link by CPU TX Q
void icmdSendMessage
(
    uint8 msgType,
    uint8 message,
    uint8 vport
)
{
    XCSR_XICSSendMessage(msgType, message, vport);
}

void icmdSendMessageWithData
(
    uint8 msgType,
    uint8 message,
    uint8 vport,
    uint32 data
)
{
    XCSR_XICSSendMessageWithData(msgType, message, vport, data);
}

void icmdSendMessageWithExtraData
(
    uint8 msgType,
    uint8 message,
    uint8 vport,
    uint32 data,
    uint32 extraDataMSW,
    uint32 extraDataLSW
)
{
    XCSR_XICSSendMessageWithExtraData(msgType, message, vport, data, MAKE_U64(extraDataMSW, extraDataLSW));
}

