//#################################################################################################
// Icron Technology Corporation - Copyright 2016
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################

//#################################################################################################
// Module Description
//#################################################################################################
// UART Fifo module.  Fifo routines used by the UART driver.  Instantiated in code because there
// can be more then 1 UART in the system
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################
//#ifdef BB_GE_COMPANION

// Includes #######################################################################################
#include <itypes.h>
#include <ibase.h>
#include <options.h>
#include "uart_loc.h"


// Constants and Macros ###########################################################################


// Data Types #####################################################################################


// Global Variables ###############################################################################


// Static Variables ###############################################################################


// Static Function Declarations ###################################################################


// Exported Function Definitions ##################################################################

// Component Scope Function Definitions ###########################################################


/*                                                                              */
/* FUNCTION NAME: UartfifoInit()                                                */
/*                                                                              */
/* @brief  - Initializes the given FIFO                                         */
/*                                                                              */
/* @param - fifo - pointer to the fifo structure to work with                   */
/* @param - buffer - pointer to the buffer to use for this fifo                 */
/* @param - bufferSize - the size of the given buffer                           */
/*                                                                              */
/* @return - nothing                                                            */
/*                                                                              */
void  UartfifoInit(UartFifoT *fifo, uint8_t *buffer, uint16_t bufferSize)
{
    // setup the buffer
    fifo->fifo = buffer;
    fifo->fifoSize = bufferSize;

    // initialize the fifo to empty, at the start
    fifo->fifoReadIndex = fifo->fifoWriteIndex = 0;
}

/*                                                                              */
/* FUNCTION NAME: fifoEmpty()                                                   */
/*                                                                              */
/* @brief  - Checks if the fifo is empty                                        */
/*                                                                              */
/* @param - fifo - pointer to the fifo structure to work with                   */
/*                                                                              */
/* @return - bool : true/false.  true if FIFO is empty                          */
/*                                                                              */
bool UartfifoEmpty(UartFifoT *fifo)
{
    return (fifo->fifoWriteIndex == fifo->fifoReadIndex);
}
/*                                                                              */
/* FUNCTION NAME: fifoFull()                                                    */
/*                                                                              */
/* @brief  - Checks if the fifo is full                                         */
/*                                                                              */
/* @param - fifo - pointer to the fifo structure to work with                   */
/*                                                                              */
/* @return - bool : true/false.  true if FIFO is full                           */
/*                                                                              */
bool  UartfifoFull(UartFifoT *fifo)
{
    return  (   ((fifo->fifoReadIndex == 0)
                && (fifo->fifoWriteIndex == (fifo->fifoSize - 1)))
            ||  (fifo->fifoReadIndex == (fifo->fifoWriteIndex + 1)));
}
/*                                                                              */
/* FUNCTION NAME: fifoSpaceAvail()                                              */
/*                                                                              */
/* @brief  - Checks how much space is available on the fifo                     */
/*                                                                              */
/* @param - fifo - pointer to the fifo structure to work with                   */
/*                                                                              */
/* @return - The number of elements available                                   */
/*                                                                              */
uint16_t UartfifoSpaceAvail(UartFifoT *fifo)
{
    int16_t spread = fifo->fifoReadIndex - fifo->fifoWriteIndex;

    if (spread > 0)
    {
        /* spread indicates how many elements are left                          */
        /* except the fact that we can't fill it                                */
        return (uint16_t)(spread - 1);
    }
    else
    {
        /* spread indicates the negative of how many elements are used          */
        return (uint16_t)(fifo->fifoSize + spread -1);
    }
}
/*                                                                              */
/* FUNCTION NAME: fifoWrite()                                                   */
/*                                                                              */
/* @brief  - Writes to the fifo                                                 */
/*                                                                              */
/* @param - fifo - pointer to the fifo structure to work with                   */
/* @param - newItem - the item to store in the fifo                             */
/*                                                                              */
/* @return - void                                                               */
/*                                                                              */
/* @note   - UNSAFE: Always call fifoFull or fifoSpaceAvail before calling      */
/*           fifoWrite, as fifoWrite doesn't check if there is space available  */
/*         - assumes fifo size is a power of 2                                  */
/*                                                                              */
/*                                                                              */
void UartfifoWrite(UartFifoT *fifo, uint8_t newItem)
{
    fifo->fifo[fifo->fifoWriteIndex] = newItem;

    /* Update write index pointer, and handle the wrap if it reaches the end */
    fifo->fifoWriteIndex++;
    /* Subtracting 1 will reveal a mask of all the lower bits */
    /* Instead of checking for the max value, we can just mask out the upper bits */
    const uint32_t fifo_mask = fifo->fifoSize - 1;
    fifo->fifoWriteIndex &= fifo_mask;
}
/*                                                                              */
/* FUNCTION NAME: fifoRead()                                                    */
/*                                                                              */
/* @brief  - Reads an element off the fifo                                      */
/*                                                                              */
/* @param - fifo - pointer to the fifo structure to work with                   */
/*                                                                              */
/* @return - The element read                                                   */
/*                                                                              */
/* @note   - UNSAFE: Always call fifoEmpty before calling fifoRead              */
/*           As fifoRead doesn't check if there is anything on the fifo         */
/*         - assumes fifo size is a power of 2                                  */
/*                                                                              */
/*                                                                              */
uint8_t UartfifoRead(UartFifoT *fifo)
{
    uint8_t readElement = fifo->fifo[fifo->fifoReadIndex];

    /* Update read index pointer, and handle the wrap if it reaches the end */
    fifo->fifoReadIndex++;
    /* Subtracting 1 will reveal a mask of all the lower bits */
    /* Instead of checking for the max value, we can just mask out the upper bits */
    const uint32_t fifo_mask = fifo->fifoSize - 1;
    fifo->fifoReadIndex &= fifo_mask;

    return readElement;
}
/*                                                                              */
/* FUNCTION NAME: fifoPeek()                                                    */
/*                                                                              */
/* @brief  - Reads an element off the fifo without modifying pointers           */
/*                                                                              */
/* @param - fifo - pointer to the fifo structure to work with                   */
/*        - offset - offset into fifo to read from                              */
/*                                                                              */
/* @return - The element read                                                   */
/*                                                                              */
/* @note   -                                                                    */
/*                                                                              */
bool UartfifoPeek(
        UartFifoT *fifo,
        uint8_t* data,
        uint16_t offset)
{
    bool valid = true;

    // requested offset within space used?
    // NOTE: SpaceAvail will handle wrapping
    // SpaceAvail is size - 1 (circ buffer design) if empty
    // So size - spaceAvail is always >= 1
    if ((fifo->fifoSize - UartfifoSpaceAvail(fifo)) <= (offset + 1))
    {
        valid = false;
        *data = 0;
//        UART_printf("PeekF");
    }
    else
    {
        uint16_t _offset = fifo->fifoReadIndex + offset;

        _offset &= (fifo->fifoSize - 1);
        *data = fifo->fifo[_offset];
    }

    return valid;
}
/*                                                                              */
/* FUNCTION NAME: CopyFifoToBuffer()                                                    */
/*                                                                              */
/* @brief  - Reads an element off the fifo without modifying pointers           */
/*                                                                              */
/* @param - fifo - pointer to the fifo structure to work with                   */
/*        - offset - offset into fifo to read from                              */
/*                                                                              */
/* @return - The element read                                                   */
/*                                                                              */
/* @note   -                                                                    */
/*                                                                              */
bool UartfifoCopyFifoToBuffer(
        uint8_t* buffer,
        UartFifoT *fifo,
        uint16_t length)
{
    bool valid = true;
    if ((fifo->fifoSize - UartfifoSpaceAvail(fifo)) >= length)
    {
        // there is at least the data bytes we need from the fifo - copy them out

        // one memcpy
        // [____WRIDX-----------RDIDX___]
        // memcpy(dst, &fifo[fifoReadIndex], length);
        if (fifo->fifoReadIndex < fifo->fifoWriteIndex)
        {
            memcpy(buffer, &(fifo->fifo[fifo->fifoReadIndex]), length);
        }
        else
        {
            // two memcpys
            // [----RDIDX___________WRIDX---]
            // First memcpy
            // [----RDIDX
            // memcpy(dst, &fifo[fifoReadIndex], (fifo->fifoSize - fifo->fifoReadIndex));
            // bytesCopied = (fifo->fifoSize - fifo->fifoReadIndex);
            // Second memcpy
            // WRIDX----]
            // memcpy( (dst + bytesCopied), &fifo[0], (length - bytesCopied));
            const uint16_t bytesCopied = (fifo->fifoSize - fifo->fifoReadIndex);
            if (length <= bytesCopied) // no wrapping
            {
                memcpy(buffer, &(fifo->fifo[fifo->fifoReadIndex]), length);
            }
            else
            {
                memcpy(buffer, &(fifo->fifo[fifo->fifoReadIndex]), bytesCopied);
                memcpy(buffer + bytesCopied, fifo->fifo, (length - bytesCopied));
            }
        }

        // update the read pointer to the new position, after the copied bytes
        const uint32_t fifo_mask = fifo->fifoSize - 1;
        fifo->fifoReadIndex += length;
        fifo->fifoReadIndex &= fifo_mask;
    }
    else
    {
        // assert
        valid = false;
    }


   return valid;
}


// Static Function Definitions ####################################################################



//#endif // GE_BB_COMPANION
