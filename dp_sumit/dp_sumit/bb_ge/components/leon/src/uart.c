///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2009, 2010
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
//!   @file  -  uart.c
//
//!   @brief -  driver for the leon uart
//
//!   @note  - The following defines may be set for this file in options.h
//
//              LEON_NO_TRAP_INIT -- This won't build any support for interrupts
//
//              LEON_DEFAULT_UART_RX_HANDLER -- The value will be the default uart RX handler
//
//              LEON_UART_TX_BUFFER_SIZE -- This will include a SW fifo to buffer up transmit
//                                          bytes with buffer size LEON_UART_TX_BUFFER_SIZE
//
///////////////////////////////////////////////////////////////////////////////
#ifndef BB_GE_COMPANION

/***************************** Included Headers ******************************/
#include <leon_uart.h>
#include <leon_traps.h>
#include <ififo.h>
#include "leon_regs.h"

#ifdef USE_OPTIONS_H
#include <options.h>
#endif

/************************ Defined Constants and Macros ***********************/
#define WORD_LEN 4

#define CONTROL_W_TX_INT_EN \
        (LEON_UART_CONTROL_RE_BIT_MASK | LEON_UART_CONTROL_TE_BIT_MASK | LEON_UART_CONTROL_RI_BIT_MASK | LEON_UART_CONTROL_TI_BIT_MASK)

#define CONTROL_W_TX_INT_DIS \
        (LEON_UART_CONTROL_RE_BIT_MASK | LEON_UART_CONTROL_TE_BIT_MASK | LEON_UART_CONTROL_RI_BIT_MASK)

#define UART_SCALER_SCALER_MASK (0xFFF)

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/
//In the future the uart rx handler could return a fcn pointer to a function to
// be called.  This would save a register window
#ifndef LEON_NO_TRAP_INIT
#ifndef LEON_DEFAULT_UART_RX_HANDLER
#define LEON_DEFAULT_UART_RX_HANDLER NULL
#endif
static pfnLEON_UartRxHandler uartRxHandler = LEON_DEFAULT_UART_RX_HANDLER;
#endif

/************************ Local Function Prototypes **************************/
#ifdef LEON_UART_TX_BUFFER_SIZE
// Call the ififo macro to create the local function prototypes
IFIFO_CREATE_FIFO_LOCK_UNSAFE(uart, uint8, LEON_UART_TX_BUFFER_SIZE)
// Should create
// static inline boolT uart_fifoEmpty(void);
// static inline boolT uart_fifoFull(void);
// static inline uint32 uart_fifoSpaceAvail(void);
// static inline void uart_fifoWrite(uint8 newElement);
// static inline uint8 uart_fifoRead(void);

#endif

static boolT atomicHWFifoTx(const uint8 * msg, uint8 len);


/************************ Local Function Definitions **************************/

static uint32_t calcUARTScaler(uint32_t baud);

/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: LEON_UartSetBaudRate115200()
*
* @brief  - Initializes the uart by setting the baud rate to 115200
*
* @return - void
*
* @note   - This doesn't enable the uart interrupt
*
*/
void LEON_UartSetBaudRate115200(void)
{
    LEON_UartSetBaudRate(115200);
}
void LEON_UartSetBaudRate(uint32_t baud)
{
    //Setup for LEON_UART_115200
    //TODO: could provide options.h for system frequency and let compiler use formula at build time
    // From Leon 11.3.4 // scaler = (((system_clk*10)/(baudrate*8))-5)/10
    // Our system clock is 60,000,000 & baudrate us 115200
    // scaler = 64.6

    WriteLeonRegister(LEON_UART_SCALER_OFFSET, calcUARTScaler(baud));

    //Setup uart control register
    //enable TX & RX + irqs for only RX
    WriteLeonRegister(LEON_UART_CONTROL_OFFSET, CONTROL_W_TX_INT_DIS);

    //Note: we don't enable uart irq, as we don't know the interrupt number of the uart
    //      Also we don't know if the user is polling driven or interrupt driven
}


/**
* FUNCTION NAME: LEON_UartAtomicTx()
*
* @brief  - Atomically write all the bytes given (will be a multiple of 4), or none at all
*
* @return - boolT, TRUE if entire message is written, FALSE if nothing is written
*
* @note   - This is intended for iLog
*
*           Locks must be on, to ensure this function is not interrupted
*/
boolT LEON_UartAtomicTx
(
    const uint8 * msg,      // Message to send
    uint8 len               // Length of message in bytes
)
{
#ifdef LEON_UART_TX_BUFFER_SIZE
    boolT ret;

    // Can we get anything out of the SW fifo and into the HW fifo?
    LEON_UartPollingModeDoWork();

    // Check if we can do a HW fifo write
    if (uart_fifoEmpty() && atomicHWFifoTx(msg, len)) // note this uses order of operations to optionally call atomicHWFifoTx()
    {
        // We did a HW fifo write
        ret = TRUE;
    }
    else if (len <= uart_fifoSpaceAvail())
    {
        // Do a SW fifo operation
        while (len)
        {
            uart_fifoWrite(*msg);
            msg++;
            len--;
        }

        // It is possible that the SW fifo was empty previously, so enable the TX interrupt
        WriteLeonRegister(LEON_UART_CONTROL_OFFSET, CONTROL_W_TX_INT_EN);

        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }

    return ret;
#else
    return atomicHWFifoTx(msg, len);
#endif
}

static boolT atomicHWFifoTx
(
    const uint8 * msg,      // Message to send
    uint8 len               // Length of message in bytes
)
{
    boolT ret;
    uint8 requiredSize;

    requiredSize = LEON_UART_TX_SIZE - len;

    if (((LEON_UART_STATUS_TX_LEVEL_MASK & ReadLeonRegister(LEON_UART_STATUS_OFFSET)) >> LEON_UART_STATUS_TX_LEVEL_OFFSET) > requiredSize)
    {
        //there is not enough vancancy in the tx register to send the whole message
        ret = FALSE;
    }
    else
    {

        while(len >= WORD_LEN)
        {
            //since we will only have multiples of 4, we can transmit only words at a time
            WriteLeonRegister(LEON_UART_WRITE_WORD_OFFSET , *((uint32*)msg));
            len -= WORD_LEN;
            msg += WORD_LEN;
        }
        ret = TRUE;
    }

    return ret;
}


/**
* FUNCTION NAME: LEON_UartByteTx()
*
* @brief  - Writes a byte to the uart fifo
*
* @return - void
*
* @note   - Blocks until byte can be written
*/
void LEON_UartByteTx
(
    uint8 txByte    // Byte to transmit
)
{
#ifdef LEON_UART_TX_BUFFER_SIZE
    boolT loopAgain = TRUE;

    do {
        if (!uart_fifoFull())
        {
            uart_fifoWrite(txByte);
            loopAgain = FALSE;

            // It is possible that the SW fifo was empty previously, so enable the TX interrupt
            WriteLeonRegister(LEON_UART_CONTROL_OFFSET, CONTROL_W_TX_INT_EN);
        }

        LEON_UartPollingModeDoWork();
    } while (loopAgain);
#else
    while(((LEON_UART_STATUS_TX_LEVEL_MASK & ReadLeonRegister(LEON_UART_STATUS_OFFSET)) >> LEON_UART_STATUS_TX_LEVEL_OFFSET) == LEON_UART_TX_SIZE)
        ;
    WriteLeonRegister(LEON_UART_WRITE_BYTE_OFFSET, txByte);
#endif
}



/**
* FUNCTION NAME: LEON_UartWaitForTx()
*
* @brief  - blocks until entire fifo is transmitted
*
* @return - void
*
* @note   - This function is designed for polling modes,
*           so we assume we are in a critical section already
*/
void LEON_UartWaitForTx(void)
{
#ifdef LEON_UART_TX_BUFFER_SIZE
    // Wait for SW fifo to empty
    while (!uart_fifoEmpty())
    {
        LEON_UartPollingModeDoWork();
    }
#endif

    // Wait for HW fifo to empty
    while (LEON_UART_STATUS_TX_LEVEL_MASK & ReadLeonRegister(LEON_UART_STATUS_OFFSET))
        ;
}


/**
* FUNCTION NAME: LEON_UartPollingModeDoWork()
*
* @brief  - Does any work that can be done now, and won't block
*
* @return - void
*
* @note   - This function is designed for polling modes,
*           so we assume we are in a critical section already
*
*         - Currently does nothing for GE, unless LEON_UART_TX_BUFFER_SIZE is defined
*
*         - This is only a TX function right now
*         - We could break out a Tx version, Rx version, & external version which calls each
*         - The Rx version would do pretty much LEON_UartInterruptHandlerRx
*/
#ifdef LEON_UART_TX_BUFFER_SIZE
void LEON_UartPollingModeDoWork(void)
{
    // check if the HW is not busy & there are bytes to transmitt
    while (!uart_fifoEmpty() &&
        !(((LEON_UART_STATUS_TX_LEVEL_MASK & ReadLeonRegister(LEON_UART_STATUS_OFFSET)) >> LEON_UART_STATUS_TX_LEVEL_OFFSET) == LEON_UART_TX_SIZE))
    {
        //write byte directly to HW
        WriteLeonRegister(LEON_UART_WRITE_BYTE_OFFSET, uart_fifoRead());
    }

    if (uart_fifoEmpty())
    {
        // Disable the interrupt
        WriteLeonRegister(LEON_UART_CONTROL_OFFSET, CONTROL_W_TX_INT_DIS);
    }
}
#endif

/**
* FUNCTION NAME: LEON_UartSetRxHandler()
*
* @brief  - Changes the uart receiver handler function
*
* @return - old handler
*
* @note   - The old handler is returned so it could possibly be changed back later
*
*           Entire function runs in a critical section
*/
#ifndef LEON_NO_TRAP_INIT
pfnLEON_UartRxHandler LEON_UartSetRxHandler
(
    pfnLEON_UartRxHandler newUartRxHandler  // The new uart handler function
)
{
    pfnLEON_UartRxHandler oldUartRxHandler = uartRxHandler;

    uartRxHandler = newUartRxHandler;

    return oldUartRxHandler;
}
#endif

/**
* FUNCTION NAME: LEON_UartRx()
*
* @brief  - Checks for a received byte in the received byte fifo
*
* @return - true if byte was received, false otherwise
*
* @note   - This function is designed for polling modes,
*           so we assume we are in a critical section already
*/
boolT LEON_UartRx
(
    uint8 * rxByte  // pointer to store the byte received
)
{
    boolT ret;

    if (ReadLeonRegister(LEON_UART_STATUS_OFFSET) & LEON_UART_STATUS_RX_LEVEL_MASK)
    {
        *rxByte = ReadLeonRegister(LEON_UART_READ_DATA_OFFSET);
         ret = TRUE;
    }
    else
    {
         ret = FALSE;
    }

    return ret;
}


/**
* FUNCTION NAME: LEON_UartInterruptHandlerRx()
*
* @brief  - The uart Rx interrupt handler
*
* @return - void
*
* @note   -
*
*/
#ifndef LEON_NO_TRAP_INIT
void LEON_UartInterruptHandlerRx(void)
{
    if (ReadLeonRegister(LEON_UART_STATUS_OFFSET) & LEON_UART_STATUS_RX_LEVEL_MASK)
    {
        // Read a byte regardless, as we need to clear the isr, but only process it if we can
        uint8 rxByte = ReadLeonRegister(LEON_UART_READ_DATA_OFFSET);
        pfnLEON_UartRxHandler handler = uartRxHandler;  // ensure global variable is only read once
        if (handler)
        {
            (*handler)(rxByte);
        }
    }
}
#endif


/**
* FUNCTION NAME: LEON_UartInterruptHandlerTx()
*
* @brief  - The uart Tx interrupt handler
*
* @return - void
*
* @note   - Called when the uart TX buffer is empty
*
*/
#ifndef LEON_NO_TRAP_INIT
#ifdef LEON_UART_TX_BUFFER_SIZE

// LEON_UartPollingModeDoWork, doesn't block and tries to fill the HW buffer
// from the SW buffer
void LEON_UartInterruptHandlerTx(void) __attribute__ ((alias("LEON_UartPollingModeDoWork")));

#else // #ifdef LEON_UART_TX_BUFFER_SIZE

// Nothing to do
void LEON_UartInterruptHandlerTx(void)
{
}

#endif // #ifdef #else LEON_UART_TX_BUFFER_SIZE
#endif // #ifndef LEON_NO_TRAP_INIT

//#################################################################################################
// Calculate UART scaler value based on CPU frequency and requested baud rate
//
// Parameters:
//      baud                - baud rate in Hz
// Return:
//      UART scaler value to store in scaler register
// Assumptions:
//#################################################################################################
static uint32_t calcUARTScaler(uint32_t baud)
{
    // From Leon 11.3.4 // scaler = (((system_clk*10)/(baudrate*8))-5)/10
    return ((((60000000 * 10) / (baud * 8)) - 5) / 10) & UART_SCALER_SCALER_MASK;
}


#endif // #ifndef BB_GE_COMPANION
