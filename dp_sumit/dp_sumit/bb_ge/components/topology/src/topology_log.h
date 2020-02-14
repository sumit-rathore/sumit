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
//!   @file  -  topology_log.h
//
//!   @brief -  logging file for the topology component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef TOPOLOGY_LOG_H
#define TOPOLOGY_LOG_H

/***************************** Included Headers ******************************/
#include <project_components.h>
#include <ilog.h>


/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/

ILOG_CREATE(TOPOLOGY_COMPONENT)

    ILOG_ENTRY(TOPOLOGY_INVALID_LOGICAL_ADDRESS_ARG, "Invalid logical address received %d @ line %d\n")
    ILOG_ENTRY(TOPOLOGY_INVALID_ENDPOINT_ARG, "Invalid endpoint received %d @ line %d\n")

    ILOG_ENTRY(TOPOLOGY_ICMD_NOT_LEX, "Duh, this is REX!!!!\n")
    ILOG_ENTRY(TOPOLOGY_ICMD_SHOW_DEVICE, "ShowDevice\n")
    ILOG_ENTRY(TOPOLOGY_ICMD_SHOW_DEVICE_ALL, "ShowDeviceAll\n")
    ILOG_ENTRY(TOPOLOGY_ICMD_SHOW_TOPOLOGY_BY_USB, "Device Topology Ordered By USB Address:\n")
    ILOG_ENTRY(TOPOLOGY_ICMD_SHOW_TOPOLOGY_BY_LA, "Device Topology Ordered By Logical Address:\n")
    ILOG_ENTRY(TOPOLOGY_ICMD_WRITE_XSST, "ShowTopologyByLogical\n")
    ILOG_ENTRY(TOPOLOGY_ICMD_WRITING_XSST1, "Writing (0x%x, 0x%x) to XSST\n")
    ILOG_ENTRY(TOPOLOGY_ICMD_WRITING_XSST2, "at address %d, endpoint %d\n")


    ILOG_ENTRY(TOPOLOGY_DEVICE_ADDING_DEVICE, "DEVICE_Add: adding device LA=%d, pLA=%d, portOnParent=%d\n")
    ILOG_ENTRY(TOPOLOGY_PORT_NOT_FOUND, "DEVICE_FindPort: port %d on parent %d not found\n")
    ILOG_ENTRY(TOPOLOGY_DISCONNECT, "Disconnecting node (LA %d) from parent %d @ port %d\n")

    ILOG_ENTRY(TOPOLOGY_INVALID_LA_IN_ICMD, "Invalid logical address %d received in ShowDevice iCmd\n")
    ILOG_ENTRY(TOPOLOGY_ICMD_VIEWING_ADDRESS_NOT_IN_SYS2, "Using unused USB address to view logical address %d\n")

    ILOG_ENTRY(TOPOLOGY_SHOW_XSST_RETRY_USAGE, "XSST Monitor: USB %d, endpoint %d has rtyUsage as 0x%x\n")

    ILOG_ENTRY(GET_ADDR_FROM_LOGICAL_MISMATCH, "Get address from logical had a mismatch.  Logical arg is %d, USB address from logical is %d, Logical address from USB is %d\n")


    ILOG_ENTRY(DISPLAY_XSST, "LA(upper bits), endpoint(lower bits) 0x%x, XSST: 0x%x 0x%x\n")
    ILOG_ENTRY(TOPOLOGY_ICMD_SHOW_XSST, "ShowXSST\n")
    ILOG_ENTRY(TOPOLOGY_SET_ADDRESS_ON_EXISTING, "Logical address %d has moved from USB address %d to USB address %d\n")
    ILOG_ENTRY(XSSTMON_LEX_SPECIFIC_ICMD, "The command executed is only valid on a device configured as a LEX\n")
    ILOG_ENTRY(HOST_REUSING_USB_ADDR, "The host is re-using USB address %d\n")
// MSA
    ILOG_ENTRY(MSA_MISCONFIGURED_PTRS1, "MSA detected misconfigured ptrs on USB address %d, inEp is %d, inPtr is %d\n")
    ILOG_ENTRY(MSA_MISCONFIGURED_PTRS2, "MSA detected misconfigured ptrs on USB address %d, outEp is %d, outPtr is %d\n")
    ILOG_ENTRY(MSA_FREE_LAT_FAILURE, "MSA Free LAT failure, MSA USB address %d, MSA LA address %d, msaLogicalAddressAllocatedBitField 0x%x\n")
    ILOG_ENTRY(MSA_FREE_INVALID, "MSA Free was given an invalid address, usb is %d, MSA LA is %d\n")
    ILOG_ENTRY(NEW_MSA_INTERFACE, "New MSA interface configured on USB %d, epIn %d, epOut %d\n")
    ILOG_ENTRY(MSA_OUT_OF_PTRS, "MSA HW is out of pointers\n")

    ILOG_ENTRY(RESET_XSST, "ResetXsst: LA %d, USB %d\n")
    ILOG_ENTRY(TOPOLOGY_INVALID_WHEN_GET_ADDR_FROM_USB, "Tried to get an XUSB address from a USB address (%d) without valid topology data at line %d\n")
    ILOG_ENTRY(TOPOLOGY_INVALID_TREE_STRUCTURE, "A topology tree invariant has been violated at line %d.\n")

    ILOG_ENTRY(MSA_TOPOLOGY_MISMATCH, "An MSA topology mismatch occured on USB address %d at line %d\n")
    ILOG_ENTRY(TOPOLOGY_EXPECTED_DEVICE_REQUIRING_CLEANUP, "Device with USB address %d should have required cleanup at line %d\n")
    ILOG_ENTRY(TOPOLOGY_INVALID_DEVICE_CONNECTED, "Found a connected device with LA %d that is not valid, but is connected at line %d\n")
    ILOG_ENTRY(TRAVERSE_TOPOLOGY, "Traverse: processing logical address %d\n")

    ILOG_ENTRY(LOGITECH_C910_FOUND, "Logitech C910 device found at usb address %d, configuring endpoint 2 as ISO\n")
    ILOG_ENTRY(FOUND_NEW_PRODUCT, "Found New Product. Usb Address: %d, Old Product ID: %d, New Product ID: %d\n")
    ILOG_ENTRY(FOUND_NEW_VENDOR, "Found New Vendor. Usb Address: %d, Old Vendor ID: %d, New Vendor ID: %d\n")
    ILOG_ENTRY(MSA_FREE_ADDR, " MSA freeing MSA LA %d, USB %d\n")
    ILOG_ENTRY(TOPOLOGY_ASSERT_XSST, "--- XSST for Devices in the Device Topology ---\n")

    ILOG_ENTRY(ADD_ENDPOINT1, "Adding Endpoint. Interface: %d, Endpoint Number: %d, Endpoint type: %d ...\n")
    ILOG_ENTRY(ADD_ENDPOINT2, "... Adding Endpoint. Alternate Setting: %d\n")
    ILOG_ENTRY(TOPOLOGY_SET_INTERFACE, "Setting Interface for LA: %d to Interface: %d, Alternate setting: %d\n")
    ILOG_ENTRY(DESC_DONE, "Descriptor has been parsed\n")
    ILOG_ENTRY(CLEAR_ENDPOINTS, "Cleared Endpoints for LA: %d\n")
    ILOG_ENTRY(TOPOLOGY_SET_CONFIGURATION, "Setting Configuration for LA: %d to value: %d\n")

    ILOG_ENTRY(ENDPOINT_INFO,               "    Interface: %d, Endpoint: %d, Type: %d\n")
    ILOG_ENTRY(MEMPOOL_ALLOC_FAILED, "Memory pool in topology is full and data will not be saved\n")
    ILOG_ENTRY(SET_INTERFACE_MEM, "Endpoints for device with LA: %d were deleted; Set Interface will not work\n")
    ILOG_ENTRY(DELETE_DISCONNECTEDDEVICES, "Removing endpoints for disconnected devices\n")
    ILOG_ENTRY(DELETE_CONFIGURED, "Deleting configured devices to free some space in the memory pool\n")
    ILOG_ENTRY(TOPOLOGY_MEMPOOL, "Number of used slots in topology memory pool: %d; number of free slots: %d\n")

    ILOG_ENTRY(NEW_DEVICE, "New device at USB %d, Vendor ID 0x%.4x, Product ID 0x%.4x\n")
    ILOG_ENTRY(DTT_GET_DEV_NODE_INVALID_ARG, "DTT get dev node invalid arg\n")
    ILOG_ENTRY(DISCONNECT_SUBTREE, "Disconnecting subtree rooted at logical address %d\n")
    ILOG_ENTRY(CONFIGURATION_VALUE,         "    Configuration value: %d\n")
    ILOG_ENTRY(ENDPOINT_INFO2,              "    Endpoint Direction: %d, MSA Paired Endpoint Number: %d, Alternate Setting: %d\n")
    ILOG_ENTRY(MSA_ENDPOINT_NONEXISTENT, "MSA Pair Endpoint does not exist\n")
    ILOG_ENTRY(BCI_BCO_SET, "Both BCI and BCO are set for LA: %d\n")
    ILOG_ENTRY(CLEAR_MSA_ENDPOINT, "Clearing MSA Overlay and Acceleration field for LA: %d, Endpoint %d, Direction %d\n")
    ILOG_ENTRY(SET_INTERFACE_UNCONFIGURED, "Setting Interface called on unconfigured device for LA: %d to Interface: %d, Alternate setting: %d\n")
    ILOG_ENTRY(IS_EP_SET_EQ_WITH_NULL_ARG, "_DTT_isEndpointSetEqualAndNoMsa: called with NULL arg\n")
    ILOG_ENTRY(SHOW_VIRT_FN,                "  Virtual function at address 0x%x\n")
    ILOG_ENTRY(SHOW_VIRT_FN_1,              "    ep: %d, epType: %d\n")
    ILOG_ENTRY(SHOW_VIRT_FN_2,              "    inFn: 0x%x, inAckFn: 0x%x\n")
    ILOG_ENTRY(SHOW_VIRT_FN_3,              "    outFn: 0x%x, setupFn: 0x%x\n")
    ILOG_ENTRY(TOPOLOGY_SHOW_TOPOLOGY_1,    "Logical Address=%d, USB Address=%d\n")
    ILOG_ENTRY(TOPOLOGY_SHOW_TOPOLOGY_2,    "  Parent LA=%d, Child LA=%d, Sibling LA=%d\n")
    ILOG_ENTRY(TOPOLOGY_SHOW_TOPOLOGY_5,    "  Requires Cleanup=%d, Max Packet Size Ep0=%d, System Control Q State=%d\n")
    ILOG_ENTRY(TOPOLOGY_SHOW_ENDPOINTS_INFO,"  Stored Endpoint Information:\n")
    ILOG_ENTRY(INPUT_STUCK_ENDPOINT, "Input Endpoint at LA: %d, Endpoint: %d is stuck.\n")
    ILOG_ENTRY(OUTPUT_STUCK_ENDPOINT, "Output Endpoint at LA: %d, Endpoint: %d is stuck\n")
    ILOG_ENTRY(TOPOLOGY_HOST_RESET_SHOULD_NEVER_FAIL, "Host reset should never fail, but it did\n")
    ILOG_ENTRY(ENDPOINT_SHARED_INTF, "In Configuration %d, Endpoint %d is being shared by two distinct interfaces\n")
    ILOG_ENTRY(EXISTING_ENDPOINT_TYPE, "In Configuration %d, adding existing endpoint with endpoint number %d, endpoint direction %d but with different endpoint type\n")
    ILOG_ENTRY(DISCONNECT_DEVICE, "Disconnecting device USB %d, logical %d\n")
    ILOG_ENTRY(RELEASE_RETRY, "In XSST Monitor, releasing Retry Buffers for USB Addr: %d, Endpoint: %d\n")
    ILOG_ENTRY(START_XSST, "XSST Monitor has been started\n")
    ILOG_ENTRY(STOP_XSST, "XSST Monitor has been stopped\n")
    ILOG_ENTRY(FLUSH_IN_QUEUE, "Flushing Input Queue for device at LA: %d, Endpoint: %d\n")
    ILOG_ENTRY(FLUSH_OUT_QUEUE, "Flushing Output Queue for device at LA: %d, Endpoint: %d\n")
    ILOG_ENTRY(XSST_MONITOR_STILL_PROCESSING, "XSST Monitor is still processing the last address\n")
    ILOG_ENTRY(TOPOLOGY_WRITING_IN_ENDPOINT, "Writing IN endpoint: usbA=%d, EP=%d, EPType=%d\n")
    ILOG_ENTRY(TOPOLOGY_WRITING_OUT_ENDPOINT, "Writing OUT endpoint: usbA=%d, EP=%d, EPType=%d\n")
    ILOG_ENTRY(TOPOLOGY_SHOW_TOPOLOGY_7,    "  VID is 0x%04x, PID is 0x%04x, device Change is %d\n")
    ILOG_ENTRY(TOPOLOGY_SHOW_TOPOLOGY_4,    "  Is Hub=%d, Is Connected=%d\n")
    ILOG_ENTRY(TOPOLOGY_TOO_MANY_CONFIGURATIONS, "Device with LA %d has %d configurations but only %d are supported.\n")
    ILOG_ENTRY(TOPOLOGY_OPTIMIZING_ENDPOINT_SET, "Removing tracking of LA %d, EP number %d, EP direction %d because the type does not vary.\n")
    ILOG_ENTRY(TOPOLOGY_SET_CONFIG_UNKNOWN, "Host performed set configuration, but descparser hasn't seen the full config descriptor. \n")
    ILOG_ENTRY(TOPOLOGY_INVALID_ADDRESS_REQUIRES_CLEANUP, "Invalid address %d requires cleanup @ line %d\n")
    ILOG_ENTRY(TOPOLOGY_INVALID_DIRECTION_ARG, "Invalid transfer direction received %d @ line %d\n")
    ILOG_ENTRY(TOPOLOGY_LA_REQUIRES_CLEANUP, "Device at LA %d requires cleanup\n")
    ILOG_ENTRY(TOPOLOGY_DEVICE_REQUIRING_CLEANUP_INSYS_CLEARED, "Device at LA %d requiring cleanup was removed from sys. Timeout counter = %d\n")
    ILOG_ENTRY(TOPOLOGY_SENT_CSPLIT_TO_REX, "Device requiring cleanup had CSPLIT sent to REX. LA = %d, ep = %d, direction = %d\n")
    ILOG_ENTRY(TOPOLOGY_XSST_WRITE_CONDITIONAL_FAILED, "XSST WriteConditional failed for device requiring cleanup. Addr = %d, ep = %d, (epType << 8) | epDir  = 0x%x\n")
    ILOG_ENTRY(TOPOLOGY_SPLIT_BLK_NO_SPLIT_PACKET, "i/oSplit and i/oBlk are set but there is no s/cSplit packet in queue\n")
    ILOG_ENTRY(TOPOLOGY_SPLIT_DEVICE_HAS_NO_UPSTREAM_HS_HUB, "Split device at LA %d has no upstream high-speed hub!\n")
    ILOG_ENTRY(TOPOLOGY_SET_SPEED_MISSING_NODE, "Tried to set the speed of device with parent LA %d and port %d to %d, but couldn't find it in the topology!\n")
    ILOG_ENTRY(TOPOLOGY_SET_SPEED, "Set the speed of device with LA %d to %d\n")
    ILOG_ENTRY(TOPOLOGY_SHOW_TOPOLOGY_3, "  Port On Parent=%d, Speed=%d, High Endpoint=%d\n")
    ILOG_ENTRY(TOPOLOGY_SHOW_TOPOLOGY_6, "  MSA LA=%d, Configuration Value=%d\n")
    ILOG_ENTRY(TOPOLOGY_SENT_MSA_FREED_TO_REX, "Sent CPU message LEX_FREED_MSA_PAIR @ USBAddr %d to Rex on Vport %d\n")
    ILOG_ENTRY(TOPOLOGY_UPDATING_XSST_MSA, "Updating XSST MSA info for USBAddr %d endpoint %d line %d\n")
ILOG_END(TOPOLOGY_COMPONENT, ILOG_MINOR_EVENT)


#endif // TOPOLOGY_LOG_H

