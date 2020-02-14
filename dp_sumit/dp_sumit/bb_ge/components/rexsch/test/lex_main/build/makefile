#sample makefile for ibuild

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        = lex_main.c fake_rex.c enum.c fake_lex.c
ASMSOURCES      =

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/test_log.h

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = lex_main.elf

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
TARG_DIR_CLEANUP_FILES  = lex_main.s19 lex_main.imemh

# the names of the dependencies of this component
# For elf targets:
#   The components are passed to the linker in the order given here
#   The linker expects the high level components first, and the drivers last, so it only needs a single pass
#   If there are circular dependencies between the components, use FORCED_LINK_DEPS below
COMPONENT_DEPS  =  linkmgr net vhub devmgr topology pll_xusb rexsch xcsr xlr xrr clm grg ulm flash_data atmel_crypto tasksch timing ilog spectareg leon crc sha2 ibase

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS =

IRAM_BASE=0x60000000

target: $(TARGET)
	$(OBJCOPY) -O srec $(targ_dir)/$(TARGET)  $(targ_dir)/$(TARGET:.elf=.s19)
	srec_cat  $(targ_dir)/$(TARGET:.elf=.s19) -offset -$(IRAM_BASE) -o  $(targ_dir)/$(TARGET:.elf=.imemh) -vmem 8

# Pull in the ibuild make rules
include $(IBUILD_RULES)

