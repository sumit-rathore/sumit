#include <leon_uart.h>
#include "memcpy_test_log.h"
#include <ibase.h>

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint32 src = 0x12345678;
    uint32 dest;

    //anounce we have started and give the vaue of src
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, STARTUP, src);
    LEON_UartWaitForTx();


    //copy the value at src to dest
    memcpy(&dest, &src, sizeof(src));

    //display the value of dest
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, MEMORY_SET, dest);
    LEON_UartWaitForTx();


    //we are done but we cannot return so...
    while(1)
    ;

}

