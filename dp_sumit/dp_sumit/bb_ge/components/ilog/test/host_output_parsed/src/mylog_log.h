#include <project_components.h>
#include <ilog.h>


ILOG_CREATE(TEST_COMPONENT)
    ILOG_ENTRY(HOST_CONNECT, "Detected a host connect to virtual hub event\n")
    ILOG_ENTRY(HOST_DISCONNECT, "Detected a host disconnect to virtual hub event %d\n")
ILOG_END(TEST_COMPONENT, ILOG_MINOR_EVENT)


