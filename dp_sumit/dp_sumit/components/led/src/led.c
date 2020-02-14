///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2016
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
//!   @file  -  led.c
//
//!   @brief -  Contains the driver for accesing the LEDs
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################
#include <ibase.h>
#include <itypes.h>
#include <gpio.h>   // Exposed header for gpio function definitions
#include <bb_core.h>
#include <led.h>
#include "led_log.h"
#include <event.h>
#include <timing_timers.h>
#include <uart.h>

// Constants and Macros ###########################################################################
#define TIMER_250MS_MASK (1 << 0)  // check bit 0 of LED_OPERATION_TIMER(125ms counter) for 4Hz blinking
#define TIMER_500MS_MASK (1 << 1)  // check bit 1 of LED_OPERATION_TIMER(125ms counter) for 2Hz blinking
#define TIMER_1000MS_MASK (1 << 2) // check bit 2 of LED_OPERATION_TIMER(125ms counter) for 1Hz blinking

#define LED_INIT_PULSE_TIME         (3500)
#define LED_OPERATION_TIMER         (125)       // minimum led tick

#define LED_OFF_ALL                 (0)
#define BOOTING_PERIOD              (4)         // 4 x 125ms
#define TEMP_WARN_PERIOD            (8)         // 8 x 125ms
#define TEMP_FAULT_PERIOD           (1)         // 1 x 125ms
#define DOWNLOAD_PERIOD             (1)         // 1 x 125ms
#define VERIFY_FAULT_PERIOD         (8)         // 8 x 125ms
#define LINK_FAIL_PERIOD            (8)         // 8 x 125ms

#define CREATE_A_PATTERN_STEP(tick, pin) {.pattern = (pin), .periodTick = (tick)}
#define CREATE_PATTERNS(stepsArray) {.steps = (stepsArray), .size = ARRAYSIZE(stepsArray)}

#define GPIO_MAP(led)               (1 << led)
#define STATE_MAP_BIT(state)        (1 << state)

// Data Types #####################################################################################
typedef uint8_t Pattern;
typedef uint16_t LedMode;

typedef struct
{
    const Pattern pattern;                      // one pattern
    const uint8_t periodTick;                   // defines pattern display period (x LED_OPERATION_TIMER)
} PatternStep;

typedef struct
{
    const PatternStep *steps;                   // array pointer
    const uint8_t size;                         // size of step array
} LedPattern;


// Static Function Declarations ###################################################################
static void LED_TimerHandler(void)                                                      __attribute__((section(".atext")));
static void LED_PatternDisplay(LedPattern ledPattern)                                   __attribute__((section(".atext")));
static void LED_Operation(void)                                                         __attribute__((section(".atext")));
static void LED_SystemControl(void)                                                     __attribute__((section(".atext")));
static void LED_Usb2Control(void)                                                       __attribute__((section(".atext")));
static void LED_Usb3Control(void)                                                       __attribute__((section(".atext")));
static void LED_VideoControl(void)                                                      __attribute__((section(".atext")));
static void LED_LinkControl(void)                                                       __attribute__((section(".atext")));
static enum LedState LED_GetCurrentState(void)                                          __attribute__((section(".atext")));
static void LED_BootingFinish(void)                                                     __attribute__((section(".atext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static const PatternStep bootPattern[] =
{
    CREATE_A_PATTERN_STEP(BOOTING_PERIOD, GPIO_MAP(LI_IO_STATUS_LED_GREEN)),
    CREATE_A_PATTERN_STEP(BOOTING_PERIOD, LED_OFF_ALL),
};

static const PatternStep tempWarnPattern[] =
{
    CREATE_A_PATTERN_STEP(TEMP_WARN_PERIOD, (
                    GPIO_MAP(LI_IO_STATUS_LED_GREEN) |
                    GPIO_MAP(LI_IO_LINK_LED_GREEN) |
                    GPIO_MAP(LI_IO_USB2_LED_GREEN) |
                    GPIO_MAP(LI_IO_USB3_LED) |
                    GPIO_MAP(LI_IO_VIDEO_LED_GREEN))),
    CREATE_A_PATTERN_STEP(TEMP_WARN_PERIOD, LED_OFF_ALL),
};

static const PatternStep tempFaultPattern[] =
{
    CREATE_A_PATTERN_STEP(TEMP_FAULT_PERIOD, (
                    GPIO_MAP(LI_IO_STATUS_LED_GREEN) |
                    GPIO_MAP(LI_IO_LINK_LED_GREEN) |
                    GPIO_MAP(LI_IO_USB2_LED_GREEN) |
                    GPIO_MAP(LI_IO_USB3_LED) |
                    GPIO_MAP(LI_IO_VIDEO_LED_GREEN))),
    CREATE_A_PATTERN_STEP(TEMP_FAULT_PERIOD, LED_OFF_ALL),
};

static const PatternStep downloadPattern[] =
{
    CREATE_A_PATTERN_STEP(DOWNLOAD_PERIOD, GPIO_MAP(LI_IO_STATUS_LED_GREEN)),
    CREATE_A_PATTERN_STEP(DOWNLOAD_PERIOD, GPIO_MAP(LI_IO_LINK_LED_GREEN)),
    CREATE_A_PATTERN_STEP(DOWNLOAD_PERIOD, GPIO_MAP(LI_IO_USB2_LED_GREEN)),
    CREATE_A_PATTERN_STEP(DOWNLOAD_PERIOD, GPIO_MAP(LI_IO_USB3_LED)),
};

static const PatternStep downloadPatternDP[] =
{
    CREATE_A_PATTERN_STEP(DOWNLOAD_PERIOD, GPIO_MAP(LI_IO_STATUS_LED_GREEN)),
    CREATE_A_PATTERN_STEP(DOWNLOAD_PERIOD, GPIO_MAP(LI_IO_LINK_LED_GREEN)),
    CREATE_A_PATTERN_STEP(DOWNLOAD_PERIOD, GPIO_MAP(LI_IO_VIDEO_LED_GREEN)),
    CREATE_A_PATTERN_STEP(DOWNLOAD_PERIOD, GPIO_MAP(LI_IO_USB2_LED_GREEN)),
    CREATE_A_PATTERN_STEP(DOWNLOAD_PERIOD, GPIO_MAP(LI_IO_USB3_LED)),
};

static const PatternStep veriFaultPattern[] =
{
    CREATE_A_PATTERN_STEP(VERIFY_FAULT_PERIOD, (GPIO_MAP(LI_IO_STATUS_LED_GREEN) | GPIO_MAP(LI_IO_USB2_LED_GREEN))),
    CREATE_A_PATTERN_STEP(VERIFY_FAULT_PERIOD, (GPIO_MAP(LI_IO_LINK_LED_GREEN) | GPIO_MAP(LI_IO_USB3_LED))),
};

static const PatternStep veriFaultPatternDP[] =
{
    CREATE_A_PATTERN_STEP(VERIFY_FAULT_PERIOD, (GPIO_MAP(LI_IO_STATUS_LED_GREEN) | GPIO_MAP(LI_IO_VIDEO_LED_GREEN) | GPIO_MAP(LI_IO_USB3_LED))),
    CREATE_A_PATTERN_STEP(VERIFY_FAULT_PERIOD, (GPIO_MAP(LI_IO_LINK_LED_GREEN) | GPIO_MAP(LI_IO_USB2_LED_GREEN))),
};

static const LedPattern ledPatternBooting = CREATE_PATTERNS(bootPattern);
static const LedPattern ledPatternTempWarn = CREATE_PATTERNS(tempWarnPattern);
static const LedPattern ledPatternTempFault = CREATE_PATTERNS(tempFaultPattern);
static const LedPattern ledPatternDownload = CREATE_PATTERNS(downloadPattern);
static const LedPattern ledPatternDownloadDP = CREATE_PATTERNS(downloadPatternDP);
static const LedPattern ledPatternVeriFault = CREATE_PATTERNS(veriFaultPattern);
static const LedPattern ledPatternVeriFaultDP = CREATE_PATTERNS(veriFaultPatternDP);

static struct
{
    uint16_t timerCounter;              // increase timer to measure spending time (count x LED_OPERATION_TIMER)
    uint16_t patternIndex;              // indicating pattern step

    TIMING_TimerHandlerT ledTimer;
    TIMING_TimerHandlerT bootingTimer;  // control booting led mode

    bool led250ms;  // indicate 250ms toggle
    bool led500ms;  // indicate 500ms toggle
    bool led1000ms; // indicate 1000ms toggle

    LedMode mode;                       // combination bits of led display mode
    Pattern ledPattern;                 // led pattern for operation mode
} ledContext;

// Exported Function Definitions ##################################################################
//#################################################################################################
// Register LED's for events.
//
// Parameters:
// Return:
// Assumptions:
//      * This function is expected to be called exactly once during system initialization.
//#################################################################################################
void LED_init(void)
{
    ledContext.ledTimer = TIMING_TimerRegisterHandler(LED_TimerHandler, true, LED_OPERATION_TIMER);
    ledContext.bootingTimer = TIMING_TimerRegisterHandler(LED_BootingFinish, false, LED_INIT_PULSE_TIME);

    LED_SetLedState(LS_BOOTING, true);                  // set Default state booting & operation
    LED_SetLedState(LS_OPERATION, true);

    TIMING_TimerStart(ledContext.ledTimer);             // start run LED control timer

#ifdef BB_PROGRAM_BB
    if(!(bb_core_getProgramBbOperation() && SPR_PROGRAM_BB_SET_FOR_GE_DOWNLOAD))                  // check programBB run by automatic GE download
    {
        LED_SetLedState(LS_DOWNLOAD, true);
    }
#else
    TIMING_TimerStart(ledContext.bootingTimer);
#endif
}


//#################################################################################################
// LED_SetLedState
//
// Parameters: LED state to be updated, enable or disable the state
// Return:
// Assumptions:
//#################################################################################################
void LED_SetLedState(enum LedState state, bool enable)
{
    enum LedState current = LED_GetCurrentState();

    if(enable)
    {
        ledContext.mode |= STATE_MAP_BIT(state);
    }
    else
    {
        ledContext.mode &= ~STATE_MAP_BIT(state);
    }

    if(((current > state) && enable) ||         // higher than current prioiry state is enabled or
        ((current == state) && !enable))        // current state is disabled
    {
        ilog_LED_COMPONENT_1(ILOG_MINOR_EVENT, LED_SET_MODE, ledContext.mode);

        ledContext.timerCounter = 0;
        ledContext.patternIndex = 0;
        ledContext.ledPattern = LED_OFF_ALL;
    }
}


//#################################################################################################
// LED_OnOff
//  It modifies LED pattern. LED pattern will be displayed when it's on LS_OPERATION mode or override is set
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void LED_OnOff(enum LedId ledName, bool ledOn)
{
    if(ledOn)
    {
        ledContext.ledPattern |= GPIO_MAP(ledName);
    }
    else
    {
        ledContext.ledPattern &= ~(GPIO_MAP(ledName));
    }
}


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################
//#################################################################################################
// LED_TimerHandler
//
// Parameters: Manage LED display depends on led status
// Return:
// Assumptions: runs every LED_OPERATION_TIMER period
//              LED_OPERATION_TIMER is 125ms
//#################################################################################################
static void LED_TimerHandler()
{
    // Check dp board with FPGA feature
    bool dpBoard = (bb_core_getFeatures() & (CORE_FEATURE_DP_SINK | CORE_FEATURE_DP_SOURCE));

    ledContext.timerCounter++;
    ledContext.led250ms = ledContext.timerCounter & TIMER_250MS_MASK;   // Assume LED_OPERATION_TIMER is 125ms
    ledContext.led500ms = ledContext.timerCounter & TIMER_500MS_MASK;   // Assume LED_OPERATION_TIMER is 125ms
    ledContext.led1000ms = ledContext.timerCounter & TIMER_1000MS_MASK; // Assume LED_OPERATION_TIMER is 125ms

    switch(LED_GetCurrentState())
    {
        case LS_USER:
            GpioSetMultipleLeds(ledContext.ledPattern);
            break;
        case LS_BOOTING:                            // Show Booting LED until LED_INIT_PULSE_TIME
            LED_PatternDisplay(ledPatternBooting);
            break;
        case LS_OPERATION:
            LED_Operation();
            break;
        case LS_DOWNLOAD:                           // BB Download case
            LED_PatternDisplay(dpBoard ? ledPatternDownloadDP : ledPatternDownload);
            break;
        case LS_TEMP_WARN_FPGA:                     // Temperature Warning (FPGA). Same with DP or non-DP
        case LS_TEMP_WARN_AQUANTIA:                 // Temperature Warning (Aquantia). Same with DP or non-DP
            LED_PatternDisplay(ledPatternTempWarn);
            break;
        case LS_TEMP_FAULT:                         // Temperature Fault (FPGA, Aquantia). Same with DP or non-DP
            LED_PatternDisplay(ledPatternTempFault);
            break;
        case LS_VERI_FAULT:                         // Atmel verification fault case
            LED_PatternDisplay(dpBoard ? ledPatternVeriFaultDP : ledPatternVeriFault);
            break;
        case LS_NUM_STATE:
        default:
            ilog_LED_COMPONENT_1(ILOG_MAJOR_ERROR, LED_INVALID_MODE, LED_GetCurrentState());
            break;
    }

}


//#################################################################################################
// LED_Operation
//  Normal status LED control routine
// Parameters:
// Return:
// Assumptions: runs every LED_OPERATION_TIMER period
//#################################################################################################
static void LED_Operation()
{
    ledContext.ledPattern = LED_OFF_ALL;

    LED_SystemControl();                    // LI_IO_STATUS_LED_GREEN
    LED_LinkControl();                      // LI_IO_LINK_LED_GREEN
    LED_Usb2Control();                      // LI_IO_USB2_LED_GREEN
    LED_Usb3Control();                      // LI_IO_USB3_LED
    LED_VideoControl();                     // LI_IO_VIDEO_LED_GREEN

    GpioSetMultipleLeds(ledContext.ledPattern);
}


//#################################################################################################
// LED_SystemControl
//  System LED controller
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LED_SystemControl()
{
    bool ledOn = false;                     // Default: LED off
    bool error = ((EVENT_GetEventInfo(ET_USB2_STATUS_CHANGE) == USB2_ERROR) ||
                (EVENT_GetEventInfo(ET_USB3_STATUS_CHANGE) == USB3_ERROR) ||
                (EVENT_GetEventInfo(ET_VIDEO_STATUS_CHANGE) == VIDEO_ERROR)) ||
                (EVENT_GetEventInfo(ET_PHYLINK_STATUS_CHANGE) == LINK_ERROR);

    if(!error || ledContext.led1000ms)
    {
        ledOn = true;
    }
    LED_OnOff(LI_IO_STATUS_LED_GREEN, ledOn);
}


//#################################################################################################
// LED_LinkControl
//  Link LED controller
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LED_LinkControl()
{
    bool ledOn = false;                     // Default: LED off

    switch(EVENT_GetEventInfo(ET_PHYLINK_STATUS_CHANGE))
    {
        case LINK_OPERATING:
            // only indicate the link is up if the Phy link AND the comlink are up
            if(EVENT_GetEventInfo(ET_COMLINK_STATUS_CHANGE))
            {
                ledOn = true;
            }
            break;

        case LINK_ERROR:    // link and status LED are blinking
            if(ledContext.led1000ms)
            {
                ledOn = true;
            }
            break;

        case LINK_IN_RESET: // all other cases the link LED is off
        default:
            break;
    }

    LED_OnOff(LI_IO_LINK_LED_GREEN, ledOn); // update the LED status
}


//#################################################################################################
// LED_Usb2Control
//  Usb2 LED controller
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LED_Usb2Control()
{
    bool ledOn = false;                     // Default: LED off

    switch (EVENT_GetEventInfo(ET_USB2_STATUS_CHANGE))
    {
        case USB2_IN_RESET:
        case USB2_DISCONNECTED:
            break;

        case USB2_BUS_RESETTING:
        case USB2_OPERATING:
        case USB2_SUSPENDING:
        case USB2_RESUMING:
            ledOn = true;
            break;

        case USB2_SUSPENDED:
        case USB2_ERROR:
            if(ledContext.led1000ms)
            {
                ledOn = true;
            }
            break;

        default:
            break;
    }
    LED_OnOff(LI_IO_USB2_LED_GREEN, ledOn);
}


//#################################################################################################
// LED_Usb3Control()
//  USB3 LED controller
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LED_Usb3Control()
{
    bool ledOn = false;                     // Default: LED off
    switch (EVENT_GetEventInfo(ET_USB3_STATUS_CHANGE))
    {
        case USB3_IN_RESET:
            break;

        case USB3_U0:
        case USB3_U1:
        case USB3_U2:
            ledOn = true;
            break;

        case USB3_ERROR:
        case USB3_U3:
            if(ledContext.led1000ms)
            {
                ledOn = true;
            }
            break;

        default:
            break;
    }
    LED_OnOff(LI_IO_USB3_LED, ledOn);
}


//#################################################################################################
// LED_VideoControl
//  Video LED controller
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LED_VideoControl()
{
    bool ledOn = false;                     // Default: LED off
    switch (EVENT_GetEventInfo(ET_VIDEO_STATUS_CHANGE))
    {
    case VIDEO_IN_RESET:
        break;

    case VIDEO_OPERATING:
        ledOn = true;
        break;

    case VIDEO_TRAINING_UP:
        // if (ledContext.led250ms)
        // {
        //     ledOn = true;
        // }
        break;
    case VIDEO_ERROR:
        if (ledContext.led1000ms)
        {
            ledOn = true;
        }
        break;

    default:
        break;
    }
    LED_OnOff(LI_IO_VIDEO_LED_GREEN, ledOn);
}


//#################################################################################################
// LED_PatternDisplay
//
// Parameters: Get LED pattern and display it
// Return:
// Assumptions:
//#################################################################################################
static void LED_PatternDisplay(const LedPattern ledPattern)
{
    if(ledContext.patternIndex >= ledPattern.size)
    {
        ledContext.patternIndex = 0;
    }

    uint8_t stepTick = ledPattern.steps[ledContext.patternIndex].periodTick;

    if(ledContext.timerCounter >= stepTick)
    {
        GpioSetMultipleLeds(ledPattern.steps[ledContext.patternIndex].pattern);

        ledContext.timerCounter = 0;
        ledContext.patternIndex++;          // increase index after displaying it
    }
}


//#################################################################################################
// LED_GetCurrentState
//
// Parameters:
// Return: Highest led mode which is currently running (could return LS_NUM_STATE)
// Assumptions:
//#################################################################################################
static enum LedState LED_GetCurrentState(void)
{
    enum LedState currentMode = LS_HIGHEST_PRIORITY_MODE;
    while(((ledContext.mode & STATE_MAP_BIT(currentMode)) == 0)
        && (currentMode != LS_NUM_STATE))
    {
        currentMode++;
    }

    return currentMode;
}


//#################################################################################################
// LED_Booting timer Handler
//  Stop displaying led booting indication
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void LED_BootingFinish(void)
{
    LED_SetLedState(LS_BOOTING, false);
}
