///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010, 2012
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
//!   @file  -  mdio_icmds.c
//
//!   @brief -  This file contains the functions for icmd
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

#ifndef BB_PROGRAM_BB
// Includes #######################################################################################
#include <bb_top.h>
#include "aquantia_loc.h"
#include "aquantia_cmd.h"
#include "aquantia_log.h"

// Constants and Macros ###########################################################################

// Data Types #####################################################################################

// Static Function Declarations ###################################################################
static void AQUANTIA_GeneralWriteHandler(void)                  __attribute__((section(".atext")));
static void AQUANTIA_ShowGeneralReadWriteResult(uint16_t value) __attribute__((section(".atext")));

// Global Variables ###############################################################################

// Static Variables ###############################################################################
static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaShortReachMode =
{
    MDIO_DEVTYPE_PMD_PMA,                                   // devType
    PMA_ShortReachMode_ADDR,                                // address
    PMA_ShortReachMode_MASK,                                // bitMask
    PMA_ShortReachMode_OFFSET,                              // bitOffset
    AQUANTIA_SHORT_REACH_MODE                               // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTestModeControl =
{
    MDIO_DEVTYPE_PMD_PMA,                                   // devType
    PMA_TestModeControl_ADDR,                               // address
    PMA_TestModeControl_MASK,                               // bitMask
    PMA_TestModeControl_OFFSET,                             // bitOffset
    AQUANTIA_TEST_MODE_CONTROL                              // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTransmitterTestFrequencies =
{
    MDIO_DEVTYPE_PMD_PMA,                                   // devType
    PMA_TransmitterTestFrequencies_ADDR,                    // address
    PMA_TransmitterTestFrequencies_MASK,                    // bitMask
    PMA_TransmitterTestFrequencies_OFFSET,                  // bitOffset
    AQUANTIA_TRANSMITTER_TEST_FREQUENCIES                   // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaFastRetrainAbility =
{
    MDIO_DEVTYPE_PMD_PMA,                                   // devType
    PMA_FastRetrainAbility_ADDR,                            // address
    PMA_FastRetrainAbility_MASK,                            // bitMask
    PMA_FastRetrainAbility_OFFSET,                          // bitOffset
    AQUANTIA_FAST_RETRAIN_ABILITY                           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaFastRetrainEnable =
{
    MDIO_DEVTYPE_PMD_PMA,                                   // devType
    PMA_FastRetrainEnable_ADDR,                             // address
    PMA_FastRetrainEnable_MASK,                             // bitMask
    PMA_FastRetrainEnable_OFFSET,                           // bitOffset
    AQUANTIA_FAST_RETRAIN_ENABLE                            // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTestModeRate =
{
    MDIO_DEVTYPE_PMD_PMA,                                   // devType
    PMA_TestModeRate_ADDR,                                  // address
    PMA_TestModeRate_MASK,                                  // bitMask
    PMA_TestModeRate_OFFSET,                                // bitOffset
    AQUANTIA_TEST_MODE_RATE                                 // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaPmaDigitalSystemLoopback =
{
    MDIO_DEVTYPE_PMD_PMA,                                   // devType
    PMA_PmaDigitalSystemLoopback_ADDR,                      // address
    PMA_PmaDigitalSystemLoopback_MASK,                      // bitMask
    PMA_PmaDigitalSystemLoopback_OFFSET,                    // bitOffset
    AQUANTIA_PMA_DIGITAL_SYSTEM_LOOPBACK                    // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaExternalPhyLoopback =
{
    MDIO_DEVTYPE_PMD_PMA,                                   // devType
    PMA_ExternalPhyLoopback_ADDR,                           // address
    PMA_ExternalPhyLoopback_MASK,                           // bitMask
    PMA_ExternalPhyLoopback_OFFSET,                         // bitOffset
    AQUANTIA_EXTERNAL_PHY_LOOPBACK                          // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaEnableFastRetrain =
{
    MDIO_DEVTYPE_PMD_PMA,                                   // devType
    PMA_EnableFastRetrain_ADDR,                             // address
    PMA_EnableFastRetrain_MASK,                             // bitMask
    PMA_EnableFastRetrain_OFFSET,                           // bitOffset
    AQUANTIA_ENABLE_FAST_RETRAIN                            // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaForceMdiConfiguration =
{
    MDIO_DEVTYPE_PMD_PMA,                                   // devType
    PMA_ForceMdiConfiguration_ADDR,                         // address
    PMA_ForceMdiConfiguration_MASK,                         // bitMask
    PMA_ForceMdiConfiguration_OFFSET,                       // bitOffset
    AQUANTIA_FORCE_MDI_CONFIGURATION                        // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaMdiConfiguration =
{
    MDIO_DEVTYPE_PMD_PMA,                                   // devType
    PMA_MdiConfiguration_ADDR,                              // address
    PMA_MdiConfiguration_MASK,                              // bitMask
    PMA_MdiConfiguration_OFFSET,                            // bitOffset
    AQUANTIA_MDI_CONFIGURATION                              // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaPcsLoopback =
{
    MDIO_DEVTYPE_PCS,                                       // devType
    PCS_Loopback_ADDR,                                      // address
    PCS_Loopback_MASK,                                      // bitMask
    PCS_Loopback_OFFSET,                                    // bitOffset
    AQUANTIA_PCS_LOOPBACK                                   // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantia10GSpeedSelection =
{
    MDIO_DEVTYPE_PCS,                                       // devType
    PCS_10GSpeedSelection_ADDR,                             // address
    PCS_10GSpeedSelection_MASK,                             // bitMask
    PCS_10GSpeedSelection_OFFSET,                           // bitOffset
    AQUANTIA_10G_SPEED_SELECTION                            // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTxScramblerDisable =
{
    MDIO_DEVTYPE_PCS,                                       // devType
    PCS_TxScramblerDisable_ADDR,                            // address
    PCS_TxScramblerDisable_MASK,                            // bitMask
    PCS_TxScramblerDisable_OFFSET,                          // bitOffset
    AQUANTIA_TX_SCRAMBLER_DISABLE                           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTxInjectCrcError =
{
    MDIO_DEVTYPE_PCS,                                       // devType
    PCS_TxInjectCrcError_ADDR,                              // address
    PCS_TxInjectCrcError_MASK,                              // bitMask
    PCS_TxInjectCrcError_OFFSET,                            // bitOffset
    AQUANTIA_TX_INJECT_CRC_ERROR                            // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTxInjectFrameError =
{

    MDIO_DEVTYPE_PCS,                                       // devType
    PCS_TxInjectFrameError_ADDR,                            // address
    PCS_TxInjectFrameError_MASK,                            // bitMask
    PCS_TxInjectFrameError_OFFSET,                          // bitOffset
    AQUANTIA_TX_INJECT_FRAME_ERROR                          // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaRxErrorLdpcFrameEnable =
{
    MDIO_DEVTYPE_PCS,                                       // devType
    PCS_RxErrorLdpcFrameEnable_ADDR,                        // addres
    PCS_RxErrorLdpcFrameEnable_MASK,                        // bitMask
    PCS_RxErrorLdpcFrameEnable_OFFSET,                      // bitOffset
    AQUANTIA_RX_ERROR_LDPC_FRAME_ENABLE                     // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaRxLdpcDecoderControl =
{
    MDIO_DEVTYPE_PCS,                                       // devType
    PCS_RxLdpcDecoderControl_ADDR,                          // address
    PCS_RxLdpcDecoderControl_MASK,                          // bitMask
    PCS_RxLdpcDecoderControl_OFFSET,                        // bitOffset
    AQUANTIA_RX_LDPC_DECODER_CONTROL                        // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaXsLoopback =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_Loopback_ADDR,                                       // address
    XS_Loopback_MASK,                                       // bitMask
    XS_Loopback_OFFSET,                                     // bitOffset
    AQUANTIA_XS_LOOPBACK                                    // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaReceiveTestPatternEnable =
{

    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_ReceiveTestPatternEnable_ADDR,                       // address
    XS_ReceiveTestPatternEnable_MASK,                       // bitMask
    XS_ReceiveTestPatternEnable_OFFSET,                     // bitOffset
    AQUANTIA_RECEIVE_TEST_PATTERN_ENABLE                    // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaPhyOperatingMode =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_PhyOperatingMode_ADDR,                               // address
    XS_PhyOperatingMode_MASK,                               // bitMask
    XS_PhyOperatingMode_OFFSET,                             // bitOffset
    AQUANTIA_PHY_OPERATING_MODE                             // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTestPatternSelect =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_TestPatternSelect_ADDR,                              // address
    XS_TestPatternSelect_MASK,                              // bitMask
    XS_TestPatternSelect_OFFSET,                            // bitOffset
    AQUANTIA_TEST_PATTERN_SELECT                            // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaLoopbackControl =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_LoopbackControl_ADDR,                                // address
    XS_LoopbackControl_MASK,                                // bitMask
    XS_LoopbackControl_OFFSET,                              // bitOffset
    AQUANTIA_LOOPBACK_CONTROL                               // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaMdiPacketGeneration =
{

    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_MdiPacketGeneration_ADDR,                            // address
    XS_MdiPacketGeneration_MASK,                            // bitMask
    XS_MdiPacketGeneration_OFFSET,                          // bitOffset
    AQUANTIA_MDI_PACKET_GENERATION                          // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaXsSystemIFPacketGeneration =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_SystemIFPacketGeneration_ADDR,                       // address
    XS_SystemIFPacketGeneration_MASK,                       // bitMask
    XS_SystemIFPacketGeneration_OFFSET,                     // bitOffset
    AQUANTIA_XS_SYSTEM_IF_PACKET_GENERATION                 // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaRate =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_Rate_ADDR,                                           // address
    XS_Rate_MASK,                                           // bitMask
    XS_Rate_OFFSET,                                         // bitOffset
    AQUANTIA_RATE                                           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTestPatternForceError =
{

    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_TestPatternForceError_ADDR,                          // address
    XS_TestPatternForceError_MASK,                          // bitMask
    XS_TestPatternForceError_OFFSET,                        // bitOffset
    AQUANTIA_TEST_PATTERN_FORCE_ERROR                       // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTestPatternMode7ForceError =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_TestPatternMode7ForceError_ADDR,                     // address
    XS_TestPatternMode7ForceError_MASK,                     // bitMask
    XS_TestPatternMode7ForceError_OFFSET,                   // bitOffset
    AQUANTIA_TEST_PATTERN_MODE_7_FORCE_ERROR                // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaXAUIRxLocalFaultInjection =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_XAUIRxLocalFaultInjection_ADDR,                      // address
    XS_XAUIRxLocalFaultInjection_MASK,                      // bitMask
    XS_XAUIRxLocalFaultInjection_OFFSET,                    // bitOffset
    AQUANTIA_XAUI_RX_LOCAL_FAULT_INJECTION                  // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTestPatternExtendedSelect =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_TestPatternExtendedSelect_ADDR,                      // address
    XS_TestPatternExtendedSelect_MASK,                      // bitMask
    XS_TestPatternExtendedSelect_OFFSET,                    // bitOffset
    AQUANTIA_TEST_PATTERN_EXTENDED_SELECT                   // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTestPatternCheckEnable =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_TestPatternCheckEnable_ADDR,                         // address
    XS_TestPatternCheckEnable_MASK,                         // bitMask
    XS_TestPatternCheckEnable_OFFSET,                       // bitOffset
    AQUANTIA_TEST_PATTERN_CHECK_ENABLE                      // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTestPatternCheckPoint =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_TestPatternCheckPoint_ADDR,                          // address
    XS_TestPatternCheckPoint_MASK,                          // bitMask
    XS_TestPatternCheckPoint_OFFSET,                        // bitOffset
    AQUANTIA_TEST_PATTERN_CHECK_POINT                       // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTestPatternInsertExtraIdles =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_TestPatternInsertExtraIdles_ADDR,                    // address
    XS_TestPatternInsertExtraIdles_MASK,                    // bitMask
    XS_TestPatternInsertExtraIdles_OFFSET,                  // bitOffset
    AQUANTIA_TEST_PATTERN_INSERT_EXTRA_IDLES                // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTestPatternCheckSelect =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_TestPatternCheckSelect_ADDR,                         // address
    XS_TestPatternCheckSelect_MASK,                         // bitMask
    XS_TestPatternCheckSelect_OFFSET,                       // bitOffset
    AQUANTIA_TEST_PATTERN_CHECK_SELECT                      // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTestPatternChannelSelect =
{

    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_TestPatternChannelSelect_ADDR,                       // address
    XS_TestPatternChannelSelect_MASK,                       // bitMask
    XS_TestPatternChannelSelect_OFFSET,                     // bitOffset
    AQUANTIA_TEST_PATTERN_CHANNEL_SELECT                    // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaChannel0TestPatternErrorCounter =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_Channel0TestPatternErrorCounter_ADDR,                // address
    XS_Channel0TestPatternErrorCounter_MASK,                // bitMask
    XS_Channel0TestPatternErrorCounter_OFFSET,              // bitOffset
    AQUANTIA_CHANNEL_0_TEST_PATTERN_ERROR_COUNTER           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaChannel1TestPatternErrorCounter =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_Channel1TestPatternErrorCounter_ADDR,                // address
    XS_Channel1TestPatternErrorCounter_MASK,                // bitMask
    XS_Channel1TestPatternErrorCounter_OFFSET,              // bitOffset
    AQUANTIA_CHANNEL_1_TEST_PATTERN_ERROR_COUNTER           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaChannel2TestPatternErrorCounter =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_Channel2TestPatternErrorCounter_ADDR,                // address
    XS_Channel2TestPatternErrorCounter_MASK,                // bitMask
    XS_Channel2TestPatternErrorCounter_OFFSET,              // bitOffset
    AQUANTIA_CHANNEL_2_TEST_PATTERN_ERROR_COUNTER           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaChannel3TestPatternErrorCounter =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_Channel3TestPatternErrorCounter_ADDR,                // address
    XS_Channel3TestPatternErrorCounter_MASK,                // bitMask
    XS_Channel3TestPatternErrorCounter_OFFSET,              // bitOffset
    AQUANTIA_CHANNEL_3_TEST_PATTERN_ERROR_COUNTER           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTestPatternMode7ErrorCounter =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_TestPatternMode7ErrorCounter_ADDR,                   // address
    XS_TestPatternMode7ErrorCounter_MASK,                   // bitMask
    XS_TestPatternMode7ErrorCounter_OFFSET,                 // bitOffset
    AQUANTIA_TEST_PATTERN_MODE_7_ERROR_COUNTER              // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaXauiTxErrorInjectionLaneSelect =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_XauiTxErrorInjectionLaneSelect_ADDR,                 // address
    XS_XauiTxErrorInjectionLaneSelect_MASK,                 // bitMask
    XS_XauiTxErrorInjectionLaneSelect_OFFSET,               // bitOffset
    AQUANTIA_XAUI_TX_ERROR_INJECTION_LANE_SELECT            // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaXauiTxInjectSynchronizationError =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_XauiTxInjectSynchronizationError_ADDR,               // address
    XS_XauiTxInjectSynchronizationError_MASK,               // bitMask
    XS_XauiTxInjectSynchronizationError_OFFSET,             // bitOffset
    AQUANTIA_XAUI_TX_INJECT_SYNCHRONIZATION_ERROR           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaXauiTxInjectAlignmentError =
{

    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_XauiTxInjectAlignmentError_ADDR,                     // address
    XS_XauiTxInjectAlignmentError_MASK,                     // bitMask
    XS_XauiTxInjectAlignmentError_OFFSET,                   // bitOffset
    AQUANTIA_XAUI_TX_INJECT_ALIGNMENT_ERROR                 // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaXauiTxInjectCodeViolation =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_XauiTxInjectCodeViolation_ADDR,                      // address
    XS_XauiTxInjectCodeViolation_MASK,                      // bitMask
    XS_XauiTxInjectCodeViolation_OFFSET,                    // bitOffset
    AQUANTIA_XAUI_TX_INJECT_CODE_VIOLATION                  // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaXauiTx10BViolationCodeword =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_XauiTx10BViolationCodeword_ADDR,                     // address
    XS_XauiTx10BViolationCodeword_MASK,                     // bitMask
    XS_XauiTx10BViolationCodeword_OFFSET,                   // bitOffset
    AQUANTIA_XAUI_TX_10B_VIOLATION_CODEWORD                 // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaPhyXsSystemLoopbackPassThrough =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_PhyXsSystemLoopbackPassThrough_ADDR,                 // address
    XS_PhyXsSystemLoopbackPassThrough_MASK,                 // bitMask
    XS_PhyXsSystemLoopbackPassThrough_OFFSET,               // bitOffset
    AQUANTIA_PHY_XS_SYSTEM_LOOPBACK_PASS_THROUGH            // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaPhyXsSystemLoopbackEnable =
{
    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_PhyXsSystemLoopbackEnable_ADDR,                      // address
    XS_PhyXsSystemLoopbackEnable_MASK,                      // bitMask
    XS_PhyXsSystemLoopbackEnable_OFFSET,                    // bitOffset
    AQUANTIA_PHY_XS_SYSTEM_LOOPBACK_ENABLE                  // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaXauiTxLocalFaultInjection =
{

    MDIO_DEVTYPE_PHY_XS,                                    // devType
    XS_XauiTxLocalFaultInjection_ADDR,                      // address
    XS_XauiTxLocalFaultInjection_MASK,                      // bitMask
    XS_XauiTxLocalFaultInjection_OFFSET,                    // bitOffset
    AQUANTIA_XAUI_TX_LOCAL_FAULT_INJECTION                  // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaRestartAutonegotiation =
{
    MDIO_DEVTYPE_AUTO_NEGO,                                        // devType
    AN_RestartAutonegotiation_ADDR,                         // address
    AN_RestartAutonegotiation_MASK,                         // bitMask
    AN_RestartAutonegotiation_OFFSET,                       // bitOffset
    AQUANTIA_RESTART_AUTONEGOTIATION                        // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaSerdesStartUpMode =
{
    MDIO_DEVTYPE_AUTO_NEGO,                                        // devType
    AN_SerdesStartUpMode_ADDR,                              // address
    AN_SerdesStartUpMode_MASK,                              // bitMask
    AN_SerdesStartUpMode_OFFSET,                            // bitOffset
    AQUANTIA_SERDES_START_UP_MODE                           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaAutonegotiationTimeout =
{
    MDIO_DEVTYPE_AUTO_NEGO,                                        // devType
    AN_AutonegotiationTimeout_ADDR,                         // address
    AN_AutonegotiationTimeout_MASK,                         // bitMask
    AN_AutonegotiationTimeout_OFFSET,                       // bitOffset
    AQUANTIA_AUTONEGOTIATION_TIMEOUT                        // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaAutonegotiationTimeoutMod =
{
    MDIO_DEVTYPE_AUTO_NEGO,                                        // devType
    AN_AutonegotiationTimeoutMod_ADDR,                      // address
    AN_AutonegotiationTimeoutMod_MASK,                      // bitMask
    AN_AutonegotiationTimeoutMod_OFFSET,                    // bitOffset
    AQUANTIA_AUTONEGOTIATION_TIMEOUT_MOD                    // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaLinkPartner1000BaseTFullDuplexAbility =
{
    MDIO_DEVTYPE_AUTO_NEGO,                                        // devType
    AN_LinkPartner1000BaseTFullDuplexAbility_ADDR,          // address
    AN_LinkPartner1000BaseTFullDuplexAbility_MASK,          // bitMask
    AN_LinkPartner1000BaseTFullDuplexAbility_OFFSET,        // bitOffset
    AQUANTIA_LINK_PARTNER_1000_BASE_T_FULL_DUPLEX_ABILITY   // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaLinkPartner1000BaseTHalfDuplexAbility =
{
    MDIO_DEVTYPE_AUTO_NEGO,                                        // devType
    AN_LinkPartner1000BaseTHalfDuplexAbility_ADDR,          // address
    AN_LinkPartner1000BaseTHalfDuplexAbility_MASK,          // bitMask
    AN_LinkPartner1000BaseTHalfDuplexAbility_OFFSET,        // bitOffset
    AQUANTIA_LINK_PARTNER_1000_BASE_T_HALF_DUPLEX_ABILITY   // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaLinkPartnerShortReach =
{
    MDIO_DEVTYPE_AUTO_NEGO,                                        // devType
    AN_LinkPartnerShortReach_ADDR,                          // address
    AN_LinkPartnerShortReach_MASK,                          // bitMask
    AN_LinkPartnerShortReach_OFFSET,                        // bitOffset
    AQUANTIA_LINK_PARTNER_SHORT_REACH                       // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaLinkPartnerAqRateDownshiftCapability =
{
    MDIO_DEVTYPE_AUTO_NEGO,                                        // devType
    AN_LinkPartnerAqRateDownshiftCapability_ADDR,           // address
    AN_LinkPartnerAqRateDownshiftCapability_MASK,           // bitMask
    AN_LinkPartnerAqRateDownshiftCapability_OFFSET,         // bitOffset
    AQUANTIA_LINK_PARTNER_AQ_RATE_DOWNSHIFT_CAPABILITY      // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaLinkPartner5G =
{
    MDIO_DEVTYPE_AUTO_NEGO,                                        // devType
    AN_LinkPartner5G_ADDR,                                  // address
    AN_LinkPartner5G_MASK,                                  // bitMask
    AN_LinkPartner5G_OFFSET,                                // bitOffset
    AQUANTIA_LINK_PARTNER_5G                                // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaLinkPartner2G =
{
    MDIO_DEVTYPE_AUTO_NEGO,                                        // devType
    AN_LinkPartner2G_ADDR,                                  // address
    AN_LinkPartner2G_MASK,                                  // bitMask
    AN_LinkPartner2G_OFFSET,                                // bitOffset
    AQUANTIA_LINK_PARTNER_2G                                // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaLinkPartner =
{
    MDIO_DEVTYPE_AUTO_NEGO,                                        // devType
    AN_LinkPartner_ADDR,                                    // address
    AN_LinkPartner_MASK,                                    // bitMask
    AN_LinkPartner_OFFSET,                                  // bitOffset
    AQUANTIA_LINK_PARTNER                                   // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaAutonegotiationProtocolErrorState =
{
    MDIO_DEVTYPE_AUTO_NEGO,                                        // devType
    AN_AutonegotiationProtocolErrorState_ADDR,              // address
    AN_AutonegotiationProtocolErrorState_MASK,              // bitMask
    AN_AutonegotiationProtocolErrorState_OFFSET,            // bitOffset
    AQUANTIA_AUTONEGOTIATION_PROTOCOL_ERROR_STATE           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaFlpIdleErrorState =
{
    MDIO_DEVTYPE_AUTO_NEGO,                                        // devType
    AN_FlpIdleErrorState_ADDR,                              // address
    AN_FlpIdleErrorState_MASK,                              // bitMask
    AN_FlpIdleErrorState_OFFSET,                            // bitOffset
    AQUANTIA_FLP_IDLE_ERROR_STATE                           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaEnableDiagnostics =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_EnableDiagnostics_ADDR,                             // address
    GLB_EnableDiagnostics_MASK,                             // bitMask
    GLB_EnableDiagnostics_OFFSET,                           // bitOffset
    AQUANTIA_ENABLE_DIAGNOSTICS                             // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaHighTempFailureThreshold =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_HighTempFailureThreshold_ADDR,                      // address
    GLB_HighTempFailureThreshold_MASK,                      // bitMask
    GLB_HighTempFailureThreshold_OFFSET,                    // bitOffset
    AQUANTIA_HIGH_TEMP_FAILURE_THRESHOLD                    // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaLowTempFailureThreshold =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_LowTempFailureThreshold_ADDR,                       // address
    GLB_LowTempFailureThreshold_MASK,                       // bitMask
    GLB_LowTempFailureThreshold_OFFSET,                     // bitOffset
    AQUANTIA_LOW_TEMP_FAILURE_THRESHOLD                     // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaHighTempWarningThreshold =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_HighTempWarningThreshold_ADDR,                      // address
    GLB_HighTempWarningThreshold_MASK,                      // bitMask
    GLB_HighTempWarningThreshold_OFFSET,                    // bitOffset
    AQUANTIA_HIGH_TEMP_WARNING_THRESHOLD                    // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaLowTempWarningThreshold =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_LowTempWarningThreshold_ADDR,                       // address
    GLB_LowTempWarningThreshold_MASK,                       // bitMask
    GLB_LowTempWarningThreshold_OFFSET,                     // bitOffset
    AQUANTIA_LOW_TEMP_WARNING_THRESHOLD                     // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaDiagnosticsSelect =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_DiagnosticsSelect_ADDR,                             // address
    GLB_DiagnosticsSelect_MASK,                             // bitMask
    GLB_DiagnosticsSelect_OFFSET,                           // bitOffset
    AQUANTIA_DIAGNOSTICS_SELECT                             // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaExtendedMdiDiagnosticsSelect =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_ExtendedMdiDiagnosticsSelect_ADDR,                  // addres
    GLB_ExtendedMdiDiagnosticsSelect_MASK,                  // bitMask
    GLB_ExtendedMdiDiagnosticsSelect_OFFSET,                // bitOffset
    AQUANTIA_EXTENDED_MDI_DIAGNOSTICS_SELECT                // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaInitiateCableDiagnostics =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_InitiateCableDiagnostics_ADDR,                      // address
    GLB_InitiateCableDiagnostics_MASK,                      // bitMask
    GLB_InitiateCableDiagnostics_OFFSET,                    // bitOffset
    AQUANTIA_INITIATE_CABLE_DIAGNOSTICS                     // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaEnableVddPowerSupplyTuning =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_EnableVddPowerSupplyTuning_ADDR,                    // address
    GLB_EnableVddPowerSupplyTuning_MASK,                    // bitMask
    GLB_EnableVddPowerSupplyTuning_OFFSET,                  // bitOffset
    AQUANTIA_ENABLE_VDD_POWER_SUPPLY_TUNING                 // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTunableExternalVddPowerSupplyPresent =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_TunableExternalVddPowerSupplyPresent_ADDR,          // address
    GLB_TunableExternalVddPowerSupplyPresent_MASK,          // bitMask
    GLB_TunableExternalVddPowerSupplyPresent_OFFSET,        // bitOffset
    AQUANTIA_TUNABLE_EXTERNAL_VDD_POWER_SUPPLY_PRESENT      // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaExternalVddChangeRequest =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_ExternalVddChangeRequest_ADDR,                      // address
    GLB_ExternalVddChangeRequest_MASK,                      // bitMask
    GLB_ExternalVddChangeRequest_OFFSET,                    // bitOffset
    AQUANTIA_EXTERNAL_VDD_CHANGE_REQUEST                    // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaEnable5ChannelRfiCancellation =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_Enable5ChannelRfiCancellation_ADDR,                 // address
    GLB_Enable5ChannelRfiCancellation_MASK,                 // bitMask
    GLB_Enable5ChannelRfiCancellation_OFFSET,               // bitOffset
    AQUANTIA_ENABLE_5TH_CHANNEL_RFI_CANCELLATION            // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaRateTransitionRequest =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_RateTransitionRequest_ADDR,                         // address
    GLB_RateTransitionRequest_MASK,                         // bitMask
    GLB_RateTransitionRequest_OFFSET,                       // bitOffset
    AQUANTIA_RATE_TRANSITION_REQUEST                        // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaTrainingSnr =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_TrainingSnr_ADDR,                                   // address
    GLB_TrainingSnr_MASK,                                   // bitMask
    GLB_TrainingSnr_OFFSET,                                 // bitOffset
    AQUANTIA_TRAINING_SNR                                   // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaGlbLoopbackControl =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_GlbLoopbackControl_ADDR,                            // address
    GLB_GlbLoopbackControl_MASK,                            // bitMask
    GLB_GlbLoopbackControl_OFFSET,                          // bitOffset
    AQUANTIA_GLB_LOOPBACK_CONTROL                           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaGlbMdiPacketGeneration =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_GlbMdiPacketGeneration_ADDR,                        // address
    GLB_GlbMdiPacketGeneration_MASK,                        // bitMask
    GLB_GlbMdiPacketGeneration_OFFSET,                      // bitOffset
    AQUANTIA_GLB_MDI_PACKET_GENERATION                      // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaGlbSystemIFPacketGeneration =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_SystemIFPacketGeneration_ADDR,                      // address
    GLB_SystemIFPacketGeneration_MASK,                      // bitMask
    GLB_SystemIFPacketGeneration_OFFSET,                    // bitOffset
    AQUANTIA_GLB_SYSTEM_IF_PACKET_GENERATION                // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaGlobalReservedProvisioningRate =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_GlobalReservedProvisioningRate_ADDR,                // address
    GLB_GlobalReservedProvisioningRate_MASK,                // bitMask
    GLB_GlobalReservedProvisioningRate_OFFSET,              // bitOffset
    AQUANTIA_GLOBAL_RESERVED_PROVISIONING_RATE              // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaPairAStatus =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_PairAStatus_ADDR,                                   // address
    GLB_PairAStatus_MASK,                                   // bitMask
    GLB_PairAStatus_OFFSET,                                 // bitOffset
    AQUANTIA_PAIR_A_STATUS                                  // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaPairBStatus =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_PairBStatus_ADDR,                                   // address
    GLB_PairBStatus_MASK,                                   // bitMask
    GLB_PairBStatus_OFFSET,                                 // bitOffset
    AQUANTIA_PAIR_B_STATUS                                  // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaPairCStatus =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_PairCStatus_ADDR,                                   // address
    GLB_PairCStatus_MASK,                                   // bitMask
    GLB_PairCStatus_OFFSET,                                 // bitOffset
    AQUANTIA_PAIR_C_STATUS                                  // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaPairDStatus =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_PairDStatus_ADDR,                                   // address
    GLB_PairDStatus_MASK,                                   // bitMask
    GLB_PairDStatus_OFFSET,                                 // bitOffset
    AQUANTIA_PAIR_D_STATUS                                  // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaStatusPairAReflection1 =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_StatusPairAReflection1_ADDR,                        // address
    GLB_StatusPairAReflection1_MASK,                        // bitMask
    GLB_StatusPairAReflection1_OFFSET,                      // bitOffset
    AQUANTIA_STATUS_PAIR_A_REFLECTION_1                     // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaStatusPairAReflection2 =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_StatusPairAReflection2_ADDR,                        // address
    GLB_StatusPairAReflection2_MASK,                        // bitMask
    GLB_StatusPairAReflection2_OFFSET,                      // bitOffset
    AQUANTIA_STATUS_PAIR_A_REFLECTION_2                     // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpulseResponseMsw =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_ImpulseResponseMsw_ADDR,                            // address
    GLB_ImpulseResponseMsw_MASK,                            // bitMask
    GLB_ImpulseResponseMsw_OFFSET,                          // bitOffset
    AQUANTIA_IMPULSE_RESPONSE_MSW                           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaStatusPairBReflection1 =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_StatusPairBReflection1_ADDR,                        // address
    GLB_StatusPairBReflection1_MASK,                        // bitMask
    GLB_StatusPairBReflection1_OFFSET,                      // bitOffset
    AQUANTIA_STATUS_PAIR_B_REFLECTION_1                     // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaStatusPairBReflection2 =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_StatusPairBReflection2_ADDR,                        // address
    GLB_StatusPairBReflection2_MASK,                        // bitMask
    GLB_StatusPairBReflection2_OFFSET,                      // bitOffset
    AQUANTIA_STATUS_PAIR_B_REFLECTION_2                     // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpulseResponseLsw =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_ImpulseResponseLsw_ADDR,                            // address
    GLB_ImpulseResponseLsw_MASK,                            // bitMask
    GLB_ImpulseResponseLsw_OFFSET,                          // bitOffset
    AQUANTIA_IMPULSE_RESPONSE_LSW                           // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaStatusPairCReflection1 =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_StatusPairCReflection1_ADDR,                        // address
    GLB_StatusPairCReflection1_MASK,                        // bitMask
    GLB_StatusPairCReflection1_OFFSET,                      // bitOffset
    AQUANTIA_STATUS_PAIR_C_REFLECTION_1                     // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaStatusPairCReflection2 =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_StatusPairCReflection2_ADDR,                        // address
    GLB_StatusPairCReflection2_MASK,                        // bitMask
    GLB_StatusPairCReflection2_OFFSET,                      // bitOffset
    AQUANTIA_STATUS_PAIR_C_REFLECTION_2                     // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaStatusPairDReflection1 =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_StatusPairDReflection1_ADDR,                        // address
    GLB_StatusPairDReflection1_MASK,                        // bitMask
    GLB_StatusPairDReflection1_OFFSET,                      // bitOffset
    AQUANTIA_STATUS_PAIR_D_REFLECTION_1                     // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaStatusPairDReflection2 =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_StatusPairDReflection2_ADDR,                        // addres
    GLB_StatusPairDReflection2_MASK,                        // bitMask
    GLB_StatusPairDReflection2_OFFSET,                      // bitOffset
    AQUANTIA_STATUS_PAIR_D_REFLECTION_2                     // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaProcessorIntensiveOperationInProgress =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_ProcessorIntensiveOperationInProgress_ADDR,         // address
    GLB_ProcessorIntensiveOperationInProgress_MASK,         // bitMask
    GLB_ProcessorIntensiveOperationInProgress_OFFSET,       // bitOffset
    AQUANTIA_PROCESSOR_INTENSIVE_OPERATION_IN_PROGRESS      // iLogIndex
};

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairAReflection1 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairAReflection1_ADDR,                     // address
//     GLB_ImpedencePairAReflection1_MASK,                     // bitMask
//     GLB_ImpedencePairAReflection1_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_A_REFLECTION_1                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairAReflection2 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairAReflection2_ADDR,                     // address
//     GLB_ImpedencePairAReflection2_MASK,                     // bitMask
//     GLB_ImpedencePairAReflection2_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_A_REFLECTION_2                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairAReflection3 =
// {

//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairAReflection3_ADDR,                     // address
//     GLB_ImpedencePairAReflection3_MASK,                     // bitMask
//     GLB_ImpedencePairAReflection3_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_A_REFLECTION_3                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairAReflection4 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairAReflection4_ADDR,                     // address
//     GLB_ImpedencePairAReflection4_MASK,                     // bitMask
//     GLB_ImpedencePairAReflection4_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_A_REFLECTION_4                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairBReflection1 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairBReflection1_ADDR,                     // address
//     GLB_ImpedencePairBReflection1_MASK,                     // bitMask
//     GLB_ImpedencePairBReflection1_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_B_REFLECTION_1                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairBReflection2 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairBReflection2_ADDR,                     // address
//     GLB_ImpedencePairBReflection2_MASK,                     // bitMask
//     GLB_ImpedencePairBReflection2_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_B_REFLECTION_2                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairBReflection3 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairBReflection3_ADDR,                     // address
//     GLB_ImpedencePairBReflection3_MASK,                     // bitMask
//     GLB_ImpedencePairBReflection3_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_B_REFLECTION_3                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairBReflection4 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairBReflection4_ADDR,                     // address
//     GLB_ImpedencePairBReflection4_MASK,                     // bitMask
//     GLB_ImpedencePairBReflection4_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_B_REFLECTION_4                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairCReflection1 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairCReflection1_ADDR,                     // address
//     GLB_ImpedencePairCReflection1_MASK,                     // bitMask
//     GLB_ImpedencePairCReflection1_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_C_REFLECTION_1                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairCReflection2 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairCReflection2_ADDR,                     // address
//     GLB_ImpedencePairCReflection2_MASK,                     // bitMask
//     GLB_ImpedencePairCReflection2_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_C_REFLECTION_2                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairCReflection3 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairCReflection3_ADDR,                     // address
//     GLB_ImpedencePairCReflection3_MASK,                     // bitMask
//     GLB_ImpedencePairCReflection3_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_C_REFLECTION_3                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairCReflection4 =
// {

//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairCReflection4_ADDR,                     // address
//     GLB_ImpedencePairCReflection4_MASK,                     // bitMask
//     GLB_ImpedencePairCReflection4_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_C_REFLECTION_4                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairDReflection1 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairDReflection1_ADDR,                     // address
//     GLB_ImpedencePairDReflection1_MASK,                     // bitMask
//     GLB_ImpedencePairDReflection1_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_D_REFLECTION_1                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairDReflection2 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairDReflection2_ADDR,                     // address
//     GLB_ImpedencePairDReflection2_MASK,                     // bitMask
//     GLB_ImpedencePairDReflection2_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_D_REFLECTION_2                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairDReflection3 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairDReflection3_ADDR,                     // address
//     GLB_ImpedencePairDReflection3_MASK,                     // bitMask
//     GLB_ImpedencePairDReflection3_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_D_REFLECTION_3                  // iLogIndex
// };

// static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaImpedancePairDReflection4 =
// {
//     MDIO_DEVTYPE_GLOBAL,                                       // devType
//     GLB_ImpedencePairDReflection4_ADDR,                     // address
//     GLB_ImpedencePairDReflection4_MASK,                     // bitMask
//     GLB_ImpedencePairDReflection4_OFFSET,                   // bitOffset
//     AQUANTIA_IMPEDENCE_PAIR_D_REFLECTION_4                  // iLogIndex
// };

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaCableLength =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_CableLength_ADDR,                                   // address
    GLB_CableLength4_MASK,                                  // bitMask
    GLB_CableLength_OFFSET,                                 // bitOffset
    AQUANTIA_CABLE_LENGTH                                   // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaGlbLoopbackStatus =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_GlbLoopbackStatus_ADDR,                             // address
    GLB_GlbLoopbackStatus_MASK,                             // bitMask
    GLB_GlbLoopbackStatus_OFFSET,                           // bitOffset
    AQUANTIA_GLB_LOOPBACK_STATUS                            // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaMdiPacketGenerationStatus =
{

    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_MdiPacketGenerationStatus_ADDR,                     // address
    GLB_MdiPacketGenerationStatus_MASK,                     // bitMask
    GLB_MdiPacketGenerationStatus_OFFSET,                   // bitOffset
    AQUANTIA_MDI_PACKET_GENERATION_STATUS                   // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaGlbSystemIFPacketGenerationStatus =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_SystemIFPacketGenerationStatus_ADDR,                // address
    GLB_SystemIFPacketGenerationStatus_MASK,                // bitMask
    GLB_SystemIFPacketGenerationStatus_OFFSET,              // bitOffset
    AQUANTIA_GLB_SYSTEM_IF_PACKET_GENERATION_STATUS         // iLogIndex
};

static const AquantiaBitFieldReadWrite __attribute__((section(".flashrodata"))) aquantiaGlobalReservedStatusRate =
{
    MDIO_DEVTYPE_GLOBAL,                                       // devType
    GLB_GlobalReservedStatusRate_ADDR,                      // address
    GLB_GlobalReservedStatusRate_MASK,                      // bitMask
    GLB_GlobalReservedStatusRate_OFFSET,                    // bitOffset
    AQUANTIA_GLOBAL_RESERVED_STATUS_RATE                    // iLogIndex
};

static AquantiaRegister aquantiaGeneralRegister;            // For general read/write register

// Exported Function Definitions ##################################################################
//#################################################################################################
// Issue MDIO request to read current core temperature
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaReadJunctionTempIcmd(void)
{
    MDIOD_aquantiaReadJunctionTemp();
}

//#################################################################################################
// MDIOD_aquantiaDblRdTest
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaDblRdTest(uint8_t dev_type, uint16_t reg1, uint16_t reg2)
{
    uint16_t data1 = MdioIndirectReadSync(
        AQUANTIA_PHY_ADDR,
        dev_type,
        reg1,
        AQUANTIA_MUXPORT);
    uint16_t data2 = MdioIndirectReadSync(
        AQUANTIA_PHY_ADDR,
        dev_type,
        reg2,
        AQUANTIA_MUXPORT);

    ilog_AQUANTIA_COMPONENT_2(ILOG_USER_LOG, AQUANTIA_DBL_READ, data1, data2);
}


//#################################################################################################
// Icmd stop generating test packet
//
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_AquantiaStopTestPackets()
{
    uint16_t data = 0x0000;

    MdioIndirectWriteSync(
        AQUANTIA_PHY_ADDR,
        MDIO_DEVTYPE_PHY_XS,
        PHY_XS_TRANSMIT_RESERVED_VENDOR_PROVISIONING5,
        data,
        AQUANTIA_MUXPORT);
}

//#################################################################################################
// Read/Write Short Reach Mode(1.83.0)
//
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaShortReachMode(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaShortReachMode, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write Test Mode Control (1.84.F:D)
//
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTestModeControl(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTestModeControl, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write PMA_TransmitterTestFrequencies (1.84.F:D)
//
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTransmitterTestFrequencies(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTransmitterTestFrequencies, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write PMA_FastRetrainAbility (1.93.4)
//
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaFastRetrainAbility(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaFastRetrainAbility, false, 0);
}

//#################################################################################################
// Read/Write PMA_FastRetrainEnable (1.93.0)
//
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaFastRetrainEnable(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaFastRetrainEnable, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write PMA_TestModeRate(1.C412.F:E)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTestModeRate(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTestModeRate, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write PMA_PmaDigitalSystemLoopback(1.D800.F)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaPmaDigitalSystemLoopback(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaPmaDigitalSystemLoopback, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write PMA_ExternalPhyLoopback(1.E400.F)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaExternalPhyLoopback(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaExternalPhyLoopback, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write PMA_EnableFastRetrain(1.E400.2)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaEnableFastRetrain(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaEnableFastRetrain, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write PMA_ForceMdiConfiguration(1.E400.1)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaForceMdiConfiguration(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaForceMdiConfiguration, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write PMA_MdiConfiguration(1.E400.0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaMdiConfiguration(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaMdiConfiguration, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write PCS_Loopback(3.0.E)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaPcsLoopback(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaPcsLoopback, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write PCS_10GSpeedSelection(3.0.5:2)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantia10GSpeedSelection(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantia10GSpeedSelection, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write PCS_TxScramblerDisable(3.D800.F)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTxScramblerDisable(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTxScramblerDisable, writeEnable, writeValue);

}

//#################################################################################################
// Read/Write PCS_TxInjectCrcError(3.D800.E)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTxInjectCrcError(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTxInjectCrcError, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write PCS_TxInjectFrameError(3.D800.D)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTxInjectFrameError(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTxInjectFrameError, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write PCS_RxErrorLdpcFrameEnable(03.E400.0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaRxErrorLdpcFrameEnable(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaRxErrorLdpcFrameEnable, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write PCS_RxLdpcDecoderControl(3.E400.F)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaRxLdpcDecoderControl(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaRxLdpcDecoderControl, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write Xs_Loopback(4.0.E)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaXsLoopback(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaXsLoopback, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write Xs_ReceiveTestPatternEnable(4.19.2)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaReceiveTestPatternEnable(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaReceiveTestPatternEnable, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write XS_PhyOperatingMode(4.C441.8:6)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaPhyOperatingMode(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaPhyOperatingMode, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_TestPatternSelect(4.19.1:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTestPatternSelect(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTestPatternSelect, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_LoopbackControl(4.C444.F:B)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaLoopbackControl(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaLoopbackControl, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write XS_MdiPacketGeneration(4.C444.5)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaMdiPacketGeneration(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaMdiPacketGeneration, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_SystemIFPacketGeneration(4.C444.2)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaXsSystemIFPacketGeneration(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaXsSystemIFPacketGeneration, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_Rate(4.C444.1:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaRate(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaRate, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write XS_TestPatternForceError(4.D800.F)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTestPatternForceError(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTestPatternForceError, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_TestPatternMode7ForceError(4.D800.E)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTestPatternMode7ForceError(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTestPatternMode7ForceError, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_XAUIRxLocalFaultInjection(4.D800.D)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaXAUIRxLocalFaultInjection(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaXAUIRxLocalFaultInjection, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_TestPatternExtendedSelect(4.D800.C:B)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTestPatternExtendedSelect(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTestPatternExtendedSelect, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_TestPatternCheckEnable(4.D800.A)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTestPatternCheckEnable(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTestPatternCheckEnable, writeEnable, writeValue);
}

//#################################################################################################
// Read/Write XS_TestPatternCheckPoint(4.D800.7)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTestPatternCheckPoint(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTestPatternCheckPoint, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_TestPatternInsertExtraIdles(4.D801.E:C)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTestPatternInsertExtraIdles(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTestPatternInsertExtraIdles, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_TestPatternCheckSelect(4.D801.B:8)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTestPatternCheckSelect(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTestPatternCheckSelect, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_TestPatternChannelSelect(4.D801.3:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTestPatternChannelSelect(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTestPatternChannelSelect, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_Channel0TestPatternErrorCounter(4.D810.F:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaChannel0TestPatternErrorCounter(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaChannel0TestPatternErrorCounter, false, 0);
}
//#################################################################################################
// Read/Write XS_Channel1TestPatternErrorCounter(4.D811.F:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaChannel1TestPatternErrorCounter(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaChannel1TestPatternErrorCounter, false, 0);
}
//#################################################################################################
// Read/Write XS_Channel2TestPatternErrorCounter(4.D812.F:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaChannel2TestPatternErrorCounter(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaChannel2TestPatternErrorCounter, false, 0);
}
//#################################################################################################
// Read/Write XS_Channel3TestPatternErrorCounter(4.D813.F:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaChannel3TestPatternErrorCounter(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaChannel3TestPatternErrorCounter, false, 0);
}
//#################################################################################################
// Read/Write XS_TestPatternMode7ErrorCounter(4.D814.F:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTestPatternMode7ErrorCounter(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaTestPatternMode7ErrorCounter, false, 0);
}
//#################################################################################################
// Read/Write XS_XauiTxErrorInjectionLaneSelect(4.F800.F:D)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaXauiTxErrorInjectionLaneSelect(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaXauiTxErrorInjectionLaneSelect, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_XauiTxInjectSynchronizationError(4.F800.C)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaXauiTxInjectSynchronizationError(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaXauiTxInjectSynchronizationError, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_XauiTxInjectAlignmentError(4.F800.B)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaXauiTxInjectAlignmentError(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaXauiTxInjectAlignmentError, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_XauiTxInjectCodeViolation(4.F800.A)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaXauiTxInjectCodeViolation(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaXauiTxInjectCodeViolation, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_XauiTx10BViolationCodeword(4.F800.9:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaXauiTx10BViolationCodeword(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaXauiTx10BViolationCodeword, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_PhyXsSystemLoopbackPassThrough(4.F802.F)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaPhyXsSystemLoopbackPassThrough(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaPhyXsSystemLoopbackPassThrough, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_PhyXsSystemLoopbackEnable(4.F802.E)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaPhyXsSystemLoopbackEnable(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaPhyXsSystemLoopbackEnable, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write XS_XauiTxLocalFaultInjection(4.F802.D)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaXauiTxLocalFaultInjection(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaXauiTxLocalFaultInjection, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write AN_RestartAutonegotiation(7.0.9)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaRestartAutonegotiation(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaRestartAutonegotiation, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write AN_SerdesStartUpMode(7.C410.F:D)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaSerdesStartUpMode(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaSerdesStartUpMode, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write AN_AutonegotiationTimeout(7.C411.F:C)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaAutonegotiationTimeout(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaAutonegotiationTimeout, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write AN_AutonegotiationTimeoutMod(7.C411.B)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaAutonegotiationTimeoutMod(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaAutonegotiationTimeoutMod, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write AN_LinkPartner1000BaseTFullDuplexAbility(7.E820.F)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaLinkPartner1000BaseTFullDuplexAbility(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaLinkPartner1000BaseTFullDuplexAbility, false, 0);
}
//#################################################################################################
// Read/Write AN_LinkPartner1000BaseTHalfDuplexAbility(7.E820.E)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaLinkPartner1000BaseTHalfDuplexAbility(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaLinkPartner1000BaseTHalfDuplexAbility, false, 0);
}
//#################################################################################################
// Read/Write AN_LinkPartnerShortReach(7.E820.D)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaLinkPartnerShortReach(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaLinkPartnerShortReach, false, 0);
}
//#################################################################################################
// Read/Write AN_LinkPartnerAqRateDownshiftCapability(7.E820.C)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantialLinkPartnerAqRateDownshiftCapability(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaLinkPartnerAqRateDownshiftCapability, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write AN_LinkPartner5G(7.E820.B)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaLinkPartner5G(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaLinkPartner5G, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write AN_LinkPartner2G(7.E820.A)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaLinkPartner2G(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaLinkPartner2G, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write AN_LinkPartner(7.E820.2)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaLinkPartner(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaLinkPartner, false, 0);
}
//#################################################################################################
// Read/Write AN_AutonegotiationProtocolErrorState(7.E831.D)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaAutonegotiationProtocolErrorState(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaAutonegotiationProtocolErrorState, false, 0);
}
//#################################################################################################
// Read/Write AN_FlpIdleErrorState(7.E831.C)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaFlpIdleErrorState(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaFlpIdleErrorState, false, 0);
}
//#################################################################################################
// Read/Write GLB_EnableDiagnostics(1E.C400.F)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaEnableDiagnostics(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaEnableDiagnostics, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_HighTempFailureThreshold(1E.C421)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaHighTempFailureThreshold(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaHighTempFailureThreshold, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_LowTempFailureThreshold(1E.C422)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaLowTempFailureThreshold(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaLowTempFailureThreshold, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_HighTempWarningThreshold(1E.C423)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaHighTempWarningThreshold(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaHighTempWarningThreshold, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_LowTempWarningThreshold(1E.C424)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaLowTempWarningThreshold(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaLowTempWarningThreshold, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_DiagnosticsSelect(1E.C470.F)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaDiagnosticsSelect(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaDiagnosticsSelect, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_ExtendedMdiDiagnosticsSelect(1E.C470.E:D)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaExtendedMdiDiagnosticsSelect(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaExtendedMdiDiagnosticsSelect, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_InitiateCableDiagnostics(1E.C470.4)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaInitiateCableDiagnostics(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaInitiateCableDiagnostics, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_EnableVddPowerSupplyTuning(1E.C472.E)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaEnableVddPowerSupplyTuning(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaEnableVddPowerSupplyTuning, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_TunableExternalVddPowerSupplyPresent(1E.C472.6)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTunableExternalVddPowerSupplyPresent(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTunableExternalVddPowerSupplyPresent, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_ExternalVddChangeRequest(1E.C472.5:2)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaExternalVddChangeRequest(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaExternalVddChangeRequest, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_Enable5ChannelRfiCancellation(1E.C472.0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaEnable5ChannelRfiCancellation(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaEnable5ChannelRfiCancellation, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_RateTransitionRequest(1E.C473.A:8)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaRateTransitionRequest(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaRateTransitionRequest, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_TrainingSnr(1E.C473.7:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaTrainingSnr(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaTrainingSnr, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_GlbLoopbackControl(1E.C47A.F:B)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaGlbLoopbackControl(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaGlbLoopbackControl, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_GlbMdiPacketGeneration(1E.C47A.5)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaGlbMdiPacketGeneration(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaGlbMdiPacketGeneration, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_SystemIFPacketGeneration(1E.C47A.3)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaGlbSystemIFPacketGeneration(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaGlbSystemIFPacketGeneration, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_GlobalReservedProvisioningRate(1E.C47A.2:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaGlobalReservedProvisioningRate(bool writeEnable, uint16_t writeValue)
{
    MDIOD_aquantiaReadWrite(&aquantiaGlobalReservedProvisioningRate, writeEnable, writeValue);
}
//#################################################################################################
// Read/Write GLB_PairAStatus(1E.C800.E:C)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaPairAStatus(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaPairAStatus, false, 0);
}
//#################################################################################################
// Read/Write GLB_PairBStatus(1E.C800.A:8)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaPairBStatus(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaPairBStatus, false, 0);
}
//#################################################################################################
// Read/Write GLB_PairCStatus(1E.C800.6:4)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaPairCStatus(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaPairCStatus, false, 0);
}
//#################################################################################################
// Read/Write GLB_PairDStatus(1E.C800.2:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaPairDStatus(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaPairDStatus, false, 0);
}
//#################################################################################################
// Read/Write GLB_StatusPairAReflection1(1E.C801.F:8)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaStatusPairAReflection1(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaStatusPairAReflection1, false, 0);
}
//#################################################################################################
// Read/Write GLB_StatusPairAReflection2(1E.C801.7:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaStatusPairAReflection2(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaStatusPairAReflection2, false, 0);
}
//#################################################################################################
// Read/Write GLB_ImpulseResponseMsw(1E.C802.F:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaImpulseResponseMsw(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaImpulseResponseMsw, false, 0);
}
//#################################################################################################
// Read/Write GLB_StatusPairBReflection1(1E.C803.F:8)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaStatusPairBReflection1(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaStatusPairBReflection1, false, 0);
}
//#################################################################################################
// Read/Write GLB_StatusPairBReflection2(1E.C803.7:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaStatusPairBReflection2(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaStatusPairBReflection2, false, 0);
}
//#################################################################################################
// Read/Write GLB_ImpulseResponseLsw(1E.C804.F:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaImpulseResponseLsw(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaImpulseResponseLsw, false, 0);
}
//#################################################################################################
// Read/Write GLB_StatusPairCReflection1(1E.C805.F:8)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaStatusPairCReflection1(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaStatusPairCReflection1, false, 0);
}
//#################################################################################################
// Read/Write GLB_StatusPairCReflection2(1E.C805.7:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaStatusPairCReflection2(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaStatusPairCReflection2, false, 0);
}

//#################################################################################################
// Read/Write GLB_StatusPairDReflection1(1E.C807.F:8)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaStatusPairDReflection1(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaStatusPairDReflection1, false, 0);
}
//#################################################################################################
// Read/Write GLB_StatusPairDReflection2(1E.C807.7:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaStatusPairDReflection2(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaStatusPairDReflection2, false, 0);
}

//#################################################################################################
// Read/Write GLB_ProcessorIntensiveOperationInProgress(1E.C831.F)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaProcessorIntensiveOperationInProgress(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaProcessorIntensiveOperationInProgress, false, 0);
}
//#################################################################################################
// Read/Write GLB_ImpedancePairAReflection1(1E.C880.E:C)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairAReflection1(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairAReflection1, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairAReflection2(1E.C880.A:8)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairAReflection2(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairAReflection2, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairAReflection3(1E.C880.6:4)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairAReflection3(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairAReflection3, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairAReflection4(1E.C880.2:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairAReflection4(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairAReflection4, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairBReflection1(1E.C881.E:C)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairBReflection1(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairBReflection1, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairBReflection2(1E.C881.A:8)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairBReflection2(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairBReflection2, false, 0);
// }

//#################################################################################################
// Read/Write GLB_ImpedancePairBReflection3(1E.C881.6:4)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairBReflection3(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairBReflection3, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairBReflection4(1E.C881.2:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairBReflection4(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairBReflection4, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairCReflection1(1E.C882.E:C)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairCReflection1(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairCReflection1, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairCReflection2(1E.C882.A:8)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairCReflection2(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairCReflection2, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairCReflection3(1E.C882.6:4)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairCReflection3(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairCReflection3, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairCReflection4(1E.C882.2:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairCReflection4(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairCReflection4, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairDReflection1(1E.C883.E:C)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairDReflection1(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairDReflection1, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairDReflection2(1E.C883.A:8)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairDReflection2(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairDReflection2, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairDReflection3(1E.C883.6:4)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairDReflection3(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairDReflection3, false, 0);
// }
//#################################################################################################
// Read/Write GLB_ImpedancePairDReflection4(1E.C883.2:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
// void MDIOD_aquantiaImpedancePairDReflection4(void)
// {
//     MDIOD_aquantiaReadWrite(&aquantiaImpedancePairDReflection4, false, 0);
// }
//#################################################################################################
// Read/Write GLB_CableLength(1E.C884.7:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaCableLength(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaCableLength, false, 0);
}
//#################################################################################################
// Read/Write GLB_GlbLoopbackStatus(1E.C888.F:B)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaGlbLoopbackStatus(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaGlbLoopbackStatus, false, 0);
}
//#################################################################################################
// Read/Write GLB_MdiPacketGenerationStatus(1E.C888.5)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaMdiPacketGenerationStatus(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaMdiPacketGenerationStatus, false, 0);
}
//#################################################################################################
// Read/Write GLB_SystemIFPacketGenerationStatus(1E.C888.3)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaGlbSystemIFPacketGenerationStatus(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaGlbSystemIFPacketGenerationStatus, false, 0);
}
//#################################################################################################
// Read/Write GLB_GlobalReservedStatusRate(1E.C888.2:0)
// Parameters: write or read, write value
// Return:
// Assumptions:
//#################################################################################################
void MDIOD_aquantiaGlobalReservedStatusRate(void)
{
    MDIOD_aquantiaReadWrite(&aquantiaGlobalReservedStatusRate, false, 0);
}
//#################################################################################################
// AQUANTIA_GeneralRead
// Parameters: device type, address
// Return:
// Assumptions:
//#################################################################################################
void AQUANTIA_GeneralRead(uint8_t devType, uint16_t address)
{
    aquantiaGeneralRegister.devType = devType;
    aquantiaGeneralRegister.address = address;

    AquantiaReadIndirectAsync(devType, address, AQUANTIA_ShowGeneralReadWriteResult);
}
//#################################################################################################
// AQUANTIA_GeneralWrite
// Parameters: device type, address, value
// Return:
// Assumptions:
//#################################################################################################
void AQUANTIA_GeneralWrite(uint8_t devType, uint16_t address, uint16_t value)
{
    aquantiaGeneralRegister.devType = devType;
    aquantiaGeneralRegister.address = address;

    MdioIndirectWriteASync(
        AQUANTIA_PHY_ADDR,
        devType,
        address,
        value,
        AQUANTIA_GeneralWriteHandler,
        AQUANTIA_MUXPORT);
}
//#################################################################################################
// Check Aquantia is in normal state
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
void AquantiaInNomalOperationIcmd(void)
{
    AquantiaInNomalOperation();
}


// Component Scope Function Definitions ###########################################################

// Static Function Definitions ####################################################################
//#################################################################################################
// AQUANTIA_GeneralWriteHandler
// Parameters:
// Return:
// Assumptions:
//#################################################################################################
static void AQUANTIA_GeneralWriteHandler(void)
{
    AQUANTIA_GeneralRead(aquantiaGeneralRegister.devType, aquantiaGeneralRegister.address);
}

//#################################################################################################
// AQUANTIA_ShowGeneralReadWriteResult
// Parameters: read result
// Return:
// Assumptions:
//#################################################################################################
static void AQUANTIA_ShowGeneralReadWriteResult(uint16_t value)
{
    ilog_AQUANTIA_COMPONENT_3(ILOG_USER_LOG, AQUANTIA_GENERAL_RW,
        aquantiaGeneralRegister.devType, aquantiaGeneralRegister.address, value);
}
#endif  //BB_PROGRAM_BB
