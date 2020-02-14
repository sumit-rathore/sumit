//#################################################################################################
// Icron Technology Corporation - Copyright 2018
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef UPP_LOG_H
#define UPP_LOG_H

// Includes #######################################################################################
#include <project_components.h>
#include <ilog.h>

// Constants and Macros ###########################################################################

ILOG_CREATE(UPP_COMPONENT)
    ILOG_ENTRY(UPP_INIT,                            "Upp Initialised \n")
    ILOG_ENTRY(UPP_ULP_STATUS,                      "UppUlpStatus(), status = %d \n")
    ILOG_ENTRY(UPP_INTERRUPT,                       "UPP interrupt %08x\n")
    ILOG_ENTRY(UPP_PROCESS_H2D,                     "Host to Device packet received\n")
    ILOG_ENTRY(UPP_PROCESS_D2H,                     "Device to Host packet processed\n")
    ILOG_ENTRY(UPP_UNHANDLED_ISR,                   "Unhandled Upp ISR ISR0:%08x ISR1:%08x\n")
    ILOG_ENTRY(UPP_PACKET_INFO,                     "Packet Data: %08x Control: %04x bytes rx:%d\n")
    ILOG_ENTRY(UPP_END_OF_PACKET,                   "End of packet time = %d microseconds vlane %d\n")
    ILOG_ENTRY(UPP_PACKET_END_PROCESSING,           "Packet end, writing device address %x\n")

    ILOG_ENTRY(UPP_HAL_SET_ENDPOINT_BAD_BINTERVAL,  "Bad bInterval value %d\n")
    ILOG_ENTRY(UPP_HAL_SET_ENDPOINT,                "Writing to Endpoint table, device:%d number:%d QID:%d\n")
    ILOG_ENTRY(UPP_HAL_SET_ENDPOINT_RAW,            "                           enable = %d config0 = %08x config1 = %08x\n")

    ILOG_ENTRY(UPP_PARSE_CONFIG_DESCRIPTOR_INFO,    "New Configuration,  Device: %d Configuration:%d Number of interfaces:%d\n")
    ILOG_ENTRY(UPP_INTERFACE_DESCRIPTOR_INFO,       "New Interface:%d Alt:%d Number of endpoints:%d\n")
    ILOG_ENTRY(UPP_ENDPOINT_ATTRIBUTE_CONTROL,      "Endpoint transfer type is Control\n")
    ILOG_ENTRY(UPP_ENDPOINT_ATTRIBUTE_ISO,          "Endpoint transfer type is Iso\n")
    ILOG_ENTRY(UPP_ENDPOINT_ATTRIBUTE_BULK,         "Endpoint transfer type is Bulk\n")
    ILOG_ENTRY(UPP_ENDPOINT_ATTRIBUTE_INTERRUPT,    "Endpoint transfer type is Interrupt\n")
    ILOG_ENTRY(UPP_ENDPOINT_ADDRESS,                "Endpoint address is %d\n")

    ILOG_ENTRY(UPP_LEX_MSG_SET_ENDPOINT,            "Endpoint set message, device %d, endpoint %d buffer %d\n")
    ILOG_ENTRY(UPP_LEX_MSG_CLEAR_ENDPOINT,          "Endpoint clear message, device %d, endpoint %d buffer %d\n")
    ILOG_ENTRY(UPP_LEX_MSG_SET_INTERFACE,           "Set interface, device %d location %08x\n")
    ILOG_ENTRY(UPP_LEX_MSG_REMOVE_DEVICE,           "Remove device %d, devicePtr %08x\n")
    ILOG_ENTRY(UPP_LEX_MSG_ROUTE_CHANGE_DONE,       "Lex Route change done, device %d\n")

	ILOG_ENTRY(UPP_REX_MSG_SET_ENDPOINT,            "Rex set endpoint, device %d, endpoint %d buffer %d\n")
    ILOG_ENTRY(UPP_REX_MSG_CLEAR_ENDPOINT,          "Rex cleared endpoint, device %d, endpoint %d buffer %d\n")
    ILOG_ENTRY(UPP_REX_MSG_ENDPOINT_NOT_RESPONSIVE, "Rex detected unresponsive endpoint, device %d, endpoint %d buffer %d\n")
    ILOG_ENTRY(UPP_REX_MSG_DEVICE_REMOVED,          "Rex removed device, device %08x\n")
    ILOG_ENTRY(UPP_REX_MSG_SET_INTERFACE,           "Rex Set interface, device address %d config settings: %08x\n")
    ILOG_ENTRY(UPP_REX_MSG_ROUTE_CHANGE_DONE,       "Rex Route change done for device %d  config settings: %08x\n")

	ILOG_ENTRY(UPP_LEX_INVALID_BUFFER_ID_ENDPOINT,  "Invalid buffer when clearing endpoint %d device %d\n")

    ILOG_ENTRY(UPP_TRANSACTION_NOT_AVAILABLE,       "Transaction not available!\n")
    ILOG_ENTRY(UPP_TRANSACTION_IN_USE,              "Transaction already in use, Device Address %d\n")
    ILOG_ENTRY(UPP_TRANSACTION_DEVICE_NOT_FOUND,    "Get Free Transaction, Device not found, Device Address %d\n")
    ILOG_ENTRY(UPP_TRANSACTION_DOWNSTREAM_NOT_FOUND,"Downstream only Transaction not found, Device Address %d, line %d\n")
    ILOG_ENTRY(UPP_NO_FREE_TRANSACTIONS,            "No free transactions to allocate\n")
    ILOG_ENTRY(UPP_ILLEGAL_FREE_TRANSACTION,        "Freeing a transaction when none allocated!\n")
    ILOG_ENTRY(UPP_ILLEGAL_FREE_TRANSACTION_INDEX,  "Illegal free transaction from bad index %d previous %d\n")
    ILOG_ENTRY(UPP_FREE_ABANDONED_TRANSACTION,      "Freeing abandoned transaction, device address %d location %8x\n")
    ILOG_ENTRY(UPP_CORRUPT_OPEN_LIST,               "Corrupt Open list index %d line %d\n")
    ILOG_ENTRY(UPP_ILLEGAL_TRANSACTION_INDEX,       "Index %d does not match transaction given line %d\n")
    ILOG_ENTRY(UPP_TRANSACTION_WATERMARK_CHANGE,    "UPP opentransactionHighWaterMark: %d\n")
    ILOG_ENTRY(UPP_TRANSACTION_HOST_DUPLICATE_SEQ,  "**Host duplicate seq number %d device %02d time stamp %d skipping rest of packet\n")
    ILOG_ENTRY(UPP_TRANSACTION_DEVICE_DUPLICATE_SEQ,"**Device duplicate seq number %d device %02d time stamp %d skipping rest of packet\n")
    ILOG_ENTRY(UPP_TRANSACTION_ILLEGAL_HOST_PACKET, "Host packet ILLEGAL!  Device %d multiple setup packet received, seq %d timestamp %d\n")
    ILOG_ENTRY(UPP_TRANSACTION_ILLEGAL_SETUP_PACKET,"ILLEGAL setup packet! Device %d, transaction type %d, sequence %d\n")
    ILOG_ENTRY(UPP_TRANSACTION_SET_ADDRESS,         "Set address, new address = %d route string = %x timeStamp %d\n")
    ILOG_ENTRY(UPP_TRANSACTION_GET_CONFIGURATION,   "Get Configuration, device address %d route %x timeStamp %d\n")

//  Devices & Topology
    ILOG_ENTRY(UPP_MAX_DEVICE_OVER,                 "Can't allocated new device. No more available slot\n")
    ILOG_ENTRY(UPP_MAX_ENDPOINT_OVER,               "Can't allocated new endpoint. No more available slot\n")
    ILOG_ENTRY(UPP_ADD_SAME_DEVICE,                 "The device address(%d) and route (0x%8x)already exist\n")
    ILOG_ENTRY(UPP_ADD_ADDRESS_EXIST,               "The device address(%d) is already exist, Can't add it!\n")
    ILOG_ENTRY(UPP_ADD_ROUTE_EXIST,                 "The device route string(0x%8x) is already in use by device %d!\n")
    ILOG_ENTRY(UPP_NULL_DEVICE,                     "Device pointer is NULL! address %d at line %d\n")
    ILOG_ENTRY(UPP_DEVICE_ALREADY_PENDING,          "Device add already pending %08x\n")
    ILOG_ENTRY(UPP_WRONG_ENDPOINT,                  "Endpoint index (%d) is wrong\n")
    ILOG_ENTRY(UPP_FOUND_REMOVE_DEVICE,             "Found the device to be removed: addr(%d), route(0x%x), mem address(0x%X) \n")
    ILOG_ENTRY(UPP_DEVICE_ADD,                      "Add device request: dev addr(%d), route(0x%x), mem address(0x%X) \n")
    ILOG_ENTRY(UPP_DEVICE_REMOVE,                   "Remove device: dev addr(%d), route(0x%x), mem address(0x%X) \n")
    ILOG_ENTRY(UPP_DEVICE_FREE,                     "Free device: dev addr(%d), route(0x%x), mem address(0x%X) \n")
    ILOG_ENTRY(UPP_INVALID_DEVICE_FREE,             "Free device %08x called with unexpected device pointer %08x\n")
    ILOG_ENTRY(UPP_DEVICE_LOCATION,                 "\n\n                             Device Address %3d. Route Path: 0x%08x. ======================\n")
    ILOG_ENTRY(UPP_DEVICE_ENDPOINT,                 "    Endpoint: Number:%2d, type %d            Route:0x%08x, \n")
    ILOG_ENTRY(UPP_DEVICE_ENDPOINT_DETAIL,          "              Active|Set|Clear|Dir: 0x%04x Max Burst: %d bInterval %d\n")
    ILOG_ENTRY(UPP_ENDPOINT_NEXT,                   "              Current:0x%08x, Next endpoint 0x%08x, Prev endpoint 0x%08x\n")
    ILOG_ENTRY(UPP_ENDPOINT_FREE,                   "Free endpoint: Number:%d, Type:%d route:0x%08x\n")
    ILOG_ENTRY(UPP_DEVICE_NEXT,                     "Next device 0x%x, Prev device 0x%x\n")
    ILOG_ENTRY(UPP_DEVICE_ADDR,                     "Allocated Memory for address %d is 0x%08X\n")
    ILOG_ENTRY(UPP_NUM_DEVICES,                     "\n             Number of allocated devices:%d, free devices:%d\n")
    ILOG_ENTRY(UPP_NUM_ENDPOINTS,                   "               Number of allocated endpoints:%d, free endpoints:%d\n\n")
    ILOG_ENTRY(UPP_NEW_ENDPOINT,                    "New endpoint: route 0x%x, number %d, type %d\n")
    ILOG_ENTRY(UPP_DEVICE_SET_INTERFACE,            "Set Interface: Device %d value %08x\n")
    ILOG_ENTRY(UPP_DEVICE_SET_CONFIFGURATION,       "Set Configuration: Device %d Route 0x%08x Config value: %d\n")
    ILOG_ENTRY(UPP_SAME_ACTIVE_INTERFACE,           "Active interface is the same with before. Route: 0x%x endpoint %08x\n")
    ILOG_ENTRY(UPP_NO_ENDPOINTS_FOR_INTERFACE,      "No endpoints found for the specified interface Route: 0x%08x\n")
    ILOG_ENTRY(UPP_ACTIVE_INTERFACE,                "Active interface callback. Endpoint Route: 0x%x, Number:%d\n")
    ILOG_ENTRY(UPP_INACTIVE_INTERFACE,              "Inactive interface callback. Endpoint Route: 0x%x, Number:%d\n")
    ILOG_ENTRY(UPP_NO_FREE_INTERFACES,              "No free interfaces available. Interface Route: 0x%x\n")
    ILOG_ENTRY(UPP_INTERFACE_LINK_ERROR,            "Error when adding interface to active list. Interface Route: 0x%x\n")
    ILOG_ENTRY(UPP_NO_ENDPOINTS_MARKED_CLEAR,       "No endpoints found that were marked to be cleared\n")
    ILOG_ENTRY(UPP_NO_ENDPOINTS_MARKED_SET,         "No endpoints found that were marked to be set\n")

    //Stat logs
    ILOG_ENTRY(UPP_ID_MGR_FIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR,        "STAT:UPP upp->id_mgr_fifo->write_engine->stats0->pkt_max_byte_cnt_err: %d\n")
    ILOG_ENTRY(UPP_ID_MGR_FIFO_WRENG_STATS0_FIFO_FULL_ERR,               "STAT:UPP upp->id_mgr_fifo->write_engine->stats0->fifo_full_err: %d\n")
    ILOG_ENTRY(UPP_ID_MGR_FIFO_WRENG_STATS0_PKT_ERR,                     "STAT:UPP upp->id_mgr_fifo->write_engine->stats0->pkt_err: %d\n")
    ILOG_ENTRY(UPP_ID_MGR_FIFO_WRENG_STATS0_PKT_SOP_ERR,                 "STAT:UPP upp->id_mgr_fifo->write_engine->stats0->pkt_sop_err: %d\n")
    ILOG_ENTRY(UPP_ID_MGR_FIFO_WRENG_STATS0_DRP_PKT_RD,                  "STAT:UPP upp->id_mgr_fifo->write_engine->stats0->drp_pkt_rd: %d\n")
    ILOG_ENTRY(UPP_ID_MGR_FIFO_WRENG_STATS0_DRP_PKT_WR,                  "STAT:UPP upp->id_mgr_fifo->write_engine->stats0->drp_pkt_wr: %d\n")
    ILOG_ENTRY(UPP_ID_MGR_FIFO_WRENG_STATS0_DRP_PKT,                     "STAT:UPP upp->id_mgr_fifo->read_engine->stats0->drp_pkt: %d\n")
    ILOG_ENTRY(UPP_ISO_REX_FIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR,       "STAT:UPP upp->iso_rex_fifo->write_engine->stats0->pkt_max_byte_cnt_err: %d\n")
    ILOG_ENTRY(UPP_ISO_REX_FIFO_WRENG_STATS0_FIFO_FULL_ERR,              "STAT:UPP upp->iso_rex_fifo->write_engine->stats0->fifo_full_err: %d\n")
    ILOG_ENTRY(UPP_ISO_REX_FIFO_WRENG_STATS0_PKT_ERR,                    "STAT:UPP upp->iso_rex_fifo->write_engine->stats0->pkt_err: %d\n")
    ILOG_ENTRY(UPP_ISO_REX_FIFO_WRENG_STATS0_PKT_SOP_ERR,                "STAT:UPP upp->iso_rex_fifo->write_engine->stats0->pkt_sop_err: %d\n")
    ILOG_ENTRY(UPP_ISO_REX_FIFO_WRENG_STATS0_DRP_PKT_RD,                 "STAT:UPP upp->iso_rex_fifo->write_engine->stats0->drp_pkt_rd: %d\n")
    ILOG_ENTRY(UPP_ISO_REX_FIFO_WRENG_STATS0_DRP_PKT_WR,                 "STAT:UPP upp->iso_rex_fifo->write_engine->stats0->drp_pkt_wr: %d\n")
    ILOG_ENTRY(UPP_ISO_REX_FIFO_WRENG_STATS0_DRP_PKT,                    "STAT:UPP upp->iso_rex_fifo->read_engine->stats0->drp_pkt: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR, "STAT:UPP upp->ctrl_trfr_h2d_fifo->write_engine->stats0->pkt_max_byte_cnt_err: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_FIFO_FULL_ERR,        "STAT:UPP upp->ctrl_trfr_h2d_fifo->write_engine->stats0->fifo_full_err: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_PKT_ERR,              "STAT:UPP upp->ctrl_trfr_h2d_fifo->write_engine->stats0->pkt_err: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_PKT_SOP_ERR,          "STAT:UPP upp->ctrl_trfr_h2d_fifo->write_engine->stats0->pkt_sop_err: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_DRP_PKT_RD,           "STAT:UPP upp->ctrl_trfr_h2d_fifo->write_engine->stats0->drp_pkt_rd: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_DRP_PKT_WR,           "STAT:UPP upp->ctrl_trfr_h2d_fifo->write_engine->stats0->drp_pkt_wr: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_DRP_PKT,              "STAT:UPP upp->ctrl_trfr_h2d_fifo->read_engine->stats0->drp_pkt: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_NFIFO_DCOUNT,         "STAT:UPP upp->ctrl_trfr_h2d_fifo->write_engine->stats0->nfifo_dcount: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_H2D_FIFO_WRENG_STATS0_PFIFO_DCOUNT,         "STAT:UPP upp->ctrl_trfr_h2d_fifo->write_engine->stats0->pfifo_dcount: %d\n")    
    ILOG_ENTRY(UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_PKT_MAX_BYTE_CNT_ERR, "STAT:UPP upp->ctrl_trfr_d2h_fifo->write_engine->stats0->pkt_max_byte_cnt_err: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_FIFO_FULL_ERR,        "STAT:UPP upp->ctrl_trfr_d2h_fifo->write_engine->stats0->fifo_full_err: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_PKT_ERR,              "STAT:UPP upp->ctrl_trfr_d2h_fifo->write_engine->stats0->pkt_err: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_PKT_SOP_ERR,          "STAT:UPP upp->ctrl_trfr_d2h_fifo->write_engine->stats0->pkt_sop_err: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_DRP_PKT_RD,           "STAT:UPP upp->ctrl_trfr_d2h_fifo->write_engine->stats0->drp_pkt_rd: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_DRP_PKT_WR,           "STAT:UPP upp->ctrl_trfr_d2h_fifo->write_engine->stats0->drp_pkt_wr: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_DRP_PKT,              "STAT:UPP upp->ctrl_trfr_d2h_fifo->read_engine->stats0->drp_pkt: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_NFIFO_DCOUNT,         "STAT:UPP upp->ctrl_trfr_d2h_fifo->write_engine->stats0->nfifo_dcount: %d\n")
    ILOG_ENTRY(UPP_CTRL_TRFR_D2H_FIFO_WRENG_STATS0_PFIFO_DCOUNT,         "STAT:UPP upp->ctrl_trfr_d2h_fifo->write_engine->stats0->pfifo_dcount: %d\n")

    // queue management state machine logs
    ILOG_ENTRY(UPP_QUEUE_STATE_TRANSITION,          "Queue manager: old state = %d, new state = %d on event = %d\n")
    ILOG_ENTRY(UPP_QUEUE_INVALID_EVENT,             "Queue manager: got invalid event %d in state %d\n")
    ILOG_ENTRY(UPP_QUEUE_SPURIOUS_INTERRUPT_EVENT,  "Queue manager: spurious interrupt received %x\n")

    ILOG_ENTRY(UPP_ENDPOINT_ENTRY_SET,              "Set Endpoint device %d endpoint %d buffer ID %d\n")
    ILOG_ENTRY(UPP_ENDPOINT_ENTRY_CLEARED,          "Clear Endpoint device %d endpoint %d buffer ID %d\n")
    ILOG_ENTRY(UPP_NO_FREE_ISO_BUFFERS,               "No more accelerated queues available!\n")

    ILOG_ENTRY(UPP_ICMD_ENABLED,                    "UPP is enabled\n")
    ILOG_ENTRY(UPP_ICMD_DISABLED,                   "UPP is disabled\n")
    ILOG_ENTRY(UPP_ICMD_CONTROL_TRANSFER_ENABLED,   "UPP control transfer is enabled\n")
    ILOG_ENTRY(UPP_ICMD_CONTROL_TRANSFER_DISABLED,  "UPP control transfer is disabled\n")
    ILOG_ENTRY(IS_UPP_ISO_ENABLE_STATUS,            "UPP ISO enabled (0 = disabled) = %d \n")
    ILOG_ENTRY(IS_UPP_CONTROL_TRANSFER_ENABLED,     "UPP control transfer = %d\n")  


ILOG_END(UPP_COMPONENT, ILOG_MINOR_EVENT)

// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // ULP_LOG_H
