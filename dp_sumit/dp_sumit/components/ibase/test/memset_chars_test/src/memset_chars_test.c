#include <leon_uart.h>
#include "memset_chars_test_log.h"
#include <ibase.h>
static char myString[] = "A not very long string about something which contains 67 characters";

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint32_t i = 0;

    //announce we have started
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();

    //display original string
    for(i = 0; i < sizeof(myString)-1; i++)
    {
        LEON_UartByteTx(myString[i]);
        LEON_UartWaitForTx();
    }

    LEON_UartByteTx('\r');

    LEON_UartByteTx('\n');

    //set entire string to a's
    memset(myString, 'a', sizeof(myString)-1);
    LEON_UartWaitForTx();


    //anounce that the string has been set
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, MEMORY_SET);
    LEON_UartWaitForTx();

    //display changed string
    for(i = 0; i < sizeof(myString)-1; i++)
    {
        LEON_UartByteTx(myString[i]);
        LEON_UartWaitForTx();
    }
    LEON_UartByteTx('\r');
    //it needs a line end
    LEON_UartByteTx('\n');
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, FINISHED);

    LEON_UartWaitForTx();
    //we are done but we cannot return so...
    while(1)
    ;

}

