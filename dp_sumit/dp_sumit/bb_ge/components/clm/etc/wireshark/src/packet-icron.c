#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <epan/packet.h>
#include <epan/reassemble.h>
#include <glib.h>
#include <stdio.h>

#define ICRON_ETHERTYPE 0x88b7 /* IEEE OUI extended ethertype */

/* Bitmasks for IPF Command Byte and MLP Header */
#define IPF_CMD_BYTE    0xFF
#define IPF_CMD         0xC0
#define IPF_BYTE_CNT    0x3E
#define IPF_ODD_PAR     0x1
#define IPF_SOF         0x80
#define IPF_EOF         0x40

#define MLP_CRC6        0x3F
#define MLP_RXSEQ       0xF40
#define MLP_CMD_MAP     0xF000
#define MLP_TXSEQ       0xF080
#define MLP_CMD         0x7
#define MLP_FLC         0x8
#define MLP_VPORT       0x70

#define MLP_TAG_TYPE    0xF0
#define MLP_FRAME_VPORT 0x7
#define MLP_FRM_STRUCT  0xC
#define MLP_RETRY_MODE  0x2
#define MLP_RETRY_EN    0x1
#define MLP_DATA_QID    0x7F
#define MLP_FRAME_NUM   0x3FF8
#define MLP_UFRAME      0x7
#define MLP_FRAME_CRC8  0xFF
#define MLP_REX_QUEUE   0xE0
#define MLP_ACCELERATION    0x1C
#define MLP_MODIFIER    0x3
#define MLP_ACTION      0xC0
#define MLP_EP_TYPE     0x30
#define MLP_EP          0xF
#define MLP_TOGGLE      0x80
#define MLP_ADDRESS     0x7F
#define MLP_EU          0x80
#define MLP_HUB_ADDR    0x7F
#define MLP_S           0x80
#define MLP_PORT_ADDR   0x7F
#define MLP_RESPONSE    0xF0
#define MLP_FLC_CLASS   0xC
#define MLP_UPS_ASY     0x30
#define MLP_UPS_PER     0xC
#define MLP_UPS_ACC     0x3
#define MLP_DNS_ASY     0x30
#define MLP_DNS_PER     0xC
#define MLP_DNS_ACC     0x3
#define MLP_XICS_SID    0x3
#define MLP_CMD_SEQ     0xC0
#define MLP_CTRL_LINK_TYPE  0x38
#define MLP_CTRL_LINK_SUB   0x7
#define MLP_ENET_TYPE   0xF0
#define MLP_CRC16       0xFFFF
#define USB_CRC16       0xFFFF
#define XCTM_CRC16      0xFFFF

#define VALID_BYTES_32  0x20

// Local Functions
guint8 reverse1byte(guint8 value);

//#define IPF_PREAMBLE1    -2138454921 //0x8089C477
//#define IPF_PREAMBLE2    2125199285// 0x7EABF7B5
#define IPF_PREAMBLE1    2009368960 //0x77C48980    IPF preamble backwards (network order?)
#define IPF_PREAMBLE2    -1242059906// 0xB5F7AB7E   2nd half of preamble backwards
static int proto_icron = -1;

/* hf_ variables hold the IDs of the possible header fields */
static int hf_ipf = -1;
static int hf_ipf_preamble = -1;
static int hf_ipf_cmd_byte = -1;
static int hf_ipf_cmd = -1;
static int hf_ipf_byte_count = -1;
static int hf_ipf_odd_par = -1;

static int hf_mlp = -1;
static int hf_mlp_header = -1;
static int hf_mlp_vport = -1;
static int hf_mlp_xy_even_par = -1;
static int hf_mlp_cmd = -1;
static int hf_mlp_flc = -1;
static int hf_mlp_cmd_map = -1;
static int hf_mlp_cmd_map_ret = -1;
static int hf_mlp_rxseq_num = -1;
static int hf_mlp_txseq_num = -1;
static int hf_mlp_crc6 = -1;
static int hf_mlp_data = -1;
static int hf_mlp_data_frame = -1;
static int hf_mlp_frame_tag_type = -1;
static int hf_mlp_frame_vport = -1;
static int hf_mlp_frm_struct = -1;
static int hf_mlp_retry_mode = -1;
static int hf_mlp_retry_en = -1;
static int hf_mlp_data_qid = -1;
static int hf_mlp_frame_num = -1;
static int hf_mlp_uframe = -1;
static int hf_mlp_frame_crc8 = -1;
static int hf_mlp_rex_queue = -1;
static int hf_mlp_acceleration = -1;
static int hf_mlp_modifier = -1;
static int hf_mlp_action = -1;
static int hf_mlp_ep_type = -1;
static int hf_mlp_ep = -1;
static int hf_mlp_toggle = -1;
static int hf_mlp_address = -1;
static int hf_mlp_EU = -1;
static int hf_mlp_hub_address = -1;
static int hf_mlp_S = -1;
static int hf_mlp_port_address = -1;
static int hf_mlp_response = -1;
static int hf_mlp_flc_class = -1;
static int hf_mlp_ups_asy = -1;
static int hf_mlp_ups_per = -1;
static int hf_mlp_ups_acc = -1;
static int hf_mlp_dns_asy = -1;
static int hf_mlp_dns_per = -1;
static int hf_mlp_dns_acc = -1;
static int hf_mlp_xics_sid = -1;
static int hf_mlp_cmd_seq = -1;
static int hf_mlp_ctrl_link_type = -1;
static int hf_mlp_ctrl_link_msg_sub = -1;
static int hf_mlp_ctrl_link_down_sub = -1;
static int hf_mlp_ctrl_link_up_sub = -1;
static int hf_mlp_enet_type = -1;
static int hf_mlp_crc16 = -1;
static int hf_usb_crc16 = -1;
static int hf_xctm_crc16 = -1;

/* ett_ variables hold the IDs of the subtrees that may be created */
static int ett_ipf = -1;
static int ett_ipf_preamble = -1;
static int ett_ipf_cmd = -1;
static int ett_ipf_cmd_byte = -1;
static int ett_ipf_byte_count = -1;
static int ett_ipf_odd_par = -1;

static int ett_mlp = -1;
static int ett_mlp_header = -1;
static int ett_mlp_vport = -1;
static int ett_mlp_xy_even_par = -1;
static int ett_mlp_cmd = -1;
static int ett_mlp_flc = -1;
static int ett_mlp_cmd_map = -1;
static int ett_mlp_cmd_map_ret = -1;
static int ett_mlp_rxseq_num = -1;
static int ett_mlp_crc6 = -1;
static int ett_mlp_data = -1;
static int ett_mlp_data_frame = -1;
static int ett_mlp_frame_tag_type = -1;
static int ett_mlp_frame_vport = -1;
static int ett_mlp_frm_struct = -1;
static int ett_mlp_retry_mode = -1;
static int ett_mlp_retry_en = -1;
static int ett_mlp_data_qid = -1;
static int ett_mlp_frame_num = -1;
static int ett_mlp_uframe = -1;
static int ett_mlp_frame_crc8 = -1;
static int ett_mlp_rex_queue = -1;
static int ett_mlp_acceleration = -1;
static int ett_mlp_modifier = -1;
static int ett_mlp_action = -1;
static int ett_mlp_ep_type = -1;
static int ett_mlp_ep = -1;
static int ett_mlp_toggle = -1;
static int ett_mlp_address = -1;
static int ett_mlp_EU = -1;
static int ett_mlp_hub_address = -1;
static int ett_mlp_S = -1;
static int ett_mlp_port_address = -1;
static int ett_mlp_response = -1;
static int ett_mlp_flc_class = -1;
static int ett_mlp_ups_asy = -1;
static int ett_mlp_ups_per = -1;
static int ett_mlp_ups_acc = -1;
static int ett_mlp_dns_asy = -1;
static int ett_mlp_dns_per = -1;
static int ett_mlp_dns_acc = -1;
static int ett_mlp_xics_sid = -1;
static int ett_mlp_cmd_seq = -1;
static int ett_mlp_ctrl_link_type = -1;
static int ett_mlp_ctrl_link_msg_sub = -1;
static int ett_mlp_ctrl_link_down_sub = -1;
static int ett_mlp_ctrl_link_up_sub = -1;
static int ett_mlp_enet_type = -1;

/* Keeps track of data being split between packets */
static int overflow = 0;

static const value_string mlp_cmd_string[] = {
    { 0, "CMD POLL" },
    { 1, "CMD STAT" },
    { 2, "CMD DATA ACK" },
    { 3, "CMD DATA NAK" },
    { 4, "CMD DATN" },
    { 0, NULL}
};

enum ipfCommand
{
    START_FRAME,
    END_FRAME,
    START_END,
    MID_FRAME,
    IDLE,
    BYTE_COUNT_ERROR,
    PARITY_ERROR
};

static const char* ipfCommandString[] =
{
    "Start of Frame",
    "End of Frame",
    "Start and End of Frame",
    "Mid Frame",
    "Idle",
    "Byte Count Error",
    "Parity Error"
};

enum command
{
    POLL,
    STAT,
    DATA_ACK,
    DATA_NACK,
    DATN,
    UNDEFINED_CMD
};

enum commandMap
{
    RQST,
    RESP,
    ACK,
    NAK,
    RST,
    UNDEFINED
};

static const char* commandMapString[] =
{
    "RQST",
    "RESP",
    "ACK",
    "NAK",
    "RST",
    "UNDEFINED"
};

static const value_string mlp_cmd_map_string[] = {
    { 0, "MAP ACK" },
    { 1, "MAP NAK" },
    { 2, "MAP RST" },
    { 0, NULL}
};

static const value_string mlp_tag_type_string[] = {
    { 0, "SOF" },
    { 1, "Downstream XUSB (Async)" },
    { 2, "Downstream XUSB (Split/Periodic)" },
    { 3, "Upstream XUSB" },
    { 4, "Flow Control" },
    { 5, "Diagnostic" },
    { 6, "Control Link (us to us)" },
    { 7, "Media (others through us to others)" },
    { 8, "Inter-Chip-Communication (us to others & others to us)" },
    { 9, "CPU Ethernet Channel (GMII/MII nodes)" },
    { 0, NULL}
};

enum tagType
{
    SOF_TAG,
    Downstream_XUSB_Async,
    Downstream_XUSB_Split,
    Upstream_XUSB,
    Flow_Control,
    Diagnostic,
    Control_Link,
    Media,
    Interchip,
    CPU_Enet,
    Error
};

static const value_string mlp_frm_struct_string[] = {
    { 0, "Single Frame Header No Data" },
    { 1, "Double Frame Header No Data" },
    { 2, "Single Frame Header With Data" },
    { 3, "Double Frame Header With Data" },
    { 0, NULL}
};

enum frmStruct
{
    Single_Frame_Header_No_Data,
    Double_Frame_Header_No_Data,
    Single_Frame_Header_With_Data,
    Double_Frame_Header_With_Data
};

static const value_string mlp_retry_mode_string[] = {
    { 0, "Advanced Delivery" },
    { 1, "Confirmed Delivery" },
    { 0, NULL}
};

static const value_string mlp_ctrl_link_string[] = {
    { 0, "Link Message" },
    { 1, "USB Downstream State Change Message" },
    { 2, "USB Upstream State Change Message" },
    { 0, NULL}
};

static const value_string mlp_ctrl_link_sub_msg_string[] = {
    { 0, "Broadcast XUSB" },
    { 1, "Answer Broadcast XUSB" },
    { 2, "Vport Assign" },
    { 0, NULL}
};

static const value_string mlp_ctrl_link_sub_down_state_string[] = {
    { 0, "Downstream USB High Speed Connect" },
    { 1, "Downstream USB Full Speed Connect" },
    { 2, "Downstream USB Low Speed Connect" },
    { 3, "Downstream USB Disconnect" },
    { 4, "Downstream USB Remote Wakeup" },
    { 0, NULL}
};

static const value_string mlp_ctrl_link_sub_up_state_string[] = {
    { 0, "Upstream USB Suspend" },
    { 1, "Upstream USB Resume" },
    { 2, "Upstream USB Resume Done" },
    { 3, "Upstream USB Disconnect" },
    { 4, "Upstream USB Low Speed Bus Reset" },
    { 5, "Upstream USB Full Speed Bust Reset" },
    { 6, "Upstream USB High Speed Bus Reset" },
    { 7, "Upstream USB Bust Reset Done" },
    { 0, NULL}
};

static const value_string mlp_modifier_string[] = {
    { 0, "NULL" },
    { 1, "PRE" },
    { 2, "SSPLIT" },
    { 3, "CSPLIT" },
    { 0, NULL}
};

static const value_string mlp_action_string[] = {
    { 0, "IN" },
    { 1, "OUT" },
    { 2, "SETUP" },
    { 3, "PING" },
    { 0, NULL}
};

static const value_string mlp_endpoint_type_string[] = {
    { 0, "Control" },
    { 1, "Isochronous" },
    { 2, "Bulk" },
    { 3, "Interrupt" },
    { 0, NULL}
};

static const value_string mlp_rex_queue_string[] = {
    { 0, "Async Hard Scheduled" },
    { 1, "Periodic Soft Scheduled" },
    { 2, "Async Soft Scheduled" },
    { 3, "InMass Storage Soft Scheduled" },
    { 4, "Internal Scheduled packet" },
    { 0, NULL}
};

static const value_string mlp_acceleration_string[] = {
    { 0, "Not Accelerated" },
    { 1, "MSA CBW" },
    { 2, "MSA Data" },
    { 3, "MSA CSW" },
    { 0, NULL}
};

static const value_string mlp_response_string[] = {
    { 0, "Null" },
    { 1, "Ack" },
    { 2, "Nak" },
    { 3, "Stall" },
    { 4, "Nyet" },
    { 5, "Err" },
    { 6, "Timeout" },
    { 7, "3k" },
    { 8, "Data0" },
    { 9, "Data1" },
    { 10, "Data2" },
    { 11, "MData" },
    { 12, "SchStop"},
    { 0, NULL}
};

static const value_string mlp_flc_class_string[] = {
    { 0, "Null" },
    { 1, "Async" },
    { 2, "Periodic" },
    { 3, "MSA" },
    { 0, NULL}
};

static const value_string mlp_flow_control_common_string[] = {
    { 0, "Neither Low or High threshold exceeded" },
    { 1, "Low threshold exceeded" },
    { 3, "High threshold exceeded" },
    { 0, NULL}
};

static const value_string mlp_xics_sid_string[] = {
    { 0, "No Low, Mid or High threshold exceeded" },
    { 1, "Low threshold exceeded" },
    { 2, "Mid threshold exceeded" },
    { 3, "High threshold exceeded" },
    { 0, NULL}
};

/* tvb is a buffer containing the packet data */
static void dissect_icron(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{   /* Used to organize trees and items displayed in WireShark */
    proto_item *IPF_item = NULL;
    proto_item *IPF_sub_item = NULL;
    proto_tree *IPF_tree = NULL;
    proto_tree *IPF_data_tree = NULL;
    proto_item *MLP_item = NULL;
    proto_item *MLP_sub_item = NULL;
    proto_tree *MLP_tree = NULL;
    proto_tree *MLP_header_tree = NULL;
    proto_tree *MLP_frame_tag_tree = NULL;
    proto_item *MLP_frame_tag_item = NULL;

    col_set_str(pinfo->cinfo, COL_PROTOCOL, "ICRON");
    /* Clear out stuff in the info column */
    col_clear(pinfo->cinfo,COL_INFO);


    if(tree)
    {/* We are asked for details, not just summary */
        guint32 offset = 0;
        guint8 cmd_byte = 0;
        guint8 ipf_cmdIndex = 0;
        guint8 ipf_byte_count = 0;
        guint8 ipf_parity = 0;
        guint8 mlp_valid_bytes = 0; /* Number of valid bytes determined from IPF cmd byte */
        guint8 sof_cmd = 0;
        guint8 eof_cmd = 0;
        guint8 cmd = 0;
        gint32 pre1 = 0; /* Variables to check for IPF preamble */
        gint32 pre2 = 0;
        guint8 xy_cmp = 0;
        guint8 rxseq_num = 0;
        guint8 txseq_num = 0;
        guint8 cmd_map = 0;
        guint8 mapped_command = 0;
        guint8 tag_type;
        guint8 ctrl_link_type;
        guint16 crc16;
        gint available;
        guint8 lsb;
        guint8 msb;
        /* Loop while there is data left in the packet */

        while(offset < tvb_reported_length(tvb)){
            pre1 = 0;
            pre2 = 0;
            /* Start parsing bytes checking for preamble */
            if(offset == 0){
                tvb_memcpy(tvb, (guint32 *)&pre1, offset, 4);
                tvb_memcpy(tvb, (guint32 *)&pre2, offset + 4, 4);
            }
            /* Check to see if there is an overflow from the last packet parsed */
            if(overflow != 0 && offset == 0 && !((pre1 == IPF_PREAMBLE1) && (pre2 == IPF_PREAMBLE2))){
                IPF_item = proto_tree_add_item(tree, proto_icron, tvb, 0, overflow, ENC_NA);
                /* Add a subtree under the main icron tree, with the above IPF_item as the top node */
                IPF_tree = proto_item_add_subtree(IPF_item, ett_ipf);
                proto_tree_add_item(IPF_tree, hf_mlp_data, tvb, offset, overflow, ENC_NA);
                offset += overflow;
                overflow = 0;
            }
            if((pre1 == IPF_PREAMBLE1) && (pre2 == IPF_PREAMBLE2)){
                /* Save command byte to parse */
                tvb_memcpy(tvb, (guint8 *)&cmd_byte, offset+8, 1);
                /* Get valid bytes by masking cmd_byte and shifting result */
                mlp_valid_bytes = cmd_byte & IPF_BYTE_CNT;
                mlp_valid_bytes = mlp_valid_bytes >> 1;
                mlp_valid_bytes += 1;
                /* Get SOF command by masking cmd_byte and shifting */
                sof_cmd = cmd_byte & IPF_SOF;
                sof_cmd = sof_cmd >> 7;
                /* Highlight preamble + IPF cmd byte + mlp valid bytes */
                /* Would be nice to highlight all MLP packets if there's more than one, but I think that could get messy */
                /* Parameters:(tree, item to add, buffer, start offset, num bytes to highlight (-1 means go to end), encoding) */
                IPF_item = proto_tree_add_item(tree, proto_icron, tvb, 0, 9 + mlp_valid_bytes, ENC_NA);
                /* Add a subtree under the main icron tree, with the above IPF_item as the top node */
                IPF_tree = proto_item_add_subtree(IPF_item, ett_ipf);
                /* Add preamble to subtree */
                proto_tree_add_item(IPF_tree, hf_ipf_preamble, tvb, offset, 8, ENC_NA);
                offset += 8;
            }else{
                /* Save command byte to parse */
                tvb_memcpy(tvb, (guint8 *)&cmd_byte, offset, 1);
                /* Get valid bytes by masking cmd_byte and shifting result */
                mlp_valid_bytes = cmd_byte & IPF_BYTE_CNT;
                mlp_valid_bytes = mlp_valid_bytes >> 1;
                mlp_valid_bytes += 1;
                /* Get SOF command by masking cmd_byte and shifting */
                sof_cmd = cmd_byte & IPF_SOF;
                sof_cmd = sof_cmd >> 7;
                if(offset == 0){
                    IPF_item = proto_tree_add_item(tree, proto_icron, tvb, offset, 1 + mlp_valid_bytes, ENC_NA);
                }
                /* Add a subtree under the main icron tree, with the above IPF_item as the top node */
                IPF_tree = proto_item_add_subtree(IPF_item, ett_ipf);
            }

            /* Start parsing MLP bytes, do at least one iteration and continue until EOF command
             * is seen in IPF command byte. Request more data as needed. */
            do{
                /* Get EOF command */
                eof_cmd = cmd_byte & IPF_EOF;
                eof_cmd = eof_cmd >> 6;
                available = tvb_reported_length_remaining(tvb, offset);
                if(available < mlp_valid_bytes){
                    /* For some reason this method for reassembling packets isn't working
                       so I just use a global var to keep track of overflow*/
                    /* If we ran out of data ask for more
                    pinfo->desegment_offset = offset;
                    pinfo->desegment_len = DESEGMENT_ONE_MORE_SEGMENT;
                    pinfo->desegment_len = mlp_valid_bytes - available; */
                    /* Overflow is bytes left to parse in the IPF frame - bytes available */
                    /* +1 is for the command byte taking one of the available byte spots, not data */
                    overflow = mlp_valid_bytes - available + 1;
                    return;
                }
                /* Add subtree for IPF Command Byte data */
                /* Uncomment section to display all idle MLP commands, but there can be a lot of them */
                /*
                IPF_sub_item = proto_tree_add_item(IPF_tree, hf_ipf_cmd_byte, tvb, offset, 1, ENC_NA);
                IPF_data_tree = proto_item_add_subtree(IPF_sub_item, ett_ipf_cmd_byte);
                proto_tree_add_item(IPF_data_tree, hf_ipf_cmd, tvb, offset, 1, ENC_NA);
                proto_tree_add_uint_format_value(IPF_data_tree, hf_ipf_byte_count, tvb, offset, 1,
                        cmd_byte, "%u", mlp_valid_bytes);
                proto_tree_add_item(IPF_data_tree, hf_ipf_odd_par, tvb, offset, 1, ENC_NA);
                */

                /* Increment for idle MLP command, if not idle then offset will be handled in if statement below */
                offset += 1;

                ipf_parity = cmd_byte & IPF_ODD_PAR;

                /* If idle */
                if(!eof_cmd && !sof_cmd && (mlp_valid_bytes == 1))
                {
                    if(ipf_parity == 1)
                    {
                        ipf_cmdIndex = IDLE;
                    }
                    else
                    {
                        ipf_cmdIndex = PARITY_ERROR;
                    }

                    // Print Idle packets only if parity error has occurred
                    if (ipf_cmdIndex== PARITY_ERROR)
                    {
                        proto_tree_add_uint_format_value(IPF_tree, hf_ipf_cmd, tvb, offset, 1, cmd_byte,
                            "%s", ipfCommandString[ipf_cmdIndex]);
                    }
                }
                else
                {
                    offset -= 1; /* Decrement offset before adding items to tree, then re-increment */
                    //IPF_sub_item = proto_tree_add_item(IPF_tree, hf_ipf_cmd_byte, tvb, offset, 1, ENC_NA);
                    //IPF_data_tree = proto_item_add_subtree(IPF_sub_item, ett_ipf_cmd_byte);
                    //proto_tree_add_item(IPF_data_tree, hf_ipf_cmd, tvb, offset, 1, ENC_NA);

                    // Start of Frame
                    if(sof_cmd && !eof_cmd)
                    {
                        // Byte Count must be 31 and Odd Parity must be 1
                        if((mlp_valid_bytes == VALID_BYTES_32) && (ipf_parity == 1))
                        {
                            ipf_cmdIndex = START_FRAME;
                        }
                        else if (ipf_parity == 0)
                        {
                            ipf_cmdIndex = PARITY_ERROR;
                        }
                        else if (mlp_valid_bytes != VALID_BYTES_32)
                        {
                            ipf_cmdIndex = BYTE_COUNT_ERROR;
                        }
                    }
                    // End of Frame
                    else if (!sof_cmd && eof_cmd)
                    {
                        ipf_cmdIndex = END_FRAME;
                    }
                    // Start and End of Frame
                    else if (sof_cmd && eof_cmd)
                    {
                        ipf_cmdIndex = START_END;
                    }
                    // Mid Frame
                    else if (!sof_cmd && !eof_cmd)
                    {
                        if( (mlp_valid_bytes == VALID_BYTES_32) && (ipf_parity == 0))
                        {
                            ipf_cmdIndex = MID_FRAME;
                        }
                        else if (ipf_parity == 1)
                        {
                            ipf_cmdIndex = PARITY_ERROR;
                        }
                        else if (mlp_valid_bytes != VALID_BYTES_32)
                        {
                            ipf_cmdIndex = BYTE_COUNT_ERROR;
                        }
                    }

                    if((ipf_cmdIndex == PARITY_ERROR) || (ipf_cmdIndex == BYTE_COUNT_ERROR))
                    {
                        proto_tree_add_uint_format_value(IPF_tree, hf_ipf_cmd, tvb, offset, 1, cmd_byte,
                            "%s", ipfCommandString[ipf_cmdIndex]);
                    }
                    else
                    {
                        proto_tree_add_uint_format_value(IPF_tree, hf_ipf_cmd, tvb, offset, 1, cmd_byte,
                                "%s, Byte Count: %u, Odd Parity: %d", ipfCommandString[ipf_cmdIndex],
                                mlp_valid_bytes, ipf_parity);
                    }

                    /* Format output so cmd_byte bits can be displayed but valid byte count is also shown */
                    //proto_tree_add_uint_format_value(IPF_data_tree, hf_ipf_byte_count, tvb, offset, 1,
                    //        cmd_byte, "%u", mlp_valid_bytes);
                    //proto_tree_add_item(IPF_data_tree, hf_ipf_odd_par, tvb, offset, 1, ENC_NA);

                    offset += 1;
                    if(sof_cmd)
                    {
                        /* Only parse MLP header for SOF command */
                        MLP_item = proto_tree_add_item(IPF_tree, hf_mlp, tvb, offset, mlp_valid_bytes, ENC_NA);
                        MLP_tree = proto_item_add_subtree(MLP_item, ett_mlp);
                        /* Add header subtree to MLP_tree */
                        MLP_sub_item = proto_tree_add_item(MLP_tree, hf_mlp_header, tvb, offset, 3, ENC_NA);
                        MLP_header_tree = proto_item_add_subtree(MLP_sub_item, ett_mlp_header);
                        /* Add header fields to MLP header subtree */
                        // Vport
                        proto_tree_add_item(MLP_header_tree, hf_mlp_vport, tvb, offset, 1, ENC_NA);
                        // FLC Status
                        proto_tree_add_item(MLP_header_tree, hf_mlp_flc, tvb, offset, 1, ENC_NA);
                        // Display command
                        proto_tree_add_item(MLP_header_tree, hf_mlp_cmd, tvb, offset, 1, ENC_NA);
                        tvb_memcpy(tvb, (guint8 *)&cmd, offset, 1);
                        // Parse the actual command
                        cmd = cmd & MLP_CMD;
                        offset += 1;

                        tvb_memcpy(tvb, (guint8 *)&msb, offset, 1);
                        tvb_memcpy(tvb, (guint8 *)&lsb, offset+1, 1);

                        // Command map depends on parsed command value
                        if(cmd != DATA_ACK && cmd != DATA_NACK)
                        {
                            cmd_map = (msb & 0xF0) >> 4;

                            // Mapping Commands
                            switch (cmd)
                            {
                                case POLL:

                                    if (cmd_map == 0)
                                    {
                                        mapped_command = RQST;
                                    }
                                    else if (cmd_map == 1)
                                    {
                                        mapped_command = RESP;
                                    }
                                    else
                                    {
                                        mapped_command = UNDEFINED;
                                    }

                                    break;

                                case STAT:

                                    if (cmd_map == 0)
                                    {
                                        mapped_command = ACK;
                                    }
                                    else if (cmd_map == 1)
                                    {
                                        mapped_command = NAK;
                                    }
                                    else if (cmd_map == 2)
                                    {
                                        mapped_command = RST;
                                    }
                                    else
                                    {
                                        mapped_command = UNDEFINED;
                                    }

                                    break;

                                case DATN:

                                    if (cmd_map == 0)
                                    {
                                        mapped_command = ACK;
                                    }
                                    else if (cmd_map == 1)
                                    {
                                        mapped_command = NAK;
                                    }
                                    else
                                    {
                                        mapped_command = UNDEFINED;
                                    }

                                    break;

                                default:
                                    mapped_command = UNDEFINED;
                                    break;
                            }


                            proto_tree_add_uint_format_value(MLP_header_tree, hf_mlp_cmd_map, tvb, offset, 1,
                                   ((guint16)(msb << 8) + (guint16) lsb), "%s", commandMapString[mapped_command]);
                        }
                        else{

                            // Add Tx Sequence number
                            txseq_num = ((msb & 0xF0) >> 4) + ((lsb & 0x80) >> 3);
                            proto_tree_add_uint_format_value(MLP_header_tree, hf_mlp_txseq_num, tvb, offset, 2,
                                    ((guint16)(msb << 8) + (guint16) lsb), "%d", txseq_num);
                        }

                        // Add Rx Sequence number
                        rxseq_num = (msb & 0xF) | ((lsb & 0x40) >> 2);

                        proto_tree_add_uint_format_value(MLP_header_tree, hf_mlp_rxseq_num, tvb, offset, 2,
                                ((guint16)(msb << 8) + (guint16) lsb),  "%d", rxseq_num);

                        // Add CRC6
                        proto_tree_add_item(MLP_header_tree, hf_mlp_crc6, tvb, offset, 2, ENC_NA);
                        offset += 2;

                        mlp_valid_bytes -= 3;
                        /* MLP data */
                        if(mlp_valid_bytes >= 8){
                            /* Get and save tag type */
                            tvb_memcpy(tvb, (guint8 *)&tag_type, offset, 1);
                            tag_type = tag_type & MLP_TAG_TYPE;
                            tag_type = tag_type >> 4;
                            /* Parse data frame depending on tag type */
                            switch(tag_type){
                                case SOF_TAG: /* SOF */
                                    MLP_frame_tag_item = proto_tree_add_item(MLP_tree, hf_mlp_data_frame, tvb, offset, 8, ENC_NA);
                                    MLP_frame_tag_tree = proto_item_add_subtree(MLP_frame_tag_item, ett_mlp_data_frame);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_tag_type, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_vport, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frm_struct, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_mode, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_en, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_data_qid, tvb, offset, 1, ENC_NA);
                                    offset += 3;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_num, tvb, offset, 2, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_uframe, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_crc8, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    mlp_valid_bytes -= 8;
                                    proto_tree_add_item(MLP_tree, hf_mlp_data, tvb, offset, mlp_valid_bytes, ENC_NA);
                                    break;
                                case Downstream_XUSB_Async: /* Downstream XUSB (Async) */
                                    MLP_frame_tag_item = proto_tree_add_item(MLP_tree, hf_mlp_data_frame, tvb, offset, 8, ENC_NA);
                                    MLP_frame_tag_tree = proto_item_add_subtree(MLP_frame_tag_item, ett_mlp_data_frame);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_tag_type, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_vport, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frm_struct, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_mode, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_en, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_data_qid, tvb, offset, 1, ENC_NA);
                                    offset += 2;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_rex_queue, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_acceleration, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_modifier, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_action, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_ep_type, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_ep, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_toggle, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_address, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_crc8, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    mlp_valid_bytes -= 8;
                                    proto_tree_add_item(MLP_tree, hf_mlp_data, tvb, offset, mlp_valid_bytes, ENC_NA);
                                    break;
                                case Downstream_XUSB_Split: /* Downstream XUSB (Split/Periodic) */
                                    MLP_frame_tag_item = proto_tree_add_item(MLP_tree, hf_mlp_data_frame, tvb, offset, 16, ENC_NA);
                                    MLP_frame_tag_tree = proto_item_add_subtree(MLP_frame_tag_item, ett_mlp_data_frame);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_tag_type, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_vport, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frm_struct, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_mode, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_en, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_data_qid, tvb, offset, 1, ENC_NA);
                                    offset += 2;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_rex_queue, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_acceleration, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_modifier, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_action, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_ep_type, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_ep, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_toggle, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_address, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_crc8, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    mlp_valid_bytes -= 8;
                                    if(mlp_valid_bytes >= 8){
                                        proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_EU, tvb, offset, 1, ENC_NA);
                                        proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_hub_address, tvb, offset, 1, ENC_NA);
                                        offset += 1;
                                        proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_S, tvb, offset, 1, ENC_NA);
                                        proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_port_address, tvb, offset, 1, ENC_NA);
                                        offset += 4;
                                        proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_num, tvb, offset, 2, ENC_NA);
                                        offset += 1;
                                        proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_uframe, tvb, offset, 1, ENC_NA);
                                        offset += 1;
                                        proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_crc8, tvb, offset, 1, ENC_NA);
                                        offset += 1;
                                        mlp_valid_bytes -= 8;
                                        proto_tree_add_item(MLP_tree, hf_mlp_data, tvb, offset, mlp_valid_bytes, ENC_NA);
                                    }
                                    break;
                                case Upstream_XUSB: /* Upstream XUSB */
                                    MLP_frame_tag_item = proto_tree_add_item(MLP_tree, hf_mlp_data_frame, tvb, offset, 8, ENC_NA);
                                    MLP_frame_tag_tree = proto_item_add_subtree(MLP_frame_tag_item, ett_mlp_data_frame);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_tag_type, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_vport, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frm_struct, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_mode, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_en, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_data_qid, tvb, offset, 1, ENC_NA);
                                    offset += 2;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_response, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_flc_class, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_modifier, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_action, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_ep_type, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_ep, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_toggle, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_address, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_crc8, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    mlp_valid_bytes -= 8;
                                    proto_tree_add_item(MLP_tree, hf_mlp_data, tvb, offset, mlp_valid_bytes, ENC_NA);
                                    break;
                                case Flow_Control: /* Flow Control */
                                    MLP_frame_tag_item = proto_tree_add_item(MLP_tree, hf_mlp_data_frame, tvb, offset, 8, ENC_NA);
                                    MLP_frame_tag_tree = proto_item_add_subtree(MLP_frame_tag_item, ett_mlp_data_frame);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_tag_type, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_vport, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frm_struct, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_mode, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_en, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_data_qid, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_ups_asy, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_ups_per, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_ups_acc, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_dns_asy, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_dns_per, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_dns_acc, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_xics_sid, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    mlp_valid_bytes -= 8;
                                    proto_tree_add_item(MLP_tree, hf_mlp_data, tvb, offset, mlp_valid_bytes, ENC_NA);
                                    break;
                                /* Tag Types 5-9 not tested with actual data yet (none available) */
                                case Control_Link: /* Control Link (us to us) */
                                    MLP_frame_tag_item = proto_tree_add_item(MLP_tree, hf_mlp_data_frame, tvb, offset, 16, ENC_NA);
                                    MLP_frame_tag_tree = proto_item_add_subtree(MLP_frame_tag_item, ett_mlp_data_frame);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_tag_type, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_vport, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frm_struct, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_mode, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_en, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_data_qid, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_cmd_seq, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_ctrl_link_type, tvb, offset, 1, ENC_NA);

                                    /* Use Control Link Type to get formate of subtype */
                                    tvb_memcpy(tvb, (guint8 *)&ctrl_link_type, offset, 1);
                                    ctrl_link_type = tag_type & MLP_CTRL_LINK_TYPE;
                                    ctrl_link_type = ctrl_link_type >> 3;
                                    switch(ctrl_link_type){
                                        case 0:
                                            proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_ctrl_link_msg_sub, tvb, offset, 1, ENC_NA);
                                            break;
                                        case 1:
                                            proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_ctrl_link_down_sub, tvb, offset, 1, ENC_NA);
                                            break;
                                        case 2:
                                        proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_ctrl_link_up_sub, tvb, offset, 1, ENC_NA);
                                            break;
                                        default:
                                            break;
                                    }
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_data, tvb, offset, 3, ENC_NA);
                                    offset += 3;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_crc8, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    mlp_valid_bytes -= 8;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_data, tvb, offset, 8, ENC_NA);
                                    offset += 8;
                                    mlp_valid_bytes -= 8;
                                    proto_tree_add_item(MLP_tree, hf_mlp_data, tvb, offset, mlp_valid_bytes, ENC_NA);
                                    break;
                                case CPU_Enet: /* CPU Ethernet Channel (GMII/MII nodes) */
                                    MLP_frame_tag_item = proto_tree_add_item(MLP_tree, hf_mlp_data_frame, tvb, offset, 16, ENC_NA);
                                    MLP_frame_tag_tree = proto_item_add_subtree(MLP_frame_tag_item, ett_mlp_data_frame);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_tag_type, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_vport, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_enet_type, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frm_struct, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_mode, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_en, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_data_qid, tvb, offset, 1, ENC_NA);
                                    offset += 5;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_crc8, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    mlp_valid_bytes -= 8;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_data, tvb, offset, 8, ENC_NA);
                                    offset += 8;
                                    mlp_valid_bytes -= 8;
                                    proto_tree_add_item(MLP_tree, hf_mlp_data, tvb, offset, mlp_valid_bytes, ENC_NA);
                                    break;
                                /* Case 5, 7, and 8 have same structure */
                                case Diagnostic: /* Diagnostic */
                                case Media: /* Media (others through us to others) */
                                case Interchip: /* Inter-Chip-Communication (us to others & others to us) */
                                    MLP_frame_tag_item = proto_tree_add_item(MLP_tree, hf_mlp_data_frame, tvb, offset, 16, ENC_NA);
                                    MLP_frame_tag_tree = proto_item_add_subtree(MLP_frame_tag_item, ett_mlp_data_frame);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_tag_type, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_vport, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frm_struct, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_mode, tvb, offset, 1, ENC_NA);
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_retry_en, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_data_qid, tvb, offset, 1, ENC_NA);
                                    offset += 2;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_data, tvb, offset, 3, ENC_NA);
                                    offset += 3;
                                    proto_tree_add_item(MLP_frame_tag_tree, hf_mlp_frame_crc8, tvb, offset, 1, ENC_NA);
                                    offset += 1;
                                    mlp_valid_bytes -= 8;
                                    proto_tree_add_item(MLP_tree, hf_mlp_data, tvb, offset, mlp_valid_bytes, ENC_NA);
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                    else
                    {
                        /* Not SOF, so the rest of the valid bytes are data */
                        MLP_item = proto_tree_add_item(IPF_tree, hf_mlp_data, tvb, offset, mlp_valid_bytes, ENC_NA);

                        if(eof_cmd)
                        {
                            if((tag_type == Downstream_XUSB_Async) || (tag_type == Downstream_XUSB_Split) ||
                                (tag_type == Upstream_XUSB))
                            {
                                offset += mlp_valid_bytes - 6;

                                // Format and Print USB CRC
                                tvb_memcpy(tvb, (guint8 *)&msb, offset, 1);
                                tvb_memcpy(tvb, (guint8 *)&lsb, offset+1, 1);

                                crc16 = (guint16) (reverse1byte(lsb)) |
                                    (guint16) (reverse1byte(msb) << 8);

                                proto_tree_add_uint_format_value(IPF_tree, hf_usb_crc16, tvb, offset, 2,
                                    crc16, "%#X", crc16);

                                offset += 2;

                                // Format and Print XCTM CRC
                                tvb_memcpy(tvb, (guint8 *)&msb, offset, 1);
                                tvb_memcpy(tvb, (guint8 *)&lsb, offset+1, 1);
                                crc16 = (guint16) (reverse1byte(lsb)) |
                                    (guint16) (reverse1byte(msb) << 8);
                                proto_tree_add_uint_format_value(IPF_tree, hf_xctm_crc16, tvb, offset, 2,
                                    crc16, "%#X", crc16);

                                offset += 2;

                                // Format and Print MLP CRC
                                tvb_memcpy(tvb, (guint8 *)&msb, offset, 1);
                                tvb_memcpy(tvb, (guint8 *)&lsb, offset+1, 1);
                                crc16 = (guint16) (reverse1byte(lsb)) |
                                    (guint16) (reverse1byte(msb) << 8);
                                proto_tree_add_uint_format_value(IPF_tree, hf_mlp_crc16, tvb, offset, 2,
                                    crc16, "%#X", crc16);

                                offset += 2;
                            }
                            else
                            {
                                offset += mlp_valid_bytes;
                            }
                        }
                    }
                }
                if(sof_cmd && eof_cmd){
                    offset += mlp_valid_bytes;
                }
                /* Parse next command byte(if needed) */
                if(!eof_cmd){
                    if(sof_cmd || mlp_valid_bytes > 1){
                        offset += mlp_valid_bytes;
                    }
                    available = tvb_reported_length_remaining(tvb, offset);
                    if(available < 1){
                        /* If we ran out of data ask for more(only need cmd byte here)
                        pinfo->desegment_offset = offset;
                        pinfo->desegment_len = DESEGMENT_ONE_MORE_SEGMENT;
                        return;*/
                        goto outer;
                    }
                    /* If still not eof get another cmd */
                    tvb_memcpy(tvb, (guint8 *)&cmd_byte, offset, 1);
                    /* Get valid bytes by masking cmd_byte and shifting */
                    mlp_valid_bytes = cmd_byte & IPF_BYTE_CNT;
                    mlp_valid_bytes = mlp_valid_bytes >> 1;
                    mlp_valid_bytes += 1;
                    /* Get SOF command by masking cmd_byte and shifting */
                    sof_cmd = cmd_byte & IPF_SOF;
                    sof_cmd = sof_cmd >> 7;
                }
            }while(!eof_cmd);
            outer:;
        }
    }
}

void proto_register_icron(void)
{
    /* A header field is something you can search/filter on.
    *
    * We create a structure to register our fields. It consists of an
    * array of hf_register_info structures, each of which are of the format
    * {&(field id), {name, abbrev, type, display, strings, bitmask, blurb, HFILL}}.
    */
    static hf_register_info hf[] = {
        { &hf_ipf,
        { "IPF", "ipf.ipf", FT_NONE, BASE_NONE, NULL, 0x0, "IPF", HFILL}},
        { &hf_ipf_preamble,
        { "Preamble", "ipf.preamble", FT_NONE, BASE_NONE, NULL, 0x0, "IPF Preamble", HFILL}},
        { &hf_ipf_cmd,
        { "IPF Command", "ipf.cmd", FT_UINT8, BASE_DEC, NULL, IPF_CMD_BYTE, "IPF Command", HFILL}},
        { &hf_mlp,
        { "MLP", "mlp.mlp", FT_NONE, BASE_NONE, NULL, 0x0, "MLP", HFILL}},
        { &hf_mlp_header,
        { "Header", "mlp.header", FT_NONE, BASE_NONE, NULL, 0x0, "MLP Header", HFILL}},
        { &hf_mlp_vport,
        { "Vport", "mlp.vport", FT_UINT8, BASE_DEC, NULL, MLP_VPORT, "MLP Vport", HFILL}},
        { &hf_mlp_flc,
        { "FLC", "mlp.flc", FT_UINT8, BASE_DEC, NULL, MLP_FLC, "MLP FLC", HFILL}},
        { &hf_mlp_cmd,
        { "MLP Command", "mlp.cmd", FT_UINT8, BASE_DEC, mlp_cmd_string, MLP_CMD, "MLP Command", HFILL}},
        { &hf_mlp_cmd_map,
        { "Command Map", "mlp.cmd_map", FT_UINT16, BASE_DEC, NULL, MLP_CMD_MAP, "MLP Command Map", HFILL}},
        { &hf_mlp_txseq_num,
        { "Txseq Number", "mlp.txseq_num", FT_UINT16, BASE_DEC, NULL, MLP_TXSEQ, "MLP Txseq Number", HFILL}},
        { &hf_mlp_rxseq_num,
        { "Rxseq Number", "mlp.rxseq_num", FT_UINT16, BASE_DEC, NULL, MLP_RXSEQ, "MLP Rxseq Number", HFILL}},
        { &hf_mlp_crc6,
        { "CRC6", "mlp.crc6", FT_UINT16, BASE_HEX, NULL, MLP_CRC6, "MLP CRC", HFILL}},
        { &hf_mlp_data,
        { "Data", "mlp.header", FT_NONE, BASE_NONE, NULL, 0x0, "Data", HFILL}},
        { &hf_mlp_data_frame,
        { "Data Frame", "mlp.data_frame", FT_NONE, BASE_NONE, NULL, 0x0, "MLP Data Frame", HFILL}},
        { &hf_mlp_frame_tag_type,
        { "Tag Type", "mlp.frame_tag_type", FT_UINT8, BASE_DEC, mlp_tag_type_string, MLP_TAG_TYPE, "MLP Frame Tag Type", HFILL}},
        { &hf_mlp_frame_vport,
        { "Vport", "mlp.frame_vport", FT_UINT8, BASE_DEC, NULL, MLP_FRAME_VPORT, "MLP Frame Vport", HFILL}},
        { &hf_mlp_frm_struct,
        { "Frm Struct", "mlp.frm_struct", FT_UINT8, BASE_DEC, mlp_frm_struct_string, MLP_FRM_STRUCT, "MLP Frm Struct", HFILL}},
        { &hf_mlp_retry_mode,
        { "Retry Mode", "mlp.retry_mode", FT_UINT8, BASE_DEC, mlp_retry_mode_string, MLP_RETRY_MODE, "MLP Frame Retry Mode", HFILL}},
        { &hf_mlp_retry_en,
        { "Retry En", "mlp.retry_en", FT_UINT8, BASE_DEC, NULL, MLP_RETRY_EN, "MLP Frame Retry En", HFILL}},
        { &hf_mlp_data_qid,
        { "Data Qid", "mlp.data_qid", FT_UINT8, BASE_DEC, NULL, MLP_DATA_QID, "MLP Frame Data QID", HFILL}},
        { &hf_mlp_frame_num,
        { "Frame Number", "mlp.frame_num", FT_UINT16, BASE_DEC, NULL, MLP_FRAME_NUM, "MLP Frame Number", HFILL}},
        { &hf_mlp_uframe,
        { "Microframe", "mlp.uframe", FT_UINT8, BASE_DEC, NULL, MLP_UFRAME, "MLP Microframe", HFILL}},
        { &hf_mlp_frame_crc8,
        { "Frame CRC", "mlp.frame_crc", FT_UINT8, BASE_HEX, NULL, MLP_FRAME_CRC8, "MLP Frame CRC", HFILL}},
        { &hf_mlp_rex_queue,
        { "Rex Queue", "mlp.rex_queue", FT_UINT8, BASE_DEC, mlp_rex_queue_string, MLP_REX_QUEUE, "MLP Rex Queue", HFILL}},
        { &hf_mlp_acceleration,
        { "Acceleration", "mlp.acceleration", FT_UINT8, BASE_DEC, mlp_acceleration_string, MLP_ACCELERATION, "MLP Acceleration", HFILL}},
        { &hf_mlp_modifier,
        { "Modifier", "mlp.modifier", FT_UINT8, BASE_DEC, mlp_modifier_string, MLP_MODIFIER, "MLP Modifier", HFILL}},
        { &hf_mlp_action,
        { "Action", "mlp.action", FT_UINT8, BASE_DEC, mlp_action_string, MLP_ACTION, "MLP Action", HFILL}},
        { &hf_mlp_ep_type,
        { "Endpoint Type", "mlp.ep_type", FT_UINT8, BASE_DEC, mlp_endpoint_type_string, MLP_EP_TYPE, "MLP Endpoint Type", HFILL}},
        { &hf_mlp_ep,
        { "Endpoint", "mlp.endpoint", FT_UINT8, BASE_DEC, NULL, MLP_EP, "MLP Endpoint", HFILL}},
        { &hf_mlp_toggle,
        { "Toggle", "mlp.toggle", FT_UINT8, BASE_DEC, NULL, MLP_TOGGLE, "MLP Toggle", HFILL}},
        { &hf_mlp_address,
        { "Address", "mlp.address", FT_UINT8, BASE_DEC, NULL, MLP_ADDRESS, "MLP Address", HFILL}},
        { &hf_mlp_EU,
        { "E/U", "mlp.eu", FT_UINT8, BASE_DEC, NULL, MLP_EU, "MLP EU", HFILL}},
        { &hf_mlp_hub_address,
        { "Hub Address", "mlp.hub_address", FT_UINT8, BASE_DEC, NULL, MLP_HUB_ADDR, "MLP Hub Address", HFILL}},
        { &hf_mlp_S,
        { "S", "mlp.s", FT_UINT8, BASE_DEC, NULL, MLP_S, "MLP S", HFILL}},
        { &hf_mlp_port_address,
        { "Port Address", "mlp.port_address", FT_UINT8, BASE_DEC, NULL, MLP_PORT_ADDR, "MLP Port Address", HFILL}},
        { &hf_mlp_response,
        { "Response", "mlp.response", FT_UINT8, BASE_DEC, mlp_response_string, MLP_RESPONSE, "MLP Response", HFILL}},
        { &hf_mlp_flc_class,
        { "FLC Class", "mlp.flc_class", FT_UINT8, BASE_DEC, mlp_flc_class_string, MLP_FLC_CLASS, "MLP FLC Class", HFILL}},
        { &hf_mlp_ups_asy,
        { "UPS Asy Status", "mlp.ups_asy", FT_UINT8, BASE_DEC, mlp_flow_control_common_string, MLP_UPS_ASY, "MLP UPS ASY Status", HFILL}},
        { &hf_mlp_ups_per,
        { "UPS Per Status", "mlp.ups_per", FT_UINT8, BASE_DEC, mlp_flow_control_common_string, MLP_UPS_PER, "MLP UPS PER Status", HFILL}},
        { &hf_mlp_ups_acc,
        { "UPS Acc Status", "mlp.ups_acc", FT_UINT8, BASE_DEC, mlp_flow_control_common_string, MLP_UPS_ACC, "MLP UPS ACC Status", HFILL}},
        { &hf_mlp_dns_asy,
        { "UPS Dns Status", "mlp.dns_asy", FT_UINT8, BASE_DEC, mlp_flow_control_common_string, MLP_DNS_ASY, "MLP DNS ASY Status", HFILL}},
        { &hf_mlp_dns_per,
        { "UPS Dns Status", "mlp.dns_per", FT_UINT8, BASE_DEC, mlp_flow_control_common_string, MLP_DNS_PER, "MLP DNS PER Status", HFILL}},
        { &hf_mlp_dns_acc,
        { "UPS Dns Status", "mlp.dns_acc", FT_UINT8, BASE_DEC, mlp_flow_control_common_string, MLP_DNS_ACC, "MLP DNS ACC Status", HFILL}},
        { &hf_mlp_xics_sid,
        { "XICS Sid Status", "mlp.xics_sid", FT_UINT8, BASE_DEC, mlp_xics_sid_string, MLP_XICS_SID, "MLP XICS SID Status", HFILL}},
        { &hf_mlp_cmd_seq,
        { "Command Sequence", "mlp.cmd_seq", FT_UINT8, BASE_DEC, NULL, MLP_CMD_SEQ, "MLP Command Sequence", HFILL}},
        { &hf_mlp_ctrl_link_type,
        { "Control Link Type", "mlp.ctrl_link_type", FT_UINT8, BASE_DEC, mlp_ctrl_link_string, MLP_CTRL_LINK_TYPE, "MLP Control Link Type", HFILL}},
        { &hf_mlp_ctrl_link_msg_sub,
        { "Control Link Message", "mlp.ctrl_link_msg_sub", FT_UINT8, BASE_DEC, mlp_ctrl_link_sub_msg_string, MLP_CTRL_LINK_SUB, "MLP Control Link Message", HFILL}},
        { &hf_mlp_ctrl_link_down_sub,
        { "Control Link Downstream State Change", "mlp.ctrl_link_down_sub", FT_UINT8, BASE_DEC, mlp_ctrl_link_sub_down_state_string, MLP_CTRL_LINK_SUB, "MLP Control Downstream State Change", HFILL}},
        { &hf_mlp_ctrl_link_up_sub,
        { "Control Link Upstream State Change", "mlp.ctrl_link_up_sub", FT_UINT8, BASE_DEC, mlp_ctrl_link_sub_up_state_string, MLP_CTRL_LINK_SUB, "MLP Control Upstream State Change", HFILL}},
        { &hf_mlp_enet_type,
        { "Ethernet Type", "mlp.enet_type", FT_UINT8, BASE_DEC, NULL, MLP_ENET_TYPE, "MLP Ethernet Type", HFILL}},
        { &hf_mlp_crc16,
        { "MLP CRC", "mlp.crc16", FT_UINT16, BASE_HEX, NULL, MLP_CRC16, "MLP CRC16", HFILL}},
        { &hf_usb_crc16,
        { "USB CRC", "usb.crc16", FT_UINT16, BASE_HEX, NULL, USB_CRC16, "USB CRC16", HFILL}},
        { &hf_xctm_crc16,
        { "XCTM CRC", "xctm.crc16", FT_UINT16, BASE_HEX, NULL, XCTM_CRC16, "XCTM CRC16", HFILL}}
        };
        /* Used when adding items to tree, keeps track of whether or not tree item is expanded */
        static int *ett[] = {
            &ett_ipf,
            &ett_ipf_preamble,
            &ett_ipf_cmd_byte,
            &ett_ipf_cmd,
            &ett_ipf_byte_count,
            &ett_ipf_odd_par,
            &ett_mlp,
            &ett_mlp_header,
            &ett_mlp_vport,
            &ett_mlp_flc,
            &ett_mlp_cmd,
            &ett_mlp_cmd_map,
            &ett_mlp_rxseq_num,
            &ett_mlp_crc6,
            &ett_mlp_data,
            &ett_mlp_data_frame,
            &ett_mlp_frame_tag_type,
            &ett_mlp_frame_vport,
            &ett_mlp_frm_struct,
            &ett_mlp_retry_mode,
            &ett_mlp_retry_en,
            &ett_mlp_data_qid,
            &ett_mlp_frame_num,
            &ett_mlp_uframe,
            &ett_mlp_frame_crc8,
            &ett_mlp_rex_queue,
            &ett_mlp_acceleration,
            &ett_mlp_modifier,
            &ett_mlp_action,
            &ett_mlp_ep_type,
            &ett_mlp_ep,
            &ett_mlp_toggle,
            &ett_mlp_address,
            &ett_mlp_EU,
            &ett_mlp_hub_address,
            &ett_mlp_S,
            &ett_mlp_port_address,
            &ett_mlp_response,
            &ett_mlp_flc_class,
            &ett_mlp_ups_asy,
            &ett_mlp_ups_per,
            &ett_mlp_ups_acc,
            &ett_mlp_dns_asy,
            &ett_mlp_dns_per,
            &ett_mlp_dns_acc,
            &ett_mlp_xics_sid,
            &ett_mlp_cmd_seq,
            &ett_mlp_ctrl_link_type,
            &ett_mlp_ctrl_link_msg_sub,
            &ett_mlp_ctrl_link_down_sub,
            &ett_mlp_ctrl_link_up_sub,
            &ett_mlp_enet_type
        };
    proto_icron = proto_register_protocol (
        "ICRON Protocol", /* name       */
        "ICRON",      /* short name */
        "icron"       /* abbrev     */
        );
        proto_register_field_array(proto_icron, hf, array_length(hf));
        proto_register_subtree_array(ett, array_length(ett));
        register_dissector("icron", dissect_icron, proto_icron);
}

void proto_reg_handoff_icron(void)
{
    static dissector_handle_t icron_handle;

    icron_handle = create_dissector_handle(dissect_icron, proto_icron);
    /* Associate with target ethertype */
    dissector_add_uint("ethertype", ICRON_ETHERTYPE, icron_handle);
}

// Reverse 1 byte
guint8 reverse1byte(guint8 value)
{
    value = (((value & 0xAA) >> 1) | ((value & 0x55) << 1));
    value = (((value & 0xCC) >> 2) | ((value & 0x33) << 2));
    value = (((value & 0xF0) >> 4) | ((value & 0x0F) << 4));

    return value;
}
