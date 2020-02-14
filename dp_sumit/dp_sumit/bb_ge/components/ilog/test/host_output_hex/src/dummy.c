#include "mylog_log.h"

#include <stdio.h>

int main()
{
    ilog_TEST_COMPONENT_0(ILOG_DEBUG, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_DEBUG, HOST_DISCONNECT, 16);

    ilog_TEST_COMPONENT_0(ILOG_MINOR_EVENT, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_MINOR_EVENT, HOST_DISCONNECT, 17);

    ilog_TEST_COMPONENT_0(ILOG_MAJOR_EVENT, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_MAJOR_EVENT, HOST_DISCONNECT, 18);

    ilog_TEST_COMPONENT_0(ILOG_WARNING, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_WARNING, HOST_DISCONNECT, 19);

    ilog_TEST_COMPONENT_0(ILOG_MINOR_ERROR, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_MINOR_ERROR, HOST_DISCONNECT, 20);

    ilog_TEST_COMPONENT_0(ILOG_MAJOR_ERROR, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_MAJOR_ERROR, HOST_DISCONNECT, 21);

    ilog_TEST_COMPONENT_0(ILOG_FATAL_ERROR, HOST_CONNECT);
    ilog_TEST_COMPONENT_1(ILOG_FATAL_ERROR, HOST_DISCONNECT, 22);

    return 0;
}

boolT ilog_TestHarnessAtomicPrint(uint8 *msg, uint8 bytes)
{
    uint8 i;

    printf(" HEAD MODL CODE LEVL ARG1_xxxx_xxxx_xxxx ARG2_xxxx_xxxx_xxxx ARG3_xxxx_xxxx_xxxx\n");

    for (i = 0; i < bytes; i++)
    {
        printf(" 0x%02x", msg[i]);
    }

    printf("\n");

    return 1;
}

void ilog_TestHarnessWaitForTx(void)
{
}

