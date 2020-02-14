# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES = $(notdir $(wildcard ../src/*.c))
ASMSOURCES = $(notdir $(wildcard ../src/*.S))

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/random_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE = random_cmd.h

COMPONENT_ICMDRESP_FILE =

TARGET          = librandom.a

# define the type of target to build (lib, lib_collector, or elf)
TARGET_TYPE     = lib


# the names of the dependencies of this component
COMPONENT_DEPS  = atmel_crypto configuration xadc timing icmd ilog sha2 ibase

FORCED_LINK_DEPS =

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES =

include $(IBUILD_RULES)

