#include <leon_flash.h>
#include "flash_cmds.h"
#include "flash_test_log.h"
#include <leon_uart.h>
#include <interrupts.h>

#define MY_DEBUG 0x80000010

#define WORD 0xDEADBEEF
#define ITERATIONS 4000
#define WORDS_AT_TIME 8

#ifdef GOLDENEARS
static inline void Flash_EraseMainMemory()
{
    //enable write
    LEON_SFISendInstruction(SFI_WREN);

    // send Chip Erase Inst
    LEON_SFISendInstruction(SFI_CHIP_ERASE_S);


    // wait for WIP (write in progress) bit to clear
    while(LEON_SFISendReadStatus(SFI_RDSR, LEON_FLASH_4_BYTE_DATA)& SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK)
        ;

}

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint32 i;
    uint32 numWritten;
    uint32 buff[] = {WORD, WORD, WORD, WORD, WORD, WORD, WORD, WORD};
    uint32 flashAddr = SERIAL_FLASH_BASE_ADDR;

    // Configure the uart

    // Initialize uart to 115200 baud rate
    LEON_UartSetBaudRate115200();
    LEON_InstallIrqHandler(IRQ_UART_RX, LEON_UartInterruptHandlerRx);
    LEON_EnableIrq(IRQ_UART_RX);
    LEON_InstallIrqHandler(IRQ_UART_TX, LEON_UartInterruptHandlerTx);
    LEON_EnableIrq(IRQ_UART_TX);



    //anounce to world that flash test is starting
    ilog_TEST_HARNESS_COMPONENT_2(ILOG_MAJOR_EVENT, STARTUP, ITERATIONS, WORDS_AT_TIME);

    Flash_EraseMainMemory();

    for(i = 0; i < ITERATIONS; i++)
    {

        ///enable for writing
        LEON_SFISendInstruction(SFI_WREN);

        //program a page
        numWritten = LEON_SFISendWrite(SFI_PAGE_PROG, flashAddr, (uint8 *)buff, WORDS_AT_TIME);

        //recalculate the bufSize
        flashAddr += numWritten;


        //wait for data to be written to flash
        while(LEON_SFISendReadStatus(SFI_RDSR, LEON_FLASH_4_BYTE_DATA) & SFI_CONTROL_TRANSFER_IN_PROG_BIT_MASK)
            ;
    }

    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, FINISHED);
    // That's it, but there is no way to finish, so lets loop forever
    while (TRUE)
        ;
}
#else
void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    // Initialize uart to 115200 baud rate
    LEON_UartSetBaudRate115200();

    // Announce that this is a bad setup
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_FATAL_ERROR, BAD_TEST);
    LEON_UartWaitForTx();

    // That's it, but there is no way to finish, so lets loop forever
    while (TRUE)
        ;
}
#endif
