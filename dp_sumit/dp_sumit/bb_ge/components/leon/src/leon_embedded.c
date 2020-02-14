///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2013
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
//!   @file  - leon_embedded.c
//
//!   @brief -
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <_leon_reg_access.h>
#include <leon_embedded.h>

/************************ Defined Constants and Macros ***********************/
#define LEON_EXTERNAL_CPU_DEBUG_ADDRESS_OFFSET (0x130)
#define LEON_EXTERNAL_CPU_DEBUG_DATA_OFFSET    (0x134)
#define LEON_EXTERNAL_CPU_DEBUG_CTRL_OFFSET    (0x138)

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/
/**
* FUNCTION NAME: LEON_EmbeddedCPUWriteAHBRAM
*
* @brief  - Copy data from SCPU to GE_CPU's AHB RAM
*
* @return - 
*
* @note   - 
*
*/
void LEON_EmbeddedCPUWriteAHBRam
(
    uint32 geAHBRamAddr,
    uint32 * data,
    uint32 size // Number of bytes
)
{
    LEON_EmbeddedExternalCPUWrite(geAHBRamAddr,
                                  data,
                                  size);
}

/**
* FUNCTION NAME: LEON_EmbeddedStartGECPU
*
* @brief  - 
*
* @return - 
*
* @note   - 
*
*/
void LEON_EmbeddedStartGECPU
(
    uint32 geControlAddr
)
{
    uint32 data = 0x0;
    uint32 byteSize = 0x4;
    LEON_EmbeddedExternalCPUWrite(geControlAddr,
                                  &data,
                                  byteSize);
}

/**
* FUNCTION NAME: LEON_EmbeddedExternalCPUWrite
*
* @brief  - 
*
* @return - 
*
* @note   - 
*
*/
void LEON_EmbeddedExternalCPUWrite
(
    uint32 addr,
    uint32 * data,
    uint32 size
)
{
    uint32 bytesWritten = 0;

    while (bytesWritten < size)
    {
        _LEON_WriteLeonRegister(LEON_EXTERNAL_CPU_DEBUG_ADDRESS_OFFSET, addr + bytesWritten);
        _LEON_WriteLeonRegister(LEON_EXTERNAL_CPU_DEBUG_DATA_OFFSET, *data);
        _LEON_WriteLeonRegister(LEON_EXTERNAL_CPU_DEBUG_CTRL_OFFSET, 0x3);
        bytesWritten += 4;
        data++;

        // Keep looping until write is complete
        while ((_LEON_ReadLeonRegister(LEON_EXTERNAL_CPU_DEBUG_CTRL_OFFSET) & 0x1) == 1)
            ;
    }
}

/**
* FUNCTION NAME: LEON_EmbeddedExternalCPURead
*
* @brief  - 
*
* @return - uint32
*
* @note   - 
*
*/
uint32 LEON_EmbeddedExternalCPURead(uint32 addr)
{
    _LEON_WriteLeonRegister(LEON_EXTERNAL_CPU_DEBUG_ADDRESS_OFFSET, addr);
    _LEON_WriteLeonRegister(LEON_EXTERNAL_CPU_DEBUG_CTRL_OFFSET, 0x1);

    // Keep looping until write is complete
    while ((_LEON_ReadLeonRegister(LEON_EXTERNAL_CPU_DEBUG_CTRL_OFFSET) & 0x1) == 1)
        ;

    return (_LEON_ReadLeonRegister(LEON_EXTERNAL_CPU_DEBUG_DATA_OFFSET));
}

/************************** Local Functions *****************************/
