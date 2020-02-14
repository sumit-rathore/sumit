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
#ifndef UPP_LOC_H
#define UPP_LOC_H

// Includes #######################################################################################
#include <ibase.h>
#include <callback.h>
#include <leon_timers.h>
#include <upp.h>
#include <usbdefs.h>
#include "upp_cmd.h"
#include "upp_topology.h"
#include "upp_queue_manager.h"

// Constants and Macros ###########################################################################

#define UPP_MAX_PACKET_CONTROL_TRANSFER_SIZE    (512+32)
#define UPP_MAX_PACKET_CONTROL_TRANSFER_WORDS   (UPP_MAX_PACKET_CONTROL_TRANSFER_SIZE/4)

#define UPP_SETUP_PACKET_DATA_LENGTH    8       // setup data packet from host is 8 bytes (USB 3.1 spec, section 9.3, page 9-14)

#define UPP_MAX_ENDPOINTS               15      // max number of endpoints per USB3 endpoint (USB 3.1 spec, section 8.8, page 8-50

#define UPP_USB_INVALID_DEVICE_ADDRESS  255     // valid endpoint address range is 0-127

#define UPP_MAX_ISO_QUEUE_BUFFERS   3               // Max ISO IN buffers available

// Max IN buffers available, queue 0 for Bulk and Interrupt, 1 to UPP_MAX_ISO_QUEUE_BUFFERS for ISO buffers
#define UPP_MAX_QUEUE_BUFFERS   (UPP_MAX_ISO_QUEUE_BUFFERS+1)

#define UPP_HUB_HUB_BM_REQUEST_TYPE         0x20    // see USB 3.1 spec, section 10.16.2, page 10-71
#define UPP_HUB_PORT_BM_REQUEST_TYPE        0x23    // see USB 3.1 spec, section 10.16.2, page 10-71

#define UPP_GET_DESCRIPTOR_BM_REQUEST       0x80    // see USB 3.1 spec, section9.4, table 9-4 Standard Device Requests
#define UPP_SET_ADDRESS_BM_REQUEST          0x00    // see USB 3.1 spec, section9.4, table 9-4 Standard Device Requests
#define UPP_SET_CONFIGURATION_BM_REQUEST    0x00    // see USB 3.1 spec, section9.4, table 9-4 Standard Device Requests
#define UPP_SET_INTERACE_BM_REQUEST         0x01    // see USB 3.1 spec, section9.4, table 9-4 Standard Device Requests

#define UPP_ENDPOINT_MAX_BURST          5    // Per Mohsen's comment, max value to set to 5.

#define UPP_DESCRIPTOR_HEADER_SIZE      2       // descriptor size + descriptor type (see USB 3.1 section 9.5, page 9-35)

#define UPP_ENDPOINT_SUPERSPEED_DESCRIPTOR_SIZE 6   // SuperSpeed Endpoint Companion descriptor size (see USB 3.1 section 9.6.7, page 9-55)

#define UPP_TRANSFER_MAX_SEQUENCE_NUMBER_MASK   0x1F    // sequence number in data packet header (DW1) is max 5 bits , see USB 3.1 section 8.6, table 8-25)

#define UPP_TRANSFER_DESCRIPTOR_BUFFER_SIZE     (32/sizeof(uint32_t))

// Data Types #####################################################################################

// Packet Type definition - see USB 3.1 spec, section 8.3.1.2, page 8-5
enum UppPacketType
{
    UPP_LINK_MANAGEMENT_PACKET          = 0x00,    // link management packet type
    UPP_TRANSACTION_PACKET              = 0x04,    // transaction packet type
    UPP_DATA_PACKET_HEADER              = 0x08,    // Data Packet Header
    UPP_ISOCHRONOUS_TIMESTAMP_PACKET    = 0x0C     // Isochronous Timestamp Packet
};

// Transaction Packet SubType definition - see USB 3.1 spec, table 8-12, page 8-30
enum UppTransactionPacketSubtype
{
    UPP_TRANSACTION_SUBTYPE_RESERVED            = 0x00,    // reserved
    UPP_TRANSACTION_SUBTYPE_ACK                 = 0x01,    // Acknowledgment
    UPP_TRANSACTION_SUBTYPE_NRDY                = 0x02,    // Not Ready
    UPP_TRANSACTION_SUBTYPE_ERDY                = 0x03,    // Endpoint Ready
    UPP_TRANSACTION_SUBTYPE_STATUS              = 0x04,    // Status
    UPP_TRANSACTION_SUBTYPE_STALL               = 0x05,    // Stall - control transfer invalid or endpoint halted
    UPP_TRANSACTION_SUBTYPE_DEVICE_NOTIFICATION = 0x06,    // Device notification
    UPP_TRANSACTION_SUBTYPE_PING                = 0x07,    // ping -
    UPP_TRANSACTION_SUBTYPE_PING_RESPONSE       = 0x08,    // ping response
};

// specific request (USB 3.1 spec, table 9-4, page 9-16)
enum UppStandardRequestCode
{
    UPP_STANDARD_REQ_GET_STATUS         = 0,
    UPP_STANDARD_REQ_CLEAR_FEATURE      = 1,
    UPP_STANDARD_REQ_RESERVED1          = 2,
    UPP_STANDARD_REQ_SET_FEATURE        = 3,
    UPP_STANDARD_REQ_RESERVED2          = 4,
    UPP_STANDARD_REQ_SET_ADDRESS        = 5,
    UPP_STANDARD_REQ_GET_DESCRIPTOR     = 6,
    UPP_STANDARD_REQ_SET_DESCRIPTOR     = 7,
    UPP_STANDARD_REQ_GET_CONFIGURATION  = 8,
    UPP_STANDARD_REQ_SET_CONFIGURATION  = 9,
    UPP_STANDARD_REQ_GET_INTERFACE      = 10,
    UPP_STANDARD_REQ_SET_INTERFACE      = 11,
    UPP_STANDARD_REQ_SYNCH_FRAME        = 12,
    UPP_STANDARD_REQ_SET_SEL            = 48,
    UPP_STANDARD_REQ_SET_ISOCH_DELAY    = 49,
};

// Hub Class Feature Selector (USB 3.1 spec, table 10-9, page 10-72)
enum UppHubClassFeatureSelector
{
    UPP_C_HUB_LOCAL_POWER = 0,
    UPP_C_HUB_OVER_CURRENT = 1,
    UPP_PORT_CONNECTION  = 0,
    UPP_PORT_OVER_CURRENT = 3,
    UPP_PORT_RESET = 4,
    UPP_PORT_LINK_STATE = 5,
    UPP_PORT_POWER = 8,
    UPP_C_PORT_CONNECTION = 16,
    UPP_C_PORT_OVER_CURRENT = 19,
    UPP_C_PORT_RESET = 20,
//    UPP_RESERVED = 21,
    UPP_PORT_U1_TIMEOUT = 23,
    UPP_PORT_U2_TIMEOUT = 24,
    UPP_C_PORT_LINK_STATE = 25,
    UPP_C_PORT_CONFIG_ERROR = 26,
    UPP_PORT_REMOTE_WAKE_MASK = 27,
    UPP_BH_PORT_RESET = 28,
    UPP_C_BH_PORT_RESET = 29,
    UPP_FORCE_LINKPM_ACCEPT = 30,
};

// Descriptor types (USB 3.1 spec, table 9-5, page 9-17)
enum UppDescriptorType
{
    UPP_DESCRIPTOR_DEVICE = 1,
    UPP_DESCRIPTOR_CONFIGURATION,
    UPP_DESCRIPTOR_STRING,
    UPP_DESCRIPTOR_INTERFACE,
    UPP_DESCRIPTOR_ENDPOINT,
    UPP_DESCRIPTOR_RESERVED1,
    UPP_DESCRIPTOR_RESERVED2,
    UPP_DESCRIPTOR_INTERFACE_POWER,
    UPP_DESCRIPTOR_OTG,
    UPP_DESCRIPTOR_DEBUG,
    UPP_DESCRIPTOR_INTERFACE_ASSOCIATION,
    UPP_DESCRIPTOR_BOS,
    UPP_DESCRIPTOR_DEVICE_CAPABILITY,
    UPP_DESCRIPTOR_SUPERSPEED_USB_ENDPOINT_COMPANION = 48,

};

// transactions we are interested in
enum UppDecodedTransaction
{
    UPP_IGNORED_TRANSACTION = 0,                    // if 0, it wasn't set - ignore
    UPP_INTERNAL_SET_ADDRESS,                       // set a devices's address
    UPP_INTERNAL_GET_CONFIGURATION,                 // get descriptor, configuration and endpoints
    UPP_INTERNAL_SET_CONFIGURATION,                 // set a devices's configuration
    UPP_INTERNAL_SET_INTERFACE,                     // set a devices's interface
    UPP_INTERNAL_HUB_PORT_CLEARED,                  // the specified hub port was cleared (endpoint attached to it was removed)
};

enum UppTransactionState
{
    UPP_TRANSACTION_FREE = 0,           // this transaction is available to be allocated
    UPP_TRANSACTION_ALLOCATED,          // this transaction is allocated; host processing active
    UPP_TRANSACTION_WAITING_FOR_DATA,   // transaction is waiting for data from endpoint
    UPP_TRANSACTION_COMPLETE,           // transaction is complete

};

// UPP cpu messages sent from the Lex to the Rex
enum UppLexSendMessage
{
    UPP_LEX_TO_REX_UPP_ENABLE_STATUS,       // UPP enable status on the Lex, sent on link up

    UPP_LEX_TO_REX_SET_ENDPOINT,            // Set this ISO endpoint on the Rex
    UPP_LEX_TO_REX_CLEAR_ENDPOINT,          // Clear this ISO endpoint on the Rex

    UPP_LEX_TO_REX_REMOVE_DEVICE,           // marker to ask the Rex to remove this endpoint from the Rex
    UPP_LEX_TO_REX_INTERFACE_SET_INFO,      // info for the Rex to send back to the Lex, when the endpoints have been freed
    UPP_LEX_TO_REX_ROUTE_CHANGE_COMPLETE,   // marker to say route change complete on Lex side
};


// UPP cpu messages sent from the Rex to the Lex
enum UppRexSendMessage
{
    UPP_REX_TO_LEX_ENDPOINT_SET,            // ISO endpoint set on the Rex
    UPP_REX_TO_LEX_ENDPOINT_CLEARED,        // ISO endpoint cleared on the Rex

    UPP_REX_TO_LEX_ENDPOINT_NOT_RESPONDING, // ISO endpoint not responding on Rex, disabled

    UPP_REX_TO_LEX_DEVICE_REMOVED,          // marker to say endpoint has been removed from Rex
    UPP_REX_TO_LEX_INTERFACE_SET_INFO,      // info for the Lex, when the Rex endpoints have been freed
    UPP_REX_TO_LEX_ROUTE_CHANGE_COMPLETE,   // marker to say route change complete on Rex side
};

enum UppSetEndpointEntryResult
{
    UPP_SET_ENDPOINT_NO_ACTION = 0x40,          // endpoint already allocated, or allocation not required
    UPP_SET_ENDPOINT_NO_BUFFERS = 0x80,         // endpoint not set because no free buffers available
    UPP_SET_ENDPOINT_BUFFER_ALLOCATED = 0xC0,   // endpoint tracking buffer set; endpoint still needs to be set in RTL table

    UPP_SET_ENDPOINT_RESULT_MASK = 0xC0         // the mask used to get the result
};

enum UPPSetInterfaceOperations
{
    UPP_SET_INTERFACE_OP,           // single interface on device has changed
    UPP_SET_CONFIGURATION_OP,       // device configuration has changed
    UPP_SET_CONFIG_COMPLETION_OP    // device configuration is completing
};

// Packet Header definition, first word, transaction and data packet (USB 3.1 spec, figure 8-2, page 8-4)
union UppPacketHeaderDw0
{
    struct
    {
        uint32_t deviceAddress:     7;  // endpoint address this is going to/came from
        uint32_t routeStringHub5:   4;  // the route string, tier #5 Hub
        uint32_t routeStringHub4:   4;  // the route string, tier #4 Hub
        uint32_t routeStringHub3:   4;  // the route string, tier #3 Hub
        uint32_t routeStringHub2:   4;  // the route string, tier #2 Hub
        uint32_t routeStringHub1:   4;  // the route string, tier #1 Hub (see USB3.1 section 8.9)
        uint32_t packetType:        5;  // the packet type
    };

    uint32_t    data;

};

// Transaction Packet Header definition, 2nd word (USB 3.1 spec, figure 8-2, page 8-4
union UppTpStatusPacketHeaderDw1
{
    struct
    {
        uint32_t reserved1:          20; // unused field for the status packet
        uint32_t endpointNumber:     4;  // endpoint address this is going to/came from
        uint32_t direction:          1;  // the route string
        uint32_t reserved2:          3;  // unused field for the status packet
        uint32_t subType:            4;  // the subtype of this packet
    };

    uint32_t    data;

};

// Data Packet Header definition, 2nd word (USB 3.0 spec, figure 8-22, page 8-23)
union UppDataPacketHeaderDw1
{
    struct
    {
        uint32_t dataLength:        16;  // length of the data part
        uint32_t setup:              1;  // true if this is a setup data packet
        uint32_t reserved2:          3;  // not used
        uint32_t endpointNumber:     4;  // the endpoint the data is coming/going to
        uint32_t direction:          1;  // 0 = host to endpoint; 1 = endpoint to host
        uint32_t endOfBurst:         1;  // true = last packet of a burst
        uint32_t reserved1:          1;  // not used
        uint32_t seqNumber:          5;  // Sequence number used
    };

    uint32_t    data;

};

// Header packet 3rd word format(USB 3.0 spec, figure 8-2)
union UppPacketHeaderDw3
{
    struct
    {
        // link control word format, see USB spec 3.1, section 8.3.14, table 8-2
        uint32_t lcwCrc5:            5;  // link control word, CRC for the previous 11 bits
        uint32_t lcwDeferred:        1;  // link control word, defer transfer
        uint32_t lcwDelayed:         1;  // link control word, header packet is delayed or resent
        uint32_t lcwHubDepth:        3;  // link control word, the hub hierarchy that issued the defer
        uint32_t lcwReserved:        3;  // link control word, reserved
        uint32_t lcwHeaderSeq:       3;  // link control word, header sequence field

//        uint32_t linkControlWord:   16;  //
        uint32_t crc16:             16;  // the CRC of the header packet
    };

    uint32_t    data;

};

// Setup data request - attached to a data packet header - (USB 3.1 spec, table 9-2, page 9-13)
union UppSetupDataDw4
{
    struct
    {
        uint32_t wValue:            16;     // value depends on the request (USB 3.1 spec, table 9-3, page 9-15)
        uint32_t bRequest:           8;     // specific request code (USB 3.1 spec, table 9-4, page 9-16)
        uint32_t reqTypeDir:         1;     // true if this in an IN request; false if it is an OUT
        uint32_t reqTypeType:        2;     // the type of request
        uint32_t reqTypeRecipient:   5;     // the recipient of this request
    } detailed;

    struct
    {
        uint32_t wValue:            16;     // value depends on the request (USB 3.1 spec, table 9-3, page 9-15)
        uint32_t bRequest:           8;     // specific request code (USB 3.1 spec, table 9-4, page 9-16)
        uint32_t bmRequestType:      8;
    } standard;

    struct
    {
        uint32_t wValue:            16;     // value depends on the request (USB 3.1 spec, table 9-3, page 9-15)
        uint32_t bRequestAndType:   16;     // specific request code bRequest and bmRequestType (USB 3.1 spec, table 9-4, 9-5)
    } block;

    struct
    {
        uint32_t descriptorType:     8;     // If bRequest == Get Descriptor, this field contains the type
        uint32_t descriptorIndex:    8;     // If bRequest == Get Descriptor, this field contains the descriptor index
        uint32_t bRequest:           8;     // specific request code (USB 3.1 spec, table 9-4, page 9-16)
        uint32_t reqTypeDir:         1;     // true if this in an IN request; false if it is an OUT
        uint32_t reqTypeType:        2;     // the type of request
        uint32_t reqTypeRecipient:   5;     // the recipient of this request
    } getDescriptor;

    struct
    {
        uint32_t reserved:          8;     // reserved - should be zero
        uint32_t newDeviceAddress:  8;     // the new device address that the host is setting
        uint32_t bRequest:          8;     // specific request code = 5 for set address (USB 3.1 spec, table 9-4, page 9-16)
        uint32_t zereoed:           8;     // for set address request, this field should be zero
    } setAddress;

    struct
    {
        uint32_t reserved:          8;     // reserved - should be zero
        uint32_t newConfigValue:    8;     // the new configuration that the host is setting
        uint32_t bRequest:          8;     // specific request code = 9 for set configuration (USB 3.1 spec, table 9-4, page 9-16)
        uint32_t zereoed:           8;     // for set address request, this field should be zero
    } setConfiguration;

    struct
    {
        uint32_t reserved:          8;     // reserved - should be zero
        uint32_t newAltSetting:     8;     // the new alt interface setting that the host wants
        uint32_t bRequest:          8;     // specific request code = 11 for set interface (USB 3.1 spec, table 9-4, page 9-16)
        uint32_t zereoed:           8;     // for set address request, this field should be zero
    } setInterface;

    uint32_t    data;

};

// Setup data request - attached to a data packet header - (USB 3.1 spec, table 9-2, page 9-13)
union UppSetupDataDw5
{
    struct
    {
        uint16_t wLength;           // Max number of bytes to transfer
        uint16_t wIndex;            // value depends on the request (USB 3.1 spec, table 9-3, page 9-15)
    };

    uint32_t    data;

};


// this structure allows us to define the various fields of an endpoint descriptor
// (USB 3.1 spec, table 9-24, page 9-51, 9-52)
struct UppDeviceIsoEndpointFields
{
    uint32_t blength:           8;  // the length of the field (skipped)
    uint32_t bDescriptorType:   8;  // the descriptor type (Endpoint, == 5)
    uint32_t endpointDirection: 1;  // (bit 7) true = IN endpoint, false = OUT endpoint
    uint32_t reserved1:         3;  // (bits (4:6) reserved
    uint32_t endpointNumber:    4;  // (bits 0:3) the number of this endpoint, 1-15
    uint32_t reserved2:         2;  // (bits 6:7) reserved,
    uint32_t isoUsageType:      2;  // (bits 4:5) ISO usage type (00: data, 01: feedback, 10: implicit feedback, 11: reserved)
    uint32_t isoSynchType:      2;  // (bits 2:3) ISO synchronization type
    uint32_t transferType:      2;  // (bits 0:1) endpoint type (Control, ISO, Bulk, Interrupt)
};


// Setup data request - attached to a data packet header - (USB 3.1 spec, table 9-2, page 9-13)
/*union UppUsb3SetupData
{
    struct
    {
        uint16_t wValue;            // value depends on the request (USB 3.1 spec, table 9-3, page 9-15)
        uint8_t  bRequest;          // specific request (USB 3.1 spec, table 9-4, page 9-16)
        uint8_t  bmRequestType;     // Characteristics of request (USB 3.1 spec, table 9-3, page 9-15)

        uint16_t wLength;           // Max number of bytes to transfer
        uint16_t wIndex;            // value depends on the request (USB 3.1 spec, table 9-3, page 9-15)
    }

    uint32_t    data[2];
};
*/

//struct
//{
//    struct
//    {
//        packetData = __builtin_bswap32(uppPtr->ctrl_trfr.s.h2d_data.dw);
//
//    };
//
//} UPP_Packet_Header;

struct UPPBufferDiag
{
    uint8_t deviceAddress;          // the device
    uint8_t numberOfReads;          // number of bytes read to make up this packet

    struct
    {
        uint16_t type:          4;      // the type of transaction this is
        uint16_t fifo:          1;      // the fifo this packet is from (0 = H2D, 1 = D2H)
        uint16_t packetSkipped: 1;      // this packet was skipped at some point
        uint16_t setupPacket:   1;      // setup control bit was set for this packet
        uint16_t deferred:      1;      // if this was a defer packet
        uint16_t extra:         8;      // Extra info depending on the transaction type
    };

    uint16_t crc16;
    uint16_t packetTimeTag;         // the time tag, sign extended, for this packet
};


// the packet we are currently decoding
struct UppPacketDecode
{
    union UppPacketHeaderDw0 header; // route string and device address for this packet

    uint32_t rxData[2];             // data that we received from the endpoint
    uint8_t  rxSize;                // number of bytes we received

    uint8_t  dWordCount;            // number of dWords we received, for parsing

    bool    skipRestOfPacket;       // true if we should skip the rest of this packet

//    uint8_t packetEvent;            // event to send for this packet

    int16_t packetTimeTag;         // the time tag, sign extended, for this packet

    uint16_t packetDataLength;      // Length of data in this packet

    struct UPPBufferDiag bufferInfo;
};

// the set of device requests we expect to see from the host.  Grouped together here so if we get an invalid one,
// we can clear the parameters before processing the new one
union UppHostDeviceRequests
{
    struct
    {
        union UppDeviceLocation location;   // the location of the device we want to set the address for

    } setAddress;

    struct
    {
        uint16_t maxLength;             // the maximum length we are expected to receive (actual can be smaller)

    } getConfiguration;

    struct
    {
        uint8_t newConfigurationValue;      // the new configuration value we want to set

    } setConfiguration;

    struct
    {
        uint8_t interface;      // interface we want to set
        uint8_t altSetting;     // the Alt setting of the interface we want to use

    } setInterface;

    struct
    {
        union UppPacketHeaderDw0 hubAddress;    // route string and device address for this hub
        uint8_t portNumber;                     // port that was cleared

    } hubPortCleared;
};

struct UppParseData
{
    union EndpointRoute epRoute;    // the endpoint topology position we are parsing

    struct UppEndpointData endpointParsed; // the current endpoint we are parsing - once saved, it is set to zero

    uint32_t descriptorBuffer[UPP_TRANSFER_DESCRIPTOR_BUFFER_SIZE];
    uint8_t  bytesInDescriptorBuffer;

    int16_t numberOfSkipBytesNeeded;    // number of bytes to skip when starting a new packet
};



// Note:  There is always only one control transfer per device, see USB 3.1, section 8.1
struct UppUsb3Transaction
{
    uint8_t deviceAddress;          // the device this transaction is for
    uint8_t type;                   // the type of transaction this is

    uint8_t outSeqNumber;           // sequence number from host data packets
    uint8_t inSeqNumber;            // sequence number from device data packets

    uint16_t maxLength;             // the maximum length we are expected to receive (actual can be smaller)
    uint16_t actualLength;          // actual length received

    LEON_TimerValueT lastPacketRx;  // used to determine if this transfer has been abandoned and should be freed

    union UppPacketHeaderDw0 location;       // route string and endpoint address for this packet

    struct
    {
        uint8_t inDirection:            1; // true if this is an IN transaction (data from endpoint); false otherwise
        uint8_t receivedStatusPacket:   1; // Host sent status packet, waiting for us to finish processing
        uint8_t receivedDefer:          1; // Received a defer in the data path
        uint8_t parseFatalError:        1; // TRUE if we encountered an unrecoverable parsing error
        uint8_t reserved:               4;
    };

    uint8_t endpointSet;            //  endpoint sent to Rex to set

    int16_t allocatedpacketTimeTag; // the time tag, sign extended, when this packet was allocated
    int16_t statusTimeTag;          // the counter value, sign extended, when we received the status.  Any data packets after this are for another transfer

    struct Device *devicePtr;       // topology's reference to this device

    union UppHostDeviceRequests coms;   // the device requests we process from the host

    struct UppParseData deviceData;

    struct UppParseData devicePacketBackup;

    uint8_t nextTransactionIndex;        // index into the next transaction; UPP_MAX_OPEN_TRANSACTIONS marks end
    uint8_t prevTransactionIndex;        // index into the previous transaction; UPP_MAX_OPEN_TRANSACTIONS marks start
};

struct UPPInterfaceSwitch
{
    struct Device *topologyDevicePtr;   // pointer to the topology endpoint on the Lex
    uint32_t location;                  // the new location we want to set
    enum UPPSetInterfaceOperations requestedOp;  // whether this is just a single interface, or a configuration
};

struct LexCpuMessage
{
    // enum UppLexSendMessage messageType;

    // this is a union of data structures we want to send to the Rex; which struct
    // is determined by the messageType
    union
    {
        struct
        {
            struct Endpoint endpoint;
            struct Endpoint *endpointPtr;
        };

        struct UPPInterfaceSwitch interface;    // information the Lex will need to complete the switch
        bool lexIsoEnabled;                     // true if Lex's ISO is bypassed
    };

    uint8_t deviceAddress;                  // the address of the device we are interested in

};

struct RexCpuMessage
{
    // enum UppRexSendMessage messageType;

    // this is a union of data structures we want to send to the Lex; which struct
    // is determined by the messageType
    union
    {
        struct Endpoint *endpointPtr;
        struct Endpoint endpoint;
        struct Device *topologyDevicePtr;       // pointer to the topology endpoint on the Lex
        struct UPPInterfaceSwitch interface;    // information the Lex will need to complete the switch
    };

    uint8_t deviceAddress;                  // the address of the device we are interested in

};


// see USB3.1, section 9.6.1, 9-36
#define UPP_USB3_MAX_CONTROL_DATA_TRANSFER_SIZE     512


// Function Declarations ##########################################################################

void UPP_LexInit(void)                                                  __attribute__((section(".lexatext")));
void UppSendCPUCommLexUppMessage(
    enum UppLexSendMessage messageType,
    struct LexCpuMessage *message)                                      __attribute__((section(".lexftext")));
void UPP_LexUppISR(void)                                                __attribute__((section(".lexftext")));

uint32_t UppSetLocation(union UppPacketHeaderDw0 hubAddress )           __attribute__((section(".lexftext")));

uint32_t UppSetRouteFromHubPort(
    union UppPacketHeaderDw0 hubAddress,
    uint8_t portNumber)                                                 __attribute__((section(".lexftext")));

void UppDeviceAddToSystem(uint32_t routeNumber)                         __attribute__((section(".lexftext")));
void UppDeviceSetConfiguration(
    struct Device *devicePtr,
    uint8_t newConfigurationValue)                                      __attribute__((section(".lexftext")));

void UppDeviceSetInterface(
    struct Device *devicePtr,
    uint8_t interface,
    uint8_t altSetting)                                                 __attribute__((section(".lexftext")));

void UppDeviceUpdateEndpointsCallback(void *param1, void *param2)       __attribute__((section(".lexftext")));

void TopologyChangeFinished(void)                                       __attribute__((section(".lexftext")));

void UPP_RexInit(void)                                                  __attribute__((section(".rexatext")));
void UPP_RexReinit(void)                                                __attribute__((section(".rexatext")));
void UppSendCPUCommRexUppMessage(
    enum UppRexSendMessage messageType,
    struct RexCpuMessage *message)                                      __attribute__((section(".rexftext")));
void UPP_RexUppISR(void)                                                __attribute__((section(".rexftext")));

// stat functions
void UPP_StatInit(void)                                                 __attribute__((section(".atext")));
void Upp_StatMonControl(bool enable)                                    __attribute__((section(".atext")));

// Queue management functions
void UppQueueManagementInit(void)                                       __attribute__((section(".lexatext")));
void UppQueueManagementDisable(void)                                    __attribute__((section(".lexatext")));
void LexQueueStateSendEventWithNoData(enum UppQueueManagerEvent event)  __attribute__((section(".lexftext")));
bool UppQueueManagementIsDisabled(void)                                 __attribute__((section(".lexftext")));

// transaction processing
void UPP_TransactionInit(void)                                          __attribute__((section(".lexftext")));
void UppTransactionReinit(void)                                         __attribute__((section(".lexftext")));

void UppTransactionStartHostPacketDecode(void)                          __attribute__((section(".lexftext")));
void UppTransactionStartDevicePacketDecode(void)                        __attribute__((section(".lexftext")));

bool UppTransactionUnallocatedDevice(void)                              __attribute__((section(".lexftext")));
void UppTransactionClearUnallocatedDevice(void)                         __attribute__((section(".lexftext")));
uint8_t UppTransactionGetDeviceAddress(void)                            __attribute__((section(".lexftext")));

void UppTransactionDownstreamPacket(void)                               __attribute__((section(".lexftext")));
void UppTransactionUpstreamPacket(void)                                 __attribute__((section(".lexftext")));
void UppTransactionContinueUpstreamPacket(struct UppUsb3Transaction *transaction)       __attribute__((section(".lexftext")));

uint8_t UppTransferSkipData(
    struct UppUsb3Transaction *transaction,
    struct UppPacketDecode *packet,
    uint8_t bytesSkipRequested)                                         __attribute__((section(".lexftext")));

void UppTransactionEndOfDownstreamPacket(struct UppUsb3Transaction *transaction)        __attribute__((section(".lexftext")));

struct UppUsb3Transaction * UppTransactionProcessUpstreamPacketData(
    struct UppUsb3Transaction *transaction,
    uint32_t data,
    uint8_t byteCount)                                                  __attribute__((section(".lexftext")));

void UppTransactionEndOfUpstreamPacket(struct UppUsb3Transaction *transaction)          __attribute__((section(".lexftext")));

void UppTransactionDataQueueEmpty(void)                                 __attribute__((section(".lexftext")));
void UppTransactionFreeFinishedHostTransfers(void)                      __attribute__((section(".lexftext")));
void UppTransactionFreeFinishedTransfers(void)                          __attribute__((section(".lexftext")));

struct UppUsb3Transaction * UppTransactionGetTransaction(uint8_t deviceAddress)         __attribute__((section(".lexftext")));
void UppTransactionFreeTransaction(struct UppUsb3Transaction *transaction)              __attribute__((section(".lexftext")));


// parsing routines
void UPP_ParseInit(void)                                                                __attribute__((section(".lexftext")));
bool UppParseDescriptors(struct UppUsb3Transaction *transaction, struct UppPacketDecode *packet)    __attribute__((section(".lexftext")));

// device functions
void UPP_DeviceInit(void)                                                               __attribute__((section(".lexftext")));
void UPP_DeviceReinit(void)                                                             __attribute__((section(".lexftext")));
void UppDeviceClearHubPort(union UppPacketHeaderDw0 hubAddress, uint8_t portNumber)     __attribute__((section(".lexftext")));
bool UppDeviceRouteRemoved(uint32_t routeNumber)                                        __attribute__((section(".lexftext")));
void UppDeviceRemoved(uint8_t deviceAddress)                                            __attribute__((section(".lexftext")));
bool UppDeviceTopologyChanging(void)                                                    __attribute__((section(".lexftext")));
void deviceRemovedCallback(void *param1, void *param2)                                  __attribute__((section(".lexftext")));
void TopologyChangeDoneCallback(void *param1, void *param2)                             __attribute__((section(".lexftext")));
void DeviceSetInterfaceClearEndpoints(void *param1, void *param2)                       __attribute__((section(".lexftext")));
void DeviceSetInterfaceSetEndpoints(void *param1, void *param2)                         __attribute__((section(".lexftext")));
void endpointInactiveCallback(void *param1, void *param2)                               __attribute__((section(".lexftext")));

// endpoint table functions
void UppEndpointTableInit(void)                                                         __attribute__((section(".lexftext")));
void UppEndpointTableReinit(void)                                                       __attribute__((section(".lexftext")));

uint8_t UppSetEndpointTableEntry(struct Endpoint *entryContents )                       __attribute__((section(".lexftext")));
uint8_t UppFreeEndpointTableEntry(uint8_t deviceAddress, uint8_t endpointNumber )       __attribute__((section(".lexftext")));

uint8_t UppEndpointExists(uint8_t deviceAddress, uint8_t endpoint)                      __attribute__((section(".lexftext")));


// HAL functions
void UPP_HalInit(void)                                                  __attribute__ ((section(".atext")));
void UPP_HalControlQueueInterrupts(bool enable);
void UPP_HalControlNotRespondingInterrupts(bool enable);
uint32_t UppHalGetIrq0Interrupts(void);
uint32_t UppHalGetIrq1Interrupts(void);
void UppHalUppEnable(bool enable);
void UppHalUppRestart(void);
void UppHalUppBypassed(bool enabled);
void UPPHalSetReadClearStats(void)                                      __attribute__ ((section(".atext")));

bool UppHalDownstreamPacketAvailable(void);
bool UppHalUpstreamPacketAvailable(void);

int16_t UppHalGetDownstreamTimeStamp(void);
int16_t UppHalGetUpstreamTimeStamp(void);
bool UppHalCompareTimestamps(int16_t firstTimestamp, int16_t secondTimestamp);

void UppHalSetDownstreamSkip(uint8_t skipValue);
void UppHalSetUpstreamSkip(uint8_t skipValue);

uint8_t UppHalReadDownstreamData(uint32_t *data);
uint8_t UppHalReadUpstreamData(uint32_t *data);

uint8_t UppHalGetDownstreamReadCount(void);
uint8_t UppHalGetUpstreamReadCount(void);

void UppHalFlushUpstreamPacketRead(void) __attribute__ ((used));

void UppHalEndTransaction(uint8_t deviceAddress);
void UppHalControlEndpoint(struct Endpoint *endpoint, bool enable);

// diagnostics
void UPP_DiagReinit(void)                                                               __attribute__((section(".lexftext")));
void UppDiagStoreBufferInfo(struct UPPBufferDiag *bufferInfo)                           __attribute__((section(".lexftext")));

void UppDiagConfigurationDescriptor(struct ConfigDescriptor *configDesc)                __attribute__((section(".lexftext")));
void UppDiagInterfaceDescriptor(struct InterfaceDescriptor *interfaceDesc)              __attribute__((section(".lexftext")));
void UppDiagEndpointDescriptor(struct EndpointDescriptor *endpointDesc)                 __attribute__((section(".lexftext")));

bool UppIsIsoEnabled(void);
bool UppControlTransferIsEnabled(void);
void UPP_EnableIso(void);
void UPP_DisableIso(void);


#endif // ULP_LOC_H
