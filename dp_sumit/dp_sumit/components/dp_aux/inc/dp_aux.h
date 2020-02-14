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
#ifndef DP_AUX_H
#define DP_AUX_H

// Includes #######################################################################################
#include <itypes.h>

// Constants and Macros ###########################################################################
#define AUX_REQUEST_COMMAND_WIDTH  4
#define AUX_REQUEST_ADDRESS_WIDTH  20
#define AUX_REQUEST_DATA_LEN_WIDTH 8  // needs to be able to represent integers in [0, 16]
#define AUX_REPLY_COMMAND_WIDTH    4
#define AUX_REPLY_PADDING_WIDTH    4
#define AUX_MAX_DATA_BURST_SIZE    16
#define AUX_MAX_REQUEST_SIZE       20
#define AUX_MIN_REQUEST_SIZE       3
#define AUX_MAX_REPLY_SIZE         17
#define AUX_MIN_REPLY_SIZE         1

// Data Types #####################################################################################
enum AUX_RequestCommand
{
    // Note: bits match DP spec -- values matter!
    I2C_AUX_WRITE                   = 0x0,
    I2C_AUX_READ                    = 0x1,
    I2C_AUX_WRITE_STATUS_UPDATE     = 0x2,  // TODO not sure if this command is ever used.
    // 0x3 Reserved
    I2C_AUX_WRITE_MOT               = 0x4,  // Middle Of Transaction (MOT) write
    I2C_AUX_READ_MOT                = 0x5,  // Middle Of Transaction (MOT) read
    I2C_AUX_WRITE_STATUS_UPDATE_MOT = 0x6,
    AUX_I2C_REQUEST_COMMAND_MAX     = I2C_AUX_WRITE_STATUS_UPDATE_MOT,
    // 0x7 Reserved
    NATIVE_AUX_WRITE                = 0x8,
    NATIVE_AUX_READ                 = 0x9,
    AUX_REQUEST_COMMAND_UPPER_BOUND
    // 0xA - 0xF reserved
} __attribute__((packed));

enum AUX_ReplyCommand
{
    // Note: bits match DP spec -- values matter!
    NATIVE_AUX_ACK        = 0x0, // Intentionally the same as I2C_AUX_ACK
    I2C_AUX_ACK           = 0x0,
    AUX_ACK               = 0x0, // Intended to be used when referring to something that may
                                 // be either a native- or I2C-over-AUX ACK
    NATIVE_AUX_NACK       = 0x1,
    NATIVE_AUX_DEFER      = 0x2,
    // 0x3 Reserved
    I2C_AUX_NACK          = 0x4,
    // 0x5 - 0x7 Reserved
    I2C_AUX_DEFER         = 0x8,
    AUX_REPLY_COMMAND_UPPER_BOUND
    // 0x9 - 0xF Reserved
} __attribute__((packed));

enum RexAuxEventCode
{
    AUX_REX_RX,                 // we have received a reply to our request
    AUX_REX_TX,                 // transmit is done
    AUX_REX_LOCAL_REQUEST,      // we have a request we want to send to the Sink
    AUX_REX_REPLY_TIMEOUT,      // we timed out waiting for a reply
    AUX_REX_RESET_REQUEST,      // reset the AUX state machine
};

enum LexAuxEventCode
{
    AUX_LEX_RX,
    AUX_LEX_RESET_REQUEST,
};

enum AuxErrorCode
{
    AUX_REQUEST_FAIL,           // Request msg send fail over MAX_RETRIES_AUX_REX times
};

struct AUX_RequestHeader
{
    uint32_t command: AUX_REQUEST_COMMAND_WIDTH;
    uint32_t address: AUX_REQUEST_ADDRESS_WIDTH;
    uint32_t dataLen: AUX_REQUEST_DATA_LEN_WIDTH;
};

struct AUX_Request
{
    union
    {
        struct
        {
            struct AUX_RequestHeader header;
            uint8_t data[AUX_MAX_DATA_BURST_SIZE];
        };
        uint8_t bytes[AUX_MAX_REQUEST_SIZE];
    };
    uint8_t len; // size in bytes of the request as it was/would be received on the wire
} __attribute__((packed));

struct AUX_ReplyHeader
{
    uint8_t command: AUX_REPLY_COMMAND_WIDTH;
    uint8_t : AUX_REPLY_PADDING_WIDTH;
};

struct AUX_Reply
{
    union
    {
        struct
        {
            struct AUX_ReplyHeader header;
            uint8_t data[AUX_MAX_DATA_BURST_SIZE];
        };
        uint8_t bytes[AUX_MAX_REPLY_SIZE];
    };
    uint8_t len; // size in bytes of the reply as it was/would be received on the wire
} __attribute__((packed));

struct AUX_RequestAndReplyContainer
{
    const struct AUX_Request *request;
    const struct AUX_Reply *reply;
};

typedef void (*AUX_RexReplyHandler)(
    const struct AUX_Request *req, const struct AUX_Reply *reply);
typedef void (*IsrCallback)(uint32_t isrType);                          // Callback to inform ISR to Policy maker
typedef void (*AuxReqHandler)(struct AUX_Request*, struct AUX_Reply*);  // aux request handler callback
typedef void (*AuxErrHandler)(enum AuxErrorCode);                       // aux error handler callback


// Function Declarations ##########################################################################
// From aux_hal.c Lex only
void AUX_LexHalInit(void)                                                   __attribute__((section(".lexftext")));
bool AUX_GetHostConnectedInfo(void)                                         __attribute__((section(".lexftext")));
bool AUX_GetHostPowerInfo(void)                                             __attribute__((section(".lexftext")));

// From aux_hal.c Rex only
void AUX_RexHalInit(void)                                                   __attribute__((section(".rexftext")));

// From aux_hal.c Generic
uint32_t AUX_GetConfiguredInterrupts(void)                                  __attribute__((section(".ftext")));
void AUX_EnableAuxInterrupts(uint32_t intsToEnable)                         __attribute__((section(".ftext")));
void AUX_DisableAuxInterrupts(uint32_t intsToDisable)                       __attribute__((section(".ftext")));
bool AUX_RequestIsI2c(const struct AUX_Request*)                            __attribute__((section(".ftext")));
bool AUX_RequestIsAddressOnly(const struct AUX_Request *request)            __attribute__((section(".ftext")));
bool AUX_DDCCIRequestIsAddressOnly(const struct AUX_Request *request)       __attribute__((section(".ftext")));
uint8_t AUX_GetDeferCnt(void)                                               __attribute__((section(".ftext")));
void AUX_CntrlSourceConnectCounter(bool state)                              __attribute__((section(".ftext")));
void AUX_CntrlSourceDisconnectCounter(bool state)                           __attribute__((section(".ftext")));
uint8_t AUX_SourceConnectCounter(void)                                      __attribute__((section(".ftext")));
uint8_t AUX_SourceDisconnectCounter(void)                                   __attribute__((section(".ftext")));
void AUX_SourceEdgeCounterRst(void)                                         __attribute__((section(".ftext")));


// From aux_rx.c
void AUX_LexInit(AuxReqHandler auxHandler)                                  __attribute__((section(".lexftext")));
void LexProcessAuxRequest(enum LexAuxEventCode ev)                          __attribute__((section(".lexftext")));

// From aux_tx.c
void AUX_RexInit(AuxErrHandler errCallback)                                 __attribute__((section(".rexftext")));
void RexStepAuxStateMachine(enum RexAuxEventCode ev)                        __attribute__((section(".rexftext")));
void AUX_RexEnqueueLocalRequest(
    const struct AUX_Request *request, AUX_RexReplyHandler replyHandler)    __attribute__((section(".rexftext")));
void AUX_RexEnqueueI2cOverAuxRead(
    uint8_t readLen,
    uint8_t i2cAddr,
    bool endOfTransaction,
    AUX_RexReplyHandler rexReplyHandler)                                    __attribute__((section(".ftext")));
void AUX_RexEnqueueI2cOverAuxWrite(
    const uint8_t *data,
    uint8_t dataLen,
    uint8_t i2cAddr,
    bool endOfTransaction,
    AUX_RexReplyHandler rexReplyHandler)                                    __attribute__((section(".ftext")));
void AUX_RexEnqueueDDCCIOverI2CWrite(
    const uint8_t *data,
    uint8_t dataLen,
    uint8_t i2cAddr,
    bool endOfTransaction,
    AUX_RexReplyHandler rexReplyHandler)                                    __attribute__((section(".ftext")));
void AUX_RexEnqueueI2cOverAuxWriteRead(
    const uint8_t *writeData,
    uint8_t writeLen,
    uint8_t readLen,
    uint8_t i2cAddr,
    bool endOfTransaction,
    AUX_RexReplyHandler rexReplyHandler)                                    __attribute__((section(".ftext")));

// From aux_hpd.c Lex only
void HPD_Connect(void)                                                      __attribute__((section(".lexftext")));
void HPD_Disconnect(void)                                                   __attribute__((section(".lexftext")));
void HPD_SendIrq(void)                                                      __attribute__((section(".lexftext")));
void HPD_SendReplug(void)                                                   __attribute__((section(".lexftext")));

// From aux_hpd.c Rex only
bool HPD_GetLineState(void)                                                 __attribute__((section(".rexftext")));

// From aux_lexIsr.c
void AUX_LexIsrInit(IsrCallback callback)                                   __attribute__((section(".lexftext")));
void AUX_LexISR(void)                                                       __attribute__((section(".lexftext")));

// From aux_rexIsr.c
void AUX_RexIsrInit(IsrCallback callback)                                   __attribute__((section(".rexftext")));
void AUX_RexISR(void)                                                       __attribute__((section(".rexftext")));

// Extern Function Declarations ##########################################################################
extern void DP_LexDisableStreamIrq(void)                                    __attribute__((section(".lexftext")));
extern void DP_LexRestoreStreamIrq(void)                                    __attribute__((section(".lexftext")));
extern void DP_LexClearBackupIrq(void)                                      __attribute__((section(".lexftext")));
#endif // DP_AUX_H
