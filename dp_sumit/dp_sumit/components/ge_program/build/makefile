# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES = $(notdir $(wildcard ../src/*.c))
ASMSOURCES = $(notdir $(wildcard ../src/*.S))

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/ge_program_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE =ge_program_icmd.h

COMPONENT_ICMDRESP_FILE = #linkmgr_cmdresp.h

# This will prevent any source code being compiled for main FW
# Main FW does not use this module but the icron file requires the ilogs
# This hack will allow the build system to create the ilogs for Main FW
# which will be included in the Icron file.
# While main FW won't use the ilogs, programBB.bin will and we don't want
# a separate Icron file for programBB.bin
ifneq ("$(PROJECT)", "programBB")
  CSOURCES =
  ASMSOURCES =
  COMPONENT_ICMD_FILE =
  COMPONENT_ICMDRESP_FILE =
endif

# Name of the target
TARGET          = libge_program.a

# define the type of target to build (lib or elf)
TARGET_TYPE     = lib

# the names of the dependencies of this component
COMPONENT_DEPS  = configuration command crc linkmgr bbge_comm led uart cpu_comm top core util gpio icmd ilog timing leon register_definitions ibase

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES =

include $(IBUILD_RULES)
