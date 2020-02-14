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
#ifndef MDIOD_SRC_MDIOD_AQUANTIA_REGS_H_
#define MDIOD_SRC_MDIOD_AQUANTIA_REGS_H_

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// General definition
#define AQUANTIA_NULL_ADDR                                  (0x0000)
#define AQUANTIA_NO_MASK                                    (0x0000)
#define AQUANTIA_NO_OFFSET                                  (0x0000)
#define AQUANTIA_ALL_DATA                                   (0xFFFF)    // use the data read - no masking

// REG Group Address / DeviceType in MDIO language
#define CORE_PHY_ADDR                                       (0x0)
#define AQUANTIA_PHY_ADDR                                   (0x4)
#define AQUANTIA_GLOBAL_REG_OFFSET                          (0x1E)
#define AQUANTIA_AUTO_NEG_REG_OFFSET                        (0x7)

// Initialize Registers ==========================================================================
// FW Loaded
#define PMA_STD_CTRL_1_OFFSET                               (0x0)
#define PMA_STD_CTRL_1_RST_MASK                             (0x8000)
// PHY out of reset
#define PMA_TX_VEND_ALM_3_OFFSET                            (0xCC02)
#define PMA_TX_VEND_ALM_3_RST_MASK                          (0x3)
// DUNE configuration
#define PHY_XS_TX_RSRVD_VEND_PROV2_OFFSET                   (0xC441)
#define PHY_XS_TX_RSRVD_VEND_PROV2_RXAUI_OP_MODE_MASK       (0x20)
// PHY junction temp
#define GLOBAL_THRML_STAT1_OFFSET                           (0xC820)
#define GLOBAL_THRML_STAT1_TEMP_MASK                        (0xFFFF)
#define GLOBAL_THRML_STAT1_TEMP_OFFSET                      (0)
// PHY junction temp read
#define GLOBAL_THRML_STAT2_OFFSET                           (0xC821)
#define GLOBAL_THRML_STAT2_TEMP_RDY_MASK                    (0x1)

// TEMP NOTES:
//  2's compliment, LSB reps 1/256 of DegC
//  -40C = 0xD800, default is 70C
//  take one's complement and add 1
//  0xD800 -> 0x27FF -> 0x2800 = 10240(10)
//  10240 / 256 = 40
//  AQR_BringUpGuide_0.6.pdf says max temp 105C
//
// High Temp Threshold
#define GLOBAL_THRML_PROV2_OFFSET                           (0xC421)
#define GLOBAL_THRML_PROV2_HIGH_TEMP_THRESH_MASK            (0xFFFF)
#define GLOBAL_THRML_PROV2_HIGH_TEMP_THRESH_OFFSET          (0)
#define GLOBAL_THRML_PROV2_HIGH_TEMP_THRESH                 (AQUANTIA_HW_CUT_HIGH_TEMPERATURE * 256)    // 108*256 = 27648 = 0x6C00
#define GLOBAL_THRML_PROV2_HIGH_TEMP_DEFAULT                (0x4600)    // Default value of Aquantia Reset status
// Low Temp Threshold
#define GLOBAL_THRML_PROV3_OFFSET                           (0xC422)
#define GLOBAL_THRML_PROV3_LOW_TEMP_THRESH_MASK             (0xFFFF)
#define GLOBAL_THRML_PROV3_LOW_TEMP_THRESH_OFFSET           (0)
#define GLOBAL_THRML_PROV3_LOW_TEMP_THRESH                  (AQUANTIA_HW_CUT_LOW_TEMPERATURE * 256)     // 0*256 = 0 = 0x0000
// High Temp Warning
#define GLOBAL_THRML_PROV4_OFFSET                           (0xC423)
#define GLOBAL_THRML_PROV4_HIGH_TEMP_WARN_MASK              (0xFFFF)
#define GLOBAL_THRML_PROV4_HIGH_TEMP_WARN_OFFSET            (0)
#define GLOBAL_THRML_PROV4_HIGH_TEMP_WARN                   (AQUANTIA_HW_WARN_HIGH_TEMPERATURE * 256)   // 98*256 = 25088 = 0x6200
// Low Temp Threshold
#define GLOBAL_THRML_PROV5_OFFSET                           (0xC424)
#define GLOBAL_THRML_PROV5_LOW_TEMP_WARN_MASK               (0xFFFF)
#define GLOBAL_THRML_PROV5_LOW_TEMP_WARN_OFFSET             (0)
#define GLOBAL_THRML_PROV5_LOW_TEMP_WARN                    (AQUANTIA_HW_WARN_LOW_TEMPERATURE * 256)    // 10*256 = 2560 = 0xA00
// Thermal Shutdown select to High temperature
#define GLOBAL_THRML_PROV6_OFFSET                           (0xC475)    // Global Reserved Provisioning 6: Address 1E.C475
#define GLOBAL_THRML_PROV6_HI_THRML_SHUTDOWN_MASK           (0xC000)    // [F:E]
// #define GLOBAL_THRML_PROV6_HI_THRML_SHUTDOWN_OFFSET         (0x0E)
// #define GLOBAL_THRML_PROV6_HI_THRML_SHUTDOWN                (0x02)      // Selects highTempFailureThreshold as the shutdown
#define GLOBAL_THRML_PROV6_HI_THRML_SHUTDOWN                (0x8000)    // Selects highTempFailureThreshold as the shutdown
// Thermal Shutdown enable
#define GLOBAL_THRML_PROV9_OFFSET                           (0xC478)    // Global Reserved Provisioning 9: Address 1E.C478
#define GLOBAL_THRML_PROV9_THRML_SHUTDOWN_MASK              (0x0400)    // A :Thermal Shutdown Enable
// #define GLOBAL_THRML_PROV9_THRML_SHUTDOWN_OFFSET            (10)
#define GLOBAL_THRML_PROV9_THRML_SHUTDOWN                   (0x0400)

// Temperature Alarm States
#define GLOBAL_ALARMS1_OFFSET                               (0xCC00)
#define GLOBAL_GNRL_STAT1_HIGH_TEMP_FAIL_MASK               (0x4000)
#define GLOBAL_GNRL_STAT1_HIGH_TEMP_FAIL_OFFSET             (14)
#define GLOBAL_GNRL_STAT1_LOW_TEMP_FAIL_MASK                (0x2000)
#define GLOBAL_GNRL_STAT1_LOW_TEMP_FAIL_OFFSET              (13)
#define GLOBAL_GNRL_STAT1_HIGH_TEMP_WARN_MASK               (0x1000)
#define GLOBAL_GNRL_STAT1_HIGH_TEMP_WARN_OFFSET             (12)
#define GLOBAL_GNRL_STAT1_LOW_TEMP_WARN_MASK                (0x800)
#define GLOBAL_GNRL_STAT1_LOW_TEMP_WARN_OFFSET              (11)

// Temperature Alarm Interrupt Mask
#define GLOBAL_INT_MASK_OFFSET                              (0xD400)
#define GLOBAL_INT_MASK_HIGH_TEMP_FAIL_MASK                 (0x4000)
#define GLOBAL_INT_MASK_HIGH_TEMP_FAIL_OFFSET               (14)
#define GLOBAL_INT_MASK_LOW_TEMP_FAIL_MASK                  (0x2000)
#define GLOBAL_INT_MASK_LOW_TEMP_FAIL_OFFSET                (13)
#define GLOBAL_INT_MASK_HIGH_TEMP_WARN_MASK                 (0x1000)
#define GLOBAL_INT_MASK_HIGH_TEMP_WARN_OFFSET               (12)
#define GLOBAL_INT_MASK_LOW_TEMP_WARN_MASK                  (0x800)
#define GLOBAL_INT_MASK_LOW_TEMP_WARN_OFFSET                (11)
#define GLOBAL_INT_MASK_TEMP_MASK                           (GLOBAL_INT_MASK_LOW_TEMP_WARN_MASK | GLOBAL_INT_MASK_HIGH_TEMP_WARN_MASK | GLOBAL_INT_MASK_LOW_TEMP_FAIL_MASK | GLOBAL_INT_MASK_HIGH_TEMP_FAIL_MASK)

// Global Chip Wide Interrupt Mask
#define GLOBAL_INT_CHIP_WIDE_INT_MASK_OFFSET                (0xFF00)

// Global Chip Wide Interrupt Flags
#define GLOBAL_INT_CHIP_WIDE_INT_FLAGS_OFFSET               (0xFC00)
#define GLOBAL_INT_CHIP_WIDE_INT_FLAGS_BIT0_MASK            (0x1)
#define GLOBAL_INT_CHIP_WIDE_INT_FLAGS_BIT0_OFFSET          (0)

// Global Chip Wide Vendor Interrupt Flags
#define GLOBAL_INT_CHIP_WIDE_VEND_FLAG_OFFSET               (0xFC01)

// Global Chip Wide interrupt mask
#define GLOBAL_INT_CHIP_WIDE_VEND_MASK_OFFSET               (0xFF01)
#define GLOBAL_INT_CHIP_WIDE_VEND_GLBL_ALM1_INT_MSK         (0x0004)
#define GLOBAL_INT_CHIP_WIDE_VEND_GLBL_ALM1_INT_OFFSET      (2)

// Global general status (1E.c831)
#define GLOBAL_GNRL_STATUS_2_REG                            (0xC831)
#define GLOBAL_GNRL_STATUS_2_PROCESSOR_BUSY_MASK            (0x8000)  // if this bit is set, the processor is busy

// LEDs
#define GLOBAL_LED0_OFFSET                                  (0xC430)
#define GLOBAL_LED0_VALUE                                   (0x000B)
#define GLOBAL_LED1_OFFSET                                  (0xC431)
#define GLOBAL_LED1_VALUE                                   (0xC0E7)
#define GLOBAL_LED2_OFFSET                                  (0xC432)
#define GLOBAL_LED2_VALUE                                   (0xC073)

// 10G to 5G
#define AUTO_NEG_STD_CTRL_REG                               (0x0000)
#define AUTO_NEG_10G_BASET_REG                              (0x0020)
#define AUTO_NEG_10G_BASET_VENDOR_STATUS_REG                (0xC800)
#define AUTO_NEG_10G_BASET_VENDOR_STATUS_CONN_RATE_MASK     (0x000E)
#define AUTO_NEG_10G_BASET_VENDOR_STATUS_10G_RATE_MASK      (0x0006)
#define AUTO_NEG_10G_BASET_VENDOR_STATUS_10G_RATE           (0x0006)
#define AUTO_NEG_10G_BASET_VENDOR_STATUS_5G_RATE_MASK       (0x000A)
#define AUTO_NEG_10G_BASET_VENDOR_STATUS_5G_RATE            (0x000A)
#define AUTO_NEG_10G_BASET_REG_10G_ABILITY_MASK             (0x1000)
#define AUTO_NEG_10G_BASET_REG_10G_ABILITY                  (0x0000)//(0x1000)

#define AUTO_NEG_STD_CTRL_EXTENDED_PAGES_ENABLE             (0x2000)
#define AUTO_NEG_STD_CTRL_AUTO_NEGOTIATION_ENABLE           (0x1000)
#define AUTO_NEG_STD_CTRL_RESTART_LINK_TRAINING             (0x0200)
#define AUTO_NEG_STD_CTRL_MASK                              (0xB200)
#define AUTO_NEG_VEND_STAT1_OFFSET                          (0xC810)
#define AUTO_NEG_VEND_STAT1_CONNECTION_MASK                 (0x3E00)
#define AUTO_NEG_VEND_STAT1_CONNECTION_OFFSET               (0x9)
#define AUTO_NEG_VEND_STAT1_MDIX_MASK                       (0x0100)
#define AUTO_NEG_VEND_STAT1_MDIX_OFFSET                     (0x8)

// VERSION
#define FIRMWARE_VERSION_ID                                 (0xC41D)
#define FIRMWARE_VERSION_VER                                (0xC41E)
#define GLOBAL_FIRMWARE_ID                                  (0x0020)
#define GLOBAL_RESERVED_STATUS                              (0xC885)
#define FIRMWARE_MAJOR_MASK                                 (0xFF00)
#define FIRMWARE_MAJOR_OFFSET                               (8)
#define FIRMWARE_MINOR_MASK                                 (0x00FF)
#define FIRMWARE_MINOR_OFFSET                               (0)
#define FIRMWARE_BUILDID_MASK                               (0x00F0)
#define FIRMWARE_BUILDID_OFFSET                             (4)

// PHY
#define PHY_XS_TRANSMIT_RESERVED_VENDOR_PROVISIONING5       (0xC444)

#define PHY_RX_LINK_UP_ADDR                                 (0xE812)    //PHY XS System Interface Connection Status<PHY XS Receive (XAUI Tx) Reserved Vendor State 3>: Address 4.E812
#define PHY_RX_LINK_UP_VALUE                                (0x2000)    //PHY Rx Link ok
#define PHY_TX_LINK_UP_VALUE                                (0x1000)    //PHY Tx Link ok
#define PMA_RX_LINK_STATUS_ADDR                             (0xE800)    //PMA Receive Vendor State 1: Address 1.E800
#define PMA_RX_LINK_STATUS_MASK                             (0x0001)    //0: PMA Receive Link Current Status
#define PMA_RX_LINK_STATUS_OFFSET                           (0x00)
#define PMA_RX_LINK_STATUS_VALUE                            (0x01)      //RX link good
#define ANG_NEGOTIATION_COMPLETE_ADDR                       (0x0001)    //Autonegotiation Standard Status 1: Address 7.1
#define ANG_NEGOTIATION_COMPLETE_MASK                       (0x0020)    //5: Autonegotiation Complete
#define ANG_NEGOTIATION_COMPLETE_OFFSET                     (0x05)
#define ANG_NEGOTIATION_COMPLETE_VALUE                      (0x0020)    //Autonegotiation complete
#define STABILITY_STATUS1_ADDR                              (0x0001)    //x.1
#define STABILITY_STATUS1_MASK                              (0x0004)    //2: Link Status
#define STABILITY_STATUS1_OFFSET                            (0x02)
#define STABILITY_STATUS1_VALUE                             (0x0004)      //Link Up
#define STABILITY_STATUS2_ADDR                              (0x0008)    //x.8
#define STABILITY_STATUS2_MASK                              (0x0C00)    //B: Transmit Fault, A: Receive Fault
#define STABILITY_STATUS2_OFFSET                            (0x0A)
#define STABILITY_STATUS2_VALUE                             (0x00)      //No Fault condition for tx, rx
#define PMA_STATUS2_ADDR                                    (0x0008)    //PMA Standard Status 2: Address 1.8
#define PMA_STATUS2_MASK                                    (0x0C00)    //B:A Transmit or Receive Fault
#define PMA_STATUS2_OFFSET                                  (0x0A)
#define PMA_STATUS2_VALUE                                   (0x00)

// For Icmds =====================================================================================
// PMA
#define PMA_ShortReachMode_ADDR                             (0x0083)    //PMA 10GBASE-T Tx Power Backoff and Short Reach Setting: Address 1.83
#define PMA_ShortReachMode_MASK                             (0x0001)    //0 Short Reach Mode
#define PMA_ShortReachMode_OFFSET                           (0)
#define PMA_TestModeControl_ADDR                            (0x0084)    //PMA 10GBASE-T Test Modes: Address 1.84
#define PMA_TestModeControl_MASK                            (0xE000)    //F:D Test Mode Control [2:0]
#define PMA_TestModeControl_OFFSET                          (0x0D)
#define PMA_TransmitterTestFrequencies_ADDR                 (0x0084)    //PMA 10GBASE-T Test Modes: Address 1.84
#define PMA_TransmitterTestFrequencies_MASK                 (0x1C00)    //C:A Transmitter Test Frequencies [2:0]
#define PMA_TransmitterTestFrequencies_OFFSET               (0x0A)
#define PMA_FastRetrainAbility_ADDR                         (0x0093)    //PMA 10GBASE-T Fast Retrain Status and Control: Address 1.93
#define PMA_FastRetrainAbility_MASK                         (0x0010)    //4 Fast Retrain Ability
#define PMA_FastRetrainAbility_OFFSET                       (0x04)
#define PMA_FastRetrainEnable_ADDR                          (0x93)      //PMA 10GBASE-T Fast Retrain Status and Control: Address 1.93
#define PMA_FastRetrainEnable_MASK                          (0x01)      //0 Fast Retrain Enable
#define PMA_FastRetrainEnable_OFFSET                        (0)
#define PMA_TestModeRate_ADDR                               (0xC412)    //PMA Transmit Reserved Vendor Provisioning 0: Address 1.C412
#define PMA_TestModeRate_MASK                               (0xC000)    //F:E Test Mode Rate[1:0]
#define PMA_TestModeRate_OFFSET                             (0x0E)
#define PMA_PmaDigitalSystemLoopback_ADDR                   (0xD800)    //PMA Transmit Vendor Debug 1: Address 1.D800
#define PMA_PmaDigitalSystemLoopback_MASK                   (0x8000)    //F PMA Digital System Loopback
#define PMA_PmaDigitalSystemLoopback_OFFSET                 (0x0F)
#define PMA_ExternalPhyLoopback_ADDR                        (0xE400)    //PMA Receive Reserved Vendor Provisioning 1: Address 1.E400
#define PMA_ExternalPhyLoopback_MASK                        (0x8000)    //F External PHY Loopback
#define PMA_ExternalPhyLoopback_OFFSET                      (0x0F)
#define PMA_EnableFastRetrain_ADDR                          (0xE400)    //PMA Receive Reserved Vendor Provisioning 1: Address 1.E400
#define PMA_EnableFastRetrain_MASK                          (0x0004)    //2 Enable Aquantia Fast Retrain
#define PMA_EnableFastRetrain_OFFSET                        (0x02)
#define PMA_ForceMdiConfiguration_ADDR                      (0xE400)    //PMA Receive Reserved Vendor Provisioning 1: Address 1.E400
#define PMA_ForceMdiConfiguration_MASK                      (0x0002)    //1 Force MDI Configuration
#define PMA_ForceMdiConfiguration_OFFSET                    (0x01)
#define PMA_MdiConfiguration_ADDR                           (0xE400)    //PMA Receive Reserved Vendor Provisioning 1: Address 1.E400
#define PMA_MdiConfiguration_MASK                           (0x0001)    //0 MDI Configuration
#define PMA_MdiConfiguration_OFFSET                         (0)

// PCS
#define PCS_Loopback_ADDR                                   (0x0000)    //PCS Standard Control 1: Address 3.0
#define PCS_Loopback_MASK                                   (0x4000)    //E Loopback
#define PCS_Loopback_OFFSET                                 (0x0E)
#define PCS_10GSpeedSelection_ADDR                          (0x0000)    //PCS Standard Control 1: Address 3.0
#define PCS_10GSpeedSelection_MASK                          (0x003C)    //5:2 10G Speed Selection [3:0]
#define PCS_10GSpeedSelection_OFFSET                        (0x02)
#define PCS_TxScramblerDisable_ADDR                         (0xD800)    //PCS Transmit Vendor Debug 1: Address 3.D800
#define PCS_TxScramblerDisable_MASK                         (0x8000)    //F PCS Tx Scrambler Disable
#define PCS_TxScramblerDisable_OFFSET                       (0x0F)
#define PCS_TxInjectCrcError_ADDR                           (0xD800)    //PCS Transmit Vendor Debug 1: Address 3.D800
#define PCS_TxInjectCrcError_MASK                           (0x4000)    //E PCS Tx Inject CRC Error
#define PCS_TxInjectCrcError_OFFSET                         (0x0E)
#define PCS_TxInjectFrameError_ADDR                         (0xD800)    //PCS Transmit Vendor Debug 1: Address 3.D800
#define PCS_TxInjectFrameError_MASK                         (0x2000)    //D PCS Tx Inject Frame Error
#define PCS_TxInjectFrameError_OFFSET                       (0x0D)
#define PCS_RxErrorLdpcFrameEnable_ADDR                     (0xE400)    //PCS Receive Vendor Provisioning 1: Address 3.E400
#define PCS_RxErrorLdpcFrameEnable_MASK                     (0x0001)    //0 PCS Rx Error LDPC Frame Enable
#define PCS_RxErrorLdpcFrameEnable_OFFSET                   (0)
#define PCS_RxLdpcDecoderControl_ADDR                       (0xE400)    //PCS Receive Vendor Provisioning 1: Address 3.E400
#define PCS_RxLdpcDecoderControl_MASK                       (0x8000)    //(Not in Datasheet but API)
#define PCS_RxLdpcDecoderControl_OFFSET                     (0x0F)

// PHY
#define XS_Loopback_ADDR                                    (0x0000)    //PHY XS Standard Control 1: Address 4.0)
#define XS_Loopback_MASK                                    (0x4000)    //E Loopback
#define XS_Loopback_OFFSET                                  (0X0E)
#define XS_ReceiveTestPatternEnable_ADDR                    (0x0019)    //PHY XS Standard XGXS Test Control: Address 4.19
#define XS_ReceiveTestPatternEnable_MASK                    (0x0004)    //2 Receive Test-Pattern Enable
#define XS_ReceiveTestPatternEnable_OFFSET                  (0x02)
#define XS_TestPatternSelect_ADDR                           (0x0019)    //PHY XS Standard XGXS Test Control: Address 4.19
#define XS_TestPatternSelect_MASK                           (0x0003)    //1:0 Test-Pattern Select [1:0]
#define XS_TestPatternSelect_OFFSET                         (0)
#define XS_PhyOperatingMode_ADDR                            (0xC441)    //PHY XS Transmit (XAUI Rx) Reserved Vendor Provisioning 2: Address 4.C441
#define XS_PhyOperatingMode_MASK                            (0x01C0)    //8:6 PHY Operating Mode [2:0]
#define XS_PhyOperatingMode_OFFSET                          (0x06)
#define XS_LoopbackControl_ADDR                             (0xC444)    //PHY XS Transmit (XAUI Rx) Reserved Vendor Provisioning 5: Address 4.C444
#define XS_LoopbackControl_MASK                             (0xF800)    //F:B Loopback Control [4:0]
#define XS_LoopbackControl_OFFSET                           (0x0B)
#define XS_MdiPacketGeneration_ADDR                         (0xC444)    //PHY XS Transmit (XAUI Rx) Reserved Vendor Provisioning 5: Address 4.C444
#define XS_MdiPacketGeneration_MASK                         (0x0020)    //5 MDI Packet Generation
#define XS_MdiPacketGeneration_OFFSET                       (0x05)
#define XS_SystemIFPacketGeneration_ADDR                    (0xC444)    //PHY XS Transmit (XAUI Rx) Reserved Vendor Provisioning 5: Address 4.C444
#define XS_SystemIFPacketGeneration_MASK                    (0x0008)    //3 System I/F Packet Generation
#define XS_SystemIFPacketGeneration_OFFSET                  (0x03)
#define XS_Rate_ADDR                                        (0xC444)    //PHY XS Transmit (XAUI Rx) Reserved Vendor Provisioning 5: Address 4.C444
#define XS_Rate_MASK                                        (0x0003)    //2:0 Rate [2:0]
#define XS_Rate_OFFSET                                      (0)
#define XS_TestPatternForceError_ADDR                       (0xD800)    //PHY XS Transmit (XAUI Rx) Vendor Debug 1: Address 4.D800
#define XS_TestPatternForceError_MASK                       (0x8000)    //F Test Pattern Force Error
#define XS_TestPatternForceError_OFFSET                     (0x0F)
#define XS_TestPatternMode7ForceError_ADDR                  (0xD800)    //PHY XS Transmit (XAUI Rx) Vendor Debug 1: Address 4.D800
#define XS_TestPatternMode7ForceError_MASK                  (0x4000)    //E Test Pattern Mode 7 Force Error
#define XS_TestPatternMode7ForceError_OFFSET                (0x0E)
#define XS_XAUIRxLocalFaultInjection_ADDR                   (0xD800)    //PHY XS Transmit (XAUI Rx) Vendor Debug 1: Address 4.D800
#define XS_XAUIRxLocalFaultInjection_MASK                   (0x2000)    //D XAUI Rx Local Fault Injection
#define XS_XAUIRxLocalFaultInjection_OFFSET                 (0x0D)
#define XS_TestPatternExtendedSelect_ADDR                   (0xD800)    //PHY XS Transmit (XAUI Rx) Vendor Debug 1: Address 4.D800
#define XS_TestPatternExtendedSelect_MASK                   (0x1800)    //C:B Test-Pattern Extended Select[1:0]
#define XS_TestPatternExtendedSelect_OFFSET                 (0x0B)
#define XS_TestPatternCheckEnable_ADDR                      (0xD800)    //PHY XS Transmit (XAUI Rx) Vendor Debug 1: Address 4.D800
#define XS_TestPatternCheckEnable_MASK                      (0x0400)    //A Test Pattern Check Enable
#define XS_TestPatternCheckEnable_OFFSET                    (0x0A)
#define XS_TestPatternCheckPoint_ADDR                       (0xD800)    //PHY XS Transmit (XAUI Rx) Vendor Debug 1: Address 4.D800
#define XS_TestPatternCheckPoint_MASK                       (0x0080)    //7 Test Pattern Check Point
#define XS_TestPatternCheckPoint_OFFSET                     (0x07)
#define XS_TestPatternInsertExtraIdles_ADDR                 (0xD801)    //PHY XS Transmit (XAUI Rx) Vendor Debug 2: Address 4.D801
#define XS_TestPatternInsertExtraIdles_MASK                 (0x7000)    //E:C Test Pattern Insert Extra Idles[2:0]
#define XS_TestPatternInsertExtraIdles_OFFSET               (0x0C)
#define XS_TestPatternCheckSelect_ADDR                      (0xD801)    //PHY XS Transmit (XAUI Rx) Vendor Debug 2: Address 4.D801
#define XS_TestPatternCheckSelect_MASK                      (0x0F00)    //B:8 Test Pattern Check Select[3:0]
#define XS_TestPatternCheckSelect_OFFSET                    (0x08)
#define XS_TestPatternChannelSelect_ADDR                    (0xD801)    //PHY XS Transmit (XAUI Rx) Vendor Debug 2: Address 4.D801
#define XS_TestPatternChannelSelect_MASK                    (0x000F)    //3:0 Test Pattern Channel Select[3:0]
#define XS_TestPatternChannelSelect_OFFSET                  (0)
#define XS_Channel0TestPatternErrorCounter_ADDR             (0xD810)    //PHY XS Transmit (XAUI Rx) Test Pattern Error Counter 1: Address 4.D810
#define XS_Channel0TestPatternErrorCounter_MASK             (0xFFFF)    //F:0 Channel 0 Test Pattern Error Counter [F:0]
#define XS_Channel0TestPatternErrorCounter_OFFSET           (0)
#define XS_Channel1TestPatternErrorCounter_ADDR             (0xD811)    //PHY XS Transmit (XAUI Rx) Test Pattern Error Counter 2: Address 4.D811
#define XS_Channel1TestPatternErrorCounter_MASK             (0xFFFF)    //F:0 Channel 1 Test Pattern Error Counter [F:0]
#define XS_Channel1TestPatternErrorCounter_OFFSET           (0)
#define XS_Channel2TestPatternErrorCounter_ADDR             (0xD812)    //PHY XS Transmit (XAUI Rx) Test Pattern Error Counter 3: Address 4.D812
#define XS_Channel2TestPatternErrorCounter_MASK             (0xFFFF)    //F:0 Channel 2 Test Pattern Error Counter [F:0]
#define XS_Channel2TestPatternErrorCounter_OFFSET           (0)
#define XS_Channel3TestPatternErrorCounter_ADDR             (0xD813)    //PHY XS Transmit (XAUI Rx) Test Pattern Error Counter 4: Address 4.D813
#define XS_Channel3TestPatternErrorCounter_MASK             (0xFFFF)    //F:0 Channel 3 Test Pattern Error Counter [F:0]
#define XS_Channel3TestPatternErrorCounter_OFFSET           (0)
#define XS_TestPatternMode7ErrorCounter_ADDR                (0xD814)    //PHY XS Transmit (XAUI Rx) Test Pattern Error Counter 5: Address 4.D814
#define XS_TestPatternMode7ErrorCounter_MASK                (0xFFFF)    //F:0 Test Pattern Mode 7 Error Counter [F:0]
#define XS_TestPatternMode7ErrorCounter_OFFSET              (0)
#define XS_XauiTxErrorInjectionLaneSelect_ADDR              (0xF800)    //PHY XS Receive (XAUI Tx) Vendor Debug 1: Address 4.F800
#define XS_XauiTxErrorInjectionLaneSelect_MASK              (0xE000)    //F:D XAUI Tx Error Injection Lane Select [2:0]
#define XS_XauiTxErrorInjectionLaneSelect_OFFSET            (0x0D)
#define XS_XauiTxInjectSynchronizationError_ADDR            (0xF800)    //PHY XS Receive (XAUI Tx) Vendor Debug 1: Address 4.F800
#define XS_XauiTxInjectSynchronizationError_MASK            (0x1000)    //C XAUI Tx Inject Synchronization Error
#define XS_XauiTxInjectSynchronizationError_OFFSET          (0x0C)
#define XS_XauiTxInjectAlignmentError_ADDR                  (0xF800)    //PHY XS Receive (XAUI Tx) Vendor Debug 1: Address 4.F800
#define XS_XauiTxInjectAlignmentError_MASK                  (0x0800)    //B XAUI Tx Inject Alignment Error
#define XS_XauiTxInjectAlignmentError_OFFSET                (0x0B)
#define XS_XauiTxInjectCodeViolation_ADDR                   (0xF800)    //PHY XS Receive (XAUI Tx) Vendor Debug 1: Address 4.F800
#define XS_XauiTxInjectCodeViolation_MASK                   (0x0400)    //A XAUI Tx Inject Code Violation
#define XS_XauiTxInjectCodeViolation_OFFSET                 (0x0A)
#define XS_XauiTx10BViolationCodeword_ADDR                  (0xF800)    //PHY XS Receive (XAUI Tx) Vendor Debug 1: Address 4.F800
#define XS_XauiTx10BViolationCodeword_MASK                  (0x03FF)    //9:0 XAUI Tx 10B Violation Codeword [9:0]
#define XS_XauiTx10BViolationCodeword_OFFSET                (0)
#define XS_PhyXsSystemLoopbackPassThrough_ADDR              (0xF802)    //PHY XS Receive (XAUI Tx) Vendor Debug 3: Address 4.F802
#define XS_PhyXsSystemLoopbackPassThrough_MASK              (0x8000)    //F PHY XS System Loopback Pass Through
#define XS_PhyXsSystemLoopbackPassThrough_OFFSET            (0x0F)
#define XS_PhyXsSystemLoopbackEnable_ADDR                   (0xF802)    //PHY XS Receive (XAUI Tx) Vendor Debug 3: Address 4.F802
#define XS_PhyXsSystemLoopbackEnable_MASK                   (0x4000)    //E PHY XS System Loopback Enable
#define XS_PhyXsSystemLoopbackEnable_OFFSET                 (0x0E)
#define XS_XauiTxLocalFaultInjection_ADDR                   (0xF802)    //PHY XS Receive (XAUI Tx) Vendor Debug 3: Address 4.F802
#define XS_XauiTxLocalFaultInjection_MASK                   (0x2000)    //D XAUI Tx Local Fault Injection
#define XS_XauiTxLocalFaultInjection_OFFSET                 (0x0D)

//AUTO NEGOCIATION
#define AN_RestartAutonegotiation_ADDR                      (0x0000)    //Autonegotiation Standard Control 1: Address 7.0
#define AN_RestartAutonegotiation_MASK                      (0x0200)    //9 Restart Autonegotiation
#define AN_RestartAutonegotiation_OFFSET                    (0x09)
#define AN_SerdesStartUpMode_ADDR                           (0xC410)    //Autonegotiation Reserved Vendor Provisioning 1: Address 7.C410
#define AN_SerdesStartUpMode_MASK                           (0xE000)    //F:D SERDES Start-Up Mode[2:0]
#define AN_SerdesStartUpMode_OFFSET                         (0x0D)
#define AN_AutonegotiationTimeout_ADDR                      (0xC411)    //Autonegotiation Reserved Vendor Provisioning 2: Address 7.C411
#define AN_AutonegotiationTimeout_MASK                      (0xF000)    //F:C Autonegotiation Timeout [3:0]
#define AN_AutonegotiationTimeout_OFFSET                    (0x0C)
#define AN_AutonegotiationTimeoutMod_ADDR                   (0xC411)    //Autonegotiation Reserved Vendor Provisioning 2: Address 7.C411
#define AN_AutonegotiationTimeoutMod_MASK                   (0x0800)    //B Autonegotiation Timeout Mode
#define AN_AutonegotiationTimeoutMod_OFFSET                 (0x0B)

#define AN_AutonegotiationStatus_ADDR                       (0xC810)    //Autonegotiation Reserved Vendor Status 1: Address 7.C810
#define AN_AutonegotiationConnectionState_MASK              (0x3E00)    //D:9 Connection State [4:0]
#define AN_AutonegotiationConnectionState_OFFSET            (0x09)      //D:9 Connection State offset - 10
#define AN_AutonegotiationConnectionState_Fail              (0x05)      //D:9 Connection State value 5 - link is in a failure state
#define AN_AutonegotiationConnectionState_NoCable           (0x0B)      //D:9 Connection State  value 0x0b - no cable detected

#define AN_LinkPartner1000BaseTFullDuplexAbility_ADDR       (0xE820)    //Autonegotiation Receive Link Partner Status 1: Address 7.E820
#define AN_LinkPartner1000BaseTFullDuplexAbility_MASK       (0x8000)    //F Link Partner 1000BASE-T Full Duplex Ability
#define AN_LinkPartner1000BaseTFullDuplexAbility_OFFSET     (0x0F)
#define AN_LinkPartner1000BaseTHalfDuplexAbility_ADDR       (0xE820)    //Autonegotiation Receive Link Partner Status 1: Address 7.E820
#define AN_LinkPartner1000BaseTHalfDuplexAbility_MASK       (0x4000)    //E Link Partner 1000BASE-T Half Duplex Ability
#define AN_LinkPartner1000BaseTHalfDuplexAbility_OFFSET     (0x0E)
#define AN_LinkPartnerShortReach_ADDR                       (0xE820)    //Autonegotiation Receive Link Partner Status 1: Address 7.E820
#define AN_LinkPartnerShortReach_MASK                       (0x2000)    //D Link Partner Short-Reach
#define AN_LinkPartnerShortReach_OFFSET                     (0x0D)
#define AN_LinkPartnerAqRateDownshiftCapability_ADDR        (0xE820)    //Autonegotiation Receive Link Partner Status 1: Address 7.E820
#define AN_LinkPartnerAqRateDownshiftCapability_MASK        (0x1000)    //C Link Partner AQRate Downshift Capability
#define AN_LinkPartnerAqRateDownshiftCapability_OFFSET      (0x0C)
#define AN_LinkPartner5G_ADDR                               (0xE820)    //Autonegotiation Receive Link Partner Status 1: Address 7.E820
#define AN_LinkPartner5G_MASK                               (0x0800)    //B Link Partner 5G
#define AN_LinkPartner5G_OFFSET                             (0x0B)
#define AN_LinkPartner2G_ADDR                               (0xE820)    //Autonegotiation Receive Link Partner Status 1: Address 7.E820
#define AN_LinkPartner2G_MASK                               (0x0400)    //A Link Partner 2.5G
#define AN_LinkPartner2G_OFFSET                             (0x0A)
#define AN_LinkPartner_ADDR                                 (0xE820)    //Autonegotiation Receive Link Partner Status 1: Address 7.E820
#define AN_LinkPartner_MASK                                 (0x0004)    //2 Aquantia Link Partner
#define AN_LinkPartner_OFFSET                               (2)
#define AN_AutonegotiationProtocolErrorState_ADDR           (0xE831)    //Autonegotiation Receive Reserved Vendor Status 2: Address 7.E831
#define AN_AutonegotiationProtocolErrorState_MASK           (0x2000)    //D Autonegotiation Protocol Error State
#define AN_AutonegotiationProtocolErrorState_OFFSET         (0x0D)
#define AN_FlpIdleErrorState_ADDR                           (0xE831)    //Autonegotiation Receive Reserved Vendor Status 2: Address 7.E831
#define AN_FlpIdleErrorState_MASK                           (0x1000)    //C FLP Idle Error State
#define AN_FlpIdleErrorState_OFFSET                         (0x0C)

//GLOBAL
#define GLB_EnableDiagnostics_ADDR                          (0xC400)    //Global Diagnostic Provisioning: Address 1E.C400
#define GLB_EnableDiagnostics_MASK                          (0x8000)    //F Enable Diagnostics
#define GLB_EnableDiagnostics_OFFSET                        (0x0F)
#define GLB_HighTempFailureThreshold_ADDR                   (0xC421)    //Global Thermal Provisioning 2: Address 1E.C421
#define GLB_HighTempFailureThreshold_MASK                   (0xFFFF)    //F:0 High Temp Failure Threshold [F:0]
#define GLB_HighTempFailureThreshold_OFFSET                 (0)
#define GLB_LowTempFailureThreshold_ADDR                    (0xC422)    //Global Thermal Provisioning 3: Address 1E.C422
#define GLB_LowTempFailureThreshold_MASK                    (0xFFFF)    //F:0 Low Temp Failure Threshold [F:0]
#define GLB_LowTempFailureThreshold_OFFSET                  (0)
#define GLB_HighTempWarningThreshold_ADDR                   (0xC423)    //Global Thermal Provisioning 4: Address 1E.C423
#define GLB_HighTempWarningThreshold_MASK                   (0xFFFF)    //F:0 High Temp Warning Threshold [F:0]
#define GLB_HighTempWarningThreshold_OFFSET                 (0)
#define GLB_LowTempWarningThreshold_ADDR                    (0xC424)    //Global Thermal Provisioning 5: Address 1E.C424
#define GLB_LowTempWarningThreshold_MASK                    (0xFFFF)    //F:0 Low Temp Warning Threshold [F:0]
#define GLB_LowTempWarningThreshold_OFFSET                  (0)
#define GLB_DiagnosticsSelect_ADDR                          (0xC470)    //Global Reserved Provisioning 1: Address 1E.C470
#define GLB_DiagnosticsSelect_MASK                          (0x8000)    //F Diagnostics Select
#define GLB_DiagnosticsSelect_OFFSET                        (0x0F)
#define GLB_ExtendedMdiDiagnosticsSelect_ADDR               (0xC470)    //Global Reserved Provisioning 1: Address 1E.C470
#define GLB_ExtendedMdiDiagnosticsSelect_MASK               (0x6000)    //E:D Extended MDI Diagnostics Select [1:0]
#define GLB_ExtendedMdiDiagnosticsSelect_OFFSET             (0x0D)
#define GLB_InitiateCableDiagnostics_ADDR                   (0xC470)    //Global Reserved Provisioning 1: Address 1E.C470
#define GLB_InitiateCableDiagnostics_MASK                   (0x0010)    //4 Initiate Cable Diagnostics
#define GLB_InitiateCableDiagnostics_OFFSET                 (0x04)
#define GLB_EnableVddPowerSupplyTuning_ADDR                 (0xC472)    //Global Reserved Provisioning 3: Address 1E.C472
#define GLB_EnableVddPowerSupplyTuning_MASK                 (0x4000)    //Enable VDD Power Supply Tuning
#define GLB_EnableVddPowerSupplyTuning_OFFSET               (0x0E)
#define GLB_TunableExternalVddPowerSupplyPresent_ADDR       (0xC472)    //Global Reserved Provisioning 3: Address 1E.C472
#define GLB_TunableExternalVddPowerSupplyPresent_MASK       (0x0040)    //6 Tunable External VDD Power Supply Present
#define GLB_TunableExternalVddPowerSupplyPresent_OFFSET     (0x06)
#define GLB_ExternalVddChangeRequest_ADDR                   (0xC472)    //Global Reserved Provisioning 3: Address 1E.C472
#define GLB_ExternalVddChangeRequest_MASK                   (0x003C)    //5:2 External VDD Change Request[3:0]
#define GLB_ExternalVddChangeRequest_OFFSET                 (0x2)
#define GLB_Enable5ChannelRfiCancellation_ADDR              (0xC472)    //Global Reserved Provisioning 3: Address 1E.C472
#define GLB_Enable5ChannelRfiCancellation_MASK              (0x0001)    //0 Enable 5th Channel RFI Cancellation
#define GLB_Enable5ChannelRfiCancellation_OFFSET            (0)
#define GLB_RateTransitionRequest_ADDR                      (0xC473)    //Global Reserved Provisioning 4: Address 1E.C473
#define GLB_RateTransitionRequest_MASK                      (0x0700)    //A:8 Rate Transition Request [2:0]
#define GLB_RateTransitionRequest_OFFSET                    (0x08)
#define GLB_TrainingSnr_ADDR                                (0xC473)    //Global Reserved Provisioning 4: Address 1E.C473
#define GLB_TrainingSnr_MASK                                (0x00FF)    //7:0 Training SNR [7:0]
#define GLB_TrainingSnr_OFFSET                              (0)
#define GLB_GlbLoopbackControl_ADDR                         (0xC47A)    //Global Reserved Provisioning 11: Address 1E.C47A
#define GLB_GlbLoopbackControl_MASK                         (0xF800)    //F:B Loopback Control [4:0]
#define GLB_GlbLoopbackControl_OFFSET                       (0x0B)
#define GLB_GlbMdiPacketGeneration_ADDR                     (0xC47A)    //Global Reserved Provisioning 11: Address 1E.C47A
#define GLB_GlbMdiPacketGeneration_MASK                     (0x0020)    //5 MDI Packet Generation
#define GLB_GlbMdiPacketGeneration_OFFSET                   (0x05)
#define GLB_SystemIFPacketGeneration_ADDR                   (0xC47A)    //Global Reserved Provisioning 11: Address 1E.C47A
#define GLB_SystemIFPacketGeneration_MASK                   (0x0008)    //3 System I/F Packet Generation
#define GLB_SystemIFPacketGeneration_OFFSET                 (0x03)
#define GLB_GlobalReservedProvisioningRate_ADDR             (0xC47A)    //Global Reserved Provisioning 11: Address 1E.C47A
#define GLB_GlobalReservedProvisioningRate_MASK             (0x0007)    //2:0 Rate [2:0]
#define GLB_GlobalReservedProvisioningRate_OFFSET           (0)
#define GLB_PairAStatus_ADDR                                (0xC800)    //Global Cable Diagnostic Status 1: Address 1E.C800
#define GLB_PairAStatus_MASK                                (0x7000)    //E:C Pair A Status[2:0]
#define GLB_PairAStatus_OFFSET                              (0x0C)
#define GLB_PairBStatus_ADDR                                (0xC800)    //Global Cable Diagnostic Status 1: Address 1E.C800
#define GLB_PairBStatus_MASK                                (0x0700)    //A:8 Pair B Status[2:0]
#define GLB_PairBStatus_OFFSET                              (0x08)
#define GLB_PairCStatus_ADDR                                (0xC800)    //Global Cable Diagnostic Status 1: Address 1E.C800
#define GLB_PairCStatus_MASK                                (0x0070)    //6:4 Pair C Status[2:0]
#define GLB_PairCStatus_OFFSET                              (0x04)
#define GLB_PairDStatus_ADDR                                (0xC800)    //Global Cable Diagnostic Status 1: Address 1E.C800
#define GLB_PairDStatus_MASK                                (0x0007)    //2:0 Pair D Status[2:0]
#define GLB_PairDStatus_OFFSET                              (0)
#define GLB_StatusPairAReflection1_ADDR                     (0xC801)    //Global Cable Diagnostic Status 2: Address 1E.C801
#define GLB_StatusPairAReflection1_MASK                     (0xFF00)    //F:8 Pair A Reflection #1[7:0]
#define GLB_StatusPairAReflection1_OFFSET                   (0x08)
#define GLB_StatusPairAReflection2_ADDR                     (0xC801)    //Global Cable Diagnostic Status 2: Address 1E.C801
#define GLB_StatusPairAReflection2_MASK                     (0x00FF)    //7:0 Pair A Reflection #2[7:0]
#define GLB_StatusPairAReflection2_OFFSET                   (0)
#define GLB_ImpulseResponseMsw_ADDR                         (0xC802)    //Global Cable Diagnostic Status 3: Address 1E.C802
#define GLB_ImpulseResponseMsw_MASK                         (0xFFFF)    //F:0 Impulse Response MSW[F:0]
#define GLB_ImpulseResponseMsw_OFFSET                       (0)
#define GLB_StatusPairBReflection1_ADDR                     (0xC803)    //Global Cable Diagnostic Status 4: Address 1E.C803
#define GLB_StatusPairBReflection1_MASK                     (0xFF00)    //F:8 Pair B Reflection #1[7:0]
#define GLB_StatusPairBReflection1_OFFSET                   (0x08)
#define GLB_StatusPairBReflection2_ADDR                     (0xC803)    //Global Cable Diagnostic Status 4: Address 1E.C803
#define GLB_StatusPairBReflection2_MASK                     (0x00FF)    //7:0 Pair B Reflection #2[7:0]
#define GLB_StatusPairBReflection2_OFFSET                   (0)
#define GLB_ImpulseResponseLsw_ADDR                         (0xC804)    //Global Cable Diagnostic Status 5: Address 1E.C804
#define GLB_ImpulseResponseLsw_MASK                         (0xFFFF)    //F:0 Impulse Response LSW [F:0]
#define GLB_ImpulseResponseLsw_OFFSET                       (0)
#define GLB_StatusPairCReflection1_ADDR                     (0xC805)    //Global Cable Diagnostic Status 6: Address 1E.C805
#define GLB_StatusPairCReflection1_MASK                     (0xFF00)    //F:8 Pair C Reflection #1[7:0]
#define GLB_StatusPairCReflection1_OFFSET                   (0x08)
#define GLB_StatusPairCReflection2_ADDR                     (0xC805)    //Global Cable Diagnostic Status 6: Address 1E.C805
#define GLB_StatusPairCReflection2_MASK                     (0x00FF)    //7:0 Pair C Reflection #2 exist or was not computed.[7:0]
#define GLB_StatusPairCReflection2_OFFSET                   (0)
#define GLB_StatusPairDReflection1_ADDR                     (0xC807)    //Global Cable Diagnostic Status 8: Address 1E.C807
#define GLB_StatusPairDReflection1_MASK                     (0xFF00)    //F:8 Pair D Reflection #1[7:0]
#define GLB_StatusPairDReflection1_OFFSET                   (0x8)
#define GLB_StatusPairDReflection2_ADDR                     (0xC807)    //Global Cable Diagnostic Status 8: Address 1E.C807
#define GLB_StatusPairDReflection2_MASK                     (0x00FF)    //7:0 Pair D Reflection #2 exist or was not computed.[7:0]
#define GLB_StatusPairDReflection2_OFFSET                   (0)
#define GLB_Temperature_ADDR                                (0xC820)    //Global Thermal Status 1: Address 1E.C820
#define GLB_Temperature_MASK                                (0xFFFF)    //F:0 Temperature [F:0]
#define GLB_Temperature_OFFSET                              (0)
#define GLB_ProcessorIntensiveOperationInProgress_ADDR      (0xC831)    //Global General Status 2: Address 1E.C831
#define GLB_ProcessorIntensiveOperationInProgress_MASK      (0x8000)    //F Processor Intensive MDIO Operation In-Progress
#define GLB_ProcessorIntensiveOperationInProgress_OFFSET    (0x0F)
#define GLB_ImpedencePairAReflection1_ADDR                  (0xC880)    //Global Cable Diagnostic Impedance 1: Address 1E.C880
#define GLB_ImpedencePairAReflection1_MASK                  (0x7000)    //E:C Pair A Reflection#1 [2:0]
#define GLB_ImpedencePairAReflection1_OFFSET                (0x0C)
#define GLB_ImpedencePairAReflection2_ADDR                  (0xC880)    //Global Cable Diagnostic Impedance 1: Address 1E.C880
#define GLB_ImpedencePairAReflection2_MASK                  (0x0700)    //A:8 Pair A Reflection#2 [2:0]
#define GLB_ImpedencePairAReflection2_OFFSET                (0x08)
#define GLB_ImpedencePairAReflection3_ADDR                  (0xC880)    //Global Cable Diagnostic Impedance 1: Address 1E.C880
#define GLB_ImpedencePairAReflection3_MASK                  (0x0070)    //6:4 Pair A Reflection#3 [2:0]
#define GLB_ImpedencePairAReflection3_OFFSET                (0x04)
#define GLB_ImpedencePairAReflection4_ADDR                  (0xC880)    //Global Cable Diagnostic Impedance 1: Address 1E.C880
#define GLB_ImpedencePairAReflection4_MASK                  (0x0007)    //2:0 Pair A Reflection#4 [2:0]
#define GLB_ImpedencePairAReflection4_OFFSET                (0)
#define GLB_ImpedencePairBReflection1_ADDR                  (0xC881)    //Global Cable Diagnostic Impedance 2: Address 1E.C881
#define GLB_ImpedencePairBReflection1_MASK                  (0x7000)    //E:C Pair B Reflection#1 [2:0]
#define GLB_ImpedencePairBReflection1_OFFSET                (0x0C)
#define GLB_ImpedencePairBReflection2_ADDR                  (0xC881)    //Global Cable Diagnostic Impedance 2: Address 1E.C881
#define GLB_ImpedencePairBReflection2_MASK                  (0x0700)    //A:8 Pair B Reflection#2 [2:0]
#define GLB_ImpedencePairBReflection2_OFFSET                (0x08)
#define GLB_ImpedencePairBReflection3_ADDR                  (0xC881)    //Global Cable Diagnostic Impedance 2: Address 1E.C881
#define GLB_ImpedencePairBReflection3_MASK                  (0x0070)    //6:4 Pair B Reflection#3 [2:0]
#define GLB_ImpedencePairBReflection3_OFFSET                (0x04)
#define GLB_ImpedencePairBReflection4_ADDR                  (0xC881)    //Global Cable Diagnostic Impedance 2: Address 1E.C881
#define GLB_ImpedencePairBReflection4_MASK                  (0x0007)    //2:0 Pair B Reflection#4 [2:0]
#define GLB_ImpedencePairBReflection4_OFFSET                (0)
#define GLB_ImpedencePairCReflection1_ADDR                  (0xC882)    //Global Cable Diagnostic Impedance 3: Address 1E.C882
#define GLB_ImpedencePairCReflection1_MASK                  (0x7000)    //E:C Pair C Reflection#1 [2:0]
#define GLB_ImpedencePairCReflection1_OFFSET                (0x0C)
#define GLB_ImpedencePairCReflection2_ADDR                  (0xC882)    //Global Cable Diagnostic Impedance 3: Address 1E.C882
#define GLB_ImpedencePairCReflection2_MASK                  (0x0700)    //A:8 Pair C Reflection#2 [2:0]
#define GLB_ImpedencePairCReflection2_OFFSET                (0x08)
#define GLB_ImpedencePairCReflection3_ADDR                  (0xC882)    //Global Cable Diagnostic Impedance 3: Address 1E.C882
#define GLB_ImpedencePairCReflection3_MASK                  (0x0070)    //6:4 Pair C Reflection#3 [2:0]
#define GLB_ImpedencePairCReflection3_OFFSET                (0x04)
#define GLB_ImpedencePairCReflection4_ADDR                  (0xC882)    //Global Cable Diagnostic Impedance 3: Address 1E.C882
#define GLB_ImpedencePairCReflection4_MASK                  (0x0007)    //2:0 Pair C Reflection#4 [2:0]
#define GLB_ImpedencePairCReflection4_OFFSET                (0)
#define GLB_ImpedencePairDReflection1_ADDR                  (0xC883)    //Global Cable Diagnostic Impedance 4: Address 1E.C883
#define GLB_ImpedencePairDReflection1_MASK                  (0x7000)    //E:C Pair D Reflection#1 [2:0]
#define GLB_ImpedencePairDReflection1_OFFSET                (0x0C)
#define GLB_ImpedencePairDReflection2_ADDR                  (0xC883)    //Global Cable Diagnostic Impedance 4: Address 1E.C883
#define GLB_ImpedencePairDReflection2_MASK                  (0x0700)    //A:8 Pair D Reflection#2 [2:0]
#define GLB_ImpedencePairDReflection2_OFFSET                (0x08)
#define GLB_ImpedencePairDReflection3_ADDR                  (0xC883)    //Global Cable Diagnostic Impedance 4: Address 1E.C883
#define GLB_ImpedencePairDReflection3_MASK                  (0x0070)    //6:4 Pair D Reflection#3 [2:0]
#define GLB_ImpedencePairDReflection3_OFFSET                (4)
#define GLB_ImpedencePairDReflection4_ADDR                  (0xC883)    //Global Cable Diagnostic Impedance 4: Address 1E.C883
#define GLB_ImpedencePairDReflection4_MASK                  (0x0007)    //2:0 Pair D Reflection#4 [2:0]
#define GLB_ImpedencePairDReflection4_OFFSET                (0)
#define GLB_CableLength_ADDR                                (0xC884)    //Global Status: Address 1E.C884
#define GLB_CableLength4_MASK                               (0X00FF)    //7:0 Cable Length[7:0]
#define GLB_CableLength_OFFSET                              (0)
#define GLB_GlbLoopbackStatus_ADDR                          (0xC888)    //Global Reserved Status 4: Address 1E.C888
#define GLB_GlbLoopbackStatus_MASK                          (0xF800)    //F:B Loopback Status[4:0]
#define GLB_GlbLoopbackStatus_OFFSET                        (0x0B)
#define GLB_MdiPacketGenerationStatus_ADDR                  (0xC888)    //Global Reserved Status 4: Address 1E.C888
#define GLB_MdiPacketGenerationStatus_MASK                  (0x0020)    //5 MDI Packet Generation Status
#define GLB_MdiPacketGenerationStatus_OFFSET                (0x05)
#define GLB_SystemIFPacketGenerationStatus_ADDR             (0xC888)    //Global Reserved Status 4: Address 1E.C888
#define GLB_SystemIFPacketGenerationStatus_MASK             (0x0008)    //3 System I/F Packet Generation Status
#define GLB_SystemIFPacketGenerationStatus_OFFSET           (3)
#define GLB_GlobalReservedStatusRate_ADDR                   (0xC888)    //Global Reserved Status 4: Address 1E.C888
#define GLB_GlobalReservedStatusRate_MASK                   (0x0007)    //2:0 Rate [2:0]
#define GLB_GlobalReservedStatusRate_OFFSET                 (0)

// For stats ===================================================================================================================================
// PMA
#define PMA_RECEIVE_LINK_STATUS_ADDR                        (0x0001)    //PMA Standard Status 1: Address 1.1
#define PMA_RECEIVE_LINK_STATUS_MASK                        (0x0004)    //2: PMA Receive link Status
#define PMA_RECEIVE_LINK_STATUS_OFFSET                      (0X02)
#define PMA_STANDARD_STATUS_2_ADDR                          (0x0008)    //PMA Standard Status 2: Address 1.8
#define PMA_TRANSMIT_FAULT_MASK                             (0x0800)    //B: Transmit Fault
#define PMA_TRANSMIT_FAULT_OFFSET                           (0x0B)
#define PMA_RECEIVE_FAULT_MASK                              (0x0400)    //A: Receive Fault
#define PMA_RECEIVE_FAULT_OFFSET                            (0x0A)
#define PMA_FAST_RETRAIN_STATUS_ADDR                        (0x0093)    //PMA 10GBASE-T Fast Retrain Status and Control: Address 1.93
#define PMA_LP_RETRAIN_MASK                                 (0xF800)    //F:B LP Fast Retrain Count [4:0]
#define PMA_LP_RETRAIN_OFFSET                               (0x0B)
#define PMA_LD_RETRAIN_MASK                                 (0x07C0)    //A:6 LD Fast Retrain Count [4:0]
#define PMA_LD_RETRAIN_OFFSET                               (0x06)
#define PMA_RETRAIN_SIGNAL_TYPE_MASK                        (0x0006)    //2:1 Fast Retrain Signal Type [1:0]
#define PMA_RETRAIN_SIGNAL_TYPE_OFFSET                      (0x01)
#define PMA_RECEIVE_VENDOR_STATE_2_ADDR                     (0xE811)    //PMA Receive Reserved Vendor State 2: Address 1.E811
#define PMA_NUM_LINK_RECOVERY_MASK                          (0xFF00)    //F:8 Total Number Of Link Recovery Events Since Last AutoNeg [7:0]
#define PMA_NUM_LINK_RECOVERY_OFFSET                        (0x08)
#define PMA_NUM_RFI_RECOVERY_MASK                           (0x00FF)    //7:0 Total Number Of RFI Training Link Recovery Events Since Last AutoNeg [7:0]
#define PMA_NUM_RFI_RECOVERY_OFFSET                         (0x00)
#define PMA_CHANNEL_A_OPERATING_MARGIN_ADDR                 (0x0085)   //PMA 10GBASE-T SNR Operating Margin Channel A: Address 1.85
#define PMA_CHANNEL_B_OPERATING_MARGIN_ADDR                 (0x0086)   //PMA 10GBASE-T SNR Operating Margin Channel B: Address 1.86
#define PMA_CHANNEL_C_OPERATING_MARGIN_ADDR                 (0x0087)   //PMA 10GBASE-T SNR Operating Margin Channel C: Address 1.87
#define PMA_CHANNEL_D_OPERATING_MARGIN_ADDR                 (0x0088)   //PMA 10GBASE-T SNR Operating Margin Channel D: Address 1.88
#define PMA_CHANNEL_A_MINIMUM_MARGIN_ADDR                   (0x0089)   //PMA 10GBASE-T SNR Minimum Operating Margin Channel A: Address 1.89
#define PMA_CHANNEL_B_MINIMUM_MARGIN_ADDR                   (0x008A)   //PMA 10GBASE-T SNR Minimum Operating Margin Channel B: Address 1.8A
#define PMA_CHANNEL_C_MINIMUM_MARGIN_ADDR                   (0x008B)   //PMA 10GBASE-T SNR Minimum Operating Margin Channel C: Address 1.8B
#define PMA_CHANNEL_D_MINIMUM_MARGIN_ADDR                   (0x008C)   //PMA 10GBASE-T SNR Minimum Operating Margin Channel D: Address 1.8C
#define PMA_CHANNEL_A_RECEIVED_SIGNAL_POWER_ADDR            (0x008D)   //PMA 10GBASE-T Receive Signal Power Channel A: Address 1.8D
#define PMA_CHANNEL_B_RECEIVED_SIGNAL_POWER_ADDR            (0x008E)   //PMA 10GBASE-T Receive Signal Power Channel B: Address 1.8E
#define PMA_CHANNEL_C_RECEIVED_SIGNAL_POWER_ADDR            (0x008F)   //PMA 10GBASE-T Receive Signal Power Channel C: Address 1.8F
#define PMA_CHANNEL_D_RECEIVED_SIGNAL_POWER_ADDR            (0x0090)   //PMA 10GBASE-T Receive Signal Power Channel D: Address 1.90
#define PMA_RECEIVE_CURRENT_LINK_STATUS_ADDR                (0xE800)   //PMA Receive Vendor State 1: Address 1.E800
#define PMA_RECEIVE_CURRENT_LINK_STATUS_MASK                (0x0001)   //0 PMA Receive Link Current Status
#define PMA_RECEIVE_CURRENT_LINK_STATUS_OFFSET              (0)
#define PMA_FAST_RETRAIN_TIME_ADDR                          (0xE810)   //PMA Receive Reserved Vendor State 1: Address 1.E810

//PCS
#define PCS_STANDARD_STATUS_2_ADDR                          (0x0008)    //PCS Standard Status 2: Address 3.8
#define PCS_TRANSMIT_FAULT_MASK                             (0x0800)    //B: Transmit Fault
#define PCS_TRANSMIT_FAULT_OFFSET                           (0x0B)
#define PCS_RECEIVE_FAULT_MASK                              (0x0400)    //A: Receive Fault
#define PCS_RECEIVE_FAULT_OFFSET                            (0x0A)
#define PCS_10G_STATUS_1_ADDR                               (0x0020)    //PCS 10G Status 1: Address 3.20
#define PCS_10G_LINK_STATUS_MASK                            (0x1000)    //C: 10G Receive Link Status
#define PCS_10G_LINK_STATUS_OFFSET                          (0x0C)
#define PCS_10G_HIGH_BER_MASK                               (0x0002)    //1: 10G High BER
#define PCS_10G_HIGH_BER_OFFSET                             (0x01)
#define PCS_10G_BLOCK_LOCK_MASK                             (0x0001)    //0: 10G PCS Block Lock
#define PCS_10G_BLOCK_LOCK_OFFSET                           (0x00)
#define PCS_10G_STATUS_2_ADDR                               (0x0021)    //PCS 10G Status 2: Address 3.21
#define PCS_BLOCK_LOCK_LATCHED_MASK                         (0x8000)    //F: PCS Block Lock Latched
#define PCS_BLOCK_LOCK_LATCHED_OFFSET                       (0x0F)
#define PCS_HIGH_BER_LATCHED_MASK                           (0x4000)    //E: High BER Latched
#define PCS_HIGH_BER_LATCHED_OFFSET                         (0x0E)
#define PCS_ERROR_FRAME_COUNT_MASK                          (0x3F00)    //D:8 Errored Frame Counter [5:0]
#define PCS_ERROR_FRAME_COUNT_OFFSET                        (0x08)
#define PCS_ERROR_BLOCK_COUNT_MASK                          (0x00FF)    //7:0 Errored Block Counter [7:0]
#define PCS_ERROR_BLOCK_COUNT_OFFSET                        (0x00)
#define PCS_TX_10G_BAD_FRAME_COUNTER_LSW_ADDR               (0xC822)    //PCS Transmit Vendor FCS Error Frame Counter 1: Address 3.C822
#define PCS_TX_10G_BAD_FRAME_COUNTER_MSW_ADDR               (0xC823)    //PCS Transmit Vendor FCS Error Frame Counter 2: Address 3.C823
#define PCS_TX_10G_BAD_FRAME_COUNTER_MASK                   (0x03FF)    //9:0 10GBASE-T Error Frame Counter MSW [19:10]
#define PCS_TX_10G_BAD_FRAME_COUNTER_OFFSET                 (0x00)
#define PCS_SYSTEM_INTERFACE_FAULT_ADDR                     (0xC8F0)    //PCS Transmit Vendor System Interface State 1: Address 3.C8F0
#define PCS_SYSTEM_INTERFACE_FAULT_MASK                     (0x0001)    //0: System Interface Transmit Fault
#define PCS_SYSTEM_INTERFACE_FAULT_OFFSET                   (0x00)
#define PCS_XAUI_INVALID_BLOCK_ADDR                         (0xCC00)    //PCS Transmit Vendor Alarms 1: Address 3.CC00
#define PCS_XAUI_INVALID_BLOCK_MASK                         (0x0001)    //0: XAUI Transmit Invalid 64B Block Detected
#define PCS_XAUI_INVALID_BLOCK_OFFSET                       (0x00)
#define PCS_CRC8_ERROR_COUNT_LSW_ADDR                       (0xE810)    //PCS Receive Vendor CRC-8 Error Counter 1: Address 3.E810 (LSW)
#define PCS_CRC8_ERROR_COUNT_LSW_MASK                       (0xFFFF)    //PCS Receive Vendor CRC-8 Error Counter 1: Address 3.E810 (LSW) Mask
#define PCS_CRC8_ERROR_COUNT_MSW_ADDR                       (0xE811)    //PCS Receive Vendor CRC-8 Error Counter 2: Address 3.E811 (MSW)
#define PCS_CRC8_ERROR_COUNT_MSW_MASK                       (0x003F)    //PCS Receive Vendor CRC-8 Error Counter 2: Address 3.E811 (MSW) Mask
#define PCS_10G_ERROR_FRAME_COUNT_LSW_ADDR                  (0xE814)    //PCS Receive Vendor FCS Error Frame Counter 1: Address 3.E814
#define PCS_10G_ERROR_FRAME_COUNT_MSW_ADDR                  (0xE815)    //PCS Receive Vendor FCS Error Frame Counter 2: Address 3.E815
#define PCS_10G_ERROR_FRAME_COUNT_MASK                      (0x03FF)    //9:0 10GBASE-T Error Frame Counter MSW[19:10]
#define PCS_10G_ERROR_FRAME_COUNT_OFFSET                    (0x00)
#define PCS_UNCORRECTED_FRAME_COUNT_LSW_ADDR                (0xE820)    //PCS Receive Vendor Uncorrected Frame Counter 1: Address 3.E820
#define PCS_UNCORRECTED_FRAME_COUNT_MSW_ADDR                (0xE821)    //PCS Receive Vendor Uncorrected Frame Counter 2: Address 3.E821
#define PCS_UNCORRECTED_FRAME_COUNT_MASK                    (0xFFFF)    //F:0 Uncorrected Frame Counter MSW [1F:10]
#define PCS_UNCORRECTED_FRAME_COUNT_OFFSET                  (0x00)
#define PCS_LDPC_CORRECTED_F2_ITERATION_COUNT_LSW_ADDR      (0xE842)    //PCS Receive Vendor Corrected Frame 2 Iteration Counter 1: Address 3.E842
#define PCS_LDPC_CORRECTED_F2_ITERATION_COUNT_MSW_ADDR      (0xE843)    //PCS Receive Vendor Corrected Frame 2 Iteration Counter 2: Address 3.E843
#define PCS_LDPC_CORRECTED_F2_ITERATION_COUNT_MASK          (0xFFFF)    //F:0 Corrected Frames 2 Iteration Counter MSW [1F:10]
#define PCS_LDPC_CORRECTED_F2_ITERATION_COUNT_OFFSET        (0x00)
#define PCS_LDPC_CORRECTED_F3_ITERATION_COUNT_LSW_ADDR      (0xE844)    //PCS Receive Vendor Corrected Frame 3 Iteration Counter 1: Address 3.E844
#define PCS_LDPC_CORRECTED_F3_ITERATION_COUNT_MSW_ADDR      (0xE845)    //PCS Receive Vendor Corrected Frame 3 Iteration Counter 2: Address 3.E845
#define PCS_LDPC_CORRECTED_F3_ITERATION_COUNT_MASK          (0xFFFF)    //F:0 Corrected Frames 3 Iteration Counter MSW [1F:10]
#define PCS_LDPC_CORRECTED_F3_ITERATION_COUNT_OFFSET        (0x00)
#define PCS_LDPC_CORRECTED_F4_ITERATION_COUNT_LSW_ADDR      (0xE846)    //PCS Receive Vendor Corrected Frame 4 Iteration Counter 1: Address 3.E846
#define PCS_LDPC_CORRECTED_F4_ITERATION_COUNT_MSW_ADDR      (0xE847)    //PCS Receive Vendor Corrected Frame 4 Iteration Counter 2: Address 3.E847
#define PCS_LDPC_CORRECTED_F4_ITERATION_COUNT_MASK          (0xFFFF)    //F:0 Corrected Frames 4 Iteration Counter MSW [1F:10]
#define PCS_LDPC_CORRECTED_F4_ITERATION_COUNT_OFFSET        (0x00)
#define PCS_LDPC_CORRECTED_F5_ITERATION_COUNT_LSW_ADDR      (0xE848)    //PCS Receive Vendor Corrected Frame 5 Iteration Counter 1: Address 3.E848
#define PCS_LDPC_CORRECTED_F5_ITERATION_COUNT_MSW_ADDR      (0xE849)    //PCS Receive Vendor Corrected Frame 5 Iteration Counter 2: Address 3.E849
#define PCS_LDPC_CORRECTED_F5_ITERATION_COUNT_MASK          (0xFFFF)    //F:0 Corrected Frames 5 Iteration Counter MSW [1F:10]
#define PCS_LDPC_CORRECTED_F5_ITERATION_COUNT_OFFSET        (0x00)
#define PCS_LDPC_CORRECTED_F6_ITERATION_COUNT_ADDR          (0xE850)    //PCS Receive Vendor Corrected Frame 6 Iteration Counter: Address 3.E850
#define PCS_LDPC_CORRECTED_F7_ITERATION_COUNT_ADDR          (0xE851)    //PCS Receive Vendor Corrected Frame 7 Iteration Counter: Address 3.E851
#define PCS_LDPC_CORRECTED_F8_ITERATION_COUNT_ADDR          (0xE852)    //PCS Receive Vendor Corrected Frame 6 Iteration Counter: Address 3.E850
#define PCS_VENDOR_ALARMS_1_ADDR                            (0xEC00)    //PCS Receive Vendor Alarms 1: Address 3.EC00
#define PCS_CRC_ERROR_MASK                                  (0x8000)    //F: CRC Error
#define PCS_CRC_ERROR_OFFSET                                (0x0F)
#define PCS_LDPC_DECODE_FAILURE_MASK                        (0x4000)    //E: LDPC Decode Failure
#define PCS_LDPC_DECODE_FAILURE_OFFSET                      (0x0E)
#define PCS_LOCAL_FAULT_DETECT_MASK                         (0x0800)    //B: Local Fault Detect
#define PCS_LOCAL_FAULT_DETECT_OFFSET                       (0x0B)
#define PCS_LOF_DETECT_MASK                                 (0x0400)    //A: LOF Detect
#define PCS_LOF_DETECT_OFFSET                               (0x0A)
#define PCS_40G_BIP_LOCK_MASK                               (0x0200)    //9: 40G BIP Lock
#define PCS_40G_BIP_LOCK_OFFSET                             (0x09)
#define PCS_INVALID_65B_BLOCK_MASK                          (0x0100)    //8: Invalid 65B Block
#define PCS_INVALID_65B_BLOCK_OFFSET                        (0x08)
#define PCS_LDPC_ERROR_EXCEEDED_MASK                        (0x0020)    //5: LDPC Consecutive Errored Frame Exceeded
#define PCS_LDPC_ERROR_EXCEEDED_OFFSET                      (0x05)
#define PCS_STANDARD_STATUS_1_ADDR                          (0x0001)   //PCS Standard Status 1: Address 3.1
#define PCS_FAULT_MASK                                      (0x0080)   //7 PCS Rx Fault
#define PCS_FAULT_OFFSET                                    (0x07)
#define PCS_RECEIVE_LINK_STATUS_MASK                        (0x0004)   //2 PCS Receive Link Status
#define PCS_RECEIVE_LINK_STATUS_OFFSET                      (0x02)
#define PCS_LDPC_1_ITERATION_CORRECTED_FRAMES_MSW_ADDR      (0xE841)   //PCS Receive Vendor Corrected Frame 1 Iteration Counter 2: Address 3.E841
#define PCS_LDPC_1_ITERATION_CORRECTED_FRAMES_LSW_ADDR      (0xE840)   //PCS Receive Vendor Corrected Frame 1 Iteration Counter 1: Address 3.E840
#define PCS_LDPC_1_ITERATION_CORRECTED_FRAMES_MASK          (0xFFFF)   //F:0 LDPC corrected frames which converged in 1 iteration
#define PCS_LDPC_1_ITERATION_CORRECTED_FRAMES_OFFSET        (0)

//XS
#define PHY_STANDARD_STATUS_2_ADDR                          (0x0008)    //PHY XS Standard Status 2: Address 4.8
#define PHY_TX_FAULT_MASK                                   (0x0800)    //B: Transmit Fault
#define PHY_TX_FAULT_OFFSET                                 (0x0B)
#define PHY_RX_FAULT_MASK                                   (0x0400)    //A: Receive Fault
#define PHY_RX_FAULT_OFFSET                                 (0x0A)
#define PHY_XGXS_LANE_STATUS_ADDR                           (0x0018)    //PHY XS Standard XGXS Lane Status: Address 4.18
#define PHY_XGXS_LANE_ALIGN_MASK                            (0x1000)    //C: PHY XGXS Lane Alignment Status
#define PHY_XGXS_LANE_ALIGN_OFFSET                          (0x0C)
#define PHY_XGXS_LANE_SYNC_MASK                             (0x000F)    //3:0 Lane Sync [3:0]
#define PHY_XGXS_LANE_SYNC_OFFSET                           (0x00)
#define PHY_TX_BAD_FRAME_COUNT_LSW_ADDR                     (0xC804)    //PHY XS Transmit (XAUI Rx) PCS Status 3: Address 4.C804
#define PHY_TX_BAD_FRAME_COUNT_MSW_ADDR                     (0xC805)    //PHY XS Transmit (XAUI Rx) PCS Status 4: Address 4.C805
#define PHY_TX_BAD_FRAME_COUNT_MASK                         (0x03FF)    //9:0 Tx Frame Error Counter MSW [9:0]
#define PHY_TX_BAD_FRAME_COUNT_OFFSET                       (0x00)
#define PHY_TRANSMIT_VENDOR_ALARMS_2_ADDR                   (0xCC01)    //PHY XS Transmit (XAUI Rx) Vendor Alarms 2: Address 4.CC01
#define PHY_XAUI_RX_DELETION_MASK                           (0x2000)    //D: XAUI Rx Sequence Ordered Set Deletion
#define PHY_XAUI_RX_DELETION_OFFSET                         (0x0D)
#define PHY_RXAUI_LANE_ALIGN_LOCK_MASK_2                    (0x0800)    //B XAUI Rx Lane Alignment Lock Status [1:0]
#define PHY_RXAUI_LANE_ALIGN_LOCK_MASK_1                    (0x0400)    //A XAUI Rx Lane Alignment Lock Status [1:0]
#define PHY_RXAUI_LANE_ALIGN_LOCK_OFFSET_2                  (0x0B)
#define PHY_RXAUI_LANE_ALIGN_LOCK_OFFSET_1                  (0x0A)
#define PHY_TX_RESERVED_XGMII_CHAR_MASK                     (0x0200)    //9: XAUI Rx Reserved XGMII Character Received
#define PHY_TX_RESERVED_XGMII_CHAR_OFFSET                   (0x09)
#define PHY_TX_INVALID_XGMII_CHAR_MASK                      (0x0100)    //8: XAUI Rx Invalid XGMII Character Received
#define PHY_TX_INVALID_XGMII_CHAR_OFFSET                    (0x08)
#define PHY_TX_CODE_VIOLATION_ERR_MASK_1                    (0x0010)    //4 XAUI Rx Code Violation Error [3:0]
#define PHY_TX_CODE_VIOLATION_ERR_MASK_2                    (0x0020)    //5 XAUI Rx Code Violation Error [3:0]
#define PHY_TX_CODE_VIOLATION_ERR_MASK_3                    (0x0040)    //6 XAUI Rx Code Violation Error [3:0]
#define PHY_TX_CODE_VIOLATION_ERR_MASK_4                    (0x0080)    //7 XAUI Rx Code Violation Error [3:0
#define PHY_TX_CODE_VIOLATION_ERR_OFFSET_1                  (0x04)
#define PHY_TX_CODE_VIOLATION_ERR_OFFSET_2                  (0x05)
#define PHY_TX_CODE_VIOLATION_ERR_OFFSET_3                  (0x06)
#define PHY_TX_CODE_VIOLATION_ERR_OFFSET_4                  (0x07)
#define PHY_TX_RUN_DISPARITY_ERR_MASK_1                     (0x0001)    //0 XAUI Rx Running Disparity Error [3:0]
#define PHY_TX_RUN_DISPARITY_ERR_MASK_2                     (0x0002)    //1 XAUI Rx Running Disparity Error [3:0]
#define PHY_TX_RUN_DISPARITY_ERR_MASK_3                     (0x0004)    //2 XAUI Rx Running Disparity Error [3:0]
#define PHY_TX_RUN_DISPARITY_ERR_MASK_4                     (0x0008)    //3 XAUI Rx Running Disparity Error [3:0]
#define PHY_TX_RUN_DISPARITY_ERR_OFFSET_1                   (0x00)
#define PHY_TX_RUN_DISPARITY_ERR_OFFSET_2                   (0x01)
#define PHY_TX_RUN_DISPARITY_ERR_OFFSET_3                   (0x02)
#define PHY_TX_RUN_DISPARITY_ERR_OFFSET_4                   (0x03)
#define PHY_RX_BAD_FRAME_COUNT_LSW_ADDR                     (0xE804)    //PHY XS Receive (XAUI Tx) PCS Status 3: Address 4.E804
#define PHY_RX_BAD_FRAME_COUNT_MSW_ADDR                     (0xE805)    //PHY XS Receive (XAUI Tx) PCS Status 4: Address 4.E805
#define PHY_RX_BAD_FRAME_COUNT_MASK                         (0x03FF)    //9:0 Tx Frame Error Counter MSW [9:0]
#define PHY_RX_BAD_FRAME_COUNT_OFFSET                       (0x00)
#define PHY_USX_AUTONEGO_NUMBER_ADDR                        (0xE810)    //PHY XS Receive (XAUI Tx) Reserved Vendor State 1: Address 4.E810
#define PHY_RECEIVE_VENDOR_STATE_E_ADDR                     (0xE812)    //PHY XS System Interface Connection Status<PHY XS Receive (XAUI Tx) Reserved Vendor State 3>: Address 4.E812
#define PHY_AUTONEGO_STATUS_MASK                            (0xC000)    //F:E System Interface Autoneg Status[1:0]
#define PHY_AUTONEGO_STATUS_OFFSET                          (0x0E)
#define PHY_RX_LINK_UP_MASK                                 (0x2000)    //D: Rx Link Up
#define PHY_RX_LINK_UP_OFFSET                               (0x0D)
#define PHY_TX_READY_MASK                                (0x1000)    //C: Tx Ready
#define PHY_RX_TX_READY_OFFSET                              (0x0C)
#define PHY_RECEIVE_VENDOR_ALARMS_1_ADDR                    (0xEC00)    //PHY XS Receive (XAUI Tx) Vendor Alarms 1: Address 4.EC00
#define PHY_RX_RESERVED_XGMII_CHAR_MASK                     (0x8000)    //F: Reserved XGMII Character Received from PCS
#define PHY_RX_RESERVED_XGMII_CHAR_OFFSET                   (0x0F)
#define PHY_RX_INVALID_XGMII_CHAR_MASK                      (0x4000)    //E: Invalid XGMII Character Received from PCS
#define PHY_RX_INVALID_XGMII_CHAR_OFFSET                    (0x0E)
#define PHY_RX_LINK_STATUS_MSG_MASK                         (0x2000)    //D: Link status message was received from the PCS
#define PHY_RX_LINK_STATUS_MSG_OFFSET                       (0x0D)
#define PHY_RECEIVE_VENDOR_ALARMS_2_ADDR                    (0xEC01)    //PHY XS Receive (XAUI Tx) Vendor Alarms 2: Address 4.EC01
#define PHY_RX_SYS_RX_LINK_UP_VALUE                         (0x8000)    //F: System Interface Rx Link Up value
#define PHY_RX_SYS_RX_LINK_UP_MASK                          (0x8000)    //F: System Interface Rx Link Up mask
#define PHY_RX_SYS_RX_LINK_UP_OFFSET                        (0x0F)
#define PHY_RX_SYS_RX_LINK_DOWN_MASK                        (0x4000)    //E: System Interface Rx Link Down
#define PHY_RX_SYS_RX_LINK_DOWN_OFFSET                      (0x0E)
#define PHY_RX_SYS_TX_READY_VALUE                           (0x2000)    //D: System Interface Tx Ready bit value
#define PHY_RX_SYS_TX_READY_MASK                            (0x2000)    //D: System Interface Tx Ready mask
#define PHY_RX_SYS_TX_READY_OFFSET                          (0x0D)
#define PHY_RX_SYS_TX_NOT_READY_MASK                        (0x1000)    //C: System Interface Tx Not Ready
#define PHY_RX_SYS_TX_NOT_READY_OFFSET                      (0x0C)
#define PHY_TRANSMIT_LINK_ALIGNMENT_STATUS_ADDR             (0x0001)   //PHY XS Standard Status 1: Address 4.1
#define PHY_TRANSMIT_LINK_ALIGNMENT_STATUS_MASK             (0x0004)   //2 PHY XS Transmit Link Alignment Status
#define PHY_TRANSMIT_LINK_ALIGNMENT_STATUS_OFFSET           (0x02)
#define PHY_SERDES_CALS_ADDR                                (0xC820)   //PHY XS Transmit (XAUI Rx) Reserved Vendor State 1: Address 4.C820
#define PHY_TRANSMIT_VENDOR_STATE_2_ADDR                    (0xC821)   //PHY XS Transmit (XAUI Rx) Reserved Vendor State 2: Address 4.C821
#define PHY_SIF_BLOCK_LOCK_TRANSITIONS_1_0_MASK             (0xFF00)   //F:8 PHY Number of SIF Block Lock Transtitions 1 - 0 [7:0]
#define PHY_SIF_BLOCK_LOCK_TRANSITIONS_1_0_OFFSET           (0x08)
#define PHY_SIF_BLOCK_LOCK_TRANSITIONS_0_1_MASK             (0x00FF)   //7:0 PHY Number of SIF Block Lock Transtitions 0 - 1 [7:0]
#define PHY_SIF_BLOCK_LOCK_TRANSITIONS_0_1_OFFSET           (0)
#define PHY_SIF_XGS_SWITCH_OVERS_ADDR                       (0xC822)   //PHY XS Transmit (XAUI Rx) Reserved Vendor State 2: Address 4.C821
#define PHY_TRANSMIT_VENDOR_ALARMS_3_ADDR                   (0xCC02)   //PHY XS Transmit (XAUI Rx) Vendor Alarms 3: Address 4.CC02
#define PHY_SIGNAL_LOSS_MASK_1                              (0x1000)   //C PHY Loss of signal [3:0]
#define PHY_SIGNAL_LOSS_MASK_2                              (0x2000)   //D PHY Loss of signal [3:0]
#define PHY_SIGNAL_LOSS_MASK_3                              (0x4000)   //E PHY Loss of signal [3:0]
#define PHY_SIGNAL_LOSS_MASK_4                              (0x8000)   //F PHY Loss of signal [3:0]
#define PHY_SIGNAL_LOSS_OFFSET_1                            (0x0C)
#define PHY_SIGNAL_LOSS_OFFSET_2                            (0x0D)
#define PHY_SIGNAL_LOSS_OFFSET_3                            (0x0E)
#define PHY_SIGNAL_LOSS_OFFSET_4                            (0x0F)

//AUTONEG
#define AUTONEG_TRANSMIT_VENDOR_ALARMS_2_ADDR               (0xCC01)    //Autonegotiation Transmit Vendor Alarms 2: Address 7.CC01
#define AUTONEG_LINK_PULSE_DETECT_MASK                      (0x8000)    //F: Link Pulse Detect
#define AUTONEG_LINK_PULSE_DETECT_OFFSET                    (0x0F)
#define AUTONEG_LINK_CONNECT_MASK                           (0x0001)    //0: Link Connect / Disconnect
#define AUTONEG_LINK_CONNECT_OFFSET                         (0x00)
#define AUTONEG_STANDARD_STATUS_1_ADDR                      (0x0001)   //Autonegotiation Standard Status 1: Address 7.1
#define AUTONEG_PARALLEL_DETECTION_FAULT_MASK               (0x0200)   //9 Parallel Detection Fault
#define AUTONEG_PARALLEL_DETECTION_FAULT_OFFSET             (0x09)
#define AUTONEG_REMOTE_FAULT_MASK                           (0x0010)   //4 Remote Fault
#define AUTONEG_REMOTE_FAULT_OFFSET                         (0x04)
#define AUTONEG_LINK_STATUS_MASK                            (0x0004)   //2 Link Status
#define AUTONEG_LINK_STATUS_OFFSET                          (0x02)
#define AUTONEG_10GBASE_T_STATUS_ADDR                       (0x0021)   //Autonegotiation 10GBASE-T Status Register: Address 7.21
#define AUTONEG_MASTER_SLAVE_CONFIG_FAULT_MASK              (0x8000)   //F MASTER-SLAVE Configuration Fault
#define AUTONEG_MASTER_SLAVE_CONFIG_FAULT_OFFSET            (0x0F)
#define AUTONEG_LOCAL_RECEIVER_MASK                         (0x2000)   //D Local Receiver Status
#define AUTONEG_LOCAL_RECEIVER_OFFSET                       (0x0D)
#define AUTONEG_REMOTE_RECEIVER_MASK                        (0x1000)   //C Remote Receiver Status
#define AUTONEG_REMOTE_RECEIVER_OFFSET                      (0x0C)
#define AUTONEG_RESTARTS_HANDLED_ADDR                       (0xC813)   //Autonegotiation Reserved Vendor Status 4: Address 7.C813
#define AUTONEG_LINK_PULSE_DETECTED_ADDR                    (0xC812)   //Autonegotiation Reserved Vendor Status 3: Address 7.C812
#define AUTONEG_LINK_PULSE_DETECTED_MASK                    (0x8000)   //F Link Pulse Detected Status
#define AUTONEG_LINK_PULSE_DETECTED_OFFSET                  (0x0F)
#define AUTONEG_TRANSMIT_VENDOR_ALARMS_1_ADDR               (0xCC00)   //Autonegotiation Transmit Vendor Alarms 1: Address 7.CC00
#define AUTONEG_AUTOMATIC_DOWNSHIFT_MASK                    (0x0002)   //1 Automatic Downshift
#define AUTONEG_AUTOMATIC_DOWNSHIFT_OFFSET                  (0x01)
#define AUTONEG_CONNECTION_STATE_CHANGE_MASK                (0x0001)   //0 Connection State Change
#define AUTONEG_CONNECTION_STATE_CHANGE_OFFSET              (0)
#define AUTONEG_RECEIVE_VENDOR_PROV_ADDR                    (0xE411)   //Autonegotiation Receive Reserved Vendor Provisioning 2: Address 7.E411
#define AUTONEG_10G_DOWNSHIFT_MASK                          (0x0800)   //B Downshift From 10G
#define AUTONEG_10G_DOWNSHIFT_OFFSET                        (0x0B)
#define AUTONEG_5G_DOWNSHIFT_MASK                           (0x0400)   //A Downshift From 5G
#define AUTONEG_5G_DOWNSHIFT_OFFSET                         (0x0A)
#define AUTONEG_2G_DOWNSHIFT_MASK                           (0x0200)   //9 Downshift From 2.5G
#define AUTONEG_2G_DOWNSHIFT_OFFSET                         (0x09)
#define AUTONEG_1G_DOWNSHIFT_MASK                           (0x0100)   //8 Downshift From 1G
#define AUTONEG_1G_DOWNSHIFT_OFFSET                         (0x08)
#define AUTONEG_MAX_ADVERTISED_RATE_MASK                    (0x00F0)   //7:4 Max Advertised Rate [3:0]
#define AUTONEG_MAX_ADVERTISED_RATE_OFFSET                  (0x04)
#define AUTONEG_RECEIVED_VENDOR_ALARMS_2_ADDR               (0xEC01)   //Autonegotiation Receive Vendor Alarms 2: Address 7.EC01
#define AUTONEG_PROTOCOL_ERROR_MASK                         (0x2000)   //D Autonegotiation Protocol Error
#define AUTONEG_PROTOCOL_ERROR_OFFSET                       (0x0D)
#define AUTONEG_FLP_IDLE_ERROR_MASK                         (0x1000)   //C FLP Idle Error
#define AUTONEG_FLP_IDLE_ERROR_OFFSET                       (0x0C)
#define AUTONEG_CONNECTION_STATE_ADDR                       (0xC810)   //Autonegotiation Reserved Vendor Status 1: Address 7.C810
#define AUTONEG_CONNECTION_STATE_MASK                       (0x3E00)   //D:9 Connection state
#define AUTONEG_CONNECTION_STATE_OFFSET                     (0x09)
#define AUTONEG_ATTEMPTS_SINCE_RESET_ADDR                   (0xC814)   //Autonegotiation Reserved Vendor Status 5: Address 7.C814

//GLOBAL
#define GLOBAL_ALARMS_1_ADDR                                (0xCC00)    //Global Alarms 1: Address 1E.CC00
#define GLOBAL_HIGH_TEMP_FAIL_MASK                          (0x4000)    //E: High Temperature Failure
#define GLOBAL_HIGH_TEMP_FAIL_OFFSET                        (0x0E)
#define GLOBAL_LOW_TEMP_FAIL_MASK                           (0x2000)    //D: Low Temperature Failure
#define GLOBAL_LOW_TEMP_FAIL_OFFSET                         (0x0D)
#define GLOBAL_HIGH_TEMP_WARNING_MASK                       (0x1000)    //C: High Temperature Warning
#define GLOBAL_HIGH_TEMP_WARNING_OFFSET                     (0x0C)
#define GLOBAL_LOW_TEMP_WARNING_MASK                        (0x0800)    //B: LOW Temperature Warning
#define GLOBAL_LOW_TEMP_WARNING_OFFSET                      (0x0B)
#define GLOBAL_DEVICE_FAULT_MASK                            (0x0010)    //4: Fault
#define GLOBAL_DEVICE_FAULT_OFFSET                          (0x04)
#define GLOBAL_ALARMS_2_ADDR                                (0xCC01)    //Global Alarms 2: Address 1E.CC01
#define GLOBAL_FAST_LINK_DROP_FAULT_MASK                    (0x4000)    //A: Fast Link Drop
#define GLOBAL_FAST_LINK_DROP_FAULT_OFFSET                  (0x0A)
#define GLOBAL_MDIOD_COMMAND_OVERFLOW_MASK                  (0x0080)    //7: MDIO Command Handling Overflow
#define GLOBAL_MDIOD_COMMAND_OVERFLOW_OFFSET                (0x07)
#define GLOBAL_THERMAL_STATUS_1_ADDR                        (0xC820)    //Global Thermal Status 1: Address 1E.C820
#define GLOBAL_GENERAL_STATUS_1_ADDR                        (0xC830)   //Global General Status 1: Address 1E.C830
#define GLOBAL_HIGH_TEMP_FAILURE_STATE_MASK                 (0x4000)   //E High Temperature Failure State
#define GLOBAL_HIGH_TEMP_FAILURE_STATE_OFFSET               (0x0E)
#define GLOBAL_LOW_TEMP_FAILURE_STATE_MASK                  (0x2000)   //D Low Temperature Failure State
#define GLOBAL_LOW_TEMP_FAILURE_STATE_OFFSET                (0x0D)
#define GLOBAL_HIGH_TEMP_WARNING_STATE_MASK                 (0x1000)   //C High Temperature Warning State
#define GLOBAL_HIGH_TEMP_WARNING_STATE_OFFSET               (0x0C)
#define GLOBAL_LOW_TEMP_WARNING_STATE_MASK                  (0x0800)   //B Low Temperature Warning State
#define GLOBAL_LOW_TEMP_WARNING_STATE_OFFSET                (0x0B)
#define GLOBAL_MESSAGE_ADDR                                 (0xC850)   //Global Fault Message: Address 1E.C850

//DTE XS
#define DTEXS_TX_LOCAL_FAULT_ADDR                           (0x0008)    //DTE TS Status2 (Need to set bb_top.mdio_ctrl.master_bus_sel = 1)
#define DTEXS_TX_LOCAL_FAULT_MASK                           (0x0800)    //B: Transmit Local Fault
#define DTEXS_TX_LOCAL_FAULT_OFFSET                         (0x0B)
#define DTEXS_RX_LOCAL_FAULT_ADDR                           (0x0008)    //DTE TS Status2 (Need to set bb_top.mdio_ctrl.master_bus_sel = 1)
#define DTEXS_RX_LOCAL_FAULT_MASK                           (0x0400)    //A: Receive Local Fault
#define DTEXS_RX_LOCAL_FAULT_OFFSET                         (0x0A)


// Data Types #####################################################################################

// Function Declarations ##########################################################################

// Component Level Variables #######################################################################

#endif /* MDIOD_SRC_MDIOD_AQUANTIA_REGS_H_ */
