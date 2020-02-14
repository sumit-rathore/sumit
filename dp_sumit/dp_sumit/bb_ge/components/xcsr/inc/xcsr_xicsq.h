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
//!   @file  - xcsr_xicsq.h
//
//!   @brief - XICSQ driver code for queue manipulation. Part of the XCSR module
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef XCSR_XICSQ_H
#define XCSR_XICSQ_H

/***************************** Included Headers ******************************/
#include <itypes.h>
#include <ilog.h> // For XCSR_XICSDumpFrame
#include <xcsr_xsst.h> //for XUSB_AddressT
#include "xusb_fw.h"    // For RTL Q definitions

/************************ Defined Constants and Macros ***********************/

// Allocate a variable length queue frame on the stack with a data capacity equal to the supplied
// value.
#define XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(_capacity_, _assignee_)                       \
    (_assignee_) = alloca(offsetof(struct XCSR_XICSQueueFrame, data) + _capacity_);     \
    (_assignee_)->dataSize = 0;                                                         \
    (_assignee_)->dataCapacity = (_capacity_);

// Flags used with the XCSR_QueueWriteRawData function
#define XCSR_WFLAGS_SOF (1 << 0) // Start Of Frame
#define XCSR_WFLAGS_SOP (1 << 1) // Start Of Packet
#define XCSR_WFLAGS_EOP (1 << 2) // End Of Packet
#define XCSR_WFLAGS_EOF (1 << 3) // End Of Frame

// Need a dummy entry to ensure enum gets a large enough type associated with it
enum XCSR_Queue { _XCSR_QUEUE_DUMMY_ENTRY };

#define MAX_QIDS                ((enum XCSR_Queue)(128))

//-----------------------------------------------------------------------------------------------
// IMPORTANT:
//
// As static queues are added, removed or moved, the LexStaticQueues or RexStaticQueues arrays in
// xcsr_xusb.c must also be updated.
//-----------------------------------------------------------------------------------------------

#define LEX_FIRST_QID           ((enum XCSR_Queue)(SQL_SOF))
#define REX_FIRST_QID           ((enum XCSR_Queue)(SQ_ASYNC))


// LEX/REX Static Queues
#define LEX_REX_SQ_ASYNC        ((enum XCSR_Queue)(SQ_ASYNC))              //Lex downstream async, Rex upstream async
#define LEX_REX_SQ_PERIODIC     ((enum XCSR_Queue)(SQ_PERIODIC))           //Lex downstream periodic, Rex upstream periodic
#define LEX_REX_SQ_ACC_BLK      ((enum XCSR_Queue)(SQ_ACC_BLK))            //Lex downstream accelerated bulk out, rex upstream accelerated bulk in response
#define LEX_REX_SQ_CPU_TX_USB   ((enum XCSR_Queue)(SQ_CPU_TX_USB))         //Lex CPU downstream XUSB Tx, Rex CPU upstream XUSB Tx
#define LEX_REX_SQ_CPU_TX       ((enum XCSR_Queue)(SQ_CPU_TX))             //CPU Tx
#define LEX_REX_SQ_CPU_RX       ((enum XCSR_Queue)(SQ_CPU_RX))             //CPU Rx
#define LEX_REX_SQ_CPU_TX_ENET  ((enum XCSR_Queue)(SQ_CPU_TX_ENET))        //CPU Tx - raw ethernet
#define LEX_REX_SQ_CPU_RX_ENET  ((enum XCSR_Queue)(SQ_CPU_RX_ENET))        //CPU Rx - raw ethernet
#define LEX_REX_SQ_QOS          ((enum XCSR_Queue)(SQ_QOS))                //Quality of service

// LEX static queues
#define LEX_SQ_SOF              ((enum XCSR_Queue)(SQL_SOF))               //SOF
#define LEX_SQ_CPU_USB_CTRL     ((enum XCSR_Queue)(SQL_CPU_USB_CTRL))      //Lex CPU USB control
#define LEX_SQ_RETRY0           ((enum XCSR_Queue)(SQL_USB_RTY0))          //Lex retry 0
#define LEX_SQ_RETRY1           ((enum XCSR_Queue)(SQL_USB_RTY1))          //Lex retry 1
#define LEX_SQ_CPU_RSP_QID      ((enum XCSR_Queue)(SQL_CPU_RSP_QID))       //

#define LEX_NUMBER_STATIC_QUEUES (SQL_COUNT)
#define LEX_FIRST_DYNAMIC_QID   ((enum XCSR_Queue)(LEX_NUMBER_STATIC_QUEUES))

// REX Static Queues
#define REX_SQ_CPU_DEV_RESP     ((enum XCSR_Queue)(SQR_CPU_DEV_RESP))      //Rex device response
#define REX_SQ_HSCH             ((enum XCSR_Queue)(SQR_HSCH))              //Inbound hard scheduled
#define REX_SQ_SCH_ASYN_INB     ((enum XCSR_Queue)(SQR_SCH_ASYNC_INB))     //Inbound soft scheduled async
#define REX_SQ_SCH_PERIODIC     ((enum XCSR_Queue)(SQR_SCH_PERIODIC))      //Inbound soft scheduled periodic
#define REX_SQ_SCH_MSA          ((enum XCSR_Queue)(SQR_SCH_MSA))           //Inbound soft scheduled MSA
#define REX_SQ_SCH_ASYNC_OTB    ((enum XCSR_Queue)(SQR_SCH_ASYNC_OTB))     //Outbound soft scheduled async
#define REX_SQ_SCH_UFRM_P0      ((enum XCSR_Queue)(SQR_SCH_UFRM_P0))       //Outbound soft scheduled periodic high priority 0 // Scheduled every even # microframe
#define REX_SQ_SCH_UFRM_P1      ((enum XCSR_Queue)(SQR_SCH_UFRM_P1))       //Outbound soft scheduled periodic high priority 1 // Scheduled every odd # microframe
#define REX_SQ_SCH_UFRM0        ((enum XCSR_Queue)(SQR_SCH_UFRM00))        //Outbound soft scheduled periodic uFrame 0.0
#define REX_SQ_SCH_UFRM1        ((enum XCSR_Queue)(SQR_SCH_UFRM01))        //Outbound soft scheduled periodic uFrame 0.1
#define REX_SQ_SCH_UFRM2        ((enum XCSR_Queue)(SQR_SCH_UFRM02))        //Outbound soft scheduled periodic uFrame 0.2
#define REX_SQ_SCH_UFRM3        ((enum XCSR_Queue)(SQR_SCH_UFRM03))        //Outbound soft scheduled periodic uFrame 0.3
#define REX_SQ_SCH_UFRM4        ((enum XCSR_Queue)(SQR_SCH_UFRM04))        //Outbound soft scheduled periodic uFrame 0.4
#define REX_SQ_SCH_UFRM5        ((enum XCSR_Queue)(SQR_SCH_UFRM05))        //Outbound soft scheduled periodic uFrame 0.5
#define REX_SQ_SCH_UFRM6        ((enum XCSR_Queue)(SQR_SCH_UFRM06))        //Outbound soft scheduled periodic uFrame 0.6
#define REX_SQ_SCH_UFRM7        ((enum XCSR_Queue)(SQR_SCH_UFRM07))        //Outbound soft scheduled periodic uFrame 0.7
#define REX_SQ_SCH_UFRM10       ((enum XCSR_Queue)(SQR_SCH_UFRM10))        //Outbound soft scheduled periodic uFrame 1.0
#define REX_SQ_SCH_UFRM11       ((enum XCSR_Queue)(SQR_SCH_UFRM11))        //Outbound soft scheduled periodic uFrame 1.1
#define REX_SQ_SCH_UFRM12       ((enum XCSR_Queue)(SQR_SCH_UFRM12))        //Outbound soft scheduled periodic uFrame 1.2
#define REX_SQ_SCH_UFRM13       ((enum XCSR_Queue)(SQR_SCH_UFRM13))        //Outbound soft scheduled periodic uFrame 1.3
#define REX_SQ_SCH_UFRM14       ((enum XCSR_Queue)(SQR_SCH_UFRM14))        //Outbound soft scheduled periodic uFrame 1.4
#define REX_SQ_SCH_UFRM15       ((enum XCSR_Queue)(SQR_SCH_UFRM15))        //Outbound soft scheduled periodic uFrame 1.5
#define REX_SQ_SCH_UFRM16       ((enum XCSR_Queue)(SQR_SCH_UFRM16))        //Outbound soft scheduled periodic uFrame 1.6
#define REX_SQ_SCH_UFRM17       ((enum XCSR_Queue)(SQR_SCH_UFRM17))        //Outbound soft scheduled periodic uFrame 1.7
#define REX_SQ_MSA_RETRY        ((enum XCSR_Queue)(SQR_MSA_RETRY))         //Mass Storage Packet Retry queue
#define REX_SQ_DEV_RESP_DATA    ((enum XCSR_Queue)(SQR_CPU_DEV_RESP_DATA)) //Device Response Data Queue
#define REX_NUMBER_STATIC_QUEUES (SQR_COUNT)
#define REX_FIRST_DYNAMIC_QID   ((enum XCSR_Queue)(REX_NUMBER_STATIC_QUEUES))

// Queue Frame member size constants - There is one type for USB control packets and another for
// ethernet frames.

#define XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_CONTENTS_SIZE 64
// 1 byte PID + 64 bytes data + 2 bytes for CRC
#define XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_DATA_SIZE     (1 + XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_CONTENTS_SIZE + 2)
// Added to pad data out to 64-bit boundary
#define XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE          (ALIGNED_SIZE(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_DATA_SIZE, 8))

// 14 bytes for ethernet + 20 bytes for IP + 4 bytes for UDP + 576 bytes for the largest UDP
// payload required (DHCP) + an extra 4 bytes to account for the fact that on receive, the buffer
// contains the ethernet CRC.
#define XCSR_QUEUE_FRAME_ETHERNET_PACKET_DATA_SIZE      (14 + 20 + 4 + 576 + 4)
#define XCSR_QUEUE_FRAME_ETHERNET_PACKET_SIZE           (ALIGNED_SIZE(XCSR_QUEUE_FRAME_ETHERNET_PACKET_DATA_SIZE, 8))

// The defines below specify bit fields within the XCSR_XICSQueueCleanupFlagsT type
//
// Perform a deallocation
#define XCSR_XICSQ_DEALLOC_OFFSET       (0)
#define XCSR_XICSQ_DEALLOC_FLAG         (1 << XCSR_XICSQ_DEALLOC_OFFSET)
// Perform a flush
#define XCSR_XICSQ_FLUSH_OFFSET         (1)
#define XCSR_XICSQ_FLUSH_FLAG           (1 << XCSR_XICSQ_FLUSH_OFFSET)
// Prevent deallocation of a non-empty queue and prevent flushing of an empty
// queue
#define XCSR_XICSQ_EMPTY_CHECK_OFFSET   (2)
#define XCSR_XICSQ_EMPTY_CHECK_FLAG     (1 << XCSR_XICSQ_EMPTY_CHECK_OFFSET)
// Prevent deallocation of an unallocated queue and prevent flushing of an
// unallocated queue
#define XCSR_XICSQ_ALLOC_CHECK_OFFSET   (3)
#define XCSR_XICSQ_ALLOC_CHECK_FLAG     (1 << XCSR_XICSQ_ALLOC_CHECK_OFFSET)

// CtrlLinkType and CtrlLinkSubType frame fields are packed into 3 bits
#define MAX_NUM_CTRL_LINK_TYPE_UNIQUE_VALUES    (8)
#define MAX_NUM_CTRL_LINK_SUBTYPE_UNIQUE_VALUES (8)

/******************************** Data Types *********************************/

typedef uint8 XCSR_XICSQueueCleanupFlagsT;

enum XCSR_XUSBAction
{
    XUSB_IN = 0,    // 0
    XUSB_OUT,       // 1
    XUSB_SETUP,     // 2
    XUSB_PING,      // 3
};

enum XCSR_XUSBResponseId
{
    XUSB_NULL = 0,  // 0
    XUSB_ACK,       // 1
    XUSB_NAK,       // 2
    XUSB_STALL,     // 3
    XUSB_NYET,      // 4
    XUSB_ERR_RESP,  // 5
    XUSB_TIMEOUT,   // 6
    XUSB_3K,        // 7
    XUSB_DATA0,     // 8
    XUSB_DATA1,     // 9
    XUSB_DATA2,     // 10
    XUSB_MDATA,     // 11
    UNDEFINED_RESPONSE_ID
};

// Accel
#define FH_ACCEL_NOT          0
#define FH_ACCEL_MSA_CBW      1
#define FH_ACCEL_MSA_DATA     2
#define FH_ACCEL_MSA_CSW      3

// Control link sub type for frame header
typedef enum {
    LINK_MESSAGE,
    USB_DOWNSTREAM_STATE_CHANGE,
    USB_UPSTREAM_STATE_CHANGE,
    SOFTWARE_MESSAGE_LEX2REX,
    NUMBER_OF_UNIQUE_CONTROL_LINK_TYPES
    // This value is packed into 3 bits, so allow at most 8 unique values
} XCSR_CtrlLinkTypeT;

    // link
typedef enum {                  // Message Parameters: (uint32 data, uint64 extraData)
    // These messages have to do with negotiating a VPort assignment for a REX
    LEX_SEEKS_PAIR,             // (LEX_MAC_ADDR OUI, LEX_MAC_ADDR : _)
    REX_PAIR_RESPONSE,          // (REX_MAC_ADDR OUI, LEX_MAC_ADDR : REX_MAC_ADDR)
    LEX_VPORT_ASSIGN,           // (VPORT, LEX_MAC_ADDR : REX_MAC_ADDR)
    // These messages have to do with checking compatibility right as a link is established
    LEX_COMPATIBILITY_QUERY,    // (LEX_FW_MAJOR : LEX_FW_MINOR : LEX_FW_REVISION, LEX_CAPABILITIES)
    REX_VETO_CONNECTION = LEX_COMPATIBILITY_QUERY, // Re-use LEX_COMPATIBILITY_QUERY as a REX->LEX
                                                   // message indicating that the REX vetoed the
                                                   // connection.  This message was introduced
                                                   // after N05, so it shouldn't be sent to N05 or
                                                   // earlier units. (_, _)
    REX_COMPATIBILITY_RESPONSE, // (REX_FW_MAJOR : REX_FW_MINOR : REX_FW_REVISION, REX_CAPABILITIES)
    LEX_LINK_CONFIRMATION,      // (_, _)
    LEX_BRAND_QUERY,            // (UNUSED : LEX_BRAND, _)
    REX_BRAND_RESPONSE,         // (UNUSED : REX_BRAND, _)
    NUMBER_OF_UNIQUE_CONTROL_LINK_MESSAGES
    // This value is packed into 3 bits, so allow at most 8 unique values
} XCSR_CtrlLinkMessageT;


// USB state event // Note: We try and keep this in sync with LG1
typedef enum {
    // Rex to Lex msgs
    DOWNSTREAM_CONNECT_HS,      // High speed device has connected
    DOWNSTREAM_CONNECT_FS,      // Full speed device has connected
    DOWNSTREAM_CONNECT_LS,      // Low speed device has connected
    DOWNSTREAM_DISCONNECT,
    DOWNSTREAM_REMOTE_WAKEUP,   // This is when the device is trying to wakeup the host from suspend
    NUMBER_OF_UNIQUE_CONTROL_DOWNSTREAM_STATE_CHANGE_MESSAGES
     // This value is packed into 3 bits, so allow at most 8 unique values
} XCSR_CtrlDownstreamStateChangeMessageT;


    // USB state event // Note: We try and keep this in sync with LG1
typedef enum {
    // Lex to Rex msgs
    UPSTREAM_SUSPEND,
    UPSTREAM_RESUME,
    UPSTREAM_RESUME_DONE,       // The host has finished its resume sequencing after a suspend
    UPSTREAM_DISCONNECT,
    UPSTREAM_BUS_RESET_LS,
    UPSTREAM_BUS_RESET_FS,
    UPSTREAM_BUS_RESET_HS,
    UPSTREAM_BUS_RESET_DONE,    // The host has finished its bus reset sequencing
    NUMBER_OF_UNIQUE_CONTROL_UPSTREAM_STATE_CHANGE_MESSAGES
    // This value is packed into 3 bits, so allow at most 8 unique values
} XCSR_CtrlUpstreamStateChangeMessageT;

    // Purely software-related messages
    // Message Parameter: (uint32 data)
typedef enum {
    LEX_FREED_MSA_PAIR,     // Parameter: (USB address of MSA device)
    LEX_LINK_UP_PROBE,      // Parameters: None
    NUMBER_OF_UNIQUE_CONTROL_SOFTWARE_MESSAGES
    // This value is packed into 3 bits, so allow at most 8 unique values
} XCSR_CtrlSwMessageT;

enum XICS_TagType
{
    SOF = 0,                             // 0
    DOWNSTREAM_XUSB_ASYNC,               // 1
    DOWNSTREAM_XUSB_SPLIT_AND_PERIODIC,  // 2
    UPSTREAM_XUSB,                       // 3
    FLOW_CONTROL,                        // 4
    DIAGNOSTIC,                          // 5
    CONTROL_LINK,                        // 6
    MEDIA,                               // 7
    INTER_CHIP_COMM,                     // 8
    ETHERNET,                            // 9
    REX_MSA                              // 10
};

enum XICS_FrameStructure
{
    SINGLE_HEADER_NO_DATA = 0,  // 0
    DOUBLE_HEADER_NO_DATA,      // 1
    SINGLE_HEADER_WITH_DATA,    // 2
    DOUBLE_HEADER_WITH_DATA     // 3
};

// CPU to CPU messages
union cpuMsgT {
    struct {
        // Generic fields
        uint64  tagType:          4;  // set to 6
        uint64  Vport:            4;  //
        uint64  unused:           4;  // See generic field for bitfield descriptions
        uint64  frmStruct:        2;  //
        uint64  retryMode:        1;  //
        uint64  retryEn:          1;  //
        uint64  unused2:          1;  //
        uint64  dataQid:          7;  //

        // Control link frame header specific fields
        uint64  cmdSeq:           2;  // Control link command sequence number, increment on every command // TODO: use this
        uint64  ctrlLinkType:     3;  // Type of CPU message // TODO: update excel spreadsheet with the message types
        uint64  msg:              3;  // Specific message for this ctrlLinkType
        uint64  data:            24;  // Data, to be used as needed
        uint64  crc8:             8;  // CRC8
    };
    uint64  rawData;
};


// Note the following structure has users that all depend on endian
struct XCSR_XICSQueueFrame
{
    struct
    {
        union {
            struct {
                uint64  tagType:          4;  // Tag type identifier
                uint64  Vport:            4;  // Virtual port
                uint64  unused:           4;
                uint64  frmStruct:        2;  // Frame structure -- msb is set if there is data in frame, lsb is set if this is a double header
                uint64  retryMode:        1;  // Retry mode select -- 00 to pass error'd data, 01 to flush error'd data
                uint64  retryEn:          1;  // Retry enable -- set to retry
                uint64  unused2:          1;
                uint64  dataQid:          7;  // Qid where data is stored if not enough space in packet
                uint64  unused1:          40;
            } generic;
            union cpuMsgT cpu_cpu; // Control link frame header
            struct {
                // Generic fields
                uint64 tagType:           4;  // Set to 1 or 2
                uint64 vPort:             4;  //
                uint64 unused:            4;  // See generic field above for bit field descriptions
                uint64 frmStruct:         2;  //
                uint64 retryMode:         1;  //
                uint64 retryEn:           1;  //
                uint64 unused2:           1;  //
                uint64 dataQid:           7;  //

                // Downstream frame header specific fields
                uint64 unused3:           8;
                uint64 rexqueue:          3;  // Rex queue used for handling this command
                uint64 accel:             3;  // Acceleration method of downstream command
                uint64 modifier:          2;  // Modifier that applies to downstream command
                uint64 action:            2;  // Type of downstream command performed
                uint64 EPType:            2;  // Endpoint type
                uint64 endpoint:          4;  // Endpoint to which downstream command was directed
                uint64 toggle:            1;  // Used to detect CTRL endpoint setup/response sequence errors
                uint64 address:           7;  // USB address
                uint64 crc8:              8;  // CRC8
            } downstream;
            struct {
                uint64 tagType:           4;  // Set to 3
                uint64 vPort:             4;  //
                uint64 unused:            4;  // See generic field above for bit field descriptions
                uint64 frmStruct:         2;  //
                uint64 retryMode:         1;  //
                uint64 retryEn:           1;  //
                uint64 unused2:           1;  //
                uint64 dataQid:           7;  //

                // Upstream frame header specific fields
                uint64 unused3:           8;
                uint64 response:          4;  // Response
                uint64 FLCClass:          2;  // FLC types: Null, Async, Periodic, MSA
                uint64 modifier:          2;  // Modifier that applies to downstream command
                uint64 action:            2;  // Type of downstream command performed
                uint64 EPType:            2;  // Endpoint type
                uint64 endpoint:          4;  // Endpoint to which downstream command was directed
                uint64 toggle:            1;  // Used to detect CTRL endpoint setup/response sequence errors
                uint64 address:           7;  // USB address
                uint64 crc8:              8;  // CRC8
            } upstream;
            uint8 byte[8];
            uint16 U16[4];
            uint32 word[2];
            uint64 dword;
        } one;

        //
        union {
            // Downstream (split and periodic) part B
            struct {
                uint64 EU:                1;  // End or Unused depending on context
                uint64 hubAddress:        7;  // Parent hub address
                uint64 S:                 1;  // Start or Speed depending on context; For speed, Full = 0, Low = 1
                uint64 portAddress:       7;  // Port on parent hub
                uint64 unused:            26;
                uint64 frameNumber:       11; // Frame number
                uint64 uFrame:            3;  // Microframe number
                uint64 crc8:              8;  // CRC8
            } downstreamSplitPeriodic;
            uint8 byte[8];
            uint16 U16[4];
            uint32 word[2];
            uint64 dword[1];
        } two;
    } header;

    // An XCSR_XICSQueueFrame struct has a variable length data member.  The dataSize member
    // specifies how many bytes of data are in data.  The dataCapacity member specifies the how big
    // the data array is in bytes.  The data member is aligned to a 4-byte boundary so that it can
    // be casted into a uint32 array to allow word access when reading and writing to the cache.
    uint16 dataSize;
    uint16 dataCapacity;
    uint8 data[] __attribute__((aligned(4)));
};

// Clients of USB control packets may cast the .data field of a XCSR_XICSQueueFrame into an
// XCSR_XICSQueueFrameUsbData to access the internal details.  The GCC specific may_alias
// attribute is applied to this data type to prevent optimizations that depend on the aliasing
// rules.  For details, see http://gcc.gnu.org/onlinedocs/gcc/Type-Attributes.html#Type-Attributes
struct XCSR_XICSQueueFrameUsbData
{
    uint8 pid;
    uint8 contents[];
} __attribute__((__may_alias__));

typedef enum
{
    NO_ERROR,       // Read completed successfully, but did not reach a packet or frame boundary
    END_OF_FRAME,   // End of frame reached without error
    CORRUPT_DATA,   // Rd error
    CRC_ERROR,      // This may be misleading. CRC's are only valid on USB DATA0/DATA1 packets
    Q_TOO_LARGE,    // Too much data for this structure (ie struct XCSR_XICSQueueFrame -> data)
    END_OF_PACKET   // End of packet reached - lower priority than end of frame
} eXCSR_QueueReadStatusT;


struct XCSR_QueueStats
{
    uint32 data0;
    uint32 data1;
    uint32 stats;
    uint16 frameCount;
    uint16 wordCount;
    uint8 emptyStatus;
};


typedef enum // matches xusb_fw.h
{
    QT_DEFAULT = QT_DEF     // All other traffic // only term that is differnt FROM RTL
    //QT_DNS_ASYNC,     // Downstream asynchronous
    //QT_DNS_PERIODIC,  // Downstream periodic
    //QT_DNS_ACC_BULK,  // Downstream accelerated bulk
    //QT_UPS_ASYNC,     // Upstream asynchronous
    //QT_UPS_PERIODIC,  // Upstream periodic
    //QT_UPS_ACC_BULK,  // Upstream accelerated bulk
    //QT_RESERVED,      // DO NOT USE
} eXCSR_QueueTypeT;


/*********************************** API *************************************/
void XCSR_XICSSetFlowCtrlThresholds(void);

// qid operations (other than read & write)
uint8 XCSR_XICSQQueueAllocate(eXCSR_QueueTypeT qType) __attribute__ ((section(".ftext")));
void XCSR_XICSQueueCleanup(enum XCSR_Queue qid, XCSR_XICSQueueCleanupFlagsT flags) __attribute__ ((section(".ftext")));

static inline void XCSR_XICSQQueueDeallocate(enum XCSR_Queue qid);
void XCSR_XICSQQueueFlush(enum XCSR_Queue qid);
static inline void XCSR_XICSQQueueFlushWithoutEmptyCheck(enum XCSR_Queue qid);
static inline void XCSR_XICSQueueFlushAndDeallocate(enum XCSR_Queue qid);


// read qids
eXCSR_QueueReadStatusT XCSR_XICSQueueReadFrame(
    enum XCSR_Queue qid,
    struct XCSR_XICSQueueFrame* frameData) __attribute__ ((section (".ftext")));
eXCSR_QueueReadStatusT XCSR_XICSQueueReadPartialFrameHeader(
    enum XCSR_Queue qid,
    struct XCSR_XICSQueueFrame* frameData) __attribute__((section(".ftext")));
eXCSR_QueueReadStatusT XCSR_XICSQueueReadPartialFrame(
    enum XCSR_Queue qid,
    struct XCSR_XICSQueueFrame* frameData,
    uint8 max64BitCacheReads) __attribute__((section(".ftext")));
uint64 XCSR_XICSQueueSnoopHead(enum XCSR_Queue qid) __attribute__((section(".ftext")));
uint64 XCSR_XICSQueueSnoopHeadGetControlReg(enum XCSR_Queue qid, uint32 * controlRegOut) __attribute__((section(".ftext")));

// stats on qids
boolT XCSR_XICSQueueContainsCompleteFrame(enum XCSR_Queue qid) __attribute__ ((section (".ftext")));
boolT XCSR_XICSQueueIsEmpty(enum XCSR_Queue qid);
uint32 XCSR_XICSQueueGetFrameCount(enum XCSR_Queue qid);
void XCSR_XICSQueueGetStats(enum XCSR_Queue qid, struct XCSR_QueueStats *);

// Write qids
static inline void XCSR_XICSSendMessage(XCSR_CtrlLinkTypeT msgType, uint8 message, uint8 vport) __attribute__ ((always_inline));
static inline void XCSR_XICSSendMessageWithData(XCSR_CtrlLinkTypeT msgType, uint8 message, uint8 vport, uint32 data) __attribute__ ((always_inline));
static inline void XCSR_XICSSendMessageWithExtraData(XCSR_CtrlLinkTypeT msgType, uint8 message, uint8 vport, uint32 data, uint64 extraData) __attribute__ ((always_inline));
void XCSR_XICSWriteFrame(
    enum XCSR_Queue qid, const struct XCSR_XICSQueueFrame *) __attribute__ ((section (".ftext")));
void XCSR_XICSWritePartialFrameHeader(
    enum XCSR_Queue qid, const struct XCSR_XICSQueueFrame* frame) __attribute__((section(".ftext")));
void XCSR_XICSWritePartialFrame(
    enum XCSR_Queue qid,
    const struct XCSR_XICSQueueFrame* frame,
    uint16 startWordOffsetInData,
    uint16 numBytesToWrite) __attribute__((section(".ftext")));
void XCSR_QueueWriteRawData(
    const uint32* data,
    uint16 numBytes,
    enum XCSR_Queue qid,
    uint8 writeFlags) __attribute__((section(".ftext")));
void XCSR_QueueWriteTestModeFrame(enum XCSR_Queue qid);
void XCSR_XICSSendRexCSplit(
    XUSB_AddressT, uint8 endpoint, enum EndpointTransferType, enum EndpointDirection, uint8 hubAddress, uint8 port, enum UsbSpeed speed);
void XCSR_XICSSetRexSupportsSwMessages(uint8 vport, uint8 major, uint8 minor, uint8 rev);
boolT XCSR_XICSGetRexSupportsSwMessages(uint8 vport);

// Frame header operations
static inline uint8 XCSR_XICSGetFrameUSBAddr(const struct XCSR_XICSQueueFrame *);
static inline uint8 XCSR_XICSGetFrameEndpoint(const struct XCSR_XICSQueueFrame *);
static inline enum XCSR_XUSBAction XCSR_XICSGetFrameAction(const struct XCSR_XICSQueueFrame *);
static inline enum XCSR_XUSBResponseId XCSR_XICSGetFrameResponseId(const struct XCSR_XICSQueueFrame *);
static inline enum XICS_TagType XCSR_XICSGetTagType(const struct XCSR_XICSQueueFrame *);
static inline boolT XCSR_XUSBIsDownStreamInVFAck(const struct XCSR_XICSQueueFrame *);
static inline void XCSR_XICSBuildUpstreamFrameHeader(struct XCSR_XICSQueueFrame * x,
        XUSB_AddressT address, uint8 ep, uint8 mod, uint16 datasize, enum XCSR_XUSBAction action,
        enum XCSR_XUSBResponseId response, enum EndpointTransferType epType, boolT toggle);
static inline void XCSR_XICSBuildDownstreamFrameHeader(struct XCSR_XICSQueueFrame * x,
        uint8 usbAddress, uint8 ep, uint8 mod, enum XCSR_XUSBAction action, enum XCSR_Queue queue, enum EndpointTransferType epType, boolT toggle, uint8 qid);
// To assist in debugging
void XCSR_XICSDumpFrame(const struct XCSR_XICSQueueFrame * pFrameData, ilogLevelT logLevel);

/******************************* Internal API ********************************/
void _XCSR_XICSSendMessage() __attribute__ ((section (".ftext")));

/****************************** Static Inlines *******************************/
/* Note: there are interesting endian affects here between the documentation, byte population order of HW, etc. */
static inline uint8 XCSR_XICSGetFrameUSBAddr(const struct XCSR_XICSQueueFrame * x)    { return x->header.one.byte[6] & 0x7F; }
static inline uint8 XCSR_XICSGetFrameEndpoint(const struct XCSR_XICSQueueFrame * x)   { return x->header.one.byte[5] & 0xF; }
static inline enum XCSR_XUSBAction XCSR_XICSGetFrameAction(const struct XCSR_XICSQueueFrame * x)          { return x->header.one.byte[5] >> 6; }
static inline enum XCSR_XUSBResponseId XCSR_XICSGetFrameResponseId(const struct XCSR_XICSQueueFrame * x)  { return x->header.one.byte[4] >> 4; }
static inline enum XICS_TagType XCSR_XICSGetTagType(const struct XCSR_XICSQueueFrame * x) { return x->header.one.byte[0] >> 4; }
static inline boolT XCSR_XUSBIsDownStreamInVFAck(const struct XCSR_XICSQueueFrame * x) { return x->header.one.byte[7] >> 7; }

static inline void XCSR_XICSQQueueDeallocate(enum XCSR_Queue qid)
{ XCSR_XICSQueueCleanup(qid, XCSR_XICSQ_DEALLOC_FLAG | XCSR_XICSQ_ALLOC_CHECK_FLAG); }
//void XCSR_XICSQQueueFlush(enum XCSR_Queue qid)
//{ XCSR_XICSQueueCleanup(qid, XCSR_XICSQ_FLUSH_FLAG | XCSR_XICSQ_EMPTY_CHECK_FLAG); }
static inline void XCSR_XICSQQueueFlushWithoutEmptyCheck(enum XCSR_Queue qid)
{ XCSR_XICSQueueCleanup(qid, XCSR_XICSQ_FLUSH_FLAG); }
static inline void XCSR_XICSQueueFlushAndDeallocate(enum XCSR_Queue qid)
{
    XCSR_XICSQueueCleanup(
        qid,
        XCSR_XICSQ_FLUSH_FLAG | XCSR_XICSQ_ALLOC_CHECK_FLAG |
        XCSR_XICSQ_EMPTY_CHECK_FLAG | XCSR_XICSQ_DEALLOC_FLAG);
}

static inline void XCSR_XICSBuildUpstreamFrameHeader(struct XCSR_XICSQueueFrame * x,
        XUSB_AddressT address, uint8 ep, uint8 mod, uint16 datasize, enum XCSR_XUSBAction action, enum XCSR_XUSBResponseId response, enum EndpointTransferType epType, boolT toggle)
{
    x->header.one.upstream.tagType   = 0x3;   //upstream xusb
    x->header.one.upstream.vPort     = 0;
    x->header.one.upstream.unused    = 0;
    x->header.one.upstream.frmStruct = datasize? 0x2: 0x0;   //single header, with or without data
    x->header.one.upstream.retryMode = 0x1;   //confirmed delivery
    x->header.one.upstream.retryEn   = 0x1;
    x->header.one.upstream.unused2   = 0;
    x->header.one.upstream.dataQid   = 0;     //data not separated from frame, no data queue id needed
    x->header.one.upstream.unused3   = 0;
    x->header.one.upstream.response  = response;
    x->header.one.upstream.FLCClass  = 0;
    x->header.one.upstream.modifier  = mod;
    x->header.one.upstream.action    = action;
    x->header.one.upstream.EPType    = epType;
    x->header.one.upstream.endpoint  = ep;
    x->header.one.upstream.toggle    = toggle;
    x->header.one.upstream.address   = XCSR_getXUSBAddrUsb(address);
    x->header.one.upstream.crc8      = 0;
}

static inline void XCSR_XICSBuildDownstreamFrameHeader(struct XCSR_XICSQueueFrame * x,
        uint8 usbAddress, uint8 ep, uint8 mod, enum XCSR_XUSBAction action, enum XCSR_Queue queue, enum EndpointTransferType epType, boolT toggle, uint8 qid)
{
    x->header.one.downstream.tagType   = 0x1; // downstream XUSB (async)
    x->header.one.downstream.vPort     = 0;
    x->header.one.downstream.unused    = 0;
    x->header.one.downstream.frmStruct = 0; // single header, no data
    x->header.one.downstream.retryMode = 0x1; // confirmed delivery
    x->header.one.downstream.retryEn   = 0x1;
    x->header.one.downstream.unused2   = 0;
    x->header.one.downstream.dataQid   = qid;
    x->header.one.downstream.unused3   = 0;
    x->header.one.downstream.rexqueue  = queue;
    x->header.one.downstream.accel     = 0;
    x->header.one.downstream.modifier  = mod;
    x->header.one.downstream.action    = action;
    x->header.one.downstream.EPType    = epType;
    x->header.one.downstream.endpoint  = ep;
    x->header.one.downstream.toggle    = toggle;
    x->header.one.downstream.address   = usbAddress;
    x->header.one.downstream.crc8      = 0;
}

static inline void XCSR_XICSSendMessage(XCSR_CtrlLinkTypeT msgType, uint8 message, uint8 vport)
{
    union cpuMsgT msgHeader;
    msgHeader.rawData = 0; // initialization
    msgHeader.frmStruct = SINGLE_HEADER_NO_DATA;
    msgHeader.Vport = vport;
    msgHeader.ctrlLinkType = msgType;
    msgHeader.msg = message;
    //TODO: we could actually chop of the LSW below
    _XCSR_XICSSendMessage(GET_MSW_FROM_64(msgHeader.rawData), GET_LSW_FROM_64(msgHeader.rawData));
}

static inline void XCSR_XICSSendMessageWithData(XCSR_CtrlLinkTypeT msgType, uint8 message, uint8 vport, uint32 data)
{
    // TODO: should add assert to ensure data will fit
    union cpuMsgT msgHeader;
    msgHeader.rawData = 0; // initialization
    msgHeader.frmStruct = SINGLE_HEADER_NO_DATA;
    msgHeader.Vport = vport;
    msgHeader.ctrlLinkType = msgType;
    msgHeader.msg = message;
    msgHeader.data = data;
    _XCSR_XICSSendMessage(GET_MSW_FROM_64(msgHeader.rawData), GET_LSW_FROM_64(msgHeader.rawData));
}

static inline void XCSR_XICSSendMessageWithExtraData(XCSR_CtrlLinkTypeT msgType, uint8 message, uint8 vport, uint32 data, uint64 extraData)
{
    //NOTE: only 56 bits are sent
    // TODO: should add assert to ensure data will fit
    union cpuMsgT msgHeader;
    msgHeader.rawData = 0; // initialization
    msgHeader.frmStruct = DOUBLE_HEADER_NO_DATA;
    msgHeader.Vport = vport;
    msgHeader.ctrlLinkType = msgType;
    msgHeader.msg = message;
    msgHeader.data = data;
    _XCSR_XICSSendMessage(GET_MSW_FROM_64(msgHeader.rawData), GET_LSW_FROM_64(msgHeader.rawData),
            GET_MSW_FROM_64(extraData), GET_LSW_FROM_64(extraData));
}

#endif // XCSR_XICSQ_H
