//#################################################################################################
// Icron Technology Corporation - Copyright 2015
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
// UART driver module.  Note that this is a custom Icron built UART and is not one of the UARTs of
// the LEON CPU.
//#################################################################################################

//#################################################################################################
// Design Notes
//#################################################################################################
//#################################################################################################


// Includes #######################################################################################

#include <options.h>
#include <sys_defs.h>
#include <leon_traps.h>
#include <interrupts.h>
#include <uart_regs.h>
#include <bb_core.h>
#include <bb_top.h>
#include <callback.h>
#include <ibase.h>
#include <stdarg.h>
#include <timing_profile.h>
#include <module_addresses_regs.h>
#include <leon_timers.h>
#include <leon2_regs.h>
#include <leon_cpu.h>
#include <uart.h>

#include "uart_loc.h"
#include "uart_cmd.h"

// Constants and Macros ###########################################################################
#define WORD_LEN 4

#ifndef LEON_NO_TRAP_INIT
#ifndef DEFAULT_UART_RX_HANDLER
#define DEFAULT_UART_RX_HANDLER NULL
#endif // DEFAULT_UART_RX_HANDLER
#endif // LEON_NO_TRAP_INIT

#define GE_TX_BUFF_SIZE (512)


// Data Types #####################################################################################

typedef struct _UartDef
{
    volatile uart_s* uart_registers;    // pointer to the base of the UART registers
    volatile old_uart_s *old_uart_registers;
    uint32_t hwRxOverRun;                 // number of Rx characters lost
    uint16_t rxOverRun;                 // number of Rx characters lost
    pfnUART_RxHandler uartRxHandler;    // Rx Handler to call
    pfnUART_TxHandler uartTxHandler;    // Tx Handler to call
    UartFifoT uartTxFifo;               // the Tx Fifo
    UartFifoT uartRxFifo;               // the Rx Fifo

    CallbackHandleT processRxCallback;    // the callback used to process Rx characters

} UartDefT;

typedef struct
{
    uint16_t rxBufferSize;
    uint16_t txBufferSize;

    uint8_t *rxBuffer;
    uint8_t *txBuffer;

} UartFifoSettings;

LEON_TimerValueT timeSpentInRx;
uint32_t maxTimeSpentInRx;
uint32_t sumOfTime;
uint32_t intCount;

LEON_TimerValueT geTimeSpentInRx;
uint32_t geMaxTimeSpentInRx;
uint32_t geSumOfTime;
uint32_t geIntCount;

// Global Variables ###############################################################################


// Static Variables ###############################################################################
static UartDefT uartCtrl[MAX_NUM_PORTS];  // used to contain all the data needed by this port

// allocate the UART FIFO's needed for BB <-> external world
static uint8_t blackBirdRxBuffer[UART_BB_RX_BUFFER_SIZE];
static uint8_t blackBirdTxBuffer[UART_BB_TX_BUFFER_SIZE];

// allocate the UART FIFO's needed for BB <-> GE
static uint8_t bbgeRxBuffer[UART_BBGE_RX_BUFFER_SIZE];
static uint8_t bbgeTxBuffer[UART_BBGE_TX_BUFFER_SIZE];

static const UartFifoSettings uartFifoDefinitions[MAX_NUM_PORTS] =
{
    {   // BB <-> external world comm port
        .rxBufferSize = sizeof(blackBirdRxBuffer),
        .txBufferSize = sizeof(blackBirdTxBuffer),

        .rxBuffer = blackBirdRxBuffer,
        .txBuffer = blackBirdTxBuffer
    },
    {   // BB <-> GE comm port
        .rxBufferSize = sizeof(bbgeRxBuffer),
        .txBufferSize = sizeof(bbgeTxBuffer),

        .rxBuffer = bbgeRxBuffer,
        .txBuffer = bbgeTxBuffer
    },
};

static uint8_t printf_msg[256];

#ifdef GE_RX_TX_BUFF_DEBUG
static uint8_t geRxBuff[1536]; // 5pkts
static uint8_t geTxBuff[GE_TX_BUFF_SIZE];
static uint16_t geTxBuffOffset;
static bool dumpingUart;
#endif // GE_RX_TX_BUFF_DEBUG

static bool newUart;

// Static Function Declarations ###################################################################
static void _UART_initPort(uint8_t port, volatile void* regAddr, uint32_t baud);

//static bool atomicHWFifoTx(const uint8_t * msg, uint8_t len);
static uint32_t calcUARTScaler(uint32_t baud);
static void UART_ProcessGeRx(void);
static void UART_ProcessRxByte(enum UartPort uartPort);
static void UART_resetHwFifoFullCount(uint8_t port);
static uint32_t UART_getHwFifoFullCount(uint8_t port);
static uint32_t UART_getSwFifoOverRunCount(uint8_t port);

// Exported Function Definitions ##################################################################

//#################################################################################################
// Initialize UART scaler, map struct to UART register address, and enable tx/rx
//
// Parameters:
//      uartRegistersAddress- base address of UART registers
//      baud                - baud rate in Hz
// Return:
// Assumptions:
//      * register pointer address is accurate.
//#################################################################################################
void UART_Init(void)
{
    volatile void* uartRegistersAddress = (volatile void*) bb_chip_uart_s_ADDRESS;
    volatile void* geUartRegistersAddress = (volatile void*) bb_chip_ge_uart_s_ADDRESS;

    newUart = true;

    bb_top_ResetBBUart(false);          // FPGA BB UART block out of reset
    bb_top_ResetGeUart(false);          // FPGA GE UART block out of reset

    _UART_initPort(UART_PORT_BB, uartRegistersAddress, LEON_BBFW_UART_BAUD);

    // GE Baudrate is fixed
    _UART_initPort(UART_PORT_GE, geUartRegistersAddress, LEON_BBGE_UART_BAUD);

    // TODO: this should really be done in _UART_initPort(), but that is also called for baud rate changes
    uartCtrl[UART_PORT_BB].processRxCallback = CALLBACK_Allocate(UART_packetizeDecodeRxData, (void *)UART_PORT_BB, NULL);
    uartCtrl[UART_PORT_GE].processRxCallback = CALLBACK_Allocate(UART_packetizeDecodeRxData, (void *)UART_PORT_GE, NULL);

#ifdef GE_RX_TX_BUFF_DEBUG
    geRxBuffOffset = 0;
#endif

//    _UART_initPort(UART_PORT_TYPE_C, typeCUartRegistersAddress, baud);
// TODO: make sure this works with poll driven?

    // LEON_WRITE_BF(LEON2_IRQCTRL2_INT_MASK, _IMASK, SECONDARY_INT_GE_UART_RX_INT_MSK | SECONDARY_INT_GE_UART_TX_INT_MSK);
    //TOPLEVEL_setPollingMask(SECONDARY_INT_GE_UART_RX_INT_MSK | SECONDARY_INT_GE_UART_TX_INT_MSK);
    LEON_EnableIrq2Bits(SECONDARY_INT_GE_UART_RX_INT_MSK | SECONDARY_INT_GE_UART_TX_INT_MSK);

    LEON_EnableIrq(IRQ_UART_RX);
    LEON_EnableIrq(IRQ_UART_TX);
}


//#################################################################################################
// Writes a byte to the uart fifo
//
// Parameters:
//      txByte              - byte to transmit
// Return:
// Assumptions:
//      * caller understands this blocks until byte can be written
//#################################################################################################
void UART_ByteTx(uint8_t txByte)
{
    UART_ByteTxByCh(UART_PORT_BB, txByte);
}


//#################################################################################################
// Writes a byte to the uart fifo
//
// Parameters:
//      txByte              - byte to transmit
// Return:
// Assumptions:
//      * caller understands this blocks until byte can be written
//#################################################################################################
void UART_ByteTxByCh(uint8_t port, uint8_t txByte)
{
    UART_WaitForTxSpaceByCh(port, 1);

    UartfifoWrite(&(uartCtrl[port].uartTxFifo), txByte);

    UART_PollingModeDoWork();   // if there is space, send it out
}

//#################################################################################################
// Blocks until entire Tx fifo is transmitted.
//
// Parameters:
// Return:
// Assumptions:
//      * used in critical section of code, designed for polling modes
//#################################################################################################
void UART_WaitForTx(void)
{
    bool fifoNotEmpty = true;
    // Wait for SW fifo to empty
    while ( !UartfifoEmpty(&(uartCtrl[UART_PORT_BB].uartTxFifo)) )
    {
        UART_PollingModeDoWork();
    }
    // Wait for HW fifo to empty

    while(fifoNotEmpty)
    {
        if(newUart)
            fifoNotEmpty = !(uartCtrl[UART_PORT_BB].uart_registers->tx_fifo.bf.fifo_empty == 1);
        else
            fifoNotEmpty = (uartCtrl[UART_PORT_BB].old_uart_registers->status.bf.tx_fifo_level > 0);

        UART_PollingModeDoWork();   // process Rx and the GE port in the meantime
    }
}

//#################################################################################################
// blocks until the required space is available
//
// Parameters:
//      spaceNeeded              - space required
// Return:
// Assumptions:
//
//#################################################################################################
void UART_WaitForTxSpace(uint16_t spaceNeeded)
{
    UART_WaitForTxSpaceByCh(UART_PORT_BB, spaceNeeded);
}


//#################################################################################################
// blocks until the required space is available
//
// Parameters:
//      port              - port to check for space
// Return:
// Assumptions:
//
//#################################################################################################
void UART_WaitForTxSpaceByCh(uint8_t port, uint16_t spaceNeeded)
{
    while ( (UartfifoSpaceAvail( UartGetTxFifo(port) ) < spaceNeeded ) &&
            ( !UartfifoEmpty(&(uartCtrl[UART_PORT_BB].uartTxFifo)) ) )
    {
        UART_PollingModeDoWork();   // process BB and the GE port in the meantime
    }
}

//#################################################################################################
// Change uart receiver handler function.
//
// Parameters:
//      newUartRxHandler    - The new uart handler function
// Return:
//      A pointer to the previous rx handler.
// Assumptions:
//      * Assume in critical section of code
//#################################################################################################
#ifndef LEON_NO_TRAP_INIT
pfnUART_RxHandler UART_SetRxHandler(pfnUART_RxHandler newUartRxHandler)
{
    return (UART_SetRxHandlerByCh(UART_PORT_BB, newUartRxHandler));
}
#endif


//#################################################################################################
// Change uart receiver handler function.
//
// Parameters:
//      newUartRxHandler    - The new uart handler function
//      port                - UART channel to apply handler to
// Return:
//      A pointer to the previous rx handler.
// Assumptions:
//      * Assume in critical section of code
//#################################################################################################
pfnUART_RxHandler UART_SetRxHandlerByCh(uint8_t port, pfnUART_RxHandler newUartRxHandler)
{
    pfnUART_RxHandler oldRxHandler = uartCtrl[port].uartRxHandler;
    uartCtrl[port].uartRxHandler = newUartRxHandler;

    // return the old handler in case the caller wants to restore it later.
    return (oldRxHandler);
}


//#################################################################################################
// Non-blocking, does any work that can be done now.
//
// Parameters:
// Return:
// Assumptions:
//      * Designed for polling modes
//      * Assume in critical section of code
//      * Only a TX function at this time
//#################################################################################################
void UART_PollingModeDoWork(void)
{
    UART_PollingModeDoWorkByCh(UART_PORT_BB);
    UART_PollingModeDoWorkByCh(UART_PORT_GE);
}


//#################################################################################################
// Non-blocking, does any work that can be done now.
//
// Parameters:
// Return:
// Assumptions:
//      * Designed for polling modes
//      * Assume in critical section of code
//      * Only a TX function at this time
//#################################################################################################
void UART_PollingModeDoWorkByCh(uint8_t port)
{
    // check if the HW is not busy & there are bytes to transmit
    if(newUart)
    {
        while (!UartfifoEmpty(&(uartCtrl[port].uartTxFifo)) &&
           !(uartCtrl[port].uart_registers->tx_fifo.bf.fifo_full))
            uartCtrl[port].uart_registers->tx_data.bf.val = UartfifoRead(&(uartCtrl[port].uartTxFifo));
    }
    else
    {
        // write byte directly to HW
        while (!UartfifoEmpty(&(uartCtrl[port].uartTxFifo)) &&
               (uartCtrl[port].old_uart_registers->status.bf.tx_fifo_level < LEON_UART_TX_SIZE))
            uartCtrl[port].old_uart_registers->wr_byte.bf.wr_byte = UartfifoRead(&(uartCtrl[port].uartTxFifo));
    }
    if ( UartfifoEmpty(&(uartCtrl[port].uartTxFifo)) )
    {
        // Disable the tx interrupt - no more bytes waiting to be sent
        if(newUart)
        {
            uartCtrl[port].uart_registers->control.bf.tx_irq_en = 0;
        }
        else
        {
            uartCtrl[port].old_uart_registers->control.bf.tx_int_en = 0;
        }

        // check if registered handler exists for TxFifoEmpty
        if (uartCtrl[port].uartTxHandler != NULL)
        {
            (*(uartCtrl[port].uartTxHandler))();
        }
    }
    else
    {
        // still bytes waiting to transmit, make sure the tx interrupt is ON
        if(newUart)
            uartCtrl[port].uart_registers->control.bf.tx_irq_en = 1;
        else
            uartCtrl[port].old_uart_registers->control.bf.tx_int_en = 1;
    }

//    UART_RxInterrupt(port);  // receive any bytes on this port as well
}


//#################################################################################################
// Change uart receiver handler function.
//
// Parameters:
//      newUartRxHandler    - The new uart handler function
//      port                - UART channel to apply handler to
// Return:
//      A pointer to the previous rx handler.
// Assumptions:
//      * Assume in critical section of code
//#################################################################################################
pfnUART_TxHandler UART_SetTxHandlerByCh(uint8_t port, pfnUART_TxHandler newUartTxHandler)
{
    pfnUART_TxHandler oldTxHandler = uartCtrl[port].uartTxHandler;
    uartCtrl[port].uartTxHandler = newUartTxHandler;

    // return the old handler in case the caller wants to restore it later.
    return (oldTxHandler);
}


//#################################################################################################
// Tries to read a single byte out of the hardware FIFO.  If a byte is present it will be written
// into the rxByte output paramemter and true will be returned.  If there is no byte in the FIFO,
// false will be returned.
//
// Parameters:
//      rxByte              - Location to store the byte that is read
// Return:
//      true if byte received, false otherwise.
// Assumptions:
//      * Designed for polling modes
//      * Assume in critical section of code
//#################################################################################################
bool UART_Rx(uint8_t* rxByte)
{
    bool dataToRead;

    if(newUart)
        dataToRead = !(uartCtrl[UART_PORT_BB].uart_registers->rx_fifo.bf.fifo_empty);
    else
        dataToRead = (uartCtrl[UART_PORT_BB].old_uart_registers->status.bf.rx_fifo_level != 0);

    if (dataToRead)
    {
        if(newUart)
            *rxByte = uartCtrl[UART_PORT_BB].uart_registers->rx_data.bf.val;
        else
            *rxByte = uartCtrl[UART_PORT_BB].old_uart_registers->rd_data.bf.rd_data;
    }

    return dataToRead;
}

//#################################################################################################
// Handler UART Rx interrupts
//
// Parameters:
// Return:
// Assumptions:
//      * handler not null, otherwise byte is lost
//#################################################################################################
void UART_InterruptHandlerRx(void)
{
    uint32_t val;
    intCount++;
    timeSpentInRx = LEON_TimerRead();

    if(UART_RxInterrupt(UART_PORT_BB))  // Empty the hardware fifo to clear the interrupt
    {
        // Run packet decoder only when we get new data, because this function is polling after an assert
        CALLBACK_Schedule(uartCtrl[UART_PORT_BB].processRxCallback);
    }

    val = LEON_TimerCalcUsecDiff(timeSpentInRx, LEON_TimerRead());
    if(val > maxTimeSpentInRx)
        maxTimeSpentInRx = val;

    sumOfTime += val;
}

//#################################################################################################
// Handler UART Rx interrupts
//
// Parameters:
// Return:
// Assumptions:
//      * handler not null, otherwise byte is lost
//#################################################################################################
void UART_InterruptHandlerRxGe(void)
{
    uint32_t val;
    geIntCount++;
    geTimeSpentInRx = LEON_TimerRead();
#ifdef BB_PROFILE
UTIL_timingProfileStartTimer(UTIL_PROFILE_TIMERS_GE_RX_IRQ);
#endif
    UART_RxInterrupt(UART_PORT_GE);
    UART_ProcessGeRx();   // This will return a character if process fails We need to do something with

    val = LEON_TimerCalcUsecDiff(geTimeSpentInRx, LEON_TimerRead());
    if(val > geMaxTimeSpentInRx)
        geMaxTimeSpentInRx = val;

    geSumOfTime += val;
#ifdef BB_PROFILE
UTIL_timingProfileStopTimer(UTIL_PROFILE_TIMERS_GE_RX_IRQ);
#endif
}

//#################################################################################################
// Handler UART Rx interrupts
//
// Parameters:
// Return:
// Assumptions:
//      * handler not null, otherwise byte is lost
//#################################################################################################
bool UART_RxInterrupt(enum UartPort uartPort)
{
    uint8_t rxByte;
    bool updateFifo = false;

    if(newUart)
    {
        if (uartCtrl[uartPort].uart_registers->rx_fifo.bf.fifo_full)
        {
            uartCtrl[uartPort].hwRxOverRun++;
        }
    }
    else
    {
        if (uartCtrl[uartPort].old_uart_registers->status.bf.rx_fifo_level >= 0x18)
        {
            uartCtrl[uartPort].hwRxOverRun++;
        }
    }

     // Read a byte regardless, as we need to clear the isr, but only process it if we can
    if(newUart)
    {
        while (!uartCtrl[uartPort].uart_registers->rx_fifo.bf.fifo_empty)
        {
            updateFifo = true;
            rxByte = uartCtrl[uartPort].uart_registers->rx_data.bf.val;
#ifdef GE_RX_TX_BUFF_DEBUG
            if ((UART_PORT_BB == uartPort))// && (validPktCounter > 2))
            {
                geRxBuff[geRxBuffOffset] = rxByte;
                geRxBuffOffset++;
            }
#endif
            // if the fifo is not full, store the received byte; otherwise
            // just drop them so we can clear the interrupt
            if ( !UartfifoFull(&(uartCtrl[uartPort].uartRxFifo)) )
            {
                UartfifoWrite(&(uartCtrl[uartPort].uartRxFifo), rxByte);
            }
            else
            {
                uartCtrl[uartPort].rxOverRun++;  // keep track of how many bytes we've lost
            }
        }
    }
    else
    {
        while (uartCtrl[uartPort].old_uart_registers->status.bf.rx_fifo_level)
        {
            updateFifo = true;
            rxByte = uartCtrl[uartPort].old_uart_registers->rd_data.bf.rd_data;
#ifdef GE_RX_TX_BUFF_DEBUG
            if ((UART_PORT_BB == uartPort))// && (validPktCounter > 2))
            {
                geRxBuff[geRxBuffOffset] = rxByte;
                geRxBuffOffset++;
            }
#endif
            // if the fifo is not full, store the received byte; otherwise
            // just drop them so we can clear the interrupt
            if ( !UartfifoFull(&(uartCtrl[uartPort].uartRxFifo)) )
            {
                UartfifoWrite(&(uartCtrl[uartPort].uartRxFifo), rxByte);
            }
            else
            {
                uartCtrl[uartPort].rxOverRun++;  // keep track of how many bytes we've lost
            }
        }
    }

    return updateFifo;
}


//#################################################################################################
// Gets the specified UART's Rx FIFO.  Used by the receive packetize functions.
//
// Parameters:
// Return:
// Assumptions:
//      * handler not null, otherwise byte is lost
//#################################################################################################

UartFifoT * UartGetRxFifo(uint8_t port)
{
    return ( &(uartCtrl[port].uartRxFifo) );
}

//#################################################################################################
// Gets the specified UART's Tx FIFO.  Used by the transmit packetize functions.
//
// Parameters:
// Return:
// Assumptions:
//      * handler not null, otherwise byte is lost
//#################################################################################################

UartFifoT * UartGetTxFifo(uint8_t port)
{
    return ( &(uartCtrl[port].uartTxFifo) );
}


//#################################################################################################
// Handler UART Tx interrupts
//
// Parameters:
// Return:
// Assumptions:
//      * called when UART tx buffer is empty
//#################################################################################################

// see if there are any more bytes to send
void UART_InterruptHandlerTx(void)
{
    UART_PollingModeDoWorkByCh(UART_PORT_BB);
}


//#################################################################################################
// Handler UART Tx interrupts
//
// Parameters:
// Return:
// Assumptions:
//      * called when UART tx buffer is empty
//#################################################################################################

// UART_PollingModeDoWork, doesn't block and tries to fill the HW buffer
// from the SW buffer
void UART_InterruptHandlerTxGe(void)
{
    UART_PollingModeDoWorkByCh(UART_PORT_GE);
}

//#################################################################################################
// Check status of RX fifo
//
// Parameters:
// Return:
// Assumptions:
//      * handler not null, otherwise byte is lost
//#################################################################################################
bool UART_IsRxFifoFullEmpty(uint8_t port, bool fullEmpty)
{
    if (fullEmpty) // check if empty
    {
        return ( UartfifoEmpty(&(uartCtrl[port].uartRxFifo)) );
    }
    else
    {
        return ( UartfifoFull(&(uartCtrl[port].uartRxFifo)) );
    }
}


//#################################################################################################
// Check status of RX fifo
//
// Parameters:
// Return:
// Assumptions:
//      * handler not null, otherwise byte is lost
//#################################################################################################
bool UART_IsTxFifoFullEmpty(uint8_t port, bool fullEmpty)
{
    if (fullEmpty) // check if empty
    {
        return ( UartfifoEmpty(&(uartCtrl[port].uartTxFifo)) );
    }
    else
    {
        return ( UartfifoFull(&(uartCtrl[port].uartTxFifo)) );
    }

}


//#################################################################################################
// Copy SW fifo into passed buffer
//
// Parameters:
// Return:
// Assumptions:
//      * handler not null, otherwise byte is lost
//#################################################################################################
bool UART_copyFifoToBuffer(uint8_t port, uint8_t* dst, uint16_t length)
{
    return ( UartfifoCopyFifoToBuffer(dst, &(uartCtrl[port].uartRxFifo), length) );
}

//#################################################################################################
// Clear TX HW fifo and SW fifo
//
// Parameters:
// Return:
// Assumptions:
//      * handler not null, otherwise byte is lost
//#################################################################################################
void UART_clearFifo(enum UartPort port)
{
    // clear TX and Rx fifo
    UartfifoClear(&(uartCtrl[port].uartTxFifo));
    UartfifoClear(&(uartCtrl[port].uartRxFifo));
}


//#################################################################################################
// Clear TX HW fifo and SW fifo
//
// Parameters:
// Return:
// Assumptions:
//      * handler not null, otherwise byte is lost
//#################################################################################################
void UART_clearGeTx(void)
{
    // clear TX fifo
    UartfifoClear(&(uartCtrl[UART_PORT_GE].uartTxFifo));
}


//#################################################################################################
// Clear RX HW fifo and SW fifo
//
// Parameters:
// Return:
// Assumptions:
//      * handler not null, otherwise byte is lost
//#################################################################################################
void UART_clearGeRx(void)
{
    // read from HW RX reg until cleared
    UART_RxInterrupt(UART_PORT_GE);

    // clear RX fifo
    UartfifoClear(&(uartCtrl[UART_PORT_GE].uartRxFifo));
}

//#################################################################################################
// Simple printf that avoids ilogs, useful for debugging
//
// Parameters:
//          character string to print
// Return:
// Assumptions:
//#################################################################################################
void UART_printf(const char* format, ...)
{
    const char* cursor = format;
    va_list vl;
    va_start(vl, format);
    uint8_t msgIdx = 0;
    memset(printf_msg, 0, sizeof(printf_msg));

    while(*cursor != 0)
    {
        if(*cursor == '%')
        {
            cursor++;
            switch(*cursor)
            {
                case '%':
                    printf_msg[msgIdx++] = '%';
                    break;

                case 'x':
                    {
                        uint32_t val = va_arg(vl, uint32_t);
                        const char* hexChars = "0123456789ABCDEF";
                        for (int8_t nibble = 7; nibble >= 0; nibble--)
                        {
                            const uint8_t nibbleVal = (val >> 28);
                            printf_msg[msgIdx++] = hexChars[nibbleVal];
                            val <<= 4;
                        }
                    }
                    break;

                case 'd':
                    {
                        uint32_t val = va_arg(vl, uint32_t);
                        uint8_t digits[10]; // 2 ** 32 is 10 digits long
                        uint8_t currentDigit = 0;
                        do
                        {
                            digits[currentDigit] = val % 10;
                            currentDigit++;
                            val /= 10;
                        } while (val != 0);

                        for (uint8_t i = 0; i < currentDigit; i++)
                        {
                            printf_msg[msgIdx++] = '0' + digits[currentDigit - (i + 1)];
                        }
                    }
                    break;

                case 's':
                    {
                        const char* val = va_arg(vl, const char*);
                        while(*val != 0)
                        {
                            printf_msg[msgIdx++] = *val;
                            val++;
                        }
                    }
                    break;

                default:
                    break;
            }
            cursor++;
        }
        else
        {
            while(*cursor != '%' && *cursor != 0)
            {
                if (*cursor == '\n')
                {
                    printf_msg[msgIdx++] = '\r';
                }
                printf_msg[msgIdx++] = *cursor;
                cursor++;
            }
        }
    }

    va_end(vl);
#ifdef BB_PACKETIZE_DISABLED
    for (int i=0; i < msgIdx; i++)
    {
        UART_ByteTx(printf_msg[i]);
    }
#else
    UART_packetizeSendDataImmediate(
        UART_PORT_BB,
        CLIENT_ID_BB_PRINTF,
        NULL,
        printf_msg,
        msgIdx);
#endif // BB_PACKETIZE_DISABLED
//    UART_WaitForTx();
}


//#################################################################################################
// Set Baudrate for BB
//
// Parameters: port, baud
// Return:
// Assumptions: uartCtrl[port].uart_registers need to be set before calling this function
//#################################################################################################
void UART_setBaudrate(uint8_t port, uint32_t baud)
{
    if(newUart)
        uartCtrl[port].uart_registers->control.bf.prescaler = calcUARTScaler(baud);
    else
        uartCtrl[port].old_uart_registers->scaler.bf.scaler = calcUARTScaler(baud);
}

#ifdef BB_PROGRAM_BB
//#################################################################################################
// Check Baud rate to indicate download type
//  Fast speed: BB FW
//  Low speed: GE automatic download
//
// Parameters: port
// Return:
// Assumptions:
//#################################################################################################
bool UART_IsHighSpeed(uint8_t port)
{
    bool fastBaud = true;
    uint32_t baudRegValue;

    if(newUart)
        baudRegValue = uartCtrl[port].uart_registers->control.bf.prescaler;
    else
        baudRegValue = uartCtrl[port].old_uart_registers->scaler.bf.scaler;

    if(baudRegValue == calcUARTScaler(LEON_UART_BAUD_115200))
    {
        fastBaud = false;
    }
//    ilog_UART_COMPONENT_1(ILOG_MINOR_EVENT, UART_HIGH_SPEED, fastBaud);
    return fastBaud;
}
#endif

// Component Scope Function Definitions ###########################################################
//#################################################################################################
// UART Rx hw fifo overflow debugging
//
// Parameters: port
// Return:
// Assumptions:
//#################################################################################################
void UART_ShowStats()
{
    ilog_UART_COMPONENT_2(ILOG_USER_LOG, BB_OVERRUN,
        UART_getHwFifoFullCount(UART_PORT_BB),
        UART_getSwFifoOverRunCount(UART_PORT_BB));

    ilog_UART_COMPONENT_1(ILOG_USER_LOG, MAX_UART_RX_TIME, (uint32_t)maxTimeSpentInRx);
    if(intCount > 0)
        ilog_UART_COMPONENT_1(ILOG_USER_LOG, AVG_UART_RX_TIME, (uint32_t)(sumOfTime/intCount));

    ilog_UART_COMPONENT_2(ILOG_USER_LOG, GE_OVERRUN,
        UART_getHwFifoFullCount(UART_PORT_GE),
        UART_getSwFifoOverRunCount(UART_PORT_GE));

    ilog_UART_COMPONENT_1(ILOG_USER_LOG, MAX_UART_RX_TIME, (uint32_t)geMaxTimeSpentInRx);
    if(geIntCount > 0)
        ilog_UART_COMPONENT_1(ILOG_USER_LOG, AVG_UART_RX_TIME, (uint32_t)(geSumOfTime/geIntCount));

    UART_resetHwFifoFullCount(UART_PORT_BB);
    UART_resetHwFifoFullCount(UART_PORT_GE);

    maxTimeSpentInRx = 0;
    intCount = 0;
    sumOfTime = 0;

    geMaxTimeSpentInRx = 0;
    geIntCount = 0;
    geSumOfTime = 0;

}

//#################################################################################################
// UART Rx hw fifo overflow debugging
//
// Parameters: port
// Return:
// Assumptions:
//#################################################################################################
void UART_BBChangeBaudRate(uint32_t baudRate)
{
    volatile void* uartRegistersAddress = (volatile void*) bb_chip_uart_s_ADDRESS;

    switch(baudRate)
    {
        case 0:
        case 115200:
             // BB FW Set Baudrate slow speed
            _UART_initPort(UART_PORT_BB, uartRegistersAddress, LEON_UART_BAUD_115200);
            break;
        case 1:
        case 460800:
             // BB FW Set Baudrate fast speed
            _UART_initPort(UART_PORT_BB, uartRegistersAddress, LEON_UART_BAUD_460800);
            break;
        case 2:
        default:
            break;
    }
}

//#################################################################################################
// Schedules another callback to the Packet decoder
//
// Parameters: port
// Return:
// Assumptions:
//#################################################################################################
void UartScheduleRxDecode(enum UartPort uartPort)
{
    CALLBACK_Schedule(uartCtrl[uartPort].processRxCallback);
}

// Static Function Definitions ####################################################################

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
#if 1
    return (bb_core_getCpuClkFrequency() / baud) & old_UART_SCALER_SCALER_MASK;
#else
    uint32_t scaler = ((((bb_core_getCpuClkFrequency() * 10) / (baud * 8)) -5) / 10);
    if (baud > 230400)
    {
        scaler += 1;
    }
    return scaler & UART_SCALER_SCALER_MASK;
#endif
    return (bb_core_getCpuClkFrequency() / baud) & 0xFFFF;
}


//#################################################################################################
// Atomically write all the bytes given (will be a multiple of 4) or none at all
//
// Parameters:
//      msg    - Pointer to the data to transmit
//      len    - Length of the message in bytes
// Return:
//      true if entire message is written, false if nothing is written.
// Assumptions:
//      * intended for hardware fifo writing words
//#################################################################################################
/*
static bool atomicHWFifoTx(const uint8_t* msg, uint8_t len)
{
    bool ret;
    uint8_t requiredSize;

    requiredSize = LEON_UART_TX_SIZE - len;

    if (uart_registers->status.bf.tx_fifo_level > requiredSize)
    {
        //there is not enough vancancy in the tx register to send the whole message
        ret = false;
    }
    else
    {

        while(len >= WORD_LEN)
        {
            //since we will only have multiples of 4, we can transmit only words at a time
            uart_registers->wr_word.bf.wr_word = *((uint32_t*)msg);
            len -= WORD_LEN;
            msg += WORD_LEN;
        }
        ret = true;
    }

    return ret;
}
*/

//#################################################################################################
// Processes GE UART Rx data
//
// Parameters:
// Return:
// Assumptions: Only GE has a non-packetized mode when it programs
//
//#################################################################################################
static void UART_ProcessGeRx()
{
    if(UART_isPacketizeEnabled(UART_PORT_GE))
    {
        UartScheduleRxDecode(UART_PORT_GE);
    }
    else
    {
        UART_ProcessRxByte(UART_PORT_GE);
    }
}

//#################################################################################################
// Processes a waiting character on the port's Fifo
//
// Parameters:
// Return:
// Assumptions:
//      * handler not null, otherwise byte is lost
//#################################################################################################
static void UART_ProcessRxByte(enum UartPort uartPort)
{
    if ( !UartfifoEmpty(&(uartCtrl[uartPort].uartRxFifo)) )
    {
        uint8_t rxByte = UartfifoRead( &(uartCtrl[uartPort].uartRxFifo));
        if (uartCtrl[uartPort].uartRxHandler)
        {
            (*(uartCtrl[uartPort].uartRxHandler))(rxByte);
        }
    }
}

//#################################################################################################
// Initialize port - wrapper for standard inits using static struct
//  Don't set baudrate in case of program_bb: It will be set before calling program_bb
//  And baud rate speed is used to detect GE automatic download (GE: lowspeed, BB: highspeed)
// Parameters:
//          port - channel to read from
// Return:
// Assumptions:
//#################################################################################################
static void _UART_initPort(uint8_t port, volatile void* regAddr, uint32_t baud)
{
    uartCtrl[port].uart_registers = regAddr;
    uartCtrl[port].old_uart_registers = regAddr;

    if(uartCtrl[port].uart_registers->version.bf.major > 3)
    {
       newUart = true;
    }
    else
    {
       newUart = false;
    }

    uartCtrl[port].uartTxFifo.fifo      = uartFifoDefinitions[port].txBuffer;
    uartCtrl[port].uartTxFifo.fifoSize  = uartFifoDefinitions[port].txBufferSize;

    uartCtrl[port].uartRxFifo.fifo      = uartFifoDefinitions[port].rxBuffer;
    uartCtrl[port].uartRxFifo.fifoSize  = uartFifoDefinitions[port].rxBufferSize;

    // rx and tx enabled
    if(newUart)
    {
        uartCtrl[port].uart_registers->control.bf.prescaler = calcUARTScaler(baud);
        uartCtrl[port].uart_registers->control.bf.rx_en     = 1;
        uartCtrl[port].uart_registers->control.bf.tx_en     = 1;

        uartCtrl[port].uart_registers->timeout.bf.val = uartCtrl[port].uart_registers->control.bf.prescaler * 8; // timeout for 8 bit times
        uartCtrl[port].uart_registers->rx_fifo.bf.fifo_aft = uartCtrl[port].uart_registers->rx_fifo.bf.fifo_depth - 24;
        uartCtrl[port].uart_registers->rx_fifo.bf.fifo_aet = 0;
        uartCtrl[port].uart_registers->tx_fifo.bf.fifo_aft = uartCtrl[port].uart_registers->tx_fifo.bf.fifo_depth - 1;;
        uartCtrl[port].uart_registers->tx_fifo.bf.fifo_aet = 1;

        // rx interrupts on (tx interrupts on only when there is something to send
        uartCtrl[port].uart_registers->control.bf.rx_irq_en = 1;
    }
    else
    {
        uartCtrl[port].old_uart_registers->scaler.bf.scaler = calcUARTScaler(baud);

        // enable the low level HW
        old_uart_control uartInitControl;

        uartInitControl.dw = 0;  // default all unused bits to 0
        uartInitControl.bf.rx_en     = 1;
        uartInitControl.bf.tx_en     = 1;
        // rx interrupts on (tx interrupts on only when there is something to send
        uartInitControl.bf.rx_int_en = 1;
        uartCtrl[port].old_uart_registers->control.dw = uartInitControl.dw;
    }
}

//#################################################################################################
// Reset full RX fifo stats
//
// Parameters:
//          port - channel to set
// Return:
// Assumptions:
//#################################################################################################
static void UART_resetHwFifoFullCount(uint8_t port)
{
    uartCtrl[port].hwRxOverRun = 0;
}

//#################################################################################################
// Get full RX fifo stats
//
// Parameters:
//          port - channel to read from
// Return:
// Assumptions:
//#################################################################################################
static uint32_t UART_getHwFifoFullCount(uint8_t port)
{
    return uartCtrl[port].hwRxOverRun;
}

//#################################################################################################
// Get full RX fifo stats
//
// Parameters:
//          port - channel to read from
// Return:
// Assumptions:
//#################################################################################################
static uint32_t UART_getSwFifoOverRunCount(uint8_t port)
{
    return uartCtrl[port].rxOverRun;
}

#ifdef GE_RX_TX_BUFF_DEBUG
//#################################################################################################
// Get full RX fifo stats
//
// Parameters:
//          port - channel to read from
// Return:
// Assumptions:
//#################################################################################################
void UART_dumpGeRxBuffer(void)
{
    uint32_t i = 0;
    uint32_t *ptr = (uint32_t*)&geRxBuff;
    UART_printf("\n\nGeRxBuffer, offset %d\n", geRxBuffOffset);
    for (i = 0; i < geRxBuffOffset; i+=4)
    {
        UART_printf("%d:%x ",i,*ptr);
        ptr++;
    }
    UART_printf("\n\nGeRxBuffer Done\n\n");
    UART_printf("BB hwRxOverRun %d, rxOverRun %d\n",
                uartCtrl[UART_PORT_BB].hwRxOverRun,
                uartCtrl[UART_PORT_BB].rxOverRun);
    geRxBuffOffset = 0;
    validPktCounter = 0;
    memset(geRxBuff, 0, sizeof(geRxBuff));
}


//#################################################################################################
// Get full RX fifo stats
//
// Parameters:
//          port - channel to read from
// Return:
// Assumptions:
//#################################################################################################
void UART_dumpGeTxBuffer(void)
{
    uint16_t i = 0;
    dumpingUart = true;
    UART_printf("\n\nGeTxBuffer printing %d bytes\n", geTxBuffOffset);
    uint32_t val = 0;
    for (i = 0; i < geTxBuffOffset; i++)
    {
        if (i % 16 == 0)
        {
            UART_printf("\n offset %d ", i);
        }
        if ((i % 4 == 0) && (i > 0))
        {
            UART_printf("%x ",val);
            val = 0;
        }
        val |= geTxBuff[i] << (24 - (i % 4)*8);
    }
    UART_printf("\n\nGeTxBuffer Done\n\n");
    geTxBuffOffset = 0;
    memset(geTxBuff, 0, sizeof(geTxBuff));
    dumpingUart = false;
}
#endif // GE_RX_TX_BUFF_DEBUG




