///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2018
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
//!   @file  -  upp_hal.c
//
//!   @brief -  Hardware abstraction layer for the UPP project:
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################

#include <ibase.h>
#include <module_addresses_regs.h>
#include <bb_top.h>
#include <interrupts.h>
#include <leon_timers.h>
#include <upp_regs.h>
#include <xusb3_regs.h>
#include <upp.h>
#include "upp_loc.h"
#include "upp_log.h"

#include <uart.h>

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################


// Static Variables ###############################################################################

static volatile upp_s   * const uppPtr     = (volatile upp_s *)   bb_chip_upp_s_ADDRESS;
static volatile xusb3_s * const xusb3Ptr   = (volatile xusb3_s *) bb_chip_xusb3_s_ADDRESS;


static struct
{
//    uint8_t deviceAddress;
//    uint8_t isoEndpointNumber;
    uint8_t upstreamWordCount;
    uint8_t downstreamWordCount;


} uppHalContext;

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize the UPP component
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_HalInit(void)
{
}

//#################################################################################################
// Turns UPP queue interrupts on or off
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_HalControlQueueInterrupts(bool enable)
{
#ifdef BLACKBIRD_ISO
    if (enable)
    {
        uppPtr->irq0.s.enable.bf.h2d_pkt = true;
        uppPtr->irq0.s.enable.bf.d2h_pkt = true;
        TOPLEVEL_setPollingMask(SECONDARY_INT_UPP_INT_MSK);
    }
    else
    {
        // turn interrupts off
        uppPtr->irq0.s.enable.dw = 0; // make sure all interrupts are off on the Lex side
        uppPtr->irq1.s.enable.dw = 0; // make sure all interrupts are off on the Lex side

        UppHalGetIrq0Interrupts();  // clear any pending interrupts
        UppHalGetIrq1Interrupts();  // clear any pending interrupts
        TOPLEVEL_clearPollingMask(SECONDARY_INT_UPP_INT_MSK);
    }
#endif
}

//#################################################################################################
// Turns UPP endpoint not responding interrupts on or off
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPP_HalControlNotRespondingInterrupts(bool enable)
{
#ifdef BLACKBIRD_ISO
    if (enable)
    {
        uppPtr->irq1.s.enable.dw = UPP_IRQ1_ENABLE_ISO_NOT_RESPOND_MASK;
        TOPLEVEL_setPollingMask(SECONDARY_INT_UPP_INT_MSK);
    }
    else
    {
        // turn interrupts off
        uppPtr->irq0.s.enable.dw = 0; // make sure all interrupts are off on the Rex side
        uppPtr->irq1.s.enable.dw = 0; // make sure all interrupts are off on the Rex side

        UppHalGetIrq0Interrupts();  // clear any pending interrupts
        UppHalGetIrq1Interrupts();  // clear any pending interrupts
        TOPLEVEL_clearPollingMask(SECONDARY_INT_UPP_INT_MSK);
    }
#endif
}

//#################################################################################################
// Get the pending UPP interrupts from irq0, and then clears them
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint32_t UppHalGetIrq0Interrupts(void)
{
#ifdef BLACKBIRD_ISO
    uint32_t pendingInterrupts = uppPtr->irq0.s.pending.dw; // get the pending interrupts
    uppPtr->irq0.s.pending.dw = pendingInterrupts;          // clear them

    return (pendingInterrupts);
#else
    return (0);
#endif
}


//#################################################################################################
// Get the pending UPP interrupts from irq1, and then clears them
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
uint32_t UppHalGetIrq1Interrupts(void)
{
#ifdef BLACKBIRD_ISO
    uint32_t pendingInterrupts = uppPtr->irq1.s.pending.dw; // get the pending interrupts
    uppPtr->irq1.s.pending.dw = pendingInterrupts;          // clear them

    return (pendingInterrupts);
#else
    return (0);
#endif
}


// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Set upp enable
//
// Parameters: enable - true to enable UPP, false to bypass UPP
// Return:
// Assumptions:
//
//#################################################################################################
void UppHalUppEnable(bool enabled)
{
#ifdef BLACKBIRD_ISO

    if (enabled)
    {
        bb_top_ApplyResetUpp(false); // take UPP out of reset, before writing to the registers

        xusb3Ptr->configuration.bf.upp_bypass = false;

        uppPtr->control.bf.ping_term_en = true;
        uppPtr->config0.bf.dp_max_time_cnt = 258; // recommendation from Mohsen, March 11, 2019
        uppPtr->id_mgr_fifo.s.write_engine.s.config0.s.mode.bf.drp_on_pkt_err = 0;

        if (bb_top_IsDeviceLex())
        {
            uppPtr->control.bf.ctrl_manager_en = UppControlTransferIsEnabled();
            UPP_HalControlQueueInterrupts(true);    // enable the UPP interrupts
        }
        else
        {
            uppPtr->control.bf.limit_max_burst = 0; // Rex will not limit, and use the endpoint max burst programmed
            uppPtr->control.bf.lex_buff_max_cnt = 38;

            uppPtr->control.bf.itp_manager_en = 1;  // turn on the ITP manager (Rex side only)
            uppPtr->control.bf.max_itp_deviation_2fix = 384; // Mohsen email June 25/2019
            uppPtr->control.bf.iso_dev_non_rspns_cnt = 3;

            UPP_HalControlNotRespondingInterrupts(true);
        }

        LEON_TimerWaitMicroSec(10);         // wait a bit for the entries to be cleared
    }
    else
    {
        // turn it off
        xusb3Ptr->configuration.bf.upp_bypass = true;       // bypass UPP

        if (bb_top_IsDeviceLex())
        {
            UPP_HalControlQueueInterrupts(false);   // disable the UPP interrupts
        }
        else
        {
            UPP_HalControlNotRespondingInterrupts(false);
        }

        bb_top_ApplyResetUpp(true); // put UPP into reset, after we have cleared the interrupts
    }
#endif
}


//#################################################################################################
// Flushes the control transfer manger and endpoint table (done for a hot or warm reset)
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UppHalUppRestart(void)
{
#ifdef BLACKBIRD_ISO
    uppPtr->control.bf.clr_ctrl_trfr_mgr = true;
    uppPtr->control.bf.init_ept          = true;

    uppPtr->control.bf.clr_ctrl_trfr_mgr = false;
    uppPtr->control.bf.init_ept          = false;

    LEON_TimerWaitMicroSec(10);         // wait a bit for the entries to be cleared
#endif
}


//#################################################################################################
// Set upp to bypass mode, but still brings it into and out of reset
//
// Parameters: enable - reset state of UPP (enable = true == bring UPP + USB3 buffers out of reset)
// Return:
// Assumptions:
//
//#################################################################################################
void UppHalUppBypassed(bool enabled)
{
    // turn UPP on/off
    bb_top_ApplyResetUpp(!enabled);
    xusb3Ptr->configuration.bf.upp_bypass = true;   // bypass UPP
    TOPLEVEL_clearPollingMask(SECONDARY_INT_UPP_INT_MSK);   // make sure UPP is not polled

    LEON_TimerWaitMicroSec(10);         // wait a bit for the traffic to be cleared from the buffers
}

//#################################################################################################
// Unblocks the endpoint; called once a device's control transfer has been processed
//
// Parameters:  deviceAddress - the device we want to unblock
// Return:
//
// Assumptions:
//
//#################################################################################################
void UppHalEndTransaction(uint8_t deviceAddress)
{
#ifdef BLACKBIRD_ISO
    uppPtr->ctrl_trfr.s.cmpltd.bf.device_addr = deviceAddress;

    ilog_UPP_COMPONENT_1(ILOG_DEBUG, UPP_PACKET_END_PROCESSING, deviceAddress);
#endif
}


//#################################################################################################
// Set upp control transfer
//
// Parameters: enable - true to enable UPP, false to turn it off
// Return:
// Assumptions:
//
//#################################################################################################
void UppHalUppCtrlEnable(bool enable)
{
#ifdef BLACKBIRD_ISO
    uppPtr->control.bf.ctrl_manager_en = enable;
#endif
}

//#################################################################################################
// Sees if a packet is waiting from the host
//
// Parameters:
// Return:      true if a packet from the host is waiting for us; false otherwise
//
// Assumptions:
//
//#################################################################################################
bool UppHalDownstreamPacketAvailable(void)
{
#ifdef BLACKBIRD_ISO
    upp_ctrl_trfr_h2d_ctrl volatile *h2dControl = (upp_ctrl_trfr_h2d_ctrl volatile *) &uppPtr->ctrl_trfr.s.h2d_ctrl;

    if (h2dControl->bf.sop && h2dControl->bf.srdy)
    {
        uppHalContext.downstreamWordCount = 0;
        return (true);
    }
#endif
    return (false);
}

//#################################################################################################
//  Sees if a packet is waiting from the device
//
// Parameters:
// Return:      true if a packet from the device is waiting for us; false otherwise
//
// Assumptions:
//
//#################################################################################################
bool UppHalUpstreamPacketAvailable(void)
{
#ifdef BLACKBIRD_ISO
    upp_ctrl_trfr_d2h_ctrl volatile *d2hControl = (upp_ctrl_trfr_d2h_ctrl volatile *) &uppPtr->ctrl_trfr.s.d2h_ctrl;

    if (d2hControl->bf.sop && d2hControl->bf.srdy)
    {
        uppHalContext.upstreamWordCount = 0;
        return (true);
    }
#endif
    return (false);
}

//#################################################################################################
// Gets the time tag of the current downstream (host) packet being read; sign extends it to 16 bits
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
int16_t UppHalGetDownstreamTimeStamp(void)
{
#ifdef BLACKBIRD_ISO
    // read the downstream control register, sign extend it to 16 bits
    upp_ctrl_trfr_h2d_ctrl volatile *h2dControl = (upp_ctrl_trfr_h2d_ctrl volatile *) &uppPtr->ctrl_trfr.s.h2d_ctrl;

    struct {signed int timeTag: 12;} tag;
    tag.timeTag = (signed int)h2dControl->bf.time_tag;

    return (tag.timeTag);
#else
    return (0);
#endif
}

//#################################################################################################
// Gets the time tag of the current upstream (device) packet being read; sign extends it to 16 bits
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
int16_t UppHalGetUpstreamTimeStamp(void)
{
#ifdef BLACKBIRD_ISO
    // read the downstream control register, sign extend it to 16 bits
    upp_ctrl_trfr_d2h_ctrl volatile *d2hControl = (upp_ctrl_trfr_d2h_ctrl volatile *) &uppPtr->ctrl_trfr.s.d2h_ctrl;

    struct {signed int timeTag: 12;} tag;
    tag.timeTag = (signed int)d2hControl->bf.time_tag;

    return (tag.timeTag);
#else
    return (0);
#endif
}

//#################################################################################################
// return true if the first timestamp is older then the second
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
bool UppHalCompareTimestamps(int16_t firstTimestamp, int16_t secondTimestamp)
{
    bool result = false; // assume the current (2nd) timestamp is older
    uint16_t absSecondTimestamp = (uint16_t)secondTimestamp;
    uint16_t absFirstTimestamp  = (uint16_t)firstTimestamp;

    // this code is meant to deal with the wrap around the counter does - it is 12 bits wide,
    // so it wraps at 4096
    if (absSecondTimestamp > 1024)  // timestamp is only 12 bits - 1/4 == 1024
    {
        if (absFirstTimestamp < absSecondTimestamp)
        {
            // first (stored) timestamp is older
            result = true;
        }
    }
    else if (firstTimestamp < secondTimestamp)
    {
        // first (stored) timestamp is older
        result = true;
    }

    return (result);
}

//#################################################################################################
// Sets the amount of skip we want to do in the current packet (Host to device);
// 0 goes to the end of the packet
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
void UppHalSetDownstreamSkip(uint8_t skipValue)
{
#ifdef BLACKBIRD_ISO
    uppPtr->ctrl_trfr.s.h2d_skip.bf.cycles = skipValue;
#endif
}


//#################################################################################################
// Sets the amount of skip we want to do in the current packet (Device to host);
// 0 goes to the end of the packet
// 1 skips 4 bytes,
// 2 skips 8 bytes, etc
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
void UppHalSetUpstreamSkip(uint8_t skipValue)
{
#ifdef BLACKBIRD_ISO
    uppPtr->ctrl_trfr.s.d2h_skip.bf.cycles = skipValue;
#endif
}

//#################################################################################################
// Reads a complete packet out of the host to device (H2D) FIFO
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
uint8_t UppHalReadDownstreamData(uint32_t *data)
{
#ifdef BLACKBIRD_ISO
    // read the control
    uint32_t volatile *packetData               = (uint32_t volatile *) &uppPtr->ctrl_trfr.s.h2d_data.dw;
    upp_ctrl_trfr_h2d_ctrl volatile *h2dControl = (upp_ctrl_trfr_h2d_ctrl volatile *) &uppPtr->ctrl_trfr.s.h2d_ctrl;
    uint8_t  volatile byteCount                 = h2dControl->bf.vlane +1;

    if ((h2dControl->bf.eop == 0) && (h2dControl->bf.srdy == 1))
    {
        *data = *packetData;
        uppHalContext.downstreamWordCount++;
    }
    else
    {
        UART_printf("downstream bytecount 0! control %x count %d\n", h2dControl->dw, uppHalContext.downstreamWordCount);
//        UppDiagPrintBufferInfo();
        byteCount = 0;
    }

    return (byteCount);

//    *data = *packetData;
//
//    uppHalContext.downstreamWordCount++;
//    return(4);
#else
    return (0);
#endif
}

//#################################################################################################
// Reads a 32 bit word out of the device to host (D2H - upstream) FIFO
//
// Parameters:
// Return: number of bytes read, returns 0 if no bytes read (error)
//
// Assumptions:
//
//#################################################################################################
uint8_t UppHalReadUpstreamData(uint32_t *data)
{
#ifdef BLACKBIRD_ISO
//    // read the control
//    uint32_t volatile *packetData               = (uint32_t volatile *)&uppPtr->ctrl_trfr.s.d2h_data.dw;
//    upp_ctrl_trfr_d2h_ctrl volatile *d2hControl = (upp_ctrl_trfr_d2h_ctrl volatile *)&uppPtr->ctrl_trfr.s.d2h_ctrl;
//    uint8_t  volatile byteCount                 = d2hControl->bf.vlane +1;
//
//    if ((d2hControl->bf.eop == 0) && (d2hControl->bf.srdy == 1))
//    {
//        *data = *packetData;
//        uppHalContext.upstreamWordCount++;
//    }
//    else
//    {
//        UART_printf("upstream bytecount %d! control %x count %d\n", byteCount, d2hControl->dw, uppHalContext.upstreamWordCount);
//        *data = *packetData;
//        uppHalContext.upstreamWordCount++;
//    }
//
//    return (byteCount);
//
    uint32_t volatile *packetData               = (uint32_t volatile *)&uppPtr->ctrl_trfr.s.d2h_data.dw;

    *data = *packetData;

    uppHalContext.upstreamWordCount++;

    return(4);
#else
    return (0);
#endif
}


//#################################################################################################
// Gets the number of Downstream words read
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
uint8_t UppHalGetDownstreamReadCount(void)
{
    return (uppHalContext.downstreamWordCount);
}

//#################################################################################################
// Gets the number of Upstream words read
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
uint8_t UppHalGetUpstreamReadCount(void)
{
    return (uppHalContext.upstreamWordCount);
}

//#################################################################################################
// flushes the remains of the packet out of the device to host (D2H - upstream) FIFO
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
void UppHalFlushUpstreamPacketRead(void)
{
#ifdef BLACKBIRD_ISO
    LEON_TimerValueT timeStart = LEON_TimerRead();

    // read the control
    upp_ctrl_trfr_d2h_ctrl volatile *d2hControl = (upp_ctrl_trfr_d2h_ctrl volatile *)&uppPtr->ctrl_trfr.s.d2h_ctrl;
    uint32_t volatile *packetData               = (uint32_t volatile *)&uppPtr->ctrl_trfr.s.d2h_data.dw;

    volatile uint32_t data;

    ilog_UPP_COMPONENT_0(ILOG_DEBUG, UPP_PROCESS_D2H);

    for (int count = 0;
         (count < UPP_MAX_PACKET_CONTROL_TRANSFER_WORDS) && (d2hControl->bf.eop == 0) && d2hControl->bf.srdy;
         count++)
    {
        data = *packetData;
        (void )(data);
        UART_printf("discarded data: %x\n", data);
    }

    // now process the last data word
    data = *packetData;
    (void )(data);
    UART_printf("end of data: %x timestamp %d \n", data, d2hControl->bf.time_tag);

    ilog_UPP_COMPONENT_2(ILOG_DEBUG, UPP_END_OF_PACKET,
                         LEON_TimerCalcUsecDiff(timeStart, LEON_TimerRead()),
                         d2hControl->bf.vlane);
#endif
}

//#################################################################################################
// Code to enable/disable the ISO endpoint for the given device
//
// Parameters:
// Return:
//
// Assumptions:
//
//#################################################################################################
void UppHalControlEndpoint(struct Endpoint *endpoint, bool enable)
{
#ifdef BLACKBIRD_ISO
    volatile upp_endpoint_tbl_config0 isoConfig0 = {0};
    volatile upp_endpoint_tbl_config1 isoConfig1 = {0};

    isoConfig0.bf.device_addr     = endpoint->route.deviceAddress;
    isoConfig0.bf.ept_num         = endpoint->info.number;

    if (endpoint->info.assignedQueue != UPP_SET_ENDPOINT_NO_BUFFERS)
    {
        isoConfig0.bf.qid             = endpoint->info.assignedQueue;
    }

    if (enable)
    {
        if (endpoint->info.assignedQueue == UPP_SET_ENDPOINT_NO_BUFFERS)
        {
            isoConfig0.bf.null_en = 1;
        }

        // limit max burst to UPP_ENDPOINT_MAX_BURST and below
        isoConfig1.bf.max_burst   = (endpoint->info.maxBurst > UPP_ENDPOINT_MAX_BURST) ? UPP_ENDPOINT_MAX_BURST : endpoint->info.maxBurst;

        isoConfig1.bf.coded_binterval = 1 << (endpoint->info.bInterval-1);

        isoConfig0.bf.bm_attributes_tt  = endpoint->info.type;
    }

    // order is important here - need to write config1 first, then config0
    // RTL uses the write to config0 to trigger a transfer to the endpoint table
    uppPtr->endpoint_tbl.s.config1.dw = isoConfig1.dw;
    uppPtr->endpoint_tbl.s.config0.dw = isoConfig0.dw;

    ilog_UPP_COMPONENT_3(ILOG_MINOR_EVENT, UPP_HAL_SET_ENDPOINT, isoConfig0.bf.device_addr, isoConfig0.bf.ept_num, isoConfig0.bf.qid);
    ilog_UPP_COMPONENT_3(ILOG_MINOR_EVENT, UPP_HAL_SET_ENDPOINT_RAW, enable, isoConfig0.dw, isoConfig1.dw);
#endif
}


//#################################################################################################
// sets the rd2clr_config for UPP registers so it will automatically clear the stat when read
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void UPPHalSetReadClearStats(void)
{
#ifdef BLACKBIRD_ISO
    uppPtr->id_mgr_fifo.s.write_engine.s.stats0.s.rd2clr_config.dw          = upp_id_mgr_fifo_write_engine_stats0_rd2clr_config_WRITEMASK;

    uppPtr->iso_lex_fifo_0.s.write_engine.s.stats0.s.rd2clr_config.dw       = upp_iso_lex_fifo_0_write_engine_stats0_rd2clr_config_WRITEMASK;
    uppPtr->iso_lex_fifo_0.s.read_engine.s.stats0.s.rd2clr_config.dw        = upp_iso_lex_fifo_0_read_engine_stats0_rd2clr_config_WRITEMASK;

    uppPtr->iso_lex_fifo_1.s.write_engine.s.stats0.s.rd2clr_config.dw       = upp_iso_lex_fifo_1_write_engine_stats0_rd2clr_config_WRITEMASK;
    uppPtr->iso_lex_fifo_1.s.read_engine.s.stats0.s.rd2clr_config.dw        = upp_iso_lex_fifo_1_read_engine_stats0_rd2clr_config_WRITEMASK;

    uppPtr->iso_lex_fifo_2.s.write_engine.s.stats0.s.rd2clr_config.dw       = upp_iso_lex_fifo_2_write_engine_stats0_rd2clr_config_WRITEMASK;
    uppPtr->iso_lex_fifo_2.s.read_engine.s.stats0.s.rd2clr_config.dw        = upp_iso_lex_fifo_2_read_engine_stats0_rd2clr_config_WRITEMASK;

    uppPtr->iso_lex_fifo_3.s.write_engine.s.stats0.s.rd2clr_config.dw       = upp_iso_lex_fifo_3_write_engine_stats0_rd2clr_config_WRITEMASK;
    uppPtr->iso_lex_fifo_3.s.read_engine.s.stats0.s.rd2clr_config.dw        = upp_iso_lex_fifo_3_read_engine_stats0_rd2clr_config_WRITEMASK;

    uppPtr->iso_rex_fifo.s.write_engine.s.stats0.s.rd2clr_config.dw         = upp_iso_rex_fifo_write_engine_stats0_rd2clr_config_WRITEMASK;

    uppPtr->ctrl_trfr_h2d_fifo.s.write_engine.s.stats0.s.rd2clr_config.dw  =
        upp_ctrl_trfr_h2d_fifo_write_engine_stats0_rd2clr_config_WRITEMASK &
            ~(UPP_CTRL_TRFR_H2D_FIFO_WRITE_ENGINE_STATS0_RD2CLR_CONFIG_PFIFO_DCOUNT_MASK | UPP_CTRL_TRFR_H2D_FIFO_WRITE_ENGINE_STATS0_RD2CLR_CONFIG_NFIFO_DCOUNT_MASK);

    uppPtr->ctrl_trfr_d2h_fifo.s.write_engine.s.stats0.s.rd2clr_config.dw  =
        upp_ctrl_trfr_d2h_fifo_write_engine_stats0_rd2clr_config_WRITEMASK &
            ~(UPP_CTRL_TRFR_D2H_FIFO_WRITE_ENGINE_STATS0_RD2CLR_CONFIG_PFIFO_DCOUNT_MASK | UPP_CTRL_TRFR_D2H_FIFO_WRITE_ENGINE_STATS0_RD2CLR_CONFIG_NFIFO_DCOUNT_MASK);

    uppPtr->id_mgr_fifo.s.read_engine.s.stats0.s.rd2clr_config.dw          = upp_id_mgr_fifo_read_engine_stats0_rd2clr_config_WRITEMASK;
    uppPtr->iso_rex_fifo.s.read_engine.s.stats0.s.rd2clr_config.dw         = upp_iso_rex_fifo_read_engine_stats0_rd2clr_config_WRITEMASK;

    uppPtr->ctrl_trfr_h2d_fifo.s.read_engine.s.stats0.s.rd2clr_config.dw   = upp_ctrl_trfr_h2d_fifo_read_engine_stats0_rd2clr_config_WRITEMASK;
    uppPtr->ctrl_trfr_d2h_fifo.s.read_engine.s.stats0.s.rd2clr_config.dw   = upp_ctrl_trfr_d2h_fifo_read_engine_stats0_rd2clr_config_WRITEMASK;
#endif
}

// Static Function Definitions ####################################################################




