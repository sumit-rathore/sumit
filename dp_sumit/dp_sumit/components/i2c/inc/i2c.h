//#################################################################################################
// Icron TechnologyUSB Corporation - Copyright 2019
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef I2C_H
#define I2C_H

// Includes #######################################################################################
#include <ibase.h>

// Constants and Macros ###########################################################################
#define I2C_SPEED_100KHZ               100000
#define I2C_SPEED_400KHZ               400000
#define I2C_SPEED_1MHZ                 1000000


// Data Types #####################################################################################
enum I2cSpeed
{
    I2C_SPEED_SLOW,                         // 100 KHz
    I2C_SPEED_FAST,                         // 400 KHz
    I2C_SPEED_FAST_PLUS,                    // 1000 KHz == 1 MHz
    NUM_OF_I2C_SPEEDS
};

enum I2cMuxPort
{
    KC705_MUX_USER_CLOCK,                   // I2C TI Mux Channel 0
    KC705_MUX_FMC_HPC_IIC,                  // I2C TI Mux Channel 1
    KC705_MUX_FMC_LPC_IIC,                  // I2C TI Mux Channel 2
    KC705_MUX_EEPROM_IIC,                   // I2C TI Mux Channel 3
    KC705_MUX_SFP_IIC,                      // I2C TI Mux Channel 4
    KC705_MUX_IIC_HDMI,                     // I2C TI Mux Channel 5
    KC705_MUX_IIC_DDR3,                     // I2C TI Mux Channel 6
    KC705_MUX_SI5326,                       // I2C TI Mux Channel 7
    I2C_MUX_NONE,
    I2C_MUX_CORE = I2C_MUX_NONE,            // I2C on core board   (0)
    I2C_MUX_MOTHERBOARD,                    // I2C on mother board (1)
    // Before modify this, need check I2C_SetPortForDevice function
};

struct I2cDevice                            // A device's I2C characteristic
{
    uint8_t deviceAddress;                  // 7bit i2c addr
    enum I2cSpeed speed;                    // i2c speed
    enum I2cMuxPort port;                   // i2c port
};


// Function Declarations ##########################################################################
void I2C_init(void (*callback)(void));

void I2c_InterruptHandler(void);

void I2C_WriteAsync( const struct I2cDevice *device,
                    uint8_t *prtToData,
                    uint8_t byteCountWrite,
                    void (*notifyWriteCompleteHandler)(bool success));

void I2C_ReadAsync( const struct I2cDevice *device,
                    uint8_t *prtToData,
                    uint8_t byteCountRead,
                    void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount));

void I2C_WriteReadAsync( const struct I2cDevice *device,
                        uint8_t *prtToData,                 // Write/Read uses the same buffer
                        uint8_t byteCountWrite,
                        uint8_t byteCountRead,
                        void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount));

void I2C_WriteReadBlockAsync( const struct I2cDevice *device,
                            uint8_t *prtToData,             // Write/Read uses the same buffer
                            uint8_t byteCountWrite,
                            uint8_t byteCountRead,          // Maximum buffer size of read byte
                            void (*notifyReadCompleteHandler)(uint8_t * data, uint8_t byteCount));

void I2C_Wake( const struct I2cDevice *device, void (*notifyWakeCompleteHandler)(void));


bool I2C_WriteBlocking( const struct I2cDevice *device,
                        uint8_t *prtToData,
                        uint8_t byteCountWrite);

bool I2C_ReadBlocking( const struct I2cDevice *device,
                        uint8_t *prtToData,
                        uint8_t byteCountRead);

bool I2C_WriteReadBlocking( const struct I2cDevice *device,
                            uint8_t *prtToData,
                            uint8_t byteCountWrite,
                            uint8_t byteCountRead);


#endif // I2C_H

