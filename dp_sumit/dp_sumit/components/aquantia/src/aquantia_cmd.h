///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2017
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
//!   @file  -  aquantia_cmd.h
//
//!   @brief -  This file contains the icmd information for this component
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////
#ifndef BB_PROGRAM_BB
#ifndef AQUANTIA_CMD_H
#define AQUANTIA_CMD_H


/***************************** Included Headers ******************************/
#include <icmd.h>

/************************ Defined Constants and Macros ***********************/

ICMD_FUNCTIONS_CREATE(AQUANTIA_COMPONENT)
// ICommands requested by HW
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaReadJunctionTempIcmd, "Read Aquantia junction temperature", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaDblRdTest, "Read two Aquantia registers: devType, reg1, reg2", uint8_t, uint16_t, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_AquantiaStopTestPackets, "Stop Generating Aquantia Packets", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaReadVersion, "Show aquantia firmware version", void)

    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaShortReachMode, "Aquantia Read/Write Short Reach Mode(1.83.0): write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTestModeControl, "Aquantia Read/Write Test Control Mode(1.84.F:D): write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTransmitterTestFrequencies, "Aquantia Read/Write Transmitter test frequencies (1.84.C:A) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaFastRetrainAbility, "Aquantia Fast Retrain Ability (1.93.4) write?, writeValue")
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaFastRetrainEnable, "Aquantia Fast Retrain Enable (1.93.0) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTestModeRate, "Aquantia Test Mode rate[1:0] (1.C412.F:E) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaPmaDigitalSystemLoopback, "Aquantia Digital System Loopback (1.D800.F) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaExternalPhyLoopback, "Aquantia External Phy Loopback (1.E400.F) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaEnableFastRetrain, "Enable Aquantia Fast Retrain (1.E400.2) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaForceMdiConfiguration, "Aquantia Force Mdi Configuration (1.E400.1) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaMdiConfiguration, "Aquantia Mdi Configuration(1.E400.0) write?, writeValue", bool, uint16_t)

    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaPcsLoopback, "Aquantia PCS loopback(3.0.E) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantia10GSpeedSelection, "Aquantia 10G Speed Selection(3.0.5:2) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTxScramblerDisable, "Aquantia Tx Scrambler Disable(3.D800.F) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTxInjectCrcError, "Aquantia Tx Inject CRC Error(3.D800.E) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTxInjectFrameError, "Aquantia Tx Inject Frame Error(3.D800.D) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaRxErrorLdpcFrameEnable, "Aquantia Enable Rx LDPC Error Frame(3.E400.0) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaRxLdpcDecoderControl, "Aquantia Control Rx LDPC Decoder (3.E400.F) write?, writeValue", bool, uint16_t)

    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaXsLoopback, "Aquantia XS Loopback (4.0.E) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaReceiveTestPatternEnable, "Enable Aquantia Receive Test Pattern (4.19.2) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaPhyOperatingMode, "Aquantia Operating Phy Mode (4.C441.8:6) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTestPatternSelect, "Aquantia Select Test Pattern (4.19.1:0) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaLoopbackControl, "Aquantia XS Loopback Control (4.C444.F:B) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaMdiPacketGeneration, "Aquantia XS MDI Packet Generation (4.C444.5) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaXsSystemIFPacketGeneration, "Aquantia XS I/F Packet Generation (4.C444.2) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaRate, "Aquantia XS Rate (4.C444.1:0) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTestPatternForceError, "Aquantia Select Test Pattern Force Error (4.D800.F) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTestPatternMode7ForceError, "Aquantia XS Test Pattern Mode 7 Force Error (4.D800.E) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaXAUIRxLocalFaultInjection, "Aquantia XAUI Rx Local Fault Injection (4.D800.D) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTestPatternExtendedSelect, "Aquantia Test-Pattern Extended Select [1:0] (4.D800.C:B) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTestPatternCheckEnable, "Aquantia Test Pattern Check Enable (4.D800.A) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTestPatternCheckPoint, "Aquantia Test Pattern Check Point (4.D800.7) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTestPatternInsertExtraIdles, "Aquantia Test Pattern Insert Extra Idles [2:0] (4.D801.E:C) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTestPatternCheckSelect, "Aquantia Test Pattern Check Select [3:0] (4.D801.B:8) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTestPatternChannelSelect, "Aquantia Test Pattern Channel Select [3:0] (4.D801.3:0) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaChannel0TestPatternErrorCounter, "Aquantia Channel 0 Test Pattern Error Counter [F:0] (4.D810.F:0) ", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaChannel1TestPatternErrorCounter, "Aquantia Channel 1 Test Pattern Error Counter [F:0] (4.D811.F:0) ", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaChannel2TestPatternErrorCounter, "Aquantia Channel 2 Test Pattern Error Counter [F:0] (4.D812.F:0)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaChannel3TestPatternErrorCounter, "Aquantia Channel 3 Test Pattern Error Counter [F:0] (4.D813.F:0) ", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTestPatternMode7ErrorCounter, "Aquantia Test Pattern Mode 7 Error Counter [F:0] (4.D814.F:0)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaXauiTxErrorInjectionLaneSelect, "Aquantia XAUI Tx Error Injection Lane Select [2:0] (4.F800.F:D) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaXauiTxInjectSynchronizationError, "Aquantia XAUI Tx Inject Synchronization Error (4.F800.C) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaXauiTxInjectAlignmentError, "Aquantia XAUI Tx Inject Alignment Error (4.F800.B) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaXauiTxInjectCodeViolation, "Aquantia XAUI Tx Inject Code Violation (4.F800.A) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaXauiTx10BViolationCodeword, "Aquantia XAUI Tx 10B Violation Codeword [9:0] (4.F800.9:0) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaPhyXsSystemLoopbackPassThrough, "Aquantia PHY XS System Loopback Pass Through (4.F802.F) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaPhyXsSystemLoopbackEnable, "Aquantia PHY XS System Loopback Enable (4.F802.E) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaXauiTxLocalFaultInjection, "Aquantia XAUI Tx Local Fault Injection (4.F802.D) write?, writeValue", bool, uint16_t)

    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaRestartAutonegotiation, "Aquantia Restart Autonegotiation (7.0.9) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaSerdesStartUpMode, "Aquantia SERDES Start-Up Mode [2:0] (7.C410.F:D) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaAutonegotiationTimeout, "Aquantia Autonegotiation Timeout [3:0] (7.C411.F:C) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaAutonegotiationTimeoutMod, "Aquantia Autonegotiation Timeout Mod (7.C411.B) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaLinkPartner1000BaseTFullDuplexAbility, "Aquantia Link Partner 1000BASE-T Full Duplex Ability (7.E820.F)", void )
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaLinkPartner1000BaseTHalfDuplexAbility, "Aquantia Link Partner 1000BASE-T Half Duplex Ability (7.E820.E)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaLinkPartnerShortReach, "Aquantia Link Partner Short-Reach (7.E820.D)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantialLinkPartnerAqRateDownshiftCapability, "Aquantia Link Partner AQRate Downshift Capability (7.E820.C) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaLinkPartner5G, "Aquantia Link Partner 5G (7.E820.B) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaLinkPartner2G, "Aquantia Link Partner 2.5G (7.E820.A) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaLinkPartner, "Aquantia Link Partner (7.E820.2)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaAutonegotiationProtocolErrorState, "Aquantia Autonegotiation Protocol Error State (7.E831.D)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaFlpIdleErrorState, "Aquantia FLP Idle Error State (7.E831.C)", void)

    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaEnableDiagnostics, "Aquantia Enable Diagnostics (1E.C400.F) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaHighTempFailureThreshold, "Aquantia High Temp Failure Threshold [F:0] (1E.C421) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaLowTempFailureThreshold, "Aquantia Low Temp Failure Threshold [F:0] [F:0] (1E.C422) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaHighTempWarningThreshold, "Aquantia High Temp Warning Threshold [F:0] [1:0] (1E.C423) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaLowTempWarningThreshold, "Aquantia Low Temp Warning Threshold [F:0] (1E.C424) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaDiagnosticsSelect, "Aquantia Diagnostics Select (1E.C470.F) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaExtendedMdiDiagnosticsSelect, "Aquantia Extended MDI Diagnostics Select [1:0] (1E.C470.E:D) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaInitiateCableDiagnostics, "Aquantia Initiate Cable Diagnostics (1E.C470.4) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaEnableVddPowerSupplyTuning, "Aquantia Enable VDD Power Supply Tuning (1E.C472.E) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTunableExternalVddPowerSupplyPresent, "Aquantia Tunable External VDD Power Supply Present (1E.C472.6)", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaExternalVddChangeRequest, "Aquantia External VDD Change Request [3:0] (1E.C472.5:2) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaEnable5ChannelRfiCancellation, "Aquantia Enable 5th Channel RFI Cancellation (1E.C472.0) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaRateTransitionRequest, "Aquantia Rate Transition Request [2:0] (1E.C473.A:8) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaTrainingSnr, "Aquantia Training SNR [7:0] (1E.C473.7:0) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaGlbLoopbackControl, "Aquantia Loopback Control [4:0] (1E.C47A.F:B) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaGlbMdiPacketGeneration, "Aquantia MDI Packet Generation (1E.C47A.5) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaGlbSystemIFPacketGeneration, "Aquantia System I/F Packet Generation (1E.C47A.3) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaGlobalReservedProvisioningRate, "Aquantia Rate [2:0] (1E.C47A.2:0) write?, writeValue", bool, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaPairAStatus, "Aquantia Pair A Status [2:0] (1E.C800.E:C)",void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaPairBStatus, "Aquantia Pair B Status [2:0] (1E.C800.A:8)",void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaPairCStatus, "Aquantia Pair C Status [2:0] (1E.C800.6:4)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaPairDStatus, "Aquantia Pair D Status [2:0] (1E.C800.2:0)",void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaStatusPairAReflection1, "Aquantia Pair A Reflection #1 [7:0] (1E.C801.F:8)",void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaStatusPairAReflection2, "Aquantia Pair A Reflection #2 [7:0] (1E.C801.7:0)",void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpulseResponseMsw, "Aquantia Impulse Response MSW [F:0] (1E.C802.F:0)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaStatusPairBReflection1, "Aquantia Pair B Reflection #1 [7:0] (1E.C803.F:8)",void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaStatusPairBReflection2, "Aquantia Pair B Reflection #2 [7:0] (1E.C803.7:0)",void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpulseResponseLsw, "Aquantia Impulse Response LSW [F:0] (1E.C804.F:0)",void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaStatusPairCReflection1, "Aquantia Pair C Reflection #1 [7:0] (1E.C805.F:8)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaStatusPairCReflection2, "Aquantia Pair C Reflection #2 [7:0] (1E.C805.7:0)",void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaStatusPairDReflection1, "Aquantia Pair D Reflection #1 [7:0] (1E.C807.F:8)",void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaStatusPairDReflection2, "Aquantia Pair D Reflection #2 [7:0] (1E.C807.7:0)",void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaProcessorIntensiveOperationInProgress, "Aquantia Processor Intensive MDIO Operation InProgress (1E.C831.F)",void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairAReflection1, "Aquantia Pair A Reflection #1 [2:0] (1E.C880.E:C)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairAReflection2, "Aquantia Pair A Reflection #2 [2:0] (1E.C880.A:8)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairAReflection3, "Aquantia Pair A Reflection #3 [2:0] (1E.C880.6:4)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairAReflection4, "Aquantia Pair A Reflection #4 [2:0] (1E.C880.2:0)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairBReflection1, "Aquantia Pair B Reflection #1 [2:0] (1E.C881.E:C)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairBReflection2, "Aquantia Pair B Reflection #2 [2:0] (1E.C881.A:8)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairBReflection3, "Aquantia Pair B Reflection #3 [2:0] (1E.C881.6:4)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairBReflection4, "Aquantia Pair B Reflection #4 [2:0] (1E.C881.2:0)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairCReflection1, "Aquantia Pair C Reflection #1 [2:0] (1E.C882.E:C)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairCReflection2, "Aquantia Pair C Reflection #2 [2:0] (1E.C882.A:8)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairCReflection3, "Aquantia Pair C Reflection #3 [2:0] (1E.C882.6:4)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairCReflection4, "Aquantia Pair C Reflection #4 [2:0] (1E.C882.2:0)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairDReflection1, "Aquantia Pair D Reflection #1 [2:0] (1E.C883.E:C)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairDReflection2, "Aquantia Pair D Reflection #2 [2:0] (1E.C883.A:8)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairDReflection3, "Aquantia Pair D Reflection #3 [2:0] (1E.C883.6:4)", void)
    // ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaImpedancePairDReflection4, "Aquantia Pair D Reflection #4 [2:0] (1E.C883.2:0)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaCableLength, "Aquantia Cable Length [7:0] (1E.C884.7:0)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaGlbLoopbackStatus, "Aquantia Loopback Status [4:0] (1E.C888.F:B)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaMdiPacketGenerationStatus, "Aquantia MDI Packet Generation Status (1E.C888.5)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaGlbSystemIFPacketGenerationStatus, "Aquantia System I/F Packet Generation Status (1E.C888.3)", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(MDIOD_aquantiaGlobalReservedStatusRate, "Aquantia Rate [2:0] (1E.C888.2:0)", void)

// ICommands for operation
    ICMD_FUNCTIONS_ENTRY_FLASH(AquantiaInNomalOperationIcmd,"Check Aquantia operation status", void)
    ICMD_FUNCTIONS_ENTRY_FLASH(AQUANTIA_GeneralRead,"Read Aquantia Register. First arg: Device Type, Second arg: Register Address", uint8_t, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(AQUANTIA_GeneralWrite,"Write Aquantia Register. First arg: Device Type, Second arg: Register Address, Third arg: Value to be Written", uint8_t, uint16_t, uint16_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(AQUANTIA_SetWarningTemperature,"Set Aquantia Warning temperature that firmware checks by reading Temperature value", uint8_t)
    ICMD_FUNCTIONS_ENTRY_FLASH(AQUANTIA_SetShutdownTemperature,"Set Aquantia Shutdown temperature that firmware checks  by reading Temperature value", uint8_t)

ICMD_FUNCTIONS_END(AQUANTIA_COMPONENT)

#endif  // AQUANTIA_CMD_H
#endif  // BB_PROGRAM_BB
