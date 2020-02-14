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
// Implementations of functions common to the Lex and Rex AUX subsystems.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// TODO
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <bb_chip_a7_regs.h>
#include <leon_timers.h>
#include <dp_aux_hpd_regs.h>  // Last because it requires uint32_t to be defined
#include <module_addresses_regs.h>

#include <dp_aux.h>
#include "aux_loc.h"
#include "aux_log.h"
#ifdef PLUG_TEST
#include <aux_api.h>
#endif // PLUG_TEST
#include <uart.h>
// Constants and Macros ###########################################################################
#define AUX_SINK_IRQ_ENABLE \
    (                                                                          \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_RX_FIFO_PKT_RECEIVED_MASK          |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_GO_BIT_CLEAR_MASK                  |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DEFER_SENT_MASK                    |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_REPLY_TIMEOUT_MASK                 |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_RX_FIFO_OVERFLOW_MASK              |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_HPD_REPLUG_MASK                    |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_HPD_IRQ                            |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_HPD_CONNECT_MASK                   |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_HPD_DISCONNECT_MASK                |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_DETECTED_MASK            |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_POWERED_MASK             |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_CONNECT_DET_MASK         |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_DISCONNECT_DET_MASK         \
    )

#define AUX_SINK_IRQ_DISABLE \
    (                                                                          \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_GO_BIT_CLEAR_MASK                  |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DEFER_SENT_MASK                    |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_REPLY_TIMEOUT_MASK                 |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_HPD_REPLUG_MASK                    |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_HPD_IRQ                            |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_DP_SOURCE_POWERED_MASK             |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_HPD_RISE_DET_MASK                  |  \
      BB_CHIP_DP_SINK_AUX_HPD_IRQ_ENABLE_HPD_FALL_DET_MASK                     \
    )

#define AUX_SOURCE_IRQ_ENABLE \
    (                                                                          \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_RX_FIFO_PKT_RECEIVED_MASK          |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_GO_BIT_CLEAR_MASK                  |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_DEFER_SENT_MASK                    |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_REPLY_TIMEOUT_MASK                 |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_RX_FIFO_OVERFLOW_MASK              |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_HPD_REPLUG_MASK                    |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_HPD_IRQ                            |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_HPD_CONNECT_MASK                   |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_HPD_DISCONNECT_MASK                |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_DP_SOURCE_DETECTED_MASK            |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_DP_SOURCE_POWERED_MASK             |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_HPD_RISE_DET_MASK                  |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_HPD_FALL_DET_MASK                     \
    )

#define AUX_SOURCE_IRQ_DISABLE \
    (                                                                            \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_GO_BIT_CLEAR_MASK                  |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_DEFER_SENT_MASK                    |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_RX_FIFO_OVERFLOW_MASK              |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_DP_SOURCE_DETECTED_MASK            |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_DP_SOURCE_POWERED_MASK             |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_HPD_RISE_DET_MASK                  |  \
      BB_CHIP_DP_SOURCE_AUX_HPD_IRQ_ENABLE_HPD_FALL_DET_MASK                     \
    )

#define AUX_TRANS_MAX_BUFF_SIZE       20000
#define AUX_TRANS_INTERVAL            20 //Max size of Aux transaction according to spec
// Data Types #####################################################################################


// Global Variables ###############################################################################


// Static Variables ###############################################################################
static volatile dp_aux_hpd_s *aux;
static uint32_t auxInterrupts;
// static uint8_t  loadedBytes[17];
// static uint8_t  cpyIdx = 0;
// static uint8_t  byteCount;
static bool     enableAuxTrans;

#ifdef PLUG_TEST
static struct
{
    uint16_t WriteIndex;
    bool bufferOverflow;
    uint8_t buffer[AUX_TRANS_MAX_BUFF_SIZE];

    uint16_t ReadIndex;
} AuxBufferInfo;
#endif // PLUG_TEST

// Static Function Declarations ###################################################################
#ifdef PLUG_TEST
static void AUX_AuxTransBuffClear(void);
static void AUX_AuxTransBufferWrite(const uint8_t* transData, uint8_t transDataSize);
static uint8_t* AUX_AuxTransBufferRead(void);
#endif // PLUG_TEST

// Exported Function Definitions ##################################################################
//#################################################################################################
// Initialize the AUX RTL on the Lex.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void AUX_LexHalInit(void)
{
    HPD_LexInit();

    aux = (volatile dp_aux_hpd_s *) bb_chip_dp_sink_aux_hpd_s_ADDRESS;
    auxInterrupts = AUX_SINK_IRQ_ENABLE;

    aux->aux_ctrl2.bf.pre_charge_length = 30;

    //Disabling Irq which are not needed
    auxInterrupts &= ~AUX_SINK_IRQ_DISABLE;
    aux->irq.s.enable.dw = auxInterrupts;

    AUX_CntrlSourceDisconnectCounter(true);
    AUX_CntrlSourceConnectCounter(true);

#ifdef PLUG_TEST
    AUX_AuxTransBuffClear();
#endif // PLUG_TEST
}

//#################################################################################################
// Initialize the AUX RTL on the Rex.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void AUX_RexHalInit(void)
{
    HPD_RexInit();

    aux = (volatile dp_aux_hpd_s *) bb_chip_dp_source_aux_hpd_s_ADDRESS;
    auxInterrupts = AUX_SOURCE_IRQ_ENABLE;

    aux->aux_ctrl2.bf.pre_charge_length = 30;

     //Disabling Irq which are not needed
    auxInterrupts &= ~AUX_SOURCE_IRQ_DISABLE;
    aux->irq.s.enable.dw = auxInterrupts;

#ifdef PLUG_TEST
    AUX_AuxTransBuffClear();
#endif // PLUG_TEST
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
uint32_t AUX_GetConfiguredInterrupts(void)
{
    return auxInterrupts;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_EnableAuxInterrupts(uint32_t intsToEnable)
{
    const dp_aux_hpd_irq_enable oldIrqs = { .dw = aux->irq.s.enable.dw };
    const dp_aux_hpd_irq_enable hwEn = { .dw = intsToEnable };
    const dp_aux_hpd_irq_enable newIrqs = { .dw = hwEn.dw | oldIrqs.dw };
    aux->irq.s.enable.dw = newIrqs.dw;

#ifdef PLUG_TEST
    enableAuxTrans = DP_GetEnableAuxTrafficStatus();
#else
    enableAuxTrans = false;
#endif // PLUG_TEST
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_DisableAuxInterrupts(uint32_t intsToDisable)
{
    const dp_aux_hpd_irq_enable oldIrqs = { .dw = aux->irq.s.enable.dw };
    const dp_aux_hpd_irq_enable hwDis =  { .dw = ~intsToDisable};
    const dp_aux_hpd_irq_enable newIrqs = { .dw = hwDis.dw & oldIrqs.dw };
    aux->irq.s.enable.dw = newIrqs.dw;

    AUX_AckPendingAuxInterrupts(intsToDisable);
}

//#################################################################################################
// Return Host connection status (Lex only)
//
// Parameters:
// Return:
// Assumptions:
//      0: DP source device is not connected
//      1: DP source device is connected
//#################################################################################################
bool AUX_GetHostConnectedInfo(void)
{
    return ((aux->aux_status.bf.aux_sense_p_in_debounce | aux->aux_status.bf.aux_sense_p_in) ? 0: 1);
}

//#################################################################################################
// Return Host power status (Lex only)
//
// Parameters:
// Return:
// Assumptions:
//      0: DP_PWR off
//      1: DP_PWR on
//#################################################################################################
bool AUX_GetHostPowerInfo(void)
{
    return aux->aux_status.bf.aux_sense_n_in;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//
//#################################################################################################
bool AUX_RequestIsAddressOnly(const struct AUX_Request *request)
{
    return request->header.dataLen == 0;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//
//#################################################################################################
bool AUX_DDCCIRequestIsAddressOnly(const struct AUX_Request *request)
{
    return ((request->header.dataLen == 6) || (request->header.dataLen == 0));
}

#ifdef PLUG_TEST
//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//
//#################################################################################################
void Aux_icmdReadAuxTrans(void)
{
    uint8_t * ptr = AUX_AuxTransBufferRead();

    for (uint8_t index = 0; index < AUX_TRANS_INTERVAL; index += 4)
    {
        ilog_DP_AUX_COMPONENT_2(ILOG_DEBUG_GREEN, AUX_TRANS_REQUEST_BYTE, (index/4),
                ptr[index] << 24 | ptr[index + 1] << 16 | ptr[index + 2]  << 8 | ptr[index + 3]);
    }
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//
//#################################################################################################
void Aux_icmdAuxTransGetReadAndWriteIndex(void)
{
    ilog_DP_AUX_COMPONENT_2(ILOG_DEBUG_GREEN, AUX_TRANS_READ_WRITE_INDEX, AuxBufferInfo.ReadIndex, AuxBufferInfo.WriteIndex);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//
//#################################################################################################
void Aux_icmdAuxTransSetReadIndex(uint16_t index)
{
    AuxBufferInfo.ReadIndex = index;
    ilog_DP_AUX_COMPONENT_1(ILOG_DEBUG_GREEN, AUX_TRANS_SET_READ_INDEX, AuxBufferInfo.ReadIndex);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//
//#################################################################################################
void Aux_icmdClearAuxTrans(void)
{
    AUX_AuxTransBuffClear();
}
#endif // PLUG_TEST

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// Loads a transaction from the AUX RX buffer into memory.
//
// Parameters:
//      buf                 - The buffer into which the transaction will be read.
// Return:
//      Size of the transaction (the bytes in between the SYNC and STOP patterns) in bytes.
// Assumptions:
//      * buf points to a memory object of at least AUX_MAX_REQUEST_SIZE bytes on the Lex or
//        AUX_MAX_REPLY_SIZE bytes on the Rex.
//      * The AUX RX buffer contains at least one complete transaction (TODO: assert this).
//#################################################################################################
uint8_t AUX_LoadTransaction(uint8_t *buf)
{
    uint8_t rxCount = 0;
    const bool isLex = bb_top_IsDeviceLex();
    const uint8_t rxCountMin = isLex ? AUX_MIN_REQUEST_SIZE : AUX_MIN_REPLY_SIZE;
    const uint8_t rxCountMax = isLex ? AUX_MAX_REQUEST_SIZE : AUX_MAX_REPLY_SIZE;

    while (rxCount <= rxCountMax)
    {
        const dp_aux_hpd_aux_rx rxReg = { .dw = aux->aux_rx.dw };
        buf[rxCount++] = rxReg.bf.rx_data;
        if (rxReg.bf.rx_flag)
        {
            break;
        }
    }
#ifdef PLUG_TEST
    if(enableAuxTrans)
    {
        AUX_AuxTransBufferWrite(buf, AUX_TRANS_INTERVAL);
    }
#endif // PLUG_TEST

    // Temporary Blocking broken RX case
    // TODO: Need to check if FPGA fix this issue.
    // iassert_DP_AUX_COMPONENT_1(rxCountMin <= rxCount && rxCount <= rxCountMax,
    //                             AUX_RX_INVALID_TRANSACTION_SIZE,
    //                             rxCount);
    if(rxCountMin > rxCount || rxCount > rxCountMax)
    {
        ilog_DP_AUX_COMPONENT_1(ILOG_FATAL_ERROR, AUX_RX_INVALID_TRANSACTION_SIZE, rxCount);
        rxCount = 0xFF;            // Indicating Error
    }
    return rxCount;
}

//#################################################################################################
// Writes a transaction from memory to the AUX TX buffer.
//
// Parameters:
//      buf                 - The buffer out of which the transaction will be written.
//      n                   - The number of bytes to write.
// Return:
//      The result of the transaction: either success or some kind of failure (see
//      enum AUX_TransactionStatus).
// Assumptions:
//#################################################################################################
enum AUX_TransactionStatus AUX_WriteTransaction(const uint8_t *buf, uint8_t n)
{
    if((bb_top_IsDeviceLex() && ((n < AUX_MIN_REPLY_SIZE)   || (n > AUX_MAX_REPLY_SIZE ))) ||
      (!bb_top_IsDeviceLex() && ((n < AUX_MIN_REQUEST_SIZE) || (n > AUX_MAX_REQUEST_SIZE ))))
    {
        for(uint8_t i=0; i<AUX_MAX_REQUEST_SIZE; i++)
        {
            ilog_DP_AUX_COMPONENT_2(ILOG_FATAL_ERROR, AUX_TX_INVALID_TRANSACTION_DATA, i, buf[i]);
        }

        if(bb_top_IsDeviceLex())
        {
            LEX_PrintHostRequest();
        }
        ifail_DP_AUX_COMPONENT_1(AUX_TX_INVALID_TRANSACTION_SIZE, n);
    }

    if (aux->aux_tx.bf.go_bit == 1)
    {
        ilog_DP_AUX_COMPONENT_1(ILOG_DEBUG, AUX_I2C_STATUS, aux->aux_tx.bf.go_bit);
        return AUX_TRANSACTION_TX_BUSY;
    }

#ifdef PLUG_TEST
    if(enableAuxTrans)
    {
        AUX_AuxTransBufferWrite(buf, AUX_TRANS_INTERVAL);
    }
#endif // PLUG_TEST

    for (uint8_t i = 0; i < n; i++)
    {
        // Need to set tx_flag on the last byte
        if(i == n-1)
        {
            const dp_aux_hpd_aux_tx txReg = { .bf = {
                .tx_flag = 1,
                .go_bit = 1,
                .tx_data = buf[i] } };
            aux->aux_tx.dw = txReg.dw;
        }
        else
        {
            const dp_aux_hpd_aux_tx txReg = { .bf = {
                .tx_flag = 0,
                .go_bit = 0,
                .tx_data = buf[i] } };
            aux->aux_tx.dw = txReg.dw;
        }
    }
    return AUX_TRANSACTION_SUCCESS;
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
uint32_t AUX_GetPendingAuxInterrupts(void)
{
    return (aux->irq.s.pending.dw & aux->irq.s.enable.dw);
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void AUX_AckPendingAuxInterrupts(uint32_t acks)
{
    aux->irq.s.pending.dw = acks;
}

//#################################################################################################
// Return Defer Cnt (Lex only)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
uint8_t AUX_GetDeferCnt(void)
{
    return aux->aux_status.bf.sent_defer_cnt;
}

//#################################################################################################
// Return Host connection status (Lex only)
//
// Parameters:
// Return:
// Assumptions: used preset enum AUX_ResponseTimeValues leftTime
//              to minimize calcuration
//
//#################################################################################################
bool AUX_ResponseTimeUnderUs(enum AUX_ResponseTimeValues leftTime)
{
    uint32_t responseTmr = aux->aux_status.bf.response_timeout_cnt;
    return responseTmr > ((uint32_t)(AUX_RESPONSE_MAX - leftTime));
}

//#################################################################################################
//
//
// Parameters:
// Return:
// Assumptions:
//
//
//#################################################################################################
bool AUX_RequestIsI2c(const struct AUX_Request *request)
{
    return request->header.command <= AUX_I2C_REQUEST_COMMAND_MAX;
}

//#################################################################################################
// Starts or stops the Source connect counter
// Parameters: Control state: 1 to start the counter, 0 to stop counter
// Return:
// Assumptions:
//#################################################################################################
void AUX_CntrlSourceConnectCounter(bool state)
{
    aux->dp_source_connect_det.bf.ctrl = state;
}

//#################################################################################################
// Starts or stops the Source connect counter
// Parameters: Control state: 1 to start the counter, 0 to stop counter
// Return:
// Assumptions:
//#################################################################################################
void AUX_CntrlSourceDisconnectCounter(bool state)
{
    aux->dp_source_disconnect_det.bf.ctrl = state;
}

//#################################################################################################
// This function will return the counter value of Source connect counter
// Parameters:
// Return: 8bit counter value of the the counter.
// Assumptions:
//  This is a saturated counter with max of 255. Default  value is 0.
//#################################################################################################
uint8_t AUX_SourceConnectCounter(void)
{
    return aux->dp_source_connect_det.bf.cnt;
}

//#################################################################################################
// This function will return the counter value of Source disconnect counter
// Parameters:
// Return: 8bit counter value of the the counter.
// Assumptions:
//  This is a saturated counter with max of 255. Default  value is 0.
//#################################################################################################
uint8_t AUX_SourceDisconnectCounter(void)
{
    return aux->dp_source_disconnect_det.bf.cnt;
}

//#################################################################################################
// This function will reset the Source connect & Source disconnect counters
// Parameters:
// Return:
// Assumptions:
//  Write 1 and then 0 to reset the counter
//#################################################################################################
void AUX_SourceEdgeCounterRst(void)
{
    //reset the Source connect counter
    aux->dp_source_connect_det.bf.rst = 1;
    aux->dp_source_connect_det.bf.rst = 0;
    //reset the Source disconnect counter
    aux->dp_source_disconnect_det.bf.rst = 1;
    aux->dp_source_disconnect_det.bf.rst = 0;
}

// Static Function Definitions ####################################################################
#ifdef PLUG_TEST
//#################################################################################################
// This function will clear all Aux transaction buffer
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AUX_AuxTransBuffClear(void)
{
    ilog_DP_AUX_COMPONENT_0(ILOG_DEBUG_GREEN, AUX_TRANS_BUFF_CLEAR);
    memset(&AuxBufferInfo, 0, sizeof(AuxBufferInfo));
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AUX_AuxTransBufferWrite(const uint8_t* transData, uint8_t transDataSize)
{
    if (!AuxBufferInfo.bufferOverflow)
    {
        memcpy(&AuxBufferInfo.buffer[AuxBufferInfo.WriteIndex], transData, transDataSize);
        AuxBufferInfo.WriteIndex += AUX_TRANS_INTERVAL;

        if (AuxBufferInfo.WriteIndex >= AUX_TRANS_MAX_BUFF_SIZE)
        {
            ilog_DP_AUX_COMPONENT_0(ILOG_MAJOR_ERROR, AUX_TRANS_BUFF_WRITE_OVERFLOW);
            AuxBufferInfo.bufferOverflow = true;
        }
    }
}

//#################################################################################################
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static uint8_t *AUX_AuxTransBufferRead(void)
{
    uint8_t* ptr = NULL;
    if (AuxBufferInfo.ReadIndex >= AUX_TRANS_MAX_BUFF_SIZE)
    {
        ilog_DP_AUX_COMPONENT_0(ILOG_MAJOR_ERROR, AUX_TRANS_BUFF_READ_OVERFLOW);
    }
    else
    {
        ptr = &AuxBufferInfo.buffer[AuxBufferInfo.ReadIndex];
        AuxBufferInfo.ReadIndex += AUX_TRANS_INTERVAL;
    }
    return ptr;
}
#endif // PLUG_TEST
