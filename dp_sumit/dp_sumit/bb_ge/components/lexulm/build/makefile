# makefile for lexulm

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        = lex.c lexulm_utils.c
ASMSOURCES      =

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/lex_log.h

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = liblexulm.a

# define the type of target to build (lib or elf)
TARGET_TYPE     = lib

# the names of the dependencies of this component
# might be better to extract vhub support from CFLAGS, as that is what all the C code does already
COMPONENT_DEPS  = gebb_comm devmgr ilog icmd ibase topology xcsr grg ulm clm leon timing storage
COMPONENT_DEPS += $(if $(findstring GOLDENEARS, $(CFLAGS)), xlr flash_data)
COMPONENT_DEPS += $(if $(findstring LG1USB3_3_DUAL_HUB_DUAL_REDRV, $(CFLAGS)), tasksch)
COMPONENT_DEPS += $(if $(findstring LG1USB3_4_VISION_2012_DEMO, $(CFLAGS)), tasksch)

# If this is a project component uncomment the following
#include project.mak

# Pull in the ibuild make rules
include $(IBUILD_RULES)

