#sample makefile for ibuild

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        =  block_protect_recovery_main.c
ASMSOURCES      = 

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/block_protect_recovery_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE = block_protect_recovery_cmd.h

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = block_protect_recovery.elf

# define the type of target to build (lib, lib_collector, or elf)
TARGET_TYPE     = elf

# the names of the dependencies of this component
# For elf targets:
#   The components are passed to the linker in the order given here
#   The linker expects the high level components first, and the drivers last, so it only needs a single pass
#   If there are circular dependencies between the components, use FORCED_LINK_DEPS below
COMPONENT_DEPS  = xmodem flash_data atmel_crypto xmodem xcsr xlr xrr clm ulm grg timing tasksch ilog icmd leon spectareg ibase

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS = 

# Files in the target directory not created by ibuild that should be cleaned up on a make clean
TARG_DIR_CLEANUP_FILES  = $(TARGET:.elf=).* icron_header \
                          grg_comp.xml ulmii_comp.xml clm_comp.xml xlr_comp.xml xrr_comp.xml xcsr_comp.xml


# We want to over ride the linker file from the project.mak file as we supply our own
export LDFLAGS=$(FLASH_WRITER_LD_FLAGS)

# If the logs are flooding, something has gone very wrong, don't drop logs, assert hard
export CFLAGS+=-DILOG_ASSERT_ON_DROPPED_LOGS

# define our own default rule
# For LG1 this depends on the stewie rule (.icr file) for the LG1 ROM
# Both LG1 & GE will build binary files for sending the flash writer by X-Modem
target: $(TARGET) target_icron_header target_spectareg_xml target_bin
	tar cfj  $(targ_dir)/$(TARGET:.elf=.icron) -C  $(targ_dir) icron_header $(TARGET:.elf=.bin) ilog icmd icmdresp icomponent grg_comp.xml ulmii_comp.xml clm_comp.xml xlr_comp.xml xrr_comp.xml xcsr_comp.xml

# Pull in the ibuild make rules
include $(IBUILD_RULES)

target_icron_header: $(targ_dir_stamp)
	echo -e "version:4\n"\
	"project:goldenears_test\n"\
	"icomponent:icomponent\n"\
	"ilog:ilog\n"\
	"icmd:icmd\n"\
	"icmdresp:icmdresp\n"\
	"spectareg_xml: grg_comp.xml ulmii_comp.xml clm_comp.xml xlr_comp.xml xrr_comp.xml xcsr_comp.xml\n"\
	"spectareg_grg_comp_offset: 0x20000000\n"\
	"spectareg_ulmii_comp_offset: 0x20000100\n"\
	"spectareg_clm_comp_offset: 0x20000200\n"\
	"spectareg_xrr_comp_offset: 0x20000300\n"\
	"spectareg_xlr_comp_offset: 0x20000400\n"\
	"spectareg_xcsr_comp_offset: 0x20000500\n"\
	"test_image:$(TARGET:.elf=.bin)\n"\
	"hobbes_default_icmd_button1: Reset: iCmds['GRG_COMPONENT']['GRG_ResetChip']()\n"\
	"hobbes_default_icmd_button2: SwVersion: iCmds['TOPLEVEL_COMPONENT']['PrintSwVersion']()\n"\
	"hobbes_default_icmd_button3: Topology: iCmds['TOPOLOGY_COMPONENT']['_DTT_showAllDeviceXSST']()" > $(targ_dir)/icron_header

target_spectareg_xml: $(targ_dir_stamp)
	cp $(TOP_LEVEL_COMPONENTS_DIR)/spectareg/etc/grg_comp.xml $(targ_dir)
	cp $(TOP_LEVEL_COMPONENTS_DIR)/spectareg/etc/ulmii_comp.xml $(targ_dir)
	cp $(TOP_LEVEL_COMPONENTS_DIR)/spectareg/etc/clm_comp.xml $(targ_dir)
	cp $(TOP_LEVEL_COMPONENTS_DIR)/spectareg/etc/xlr_comp.xml $(targ_dir)
	cp $(TOP_LEVEL_COMPONENTS_DIR)/spectareg/etc/xrr_comp.xml $(targ_dir)
	cp $(TOP_LEVEL_COMPONENTS_DIR)/spectareg/etc/xcsr_comp.xml $(targ_dir)

#binary for Xmodem
target_bin: $(TARGET)
	$(OBJCOPY) -O binary $(targ_dir)/$(TARGET) $(targ_dir)/$(TARGET:.elf=.bin)

.PHONY: target



