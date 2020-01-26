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
#ifndef DPCD_H
#define DPCD_H

// Includes #######################################################################################

// Constants and Macros ###########################################################################

// DPCD addresses

// Receiver capability field
#define DPCD_REV                                0x00000
#define MAX_LINK_RATE                           0x00001
#define MAX_LINK_RATE_DEFAULT                   0x14        // Defalut value to MAX_LINK_RATE 5.4Gbps/lane
#define MAX_LANE_COUNT                          0x00002
#define MAX_LANE_COUNT_MASK                     0x1F        // From Bit 0:4 is for MAX_LANE_COUNT
#define MAX_LANE_COUNT_DEFAULT                  0xC4        // Max Lane count = 4, TPS3 supported, Enhanced Framing Enabled
#define TPS3_SUPPORTED                          0x40
#define ENHANCED_FRAMING_ENABLE                 0x80
#define DP_PWR_VLTG_CAP                         0x1F        // Mask to clear the DP power cap
#define MAX_DOWNSPREAD                          0x00003
#define DOWNSPREAD_SUPPORTED                    0x00001     // Up to 0.5% down-spread supported
#define NORP_AND_DP_POWER_VOLTAGE_CAP           0x00004
#define NORP_AND_DP_POWER_VOLTAGE_CAP_DEFAUT    0x01
#define DOWN_STREAM_PORT_PRESENT                0x00005
#define MAIN_LINK_CHANNEL_CODING                0x00006
#define DOWN_STREAM_PORT_COUNT                  0x00007
#define RECEIVE_PORT0_CAP_0                     0x00008
#define RECEIVE_PORT0_CAP_1                     0x00009
#define RECEIVE_PORT1_CAP_0                     0x0000A
#define RECEIVE_PORT1_CAP_1                     0x0000B
#define I2C_SPEED_CONTROL_CAPABILITIES_BIT_MAP  0x0000C
#define TRAINING_AUX_RD_INTERVAL                0x0000E

#define LINK_BW_SET                             0x00100
#define LANE_COUNT_SET                          0x00101
#define TRAINING_PATTERN_SET                    0x00102
#define TRAINING_LANE0_SET                      0x00103
#define VOLTAGE_SWING_SET_OFFSET                0           // Bit0 of TRAINING_LANEX_SET
#define MAX_SWING_REACHED_OFFSET                2           // Bit2 of TRAINING_LANEX_SET
#define PREEMPHASIS_SET_OFFSET                  3           // Bit3 of TRAINING_LANEX_SET
#define MAX_PREEMPHASIS_REACHED_OFFSET          5           // Bit5 of TRAINING_LANEX_SET
#define TRAINING_LANE1_SET                      0x00104
#define TRAINING_LANE2_SET                      0x00105
#define TRAINING_LANE3_SET                      0x00106
#define DOWNSPREAD_CTRL                         0x00107
#define MAIN_LINK_CHANNEL_CODING_SET            0x00108
#define I2C_SPEED_CONTROL_STATUS_BIT_MAP        0x00109
#define EDP_CONFIGURATION_SET                   0x0010A
#define LINK_QUAL_LANE0_SET                     0x0010B
#define LINK_QUAL_LANE1_SET                     0x0010C
#define LINK_QUAL_LANE2_SET                     0x0010D
#define LINK_QUAL_LANE3_SET                     0x0010E
#define RESERVED_POST_CURSOR_2_0                0x0010F
#define RESERVED_POST_CURSOR_2_1                0x00110
#define MSTM_CTRL                               0x00111

#define SINK_COUNT                              0x00200
#define DEVICE_SERVICE_IRQ_VECTOR               0x00201
#define AUTOMATED_TEST_REQUEST                  (1<<1)      // Link test specs 3.1.3 (Dp_v1.4 00201h)
#define SINK_SPECIFIC_IRQ                       (1<<6)      // IRQ Type
#define LANE0_1_STATUS                          0x00202
#define LANE2_3_STATUS                          0x00203
#define LANE_ALIGN_STATUS_UPDATED               0x00204
#define CLEAR_INTERLANE_ALIGN_DONE_OFFSET       0xFE        // Clear Bit0 of LANE_ALIGN_STATUS_UPDATED
#define LINK_STATUS_UPDATED                     (1<<7)      // Bit7 of LANE_ALIGN_STATUS_UPDATED
#define SINK_STATUS                             0x00205
#define ADJUST_REQUEST_LANE0_1                  0x00206
#define ADJUST_REQUEST_LANE2_3                  0x00207

#define TEST_REQUEST                            0x00218     //Automated tests
#define PHY_TEST_PATTERN_ADDR                   0x00248     //Automated tests
#define TEST_LINK_RATE                          0x00219     //Automated tests
#define TEST_LANE_COUNT                         0x00220     //Automated tests
#define TEST_RESPONSE                           0x00260     //Automated tests
#define TEST_80BIT_CUSTOM_PATTERN               0x00250     //Automated tests

#define SOURCE_IEEE_OUI0                        0x00300
#define SOURCE_IEEE_OUI1                        0x00301
#define SOURCE_IEEE_OUI2                        0x00302
#define SOURCE_DIS_0                            0x00303
#define SOURCE_DIS_1                            0x00304
#define SOURCE_DIS_2                            0x00305
#define SOURCE_DIS_3                            0x00306
#define SOURCE_DIS_4                            0x00307
#define SOURCE_DIS_5                            0x00308
#define SOURCE_HARDWARE_REVISION                0x00309
#define SOURCE_FIRMWARE_MAJOR_REVISION          0x0030A
#define SOURCE_FIRMWARE_MINOR_REVISION          0x0030B

#define SET_POWER_AND_SET_DP_PWR_VOLTAGE        0x00600
#define SET_NORMAL_OPERATION                    0x01
#define SET_POWERDOWN                           0x02
#define SET_POWERDOWN_AUX_OPERATION             0x05

#define LINK_SERVICE_IRQ_VECTOR_ESI0            0x02005
#define RX_CAP_CAHANGED                         (1<<0)
#define LINK_STATUS_CHANGED                     (1<<1)
#define LANE0_1_STATUS_ESI                      0x0200C
#define LANE2_3_STATUS_ESI                      0x0200D
#define LANE_ALIGN_STATUS_UPDATED_ESI           0x0200E
#define SINK_STATUS_ESI                         0x0200F

#define EDID_ADDRESS                            0x00050     // AUX I2C addresses
#define MCCS_ADDRESS                            0x00037     // DDC/CI Display Device


// Data Types #####################################################################################

// Function Declarations ##########################################################################

#endif // DPCD_H


