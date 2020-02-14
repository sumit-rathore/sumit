#include <leon_uart.h>
#include "memcpy_chars_test_log.h"
#include <ibase.h>
static char myString[] = "A not very long string about something which contains 67 characters\r\n";

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    char buffer[sizeof(myString)];
    uint32_t i;

    //tell the world we are starting
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);

    //display original string
    for(i = 0; i < sizeof(myString)-1; i++)
    {
        LEON_UartByteTx(myString[i]);
        LEON_UartWaitForTx();
    }


    //copy the original string to our buffer
    memcpy(buffer, myString, sizeof(myString));

    //tell the world we are abuot to read out the contents of our buffer
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, MEMORY_COPIED);

    //display the contents of our buffer
    for(i = 0; i < sizeof(myString)-1; i++)
    {
        LEON_UartByteTx(buffer[i]);
        LEON_UartWaitForTx();
    }
    //end the line

    //we are done
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, FINISHED);
    LEON_UartWaitForTx();

    //we are done but we cannot return so...
    while(1)
    ;

}

