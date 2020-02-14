//#################################################################################################
// Icron Technology Corporation - Copyright 2015
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################

//#################################################################################################
// Module Description
//#################################################################################################
// The I2C controller exists to manage the dynamic selection of which ports to enable on a TI
// PCA9548 I2C switch.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// The I2C controller has a FIFO on top of the FIFO which already exists in i2c_raw.c.  The reason
// for this is that the I2C controller holds pending I2C transactions while it sets up the
// switch for the transactions.
//
// TODO: Maybe we should consider a compile-time option for i2c_raw.c that will be used when the
// I2C controller is also in use.  This switch would remove the FIFO support from i2c_raw.c or at
// least reduce the FIFO size to 1.
//#################################################################################################


// Includes #######################################################################################
// #include <i2c_raw.h>
// #include <i2c_access.h>
// #include <ipool.h>
// #include <ififo.h>
// #include <i2cd_switch.h>
// #include <i2c_controller.h>
// #include <ilog.h>
// #include "i2c_log.h"

// Constants and Macros ###########################################################################
// #define I2C_CONTROLLER_OPERATION_FIFO_SIZE (5)


// Data Types #####################################################################################

// typedef uint8_t I2CDeviceListNodeIndex;

// // NOTE: We use a union to force the size of the struct to a power of 2 so that memory pool index
// // to pointer conversion is efficient.
// union I2CDeviceListNode
// {
//     struct I2CDeviceLN
//     {
//         uint8_t i2cAddress;
//         uint8_t portVector;
//         I2CDeviceListNodeIndex next;
//     } deviceNode;
//     uint32_t raw;
// };

// // Used for storing which type of operation is pending upon successful setup of the I2C bus.
// enum OperationType
// {
//     OT_WRITE,
//     OT_READ,
//     OT_WRITE_READ,
//     OT_WRITE_READ_BLOCK,
//     OT_WAKE
// };


// Global Variables ###############################################################################


// Static Variables ###############################################################################

// static union I2CDeviceListNode* head;

// static struct sI2CHandle handlesForTISwitch[] =
// {
//     { .rtlMuxPort = 0, .u = { .muxPort0 = { .switchPortNum = 0} } },
//     { .rtlMuxPort = 0, .u = { .muxPort0 = { .switchPortNum = 1} } },
//     { .rtlMuxPort = 0, .u = { .muxPort0 = { .switchPortNum = 2} } },
//     { .rtlMuxPort = 0, .u = { .muxPort0 = { .switchPortNum = 3} } },
//     { .rtlMuxPort = 0, .u = { .muxPort0 = { .switchPortNum = 4} } },
//     { .rtlMuxPort = 0, .u = { .muxPort0 = { .switchPortNum = 5} } },
//     { .rtlMuxPort = 0, .u = { .muxPort0 = { .switchPortNum = 6} } },
//     { .rtlMuxPort = 0, .u = { .muxPort0 = { .switchPortNum = 7} } },
// };
// static struct sI2CHandle rtlMuxPort1Handle = { .rtlMuxPort = 1 };
// static struct sI2CHandle rtlMuxPort2Handle = { .rtlMuxPort = 2 };

// static const struct I2cInterface controllerInterface =
// {
//     .registerAddress = &I2C_switchCtrlRegisterNewI2CAddress,
//     .writeAsync = &I2C_switchCtrlWriteAsync,
//     .readAsync = &I2C_switchCtrlReadAsync,
//     .writeReadAsync = &I2C_switchCtrlWriteReadAsync,
//     .writeReadBlockAsync = &I2C_switchCtrlWriteReadBlockAsync,
//     .wake = &I2C_switchCtrlWake
// };

// struct ControllerOperation
// {
//     enum OperationType operation;

//     struct
//     {
//         // I2C handle for accessing the i2c driver via i2c interface
//         I2CHandle handle;
//     } common;
//     union
//     {
//         struct
//         {
//             // I2C device address
//             uint8_t device;
//             // Speed
//             enum I2cSpeed speed;
//             union
//             {
//                 struct
//                 {
//                     // Completion callback
//                     void (*notifyWriteCompleteHandler)(bool success);
//                     // Read data buffer
//                     uint8_t* data;
//                     // Write data byte count
//                     uint8_t byteCount;
//                 } write;

//                 struct
//                 {
//                     // Completion callback
//                     void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount);
//                     // Read data buffer
//                     uint8_t* data;
//                     // Read data byte count
//                     uint8_t byteCount;
//                 } read;

//                 struct
//                 {
//                     // Completion callback
//                     void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount);
//                     // Write/read data buffer
//                     uint8_t* data;
//                     // Write data byte count
//                     uint8_t writeByteCount;
//                     // Read data byte count
//                     uint8_t readByteCount;
//                 } writeRead;

//                 struct
//                 {
//                     // Completion callback
//                     void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount);
//                     // Write/read data buffer
//                     uint8_t* data;
//                     // Write data byte count
//                     uint8_t writeByteCount;
//                     // Length of the data buffer
//                     uint8_t dataBufferLength;
//                 } writeReadBlock;
//             } u;
//         } deviceOp;
//         struct
//         {
//             void (*notifyWakeCompleteHandler)(void);
//         } wake;
//     } u;
// };

// static void (*_setRtlMuxPort)(uint8_t rtlMuxPort);

// // Static Function Declarations ###################################################################
// IFIFO_CREATE_FIFO_LOCK_UNSAFE(
//     i2c_operations, struct ControllerOperation, I2C_CONTROLLER_OPERATION_FIFO_SIZE)
// IPOOL_CREATE(I2C_switchCtrl, union I2CDeviceListNode, MAX_UNIQUE_I2C_ADDRESSES_SUPPORTED);
// static void submitOperation(struct ControllerOperation* op);
// static void startOperation(void);
// static union I2CDeviceListNode* findNodeForAddress(uint8_t i2cAddr);
// static void enableI2cAccess(const I2CHandle handle);
// static void setupSwitchForAccess(const I2CHandle handle, uint8_t i2cAddr);
// static void setupI2cSwitchComplete(void);


// // Exported Function Definitions ##################################################################

// //#################################################################################################
// // Initialize I2C switch controller.
// //
// // Parameters:
// //      a0Set               -
// //      a1Set               -
// //      a2Set               -
// //      initDone            - call back for post initialization
// // Return:
// // Assumptions:
// //      * This function is expected to be called exactly once during system initialization.
// //#################################################################################################
// void I2C_switchCtrlInit(
//     bool a0Set, bool a1Set, bool a2Set)
// {
//     I2C_switchCtrl_poolInit();
//     head = NULL;
// #ifdef PLATFORM_K7
//     const uint8_t noPortsEnabled = 0x00;
//     I2CD_pC9548Init(
//         NULL, I2cGetInterface(), a0Set, a1Set, a2Set, noPortsEnabled);
// #endif
// }

// //#################################################################################################
// // Set's local function pointer for setting rtlMuxPort
// //
// // Parameters:
// //              setRtlMuxPort - function pointer to set RTL I2C mux port
// // Return:
// //              none
// // Assumptions:
// //      * This function is expected to be called exactly once during system initialization.
// //#################################################################################################
// void I2C_controllerInit(void (*setRtlMuxPort)(uint8_t rtlMuxPort))
// {
//     _setRtlMuxPort = setRtlMuxPort;
//     const bool a0Set = false;
//     const bool a1Set = false;
//     const bool a2Set = true;
//     I2C_switchCtrlInit(a0Set, a1Set, a2Set);
// }


// //#################################################################################################
// // Retrieve interface
// //
// // Parameters:
// // Return:
// //      I2cInterface pointer
// // Assumptions:
// //#################################################################################################
// const struct I2cInterface* I2C_switchCtrlGetInterface(void)
// {
//     return &controllerInterface;
// }

// //#################################################################################################
// // Get handler for passed port
// //
// // Parameters:
// //      port                - i2c switch port
// // Return:
// //      I2CHandle for requested port
// // Assumptions:
// //#################################################################################################
// I2CHandle I2C_switchCtrlGetHandleForPortOnTISwitch(uint8_t switchPort)
// {
//     iassert_I2C_COMPONENT_1(switchPort <= 7, UNSUPPORTED_PORT, switchPort);
//     return &(handlesForTISwitch[switchPort]);
// }


// //#################################################################################################
// // Get handler for passed mux port
// //
// // Parameters:
// //              muxPort - rtl mux port
// // Return:
// //              I2CHandle for requested port
// // Assumptions:
// //#################################################################################################
// I2CHandle I2C_switchCtrlGetHandleForRTLMuxPort(uint8_t muxPort)
// {
//     if (muxPort == 1)
//     {
//         return &(rtlMuxPort1Handle);
//     }
//     else if (muxPort == 2)
//     {
//         return &(rtlMuxPort2Handle);
//     }
//     else
//     {
//         ifail_I2C_COMPONENT_2(CTRLLR_INVALID_MUX_PORT, muxPort, __LINE__);
//     }
//     // to satisfy compiler
//     return NULL;
// }

// //#################################################################################################
// // Update the I2CDeviceListNode by registering device's i2c address and the port it is attached to.
// //
// // Parameters:
// //      handle              - I2CHandle for device
// //      i2cAddr             - device's i2c address
// // Return:
// // Assumptions:
// //#################################################################################################
// void I2C_switchCtrlRegisterNewI2CAddress(const I2CHandle handle, uint8_t i2cAddr)
// {
//     union I2CDeviceListNode* node = findNodeForAddress(i2cAddr);
//     if (node == NULL)
//     {
//         uint8_t numFreeElements = I2C_switchCtrl_poolGetNumOfFreeElements();
//         iassert_I2C_COMPONENT_1(
//             numFreeElements != 0,
//             I2C_POOL_IS_FULL,
//             MAX_UNIQUE_I2C_ADDRESSES_SUPPORTED);
//         node = I2C_switchCtrl_poolAlloc();
//         node->deviceNode.i2cAddress = i2cAddr;
//         node->deviceNode.portVector = 0;

//         node->deviceNode.next = I2C_switchCtrl_poolPtrToIndex(head);
//         head = node;
//     }

//     if (handle->rtlMuxPort == 0)
//     {
//         iassert_I2C_COMPONENT_2(
//             ((node->deviceNode.portVector >> handle->u.muxPort0.switchPortNum) & 0x1) == 0,
//             ADDRESS_PORT_ALREADY_EXIST,
//             i2cAddr,
//             (handle->u.muxPort0.switchPortNum));
//         node->deviceNode.portVector |= (1 << handle->u.muxPort0.switchPortNum);
//     }
//     else if (handle->rtlMuxPort == 1)
//     {
//         node->deviceNode.portVector |= (1 << 8);
//     }
//     else if (handle->rtlMuxPort == 2)
//     {
//         node->deviceNode.portVector |= (1 << 9);
//     }
//     else
//     {
//     }
// }

// //#################################################################################################
// // Implementation of writeAsync funtion in i2cSwitchController
// //
// // Parameters:
// //      handle              - I2CHandle for device
// //      device              - device's i2c address
// //      speed               - clock speed for device
// //      data                - pointer to data to be transferred
// //      byteCount           - number of bytes to write
// //      notifyWriteCompleteHandler - call back when write completes
// // Return:
// // Assumptions:
// //#################################################################################################
// void I2C_switchCtrlWriteAsync(
//     const I2CHandle handle,
//     uint8_t device,
//     enum I2cSpeed speed,
//     uint8_t* data,
//     uint8_t byteCount,
//     void (*notifyWriteCompleteHandler)(bool success))
// {
//     struct ControllerOperation op;
//     op.operation = OT_WRITE;
//     op.common.handle = handle;
//     op.u.deviceOp.device = device;
//     op.u.deviceOp.speed = speed;
//     op.u.deviceOp.u.write.data = data;
//     op.u.deviceOp.u.write.byteCount = byteCount;
//     op.u.deviceOp.u.write.notifyWriteCompleteHandler = notifyWriteCompleteHandler;

//     submitOperation(&op);
// }

// //#################################################################################################
// // Implementation of readAsync funtion in i2cSwitchController
// //
// // Parameters:
// //      handle              - I2CHandle for device
// //      device              - device's i2c address
// //      speed               - clock speed for device
// //      data                - pointer to data to be transferred
// //      byteCount           - number of bytes to read
// //      notifyReadCompleteHandler - call back when read completes
// // Return:
// // Assumptions:
// //#################################################################################################
// void I2C_switchCtrlReadAsync(
//     const I2CHandle handle,
//     uint8_t device,
//     enum I2cSpeed speed,
//     uint8_t* data,
//     uint8_t byteCount,
//     void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount))
// {
//     struct ControllerOperation op;
//     op.operation = OT_READ;
//     op.common.handle = handle;
//     op.u.deviceOp.device = device;
//     op.u.deviceOp.speed = speed;
//     op.u.deviceOp.u.read.data = data;
//     op.u.deviceOp.u.read.byteCount = byteCount;
//     op.u.deviceOp.u.read.notifyReadCompleteHandler = notifyReadCompleteHandler;

//     submitOperation(&op);
// }

// //#################################################################################################
// // Implementation of writeReadAsync funtion in i2cSwitchController.
// //
// // Parameters:
// //      handle              - I2CHandle for device
// //      device              - device's i2c address
// //      speed               - clock speed for device
// //      data                - pointer to data to be transferred
// //      writeByteCount      - number of bytes to write
// //      readByteCount       - number of bytes to read
// //      notifyReadCompleteHandler - call back when read completes
// // Return:
// // Assumptions:
// //#################################################################################################
// void I2C_switchCtrlWriteReadAsync(
//     const I2CHandle handle,
//     uint8_t device,
//     enum I2cSpeed speed,
//     uint8_t* data,
//     uint8_t writeByteCount,
//     uint8_t readByteCount,
//     void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount))
// {
//     struct ControllerOperation op;
//     op.operation = OT_WRITE_READ;
//     op.common.handle = handle;
//     op.u.deviceOp.device = device;
//     op.u.deviceOp.speed = speed;
//     op.u.deviceOp.u.writeRead.data = data;
//     op.u.deviceOp.u.writeRead.writeByteCount = writeByteCount;
//     op.u.deviceOp.u.writeRead.readByteCount = readByteCount;
//     op.u.deviceOp.u.writeRead.notifyReadCompleteHandler = notifyReadCompleteHandler;

//     submitOperation(&op);
// }

// //#################################################################################################
// // Implementation of writeReadBlockAsync funtion in i2cSwitchController.
// //
// // Parameters:
// //      handle              - I2CHandle for device
// //      device              - device's i2c address
// //      speed               - clock speed for device
// //      data                - pointer to data to be transferred
// //      writeByteCount      - number of bytes to write
// //      dataBufferLength    - block size of data to read
// //      notifyReadCompleteHandler - call back when read completes
// // Return:
// // Assumptions:
// //#################################################################################################
// void I2C_switchCtrlWriteReadBlockAsync(
//     const I2CHandle handle,
//     uint8_t device,
//     enum I2cSpeed speed,
//     uint8_t* data,
//     uint8_t writeByteCount,
//     uint8_t dataBufferLength,
//     void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount))
// {
//     struct ControllerOperation op;
//     op.operation = OT_WRITE_READ_BLOCK;
//     op.common.handle = handle;
//     op.u.deviceOp.device = device;
//     op.u.deviceOp.speed = speed;
//     op.u.deviceOp.u.writeReadBlock.data = data;
//     op.u.deviceOp.u.writeReadBlock.writeByteCount = writeByteCount;
//     op.u.deviceOp.u.writeReadBlock.dataBufferLength = dataBufferLength;
//     op.u.deviceOp.u.writeReadBlock.notifyReadCompleteHandler = notifyReadCompleteHandler;

//     submitOperation(&op);
// }

// //#################################################################################################
// // Implementation of wake funtion in i2cSwitchController
// //
// // Parameters:
// //      handle              - I2CHandle for device
// //      notifyWakeCompleteHandler - call back when read completes
// // Return:
// // Assumptions:
// //#################################################################################################
// void I2C_switchCtrlWake(const I2CHandle handle, void (*notifyWakeCompleteHandler)(void))
// {
//     struct ControllerOperation op;
//     op.operation = OT_WAKE;
//     op.common.handle = handle;
//     op.u.wake.notifyWakeCompleteHandler = notifyWakeCompleteHandler;

//     submitOperation(&op);
// }


// // Component Scope Function Definitions ###########################################################


// // Static Function Definitions ####################################################################

// static void submitOperation(struct ControllerOperation* op)
// {
//     iassert_I2C_COMPONENT_1(
//         !i2c_operations_fifoFull(), CTRLLR_I2C_CONTROLLER_FIFO_OVERFLOW, __LINE__);
//     // if handle <= 7 rtlMux = 0
//     // if handle == 8 rtlMux = 1
//     // if handle == 9 rtlMux = 2
//     i2c_operations_fifoWrite(*op);
//     if (i2c_operations_fifoSpaceUsed() == 1)
//     {
//         // The new operation is the only operation, so start it
// #ifdef PLATFORM_K7
//         if (op->common.handle->rtlMuxPort == 0)
//         {
//             _setRtlMuxPort(0);
//         }
//         else if (op->common.handle->rtlMuxPort == 1)
//         {
//             _setRtlMuxPort(1); // DP130
//         }
//         else if (op->common.handle->rtlMuxPort == 2)
//         {
//             _setRtlMuxPort(2); // DP159
//         }
//         else
//         {
//             ifail_I2C_COMPONENT_2(CTRLLR_INVALID_MUX_PORT, op->common.handle->rtlMuxPort, __LINE__);
//         }
// #endif
//         startOperation();
//     }
// }


// static void startOperation(void)
// {
//     iassert_I2C_COMPONENT_1(
//         !i2c_operations_fifoEmpty(), CTRLLR_I2C_CONTROLLER_FIFO_UNDERFLOW, __LINE__);

//     struct ControllerOperation* op = i2c_operations_fifoPeekReadPtr();
//     if (op->operation == OT_WAKE)
//     {
//         enableI2cAccess(op->common.handle);
//     }
//     else
//     {
//         setupSwitchForAccess(op->common.handle, op->u.deviceOp.device);
//     }
// }

// //#################################################################################################
// // Search for the I2CDeviceListNode with the given I2C address.
// //
// // Parameters:
// //      i2cAddr             - The I2C address to search for
// // Return:
// //      The I2CDeviceListNode for the given address or NULL if one does not exist.
// // Assumptions:
// //#################################################################################################
// static union I2CDeviceListNode* findNodeForAddress(uint8_t i2cAddr)
// {
//     union I2CDeviceListNode* n = head;
//     while (n != NULL)
//     {
//         if (n->deviceNode.i2cAddress == i2cAddr)
//         {
//             break;
//         }
//         n = I2C_switchCtrl_poolIndexToPtr(n->deviceNode.next);
//     }
//     return n;
// }

// //#################################################################################################
// // Enables I2C address to all devices associated with the given handle.  The implication of this is
// // that we must disable any port not associated with this handle, that has a downstream I2C device
// // with an address that is present on the bus implied by the handle.
// //
// // Parameters:
// //      handle              - Handle to the I2C bus to enable access to.
// // Return:
// // Assumptions:
// //#################################################################################################
// static void enableI2cAccess(const I2CHandle handle)
// {
//     union I2CDeviceListNode* n = head;
//     uint8_t portsToDisable = 0;
//     if (handle->rtlMuxPort == 0)
//     {
//         const uint8_t portToEnableBit = (1 << handle->u.muxPort0.switchPortNum);
//         while (n != NULL)
//         {
//             if (n->deviceNode.portVector & portToEnableBit)
//             {
//                 // n->i2cAddress is used on the port we wish to enable, so we need to disable all other
//                 // ports which also use this I2C address.
//                 portsToDisable |= (n->deviceNode.portVector & (~portToEnableBit));
//             }
//             n = I2C_switchCtrl_poolIndexToPtr(n->deviceNode.next);
//         }
//         I2C_pC9548SelectPorts(
//             portToEnableBit, (portToEnableBit | portsToDisable), &setupI2cSwitchComplete);
//     }
//     else
//     {
//         setupI2cSwitchComplete();
//     }
// }

// //#################################################################################################
// // Setup the I2C switch for access to the device specified by the given handle and I2C device
// // address.
// //
// // Parameters:
// //      handle              - The handle for the device to be accessed.
// //      i2cAddr             - The I2C address of the device to be accessed.
// // Return:
// // Assumptions:
// //      * The target device has been registered with the controller.
// //#################################################################################################
// static void setupSwitchForAccess(const I2CHandle handle, uint8_t i2cAddr)
// {
//     ilog_I2C_COMPONENT_2(
//         ILOG_DEBUG,
//         SETUP_SWITCH_FOR_ACCESS,
//         i2cAddr,
//         handle->u.muxPort0.switchPortNum);

//     if (handle->rtlMuxPort == 0)
//     {
//         union I2CDeviceListNode* nodeForAddr = findNodeForAddress(i2cAddr);

//         const bool addrRegisteredOnPort =
//             (nodeForAddr != NULL)
//          && ((nodeForAddr->deviceNode.portVector & (1 << handle->u.muxPort0.switchPortNum)) != 0);

//         iassert_I2C_COMPONENT_2(
//             addrRegisteredOnPort,
//             ADDRESS_NOT_EXISTS_FOR_HANDLE,
//             i2cAddr,
//             (handle->u.muxPort0.switchPortNum));

//         const uint8_t newVal = ~(nodeForAddr->deviceNode.portVector)
//                                 | (1 << handle->u.muxPort0.switchPortNum);

//         I2C_pC9548SelectPorts(
//             newVal,
//             nodeForAddr->deviceNode.portVector,
//             &setupI2cSwitchComplete);
//     }
//     else
//     {
//         setupI2cSwitchComplete();
//     }
// }

// //#################################################################################################
// // Callback that is called upon successful setup of the I2C switch for the operation which is at
// // the head of the FIFO.  This function will submit the I2C operation which was initially sent to
// // the controller for execution.
// //
// // Parameters:
// // Return:
// // Assumptions:
// //#################################################################################################
// static void setupI2cSwitchComplete(void)
// {
//     iassert_I2C_COMPONENT_1(
//         !i2c_operations_fifoEmpty(), CTRLLR_I2C_CONTROLLER_FIFO_UNDERFLOW, __LINE__);

//     // Now that we are submitting the transaction to the native I2C layer, we can remove the
//     // operation from the I2C controller FIFO.
//     struct ControllerOperation op = *(i2c_operations_fifoReadPtr());
//     switch (op.operation)
//     {
//         case OT_WRITE:
//             I2C_WriteAsync(
//                 op.common.handle,
//                 op.u.deviceOp.device,
//                 op.u.deviceOp.speed,
//                 op.u.deviceOp.u.write.data,
//                 op.u.deviceOp.u.write.byteCount,
//                 op.u.deviceOp.u.write.notifyWriteCompleteHandler);
//             break;

//         case OT_READ:
//             I2C_ReadAsync(
//                 op.common.handle,
//                 op.u.deviceOp.device,
//                 op.u.deviceOp.speed,
//                 op.u.deviceOp.u.read.data,
//                 op.u.deviceOp.u.read.byteCount,
//                 op.u.deviceOp.u.read.notifyReadCompleteHandler);
//             break;

//         case OT_WRITE_READ:
//             I2C_WriteReadAsync(
//                 op.common.handle,
//                 op.u.deviceOp.device,
//                 op.u.deviceOp.speed,
//                 op.u.deviceOp.u.writeRead.data,
//                 op.u.deviceOp.u.writeRead.writeByteCount,
//                 op.u.deviceOp.u.writeRead.readByteCount,
//                 op.u.deviceOp.u.writeRead.notifyReadCompleteHandler);
//             break;

//         case OT_WRITE_READ_BLOCK:
//             I2C_WriteReadBlockAsync(
//                 op.common.handle,
//                 op.u.deviceOp.device,
//                 op.u.deviceOp.speed,
//                 op.u.deviceOp.u.writeReadBlock.data,
//                 op.u.deviceOp.u.writeReadBlock.writeByteCount,
//                 op.u.deviceOp.u.writeReadBlock.dataBufferLength,
//                 op.u.deviceOp.u.writeReadBlock.notifyReadCompleteHandler);
//             break;

//         case OT_WAKE:
//             I2C_Wake(
//                 op.common.handle,
//                 op.u.wake.notifyWakeCompleteHandler);
//             break;

//         default:
//             ifail_I2C_COMPONENT_0(CTRLLR_UNHANDLED_SWITCH_CASE);
//             break;
//     }

//     // This operation is now completed, so start a new operation if there is one in the FIFO
//     if (!i2c_operations_fifoEmpty())
//     {
//         startOperation();
//     }
// }

