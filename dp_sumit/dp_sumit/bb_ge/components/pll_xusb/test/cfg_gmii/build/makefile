#sample makefile for ibuild

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        =  pll_cfg_gmii_main.c
ASMSOURCES      = 

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/pll_cfg_gmii_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
#COMPONENT_ICMD_FILE = pll_cfg_gmii_cmd.h

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = pll_cfg_gmii.elf

# define the type of target to build (lib, lib_collector, or elf)
TARGET_TYPE     = elf

# Files in the target directory not created by ibuild that should be cleaned up on a make clean
TARG_DIR_CLEANUP_FILES  = $(TARGET:.elf=.s19) $(TARGET:.elf=.imemh)

# the names of the dependencies of this component
# For elf targets:
#   The components are passed to the linker in the order given here
#   The linker expects the high level components first, and the drivers last, so it only needs a single pass
#   If there are circular dependencies between the components, use FORCED_LINK_DEPS below
COMPONENT_DEPS  = pll_xusb flash_data storage atmel_crypto eeprom grg tasksch timing ilog icmd leon ibase

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS = 

# Files in the target directory not created by ibuild that should be cleaned up on a make clean
TARG_DIR_CLEANUP_FILES  = $(TARGET:.elf=).* icron_header

IRAM_BASE=0x60000000

target: $(TARGET)
	$(OBJCOPY) -O binary $(targ_dir)/$(TARGET) $(targ_dir)/$(TARGET:.elf=.bin)
	echo -e "version:2\nproject:goldenears_test\nilog:ilog\nicmd:icmd\nicomponent:icomponent\nicmdresp:icmdresp\ntest_image:$(TARGET:.elf=.bin)" > $(targ_dir)/icron_header
	tar cfj  $(targ_dir)/$(TARGET:.elf=.icron) -C  $(targ_dir) icron_header $(TARGET:.elf=.bin) ilog icmd icmdresp icomponent
	$(OBJCOPY) -O srec $(targ_dir)/$(TARGET)  $(targ_dir)/$(TARGET:.elf=.s19)
	srec_cat  $(targ_dir)/$(TARGET:.elf=.s19) -offset -$(IRAM_BASE) -o  $(targ_dir)/$(TARGET:.elf=.imemh) -vmem 8

.PHONY: target

# Pull in the ibuild make rules
include $(IBUILD_RULES)

