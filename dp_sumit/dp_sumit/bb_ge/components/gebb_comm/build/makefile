PRE_BUILD_DEPS = register_definitions

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES = $(notdir $(wildcard ../src/*.c))
ASMSOURCES = $(notdir $(wildcard ../src/*.S))

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/ge_bb_comm_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE =

COMPONENT_ICMDRESP_FILE =

# Name of the target
TARGET          = libgebb_comm.a

# define the type of target to build (lib or elf)
TARGET_TYPE     = lib

# the names of the dependencies of this component
COMPONENT_DEPS  = crc linkmgr clm lexulm rexulm xcsr grg icmd ilog timing leon $(if $(findstring DBLACKBIRD,$(CFLAGS)), uart register_definitions, ) ibase

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES = 

include $(IBUILD_RULES)

