/*Icron Technologies ***** Copyright 2015 All Rights Reserved. ******/
/**/
/**** This file is auto generated by IDesignSpec (http://www.agnisys.com) . Please do not edit this file. ****/
/* created by        : Remco van Steeden*/
/* generated by      : Remco.vanSteeden*/
/* generated from    : C:\cygwin64\home\Remco.VanSteeden\blackbird_emulation\m_mdio_master\regs\ids\mdio_master_regs.docx*/
/* IDesignSpec rev   : 6.8.10.0*/

/**** This code is generated with following settings ****/
/* Reg Width                  : 32*/
/* Address Unit               : 8*/
/* C++ Types int              : uint%d_t*/
/* Bus Type                   : APB*/
/* BigEndian                  : true*/
/* LittleEndian               : true*/
/* Dist. Decode and Readback  : false*/
/*--------------------------------------------------------------------------------------------------------------- */

/*block : mdio_master */

#ifndef _MDIO_MASTER_REGS_H_
#define _MDIO_MASTER_REGS_H_

#ifndef __ASSEMBLER__
#ifndef __ASSEMBLER__
typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv24 : 8;
            uint32_t major : 8;           /* 23:16 SW=ro HW=ro 0x3 */
            uint32_t minor : 8;           /* 15:8 SW=ro HW=ro 0x0 */
            uint32_t patch : 8;           /* 7:0 SW=ro HW=ro 0x1 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t patch : 8;           /* 0:7 SW=ro HW=ro 0x1 */
            uint32_t minor : 8;           /* 8:15 SW=ro HW=ro 0x0 */
            uint32_t major : 8;           /* 16:23 SW=ro HW=ro 0x3 */
            uint32_t resv24 : 8;
        } bf;
        
    #endif
    uint32_t  dw;
} mdio_master_version;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv24 : 8;
            uint32_t hold_clks : 8;           /* 23:16 SW=rw HW=ro 0x0 */
            uint32_t setup_clks : 8;           /* 15:8 SW=rw HW=ro 0x0 */
            uint32_t half_mdc_clks : 8;           /* 7:0 SW=rw HW=ro 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t half_mdc_clks : 8;           /* 0:7 SW=rw HW=ro 0x0 */
            uint32_t setup_clks : 8;           /* 8:15 SW=rw HW=ro 0x0 */
            uint32_t hold_clks : 8;           /* 16:23 SW=rw HW=ro 0x0 */
            uint32_t resv24 : 8;
        } bf;
        
    #endif
    uint32_t  dw;
} mdio_master_timing;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv25 : 7;
            uint32_t phyad : 5;           /* 24:20 SW=rw HW=ro 0x0 */
            
            uint32_t resv17 : 3;
            uint32_t regad : 5;           /* 16:12 SW=rw HW=ro 0x0 */
            
            uint32_t resv10 : 2;
            uint32_t op : 2;           /* 9:8 SW=rw HW=ro 0x0 */
            
            uint32_t resv6 : 2;
            uint32_t st : 2;           /* 5:4 SW=rw HW=ro 0x0 */
            
            uint32_t resv1 : 3;
            uint32_t go : 1;           /* 0 SW=rw HW=ro 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t go : 1;           /* 0 SW=rw HW=ro 0x0 */
            
            uint32_t resv1 : 3;
            uint32_t st : 2;           /* 4:5 SW=rw HW=ro 0x0 */
            
            uint32_t resv6 : 2;
            uint32_t op : 2;           /* 8:9 SW=rw HW=ro 0x0 */
            
            uint32_t resv10 : 2;
            uint32_t regad : 5;           /* 12:16 SW=rw HW=ro 0x0 */
            
            uint32_t resv17 : 3;
            uint32_t phyad : 5;           /* 20:24 SW=rw HW=ro 0x0 */
            uint32_t resv25 : 7;
        } bf;
        
    #endif
    uint32_t  dw;
} mdio_master_control;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv16 : 16;
            uint32_t wr_data : 16;           /* 15:0 SW=rw HW=ro 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t wr_data : 16;           /* 0:15 SW=rw HW=ro 0x0 */
            uint32_t resv16 : 16;
        } bf;
        
    #endif
    uint32_t  dw;
} mdio_master_wr_data;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv16 : 16;
            uint32_t rd_data : 16;           /* 15:0 SW=ro HW=wo 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t rd_data : 16;           /* 0:15 SW=ro HW=wo 0x0 */
            uint32_t resv16 : 16;
        } bf;
        
    #endif
    uint32_t  dw;
} mdio_master_rd_data;

/*section : irq */

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv1 : 31;
            uint32_t done : 1;           /* 0 SW=rw HW=na 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t done : 1;           /* 0 SW=rw HW=na 0x0 */
            uint32_t resv1 : 31;
        } bf;
        
    #endif
    uint32_t  dw;
} mdio_master_irq_enable;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv1 : 31;
            uint32_t done : 1;           /* 0 SW=r/w1c HW=wo 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t done : 1;           /* 0 SW=r/w1c HW=wo 0x0 */
            uint32_t resv1 : 31;
        } bf;
        
    #endif
    uint32_t  dw;
} mdio_master_irq_pending;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv1 : 31;
            uint32_t done : 1;           /* 0 SW=ro HW=na 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t done : 1;           /* 0 SW=ro HW=na 0x0 */
            uint32_t resv1 : 31;
        } bf;
        
    #endif
    uint32_t  dw;
} mdio_master_irq_pending_irq;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv1 : 31;
            uint32_t done : 1;           /* 0 SW=ro HW=wo 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t done : 1;           /* 0 SW=ro HW=wo 0x0 */
            uint32_t resv1 : 31;
        } bf;
        
    #endif
    uint32_t  dw;
} mdio_master_irq_raw;

typedef struct {
    mdio_master_irq_enable  enable;
    mdio_master_irq_pending  pending;
    mdio_master_irq_pending_irq  pending_irq;
    mdio_master_irq_raw  raw;
    
    
    
} mdio_master_irq;


typedef struct {
    mdio_master_version  version;
    mdio_master_timing  timing;
    mdio_master_control  control;
    mdio_master_wr_data  wr_data;
    mdio_master_rd_data  rd_data;
    
    union {
        mdio_master_irq s;
        uint8_t filler[0x10];
    } irq;
    
    
    uint8_t filler11[0xDC];
    
    
} mdio_master_s;


#endif   //__ASSEMBLER__


#endif // __ASSEMBLER__
#define mdio_master_version_READMASK 0xFFFFFF
#define mdio_master_version_WRITEMASK 0x0
#define mdio_master_version_VOLATILEMASK 0x0
#define mdio_master_version_RESETMASK 0xFFFFFF
#define mdio_master_version_DEFAULT 0x00030001

#define mdio_master_timing_READMASK 0xFFFFFF
#define mdio_master_timing_WRITEMASK 0xFFFFFF
#define mdio_master_timing_VOLATILEMASK 0x0
#define mdio_master_timing_RESETMASK 0xFFFFFF
#define mdio_master_timing_DEFAULT 0x00000000

#define mdio_master_control_READMASK 0x1F1F331
#define mdio_master_control_WRITEMASK 0x1F1F331
#define mdio_master_control_VOLATILEMASK 0x0
#define mdio_master_control_RESETMASK 0x1F1F331
#define mdio_master_control_DEFAULT 0x00000000

#define mdio_master_wr_data_READMASK 0xFFFF
#define mdio_master_wr_data_WRITEMASK 0xFFFF
#define mdio_master_wr_data_VOLATILEMASK 0x0
#define mdio_master_wr_data_RESETMASK 0xFFFF
#define mdio_master_wr_data_DEFAULT 0x00000000

#define mdio_master_rd_data_READMASK 0xFFFF
#define mdio_master_rd_data_WRITEMASK 0x0
#define mdio_master_rd_data_VOLATILEMASK 0xFFFF
#define mdio_master_rd_data_RESETMASK 0xFFFF
#define mdio_master_rd_data_DEFAULT 0x00000000

#define mdio_master_irq_enable_READMASK 0x1
#define mdio_master_irq_enable_WRITEMASK 0x1
#define mdio_master_irq_enable_VOLATILEMASK 0x0
#define mdio_master_irq_enable_RESETMASK 0x1
#define mdio_master_irq_enable_DEFAULT 0x00000000

#define mdio_master_irq_pending_READMASK 0x1
#define mdio_master_irq_pending_WRITEMASK 0x1
#define mdio_master_irq_pending_VOLATILEMASK 0x1
#define mdio_master_irq_pending_RESETMASK 0x1
#define mdio_master_irq_pending_DEFAULT 0x00000000

#define mdio_master_irq_pending_irq_READMASK 0x1
#define mdio_master_irq_pending_irq_WRITEMASK 0x0
#define mdio_master_irq_pending_irq_VOLATILEMASK 0x0
#define mdio_master_irq_pending_irq_RESETMASK 0x1
#define mdio_master_irq_pending_irq_DEFAULT 0x00000000

#define mdio_master_irq_raw_READMASK 0x1
#define mdio_master_irq_raw_WRITEMASK 0x0
#define mdio_master_irq_raw_VOLATILEMASK 0x1
#define mdio_master_irq_raw_RESETMASK 0x1
#define mdio_master_irq_raw_DEFAULT 0x00000000

#define mdio_master_s_SIZE 0x100
#define mdio_master_version_SIZE 0x4
#define mdio_master_timing_SIZE 0x4
#define mdio_master_control_SIZE 0x4
#define mdio_master_wr_data_SIZE 0x4
#define mdio_master_rd_data_SIZE 0x4
#define mdio_master_irq_SIZE 0x10
#define mdio_master_irq_enable_SIZE 0x4
#define mdio_master_irq_pending_SIZE 0x4
#define mdio_master_irq_pending_irq_SIZE 0x4
#define mdio_master_irq_raw_SIZE 0x4

#define mdio_master_s_OFFSET 0x0
#define mdio_master_version_OFFSET 0x0
#define mdio_master_timing_OFFSET 0x4
#define mdio_master_control_OFFSET 0x8
#define mdio_master_wr_data_OFFSET 0xC
#define mdio_master_rd_data_OFFSET 0x10
#define mdio_master_irq_OFFSET 0x14
#define mdio_master_irq_enable_OFFSET 0x0
#define mdio_master_irq_pending_OFFSET 0x4
#define mdio_master_irq_pending_irq_OFFSET 0x8
#define mdio_master_irq_raw_OFFSET 0xC

#define mdio_master_s_ADDRESS 0x000
#define mdio_master_version_ADDRESS 0x000
#define mdio_master_timing_ADDRESS 0x004
#define mdio_master_control_ADDRESS 0x008
#define mdio_master_wr_data_ADDRESS 0x00C
#define mdio_master_rd_data_ADDRESS 0x010
#define mdio_master_irq_ADDRESS 0x014
#define mdio_master_irq_enable_ADDRESS 0x014
#define mdio_master_irq_pending_ADDRESS 0x018
#define mdio_master_irq_pending_irq_ADDRESS 0x01C
#define mdio_master_irq_raw_ADDRESS 0x020
#define MDIO_MASTER_VERSION_MAJOR_OFFSET 16
#define MDIO_MASTER_VERSION_MAJOR_MASK 0xFF0000
#define MDIO_MASTER_VERSION_MINOR_OFFSET 8
#define MDIO_MASTER_VERSION_MINOR_MASK 0xFF00
#define MDIO_MASTER_VERSION_PATCH_OFFSET 0
#define MDIO_MASTER_VERSION_PATCH_MASK 0xFF
#define MDIO_MASTER_TIMING_HOLD_CLKS_OFFSET 16
#define MDIO_MASTER_TIMING_HOLD_CLKS_MASK 0xFF0000
#define MDIO_MASTER_TIMING_SETUP_CLKS_OFFSET 8
#define MDIO_MASTER_TIMING_SETUP_CLKS_MASK 0xFF00
#define MDIO_MASTER_TIMING_HALF_MDC_CLKS_OFFSET 0
#define MDIO_MASTER_TIMING_HALF_MDC_CLKS_MASK 0xFF
#define MDIO_MASTER_CONTROL_PHYAD_OFFSET 20
#define MDIO_MASTER_CONTROL_PHYAD_MASK 0x1F00000
#define MDIO_MASTER_CONTROL_REGAD_OFFSET 12
#define MDIO_MASTER_CONTROL_REGAD_MASK 0x1F000
#define MDIO_MASTER_CONTROL_OP_OFFSET 8
#define MDIO_MASTER_CONTROL_OP_MASK 0x300
#define MDIO_MASTER_CONTROL_ST_OFFSET 4
#define MDIO_MASTER_CONTROL_ST_MASK 0x30
#define MDIO_MASTER_CONTROL_GO_OFFSET 0
#define MDIO_MASTER_CONTROL_GO_MASK 0x1
#define MDIO_MASTER_CONTROL_GO 0x1
#define MDIO_MASTER_WR_DATA_WR_DATA_OFFSET 0
#define MDIO_MASTER_WR_DATA_WR_DATA_MASK 0xFFFF
#define MDIO_MASTER_RD_DATA_RD_DATA_OFFSET 0
#define MDIO_MASTER_RD_DATA_RD_DATA_MASK 0xFFFF
#define MDIO_MASTER_IRQ_ENABLE_DONE_OFFSET 0
#define MDIO_MASTER_IRQ_ENABLE_DONE_MASK 0x1
#define MDIO_MASTER_IRQ_ENABLE_DONE 0x1
#define MDIO_MASTER_IRQ_PENDING_DONE_OFFSET 0
#define MDIO_MASTER_IRQ_PENDING_DONE_MASK 0x1
#define MDIO_MASTER_IRQ_PENDING_DONE 0x1
#define MDIO_MASTER_IRQ_PENDING_IRQ_DONE_OFFSET 0
#define MDIO_MASTER_IRQ_PENDING_IRQ_DONE_MASK 0x1
#define MDIO_MASTER_IRQ_PENDING_IRQ_DONE 0x1
#define MDIO_MASTER_IRQ_RAW_DONE_OFFSET 0
#define MDIO_MASTER_IRQ_RAW_DONE_MASK 0x1
#define MDIO_MASTER_IRQ_RAW_DONE 0x1
#endif /* _MDIO_MASTER_REGS_H_ */

/* end */
