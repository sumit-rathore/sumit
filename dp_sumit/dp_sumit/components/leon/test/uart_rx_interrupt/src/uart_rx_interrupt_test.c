#include <interrupts.h> //Defined in the project file that knows the mappings of the interrupts
#include <leon_uart.h>
#include <leon_traps.h>

#define BUFFER_SIZE 1000

// Random string.  This was grabbed from the wikipedia page for a novel.
static char buffer[BUFFER_SIZE];
static void RxHandler(uint8_t rxByte);

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{


    // Configure the uart
    LEON_UartSetBaudRate115200();
    LEON_InstallIrqHandler(IRQ_UART_TX, LEON_UartInterruptHandlerTx);
    LEON_InstallIrqHandler(IRQ_UART_RX, LEON_UartInterruptHandlerRx);

    //install the rx hadnler
    LEON_UartSetRxHandler(RxHandler);
    LEON_EnableIrq(IRQ_UART_RX);
    LEON_EnableIrq(IRQ_UART_TX);

    //wait
    while(true)
        ;
}

//handles the recieved bytes and sends them back
static void RxHandler(uint8_t rxByte)
{
    static uint32_t i = 0;
    static bool written = false;

    if(written != true)
    {
        //we have not already written our buffer back to the serial
        buffer[i] = rxByte;
        if((i >= BUFFER_SIZE -1)
            ||(buffer[i] == '\n'))
            {
                uint32_t index = 0;

                //our buffer is full or we are at the end of the line, write the buffer out to the uart
                for(index = 0; index <= i; index++)
                {
                    LEON_UartByteTx(buffer[index]);
                }
                if(buffer[i] != '\n')
                {
                    //add a line end, if the last character wasn't one
                    LEON_UartByteTx('\n');
                }

                //we have now written out our recieved characters, we do not want to write them again
                written = true;
            }
        i++;
    }
}

