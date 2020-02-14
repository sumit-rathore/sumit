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
//!   @file  -  xcsr_log.h
//
//!   @brief -  The XCSR driver logs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XCSR_LOG_H
#define XCSR_LOG_H

/***************************** Included Headers ******************************/

#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

ILOG_CREATE(XCSR_COMPONENT)
    ILOG_ENTRY(XCSR_Q_ALLOCATE_ERROR_LOG, "Queue allocate error @ line %d\n")
    ILOG_ENTRY(XCSR_INVALID_QID_ERROR, "Invalid QID received during static Q allocation for Q %d, expecting %d @ line %d\n")
    ILOG_ENTRY(XCSR_Q_FLUSH_ERROR_LOG, "Invalid QID %d received in XCSR queue flush @ line %d\n")
    ILOG_ENTRY(XCSR_Q_WRITE_ERROR_LOG, "Queue write error @ line %d\n")
    ILOG_ENTRY(XCSR_Q_STATUS_ERROR_LOG, "Invalid QID %d received in XCSR queue get empty status @ line %d\n")
    ILOG_ENTRY(XCSR_READ_LOOKUP_TABLE_INVALID_ARG_ERROR_LOG, "Invalid argument received in XCSR_ReadDeviceStatus @ line %d\n")
    ILOG_ENTRY(XCSR_INVALID_XUSB_CHIP_ID_ERROR_LOG, "Invalid XUSB chip ID, expecting 0x%x, read 0x%x\n")
    ILOG_ENTRY(XCSR_INVALID_ENDPOINT_NUMBER, "Invalid endpoint number %d @ line %d\n")
    ILOG_ENTRY(XCSR_INVALID_ENDPOINT_TYPE, "Invalid endpoint type %d @ line %d\n")
    ILOG_ENTRY(XCSR_WRITE_FRAME, "XCSR_XICSWriteFrame writing qid %d, with header 0x%x 0x%x\n")
    ILOG_ENTRY(XCSR_XSST_WRITE_MASK_LSW, "XCSR_XSSTWriteMask LSW value is 0x%x, mask is 0x%x, old value is 0x%x\n")
    ILOG_ENTRY(XCSR_XSST_WRITE_MASK_MSW, "XCSR_XSSTWriteMask MSW value is 0x%x, mask is 0x%x, old value is 0x%x\n")
    ILOG_ENTRY(XICSQ_ICMD_GET_Q_STATS1, "Qid %d: frame count 0x%x, word count 0x%x\n")
    ILOG_ENTRY(XICSQ_ICMD_GET_Q_STATS2, "empty status %d, data0 0x%x, data1 0x%x\n")
    ILOG_ENTRY(CLR_BCO, "Clearing BCO for usb %d, logical %d, valid(msb) / insys(lsb) as 2 bits 0x%x\n")
    ILOG_ENTRY(CLR_BCI, "Clearing BCI for usb %d, logical %d, valid(msb) / insys(lsb) as 2 bits 0x%x\n")
    ILOG_ENTRY(XCSR_INVALID_XUSB_CHIP_MINOR_REVISION_ERROR_LOG, "Invalid minor XUSB chip revision, expecting 0x%x, read 0x%x\n")
    ILOG_ENTRY(XCSR_INVALID_XUSB_CHIP_MAJOR_REVISION_ERROR_LOG, "Invalid major XUSB chip revision, expecting 0x%x, read 0x%x\n")
    ILOG_ENTRY(XCSR_Q_STATUS_ERROR_CONTAIN_COMPLT_FRAME, "Invalid QID %d in XCSR_XICSQueueContainsCompleteFrame\n")
    ILOG_ENTRY(XCSR_XICS_WRITE_FRAME_NULL_ARG, "XCSR_XICSWriteFrame passed a null arg\n")
    ILOG_ENTRY(ICMD_INVALID_QID, "Invalid qid %d\n")
    ILOG_ENTRY(XICSQ_ICMD_GET_Q_STATS3, "empty status %d\n")

    ILOG_ENTRY(ICMD_CACHE_STATS_QID_SID_FREE_COUNT, "Free QIDs %d, free SIDs %d\n")
    ILOG_ENTRY(ICMD_READ_FRAME_SHOW_ERROR, "Reading from Q %d, Error %d\n")
    ILOG_ENTRY(ICMD_READ_FRAME_SHOW_Q, "Reading from Q %d\n")
    ILOG_ENTRY(ICMD_READ_FRAME_SHOW_HEADER, "Frame header: 0x%x 0x%x\n")
    ILOG_ENTRY(ICMD_READ_FRAME_SHOW_HEADER2, "              0x%x 0x%x\n")
    ILOG_ENTRY(ICMD_READ_FRAME_SHOW_DATA, "Frame data: 0x%x 0x%x\n")

    ILOG_ENTRY(WRITE_FRAME_TOO_MUCH_DATA, "Write Frame was called with too much data, %d bytes to be exact\n")

    ILOG_ENTRY(IRQ2XICSSIDEMPTY, "XICS SIDs are exhausted\n")
    ILOG_ENTRY(IRQ2XICSSIDAEMPTYLTHRESH, "Above XICS SID almost empty low threshold\n")
    ILOG_ENTRY(IRQ2XICSSIDAEMPTYMTHRESH, "Above XICS SID almost empty medium threshold\n")
    ILOG_ENTRY(IRQ2XICSSIDAEMPTYHTHRESH, "Above XICS SID almost empty high threshold\n")
    ILOG_ENTRY(IRQ2XICSQIDEMPTY, "IRQ2XICSQIDEMPTY\n")
    ILOG_ENTRY(IRQ2XICSQIDAEMPTY, "IRQ2XICSQIDAEMPTY\n")

    ILOG_ENTRY(DUMP_FRAME_NULL_ARG, "XCSR_XICSDumpFrame: got null arg\n")
    ILOG_ENTRY(DUMP_FRAME_HEADER_1, "XCSR_XICSDumpFrame: 1st header cache word 0x%x 0x%x\n")
    ILOG_ENTRY(DUMP_FRAME_HEADER_2, "XCSR_XICSDumpFrame: 2nd header cache word 0x%x 0x%x\n")
    ILOG_ENTRY(DUMP_FRAME_SIZE, "XCSR_XICSDumpFrame: Frame size is %d bytes\n")
    ILOG_ENTRY(DUMP_FRAME_RAW_BYTE, "XCSR_XICSDumpFrame: raw dump %.02x\n")

    ILOG_ENTRY(LOG_XSST_WRITE_LAT, "XSST Write LAT addr %d endPoint %d, Data 0x%x\n")
    ILOG_ENTRY(LOG_XSST_WRITE_SST, "XSST Write addr(MSW)/endPoint(LSW) 0x%x, Data 0x%x 0x%x\n")
    ILOG_ENTRY(LOG_XSST_READ, "XSST Read addr(MSW)/endPoint(LSW) 0x%x, Data 0x%x 0x%x\n")
    ILOG_ENTRY(LOG_LAT_READ, "LAT Read addr %d endPoint %d, Data 0x%x\n")
    ILOG_ENTRY(WRITING_ENDPOINT_DATA1, "writing XSST LA %d, endpoint number %d, endpoint type %d\n")
    ILOG_ENTRY(WRITING_ENDPOINT_DATA2, "value: 0x%x 0x%x\n")
    ILOG_ENTRY(WRITING_ENDPOINT_DATA3, "mask:  0x%x 0x%x\n")
    ILOG_ENTRY(FLUSHED_AND_DEALLOCATED_QID, "QID %d flushed and deallocated\n")

    ILOG_ENTRY(SHOW_XSST_NOT_IN_SYS, "USB %d, Not in-sys\n")
    ILOG_ENTRY(SHOW_XSST_IN_SYS, "USB %d, LA %d\n")
    ILOG_ENTRY(SHOW_XSST_ENDPOINT_ENTRY, "  XSST(%d,%d) = 0x%x\n")
    ILOG_ENTRY(SHOW_XSST_ENDPOINT_ENTRY2, "              0x%x\n")
    ILOG_ENTRY(SHOW_XSST_ALL_ZEROES, "  XSST is all zeros\n")
    ILOG_ENTRY(DECODE_XSST_IN_BLOCKING, "  IN Blocking set\n")
    ILOG_ENTRY(DECODE_XSST_OUT_BLOCKING, "  OUT Blocking set\n")
    ILOG_ENTRY(DECODE_XSST_IN_ERRORS, "  In Errors = %d\n")
    ILOG_ENTRY(DECODE_XSST_OUT_ERRORS, "  Out Errors = %d\n")
    ILOG_ENTRY(DECODE_XSST_RETRY_USAGE, "  Retry usage = %d\n")
    ILOG_ENTRY(DECODE_XSST_BCO, "  BCO set\n")
    ILOG_ENTRY(DECODE_XSST_BCI, "  BCI set\n")
    ILOG_ENTRY(DECODE_XSST_EP_INSYS_OVERRIDE, "  Endpoint in-sys override set\n")
    ILOG_ENTRY(DECODE_XSST_SETUP_RESP_TOGGLE, "  Setup response toggle set\n")
    ILOG_ENTRY(DECODE_XSST_SETUP_RESP_PENDING, "  Setup response pending set\n")
    ILOG_ENTRY(DECODE_XSST_SETUP_RESP_STALL, "  Setup response stall set\n")
    ILOG_ENTRY(DECODE_XSST_INVALID_ENDPOINT_TYPE, "  Invalid Endpoint type %d in XSST entry\n")
    ILOG_ENTRY(DECODE_XSST_IN_CLEAR, "  IN Clear set\n")
    ILOG_ENTRY(DECODE_XSST_OUT_CLEAR, "  OUT Clear set\n")
    ILOG_ENTRY(DECODE_XSST_IN_DETECT, "  IN Detect set\n")
    ILOG_ENTRY(DECODE_XSST_OUT_DETECT, "  OUT Detect set\n")
    ILOG_ENTRY(DECODE_XSST_OUT_QID, "  Out QID = %d\n")
    ILOG_ENTRY(DECODE_XSST_IN_QID, "  In QID = %d\n")
    ILOG_ENTRY(DECODE_XSST_ALTERNATE_RESPONSE, "  Alternate response set\n")
    ILOG_ENTRY(DECODE_XSST_ACCELERATION, "  Acceleration set\n")

    ILOG_ENTRY(DECODE_LAT_LA_INSYS_VAL, "LA %d: in-sys %d, valid %d\n")
    ILOG_ENTRY(DECODE_LAT_VPORT, "  Vport ID %d\n")
    ILOG_ENTRY(DECODE_LAT_SPLIT, "  Split device\n")
    ILOG_ENTRY(DECODE_LAT_VFEN, "  Virtual function enabled\n")


    ILOG_ENTRY(XCSR_NON_ZERO_IN_QID_BEFORE_WRITE, "Nonzero in endpoint QID seen before upstream frame: qid %d @ line %d\n")
    ILOG_ENTRY(XCSR_NON_ZERO_OUT_QID_BEFORE_WRITE, "Nonzero out endpoint QID seen before upstream frame: qid %d @ line %d\n")

    ILOG_ENTRY(ICMD_WRITING_QUEUE, "Write Queue iCmd: Writing a queue with %d bytes of arbitrary data\n")
    ILOG_ENTRY(UPDATE_ADDRESS, "Updating Address USB:%d,LA:%d,VPORT:%d\n")
    ILOG_ENTRY(DECODE_XSST_INOTIFY_COUNT, "  iNotify Count = %d\n")
    ILOG_ENTRY(DECODE_XSST_ONOTIFY_COUNT, "  oNotify Count = %d\n")
    ILOG_ENTRY(XCSR_NON_ZERO_INTFYCNT_BEFORE_WRITE, "Nonzero iNotify count seen before upstream frame: count %d @ line %d\n")
    ILOG_ENTRY(XCSR_NON_ZERO_ONTFYCNT_BEFORE_WRITE, "Nonzero oNotify count seen before upstream frame: count %d @ line %d\n")
    ILOG_ENTRY(XSST_BITFIELD_CHECK_FAILURE, "XSST bitfield check failed at line %d\n")


    ILOG_ENTRY(XCSR_SHOW_XUSB_ADDRESS_1, "XUSB Address - Logical Address: %d, USB Address: %d, VPort: %d\n")
    ILOG_ENTRY(XCSR_SHOW_XUSB_ADDRESS_2, "               USB Valid: %d, In System: %d, Virtual Function: 0x%x\n")

    ILOG_ENTRY(LAT_BITFIELD_CHECK_FAILURE, "LAT bitfield check failed at line %d\n")
    ILOG_ENTRY(XSST_CONDITIONAL_WRITE_FAILURE, "XSST conditional write failed! LA%d: USB address %d, endpoint %d\n")
    ILOG_ENTRY(DECODE_XSST_IHOST_ACCESS, "  In Host Access set\n")
    ILOG_ENTRY(DECODE_XSST_OHOST_ACCESS, "  Out Host Access set\n")
    ILOG_ENTRY(Q_UNDERFLOW_ERROR_DETAILS, "A queue underflow occurred while performing operation %d (0=READ,1=WRITE) on Q ID %d from interface %d\n")
    ILOG_ENTRY(Q_OVERFLOW_ERROR_DETAILS, "A queue overflow occurred while performing operation %d (0=READ,1=WRITE) on Q ID %d from interface %d\n")
    ILOG_ENTRY(XCSR_READ_EMPTY_Q, "Tried to read an empty queue (%d) at line %d\n")
    ILOG_ENTRY(INVALID_QID, "Received an invalid qid (%d) at line %d\n")
    ILOG_ENTRY(XCSR_ASSERT_Q_STATS, "--- XCSR Non-Empty Queue Stats: ---\n")
    ILOG_ENTRY(XCSR_ASSERT_Q_STATS_1, "Qid %d: frame count 0x%x, word count 0x%x\n")
    ILOG_ENTRY(XCSR_ASSERT_Q_STATS_2, "    data0 0x%x, data1 0x%x\n")

    ILOG_ENTRY(XICSSTATUS, "Xics Status : Qid %d Sid %d\n")

    ILOG_ENTRY(XUSBIRQ_XCRMNOTINSYS,   "We got an upstream packet for an endpoint that is not in sys\n")
    ILOG_ENTRY(XUSBIRQ_XCRMAFIFOUFLOW, "The xcrm fifo has underflowed, damn fifo!!\n")
    ILOG_ENTRY(XUSBIRQ_XCRMAFIFOOFLOW, "The xcrm fifo has overflowed, who designed this thing!!\n")
    ILOG_ENTRY(XUSBIRQ_XCRMAFIFOSYNC,  "The xcrm fifo synced?? what is that, a disease?\n")
    ILOG_ENTRY(XUSBIRQ_XCRMDROPFRM,    "The xcrm has dropped a frame, why?\n")
    ILOG_ENTRY(XUSBIRQ_XCRMFRAMING,    "xcrm framing error, how lame!\n")
    ILOG_ENTRY(XUSBIRQ_XCRMFRMCRC,     "xcrm frame crc error, how scary\n")
    ILOG_ENTRY(XUSBIRQ_XCRMPLDCRC,     "xcrm payload error, ouch\n")
    ILOG_ENTRY(XUSBIRQ_XCRMTOUT,       "xcrm time out, canucks suck!!\n")
    ILOG_ENTRY(XUSBIRQ_XCTMAFIFOUFLOW, "The xctm fifo has underflowed, stupid fifo\n")
    ILOG_ENTRY(XUSBIRQ_XCTMAFIFOOFLOW, "The xctm fifo has overflowed, Ken is this your??\n")
    ILOG_ENTRY(XUSBIRQ_XCTMPKT,        "xctm packet framing error, boring!\n")
    ILOG_ENTRY(IRQ2XICSSIDAEMPTYNTHRESH, "Below XICS SID almost empty low threshold\n")
    ILOG_ENTRY(FLC_EVENT_LOCAL_INTERRUPT,  "Received local flow control event interrupt\n")
    ILOG_ENTRY(FLC_EVENT_REMOTE_INTERRUPT, "Received remote flow control event interrupt\n")
    ILOG_ENTRY(DECODE_XSST_ENDPOINT_CTRL_IN, "  Endpoint type: CTRL IN\n")
    ILOG_ENTRY(DECODE_XSST_ENDPOINT_ISO_IN, "  Endpoint type: ISO IN\n")
    ILOG_ENTRY(DECODE_XSST_ENDPOINT_BULK_IN, "  Endpoint type: BULK IN\n")
    ILOG_ENTRY(DECODE_XSST_ENDPOINT_INT_IN, "  Endpoint type: INT IN\n")
    ILOG_ENTRY(DECODE_XSST_ENDPOINT_CTRL_OUT, "  Endpoint type: CTRL OUT\n")
    ILOG_ENTRY(DECODE_XSST_ENDPOINT_ISO_OUT, "  Endpoint type: ISO OUT\n")
    ILOG_ENTRY(DECODE_XSST_ENDPOINT_BULK_OUT, "  Endpoint type: BULK OUT\n")
    ILOG_ENTRY(DECODE_XSST_ENDPOINT_INT_OUT, "  Endpoint type: INT OUT\n")
    ILOG_ENTRY(DECODE_XSST_MSAPAIREP, "  MSA Paired Endpoint: %d\n")
    ILOG_ENTRY(DECODE_XSST_INT_COPYTOCPU, "  Copy IN response data to CPU\n")
    ILOG_ENTRY(DECODE_XSST_INT_FULLHALFRATE, "  Standard forwarding to maximize effective polling rate\n")
    ILOG_ENTRY(XCSR_VP_LINK_DOWN, "Vport %d down.  Cleaning up flow control rules.\n")
    ILOG_ENTRY(RELEASE_RETRY, "In XSST Reset, releasing Retry Buffers for USB Addr: %d, Endpoint: %d\n")
    ILOG_ENTRY(XCSR_SPECTAREG_READ, "Read XCSR Register: 0x%x, Value: 0x%x\n")
    ILOG_ENTRY(XCTM_DISABLE_FAILURE, "Unable to disable the XCTM\n")
    ILOG_ENTRY(XICS_READ_Q_SNOOP_ERR, "Unable to snoop queue: %d\n")
    ILOG_ENTRY(IRQ2XURMPKTLIMIT, "IRQ2XURMPKTLIMIT\n")
    ILOG_ENTRY(XICS_SENDING_CSPLIT_TOWARDS_HS_DEVICE, "Sending a CSPLIT towards a high speed device?! Device address = %d\n")
    ILOG_ENTRY(XCSR_DEALLOCATED_STATIC_QUEUE, "Attempted to deallocate a static queue: qid %d\n")
    ILOG_ENTRY(XCSR_MSA_OUT_INT_IN_DISABLED_OPTIMIZED_NAK, "Disabled optimized NAK for endpoint %d at USB address %d\n")
ILOG_END(XCSR_COMPONENT, ILOG_MINOR_EVENT)

#endif // #ifndef XCSR_LOG_H

