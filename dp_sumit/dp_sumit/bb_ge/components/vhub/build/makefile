#sample makefile for ibuild

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        = vhub_link_messaging.c vhub_usb_messaging.c vhub_port_manager.c vhub_state.c
ASMSOURCES      = 

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/vhub_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE = vhub_cmd.h

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = libvhub.a

# define the type of target to build (lib, lib_collector, or elf)
TARGET_TYPE     = lib

# the names of the dependencies of this component
COMPONENT_DEPS  = lexulm devmgr topology flash_data storage xcsr ulm timing ilog icmd leon ibase
# Add in the xlr for golden ears only, as it doesn't exist on LG1
COMPONENT_DEPS += $(if $(findstring GOLDENEARS, $(CFLAGS)), xlr)

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS =

TEST_HARNESSES = 

# Pull in the ibuild make rules
include $(IBUILD_RULES)

