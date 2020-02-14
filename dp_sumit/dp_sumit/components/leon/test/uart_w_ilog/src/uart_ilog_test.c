#include <leon_uart.h>
#include "uart_ilog_test_log.h"

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint32_t i;

    // Configure the uart
    LEON_UartSetBaudRate115200();

    // Tell the world that we have started
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();

    // Hammer away some log messages
    for (i = 0; i < 1024; i++)
    {
        ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, ITERATION, i);


    }

    // Tell the world that we are finished
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, FINISHED);
    LEON_UartWaitForTx();

    // That's it, but there is no way to finish, so lets loop forever
    while (true)
        ;
}

