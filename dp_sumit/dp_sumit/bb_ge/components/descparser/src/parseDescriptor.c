///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2007, 2008, 2013
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
//!   @file  -  parseDescriptor.c
//
//!   @brief -  parse setup descriptor responses from the device
//
//
//!   @note  -
//
//
///////////////////////////////////////////////////////////////////////////////


/**************************** Included Headers *******************************/
#include <ibase.h>
#include <usbdefs.h>
#include <grg.h>
#include <storage_Data.h>
#include <linkmgr.h>

#include "descparser_loc.h"

/************************ Defined Constants and Macros ***********************/
#define DESC_TRANSFER_TYPE_MASK     (0x03)


// Mass storage class
#define INTF_PROTOCOL_MASS_STORAGE_BULK_ONLY    (0x50)


/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static inline void parseDeviceDescriptor(
    SetupResponseT * currentResponse,
    uint8 currentByte,
    uint8 currentOffset,
    XUSB_AddressT address);
static inline void parseConfigurationDescriptor(
    SetupResponseT * currentResponse, uint8 currentByte, uint8 currentOffset);
static inline void parseInterfaceDescriptor(
    SetupResponseT * currentResponse,
    uint8 currentByte,
    uint8 currentOffset,
    XUSB_AddressT address);
static inline void parseEndpointDescriptor(
    SetupResponseT * currentResponse,
    uint8 currentByte,
    uint8 currentOffset,
    XUSB_AddressT address);
static inline boolT shouldEndpointBeBlocked(
    const SetupResponseT* currentResponse,
    enum EndpointTransferType endpointType,
    XUSB_AddressT address);
static inline void parseUnknownDescriptor(
    SetupResponseT * currentResponse, uint8 currentByte, uint8 currentOffset);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: PARSER_Init()
*
* @brief  - Initialize the descriptor parser.
*
* @return - void.
*/
void PARSER_init(void)
{
    msaEnabled =
        ((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >> TOPLEVEL_SUPPORT_MSA_OFFSET) & 0x1);
}

/**
* FUNCTION NAME: PARSER_InitParseResponsePacket()
*
* @brief - Initializes the parsing of a brand new host request
*
* @param - logicalAddr - USB logical Address
* @param - requestedLength - Host requested length for this request
*
* @return - void
*/
void PARSER_InitParseResponsePacket(SetupResponseT * currentResponse, uint16 requestedLength)
{
    //ilog_DESCPARSER_COMPONENT_2(
    //    ILOG_DEBUG, INIT_PARSE_RESPONSE_PACKET, logicalAddr, requestedLength);

    // Just setup the minimum elements for a quick return
    currentResponse->currentDescriptorType = UNKNOWN_DESC;
    currentResponse->currentDescriptorOffset = 0;
    currentResponse->configurationValue = 0;

    // We don't know the actual length of the descriptor yet, but it will be sent in 
    // the first packet of the descriptor. We set this to the requestedLength as an
    // initial guess.
    currentResponse->descriptorLength = requestedLength;
}

/**
* FUNCTION NAME: PARSER_ParseResponsePacket()
*
* @brief - Runs the parsing of setup packets.
*
* @return - void
*
* @note - bug 1474: There is a device that sends data beyond the end of the descriptors.
*/
void PARSER_ParseResponsePacket
(
    SetupResponseT * currentResponse,   // a pointer to the current response state structure
    const XUSB_AddressT address,        // XUSB address
    const uint8 * data,                 // a pointer to the incoming data
    uint16 dataSize,                    // The amount of data to process
    uint16 currentByteOffset            // How many bytes into this request are we
)
{
    while (dataSize)
    {
        const uint8 currentByte = *data;          // The current byte to be parsed

        // Check if the current byte is beyond the end of the descriptor.  See bug 1474, for a
        // buggy flash drive.
        if (currentByteOffset < currentResponse->descriptorLength)
        {
            uint8 currentOffset = currentResponse->currentDescriptorOffset;

            ilog_DESCPARSER_COMPONENT_3(
                ILOG_DEBUG,
                PARSE_PACKET,
                XCSR_getXUSBAddrLogical(address),
                currentOffset,
                currentByte);

            //1) Call the parser for this packet type
            switch (currentResponse->currentDescriptorType)
            {
                case DEVICE_DESC:
                    parseDeviceDescriptor(currentResponse, currentByte, currentOffset, address);
                    break;

                case CONFIG_DESC:
                    parseConfigurationDescriptor(currentResponse, currentByte, currentOffset);
                    break;

                case INTF_DESC:
                    parseInterfaceDescriptor(currentResponse, currentByte, currentOffset, address);
                    break;

                case ENDPOINT_DESC:
                    parseEndpointDescriptor(currentResponse, currentByte, currentOffset, address);
                    break;

                default:
                    // The rest are don't care's.  Leave as parseUnknownDescriptor.
                    parseUnknownDescriptor(currentResponse, currentByte, currentOffset);
                    break;
            }

            //2) update the offset
            currentOffset++;

            //3) Check for the end of the descriptor & update currentResponse
            if (currentOffset >= currentResponse->currentDescriptorLength)
            {
                currentResponse->currentDescriptorType = UNKNOWN_DESC;
                currentResponse->currentDescriptorOffset = 0;
            }
            else
            {
                currentResponse->currentDescriptorOffset = currentOffset;
            }
        }
        else
        {
            // Ignored byte as it is beyond the end of the descriptor
            ilog_DESCPARSER_COMPONENT_3(
                ILOG_DEBUG,
                PARSE_PACKET_IGNORED_BYTE,
                XCSR_getXUSBAddrLogical(address),
                currentByte,
                currentByteOffset);
        }

        currentByteOffset++;
        data++;
        dataSize--;
    }
}

/**
* FUNCTION NAME: PARSER_FinalizeParsing()
*
* @brief - Calls into the topology code when a complete configuration descriptor (including all of
*          the nested descriptors has been read).
*
* @return - void
*/
void PARSER_FinalizeParsing
(
    SetupResponseT * currentResponse,
    const XUSB_AddressT address
)
{
    if (currentResponse->configurationValue != 0 &&
        currentResponse->bytesParsed >= currentResponse->descriptorLength)
    {
        DTT_ParseConfigurationDone(address, currentResponse->configurationValue);
    }

    // Ensure the cfg value is 0
    // NOTE: on LG1 the compiler does better optimization here, rather than inside the if statement
    currentResponse->configurationValue = 0;
}

/**
* FUNCTION NAME: parseDeviceDescriptor()
*
* @brief - Parse a Device Descriptor byte
*
* @return - void
*
* @note -   Device descriptors are not part of a configuration descriptor.  This descriptor is the
*           only one returned for this request
*/
static inline void parseDeviceDescriptor
(
    SetupResponseT * currentResponse,   // Structure for keeping track of this setup response
    uint8 currentByte,                  // Curent byte being parsed
    uint8 currentOffset,                // Offset into the device descriptor of this byte
    XUSB_AddressT address               // XUSB address of current device
)
{
    // For the device descriptor, the total size of the transfer is the size of
    // the device descriptor itself.
    currentResponse->descriptorLength = currentResponse->currentDescriptorLength;

    ilog_DESCPARSER_COMPONENT_0(ILOG_DEBUG, PARSE_DEV_DESC);

    // Bytes of interest:
    // 4 is the class,
    // 7 is the max packet size for end point 0 (LG SW refers to this as frame size),
    // 8 is the first byte of the vendor ID
    if (currentOffset == OFFSET_OF(struct DeviceDescriptor, bcdUSB[0]))
    {
        currentResponse->data.devDesc.version0 = currentByte;
    }
    else if (currentOffset == OFFSET_OF(struct DeviceDescriptor, bcdUSB[1]))
    {
        // From USB 2.0 standard, section 9.6.1, paragraph 4:
        // If usbVersion is 0xJJMN, then version is JJ.M.N where:
        // JJ   - major version number
        // M    - minor version number
        // N    - sub-minor version number
        const uint8 major = currentByte;
        const uint8 minor = currentResponse->data.devDesc.version0 >> 4;
        const uint8 subMinor = currentResponse->data.devDesc.version0 & 0x0F;

        ilog_DESCPARSER_COMPONENT_3(ILOG_MAJOR_EVENT, USB_VERSION, major, minor, subMinor);
    }
    else if (currentOffset == OFFSET_OF(struct DeviceDescriptor, bDeviceClass))
    {
        // Do nothing.  Lots of devices only declare class information in the interface descriptor
        // So this field is somewhat useless
    }
    else if (currentOffset == OFFSET_OF(struct DeviceDescriptor, bMaxPacketSize0))
    {
        DTT_SetMaxPacketSizeEndpoint0(address, currentByte);
    }
    else if (currentOffset == (OFFSET_OF(struct DeviceDescriptor, idVendor[0])))
    {
        currentResponse->data.devDesc.idVendor0 = currentByte;
    }
    else if (currentOffset == (OFFSET_OF(struct DeviceDescriptor, idVendor[1])))
    {
        const uint16 idVendor = currentResponse->data.devDesc.idVendor0 + (currentByte << 8);

        DTT_CheckAndSetVendorId(address, idVendor);
    }
    else if (currentOffset == (OFFSET_OF(struct DeviceDescriptor, idProduct[0])))
    {
        currentResponse->data.devDesc.idProduct0 = currentByte;
    }
    else if (currentOffset == (OFFSET_OF(struct DeviceDescriptor, idProduct[1])))
    {
        const uint16 idProduct = currentResponse->data.devDesc.idProduct0 + (currentByte << 8);

        DTT_CheckAndSetProductId(address, idProduct);
    }
    else if (currentOffset == (OFFSET_OF(struct DeviceDescriptor, bNumConfigs)))
    {
        DTT_SetNumConfigurations(address, currentByte);
    }
}

/**
* FUNCTION NAME: parseConfigurationDescriptor()
*
* @brief - Parse a Configuration Descriptor byte
*
* @return - void
*
* @note - Configuration descriptors have multiple interface and endpoint descriptors following.
*/
static inline void parseConfigurationDescriptor
(
    SetupResponseT * currentResponse,   // Structure for keeping track of this setup response
    uint8 currentByte,                  // Curent byte being parsed
    uint8 currentOffset                 // Offset into the device descriptor of this byte
)
{
    ilog_DESCPARSER_COMPONENT_0(ILOG_DEBUG, PARSE_CFG_DESC);

    // Bytes of interest: 2 & 3 form wTotalLength
    // Since this spans 2 bytes, we need to record the first byte before wTotalLength can be updated
    if (currentOffset == OFFSET_OF(struct ConfigDescriptor, wTotalLength[0]))
    {
        currentResponse->data.cfgDesc.wTotalLengthByte0 = currentByte;
    }
    else if (currentOffset == OFFSET_OF(struct ConfigDescriptor, wTotalLength[1]))
    {
        const uint16 wTotalLength =
            (currentResponse->data.cfgDesc.wTotalLengthByte0) + (currentByte << 8);

        // Record the total length of the descriptor, including the interface and end point
        // descriptors.
        currentResponse->descriptorLength = wTotalLength;

        ilog_DESCPARSER_COMPONENT_2(
            ILOG_DEBUG, PARSE_CFG_DESC_LENGTHS, wTotalLength, currentResponse->descriptorLength);
   
    }
    else if (currentOffset == OFFSET_OF(struct ConfigDescriptor, bConfigurationVal))
    {
        currentResponse->configurationValue = currentByte;
    }
}


/**
* FUNCTION NAME: parseInterfaceDescriptor()
*
* @brief - Parse an Interface Descriptor byte
*
* @return - void
*
* @note - Interface descriptors are a part of a configuration request
*/
static inline void parseInterfaceDescriptor
(
    SetupResponseT * currentResponse,       // Structure for keeping track of this setup response
    uint8 currentByte,                      // Curent byte being parsed
    uint8 currentOffset,                    // Offset into the device descriptor of this byte
    XUSB_AddressT address                   // XUSB address, duh
)
{
    ilog_DESCPARSER_COMPONENT_0(ILOG_DEBUG, PARSE_INTF_DESC);

    if (currentOffset == OFFSET_OF(struct InterfaceDescriptor, bInterfaceNumber))
    {
        currentResponse->interfaceNumber = currentByte;
    }
    else if (currentOffset == OFFSET_OF(struct InterfaceDescriptor, bAlternateSetting))
    {
        currentResponse->alternateSetting = currentByte;
    }
    // Bytes of interest: 5 is the class of this interface
    else if (currentOffset == OFFSET_OF(struct InterfaceDescriptor, bInterfaceClass))
    {
        if (currentByte == CLASS_HUB)
        {
            DTT_SetHub(address);
        }

        // Set the interface class type
        currentResponse->interfaceClass = currentByte;
    }
    else if (currentOffset == OFFSET_OF(struct InterfaceDescriptor, bInterfaceProtocol))
    {
        // Clear all MSA state tracking
        currentResponse->MSA.raw = 0;

        if (currentResponse->interfaceClass == CLASS_MASS_STORAGE &&
            currentByte == INTF_PROTOCOL_MASS_STORAGE_BULK_ONLY)
        {
            uint64 rexCapabilities;

            // mass storage acceleration interface found!
            ilog_DESCPARSER_COMPONENT_1(
                ILOG_DEBUG, MASS_STORAGE_BULK_ONLY_INTF_FOUND, XCSR_getXUSBAddrLogical(address));

            rexCapabilities = LINKMGR_getRexLinkOptions(XCSR_getXUSBAddrVPort(address));

            currentResponse->MSA.inProgress = msaEnabled && (rexCapabilities & (1 << TOPLEVEL_SUPPORT_MSA_OFFSET) ); // only enable if MSA is Enabled
        }
    }
}


/**
* FUNCTION NAME: parseEndpointDescriptor()
*
* @brief - Parse an Endpoint Descriptor byte
*
* @return - void
*
* @note - Endpoint descriptors are part of a configuration request
*/
static inline void parseEndpointDescriptor
(
    SetupResponseT * currentResponse,       // Structure for keeping track of this setup response
    uint8 currentByte,                      // Curent byte being parsed
    uint8 currentOffset,                    // Offset into the device descriptor of this byte
    XUSB_AddressT address                   // XUSB address, duh
)
{
    ilog_DESCPARSER_COMPONENT_0(ILOG_DEBUG, PARSE_EP_DESC);

    // Bytes of interest: 2 is the endpoint address, 3 is the endpoint attributes
    if (currentOffset == OFFSET_OF(struct EndpointDescriptor, bEndpointAddress))
    {
        // The only thing we can do is store this until we are ready to configure the endpoint
        currentResponse->data.endpointDesc.bEndpointAddress = currentByte;

    }
    else if (currentOffset == OFFSET_OF(struct EndpointDescriptor, bmAttributes))
    {
        const uint8 epNum =
            currentResponse->data.endpointDesc.bEndpointAddress & ENDPOINT_NUMBER_MASK;
        const enum EndpointDirection epDir =
            currentResponse->data.endpointDesc.bEndpointAddress >> ENDPOINT_DIRECTION_OFFSET;
        const enum EndpointTransferType epType = (currentByte & DESC_TRANSFER_TYPE_MASK);
        const boolT blockEndpoint = shouldEndpointBeBlocked(currentResponse, epType, address);

        // Configure the endpoint
        ilog_DESCPARSER_COMPONENT_0(ILOG_DEBUG, PARSE_EP_DESC_UPDATING);

        // Setup mass storage acceleration, if this is a mass storage bulk only device
        if (currentResponse->MSA.inProgress && epType == EP_TYPE_BULK && !blockEndpoint)
        {
            if (currentResponse->MSA.firstEndpointNum == 0)
            {
                // This is the first end point to be parsed
                currentResponse->MSA.firstEndpointNum = epNum;
                currentResponse->MSA.firstEndpointDir = epDir;
            }
            else
            {
                // This is the second end point to be parsed

                // Ensure the directions of the 2 endpoints are opposite.
                if (epDir == currentResponse->MSA.firstEndpointDir)
                {
                    // The endpoints are in the same direction.  Endpoints will be treated as
                    // non-MSA.
                    DTT_WriteEndpointData(
                            address,
                            currentResponse->configurationValue,
                            currentResponse->interfaceNumber,
                            currentResponse->alternateSetting,
                            currentResponse->MSA.firstEndpointNum,
                            EP_TYPE_BULK,
                            currentResponse->MSA.firstEndpointDir,
                            blockEndpoint);

                    DTT_WriteEndpointData(
                            address,
                            currentResponse->configurationValue,
                            currentResponse->interfaceNumber,
                            currentResponse->alternateSetting,
                            epNum,
                            EP_TYPE_BULK,
                            epDir,
                            blockEndpoint);

                    ilog_DESCPARSER_COMPONENT_3(
                        ILOG_MAJOR_ERROR,
                        MSA_REJ_EP_SAME_DIR,
                        XCSR_getXUSBAddrLogical(address),
                        currentResponse->MSA.firstEndpointNum,
                        epNum);
                }
                else
                {
                    // we have 2 valid endpoints, both bulk, different directions.  Good to go
                    uint8 epInNum;
                    uint8 epOutNum;

                    if (epDir == EP_DIRECTION_IN)
                    {
                        epInNum = epNum;
                        epOutNum = currentResponse->MSA.firstEndpointNum;
                    }
                    else
                    {
                        epInNum = currentResponse->MSA.firstEndpointNum;
                        epOutNum = epNum;
                    }

                    ilog_DESCPARSER_COMPONENT_3(
                        ILOG_DEBUG,
                        MSA_GOOD_TO_GO,
                        XCSR_getXUSBAddrLogical(address),
                        epInNum,
                        epOutNum);

                    DTT_AddNewMsaPair(
                        address,
                        currentResponse->configurationValue,
                        currentResponse->interfaceNumber,
                        currentResponse->alternateSetting,
                        epInNum,
                        epOutNum);
                }
            }
        }
        else
        {
            DTT_WriteEndpointData(
                    address,
                    currentResponse->configurationValue,
                    currentResponse->interfaceNumber,
                    currentResponse->alternateSetting,
                    epNum,
                    epType,
                    epDir,
                    blockEndpoint);
        }
    }
}


/**
* FUNCTION NAME: shouldEndpointBeBlocked()
*
* @brief  - Checks to see if the endpoint that is currently being processed by the descriptor
*           parser should be blocked from working based on the endpoint filtering settings.
*
* @return - TRUE if the endpoint should be blocked from use or FALSE otherwise.
*/
static inline boolT shouldEndpointBeBlocked(
    const SetupResponseT* currentResponse,
    enum EndpointTransferType endpointType,
    XUSB_AddressT address)
{
    boolT allowIso;
    boolT blockMassStorage;
    boolT blockAllButHidAndHub;
    boolT blockAllButHidHubAndSmartCard;
    boolT blockAllButAudioAndVendorSpecific;
    const uint8 vport = XCSR_getXUSBAddrVPort(address);
    const union STORAGE_VariableData* configBits = STORAGE_varGet(CONFIGURATION_BITS);
    uint64 rexCapabilities;

    rexCapabilities = LINKMGR_getRexLinkOptions(vport);

    allowIso = ( (rexCapabilities & configBits->doubleWord) & (1 << TOPLEVEL_ALLOW_ISO_DEVICES_OFFSET)) ? true : false;

    blockMassStorage =
            ( (rexCapabilities | configBits->doubleWord) & (1 << TOPLEVEL_BLOCK_MASS_STORAGE_OFFSET)) ? true : false;

    blockAllButHidAndHub =
            ( (rexCapabilities | configBits->doubleWord) & (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_AND_HUB_OFFSET)) ? true : false;

    blockAllButHidHubAndSmartCard =
            ( (rexCapabilities | configBits->doubleWord) & (1 << TOPLEVEL_BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD_OFFSET)) ? true : false;

    blockAllButAudioAndVendorSpecific =
            ( (rexCapabilities | configBits->doubleWord) & (1 << TOPLEVEL_BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC_OFFSET)) ? true : false;

    const boolT blockEndpoint =
                (blockAllButHidAndHub
                    &&  currentResponse->interfaceClass != CLASS_HUB
                    &&  currentResponse->interfaceClass != CLASS_HID)
                ||
                (blockAllButHidHubAndSmartCard
                    &&  currentResponse->interfaceClass != CLASS_HUB
                    &&  currentResponse->interfaceClass != CLASS_HID
                    &&  currentResponse->interfaceClass != CLASS_SMARTCARD)
                ||
                (blockAllButAudioAndVendorSpecific
                    &&  currentResponse->interfaceClass != CLASS_HUB
                    &&  currentResponse->interfaceClass != CLASS_AUDIO
                    &&  currentResponse->interfaceClass != CLASS_VENDOR_SPECIFIC)
        ||  (!allowIso && endpointType == EP_TYPE_ISO)
        ||  (blockMassStorage && currentResponse->interfaceClass == CLASS_MASS_STORAGE);
    return blockEndpoint;
}


/**
* FUNCTION NAME: parseUnknownDescriptor()
*
* @brief - Parse a byte of an unknown descriptor type.  Used to read the initial bytes that define
*          the type and size.
*
* @return - void
*/
static inline void parseUnknownDescriptor
(
    SetupResponseT * currentResponse,   // Structure for keeping track of this setup response
    uint8 currentByte,                  // Curent byte being parsed
    uint8 currentOffset                 // Offset into the device descriptor of this byte
)
{
    ilog_DESCPARSER_COMPONENT_0(ILOG_DEBUG, PARSE_UNKNOWN_DESC);

    // Bytes of interest: 0 is the descriptor length, 1 is the descriptor type
    if (currentOffset == OFFSET_OF(struct BaseDescriptor, bLength))
    {
        currentResponse->currentDescriptorLength = currentByte;
    }
    else if (currentOffset == OFFSET_OF(struct BaseDescriptor, bDescriptorType))
    {
        ilog_DESCPARSER_COMPONENT_1(ILOG_DEBUG, PARSE_UNKOWN_DESC_TYPE, currentByte);
        currentResponse->currentDescriptorType = currentByte;
    }
    else
    {
        // One of the descriptor types that is a "don't care" is getting processed
    }
}

