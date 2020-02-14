///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2012, 2013
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
//!   @file  -  random.c
//
//!   @brief -
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <configuration.h>
#include <itypes.h>
#include <random.h>
#include <timing_timers.h>
#include "random_cmd.h"
#include "random_log.h"
#include <sha2.h>
#include <xadc.h>
#include <atmel_crypto.h>

/************************ Defined Constants and Macros ***********************/
extern const struct ATMEL_KeyStore ATMEL_secretKeyStore;

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/**
* FUNCTION NAME: getShaRandom()
*
* @brief  - get real random using temperature ADC seed with secure code
*
* @return - void
*
* @note   - length should be under 32
*
*/
void getShaRandom(uint8_t *input, uint8_t length)
{
    static uint8_t shaRandom[32];                   // random result
    static uint8_t shaDigest[64];                   // sha256 buffer having previous value
    static uint8_t referencePointer = 0;
    const uint16_t temp = XADC_readTemperature();

    int8_t slot = temp & 0x07;                      // start slot
    int8_t amount = temp & 0x1F;                    // amount of reading key

    if(amount == 0)
    {
        amount = 32;
    }

    uint8_t *keyPtr = (uint8_t*)&ATMEL_secretKeyStore.key[8];     // initial location

    if(referencePointer == 0)                       // Initial case, setup start location
    {
        referencePointer += slot << 5;              // new location = slot x 32
    }
    keyPtr += referencePointer;

    memcpy(&shaDigest[0], keyPtr, amount);          // (32 - amount) will be garbage random factor
    memcpy(&shaDigest[32], shaRandom, 32);
    sha256(shaDigest, 64, shaRandom);

    referencePointer += amount;                     // Max 256
    memcpy(input, shaRandom, length);
}

// void printShaRandom()
// {
//     uint8_t input[20];
//     getShaRandom(input, 20);

//     UART_printf("SHA Random: %x%x%x%x%x \n",
//     (input[19] << 24) + (input[18] << 16) + (input[17] << 8) + input[16],
//     (input[15] << 24) + (input[14] << 16) + (input[13] << 8) + input[12],
//     (input[11] << 24) + (input[10] << 16) + (input[9] << 8) + input[8],
//     (input[7] << 24) + (input[6] << 16) + (input[5] << 8) + input[4],
//     (input[3] << 24) + (input[2] << 16) + (input[1] << 8) + input[0]
//     );
// }
