//#################################################################################################
// Icron Technology Corporation - Copyright 2017
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
// This file contains code to interface to the 5G link module, used for fiber
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//
//
//#################################################################################################

// Includes #######################################################################################
#include <ibase.h>
#include <bb_top.h>
#include <bb_top_regs.h>
#include <bb_top_dp.h>
#include <leon_timers.h>
#include <timing_timers.h>
#include <fiber5g.h>
#include "fiber5g_log.h"
#include <module_addresses_regs.h>

// #include <uart.h>       // for UART_printf()

// Constants and Macros ###########################################################################

// the actual debounce time is twice this; one cycle is used to prevent spurious interrupts
// from bogging down the system, the 2nd interval is to make sure it doesn't change state
#define SL_5G_LOS_STATUS_DEBOUNCE_TIME  50

// the amount of time to wait after Rx has trained before checking to see if it is valid
#define SL_5G_LINK_STABILITY_CHECK_TIME  250

#define FIBER_STATS_POLL_INTERVAL     (100/STATISTIC_INTERVAL_TICK)        // poll the fiber stats every 100ms

// Absolute, should not change -- if changes - print
#define FIBER_STAT_FPGA_REG_NOCHNG_ABS(fpgaAddress, ilogCode)     \
    STATMON_FPGA_READ_PARAM(fpgaAddress, 0, LINKMGR_COMPONENT, ilogCode)

#define FIBER_MAX_RX_TRAIN_TIME     (10*1000)   // maximum Rx train time is 10ms; typically is ~550uS

#define BB_TOP_ADDRESS(componentStatAddress) (bb_chip_bb_top_s_ADDRESS + componentStatAddress)

// Data Types #####################################################################################
enum SL5GLosDebounceState
{
    SL_5G_DEBOUNCE_DELAY,               // delay to make sure we don't change state too fast
    SL_5G_DEBOUNCE_LOOK_FOR_CHANGE,     // LOS interrupt enabled; seeing if the signal changes
    SL_5G_DEBOUNCE_DONE,                // LOS interrupt enabled; signal debounced
};

struct DrpAddressValuePair
{
    uint16_t addr;
    uint16_t val;
};


// Static Function Declarations ###################################################################
static void Link_SL_5G_LosDebounceTimer(void)                           __attribute__ ((section(".atext")));
static void Link_SL_5G_StabilityCheck(void)                             __attribute__ ((section(".atext")));
static void Link_SL_5G_UpdateLinkStatus(bool linkUp)                    __attribute__ ((section(".atext")));
static void Link_SL_5G_TakedownReceive(void)                            __attribute__ ((section(".atext")));
static void Link_SL_5G_BringUpReceive(void)                             __attribute__ ((section(".atext")));
static void Link_SL_5G_TakedownTransmit(void)                           __attribute__ ((section(".atext")));
static void Link_SL_5G_BringUpTransmit(void)                            __attribute__ ((section(".atext")));
static void Link_SL_5G_RxPowerChangeCallback(bool aboveThreshold)       __attribute__ ((section(".atext")));
static void Link_SL_5G_TogggleElasticBuffer(void)                       __attribute__ ((section(".atext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static const struct DrpAddressValuePair s15G_Init[] =
{
    {0x0A, 0x1104 },
    {0x0C, 0x1208 },
    {0x0E, 0x1082 },
    {0x0F, 0x0000 },
};

static const StatFpgaReg FiberStatU32Regs[] =
{
    FIBER_STAT_FPGA_REG_NOCHNG_ABS( BB_TOP_ADDRESS(bb_top_sl_5g_stats0_gt_disp_err_ADDRESS),         LINK_5G_FIBER_STATS_RX_DISPARITY),
    FIBER_STAT_FPGA_REG_NOCHNG_ABS( BB_TOP_ADDRESS(bb_top_sl_5g_stats0_gt_not_in_table_ADDRESS),     LINK_5G_FIBER_STATS_RX_SYMBOL_NOT_IN_TABLE),
    FIBER_STAT_FPGA_REG_NOCHNG_ABS( BB_TOP_ADDRESS(bb_top_sl_5g_stats0_gt_rxbyterealign_ADDRESS),    LINK_5G_FIBER_STATS_RX_REALIGN_COUNT),
};

static uint32_t FiberStatStatU32data[ARRAYSIZE(FiberStatU32Regs)];

const StatRegistration Fiber16bitsStats = STATMON_REGISTRATION_INIT(
        STATISTIC_DATA_SIZE_32_BITS,
        FiberStatU32Regs,
        FiberStatStatU32data,
        STATSMON_FpgaRegisterRead,
        STATSMON_ClearStatData,
        FIBER_STATS_POLL_INTERVAL);


static struct SL_5GContext
{
    volatile bb_top_s *bb_top_regs;

    LinkStatusChangeHandler notifyStatusChange;     // handler to notify on status change
    bool linkStatus;                                // true if link is up, false if it is down

    TIMING_TimerHandlerT  LosDebounceTimer;         // Loss of Signal (LOS) debounce timer
    enum SL5GLosDebounceState losDebounceState;     // the debounce state we are in
    bool losState;                                  // the current LOS state we have

    LEON_TimerValueT rxStartTrainTime;              // timestamp for when we started training

    TIMING_TimerHandlerT  stabilityCheck;           // link stability check timer
    bool stabilityPassed;                           // flag to mark that the stability check passed

    LinkDisconnectHandler disconnectHandler;        // To inform link error
} sl_5Gvars;

//static volatile bb_chip_s* bb_chip = (volatile void*)(bb_chip_s_ADDRESS);
//static volatile bb_top_s* bb_top_registers = ((bb_chip_s*)((volatile void*)(bb_chip_s_ADDRESS)));

// Exported Function Definitions ##################################################################


// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Initialize the SL (single lane) 5Gbit module
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void Link_SL_5G_Init(LinkStatusChangeHandler notifyChangeHandler, LinkDisconnectHandler disconnectHandler)
{
    ilog_FIBER5G_COMPONENT_0(ILOG_MINOR_EVENT, LINK_5G_INIT);

    sl_5Gvars.bb_top_regs = (volatile bb_top_s *)bb_chip_bb_top_s_ADDRESS;
    sl_5Gvars.notifyStatusChange = notifyChangeHandler;
    sl_5Gvars.disconnectHandler = disconnectHandler;

    // Select SL 5G
    sl_5Gvars.bb_top_regs->link_ctrl.bf.sel = 1;

    sl_5Gvars.LosDebounceTimer = TIMING_TimerRegisterHandler(
        Link_SL_5G_LosDebounceTimer,
        true,   // periodic polling needed
        SL_5G_LOS_STATUS_DEBOUNCE_TIME);

    sl_5Gvars.stabilityCheck = TIMING_TimerRegisterHandler(
        Link_SL_5G_StabilityCheck,
        true,   // periodically check until system up or error detected
        SL_5G_LINK_STABILITY_CHECK_TIME);

    STATSMON_RegisterStatgroup(&Fiber16bitsStats);

    I2CD_sfpFinisarInit();

    I2CD_sfpFinisarRxPowerPollingInit(Link_SL_5G_RxPowerChangeCallback);

    // Initialize the DRP for SL 5G hardware.
    for (uint8_t writeCount = 0; writeCount < ARRAYSIZE(s15G_Init); writeCount++)
    {
        bb_top_dpDrpWrite(
            s15G_Init[writeCount].addr,
            s15G_Init[writeCount].val,
            BB_TOP_DRP_DRP_EN_MASK_LINK_MMCM_MASK);
    }

    sl_5Gvars.bb_top_regs->sl_5g.s.stats0.s.rd2clr_config.dw = bb_top_sl_5g_stats0_rd2clr_config_WRITEMASK;
}


//#################################################################################################
// Transmit FSM done IRQ handler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void Link_SL_5G_TxFsmResetDoneIrq(void)
{
    ilog_FIBER5G_COMPONENT_0(ILOG_USER_LOG, LINK_5G_FIBER_TX_FSM_RESET_DONE);

    sl_5Gvars.bb_top_regs->irq.s.enable.bf.sl_5g_tx_fsm_reset_done  = 0;    // we only need this interrupt once
    sl_5Gvars.bb_top_regs->irq.s.pending.bf.sl_5g_tx_fsm_reset_done = 1;    // clear pending

    // Based on Kris’ recommendations the TXDIFFCTRL value (bb_top.sl_5g.gt_tx_ctrl.txdiffctrl) should
    // be between 700mV and 800mV but by default is set to 499mv.  Set it to 0x8 (743mV).
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_tx_ctrl.bf.txdiffctrl = 0x08;
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_tx_ctrl.bf.txpostcursor = 0x09;       // 2.21 dB Emphasis requested by Bug#5029

    // Take Tx transceivers out of power down
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_tx_ctrl.bf.txpd = 0x0;

    bb_top_applySfpTransmitterEnable(true);   // make sure the fiber transmitter is on

    // Ok, TX side is setup - now work on the receive side

    Link_SL_5G_LosChangeIrq();  // process the current LOS value
}

//#################################################################################################
// Receive FSM done IRQ handler
//
// Final step in receive bring up - training is complete; say this part of the link is up
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void Link_SL_5G_RxFsmResetDoneIrq(void)
{
    sl_5Gvars.bb_top_regs->irq.s.enable.bf.sl_5g_rx_fsm_reset_done  = 0;    // we only need this interrupt once
    sl_5Gvars.bb_top_regs->irq.s.pending.bf.sl_5g_rx_fsm_reset_done = 1;    // clear pending

    // attribute unused and volatile qualifiers used to prevent compiler from optimizing out this
    // code.  Registers need to be read once to be cleared
    volatile uint32_t __attribute__ ((unused)) temp;
    temp = (volatile uint32_t)sl_5Gvars.bb_top_regs->sl_5g.s.stats0.s.gt_disp_err.dw;
    temp = (volatile uint32_t)sl_5Gvars.bb_top_regs->sl_5g.s.stats0.s.gt_not_in_table.dw;
    temp = (volatile uint32_t)sl_5Gvars.bb_top_regs->sl_5g.s.stats0.s.gt_rxbyterealign.dw;

    uint32_t rxTrainTime = LEON_TimerCalcUsecDiff(sl_5Gvars.rxStartTrainTime, LEON_TimerRead());
    ilog_FIBER5G_COMPONENT_1(ILOG_USER_LOG, LINK_5G_FIBER_RX_FSM_RESET_DONE, rxTrainTime);

    // training done, start the stability check
    TIMING_TimerStart(sl_5Gvars.stabilityCheck);
}


//#################################################################################################
// Receive Elastic buffer underflow IRQ handler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void Link_SL_5G_RxBufferUnderflowIsr(void)
{
    sl_5Gvars.bb_top_regs->irq.s.pending.bf.sl_5g_rx_buf_underflow = 1;    // clear pending

    Link_SL_5G_TogggleElasticBuffer();

    ilog_FIBER5G_COMPONENT_0(ILOG_MAJOR_ERROR, LINK_5G_FIBER_RX_BUF_UNDERFLOW);
}

//#################################################################################################
// Receive Elastic buffer overflow IRQ handler
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void Link_SL_5G_RxBufferOverflowIsr(void)
{
    sl_5Gvars.bb_top_regs->irq.s.pending.bf.sl_5g_rx_buf_overflow = 1;    // clear pending

    Link_SL_5G_TogggleElasticBuffer();

    ilog_FIBER5G_COMPONENT_0(ILOG_MAJOR_ERROR, LINK_5G_FIBER_RX_BUF_OVERFLOW);
}

//#################################################################################################
// LOS (loss of signal) change interrupt handler
//
// 1st step in bringing up the receive side of the fiber.  Starts the timer to debounce the signal,
// then the timer callback does the next step in the receive setup
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void Link_SL_5G_LosChangeIrq(void)
{
    // get the current value
    bool currentLosValue = sl_5Gvars.bb_top_regs->irq.s.raw.bf.sfp_los;

    sl_5Gvars.bb_top_regs->irq.s.enable.bf.sfp_los  = 0;    // disable this interrupt
    sl_5Gvars.bb_top_regs->irq.s.pending.bf.sfp_los = 1;    // clear the pending flag

    sl_5Gvars.losState = currentLosValue;

    // disable power checking - once the signal is detected again, the timer callback will
    // turn it back on
    I2CD_sfpFinisarRxPowerPollingDisable();

    // We've got a LOS change interrupt - go immediately to the link down state
    if (sl_5Gvars.stabilityPassed == true)
    {
        sl_5Gvars.disconnectHandler();          // link error detected
    }
    else
    {
        Link_SL_5G_UpdateLinkStatus(false);
        Link_SL_5G_TakedownReceive();

        // debounce the change in LOS (will re-enable interrupt once debounced, to prevent
        // spurious interrupts from bogging us down)
        TIMING_TimerStart(sl_5Gvars.LosDebounceTimer);
        sl_5Gvars.losDebounceState = SL_5G_DEBOUNCE_DELAY;
    }

    ilog_FIBER5G_COMPONENT_0(ILOG_USER_LOG, (currentLosValue ? LINK_5G_FIBER_LOS_IRQ_ACTIVE : LINK_5G_FIBER_LOS_IRQ_INACTIVE));
}


//#################################################################################################
// Starts and stops the SL 5G module
//
// Parameters: enable - true = on, false = off
// Return:
// Assumptions:
//
//#################################################################################################
void Link_SL_5G_Control(bool enable)
{
    ilog_FIBER5G_COMPONENT_0(ILOG_USER_LOG, (enable ? LINK_5G_FIBER_ENABLED : LINK_5G_FIBER_DISABLED));

    if (enable)
    {
        Link_SL_5G_BringUpTransmit();  // now start bringing up the transmitter
    }
    else
    {
        I2CD_sfpFinisarRxPowerPollingDisable();

        Link_SL_5G_TakedownTransmit();  // link disabled, bring down transmit
        Link_SL_5G_TakedownReceive();   // and receive
    }

    // link is down - if we are enabled, we will indicate when it is up
    sl_5Gvars.linkStatus = false;

    TIMING_TimerStop(sl_5Gvars.LosDebounceTimer);
    TIMING_TimerStop(sl_5Gvars.stabilityCheck);

    sl_5Gvars.losDebounceState = SL_5G_DEBOUNCE_DELAY;

}

//#################################################################################################
// Restarts the Rx side of the Fiber
//
// Parameters: enable - true = on, false = off
// Return:
// Assumptions:
//
//#################################################################################################
void Link_SL_5G_RestartRx(void)
{
    if (sl_5Gvars.stabilityPassed == true)
    {
        sl_5Gvars.disconnectHandler();          // link error detected
    }
    else
    {
        // link errors detected!  Restart Rx training
        Link_SL_5G_LosChangeIrq();
    }
}


//#################################################################################################
// Called from the link manager when system up has been detected
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
void Link_SL_5G_SystemUp(void)
{
    ilog_FIBER5G_COMPONENT_0(ILOG_USER_LOG, LINK_5G_SYSTEM_UP);

    // stop stability checks - system is up!
    TIMING_TimerStop(sl_5Gvars.stabilityCheck);

    STATSMON_StatgroupControl(&Fiber16bitsStats, true); // start monitoring stats
}


//#################################################################################################
// Gets the SL 5G link status
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
bool Link_SL_5G_Status( void )
{
    return(sl_5Gvars.linkStatus);
}

// Static Function Definitions ####################################################################

//#################################################################################################
// Updates the rest of the system with a link status change
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void Link_SL_5G_UpdateLinkStatus(bool linkUp)
{
//    UART_printf("Link_SL_5G_UpdateLinkStatus %d\n",linkUp);

    if (linkUp != sl_5Gvars.linkStatus)
    {
        sl_5Gvars.linkStatus = linkUp;
        sl_5Gvars.notifyStatusChange(linkUp? LINK_OPERATING: LINK_IN_RESET);
    }
}

//#################################################################################################
// Debounces LOS
//
// 2nd step in bringing up the receive side
//
// Once we get a stable LOS, turn the LOS interrupt on.  If the interrupt fires, LOS is still not
// stable, and go back and wait some more.  If there was no change while interrupts are on,
// consider LOS stable and go on to the next state in the receive setup
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void Link_SL_5G_LosDebounceTimer(void)
{
    ilog_FIBER5G_COMPONENT_3(ILOG_USER_LOG, LINK_5G_FIBER_SIGNAL_DEBOUNCE,
        sl_5Gvars.losDebounceState, sl_5Gvars.losState, sl_5Gvars.bb_top_regs->irq.s.raw.bf.sfp_los);

    switch (sl_5Gvars.losDebounceState)
    {
        case SL_5G_DEBOUNCE_DELAY:              // delay to make sure we don't change state too fast
            if (sl_5Gvars.losState == sl_5Gvars.bb_top_regs->irq.s.raw.bf.sfp_los)
            {
                sl_5Gvars.losDebounceState = SL_5G_DEBOUNCE_LOOK_FOR_CHANGE;

                // no change since last time - enable interrupts.
                // if no change found after waiting a while more, then consider it debounced
                sl_5Gvars.bb_top_regs->irq.s.pending.bf.sfp_los = 1;    // clear the pending flag
                sl_5Gvars.bb_top_regs->irq.s.enable.bf.sfp_los  = 1;    // enable this interrupt
            }
            else
            {
                sl_5Gvars.losState = sl_5Gvars.bb_top_regs->irq.s.raw.bf.sfp_los;
            }
            break;

        case SL_5G_DEBOUNCE_LOOK_FOR_CHANGE:    // LOS interrupt enabled; seeing if the signal changes
            // interrupts are on, no change detected.  Signal is debounced
            if (sl_5Gvars.losState == 0)
            {
                // signal detected - finish up with rest of Rx setup
                I2CD_sfpFinisarRxPowerPollingEnable();  // make sure the signal is valid before linking up
                ilog_FIBER5G_COMPONENT_0(ILOG_USER_LOG, LINK_5G_FIBER_LOS_INACTIVE);
            }

            sl_5Gvars.losDebounceState = SL_5G_DEBOUNCE_DONE;
            TIMING_TimerStop(sl_5Gvars.LosDebounceTimer);
            break;

        case SL_5G_DEBOUNCE_DONE:               // LOS interrupt enabled; signal debounced
        default:
            break;
    }
}


//#################################################################################################
// Stability check once link training is done, to make sure the link is valid
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void Link_SL_5G_StabilityCheck(void)
{
    uint32_t dispErr    = (volatile uint32_t)sl_5Gvars.bb_top_regs->sl_5g.s.stats0.s.gt_disp_err.dw;
    uint32_t notInTable = (volatile uint32_t)sl_5Gvars.bb_top_regs->sl_5g.s.stats0.s.gt_not_in_table.dw;
    uint32_t gtRealign  = (volatile uint32_t)sl_5Gvars.bb_top_regs->sl_5g.s.stats0.s.gt_rxbyterealign.dw;

    if ( (dispErr + notInTable + gtRealign) == 0)
    {
        if (sl_5Gvars.stabilityPassed == false)
        {
            sl_5Gvars.stabilityPassed = true;

            ilog_FIBER5G_COMPONENT_3(ILOG_USER_LOG, LINK_5G_FIBER_RX_STABILITY, dispErr, notInTable, gtRealign);

            // stability check passed - declare Phy layer up, and turn on and clear the
            // elastic buffer overflow/underflow IRQ's
            // clear pending, first
            sl_5Gvars.bb_top_regs->irq.s.pending.bf.sl_5g_rx_buf_overflow   = 1;
            sl_5Gvars.bb_top_regs->irq.s.pending.bf.sl_5g_rx_buf_underflow  = 1;

            // now enable the interrupts
            sl_5Gvars.bb_top_regs->irq.s.enable.bf.sl_5g_rx_buf_overflow    = 1;
            sl_5Gvars.bb_top_regs->irq.s.enable.bf.sl_5g_rx_buf_underflow   = 1;

            Link_SL_5G_TogggleElasticBuffer();  // reset the Rx elastic buffer

            // this takes the RX shim logic between the RS and the transceiver out of reset
            sl_5Gvars.bb_top_regs->grm.s.soft_rst_ctrl.bf.sl_5g_rx_rst = 0;
            LEON_TimerWaitMicroSec(10);  // give at least 10us for the shim logic to properly start working

            Link_SL_5G_UpdateLinkStatus(true);  // link is up!
        }
    }
    else
    {
        ilog_FIBER5G_COMPONENT_3(ILOG_USER_LOG, LINK_5G_FIBER_RX_STABILITY, dispErr, notInTable, gtRealign);

        if (sl_5Gvars.stabilityPassed == true)
        {
            sl_5Gvars.disconnectHandler();          // link error detected
        }
        else
        {
            // link errors detected!  Restart Rx training
            Link_SL_5G_RestartRx();
        }
    }
}


//#################################################################################################
// Signal lost - put receive into reset
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void Link_SL_5G_TakedownReceive(void)
{
    STATSMON_StatgroupControl(&Fiber16bitsStats, false);

    // Put SL 5G Rx Support Logic into reset
    sl_5Gvars.bb_top_regs->grm.s.soft_rst_ctrl.bf.sl_5g_rx_rst = 1;

    sl_5Gvars.bb_top_regs->irq.s.enable.bf.sfp_los  = 0;
    sl_5Gvars.bb_top_regs->irq.s.enable.bf.sl_5g_rx_fsm_reset_done  = 0;
    sl_5Gvars.bb_top_regs->irq.s.enable.bf.sl_5g_rx_buf_overflow    = 0;
    sl_5Gvars.bb_top_regs->irq.s.enable.bf.sl_5g_rx_buf_underflow   = 0;

    // Put the SL 5G Rx into reset
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_rst_ctrl.bf.soft_reset_rx = 1;

    // power down the Rx transceivers
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_rx_ctrl.bf.rxpd = 0x03;

    TIMING_TimerStop(sl_5Gvars.stabilityCheck); // make sure the stability check timer is off
    sl_5Gvars.stabilityPassed = false;
}


//#################################################################################################
// Receive Bring up
//
// 4th step in receive bring up - signal levels are ok; start the receive training state machine,
// and wait for it to finish
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void Link_SL_5G_BringUpReceive(void)
{
   // Enable RX FSM reset done interrupt
    sl_5Gvars.bb_top_regs->irq.s.pending.bf.sl_5g_rx_fsm_reset_done = 1;    // clear pending, first
    sl_5Gvars.bb_top_regs->irq.s.enable.bf.sl_5g_rx_fsm_reset_done  = 1;    // enable the interrupt, next

    // Take RX transceivers out of power down
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_rx_ctrl.bf.rxpd = 0x0;

    // Take the SL 5G out of reset
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_rst_ctrl.bf.soft_reset_rx = 0;

    // setup to only allow resets when the errors persist for more then 100uS
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_rx_ctrl.bf.data_valid_en = 1;
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_rst_ctrl.bf.dont_reset_on_data_error = 0;

    sl_5Gvars.rxStartTrainTime = LEON_TimerRead();

    // make sure all stats are read to clear
    sl_5Gvars.bb_top_regs->sl_5g.s.stats0.s.rd2clr_config.dw = bb_top_sl_5g_stats0_rd2clr_config_WRITEMASK;

    // Wait for ISR from sl_5g_rx_fsm_reset_done
    ilog_FIBER5G_COMPONENT_0(ILOG_USER_LOG, LINK_5G_FIBER_RX_FSM_RESET_START);
}

//#################################################################################################
// take down the transmitter
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void Link_SL_5G_TakedownTransmit(void)
{
    sl_5Gvars.bb_top_regs->irq.s.enable.bf.sl_5g_tx_fsm_reset_done  = 0;    // Disable TX FSM reset done interrupt

    // Put SL 5G Support Logic into reset
    sl_5Gvars.bb_top_regs->grm.s.soft_rst_ctrl.bf.sl_5g_tx_rst = 1;

    bb_top_applySfpTransmitterEnable(false);   // make sure the fiber transmitter is off

    // Disable interrupts
    sl_5Gvars.bb_top_regs->irq.s.enable.bf.sl_5g_tx_fsm_reset_done = 0;

    // Put the SL 5G into reset
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_rst_ctrl.bf.soft_reset_tx = 1;

    // power down the Tx transceiver
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_tx_ctrl.bf.txpd = 0x03;
}


//#################################################################################################
// Transmit bring up
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void Link_SL_5G_BringUpTransmit(void)
{
    sl_5Gvars.bb_top_regs->irq.s.pending.bf.sl_5g_tx_fsm_reset_done = 1;    // clear Tx FSM reset done pending
    sl_5Gvars.bb_top_regs->irq.s.enable.bf.sl_5g_tx_fsm_reset_done  = 1;    // Enable TX FSM reset done interrupt

    // Take SL 5G TX Support Logic out of reset (still held in reset if link_mmcm_lock is not 1)
    sl_5Gvars.bb_top_regs->grm.s.soft_rst_ctrl.bf.sl_5g_tx_rst = 0;

    // Take the SL 5G out of reset
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_rst_ctrl.bf.soft_reset_tx = 0;

    if(bb_top_getCoreBoardRev() >= BOM_23_00200_A03)
    {
        // A03+ boards use an inverted Tx
        sl_5Gvars.bb_top_regs->sl_5g.s.gt_tx_ctrl.bf.txpolarity = 1;
    }

    // Wait for ISR from sl_5g_tx_fsm_reset_done interrupt to go on to the next phase
    ilog_FIBER5G_COMPONENT_0(ILOG_USER_LOG, LINK_5G_FIBER_TX_FSM_RESET_START);
}


//#################################################################################################
// Called when there is a change in the received power - true if we are above our threshold
//
// 3rd step in receive bring up - if the signal meets the power constraints,
// (signalValid == true), then bring up the receiver; if the signal is not valid, then go restart
// the sequence
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void Link_SL_5G_RxPowerChangeCallback(bool signalValid)
{
    if (signalValid)
    {
        ilog_FIBER5G_COMPONENT_0(ILOG_USER_LOG, LINK_5G_FIBER_SIGNAL_VALID);
        Link_SL_5G_BringUpReceive();
    }
    else
    {
        // signal not valid - bring down the link
        ilog_FIBER5G_COMPONENT_0(ILOG_USER_LOG, LINK_5G_FIBER_SIGNAL_INVALID);
        Link_SL_5G_UpdateLinkStatus(false);
        Link_SL_5G_TakedownReceive();
    }
}

//#################################################################################################
// Called to reset the elastic buffer
//
// Parameters:
// Return:
// Assumptions:
//
//#################################################################################################
static void Link_SL_5G_TogggleElasticBuffer(void)
{
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_rst_ctrl.bf.rxbufreset = 1;
    sl_5Gvars.bb_top_regs->sl_5g.s.gt_rst_ctrl.bf.rxbufreset = 0;
}


