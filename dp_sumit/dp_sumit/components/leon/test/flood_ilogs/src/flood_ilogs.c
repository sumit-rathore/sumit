#include <interrupts.h>
#include <leon_uart.h>
#include "flood_ilogs_log.h"
#include <leon_traps.h>

#define FLOOD_COUNT 10000
#define FLOOD_COUNT_CONTROLLED 100


void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint32_t count;

     // Configure the uart
    LEON_UartSetBaudRate115200();

    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, START_FLOOD);
    LEON_UartWaitForTx();

    for (count = 0; count < FLOOD_COUNT; count++)
    {
        ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, FLOOD, count);
    }


    LEON_UartWaitForTx();
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, START_CONTROLLED_FLOOD);
    LEON_UartWaitForTx();

    for (count = 0; count < FLOOD_COUNT_CONTROLLED; count++)
    {
        ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, FLOOD_CONTROLLED, count);
        LEON_UartWaitForTx();
    }

    LEON_UartWaitForTx();
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, END);
    LEON_UartWaitForTx();

    // That's it, but there is no way to finish, so lets loop forever
    while (true)
        ;
}

