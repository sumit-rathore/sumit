#include <interrupts.h>
#include <leon_uart.h>
#include "time_difference_test_log.h"
#include <leon_timers.h>
#include <leon_traps.h>

#define USECONDS_TO_BLOCK 5000000

#define GET_SECONDS(x) x/1000000;

void imain(void) __attribute__ ((noreturn, section(".ftext")));
void imain(void)
{
    uint8 byte = '\0'; //initialize it to anything other than '\n' which marks the end of a buffer
    LEON_TimerValueT startTime;
    LEON_TimerValueT endTime;
    uint32 timeDifference;

     // Configure the uart
    LEON_UartSetBaudRate115200();

    //init the timers
    LEON_TimerInit();

    //print out start message
    ilog_TEST_HARNESS_COMPONENT_0(ILOG_MAJOR_EVENT, STARTUP);
    LEON_UartWaitForTx();

    //wait for recieved bytes
    do {
        LEON_UartRx(&byte);
    } while(byte != '\n');

       //we have reached the end of the recieved line, get the time
    startTime = LEON_TimerRead();

    //wait for next bytes
    while(LEON_UartRx(&byte) == FALSE)
    ;
    //the next bytes have been recieved, get the new time
    endTime = LEON_TimerRead();

    //calculate the time difference and output it
    timeDifference = LEON_TimerCalcUsecDiff(startTime, endTime);

     ilog_TEST_HARNESS_COMPONENT_1(ILOG_MAJOR_EVENT, FINISHED, timeDifference);
    LEON_UartWaitForTx();

    // That's it, but there is no way to finish, so lets loop forever
    while (TRUE)
        ;
}

