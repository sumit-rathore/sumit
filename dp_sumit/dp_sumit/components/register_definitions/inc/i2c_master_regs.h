/*Icron Technologies ***** Copyright 2015 All Rights Reserved. ******/
/**/
/**** This file is auto generated by IDesignSpec (http://www.agnisys.com) . Please do not edit this file. ****/
/* created by        : Remco van Steeden*/
/* generated by      : Remco.vanSteeden*/
/* generated from    : C:\cygwin64\home\Remco.VanSteeden\blackbird_vc707\m_i2c_master\regs\ids\i2c_master_regs.docx*/
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

/*block : i2c_master */

#ifndef _I2C_MASTER_REGS_H_
#define _I2C_MASTER_REGS_H_

#ifndef __ASSEMBLER__
#ifndef __ASSEMBLER__
typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv24 : 8;
            uint32_t major : 8;           /* 23:16 SW=ro HW=ro 0x3 */
            uint32_t minor : 8;           /* 15:8 SW=ro HW=ro 0x0 */
            uint32_t patch : 8;           /* 7:0 SW=ro HW=ro 0x3 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t patch : 8;           /* 0:7 SW=ro HW=ro 0x3 */
            uint32_t minor : 8;           /* 8:15 SW=ro HW=ro 0x0 */
            uint32_t major : 8;           /* 16:23 SW=ro HW=ro 0x3 */
            uint32_t resv24 : 8;
        } bf;
        
    #endif
    uint32_t  dw;
} i2c_master_version;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv16 : 16;
            uint32_t prescaler : 16;           /* 15:0 SW=rw HW=ro 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t prescaler : 16;           /* 0:15 SW=rw HW=ro 0x0 */
            uint32_t resv16 : 16;
        } bf;
        
    #endif
    uint32_t  dw;
} i2c_master_prescaler;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv28 : 4;
            uint32_t fifo_af : 1;           /* 27 SW=ro HW=wo 0x0 */
            uint32_t fifo_ae : 1;           /* 26 SW=ro HW=wo 0x0 */
            uint32_t fifo_full : 1;           /* 25 SW=ro HW=wo 0x0 */
            uint32_t fifo_empty : 1;           /* 24 SW=ro HW=wo 0x0 */
            uint32_t fifo_aft : 8;           /* 23:16 SW=rw HW=ro 0x0 */
            uint32_t fifo_aet : 8;           /* 15:8 SW=rw HW=ro 0x0 */
            uint32_t fifo_depth : 8;           /* 7:0 SW=ro HW=wo 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t fifo_depth : 8;           /* 0:7 SW=ro HW=wo 0x0 */
            uint32_t fifo_aet : 8;           /* 8:15 SW=rw HW=ro 0x0 */
            uint32_t fifo_aft : 8;           /* 16:23 SW=rw HW=ro 0x0 */
            uint32_t fifo_empty : 1;           /* 24 SW=ro HW=wo 0x0 */
            uint32_t fifo_full : 1;           /* 25 SW=ro HW=wo 0x0 */
            uint32_t fifo_ae : 1;           /* 26 SW=ro HW=wo 0x0 */
            uint32_t fifo_af : 1;           /* 27 SW=ro HW=wo 0x0 */
            uint32_t resv28 : 4;
        } bf;
        
    #endif
    uint32_t  dw;
} i2c_master_fifo;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            uint32_t error : 1;           /* 31 SW=ro HW=wo 0x0 */
            uint32_t clock_stretch_active : 1;           /* 30 SW=ro HW=wo 0x0 */
            
            uint32_t resv26 : 4;
            uint32_t force_sda_low : 1;           /* 25 SW=rw HW=ro 0x0 */
            
            uint32_t resv21 : 4;
            uint32_t slave_addr : 7;           /* 20:14 SW=rw HW=ro 0x0 */
            uint32_t read_byte_count : 8;           /* 13:6 SW=rw HW=ro 0x0 */
            uint32_t clear : 1;           /* 5 SW=rw HW=ro 0x0 */
            
            uint32_t resv4 : 1;
            uint32_t opmode : 2;           /* 3:2 SW=rw HW=ro 0x0 */
            uint32_t clock_stretch_en : 1;           /* 1 SW=rw HW=ro 0x1 */
            uint32_t go : 1;           /* 0 SW=rw HW=ro 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t go : 1;           /* 0 SW=rw HW=ro 0x0 */
            uint32_t clock_stretch_en : 1;           /* 1 SW=rw HW=ro 0x1 */
            uint32_t opmode : 2;           /* 2:3 SW=rw HW=ro 0x0 */
            
            uint32_t resv4 : 1;
            uint32_t clear : 1;           /* 5 SW=rw HW=ro 0x0 */
            uint32_t read_byte_count : 8;           /* 6:13 SW=rw HW=ro 0x0 */
            uint32_t slave_addr : 7;           /* 14:20 SW=rw HW=ro 0x0 */
            
            uint32_t resv21 : 4;
            uint32_t force_sda_low : 1;           /* 25 SW=rw HW=ro 0x0 */
            
            uint32_t resv26 : 4;
            uint32_t clock_stretch_active : 1;           /* 30 SW=ro HW=wo 0x0 */
            uint32_t error : 1;           /* 31 SW=ro HW=wo 0x0 */
        } bf;
        
    #endif
    uint32_t  dw;
} i2c_master_control;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv8 : 24;
            uint32_t wr_data : 8;           /* 7:0 SW=wo HW=ro 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t wr_data : 8;           /* 0:7 SW=wo HW=ro 0x0 */
            uint32_t resv8 : 24;
        } bf;
        
    #endif
    uint32_t  dw;
} i2c_master_wr_data;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv8 : 24;
            uint32_t rd_data : 8;           /* 7:0 SW=ro HW=wo 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t rd_data : 8;           /* 0:7 SW=ro HW=wo 0x0 */
            uint32_t resv8 : 24;
        } bf;
        
    #endif
    uint32_t  dw;
} i2c_master_rd_data;

/*section : irq */

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv3 : 29;
            uint32_t fifo_ae : 1;           /* 2 SW=rw HW=na 0x0 */
            uint32_t fifo_af : 1;           /* 1 SW=rw HW=na 0x0 */
            uint32_t done : 1;           /* 0 SW=rw HW=na 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t done : 1;           /* 0 SW=rw HW=na 0x0 */
            uint32_t fifo_af : 1;           /* 1 SW=rw HW=na 0x0 */
            uint32_t fifo_ae : 1;           /* 2 SW=rw HW=na 0x0 */
            uint32_t resv3 : 29;
        } bf;
        
    #endif
    uint32_t  dw;
} i2c_master_irq_enable;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv3 : 29;
            uint32_t fifo_ae : 1;           /* 2 SW=r/w1c HW=wo 0x0 */
            uint32_t fifo_af : 1;           /* 1 SW=r/w1c HW=wo 0x0 */
            uint32_t done : 1;           /* 0 SW=r/w1c HW=wo 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t done : 1;           /* 0 SW=r/w1c HW=wo 0x0 */
            uint32_t fifo_af : 1;           /* 1 SW=r/w1c HW=wo 0x0 */
            uint32_t fifo_ae : 1;           /* 2 SW=r/w1c HW=wo 0x0 */
            uint32_t resv3 : 29;
        } bf;
        
    #endif
    uint32_t  dw;
} i2c_master_irq_pending;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv3 : 29;
            uint32_t fifo_ae : 1;           /* 2 SW=ro HW=na 0x0 */
            uint32_t fifo_af : 1;           /* 1 SW=ro HW=na 0x0 */
            uint32_t done : 1;           /* 0 SW=ro HW=na 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t done : 1;           /* 0 SW=ro HW=na 0x0 */
            uint32_t fifo_af : 1;           /* 1 SW=ro HW=na 0x0 */
            uint32_t fifo_ae : 1;           /* 2 SW=ro HW=na 0x0 */
            uint32_t resv3 : 29;
        } bf;
        
    #endif
    uint32_t  dw;
} i2c_master_irq_pending_irq;

typedef union {
    #ifdef IDS_BIG_ENDIAN
        struct {
            
            uint32_t resv3 : 29;
            uint32_t fifo_ae : 1;           /* 2 SW=ro HW=wo 0x0 */
            uint32_t fifo_af : 1;           /* 1 SW=ro HW=wo 0x0 */
            uint32_t done : 1;           /* 0 SW=ro HW=wo 0x0 */
        } bf;
        
    #else     /* IDS_LITTLE_ENDIAN */
        struct {
            uint32_t done : 1;           /* 0 SW=ro HW=wo 0x0 */
            uint32_t fifo_af : 1;           /* 1 SW=ro HW=wo 0x0 */
            uint32_t fifo_ae : 1;           /* 2 SW=ro HW=wo 0x0 */
            uint32_t resv3 : 29;
        } bf;
        
    #endif
    uint32_t  dw;
} i2c_master_irq_raw;

typedef struct {
    i2c_master_irq_enable  enable;
    i2c_master_irq_pending  pending;
    i2c_master_irq_pending_irq  pending_irq;
    i2c_master_irq_raw  raw;
    
    
    
} i2c_master_irq;


typedef struct {
    i2c_master_version  version;
    i2c_master_prescaler  prescaler;
    i2c_master_fifo  fifo;
    i2c_master_control  control;
    i2c_master_wr_data  wr_data;
    i2c_master_rd_data  rd_data;
    
    union {
        i2c_master_irq s;
        uint8_t filler[0x10];
    } irq;
    
    
    uint8_t filler11[0xD8];
    
    
} i2c_master_s;


#endif   //__ASSEMBLER__


#endif // __ASSEMBLER__
#define i2c_master_version_READMASK 0xFFFFFF
#define i2c_master_version_WRITEMASK 0x0
#define i2c_master_version_VOLATILEMASK 0x0
#define i2c_master_version_RESETMASK 0xFFFFFF
#define i2c_master_version_DEFAULT 0x00030003

#define i2c_master_prescaler_READMASK 0xFFFF
#define i2c_master_prescaler_WRITEMASK 0xFFFF
#define i2c_master_prescaler_VOLATILEMASK 0x0
#define i2c_master_prescaler_RESETMASK 0xFFFF
#define i2c_master_prescaler_DEFAULT 0x00000000

#define i2c_master_fifo_READMASK 0xFFFFFFF
#define i2c_master_fifo_WRITEMASK 0xFFFF00
#define i2c_master_fifo_VOLATILEMASK 0xF0000FF
#define i2c_master_fifo_RESETMASK 0xFFFFFFF
#define i2c_master_fifo_DEFAULT 0x00000000

#define i2c_master_control_READMASK 0xC21FFFEF
#define i2c_master_control_WRITEMASK 0x21FFFEF
#define i2c_master_control_VOLATILEMASK 0xC0000000
#define i2c_master_control_RESETMASK 0xC21FFFEF
#define i2c_master_control_DEFAULT 0x00000002

#define i2c_master_wr_data_READMASK 0x0
#define i2c_master_wr_data_WRITEMASK 0xFF
#define i2c_master_wr_data_VOLATILEMASK 0x0
#define i2c_master_wr_data_RESETMASK 0xFF
#define i2c_master_wr_data_DEFAULT 0x00000000

#define i2c_master_rd_data_READMASK 0xFF
#define i2c_master_rd_data_WRITEMASK 0x0
#define i2c_master_rd_data_VOLATILEMASK 0xFF
#define i2c_master_rd_data_RESETMASK 0xFF
#define i2c_master_rd_data_DEFAULT 0x00000000

#define i2c_master_irq_enable_READMASK 0x7
#define i2c_master_irq_enable_WRITEMASK 0x7
#define i2c_master_irq_enable_VOLATILEMASK 0x0
#define i2c_master_irq_enable_RESETMASK 0x7
#define i2c_master_irq_enable_DEFAULT 0x00000000

#define i2c_master_irq_pending_READMASK 0x7
#define i2c_master_irq_pending_WRITEMASK 0x7
#define i2c_master_irq_pending_VOLATILEMASK 0x7
#define i2c_master_irq_pending_RESETMASK 0x7
#define i2c_master_irq_pending_DEFAULT 0x00000000

#define i2c_master_irq_pending_irq_READMASK 0x7
#define i2c_master_irq_pending_irq_WRITEMASK 0x0
#define i2c_master_irq_pending_irq_VOLATILEMASK 0x0
#define i2c_master_irq_pending_irq_RESETMASK 0x7
#define i2c_master_irq_pending_irq_DEFAULT 0x00000000

#define i2c_master_irq_raw_READMASK 0x7
#define i2c_master_irq_raw_WRITEMASK 0x0
#define i2c_master_irq_raw_VOLATILEMASK 0x7
#define i2c_master_irq_raw_RESETMASK 0x7
#define i2c_master_irq_raw_DEFAULT 0x00000000

#define i2c_master_s_SIZE 0x100
#define i2c_master_version_SIZE 0x4
#define i2c_master_prescaler_SIZE 0x4
#define i2c_master_fifo_SIZE 0x4
#define i2c_master_control_SIZE 0x4
#define i2c_master_wr_data_SIZE 0x4
#define i2c_master_rd_data_SIZE 0x4
#define i2c_master_irq_SIZE 0x10
#define i2c_master_irq_enable_SIZE 0x4
#define i2c_master_irq_pending_SIZE 0x4
#define i2c_master_irq_pending_irq_SIZE 0x4
#define i2c_master_irq_raw_SIZE 0x4

#define i2c_master_s_OFFSET 0x0
#define i2c_master_version_OFFSET 0x0
#define i2c_master_prescaler_OFFSET 0x4
#define i2c_master_fifo_OFFSET 0x8
#define i2c_master_control_OFFSET 0xC
#define i2c_master_wr_data_OFFSET 0x10
#define i2c_master_rd_data_OFFSET 0x14
#define i2c_master_irq_OFFSET 0x18
#define i2c_master_irq_enable_OFFSET 0x0
#define i2c_master_irq_pending_OFFSET 0x4
#define i2c_master_irq_pending_irq_OFFSET 0x8
#define i2c_master_irq_raw_OFFSET 0xC

#define i2c_master_s_ADDRESS 0x000
#define i2c_master_version_ADDRESS 0x000
#define i2c_master_prescaler_ADDRESS 0x004
#define i2c_master_fifo_ADDRESS 0x008
#define i2c_master_control_ADDRESS 0x00C
#define i2c_master_wr_data_ADDRESS 0x010
#define i2c_master_rd_data_ADDRESS 0x014
#define i2c_master_irq_ADDRESS 0x018
#define i2c_master_irq_enable_ADDRESS 0x018
#define i2c_master_irq_pending_ADDRESS 0x01C
#define i2c_master_irq_pending_irq_ADDRESS 0x020
#define i2c_master_irq_raw_ADDRESS 0x024
#define I2C_MASTER_VERSION_MAJOR_OFFSET 16
#define I2C_MASTER_VERSION_MAJOR_MASK 0xFF0000
#define I2C_MASTER_VERSION_MINOR_OFFSET 8
#define I2C_MASTER_VERSION_MINOR_MASK 0xFF00
#define I2C_MASTER_VERSION_PATCH_OFFSET 0
#define I2C_MASTER_VERSION_PATCH_MASK 0xFF
#define I2C_MASTER_PRESCALER_PRESCALER_OFFSET 0
#define I2C_MASTER_PRESCALER_PRESCALER_MASK 0xFFFF
#define I2C_MASTER_FIFO_FIFO_AF_OFFSET 27
#define I2C_MASTER_FIFO_FIFO_AF_MASK 0x8000000
#define I2C_MASTER_FIFO_FIFO_AF 0x8000000
#define I2C_MASTER_FIFO_FIFO_AE_OFFSET 26
#define I2C_MASTER_FIFO_FIFO_AE_MASK 0x4000000
#define I2C_MASTER_FIFO_FIFO_AE 0x4000000
#define I2C_MASTER_FIFO_FIFO_FULL_OFFSET 25
#define I2C_MASTER_FIFO_FIFO_FULL_MASK 0x2000000
#define I2C_MASTER_FIFO_FIFO_FULL 0x2000000
#define I2C_MASTER_FIFO_FIFO_EMPTY_OFFSET 24
#define I2C_MASTER_FIFO_FIFO_EMPTY_MASK 0x1000000
#define I2C_MASTER_FIFO_FIFO_EMPTY 0x1000000
#define I2C_MASTER_FIFO_FIFO_AFT_OFFSET 16
#define I2C_MASTER_FIFO_FIFO_AFT_MASK 0xFF0000
#define I2C_MASTER_FIFO_FIFO_AET_OFFSET 8
#define I2C_MASTER_FIFO_FIFO_AET_MASK 0xFF00
#define I2C_MASTER_FIFO_FIFO_DEPTH_OFFSET 0
#define I2C_MASTER_FIFO_FIFO_DEPTH_MASK 0xFF
#define I2C_MASTER_CONTROL_ERROR_OFFSET 31
#define I2C_MASTER_CONTROL_ERROR_MASK 0x80000000
#define I2C_MASTER_CONTROL_ERROR 0x80000000
#define I2C_MASTER_CONTROL_CLOCK_STRETCH_ACTIVE_OFFSET 30
#define I2C_MASTER_CONTROL_CLOCK_STRETCH_ACTIVE_MASK 0x40000000
#define I2C_MASTER_CONTROL_CLOCK_STRETCH_ACTIVE 0x40000000
#define I2C_MASTER_CONTROL_FORCE_SDA_LOW_OFFSET 25
#define I2C_MASTER_CONTROL_FORCE_SDA_LOW_MASK 0x2000000
#define I2C_MASTER_CONTROL_FORCE_SDA_LOW 0x2000000
#define I2C_MASTER_CONTROL_SLAVE_ADDR_OFFSET 14
#define I2C_MASTER_CONTROL_SLAVE_ADDR_MASK 0x1FC000
#define I2C_MASTER_CONTROL_READ_BYTE_COUNT_OFFSET 6
#define I2C_MASTER_CONTROL_READ_BYTE_COUNT_MASK 0x3FC0
#define I2C_MASTER_CONTROL_CLEAR_OFFSET 5
#define I2C_MASTER_CONTROL_CLEAR_MASK 0x20
#define I2C_MASTER_CONTROL_CLEAR 0x20
#define I2C_MASTER_CONTROL_OPMODE_OFFSET 2
#define I2C_MASTER_CONTROL_OPMODE_MASK 0xC
#define I2C_MASTER_CONTROL_CLOCK_STRETCH_EN_OFFSET 1
#define I2C_MASTER_CONTROL_CLOCK_STRETCH_EN_MASK 0x2
#define I2C_MASTER_CONTROL_CLOCK_STRETCH_EN 0x2
#define I2C_MASTER_CONTROL_GO_OFFSET 0
#define I2C_MASTER_CONTROL_GO_MASK 0x1
#define I2C_MASTER_CONTROL_GO 0x1
#define I2C_MASTER_WR_DATA_WR_DATA_OFFSET 0
#define I2C_MASTER_WR_DATA_WR_DATA_MASK 0xFF
#define I2C_MASTER_RD_DATA_RD_DATA_OFFSET 0
#define I2C_MASTER_RD_DATA_RD_DATA_MASK 0xFF
#define I2C_MASTER_IRQ_ENABLE_FIFO_AE_OFFSET 2
#define I2C_MASTER_IRQ_ENABLE_FIFO_AE_MASK 0x4
#define I2C_MASTER_IRQ_ENABLE_FIFO_AE 0x4
#define I2C_MASTER_IRQ_ENABLE_FIFO_AF_OFFSET 1
#define I2C_MASTER_IRQ_ENABLE_FIFO_AF_MASK 0x2
#define I2C_MASTER_IRQ_ENABLE_FIFO_AF 0x2
#define I2C_MASTER_IRQ_ENABLE_DONE_OFFSET 0
#define I2C_MASTER_IRQ_ENABLE_DONE_MASK 0x1
#define I2C_MASTER_IRQ_ENABLE_DONE 0x1
#define I2C_MASTER_IRQ_PENDING_FIFO_AE_OFFSET 2
#define I2C_MASTER_IRQ_PENDING_FIFO_AE_MASK 0x4
#define I2C_MASTER_IRQ_PENDING_FIFO_AE 0x4
#define I2C_MASTER_IRQ_PENDING_FIFO_AF_OFFSET 1
#define I2C_MASTER_IRQ_PENDING_FIFO_AF_MASK 0x2
#define I2C_MASTER_IRQ_PENDING_FIFO_AF 0x2
#define I2C_MASTER_IRQ_PENDING_DONE_OFFSET 0
#define I2C_MASTER_IRQ_PENDING_DONE_MASK 0x1
#define I2C_MASTER_IRQ_PENDING_DONE 0x1
#define I2C_MASTER_IRQ_PENDING_IRQ_FIFO_AE_OFFSET 2
#define I2C_MASTER_IRQ_PENDING_IRQ_FIFO_AE_MASK 0x4
#define I2C_MASTER_IRQ_PENDING_IRQ_FIFO_AE 0x4
#define I2C_MASTER_IRQ_PENDING_IRQ_FIFO_AF_OFFSET 1
#define I2C_MASTER_IRQ_PENDING_IRQ_FIFO_AF_MASK 0x2
#define I2C_MASTER_IRQ_PENDING_IRQ_FIFO_AF 0x2
#define I2C_MASTER_IRQ_PENDING_IRQ_DONE_OFFSET 0
#define I2C_MASTER_IRQ_PENDING_IRQ_DONE_MASK 0x1
#define I2C_MASTER_IRQ_PENDING_IRQ_DONE 0x1
#define I2C_MASTER_IRQ_RAW_FIFO_AE_OFFSET 2
#define I2C_MASTER_IRQ_RAW_FIFO_AE_MASK 0x4
#define I2C_MASTER_IRQ_RAW_FIFO_AE 0x4
#define I2C_MASTER_IRQ_RAW_FIFO_AF_OFFSET 1
#define I2C_MASTER_IRQ_RAW_FIFO_AF_MASK 0x2
#define I2C_MASTER_IRQ_RAW_FIFO_AF 0x2
#define I2C_MASTER_IRQ_RAW_DONE_OFFSET 0
#define I2C_MASTER_IRQ_RAW_DONE_MASK 0x1
#define I2C_MASTER_IRQ_RAW_DONE 0x1
#endif /* _I2C_MASTER_REGS_H_ */

/* end */
