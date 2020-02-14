///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2011, 2012, 2013
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
//!   @file  -  vhub_usb_messaging.c
//
//!   @brief -  Contains functions for handling all USB data transactions
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "vhub_loc.h"
#include <usbdefs.h>

/************************ Defined Constants and Macros ***********************/
#define BMAXPACKETSIZE0 (64)


enum hubRecipient
{
    RECIPIENT_HUB,
    RECIPIENT_PORT = 3
};

#define GET_HUB_RECIPIENT(requestType) (requestType & 0x3)

// Hub class feature selectors
enum hubFeatures
{
    C_HUB_LOCAL_POWER,
    C_HUB_OVER_CURRENT
};

//these pointer assignments are repeated everywhere:
#define SETUP_IN_HANDLERS(pVF) do {                                         \
    struct DTT_VF_EndPointHandle * x = (pVF);                               \
    x->out      =   &vhubInTransaction_StatusPhase_ZeroLengthOutHandler;    \
    x->in       =   &vhubInTransaction_DataPhase_InHandler_EP0;             \
    x->inAck    =   &vhubInAckHandler;                                      \
} while (FALSE)

#define SETUP_OUT_HANDLERS(pVF) do {                                        \
    struct DTT_VF_EndPointHandle * x = (pVF);                               \
    x->in       =   &vhubOutTransaction_StatusPhase_InHandler;              \
    x->inAck    =   &vhubOutTransaction_StatusPhase_InAckHandler;           \
    /* All hub OUT requests are for zero length data phases*/               \
    /* Also the default is already a stall, no need to change again */      \
    /* x->out   =   &DTT_VFSendCtrlStallOut; */                             \
} while (FALSE)

#define SETUP_RESET_HANDLERS(pVF) do {                                      \
    struct DTT_VF_EndPointHandle * x = (pVF);                               \
    x->in       =   &DTT_VFSendCtrlStallIn;                                 \
    x->inAck    =   NULL;                                                   \
    x->out      =   &DTT_VFSendCtrlStallOut;                                \
} while (FALSE)


/******************************** Data Types *********************************/


// Convenient wrapper struct to provide a contiguous block of the vhub descriptor topology as shown
// in USB 2.0 spec 9.4.3:
// config + interface 1 + endpoints for interface 1 + ... + interface N + endpoints for interface N
struct TotalConfigDescriptor {
    struct ConfigDescriptor ConfigDesc;
    struct InterfaceDescriptor InterfaceDesc;
    struct EndpointDescriptor endpointDesc;
} __attribute__((__packed__));

//USB 2.0 spec 11.23.2.1, table 11-13, Hub Descriptor
struct HubDescriptor {
    uint8  bDescLength;         // Number of bytes in this descriptor, including this bye
    uint8  bDescriptorType;     // Descriptor Type, 0x29 for hub descriptor
    uint8  bNbrPorts;           // Number of downstream facing ports
    uint16 wHubCharacteristics; // See table 11-13
    uint8  bPwrOn2PwrGood;      // # of 2ms intervals from power on sequence until power good
    uint8  bHubContrCurrent;    // Maximum current requirements in mA
    // TODO: This is initialized to 0, but never used.  Furthermore, the comment leads me to
    //       believe that the array size is not quite correct because bit0 is reserved.
    uint8  DeviceRemovable[VHUB_BIT_SIZE_IN_BYTES]; //bits (0=removable, 1=non), bit0 reserved, bit1 = port1..bitn = portn
    // TODO: It seems like this array is initialized to 0xFF, but never used
    uint8 PortPwrCtrlMask[VHUB_BIT_SIZE_IN_BYTES];
} __attribute__((__packed__));


/************************ Local Function Prototypes **************************/
//TODO: move all local functions to the end of file
static void VhubPortResetTimerComplete(void);
static void VhubPortResumeTimerComplete(void);

/**
 * upstream helper functions
 */
static void writeData(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    boolT toggle); //TODO: __attribute__((section(".lextext")));

/*
 * Setup
 */
static void vhubSetupHandler(
    struct DTT_VF_EndPointHandle *pVF,
    uint8 bRequestType,
    uint8 bRequest,
    uint16 wValue,
    uint16 wIndex,
    uint16 wLength,
    XUSB_AddressT address); //TODO: __attribute__((section(".lextext")));

/*
 * Data
 */
static void vhubInTransaction_DataPhase_InHandler_EP1(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    boolT toggle) __attribute__((section(".lextext")));
static void vhubInTransaction_DataPhase_InHandler_EP0(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    boolT toggle); //TODO: __attribute__((section(".lextext")));
static void vhubInAckHandler(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address); //TODO: __attribute__((section(".lextext")));

/*
 * Status
 */
static void vhubInTransaction_StatusPhase_ZeroLengthOutHandler(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    uint8 pid,
    uint8 * data,
    uint16 dataSize,
    boolT toggle); //TODO: __attribute__((section(".lextext")));
static void vhubOutTransaction_StatusPhase_InHandler(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    boolT toggle); //TODO: __attribute__((section(".lextext")));
static void vhubOutTransaction_StatusPhase_InAckHandler(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address); //TODO: __attribute__((section(".lextext")));


/***************************** Global Variables ******************************/

/**
 * HOOKS INTO VHUB FOR EXTERNAL USE
 */
struct DTT_VF_EndPointHandles vhub_vf =
{
    .numOfEndpoints = 2,
    .endpoint = {
        {
            .epType = EP_TYPE_CTRL,
            .epMaxPacketSize = BMAXPACKETSIZE0,
            .setup = &vhubSetupHandler
        },
        {
            .epType = EP_TYPE_INT,
            .epMaxPacketSize = VHUB_BIT_SIZE_IN_BYTES,
            .in = &vhubInTransaction_DataPhase_InHandler_EP1,
            .inAck = &vhubInAckHandler
        }
    }
};


/***************************** Local Variables *******************************/

static struct HubDescriptor vhubHubDescriptor =
{
    .bDescLength = sizeof(struct HubDescriptor),
    .bDescriptorType = HUB_DESC,
    .bNbrPorts = VHUB_NUM_PORTS,
    .wHubCharacteristics = HOST_ENDIAN_TO_USB_16_UNSAFE(
            0x0011),        // individual port power,
                            // hub is not part of compound dev,
                            // no over-current protection,
                            // TT requires at most 8 FS bit times,
                            // no port indicator support
    .bPwrOn2PwrGood = 0x32, //100ms, see USB2.0 spec, 11.11 last sentence
    .bHubContrCurrent = 0x64, //100mA
    //.DeviceRemovable[] = 0x0, // all Rex's are removable
    .PortPwrCtrlMask[0] = 0xFF,
#if (NUM_OF_VPORTS > 8)
    .PortPwrCtrlMask[1] = 0xFF,
#endif
};

static struct DeviceQualifier vhubDeviceQualifier =
{
    // NOTE: This is basically a device descriptor for the device's other speed capability
    .bLength = sizeof(struct DeviceQualifier),
    .bDescriptorType = DEVICE_QUALIFIER_DESC,
    .bcdUSB = {0x00, 0x02},                     // 2.0.0 in little endian bcd
    .bDeviceClass = 0x9,                        //hub classcode
    .bDeviceSubClass = 0,                       //don't have one
    .bDeviceProtocol = 1,                       //In high speed 1 for single TT //In full speed 0
    .bMaxPacketSize0 = BMAXPACKETSIZE0,         //64 for high speed
    .bNumConfigs = 0x1,                         //no other-speed configs
    .bReserved = 0
};

static struct DeviceDescriptor vhubDeviceDescriptor =
{
    .bLength = sizeof(struct DeviceDescriptor),
    .bDescriptorType = DEVICE_DESC,
    .bcdUSB = {0x00, 0x02},             // 2.0.0 in little endian bcd
    .bDeviceClass = 0x9,                //hub classcode
    .bDeviceSubClass = 0,               //don't have one
    .bDeviceProtocol = 1,               //In high speed 1 for single TT //In full speed 0
    .bMaxPacketSize0 = BMAXPACKETSIZE0, //64 for high speed
    .idVendor =  { VHUB_VENDOR_ID & 0xFF,  VHUB_VENDOR_ID >> 8 },   //vendor ID: LSB, then MSB for USB LE format
    .idProduct = { VHUB_PRODUCT_ID & 0xFF, VHUB_PRODUCT_ID >> 8 },  //product ID: LSB, then MSB for USB LE format
    .bcdDevice = {
#if SOFTWARE_MAJOR_REVISION == DEVELOPMENT_BUILD
            0x00, 0x00 //  Sandbox build
#else
#if (SOFTWARE_MAJOR_REVISION  > 99) || (SOFTWARE_MINOR_REVISION > 9) || (SOFTWARE_DEBUG_REVISION > 9)
#error "Unable to fit SW revision into VHub USB Device Descriptor bcdDevice in XX.Y.Z format, IE (0-99).(0-9).(0-9)"
#endif
        ((SOFTWARE_MINOR_REVISION & 0xF) << 4) | (SOFTWARE_DEBUG_REVISION & 0xF),
        SOFTWARE_MAJOR_REVISION
#endif
    },
    .iManufacturer = 0,                 //don't have one
    .iProduct = 0,                      //don't have one
    .iSerialNumber = 0,                 //don't have one
    .bNumConfigs = 0x1                  //only 1 configuration for vhub
};

//getConfigurationDescriptor will request config, interface, and endpoint descriptors
//together
static struct TotalConfigDescriptor vhubConfigDescriptor =
{
    .ConfigDesc =
    {
        .bLength = sizeof(struct ConfigDescriptor),
        .bDescriptorType = CONFIG_DESC, // NOTE: this changes between CONFIG & OTHER_SPEED_CONFIG
        .wTotalLength =                 //length, little endian
        {
            (sizeof(struct TotalConfigDescriptor) >> 0) & 0xFF, // LSB
            (sizeof(struct TotalConfigDescriptor) >> 8) & 0xFF  // MSB
        },
        .bNumInterfaces = 1,            //only 1 interface on vhub
        .bConfigurationVal = 1,         //the one config is config 1
        .iConfiguration = 0,            //don't have one
        .bmAttributes = 0xe0,           //Lex is capable of both self- and bus-powered if bMaxPower > 0; remote wakeup
        .bMaxPower = 0x32               //say, 100ma to be like a normal hub
    },
    .InterfaceDesc =
    {
        .bLength = sizeof(struct InterfaceDescriptor),
        .bDescriptorType = INTF_DESC,
        .bInterfaceNumber = 0,          //this interface is interface 0
        .bAlternateSetting = 0,         //use 0 to select this interface
        .bNumEndpoints = 1,             //the interrupt endpoint
        .bInterfaceClass = CLASS_HUB,
        .bInterfaceSubClass = 0,        //don't have one
        .bInterfaceProtocol = 0,        //single TT & also 0 for full speed hub
        .iInterface = 0                 //don't have one
    },
    .endpointDesc =
    {
        .bLength             = sizeof(struct EndpointDescriptor),
        .bDescriptorType     = ENDPOINT_DESC, //type: ENDPOINT
        .bEndpointAddress    = 0x81,       //IN endpoint, number 1
        .bmAttributes        = 0x3,        //data endpoint, no synchronization, interrupt type
        .wMaxPacketSize      = { VHUB_BIT_SIZE_IN_BYTES, 0x00 },
        //polling interval, high speed interrupt 1-16, chose same as NEC hub (0xc)
        .bInterval           = 0xc,
    }
};


/************************** Function Definitions *****************************/
/**
* FUNCTION NAME: VhubPortResetTimerComplete()
*
* @brief  - Change the status of the port after port reset timer expires
*
* @return - void
*
* @note   -
*
*/
void VhubPortResetTimerComplete(void)
{
    uint8 i;
    for(i = 1; i <= VHUB_NUM_PORTS; i++)
    {
        if (vhub.portInReset & (1 << i))
        {
            // Note that due to the way values are assigned in the UsbSpeed enum,
            // this MAX is actually finding the slower of the two speeds
            const uint8 speed = MAX(vhub.portInfo[i].speed, vhub.linkSpeed);

            XUSB_AddressT parent_address = DTT_GetAddressFromVF(&vhub_vf);

            if(VHUB_IsPortEnabled(i))
            {
                XCSR_XICSSendMessage(
                        USB_UPSTREAM_STATE_CHANGE,
                        UPSTREAM_BUS_RESET_DONE,
                        i);
                DEVMGR_ProcessPortResetRequest(
                        parent_address,
                        i,
                        i);
                DTT_SetOperatingSpeed(
                        parent_address,
                        i,
                        speed);
            }

            vhub.portInfo[i].usbCh11Status.status = 0;
            vhub.portInfo[i].usbCh11Status.change = 0;

            VHUB_SET_PORT_STATUS_BIT(i, VHUB_PSTATUS_ENABLE);
            VHUB_SET_PORT_STATUS_BIT(i, VHUB_PSTATUS_CONNECTION);
            VHUB_SET_PORT_STATUS_BIT(i, VHUB_PSTATUS_POWER);

            if(speed == USB_SPEED_LOW)
            {
                VHUB_SET_PORT_STATUS_BIT(i, VHUB_PSTATUS_LOW_SPEED);
            }
            if(speed == USB_SPEED_HIGH)
            {
                VHUB_SET_PORT_STATUS_BIT(i, VHUB_PSTATUS_HIGH_SPEED);
            }

            VHUB_SET_PORT_CHANGE_BIT(i, VHUB_PCHANGE_RESET);

            vhub.portInfo[i].state = OPERATING;

            VHUB_SET_INTEP1_DATA_BIT(i);
        }
    }

    vhub.portInReset = 0;
}

/**
* FUNCTION NAME: VhubPortResumeTimerComplete()
*
* @brief  - Change the status of the port after port has resumed
*
* @return - void
*
* @note   -
*
*/
void VhubPortResumeTimerComplete(void)
{
    uint8 i;
    for(i = 1; i <= VHUB_NUM_PORTS; i++)
    {
        if (vhub.portInResume & (1 << i))
        {
            VHUB_FinishResumePort(i);
        }
    }

    vhub.portInResume = 0;
}

/**
* FUNCTION NAME: VHUB_Reset()
*
* @brief  - Go to Powered-off state on reception of Set Configuration
*
* @return - void
*
* @note   -
*
*/
void VHUB_Reset(void)
{
    uint8 i;
    uint8 j;

    //stop port reset timer if one is in progress
    TIMING_TimerStop(vhub.portResetTimerCompleteHandle);
    vhub.portInReset = 0;

    //stop port resume timer if one is in progress
    TIMING_TimerStop(vhub.portResumeTimerCompleteHandle);
    vhub.portInResume = 0;

    // hub
    vhub.portInfo[VHUB_HUB_STATUS_INDEX].usbCh11Status.status = 0;
    vhub.portInfo[VHUB_HUB_STATUS_INDEX].usbCh11Status.change = 0;

    // According to section 9.1.1.6 of the USB 2.0 spec, when a device
    // is reset, remote wakeup signaling must be disabled
    vhub.remoteWakeupEnabled = FALSE;

    // ports
    for(i = 1; i <= VHUB_NUM_PORTS; i++)
    {
        // default: power off, port disabled
        vhub.portInfo[i].usbCh11Status.status = 0;
        vhub.portInfo[i].usbCh11Status.change = 0;
    }

    for(j = 0; j < VHUB_BIT_SIZE_IN_BYTES; j++)
    {
        vhub.intEp1.hubAndPortStatusChangeMap[j] = 0;
    }

    for(i = 1; i <= VHUB_NUM_PORTS; i++)
    {
        if(VHUB_IsPortEnabled(i))
        {
            XCSR_XICSSendMessage(USB_UPSTREAM_STATE_CHANGE, UPSTREAM_DISCONNECT, i);
            VHUB_DisConnectDev(i);
        }
    }

    VHUB_InitPortManager();

    vhub_vf.endpoint[1].state.currentRequestData     = NULL;
    vhub_vf.endpoint[1].state.currentRequestDataLeft = 0;
    vhub_vf.endpoint[1].state.pid0Next               = TRUE;
    vhub_vf.endpoint[1].state.current_wIndex         = 0;
}

void VHUB_Init(uint8 numPorts, uint16 vendorId, uint16 productId)
{
    // portInReset and portInResume are uint8's with each bit representing each vport
    // being reset and resumed, respectively. Bit 0 is unused.
    // If number of vports exceed 7, we need to create an array of uint8's of size 2 where
    // array index 0 represents ports 1 to 7 and index 1 represents ports 8 to 15.
#if VHUB_NUM_PORTS > 7
#error "Number of Vports exceed 7; need to change the portInReset/Resume variable to an array of uint8"
#endif

    // According to section 11.5.1.10 of the USB 2.0 specification, a single timer
    // can be used for resetting and resuming intervals, but only up to 10 ports.
#if VHUB_NUM_PORTS > 10
#error "Number of Vports exceed 10; Vhub needs per-port Resume and Reset timers"
#endif

    vhub.portResetTimerCompleteHandle =
        TIMING_TimerRegisterHandler(&VhubPortResetTimerComplete, FALSE, 10);

    vhub.portResumeTimerCompleteHandle = 
        TIMING_TimerRegisterHandler(&VhubPortResumeTimerComplete, FALSE, 20);

    vhub.linkState = DISCONNECTED;
    vhub.linkSpeed = USB_SPEED_INVALID;
    vhub.currentConfiguration = 0;
    vhub.remoteWakeupEnabled = FALSE;

    // Set up VID, PID and number of Vports
    vhubHubDescriptor.bNbrPorts = numPorts;
    // The VID and PID are stored in USB byte order (little endian) in the hub descriptor.
    vhubDeviceDescriptor.idVendor[0] = vendorId >> 8;
    vhubDeviceDescriptor.idVendor[1] = vendorId;
    vhubDeviceDescriptor.idProduct[0] = productId >> 8;
    vhubDeviceDescriptor.idProduct[1] = productId;

    VHUB_InitPortManager();

    LEX_UlmMessageHandler(DOWNSTREAM_CONNECT_HS);
}

/*****************************************************************************
 * UPSTREAM HELPER FUNCTIONS
 *****************************************************************************/

// TODO: Should this be moved to the DTT_ component?
static void writeData
(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    boolT toggle
)
{
    struct XCSR_XICSQueueFrame* frame;
    // Set the data size for a short packet, or normal full packet
    const uint8 dataSize = MIN(pVF->state.currentRequestDataLeft, pVF->epMaxPacketSize);
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

    // create In response
    frame->dataSize = 1 + dataSize; // PID + data
    XCSR_XICSBuildUpstreamFrameHeader(
            frame,
            address,
            endpoint,
            0,
            frame->dataSize,
            XUSB_IN,
            (pVF->state.pid0Next ? XUSB_DATA0 : XUSB_DATA1),
            pVF->epType,
            toggle);
    ((struct XCSR_XICSQueueFrameUsbData*)frame->data)->pid =
        pVF->state.pid0Next ? DATA0_PID : DATA1_PID;
    memcpy(
            ((struct XCSR_XICSQueueFrameUsbData*)frame->data)->contents,
            pVF->state.currentRequestData,
            dataSize);

    ilog_VHUB_COMPONENT_2(
            ILOG_DEBUG, VHUB_IN_DESC_HANDLER, frame->dataSize, (uint32)pVF->state.currentRequestData);

    // Update globals
    pVF->state.currentRequestData += dataSize;
    pVF->state.currentRequestDataLeft -= dataSize;

    // Send the frame to the host
    XCSR_XUSBSendUpstreamUSBFrame(address, endpoint, TRUE, frame);
}


/*****************************************************************************
 * IN/OUT TRANSACTION HANDLERS FOR ENDPOINT 0
 *
 * TYPICAL ASYNC TRANSACTION BEHAVIORS ON ENDPOINT 0, **'s show the handlers connected here (or
 * already connected by default)
 *
 * IN requests:
 *   SETUP - handled externally, setuphandler is responsible for setting up DATA phase handlers
 *    setup ->   parsed before setuphandler (in sys_ctrl_q)
 *    data0 ->   parsed before setuphandler (in sys_ctrl_q)
 *    ack   <-   automatic                  (in sys_ctrl_q)
 *   DATA
 *    in    -> **vhubInTransaction_DataPhase_InHandler_EP0, provides DATA1..N response
 *    dataN <-   see above
 *    ack   -> **vhubInAckHandler
 *   STATUS
 *    out   -> **vhubInTransaction_StatusPhase_ZeroLengthOutHandler, expects 0 data and must ack it
 *    data1 ->   see above
 *    ack   <-   see above
 *
 * OUT requests:
 *  SETUP - handled externally, setuphandler is responsible for setting up DATA phase handlers
 *   setup ->   parsed before setuphandler (in sys_ctrl_q)
 *   data0 ->   parsed before setuphandler (in sys_ctrl_q)
 *   ack   <-   automatic                  (in sys_ctrl_q)
 *  DATA (TODO: DOES NOT EXIST FOR 0 data OUT transactions, if out transactions containing a data
 *              phase are needed, some changes may be required)
 *   out   -> **CRK: typically not used right now due to all supported OUT transactions being 0 data.
 *   dataN ->   see above
 *   ack   <-   send by out handler
 *  STATUS
 *   in    -> **vhubOutTransaction_StatusPhase_InHandler, sends 0 length response to acknowledge
 *   data1 <-   see above
 *   ack   -> **vhubInAckHandler
 *****************************************************************************/
void vhubSetupHandler
(
    struct DTT_VF_EndPointHandle *pVF,
    uint8 bRequestType,
    uint8 bRequest,
    uint16 wValue,
    uint16 wIndex,
    uint16 wLength,
    XUSB_AddressT address
)
{
    boolT inDirection = GET_REQUEST_TYPE_DIR(bRequestType);

    ilog_VHUB_COMPONENT_3(ILOG_DEBUG, VHUB_SETUP_HANDLER, bRequestType, bRequest, wValue);

    // maintain the wIndex info for subsequent in/out transactions as needed
    pVF->state.current_wIndex = wIndex;

    // Setup the Out & In pointers as stall senders, that are overwritten if this setup packet is
    // supported
    SETUP_RESET_HANDLERS(pVF);

    // reset pid toggle
    pVF->state.pid0Next = FALSE;

    if (GET_REQUEST_TYPE_TYPE(bRequestType) == REQUESTTYPE_TYPE_STANDARD)
    {
        switch (bRequest)
        {
            case STD_REQ_GET_STATUS:
                ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_STD_REQ_GET_STATUS);
                if ((!inDirection) || (wLength != 2) || (wValue != 0))
                {
                    // send a STALL
                }
                else
                {
                    switch(GET_REQUEST_TYPE_REC(bRequestType))
                    {
                        case REQUESTTYPE_REC_DEVICE:
                            {
                                const uint16 device = wIndex;
                                if(device == 0)
                                {
                                    SETUP_IN_HANDLERS(pVF);
                                    // Rex is self-powered and supports remote wakeup
                                    vhub.controlRequestReplyScratchArea.bytes[0] = 0x3;
                                    vhub.controlRequestReplyScratchArea.bytes[1] = 0;
                                    pVF->state.currentRequestData = &vhub.controlRequestReplyScratchArea.bytes[0];
                                    pVF->state.currentRequestDataLeft = 2;
                                    ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_STD_REQ_GET_STATUS_DEV);
                                }
                            }
                            break;
                        case REQUESTTYPE_REC_INTF:
                            {
                                const uint16 interface = wIndex;
                                if(interface == 0)
                                {
                                    SETUP_IN_HANDLERS(pVF);
                                    vhub.controlRequestReplyScratchArea.hword = 0; // always reply with 0
                                    pVF->state.currentRequestData = &vhub.controlRequestReplyScratchArea.bytes[0];
                                    pVF->state.currentRequestDataLeft = 2;
                                    ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_STD_REQ_GET_STATUS_INT);
                                }
                            }
                            break;
                        case REQUESTTYPE_REC_ENDP:
                            {
                                const uint16 endp = (wIndex & ENDPOINT_NUMBER_MASK);

                                if(endp == 1)
                                {
                                    SETUP_IN_HANDLERS(pVF);
                                    vhub.controlRequestReplyScratchArea.bytes[0] = (vhub.intEp1.halted ? 1 : 0);
                                    vhub.controlRequestReplyScratchArea.bytes[1] = 0;
                                    pVF->state.currentRequestData = &vhub.controlRequestReplyScratchArea.bytes[0];
                                    pVF->state.currentRequestDataLeft = 2;
                                    ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_STD_REQ_GET_STATUS_EP);
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
                break;

            case STD_REQ_CLEAR_FEATURE:
                ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_STD_REQ_CLEAR_FEATURE);
                if (inDirection || (wLength != 0))
                {
                    // send a STALL
                }
                else
                {
                    switch(GET_REQUEST_TYPE_REC(bRequestType))
                    {
                        case REQUESTTYPE_REC_DEVICE:
                            if (wValue == DEVICE_REMOTE_WAKEUP)
                            {
                                // disable remote wakeup
                                SETUP_OUT_HANDLERS(pVF);
                                vhub.remoteWakeupEnabled = FALSE;
                            }
                            break;
                        case REQUESTTYPE_REC_ENDP:
                            {
                                const uint8 endp = wIndex & ENDPOINT_NUMBER_MASK;
                                // Clear Interrupt IN endpoint halt
                                if(wValue == ENDPOINT_HALT && endp == 1)
                                {
                                    // 0 data OUT transaction means no data phase, do work here and use
                                    // generic IN zero response handler for status phase
                                    SETUP_OUT_HANDLERS(pVF);
                                    vhub.intEp1.halted = FALSE;
                                    //TODO: stop endpoint from stalling all endpoints.  Post something if applicable
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
                break;

            case STD_REQ_SET_FEATURE:
                ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_STD_REQ_SET_FEATURE);
                if (inDirection || (wLength != 0))
                {
                    // send a STALL
                }
                else
                {
                    switch(GET_REQUEST_TYPE_REC(bRequestType))
                    {
                        case REQUESTTYPE_REC_DEVICE:
                            if(wValue == DEVICE_REMOTE_WAKEUP)
                            {
                                SETUP_OUT_HANDLERS(pVF);
                                vhub.remoteWakeupEnabled = TRUE;
                            }
                            break;

                        case REQUESTTYPE_REC_ENDP:
                            {
                                const uint8 endp = wIndex & ENDPOINT_NUMBER_MASK;
                                // Set Interrupt IN endpoint halt
                                if(wValue == ENDPOINT_HALT && endp == 1)
                                {
                                    SETUP_OUT_HANDLERS(pVF);
                                    vhub.intEp1.halted = TRUE;
                                    //TODO: actually setup endpoint to STALL all packets
                                }
                            }
                            break;

                        default:
                            break;
                    }
                }
                break;

            case STD_REQ_GET_DESCRIPTOR:
                ilog_VHUB_COMPONENT_1(ILOG_MINOR_EVENT, VHUB_STD_REQ_GET_DESC, wValue);
                if (!inDirection)
                {
                    // send a STALL
                }
                else
                {
                    switch (GET_DESC_TYPE_FROM_VALUE(wValue))
                    {
                        case DEVICE_DESCRIPTOR:
                            SETUP_IN_HANDLERS(pVF);
                            // In high speed report the high speed setting of 1
                            // In full speed report the full speed setting of 0
                            vhubDeviceDescriptor.bDeviceProtocol =
                                (vhub.linkSpeed == USB_SPEED_HIGH) ?  1 : 0;
                            pVF->state.currentRequestData = (uint8 *)&vhubDeviceDescriptor;
                            pVF->state.currentRequestDataLeft =
                                MIN(wLength, sizeof(struct DeviceDescriptor));
                            ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_STD_REQ_GET_DESC_DEV);

                            // Device descriptor, this is where the parse would process the device descriptor
                            DTT_CheckAndSetVendorId(address, VHUB_VENDOR_ID);
                            DTT_CheckAndSetProductId(address,  VHUB_PRODUCT_ID);
                            break;

                        case CONFIGURATION_DESCRIPTOR:
                            SETUP_IN_HANDLERS(pVF);
                            // get current speed config (maniuplate anything needed in config
                            // descriptor for current speed before returning it)
                            vhubConfigDescriptor.ConfigDesc.bDescriptorType = CONFIG_DESC; // configuration
                            pVF->state.currentRequestData = (uint8 *)&vhubConfigDescriptor;
                            pVF->state.currentRequestDataLeft =
                                MIN(wLength, sizeof(struct TotalConfigDescriptor));
                            ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_STD_REQ_GET_DESC_CONF);

                            // CFG Descriptor is being read. Inform topology of our configuration, just like the parser would do while parsing configuration descriptors
                            {
                                const uint8 configurationValue = 1;
                                const uint8 interfaceNumber = 0;
                                const uint8 alternateSetting = 0;
                                const uint8 endpointNumber = 1;
                                const uint8 endpointType = EP_TYPE_INT;
                                const uint8 endpointDirection = EP_DIRECTION_IN;
                                const boolT blockAccess = FALSE;
                                // interface descriptor info
                                DTT_SetHub(address); // do this next, as Vendor & Product ID functions could clear the hub bit
                                // endpoint descriptor info
                                DTT_WriteEndpointData(
                                        address,
                                        configurationValue,
                                        interfaceNumber,
                                        alternateSetting,
                                        endpointNumber,
                                        endpointType,
                                        endpointDirection,
                                        blockAccess);
                            }

                            break;
                        case STRING_DESCRIPTOR:
                            // no strings are supported (yet); SETUP_IN_HANDLERS(pVF);
                            break;
                        case DEVICE_QUALIFIER_DESCRIPTOR:
                            SETUP_IN_HANDLERS(pVF);
                            // In high speed report the full speed setting of 0
                            // In full speed report the high speed setting of 1
                            vhubDeviceQualifier.bDeviceProtocol =
                                (vhub.linkSpeed == USB_SPEED_HIGH) ?  0 : 1;
                            pVF->state.currentRequestData = (uint8 *)&vhubDeviceQualifier;
                            pVF->state.currentRequestDataLeft =
                                MIN(wLength, sizeof(struct DeviceQualifier));
                            ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_STD_REQ_GET_DESC_DEV_QUAL);
                            break;
                        case OTHER_SPEED_CONFIGURATION_DESCRIPTOR:
                            SETUP_IN_HANDLERS(pVF);
                            // get other speed config (maniuplate anything needed in config descriptor
                            // for other speed before returning it)
                            vhubConfigDescriptor.ConfigDesc.bDescriptorType = OTHER_SPEED_CONFIG_DESC; //other speed config
                            pVF->state.currentRequestData = (uint8 *)&vhubConfigDescriptor;
                            pVF->state.currentRequestDataLeft =
                                MIN(wLength, sizeof(struct TotalConfigDescriptor));
                            ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_STD_REQ_GET_DESC_OTHER_SPEED);
                            break;
                        default:
                            break;
                    }
                }
                break;

            case STD_REQ_SET_DESCRIPTOR:
                break;

            case STD_REQ_GET_CONFIGURATION:
                if (!inDirection || (wValue != 0) || (wIndex != 0) || (wLength != 1))
                {
                    // send a STALL
                }
                else
                {
                    SETUP_IN_HANDLERS(pVF);
                    pVF->state.currentRequestData = (uint8 *)&vhub.currentConfiguration;
                    pVF->state.currentRequestDataLeft = MIN(wLength, sizeof(uint8));
                    ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_STD_REQ_GET_DEV_CONFIG);
                }
                break;

            case STD_REQ_SET_CONFIGURATION:
                {
                    const uint8 configurationValue = wValue & 0xF;

                    if (inDirection || ((wValue >> 8) != 0) || (wIndex != 0) || (wLength != 0))
                    {
                        // send a STALL
                    }
                    else if (    (configurationValue != 0)
                              && (configurationValue != vhubConfigDescriptor.ConfigDesc.bConfigurationVal))
                    {
                        // send a STALL
                    }
                    else
                    {
                        vhub.currentConfiguration = configurationValue;
                        SETUP_OUT_HANDLERS(pVF);
                        VHUB_Reset();
                        ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_STD_REQ_SET_DEV_CONFIG);
                    }
                }
                break;

            default:
                break;
        }
    }
    else if (GET_REQUEST_TYPE_TYPE(bRequestType) == REQUESTTYPE_TYPE_CLASS)
    {
        switch (bRequest)
        {
            case HUB_REQ_GET_STATUS:
                ilog_VHUB_COMPONENT_0(ILOG_DEBUG, VHUB_HUB_REQ_GET_STATUS);
                if (!inDirection)
                {
                    // send a STALL
                }
                else if (    (GET_HUB_RECIPIENT(bRequestType) == RECIPIENT_HUB)
                          && (wValue == 0)
                          && (wIndex == 0)
                          && (wLength == sizeof(union usbCh11StatusStruct))) //GetHubStatus
                {
                    COMPILE_TIME_ASSERT(4 == sizeof(union usbCh11StatusStruct));
                    // return 2 words, wHubStatus, followed by wHubChange (USB2.0 spec 11.24.2.6,
                    // tables 11-19/20)
                    SETUP_IN_HANDLERS(pVF);
                    pVF->state.currentRequestData = CAST(&vhub.portInfo[VHUB_HUB_STATUS_INDEX].usbCh11Status.raw, uint32 *, uint8 *);
                    pVF->state.currentRequestDataLeft = sizeof(union usbCh11StatusStruct);
                    ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_HUB_REQ_GET_STATUS_HUB);
                }
                else if (    (GET_HUB_RECIPIENT(bRequestType) == RECIPIENT_PORT)
                          && (wValue == 0)
                          && (wIndex <= VHUB_NUM_PORTS)
                          && (wLength == sizeof(union usbCh11StatusStruct))) //GetPortStatus
                {
                    COMPILE_TIME_ASSERT(4 == sizeof(union usbCh11StatusStruct));
                    // return 2 words, wPortStatus, followed by wPortChange (USB2.0 spec 11.24.2.6,
                    // tables 11-19/20)
                    SETUP_IN_HANDLERS(pVF);
                    pVF->state.currentRequestData = CAST(&vhub.portInfo[wIndex].usbCh11Status.raw, uint32 *, uint8 *);
                    pVF->state.currentRequestDataLeft = sizeof(union usbCh11StatusStruct);

                    ilog_VHUB_COMPONENT_1(ILOG_MINOR_EVENT, VHUB_HUB_REQ_GET_STATUS_PORT, wIndex);
                }
                break;

            case HUB_REQ_CLEAR_FEATURE:
                ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_HUB_REQ_CLEAR_FEATURE);
                if (inDirection || (wLength != 0))
                {
                    // send a STALL
                }
                else if ((GET_HUB_RECIPIENT(bRequestType) == RECIPIENT_HUB) && (wIndex == 0))
                {
                    const uint8 feature = wValue;

                    ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_HUB_REQ_CLEAR_FEATURE_HUB);

                    // 11.24.2: Table 11-17: all applicable features
                    // It is not possible for us to report if the local power source is good or
                    // if an over-current condition exists.
                    // We will just reply with an ACK and not perform any actions.
                    switch(feature)
                    {
                        case C_HUB_LOCAL_POWER:
                            SETUP_OUT_HANDLERS(pVF);
                            break;
                        case C_HUB_OVER_CURRENT:
                            SETUP_OUT_HANDLERS(pVF);
                            break;
                        default:
                            break;
                    }
                }
                else if ((GET_HUB_RECIPIENT(bRequestType) == RECIPIENT_PORT) && (wIndex <= VHUB_NUM_PORTS))
                {
                    const uint8 port = wIndex;
                    const uint8 feature = wValue;

                    ilog_VHUB_COMPONENT_1(ILOG_MINOR_EVENT, VHUB_HUB_REQ_CLEAR_FEATURE_PORT, port);

                    // 11.24.2.2: applicable features
                    switch(feature)
                    {
                        case PORT_CONNECTION:
                        case PORT_OVER_CURRENT:
                        case PORT_RESET:
                        case PORT_LOW_SPEED:
                        case PORT_HIGH_SPEED:
                        case PORT_INDICATOR:
                        case C_PORT_OVER_CURRENT:
                            // 11.24.2.7: treat all these requests as functional no-operations
                            SETUP_OUT_HANDLERS(pVF);
                            break;

                        case PORT_ENABLE:
                            // 11.24.2.2: Clearing the PORT_ENABLE feature causes the port to be placed in
                            // the Disabled state. If the port is in the Powered-off state, the hub should
                            // treat this request as a functional no-operation.
                            // 11.5.1.4: From any but the Powered-off, Disconnected, or Not Configured
                            // states on receipt of a ClearPortFeature(PORT_ENABLE) request.
                            if(vhub.portInfo[port].state != DISCONNECTED)
                            {
                                // Not changing per-port ULM state because we don't keep track of
                                // DISABLED state in the ULM. DISABLED state is between DISCONNECTED
                                // and RESETTING states.
                                if(    (vhub.portInfo[port].usbCh11Status.status == 0)
                                    || !(vhub.portInfo[port].usbCh11Status.status & HOST_ENDIAN_TO_USB_16(1 << VHUB_PSTATUS_POWER))
                                    || !(vhub.portInfo[port].usbCh11Status.status & HOST_ENDIAN_TO_USB_16(1 << VHUB_PSTATUS_CONNECTION)))
                                {
                                    // functional no-operation
                                }
                                else
                                {
                                    // Not sending message to REX because port is still powered
                                    // and we are awaiting a BUS reset
                                    VHUB_CLR_PORT_STATUS_BIT(port, VHUB_PSTATUS_ENABLE);
                                }
                            }
                            SETUP_OUT_HANDLERS(pVF);
                            break;

                        case PORT_SUSPEND:
                            VHUB_StartResumePort(port);
                            vhub.portInResume |= (1 << port);
                            TIMING_TimerStart(vhub.portResumeTimerCompleteHandle);
                            SETUP_OUT_HANDLERS(pVF);
                            break;

                        case PORT_POWER:
                            // When Clear_Feature(PORT_POWER) is received, VHUB will send a message
                            // to the REX to disconnect and then, the REX will initiate another
                            // host connect.
                            {
                                XUSB_AddressT parent_address = DTT_GetAddressFromVF(&vhub_vf);

                                ilog_VHUB_COMPONENT_1(ILOG_MAJOR_EVENT, VHUB_DISCONNECT_DEV, port);

                                vhub.portInfo[port].state = DISCONNECTED;
                                DTT_PortDisconnect(parent_address, port);

                                // Clear Port all status bits
                                vhub.portInfo[port].usbCh11Status.status = 0;

                                VHUB_CLR_PORT_CHANGE_BIT(port, VHUB_PCHANGE_CONNECTION);
                                if (vhub.portInfo[port].usbCh11Status.change == 0)
                                {
                                    VHUB_CLR_INTEP1_DATA_BIT(port);
                                }

                                XCSR_XICSSendMessage(USB_UPSTREAM_STATE_CHANGE, UPSTREAM_DISCONNECT, port);
                                SETUP_OUT_HANDLERS(pVF);
                            }
                            break;
                        case C_PORT_CONNECTION:
                        case C_PORT_ENABLE:
                        case C_PORT_SUSPEND:
                        case C_PORT_RESET:
                            switch(feature)
                            {
                                case C_PORT_CONNECTION:

                                    VHUB_CLR_PORT_CHANGE_BIT(port, VHUB_PCHANGE_CONNECTION);
                                    SETUP_OUT_HANDLERS(pVF);
                                    break;
                                case C_PORT_ENABLE:
                                    VHUB_CLR_PORT_CHANGE_BIT(port, VHUB_PCHANGE_ENABLE);
                                    SETUP_OUT_HANDLERS(pVF);
                                    break;
                                case C_PORT_SUSPEND:
                                    VHUB_CLR_PORT_CHANGE_BIT(port, VHUB_PCHANGE_SUSPEND);
                                    SETUP_OUT_HANDLERS(pVF);
                                    break;
                                case C_PORT_RESET:
                                    VHUB_CLR_PORT_CHANGE_BIT(port, VHUB_PCHANGE_RESET);
                                    SETUP_OUT_HANDLERS(pVF);
                                    break;
                                default:
                                    break;
                            }

                            if (vhub.portInfo[port].usbCh11Status.change == 0)
                            {
                                VHUB_CLR_INTEP1_DATA_BIT(port);
                            }
                            break;
                        default:
                            break;
                    }
                }
                break;
            case HUB_REQ_SET_FEATURE:
                if (inDirection || (wLength != 0))
                {
                    // do nothing
                }
                else if ((GET_HUB_RECIPIENT(bRequestType) == RECIPIENT_HUB) && (wIndex == 0))
                {
                    const uint8 feature = wValue;

                    ilog_VHUB_COMPONENT_2(ILOG_MINOR_EVENT, VHUB_HUB_REQ_SET_FEATURE_HUB, bRequestType, feature);

                    // It is not possible for us to report if the local power source is good or
                    // if an over-current condition exists.
                    // We will just reply with an ACK and not perform any actions.
                    switch(feature)
                    {
                        case C_HUB_LOCAL_POWER:
                            SETUP_OUT_HANDLERS(pVF);
                            break;
                        case C_HUB_OVER_CURRENT:
                            SETUP_OUT_HANDLERS(pVF);
                            break;
                        default:
                            break;
                    }
                }
                else if ((GET_HUB_RECIPIENT(bRequestType) == RECIPIENT_PORT) && wIndex <= VHUB_NUM_PORTS) //SetPortFeature
                {
                    const uint8 port = wIndex;
                    const uint8 feature = wValue;

                    ilog_VHUB_COMPONENT_3(ILOG_MINOR_EVENT, VHUB_HUB_REQ_SET_FEATURE_PORT, port, bRequestType, feature);

                    switch(feature)
                    {
                        case PORT_CONNECTION:
                        case PORT_OVER_CURRENT:
                        case PORT_LOW_SPEED:
                        case PORT_HIGH_SPEED:
                        case PORT_INDICATOR:
                        case C_PORT_OVER_CURRENT:
                            // 11.24.2.7: treat all these requests as functional no-operations
                            SETUP_OUT_HANDLERS(pVF);
                            break;

                        case PORT_ENABLE:
                            // 11.24.2.7.1.2: The preferred behavior is that the hub respond with a Request Error [STALL]
                            // Port Enable can't be set this way.  As a port reset will set it.
                            // send a stall
                            break;

                        case PORT_SUSPEND:
                            // 11.5.1.9: From the Enabled state when it receives a SetPortFeature(PORT_SUSPEND) request
                            if(vhub.portInfo[port].state == OPERATING)
                            {
                                vhub.portInfo[port].state = SUSPENDED;

                                if(vhub.portInfo[port].usbCh11Status.status & HOST_ENDIAN_TO_USB_16(1 << VHUB_PSTATUS_ENABLE))
                                {
                                    VHUB_SET_PORT_STATUS_BIT(port, VHUB_PSTATUS_SUSPEND);
                                    XCSR_XICSSendMessage(USB_UPSTREAM_STATE_CHANGE, UPSTREAM_SUSPEND, port);
                                }
                            }
                            SETUP_OUT_HANDLERS(pVF);
                            break;

                        case PORT_RESET:
                            // 11.5.1.5: Unless it is in the Powered-off or Disconnected states, a port transitions to the Resetting
                            // state upon receipt of a SetPortFeature(PORT_RESET) request.
                            if(vhub.portInfo[port].state != DISCONNECTED)
                            {
                                vhub.portInfo[port].state = BUS_RESETTING;

                                switch(MAX(vhub.portInfo[port].speed, vhub.linkSpeed))
                                {
                                    case USB_SPEED_LOW:
                                        XCSR_XICSSendMessage(
                                                USB_UPSTREAM_STATE_CHANGE,
                                                UPSTREAM_BUS_RESET_LS,
                                                port);
                                        break;
                                    case USB_SPEED_FULL:
                                        XCSR_XICSSendMessage(
                                                USB_UPSTREAM_STATE_CHANGE,
                                                UPSTREAM_BUS_RESET_FS,
                                                port);
                                        break;
                                    case USB_SPEED_HIGH:
                                        XCSR_XICSSendMessage(
                                                USB_UPSTREAM_STATE_CHANGE,
                                                UPSTREAM_BUS_RESET_HS,
                                                port);
                                        break;
                                    default:
                                        break;
                                }

                                ilog_VHUB_COMPONENT_1(
                                    ILOG_MAJOR_EVENT, VHUB_HUB_REQ_SET_FEATURE_PORT_RESET, port);

                                //place port in resetting state
                                VHUB_SET_PORT_STATUS_BIT(port, VHUB_PSTATUS_RESET);
                                VHUB_CLR_PORT_STATUS_BIT(port, VHUB_PSTATUS_ENABLE);
                                vhub.portInReset |= (1 << port);
                            }

                            // TODO: why is this not in the IF statement?
                            TIMING_TimerStart(vhub.portResetTimerCompleteHandle);

                            SETUP_OUT_HANDLERS(pVF);
                            break;

                        case PORT_POWER:
                            if (vhub.portInfo[port].state != DISCONNECTED)
                            {
                                VHUB_SET_PORT_STATUS_BIT(port, VHUB_PSTATUS_CONNECTION);

                                if (vhub.portInfo[port].speed == USB_SPEED_LOW)
                                {
                                    VHUB_SET_PORT_STATUS_BIT(port, VHUB_PSTATUS_LOW_SPEED);
                                }
                                if (vhub.portInfo[port].speed == USB_SPEED_HIGH)
                                {
                                    VHUB_SET_PORT_STATUS_BIT(port, VHUB_PSTATUS_HIGH_SPEED);
                                }

                                VHUB_SET_PORT_CHANGE_BIT(port, VHUB_PCHANGE_CONNECTION);
                                VHUB_SET_INTEP1_DATA_BIT(port);

                                ilog_VHUB_COMPONENT_2(ILOG_MAJOR_EVENT, VHUB_CONNECT_DEV, port, vhub.portInfo[port].speed);
                            }

                            VHUB_SET_PORT_STATUS_BIT(wIndex, VHUB_PSTATUS_POWER);

                            SETUP_OUT_HANDLERS(pVF);
                            break;

                        case C_PORT_CONNECTION:
                        case C_PORT_ENABLE:
                        case C_PORT_SUSPEND:
                        case C_PORT_RESET:
                            // 11.24.2.7.2: Hub may treat these as a Request Error or as a functional
                            // no-operation

                            // We are treating these as functional no-operation. 
                            SETUP_OUT_HANDLERS(pVF);
                            break;

                        default:
                            break;
                    }
                }
                break;
            case HUB_REQ_GET_DESCRIPTOR:
                if (!inDirection)
                {
                    // do nothing
                }
                else
                {
                    SETUP_IN_HANDLERS(pVF);
                    pVF->state.currentRequestData = (uint8 *)&vhubHubDescriptor;
                    pVF->state.currentRequestDataLeft = MIN(wLength, sizeof(struct HubDescriptor));
                    ilog_VHUB_COMPONENT_0(ILOG_MINOR_EVENT, VHUB_HUB_REQ_GET_DESCRIPTOR);
                }
                break;

            // 8: ClearTTBuffer: Read & Chuck
            // 9: ResetTT: Read & Chuck
            // 7: SetHubDescriptor: Optional, we will send a stall to the out packet
            // 10: GetTTState: The USB spec. notes that this is for debugging, and should return a
            //     vendor specific format.  We could just return a zero length packet.
            // 11: StopTT: Read & Chuck
            default:
                break;
        }
    }

    if (    (pVF->in == &DTT_VFSendCtrlStallIn)
        &&  (pVF->out == &DTT_VFSendCtrlStallOut))
    {
        ilog_VHUB_COMPONENT_3(ILOG_MAJOR_ERROR, VHUB_SETUP_STALL, bRequestType, bRequest, wValue);
    }
}

static void vhubInTransaction_DataPhase_InHandler_EP0
(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    boolT toggle
)
{
    writeData(pVF, endpoint, address, toggle);
}

// Function expecting a data1 packet in the out direction, with size 0, IE a host side STATUS phase
// ack to a successful IN transaction.  ACK sent in response
static void vhubInTransaction_StatusPhase_ZeroLengthOutHandler
(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    uint8 pid,
    uint8 * data,
    uint16 dataSize,
    boolT toggle
)
{
    struct XCSR_XICSQueueFrame* frame;
    ilog_VHUB_COMPONENT_3(ILOG_DEBUG, VHUB_OUT_HANDLER, 0, pid, dataSize);

    // Expecting data size to be 0.  If it isn't something has gone wrong, and we should stall,
    // else ACK it
    // TODO: add check for PID
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);
    frame->dataSize = 0;
    XCSR_XICSBuildUpstreamFrameHeader(
            frame, address, endpoint, 0, frame->dataSize, XUSB_OUT,
            (dataSize == 0) ? XUSB_ACK : XUSB_STALL, // *** ACK or STALL ***
            pVF->epType, toggle);
    XCSR_XUSBSendUpstreamUSBFrame(address, endpoint, FALSE, frame);

    // Reset the In and Out handlers
    SETUP_RESET_HANDLERS(pVF);
}

//during the status phase an IN transaction will require a 0 data, data1 response.
static void vhubOutTransaction_StatusPhase_InHandler
(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    boolT toggle
)
{
    // TODO: add check for PID
    // Send zero length data response
    struct XCSR_XICSQueueFrame* frame;
    XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

    frame->dataSize = 1; // This is zero length packet, there is no data (except the PID)
    XCSR_XICSBuildUpstreamFrameHeader(
        frame, address, endpoint, 0, frame->dataSize, XUSB_IN, XUSB_DATA1, pVF->epType, toggle);

    ((struct XCSR_XICSQueueFrameUsbData*)frame->data)->pid = DATA1_PID;
    XCSR_XUSBSendUpstreamUSBFrame(address, endpoint, TRUE, frame);

    // Reset the In and Out handlers
    pVF->in = DTT_VFSendCtrlStallIn;
    pVF->out = DTT_VFSendCtrlStallOut;
    // the inAck handler will be reset in the inAckHandler
}

static void vhubInAckHandler
(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address
)
{
    ilog_VHUB_COMPONENT_0(ILOG_DEBUG, VHUB_IN_ACK_DESC_HANDLER);
    pVF->state.pid0Next = !pVF->state.pid0Next;
}

static void vhubOutTransaction_StatusPhase_InAckHandler
(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address
)
{
    SETUP_RESET_HANDLERS(pVF);
}


/**
* FUNCTION NAME: VHUB_PushStatusEndpoint()
*
* @brief  - Add Interrupt IN data to queue if data has already not been written.
*
* @return - void
*
* @note   - A race condition with VHUB_PushStatusEndpoint().
*           1. Host issues Port Reset
*           2. VHUB SW will start the reset timer.
*           3. Port reset timer expires. SW will set Port's reset change bit, set the Port's bit in the INT IN
*              data and will call VHUB_PushStatusEndpoint().
*           4. Host issues INT IN for Ep 1, which will set iBlk. SW will write INT IN Ep 1 data to queue.
*           5. Host issues Clear_Feature(C_Port_Reset) which will clear the Port's reset change bit. We cannot
*              change the INT Ep data because it has already been written to queue.
*           6. Host issues INT IN for Ep 1. Incorrect data from Step 4 will be sent to the host and XSST will
*              clear iBlk.
*
*/
void VHUB_PushStatusEndpoint(void)
{
    XUSB_AddressT address = DTT_GetAddressFromVF(&vhub_vf);
    static uint8  HubAndPortStatusChangeMap_BeingServed[VHUB_BIT_SIZE_IN_BYTES];

    iassert_VHUB_COMPONENT_1(XCSR_getXUSBAddrInSys(address), ADDR_NOT_IN_SYS,
            XCSR_getXUSBAddrUsb(address));

    iassert_VHUB_COMPONENT_0(
            vhub.intEp1.hubAndPortStatusChangeMap[0] != 0,
            PUSH_STATUS_WITH_NO_STATUS);

    // no need to check currentRequestDataLeft because Interrupt IN data is only 1 packet
    ilog_VHUB_COMPONENT_0(ILOG_MAJOR_EVENT, VHUB_INT_IN_REQUEST_SERV);

    // make a copy of status to be serviced (incase multiple IN requests are needed concurrent
    // to normal system operation)
    memcpy(HubAndPortStatusChangeMap_BeingServed,
            vhub.intEp1.hubAndPortStatusChangeMap,
            sizeof(HubAndPortStatusChangeMap_BeingServed));

    //prepare transfer
    vhub_vf.endpoint[1].state.currentRequestData =
        (uint8 *)&HubAndPortStatusChangeMap_BeingServed[0];
    vhub_vf.endpoint[1].state.currentRequestDataLeft =
        sizeof(HubAndPortStatusChangeMap_BeingServed);
    vhub_vf.endpoint[1].state.current_wIndex = 0;

    writeData(&vhub_vf.endpoint[1], 1, address, FALSE);
}

/**
 * Incoming requests for interrupt endpoint, if an outgoing push was requested, write data..
 * otherwise note a pending request
 *
 * @param pVF
 * @param endpoint
 * @param address
 * @param toggle
 */
static void vhubInTransaction_DataPhase_InHandler_EP1
(
    struct DTT_VF_EndPointHandle * pVF,
    uint8 endpoint,
    XUSB_AddressT address,
    boolT toggle
)
{
    boolT areAllChangeMapsZero = TRUE;
    uint8 i;

    //check if any of the port change bits are set
    for(i = 0; i < VHUB_BIT_SIZE_IN_BYTES; i++)
    {
        if (vhub.intEp1.hubAndPortStatusChangeMap[i] != 0)
        {
            areAllChangeMapsZero = FALSE;
        }
    }

    if (!areAllChangeMapsZero)
    {
        ilog_VHUB_COMPONENT_0(ILOG_MAJOR_EVENT, VHUB_INT_IN_REQUEST);
        VHUB_PushStatusEndpoint();
    }
    else
    {
        // Send a NAK
        struct XCSR_XICSQueueFrame* frame;
        // Set the data size for a short packet, or normal full packet
        XCSR_XICS_QUEUE_FRAME_STACK_ALLOC(XCSR_QUEUE_FRAME_XUSB_CTRL_PACKET_SIZE, frame);

        // create In response
        frame->dataSize = 0; // NAK is in the header
        XCSR_XICSBuildUpstreamFrameHeader(
                frame,
                address,
                endpoint,
                0,
                frame->dataSize,
                XUSB_IN,
                XUSB_NAK,
                pVF->epType,
                toggle);

        // Send the frame to the host
        XCSR_XUSBSendUpstreamUSBFrame(address, endpoint, TRUE, frame);
    }
}

