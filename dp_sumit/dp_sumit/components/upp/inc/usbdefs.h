///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013
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
#ifndef USBDEFS_H
#define USBDEFS_H

/***************************** Included Headers ******************************/
#include <ibase.h>

/************************ Defined Constants and Macros ***********************/

#define USB_SETUP_DEVICE_MAX_RESPONSE_TIME      (500*1000) // 500 ms for device to start responding to host (USB 3.1, section 9.2.6.4)

// PID definitions
#define DATA0_PID 0xC3
#define DATA1_PID 0x4B
#define MDATA_PID 0x0F
#define SETUP_PID 0x2D
#define NAK_PID   0x5A


// Descriptor types
#define DEVICE_DESC             (0x01)
#define CONFIG_DESC             (0x02)
#define STRING_DESC             (0x03)
#define INTF_DESC               (0x04)
#define ENDPOINT_DESC           (0x05)
#define DEVICE_QUALIFIER_DESC   (0x06)
#define OTHER_SPEED_CONFIG_DESC (0x07)
#define HUB_DESC                (0x29)
#define UNKNOWN_DESC            (0xFF) // Not really a descriptor type

// Endpoint Address decoding
#define ENDPOINT_NUMBER_MASK        (0x0F)
#define ENDPOINT_DIRECTION_OFFSET   (7)

enum EndpointDirection
{
    EP_DIRECTION_OUT = 0,
    EP_DIRECTION_IN
};

enum EndpointTransferType
{
    EP_TYPE_CTRL = 0,
    EP_TYPE_ISO,
    EP_TYPE_BULK,
    EP_TYPE_INT
};

enum UsbSpeed
{
    USB_SPEED_HIGH       = 0,    // These values are set to the match the HW registers
    USB_SPEED_FULL       = 1,
    USB_SPEED_LOW        = 2,
    USB_SPEED_INVALID    = 3
};

// Device/Interface class definitions
#define CLASS_HID           (3)
#define CLASS_MASS_STORAGE  (8)
#define CLASS_HUB           (9)
#define CLASS_SMARTCARD     (11)

// Hub port features
enum PortFeatures
{
    PORT_CONNECTION,
    PORT_ENABLE,
    PORT_SUSPEND,
    PORT_OVER_CURRENT,
    PORT_RESET,
    PORT_RESERVED,
    PORT_RESERVED2,
    PORT_RESERVED3,
    PORT_POWER,
    PORT_LOW_SPEED,
    PORT_HIGH_SPEED,
    PORT_TEST,
    PORT_INDICATOR,
    PORT_RESERVED4,
    PORT_RESERVED5,
    PORT_RESERVED6,
    C_PORT_CONNECTION,
    C_PORT_ENABLE,
    C_PORT_SUSPEND,
    C_PORT_OVER_CURRENT,
    C_PORT_RESET
};

// Mass Storage feature selectors
enum MassStorageFeatures
{
    MSC_GET_MAX_LUN = 0xFE,
    MSC_MASS_STORAGE_RESET = 0xFF
};

#define PORT_CONNECTION_STS(portStatus)  ((portStatus & (1 << PORT_CONNECTION)) != 0)
#define PORT_ENABLE_STS(portStatus)      ((portStatus & (1 << PORT_ENABLE))     != 0)
#define PORT_LOW_SPEED_STS(portStatus)   ((portStatus & (1 << PORT_LOW_SPEED))  != 0)
#define PORT_HIGH_SPEED_STS(portStatus)  ((portStatus & (1 << PORT_HIGH_SPEED)) != 0)
#define SET_ADDRESS_STANDARD_REQUEST        5
#define GET_DESCRIPTOR_STANDARD_REQUEST     6
#define SET_DESCRIPTOR_STANDARD_REQUEST     7
#define GET_CONFIGURATION_STANDARD_REQUEST  8
#define SET_CONFIGURATION_STANDARD_REQUEST  9
#define GET_INTERFACE_STANDARD_REQUEST      10
#define SET_INTERFACE_STANDARD_REQUEST      11
#define SYNCH_FRAME_STANDARD_REQUEST        12

/******************************** Data Types *********************************/
/******************************************************************************
 * DEVICE REQUESTS - chapter 9
 ******************************************************************************/
#define CREATE_REQUEST_TYPE(recipient, type, direction) (recipient | type << 5 | direction << 7)
#define GET_REQUEST_TYPE_REC(requestType) ((requestType >> 0) & 0x1f)
#define GET_REQUEST_TYPE_TYPE(requestType) ((requestType >> 5) & 0x3)
#define GET_REQUEST_TYPE_DIR(requestType) ((requestType >> 7) & 0x1)

#define GET_DESC_TYPE_FROM_VALUE(value) ((value >> 8) & 0xFF)

enum RequestTypeRecipient
{
    REQUESTTYPE_REC_DEVICE,
    REQUESTTYPE_REC_INTF,
    REQUESTTYPE_REC_ENDP,
    REQUESTTYPE_REC_OTHER
};

enum RequestTypeType
{
    REQUESTTYPE_TYPE_STANDARD,
    REQUESTTYPE_TYPE_CLASS,
    REQUESTTYPE_TYPE_VENDOR,
    REQUESTTYPE_TYPE_RESERVED
};

enum RequestTypeDirection
{
    REQUESTTYPE_DIR_H2D,
    REQUESTTYPE_DIR_D2H
};

enum StandardRequests
{
    STD_REQ_GET_STATUS,
    STD_REQ_CLEAR_FEATURE,
    STD_REQ_RESERVED,
    STD_REQ_SET_FEATURE,
    STD_REQ_RESERVED2,
    STD_REQ_SET_ADDRESS,
    STD_REQ_GET_DESCRIPTOR,
    STD_REQ_SET_DESCRIPTOR,
    STD_REQ_GET_CONFIGURATION,
    STD_REQ_SET_CONFIGURATION,
    STD_REQ_GET_INTERFACE,
    STD_REQ_SET_INTERFACE,
    STD_REQ_SYNC_FRAME
};

enum HubClassRequests
{
    HUB_REQ_GET_STATUS = 0,
    HUB_REQ_CLEAR_FEATURE,
    HUB_REQ_SET_FEATURE = 3,
    HUB_REQ_GET_DESCRIPTOR = 6,
    HUB_REQ_SET_DESCRIPTOR,
    HUB_REQ_CLEAR_TT_BUFFER,
    HUB_REQ_RESET_TT,
    HUB_REQ_GET_TT_STATE,
    HUB_REQ_STOP_TT
};

enum DescriptorTypes
{
    DEVICE_DESCRIPTOR = 1,
    CONFIGURATION_DESCRIPTOR,
    STRING_DESCRIPTOR,
    INTERFACE_DESCRIPTOR,
    ENDPOINT_DESCRIPTOR,
    DEVICE_QUALIFIER_DESCRIPTOR,
    OTHER_SPEED_CONFIGURATION_DESCRIPTOR,
    INTERFACE_POWER_DESCRIPTOR
};

enum FeatureSelector
{
    ENDPOINT_HALT,
    DEVICE_REMOTE_WAKEUP,
    TEST_MODE
};

struct DeviceRequest
{
    uint8_t pid;
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint8_t wValue[2];
    uint8_t wIndex[2];
    uint8_t wLength[2];
}__attribute__((__packed__));

/******************************************************************************
 * STANDARD DESCRIPTORS - chapter 9
 ******************************************************************************/

// All descriptors start with these 2 bytes
struct BaseDescriptor {
    uint8_t  bLength;         // size of this descriptor in bytes
    uint8_t  bDescriptorType; // descriptor type
} __attribute__((__packed__));

//USB 2.0 spec 9.6.1, Device Descriptor
struct DeviceDescriptor {
    uint8_t  bLength;         //size of this descriptor in bytes
    uint8_t  bDescriptorType; //DEVICE descriptor type
    uint8_t  bcdUSB[2];       //USB spec release #, in BCD (eg: 2.10 = 0x210)
    uint8_t  bDeviceClass;    //class code (assigned by USB-IF, hub classes in usb spec 11.23.1)
    uint8_t  bDeviceSubClass; //subclass code (assigned by USB-IF)
    uint8_t  bDeviceProtocol; //protocol code (assigned by USB-IF)
    uint8_t  bMaxPacketSize0; //maximum packet size for endpoint0 (8,16,32, or 64)
    uint8_t  idVendor[2];     //vendor ID (assigned by USB-IF)
    uint8_t  idProduct[2];    //product ID (assigned by manufacturer)
    uint8_t  bcdDevice[2];    //Device release number in BCD
    uint8_t  iManufacturer;   //Index of string descriptor describing manufacturer
    uint8_t  iProduct;        //Index of string descriptor describing product
    uint8_t  iSerialNumber;   //Index of string descriptor describing device serial #
    uint8_t  bNumConfigs;     //Number of possible configurations
} __attribute__((__packed__));

//USB 2.0 spec 9.6.2, Device_Qualifier
struct DeviceQualifier {
    uint8_t  bLength;         //size of this descriptor in bytes
    uint8_t  bDescriptorType; //DEVICE descriptor type
    uint8_t  bcdUSB[2];        //USB spec release #, in BCD (eg: 2.10 = 0x210)
    uint8_t  bDeviceClass;    //class code (assigned by USB-IF, hub classes in usb spec 11.23.1)
    uint8_t  bDeviceSubClass; //subclass code (assigned by USB-IF)
    uint8_t  bDeviceProtocol; //protocol code (assigned by USB-IF)
    uint8_t  bMaxPacketSize0; //maximum packet size for endpoint0 (8,16,32, or 64)
    uint8_t  bNumConfigs;     //Number of possible configurations
    uint8_t  bReserved;       //Number of possible configurations
} __attribute__((__packed__));

//USB 2.0 spec 9.6.3, Standard Configuration Descriptor
struct ConfigDescriptor
{
    uint8_t  bLength;           // size of this descriptor in bytes
    uint8_t  bDescriptorType;   // DEVICE descriptor type
    uint8_t  wTotalLength[2];   // total length of data, including all descriptors
    uint8_t  bNumInterfaces;    // number of interfaces supported
    uint8_t  bConfigurationVal; // Value to setConfig to select this config
    uint8_t  iConfiguration;    // index of string desc. describing this config
    uint8_t  bmAttributes;      // configuration charactersistics:
                                //   D7 Reserved, set to 1. (USB 1.0 Bus Powered),
                                //   D6 Self Powered, D5 Remote Wakeup, D4..0 Reserved, set to 0.
    uint8_t  bMaxPower;         // maximum power consumption from bus (multiples of 2mA)
} __attribute__((__packed__));

//USB 2.0 spec 9.6.5, Standard Interface Descriptor
struct InterfaceDescriptor
{
    uint8_t bLength;              // size of this descriptor in bytes
    uint8_t bDescriptorType;      // INTERFACE descriptor type
    uint8_t bInterfaceNumber;     // number of this interface
    uint8_t bAlternateSetting;    // alternate interface number to use to access this interface
    uint8_t bNumEndpoints;        // number of endpoints on this interface
    uint8_t bInterfaceClass;      // class of this interface
    uint8_t bInterfaceSubClass;   // subclass of this interface
    uint8_t bInterfaceProtocol;   // protocol code (class specific?  if not, 0)
    uint8_t iInterface;           // index of string desc. describing this interface
} __attribute__((__packed__));

//USB 2.0 spec 9.6.6, Standard Endpoint Descriptor
struct EndpointDescriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress; // address of this endpoint on the USB device
                            //   Bits 0..3b Endpoint Number.
                            //   Bits 4..6b Reserved. Set to Zero
                            //   Bits 7 Direction 0 = Out, 1 = In (Ignored for Control Endpoints)
    uint8_t bmAttributes;     // endpoint attributes when configured using bConfigrationValue
                            //   Bits 0..1 Transfer Type
                            //     00 = Control 01 = Isochronous 10 = Bulk 11 = Interrupt
                            //   If not ISO, 5..2 reserved and set to 0, if ISO:
                            //   Bits 3..2 = Synchronisation Type (Iso Mode)
                            //     00 = No Synchonisation 01 = Asynchronous 10 = Adaptive 11 =
                            //     Synchronous
                            //   Bits 5..4 = Usage Type (Iso Mode)
                            //     00 = Data Endpoint 01 = Feedback Endpoint 10 = Explicit Feedback
                            //     Data Endpoint 11 = Reserved
    uint8_t wMaxPacketSize[2];// max packet size for this endpoint
    uint8_t bInterval;        // polling interval for endpoint transfers
} __attribute__((__packed__));

// 9.6.7 USB3.1 SuperSpeed Endpoint Companion
struct SuperSpeedEndpointCompanionDescriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bMaxBurst;      // The maximum number of packets the endpoint can send or receive as part of a burst
    uint8_t bmAttributes;   // (definition depends on the endpoint type)
    uint8_t wBytesPerInterval[2];// The total number of bytes this endpoint will transfer every service interval (SI).
} __attribute__((__packed__));


/*********************************** API *************************************/


#endif //USBDEFS_H

