///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2014
///
///
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or
///   disclosure, in whole or in part, to anyone outside of Icron without the
///   written approval of a Icron officer under a Non-Disclosure Agreement, or
///   to any employee of Icron who has not previously obtained written
///   authorization for access from the individual responsible for the source
///   code, will have a significant detrimental effect on Icron and is
///   expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//!   @file  - kc705_i2cMux.c
//
//!   @brief -
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "kc705_loc.h"
#include <ibase.h>
#ifdef BLACKBIRD
#include <bgrg_i2c.h>
#else
#include <grg_i2c.h>
#endif

/************************ Defined Constants and Macros ***********************/
#define KC705_MUX_I2C_ADDRESS               (0x74)

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
static struct {
    uint8 bus;
    uint8 device;
    uint8 speed;
    uint8 reg;
    uint8 value; 
} _Random1ByteData;

/************************ Local Function Prototypes **************************/
static void _i2cRandom1ByteRead2(boolT success);
static void i2cRandom1ByteReadComplete(uint8 *data, uint8 byteCount);


/************************** Function Definitions *****************************/
/**
* FUNCTION NAME: KC705_Select
*
* @brief  - Select an I2C operation using the I2C Mux
*
* @return - 
*
* @note   - 
*
*/
void KC705_Select(enum KC705_MUX muxOperation, void (*notifyCompleteHandler)(boolT success))
{
    uint8 bus = 0;
    uint8 device = KC705_MUX_I2C_ADDRESS;
    uint8 byteCount = 1;
    uint8 data = CAST(muxOperation, enum KC705_MUX, uint8);
#ifdef BLACKBIRD
    enum BGRG_I2cSpeed speed = BGRG_I2C_SPEED_SLOW;
    BGRG_I2cWriteASync
#else
    enum GRG_I2cSpeed speed = GRG_I2C_SPEED_SLOW;
    GRG_I2cWriteASync
#endif
                      (bus,
                       device,
                       speed,
                       &data,
                       byteCount,
                       notifyCompleteHandler);
}

/**
* FUNCTION NAME: i2cRandom1ByteRead
*
* @brief  - Read 1 byte using I2c via the mux
*
* @return - 
*
* @note   - 
*
*/

void i2cRandom1ByteRead
(
    uint8 bus,
    uint8 device,
    uint8 speed,
    uint8 startAddress,
    uint8 mux
)
{
    _Random1ByteData.bus = bus;
    _Random1ByteData.device = device;
    _Random1ByteData.speed = speed;
    _Random1ByteData.reg = startAddress;

    // Value will be re-used
    _Random1ByteData.value = startAddress;

    KC705_Select(mux, &_i2cRandom1ByteRead2);
}

/************************ Local Function Definitions *****************************/

static void _i2cRandom1ByteRead2(boolT success)
{
    uint8 writeByteCount = 1;
    uint8 readByteCount = 1;

    iassert_KC705_COMPONENT_0((success == TRUE), I2C_WRITE_FAILED);

#ifdef BLACKBIRD
    BGRG_I2cWriteReadASync(
#else
    GRG_I2cWriteReadASync(
#endif
        _Random1ByteData.bus,
        _Random1ByteData.device,
        _Random1ByteData.speed,
        &_Random1ByteData.value, // re-uses write buffer as read buffer
        writeByteCount,
        readByteCount,
        &i2cRandom1ByteReadComplete);
}

static void i2cRandom1ByteReadComplete(uint8 * data, uint8 byteCount)
{
    iassert_KC705_COMPONENT_0((byteCount == 1), I2C_READ_FAILED);
    // TODO: need to make sure that another i2c random read does not start before the current
    // one prints the start address value

    // TODO: need a way to not print this because this might spam
    // add remove logging functionality from some ilogs responses
    ilog_KC705_COMPONENT_2(ILOG_USER_LOG, I2C_RANDOM_READ_1BYTE_RESP, _Random1ByteData.reg, _Random1ByteData.value);
}

