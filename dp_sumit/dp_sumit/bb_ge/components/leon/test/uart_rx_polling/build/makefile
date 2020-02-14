#sample makefile for ibuild

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        =  uart_rx_polling_test.c
ASMSOURCES      = 

#Specify the name of the ilog file for this component with the path
#COMPONENT_ILOG_FILE = ../src/<Component name goes here>_log.h

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = leon_uart_rx_polling_test.elf

# define the type of target to build (lib, lib_collector, or elf)
TARGET_TYPE     = elf

# the names of the dependencies of this component
# For elf targets:
#   The components are passed to the linker in the order given here
#   The linker expects the high level components first, and the drivers last, so it only needs a single pass
#   If there are circular dependencies between the components, use FORCED_LINK_DEPS below
COMPONENT_DEPS  = leon ibase

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS = 

# Files in the target directory not created by ibuild that should be cleaned up on a make clean
TARG_DIR_CLEANUP_FILES  = $(TARGET:.elf=).* icron_header

target: $(TARGET)
	$(OBJCOPY) -O srec $(targ_dir)/$(TARGET) $(targ_dir)/$(TARGET:.elf=.s19)
	srec_cat $(targ_dir)/$(TARGET:.elf=.s19) -o $(targ_dir)/$(TARGET:.elf=.icr) -Stewie -Enable_Sequence_Warnings
	$(OBJDUMP) -t $(targ_dir)/$(TARGET) |sort > $(targ_dir)/$(TARGET:.elf=.map)
	$(OBJDUMP) -d $(targ_dir)/$(TARGET) > $(targ_dir)/$(TARGET:.elf=.s)

	echo -e "version:1\nproject:lg1_test\ntest_image:$(TARGET:.elf=.icr)" > $(targ_dir)/icron_header

	tar cfj  $(targ_dir)/$(TARGET:.elf=.icron) -C  $(targ_dir) icron_header $(TARGET:.elf=.icr)


.PHONY: target

# Pull in the ibuild make rules
include $(IBUILD_RULES)

