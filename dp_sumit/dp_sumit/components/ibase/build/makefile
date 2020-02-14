#makefile for ibase

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES = $(notdir $(wildcard ../src/*.c))
ASMSOURCES = $(notdir $(wildcard ../src/*.S))

# This is the lowest level component, so we can't depend on ilog
COMPONENT_ILOG_FILE = 

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = libibase.a

# define the type of target to build (lib, lib_collector, or elf)
TARGET_TYPE     = lib

# Files in the target directory not created by ibuild that should be cleaned up on a make clean
TARG_DIR_CLEANUP_FILES  =

# the names of the dependencies of this component
# For elf targets:
#   The components are passed to the linker in the order given here
#   The linker expects the high level components first, and the drivers last, so it only needs a single pass
#   If there are circular dependencies between the components, use FORCED_LINK_DEPS below
# This is the lowest level component so we don't depend on anything
COMPONENT_DEPS  = 

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS = 

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES = memset_test memset_chars_test memcpy_chars_test memcpy_test

# If this is a project component uncomment the following
#include project.mak

# Pull in the ibuild make rules
include $(IBUILD_RULES)


