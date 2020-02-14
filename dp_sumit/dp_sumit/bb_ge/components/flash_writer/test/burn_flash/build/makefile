#sample makefile for ibuild

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        =  imain.c flash_functions.c
ASMSOURCES      = 

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/burn_flash_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE = burn_flash_cmd.h

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = burnflash.icron

# define the type of target to build (lib, lib_collector, or elf)
TARGET_TYPE     = icron

# Files in the target directory not created by ibuild that should be cleaned up on a make clean
TARG_DIR_CLEANUP_FILES  =

# ensure that this icmd is included in the test harness
NO_ICMD_SUPPORT=
unexport NO_ICMD_SUPPORT

# the names of the dependencies of this component
# For elf targets:
#   The components are passed to the linker in the order given here
#   The linker expects the high level components first, and the drivers last, so it only needs a single pass
#   If there are circular dependencies between the components, use FORCED_LINK_DEPS below
COMPONENT_DEPS  = flash_data icmd ilog timing leon ibase

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS = 


HOBBES_DEFAULT_ICMD_BUTTON1=Start: iCmds['TEST_HARNESS_COMPONENT']['startTests']()
HOBBES_DEFAULT_ICMD_BUTTON2=Stop: iCmds['TEST_HARNESS_COMPONENT']['stopTests']()
HOBBES_DEFAULT_ICMD_BUTTON3=Stats: iCmds['TEST_HARNESS_COMPONENT']['getStats']()
HOBBES_DEFAULT_ICMD_BUTTON4=Address Test: iCmds['TEST_HARNESS_COMPONENT']['doAddressTest']()

# Pull in the ibuild make rules
include $(IBUILD_RULES)

