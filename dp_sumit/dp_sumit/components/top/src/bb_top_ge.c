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
// Implementations of functions common to the Lex and Rex GE control.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################

// Includes #######################################################################################
#include <bb_top_ge.h>
#include <bb_top_a7_regs.h>
#include <timing_timers.h>
#include "bb_top_log.h"
#include <module_addresses_regs.h>

// Constants and Macros ###########################################################################
#define GE_RESET_CHECK_TIME                     3000    // set the GE watchdog timer to 3s
#define GE_ALIVE_CHECK_TIME                     3000    // set the GE watchdog timer to 3s

// Data Types #####################################################################################
enum Ge_OperationMode
{
    GE_OPERATION_RESET,         // GE is on reset mode (reset port low: reset on)
    GE_OPERATION_RUN,           // GE is on running mode (reset port high: reset off, boot port high: boot off)
    GE_OPERATION_BOOTLOAD,      // GE is on bootloader mode (reset port high: reset off, boot port low: boot on))
};

// Static Function Declarations ###################################################################
static void bb_top_applyGEBootSelProgram(bool program)      __attribute__((section(".atext")));
static void bb_top_setGeUartSlave(enum GeUartSlaveSel sel)  __attribute__((section(".atext")));
static void bb_top_a7_ControlGeDataPhy(bool reset);
static void bb_top_a7_applyGEVbusDetect(bool setVbus);
static void resetWatchdogHandler(void)                      __attribute__ ((section(".atext")));
static void runWatchdogHandler(void)                        __attribute__ ((section(".atext")));

// Static Variables ###############################################################################
static volatile bb_top_s* bb_top_registers;
static struct BB_TOP_GE_Context
{
    TIMING_TimerHandlerT geWatchdogResetTimer;      // check GE is alive after Reset
    TIMING_TimerHandlerT geWatchdogRunningTimer;    // check GE is alive after beginning work
    WatchdogCallback resetWatchdogCallback;         // callback function when reset watchdog occur
    WatchdogCallback runWatchdogCallback;           // callback function when run watchdog occur
    enum Ge_OperationMode geOperationMode;          // To identify GE's current mode
} bbTopGeContext;

// Component Variables ############################################################################

// Global Variables ###############################################################################

// Exported Function Definitions ##################################################################


//#################################################################################################
// Initialize register pointer
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ge_Init(void)
{
    bb_top_registers = (volatile bb_top_s*) bb_chip_bb_top_s_ADDRESS;

    bbTopGeContext.geWatchdogResetTimer = TIMING_TimerRegisterHandler(
        resetWatchdogHandler,
        false,
        GE_RESET_CHECK_TIME);

    bbTopGeContext.geWatchdogRunningTimer = TIMING_TimerRegisterHandler(
        runWatchdogHandler,
        false,
        GE_ALIVE_CHECK_TIME);

}

//#################################################################################################
// GE ASIC interrupt
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyGEInterrupt(bool setInt)
{
    // int is active low
    bb_top_registers->ge_ctrl.bf.int_n = !setInt;
}

//#################################################################################################
// Put GE into reset mode
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_SetGEToResetMode(void)
{
#ifndef BB_ROM
    ilog_TOP_COMPONENT_0(ILOG_USER_LOG, BB_TOP_HOLD_GE_RESET);
#endif    //BB_ROM
    bbTopGeContext.geOperationMode = GE_OPERATION_RESET;

    bb_top_ApplyGEReset(true);
    bb_top_StopGEWatchdogResetTimer();
    bb_top_StopGEWatchdogRunningTimer();
}

//#################################################################################################
// Release Reset and Start GE
//
// Parameters: WatchdogCallbacks to check reset or running (both can be NULL when it doesn't need check)
// Return:
// Assumptions: Start GE called when GE is on reset mode. That means GE run mode is not called when it run already
//#################################################################################################
void bb_top_SetGEToRunMode(WatchdogCallback resetHandler, WatchdogCallback runHandler)
{
#ifndef BB_ROM
    iassert_TOP_COMPONENT_0(bbTopGeContext.geOperationMode != GE_OPERATION_RUN, BB_TOP_GE_RUN_AGAIN);
    ilog_TOP_COMPONENT_0(ILOG_USER_LOG, BB_TOP_GE_RUN);
#endif    //BB_ROM
    
    bbTopGeContext.geOperationMode = GE_OPERATION_RUN;

    bb_top_applyGEBootSelProgram(false);                    // make sure GE is in run mode
    bb_top_setGeUartSlave(GE_UART_SLAVE_SEL_GE_UART);       // set internal GE uart (default: mother board)

    // remove reset from GE
    // Doesn't need stable time between bootsel and reset. FPGA has internal delay when it set reset
    bb_top_ApplyGEReset(false);

    bbTopGeContext.resetWatchdogCallback = resetHandler;
    bbTopGeContext.runWatchdogCallback = runHandler;

    TIMING_TimerStart(bbTopGeContext.geWatchdogResetTimer);
}

//#################################################################################################
// Set GE Bootloader mode
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_SetGEToBootloaderMode(void)
{
#ifndef BB_ROM
    ilog_TOP_COMPONENT_0(ILOG_USER_LOG, BB_TOP_GE_BOOTLOADER);
    bbTopGeContext.geOperationMode = GE_OPERATION_BOOTLOAD;
#endif    //BB_ROM
    bb_top_applyGEBootSelProgram(true);                     // set GE bootloader mode
    bb_top_setGeUartSlave(GE_UART_SLAVE_SEL_GE_UART);       // set internal GE uart (default: mother board)

    bb_top_StopGEWatchdogResetTimer();
    bb_top_StopGEWatchdogRunningTimer();

    // remove reset from GE
    // Doesn't need stable time between bootsel and reset. FPGA has internal delay when it set reset
    bb_top_ApplyGEReset(false);
}

//#################################################################################################
// stop reset watchdog timer
//
// Parameters:
// Return:
// Assumptions: It should to be called when the watchdog doesn't need to be checked anymore
//#################################################################################################
void bb_top_StopGEWatchdogResetTimer(void)
{
    TIMING_TimerStop(bbTopGeContext.geWatchdogResetTimer);
    bbTopGeContext.resetWatchdogCallback = NULL;
}

//#################################################################################################
// stop running watchdog timer
//
// Parameters:
// Return:
// Assumptions: Need to be called when the watchdog isn't required
//#################################################################################################
void bb_top_StopGEWatchdogRunningTimer(void)
{
    TIMING_TimerStop(bbTopGeContext.geWatchdogRunningTimer);
    bbTopGeContext.runWatchdogCallback = NULL;
}

//#################################################################################################
// stop running watchdog timer
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_StartGEWatchdogRunningTimer(void)
{
    // Not to start watchdog timer by late GE message after GE reset is on
    if(bbTopGeContext.geOperationMode == GE_OPERATION_RUN)
    {
        TIMING_TimerStart(bbTopGeContext.geWatchdogRunningTimer);
    }
}

//#################################################################################################
// Control the reset signal to the GE Phy (CLM channel between Blackbird and GE)
//
// Parameters:
//      reset               - true to apply reset or false to clear reset
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ControlGeDataPhy(bool enable)
{
#ifdef PLATFORM_A7
    bb_top_a7_ControlGeDataPhy(enable);
#endif
#ifdef PLATFORM_K7
    bb_top_k7_ApplyResetPhyRx(reset);
#endif
}

//#################################################################################################
// GE ASIC vbus_det
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyGEVbusDetect(bool setVbus)
{
#ifdef PLATFORM_A7
    bb_top_a7_applyGEVbusDetect(setVbus);
#endif
}

//#################################################################################################
// GE ASIC reset control
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void bb_top_ApplyGEReset(bool reset)
{
    // reset is active low
    bb_top_registers->ge_ctrl.bf.rst_b = !reset;
}

//#################################################################################################
// GE ASIC reset information check
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
bool bb_top_isGEResetOn(void)
{
    // reset is active low
    return !(bb_top_registers->ge_ctrl.bf.rst_b);
}

// Component Scope Function Definitions ###########################################################


// Static Function Definitions ####################################################################
//#################################################################################################
// GE ASIC boot select
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void bb_top_applyGEBootSelProgram(bool program)
{
    // program is active low
    bb_top_registers->ge_ctrl.bf.boot_sel = !program;
}

//#################################################################################################
// Select GE Uart Direction.
//
// Parameters:
//      sel                 - mux output GE ASIC's UART will be connected to
// Return:
// Assumptions:
//#################################################################################################
static void bb_top_setGeUartSlave(enum GeUartSlaveSel sel)
{
    bb_top_registers->uart_ctrl.bf.ge_uart_sel = sel;
}

//#################################################################################################
// Controls the data Phy link between GE and Blackbird
//
// Parameters:
//      reset               - true to enable, false to disable
// Return:
// Assumptions:
//#################################################################################################
static void bb_top_a7_ControlGeDataPhy(bool enable)
{
    if (enable)
    {
        // enabled, make sure the clocks are turned on first
        bb_top_registers->ge_ctrl.bf.clm_tx_clk_en = 1;
        bb_top_registers->ge_ctrl.bf.ref_clk_en = 1;

        // Data Phy linked is enabled - Rx is brought up first, then Tx,
        // so receive is ready when transmission starts
        bb_top_registers->grm.s.soft_rst_ctrl.bf.ge_clm_rx_rst = 0;
        bb_top_registers->grm.s.soft_rst_ctrl.bf.ge_clm_tx_rst = 0;
    }
    else
    {
        // Data Phy linked is disabled - Tx is shutdown first, then Rx
        // so no data is being sent when the receiver is turned off
        bb_top_registers->grm.s.soft_rst_ctrl.bf.ge_clm_tx_rst = 1;
        bb_top_registers->grm.s.soft_rst_ctrl.bf.ge_clm_rx_rst = 1;

        // disabled, turn off clocks
        bb_top_registers->ge_ctrl.bf.clm_tx_clk_en = 0;
        bb_top_registers->ge_ctrl.bf.ref_clk_en = 0;
    }
}

//#################################################################################################
// GE ASIC vbus_det
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void bb_top_a7_applyGEVbusDetect(bool setVbus)
{
    bb_top_registers->ge_ctrl.bf.vbus_det = setVbus;
}

//#################################################################################################
// resetWatchdogHandler
//  Timer callback of reset watchdog
// Parameters:
// Return:
// Assumptions: It runs callback which was registered when GE reset released
//#################################################################################################
static void resetWatchdogHandler(void)
{
#ifndef BB_ROM
    iassert_TOP_COMPONENT_0(bbTopGeContext.resetWatchdogCallback != NULL, BB_TOP_GE_NULL_RESET_WATCHDOG_CALLBACK);
    bbTopGeContext.resetWatchdogCallback();
#endif    //BB_ROM
}

//#################################################################################################
// GE ASIC vbus_det
//  Timer callback of running watchdog
// Parameters:
// Return:
// Assumptions: It runs callback which was registered when GE reset released
//#################################################################################################
static void runWatchdogHandler(void)
{
#ifndef BB_ROM
    iassert_TOP_COMPONENT_0(bbTopGeContext.runWatchdogCallback != NULL, BB_TOP_GE_NULL_RUN_WATCHDOG_CALLBACK);
    bbTopGeContext.runWatchdogCallback();
#endif    //BB_ROM
}
