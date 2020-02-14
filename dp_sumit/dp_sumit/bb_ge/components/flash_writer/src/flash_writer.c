///////////////////////////////////////////////////////////////////////////////
//
//   Icron Technology Corporation - Copyright 2010
//
//
//   This source file and the information contained in it are confidential and
//   proprietary to Icron Technology Corporation. The reproduction or disclosure, in whole
//   or in part, to anyone outside of Icron without the written approval of a
//   Icron  officer under a Non-Disclosure Agreement, or to any employee of Icron
//   who has not previously obtained written authorization for access from the
//   individual responsible for the source code, will have a significant detrimental
//   effect on Icron  and is expressly prohibited.
//
///////////////////////////////////////////////////////////////////////////////
//
//   Title       :  flash_writer.c
//
//   Description :  The flash writer
//
//   NOTES:
//
///////////////////////////////////////////////////////////////////////////////

/***************************** Included Headers ******************************/
#include <options.h>
#include <ibase.h>
#include <leon_uart.h>
#include <interrupts.h>
#include <leon_timers.h>
#include <xmodem.h>
#include <flash_raw.h>
#include "flash_log.h"
#include <stdarg.h>
#include <grg.h>
#include "command.h"
#include "command_loc.h"
#include <crc.h>

/************************ Defined Constants and Macros ***********************/

/******************************** Data Types *********************************/

/***************************** Local Variables *******************************/

/************************ Local Function Prototypes **************************/
static inline void Flash_EraseCodeBlock(void);
static inline void Flash_EraseFPGAImageAndSw(void);
static inline void Flash_EraseAll(void);

static inline boolT Flash_VerifyErase(void);
static inline boolT verifyFlash(uint32 flashAddr, uint32 * buf, uint32 bufSize);

//static uint32 * incomingDataHandler(uint32 * buf, uint32 bufSize);

//static void  flashWriterPrintf(const char* format, ...);
#if defined(GOLDENEARS) && defined(RECOVERY)
static void _BlockProtectRecoveryImage(void);
#endif


/************************** Function Definitions *****************************/

/**
* FUNCTION NAME: UART_Polling()
*
* @brief  - Erases the code block of flash
*
* @return - void
*
* @note   - GE only
*
*/
void Flash_uartPolling(void) __attribute__  ((noreturn));
void Flash_uartPolling(void)
{
    // Poll on UART RX, if we receive a byte, call the RX ISR
    // This should then trigger the packetize stuff
    // We should call the process to handle the fifo too
    // We don't have interrupts so copying from the RX FIFO into our SW fifo isn't happening
    uint32_t timeout = 1000000; // usec
    LEON_TimerValueT startTime = LEON_TimerRead();
    while(1)
    {
        LEON_UartInterruptHandlerRx();
        UART_ProcessRx(); // run our packetizeDecodeRxData
        LEON_UartInterruptHandlerRx();
        CMD_timerHandler(); // check for timeouts
        if (!CMD_programEraseReceived())
        {
            if (LEON_TimerCalcUsecDiff(startTime, LEON_TimerRead()) > timeout)
            {
                CMD_sendSoftwareVersion();
                startTime = LEON_TimerRead();
            }
        }
        //LEON_UartInterruptHandlerTx();
    }
}

/**
* FUNCTION NAME: Flash_EraseCodeBlock()
*
* @brief  - Erases the code block of flash
*
* @return - void
*
* @note   - GE only
*
*/
static inline void Flash_EraseCodeBlock(void)
{
    flashWriterPrintf(ERASE_CODE_BLOCK);
    FLASHRAW_eraseCode();
    flashWriterPrintf(ERASE_COMPLETE);
}

/**
* FUNCTION NAME: Flash_EraseFPGAImageAndSw()
*
* @brief  - Erases the FPGA Image and SW image and data segments
*
* @return - void
*
* @note   -
*
*/
static inline void Flash_EraseFPGAImageAndSw(void)
{
    flashWriterPrintf(ERASE);
    FLASHRAW_eraseGeneric(FLASHWRITER_BASE_ADDRESS, FLASHWRITER_FPGA_SW_SIZE);
    flashWriterPrintf(ERASE_COMPLETE);
}


/**
* FUNCTION NAME: Flash_EraseAll()
*
* @brief  - Erases the entire flash
*
* @return - void
*
* @note   - 
*
*/
static inline void Flash_EraseAll(void)
{
    flashWriterPrintf(ERASE);
    FLASHRAW_eraseChip();
    flashWriterPrintf(ERASE_COMPLETE);
}


/**
* FUNCTION NAME: Flash_VerifyErase()
*
* @brief  - Verifies the erasure of flash
*
* @return - TRUE on success, FALSE on failure
*
* @note   -
*
*/
static inline boolT Flash_VerifyErase(void)
{
    uint32 *flashPtr;
    uint32 flashVal;
    boolT ret = TRUE;

    // Now we will start verifying our erase of flash
    flashWriterPrintf(ERASE_CHECK);

    for( flashPtr = (uint32 *)FLASHWRITER_BASE_ADDRESS;
         flashPtr < (uint32 *)(FLASHWRITER_BASE_ADDRESS + FLASHWRITER_ERASE_CHECK_SIZE);
         flashPtr++)
    {
        flashVal = *flashPtr;

        if(0xFFFFFFFF != flashVal) // Blank flash is always all binary ones
        {
            //the value read from flash is unexpected - log an error
            flashWriterPrintf(ERASE_CHECK_FAIL, (uint32)flashPtr, flashVal);
            ret = FALSE;
        }
    }
    flashWriterPrintf(ERASE_CHECK_DONE);

    return ret;
}



/**
* FUNCTION NAME: verifyFlash()
*
* @brief  - Verify a block of flash was programmed correctly
*
* @return - TRUE on success. FALSE on failure
*
* @note   -
*
*/
static inline boolT verifyFlash
(
    uint32 flashAddr,           // The flash address to verify
    uint32 * buf,               // The data buffer to verify against
    uint32 bufSize              // The size of the verification data buffer
)
{
    uint32 * pFlash = (uint32 *)(flashAddr);
    boolT ret = TRUE;

    // Verify flash contents
    while (bufSize && ret)
    {
        // Verify data
        ret = (*pFlash == *buf); // Endian assumption isn't an issue, as this is
                                 // on the same machine as the receiving XModem library

        if(!ret)
        {
            //well, the current value was wrong print it out
            flashWriterPrintf(PROGRAM_CHECK_ERROR, *buf, *pFlash, CAST(pFlash, uint32 *, uint32));
        }

        // update iterative variables
        bufSize -= 4;
        buf++;
        pFlash++;
    }

    return ret;
}

#if 0
/**
* FUNCTION NAME: incomingDataHandler()
*
* @brief  - Handles incoming data from the xmodem transfer
*
* @return - the next buffer for the X-Modem library, actually just recycles
*           this buffer, or returns NULL on a fatal error
*
* @note   -
*
*/
static uint32 * incomingDataHandler
(
    uint32 * buf,       // A buffer of incoming data
    uint32 bufSize      // The size of the buffer
)
{
    static uint32 curFlashAddress = FLASHWRITER_BASE_ADDRESS;

    //we received a whole buffer of data, write it to flash
    FLASHRAW_write((uint8 *)curFlashAddress, (uint8 *)buf, bufSize);

    //make sure what was written to flash is what we wanted to write
    if (verifyFlash(curFlashAddress, buf, bufSize))
    {
        // Update our current address state variable
        curFlashAddress += bufSize;
        return buf; // We're done this buffer, so it can be recycled for the next buffer
    }
    else
    {
        return NULL;
    }
}
#endif

/**
* FUNCTION NAME: flashWriterPrintf()
*
* @brief  - A very minimal printf that writes to the UART.  Only %x and %s
* formatting is supported and no widths or padding is supported.  Literal %
* chars can be output using "%%".
*
* @return - void.
*/
void flashWriterPrintf(const char* format, ...)
{
    const char* cursor = format;
    va_list vl;
    va_start(vl, format);

    uint8_t msg[256];
    uint8_t msgIdx = 0;

    while(*cursor != 0)
    {
        if(*cursor == '%')
        {
            cursor++;
            switch(*cursor)
            {
                case '%':
                    //LEON_UartByteTx('%');
                    msg[msgIdx++] = '%';
                    break;

                case 'x':
                    {
                        uint32 val = va_arg(vl, uint32);
                        const char* hexChars = "0123456789ABCDEF";
                        for (sint8 nibble = 7; nibble >= 0; nibble--)
                        {
                            const uint8 nibbleVal = (val >> 28);
                            //LEON_UartByteTx(hexChars[nibbleVal]);
                            msg[msgIdx++] = hexChars[nibbleVal];
                            val <<= 4;
                        }
                    }
                    break;

                case 's':
                    {
                        const char* val = va_arg(vl, const char*);
                        while(*val != 0)
                        {
                            //LEON_UartByteTx(*val);
                            msg[msgIdx++] = *val;
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
                    //LEON_UartByteTx('\r'); // print a CR before a LF
                    msg[msgIdx++] = '\r';
                }
                //LEON_UartByteTx(*cursor);
                msg[msgIdx++] = *cursor;
                cursor++;
            }
        }
    }

    va_end(vl);
    UART_packetizeSendDataImmediate(
        CLIENT_ID_GE_PRINTF,
        NULL,
        msg,
        msgIdx);


    //LEON_UartWaitForTx();
}


/**
* FUNCTION NAME: _BlockProtectRecoveryImage()
*
* @brief  - Recovery image is between address 0 to (0x200000 - 1)
*
* @return - never
*
* @note   - Based on Page 20 of winbond W25Q32FV datasheet (Rev D):
*           - To protect 0h to 1FFFFFh, need to write the following registers:
*               - WPS = 0
*               - CMP = 0
*               - SEC = 0
*               - TB  = 1
*               - BP2 = 1
*               - BP1 = 1
*               - BP0 = 0
*
*           Leave Quad Enable (QE) bit set because it can be used later to program
*           FPGA in QPI mode.
*
*/
#if defined(GOLDENEARS) && defined(RECOVERY)
static void _BlockProtectRecoveryImage(void)
{
    uint8 statusReg1 = 0;
    uint8 statusReg2 = 0;
    uint8 statusReg3 = 0;

    statusReg1 |= (1 << 3); // BP1
    statusReg1 |= (1 << 4); // BP2
    statusReg1 |= (1 << 5); // TB

    FLASHRAW_writeStatusRegister(1, statusReg1);

    statusReg2 |= (1 << 1); // QE

    FLASHRAW_writeStatusRegister(2, statusReg2);

    FLASHRAW_writeStatusRegister(3, statusReg3);
}
#endif


/**
* FUNCTION NAME: imain()
*
* @brief  - Entry point for the flash writer.
*
* @return -
*
* @note   -
*
*/
void imain(void) __attribute__  ((noreturn));
void imain(void)
{
#if 0
    uint32 buf1[XMODEM_BLOCK_SIZE_32BIT_WORDS];
    uint8 major;
    uint8 minor;
    uint8 debug;
    uint8 data = 0;
#endif

    // Lock out interrupts for the initialization code
    LEON_LockIrq();
    {
        // Initialize uart
        LEON_UartSetBaudRate(115200);
        //LEON_UartSetBaudRate115200(); - this calls bootrom version!!!
        LEON_EnableIrq(IRQ_UART_RX);

        // Initialize the timers
        LEON_TimerInit();
        LEON_EnableIrq(IRQ_TIMER2);

        // Basic XModem configuration
        // XMODEM_InterruptModeInit();
    }

    // We are ready to receive
    // TODO packetize flashWriterPrintf

    FLASHRAW_setCallback(&LEON_UartInterruptHandlerRx);

    UART_packetizeInit();

    // Setup our handlers for Command
    CMD_Init();

    crcInit();

    CMD_sendSoftwareVersion();

    // We need a while somewhere that polls for UART messages
    Flash_uartPolling();

#if 0
#if defined(GOLDENEARS) && defined(RECOVERY)
    // Erase Entire Chip
    Flash_EraseAll();
#elif defined(GOLDENEARS) && !defined(ERASE_ALL)
    // Erase software image
    Flash_EraseCodeBlock();
#elif defined(GOLDENEARS)
    // Erase FPGA main image and software image
    Flash_EraseFPGAImageAndSw();
#else
    // For LG1, just erase the whole chip
    Flash_EraseAll();
#endif
    if (!Flash_VerifyErase())
    {
        // log failure and abort
        flashWriterPrintf(ERASE_FAILED_ABORT);
    }
    else if (!XMODEM_PolledModeReceive(&incomingDataHandler, buf1)) // XModem transfer
    {
        // Transfer failed
        flashWriterPrintf(FAILED);
    }
    else
    {
#if defined(GOLDENEARS) && defined(RECOVERY)
        // Block-protect Recovery Image
        _BlockProtectRecoveryImage();

#endif
        // Signal to the downloader that we are done
        flashWriterPrintf(PROG_COMP_WAIT_FOR_USER_RESET);
    }

    // If the FPGA revision is at least 0.6.a, the user may
    // reset the device by sending CR-LF (actually just LF)
    GRG_GetChipRev(&major, &minor, &debug);
    if ((major > 0) ||
        (major == 0 && minor > 6) ||
        (minor == 6 && debug >= 0xA))
    {
        flashWriterPrintf(PROG_COMP_SEND_CR_LF_TO_RESET);
        LEON_UartWaitForTx();
        do {
            LEON_UartRx(&data);
        } while (data != '\n');
        GRG_ResetChip();
    }
    else
    {
        LEON_UartWaitForTx(); //blocks until entire buffer is transmitted

        // Loop forever, waiting for the user to reset the chip
        while (1)
            ;
    }
#endif

}

