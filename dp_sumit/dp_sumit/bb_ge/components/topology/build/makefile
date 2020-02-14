# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        = topology.c xsstmon.c xsstmon_ge.c topology_icmds.c device.c parsing.c virtual_function.c xusb_addr.c msa.c
ASMSOURCES      =

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/topology_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE = topology_cmd.h

# Name of the target
TARGET          = libtopology.a

# define the type of target to build (lib or elf)
TARGET_TYPE     = lib

# the names of the dependencies of this component
COMPONENT_DEPS  = xcsr flash_data storage xlr grg icmd ilog timing tasksch spectareg leon ibase

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
# The virtual function test harness is a GE only test harness
# TODO: temporarily commenting out the virtual_function test harness, as it is too bit to link.  Out of IRAM
# TEST_HARNESSES = $(if $(findstring GOLDENEARS, $(CFLAGS)), virtual_function)
TEST_HARNESSES =

include $(IBUILD_RULES)

