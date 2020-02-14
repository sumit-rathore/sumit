#include <leon_uart.h>
#include "memset_test_log.h"
#include <ibase.h>

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint32 num = 0x12345678;

    //anounce we have started and give original vaue of num
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, STARTUP, num);
    LEON_UartWaitForTx();


    //set num to 0
    memset(&num, 0, sizeof(num));

    //display new value of num
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, MEMORY_SET, num);
    LEON_UartWaitForTx();


    //we are done but we cannot return so...
    while(1)
    ;

}

