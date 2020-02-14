///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - linkmgr_button_pairing.c
//
//!   @brief - Handles the small push-button on the back of the devices that is used for 3
//             different purposes.  When pressed and held at startup or shortly after, the device
//             will enable DHCP mode and then reset.  When the unit is running normally, if the
//             button is held for a long duration, all pairings will be removed.  When the unit is
//             running normally and the button is pressed and released prior to the previously
//             mentioned unpair-all timeout, the unit will enter a pairing mode where it searches
//             for any other unit on the network also in pairing mode and establishes a pair with
//             it.
//
//    State Transitions:
//
//                 (1)
//      STARTUP -------> HOLD_FOR_DHCP      (1) During the startup time, the button is pressed for
//         |                   |                longer than the debounce time.
//      (2)|                   |(7)         (2) The startup timer expires and we enter normal
//         |                   |                execution.
//         +---> EXECUTING <---+            (3) The button is pressed for longer then the debounce
//                ^  |  ^                       time.
//      (6)+------+  |  |                   (4) The button is released and we begin trying to pair.
//         |         |  |                   (5) Pairing succeeded or pairing timeout exceeded.
// PAIRING_BEGIN <---+  |(5)                (6) The button is held long enough to initiate an
//        |       (3)   |                       unpair of all extenders.
//     (4)|             |                   (7) Either the button was held long enough and we
//        +------> PAIRING                      enable DHCP or the button was not held long enough
//                                              and we return to normal execution.
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "linkmgr_loc.h"
#include <storage_Data.h>

/************************ Defined Constants and Macros ***********************/
#define LINKMGR_STARTUP_TIMEOUT              5000
#define LINKMGR_BUTTON_DEBOUNCE_TIMEOUT      10
#define LINKMGR_USE_DHCP_HOLD_TIMEOUT        5000
#define LINKMGR_TEST_HOLD_TIMEOUT            3000
#define LINKMGR_RESET_PAIRING_HOLD_TIMEOUT   10000
#define LINKMGR_PUSH_BUTTON_PAIRING_TIMEOUT  (10 * 60 * 1000) // 10 minutes

/******************************** Data Types *********************************/
enum LINKMGR_PushButtonState
{
    // NOTE: The values are selected to help optimize switch statements
    LINKMGR_STATE_STARTUP       = 0, // initial state on power up
    LINKMGR_STATE_HOLD_FOR_DHCP = 3, // if the button is held long enough, we will enable DHCP
    LINKMGR_STATE_EXECUTING     = 1, // regular execution
    LINKMGR_STATE_PAIRING_BEGIN = 4, // just entered pairing and button has not been released
    LINKMGR_STATE_PAIRING       = 2, // in pairing state after button has been released
    LINKMGR_STATE_BUTTON_TEST   = 5, // only used for testing the PBP button in direct link mode
};

/***************************** Local Variables *******************************/
static enum LINKMGR_PushButtonState _buttonState;
static TIMING_TimerHandlerT _timeoutTimer;
static TIMING_TimerHandlerT _pressTimer;
static void (*_beginButtonPairingCallback)(void);
static void (*_endButtonPairingCallback)(void);

/************************ Local Function Prototypes **************************/
static void _LINKMGR_timeoutHandler(void) __attribute__ ((section (".ftext")));
static void _LINKMGR_buttonPressTimeoutHandler(void) __attribute__ ((section (".ftext")));
static void _LINKMGR_buttonPressTimeoutTestHandler(void) __attribute__ ((section (".ftext")));
static void _LINKMGR_pairButtonIrq(void) __attribute__ ((section (".ftext")));
static void _LINKMGR_pairButtonTestIrq(void) __attribute__ ((section (".ftext")));

/**
* FUNCTION NAME: LINKMGR_isPushButtonPairingActive()
*
* @brief  - Tells whether push button pairing is currently active.
*
* @return - TRUE if push button pairing is active or FALSE otherwise.
*/
boolT LINKMGR_isPushButtonPairingActive(void)
{
    return (_buttonState == LINKMGR_STATE_PAIRING);
}

/**
* FUNCTION NAME: LINKMGR_setButtonPairingCallbacks()
*
* @brief  - Sets the functions to be called when entering and exiting push button pairing mode.
*
* @return - void.
*/
void LINKMGR_setButtonPairingCallbacks(
    void (*beginButtonPairingCallback)(void), void (*endButtonPairingCallback)(void))
{
    iassert_LINKMGR_COMPONENT_2(
        _beginButtonPairingCallback == NULL && _endButtonPairingCallback == NULL,
        LINKMGR_EXPECTING_NULL_CALLBACK,
        (uint32)_endButtonPairingCallback,
        (uint32)_endButtonPairingCallback);

    _beginButtonPairingCallback = beginButtonPairingCallback;
    _endButtonPairingCallback = endButtonPairingCallback;
}


/**
* FUNCTION NAME: _LINKMGR_buttonPairingInit()
*
* @brief  - Initializes the push button pairing button state management.
*
* @return - void.
*/
void _LINKMGR_buttonPairingInit(void)
{
    if (linkState.linkMode != LINK_MODE_DIRECT)
    {
        if ((STORAGE_varGet(CONFIGURATION_BITS)->doubleWord >>
             TOPLEVEL_USE_BCAST_NET_CFG_PROTO_OFFSET) & 0x1)
        {
            // Begin in the executing state to skip the DHCP functionality which isn't valid for
            // the Crestron platform.
            _buttonState = LINKMGR_STATE_EXECUTING;
        }
        else
        {
            // Begin in the startup state
            _buttonState = LINKMGR_STATE_STARTUP;
        }

        _timeoutTimer = TIMING_TimerRegisterHandler(
            &_LINKMGR_timeoutHandler, FALSE, LINKMGR_STARTUP_TIMEOUT);
        TIMING_TimerStart(_timeoutTimer);

        // Do not start the timer until the button is pressed
        _pressTimer = TIMING_TimerRegisterHandler(&_LINKMGR_buttonPressTimeoutHandler, FALSE, 0);

        GRG_GpioRegisterIrqHandler(GPIO_IN_PAIRING_BUTTON, &_LINKMGR_pairButtonIrq);
    }
    else // linkState.linkMode == LINK_MODE_DIRECT
    {
        // Push button pairing is not supported in direct link mode because there is no notion of a
        // pair. Instead, in direct link mode, we repurpose the _pressTimer timer to turn on the
        // link LED when the button is held for longer than 3 seconds.  When the button is release
        // the LED will return to its typical behaviour.  The purpose of this feature is to allow
        // manufacturing to excercise the functionality of the button on a new unit (which will
        // default to LINK_MODE_DIRECT).

        _buttonState = LINKMGR_STATE_BUTTON_TEST;
        // Do not start the timer until the button is pressed
        _pressTimer = TIMING_TimerRegisterHandler(
            &_LINKMGR_buttonPressTimeoutTestHandler, FALSE, LINKMGR_TEST_HOLD_TIMEOUT);

        GRG_GpioRegisterIrqHandler(GPIO_IN_PAIRING_BUTTON, &_LINKMGR_pairButtonTestIrq);
    }

    GRG_GpioEnableIrq(GPIO_IN_PAIRING_BUTTON);

    // The last step of initialization is to call the function that would be called on a button
    // state change because the button may already be pressed on startup.
    _LINKMGR_pairButtonTestIrq();
}

/**
* FUNCTION NAME: LINKMGR_completeButtonPairing()
*
* @brief  - Called to indicate that the pairing has completed and thus button pairing mode is
*           finished.
*
* @return - void.
*/
void LINKMGR_completeButtonPairing(void)
{
    TIMING_TimerStop(_pressTimer);
    TIMING_TimerStop(_timeoutTimer);
    _buttonState = LINKMGR_STATE_EXECUTING;
    _LINKMGR_updateLinkLed();
    (*_endButtonPairingCallback)();
}


static void _LINKMGR_timeoutHandler(void)
{
    LINKMGR_completeButtonPairing();
}


/**
* FUNCTION NAME: _LINKMGR_buttonPressTimeoutHandler()
*
* @brief  - Called when the push button has been pressed for continuously for longer than the
*           timeout.
*
* @return - void.
*/
static void _LINKMGR_buttonPressTimeoutHandler(void)
{
    switch(_buttonState)
    {
        case LINKMGR_STATE_STARTUP:
            _buttonState = LINKMGR_STATE_HOLD_FOR_DHCP;
            // Stop the timer that would have moved us out of the startup state since we have just
            // made the state transition manually.
            TIMING_TimerStop(_timeoutTimer);
            TIMING_TimerResetTimeout(_pressTimer, LINKMGR_USE_DHCP_HOLD_TIMEOUT);
            TIMING_TimerStart(_pressTimer);
            break;

        case LINKMGR_STATE_HOLD_FOR_DHCP:
        {
            // Enable DHCP and cause a system reset
            union STORAGE_VariableData* networkAcquisitionMode =
                STORAGE_varExists(NETWORK_ACQUISITION_MODE) ?
                STORAGE_varGet(NETWORK_ACQUISITION_MODE) :
                STORAGE_varCreate(NETWORK_ACQUISITION_MODE);
            networkAcquisitionMode->bytes[0] = TOPLEVEL_NETWORK_ACQUISITION_DHCP;
            STORAGE_systemResetOnWriteComplete();
            STORAGE_varSave(NETWORK_ACQUISITION_MODE);

            _buttonState = LINKMGR_STATE_EXECUTING;
            break;
        }

        case LINKMGR_STATE_EXECUTING:
            _buttonState = LINKMGR_STATE_PAIRING_BEGIN;
            _LINKMGR_updateLinkLed();
            TIMING_TimerResetTimeout(_pressTimer, LINKMGR_RESET_PAIRING_HOLD_TIMEOUT);
            TIMING_TimerStart(_pressTimer);
            break;

        case LINKMGR_STATE_PAIRING_BEGIN:
            _buttonState = LINKMGR_STATE_EXECUTING;
            (*linkState.xusbLinkMgr.removeAllDeviceLinks)();
            _LINKMGR_updateLinkLed();
            break;

        case LINKMGR_STATE_PAIRING:
            LINKMGR_completeButtonPairing();
            break;

        default:
            iassert_LINKMGR_COMPONENT_2(
                FALSE, LINKMGR_INVALID_BUTTON_STATE, _buttonState, __LINE__);
            break;
    }
}


/**
* FUNCTION NAME: _LINKMGR_pairButtonIrq()
*
* @brief  - Called when the pairing push button is pressed or released.
*
* @return - void.
*/
static void _LINKMGR_pairButtonIrq(void)
{
    // A value of 0 is pressed and 1 is released except on the Kintex platform where the logic is
    // inverted.
    const boolT buttonPressed =
        (!GRG_GpioRead(GPIO_IN_PAIRING_BUTTON)) ^
        (GRG_GetPlatformID() == GRG_PLATFORMID_KINTEX7_DEV_BOARD);
    ilog_LINKMGR_COMPONENT_2(
        ILOG_MAJOR_EVENT, LINKMGR_PAIR_BUTTON_EVENT, buttonPressed, _buttonState);

    // If the button press timer was running, the fact that we received an interrupt means that
    // even if we read that the button is still pressed, it did become unpressed at some point.
    TIMING_TimerStop(_pressTimer);

    switch(_buttonState)
    {
        case LINKMGR_STATE_STARTUP:
        case LINKMGR_STATE_EXECUTING:
        case LINKMGR_STATE_PAIRING:
            if(buttonPressed)
            {
                TIMING_TimerResetTimeout(_pressTimer, LINKMGR_BUTTON_DEBOUNCE_TIMEOUT);
                TIMING_TimerStart(_pressTimer);
            }
            else
            {
                // The timer is already stopped above, so there is nothing to do here
            }
            break;

        case LINKMGR_STATE_HOLD_FOR_DHCP:
            // Any button change in this state means that we are no longer steadily holding the
            // button waiting for DHCP to be enabled.
            _buttonState = LINKMGR_STATE_EXECUTING;
            break;

        case LINKMGR_STATE_PAIRING_BEGIN:
            _buttonState = LINKMGR_STATE_PAIRING;
            (*_beginButtonPairingCallback)();
            TIMING_TimerResetTimeout(_timeoutTimer, LINKMGR_PUSH_BUTTON_PAIRING_TIMEOUT);
            TIMING_TimerStart(_timeoutTimer);
            break;

        default:
            iassert_LINKMGR_COMPONENT_2(FALSE, LINKMGR_INVALID_BUTTON_STATE, _buttonState, __LINE__);
            break;
    }
    _LINKMGR_updateLinkLed();
}


/**
* FUNCTION NAME: _LINKMGR_buttonPressTimeoutTestHandler()
*
* @brief  - Called only in direct-link mode when the push button has been continuously pressed
*           for longer than the timeout.
*
* @return - void.
*/
static void _LINKMGR_buttonPressTimeoutTestHandler(void)
{
    // The pairing push-button has been held for longer than LINKMGR_TEST_HOLD_TIMEOUT.  Turn the
    // link LED on to indicate the push-button works.
    GRG_TurnOnLed(LI_LED_SYSTEM_LINK);
}


/**
* FUNCTION NAME: _LINKMGR_pairButtonTestIrq()
*
* @brief  - Called only in direct-link mode when the pairing push button is pressed or released.
*
* @return - void.
*/
static void _LINKMGR_pairButtonTestIrq(void)
{
    const boolT buttonPressed =
        !GRG_GpioRead(GPIO_IN_PAIRING_BUTTON) // a value of 0 is pressed and 1 is released.
        ^ (GRG_GetPlatformID() == GRG_PLATFORMID_KINTEX7_DEV_BOARD); // inverted on Kintex
    ilog_LINKMGR_COMPONENT_2(
        ILOG_MAJOR_EVENT, LINKMGR_PAIR_BUTTON_EVENT, buttonPressed, _buttonState);

    if (buttonPressed)
    {
        TIMING_TimerStart(_pressTimer);
    }
    else
    {
        TIMING_TimerStop(_pressTimer);

        // If the button was released, we need to update the link LED so that it will go back to
        // the appropriate value for regular operation.
        _LINKMGR_updateLinkLed();
    }
}
