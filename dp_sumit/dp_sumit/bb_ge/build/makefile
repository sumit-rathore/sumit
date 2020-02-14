# makefile for Golden Ears Software

#default rule
#all: expresslink expresslink_debug logreader ge_asic ge_asic_bb_companion_debug ge_asic_bb_companion ge_spartan ge_asic_debug ge_spartan_debug ge_core ge_core_debug ge_sim ge_core_sim ge_test
all: ge_asic_bb_companion_debug ge_asic_bb_companion

# Stupid fix for calling $(MAKE), since it stupidly adds -w
# Unless the user specifically requests directories to be printed, add the flag --no-print-directory.  No bearing if silent output was selected
ifneq ($(findstring $(MAKEFLAGS),s),s)
ifneq ($(findstring $(MAKEFLAGS),w),w)
ifneq ($(findstring $(MAKEFLAGS),--print-directory),--print-directory)
MAKEFLAGS+=--no-print-directory
endif
endif
endif


# The variables passed to clean_helper should be the same as in the rules below in order to clean
# with the same settings as the build.
clean:
	PROJECT=expresslink             $(MAKE) clean -C $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/build
	PROJECT=expresslink_debug       $(MAKE) clean -C $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/build
	$(MAKE) -C $(TOP_LEVEL_COMPONENTS_DIR)/ilog/etc/LogReader clean
	PROJECT=ge_asic_bb_companion    $(MAKE) clean_helper BB_GE_COMPANION=1
	PROJECT=ge_asic_bb_companion_debug    $(MAKE) clean_helper BB_GE_COMPANION=1 DEBUG=1

clean_full:
	PROJECT=expresslink             $(MAKE) clean -C $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/build
	PROJECT=expresslink_debug       $(MAKE) clean -C $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/build
	$(MAKE) -C $(TOP_LEVEL_COMPONENTS_DIR)/ilog/etc/LogReader clean
	PROJECT=ge_asic_bb_companion    $(MAKE) clean_helper BB_GE_COMPANION=1
	PROJECT=ge_asic_bb_companion_debug    $(MAKE) clean_helper BB_GE_COMPANION=1 DEBUG=1
	PROJECT=ge_asic                 $(MAKE) clean_helper
	PROJECT=ge_spartan              $(MAKE) clean_helper
	PROJECT=ge_asic_debug           $(MAKE) clean_helper DEBUG=1
	PROJECT=ge_spartan_debug        $(MAKE) clean_helper DEBUG=1
	PROJECT=ge_core 				$(MAKE) clean_helper GE_CORE=1
	PROJECT=ge_core_debug 			$(MAKE) clean_helper GE_CORE=1 DEBUG=1
	PROJECT=ge_sim					$(MAKE) clean_helper SIM=1 DEBUG=1
	PROJECT=ge_core_sim				$(MAKE) clean_helper GE_CORE=1 SIM=1 DEBUG=1
	PROJECT=ge_test					$(MAKE) clean_helper DEBUG=1


clean_helper:
	$(MAKE) clean_ibuild
	$(MAKE) clean -C $(TOP_LEVEL_COMPONENTS_DIR)/flash_writer/build
	PROJECT=$(PROJECT)_full $(MAKE) clean -C $(TOP_LEVEL_COMPONENTS_DIR)/flash_writer/build

expresslink: ge_asic ge_spartan
	mkdir -p $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_obj
	cp ../ge_spartan_bin/lex_firmware.bin $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_obj/firmware_ge_spartan_lex.bin
	cp ../ge_spartan_bin/rex_firmware.bin $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_obj/firmware_ge_spartan_rex.bin
	cp ../ge_spartan_bin/flash_writer_full.bin $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_obj/flash_writer_ge_spartan.bin
	cp ../ge_asic_bin/ge_asic.bin $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_obj/firmware_ge.bin
	cp ../ge_asic_bin/flash_writer.bin $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_obj/flash_writer_ge.bin
	PROJECT=$@ $(MAKE) -C $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/build
	mkdir -p ../$(@)_bin
	PROJECT=$@ cp $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_bin/ExpressLink.exe ../$(@)_bin/ExpressLink.exe

expresslink_debug: ge_asic_debug ge_spartan_debug
	mkdir -p $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_obj
	cp ../ge_spartan_debug_bin/lex_firmware.bin $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_obj/firmware_ge_spartan_lex.bin
	cp ../ge_spartan_debug_bin/rex_firmware.bin $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_obj/firmware_ge_spartan_rex.bin
	cp ../ge_spartan_debug_bin/flash_writer_full.bin $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_obj/flash_writer_ge_spartan.bin
	cp ../ge_asic_debug_bin/ge_asic_debug.bin $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_obj/firmware_ge.bin
	cp ../ge_asic_debug_bin/flash_writer.bin $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_obj/flash_writer_ge.bin
	PROJECT=$@ $(MAKE) -C $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/build $(if $(EXPRESS_LINK_RELEASE_NAME), EXPRESS_LINK_RELEASE_NAME="$(EXPRESS_LINK_RELEASE_NAME) DEBUG")
	mkdir -p ../$(@)_bin
	PROJECT=$@ cp $(TOP_LEVEL_COMPONENTS_DIR)/expresslink/$(@)_bin/ExpressLink.exe ../$(@)_bin/ExpressLink.exe

# log reader fails with parallel build
logreader:
	mkdir -p ../$(@)_bin
	( cd $(TOP_LEVEL_COMPONENTS_DIR)/ilog/etc/LogReader && make )
	cp $(TOP_LEVEL_COMPONENTS_DIR)/ilog/etc/LogReader/bin/Release/LogReader.exe $(@)_bin


ge_asic_bb_companion:
	PROJECT=$@ $(MAKE) target GE_ASIC=1 BB_GE_COMPANION=1

ge_asic_bb_companion_debug:
	PROJECT=$@ $(MAKE) target GE_ASIC=1 BB_GE_COMPANION=1 DEBUG=1

ge_asic:
	PROJECT=$@ $(MAKE) target GE_ASIC=1

ge_spartan:
	PROJECT=$@ $(MAKE) target BUILD_FULL_FLASHWRITER=1 GE_SPARTAN=1

ge_asic_debug:
	PROJECT=$@ $(MAKE) target DEBUG=1 GE_ASIC=1

ge_spartan_debug:
	PROJECT=$@ $(MAKE) target DEBUG=1 BUILD_FULL_FLASHWRITER=1 GE_SPARTAN=1

ge_core:
	PROJECT=$@ $(MAKE) target GE_CORE=1

ge_core_debug:
	PROJECT=$@ $(MAKE) target GE_CORE=1 DEBUG=1

ge_sim:
	PROJECT=$@ $(MAKE) target SIM=1 DEBUG=1
	PROJECT=$@ $(MAKE) test SIM=1 DEBUG=1

ge_core_sim:
	PROJECT=$@ $(MAKE) target GE_CORE=1 SIM=1 DEBUG=1
	PROJECT=$@ $(MAKE) test GE_CORE=1 SIM=1 DEBUG=1

ge_test:
	PROJECT=$@ $(MAKE) target DEBUG=1
	PROJECT=$@ $(MAKE) test DEBUG=1

ge_profile:
	PROJECT=$@ $(MAKE) target DEBUG=1 GE_ASIC=1 GE_PROFILE=1
	PROJECT=$@ $(MAKE) target DEBUG=1 BUILD_FULL_FLASHWRITER=1 GE_SPARTAN=1 GE_PROFILE=1



# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        = imain.c fetch_n_run_image.c lexrex_msgs.c asserts.c flash_vars.c $(if $(GE_CORE), , authenticate.c) cypress_hx3_firmware.c
ASMSOURCES      = traps.S uninit_isr.S

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/toplevel_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE = toplevel_cmd.h

# Build a dependency graph
IBUILD_DEP_GRAPH_PNG=$(PROJECT).png
# Exclude the following components from the dependency graph (eg. ibase, ilog, & icmd, which everything uses)
IBUILD_DEP_GRAPH_EXCLUDES=timing tasksch icmd ilog leon ibase

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = $(PROJECT).elf

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
TARG_DIR_CLEANUP_FILES  = \
    $(PROJECT).* \
    flash_writer*.* \
    icron_header \
    $(SPECTAREG_XML_FILES) \
    ExpressLink*.exe \
	LogReader.exe \
    ?ex_firmware.bin \
    ?ex_firmware.srec \
    ?ex_firmware.mcs \
    ?ex_firmware.*mb.checksum \
    ?ex_flash_img.bin \
    ?ex_flash_img.srec \
    ?ex_flash_img.*mb.checksum



# the names of the dependencies of this component
# For elf targets:
#   The components are passed to the linker in the order given here
#   The linker expects the high level components first, and the drivers last, so it only needs a single pass
#   If there are circular dependencies between the components, use FORCED_LINK_DEPS below
COMPONENT_DEPS  = \
    $(if $(GE_CORE), , flash_writer) \
    $(if $(BB_GE_COMPANION), gebb_comm, ) \
    $(if $(GE_CORE), , xmodem) \
    linkmgr \
    pll_xusb \
    sys_ctrl_q \
    devmgr \
    descparser \
    lexulm \
    $(if $(GE_CORE), , vhub) \
    rexulm \
    $(if $(GE_CORE), , netcfg) \
    $(if $(GE_CORE), , net) \
    topology \
    rexsch \
    storage \
	flash_data \
	eeprom \
    $(if $(GE_CORE), , atmel_crypto) \
    kc705 \
    xcsr \
    xlr \
    xrr \
    clm \
    ulm \
    grg \
    timing \
    tasksch \
    random \
    ilog \
    icmd \
    leon \
    spectareg \
    $(if $(GE_CORE), , crc) \
    $(if $(GE_CORE), , sha2) \
    ibase

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS =

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES = usb_test_mode just_icmds block_protect_recovery ram_test

# If this is a project component uncomment the following
include project.mak

# Set cflag for sim build
ifeq ($(SIM),1)
  CFLAGS += -DBUILD_FOR_SIM
endif

# ASIC build flag
ifeq ($(GE_ASIC),1)
	CFLAGS += -DGE_ASIC
endif

# Spartan build flag
ifeq ($(GE_SPARTAN),1)
	CFLAGS += -DGE_SPARTAN
endif

# Set cflag for goldenears core configuration
ifeq ($(GE_CORE),1)
  CFLAGS += -DGE_CORE
endif

# Set the debug flag
ifeq ($(DEBUG),1)
  CFLAGS += -DDEBUG
endif

# Set the profile flag
ifeq ($(GE_PROFILE),1)
  CFLAGS += -DGE_PROFILE
endif


# Supply the build time flags for the buildtime file & strip leading zero so nothing is treated as octal
# Note: hour, minute & second could be zero, so we need to ensure we only strip the leading zero from 00
imain.c_CFLAGS=                                         \
 -DMAKE_BUILD_YEAR=$(shell date +"%Y" | sed 's/^0//g')      \
 -DMAKE_BUILD_MONTH=$(shell date +"%m" | sed 's/^0//g')     \
 -DMAKE_BUILD_DAY=$(shell date +"%d" | sed 's/^0//g')       \
 -DMAKE_BUILD_HOUR=$(shell date +"%H" | sed 's/^0//g')      \
 -DMAKE_BUILD_MINUTE=$(shell date +"%M" | sed 's/^0//g')    \
 -DMAKE_BUILD_SECOND=$(shell date +"%S" | sed 's/^0//g')


# Pull in the ibuild make rules if we are ready to build a project
ifneq ("$(PROJECT)", "")
include $(IBUILD_RULES)
endif


target_icron_header: $(targ_dir_stamp)
	echo -e "version:4\n"\
	"project:goldenears\n"\
	"icomponent:icomponent\n"\
	"ilog:ilog\n"\
	"icmd:icmd\n"\
	"icmdresp:icmdresp\n"\
	$(IBUILD_ICRON_HEADER_SPECTAREG_BLOCK)\
	"flash_writer:flash_writer.bin\n"\
	"main_firmware:$(TARGET:.elf=.bin)\n"\
	"hobbes_default_icmd_button1: Reset: iCmds['GRG_COMPONENT']['GRG_ResetChip']()\n"\
	"hobbes_default_icmd_button2: SwVersion: iCmds['TOPLEVEL_COMPONENT']['PrintSwVersion']()\n"\
	"hobbes_default_icmd_button3: Topology: iCmds['TOPOLOGY_COMPONENT']['_DTT_showAllDeviceXSST']()\n"\
	"hobbes_default_icmd_button4: DumpAllFlashVars: iCmds['STORAGE_COMPONENT']['STORAGE_icmdDumpAllVars']()"\
	| sed -r 's/^[ \t]*//g' > $(targ_dir)/icron_header

target_spectareg_xml_files:
	for p in $(SPECTAREG_XML_FILES); do cp $(TOP_LEVEL_COMPONENTS_DIR)/spectareg/etc/$$p $(targ_dir); done

target_bin: $(TARGET)
	$(OBJCOPY) -O binary $(targ_dir)/$(TARGET) $(targ_dir)/$(TARGET:.elf=.bin)

target_map: $(TARGET)
	$(OBJDUMP) -t $(targ_dir)/$(TARGET) |sort > $(targ_dir)/$(TARGET:.elf=.map)

target_asm: $(TARGET)
	$(OBJDUMP) -d -z $(targ_dir)/$(TARGET) > $(targ_dir)/$(TARGET:.elf=.s)

# Output everything about the size of the binary
# All output is collect and then displayed in 1 shot, to work with parallel make builds
target_size: $(TARGET)
#echo ">>> $(shell date)"
	output=$$(. $(TOP_LEVEL_COMPONENTS_DIR)/ibuild/etc/checkRamUseGeneric.sh; \
	    echo -e "\nGE Ram use for $(TARGET):" && \
	    $(SIZE) $(targ_dir)/$(TARGET) && \
	    echo -n "Lex IRAM: " && LexIramUse "$(targ_dir)/$(TARGET)" 48 && \
	    echo -n "Lex DRAM: " && LexDramUse "$(targ_dir)/$(TARGET)" 16 && \
	    echo -n "Rex IRAM: " && RexIramUse "$(targ_dir)/$(TARGET)" 48 && \
	    echo -n "Rex DRAM: " && RexDramUse "$(targ_dir)/$(TARGET)" 16); \
	echo "$$output"
	echo ">>> $(shell date)"

target_flash: $(targ_dir_stamp) flash_writer.component
	cp $(TOP_LEVEL_COMPONENTS_DIR)/flash_writer/$(PROJECT)_flash_bin/flash_writer.bin $(targ_dir)

target_flash_full: $(targ_dir_stamp)
	PROJECT=$(PROJECT)_full FLASHWRITER_FULL_IMAGE=Y $(MAKE) -C $(TOP_LEVEL_COMPONENTS_DIR)/flash_writer/build
	cp $(TOP_LEVEL_COMPONENTS_DIR)/flash_writer/$(PROJECT)_full_flash_bin/flash_writer.bin $(targ_dir)/flash_writer_full.bin

target_icron_file: target_icron_header target_spectareg_xml_files target_bin target_flash
	tar cfj  $(targ_dir)/$(TARGET:.elf=.icron) -C  $(targ_dir) icron_header $(TARGET:.elf=.bin) flash_writer.bin ilog icmd icmdresp icomponent $(SPECTAREG_XML_FILES)

target_full_image: target_bin
	$(OBJCOPY) -I ihex -O binary --gap-fill 0xFF --pad-to 0x380000 ../etc/ge_top_s6_lex.mcs $(targ_dir)/lex_firmware.bin
	$(OBJCOPY) -I ihex -O binary --gap-fill 0xFF --pad-to 0x380000 ../etc/ge_top_s6_rex.mcs $(targ_dir)/rex_firmware.bin
	cat $(targ_dir)/$(TARGET:.elf=.bin) >> $(targ_dir)/lex_firmware.bin
	cat $(targ_dir)/$(TARGET:.elf=.bin) >> $(targ_dir)/rex_firmware.bin

target_flash_img: target_full_image
	cat ../etc/recoveryImage.bin $(targ_dir)/lex_firmware.bin > $(targ_dir)/lex_flash_img.bin
	cat ../etc/recoveryImage.bin $(targ_dir)/rex_firmware.bin > $(targ_dir)/rex_flash_img.bin

target_flash_image_srec: target_flash_img
	$(OBJCOPY) -I binary -O srec $(targ_dir)/lex_flash_img.bin $(targ_dir)/lex_flash_img.srec
	$(OBJCOPY) -I binary -O srec $(targ_dir)/rex_flash_img.bin $(targ_dir)/rex_flash_img.srec

target_srec: target_bin
	$(OBJCOPY) -I binary -O srec $(targ_dir)/$(TARGET:.elf=.bin) $(targ_dir)/$(TARGET:.elf=.srec)

# The change-address argument specifies which address the MCS file should start
# from
target_mcs: target_full_image
	$(OBJCOPY) -I binary -O ihex --change-address 0x200000 $(targ_dir)/lex_firmware.bin $(targ_dir)/lex_firmware.mcs
	$(OBJCOPY) -I binary -O ihex --change-address 0x200000 $(targ_dir)/rex_firmware.bin $(targ_dir)/rex_firmware.mcs

target_checksum: target_flash_img
	$(TOP_LEVEL_COMPONENTS_DIR)/ibase/etc/checksum.py $$((1 << 22)) < $(targ_dir)/lex_flash_img.bin > $(targ_dir)/lex_flash_img.32mb.checksum
	$(TOP_LEVEL_COMPONENTS_DIR)/ibase/etc/checksum.py $$((1 << 22)) < $(targ_dir)/rex_flash_img.bin > $(targ_dir)/rex_flash_img.32mb.checksum


target: $(TARGET) target_icron_header target_spectareg_xml_files target_bin target_map target_asm target_size target_flash $(if $(BUILD_FULL_FLASHWRITER), target_flash_full) target_icron_file target_flash_image_srec target_srec target_checksum target_mcs

.PHONY: target_icron_header target_spectareg_xml_files target_bin target_map target_asm target_size target_flash target_flash_full target_icron_file target_full_image target_flash_img target_flash_image_srec target_srec target_mcs target_checksum target clean_full

