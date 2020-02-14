# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        = init.c lex.c testmode.c stats.c xlr_msa.c
ASMSOURCES      =

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/xlr_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE = xlr_cmd.h

# Name of the target
TARGET          = libxlr.a

# define the type of target to build (lib or elf)
TARGET_TYPE     = lib

# the names of the dependencies of this component
COMPONENT_DEPS  = icmd ilog spectareg timing leon ibase

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES =

include $(IBUILD_RULES)

