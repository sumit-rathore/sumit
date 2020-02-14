//#################################################################################################
// Icron Technology Corporation - Copyright 2016
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef ULP_LOC_H
#define ULP_LOC_H

// Includes #######################################################################################
#include <ulp.h>

#include <ulp_core_regs.h>

// Constants and Macros ###########################################################################


// control flag definitions
// note that usbControl is only 8 bits and will need to be expanded if we have more then 8 flags
#define ULP_USB_MODULE_LOCAL_ENABLED    0x01    // set when this USB module is enabled
#define ULP_USB_MODULE_PARTNER_ENABLED  0x02    // set when the other side is enabled
#define ULP_USB_MODULE_PARTNER_READY    0x04    // set when the other side is ready, waiting for a connection
#define ULP_USB_MODULE_SYSTEM_UP        0x08    // set when the system is ready for regular operation
#define ULP_USB_MODULE_CHANNEL_UP       0x10    // set when the MCA channel for this module is available
#define ULP_USB_MODULE_RESET_DONE       0x20    // set when the module is not in reset

#define ULP_USB_MODULE_SYSTEM_ENABLED   (ULP_USB_MODULE_LOCAL_ENABLED | ULP_USB_MODULE_PARTNER_ENABLED)

#define ULP_BIT_MASK(bitoffset) (1 << bitoffset)

// LTSSM major states we are interested in
#define ULP_LTSSM_DISABLED      0x10        // disabled state
#define ULP_LTSSM_INACTIVE      0x20        // inactive state, waiting for warm reset
#define ULP_LTSSM_RX_DETECT     0x30        // looking for Rx terminations
#define ULP_LTSSM_POLLING       0x40        // polling for port negotiation and link training
#define ULP_LTSSM_COMPLIANCE    0x50        // Tx compliance mode
#define ULP_LTSSM_U0            0x60        // U0 - active link
#define ULP_LTSSM_U3            0x90        // U3 - link is in standby
#define ULP_LTSSM_RECOVERY      0xA0        // Link is in recovery
#define ULP_LTSSM_LOOPBACK      0xB0        // Link is in loopback

#define ULP_LTSSM_MAJOR_STATE_MASK  0xF0        // Mask to only get the LTSSM major state

// IRQ0 ULP interrupts
#define LEX_ULP_USB3_IRQ0 \
       (ULP_CORE_IRQ0_RAW_RX_TERMINATION_DET    | \
        ULP_CORE_IRQ0_RAW_IN_HOT_RESET          | \
        ULP_CORE_IRQ0_RAW_COMPLETED_HOT_RESET   | \
        ULP_CORE_IRQ0_RAW_IN_WARM_RESET         | \
        ULP_CORE_IRQ0_RAW_COMPLETED_WARM_RESET  | \
        ULP_CORE_IRQ0_RAW_U1_ENTRY_RECEIVED     | \
        ULP_CORE_IRQ0_RAW_U2_ENTRY_RECEIVED     | \
        ULP_CORE_IRQ0_RAW_U3_ENTRY_RECEIVED     | \
        ULP_CORE_IRQ0_RAW_IN_U3                 | \
        ULP_CORE_IRQ0_RAW_U3_EXIT_INITIATED     | \
        ULP_CORE_IRQ0_RAW_U3_EXIT_COMPLETED)

// IRQ1 ULP interrupts
#define LEX_ULP_USB3_IRQ1 \
       (ULP_CORE_IRQ1_RAW_IN_INACTIVE           | \
        ULP_CORE_IRQ1_RAW_IN_RX_DETECT          | \
        ULP_CORE_IRQ1_RAW_IN_POLLING            | \
        ULP_CORE_IRQ1_RAW_IN_U0                 | \
        ULP_CORE_IRQ1_RAW_IN_DISABLED           | \
        ULP_CORE_IRQ1_RAW_IN_COMPLIANCE         | \
        ULP_CORE_IRQ1_RAW_IN_LOOPBACK           | \
        ULP_CORE_IRQ1_RAW_IN_RECOVERY)


// Data Types #####################################################################################

enum UlpLexUsbControl
{
    ULP_USB_LEX_ENABLE,         // enable the USB system on the Lex
    ULP_USB_LEX_DISABLE,        // disable the USB system on the Lex

    ULP_USB_LEX_COMLINK_UP,     // comm link is active
    ULP_USB_LEX_COMLINK_DOWN,   // comm link is inactive

    ULP_USB_LEX_HUB_DOWN,       // Hub is not ready (being programmed)
    ULP_USB_LEX_HUB_READY,      // Hub has been initialized

    // after a com link up event, the paired device might take some time to be available
    // these two messages allow one side to synchronize with the other side
    // (also when doing a USB3 reset)
    ULP_USB_LEX_DEVICE_READY,       // The other paired device is ready (synchronization message)
    ULP_USB_LEX_DEVICE_BUSY,        // The other paired device is busy (synchronization message)

    ULP_USB_LEX_DEVICE_UNAVAILABLE, // The other paired device is not ready

    ULP_USB_LEX_I_AM_ALIVE,         // sent out whenever the Lex is back from a link down and hasn't heard from the Rex
};

enum UlpRexUsbControl
{
    ULP_USB_REX_ENABLE,         // enable the USB system on the Rex
    ULP_USB_REX_DISABLE,        // disable the USB system on the Rex

    ULP_USB_REX_COMLINK_UP,     // comm link is active
    ULP_USB_REX_COMLINK_DOWN,   // comm link is inactive

    ULP_USB_REX_HUB_DOWN,       // Hub is not ready (being programmed)
    ULP_USB_REX_HUB_READY,      // Hub has been initialized

    ULP_USB_REX_VBUS_DOWN,       // Rex Vbus in a minimum low cycle
    ULP_USB_REX_VBUS_READY,      // Rex Vbus ready to be put high

    // after a com link up event, the paired device might take some time to be available
    // these two messages allow one side to synchronize with the other side
    // (also when doing a USB3 reset)
    ULP_USB_REX_DEVICE_READY,       // The other paired device is ready (synchronization message)
    ULP_USB_REX_DEVICE_BUSY,        // The other paired device is busy (synchronization message)

    ULP_USB_REX_DEVICE_UNAVAILABLE, // The other paired device is not ready
};

enum UlpUsbControl
{
    ULP_USB_CONTROL_ENABLE,             // enable this USB module
    ULP_USB_CONTROL_DISABLE,            // disable this USB module

    ULP_USB_CONTROL_PARTNER_ENABLED,    // the other side is enabled
    ULP_USB_CONTROL_PARTNER_SHUTDOWN,   // the other side is disabled

    ULP_USB_CONTROL_PARTNER_READY,      // the other side is ready, waiting for a connection
    ULP_USB_CONTROL_PARTNER_DISABLED,   // the other side is disabled

    ULP_USB_CONTROL_SYSTEM_UP,          // system is up and ready
    ULP_USB_CONTROL_SYSTEM_DOWN,        // system is down, not connected or ready

    ULP_USB_CONTROL_MCA_CHANNEL_RDY,    // MCA channel ready
    ULP_USB_CONTROL_MCA_CHANNEL_DOWN,   // MCA channel unavailable

    ULP_USB_CONTROL_RESET_ACTIVE,       // reset in progress
    ULP_USB_CONTROL_RESET_DONE,         // reset done
};

enum Ulp_UxStateNames
{
    ULP_U1_POWER_STATE,             // specifies the U1 power state
    ULP_U2_POWER_STATE,             // specifies the U2 power state
    ULP_U3_POWER_STATE,             // specifies the U3 power state (P2 rather then P3)
    ULP_SS_DISABLED_POWER_STATE,    // when in SS_DISABLED, go to a low power state (P2 rather then P3)
};

// Non specific USB cpu messages sent from the LEX to the Rex
enum UlpLexSendUsbMessage
{
    ULP_LEX_TO_REX_LEX_COMLINK_UP,          // Lex Com link is up
    ULP_LEX_TO_REX_LEX_USB_READY,           // Lex is ready to start USB operations
    ULP_LEX_TO_REX_LEX_BUSY,                // Lex is busy setting up USB
};

// USB2 cpu messages sent from the LEX to the Rex
enum UlpLexSendUsb2Message
{
    ULP_LEX_TO_REX_USB2_MODULE_DISABLED,    // Lex USB2 module is disabled
    ULP_LEX_TO_REX_USB2_MODULE_ENABLED,     // Lex USB2 module is enabled
    ULP_LEX_TO_REX_USB2_FAILED,             // USB 2 state machine has failed on the Lex
    ULP_LEX_TO_REX_USB2_DISABLED,           // USB 2 state machine is in the disabled state on the Lex
    ULP_LEX_TO_REX_USB2_CONNECTED,          // Lex has detected a USB 2 only Host port
    ULP_LEX_TO_REX_USB2_DISCONNECTED,       // Host port is disconnected from the Lex
};

// USB3 cpu messages sent from the LEX to the Rex
enum UlpLexSendUsb3Message
{
    ULP_LEX_TO_REX_USB3_MODULE_DISABLED,    // Lex USB3 module is disabled
    ULP_LEX_TO_REX_USB3_MODULE_ENABLED,     // Lex USB3 module is enabled
    ULP_LEX_TO_REX_USB3_DISABLED,           // USB 3 state machine is in the disabled state on the Lex
    ULP_LEX_TO_REX_USB3_CONNECTED,          // Lex has detected a USB 3 + 2 Host port
    ULP_LEX_TO_REX_USB3_DISCONNECTED,       // Lex far end terminations have been removed
    ULP_LEX_TO_REX_USB3_SETUP_SUCCESS,      // USB 3 setup on Lex successful
    ULP_LEX_TO_REX_USB3_FAILURE,             // USB 3 failed on the Lex
    ULP_LEX_TO_REX_USB3_WARM_RESET,         // Lex has gone through a USB3 warm reset

    ULP_LEX_TO_REX_LEX_IN_HOT_RESET,        // Lex is in hot reset
    ULP_LEX_TO_REX_LEX_IN_STANDBY,          // Lex is in standby
    ULP_LEX_TO_REX_LEX_EXITING_STANDBY,     // Lex is exiting standby
};

// USB3 reset cpu messages sent from the LEX to the Rex
enum UlpLexSendUsb3ResetMessage
{
    ULP_LEX_TO_REX_USB3_RESET_LEX_ONLY_RESET_REQ,   // To Rex: Lex Only Reset requested
    ULP_LEX_TO_REX_USB3_RESET_LEX_ONLY_RESET_MCA_UP,    // To Rex: bring up your MCA channel
    ULP_LEX_TO_REX_USB3_RESET_LEX_ONLY_RESET_DONE,  // To Rex: Lex Only Reset done

    ULP_LEX_TO_REX_USB3_RESET_START_HARD_RESET, // To Rex: Start USB3 reset (bring down system)
    ULP_LEX_TO_REX_USB3_RESET_START_USB3_RESTART,   // To Rex: Start USB3 reset->restart
    ULP_LEX_TO_REX_USB3_RESET_TAKE_DOWN_USB3,   // To Rex: take down USB3
    ULP_LEX_TO_REX_USB3_RESET_TAKE_DOWN_MCA,    // To Rex: take down MCA
    ULP_LEX_TO_REX_USB3_RESET_ULP_RESET,        // To Rex: reset ULP
    ULP_LEX_TO_REX_USB3_RESET_PHY_RESET,        // To Rex: reset PHY
    ULP_LEX_TO_REX_USB3_RESET_ACTIVATE_PHY,     // To Rex: take Phy out of reset
    ULP_LEX_TO_REX_USB3_RESET_ACTIVATE_PHY_2,   // To Rex: take Phy out of reset, stage 2
    ULP_LEX_TO_REX_USB3_RESET_ACTIVATE_ULP,     // To Rex: activate ULP
    ULP_LEX_TO_REX_USB3_RESET_BRING_UP_MCA,     // To Rex: bring up MCA

};

// cpu messages sent from the Rex to the Lex
enum UlpRexSendUsbMessage
{
    ULP_REX_TO_LEX_REX_USB_MODULE_DISABLED,     // Rex to Lex - Rex USB module is currently disabled
    ULP_REX_TO_LEX_REX_COMLINK_UP,              // Rex Com link is up
    ULP_REX_TO_LEX_REX_USB_READY,               // Rex is ready to start USB operations
    ULP_REX_TO_LEX_REX_BUSY,                    // Rex is busy setting up USB
};

// cpu messages sent from the Rex to the Lex
enum UlpRexSendUsb2Message
{
    ULP_REX_TO_LEX_USB2_MODULE_DISABLED,    // Rex to Lex - Rex USB2 module is disabled
    ULP_REX_TO_LEX_USB2_MODULE_ENABLED,     // Rex to Lex - Rex USB2 module is enabled
    ULP_REX_TO_LEX_USB2_FAILED,             // Rex to Lex - Rex USB2 has failed
    ULP_REX_TO_LEX_USB2_DISABLED,           // Rex to Lex - Rex USB2 is currently disabled
};

// cpu messages sent from the Rex to the Lex
enum UlpRexSendUsb3Message
{
    ULP_REX_TO_LEX_USB3_MODULE_DISABLED,    // Rex to Lex - Rex USB3 module is disabled
    ULP_REX_TO_LEX_USB3_MODULE_ENABLED,     // Rex to Lex - Rex USB3 module is enabled
    ULP_REX_TO_LEX_USB3_DISABLED,           // Rex to Lex - Rex USB3 is currently disabled
    ULP_REX_TO_LEX_USB3_READY,              // Rex to Lex - Rex USB3 is enabled and ready
    ULP_REX_TO_LEX_USB3_REX_AT_U0,          // USB3 setup on Rex successful
    ULP_REX_TO_LEX_USB3_FAILURE,            // USB3 on Rex failed
    ULP_REX_TO_LEX_USB3_DEVICE_DISCONNECTED,     // Rex to Lex - Device disconnected
    ULP_REX_TO_LEX_USB3_REX_AT_INACTIVE,    // the Rex is in the INACTIVE state, and needs a warm reset
};

// cpu messages sent from the Rex to the Lex
enum UlpRexSendUsb3ResetMessage
{
    ULP_REX_TO_LEX_USB3_RESET_DISABLED,         // To Lex: Rex USB3 reset not allowed (USB3 disabled)

    ULP_REX_TO_LEX_USB3_RESET_LEX_ONLY_RESET_ACK,   // To Lex: Lex only reset acknowledged

    ULP_REX_TO_LEX_USB3_RESET_START,            // To Lex: USB3 reset started
    ULP_REX_TO_LEX_USB3_RESET_TAKE_DOWN_USB3,   // To Lex: USB3 taken down
    ULP_REX_TO_LEX_USB3_RESET_TAKE_DOWN_MCA,    // To Lex: MCA USB3 channel shut down
    ULP_REX_TO_LEX_USB3_RESET_ULP_RESET,        // To Lex: ULP reset
    ULP_REX_TO_LEX_USB3_RESET_PHY_RESET,        // To Lex: PHY reset
    ULP_REX_TO_LEX_USB3_RESET_ACTIVATE_PHY,     // To Lex: Phy taken out of reset
    ULP_REX_TO_LEX_USB3_RESET_ACTIVATE_PHY_2,   // To Lex: Phy reset, stage 2
    ULP_REX_TO_LEX_USB3_RESET_ACTIVATE_ULP,     // To Lex: activate ULP
    ULP_REX_TO_LEX_USB3_RESET_BRING_UP_MCA,     // To Lex: bring up MCA
};

// cpu messages sent from the REX and Lex GE control module
enum UlpGeControlMessage
{
    ULP_GE_CONTROL_MSG_GE_ENABLED,              // GE enabled on the far side
    ULP_GE_CONTROL_MSG_GE_DISABLED,             // GE disabled on the far side
    ULP_GE_CONTROL_MSG_START_GE,                // Start GE on your side (GE running on Rex)

};

// control states for UlpGeControl()
enum UlpGeControlStates
{
    ULP_GE_CONTROL_LINK_DOWN,               // bring GE link down, and put GE in reset
    ULP_GE_CONTROL_LINK_UP,                 // Start bringing the GE link up
    ULP_GE_CONTROL_GE_ENABLE,               // take GE out of reset (assumes link (channel) is ready)
};

enum UlpUsb3ResetReason
{
    ULP_USB3_RESET_IDLE,                    // USB3 reset not active
    ULP_USB3_RESET_USB3_RESTART,            // USB3 is resetting->restarting
    ULP_USB3_RESET_HARD_RESET,              // hard reset required
};

enum UlpUsbControlResult
{
    ULP_USB_CONTROL_RESULT_DISABLE,         // this USB module can be disabled
    ULP_USB_CONTROL_RESULT_ENABLE,          // this USB module can be enabled
    ULP_USB_CONTROL_RESULT_UNCHANGED,       // no change since the last time this module was called
};

// used to declare the status of the USB2 or USB3 state machines
enum UlpUsbModuleStatus
{
    ULP_USB_MODULE_READY,           // module is active
    ULP_USB_MODULE_CONNECTED,       // module is connected, running
    ULP_USB_MODULE_DISCONNECTED,    // module is disconnected, stopped
    ULP_USB_MODULE_DISABLED,        // module is inactive

    ULP_USB_MODULE_SHUTDOWN,        // module is shutdown - disabled by user or the other side
};

// Function Declarations ##########################################################################

// common functions
void ULP_LexInit(void)                                                                  __attribute__((section(".atext")));
void ULP_RexInit(void)                                                                  __attribute__((section(".atext")));
void _ULP_HalInit(void)    __attribute__((section(".atext")));
void UlpHalEnableUlpInterrupts(uint32_t ulpIrq0ToEnable, uint32_t ulpIrq1ToEnable)      __attribute__((section(".atext")));
void UlpHalDisableUlpInterrupts(uint32_t ulpIrq0ToDisable, uint32_t ulpIrq1ToDisable)   __attribute__((section(".atext")));
void _ULP_coreHalSetReadClearStats(void)                                                __attribute__((section(".atext")));
void _ULP_phyHalSetReadClearStats(void)                                                 __attribute__((section(".atext")));
void UlpHalSplitDp(void)                                                                __attribute__((section(".atext")));
void UlpHalSetupUlpCoreGuards(void)                                                     __attribute__((section(".atext")));
void UlpHalRxPartnerFifo(void)                                                          __attribute__((section(".atext")));
enum UlpUsbControlResult ULP_UsbSetControl(
    uint8_t *usbControl,
    enum UlpUsbControl controlOperation)                                            __attribute__((section(".atext")));
bool ULP_UsbControlEnabled(const uint8_t usbControl)                                __attribute__((section(".atext")));
bool ULP_Usb2SystemEnabled(void)                                                    __attribute__((section(".atext")));
bool ulp_Usb3ResetOnDisconnect(void)                                                __attribute__((section(".atext")));
void ulp_StatMonCoreControl(bool enable)                                            __attribute__((section(".atext")));
void ulp_StatMonPhyControl(bool enable)                                             __attribute__((section(".atext")));
void ULP_StatInit(void)                                                             __attribute__((section(".atext")));

void ULP_controlTurnOffUSB3(void)                                                   __attribute__((section(".atext")));
//void ULP_controlTakedownUSB3(void)                                                  __attribute__((section(".atext")));
void UlpVbusControl(bool enable)                                                    __attribute__((section(".atext")));
void UlpPhyVbusControl(bool enable)                                                 __attribute__((section(".atext")));

void UlpStatusChange(enum UlpStatus status )                                        __attribute__((section(".atext")));

void _ULP_halUsb3SetLowPowerStates(enum Ulp_UxStateNames enableUxState)             __attribute__((section(".atext")));
void _ULP_halControlStandbyMode(bool enabled)                                       __attribute__((section(".atext")));
void _ULP_halControlLowPowerWaitMode(bool enabled)                                  __attribute__((section(".atext")));
void _ULP_halUsb3SetUpstreamPort(bool setToUpstream)                                __attribute__((section(".atext")));
void UlpHalSetTxMargin(uint8_t txMargin)                                            __attribute__((section(".atext")));
void UlpHalControlCreditHpTimer(bool enableCreditTimer)                             __attribute__((section(".atext")));
void _ULP_halUsb3ConfigInternalLoopback(bool enableLoopback)                        __attribute__((section(".atext")));
void _ULP_halControlAutoRxTerminations(bool enabled)                                __attribute__((section(".atext")));
void _ULP_halControlWaitInPolling(bool enable)                                      __attribute__((section(".atext")));
void _ULP_halControlGeVbus(bool active)                                             __attribute__((section(".atext")));
void UlpPendingHpTimerCtrl(bool enable)                                             __attribute__((section(".atext")));

// Lex only
void ULP_LexHostInit(void)                                                          __attribute__((section(".lexatext")));
void ULP_LexUsbControl(enum UlpLexUsbControl operation)                             __attribute__((section(".lexatext")));
bool UlpLexComlinkActive(void)                                                      __attribute__((section(".lexatext")));
bool UlpLexRexUsbDisabled(void)                                                     __attribute__((section(".lexatext")));
void ULP_LexHostControl(bool enable)                                                __attribute__((section(".lexatext")));
bool ULP_LexHostConnected(void)                                                     __attribute__((section(".lexatext")));
void UlpLexUsb3LexOnlyResetDone(void)                                               __attribute__((section(".lexatext")));
void ULP_LexHostUsb3Connected(bool usb3Present)                                     __attribute__((section(".lexatext")));
void UlpLexHostSendUsb2Status(enum UlpUsbModuleStatus usb2Status)                   __attribute__((section(".lexatext")));
void UlpLexHostSendUsb3Status(enum UlpUsbModuleStatus usb3Status)                   __attribute__((section(".lexatext")));
void ULP_LexHostUsb3RestartRequest(void)                                            __attribute__((section(".lexatext")));
void ULP_LexHostUsb3ResetDone(void)                                                 __attribute__((section(".lexatext")));
void ULP_LexHostCycleRequest(void)                                                  __attribute__((section(".lexatext")));
bool ULP_LexUsb2LocalEnabled(void)                                                  __attribute__((section(".lexatext")));
bool ULP_LexUsb2SystemEnabled(void)                                                 __attribute__((section(".lexatext")));
bool ULP_LexUsb3Enabled(void)                                                       __attribute__((section(".lexatext")));
bool ULP_LexUsb3SystemEnabled(void)                                              __attribute__((section(".lexatext")));
void UlpLexRexUsbAllowed(void)                                                      __attribute__((section(".lexatext")));

enum UlpUsbModuleStatus UlpLexUsb3Status(void)                                      __attribute__((section(".lexatext")));
void UlpLexUsb3HostConnect(bool connect)                                            __attribute__((section(".lexatext")));

enum UlpUsbModuleStatus UlpLexUsb2Status(void)                                      __attribute__((section(".lexatext")));
void UlpLexUsb2HostConnect(bool connect)                                            __attribute__((section(".lexatext")));
void UlpLexUsb2Failed(void)                                                         __attribute__((section(".lexatext")));

void UlpSendCPUCommLexUsbMessage(enum UlpLexSendUsbMessage lexMessage)              __attribute__((section(".lexatext")));
void UlpSendCPUCommLexUsb2Message(enum UlpLexSendUsb2Message lexMessage)            __attribute__((section(".lexatext")));
void UlpSendCPUCommLexUsb3Message(enum UlpLexSendUsb3Message lexMessage)            __attribute__((section(".lexatext")));
void UlpSendCPUCommLexUsb3ResetMessage(enum UlpLexSendUsb3ResetMessage lexMessage)  __attribute__((section(".lexatext")));
void UlpLexProgramCypressHub(void)                                                  __attribute__((section(".lexatext")));

// interrupt functions - need to be in IRAM for speed
void ULP_LexUsb3Irq0(uint32_t lexUlpInts)                                           __attribute__((section(".lexftext")));
void ULP_LexUsb3Irq1(uint32_t lexUlpInts)                                           __attribute__((section(".lexftext")));

uint32_t UlpHalGetIrq0Interrupts(void);
uint32_t UlpHalGetIrq1Interrupts(void);

void ULP_LexUsb2Init(bool enabled)                                                  __attribute__((section(".lexatext")));
bool ULP_LexUsb2Control(enum UlpUsbControl controlOperation)                        __attribute__((section(".lexatext")));
void ULP_LexUsb2RxCpuMessageHandler(
    uint8_t subType, const uint8_t *msg, uint16_t msgLength)                        __attribute__((section(".lexatext")));

void ULP_LexUsb3Init(bool enabled)                                                  __attribute__((section(".lexatext")));
bool ULP_LexUsb3Control(enum UlpUsbControl controlOperation)                        __attribute__((section(".lexatext")));
void ULP_LexUsb3RxCpuMessageHandler(
    uint8_t subType, const uint8_t *msg, uint16_t msgLength)                        __attribute__((section(".lexatext")));

void ULP_controlLexInitUSB3(void)                                                   __attribute__((section(".lexatext")));
void ULP_controlLexBringUpUSB3(void)                                                __attribute__((section(".lexatext")));

void ULP_UlpLexResetUsb3Init(void)                                                  __attribute__((section(".lexatext")));
void ULP_UlpLexUsb3ResetControl(bool enable)                                        __attribute__((section(".lexatext")));
void ULP_UlpLexUsb3ResetStart(void)                                                 __attribute__((section(".lexatext")));
void ULP_UlpLexUsb3ResetStop(void)                                                  __attribute__((section(".lexatext")));
void ULP_UlpLexUsb3LexOnlyResetStart(void)                                          __attribute__((section(".lexatext")));
void ULP_UlpLexUsb3LexOnlyResetStop(void)                                           __attribute__((section(".lexatext")));
void ULP_LexUsb3ResetRxCpuMessageHandler(
    uint8_t subType, const uint8_t *msg, uint16_t msgLength)                        __attribute__((section(".lexatext")));

void UlpHalSetSSdisable(void)                                                       __attribute__((section(".atext")));
void UlpHalGoToInactive(void)                                                     __attribute__((section(".atext")));
bool _ULP_halIsInactivePending(void)                                                __attribute__((section(".atext")));
void UlpHalSetRxDetect(void)                                                        __attribute__((section(".atext")));
void UlpHalControlRxTerminations(bool enabled)                                      __attribute__((section(".atext")));

// Hot and warm reset have tight timing constraints - leave in the fastest RAM
void _ULP_halSetHotResetWait(bool enabled);
void _ULP_halSetHotReset(void);
void _ULP_halSetWarmReset(void);

// Rex only
void ULP_controlRexInitUSB3(void)                                                   __attribute__((section(".rexatext")));
void ULP_RexUsbControl(enum UlpRexUsbControl operation)                             __attribute__((section(".rexatext")));

void ULP_RexUsb2Init(bool enabled)                                                  __attribute__((section(".rexatext")));
bool ULP_RexUsb2Control(enum UlpUsbControl controlOperation)                        __attribute__((section(".rexatext")));
bool RexUlpUsb2IsActive(void)                                                       __attribute__((section(".rexatext")));
void ULP_RexUsb2RxCpuMessageHandler(
    uint8_t subType, const uint8_t *msg, uint16_t msgLength)                        __attribute__((section(".rexatext")));
void UlpRexUsb2Failed(void)                                                         __attribute__((section(".rexatext")));
bool ULP_RexUsb2Enabled(void)                                                       __attribute__((section(".rexatext")));
bool ULP_RexUsb2SystemEnabled(void)                                                 __attribute__((section(".rexatext")));

void UlpSendCPUCommRexUsbMessage(enum UlpRexSendUsbMessage lexMessage)              __attribute__((section(".rexatext")));
void UlpSendCPUCommRexUsb2Message(enum UlpRexSendUsb2Message lexMessage)            __attribute__((section(".rexatext")));
void UlpSendCPUCommRexUsb3Message(enum UlpRexSendUsb3Message lexMessage)            __attribute__((section(".rexatext")));
void UlpSendCPUCommRexUsb3ResetMessage(enum UlpRexSendUsb3ResetMessage lexMessage)  __attribute__((section(".rexatext")));

void ULP_RexUsb3Irq0(uint32_t rexUlpInts)                                           __attribute__((section(".rexftext")));
void ULP_RexUsb3Irq1(uint32_t rexUlpInts)                                           __attribute__((section(".rexftext")));

void ULP_RexUsb3Init(bool enabled)                                                  __attribute__((section(".rexatext")));
bool ULP_RexUsb3Control(enum UlpUsbControl controlOperation)                        __attribute__((section(".rexatext")));
bool RexUlpUsb3IsActive(void)                                                       __attribute__((section(".rexatext")));
void ULP_RexUsb3RxCpuMessageHandler(
    uint8_t subType, const uint8_t *msg, uint16_t msgLength)                        __attribute__((section(".rexatext")));
void ULP_RexUsb3ResetRxCpuMessageHandler(
    uint8_t subType, const uint8_t *msg, uint16_t msgLength)                        __attribute__((section(".rexatext")));
void UlpRexProgramCypressHub(void)                                                  __attribute__((section(".rexatext")));
void UlpRexSetVbus(bool enable)                                                     __attribute__((section(".rexatext")));


void UlpHalRexVbusActive(bool active)                                             __attribute__((section(".rexatext")));

bool ULP_RexUsb3Enabled(void)                                                       __attribute__((section(".rexatext")));

// Generic
uint8_t _ULP_GetLtssmMajorState(void)                                               __attribute__((section(".atext")));
uint8_t UlpHalGetLTSSM(void)                                                    __attribute__((section(".atext")));
uint32_t _ULP_GetControlValue(void)                                                 __attribute__((section(".atext")));
uint32_t _ULP_GetConfigurationValue(void)                                           __attribute__((section(".atext")));
bool _ULP_IsLtssmValid(void)                                                        __attribute__((section(".atext")));
bool _ULP_isVbusDetSet(void);
bool UlpHalRxTerminationsPresent(void);

// GE control functions
void ULP_GeControlInit(void)                                                        __attribute__((section(".atext")));
void UlpGeControl(bool enabled)                                                     __attribute__((section(".atext")));

#endif // ULP_LOC_H
