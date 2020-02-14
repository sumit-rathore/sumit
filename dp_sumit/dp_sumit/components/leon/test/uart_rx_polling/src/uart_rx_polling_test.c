#include <interrupts.h> //Defined in the project file that knows the mappings of the interrupts
#include <leon_uart.h>
#include <leon_traps.h>

#define BUFFER_SIZE 1000

// Random string.  This was grabbed from the wikipedia page for a novel.
static char buffer[BUFFER_SIZE];

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint32_t i = 0;
    uint8_t rxByte = '\0'; //initialize it to anything other than '\n' which marks the end of a buffer


    // Configure the uart
    LEON_UartSetBaudRate115200();


    //read a string from the uart by polling until buffer is full or we get a line feed
    while((i < BUFFER_SIZE)
         &&(rxByte != '\n'))
    {
        if(LEON_UartRx(&rxByte) == true)
        {
            buffer[i] = rxByte;
            i++;
        }
    };

    //Write back the string read from the uart to guarantee it was received properly
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        LEON_UartByteTx(buffer[i]);
        if(buffer[i] == '\n')
        {
            break;
        }
    }
    if(buffer[i] != '\n')
    {
        LEON_UartByteTx('\n');
    }

    // Wait for everything to be written
    LEON_UartWaitForTx();

    // That's it, but there is no way to finish, so lets loop forever
    while (true)
        ;
}

