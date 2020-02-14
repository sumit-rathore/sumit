///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2007, 2008, 2009
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
//!   @file  -  linkmgr_mii.h
//
//!   @brief -  Some def's for the 802.3 standard MII registers
//
//
//!   @note  - These definitions were taken from:
//     http://linux.sourcearchive.com/documentation/2.6.28-2.2/mii_8h-source.html
//
//      They were modified according to the needs of this project.
//
//
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LINKMGR_MII_H
#define LINKMGR_MII_H

/***************************** Included Headers ******************************/

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/*********************************** API *************************************/

/*
 * linux/mii.h: definitions for MII-compatible transceivers
 * Originally drivers/net/sunhme.h.
 *
 * Copyright (C) 1996, 1999, 2001 David S. Miller (davem@redhat.com)
 */


// Generic MII registers.

#define MII_BMCR                    0x00        // Basic mode control register
#define MII_BMSR                    0x01        // Basic mode status register
#define MII_PHYSID1                 0x02        // PHYS ID 1
#define MII_PHYSID2                 0x03        // PHYS ID 2
#define MII_ADVERTISE               0x04        // Advertisement control reg
#define MII_LPA                     0x05        // Link partner ability reg
#define MII_EXPANSION               0x06        // Expansion register
#define MII_CTRL1000                0x09        // 1000BASE-T control
#define MII_STAT1000                0x0a        // 1000BASE-T status
#define MII_ESTATUS                 0x0f        // Extended Status
#define MII_EPAGSR                  0x1e        // Extension Page Select Register
#define MII_PAGSEL                  0x1f        // Page Select Register

#if 0
#define MII_DCOUNTER                0x12        // Disconnect counter
#define MII_FCSCOUNTER              0x13        // False carrier counter
#define MII_NWAYTEST                0x14        // N-way auto-neg test reg
#define MII_RERRCOUNTER             0x15        // Receive error counter
#define MII_SREVISION               0x16        // Silicon revision
#define MII_RESV1                   0x17        // Reserved...
#define MII_LBRERROR                0x18        // Lpback, rx, bypass error
#define MII_PHYADDR                 0x19        // PHY address
#define MII_RESV2                   0x1a        // Reserved...
#define MII_TPISTATUS               0x1b        // TPI status for 10mbps
#define MII_NCONFIG                 0x1c        // Network interface config
#endif

// Basic mode control register.
#define BMCR_RESV                   0x003fU  // Unused...
#define BMCR_SPEED1000              0x0040U  // MSB of Speed (1000)
#define BMCR_CTST                   0x0080U  // Collision test
#define BMCR_FULLDPLX               0x0100U  // Full duplex
#define BMCR_ANRESTART              0x0200U  // Auto negotiation restart
#define BMCR_ISOLATE                0x0400U  // Disconnect DP83840 from MII
#define BMCR_PDOWN                  0x0800U  // Powerdown the DP83840
#define BMCR_ANENABLE               0x1000U  // Enable auto negotiation
#define BMCR_SPEED100               0x2000U  // Select 100Mbps
#define BMCR_LOOPBACK               0x4000U  // TXD loopback bits
#define BMCR_RESET                  0x8000U  // Reset the DP83840

// Basic mode status register.
#define BMSR_ERCAP                  0x0001U  // Ext-reg capability
#define BMSR_JCD                    0x0002U  // Jabber detected
#define BMSR_LSTATUS                0x0004U  // Link status
#define BMSR_ANEGCAPABLE            0x0008U  // Able to do auto-negotiation
#define BMSR_RFAULT                 0x0010U  // Remote fault detected
#define BMSR_ANEGCOMPLETE           0x0020U  // Auto-negotiation complete
#define BMSR_RESV                   0x00c0U  // Unused...
#define BMSR_ESTATEN                0x0100U  // Extended Status in R15
#define BMSR_100HALF2               0x0200U  // Can do 100BASE-T2 HDX
#define BMSR_100FULL2               0x0400U  // Can do 100BASE-T2 FDX
#define BMSR_10HALF                 0x0800U  // Can do 10mbps, half-duplex
#define BMSR_10FULL                 0x1000U  // Can do 10mbps, full-duplex
#define BMSR_100HALF                0x2000U  // Can do 100mbps, half-duplex
#define BMSR_100FULL                0x4000U  // Can do 100mbps, full-duplex
#define BMSR_100BASE4               0x8000U  // Can do 100mbps, 4k packets

// Advertisement control register.
#define ADVERTISE_SLCT              0x001fU  // Selector bits
#define ADVERTISE_CSMA              0x0001U  // Only selector supported
#define ADVERTISE_10HALF            0x0020U  // Try for 10mbps half-duplex
#define ADVERTISE_1000XFULL         0x0020U  // Try for 1000BASE-X full-duplex
#define ADVERTISE_10FULL            0x0040U  // Try for 10mbps full-duplex
#define ADVERTISE_1000XHALF         0x0040U  // Try for 1000BASE-X half-duplex
#define ADVERTISE_100HALF           0x0080U  // Try for 100mbps half-duplex
#define ADVERTISE_1000XPAUSE        0x0080U  // Try for 1000BASE-X pause
#define ADVERTISE_100FULL           0x0100U  // Try for 100mbps full-duplex
#define ADVERTISE_1000XPSE_ASYM     0x0100U  // Try for 1000BASE-X asym pause
#define ADVERTISE_100BASE4          0x0200U  // Try for 100mbps 4k packets
#define ADVERTISE_PAUSE_CAP         0x0400U  // Try for pause
#define ADVERTISE_PAUSE_ASYM        0x0800U  // Try for asymetric pause
#define ADVERTISE_RESV              0x1000U  // Unused...
#define ADVERTISE_RFAULT            0x2000U  // Say we can detect faults
#define ADVERTISE_LPACK             0x4000U  // Ack link partners response
#define ADVERTISE_NPAGE             0x8000U  // Next page bit

#define ADVERTISE_FULL (ADVERTISE_100FULL | ADVERTISE_10FULL | \
                  ADVERTISE_CSMA)
#define ADVERTISE_ALL (ADVERTISE_10HALF | ADVERTISE_10FULL | \
                       ADVERTISE_100HALF | ADVERTISE_100FULL)

// Link partner ability register.
#define LPA_SLCT                    0x001fU  // Same as advertise selector
#define LPA_10HALF                  0x0020U  // Can do 10mbps half-duplex
#define LPA_1000XFULL               0x0020U  // Can do 1000BASE-X full-duplex
#define LPA_10FULL                  0x0040U  // Can do 10mbps full-duplex
#define LPA_1000XHALF               0x0040U  // Can do 1000BASE-X half-duplex
#define LPA_100HALF                 0x0080U  // Can do 100mbps half-duplex
#define LPA_1000XPAUSE              0x0080U  // Can do 1000BASE-X pause
#define LPA_100FULL                 0x0100U  // Can do 100mbps full-duplex
#define LPA_1000XPAUSE_ASYM         0x0100U  // Can do 1000BASE-X pause asym
#define LPA_100BASE4                0x0200U  // Can do 100mbps 4k packets
#define LPA_PAUSE_CAP               0x0400U  // Can pause
#define LPA_PAUSE_ASYM              0x0800U  // Can pause asymetrically
#define LPA_RESV                    0x1000U  // Unused...
#define LPA_RFAULT                  0x2000U  // Link partner faulted
#define LPA_LPACK                   0x4000U  // Link partner acked us
#define LPA_NPAGE                   0x8000U  // Next page bit

#define LPA_DUPLEX                  (LPA_10FULL | LPA_100FULL)
#define LPA_100                     (LPA_100FULL | LPA_100HALF | LPA_100BASE4)

// Expansion register for auto-negotiation.
#define EXPANSION_NWAY              0x0001U  // Can do N-way auto-nego
#define EXPANSION_LCWP              0x0002U  // Got new RX page code word
#define EXPANSION_ENABLENPAGE       0x0004U  // This enables npage words
#define EXPANSION_NPCAPABLE         0x0008U  // Link partner supports npage
#define EXPANSION_MFAULTS           0x0010U  // Multiple faults detected
#define EXPANSION_RESV              0xffe0U  // Unused...

#define ESTATUS_1000_TFULL          0x2000U  // Can do 1000BT Full
#define ESTATUS_1000_THALF          0x1000U  // Can do 1000BT Half

// N-way test register.
#define NWAYTEST_RESV1              0x00ffU  // Unused...
#define NWAYTEST_LOOPBACK           0x0100U  // Enable loopback for N-way
#define NWAYTEST_RESV2              0xfe00U  // Unused...

// 1000BASE-T Control register
#define ADVERTISE_1000FULL          0x0200U  // Advertise 1000BASE-T full duplex
#define ADVERTISE_1000HALF          0x0100U  // Advertise 1000BASE-T half duplex

// 1000BASE-T Status register
#define LPA_1000LOCALRXOK           0x2000U  // Link partner local receiver status
#define LPA_1000REMRXOK             0x1000U  // Link partner remote receiver status
#define LPA_1000FULL                0x0800U  // Link partner 1000BASE-T full duplex
#define LPA_1000HALF                0x0400U  // Link partner 1000BASE-T half duplex

// Generic Page Selections
#define MII_PAGSEL_ZERO             0x0000U  // Page zero - default MII register access
#define MII_PAGSEL_EXT              0x0007U  // Extension page

// Realtek PHY identifiers
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

// Broadcom PHY identifiers
// last 4 bits of of the LSB will change with revision #
#define PHYID_REVISION_MASK         0xF
#define BCM5481_PHYID_MSB           0x0143U
#define BCM5481_PHYID_LSB           0xBCA0U
// BroadR-Reach PHY identifiers
#define BCM54810_PHYID_MSB          0x0362U
#define BCM54810_PHYID_LSB          0x5D00U

// BroadR-Reach PHY LRE registers
#define LRE_CONTROL                 0x00    // LRE Control register
#define LRE_STATUS                  0x01    // LRE Status register
#define LDS_ADVERTISE               0x04    // Advertisement ability reg
#define LDS_ADVERTISE_CTRL          0x05    // Advertisement control reg
#define LDS_LPA                     0x07    // Link partner ability reg
#define LDS_EXPANSION               0x0a    // Expansion register
#define MII_CTRL1000                0x09    // 1000BASE-T control
#define MII_STAT1000                0x0a    // 1000BASE-T status
#define LRE_ACCESS                  0x0e    // LRE Access register
#define LRE_ESTATUS                 0x0f    // Extended Status

#define LRE_EXPANSION_ACCESS        0x17    // Expansion register access register
#define LRE_EXPANSION_REG           0x15    // Expansion register access register
#define LRE_EXPANSION_SEL_EN        0x0F00  // Expansion register access enable
#define LRE_EXP_LRE_MISC_CTRL       0x91    // Expansion register LRE MISC control
#define LRE_EXP_LRE_MISC_BER        0x0008  // LRE MISC control BER

#define LRE_ACCESS_EN               0x0001  // Enable LRE register set
#define LRE_SPEED_SEL_100MBPS       0x0200  // 100Mbps speed select
#define LRE_2PAIR_SEL               0x0010  // 2 pair select
#define LRE_4PAIR_SEL               0x0020  // 4 pair select
#define LRE_MASTER_SEL              0x0008  // Master select
#define LRE_LINK_STATUS_UP          0x0004  // Link status up

#define LRE_IEEE_MASK               0x1
#define LRE_MODE_IEEE               0x0     // IEEE register set mode
#define LRE_MODE_LRE                0x1     // LRE register set mode
#define LRE_IEEE_OVERRIDE           0x4     // Override to enable setting between IEEE and LRE mode
#define LRE_IEEE_NORMAL             0x0     // Normal operation

#endif // LINKMGR_MII_H

