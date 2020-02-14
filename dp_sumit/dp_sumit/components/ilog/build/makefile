#sample makefile for ibuild
PRE_BUILD_DEPS = register_definitions

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES = $(notdir $(wildcard ../src/*.c))
ASMSOURCES = $(notdir $(wildcard ../src/*.S))

COMPONENT_ILOG_FILE = ../src/ilog_log.h

COMPONENT_ICMD_FILE = ilog_cmd.h

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = libilog.a

# define the type of target to build (lib, lib_collector, or elf)
TARGET_TYPE     = lib

# the names of the dependencies of this component
COMPONENT_DEPS  = icmd ibase

TEST_HARNESSES  = ilog_flood

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS = 

# If this is a project component uncomment the following
#include project.mak

# Pull in the backend details
# Check for test harness first, so this could over ride any other setting
ifeq ($(ILOG_BACKEND), TEST_HARNESS)
CFLAGS+=-DILOG_UNDER_TEST_HARNESS
else
ifeq ($(ILOG_BACKEND), LEON)
CFLAGS += -DILOG_USE_LEON
COMPONENT_DEPS += leon
endif
endif
COMPONENT_DEPS += $(if $(findstring D__MSP430__,$(CFLAGS)), msp430x2xxx) $(if $(findstring DBLACKBIRD,$(CFLAGS)), uart, )

# Pull in the ibuild make rules
include $(IBUILD_RULES)

# setup rule which won't overwrite rules above, just adding another rule
# this rule says when creating dependencies required by CSOURCES also create
# the istatus.h file by making it a prerequisit

pre_build: ../inc/istatus.h

.PHONY: pre_build

clean:
	rm -rf ../inc/istatus.h

../inc/istatus.h: ../../../fg_icron_files/istatus
	$(shell python generate_istatus_h.py)

