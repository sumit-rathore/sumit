# makefile for linkmgr

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        =  linkmgr_init.c linkmgr_enet.c linkmgr_utils.c linkmgr_phy_mgr.c linkmgr_xusb_universal_lex.c linkmgr_xusb_universal_rex.c linkmgr_button_pairing.c
ASMSOURCES      = 

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/linkmgr_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE = linkmgr_cmd.h

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = liblinkmgr.a

# define the type of target to build (lib or elf)
TARGET_TYPE     = lib

# the names of the dependencies of this component
COMPONENT_DEPS  =  net vhub rexulm lexulm pll_xusb clm xcsr xlr flash_data storage grg ilog tasksch timing leon icmd ibase

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES =

# Pull in the ibuild make rules
include $(IBUILD_RULES)

