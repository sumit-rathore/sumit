# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES = $(notdir $(wildcard ../src/*.c))
ASMSOURCES = $(notdir $(wildcard ../src/*.S))

#Specify the name of the ilog file for this component with the path
#NOTE:  We are too low level to support ilog.  ilog depends on us, not the
#       other way around
COMPONENT_ICMD_FILE = rs232_cmd.h
COMPONENT_ILOG_FILE = ../src/rs232_log.h

# Name of the target
TARGET          = librs232.a

# define the type of target to build (lib or elf)
TARGET_TYPE     = lib

# the names of the dependencies of this component
COMPONENT_DEPS  = configuration cpu_comm event mca gpio icmd ilog top core leon ibase register_definitions

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES =

include $(IBUILD_RULES)

