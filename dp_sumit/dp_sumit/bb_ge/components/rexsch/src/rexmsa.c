
///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013, 2014
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
//!   @file  -  rexmsa.c
//
//!   @brief -  Schedules all MSA transactions out of rex.
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
/*
   Token Responses

   Out
      Ack      Device Acked this packet so move to the next
      Nak      Not valid since the hardware will send a Ping after this
      NYet     Device has Acked this packet and wants a Ping sent before the next Out
               Only send the Ping if all of the Out data has not been sent.
      Stall    Device does not want any more data, so flush all data for this endp and
               send the Stall upstream
      Timeout  Host sent a Out packet with a bad crc. Just ignore this since the host will retry
      3k       Rex retried this packet 3 times. Flush all data for this endp and send upstream Stall

   Ping
      Ack      No valid since the hardware will retry the Out after the Ping Ack
      Nak      Will occur if the nak counter is reached, Send another Ping
      Stall    Device does not want any more data, so flush all data for this endp and
               send the Stall upstream
      3k       Rex retried this packet 3 times. Flush all data for this endp and send upstream Stall

   In
      (error)  Rex received a crc error or pid data error so just ignore this packet
      Datax    Received a data packet so send another In if we have not received all of the data
               The hardware sends the data packet upstream
      Nak      Will occur if the nak counter is reached, Send another In
      Stall    Device does not want any more data, so send the Stall upstream
      3k       Rex retried this packet 3 times. Send upstream Stall
*/
/*
   Error Handling

   Out
      timeout     hardware will retry
      pid error   hardware will retry
      crc error   This is for the packet sent from Rex, sw should ignore this

   Ping
      timeout     hardware will retry
      pid error   hardware will retry

   In
      timeout        hardware will retry
      pid error      hardware will retry
      pid data error hardware will retry, frame header will be sent to sw and sw should ignore
      crc error      hardware will retry, frame header will be sent to sw and sw should ignore

*/

/***************************** Included Headers ******************************/
#include "rexsch_loc.h"
#include <usbdefs.h>

/************************ Defined Constants and Macros ***********************/
enum MSA_STATE
{
    MSA_STATE_IDLE = 0,         // 0
    MSA_STATE_CBW,              // 1
    MSA_STATE_DATA,             // 2 // TODO: still used ? remove?
    MSA_STATE_IN_DATA,          // 3
    MSA_STATE_OUT_DATA,         // 4
    MSA_STATE_CSW,              // 5
    MSA_STATE_STALL_SETUP_RESP, // 6
    MSA_STATE_STALL_IN_RESP,    // 7
    MSA_STATE_IN_FLC_PAUSE,     // 8
    MSA_STATE_PING,             // 9
    MSA_STATE_START_CLEANUP     // 10
};

enum _REXMSA_PacketStatus
{
    PACKET_ADDED_TO_QUEUE = 0,
    PACKET_SENT,
    PACKET_NOT_SENT
};

enum _REXMSA_CBWDirection
{
    CBW_DIR_OUT,
    CBW_DIR_IN
};

enum _REXMSA_FlushType
{
    FLUSH_ALL,
    FLUSH_OUT_ONLY
};

// this will set the max # naks to MAX_NAK_CNT * 65536
// 256 = 16,777,216 naks
#define MAX_NAK_CNT  256

struct _REXMSA_MsaStatCounter
{
    uint32 cbw_cnt;
    uint32 csw_cnt;
    uint32 in_stall_cnt;
    uint32 out_stall_cnt;
    uint32 send_clr_halt_cnt;
    uint32 flc_in_cnt;
};

enum _REXMSA_SendDownstreamMode
{
    SEND_TO_DEVICE_FROM_QUEUE,
    SEND_TO_DEVICE_OR_ADD_TO_QUEUE,
    RETRY_DOWNSTREAM,
    SEND_TO_DEVICE_NEXT_UFRAME,
    SEND_TO_DEVICE_WITH_NEW_PID,
    SEND_DOWNSTREAM
};

/******************************** Data Types *********************************/

struct _REXSCH_MsaStatus
{
   // The length (in bytes) of the MSA transaction as specified in the CBW by the
   // dCBWDataTransferLength field.
   uint32 data_len;
   // counter that accumulates the length of every IN or OUT data packet
   uint32 data_cnt;
   // NOTE: at the end of the MSA transaction, the following conditions must be met:
   // for OUT transaction: data_cnt == data_len
   // for IN transaction : data_cnt <= data_len, since the device is allowed to send
   //                      a short packet

   // debug
   uint32 in_nak_cnt;
   uint32 out_nak_cnt;

   // number of packets in the queue with QID = msaQid
   // KARAN: Need to replace this with a boolT variable named "isResponsePending"
   // The number of packets in msaQid queue can be retrieved by using XRR_Get_Q_Stat function
   uint16 numPacketsInQueue;

   enum MSA_STATE state;
   uint8 addr;
   uint8 endp;

   // queue where downstream MSA packets are stored
   uint8 msaQid;

   // DATA0/DATA1 toggle checks from Lex
   // valid values are 0x00 (unknown), 0xC3 (DATA0_PID) or 0x4B (DATA1_PID)
   // unknown or uninitialized is 0x0 so the memset can just clean the whole struct
   uint8 lastOutToggle;
   uint8 lastInToggle;
};

/************************ Local Function Prototypes **************************/

static void _REXMSA_SendClearEndpointHaltSetup(
   uint64 frameHeader,
   struct _REXSCH_MsaStatus *status)
      __attribute__((section(".rextext")));

static void _REXMSA_FlushQueue(
        struct _REXSCH_MsaStatus * status,
        enum _REXMSA_FlushType flushType) __attribute__((section(".rextext")));

static enum _REXMSA_PacketStatus _REXMSA_SendDownstream(
        struct _REXSCH_MsaStatus * status,
        enum _REXMSA_SendDownstreamMode mode,
        uint8 action) __attribute__((section(".rextext")));

static void _REXMSA_SendUpstream(
        enum REXSCH_Response) __attribute__((section(".rextext")));

void _REXMSA_SendToDevice (
   XRR_FrameHeaderRaw         raw,
   enum XRR_SchType           sch_type,
   struct _REXSCH_MsaStatus   *status)
      __attribute__((section(".rextext")));

static struct _REXSCH_MsaStatus * _REXMSA_InitMsaStatusAndAllocateQueue(
   uint8 addr, uint8 endp)
      __attribute__((section(".rextext")));

static void _REXMSA_ResetMsaStatusAndDeallocateQueue(
        struct _REXSCH_MsaStatus * status) __attribute__((section(".rextext")));

static void _REXMSA_DisplayMsaStatCounters(void);

static void _REXMSA_RestartPausedEndpoints(void)
   __attribute__((section(".rextext")));

static struct _REXSCH_MsaStatus* _REXMSA_GetMsaStatusEntry(uint8 usbAddr)
   __attribute__((section(".rextext")));

void Process_CBW_Response (
   const union XRR_FrameHeader *frmHdr,
   const union XRR_InputFrameHeader2 *frmHdr2,
   struct _REXSCH_MsaStatus *status)
      __attribute__((section(".rextext")));

void Process_Out_Ping_Response(
   const union XRR_FrameHeader *frmHdr,
   const union XRR_InputFrameHeader2 *frmHdr2,
   struct _REXSCH_MsaStatus *status)
      __attribute__((section(".rextext")));

void Process_In_Response (
   const union XRR_FrameHeader *frmHdr,
   const union XRR_InputFrameHeader2 *frmHdr2,
   struct _REXSCH_MsaStatus *status)
      __attribute__((section(".rextext")));

void Process_In_CSW_Response (
   const union XRR_FrameHeader *frmHdr,
   struct _REXSCH_MsaStatus *status)
      __attribute__((section(".rextext")));

/***************************** Local Variables *******************************/

struct _REXSCH_MsaStatus msaStatus[MSA_ENDP_COUNT] __attribute__((section(".rexbss")));
struct _REXMSA_MsaStatCounter msaStatCounter __attribute__((section(".rexbss")));
boolT inDirectionFlowControlEnabled __attribute__((section(".rexbss")));
uint32 sof_cnt __attribute__((section(".rexbss")));

// MSA transaction is currently in progress; save the setup packet
// to _REXSCH_SetupDataQid dynamic queue and once the REX receives
// a response from the device, the REX will reset the MSA state
// and send this setup packet downstream.
uint8 _REXSCH_SetupDataQid __attribute__((section(".rexbss")));

/************************** Function Definitions *****************************/

/**
 * FUNCTION NAME: REXMSA_ResetMsaStatus
 *
 * @brief  - Clears all of the entries in the MSA status table, MSA status counters and SOF
 *           counter.
 *
 * @return - void
 *
 * @note   - Must be called before any mass storage packets are sent.
 */
void REXMSA_ResetMsaStatus(void)
{
    uint32 index;

    ilog_REXSCH_COMPONENT_1(ILOG_MAJOR_EVENT, MSA_INIT, MSA_ENDP_COUNT);

    // All of the field need to be set to zero, so just memset the whole array.
    memset(msaStatus, 0, sizeof(struct _REXSCH_MsaStatus) * MSA_ENDP_COUNT);

    // clear the msa status table
    for (index = 0; index < MSA_ENDP_COUNT; index++)
    {
        msaStatus[index].state = MSA_STATE_IDLE;
    }

    memset(&msaStatCounter, 0, sizeof(struct _REXMSA_MsaStatCounter));

    inDirectionFlowControlEnabled = FALSE;

    sof_cnt = 0;
}

/**
 * FUNCTION NAME: REXMSA_ScheduleMsaPacketDownstream
 *
 * @brief  - Called whenever a packet is delivered to the scheduled MSA queue from the LEX.
 *
 * @return - void
 *
 * @note   - This is triggered by the rdfrmHdr.generic interrupt.
 *         - The rex scheduler will take this packet and schedule it to another queue for sending
 *           to the device.
 */
void REXMSA_ScheduleMsaPacketDownstream(void)
{
    union XRR_FrameHeader frmHdr;
    union XRR_InputFrameHeader2 frmHdr2;
    struct _REXSCH_MsaStatus* status;
    boolT sendOrAddPacketToQueue = TRUE;

    // Reading the SQ_SCH_MSA Queue will update the RDFRM_HDR* registers
    // Populate the structure
    frmHdr.raw = XRR_ReadQueue(REX_SQ_SCH_MSA);
    frmHdr2.frameHeader = XRR_GetInputFrameHeader2();

    // get a reference to the msa stat for this address
    status = _REXMSA_GetMsaStatusEntry(frmHdr.generic.addr);

    // get a new msa status entry to save the msa stats, and get a queue
    if (status == NULL) {
        status = _REXMSA_InitMsaStatusAndAllocateQueue(frmHdr.generic.addr, frmHdr.generic.endp);

        // LEX RTL needs to keep track of the number of simultaneous MSA transactions and if it
        // exceeds N, the next bulk transaction will be un-accelerated
        iassert_REXSCH_COMPONENT_0(status != NULL, TOO_MANY_SIMULTANEOUS_TRAN);
    }

    if (frmHdr.generic.action == ACTION_OUT)
    {
        // Check the toggle

        uint64 snoopedData;
        uint8 pid;

        iassert_REXSCH_COMPONENT_2(frmHdr.generic.dataQid != 0, INVALID_STATE, __LINE__, 0);

        // KARAN: For snooping purposes, usbdefs.h should probably have a union struct with pid and
        // data, where data could be IN data, OUT data or standard setup data.
        snoopedData = XCSR_XICSQueueSnoopHead(frmHdr.generic.dataQid);

        // KARAN: struct might be easier to read
        pid = (snoopedData >> 56ULL) & 0xFF;

        // this is MSA, ensure the PID is valid
        iassert_REXSCH_COMPONENT_2(
            (pid == DATA0_PID) || (pid == DATA1_PID), INVALID_STATE, __LINE__, pid);

        if (status->lastOutToggle == pid)
        {
            ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_ERROR, DUPLICATED_OUT_TOGGLE);
        }
        status->lastOutToggle = pid;
    }

    // CSW data is always upstream
    // ?? why is this here. should never get a downstream csw
    iassert_REXSCH_COMPONENT_0(
        frmHdr2.frameHeaderStruct.accel != FH_ACCEL_MSA_CSW, DOWNSTREAM_CSW);

    switch (status->state) {
        case MSA_STATE_IDLE:
            if (frmHdr2.frameHeaderStruct.accel != FH_ACCEL_MSA_CBW) {
                // If the LEX gets a CBW with invalid signature, it will set the acceleration type
                // to FH_ACCEL_NOT and send the packet to the REX MSA queue. REX SW will change the
                // schedule type to SCHTYPE_NONE which informs the REX RTL that the remainder of
                // the transaction will be handled by REX RTL.
                ilog_REXSCH_COMPONENT_2(ILOG_MINOR_EVENT, MSA_NOT_CBW_PKT_IN_IDLE,
                     frmHdr2.frameHeaderStruct.accel, frmHdr.generic.action);
                _REXMSA_ResetMsaStatusAndDeallocateQueue(status);
                // send the packet to the device and do not schedule so any response
                // goes directly upstream
                XRR_UpdateSchPacket(SCHTYPE_NONE, REX_SQ_SCH_ASYNC_OTB);
                sendOrAddPacketToQueue = FALSE;
            } else {
                // check that action is out
                iassert_REXSCH_COMPONENT_2(frmHdr.generic.action == ACTION_OUT,
                    MSA_NOT_CBW_PKT_IN_IDLE, frmHdr2.frameHeaderStruct.accel, frmHdr.generic.action);
                // Valid CBW
                msaStatCounter.cbw_cnt++; // debug stats
                status->state = MSA_STATE_CBW;
            }
            break;

        case MSA_STATE_CBW:
            // we should not get a downstream packet in this state since we are waiting
            // for the response to the CBW
            iassert_REXSCH_COMPONENT_2(FALSE, MSA_GOT_DNS_PKT_IN_CBW,
                frmHdr2.frameHeaderStruct.accel, frmHdr.generic.action);
            break;

        // We are now sending Out's to the device so add any more to a queue
        case MSA_STATE_OUT_DATA:
        case MSA_STATE_PING:
            {
                enum _REXMSA_PacketStatus packetStatus;
                iassert_REXSCH_COMPONENT_0((frmHdr2.frameHeaderStruct.accel != FH_ACCEL_MSA_CBW),
                        MSA_DBG_CBW_OUT_DATA);
                // send packet to device or add to queue if the previous OUT-data is in progress
                packetStatus = _REXMSA_SendDownstream(status, SEND_TO_DEVICE_OR_ADD_TO_QUEUE, 0);

                // KARAN: need to ensure here that the accumulated the host is only issuing OUT packet
                // until the data_cnt is equal to the len field of status. The CSW IN packet should only
                // come after status->data_cnt == status->len similar to comparison performed in
                // REXMSA_Process_Response in MSA_STATE_OUT_DATA/RESP_ACK case.
                // What should be done if the host does not issues OUT data packet with length less than
                // what was stated in the CBW?

                // If we got a downstream In in this state then the Host as sent all of the
                // Outs and this In is for the CSW
                if ((packetStatus == PACKET_SENT) && (frmHdr.generic.action == ACTION_IN)) {
                    status->state = MSA_STATE_CSW;
                }
                sendOrAddPacketToQueue = FALSE;
            }
            break;

        case MSA_STATE_IN_DATA:
            iassert_REXSCH_COMPONENT_0(status->numPacketsInQueue == 0, MULTIPLE_IN_IN_PHASE);
            iassert_REXSCH_COMPONENT_1(frmHdr2.frameHeaderStruct.accel == FH_ACCEL_MSA_DATA,
                    MSA_DNS_IN_INCORRECT_ACCEL, frmHdr2.frameHeaderStruct.accel);
            iassert_REXSCH_COMPONENT_1(frmHdr.generic.action == ACTION_IN,
                    INVALID_DNS_IN_INPHASE, frmHdr.generic.action);

            // We should not get an IN until the CBW has been acked; send packet
            break;

        case MSA_STATE_STALL_SETUP_RESP:
        case MSA_STATE_STALL_IN_RESP:
            // Clear Feature Endpoint Halt States in MSA OUT transaction
            if (frmHdr.generic.action == ACTION_OUT) {
                // this is the case when OUT-Data gets stalled and we get the next OUT-Data
                // from the LEX before the LEX has received the STALL and LEX stops issuing OUT-Data
                if (frmHdr.generic.dataQid != 0) {
                    ilog_REXSCH_COMPONENT_2(ILOG_MINOR_ERROR,
                            MSA_IN_RECOVERY_FLUSHING_DATA_QID,
                            frmHdr.generic.addr, frmHdr.generic.dataQid);
                    XCSR_XICSQueueFlushAndDeallocate(frmHdr.generic.dataQid);
                }

                sendOrAddPacketToQueue = FALSE;
            } else {
                // this IN is for the subsequent CSW stage which will be scheduled after
                // endpoint halt has been cleared
            }
            break;

        case MSA_STATE_CSW:
            iassert_REXSCH_COMPONENT_0(frmHdr2.frameHeaderStruct.accel != FH_ACCEL_MSA_CBW,
                    MSA_DBG_CBW_IN_CSW);

            // We get IN downstream in two cases:
            // 1. CSW IN at the end of the MSA OUT transaction
            // 2. CSW IN after Clear Endpoint halt transaction
            // 3. CSW IN when there is no data stage

            // We can also get an OUT downstream in the following case:
            // - REX gets a 3K timeout, sends STALL upstream and changes state to CSW.
            // - Before LEX receives the STALL, it sends the next OUT data to the REX.

            // KARAN: IN MSA_STATE_OUT_DATA or MSA_STATE_PING, RESP_3K is the only case
            // in which the REX SW does not reset the state to MSA_STATE_IDLE, and the state
            // is reset to MSA_STATE_CSW.
            if (frmHdr.generic.action == ACTION_OUT) {
                if (frmHdr.generic.dataQid != 0) {
                    ilog_REXSCH_COMPONENT_2(ILOG_MINOR_ERROR,
                            MSA_IN_RECOVERY_FLUSHING_DATA_QID,
                            frmHdr.generic.addr, frmHdr.generic.dataQid);
                    XCSR_XICSQueueFlushAndDeallocate(frmHdr.generic.dataQid);
                }

                sendOrAddPacketToQueue = FALSE;
            }
            break;

        case MSA_STATE_START_CLEANUP:
            // Host has sent a setup-data and because REX was transferring a packet,
            // we have delayed sending the setup-data to the device.
            // LEX should not send the data phase packets until the REX sends the ACK
            // to the LEX.
            // REX should not get a packet from the LEX in this state.
            iassert_REXSCH_COMPONENT_0(FALSE, SETUP_TRANSACTION_PENDING); // TODO: could send a stall upstream
            break;

        case MSA_STATE_DATA:
        case MSA_STATE_IN_FLC_PAUSE: // TODO: what should we be doing in this state?
            // KARAN: There are two cases here:
            // 1. Going into flow control: If an IN packet is sent first followed by the flow control message and
            //    REX SW services the flow control ISR first, REX SW will change state to MSA_STATE_IN_FLC_PAUSE and
            //    REX SW will then service the MSA ISR which will assert with the MSA_DNS_BAD_STATE message. To make
            //    the code not rely on the order of interrupts serviced, REX SW needs to add code to save any packets
            //    that are received to status->msaQid using XRR_UpdateSchPacket(SCHTYPE_MSA, status->msaQid) function call.
            // 2. Coming out of flow control: Conversely, if the flow control message is sent first followed by the IN packet and
            //    REX SW services the data packets first, REX SW will assert with the MSA_DNS_BAD_STATE message. To make
            //    the code not rely on the order of interrupts serviced, REX SW needs to add code to save any packets
            //    that are received to status->msaQid using XRR_UpdateSchPacket(SCHTYPE_MSA, status->msaQid) function call.
            //    On second thought, there shouldn't be any packet sent by the LEX after the flow control message because
            //    we enter MSA_STATE_IN_FLC_PAUSE state after a data response from the device and MSA protocol allows only
            //    IN from the LEX to the REX for read transactions.
        default :
            iassert_REXSCH_COMPONENT_0(FALSE, MSA_DNS_BAD_STATE);
            break;
    }

    // KARAN: code will be easier to read if this function call is added to each case statement
    if (sendOrAddPacketToQueue) {
        const uint8 action = 0;
        _REXMSA_SendDownstream(status, SEND_TO_DEVICE_OR_ADD_TO_QUEUE, action);
    }
}

//-----------------------------------------------------------------
/**
* FUNCTION NAME: REXMSA_Process_Response
*
* @brief  - Process response from the device.
*
* @return -
*
* @note   -
*
*/
void REXMSA_Process_Response (uint64 frameHeader) {

    struct _REXSCH_MsaStatus* status;
//     enum _REXMSA_PacketStatus packetStatus;
    const union XRR_FrameHeader frmHdr = { .raw = frameHeader };
    const union XRR_InputFrameHeader2 frmHdr2 = { .frameHeader = XRR_GetInputFrameHeader2() };

    // get a refernce to the msa stat for this address
    status = _REXMSA_GetMsaStatusEntry(frmHdr.generic.addr);
    iassert_REXSCH_COMPONENT_1(
            status != NULL,
            UNEXPECTED_MSA_RESPONSE,
            frmHdr.generic.addr);

    // As soon as the msa int routine is called then it reads the packet from the cache
    status->numPacketsInQueue--;

    switch (status->state)
    {
        case MSA_STATE_IDLE:
            // should not get a response in this state
            // if we do get a response then send it upstream
            if (RESPONSE_IS_DATA(frmHdr.input.response)) {
                // RTL has already sent the data upstream.
            } else {
               // ?? should need this
                _REXMSA_SendUpstream(frmHdr.input.response);
            }

            // With the USB If MSC test it will send a bad cbw,
            // and we should send the devices response upstream
            ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_EVENT, MSA_RESP_IDLE);
            break;

        // Wait for CBW response
        case MSA_STATE_CBW:
            Process_CBW_Response(&frmHdr, &frmHdr2, status);
            break;

        case MSA_STATE_OUT_DATA:
        case MSA_STATE_PING:
            Process_Out_Ping_Response(&frmHdr, &frmHdr2, status);
            break;

        case MSA_STATE_IN_DATA:
            Process_In_Response(&frmHdr, &frmHdr2, status);
            break;

        // check the setup response
        case MSA_STATE_STALL_SETUP_RESP:

         // ?? what
//             iassert_REXSCH_COMPONENT_2((frmHdr.generic.endp == 0),
//                     RECEIVED_INVALID_CLR_HLTEP_RESPONSE, frmHdr.generic.addr, __LINE__);

            switch (frmHdr.input.response) {
                case RESP_ACK:
                    // Send the IN for the status phase with SCHTYPE_LOCAL scheduling type
                    // which will copy the response data to SW instead of the case of
                    // SCHTYPE_MSA which will send the response data to the LEX and only
                    // generate an interrupt for SW with the packet header
                    status->numPacketsInQueue++;
                    XRR_UpdateSchPacketNewAction(SCHTYPE_LOCAL, REX_SQ_SCH_ASYNC_OTB, ACTION_IN);
                    status->state = MSA_STATE_STALL_IN_RESP;
                    break;

                case RESP_TIMEOUT:
                case RESP_3K:
                    _REXMSA_SendUpstream(RESP_STALL);

                    _REXMSA_FlushQueue(status, FLUSH_ALL);

                    // end this MSA cycle
                    _REXMSA_ResetMsaStatusAndDeallocateQueue(status);
                    break;

                default :
                    // KARAN: if device sends an invalid response, send a STALL or the same response
                    // upstream and let the host handle it.
                    ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_EVENT, MSA_RESP_STALL_SETUP_INVALID);
                    break;
            }
            break;

        // check the Ctrl in response
        // SchLocal mode
        case MSA_STATE_STALL_IN_RESP:

            // why ??
//             iassert_REXSCH_COMPONENT_2((frmHdr.generic.endp == 0),
//                     RECEIVED_INVALID_CLR_HLTEP_RESPONSE, frmHdr.generic.addr, __LINE__);

            // why ??
//             iassert_REXSCH_COMPONENT_2((frmHdr.input.schType == SCHTYPE_LOCAL),
//                     RECEIVED_INVALID_CLR_HLTEP_SCHTYPE, frmHdr.input.schType, frmHdr.generic.addr);

            switch (frmHdr.input.response)
            {
                case RESP_NAK:
                    status->numPacketsInQueue++;
                    XRR_UpdateSchPacket(SCHTYPE_LOCAL, REX_SQ_SCH_ASYNC_OTB);
                    break;

                case RESP_DATA0:
                case RESP_DATA1:
                    {
                        struct XCSR_XICSQueueFrame rd_frame;
                        // This should be a 3 byte packet of { DATA1, CRC, CRC }
                        // Just flush & de-allocate it,
                        ilog_REXSCH_COMPONENT_2(ILOG_MINOR_ERROR,
                                MSA_IN_RECOVERY_FLUSHING_DATA_QID,
                                frmHdr.generic.addr, frmHdr.generic.dataQid);

                        // TODO: Are we really flushing the queue?
                        XCSR_XICSQueueReadFrame(REX_SQ_DEV_RESP_DATA, &rd_frame);

                        // Send the CSW IN
                        _REXMSA_SendDownstream(status, SEND_TO_DEVICE_FROM_QUEUE, 0);
                        status->state = MSA_STATE_CSW;

                        // KARAN: what if the ACK by the REX RTL is lost/corrupted?
                        break;
                    }
                case RESP_STALL:
                case RESP_TIMEOUT:
                case RESP_3K:
                    _REXMSA_SendUpstream(RESP_STALL);

                    _REXMSA_FlushQueue(status, FLUSH_ALL);
                    // end this MSA cycle
                    _REXMSA_ResetMsaStatusAndDeallocateQueue(status);
                    break;

                default :
                    // KARAN: if device sends an invalid response, send a STALL or the same response
                    // upstream and let the host handle it.
                    ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_EVENT, MSA_RESP_STALL_IN_INVALID);
                    break;
            }
            break;

        // check the response for the out csw
        case MSA_STATE_CSW:
            Process_In_CSW_Response(&frmHdr, status);
            break;

        case MSA_STATE_START_CLEANUP:
            _REXMSA_FlushQueue(status, FLUSH_ALL);
            _REXMSA_ResetMsaStatusAndDeallocateQueue(status);

            // send setup packet
            XRR_CopyFrameHeader(SCHTYPE_NONE, _REXSCH_SetupDataQid, REX_SQ_SCH_ASYNC_OTB);

            XCSR_XICSQueueFlushAndDeallocate(_REXSCH_SetupDataQid);
            break;

        case MSA_STATE_DATA: // KARAN: needs to be removed
        case MSA_STATE_IN_FLC_PAUSE: // TODO: what should we be doing in this state?
            // KARAN: The only way to get into MSA_STATE_IN_FLC_PAUSE state is after REX
            // receives a data response for an IN. We should not receive any response from
            // the device in this state and we can assert here.
        default :
            break;
    }
}

//-----------------------------------------------------------------
void Process_CBW_Response (
   const union XRR_FrameHeader *frmHdr,
   const union XRR_InputFrameHeader2 *frmHdr2,
   struct _REXSCH_MsaStatus *status) {

   switch (frmHdr->input.response) {
       // nyet means the out was accepted, but send a ping before the next packet
       // only send a ping if the MSA direction is out and the length > 0
       case RESP_ACK:
       case RESP_NYET:
           XRR_UpdateSchPacketNewResponse(
                   SCHTYPE_UPS_ASYNC,
                   REX_SQ_SCH_ASYNC_OTB,
                   RESP_ACK);

           // save the msa cbw info
           // this is the length of data that has been sent for an out, or the amount of data
           // rxed for an in
           status->data_cnt = 0;
           status->data_len = XRR_GetTransferLen();

           if (status->data_len == 0) {
               // there is no data for the Data phase; move directly to CSW
               status->state = MSA_STATE_CSW;
           } else if (frmHdr2->frameHeaderStruct.cbwDir == CBW_DIR_OUT) {
               if (frmHdr->input.response == RESP_NYET) {
                   // KARAN: the only reason why I am not using XRR_UpdateSchPacketNewAction API
                   // is because it locks up the REX.
                   // ?? need to confirm this
                   // send a ping
                   union XRR_FrameHeader pingFrmHdr = { .raw = frmHdr->raw };
                   // dont need this
                   pingFrmHdr.output.count = frmHdr2->frameHeaderStruct.count;
                   // dont need this
                   pingFrmHdr.output.response = frmHdr->input.response;
                   pingFrmHdr.generic.action = ACTION_PING;
                   // dont need this
                   pingFrmHdr.generic.dataQid = 0;
                   // ?? just use the replace action function
                   _REXMSA_SendToDevice(pingFrmHdr.raw, SCHTYPE_MSA, status);

                   status->state = MSA_STATE_PING;
               } else {
                   status->state = MSA_STATE_OUT_DATA;
               }
           } else {
               status->state = MSA_STATE_IN_DATA;
           }
           break;

       // ?? is this valid
       case RESP_NAK:
           _REXMSA_SendDownstream(status, RETRY_DOWNSTREAM, 0);
           break;

       case RESP_STALL:
       case RESP_3K:
           _REXMSA_SendUpstream(RESP_STALL);

           // a queue was allocated but no packet was added to it; no need to flush the queue

           // end this msa cycle
           _REXMSA_ResetMsaStatusAndDeallocateQueue(status);

           // Mass Storage Spec 6.6.1: Device will STALL if CBW is invalid;
           // the device shall maintain this state until a Reset Recovery.
           // Mass Storage Spec 3.1: This class-specific request shall ready
           // the device for the next CBW from the host.

           // After CBW-STALL, we are expecting another CBW; change state to IDLE

           break;

         case RESP_TIMEOUT:
               // TIMEOUT means the CRC in the OUT-data is incorrect; RTL sends a
               // response to the REXSCH to avoid retrying the same error packet
               // we sent a crc errored packet from the host so ignore it and send the next packet
               _REXMSA_SendDownstream(status, SEND_TO_DEVICE_FROM_QUEUE, 0);
            break;

       default :
           // KARAN: if device sends an invalid response, send a STALL or the same response
           // upstream and let the host handle it.
           ilog_REXSCH_COMPONENT_1(ILOG_MAJOR_EVENT, MSA_RESP_CBW_INVALID, frmHdr->input.response);
           break;
   }
}
//-----------------------------------------------------------------
void Process_Out_Ping_Response(
   const union XRR_FrameHeader *frmHdr,
   const union XRR_InputFrameHeader2 *frmHdr2,
   struct _REXSCH_MsaStatus *status) {

   enum _REXMSA_PacketStatus packetStatus;

   switch (frmHdr->input.response)
   {
       // KARAN: move this to each case for better readability
       uint32 pkt_len;

       case RESP_ACK:
           status->out_nak_cnt = 0;

           // check to see if there is another packet to send
           packetStatus = _REXMSA_SendDownstream(status, SEND_TO_DEVICE_FROM_QUEUE, 0);

           // KARAN: Could add an assert check here to ensure that if a packet
           // has been sent, it is an OUT packet. To do this, call XRR_ReadQueue(status->msaQid)
           // and populate a new frame header
           if (status->state == MSA_STATE_OUT_DATA) {
               // subtract the crc and pid from the data packet
               pkt_len = XRR_GetTransferLen() - 3;

               // KARAN: data_cnt is accumulated after the ACK for the OUT-data packet is received
               status->data_cnt += pkt_len;
               if (status->data_cnt > status->data_len) {
                   ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_ERROR, MSA_OUT_TOO_MUCH_DATA);
               }

               // if this is the last out data then go to the in csw
               if (status->data_cnt >= status->data_len) {
                   status->state = MSA_STATE_CSW;
               }
           } else { // state is MSA_STATE_PING
               // ?? I think this is bogus
               // KARAN: frmHdr.generic.action is for the response packet and
               // not for the packet that was sent to the device. If a packet was sent, we need
               // to get the action from XRR-SchFrmHdr0-Action register, and then perform the
               // comparison.
               if (    (packetStatus == PACKET_SENT)
                    && (frmHdr->generic.action == ACTION_IN))
               {
                   // Cannot get CSW in PING state
                   iassert_REXSCH_COMPONENT_3(FALSE, MSA_DBG_PING_ACK_CSW_IN,
                           frmHdr->generic.addr, status->data_cnt, status->data_len);
               }
               status->state = MSA_STATE_OUT_DATA;
           }

           break;

       case RESP_NAK:
           // Retry OUT packet, this will only be called for a ping nak
           status->out_nak_cnt++;

           if (status->out_nak_cnt >= MAX_NAK_CNT) {
               ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_EVENT, MSA_TOO_MANY_NAKS);
               _REXMSA_FlushQueue(status, FLUSH_ALL);
               // end this MSA cycle
               _REXMSA_ResetMsaStatusAndDeallocateQueue(status);
           } else {
              // resend this packet
              _REXMSA_SendDownstream(status, RETRY_DOWNSTREAM, 0);
            //   ilog_REXSCH_COMPONENT_2(ILOG_DEBUG, MSA_RESP_OUT_NAK_CNT,
            //         frmHdr->generic.addr, (status->out_nak_cnt << 16));
           }
           break;

       case RESP_STALL:
           status->out_nak_cnt = 0;
           _REXMSA_SendUpstream(RESP_STALL);
           status->lastOutToggle = 0x00;

           // remove all out packets from the msa queue
           _REXMSA_FlushQueue(status, FLUSH_OUT_ONLY);

           // we can get CSW IN which is stored in the msaQid;
           // thus, we are not deallocating the msaQid

           msaStatCounter.out_stall_cnt++;

           _REXMSA_SendClearEndpointHaltSetup(frmHdr->raw, status);
           status->state = MSA_STATE_STALL_SETUP_RESP;
           break;

       // If we get a 3 strikes timeout then it is most likely that the device has been
       // disconnected, so flush all of the out packets from the cache, and if there is
       // an In in the cache then send it to the device. We should get a 3 strikes timeout
       // for the In also and when we do we will send an upstream Stall to Lex which will
       // end its MSA cycle.
       case RESP_3K:
           status->out_nak_cnt = 0;
           _REXMSA_SendUpstream(RESP_STALL);
           status->lastOutToggle = 0x00;

           ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_EVENT, MSA_DBG_OUT_TIMEOUT);
           // end this msa cycle
           _REXMSA_FlushQueue(status, FLUSH_OUT_ONLY);

           // Host is under the impression that the endpoint is halted.
           // If the device is disconnected, host will not send us any
           // packets. If the endpoint is dead, host will send us
           // Clear_Feature(Endpoint_Halt) and then send us CSW.
            _REXMSA_SendDownstream(status, SEND_TO_DEVICE_FROM_QUEUE, 0);

           status->state = MSA_STATE_CSW;

           // If the device is disconnected, state is still set to CSW
           // but the state will be reset on the subsequent Set_Address
           break;

       case RESP_NYET:
           if (status->state == MSA_STATE_OUT_DATA)
           {
               status->out_nak_cnt = 0;

               // subtract the crc and pid from the data packet
               pkt_len = XRR_GetTransferLen() - 3;

               if((pkt_len > 512) || (pkt_len < 0)) {
                   iassert_REXSCH_COMPONENT_2(FALSE, MSA_DBG_BAD_NYET_PKT_LEN, frmHdr->generic.addr, pkt_len);
               }

               status->data_cnt += pkt_len;

               // if this is the last out data then go to the in csw
               if (status->data_cnt >= status->data_len) {
                   // check to see if there is another packet to send
                   packetStatus = _REXMSA_SendDownstream(status, SEND_TO_DEVICE_FROM_QUEUE, 0);

                   status->state = MSA_STATE_CSW;
               } else {
                   // send a ping
                   union XRR_FrameHeader pingFrmHdr = { .raw = frmHdr->raw };
                   // dont need to do this
                   pingFrmHdr.output.count = frmHdr2->frameHeaderStruct.count;
                   // dont need to do this
                   pingFrmHdr.output.response = frmHdr->input.response;
                   pingFrmHdr.generic.action = ACTION_PING;
                   // dont need to do this
                   pingFrmHdr.generic.dataQid = 0;
                   // ? why not just call the function to replace the action, thats what it was
                   // designed for
                   _REXMSA_SendToDevice(pingFrmHdr.raw, SCHTYPE_MSA, status);
                   status->state = MSA_STATE_PING;
               }
           } else {
               iassert_REXSCH_COMPONENT_1(FALSE, REXSCH_BAD_PING_RESPONSE, frmHdr->input.response);
           }
           break;

      case RESP_TIMEOUT:
         // cant get a timeout for a ping would only get 3k
         if (status->state == MSA_STATE_OUT_DATA) {
            // TIMEOUT means the CRC in the OUT-data is incorrect; RTL sends a
            // response to the REXSCH to avoid retrying the same error packet
            // we sent a crc errored packet from the host so ignore it and send the next packet
            _REXMSA_SendDownstream(status, SEND_TO_DEVICE_FROM_QUEUE, 0);
         }
         break;

        default :
           // KARAN: if device sends an invalid response, send a STALL or the same response
           // upstream and let the host handle it.
           iassert_REXSCH_COMPONENT_2(FALSE, MSA_RESP_OUT_DATA_INVALID,
                   frmHdr->generic.addr, frmHdr->input.response);
           break;
    }
}
//-----------------------------------------------------------------
// ?? it would be nice to merge frmHdr and frmHdr2
void Process_In_Response (
   const union XRR_FrameHeader         *frmHdr,
   const union XRR_InputFrameHeader2   *frmHdr2,
   struct _REXSCH_MsaStatus            *status) {

   // In Data
   // In CSW
   // Stall
   // check the error first
   // If there is no error then that means the pid and crc are valid
   if (frmHdr->input.error == 0) {
      switch (frmHdr->input.response) {
         case RESP_DATA0:
         case RESP_DATA1:
            // If we sent an In and we got a CSW response then this is the end of this
            // MSA transfer
            if (frmHdr2->frameHeaderStruct.accel == FH_ACCEL_MSA_CSW) {
               msaStatCounter.csw_cnt++;  // just for debug
               _REXMSA_FlushQueue(status, FLUSH_ALL);
               _REXMSA_ResetMsaStatusAndDeallocateQueue(status); // ?? this changes the state to idle
            } else  {
               // ?? dump this toggle stuff

//                // Duplicated PID means the ACK from the Rex to the device was lost & the device resent the last packet
//                const boolT duplicatedPid =
//                  (    ((frmHdr->input.response == RESP_DATA0) && (status->lastInToggle == DATA0_PID))
//                       ||  ((frmHdr->input.response == RESP_DATA1) && (status->lastInToggle == DATA1_PID)));
//
//                // Set lastInToggle for the analysis of the next data packet
//                status->lastInToggle = (frmHdr->input.response == RESP_DATA0) ? DATA0_PID : DATA1_PID;
//
//                if (duplicatedPid) {
//                   ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_ERROR, DUPLICATED_IN_TOGGLE);
//                } else {

                  // subtract the crc and pid from the data packet
                  const uint32 pkt_len = XRR_GetTransferLen() - 3;
                  status->data_cnt += pkt_len;

                  // Data returned by device should not exceed length in CBW
                  if (status->data_cnt > status->data_len) {
                     ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_ERROR, MSA_IN_TOO_MUCH_DATA);
                  }
//                }

               if (inDirectionFlowControlEnabled) {
                  // if flow control is set then wait

                  // save the endp number of the in
                  status->endp = frmHdr->generic.endp;
                  status->state = MSA_STATE_IN_FLC_PAUSE;
                  // inc the flow control in status count
                  msaStatCounter.flc_in_cnt++;
               } else {
                  enum _REXMSA_SendDownstreamMode mode;

                  if (status->data_cnt >= status->data_len) {
                     // Received the last Data packet
                     status->state = MSA_STATE_CSW;

                     // ?? I really question if this is valid, it definitely slows down msa
                     // Schedule CSW in the next uFrame to work around buggy devices
                     // buggy device or buggy software programmer ?
                     mode = SEND_TO_DEVICE_NEXT_UFRAME;
                  } else {
                      // send another IN
                      mode = SEND_DOWNSTREAM;
                  }

                  _REXMSA_SendDownstream(status, mode, 0);
               }
            }
            status->in_nak_cnt = 0;
            break;

         case RESP_NAK:
            status->in_nak_cnt++;

            if (status->in_nak_cnt >= MAX_NAK_CNT) {
               ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_EVENT, MSA_TOO_MANY_NAKS);
               // end this MSA cycle
               _REXMSA_ResetMsaStatusAndDeallocateQueue(status);
            } else {
            //   _REXMSA_SendDownstream(status, RETRY_DOWNSTREAM, 0);
            //    ilog_REXSCH_COMPONENT_2(ILOG_MAJOR_EVENT, MSA_RESP_IN_NAK_CNT, frmHdr->generic.addr, (status->in_nak_cnt << 16));
            }
            break;

         case RESP_TIMEOUT:
         case RESP_3K:
            _REXMSA_FlushQueue(status, FLUSH_ALL);
            // fall through
         case RESP_STALL:
            _REXMSA_SendUpstream(RESP_STALL);
            status->lastInToggle = 0x00;

            // we can get CSW IN which is stored in the msaQid;
            // thus, we are not deallocating and flushing the queue
            if (frmHdr->input.response == RESP_STALL) {
               msaStatCounter.in_stall_cnt++;
            }

            status->in_nak_cnt = 0;
            status->state = MSA_STATE_CSW;
            break;

         default :
            // KARAN: if device sends an invalid response, send a STALL or the same response
            // upstream and let the host handle it.
            iassert_REXSCH_COMPONENT_1(FALSE, MSA_RESP_IN_DATA_INVALID, frmHdr->input.response);
            break;
      }
   // error
   } else {
      // error response
      // The hardware will retry if we get a crc errored packet so just ignore
      // the response
      // SW broke this. If we get a crc error then do not dec the number of packets in the q
      // since HW will retry the packet and will not move to the next packet
      ilog_REXSCH_COMPONENT_1(ILOG_MAJOR_EVENT, MSA_RESP_IN_ERROR, frmHdr->input.response);
      status->numPacketsInQueue++;
   }
}
//-----------------------------------------------------------------
void Process_In_CSW_Response (
   const union XRR_FrameHeader *frmHdr,
   struct _REXSCH_MsaStatus *status) {

   if (frmHdr->input.error == 0) {
       // is the packet error free
       switch (frmHdr->input.response)
       {
           case RESP_DATA0:
           case RESP_DATA1:
               // check for the csw ??
               msaStatCounter.csw_cnt++;
               // end this MSA cycle
               _REXMSA_ResetMsaStatusAndDeallocateQueue(status);

               // KARAN: What if the ACK is lost? Need to save the data toggle
               // and check if the same toggle is sent by the device in the next
               // MSA transaction. _REXMSA_ResetMsaStatusAndDeallocateQueue should
               // not clear the data toggle field.
               // Wrong assumption, Host will move on to the next Out CBW

               break;

           case RESP_NAK:
               status->in_nak_cnt++;

               if (status->in_nak_cnt >= MAX_NAK_CNT) {
                  ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_EVENT, MSA_TOO_MANY_NAKS);
                  // end this MSA cycle
                  _REXMSA_ResetMsaStatusAndDeallocateQueue(status);
               } else {
                 _REXMSA_SendDownstream(status, RETRY_DOWNSTREAM, 0);
                //   ilog_REXSCH_COMPONENT_2(ILOG_DEBUG, MSA_CSW_RESP_IN_NAK_CNT,
                //            frmHdr->generic.addr, (status->in_nak_cnt << 16));
               }
               break;

           case RESP_STALL:
           case RESP_3K:
               _REXMSA_SendUpstream(RESP_STALL);
               _REXMSA_FlushQueue(status, FLUSH_ALL);
               // end this MSA cycle
               _REXMSA_ResetMsaStatusAndDeallocateQueue(status);
               break;

           default :
               ilog_REXSCH_COMPONENT_2(ILOG_MAJOR_ERROR, MSA_RESP_CSW_INVALID,
                       frmHdr->generic.addr, frmHdr->input.response);
               // Send a timeout to the LEX in the hope that the host will re-try,
               // or after 3 timeouts in a row the LEX will STALL
               _REXMSA_SendUpstream(RESP_TIMEOUT);
               break;
       }
   } else  {
       ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_EVENT, MSA_RESP_CSW_ERROR);
       // KARAN: similar to the CRC error case in MSA_STATE_IN_DATA,
       // The hardware will retry if we get a crc errored packet so just ignore
       // the response

       // KARAN: Need to increment numPacketsInQueue
   }
}
//-----------------------------------------------------------------
/**
* FUNCTION NAME: REXMSA_ResetDevice()
*
* @brief  - Start resetting the MSA status in SW and RTL
*
* @return - TRUE: If MSA status has been reset and queue has been
*                 flushed and deallocated.
*           FALSE: otherwise
*/
boolT REXMSA_ResetDevice
(
    uint8 usbAddr
)
{
    boolT resetComplete = TRUE;

    struct _REXSCH_MsaStatus* msaStat = _REXMSA_GetMsaStatusEntry(usbAddr);

    if ((msaStat == NULL) || (usbAddr == 0))
    {
        return resetComplete;
    }

    ilog_REXSCH_COMPONENT_1(ILOG_WARNING, REXMSA_RESET_DEVICE, usbAddr);

    switch(msaStat->state)
    {
        case MSA_STATE_IN_DATA:
        case MSA_STATE_OUT_DATA:
        case MSA_STATE_PING:
        case MSA_STATE_CSW:
        case MSA_STATE_STALL_SETUP_RESP:
        case MSA_STATE_STALL_IN_RESP:
        case MSA_STATE_CBW:
            if(msaStat->numPacketsInQueue > 0)
            {
                resetComplete = FALSE;
                msaStat->state = MSA_STATE_START_CLEANUP;
            }
            else
            {
                // KARAN: why are we not flushing the IN packets?
//                 _REXMSA_FlushQueue(msaStat, FLUSH_OUT_ONLY);
               // We really want to flush everything
                _REXMSA_FlushQueue(msaStat, FLUSH_ALL);
                _REXMSA_ResetMsaStatusAndDeallocateQueue(msaStat);
                resetComplete = TRUE;
            }
            break;

        case MSA_STATE_IN_FLC_PAUSE:
            msaStatCounter.flc_in_cnt--;
//             _REXMSA_FlushQueue(msaStat, FLUSH_OUT_ONLY);
            _REXMSA_FlushQueue(msaStat, FLUSH_ALL);
            _REXMSA_ResetMsaStatusAndDeallocateQueue(msaStat);
            // fall thru
        case MSA_STATE_IDLE:
            resetComplete = TRUE;
            break;

        case MSA_STATE_START_CLEANUP:
            // Another setup is already queued
            resetComplete = FALSE;
            break;

        case MSA_STATE_DATA:
        default:
            // KARAN: add assert here
            // iassert
            break;
    }

    return resetComplete;
}

/**
* FUNCTION NAME: REXMSA_CheckFlowControl
*
* @brief  - Check the flow control status and if there is no flow control, start normal operation.
*
* @return - void
*/
void REXMSA_CheckFlowControl(void)
{
    struct XCSR_FlowControlStatus flowControlStatus;

    sof_cnt++;
    XCSR_XFLC_Get_Status(&flowControlStatus);

    const boolT enableFlowControl = (flowControlStatus.BlkAccIn != 0);
    if (!enableFlowControl && inDirectionFlowControlEnabled)
    {
        _REXMSA_RestartPausedEndpoints();
    }
    inDirectionFlowControlEnabled = enableFlowControl;
}

/**
* FUNCTION NAME: _REXMSA_GetMsaStatusEntry
*
* @brief  - Find the MSA status entry for the active endpoint on the given USB address.
*
* @return - The MSA status entry for the given address or NULL if one does not exist.
*/
struct _REXSCH_MsaStatus* _REXMSA_GetMsaStatusEntry (uint8 usbAddr) {
    uint8 entryIdx;

    for (entryIdx = 0; entryIdx < MSA_ENDP_COUNT; entryIdx++) {
        struct _REXSCH_MsaStatus* current = &msaStatus[entryIdx];
        if (current->addr == usbAddr) {
            return current;
        }
    }

    // return error if not found
    return NULL;
}

/**
* FUNCTION NAME: REXMSA_Disp_Stat
*
* @brief  - Display the status of the active msa addresses
*
* @return - void
*/
void REXMSA_Disp_Stat(void)
{
    int endp;
    struct _REXSCH_MsaStatus* status;

    ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_EVENT, MSA_DISP_STAT_HDR);

    _REXMSA_DisplayMsaStatCounters();

    for (endp = 0; endp < MSA_ENDP_COUNT; endp++) {
        status = &msaStatus[endp];

        ilog_REXSCH_COMPONENT_2(ILOG_MAJOR_EVENT, MSA_DISP_STAT_ADDR, status->addr, status->endp);
        ilog_REXSCH_COMPONENT_1(ILOG_MAJOR_EVENT, MSA_DISP_STAT_NAK_CNT, status->in_nak_cnt);
        ilog_REXSCH_COMPONENT_1(ILOG_MAJOR_EVENT, MSA_DISP_STAT_NAK_CNT, status->out_nak_cnt);
        ilog_REXSCH_COMPONENT_3(
            ILOG_MAJOR_EVENT,
            MSA_DISP_STAT,
            status->state,
            status->msaQid,
            status->numPacketsInQueue);
        ilog_REXSCH_COMPONENT_2(
            ILOG_MAJOR_EVENT, MSA_DISP_STAT_DATA, status->data_len, status->data_cnt);
    }

    ilog_REXSCH_COMPONENT_1(ILOG_MAJOR_EVENT, MSA_SOF_FLC, sof_cnt);
}

/***************************** Local Functions *******************************/

/**
* FUNCTION NAME: _REXMSA_SendClearEndpointHaltSetup
*
* @brief  - Send the Setup packet for Clear Feature Halt to the endpoint which stalled
*
* @return - void
*/
void _REXMSA_SendClearEndpointHaltSetup (
   uint64                     haltedEpFrmHdr,
   struct _REXSCH_MsaStatus   *status
) {
    union XRR_FrameHeader ctrlEpFrmHdr;
    const uint16 setupPacketSize = 9;
    union
    {
        uint32 data[3];
        struct
        {
            uint8 pid;
            uint8 bmRequestType;
            uint8 bRequest;
            uint8 wValueLSB;
            uint8 wValueMSB;
            uint8 wIndexLSB;
            uint8 wIndexMSB;
            uint8 wLengthLSB;
            uint8 wLengthMSB;
            uint8 reserved[3];
        } dataStruct;
    } setupPacket = {
        .dataStruct = {
            .pid = DATA0_PID,
            .bmRequestType = CREATE_REQUEST_TYPE(
                REQUESTTYPE_REC_ENDP, REQUESTTYPE_TYPE_STANDARD, REQUESTTYPE_DIR_H2D),
            .bRequest = STD_REQ_CLEAR_FEATURE,
            .wValueLSB = ENDPOINT_HALT,
            .wIndexLSB = 0
        }
    };

    // Setup the frame header for the control endpoint
    ctrlEpFrmHdr.raw = haltedEpFrmHdr;
    // ctrlEpFrmHdr.generic.endp will contain the endpoint that is halted
    setupPacket.dataStruct.wIndexLSB = ctrlEpFrmHdr.generic.endp;

    // Set the control endpoint parameters
    ctrlEpFrmHdr.generic.endp = 0;
    ctrlEpFrmHdr.generic.endpType = ENDP_CTRL;
    ctrlEpFrmHdr.generic.action = ACTION_SETUP;
    ctrlEpFrmHdr.generic.dataQid = XCSR_XICSQQueueAllocate(QT_DNS_ASYNC);

    XCSR_QueueWriteRawData(
        setupPacket.data,
        setupPacketSize,
        ctrlEpFrmHdr.generic.dataQid,
        (XCSR_WFLAGS_SOF | XCSR_WFLAGS_SOP | XCSR_WFLAGS_EOP | XCSR_WFLAGS_EOF));

//     schedule 1 frame ahead
//     const uint8 schFrame = (XRR_GetLsbFrameAndUFrame() ^ 0x8) + REX_SQ_SCH_UFRM0;
//     XRR_WriteSchPacket(ctrlEpFrmHdr.raw, SCHTYPE_LOCAL, schFrame);
//     status->numPacketsInQueue++;

    _REXMSA_SendToDevice(ctrlEpFrmHdr.raw, SCHTYPE_LOCAL, status);

    msaStatCounter.send_clr_halt_cnt++;
}

/**
* FUNCTION NAME: _REXMSA_RestartPausedEndpoints
*
* @brief  - Start scheduling packets for MSA IN endpoints that were previously paused.
*
* @return - void
*/
void _REXMSA_RestartPausedEndpoints (void)
{
    uint8 endp;

    for (endp = 0; endp < MSA_ENDP_COUNT; endp++) {
        if (msaStatus[endp].state == MSA_STATE_IN_FLC_PAUSE) {
            const union XRR_FrameHeader frmHdr = {
                .generic = {
                    .addr = msaStatus[endp].addr,
                    .endp = msaStatus[endp].endp,
                    .action = ACTION_IN,
                    .endpType = ENDP_BULK,
                    .modifier = MOD_NORMAL
                }
            };

            msaStatus[endp].state = MSA_STATE_IN_DATA;

            _REXMSA_SendToDevice(frmHdr.raw, SCHTYPE_MSA, &msaStatus[endp]);
        }
    }
}

/**
* FUNCTION NAME: _REXMSA_SendDownstream
*
* @brief  - Send the downstream packet to the device or save to the out save queue
*
* @return - PACKET_SENT: if packet was transmitted
*           PACKET_ADDED_TO_QUEUE: packet was added to queue
*           PACKET_NOT_SENT: packet was not sent
*
* @note   - If we are currently sending a packet to the device then save this packet to the out
*           save queue.
*/
enum _REXMSA_PacketStatus _REXMSA_SendDownstream
(
    struct _REXSCH_MsaStatus* status,
    enum _REXMSA_SendDownstreamMode mode,
    uint8 action
) {
    enum _REXMSA_PacketStatus packetStatus = PACKET_SENT;
    // Set destination queue to the Outbound queue for sending packets to the device
    uint8 destQueue = REX_SQ_SCH_ASYNC_OTB;

    // KARAN: inline the respective mode to the caller for better readability
    switch (mode)
    {
        // send a packet from the queue
        case SEND_TO_DEVICE_FROM_QUEUE:
            if (status->numPacketsInQueue != 0) {
                // Queue has packets
                if (XRR_Get_Q_Stat(status->msaQid) == 0) {
                    ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_EVENT, MSA_OUT_SAVE_QUEUE_EMPTY);
                }

                XRR_CopyFrameHeader(SCHTYPE_MSA, status->msaQid, destQueue);
            } else {
                // Queue is empty
                packetStatus = PACKET_NOT_SENT;
            }
            break;

        // replace the action with the new one, good for converting an out to a ping
        case SEND_TO_DEVICE_WITH_NEW_PID:
            status->numPacketsInQueue++;
            XRR_UpdateSchPacketNewAction(SCHTYPE_MSA, destQueue, action);
            break;

        // ?? why are these 2 merged
        case SEND_TO_DEVICE_NEXT_UFRAME:
        case SEND_TO_DEVICE_OR_ADD_TO_QUEUE:
            if (mode == SEND_TO_DEVICE_NEXT_UFRAME) {
                destQueue = ((XRR_GetLsbFrameAndUFrame() + 1) & 0xF) + REX_SQ_SCH_UFRM0;
            } else if (status->numPacketsInQueue == 0) {
                // since rex is not currently processing packet then send to device
            } else {
                // since we are currently sending a packet then save to the out save queue
                destQueue = status->msaQid;
                packetStatus = PACKET_ADDED_TO_QUEUE;
            }
        // fall through
        case RETRY_DOWNSTREAM:
        case SEND_DOWNSTREAM:
            status->numPacketsInQueue++;
            XRR_UpdateSchPacket(SCHTYPE_MSA, destQueue);
            break;

        default:
            iassert_REXSCH_COMPONENT_0(FALSE, INVALID_SEND_DOWNSTREAM_MODE);
    }

    return packetStatus;
}

/**
* FUNCTION NAME: _REXMSA_InitMsaStatusAndAllocateQueue
*
* @brief  - Return a new MSA Stat buffer if available
*
* @return - Pointer to MSA stat buffer or NULL if one is not available.
*/
struct _REXSCH_MsaStatus* _REXMSA_InitMsaStatusAndAllocateQueue
(
    uint8 addr,
    uint8 endp
)
{
    int index;

    for (index = 0; index < MSA_ENDP_COUNT; index++) {
        if (msaStatus[index].addr == 0) {
            msaStatus[index].addr = addr;
            msaStatus[index].endp = endp;
            // set the state to the idle state
            msaStatus[index].state = MSA_STATE_IDLE;
            msaStatus[index].numPacketsInQueue = 0;

            // Get a new queue to save the msa out packets to
            msaStatus[index].msaQid = XCSR_XICSQQueueAllocate(QT_DNS_ACCBULK);

            return &msaStatus[index];
        }
    }

    ilog_REXSCH_COMPONENT_0(ILOG_MAJOR_ERROR, MSA_ALLOCATE_NO_FREE_REF);
    return NULL;
}

/**
* FUNCTION NAME: _REXMSA_ResetMsaStatusAndDeallocateQueue
*
* @brief  - Clear the MSA stat and deallocate the out save queue
*
* @return - void
*
* @note   - Call this when the MSA transfer is complete
*/
void _REXMSA_ResetMsaStatusAndDeallocateQueue
(
    struct _REXSCH_MsaStatus* status
)
{
    XCSR_XICSQQueueDeallocate(status->msaQid);
    memset(status, 0, sizeof(struct _REXSCH_MsaStatus));
    status->state = MSA_STATE_IDLE;
}

/**
* FUNCTION NAME: _REXMSA_FlushQueue
*
* @brief  - flush all of the packets from the out save queue
*
* @return - void
*/
void _REXMSA_FlushQueue (
    struct _REXSCH_MsaStatus* status,
    enum _REXMSA_FlushType flushType) {
    // read all of the frame headers in the out save queue
ilog_REXSCH_COMPONENT_0(ILOG_USER_LOG, REXSCH_DEBUG_MSA_FLUSH_Q);

    while (XRR_Get_Q_Stat(status->msaQid) != 0) {
        union XRR_FrameHeader frmHdr;

        // Update the rdfrmHdr.generic registers
        frmHdr.raw = XRR_ReadQueue(status->msaQid);

        // if there is an In sitting in the queue then it is for the csw so copy it back
        if (frmHdr.generic.action == ACTION_IN) {
            if (flushType == FLUSH_OUT_ONLY) {
                XRR_UpdateSchPacket(SCHTYPE_MSA, status->msaQid);
                return;
            }
        } else {
            // packet is an out
            // read the data qid from the frame header and flush it
            const uint8 dataQid = frmHdr.generic.dataQid;
            if (dataQid != 0)
            {
                if (XRR_Get_Q_Stat(dataQid) == 0) {
                    ilog_REXSCH_COMPONENT_1(ILOG_MAJOR_EVENT, MSA_FLUSH_QUEUE_ERROR, 0);
                }

                ilog_REXSCH_COMPONENT_2(ILOG_MINOR_ERROR, MSA_IN_RECOVERY_FLUSHING_DATA_QID,
                    frmHdr.generic.addr, frmHdr.generic.dataQid);
                XCSR_XICSQueueFlushAndDeallocate(dataQid);
            }
        }
        status->numPacketsInQueue--;
    }

    // ?? dont assert that is dumb, just print a warning
    if (status->numPacketsInQueue != 0)
      ilog_REXSCH_COMPONENT_1(ILOG_MINOR_ERROR, NUM_PACKETS_IN_QUEUE_NOT_ZERO,
                    status->numPacketsInQueue);
//     iassert_REXSCH_COMPONENT_1(status->numPacketsInQueue == 0, NUM_PACKETS_IN_QUEUE_NOT_ZERO, status->numPacketsInQueue);
}

//------------------------------------------------------
/**
* FUNCTION NAME: _REXMSA_SendUpstream
*
* @brief  - Send a response upstream
*
* @return - void
*/
void _REXMSA_SendUpstream
(
    enum REXSCH_Response response
)
{
    const uint8 newResponse = CAST(response, enum REXSCH_Response, uint8);
    XRR_UpdateSchPacketNewResponse(SCHTYPE_UPS_ACC, REX_SQ_SCH_ASYNC_OTB, newResponse);
}

//------------------------------------------------------
void _REXMSA_SendToDevice (
   XRR_FrameHeaderRaw         raw,
   enum XRR_SchType           sch_type,
   struct _REXSCH_MsaStatus   *status) {

   XRR_WriteSchPacket(raw, sch_type, REX_SQ_SCH_ASYNC_OTB);
   status->numPacketsInQueue++;
}

/**
* FUNCTION NAME: _REXMSA_DisplayMsaStatCounters
*
* @brief  - Display the status of the mass storage acceleration.
*
* @return - void
*/
static void _REXMSA_DisplayMsaStatCounters(void)
{
    ilog_REXSCH_COMPONENT_3(
        ILOG_MAJOR_EVENT,
        MSA_DISP_STAT_CNT,
        msaStatCounter.send_clr_halt_cnt,
        msaStatCounter.cbw_cnt,
        msaStatCounter.csw_cnt);

    ilog_REXSCH_COMPONENT_3(
        ILOG_MAJOR_EVENT,
        MSA_DISP_STAT_CNT2,
        msaStatCounter.in_stall_cnt,
        msaStatCounter.out_stall_cnt,
        msaStatCounter.flc_in_cnt);
}

