///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010-2012
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
//!   @file  - topology.c
//
//!   @brief - manages the device topology tree
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "topology_loc.h"
#include <ipool.h>
#include <leon_uart.h>
#include <xcsr_mmreg_macro.h>

/************************ Defined Constants and Macros ***********************/

// Intended to be used as the timeout for xsstCleanupTimer.
// 3 ms (frames) to allow the host to finish up any pending transactions.
#define XSST_CLEANUP_TIMEOUT 3

// Maximum number of xsstCleanupTimer expirations before we forcibly
// remove a split device requiring cleanup from sys. Currently corresponds to
// a timeout of 750 ms, or 250 3 ms timer expirations.
#define REQUIRES_CLEANUP_AND_INSYS_MAX_COUNT (750 / XSST_CLEANUP_TIMEOUT)

/******************************** Data Types *********************************/

// This type is passed as context to the describeTopologyHelper function which
// is called by TraverseTopology.
struct DescribeTopologyContext
{
    DTT_DescriptionHandlerT descriptionHandler;
    void* callbackData;
};

// Enumeration of the possible states that may lead to race conditions when
// devices are disconnected during a split transaction. These values are only intended
// to be used in the ResetXsstEntries, GetSplitRaceType, and GetSplitRaceTypeHelper functions.
// Refer to doc/split_workaround_notes.txt ("notes") for more information.
enum DTT_SplitRaceType
{
    DTT_SPLIT_RACE_NONE,    // No race

    DTT_SPLIT_RACE_WAIT,    // Potential race or we are waiting for the host to do something;
                            // either way we need to wait for the next timeout

    DTT_SPLIT_RACE_SPLIT,   // i/oSplit && ~i/oBlk (Issue 2 in notes)

    DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT,      // i/oSplit &&  i/oBlk && snoopHead(qid) == SSPLIT
                                          // (Issue 1 in notes)

    DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_BULK, // (DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT) && epType == BULK
                                          // This is a specialization of the above to indicate we
                                          // are dealing with a bulk endpoint

    DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_CTRL, // (DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT) && epType == CTRL
                                          // This is a specialization of the above to indicate we
                                          // are dealing with a control endpoint

    DTT_SPLIT_RACE_SPLIT_BLK_CSPLIT       // i/oSplit &&  i/oBlk && snoopHead(qid) == CSPLIT
                                          // (Issue 3 in notes)
};

/******************** Set Interface and Configuration Description **************

  Parsing and Storing Interface & Endpoint Information

  After parsing the endpoint descriptor for an interface, the endpoint
  information will be stored in the EndpointData node. The following
  are some of the features of the algorithm:
    - Endpoints will be sorted in the ascending order by endpoint
      number and then by endpoint direction (for GE only).
    - For non-MSA endpoints only, after all endpoints have been parsed,
      remove endpoints if endpoint number and endpoint direction (for GE only)
      combination is the same in different alternate settings and configurations.

  MSA Hardware Limitation:
  In the context of MSA pointer and status tables, endpoint refers to the endpoint
  number instead of the combination of endpoint number and endpoint direction.
  Thus, the program will disable MSA for mass storage endpoints when IN and OUT
  endpoints with one endpoint number are paired with OUT and IN endpoints of a
  different endpoint number, respectively.

  E.g. The following descriptors are read by the host:

  Configuration 1

  Interface 0, Alternate Setting 0:
  End Point 1: Type BULK IN, Non-MSA
  End Point 1: Type BULK OUT, Non-MSA
  End Point 2: Type INTERRUPT IN

  Interface 0, Alternate Setting 1:
  End Point 1: Type BULK IN, Non-MSA
  End Point 1: Type INTERRUPT OUT
  End Point 2: Type INTERRUPT IN

  For GE:
  The endpoints will be stored in the following format:
  // TopologyNode contains the memory pool index of the first
  // endpoint for that logical address
  deviceNode->endpointDataList = 1

  Before Optimization:

  EndpointData:
  Memory Index  (Members)
  1             (configuration:     1,
                 interface:         0,
                 alternateSetting:  0,
                 endpointNumber:    1,
                 endpointType:      BULK,
                 endpointDirection: OUT, // OUT = 0
                 msaPairEpNumber:   0, // 0: Non-MSA
                 next:              2)
  NOTE: next member variable contains the index to the next node of EndpointData.
  2             (configuration:     1,
                 interface:         0,
                 alternateSetting:  0,
                 endpointNumber:    1,
                 endpointType:      INTERRUPT,
                 endpointDirection: OUT,
                 msaPairEpNumber:   0,
                 next:              3)
  3             (configuration:     1,
                 interface:         0,
                 alternateSetting:  0,
                 endpointNumber:    1,
                 endpointType:      BULK,
                 endpointDirection: IN, // IN = 1
                 msaPairEpNumber:   0,
                 next:              4)
  4             (configuration:     1,
                 interface:         0,
                 alternateSetting:  1,
                 endpointNumber:    1,
                 endpointType:      BULK,
                 endpointDirection: IN,
                 msaPairEpNumber:   0,
                 next:              5)
  5             (configuration:     1,
                 interface:         0,
                 alternateSetting:  0,
                 endpointNumber:    2,
                 endpointType:      INTERRUPT,
                 endpointDirection: IN,
                 msaPairEpNumber:   0,
                 next:              6)
  6             (configuration:     1,
                 interface:         0,
                 alternateSetting:  1,
                 endpointNumber:    2,
                 endpointType:      INTERRUPT,
                 endpointDirection: IN,
                 msaPairEpNumber:   0,
                 next:              NULL)

  After Optimization:
  - For Endpoint Number 1 and Direction OUT, there are two distinct types i.e. BULK and INTERRUPT.
    Thus, EndpointData @ Index 1 and EndpointData @ Index 2 are kept.
  - For Endpoint Number 1 and Direction IN, there is only one type i.e. BULK.
    Thus, EndpointData Indices 3 and 4 are deleted.
  - For Endpoint Number 2 and Direction IN, there is only one type i.e. INTERRUPT.
    Thus, EndpointData Indices 5 and 6 are deleted.

 EndpointData:
  Memory Index  (Members)
  1             (configuration:     1,
                 interface:         0,
                 alternateSetting:  0,
                 endpointNumber:    1,
                 endpointType:      BULK,
                 endpointDirection: OUT,
                 msaPairEpNumber:   0,
                 next:              2)
  2             (configuration:     1,
                 interface:         0,
                 alternateSetting:  0,
                 endpointNumber:    1,
                 endpointType:      INTERRUPT,
                 endpointDirection: OUT,
                 msaPairEpNumber:   0,
                 next:              NULL)


*********************************End of Description**************************/


/***************************** Local Variables *******************************/
static struct {
    TIMING_TimerHandlerT xsstCleanUpTimer;
} topologyState __attribute__ ((section(".lexbss")));

IPOOL_CREATE(_DTT, struct EndpointData, TOPOLOGY_ENDPOINT_DATA_MEMPOOL_SIZE);

// Tracks how long a device has had its requiresCleanup bit set
// TODO Maybe replace this global counter with a per-device counter member in DeviceTopology?
static uint8 deviceRequiresCleanupCounter[MAX_USB_DEVICES];

/************************ Local Function Prototypes **************************/
static void ResetXsstEntries(void);
static void ResetXsst ( XUSB_AddressT oldAddress, struct DeviceTopology* deviceNode) __attribute__((section(".lextext")));
static boolT ForceCleanupUsbAddr(uint8 usbAddr);
static enum DTT_SplitRaceType GetSplitRaceType(XUSB_AddressT address, uint8 endPoint, enum EndpointDirection epDir);
static inline enum DTT_SplitRaceType GetSplitRaceTypeHelper(struct XCSR_Xsst, enum EndpointDirection);

static void _DTT_Disconnect(uint8 logicalAddressToDisconnect,
                            enum DTT_OperationType disconnectType);
static void _DTT_CleanupHelper(XUSB_AddressT address,
                               struct DeviceTopology* node,
                               void* arg);
static void _DTT_CleanupNowHelper(XUSB_AddressT address, struct DeviceTopology* node, void*);
static struct EndpointData * _DTT_CreateEndpointNode(void);
static struct EndpointData * _DTT_InsertEndpointToList(memPoolIndex_t * endpointIndex,
                                      struct EndpointData * newEndpointData);
static struct EndpointData * _DTT_HandleMemPoolFull(void);


static inline sint8 NewLogicalAddress(
    uint8 parentAddress, uint8 portOnParent);

static sint8 FindNode(uint8 parentLA, uint8 port);
static boolT FindFirstHighSpeedAncestor(
    uint8 deviceLA, uint8 * highSpeedHubLA, uint8 * descendantPort);

static void TraverseTopology(
    uint8 startNodeLA,
    struct DeviceTopology* startNode,
    void (*pTopologyProcessor)(
        XUSB_AddressT address,
        struct DeviceTopology* deviceNode,
        void* arg),
    void* arg) __attribute__ ((section(".lextext")));

static struct EndpointData * _DTT_FindMsaEndpoint
    (struct EndpointData *,
     uint8 configurationValue,
     uint8 alternateSetting,
     uint8 endpointNumber,
     enum EndpointDirection endpointDirection);

static void _DTT_DisableUnsupportedMSA(struct DeviceTopology* deviceNode);

static struct EndpointData * _DTT_FindEndpoint
    (struct EndpointData * endpointStart,
     uint8 configurationValue,
     uint8 alternateSetting,
     uint8 endpointNumber,
     enum EndpointDirection endpointDirection);

static void describeTopologyHelper(
    XUSB_AddressT address, struct DeviceTopology* node, void* arg);

static struct EndpointData * _DTT_WriteEndpointData
(
    XUSB_AddressT address,
    uint8 configurationValue,
    uint8 interfaceNumber,
    uint8 alternateSetting,
    uint8 endpointNumber,
    uint8 endpointType,
    enum EndpointDirection endpointDirection,
    boolT blockAccess
);

static void _DTT_WriteEndpointToXsst
(
    XUSB_AddressT address,
    struct EndpointData * endpoint,
    struct DeviceTopology * deviceNode,
    boolT allowMsaChanges
) __attribute__((section(".lextext")));


static boolT _DTT_isEndpointSetEqualAndNoMsa(
    struct EndpointData *  startSetEndpoint,
    struct EndpointData ** endSetEndpoint);
static void _DTT_RemoveEndpointSet(
    memPoolIndex_t * prevNodeNextIndex,
    struct EndpointData * endSetEndpoint);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: DTT_Init()
*
* @brief  - Ensure the topology tree is fresh and has had its shower
*
* @return - nothing
*/
void DTT_Init(void)
{
    _DTT_poolInit();

    // Set a sane default logging level
    ilog_SetLevel(ILOG_MAJOR_EVENT, TOPOLOGY_COMPONENT);

    topologyState.xsstCleanUpTimer = TIMING_TimerRegisterHandler(&ResetXsstEntries, TRUE, XSST_CLEANUP_TIMEOUT);
    _DTT_XSSTMonInit();

    // The root device can never be dynamically re-assigned, so just mark it in the topology on
    // startup.
    _DTT_GetDeviceNode(ROOT_LOGICAL_ADDRESS)->inTopology = TRUE;
}

/**
* FUNCTION NAME: DTT_assertHook()
*
* @brief  - Called during an assert to dump useful information.
*
* @return - void.
*/
void DTT_assertHook(void)
{
    uint8 la;
    ilog_TOPOLOGY_COMPONENT_0(ILOG_FATAL_ERROR, TOPOLOGY_ASSERT_XSST);
    _DTT_showAllDeviceTopologyByLA(1);
    for(la = 0; la < MAX_USB_DEVICES; la++)
    {
        struct DeviceTopology* node = _DTT_GetDeviceNode(la);
        if(node->inTopology)
        {
            _DTT_showDeviceXSST(la);

            // Make sure we don't overflow the UART buffer
            LEON_UartWaitForTx();
        }
    }
}


/**
* FUNCTION NAME: DTT_EnableAddress0()
*
* @brief  - Enable USB address 0 by placing it in-sys in the LAT
*
* @return - void
*
* @note   - USB address 0, must be configured by one of the DTT_*Reset*() functions first
*
*           The reset process is done in 2 steps
*           1) DTT_*Reset*() function.  Cleans up the XSST
*           2) DTT_EnableAddress0()
*           By doing this in 2 stages, higher level code can
*           reset/initialize/etc RTL/SW blocks in-between the stages
*/
void DTT_EnableAddress0(void)
{
    XCSR_XSSTAddr0SetInsys();
}


/**
* FUNCTION NAME: DTT_HostReset()
*
* @brief  - Process's a reset on the root device of the topology
*
* @return - void
*
* @note   - This is called when the root device is not a virtual function
*           Which means SW can assume there is only 1 Vport,
*           as VHub would be needed otherwise
*
*           If a VF was the root device DTT_HostResetVirtualFunction(),
*           would be called instead of this function
*/
void DTT_HostReset(void)
{
    XUSB_AddressT addressZero;

    // Device to reset is the root device, which is always logical address 0
    // All fields are being left as zero, as this isn't a valid address yet, and it isn't in-sys
    XCSR_initXUSBAddr(&addressZero);
    COMPILE_TIME_ASSERT(ROOT_LOGICAL_ADDRESS == 0);
    // Virtual functions don't use this function, so we know there is no VHub, and only a single Rex
    XCSR_setXUSBAddrVPort(&addressZero, ONLY_REX_VPORT);

    iassert_TOPOLOGY_COMPONENT_0(
        _DTT_Reset(addressZero, DTT_UPSTREAM_BUS_OPERATION),
        TOPOLOGY_HOST_RESET_SHOULD_NEVER_FAIL);
}


/**
* FUNCTION NAME: DTT_PortReset()
*
* @brief  - Process's a hub port reset operation
*
* @return - success, or a failure code
*
* @note   - This is for a non-VF
*           If the device was a VF call DTT_portResetVirtualFunction() instead
*
*           This is for a non-root device
*           DTT_HostReset() is used for the root device
*/
enum TOPOLOGY_PortResetResult DTT_PortReset
(
    XUSB_AddressT parentAddress,
    uint8 portNumber,
    uint8 vport
)
{
    enum TOPOLOGY_PortResetResult result = TOPOLOGY_PR_NO_MORE_ADDRESSES;
    // find the device to reset
    sint8 laToReset = _DTT_GetLogicalOrCreateNew(parentAddress, portNumber);

    // If we don't have a valid laToReset at this point, then there are no more available addresses
    if(laToReset != -1)
    {
        // Setup the new address for all fields that are non-zero
        // NOTE: leaving in-sys and valid zero, as _DTT_Reset will set those fields
        XUSB_AddressT addressZero;
        XCSR_initXUSBAddr(&addressZero);
        XCSR_setXUSBAddrLogical(&addressZero, laToReset);
        XCSR_setXUSBAddrVPort(&addressZero, vport);

        result =
            _DTT_Reset(addressZero, DTT_DOWNSTREAM_PORT_OPERATION) ?
                TOPOLOGY_PR_SUCCESS : TOPOLOGY_PR_ADDRESS_ZERO_IN_USE;
    }

    return result;
}


/**
* FUNCTION NAME: _DTT_GetLogicalOrCreateNew()
*
* @brief  - Gets a logical address or creates a new one
*
* @return - -1 on failure, logical address otherwise
*
* @note   - 
*
*/
sint8 _DTT_GetLogicalOrCreateNew
(
    XUSB_AddressT parentAddress,
    uint8 portNumber
)
{
    // This is when a new logical address is brought into the system by a USB port reset
    sint8 laToReset = FindNode(XCSR_getXUSBAddrLogical(parentAddress), portNumber);

    if(laToReset == -1)
    {
        laToReset = NewLogicalAddress(XCSR_getXUSBAddrLogical(parentAddress), portNumber);
    }

    return laToReset;
}


/**
* FUNCTION NAME: _DTT_Reset()
*
* @brief  - Reset the device identified by parent and port number and all it's descendants
*
* @return - TRUE on success or FALSE if address zero is already in use.
*/
boolT _DTT_Reset(
    XUSB_AddressT addressZero,          // Should contain logical addr, VF info, and have the USB
                                        // addr initialized to 0
    enum DTT_OperationType resetType    // type of reset to be performed
)
{
    const uint8 logicalAddrToReset = XCSR_getXUSBAddrLogical(addressZero);
    struct DeviceTopology * deviceNode;
    boolT cleanupCompleted;

    deviceNode = _DTT_GetDeviceNode(logicalAddrToReset);

    if(deviceNode->isConnected)
    {
        // Process all devices that need resetting
        TraverseTopology(logicalAddrToReset, deviceNode, &_DTT_CleanupHelper, (void*)resetType);

        if (resetType == DTT_DOWNSTREAM_PORT_OPERATION)
        {
            // Bug 1612
            // Clean up XSST after some delay, such that LEX has enough time to finish off any last
            // packets which prevents (hopefully) returning a queue multiple times (once by SW and
            // then by LEX)
            TIMING_TimerStart(topologyState.xsstCleanUpTimer);
        }
    }

    if(deviceNode->requiresCleanup)
    {
        // Ensure the device we are working with is cleaned up now
        XUSB_AddressT oldAddress;
        XCSR_initXUSBAddr(&oldAddress);
        XCSR_setXUSBAddrUsb(&oldAddress, deviceNode->usbAddr);
        XCSR_setXUSBAddrLogical(&oldAddress, logicalAddrToReset);
        ResetXsst(oldAddress, deviceNode);
    }

    // Need to call this to ensure that address zero is cleaned up before using it since it may
    // have been scheduled to be cleaned up on the timer.
    cleanupCompleted = ForceCleanupUsbAddr(0);

    if(cleanupCompleted)
    {
        // After the topology cleanup is finished, configure this device in the LAT
        XCSR_setXUSBAddrValid(&addressZero, TRUE);
        XCSR_setXUSBAddrInSys(&addressZero, FALSE);
        XCSR_XSSTUpdateAddress(addressZero, FALSE);

        // Set up our topology data.  Since the device is being reset, its USB address becomes zero.
        deviceNode->usbAddr = 0;
        deviceNode->isConnected = TRUE;
        deviceNode->requiresCleanup = FALSE;
        deviceNode->pVirtualFunction = XCSR_getXUSBAddrVirtFuncPointer(addressZero);

        // The configuration value of the device goes back to 0 which is a special default
        // configuration indicating that the device is not yet configured.
        deviceNode->configurationValue = 0;
    }

    return cleanupCompleted;
}


/**
* FUNCTION NAME: FindNode()
*
* @brief  - Given a parent logical address and a port number on that parent, find the node the
*           device is living on if it exits
*
* @return - logical address of the node or -1 if there is no matching device
*/
static sint8 FindNode(uint8 parentLA, uint8 port)
{
    struct DeviceTopology * pParentNode = _DTT_GetDeviceNode(parentLA);

    if (pParentNode->childLA != 0)
    {
        uint8 currentNodeLA = pParentNode->childLA;
        struct DeviceTopology * pCurrentNode = _DTT_GetDeviceNode(currentNodeLA);

        // check whether the current node's port is the one we are looking for
        while (pCurrentNode->portOnParent != port)
        {
            if (pCurrentNode->siblingLA == 0)
            {
                // No more siblings to check and we didn't find the port
                ilog_TOPOLOGY_COMPONENT_2(
                    ILOG_MINOR_EVENT, TOPOLOGY_PORT_NOT_FOUND, port, parentLA);
                return -1;
            }
            // go to the current node's sibling node
            currentNodeLA = pCurrentNode->siblingLA;
            pCurrentNode = _DTT_GetDeviceNode(currentNodeLA);
        }
        return currentNodeLA;
    }
    else
    {
        return -1;
    }
}


/**
* FUNCTION NAME: TraverseTopology()
*
* @brief  - Call the supplied function on the node with the given logical address and all of its
*           descendants.
*
* @return - void
*
* @note   - This uses a depth first algorithm with a delayed callback.  We don't call the callback,
*           until we are done with the address.  This is to protect against a callback that might
*           clear all the address fields
*/
static void TraverseTopology(
    uint8 startNodeLA,
    struct DeviceTopology * startNode,
    void (*pTopologyProcessor)(
        XUSB_AddressT address, struct DeviceTopology * deviceNode, void* arg),
    void* arg)
{
    struct DeviceTopology* currNode = startNode;
    uint8 currLA = startNodeLA;

    // Start our traversal from the lowest, leftmost child of the subtree below startNodeLA if
    // children are drawn below their parent and siblings are drawn to the right.
    // eg.
    //                          root ---> x
    //                                    |
    //                                    x-----x
    //                                    |     |
    //                    start here ---> x     x-----x
    //                                          |
    //                                          x

    // Goto's break every coding standard, but it simplifies the code in this case
    // We jump to the point of finding the lower, left in the loop
    goto traverseStart;

    // On each iteration of this loop it is assumed that the children of the
    // current node have already being processed, so it checks the sibling of
    // the current node, and processes that subtree depth first, or if no more
    // siblings exist, it steps up to the parent
    do
    {
        uint8 siblingLA;
        uint8 parentLA;

        if(siblingLA != 0)
        {
            currLA = currNode->siblingLA;
            currNode = _DTT_GetDeviceNode(currLA );
            // Go as deep as possible from the current node
traverseStart:
            while(currNode->childLA != 0)
            {
                currLA = currNode->childLA;
                currNode = _DTT_GetDeviceNode(currLA);
            }
        }
        else
        {
            currLA = parentLA;
            currNode = _DTT_GetDeviceNode(currLA);
        }

        // Initialize sibling & parent pointers for the next loop
        siblingLA = currNode->siblingLA;
        parentLA = currNode->parentLA;

        // Process logical address
        ilog_TOPOLOGY_COMPONENT_1(ILOG_DEBUG, TRAVERSE_TOPOLOGY, currLA);
        (*pTopologyProcessor)(_DTT_GetAddressFromLogical(currLA), currNode, arg);

    } while (currNode != startNode);
}

/**
* FUNCTION NAME: DTT_FinalizeSetAddress()
*
* @brief  - In the atypical scenario where a set address request is received on a non-zero USB
*           address, we update the associated USB address in the topology based on a temporary
*           value that is set when the set address request was received.
*
* @return - none
*
* @note   - The value cannot be set earlier because we need to wait for the set address request to
*           complete.
*/
void DTT_FinalizeSetAddress
(
    const XUSB_AddressT oldAddress // Address that the set address request originated from
)
{
    struct DeviceTopology * deviceNode = _DTT_GetDeviceNode(XCSR_getXUSBAddrLogical(oldAddress));
    XUSB_AddressT newAddress;

    deviceNode->usbAddr = deviceNode->setupTransactionState.newUsbAddr;

    newAddress = oldAddress;
    XCSR_setXUSBAddrUsb(&newAddress, deviceNode->usbAddr);

    // Need to call this to ensure that the new USB address is cleaned up before using it since it
    // may have been scheduled to be cleaned up on the timer.
    iassert_TOPOLOGY_COMPONENT_1(
        ForceCleanupUsbAddr(deviceNode->usbAddr),
        TOPOLOGY_EXPECTED_DEVICE_REQUIRING_CLEANUP,
        deviceNode->usbAddr);

    // Cleanup old address
    // THIS MUST BE DONE BEFORE ADDING THE NEW ADDRESS
    //  * LG1 XCSR driver depends on it
    //  * It mimicks what a real device does on a SetAddress command
    XCSR_XSSTClearLAT(oldAddress);
    _DTT_freeMsa(oldAddress, deviceNode, FALSE); //NOTE: It will get re-enabled on SetConfiguration

    // Write the logical address and set in-sys bit in device status table, as well as the split
    // bit if needed
    XCSR_XSSTUpdateAddress(newAddress, isSplitDevice(XCSR_getXUSBAddrLogical(oldAddress)));
}

/**
* FUNCTION NAME: DTT_SetAddress()
*
* @brief  - Update the topology and XSST and look up table with the new USB address
*
* @return - FALSE if topology is in a bad state
*
* @note   - The topology can be in a bad state, if
*           1) we are doing a quick enumeration without any endpoint information
*           2) The host is re-using USB addresses
*/
boolT DTT_SetAddress
(
    const XUSB_AddressT oldAddress,
    XUSB_AddressT * pNewAddress
)
{
    struct DeviceTopology * deviceNode;

    uint8 newUsbAddr = XCSR_getXUSBAddrUsb(*pNewAddress);
    *pNewAddress = oldAddress;
    XCSR_setXUSBAddrUsb(pNewAddress, newUsbAddr);

    // Check to see if the host was re-using USB addresses
    if (XCSR_getXUSBAddrInSys(DTT_GetAddressFromUSB(XCSR_getXUSBAddrUsb(*pNewAddress))))
    {
        // "The host is re-using USB address %d"
        ilog_TOPOLOGY_COMPONENT_1(
            ILOG_FATAL_ERROR, HOST_REUSING_USB_ADDR, XCSR_getXUSBAddrUsb(*pNewAddress));
        return FALSE;
    }

    deviceNode = _DTT_GetDeviceNode(XCSR_getXUSBAddrLogical(*pNewAddress));

    // Inform the XCSR driver of our intent to disable the old address, before it is disabled
    // NOTE: this is for working around LG1 bugs
    XCSR_XSSTIntentToClearLAT(oldAddress);

    // Store the new USB address received in the set address request so that the topology can be
    // updated once the set address response is received.
    deviceNode->setupTransactionState.newUsbAddr = XCSR_getXUSBAddrUsb(*pNewAddress);

    if (XCSR_getXUSBAddrUsb(oldAddress) != 0)
    {
        ilog_TOPOLOGY_COMPONENT_3(
            ILOG_DEBUG, TOPOLOGY_SET_ADDRESS_ON_EXISTING,
            XCSR_getXUSBAddrLogical(*pNewAddress),
            XCSR_getXUSBAddrUsb(oldAddress),
            XCSR_getXUSBAddrUsb(*pNewAddress));
    }

    return TRUE;
}


/**
* FUNCTION NAME: NewLogicalAddress()
*
* @brief  - Find the next available (unused) logical address, or a previously disconnected one if
*           no unused addresses are available.
*
* @return - A valid logical address (in the range 0 to MAX_USB_DEVICES - 1) or -1 if no logical
*           addresses are available.
*/
static inline sint8 NewLogicalAddress(
    uint8 parentAddress,    // Logical address that will be the parent of the new node
    uint8 portOnParent      // Port on which the new device will exist
)
{
    sint8 logicalAddress;
    sint8 newAddr = -1;
    boolT newAddrIsDisconnectedLeaf;

    // Find next available logical address in the topology
    // Loop through all addresses and stop (by break statement) when a device is non in the
    // topology.  Or use the last logical address that was found that isn't connected and has no
    // children nodes.
    for (logicalAddress = 0; logicalAddress < MAX_USB_DEVICES; logicalAddress++)
    {
        struct DeviceTopology* node = _DTT_GetDeviceNode(logicalAddress);
        if (!node->inTopology)
        {
            newAddr = logicalAddress;
            newAddrIsDisconnectedLeaf = FALSE;
            break;
        }
        else if(!node->isConnected && node->childLA == 0)
        {
            // The fall-back if there are no free logical addresses is to choose a logical address
            // corresponding to a disconnected device that has no children.  If a device is
            // disconnected, then it's children will also be disconnected.  It then follows
            // logically that if there is any device that is disconnected, there must be at least
            // one such device which is a leaf node (childLA == 0).
            newAddr = logicalAddress;
            newAddrIsDisconnectedLeaf = TRUE;
        }
    }
    if(newAddr != -1)
    {
        // A new address has been found.  Clean up its old location and configure its new location.
        struct DeviceTopology* newNode = _DTT_GetDeviceNode(newAddr);
        struct DeviceTopology* newParent = _DTT_GetDeviceNode(parentAddress);
        if(newAddrIsDisconnectedLeaf)
        {
            struct DeviceTopology* oldParent = _DTT_GetDeviceNode(newNode->parentLA);
            if(newNode->requiresCleanup)
            {
                XUSB_AddressT oldAddress;
                XCSR_initXUSBAddr(&oldAddress);
                XCSR_setXUSBAddrUsb(&oldAddress, newNode->usbAddr);
                XCSR_setXUSBAddrLogical(&oldAddress, newAddr);
                ResetXsst(oldAddress, newNode);
            }

            _DTT_ClearEndpoints(newAddr);
            newNode->idProduct = 0;
            newNode->idVendor = 0;

            if(oldParent->childLA == newAddr)
            {
                // If the device being re-used is the first child of its parent, re-assign the
                // parent's child to be the sibling of the node we are taking.
                oldParent->childLA = newNode->siblingLA;
            }
            else
            {
                // The device being re-used is the 2nd or later child of oldParentLA, so we need to
                // update the sibling before the device to be re-used to point to the sibling after
                // it.
                struct DeviceTopology* curr = _DTT_GetDeviceNode(oldParent->childLA);
                while(curr->siblingLA != newAddr)
                {
                    iassert_TOPOLOGY_COMPONENT_1(
                        curr->siblingLA != 0, TOPOLOGY_INVALID_TREE_STRUCTURE, __LINE__);
                    curr = _DTT_GetDeviceNode(curr->siblingLA);
                }
                curr->siblingLA = newNode->siblingLA;
            }
        }
        newNode->inTopology = TRUE;
        newNode->siblingLA = newParent->childLA;
        newParent->childLA = newAddr;
        newNode->parentLA = parentAddress;
        newNode->portOnParent = portOnParent;

        ilog_TOPOLOGY_COMPONENT_3(
            ILOG_MINOR_EVENT, TOPOLOGY_DEVICE_ADDING_DEVICE, newAddr, parentAddress, portOnParent);
    }
    return newAddr;
}

static void _DTT_CleanupNowHelper(XUSB_AddressT address, struct DeviceTopology* node, void* ignored)
{
    if(XCSR_getXUSBAddrValid(address) && node->requiresCleanup)
    {
        ResetXsst(address, node);
    }
}

/**
* FUNCTION NAME: ResetXsst()
*
* @brief  - Clears the XSST entry and LAT for the given logical address
*
* @return - nothing
*
* @note   -
*/
static void ResetXsst
(
    XUSB_AddressT oldAddress,
    struct DeviceTopology* deviceNode
)
{
    ilog_TOPOLOGY_COMPONENT_2(
        ILOG_DEBUG, RESET_XSST, XCSR_getXUSBAddrLogical(oldAddress), XCSR_getXUSBAddrUsb(oldAddress));

    // Mark the device as no longer needing cleanup first, so the compiler
    // doesn't need to keep deviceNode in registers for the whole function
    deviceNode->requiresCleanup = FALSE;

    XCSR_XSSTClearInsys(oldAddress);

    _DTT_freeMsa(oldAddress, deviceNode, TRUE);
    XCSR_XSSTResetEndpoints(oldAddress, 0, deviceNode->highEndpoint);
    // Clean up the logical address table
    XCSR_XSSTClearLAT(oldAddress);
}


/**
* FUNCTION NAME: DTT_HostDisconnect()
*
* @brief  - Disconnects the root device.
*
* @return - None.
*/
void DTT_HostDisconnect(void)
{
    if(_DTT_GetDeviceNode(ROOT_LOGICAL_ADDRESS)->isConnected)
    {
        _DTT_Disconnect(ROOT_LOGICAL_ADDRESS, DTT_UPSTREAM_BUS_OPERATION);
    }
}

/**
* FUNCTION NAME: DTT_PortDisconnect()
*
* @brief  - Disconnects the device on the given parent number of the specified device.
*
* @return - None.
*/
void DTT_PortDisconnect(XUSB_AddressT parentAddress, uint8 portNumber)
{
    // find the device to disconnect
    sint8 laToDisconnect = FindNode(XCSR_getXUSBAddrLogical(parentAddress), portNumber);

    if(laToDisconnect != -1)
    {
        struct DeviceTopology * pNode = _DTT_GetDeviceNode(laToDisconnect);
        if(pNode->isConnected)
        {
            ilog_TOPOLOGY_COMPONENT_3(
                ILOG_MAJOR_EVENT, TOPOLOGY_DISCONNECT,
                laToDisconnect, XCSR_getXUSBAddrLogical(parentAddress), portNumber);

            _DTT_Disconnect(laToDisconnect, DTT_DOWNSTREAM_PORT_OPERATION);
        }
    }
}


/**
* FUNCTION NAME: DTT_DeviceDisconnect()
*
* @brief  - Disconnects the device
*
* @return - void
*
* @note   - Intended for device class filtering
*
*/
void DTT_DeviceDisconnect
(
    XUSB_AddressT devAddress    // Device to disconnect
)
{
    ilog_TOPOLOGY_COMPONENT_2(
        ILOG_WARNING,
        DISCONNECT_DEVICE,
        XCSR_getXUSBAddrUsb(devAddress),
        XCSR_getXUSBAddrLogical(devAddress));

    // NOTE: The following is going to (potentially) take the device out of the system right now.
    // This call is probably going to be followed by some DTT_WriteEndpoint() calls as higher level
    // code is parsing descriptors.  This is OKAY, as the device is only being taken out of sys,
    // and its logical address mapping is still valid.

    // NOTE: There may be a delay in taking the device out of sys.  This isn't an issue, as the
    // device will be taken out of sys as requested shortly.
    _DTT_Disconnect(XCSR_getXUSBAddrLogical(devAddress), DTT_DOWNSTREAM_PORT_OPERATION);
}


/**
* FUNCTION NAME: _DTT_Disconnect()
*
* @brief  - Clean up the topology after a disconnect
*
* @return - nothing
*
* @note   - caller must ensure device is actually connected
*/
static void _DTT_Disconnect
(
    uint8 logicalAddrToDisconnect,          // Logical addr of device to disconnect
    enum DTT_OperationType disconnectType   // Host or port disconnect
)
{
    if (disconnectType == DTT_DOWNSTREAM_PORT_OPERATION)
    {
        // Bug 1612
        // Clean up XSST after some delay, such that LEX has enough time to finish off any last
        // packets which prevents (hopefully) returning a queue multiple times (once by SW and
        // then by LEX)
        TIMING_TimerStart(topologyState.xsstCleanUpTimer);
    }

    // Note: the _DTT_GetDeviceNode() call does sanity assert checks
    // Note: done at the end of the function to allow the compiler to do tail call optimization
    TraverseTopology(logicalAddrToDisconnect, _DTT_GetDeviceNode(logicalAddrToDisconnect), &_DTT_CleanupHelper, (void*)disconnectType);
}


/**
* FUNCTION NAME: _DTT_CleanupHelper()
*
* @brief  - Cleans up the data associated with the given device.  This is either done immediately
*           within this function or postponed to a timer task by setting node.requiresCleanup.
*
* @return - void
*/
static void _DTT_CleanupHelper(XUSB_AddressT address, struct DeviceTopology* node, void* arg)
{
    enum DTT_OperationType operationType = (enum DTT_OperationType)arg; // Host or port operation
    if(XCSR_getXUSBAddrValid(address))
    {
        // Delay cleanup for all port operations in LG1 and only port operations on split devices
        // in GE, see comments in reset of function
        node->requiresCleanup =
            (operationType == DTT_DOWNSTREAM_PORT_OPERATION) &&
            isSplitDevice(XCSR_getXUSBAddrLogical(address));

        // Mark the logical address for reuse
        node->isConnected = FALSE;
        node->pVirtualFunction = NULL;

        // If this is not a delayed cleanup, clean up now
        if(!node->requiresCleanup)
        {
            ResetXsst(address, node);

            // don't delay cleanup on splits, as the hub isn't there to respond to complete splits
            // In VHub, this will prevent spam of Lex sending requests downstream to disconnected REXs
            if (node->isHub && !isSplitDevice(XCSR_getXUSBAddrLogical(address)))
            {
                // walk all split children, cleaning up instantly
                uint8 childLA = node->childLA;

                while (childLA != 0)
                {
                    struct DeviceTopology * childNode = _DTT_GetDeviceNode(childLA);

                    if (isSplitDevice(childLA))
                    {
                        TraverseTopology(childLA, childNode, &_DTT_CleanupNowHelper, NULL);
                    }

                    childLA = childNode->siblingLA;
                }
            }
        }
        else
        {
            // Device requiresCleanup, delayed cleanup
            if (isSplitDevice(XCSR_getXUSBAddrLogical(address)))
            {
                // Splits need to stay in sys, so the complete splits can finish.
                // Upon disconnect or reset, reset a split device's cleanup counter.
                deviceRequiresCleanupCounter[XCSR_getXUSBAddrLogical(address)] = 0;
            }
        }
    }
    else
    {
        iassert_TOPOLOGY_COMPONENT_2(
            !node->isConnected, TOPOLOGY_INVALID_DEVICE_CONNECTED, node->usbAddr, __LINE__);
    }
}


/**
* FUNCTION NAME: ResetXsstEntries()
*
* @brief  - Clears the XSST entry for all addresses in the topology that are marked
*           as requiring cleanup via the "requiresCleanup" bit. Also includes a workaround
*           for a race condition that can occur when a device participating in a split transaction
*           is disconnected before the split completes. Refer to doc/split_workaround_notes.txt
*           for more information.
*
* @return - nothing
*
* @note   - Run from the xsst cleanup timer
*/
static void ResetXsstEntries(void)
{
    for (uint8 LA = 0; LA < MAX_USB_DEVICES; LA++)
    {
        struct DeviceTopology * node = _DTT_GetDeviceNode(LA);
        if (node->requiresCleanup)
        {
            XUSB_AddressT xAddr = _DTT_GetAddressFromLogical(LA);
            ilog_TOPOLOGY_COMPONENT_1(ILOG_DEBUG, TOPOLOGY_LA_REQUIRES_CLEANUP, LA);

            // This address requires cleanup, so it better be a valid address
            iassert_TOPOLOGY_COMPONENT_2(
                XCSR_getXUSBAddrValid(xAddr),
                TOPOLOGY_INVALID_ADDRESS_REQUIRES_CLEANUP,
                (uint32)xAddr,
                __LINE__);

            if (XCSR_getXUSBAddrInSys(xAddr))
            {
                // Check whether any of the device's endpoints are in a split transaction
                // that should not require our intervention to complete.
                // If any are, leave the device inSys until the transaction completes.
                // Otherwise, remove it from sys.
                boolT deviceInHostSplitTransaction = FALSE;
                for ( uint8 ep = 0;
                            ep <= node->highEndpoint && !deviceInHostSplitTransaction;
                            ep++)
                {
                    // Check if there is a need to wait for the host to finish up the split
                    // transaction. In the DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_{BULK,CTRL} state
                    // we have never ACK'd a SSPLIT to the host in a bulk or control transaction,
                    // so the host will never continue this transaction
                    const enum DTT_SplitRaceType in  = GetSplitRaceType(xAddr, ep, EP_DIRECTION_IN);
                    const enum DTT_SplitRaceType out = GetSplitRaceType(xAddr, ep, EP_DIRECTION_OUT);
                    deviceInHostSplitTransaction =
                              (    in != DTT_SPLIT_RACE_NONE
                                && in != DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_BULK
                                && in != DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_CTRL
                              )
                          ||
                              (    out != DTT_SPLIT_RACE_NONE
                                && out != DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_BULK
                                && out != DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_CTRL
                              );
                }

                if (!deviceInHostSplitTransaction ||
                     deviceRequiresCleanupCounter[LA] > REQUIRES_CLEANUP_AND_INSYS_MAX_COUNT)
                {
                    XCSR_XSSTClearInsys(xAddr);
                    ilog_TOPOLOGY_COMPONENT_2(
                        ILOG_DEBUG,
                        TOPOLOGY_DEVICE_REQUIRING_CLEANUP_INSYS_CLEARED,
                        LA,
                        deviceRequiresCleanupCounter[LA]);
                    deviceRequiresCleanupCounter[LA] = 0;
                }
                else
                {
                    deviceRequiresCleanupCounter[LA]++;
                }
            }
            else // Device is not inSys
            {
                for (uint8 ep = 0; ep <= node->highEndpoint; ep++)
                {
                    const enum EndpointDirection dir[] = {EP_DIRECTION_OUT, EP_DIRECTION_IN};
                    for (uint8 i = 0; i < sizeof(dir) / sizeof(dir[0]); i++)
                    {
                        const enum DTT_SplitRaceType raceType = GetSplitRaceType(xAddr, ep, dir[i]);

                        if (raceType == DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_BULK ||
                            raceType == DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_CTRL)
                        {
                            uint8 upstreamPort;    // Device's port on the nearest upstream HS hub
                            uint8 hubLA;           // Nearest HS hub
                            // Note the assignment to hubLA and upstreamPort via
                            // FindFirstHighSpeedAncestor
                            iassert_TOPOLOGY_COMPONENT_1(
                                FindFirstHighSpeedAncestor(LA, &hubLA, &upstreamPort),
                                TOPOLOGY_SPLIT_DEVICE_HAS_NO_UPSTREAM_HS_HUB,
                                LA);
                            const enum EndpointTransferType epType =
                                    raceType == DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_BULK ?
                                    EP_TYPE_BULK : EP_TYPE_CTRL;

                            struct DeviceTopology * hubNode = _DTT_GetDeviceNode(hubLA);

                            // Inject a CSPLIT packet to the device's nearest upstream HS hub to clear
                            // the SSPLIT (that will never be dealt with by the host) from its port's
                            // TT buffer. Since the device is out of sys, its response will be ignored.
                            XCSR_XICSSendRexCSplit(
                                xAddr, ep, epType, dir[i], hubNode->usbAddr, upstreamPort, node->speed);

                            ilog_TOPOLOGY_COMPONENT_3(
                                ILOG_MINOR_EVENT, TOPOLOGY_SENT_CSPLIT_TO_REX, LA, ep, dir[i]);
                        }
                    }
                }
                // Finally, reset the device's XSST entry now that it's fully cleaned up.
                ResetXsst(xAddr, node);
            }
        }
    }
}

/**
* FUNCTION NAME: GetSplitRaceType
*
* @brief  - For a given endpoint belonging to a device requiring cleanup, determines if
*           the endpoint is vulnerable to a race condition that can occur when a device
*           is disconnected in the middle of a split transaction.
*
* @return - DTT_SplitRaceType, an enum specifying the type of race vulnerability
*           (including "no race")
*
* @note   - This function is unlikely to be useful outside of ResetXsstEntries()
*         - Refer to doc/split_workaround_notes.txt for more information
*/
static enum DTT_SplitRaceType GetSplitRaceType
(
    XUSB_AddressT address,        // XUSB address of the device we're checking
    uint8 endPoint,               // Endpoint of the device we're checking
    enum EndpointDirection epDir  // Transfer direction of the endpoint we're checking
)
{
    // Read the XSST and do any queue snooping in a ReadConditional-WriteConditional
    // critical section to reduce the likelihood of situations where RTL
    // modifies/invalidates a Qid after we've snooped it. Note the matching call to
    // WriteConditional below.
    const struct XCSR_Xsst xsstRow = { .sst = XCSR_XSSTReadConditional(address, endPoint) };

    const enum EndpointTransferType epType = epDir == EP_DIRECTION_OUT ?
            xsstRow.sstStruct.oEpType : xsstRow.sstStruct.iEpType;

    enum DTT_SplitRaceType raceType = DTT_SPLIT_RACE_NONE;

    iassert_TOPOLOGY_COMPONENT_2(
            epDir == EP_DIRECTION_OUT || epDir == EP_DIRECTION_IN,
            TOPOLOGY_INVALID_DIRECTION_ARG,
            epDir,
            __LINE__);

    // First handle SETUP SPLIT as this is the unusal transaction
    if (xsstRow.sstStruct.iEpType == EP_TYPE_CTRL &&
        xsstRow.sstStruct.oEpType == EP_TYPE_CTRL &&
        endPoint == 0                             &&
        xsstRow.ovrLay.ctrlEndPointStruct.setupRspPndg)
    {
        raceType = DTT_SPLIT_RACE_WAIT;
    }
    else if (epType == EP_TYPE_BULK || epType == EP_TYPE_CTRL)
    {
        // These are the endpoint types for which we may need to send a CSPLIT,
        // so do the extra checks to determine if we need to send one
        raceType = GetSplitRaceTypeHelper(xsstRow, epDir);
        if (raceType == DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT)
        {
            // Specialize raceType based on our endpoint type so we can
            // send the right endpoint type in XCSR_XICSSendRexCSplit later
            raceType = epType == EP_TYPE_BULK ?
                DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_BULK :
                DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_CTRL;
        }
    }

    if (!XCSR_XSSTWriteConditional(address, endPoint, xsstRow.sst))
    {
        // Conditional write failed -- just try again next timer expiration
        // if the race type isn't the kind for which we send a CSPLIT
        if (raceType != DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_BULK &&
            raceType != DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT_CTRL)
        {
            raceType = DTT_SPLIT_RACE_WAIT;
        }

        ilog_TOPOLOGY_COMPONENT_3(
                ILOG_DEBUG,
                TOPOLOGY_XSST_WRITE_CONDITIONAL_FAILED,
                XCSR_getXUSBAddrLogical(address),
                endPoint,
                (epType << 8) | epDir);
    }

    return raceType;
}

/**
* FUNCTION NAME: GetSplitRaceTypeHelper
*
* @brief  - This is a helper function used to determine whether a recently disconnected device is
*           vulnerable to race conditions that can occur when devices are
*           disconnected during split transactions, and if so, the type of vulnerability.
*
* @return - DTT_SplitRaceType, an enum specifying the type of race vulnerability
*           (including "no race")
*
* @note   - This helper function is unlikely to be useful outside of GetSplitRaceType()
*         - Refer to doc/split_workaround_notes.txt for more information
*/
static inline enum DTT_SplitRaceType GetSplitRaceTypeHelper
(
    struct XCSR_Xsst xsstRow,     // XSST row for the endpoint we are checking
    enum EndpointDirection epDir  // Endpoint direction to check
)
{
    enum DTT_SplitRaceType raceType = DTT_SPLIT_RACE_NONE;
    boolT split;
    boolT blk;
    enum XCSR_Queue qid;

    if (epDir == EP_DIRECTION_IN)
    {
        split = xsstRow.sstStruct.iSplit;
        blk   = xsstRow.sstStruct.iBlk;
        qid   = xsstRow.sstStruct.iQid;
    }
    else if (epDir == EP_DIRECTION_OUT)
    {
        split = xsstRow.sstStruct.oSplit;
        blk   = xsstRow.sstStruct.oBlk;
        qid   = xsstRow.sstStruct.oQid;
    }
    else
    {
        iassert_TOPOLOGY_COMPONENT_2(FALSE, TOPOLOGY_INVALID_DIRECTION_ARG, epDir, __LINE__);
    }

    if (split)
    {
        if (blk)
        {
            if (qid == 0)
            {
                // If split is set but qid is invalid (== 0), we need to wait for qid to
                // become valid. Return DTT_SPLIT_RACE_WAIT to indicate that our device
                // still requires cleanup.
                raceType = DTT_SPLIT_RACE_WAIT;
            }
            else // qid != 0
            {
                uint32 controlRegOut;
                const struct XCSR_XICSQueueFrame frame = {
                    .header        = {
                        .one = {
                            .dword = XCSR_XICSQueueSnoopHeadGetControlReg(qid, &controlRegOut)
                        }
                    }
                };

                // Ensure that we actually read the start of the frame
                if (!XCSR_XICS_CONTROLQACC_RSOF_GET_BF(controlRegOut))
                {
                    raceType = DTT_SPLIT_RACE_WAIT;
                }
                else if (frame.header.one.upstream.modifier == 2) // 2 <=> SSPLIT
                {
                    raceType = DTT_SPLIT_RACE_SPLIT_BLK_SSPLIT;
                }
                else if (frame.header.one.upstream.modifier == 3) // 3 <=> CSPLIT
                {
                    raceType = DTT_SPLIT_RACE_SPLIT_BLK_CSPLIT;
                }
                else
                {
                    raceType = DTT_SPLIT_RACE_SPLIT;
                    ilog_TOPOLOGY_COMPONENT_0(ILOG_FATAL_ERROR, TOPOLOGY_SPLIT_BLK_NO_SPLIT_PACKET);
                }
            }
        }
        else
        {
            raceType = DTT_SPLIT_RACE_SPLIT;
        }
    }

    return raceType;
}


/**
* FUNCTION NAME: ForceCleanupUsbAddr()
*
* @brief  - This function exists to force cleanup of a USB device to ensure that it is fully
*           cleaned up and wasn't waiting to be cleaned up by the timer task.
*
* @return - TRUE if the device was cleaned up.  FALSE if the device was not cleaned up because it
*           is currently valid and not waiting for a pending cleanup.  This can happen if a host
*           port resets two different devices in a row without doing a set address in the middle.
*           This is most often done by computer BIOS implementations.
*/
static boolT ForceCleanupUsbAddr(uint8 usbAddr)
{
    XUSB_AddressT xusbAddr = DTT_GetAddressFromUSB(usbAddr);
    boolT cleaned = TRUE;

    if(XCSR_getXUSBAddrValid(xusbAddr))
    {
        struct DeviceTopology* node = _DTT_GetDeviceNode(XCSR_getXUSBAddrLogical(xusbAddr));

        if(node->requiresCleanup)
        {
            ResetXsst(xusbAddr, node);
        }
        else
        {
            cleaned = FALSE;
        }
    }
    return cleaned;
}


/**
* FUNCTION NAME: DTT_WriteEndpointData()
*
* @brief  - Writes endpoint data to the xsst, updates the topology high endpoint data.
*
* @return - nothing
*/
void DTT_WriteEndpointData
(
    XUSB_AddressT address,
    uint8 configurationValue,
    uint8 interfaceNumber,
    uint8 alternateSetting,
    uint8 endpointNumber,
    enum EndpointTransferType endpointType,
    enum EndpointDirection endpointDirection,
    boolT blockAccess
)
{
    // The only difference between this function and the helper, is that helper returns a non-void
    // value for a type local to this C file.
    _DTT_WriteEndpointData(
        address,
        configurationValue,
        interfaceNumber,
        alternateSetting,
        endpointNumber,
        endpointType,
        endpointDirection,
        blockAccess);
}

/**
* FUNCTION NAME: _DTT_WriteEndpointData()
*
* @brief  - Writes endpoint data to the xsst, updates the topology high endpoint data
*
* @return - EndpointData node that was added to the EndpointData linked-list
*/
static struct EndpointData * _DTT_WriteEndpointData
(
    XUSB_AddressT address,
    uint8 configurationValue,
    uint8 interfaceNumber,
    uint8 alternateSetting,
    uint8 endpointNumber,
    enum EndpointTransferType endpointType,
    enum EndpointDirection endpointDirection,
    boolT blockAccess
)
{
    struct DeviceTopology * deviceNode;
    struct EndpointData * currentEndpoint;
    struct EndpointData * newEndpoint;
    struct EndpointData dummyEndpoint;
    struct EndpointData * ret;

    iassert_TOPOLOGY_COMPONENT_2(
        endpointNumber <= MAX_ENDPOINTS, TOPOLOGY_INVALID_ENDPOINT_ARG, endpointNumber, __LINE__);


    ilog_TOPOLOGY_COMPONENT_3(
        ILOG_DEBUG, ADD_ENDPOINT1, interfaceNumber, endpointNumber, endpointType);

    ilog_TOPOLOGY_COMPONENT_1(ILOG_DEBUG, ADD_ENDPOINT2, alternateSetting);

    newEndpoint = _DTT_CreateEndpointNode();

    if (newEndpoint == NULL)
    {
        ilog_TOPOLOGY_COMPONENT_0(ILOG_MAJOR_ERROR, MEMPOOL_ALLOC_FAILED);
        currentEndpoint = &dummyEndpoint;
    }
    else
    {
        currentEndpoint = newEndpoint;
    }

    // Populate endpoint data
    currentEndpoint->configurationValue = configurationValue;
    currentEndpoint->interfaceNumber = interfaceNumber;
    currentEndpoint->alternateSetting = alternateSetting;
    currentEndpoint->next = _DTT_poolPtrToIndex(NULL);
    currentEndpoint->isActive = FALSE; // Set by DTT_SetConfiguration and DTT_SetInterface
    currentEndpoint->endpointDirection = endpointDirection;
    currentEndpoint->msaPairEpNumber = NOT_MSA; // Initial value, caller can override
    currentEndpoint->blockAccess = blockAccess;
    currentEndpoint->endpointType = endpointType;
    currentEndpoint->endpointNumber = endpointNumber;

    // get the device node
    deviceNode = _DTT_GetDeviceNode(XCSR_getXUSBAddrLogical(address));

    // Insert the endpoint into the list
    // NOTE:    currentEndpoint may point a pre-existing endpoint
    //          So msaPairEpNumber setting is now valid
    if (newEndpoint != NULL)
    {
        // Insert Endpoint node to list sorted by endpoint number and then by endpoint direction
        currentEndpoint = _DTT_InsertEndpointToList(&(deviceNode->endpointDataList), newEndpoint);

        // If we found a pre-existing entry for this endpoint in the endpointDataList,
        // currentEndpoint is now a pointer to the existing entry. However, because the
        // blockAccess member is not used to identify an endpoint, the pre-existing endpoint
        // may have a different value for blockAccess than was passed into this function.
        // Thus, we need to update the blockAccess member with the freshest value.
        currentEndpoint->blockAccess = blockAccess;

        // At list point we can determine what our return value is
        // A pointer into the linked list.
        ret = currentEndpoint;
    }
    else
    {
        // Don't return a pointer to the stack, as we are using the dummy endpoint
        ret = NULL;
    }

    // Update the topology table with the endpoint number
    if (endpointNumber > deviceNode->highEndpoint)
    {
        deviceNode->highEndpoint = endpointNumber;
    }

    // 1st time through, write in all the endpoint settings.  Then if the mempool fills up before
    // the SetConfiguration, we will at least retain some settings.  Once the device is configured,
    // we don't want to mess with the current settings.
    if (deviceNode->configurationValue == 0)
    {
        // ALSO: DTT_SetConfiguration depends on valid settings already in the XSST for the case
        // with no EP conflicts.
        _DTT_WriteEndpointToXsst(address, currentEndpoint, deviceNode, FALSE);
    }

    return ret;
}

/**
* FUNCTION NAME: _DTT_WriteEndpointToXsst()
*
* @brief  - Writes the contents of an EndpointData struct to the XSST and
*           conditionally configures MSA for that endpoint
*
* @return - void
*
* @note   -
*/
static void _DTT_WriteEndpointToXsst
(
    XUSB_AddressT address,
    struct EndpointData * endpoint,
    struct DeviceTopology * deviceNode,
    boolT allowMsaChanges
)
{
    const uint8 endpointNumber = endpoint->endpointNumber;
    const uint8 endpointType = endpoint->endpointType;
    const enum EndpointDirection endpointDirection = endpoint->endpointDirection;
    boolT isOtherDirectionMsa = FALSE;
    {
        struct EndpointData * otherDirectionEndpoint = _DTT_FindEndpoint(
                    _DTT_poolIndexToPtr(deviceNode->endpointDataList),
                    endpoint->configurationValue,
                    endpoint->alternateSetting,
                    endpoint->endpointNumber,
                    !endpoint->endpointDirection);
        if (otherDirectionEndpoint != NULL)
        {
            isOtherDirectionMsa = (otherDirectionEndpoint->msaPairEpNumber != NOT_MSA);
        }
    }
    const boolT preserveMsa = (endpoint->msaPairEpNumber != NOT_MSA || !allowMsaChanges);
    const struct XCSR_Xsst oldXsst = { .sst =
            XCSR_XSSTWriteEndpoint(
                address,
                endpointNumber,
                endpointType,
                endpointDirection,
                deviceNode->isHub,
                isSplitDevice(XCSR_getXUSBAddrLogical(address)),
                isOtherDirectionMsa,
                preserveMsa,
                endpoint->blockAccess)};

    if (endpointDirection == EP_DIRECTION_OUT)
    {
        if (oldXsst.sstStruct.oEpType != endpointType)
        {
            // Only when an endpoint type changes, post a log event
            ilog_TOPOLOGY_COMPONENT_3(
                    ILOG_MAJOR_EVENT, TOPOLOGY_WRITING_OUT_ENDPOINT,
                    XCSR_getXUSBAddrUsb(address), endpointNumber, endpointType);
        }
    }
    else // IN direction
    {
        if (oldXsst.sstStruct.iEpType != endpointType)
        {
            // Only when an endpoint type changes, post a log event
            ilog_TOPOLOGY_COMPONENT_3(
                    ILOG_MAJOR_EVENT, TOPOLOGY_WRITING_IN_ENDPOINT,
                    XCSR_getXUSBAddrUsb(address), endpointNumber, endpointType);
        }
    }

    // Allocate pointer in the MSA status table
    if (endpoint->msaPairEpNumber != NOT_MSA && allowMsaChanges)
    {
        uint8 epIn;
        uint8 epOut;

        // Called with one endpoint (containing both IN and OUT) or two endpoint numbers which are to be paired.
        // First call with ep1 and ep2, this will write one or two entries to the pointer table
        // and the entry(s) will point to a single location in the status table.
        // Second call with ep2 and ep1 will do nothing since they are already configured.
        if (endpointDirection == EP_DIRECTION_IN)
        {
            epIn = endpoint->endpointNumber;
            epOut = endpoint->msaPairEpNumber;
        }
        else
        {
            epOut = endpoint->endpointNumber;
            epIn = endpoint->msaPairEpNumber;
        }
        _DTT_ConfigureMsa(address, epIn, epOut);
    }
}

/**
* FUNCTION NAME: DTT_AddNewMsaPair()
*
* @brief  - Write MSA information for an MSA pair to topology node.
* @return - nothing
*/
void DTT_AddNewMsaPair
(
    XUSB_AddressT address,
    uint8 configuration,
    uint8 interface,
    uint8 alternateSetting,
    uint8 inEndpointNumber,
    uint8 outEndpointNumber
)
{
    struct EndpointData * inEndpoint;
    struct EndpointData * outEndpoint;
    // This function must only be called for endpoints that we don't want to block.
    boolT blockAccess = FALSE;

    inEndpoint = _DTT_WriteEndpointData(
            address,
            configuration,
            interface,
            alternateSetting,
            inEndpointNumber,
            EP_TYPE_BULK,
            EP_DIRECTION_IN,
            blockAccess);

    outEndpoint = _DTT_WriteEndpointData(
            address,
            configuration,
            interface,
            alternateSetting,
            outEndpointNumber,
            EP_TYPE_BULK,
            EP_DIRECTION_OUT,
            blockAccess);


    // If Endpoint was not added topology because of memory pool being full
    if ((outEndpoint == NULL) || (inEndpoint == NULL))
    {
        // do nothing
    }
    else
    {
        outEndpoint->msaPairEpNumber = inEndpointNumber;
        inEndpoint->msaPairEpNumber = outEndpointNumber;
    }
}

/**
* FUNCTION NAME: DTT_SetInterface()
*
* @brief  - Change the alternate setting for a device's endpoints in a given interface.
*           Write the updated endpoint information back to the XCSR XSST.
*
* @return - nothing
*
* @note   - We currently only allow one active MSA pair per device.
*/
void DTT_SetInterface
(
    XUSB_AddressT address,
    uint8 interface,
    uint8 alternateSetting
)
{
    MSA_AddressT msaAddr;
    struct EndpointData * endpoint;
    struct EndpointData * candidateMsaEp[2] = {NULL, NULL};
    const uint8 logicalAddress = XCSR_getXUSBAddrLogical(address);
    struct DeviceTopology * deviceNode = _DTT_GetDeviceNode(logicalAddress);
    const uint8 cfgValue = deviceNode->configurationValue;

    // Verify that this device is configured
    if (cfgValue == 0)
    {
        ilog_TOPOLOGY_COMPONENT_3(ILOG_FATAL_ERROR, SET_INTERFACE_UNCONFIGURED,
                logicalAddress, interface, alternateSetting);
    }
    else
    {
        ilog_TOPOLOGY_COMPONENT_3(
            ILOG_MAJOR_EVENT, TOPOLOGY_SET_INTERFACE, logicalAddress, interface, alternateSetting);
    }

    // 1. Clean up all MSA settings and update the isActive fields of endpoints in this interface.
    msaAddr = XLR_msaGetAddrFromMsaLat(XCSR_getXUSBAddrUsb(address));
    if (XLR_msaAddressGetValid(msaAddr))
    {
        for (  endpoint = _DTT_poolIndexToPtr(deviceNode->endpointDataList);
               endpoint != NULL;
               endpoint = _DTT_poolIndexToPtr(endpoint->next)
            )
        {
            if (    endpoint->configurationValue == cfgValue
                &&  endpoint->interfaceNumber    == interface)
            {
                endpoint->isActive = (endpoint->alternateSetting == alternateSetting);

                if (endpoint->msaPairEpNumber != NOT_MSA)
                {
                    // Note that we may be taking down an MSA connection and only replacing it with
                    // an identical MSA connection. We could also be getting a call to move from the
                    // current interface alternate setting to the same alternate setting.
                    // Currently there doesn't seem to be much benefit in devoting additional logic
                    // to handling these special cases.

                    // For all MSA endpoints in the requested interface, free
                    // their respective entries from the MSA pointer and status table.
                    _DTT_clearMsaPair(msaAddr, endpoint->endpointNumber);

                    // Clear Overlay and Acceleration fields for the endpoint
                    XCSR_XSSTClearOverlayAndAccel(
                            address, endpoint->endpointNumber, endpoint->endpointDirection);
                    ilog_TOPOLOGY_COMPONENT_3(
                            ILOG_MAJOR_EVENT,
                            CLEAR_MSA_ENDPOINT,
                            logicalAddress,
                            endpoint->endpointNumber,
                            endpoint->endpointDirection);
                }
            }
        }
    }

    // At this point all accel and MSA-related overlay bits for the endpoints in this interface
    // have been cleared in the XSST.

    // 2. Check if there are any active MSA pairs for the device. If there are, they won't be on
    //    this interface, and we're done. If there aren't, enable the first (if there is one)
    //    available pair we encounter on this alternate setting.  If there are no active MSA pairs
    //    outside this interface, and no available ones for this alternate setting, look for
    //    inactive MSA pairs outside this interface to enable.  If we find one, enable it;
    //    otherwise, there are no available MSA pairs to enable on this device.
    for (  endpoint = _DTT_poolIndexToPtr(deviceNode->endpointDataList);
           endpoint != NULL;
           endpoint = _DTT_poolIndexToPtr(endpoint->next)
        )
    {
        if (     endpoint->isActive
             &&  endpoint->msaPairEpNumber != NOT_MSA
             &&  endpoint->interfaceNumber != interface
           )
        {
            if (_DTT_isMsaActive(
                    deviceNode->usbAddr, endpoint->endpointNumber, endpoint->endpointDirection))
            {
                // The existence of an active MSA-paired endpoint implies the existence of its
                // counterpart, so we don't need to look for it. At this point we know we don't have
                // to change anything related to MSA, since this device already has an active MSA
                // pair. NULL out the candidates so we don't try to enable them in step 4.
                candidateMsaEp[0] = NULL;
                candidateMsaEp[1] = NULL;
                break;
            }
            else if (candidateMsaEp[1] == NULL)
            {
                // We have an active, MSA-capable endpoint whose MSA is not currently enabled.
                // Also, this is the first one we've seen.
                // Hold on to it, as we may want to enable it and its counterpart below.
                candidateMsaEp[1] = endpoint;
            }
        }
        else if (
                endpoint->configurationValue == cfgValue
            &&  endpoint->interfaceNumber    == interface
            &&  endpoint->alternateSetting   == alternateSetting
            &&  endpoint->msaPairEpNumber    != NOT_MSA
            &&  candidateMsaEp[0]            == NULL
           )
        {
            // This endpoint on the alternate setting we're updating is a candidate for
            // participation in an active MSA pair. Also, this is the first one we've seen.
            // Hold on to it for now, and if we find no other active MSA pairs, we will enable it
            // and its counterpart.
            candidateMsaEp[0] = endpoint;
        }
    }

    // 4. If there are no currently active MSA interfaces for the device, and we have an MSA-capable
    //    active endpoint on this alternate setting (preferred) or elsewhere (second choice)...
    //
    //    NOTE: The meaning of the candidateMsaEp variable changes in this step to be the in/out
    //          direction of the chosen pair rather than the first/second choice for acceleration.
    if (candidateMsaEp[0] != NULL || candidateMsaEp[1] != NULL)
    {
        // Prefer candidate endpoints from the alternate setting we are enabling (index 0)
        const uint8 index = candidateMsaEp[0] != NULL ? 0 : 1;

        // Find our MSA-capable endpoint's counterpart, and enable it in step 5 below
        candidateMsaEp[!index] = _DTT_FindMsaEndpoint(
                _DTT_poolIndexToPtr(deviceNode->endpointDataList),
                candidateMsaEp[index]->configurationValue,
                candidateMsaEp[index]->alternateSetting,
                candidateMsaEp[index]->msaPairEpNumber,
                !candidateMsaEp[index]->endpointDirection);
        // MSA endpoints always come in pairs
        iassert_TOPOLOGY_COMPONENT_0(candidateMsaEp[!index] != NULL, MSA_ENDPOINT_NONEXISTENT);
    }

    // 5. Write back to the XSST
    for (  endpoint = _DTT_poolIndexToPtr(deviceNode->endpointDataList);
           endpoint != NULL;
           endpoint = _DTT_poolIndexToPtr(endpoint->next)
        )
    {
        // Since this XSST write will enable MSA for MSA-paired endpoints if its allowMsaChanges
        // parameter is true, only pass true for our (now known to be successful) candidate
        // endpoints. The OR clause in the if below is necessary because we may
        // need to enable MSA for endpoints not on the interface/alternate setting we're updating.
        const boolT isMsaEp = (endpoint == candidateMsaEp[0] || endpoint == candidateMsaEp[1]);
        if (    (     endpoint->configurationValue == cfgValue
                  &&  endpoint->interfaceNumber    == interface
                  &&  endpoint->alternateSetting   == alternateSetting
                )
             || isMsaEp
           )
        {
            _DTT_WriteEndpointToXsst(
                    address,
                    endpoint,
                    deviceNode,
                    (   candidateMsaEp[0] != NULL ? // [1] guaranteed non-NULL if [0] is non-NULL
                            isMsaEp : FALSE
                        // TODO? could potentially optimize out half of these XSST writes by
                        //       only writing for one direction per endpoint
                    )
            );

            if (isMsaEp)
            {
                ilog_TOPOLOGY_COMPONENT_3(
                    ILOG_MAJOR_EVENT,
                    TOPOLOGY_UPDATING_XSST_MSA,
                    XCSR_getXUSBAddrUsb(address),
                    endpoint->endpointNumber,
                    __LINE__);
            }
        }
    }
}


/**
* FUNCTION NAME: DTT_ParseConfigurationDone()
*
* @brief  - Perform any additional operations after a configuration descriptor has been read.
*
* @return - nothing
*/
void DTT_ParseConfigurationDone
(
    XUSB_AddressT address,  // Address of the device that the configuration is for
    uint8 configurationNum  // Number of the configuration that has completed parsing
)
{
    uint8 numConfigurationsSupported;
    uint8 numConfigurationsParsed;
    uint8 logicalAddress = XCSR_getXUSBAddrLogical(address);
    struct DeviceTopology* deviceNode = _DTT_GetDeviceNode(logicalAddress);

    // Disable MSA for mass storage endpoints when IN and OUT endpoints with one endpoint number
    // are paired with OUT and IN endpoints of a different endpoint number
    _DTT_DisableUnsupportedMSA(deviceNode);

    // configurationsParsed is a bit field, so the number of configurations supported is 8 times
    // the width of configurationsParsed.
    numConfigurationsSupported = sizeof(deviceNode->configurationsParsed) * 8;
    if (configurationNum <= numConfigurationsSupported)
    {
        deviceNode->configurationsParsed |= (1 << (configurationNum - 1));
    }

    numConfigurationsParsed = __builtin_popcount(deviceNode->configurationsParsed);
    if (numConfigurationsParsed == deviceNode->numConfigurations ||
        numConfigurationsParsed == numConfigurationsSupported)
    {
        // We have received all of the configuration descriptors for this device, so we can safely
        // erase all of the information for any endpoint number/direction which never changes for
        // all configurations, interfaces and alternate settings.

        memPoolIndex_t* prevNodeNextIndex = &(deviceNode->endpointDataList);
        struct EndpointData* endpoint = _DTT_poolIndexToPtr(*prevNodeNextIndex);
        struct EndpointData* endSetEndpoint = NULL;
        while (endpoint != NULL)
        {
            const boolT isEndpointSetEqual =
                _DTT_isEndpointSetEqualAndNoMsa(endpoint, &endSetEndpoint);
            endpoint = _DTT_poolIndexToPtr(endSetEndpoint->next);
            if (isEndpointSetEqual)
            {
                ilog_TOPOLOGY_COMPONENT_3(
                    ILOG_DEBUG,
                    TOPOLOGY_OPTIMIZING_ENDPOINT_SET,
                    logicalAddress,
                    endSetEndpoint->endpointNumber,
                    endSetEndpoint->endpointDirection);
                _DTT_WriteEndpointToXsst(address, endSetEndpoint, deviceNode, FALSE);
                _DTT_RemoveEndpointSet(prevNodeNextIndex, endSetEndpoint);
            }
            else
            {
                // endpoints are of different types, so keep them
                prevNodeNextIndex = &endSetEndpoint->next;
            }
        }
    }

    ilog_TOPOLOGY_COMPONENT_0(ILOG_DEBUG, DESC_DONE);
}

/**
* FUNCTION NAME: DTT_SetConfiguration()
*
* @brief  - Change the configuration to the specified value
*
* @return - FALSE if the set configuration request was not handled successfully.
*
* @note   - Before SetConfiguration is called endpoints are configured in the XSST.  In the case
*           where there are no conflicting EPs, then there is no need to write to the XSST.  If
*           alternateSetting 0, is at the start of the list there will be a harmless write,
*           otherwise the endpoints will be garbage collected before this function calls
*           _DTT_WriteEndpointToXsst().
*
*           If an endpoint is part of MSA, it will never be garbage collected,
*           so the above comment won't apply.
*
*           Also note that we currently only allow one MSA pair to be active for a device.
*           This function is one of the two places where we might activate an MSA pair,
*           the other being DTT_SetInterface().
*/
boolT DTT_SetConfiguration
(
    XUSB_AddressT address,          // Address of the device to set the configuration for
    uint8 configurationValue        // New configuration index
)
{
    struct EndpointData* endpoint = NULL;
    struct EndpointData * candidateMsaEp[2] = {NULL, NULL};
    const uint8 logicalAddress = XCSR_getXUSBAddrLogical(address);
    struct DeviceTopology* const deviceNode = _DTT_GetDeviceNode(logicalAddress);

    ilog_TOPOLOGY_COMPONENT_2(
        ILOG_MAJOR_EVENT, TOPOLOGY_SET_CONFIGURATION, logicalAddress, configurationValue);

    // New configuration.  Clean up any old MSA settings from the old configuration.
    _DTT_freeMsa(address, deviceNode, FALSE);

    if (configurationValue != 0)
    {
        // configurationsParsed is a bit field, so the number of configurations supported is 8
        // times the width in bytes of configurationsParsed.
        const uint8 numConfigurationsSupported = sizeof(deviceNode->configurationsParsed) * 8;
        if (configurationValue <= numConfigurationsSupported)
        {
            if (deviceNode->configurationsParsed & (1 << (configurationValue - 1)))
            {
                // This is a configuration which the descriptor parser has seen.  Just fall-through
                // to the code below.
            }
            else
            {
                // The host is doing a set configuration  and we haven't seen the full
                // configuration descriptor.
                ilog_TOPOLOGY_COMPONENT_0(ILOG_FATAL_ERROR, TOPOLOGY_SET_CONFIG_UNKNOWN);
                if (logicalAddress == ROOT_LOGICAL_ADDRESS)
                {
                    // If the host tries to do a set configuration on the root device and we
                    // haven't seen the full configuration descriptor, return FALSE which will
                    // cause a USB disconnect.  For devices other than the root device, we log the
                    // message above and continue on making our best effort to work.  It's likely
                    // that the device won't work properly because we probably won't have the
                    // correct endpoint types set in the XSST.  Ideally we would be able to
                    // encourage the host to do a port reset and then re-read the entire
                    // configuration descriptor, but we haven't found a way to do this yet.
                    return FALSE;
                }
            }
        }
        else
        {
            // We can't determine whether or not this configuration has been seen by the descriptor
            // parser since the configuration requested is greater than the number of
            // configurations tracked by configurationsParsed.  In practice, we have never seen a
            // USB device which has more than 2 configurations and the vast majority only have 1.
        }
    }

    // 1. Update the isActive field for all endpoints on the device, while searching for an
    //    MSA-capable endpoint in this (configuration, alternate setting) to later enable.
    for (  endpoint = _DTT_poolIndexToPtr(deviceNode->endpointDataList);
           endpoint != NULL;
           endpoint = _DTT_poolIndexToPtr(endpoint->next))
    {
        // Recall that a set configuration implies a set interface with alternate setting 0 for all
        // interfaces in the configuration.
        endpoint->isActive =
            (       endpoint->configurationValue == configurationValue
                &&  endpoint->alternateSetting == 0);

        if (endpoint->isActive && endpoint->msaPairEpNumber != NOT_MSA && candidateMsaEp[0] == NULL)
        {
            // We have an active, MSA-capable endpoint whose MSA is not currently enabled.
            // Also, this is the first one we've seen.
            // Hold on to it, and enable it and its counterpart below.
            candidateMsaEp[0] = endpoint;
        }
    }

    // 2. Find the corresponding endpoint of our MSA-capable candidate endpoint if we have one
    if (candidateMsaEp[0] != NULL)
    {
        // Find our MSA-capable endpoint's counterpart, and enable it in step 5 below
        candidateMsaEp[1] = _DTT_FindMsaEndpoint(
                _DTT_poolIndexToPtr(deviceNode->endpointDataList),
                candidateMsaEp[0]->configurationValue,
                candidateMsaEp[0]->alternateSetting, // MSA pairs are always found on the same A.S.
                candidateMsaEp[0]->msaPairEpNumber,
               !candidateMsaEp[0]->endpointDirection);
        // MSA endpoints always come in pairs
        iassert_TOPOLOGY_COMPONENT_0(candidateMsaEp[1] != NULL, MSA_ENDPOINT_NONEXISTENT);
    }

    // 3. Write back to the XSST
    for (  endpoint = _DTT_poolIndexToPtr(deviceNode->endpointDataList);
           endpoint != NULL;
           endpoint = _DTT_poolIndexToPtr(endpoint->next)
        )
    {
        if (endpoint->configurationValue == configurationValue && endpoint->alternateSetting == 0)
        {
            // Since this XSST write will enable MSA for MSA-paired endpoints if its allowMsaChanges
            // parameter is true, only pass true for our (now known to be successful) candidate
            // endpoints.
            const boolT isMsaEp = (endpoint == candidateMsaEp[0] || endpoint == candidateMsaEp[1]);
            _DTT_WriteEndpointToXsst(
                    address,
                    endpoint,
                    deviceNode,
                    (   candidateMsaEp[0] != NULL ? // [1] guaranteed non-NULL if [0] is non-NULL
                            isMsaEp : FALSE
                        // TODO? could potentially optimize out half of these XSST writes by
                        //       only writing for one direction per endpoint
                    )
            );

            if (isMsaEp)
            {
                ilog_TOPOLOGY_COMPONENT_3(
                        ILOG_DEBUG,
                        TOPOLOGY_UPDATING_XSST_MSA,
                        XCSR_getXUSBAddrUsb(address),
                        endpoint->endpointNumber,
                        __LINE__);
            }
        }
    }

    deviceNode->configurationValue = configurationValue;

    return TRUE;
}


/**
* FUNCTION NAME: DTT_SetOperatingSpeed()
*
* @brief  - Update the topology with the speed at which a device is operating
*
* @return - nothing
*/
void DTT_SetOperatingSpeed
(
    XUSB_AddressT parentAddress,    // parent address of the device to set speed for
    uint8 portNumber,               // port number on the parent
    enum UsbSpeed speed             // speed to set
)
{
    sint8 laToMark;
    uint8 parentLogicalAddress = XCSR_getXUSBAddrLogical(parentAddress);

    laToMark = FindNode(parentLogicalAddress, portNumber);
    iassert_TOPOLOGY_COMPONENT_3(
            laToMark >= 0,
            TOPOLOGY_SET_SPEED_MISSING_NODE,
            parentLogicalAddress,
            portNumber,
            speed);

    DTT_SetOperatingSpeedLA(laToMark, speed);
}

/**
* FUNCTION NAME: DTT_SetOperatingSpeedLA()
*
* @brief  - Update the topology with the speed at which a device is operating
*
* @return - nothing
*/
void DTT_SetOperatingSpeedLA
(
    uint8 LA,              // logical address of the device whose speed we wish to set
    enum UsbSpeed speed    // speed to set
)
{
    _DTT_GetDeviceNode(LA)->speed = speed;
    ilog_TOPOLOGY_COMPONENT_2(ILOG_DEBUG, TOPOLOGY_SET_SPEED, LA, speed);
}

void DTT_ResetEndpoint
(
    XUSB_AddressT address,
    uint8 endpoint
)
{
    XCSR_XSSTResetEndpoint(address, endpoint);
}


void DTT_ResetInterface
(
    XUSB_AddressT address,
    uint8 interface
)
{
    const uint8 logicalAddress = XCSR_getXUSBAddrLogical(address);
    struct DeviceTopology * deviceNode = _DTT_GetDeviceNode(logicalAddress);
    struct EndpointData * endpoint;

    for (   endpoint = _DTT_poolIndexToPtr(deviceNode->endpointDataList);
            endpoint != NULL;
            endpoint = _DTT_poolIndexToPtr(endpoint->next)
        )
    {
        if (endpoint->interfaceNumber == interface)
        {
            XCSR_XSSTResetEndpoint(address, endpoint->endpointNumber);
        }
    }
}


/**
* FUNCTION NAME: DTT_describeTopology()
*
* @brief  - Provides clients with a means to get a read-only summary of the current contents of the
*           topology by way of a function callback for each node in the topology.
*
* @return - None
*
* @note   - This function was designed this way in order to prevent exposing the internals of the
*           topology component.
*/
void DTT_describeTopology(DTT_DescriptionHandlerT dhCallback, void* cbData)
{
    struct DescribeTopologyContext ctxt = { .descriptionHandler=dhCallback, .callbackData=cbData };
    TraverseTopology(
        ROOT_LOGICAL_ADDRESS,
        _DTT_GetDeviceNode(ROOT_LOGICAL_ADDRESS),
        &describeTopologyHelper,
        &ctxt);
}


/**
* FUNCTION NAME: _DTT_memPoolIndexToEndpoint()
*
* @brief  - Converts a memory pool index value into a struct EndpointData pointer.
*
* @return - A struct EndpointData pointer which may be NULL if the null index was passed to this
*           function.
*
* @note   - This function is simply a call-through to the regular ipool function that would not
*           otherwise be visible outside of this file.
*/
struct EndpointData* _DTT_memPoolIndexToEndpoint(memPoolIndex_t index)
{
    return _DTT_poolIndexToPtr(index);
}


/**
* FUNCTION NAME: _DTT_ClearEndpoints
*
* @brief  - Clear all endpoint and alternate settings information from topology
*
* @return - None
*
* @note   -
*/

void _DTT_ClearEndpoints
(
    uint8 logicalAddress
)
{
    struct EndpointData * endpoint;
    struct EndpointData * nextEndpoint;
    boolT isEmpty = TRUE;
    struct DeviceTopology * node = _DTT_GetDeviceNode(logicalAddress);

    endpoint = (struct EndpointData*)_DTT_poolIndexToPtr(node->endpointDataList);

    while (endpoint != NULL)
    {
        isEmpty = FALSE;

        nextEndpoint = _DTT_poolIndexToPtr(endpoint->next);

        _DTT_poolFree(endpoint);

        endpoint = nextEndpoint;
    }

    node->endpointDataList = _DTT_poolPtrToIndex(NULL);

    if (!isEmpty)
    {
        ilog_TOPOLOGY_COMPONENT_1(ILOG_MAJOR_EVENT, CLEAR_ENDPOINTS, logicalAddress);
    }
}

/**
* FUNCTION NAME: _DTT_InsertEndpointToList
*
* @brief  - Inserts endpoints to the list sorted by endpoint number and
*           endpoint direction.
*
* @return - A pointer to the endpoint inserted into the list.  The pointer will be the same as the
*           newEndpointData argument so long as an identical endpoint does not already exist in the
*           list.  If an identical endpoint is found, a pointer to it will be returned and the
*           resources for the endpoint passed to this function will be released back to the memory
*           pool.
*
* @note   - None
*/
static struct EndpointData * _DTT_InsertEndpointToList
(
    memPoolIndex_t * endpointIndex,
    struct EndpointData * newEndpointData
)
{
    struct EndpointData * endpointData = _DTT_poolIndexToPtr(*endpointIndex);

    while (endpointData != NULL)
    {
        // If the endpointData number and direction of the new node is less than that
        // of the existing nodes, stop traversing the list and insert the
        // new node before the existing node
        if ((endpointData->endpointNumber > newEndpointData->endpointNumber) ||
            ((endpointData->endpointNumber == newEndpointData->endpointNumber) &&
             (endpointData->endpointDirection > newEndpointData->endpointDirection)))
        {
            break;
        }

        // check if new endpointData already exists by comparing with
        // endpointData because the endpoints are cleared only if the product
        // and vendor IDs are different; if the product and vendor IDs
        // are the same, duplicate endpoints will be added
        else if (
            (endpointData->configurationValue == newEndpointData->configurationValue) &&
            (endpointData->alternateSetting == newEndpointData->alternateSetting) &&
            (endpointData->endpointNumber == newEndpointData->endpointNumber) &&
            (endpointData->endpointDirection == newEndpointData->endpointDirection)
            // NOTE: DO NOT CHECK -> (endpointData->msaPairEpNumber == newEndpointData->msaPairEpNumber)
            // The msaPairEpNumber for this new endpoint is not set until after the insertion
            )
        {
            // Same endpoint as one already in the device list

            if (endpointData->interfaceNumber != newEndpointData->interfaceNumber)
            {
                // This is a bug, because endpoint (# & direction) can not be shared between interfaces
                // This could be an Icron bug, or a device bug
                ilog_TOPOLOGY_COMPONENT_2(ILOG_FATAL_ERROR, ENDPOINT_SHARED_INTF, newEndpointData->configurationValue,
                        newEndpointData->endpointNumber);

                // Do nothing special, just add the new endpoint to the list
            }
            else
            {
                // Uniquely identified endpoint is already in the device list

                if (endpointData->endpointType != newEndpointData->endpointType)
                {
                    // This is a bug, because a uniquely identified endpoint has 2 different types
                    // This could be an Icron bug, or a device bug
                    ilog_TOPOLOGY_COMPONENT_3(ILOG_FATAL_ERROR, EXISTING_ENDPOINT_TYPE, newEndpointData->configurationValue,
                            newEndpointData->endpointNumber, newEndpointData->endpointDirection);

                    // New information is probably more accurate than old information, so lets update the type
                    endpointData->endpointType = newEndpointData->endpointType;
                }
                else
                {
                    // This exact endpoint data is already in the device list
                }

                // Don't add new endpoint as it is already in the device list
                _DTT_poolFree(newEndpointData);
                return endpointData;
            }
        }

        endpointIndex = &(endpointData->next);
        endpointData = _DTT_poolIndexToPtr(endpointData->next);
    }

    *endpointIndex =  _DTT_poolPtrToIndex(newEndpointData);

    newEndpointData->next = _DTT_poolPtrToIndex(endpointData);

    return newEndpointData;
}


/**
* FUNCTION NAME: _DTT_CreateEndpointNode
*
* @brief  - Create a new endpoint node
*
* @return - Returns a new endpoint node, or NULL on failure
*/
static struct EndpointData * _DTT_CreateEndpointNode(void)
{
    struct EndpointData * endpoint = _DTT_poolAlloc();

    ilog_TOPOLOGY_COMPONENT_2(ILOG_DEBUG, TOPOLOGY_MEMPOOL,
            _DTT_poolGetNumOfUsedElements(),
            _DTT_poolGetNumOfFreeElements());

    if (endpoint == NULL)
    {
        endpoint = _DTT_HandleMemPoolFull();
    }

    return endpoint;
}


/**
* FUNCTION NAME: _DTT_RemoveUnnecessaryEndpoints
*
* @brief  - Remove all unnecessary endpoints, i.e. endpoints
*           for which endpoint type does not change for all
*           alternate settings.
*
* @return - None.
*
* @note   - A set is a group of endpoint settings that are of the same endpoint #, endpoint
*           direction(GE only)
*/


/**
* FUNCTION NAME:_DTT_isEndpointSetEqualAndNoMsa
*
* @brief  - In an endpoint set (set of endpoints with the same endpoint number
*           and direction), check whether the endpoint type is the same
*           and whether any endpoint is MSA
* @return - True : if all endpoints are the same type in the endpoint set
*           False: if any endpoint is not the same type or if any endpoint
*                  is MSA in the endpoint set
*/
static boolT _DTT_isEndpointSetEqualAndNoMsa
(
    struct EndpointData *  startSetEndpoint,    // INPUT
    struct EndpointData ** endSetEndpoint       // OUTPUT: returns the last endpoint in this set
)
{
    struct EndpointData * prevEndpoint;
    struct EndpointData * currEndpoint;
    boolT areEqualAndNotMsa = TRUE;
    uint8 endpointNumber;
    uint8 endpointType;
    uint8 endpointDirection;

    iassert_TOPOLOGY_COMPONENT_0(startSetEndpoint != NULL, IS_EP_SET_EQ_WITH_NULL_ARG);

    // Set up our endpoint set information
    endpointNumber = startSetEndpoint->endpointNumber;
    endpointType = startSetEndpoint->endpointType;
    endpointDirection = startSetEndpoint->endpointDirection;

    // We need to compare the first element in the list to itself, because the first element might be MSA
    prevEndpoint = startSetEndpoint;
    currEndpoint = startSetEndpoint;

    // Loop through this entire set
    while (     (currEndpoint != NULL)
            &&  (currEndpoint->endpointNumber == endpointNumber)
            &&  (currEndpoint->endpointDirection == endpointDirection)
          )
    {
        // On each loop note if this is no longer a set of identical endpoints with no MSA
        if (currEndpoint->msaPairEpNumber != NOT_MSA || currEndpoint->endpointType != endpointType)
        {
            areEqualAndNotMsa = FALSE;
        }

        prevEndpoint = currEndpoint;
        currEndpoint = _DTT_poolIndexToPtr(currEndpoint->next);
    }

    // Set the last endpoint in the set & return
    *endSetEndpoint = prevEndpoint;
    return areEqualAndNotMsa;
}

/**
* FUNCTION NAME:_DTT_RemoveEndpointSet
*
* @brief  - Remove all the endpoints in the endpoint set
* @return - none
*
*/

static void _DTT_RemoveEndpointSet
(
    memPoolIndex_t * prevNodeNextIndex,     // This is a pointer to the previous node's next index.
                                            // By manipulating the value pointed to, we are able to
                                            // splice elements out of the list.  This parameter is
                                            // also used in order to determine the first element in
                                            // the set.

    struct EndpointData * endSetEndpoint    // This is the last endpoint in the set.
)
{
    memPoolIndex_t stopIndex;
    memPoolIndex_t endpointIndex = *prevNodeNextIndex;

    // update the prevNodeNextIndex (which is a next pointer of the previous set) to the next set
    *prevNodeNextIndex = endSetEndpoint->next;

    // Delete all endpoints in the current set
    stopIndex = endSetEndpoint->next;
    while (endpointIndex != stopIndex)
    {
        struct EndpointData * tmp = _DTT_poolIndexToPtr(endpointIndex);
        endpointIndex = tmp->next;
        _DTT_poolFree(tmp);
    }
}

/**
* FUNCTION NAME: _DTT_HandleMemPoolFull()
*
* @brief  - Delete endpoints to allow allocating new endpoints
*
* @return - An allocated endpoint if one can be found, NULL otherwise
*/

static struct EndpointData * _DTT_HandleMemPoolFull(void)
{
    uint8 logicalAddr;
    struct EndpointData * endpoint = NULL;

    ilog_TOPOLOGY_COMPONENT_0(ILOG_MAJOR_EVENT, DELETE_DISCONNECTEDDEVICES);

    // Delete all disconnected devices
    for (
            logicalAddr = 0;
            (logicalAddr < MAX_USB_DEVICES) && (endpoint == NULL);
            logicalAddr++
        )
    {
        struct DeviceTopology * node = _DTT_GetDeviceNode(logicalAddr);

        if (!node->isConnected)
        {
            _DTT_ClearEndpoints(logicalAddr);

            // Try allocating again
            endpoint = _DTT_poolAlloc();
        }
    }

    // No memory is still available in the pool
    if (endpoint == NULL)
    {
        ilog_TOPOLOGY_COMPONENT_0(ILOG_MAJOR_EVENT, DELETE_CONFIGURED);

        // Delete endpoints for which set configuration has been received
        for (   logicalAddr = 0;
                (logicalAddr < MAX_USB_DEVICES) && (endpoint == NULL);
                logicalAddr++
            )
        {
            struct DeviceTopology * node = _DTT_GetDeviceNode(logicalAddr);

            // Since endpoint information is deleted for all configured devices, set interface will
            // not work.
            if (node->configurationValue != 0)
            {
                _DTT_ClearEndpoints(logicalAddr);

                ilog_TOPOLOGY_COMPONENT_1(ILOG_MAJOR_EVENT, SET_INTERFACE_MEM, logicalAddr);
                // Try allocating again
                endpoint = _DTT_poolAlloc();
            }
        }
    }

    return endpoint;
}

/**
* FUNCTION NAME: _DTT_DisableUnsupportedMSA()
*
* @brief  -  Disable MSA for mass storage endpoints when IN and OUT endpoints with one
*            endpoint number are paired with OUT and IN endpoints of a different
*            endpoint number, respectively
*
* @return - None.
*/
static void _DTT_DisableUnsupportedMSA(struct DeviceTopology* deviceNode)
{
    /*
    *  Let X, Y and Z be endpoint numbers
    *  Let A and Not(A) be endpoint directions
    *
    *  Endpoint number   : X ,
    *  Endpoint direction: A ------------ MSA Pair ------- Endpoint number   : Y
    *                                                      Endpoint direction: Not(A)
    *
    *  Endpoint number   : X ,
    *  Endpoint direction: Not(A)-------- MSA Pair ------- Endpoint number   : Z
    *                                                      Endpoint direction: A
    *
    * If the above case exists, disable MSA for the first pairing only.
    *
    */

    for (   struct EndpointData * endpoint = _DTT_poolIndexToPtr(deviceNode->endpointDataList);
            endpoint != NULL;
            endpoint = _DTT_poolIndexToPtr(endpoint->next))
    {
        // Endpoint is MSA and endpoint number is not equal to MSA-paired endpoint number
        if (    (endpoint->msaPairEpNumber != NOT_MSA)
             && (endpoint->msaPairEpNumber != endpoint->endpointNumber))
        {
            struct EndpointData * endpointOppositeDirection = _DTT_FindMsaEndpoint(
                endpoint,
                endpoint->configurationValue,
                endpoint->alternateSetting,
                endpoint->endpointNumber,
                !endpoint->endpointDirection);
            if (endpointOppositeDirection == NULL)
            {
                // No conflicts, the opposite direction isn't an MSA endpoint (or doesn't even exist)
            }
            else
            {
                // This opposite direction is MSA, and it isn't a pair of this EP
                // ISSUE: Lets disable MSA on this pair
                // NOTE: The other pair will continue to work, as this pair will be deconfigured as
                //       MSA, and no more conflicts should arise.
                struct EndpointData * pairedEndpoint = _DTT_FindMsaEndpoint(
                    _DTT_poolIndexToPtr(deviceNode->endpointDataList),
                    endpoint->configurationValue,
                    endpoint->alternateSetting,
                    endpoint->msaPairEpNumber,
                    !endpoint->endpointDirection);

                // This should always exist, as we work in pairs
                iassert_TOPOLOGY_COMPONENT_0(pairedEndpoint != NULL, MSA_ENDPOINT_NONEXISTENT);

                // Do the disable
                endpoint->msaPairEpNumber = NOT_MSA;
                pairedEndpoint->msaPairEpNumber = NOT_MSA;
            }
        }
    }
}

/**
* FUNCTION NAME: _DTT_FindMsaEndpoint()
*
* @brief  - Find MSA endpoint
*
* @return - None.
*/
static struct EndpointData * _DTT_FindMsaEndpoint
(
    struct EndpointData * endpointStart,
    uint8 configurationValue,
    uint8 alternateSetting,
    uint8 endpointNumber,
    enum EndpointDirection endpointDirection
)
{
    struct EndpointData * endpoint;

    endpoint = _DTT_FindEndpoint(
        endpointStart, configurationValue, alternateSetting, endpointNumber, endpointDirection);

    if (endpoint != NULL)
    {
        if (endpoint->msaPairEpNumber == NOT_MSA)
        {
            endpoint = NULL;
        }
    }

    return endpoint;
}

/**
* FUNCTION NAME: _DTT_FindEndpoint()
*
* @brief  - Find endpoint with the following combination:
*           - Configuration value
*           - Alternate Setting
*           - Endpoint Number
*           - Endpoint Direction
*
* @return - None.
*/
static struct EndpointData * _DTT_FindEndpoint
(
    struct EndpointData * endpointStart,
    uint8 configurationValue,
    uint8 alternateSetting,
    uint8 endpointNumber,
    enum EndpointDirection endpointDirection
)
{
    struct EndpointData * endpoint = endpointStart;

    // NOTE: The list is sorted, we can stop once we've passed the desired endpoint
    while (endpoint != NULL)
    {
        if (endpoint->endpointNumber > endpointNumber)
        {
            // Force out of the loop, we have passed the desired endpoint
            endpoint = NULL;
        }
        else if ((endpoint->configurationValue == configurationValue) &&
                (endpoint->alternateSetting == alternateSetting) &&
                (endpoint->endpointNumber == endpointNumber) &&
                (endpoint->endpointDirection == endpointDirection)
                )
        {
            // Exit condition, endpoint found
            break;
        }
        else
        {
            endpoint = _DTT_poolIndexToPtr(endpoint->next);
        }
    }
    return endpoint;
}

/**
* FUNCTION NAME: describeTopologyHelper()
*
* @brief  - This function is of the type that can be passed to the TraverseTopology function.  This
*           function calls another function via a function pointer embedded in the void* argument
*           in order to report topology information back to an interested party.
*
* @return - None.
*/
static void describeTopologyHelper(
    XUSB_AddressT address, struct DeviceTopology* node, void* arg)
{
    struct DescribeTopologyContext* context = arg;
    if(XCSR_getXUSBAddrInSys(address))
    {
        const uint8 usbAddress = node->usbAddr;
        // We say that the parent USB address is zero when we are looking at
        // the root device because we don't know if there is a parent device of
        // the REX hub.
        const uint8 parentUSBAddress = XCSR_getXUSBAddrLogical(address) == ROOT_LOGICAL_ADDRESS ?
            0 : _DTT_GetDeviceNode(node->parentLA)->usbAddr;
        const uint8 portOnParent = node->portOnParent;
        const boolT isDeviceHub = node->isHub;
        const uint16 vendorId = node->idVendor;
        const uint16 productId = node->idProduct;
        (*context->descriptionHandler)(
            usbAddress,
            parentUSBAddress,
            portOnParent,
            isDeviceHub,
            vendorId,
            productId,
            context->callbackData);
    }
}


/**
* FUNCTION NAME: FindFirstHighSpeedAncestor()
*
* @brief  - Finds the nearest ancestor (upstream) high-speed hub for a device
*
* @return - TRUE if the device has a high-speed hub ancestor, FALSE otherwise
*
* @note   - the output parameters are only written on a TRUE return
*         - it is valid to pass null pointers for the output parameters if you don't
*           need their values
*/
static boolT FindFirstHighSpeedAncestor
(
    uint8 deviceLA,          // Logical address of the device whose upstream HS ancestor we are searching for
    uint8 * highSpeedHubLA,  // Output parameter - written with the logical address of the first
                             // high-speed ancestor of the device
    uint8 * descendantPort   // Output parameter - written with the port on the first HS
                             // ancestor of the device corresponding to deviceLA
)
{
    boolT found = FALSE;
    const struct DeviceTopology * currentNode = _DTT_GetDeviceNode(deviceLA);

    if (deviceLA != ROOT_LOGICAL_ADDRESS)
    {
        uint8 currentLA;
        do
        {
            const uint8 portOfDescendant = currentNode->portOnParent;
            currentLA = currentNode->parentLA;
            currentNode = _DTT_GetDeviceNode(currentNode->parentLA);
            if (currentNode->isHub && currentNode->speed == USB_SPEED_HIGH)
            {
                found = TRUE;
                if (highSpeedHubLA != NULL)
                {
                    *highSpeedHubLA = currentLA;
                }
                if (descendantPort != NULL)
                {
                    *descendantPort = portOfDescendant;
                }
            }
        } while (!found && currentLA != ROOT_LOGICAL_ADDRESS);
    }

    return found;
}

/**
* FUNCTION NAME: isSplitDevice()
*
* @brief  - Returns whether a device is a split device (low/full speed device with an ancestor
*           hub present in our topology that is operating at high speed)
*
* @return - TRUE if the device is a split device, otherwise FALSE
*/
boolT isSplitDevice
(
    uint8 LA    // Logical address of the device we are checking for splitness
)
{
    const struct DeviceTopology * deviceNode = _DTT_GetDeviceNode(LA);
    return ( (deviceNode->speed == USB_SPEED_LOW || deviceNode->speed == USB_SPEED_FULL) &&
            FindFirstHighSpeedAncestor(LA, NULL, NULL));
}

