#makefile for xsst_rmw_masks_test

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        =  xsst_rmw_masks_test.c
ASMSOURCES      = 

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/xsst_rmw_masks_test_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
#COMPONENT_ICMD_FILE = <Component name goes here>_cmd.h

# Build a dependency graph
#IBUILD_DEP_GRAPH_PNG=<Component name goes here>.png
# Exclude the following components from the dependency graph (eg. ibase, ilog, & icmd, which everything uses)
#IBUILD_DEP_GRAPH_EXCLUDES=

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = xsst_rmw_masks_test.elf

# define the type of target to build (lib, lib_collector, or elf)
TARGET_TYPE     = elf
# Note: lib targets will use the dependencies to setup include paths
#       elf targets and lib_collectors will build all of their dependencies
# IMPORTANT: For elf targets there should be a mechanism to ensure the dependencies are not built simutaneously by parallel makes
#            This could happen if multiple elf file targets all use ibase with the same PROJECT name.
#            A solution would be to add a line here containing
#            PROJECT := $(PROJECT)_identifier
#            where identifier is unique to the target (probably $(TARGET) but without .elf at the end)
#            NOTE: THIS SOLUTION WOULD BREAK ILOG. See the flash_writer for a more complicated example of working around this

# Files in the target directory not created by ibuild that should be cleaned up on a make clean
TARG_DIR_CLEANUP_FILES  =

# the names of the dependencies of this component
# For elf targets:
#   The components are passed to the linker in the order given here
#   The linker expects the high level components first, and the drivers last, so it only needs a single pass
#   If there are circular dependencies between the components, use FORCED_LINK_DEPS below
COMPONENT_DEPS  = xcsr ilog leon spectareg ibase

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS = 

# Pull in the ibuild make rules
include $(IBUILD_RULES)


