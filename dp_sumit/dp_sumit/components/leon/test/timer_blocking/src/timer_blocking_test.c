#include <interrupts.h>
#include <leon_uart.h>
#include "timer_blocking_test_log.h"
#include <leon_timers.h>
#include <leon_traps.h>

#define USECONDS_TO_BLOCK 5000000

#define GET_SECONDS(x) x/1000000;

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint8_t seconds  = GET_SECONDS(USECONDS_TO_BLOCK);


     // Configure the uart
    LEON_UartSetBaudRate115200();

    //init timers
    LEON_TimerInit();

    //send message indicating how long we will wait for
    ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, STARTUP, seconds);
    LEON_UartWaitForTx();

    //wait for specified time period
    LEON_TimerWaitMicroSec(USECONDS_TO_BLOCK);

    //send message indicating we are done waiting
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, FINISHED);
    LEON_UartWaitForTx();


    // That's it, but there is no way to finish, so lets loop forever
    while (true)
        ;
}

