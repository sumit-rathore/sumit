#sample makefile for ibuild

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        = test_main.c test_default_isr.c
ASMSOURCES      = test_traps.S

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/test_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE = test_cmd.h

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = test.icron

# define the type of target to build (icron, lib, lib_collector, or elf)
TARGET_TYPE     = icron

# Files in the target directory not created by ibuild that should be cleaned up on a make clean
TARG_DIR_CLEANUP_FILES  =

# the names of the dependencies of this component
# For elf targets:
#   The components are passed to the linker in the order given here
#   The linker expects the high level components first, and the drivers last, so it only needs a single pass
#   If there are circular dependencies between the components, use FORCED_LINK_DEPS below
COMPONENT_DEPS  = tasksch timing ilog icmd leon ibase

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS =

HOBBES_DEFAULT_ICMD_BUTTON1=Status: iCmds['TEST_HARNESS_COMPONENT']['TEST_status']()

# Pull in the ibuild make rules
include $(IBUILD_RULES)

