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
//
//!   @file  -  mdiod_phy_regs.h
//
//!   @brief -  Registers header file for mdiod_phy_driver
//
//!   @note  -
//
//#################################################################################################
//#################################################################################################
#ifndef MDIOD_PHY_REGS_H
#define MDIOD_PHY_REGS_H

// Includes #######################################################################################
// Constants and Macros ###########################################################################
/*
 * linux/mii.h: definitions for MII-compatible transceivers
 * Originally drivers/net/sunhme.h.
 *
 * Copyright (C) 1996, 1999, 2001 David S. Miller (davem@redhat.com)
 */
// Generic MII registers.
#define MII_BMCR                            0x00        // Basic mode control register
#define MII_BMSR                            0x01        // Basic mode status register
#define MII_PHYSID1                         0x02        // PHYS ID 1
#define MII_PHYSID2                         0x03        // PHYS ID 2
#define MII_ADVERTISE                       0x04        // Advertisement control reg
//#define MII_LPA                             0x05     // Link partner ability reg
//#define MII_EXPANSION                       0x06     // Expansion register
#define MII_CTRL1000                        0x09        // 1000BASE-T control
//#define MII_STAT1000                        0x0a     // 1000BASE-T status
#define MII_ESTATUS                         0x0f        // Extended Status
#define MII_PHYCR                           (0x10)      // PHY Control register
#define MII_PHYSR                           (0x11)      // PHY Status register
#define MII_INTEN_REG                       (0x12)      // Enable Interrupts
#define MII_INTSTAT_REG                     (0x13)      // Interrupt Status reg - read once to clear ISR

// Basic mode control register, addr        ess 0x00
#define BMCR_RESV                           0x003fU     // Unused...
#define BMCR_SPEED1000                      0x0040U     // MSB of Speed (1000)
#define BMCR_CTST                           0x0080U     // Collision test
#define BMCR_FULLDPLX                       0x0100U     // Full duplex
#define BMCR_ANRESTART                      0x0200U     // Auto negotiation restart
#define BMCR_ISOLATE                        0x0400U     // Disconnect DP83840 from MII
#define BMCR_PDOWN                          0x0800U     // Powerdown the DP83840
#define BMCR_ANENABLE                       0x1000U     // Enable auto negotiation
#define BMCR_SPEED100                       0x2000U     // Select 100Mbps
#define BMCR_SPEED10                        0x0000U     // Select 10Mbps
#define BMCR_LOOPBACK                       0x4000U     // TXD loopback bits
#define BMCR_RESET                          0x8000U     // Reset the DP83840

// Basic mode status register, addre        ss 0x01
#define BMSR_ERCAP                          0x0001U     // Ext-reg capability
#define BMSR_JCD                            0x0002U     // Jabber detected
#define BMSR_LSTATUS                        0x0004U     // Link status
#define BMSR_ANEGCAPABLE                    0x0008U     // Able to do auto-negotiation
#define BMSR_RFAULT                         0x0010U     // Remote fault detected
#define BMSR_ANEGCOMPLETE                   0x0020U     // Auto-negotiation complete
#define BMSR_RESV                           0x00c0U     // Unused...
#define BMSR_ESTATEN                        0x0100U     // Extended Status in R15
#define BMSR_100HALF2                       0x0200U     // Can do 100BASE-T2 HDX
#define BMSR_100FULL2                       0x0400U     // Can do 100BASE-T2 FDX
#define BMSR_10HALF                         0x0800U     // Can do 10mbps, half-duplex
#define BMSR_10FULL                         0x1000U     // Can do 10mbps, full-duplex
#define BMSR_100HALF                        0x2000U     // Can do 100mbps, half-duplex
#define BMSR_100FULL                        0x4000U     // Can do 100mbps, full-duplex
#define BMSR_100BASE4                       0x8000U     // Can do 100mbps, 4k packets

// 1000BAse-T extended status regist        er, addre   ss 0x0F
#define ESTATUS_1000_TFULL                  0x2000U     // Can do 1000BT Full
#define ESTATUS_1000_THALF                  0x1000U     // Can do 1000BT Half


#define MMD_ACCESS_CTRL                     0x0d        // MMD Access Control Register
#define MMD_ACCESS_CTRL_FCN_ADDR            0x0000      // MMD Access Control Addr function
#define MMD_ACCESS_CTRL_FCN_DATA            0x4000      // MMD Access Control Data function
#define MMD_ACCESS_ADDR                     0x0e        // MMD Access Address Data Register
#define MMD_AN                              7           // Auto-neg registers
#define MMD_AN_EEE_ADVERT                   0x3c        // EEE Advertisement Register

// 1000BASE-T Control register
#define ADVERTISE_1000FULL                  0x0200U     // Advertise 1000BASE-T full duplex
#define ADVERTISE_1000HALF                  0x0100U     // Advertise 1000BASE-T half duplex

#define ADVERTISE_10_FULL                   0x441       // PauseFrame, 10BASE-T full, 4:0 are RO and set to 1
#define ADVERTISE_100_FULL                  0x501       // PauseFrame, 100BASE-T full, 4:0 are RO and set to 1
#define ADVERTISE_100_10_FULL               0x541       // PauseFrame, 100 and 10 BASE-T full, 4:0 are RO and set to 1

#define PHY_ADDR_MAX                        31          // Highest valid MDIO address

#define PHY_DISCOVERY_TIMEOUT               1000000     // 1sec phy discovery timeout

#define PHYCR_DISABLE_CLK125                (0x10)      // disable the 125 MHz clk used by 1G

#define PHYCR_ENABLE_CLK125                 (0xFFEF)    // Enable the 125 MHz clk used by 1G
#define PHYSR_LINK_STATUS                   (0x400)     // Phy Real Time Link Status bit
#define MII_INTSTAT_REG_LINK_STATE_CHNG     (0x400)     // Link Status Change

// thresholds used to determine the speed of the connected device we negotiated with
#define MII_10_FREQ_THRESHOLD_LOWER         (2000000)
#define MII_10_FREQ_THRESHOLD_UPPER         (3000000)
#define MII_100_FREQ_THRESHOLD_LOWER        (20000000)
#define MII_100_FREQ_THRESHOLD_UPPER        (30000000)
#define GMII_FREQ_THRESHOLD_LOWER           (120000000)
#define GMII_FREQ_THRESHOLD_UPPER           (130000000)

// Generic Page Selections
#define MII_EPAGSR                          0x1e        // Extension Page Select Register
#define MII_PAGSEL                          0x1f        // Page Select Register
#define MII_PAGSEL_ZERO                     0x0000U     // Page zero - default MII register access
#define MII_PAGSEL_EXT                      0x0007U     // Extension page

// Realtek PHY identifiers
#define PHYID_REVISION_MASK                 0xF
#define REALTEK8211E_PHYID_MSB              0x001CU
#define REALTEK8211E_PHYID_LSB              0xC915U

#define REALTEK8211E_LACR_REG               0x1AU
#define REALTEK8211E_LACR_LED0_OFFSET       0x04U
#define REALTEK8211E_LACR_LED1_OFFSET       0x05U
#define REALTEK8211E_LACR_LED2_OFFSET       0x06U

#define REALTEK8211E_LCR_PAGE               0x2CU
#define REALTEK8211E_LCR_REG                0x1CU
#define REALTEK8211E_LCR_LED0_OFFSET        0x00U
#define REALTEK8211E_LCR_LED1_OFFSET        0x04U
#define REALTEK8211E_LCR_LED2_OFFSET        0x08U
#define REALTEK8211E_LCR_10M_FIELD_OFFSET   0x00U
#define REALTEK8211E_LCR_100M_FIELD_OFFSET  0x01U
#define REALTEK8211E_LCR_1000M_FIELD_OFFSET 0x02U
#define REALTEK8211E_EEE_DISABLE_PAGE       0x05U
#define REALTEK8211E_EEE_DISABLE_REG5       0x05U
#define REALTEK8211E_EEE_DISABLE_REG5_DATA  0x8B82U
#define REALTEK8211E_EEE_DISABLE_REG6       0x06U
#define REALTEK8211E_EEE_DISABLE_REG6_DATA  0x052BU

// Data Types #####################################################################################

#endif  // MDIOD_PHY_REGS_H
