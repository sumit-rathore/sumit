#include <leon_traps.h>
#include <xcsr_xsst.h>
#include <xcsr_xicsq.h>

#include "test_log.h"
#include "xics_ge.h"
#include "enum.h"

// move this ??
#include "rexsch.h"

#define GET_STATUS               0
#define CLEAR_FEATURE            1
#define SET_FEATURE              3
#define SET_ADDRESS              5
#define GET_DESCRIPTOR           6
#define SET_DESCRIPTOR           7
#define GET_CONFIGURATION        8
#define SET_CONFIGURATION        9
#define GET_INTERFACE           10
#define SET_INTERFACE           11
#define SYNCH_FRAME             12

typedef struct {
   uint8    bmreqtype;
   uint8    bmreqtype_dir;
   uint8    bmreqtype_type;
   uint8    bmreqtype_recipient;
   uint8    brequest;
   uint32   wvalue;
   uint32   windex;
   uint32   wlength;
} t_des;

static void ENUM_Process_SETUP_Packet(struct XCSR_XICSQueueFrame*);
static void ENUM_Parse_Setup_Payload(uint8 *,
                              t_des      *des_ptr);
static void ENUM_Clear_BCI(uint8 addr, uint8 endp);
static void ENUM_Clear_BCO(uint8 addr, uint8 endp);

//-----------------------------------------------------------
void ENUM_Process_CPU_Packet(void) {

   uint32      cpu_ctrl_qid_errors = 0;
   eXCSR_QueueReadStatusT status;
   struct XCSR_XICSQueueFrame* frameData;


   //
   // This is either a IN OUT/PING/DATA.  Since the curr_des_req describes state of this endpoints
   // control sequence as the CPU see's it it will determine how to route the information
   // in this packet to the appropriate handler.
   //

//    ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, PCP);
   //
   // Filter on TAG's First
   //

   XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(16, frameData);
   status = XCSR_XICSQueueReadFrame(LEX_SQ_CPU_USB_CTRL, frameData);

//    ilog_TEST_HARNESS_COMPONENT_2(ILOG_DEBUG, TAG, frameData->header.one.generic.tagType, frameData->header.one.downstream.action);

   switch (frameData->header.one.generic.tagType) {
      case DOWNSTREAM_XUSB_ASYNC:
      case DOWNSTREAM_XUSB_SPLIT_AND_PERIODIC:
         switch (frameData->header.one.downstream.action) {
            case ACTION_SETUP :
               ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, DNS_SETUP);
               ENUM_Process_SETUP_Packet(frameData);
               break;

            case ACTION_OUT   :
                  ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, DNS_OUT);
//                Process_OUT_Packet(rx_pkt);
               break;

            case ACTION_PING  :
               ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, DNS_PING);
//                Process_PING_Packet(rx_pkt);
               break;

            default      :
               //             $display("TBERROR: %m() -> ACTION = %s found in the  CPU_CTRL_QID", class_common::Get_Action_Name(frameData->header.one.downstream.action));
               cpu_ctrl_qid_errors++;
               break;
         }
         break;

      case UPSTREAM_XUSB:
         switch (frameData->header.one.upstream.response) {
            case RESP_ACK :
               ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, UPS_ACK);
               ENUM_Clear_BCI(frameData->header.one.upstream.address, frameData->header.one.upstream.endpoint);
               break;

//          case RESP_TIMEOUT :
// //                three_strikes_count++;
//                break;

//          case RESP_STALL :
//                //the hardware will keep issueing stalls until a new setup...nothing else should be in the cache
// //                three_strikes_count++;
//                break;

            case RESP_DATA0 :
            case RESP_DATA1 :
               ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, UPS_DATA);
               ENUM_Clear_BCI(frameData->header.one.upstream.address, frameData->header.one.upstream.endpoint);
// //                if (~rx_pkt.data_crc_val) {
// //                   $display("TBNOTE: %m() %s ->  CPU read data from the cache with CRC errors!", c_setup_req.curr_req.str_brequest);
// //                   three_strikes_count++;
// //                } else {
// //                   Process_DATA_Packet(rx_pkt);
// //                }
               break;

            case RESP_NAK :
            case RESP_NYET :
//                $display("TBERROR: %m() %s -> %s not allowed in the CPU QID for non-virtual function address/endpoints", c_setup_req.curr_req.str_brequest, class_common::Get_RespId_Name(rx_pkt.frm_hdr.resp));
               cpu_ctrl_qid_errors++;
               break;

            default :
//                $display("TBERROR: %m() %s -> Unhandled response in the CPU_CTRL_QID : %s", c_setup_req.curr_req.str_brequest, class_common::Get_RespId_Name(rx_pkt.frm_hdr.resp) );
               cpu_ctrl_qid_errors++;
               break;
         }
         break;

      default :
//          $display("TBERROR: %m() %s -> Invalid FrameHeader TagType in the CPU_CTRL_QID", c_setup_req.curr_req.str_brequest);
         cpu_ctrl_qid_errors++;
         break;
   }
}
//-----------------------------------------------------------
static void ENUM_Process_SETUP_Packet(struct XCSR_XICSQueueFrame * frameData) {

   t_des    setup_des;

   // Based on the Setup Request Other Actions may need to be implemented
   // This is where this would be done...
   ENUM_Parse_Setup_Payload(&frameData->data, &setup_des);

//    ilog_TEST_HARNESS_COMPONENT_1(ILOG_DEBUG, BREQ, setup_des.brequest);

   if ((setup_des.brequest == SET_ADDRESS) |
       (setup_des.brequest == SET_INTERFACE) |
       (setup_des.brequest == SET_CONFIGURATION ))
      // For now just clear BCI
      ENUM_Clear_BCI(frameData->header.downstream.address, frameData->header.downstream.endpoint);
      ENUM_Clear_BCO(frameData->header.downstream.address, frameData->header.downstream.endpoint);
//    } else {
//       //
//       // This is not a valid request...Now determine why it is not valid.
//       //
//       if (rx_pkt.data_crc_val) begin
//       //if (XSST.ReflectAllSetupRequests == 1'b1) begin
//       //  Then this is still valid and only a note saying that this request was
//       //  sent in the CPU_CTRL_QID
//       //end else begin
//       //  Then this is an error and must be shown
//       //end
//       $display("TBERROR: %m() -> LEX CPU CTRL QID contains and invalid request", c_setup_req.curr_req.str_brequest);
// //       cpu_ctrl_qid_errors++;
//
//       } else {
//       //
//       // This is invalid because the CRC16 failed
//       //
//          $display("TBNOTE: %m() -> SETUP Packet was received with CRC16 error!");
//       }
//    }
}
//-----------------------------------------------------------
static void ENUM_Parse_Setup_Payload(uint8 * data,
                              t_des      *des_ptr) {

   des_ptr->bmreqtype            = data[1];
   des_ptr->bmreqtype_dir        = ((data[1] >> 7) & 0x01);
   des_ptr->bmreqtype_type       = ((data[1] >> 5) & 0x03);
   des_ptr->bmreqtype_recipient  = (data[1] & 0x1F);
   des_ptr->brequest             = (data[2]);
   des_ptr->wvalue               = ((data[4] << 8) | data[3]);
   des_ptr->windex               = ((data[6] << 8) | data[5]);
   des_ptr->wlength              = ((data[8] << 8) | data[7]);
}
//-----------------------------------------------------------
static void ENUM_Clear_BCI(uint8 addr, uint8 endp) {
   struct XCSR_Xsst mask;
   ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, CLR_BCI);
   mask.sst = 0;
   mask.ovrLay.ctrlEndPointStruct.bci = ~0;
   XCSR_XSSTWriteMask(addr, endp, 0, mask);
}
//-----------------------------------------------------------
static void ENUM_Clear_BCO(uint8 addr, uint8 endp) {
   struct XCSR_Xsst mask;
   ilog_TEST_HARNESS_COMPONENT_0(ILOG_DEBUG, CLR_BCO);
   mask.sst = 0;
   mask.ovrLay.ctrlEndPointStruct.bco = ~0;
   XCSR_XSSTWriteMask(addr, endp, 0, mask);
}

