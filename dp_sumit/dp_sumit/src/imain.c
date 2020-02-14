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
#include <ibase.h>
#include <bb_top.h>
#include <bb_top_ge.h>
#include <bb_top_dp.h>
#include <bb_core.h>
#include <gpio.h>
#include <leon_traps.h>
#include <leon_cpu.h>
#include <mdiod_phy_driver.h>
#include <i2c.h>
#include <i2c_slave.h>
#include <uart.h>
#include <bb_top_regs.h>
#include <bb_chip_regs.h>
#include <xaui.h>
#include <mac.h>
#include <ulp.h>
#include <lan_port.h>
#include <cpu_comm.h>
#include <mca.h>
#include <atmel_crypto.h>
#include <idt_clk.h>
#include <aquantia.h>
#include <callback.h>
#include <event.h>
#include <stats_mon.h>
#include <led.h>
#include <bb_ge_comm.h>
#include <linkmgr.h>
#include <flash_data.h>
#include <sfi.h>
#include <configuration.h>
#include <xadc.h>
#include <command.h>
#include <crc.h>
#include <fiber5g.h>
#include <flash_raw.h>
#include <rs232.h>
#include <sys_funcs.h>
#include <aux_api.h>
#include <test_diagnostics.h>
#include "toplevel_loc.h"
#include "toplevel_cmd.h"
#include <upp.h>
#include <i2cd_dp130.h>
#include <i2cd_dp159.h>
#include <interrupts.h>

#include <uart_regs.h>
#include <module_addresses_regs.h>

void* imain(void) __attribute__ ((section(".atext")));

// Constants and Macros ###########################################################################

#define AHB_RAM_BASE_ADDRESS    (0x60000000)
#define GE_CPU_CONTROL_ADDRESS  (0x80000100)
#define GE_CPU_DEBUG_ADDRESS    (0x80000120)
#define GE_CPU_DEBUG_DATA       (0x80000124)
#define GE_CPU_DEBUG_CTRL       (0x80000128)

// Required FPGA version & build information (date and time)
#define FPGA_MAJOR_VER          0x10           // Required FPGA version 7.0.0
#define FPGA_MINOR_VER          0
#define FPGA_MINMINOR_VER       0
#define FPGA_YEAR               0x2018        // Required FPGA Date 2017/10/20
#define FPGA_MONTH              0x06
#define FPGA_DATE               0x29
#define FPGA_HOUR               0x15          // Required FPGA Time 13:03:20
#define FPGA_MINUTE             0x48
#define FPGA_SEC                0x06

// Offset and mask values to get FPGA version
#define FPGA_MAJOR_VER_OFFSET   (16)            // Fpga major version base 16th bit
#define FPGA_MINOR_VER_OFFSET   (8)             // Fpga minor version base 16th bit
#define FPGA_MINMINOR_VER_OFFSET    (0)         // Fpga minminor version base 16th bit
#define FPGA_VER_MASK           (0xFF)          // Version is 8bit data per major, minor, minminor
#define FPGA_YEAR_OFFSET        (16)            // Fpga build year base 16th bit
#define FPGA_MONTH_OFFSET       (8)             // Fpga build month base 8th bit
#define FPGA_DATE_OFFSET        (0)             // Fpga build date base 0th bit
#define FPGA_DATE_MASK          (0xFF)          // Date is 8bit data per month, date
#define FPGA_HOUR_OFFSET        (16)            // Fpga build Hour base 16th bit
#define FPGA_MIN_OFFSET         (8)             // Fpga build Minute base 8th bit
#define FPGA_SEC_OFFSET         (0)             // Fpga build Second base 0th bit
#define FPGA_TIME_MASK          (0xFF)          // Time is 8bit data per hour, min, sec

#define BB_MEMORY_OVERLAYS  5   // 5 overlay regions: ftext, atext, rodata, srodata, data


// Data Types #####################################################################################
// used to help copy the Lex (or Rex) overlays to IRAM, AHBRAM, or DRAM
struct LexRexOverlay
{
    uint32_t * destinationAddress;
    uint32_t * sourceAddress;
    uint32_t * overlaySize;
};

struct ChipInfo
{
    uint32_t fpgaVersion;           // Fpga version information
    uint32_t fpgaDate;              // Fpga compile date information
    uint32_t fpgaTime;              // Fpga compile time information
};

// Static Function Declarations ###################################################################

static void K7Init_Chips(void)                                                  __attribute__ ((section(".atext")));

static void logBuildInfo(void)                                                  __attribute__ ((section(".atext")));
static void ShowBuildVersionInfo(void)                                          __attribute__ ((section(".atext")));
static void assertPreHook(void)                                                 __attribute__ ((section(".atext")));

static void assertPostHook()                                                    __attribute__ ((noreturn));

#ifdef PLATFORM_A7
// A7 only; there is no Atmel crypto authentication chip on the K7.
// This #ifdef will need to revisited when we introduce new platforms.
// static void atmelInitDone(bool atmelRead, bool dataAndOtpZonesLocked, bool configZoneLocked)    __attribute__ ((section(".atext")));
#endif

static void copyLexRexOverlays(bool isLex)                                      __attribute__((section(".atext")));
static void ATMEL_TimerValidation(enum ATMEL_processState atmelState, uint8_t *featureBuffer) __attribute__((section(".atext")));
static void validationTimerHandler(void)                                        __attribute__((section(".atext")));
static void system_StartUp(void)                                                __attribute__((section(".atext")));
static void CheckFpgaVersion(void)                                              __attribute__((section(".atext")));
static void ATMEL_Validation_init(void)                                         __attribute__((section(".atext")));
static void CRC_secondaryImageCheck(void)                                       __attribute__((section(".atext")));
static bool CRC_validateSecondaryImage(void)                                    __attribute__((section(".atext")));

#ifdef PLUG_TEST
static void setPlugTestLogLevel(void)                                           __attribute__((section(".atext")));
#endif //PLUG_TEST

// Global Variables ###############################################################################

extern const uint32_t chip_version;
extern const uint32_t chip_date;
extern const uint32_t chip_time;
extern uint32_t flash_bin_table[];

// Overlays
// IRAM
extern uint32_t __rex_lex_iram_overlay_start;
extern uint32_t __load_start_rexftext;
extern uint32_t __load_start_lexftext;
extern uint32_t __lex_ftext_size;
extern uint32_t __rex_ftext_size;

extern uint32_t __rex_lex_rodata_overlay_start;
extern uint32_t __load_start_rexrodata;
extern uint32_t __load_start_lexrodata;
extern uint32_t __lex_rodata_size;
extern uint32_t __rex_rodata_size;

// AHBRAM
extern uint32_t __rex_lex_ahbram_overlay_start;
extern uint32_t __load_start_rexstext;
extern uint32_t __load_start_lexstext;
extern uint32_t __lex_stext_size;
extern uint32_t __rex_stext_size;

extern uint32_t __rex_lex_srodata_overlay_start;
extern uint32_t __load_start_rexsrodata;
extern uint32_t __load_start_lexsrodata;

extern uint32_t __lex_srodata_size;
extern uint32_t __rex_srodata_size;

// DRAM
extern uint32_t __rex_lex_data_overlay_start;
extern uint32_t __load_start_rexdata;
extern uint32_t __load_start_lexdata;

extern uint32_t __lex_data_size;
extern uint32_t __rex_data_size;


// Static Variables ###############################################################################

static const struct ChipInfo fpgaInfo =
{
    .fpgaVersion = ((FPGA_MAJOR_VER << FPGA_MAJOR_VER_OFFSET) |
                    (FPGA_MINOR_VER << FPGA_MINOR_VER_OFFSET) |
                    (FPGA_MINMINOR_VER << FPGA_MINMINOR_VER_OFFSET)),
    .fpgaDate = ((FPGA_YEAR << FPGA_YEAR_OFFSET) |
                    (FPGA_MONTH << FPGA_MONTH_OFFSET) |
                    (FPGA_DATE << FPGA_DATE_OFFSET)),
    .fpgaTime = ((FPGA_HOUR << FPGA_HOUR_OFFSET) |
                    (FPGA_MINUTE << FPGA_MIN_OFFSET) |
                    (FPGA_SEC << FPGA_SEC_OFFSET))
};

enum UsbOptions usbOptions;
static TIMING_TimerHandlerT validationTimer;
static TIMING_TimerHandlerT unlockedExecutionTimer;

// this is here to force the linker to not optimize flash_bin_table[] out
static uint32_t const * const keepBinTableVar __attribute__((section(".flashrodata"), used)) = flash_bin_table;

// unlock atmel working time: using variable and shifted value not to be leaked by binary search
static volatile uint32_t const unlockedExecutionTime = ((3 * 60 + 30) * 1000 ) << 3;

// indicate system fell into assert status
static bool underAssert = false;

//indicates if system startup is done
static bool startUpDone = false;

// Count atmel validation error
static uint8_t atmelErrorCnt = 0;

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
//
//#################################################################################################
void* imain(void)
{
    // Traps a.k.a Interrupts are disabled in startup.s though the STARTING_PSR_VALUE
    startUpDone = false;

    BB_interruptInit();         // Initialize bb_chip_registers address

    bb_top_Init();

    bb_core_Init();
    LEON_resetIrqs();           // Diable & Clear all interrupts first

    bb_top_nonCpuModuleReset();

    // don't switch if a fallback was triggered from the current image
    if(bb_top_IsFpgaGoldenImage())
    {
        if(!bb_top_a7_isFpgaFallback())
        {
            if(0x009A55ED == *(uint32_t*)(FPGA_CURRENT_FLASH_START_ADDRESS + 4))
            {
                bb_top_switchFpgaImage(); // jump to the current image
            }
        }
    }

    bb_top_disableFpgaWatchdog();

    const bool isLex = bb_top_IsDeviceLex();
    copyLexRexOverlays(isLex);  // this needs to be done before almost everything else

    CALLBACK_Init();            // setup the callback system
    EVENT_Init();               // initialize the event manager

    // Configure the uart
    UART_Init();
    UART_packetizeInit();       // setup the UART packetize system

    // Initialize assert handling
    iassert_Init(&assertPreHook, &assertPostHook);  // Initialize assert handling

    // setup the Low level timer registers
    iassert_TOPLEVEL_COMPONENT_0(LEON_TimerInit(), TIMER1_ERROR);

#ifdef PLUG_TEST
    setPlugTestLogLevel();
#endif //PLUG_TEST

    ShowBuildVersionInfo();     // show build info and FPGA module version

    Configuration_Init();       // first pass configuration - loads defaults, and pin straps
    
    LEON_EnableIrq(IRQ_AHB_RESPONSE_ERROR);

    GpioInit();

    CheckFpgaVersion();         // check fpga version
    bb_top_a7_writeUserReg(0);      // clear the user register (used for GE programming)
    BBGE_COMM_init();           // Assign the GE packet handlers

    CMD_Init();

    ICMD_Init();

    CMD_sendSoftwareVersion(INFO_MESSAGE_SECONDARY_ID_VERSION_FIRMWARE);

    UART_WaitForTx();   // make sure the software version goes out before we do anything else

    // Initialize SFI
    LEON_SFIInit(
        !bb_top_IsFpgaGoldenImage(),
        bb_top_IsASIC());

    FLASH_init(NULL);
    crcInit();
    crc16Init();

    // system support modules initialization
    STATSMON_Init();        // initialize statistic groups monitoring

    I2C_init(NULL);

    I2C_SlaveInit();
    _MdioInit(NULL);

    Configuration_LoadNVM();  // setup NVM, load variables - this could take some time

    idtConfigurationSetup();            // Set IDK Clock after atmel featurebits load finish

    // enable secondary IRQ
    LEON_EnableIrq(IRQ_SECONDARY_IRQ); // secondary interrupt handler

    LED_init();

    bb_top_drpInit();

    // Raise the log level a bit because the XADC timer we start below
    // is a bit spammy. Users interested in temperature readings can reduce
    // the log level via icmd.
    ilog_SetLevel(ILOG_MAJOR_EVENT, TOPLEVEL_COMPONENT);

    XADC_init();

    if(bb_top_IsFpgaGoldenImage()) // we are in the Golden boot image
    {
        // if the CRC check passes it will load the current FPGA
        CRC_secondaryImageCheck();

        // Warn user only if in fallback image
        ILOG_istatus(ISTATUS_BOOTUP_FW_VERSION_FALLBACK_IMAGE, 0);
    }

#ifdef PLUG_TEST
    ilog_TOPLEVEL_COMPONENT_0(ILOG_MAJOR_ERROR, PLUG_TEST_IMAGE);
#endif //PLUG_TEST

    ATMEL_init();
    ATMEL_Validation_init();

    return &MainLoop;
}

//#################################################################################################
// Check if the FPGA has had a CRC run on it
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once on startup.
//
//#################################################################################################
static void CRC_secondaryImageCheck(void)
{
    uint32_t fpgaImageSize = *(uint32_t*)(FPGA_CURRENT_FLASH_START_ADDRESS); // point to the current image FPGA
    uint32_t passedValue = 0;

    if(fpgaImageSize < FLASH_BIN_TABLE_OFFSET) // sanity check on the image size..... should be under the start of the code block
       passedValue = *(uint32_t*)(FPGA_CURRENT_FLASH_START_ADDRESS + 4);

    if(passedValue == 0xFFFFFFFF) // Do we need to do a check on the current image?
    {
        ILOG_istatus(ISTATUS_BOOTUP_FW_CHECK_CURRENT_IMAGE, 0);

        if(CRC_validateSecondaryImage()) // The crc64 matched
        {
            bb_top_switchFpgaImage();
        }
    }
}

//#################################################################################################
// Check the validity of the current image.
//
// Parameters:
// Return:
// Assumptions:
//      * This function will be called exactly once on startup.
//
//#################################################################################################
static bool CRC_validateSecondaryImage(void)
{
#ifdef PLUG_TEST
    return true;
#else
    uint32_t val;

    LEON_TimerValueT ledBlinkTime;

    uint32_t crcCheckAddress = (FPGA_CURRENT_FLASH_START_ADDRESS + 0x100); // initialize where we want to start the check

    // The size of the FPGA image is stored at FPGA_CURRENT_FLASH_START_ADDRESS
    uint32_t endOfFpgaImage = crcCheckAddress + *(uint32_t *)FPGA_CURRENT_FLASH_START_ADDRESS;
    bool ledOn = true;

    crc64 crc = crcFastInit(); // Initialize the table
    ledBlinkTime = LEON_TimerRead();
    LED_OnOff(LI_IO_STATUS_LED_GREEN, ledOn);

    // Compute the 64 bit CRC
    UART_WaitForTx();
    for(; crcCheckAddress < endOfFpgaImage; crcCheckAddress += SFI_FLASH_SECTOR_SIZE)
    {
        if((crcCheckAddress + SFI_FLASH_SECTOR_SIZE) >  endOfFpgaImage)
        {
            crc = crcFastBlock((uint8_t *)crcCheckAddress, (endOfFpgaImage -crcCheckAddress), crc); // points past the size and CRC
            crcCheckAddress = endOfFpgaImage;
        }
        else
            crc = crcFastBlock((uint8_t *)crcCheckAddress, SFI_FLASH_SECTOR_SIZE, crc); // points past the size and CRC


        if(LEON_TimerCalcUsecDiff(ledBlinkTime, LEON_TimerRead()) > 500000)
        {
            ledBlinkTime = LEON_TimerRead();

            if(ledOn)
            {
                ledOn = false;
                GpioClear(GPIO_CONN_SYS_STATUS_LED0);
            }
            else
            {
               ledOn = true;
               GpioSet(GPIO_CONN_SYS_STATUS_LED0);
            }
        }
    }

    crc = crcFastFinalize(crc); // Do a XOR of the final CRC

    if(crc == *(crc64 *)(FPGA_CURRENT_FLASH_START_ADDRESS + 8)) // if computed and stored CRCs match then jump to the current image
    {
        val = 0x009A55ED; // if the FPGA has already been checked this value will be in the flash at some point passed the end of the configuration image
        FLASHRAW_write((const uint8_t *)(FPGA_CURRENT_FLASH_START_ADDRESS + 4),   (const uint8_t *)&val, sizeof(uint32_t));
        return(true);
    }
    else // The stored and computed CRCs do not match, we don't want to continually check the CRC on each startup
    {
        val = 0xDEADBEEF;
        FLASHRAW_write((const uint8_t *)(FPGA_CURRENT_FLASH_START_ADDRESS + 4),   (const uint8_t *)&val, sizeof(uint32_t));
    }
    return(false);
#endif // PLUG_TEST
}

//#################################################################################################
// killSystem: Disable All features after finding atmel validation Error
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void killSystem()
{
    ilog_TOPLEVEL_COMPONENT_0(ILOG_MAJOR_ERROR, KILL_SYSTEM);

    TIMING_TimerStop(validationTimer);

    Config_DisableFeature();                // Disable all features
    LINKMGR_phyShutdown();                  // Link Down
}


//#################################################################################################
// ATMEL_Validation: Handle ATMEL_checkValidation result
//
// Parameters: ATMEL_processState, featureBits Buffer
// Return:
// Assumptions:
//#################################################################################################
// static void ATMEL_Validation(enum ATMEL_processState atmelState, uint8_t *featureBuffer)
static void ATMEL_Validation_init()
{
    unlockedExecutionTimer = TIMING_TimerRegisterHandler(killSystem, false, unlockedExecutionTime >> 3);
    validationTimer = TIMING_TimerRegisterHandler(validationTimerHandler, true, VALIDATION_TIME_MSECS);

    TIMING_TimerStart(validationTimer);             // start validation periodic timer

    Config_EnableFeature();                         // enable all features for HW testing
    TIMING_TimerStart(unlockedExecutionTimer);      // start kill timer. it will stop if Atmel verification is successful
    //system_StartUp(); Sounak:Moved after ATMEL validation check to avoid I2C conflict

    ATMEL_checkValidation(ATMEL_TimerValidation);
}

//#################################################################################################
// system_StartUp: All feature initialization start
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void system_StartUp()
{
    K7Init_Chips();

    LINKMGR_init(); // setup the Phy layer
    MAC_Init();     // the MAC layer
    MCA_Init();     // and the MCA

    if(!bb_top_IsFpgaGoldenImage())
    {
        #ifndef PLUG_TEST
        UPP_Init();
        #endif //PLUG_TEST
#if !defined BB_ISO && !defined BB_USB
        AUX_Init();        // startup DP and Aux, if enabled
#endif
        #ifndef PLUG_TEST
        LANPORT_LanPortInit();
        RS232_Init();
        #endif //PLUG_TEST
        LINKMGR_phyEnable(true);    // bring up the Lex/Rex link, start normal operations

        // Reset GE and check GE version & CRC
        // Be careful this location. If we miss the GE's first message, it could keep download GE again & again
        bb_core_setProgramBbOperation(0);
        BBGE_RunGEVerify();
    }

    //Initialize dp_159 and dp_130 even if dp is not enabled, so that excom can find out these chips
    //are not on board in case of raven: Manvir
    if (bb_top_IsDeviceLex())
    {
        I2CD_dp159Init();
    }
    else
    {
        I2CD_dp130Init();
    }
    TEST_DiagnosticInit();
}
//#################################################################################################
// validationTimerHandler: Periodic check ATMEL
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void validationTimerHandler()
{
    ATMEL_checkValidation(ATMEL_TimerValidation);
}


//#################################################################################################
// ATMEL_TimerValidation: Handle ATMEL_checkValidation result
//
// Parameters: ATMEL_processState, featureBits Buffer
// Return:
// Assumptions:
//#################################################################################################
static void ATMEL_TimerValidation(enum ATMEL_processState atmelState, uint8_t *featureBuffer)
{
    bool atmelError = false;

    //Run startup sequence only once whatever state the Atmel Chip is in
    if (!startUpDone)
    {
        system_StartUp();
        startUpDone = true;
    }

    switch(atmelState)
    {
        case ATMEL_NOT_PROGRAMMED:
            ilog_TOPLEVEL_COMPONENT_1(ILOG_MAJOR_ERROR, ATMEL_NOT_PROGRAM, atmelState);
            atmelError = !TIMING_TimerEnabled(unlockedExecutionTimer);      // Error if not programmed Atmel and kill timer is off
            break;

        case ATMEL_FEATURE_READ:
            if(IsFeatureEnableNotChanged(featureBuffer))
            {
                // If newly read featureEnable is the same with previous value, keep working.
                // Also, Check if pre-programmed Atmel before Badger configuration is on board
                if(featureBuffer[0] | featureBuffer[1] | featureBuffer[2] | featureBuffer[3])
                {
                    atmelErrorCnt = 0;
                    TIMING_TimerStop(unlockedExecutionTimer);
                }
                else
                {
                    ilog_TOPLEVEL_COMPONENT_1(ILOG_MAJOR_ERROR, ATMEL_NOT_PROGRAM, atmelState);
                }
                break;
            }
            else if(Config_IsDefaultFeatureEnable())
            {
                // Feature enable has changed but it has initial value, update it
                Config_UpdateFeatures(featureBuffer);
                break;
            }
            // fall through for error case
        case ATMEL_NO_EXIST:
        case ATMEL_AUTH_FAIL:
        case ATMEL_FEATURE_FAIL:
        case ATMEL_LOCKED:                              // ATMEL_LOCKED is transition status, can't come to the result
        case ATMEL_AUTH_PASS:                           // ATMEL_AUTH_PASS is transition status, can't come to the result
        default:
            atmelError = true;
            break;
    }

    if(atmelError)
    {
        if(atmelErrorCnt >= VALIDATION_MAX_ERR)
        {
            killSystem();                               // link-down, stop Atmel validation
        }
        else
        {
            atmelErrorCnt++;
            ilog_TOPLEVEL_COMPONENT_2(ILOG_MAJOR_ERROR, ATMEL_VALIDATION_FAIL, atmelState, atmelErrorCnt);
        }
    }
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
    // Warn user only if in fallback image
    if (bb_top_IsFpgaGoldenImage())
    {
        ILOG_istatus(
            ISTATUS_BOOTUP_FW_VERSION_FALLBACK_IMAGE,
            0);
    }


}

//#################################################################################################
// Print out the current software version and the unit type.
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void Badger_getDeviceInfo(void)
{
    // Test information is optional data in Badger point of view. Because Raven doesn't support it
    // It should be located on top before sending the other messages
    TEST_ReadFlashProtectIcmd();
    TEST_GetTestStatusFlashVariableIcmd();

    ilog_TOPLEVEL_COMPONENT_3(ILOG_MAJOR_EVENT, SW_VERSION,
                              SOFTWARE_MAJOR_REVISION,
                              SOFTWARE_MINOR_REVISION,
                              SOFTWARE_DEBUG_REVISION);

    ilog_TOPLEVEL_COMPONENT_3(ILOG_MAJOR_EVENT, CHIP_REV,
                              chip_version >> FPGA_MAJOR_VER_OFFSET,
                              (chip_version >> FPGA_MINOR_VER_OFFSET) & FPGA_VER_MASK,
                              chip_version & FPGA_VER_MASK);
    ilog_TOPLEVEL_COMPONENT_1(ILOG_MAJOR_EVENT, UNIT_TYPE, bb_top_IsDeviceLex());
    ilog_TOPLEVEL_COMPONENT_1(ILOG_MAJOR_EVENT, PLATFORM_ID, 1);
    ilog_TOPLEVEL_COMPONENT_1(ILOG_MAJOR_EVENT, VARIANT_ID, 1);


    uint32_t returnValue[9];

    returnValue[0] = SOFTWARE_MAJOR_REVISION;
    returnValue[1] = SOFTWARE_MINOR_REVISION;
    returnValue[2] = SOFTWARE_DEBUG_REVISION;
    returnValue[3] = chip_version >> FPGA_MAJOR_VER_OFFSET;
    returnValue[4] = (chip_version >> FPGA_MINOR_VER_OFFSET) & FPGA_VER_MASK;
    returnValue[5] = chip_version & FPGA_VER_MASK;
    returnValue[6] = bb_top_IsDeviceLex();
    returnValue[7] = CMD_GetPlatformId();   // Platform ID
    returnValue[8] = 1;                     // Variant ID

    // send the values back to the requester
    UART_packetizeSendResponseImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_ICMD,
        ICMD_GetResponseID(),
        &returnValue,
        sizeof(returnValue));
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

static void K7Init_Chips(void)
{
#ifdef PLATFORM_K7
    bb_top_ControlGeDataPhy(false);
    bb_top_ApplyResetPhyGtx(false);
    const bool isLex = bb_top_IsDeviceLex();

    EEPROM_Init();
    I2CD_gpioExpInit();

    // I2CD_gpioExpInit sets USB3_RST high to ensure USB3 PHY receives reset
    // We must now clear that reset to enable its usage
    // TODO Put this into USB3 driver/module
    // Need to ensure 1us reset for GPIO1 RST
    LEON_TimerWaitMicroSec(2);
    I2CD_gpioExpClearPinBlocking(GPIO_GPIO1_RST); // NOTE: logic is reversed in PCB

    if (isLex)
    {
       I2CD_gpioExpSetPinBlocking(GPIO_REXN_LEX);
    }

    // main clock - similar to IDT for ARTIX
    bb_top_ApplyResetDejitterChip(false);
    I2CD_setupDeJitterChip(BBTop_si5326IrqDoneHandler);

    // enable deJitter interrupt
    bb_top_ApplyEnableDejitterInterrupt(true);;

#endif
}

//#################################################################################################
// send out SW version info & FPGA modules' version
//
// Parameters:
//
// Return:
// Assumptions:
//#################################################################################################
static void ShowBuildVersionInfo(void)
{
#ifndef PLUG_TEST
    logBuildInfo();
    BBCORE_printHWModuleVersion();
    UART_WaitForTx();
#endif //PLUG_TEST
}

//#################################################################################################
// Logs the build version & time.
//
// Parameters:
//
// Return:
// Assumptions:
//#################################################################################################
static void logBuildInfo(void)
{
    // NOTE: The #defines come from inc/options.h
   ILOG_istatus(
        ISTATUS_BOOTUP_FW_VERSION_SW_VERSION,
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
        chip_version >> FPGA_MAJOR_VER_OFFSET,
        (chip_version >> FPGA_MINOR_VER_OFFSET) & FPGA_VER_MASK,
        chip_version & FPGA_VER_MASK);

    ILOG_istatus(
        ISTATUS_BOOTUP_FW_VERSION_CHIP_BUILD_DATE,
        3, // Number of arguments
        chip_date >> FPGA_YEAR_OFFSET,
        (chip_date >> FPGA_MONTH_OFFSET) & FPGA_DATE_MASK,
        chip_date & FPGA_DATE_MASK);

    ILOG_istatus(
        ISTATUS_BOOTUP_FW_VERSION_CHIP_BUILD_TIME,
        3, // Number of arguments
        (chip_time >> FPGA_HOUR_OFFSET) & FPGA_TIME_MASK,
        (chip_time >> FPGA_MIN_OFFSET) & FPGA_TIME_MASK,
        chip_time & FPGA_TIME_MASK);

    ilog_TOPLEVEL_COMPONENT_1(
        ILOG_MINOR_EVENT,
        TOPLEVEL_BOARD_INFO,
        bb_top_getCoreBoardRev()+1 );               // 0: A01, 1: A02, 2: A03
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
// CheckFpgaVersion
//  check supported fpga version, date, time and assert if it's not matching
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void CheckFpgaVersion(void)
{
    if((fpgaInfo.fpgaVersion != chip_version)
        || (fpgaInfo.fpgaDate != chip_date)
        || (fpgaInfo.fpgaTime != chip_time))
    {
        ilog_TOPLEVEL_COMPONENT_0(ILOG_MAJOR_ERROR, HW_SW_VER_MISMATCH);
#ifndef DEBUG       // Debug version doesn't care FPGA date and time
        LINKMGR_phyShutdown();
#endif
    }
}

static void copyLexRexOverlays(bool isLex)
{
    const struct LexRexOverlay lexOverlays[BB_MEMORY_OVERLAYS] =
    {
        {&__rex_lex_iram_overlay_start,   &__load_start_lexftext,  &__lex_ftext_size},  // ftext, atext (IRAM)
        {&__rex_lex_ahbram_overlay_start, &__load_start_lexstext,  &__lex_stext_size},  // stext        (AHBRAM)
        {&__rex_lex_rodata_overlay_start, &__load_start_lexrodata, &__lex_rodata_size}, // rodata       (Iram)
        {&__rex_lex_srodata_overlay_start,&__load_start_lexsrodata,&__lex_srodata_size},// srodata      (AhbRam)
        {&__rex_lex_data_overlay_start,   &__load_start_lexdata,   &__lex_data_size},   // data         (Dram)
    };

    const struct LexRexOverlay rexOverlays[BB_MEMORY_OVERLAYS] =
    {
        {&__rex_lex_iram_overlay_start,   &__load_start_rexftext,  &__rex_ftext_size},  // ftext, atext (IRAM)
        {&__rex_lex_ahbram_overlay_start, &__load_start_rexstext,  &__rex_stext_size},  // stext        (AHBRAM)
        {&__rex_lex_rodata_overlay_start, &__load_start_rexrodata, &__rex_rodata_size}, // rodata       (Iram)
        {&__rex_lex_srodata_overlay_start,&__load_start_rexsrodata,&__rex_srodata_size},// srodata      (AhbRam)
        {&__rex_lex_data_overlay_start,   &__load_start_rexdata,   &__rex_data_size},   // data         (Dram)
    };

    struct LexRexOverlay const * const overlayPtr = isLex ? lexOverlays : rexOverlays;

    for (uint32_t i = 0; i < BB_MEMORY_OVERLAYS; i++)
    {
        memcpy(
            overlayPtr[i].destinationAddress,
            overlayPtr[i].sourceAddress,
            (uint32_t)(overlayPtr[i].overlaySize));
    }
}

#ifdef DEBUG
void TOPLEVEL_DEBUG_ASSERT(void) __attribute__((noreturn));
void TOPLEVEL_DEBUG_ASSERT(void)
{
    ifail_TOPLEVEL_COMPONENT_0(DEBUG_ASSERT_BB);
    // Add dummy loop so compiler doesn't complain about noreturn attribute
    while (true)
    ;
}
#endif

#ifdef PLUG_TEST
//#################################################################################################
// Set log level of all non-dp components to fatal_error for plug test
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void setPlugTestLogLevel(void)
{
    for (uint8_t component = TOPLEVEL_COMPONENT; component < NUMBER_OF_ICOMPONENTS; component++)
    {
        if ((component != DP_COMPONENT) &&
            (component != DP_AUX_COMPONENT) &&
            (component != DP_STREAM_COMPONENT) &&
            (component != FLASH_DATA_COMPONENT))
            {
                ilog_SetLevel(ILOG_FATAL_ERROR, component);
            }
    }
}
#endif //PLUG_TEST
