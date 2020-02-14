#sample makefile for ibuild

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        =  flash_icmds_main.c
ASMSOURCES      = 

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/flash_icmds_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE = flash_icmds_cmd.h

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = flash_icmds.elf

# define the type of target to build (lib, lib_collector, or elf)
TARGET_TYPE     = elf

# Files in the target directory not created by ibuild that should be cleaned up on a make clean
TARG_DIR_CLEANUP_FILES  =

# the names of the dependencies of this component
# For elf targets:
#   The components are passed to the linker in the order given here
#   The linker expects the high level components first, and the drivers last, so it only needs a single pass
#   If there are circular dependencies between the components, use FORCED_LINK_DEPS below
COMPONENT_DEPS  = $(if $(findstring DBLACKBIRD,$(CFLAGS)), sfi, ) main_loop timing ilog icmd leon ibase sfi

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS = 

# Files in the target directory not created by ibuild that should be cleaned up on a make clean
TARG_DIR_CLEANUP_FILES  = $(TARGET:.elf=).* icron_header


target: $(TARGET)
	$(OBJCOPY) -O binary $(targ_dir)/$(TARGET) $(targ_dir)/$(TARGET:.elf=.bin)
	echo -e "version:2\nproject:goldenears_test\nilog:ilog\nicmd:icmd\nicomponent:icomponent\nicmdresp:icmdresp\ntest_image:$(TARGET:.elf=.bin)" > $(targ_dir)/icron_header
	tar cfj  $(targ_dir)/$(TARGET:.elf=.icron) -C  $(targ_dir) icron_header $(TARGET:.elf=.bin) ilog icmd icmdresp icomponent

.PHONY: target

# Pull in the ibuild make rules
include $(IBUILD_RULES)

