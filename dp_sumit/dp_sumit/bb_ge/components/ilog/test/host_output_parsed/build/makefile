#sample makefile for ibuild

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        =  dummy.c ilog_parser.c
ASMSOURCES      = 

COMPONENT_ILOG_FILE = ../src/mylog_log.h

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = ilog_test_parser

# define the type of target to build (lib, lib_collector, or elf)
TARGET_TYPE     = elf

# the names of the dependencies of this component
COMPONENT_DEPS  = ibase ilog

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS = 

# If this is a project component uncomment the following
include project.mak

# Pull in the ibuild make rules
include $(IBUILD_RULES)

