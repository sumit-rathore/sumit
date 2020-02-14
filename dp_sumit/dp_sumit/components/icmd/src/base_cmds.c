///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2010
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
//!   @file  -  base_cmds.c
//
//!   @brief -  Defines the base icmds that would be used by all projects
//
//
//!   @note  -
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include "icmd_log.h"
#include "icmd_cmd.h"
#include <sys_defs.h>
#include "uart.h"

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/

/************************** Function Definitions *****************************/


/**
* FUNCTION NAME: readMemory()
*
* @brief  - reads any 32 bit memory location
*
* @return - void
*
* @note   - its an icmd function
*
*/
void readMemory(uint32_t address)
{
    uint32_t returnValue[2];

    returnValue[0] = address;
    returnValue[1] = *((volatile uint32_t *)address);

    // send the values back to the requester
    UART_packetizeSendResponseImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_ICMD,
        ICMD_GetResponseID(),
        &returnValue[1],
        sizeof(returnValue[1]));

    ilog_ICMD_COMPONENT_2(ILOG_USER_LOG, BASE_READ_MEM, returnValue[0], returnValue[1]);
}


/**
* FUNCTION NAME: writeMemory()
*
* @brief  - writes any 32 bit memory location
*
* @return - void
*
* @note   - its an icmd function
*
*/
void writeMemory
(
    uint32_t address,
    uint32_t value
)
{
    *((volatile uint32_t *)address) = value;

    ilog_ICMD_COMPONENT_2(ILOG_USER_LOG, BASE_WRITE_MEM, address, value);
}


/**
* FUNCTION NAME: readModifyWriteMemory()
*
* @brief  - writes any 32 bit memory location
*
* @return - void
*
* @note   - its an icmd function
*
*/
void readModifyWriteMemory
(
    uint32_t address,
    uint32_t setBitMask,
    uint32_t clearBitMask
)
{
    if (clearBitMask & setBitMask)
    {
        ilog_ICMD_COMPONENT_3(ILOG_USER_LOG, BASE_READ_MODIFY_WRITE_CONFLICT_MASKS, address, setBitMask, clearBitMask);
    }
    else
    {
        uint32_t readValue = *((volatile uint32_t *)address);

        uint32_t writeValue = (readValue | setBitMask) & ~clearBitMask;

        *((volatile uint32_t *)address) = writeValue;

        ilog_ICMD_COMPONENT_3(ILOG_USER_LOG, BASE_READ_MODIFY_WRITE, address, readValue, writeValue);
    }
}

/**
* FUNCTION NAME: modifyBitfield()
*
* @brief  - writes data from given position within any memory address
*
* @return - void
*
* @note   - its an icmd function
*/
void modifyBitfield
(
    uint32_t address,
    uint8_t position,
    uint8_t width,
    uint32_t value
)
{
    uint32_t mask;
    uint32_t oldValue = *((volatile uint32_t *)address);
    uint32_t newValue;

    mask = CREATE_MASK(width, position, uint32_t);
    oldValue = oldValue & ~mask;
    newValue = oldValue | (mask & (value << position));
    *((volatile  uint32_t *)address) = newValue;

    ilog_ICMD_COMPONENT_3(ILOG_USER_LOG, BASE_MODIFY_BITFIELD, address, position, width);
    ilog_ICMD_COMPONENT_3(ILOG_USER_LOG, MODIFY_BITFIELD_RESULT, value, oldValue, newValue);
}


/**
* FUNCTION NAME: dumpMemory32()
*
* @brief  - dumps data from requested 32 bit memory locations
*
* @return - void
*
* @note   - its an icmd function
*/
void dumpMemory32
(
    uint32_t address,
    uint8_t length
)
{

    if((address & 3) != 0)
    {
        ilog_ICMD_COMPONENT_1(ILOG_USER_LOG,DUMP_MEMORY_ADDR_INVALID, address);
    }
    else
    {
        int i;
        for(i = 0; i < length;i++)
        {
            const uint32_t value = *((volatile uint32_t *)address);
            ilog_ICMD_COMPONENT_2(ILOG_USER_LOG, BASE_READ_MEM, address, value);
            address += 4;
        }
    }
}


/**
* FUNCTION NAME: callFunction()
*
* @brief  - calls function from 32 bit memory location
*
* @return - void
*
* @note   - its an icmd function
*/
void callFunction
(
    uint32_t address,
    uint32_t arg1,
    uint32_t arg2,
    uint32_t arg3,
    uint32_t arg4,
    uint32_t arg5
)
{
    void (*funcptr)() = (void (*)())address;
    (*funcptr)(arg1, arg2, arg3, arg4, arg5);
}


/**
* FUNCTION NAME: ICMD_deprecatedIcmdFunction()
*
* @brief  - Dummy function that can replace a deleted icmd
*
* @return - void
*
* @note   - its an icmd function
*/
void ICMD_deprecatedIcmdFunction(void)
{
    ilog_ICMD_COMPONENT_0(ILOG_USER_LOG, DEPRECATED_ICMD);
}


/**
* FUNCTION NAME: readMemory16()
*
* @brief  - reads any 16 bit memory location
*
* @return - void
*
* @note   - its an icmd function
*
*/
void readMemory16
(
#ifdef __MSP430__
    uint16_t address
#else
    uint32_t address
#endif
)
{
    uint16_t value = *((volatile uint16_t *)address);

    ilog_ICMD_COMPONENT_2(ILOG_USER_LOG, BASE_READ_MEM16, address, value);
}


/**
* FUNCTION NAME: writeMemory16()
*
* @brief  - writes any 16 bit memory location
*
* @return - void
*
* @note   - its an icmd function
*
*/
void writeMemory16
(
#ifdef __MSP430__
    uint16_t address,
#else
    uint32_t address,
#endif
    uint16_t value
)
{
    *((volatile uint16_t *)address) = value;

    ilog_ICMD_COMPONENT_2(ILOG_USER_LOG, BASE_WRITE_MEM16, address, value);
}


/**
* FUNCTION NAME: readModifyWriteMemory16()
*
* @brief  - writes any 16 bit memory location
*
* @return - void
*
* @note   - its an icmd function
*
*/
void readModifyWriteMemory16
(
#ifdef __MSP430__
    uint16_t address,
#else
    uint32_t address,
#endif
    uint16_t setBitMask,
    uint16_t clearBitMask
)
{
    if (clearBitMask & setBitMask)
    {
        ilog_ICMD_COMPONENT_3(ILOG_USER_LOG, BASE_READ_MODIFY_WRITE_CONFLICT_MASKS16, address, setBitMask, clearBitMask);
    }
    else
    {
        uint16_t readValue = *((volatile uint16_t *)address);

        uint16_t writeValue = (readValue | setBitMask) & ~clearBitMask;

        *((volatile uint16_t *)address) = writeValue;

        ilog_ICMD_COMPONENT_3(ILOG_USER_LOG, BASE_READ_MODIFY_WRITE16, address, readValue, writeValue);
    }
}


void readMemory8
(
    uint32_t address
)
{
    uint8_t value = *((volatile uint8_t *)address);

    ilog_ICMD_COMPONENT_2(ILOG_USER_LOG, BASE_READ_MEM, address, value);
}

void writeMemory8
(
    uint32_t address,
    uint8_t value
)
{
    *((volatile uint8_t *)address) = value;

    ilog_ICMD_COMPONENT_2(ILOG_USER_LOG, BASE_WRITE_MEM, address, value);
}

