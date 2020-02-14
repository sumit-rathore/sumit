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
// HAL layer implementation of the CPU to CPU communications infrastructure.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################


// Includes #######################################################################################
#include "cpu_comm_log.h"
#include <bb_core_regs.h>
#include <leon_timers.h>
#include <module_addresses_regs.h>
#include "cpu_comm_loc.h"

#include <uart.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static volatile bb_core_s* bb_core_registers;


// Static Function Declarations ###################################################################

// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Initialize the CPU_COMM RTL.
//
// Parameters:
//      bb_core_registerAddress - The base address of the bb_core hardware block.
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void CpuComm_HalInit(void)
{
    bb_core_registers = (volatile bb_core_s*) bb_chip_bb_core_s_ADDRESS;
}

//#################################################################################################
// Reads a complete message out of the hardware and writes it into dataBuffer.  If the data read
// out of hardware is larger than dataBufferLength, then the first dataBufferLength bytes of
// dataBuffer will be valid and the remainder of the message is lost.
//
// Parameters:
//      dataBuffer          - The buffer to read the message data into.
//      dataBufferLength    - The length of dataBuffer in bytes.
// Return:
//      The size of the message that was read.  Note that this may be larger than dataBufferLength.
//      0 if no message read
// Assumptions:
//#################################################################################################
uint32_t CpuComm_HalRead(CpuCommMessage *rxPacket)
{
    volatile bb_core_cpu_comm_rx s = {.dw = 0};
    LEON_TimerValueT timeStart = LEON_TimerRead();
    uint32_t index = 0;
    uint8_t *dataPtr = (uint8_t *)rxPacket;

    s.dw = bb_core_registers->cpu_comm_rx.dw;           // read the data

    if (s.bf.sop == 1)
    {
        *dataPtr++ = s.bf.data;
        index++;

        while (s.bf.eop == 0)
        {
            s.dw = bb_core_registers->cpu_comm_rx.dw;   // read the data

            if (index < CPU_COMM_MAX_PACKET_SIZE)
            {
                *dataPtr++ = s.bf.data;
                index++;
            }
            else if (LEON_TimerCalcUsecDiff(timeStart, LEON_TimerRead()) > 500)
            {
//                ifail_CPU_COMM_COMPONENT_1(CPU_COMM_RECEIVED_MSG_TIMEOUT, index);
                ilog_CPU_COMM_COMPONENT_1(ILOG_MAJOR_ERROR, CPU_COMM_RECEIVED_MSG_TIMEOUT, index);
                index = 0;
                break;  // exit out with an error
            }
            else
            {
                index++;
                if (index == 0)
                {
                    // hold at max
                    index--;
                }
            }
        }

        if (index > CPU_COMM_MAX_PACKET_SIZE)
        {
            // log the error, and set the index to 0 so we catch the error in the upper layer
            ilog_CPU_COMM_COMPONENT_1(ILOG_MAJOR_ERROR, CPU_COMM_RECEIVED_OVERSIZED_MSG, index);
            index = 0;
        }

    }

    return index;   // note this can be zero if no packet read, or an error occurred
}


//#################################################################################################
// Writes message to CPU comm transmit buffer.
//
// Parameters:
//      data                - data buffer
//      dataLength          - length of the data buffer in bytes
// Return:
// Assumptions:
//#################################################################################################
void CpuComm_HalWrite(CpuCommMessage *txPacket)
{
    uint16_t size = sizeof(txPacket->header) + txPacket->header.size -2;    // -2: remove start and last packet
    uint8_t *dataPtr = (uint8_t *)txPacket;

//    iassert_CPU_COMM_COMPONENT_2(size <= CPU_COMM_MAX_SW_BUFFER_SIZE,
//        CPU_COMM_TX_SIZE_INVALID,
//            txPacket->header.messageType,
//            txPacket->header.subType,
//            txPacket->header.size);

    bb_core_cpu_comm_tx packet = {.bf.sop = 1, .bf.eop = 0, .bf.data = *dataPtr++};
    bb_core_registers->cpu_comm_tx.dw = packet.dw;                          // Send the first packet with setting sop

    packet.bf.sop = 0;                                                      // Send middle packets with cleareing sop

    while (size > 0)
    {
        packet.bf.data = *dataPtr++;
        bb_core_registers->cpu_comm_tx.dw = packet.dw;
        size--;
    }

    packet.bf.eop = 1;                                                      // Send the last packet with setting eop
    packet.bf.data = *dataPtr;
    bb_core_registers->cpu_comm_tx.dw = packet.dw;

    ilog_CPU_COMM_COMPONENT_3(ILOG_DEBUG, CPU_COMM_SENT_MSG,
        txPacket->header.sequenceNumber,
        (txPacket->header.messageType << 8) | (txPacket->header.subType),
        txPacket->header.size);
}


//#################################################################################################
// Clears the receive message interrupt.  The interrupt will not fire again unless the interrupt is
// cleared each time tha handler is called.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void CpuComm_HalClearRxIrq(void)
{
    bb_core_registers->irq.s.pending.bf.mca_rx_cpu_srdy = 1;
}


// Static Function Definitions ####################################################################

