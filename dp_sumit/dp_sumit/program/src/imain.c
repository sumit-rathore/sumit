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
// This file contains the main functions for the Blackbird SW.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
// * The imain<N> functions exist to facilitate the continuation of system initialization following
//   an asynchronous initialization step.
//#################################################################################################

// Includes #######################################################################################
#include <bb_top.h>
#include <bb_top_regs.h>
#include <bb_core.h>
#include <uart.h>
#include <gpio.h>
#include <i2c.h>
#include <callback.h>
#include <event.h>
#include <led.h>
#include <ge_program.h>
#include <leon_cpu.h>
#include <sfi.h>
#include <flash_data.h>
#include <command.h>
#include <crc.h>
#include <configuration.h>
#include "toplevel_loc.h"
#include <i2cd_dp130.h>
#include <i2cd_dp159.h>
#include <interrupts.h>

// Constants and Macros ###########################################################################
#define AHB_RAM_BASE_ADDRESS        (0x60000000)
#define GE_CPU_CONTROL_ADDRESS      (0x80000100)
#define GE_CPU_DEBUG_ADDRESS        (0x80000120)
#define GE_CPU_DEBUG_DATA           (0x80000124)
#define GE_CPU_DEBUG_CTRL           (0x80000128)
#define CYPRESS_RESET_TIME_IN_US    (10000)
#define ETH_PHY_POST_RST_TIMEOUT    (30)

#define FW_ALIVE_TIMER_PERIOD_MS    (1000)

// Data Types #####################################################################################


enum NvmBasedInitialization
{
    NVM_USB_OPTION_USB2 = 0x1,
    NVM_USB_OPTION_USB3 = 0x2,
    NVM_DISPLAY_PORT    = 0x4
};


enum Kc705BoardRev
{
    KC705_BOARD_REV_1_0,
    KC705_BOARD_REV_1_1
};


// Static Function Declarations ###################################################################

static void logBuildInfo(void)                                      __attribute__ ((section(".atext")));
static void assertPreHook(void)                                     __attribute__ ((section(".atext")));
static void assertPostHook()                                        __attribute__ ((noreturn));
static void _fwAliveTimerHandler(void)                              __attribute__ ((section(".atext")));
static void Check_HW_BoardInfo(void);

// Global Variables ###############################################################################

extern const uint32_t chip_version;
extern const uint32_t chip_date;
extern const uint32_t chip_time;
extern const uint32_t romRev;

// Static Variables ###############################################################################

static TIMING_TimerHandlerT fwAliveTimer;

// indicate system fell into assert status
static bool underAssert = false;

// Exported Function Definitions ##################################################################

//#################################################################################################
// The main function for the Blackbird firmware.
//
// Parameters:
// Return:
//      A function pointer to the function representing the main loop of the firmware.  In the case
//      of blackbird, this is TASKSCH_MainLoop.
// Assumptions:
//      * This function will be called exactly once on startup.
//#################################################################################################
void* imain(void) __attribute__ ((section(".atext")));
void* imain(void)
{
    // Traps a.k.a Interrupts are disabled in startup.s though the STARTING_PSR_VALUE
    BB_interruptInit();         // Initialize bb_chip_registers address

    bb_top_Init();
    bb_top_disableFpgaWatchdog();

    bb_core_Init();
    LEON_resetIrqs();

    bb_top_nonCpuModuleReset();

    CALLBACK_Init();
    EVENT_Init();

    // Configure the uart
    UART_Init();
    UART_packetizeInit();

#ifdef PLATFORM_A7
//    UART_SetRxHandlerByCh(UART_PORT_GE, &topLevelUartDebug);
    UART_SetRxHandlerByCh(UART_PORT_GE, &_GE_PGM_processRxByte);
#endif
//    UART_printf("PROGRAM BB starting\n");

    // Initialize SFI
    LEON_SFIInit(
        !bb_top_IsFpgaGoldenImage(),
        bb_top_IsASIC());

    LEON_EnableIrq(IRQ_AHB_RESPONSE_ERROR);

    // Initialize assert handling
    iassert_Init(&assertPreHook, &assertPostHook);

    // send out our version info
    logBuildInfo();
    UART_WaitForTx();

    // Setup the timers
#ifdef PLATFORM_K7
    // This insertion results in K7's timerInit to actually work
    // TODO: find out why this happens!!!
#endif
    // setup the Low level timer registers
    iassert_TOPLEVEL_COMPONENT_0(LEON_TimerInit(), TIMER1_ERROR);

    CMD_Init();

    crc16Init();
    crcInit();

    FLASH_init(NULL);

    ICMD_Init();

    bb_top_setupI2c();

    I2C_init(NULL);

    _MdioInit(NULL);
#ifdef PLATFORM_A7
    bb_top_TriStateMdioMdc(false);
#endif

    // enable secondary IRQ
    LEON_EnableIrq(IRQ_SECONDARY_IRQ); // secondary interrupt handler

    ilog_TOPLEVEL_COMPONENT_0(ILOG_MINOR_EVENT, bb_top_IsDeviceLex() ? THIS_IS_LEX : THIS_IS_REX);

    GpioInit();

    LED_init();

    // Raise the log level a bit because the timer handler we start below
    // is a bit spammy. Users interested in temperature readings can reduce
    // the log level via icmd.
    ilog_SetLevel(ILOG_MAJOR_EVENT, TOPLEVEL_COMPONENT);

    // Begin callback code
#ifdef PLATFORM_A7
    GE_PROGRAM_init();
#endif

    // Send I'mAlive InfoMessage
    CMD_sendSoftwareVersion(INFO_MESSAGE_SECONDARY_ID_VERSION_PROGRAM_BB);

    // Start I'mAlive timer
    fwAliveTimer = TIMING_TimerRegisterHandler(
        &_fwAliveTimerHandler,
        true, // true means periodic, so while slightly inconvenient in terms of IRQ, we can more easily re-enable in command component
        FW_ALIVE_TIMER_PERIOD_MS);

    TIMING_TimerStart(fwAliveTimer);

    Check_HW_BoardInfo();   // To detect HW board

    GE_PGM_checkAutoDownload();

    return &MainLoop;
}

//#################################################################################################
// killSystem: Does nothing in program BB. created for removing compile option
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void killSystem()
{
    // ilog_TOPLEVEL_COMPONENT_0(ILOG_MAJOR_ERROR, KILL_SYSTEM);
    // Config_DisableFeature();
    // TIMING_TimerStop(validationTimer);
    // TIMING_TimerStop(unlockedExecutionTimer);
}

// Component Scope Function Definitions ###########################################################

//#################################################################################################
// Print out the current software version as well as the build date and time.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void PrintSwVersion(void)
{
    logBuildInfo();
}

//#################################################################################################
// IsSystemUnderAssert
//
// Parameters:
// Return: True if system is under assert status
// Assumptions:
//#################################################################################################
bool IsSystemUnderAssert(void)
{
    return underAssert;
}



#ifdef PLATFORM_K7
//#################################################################################################
// Print out the current software version as well as the build date and time.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void readFromLMK(uint8_t reg)
{
    ilog_TOPLEVEL_COMPONENT_2(
        ILOG_USER_LOG,
        TOPLEVEL_READ_INREVIUM_LMK_REG,
        reg,
        PLL_lmk04906ReadFromReg(reg));
}
#endif

// Static Function Definitions ####################################################################

//#################################################################################################
// Print out the current software version as well as the build date and time.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void _fwAliveTimerHandler(void)
{
    if (!CMD_receivedProgramStart())
    {
        CMD_sendSoftwareVersion(INFO_MESSAGE_SECONDARY_ID_VERSION_PROGRAM_BB);
        CMD_hardwareInfo(INFO_MESSAGE_SECONDARY_ID_VERSION_HARDWARE);
        TIMING_TimerStart(fwAliveTimer);
    }
}


//#################################################################################################
// Logs the build version & time.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void logBuildInfo(void)
{
   // NOTE: The #defines come from inc/options.h
   ILOG_istatus(
        ISTATUS_BOOTUP_PROGRAMBB_SW_VERSION,
        3, // Number of arguments
        SOFTWARE_MAJOR_REVISION,
        SOFTWARE_MINOR_REVISION,
        SOFTWARE_DEBUG_REVISION);

    ILOG_istatus(
        ISTATUS_BOOTUP_FW_VERSION_SW_BUILD_DATE,
        3, // Number of arguments
        MAKE_BUILD_YEAR,
        MAKE_BUILD_MONTH,
        MAKE_BUILD_DAY);

    ILOG_istatus(
        ISTATUS_BOOTUP_FW_VERSION_SW_BUILD_TIME,
        3, // Number of arguments
        MAKE_BUILD_HOUR,
        MAKE_BUILD_MINUTE,
        MAKE_BUILD_SECOND);

    ILOG_istatus(
        bb_top_IsDeviceLex() ?
        ISTATUS_BOOTUP_FW_VERSION_LEX : ISTATUS_BOOTUP_FW_VERSION_REX,
        0);

    ILOG_istatus(
        ISTATUS_BOOTUP_FW_VERSION_CHIP_ID,
        3, // Number of arguments
        chip_version >> 16,
        (chip_version >> 8) & 0xFF,
        chip_version & 0xFF);

    ILOG_istatus(
        ISTATUS_BOOTUP_FW_VERSION_CHIP_BUILD_DATE,
        3, // Number of arguments
        chip_date >> 16,
        (chip_date >> 8) & 0xFF,
        chip_date & 0xFF);

    ILOG_istatus(
        ISTATUS_BOOTUP_FW_VERSION_CHIP_BUILD_TIME,
        3, // Number of arguments
        (chip_time >> 16) & 0xFF,
        (chip_time >> 8) & 0xFF,
        chip_time & 0xFF);

    // Warn user only if in fallback image
    if (bb_top_IsFpgaGoldenImage())
    {
        ILOG_istatus(
            ISTATUS_BOOTUP_FW_VERSION_FALLBACK_IMAGE,
            0);
    }
}


//#################################################################################################
// This function will be called at the beginning of a failed assert.  The intent is to write an
// output GPIO to enable external equipment such as an oscilloscope to trigger on this event.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void assertPreHook(void)
{
    // TODO: GRG_GpioSet(GPIO_OUT_LED_ASSERT);

    underAssert = true;
}


//#################################################################################################
// Called after an assert fails.  The idea is for this function to call into functions in each
// component that contain useful information to dump on assert.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void assertPostHook(void)
{
    LEON_CPUDisableIRQ();   // make sure interrupts are off (should be anyways, at this point)
#ifdef DEBUG
    CALLBACK_Reinit(); // re-initialize the callback module so we can use it again

    // If an assert happened by UART (e.g. Icmd),
    // Packetizer state stopped in RECEIVE_PACKET_CHECK_EOT with last offset value
    UART_ResetPacketizeRxState();

    while (true)
    {
        UART_InterruptHandlerRx(); // Pull a character out of the BB UART
        callBackTask();            // Act on the packet;
        UART_PollingModeDoWork();  // Transmit characters
    }
#else
    bb_top_systemReset();
    while (true);
#endif // DEBUG
}


//#################################################################################################
// Check HW board to decide correct firmware image
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void Check_HW_BoardInfo(void)
{
    if (bb_top_IsDeviceLex())
    {
        I2CD_dp159Init();
    }
    else
    {
        I2CD_dp130Init();
    }
}







