# makefile for Blackbird Software
PRE_BUILD_DEPS =ilog register_definitions

#default rule
ifeq ($(NO_DEBUG), 1)
  all: blackbird_a7_debug blackbird_a7
else
  all: blackbird_a7_debug
endif

# Stupid fix for calling $(MAKE), since it stupidly adds -w
# Unless the user specifically requests directories to be printed, add the flag --no-print-directory.  No bearing if silent output was selected
ifneq ($(findstring $(MAKEFLAGS),s),s)
ifneq ($(findstring $(MAKEFLAGS),w),w)
ifneq ($(findstring $(MAKEFLAGS),--print-directory),--print-directory)
#MAKEFLAGS+=--no-print-directory
endif
endif
endif

# print-%: ; @echo $* = $($*)
blackbird_a7_debug:
	PROJECT=blackbird_a7_debug $(MAKE) target DEBUG=1 PLATFORM_A7=1

blackbird_a7:
	PROJECT=blackbird_a7 $(MAKE) target PLATFORM_A7=1

blackbird_k7:
	PROJECT=blackbird_k7 $(MAKE) target DEBUG=1 PLATFORM_K7=1

blackbird_a7_sim:
	PROJECT=blackbird_a7_sim $(MAKE) target SIM=1 DEBUG=1 PLATFORM_A7=1

blackbird_k7_sim:
	PROJECT=blackbird_k7_sim $(MAKE) target SIM=1 DEBUG=1 PLATFORM_K7=1

clean_misc:
#echo ">>> $(shell date)"
	$(RM) ../blackbird_a7_bin/*.orig
	$(RM) ../blackbird_a7_sim_bin/*.orig
	$(RM) ../blackbird_a7_debug_bin/*.orig
	$(RM) ../blackbird_k7_bin/*.orig
	$(RM) ../blackbird_k7_sim_bin/*.orig
	$(RM) ../blackbird_a7_bin/*.S
	$(RM) ../blackbird_a7_sim_bin/*.S
	$(RM) ../blackbird_a7_debug_bin/*.S
	$(RM) ../blackbird_k7_bin/*.S
	$(RM) ../blackbird_k7_sim_bin/*.S
	$(RM) ../blackbird_a7_bin/*.icron
	$(RM) ../blackbird_a7_sim_bin/*.icron
	$(RM) ../blackbird_a7_debug_bin/*.icron
	$(RM) ../blackbird_k7_bin/*.icron
	$(RM) ../blackbird_k7_sim_bin/*.icron
	$(RM) ../blackbird_a7_bin/*.map
	$(RM) ../blackbird_a7_sim_bin/*.map
	$(RM) ../blackbird_a7_debug_bin/*.map
	$(RM) ../blackbird_k7_bin/*.map
	$(RM) ../blackbird_k7_sim_bin/*.map
	$(RM) ../blackbird_a7_bin/*.json
	$(RM) ../blackbird_a7_sim_bin/*.json
	$(RM) ../blackbird_a7_debug_bin/*.json
	$(RM) ../blackbird_a7_bin/*.bin
	$(RM) ../blackbird_a7_sim_bin/*.bin
	$(RM) ../blackbird_a7_debug_bin/*.bin
	$(RM) ../blackbird_a7_bin/*.xml
	$(RM) ../blackbird_a7_sim_bin/*.xml
	$(RM) ../blackbird_a7_debug_bin/*.xml
	$(RM) ../blackbird_a7_bin/*.mcs
	$(RM) ../blackbird_a7_sim_bin/*.mcs
	$(RM) ../blackbird_a7_bin/ge_asic_debug
	$(RM) ../blackbird_a7_sim_bin/ge_asic_debug
	$(RM) ../blackbird_a7_debug_bin/ge_asic_debug
	$(RM) ./ge_flshwtr.o
	$(RM) ./ge_fw.o


clean: clean_misc
	PROJECT=blackbird_a7_debug $(MAKE) clean_ibuild DEBUG=1 PLATFORM_A7=1
	PROJECT=blackbird_a7 $(MAKE) clean_ibuild PLATFORM_A7=1
	PROJECT=blackbird_k7 $(MAKE) clean_ibuild DEBUG=1 PLATFORM_K7=1
	PROJECT=blackbird_a7_sim $(MAKE) clean_ibuild SIM=1 DEBUG=1 PLATFORM_A7=1
	PROJECT=blackbird_k7_sim $(MAKE) clean_ibuild SIM=1 DEBUG=1 PLATFORM_K7=1

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES = $(notdir $(wildcard ../src/*.c))
ASMSOURCES = $(notdir $(wildcard ../src/*.S))

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/toplevel_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE = toplevel_cmd.h

# Build a dependency graph
IBUILD_DEP_GRAPH_PNG=$(PROJECT).png
# Exclude the following components from the dependency graph (eg. ibase, ilog, & icmd, which everything uses)
IBUILD_DEP_GRAPH_EXCLUDES=ibase timing leon icmd ilog i2c

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = $(PROJECT).icron

# define the type of target to build (lib, lib_collector, or elf)
TARGET_TYPE     = icron
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

#OBJ_DIR_CLEANUP_FILES = ge_core.bin

# the names of the dependencies of this component
# For elf targets:
# Note: register_definitions is placed first because it needs to be built first so that the headers
# required by the other components will exist
#   The components are passed to the linker in the order given here.  The linker expects the high
#   level components first, and the drivers last, so it only needs a single pass.  If there are
#   circular dependencies between the components, use FORCED_LINK_DEPS below.
COMPONENT_DEPS_BASE  = rs232 rom command ge_program lan_port dp dp_stream dp_aux linkmgr xaui idt_clk fiber5g i2cd led bbge_comm \
                  gpio eeprom pcs_pma atmel_crypto upp\
                  i2c i2c_slave mdiod aquantia mdio xmodem \
                  mca cpu_comm test configuration flash_data random xadc stats_mon timing callback event\
                  main_loop mac \
                  icmd top util \
                  ilog sfi crc \
                  bbge_comm uart \
                  leon core sha2 \
                  ibase register_definitions crc

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS = i2cd

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES = just_icmds

# default buttons for Hobbes for icron targets
# example
#HOBBES_DEFAULT_ICMD_BUTTON1=Start: iCmds['TEST_HARNESS_COMPONENT']['startTests']()
HOBBES_DEFAULT_ICMD_BUTTON1=Print SW Version: iCmds['TOPLEVEL_COMPONENT']['PrintSwVersion']()
HOBBES_DEFAULT_ICMD_BUTTON2=
HOBBES_DEFAULT_ICMD_BUTTON3=
HOBBES_DEFAULT_ICMD_BUTTON4=
HOBBES_DEFAULT_ICMD_BUTTON5=
HOBBES_DEFAULT_ICMD_BUTTON6=
HOBBES_DEFAULT_ICMD_BUTTON7=
HOBBES_DEFAULT_ICMD_BUTTON8=
HOBBES_DEFAULT_ICMD_BUTTON9=


# If this is a project component uncomment the following
include project.mak

# Set cflag for sim build
ifeq ($(SIM),1)
  CFLAGS += -DBUILD_FOR_SIM
endif


# Set the debug flag
ifeq ($(DEBUG),1)
  CFLAGS += -DDEBUG
endif

ifeq ($(BB_PROFILE),1)
  CFLAGS += -DBB_PROFILE
endif

ifeq ($(PLATFORM_A7),1)
  CFLAGS += -DPLATFORM_A7
endif

ifeq ($(PLATFORM_K7),1)
  CFLAGS += -DPLATFORM_K7
endif

# Add DP by default unless someone defines BUILD_NO_DP as an input param to the make command
ifeq ($(BUILD_NO_USB),1)
  CFLAGS += -DBUILD_NO_USB
else
  COMPONENT_DEPS += ulp
endif

COMPONENT_DEPS += $(COMPONENT_DEPS_BASE)

GE_BIN = "../bb_ge/ge_asic_bb_companion_debug_bin/"

# Supply the build time flags for the buildtime file & strip leading zero so nothing is treated as octal
# Note: hour, minute & second could be zero, so we need to ensure we only strip the leading zero from 00
# Don't change anything in it, it is also being used to define timestamp in options.h to be used in excom
# check comments in options.h for more info
export DATE_CFLAGS=                                         \
 -DMAKE_BUILD_YEAR=$(shell date +"%Y" | sed 's/^0//g')      \
 -DMAKE_BUILD_MONTH=$(shell date +"%m" | sed 's/^0//g')     \
 -DMAKE_BUILD_DAY=$(shell date +"%d" | sed 's/^0//g')       \
 -DMAKE_BUILD_HOUR=$(shell date +"%H" | sed 's/^0//g')      \
 -DMAKE_BUILD_MINUTE=$(shell date +"%M" | sed 's/^0//g')    \
 -DMAKE_BUILD_SECOND=$(shell date +"%S" | sed 's/^0//g')

imain.c_CFLAGS=$(DATE_CFLAGS)
#put the timestamp in build_time.txt file with DATEFLAGS, to be used in EXCOM
$(shell echo 'SOFTWARE_TIME_STAMP $(DATE_CFLAGS)' > ../inc/build_time.txt)

# Add a link to the GE binary as a dependency.  Otherwise make clean would be needed
# NOTE: this is set here and in the source file
#ge_core.S_DEPS=$(obj_dir)/ge_core.bin
#ge_core.S_CFLAGS+=-DOBJ_DIR=$(obj_dir)

# Pull in the ibuild make rules if we are ready to build a project
ifneq ("$(PROJECT)", "")
include $(IBUILD_RULES)
endif

# NOTE: define rules which use ibuild variables, AFTER ibuild_rules is included, so obj_dir gets expanded properly
#$(obj_dir)/ge_core.bin: $(obj_dir_stamp)
#	$(CP) $(TOP_LEVEL_COMPONENTS_DIR)/goldenears/ge_core_bin/ge_core.bin $@

ifeq ($(DEBUG),1)
# dummy rule to build the target
  target: gen_ge_flshwtr gen_ge_fw $(if $(findstring testrom_blackbird.ld, $(LDFLAGS)), gen_rom,) gen_programbb $(TARGET)
	  $(OBJDUMP) -t $(targ_dir)/target.elf | sort > $(targ_dir)/target.map
	  $(OBJDUMP) -d $(targ_dir)/target.elf > $(targ_dir)/target.S
	  output=$$(. $(TOP_LEVEL_COMPONENTS_DIR)/ibuild/etc/checkRamUseGeneric.sh; \
	      echo -e "\nBB Ram use for $(TARGET):" && \
	      $(SIZE) $(targ_dir)/target.elf && \
	      echo -n "Lex: " && LexAhbRamUse   "$(targ_dir)/target.elf" 0 && \
	      echo -n "Lex: " && LexIramUse     "$(targ_dir)/target.elf" 160 && \
	      echo -n "Lex: " && LexDramUse     "$(targ_dir)/target.elf" 56 && \
	      echo -n "Rex: " && RexAhbRamUse   "$(targ_dir)/target.elf" 0 && \
	      echo -n "Rex: " && RexIramUse     "$(targ_dir)/target.elf" 160 && \
	      echo -n "Rex: " && RexDramUse     "$(targ_dir)/target.elf" 56 && \
	      FlashUse "$(targ_dir)/target.elf" 384); \
	  echo "$$output"

  gen_programbb:
	  echo "Converting ProgramBB from bin to object file to link with"
	  $(OBJCOPY) --rename-section .data=.pgmbb_data -I binary -O elf32-sparc -B sparc ../program/programBB_bin/programBB.bin pgmbb.o

  # temporary for testing ROM don't keep
  gen_rom:
	  echo "Converting ROM from bin to object file to link with"
	  $(OBJCOPY) --rename-section .data=.rom_data -I binary -O elf32-sparc -B sparc ../rom/blackbird_rom_bin/blackbird_rom.bin rom.o

  gen_ge_flshwtr:
	  echo "Converting GE flashwriter from bin to object file to link with"
	  $(OBJCOPY) --rename-section .data=.ge_flwr_data -I binary -O elf32-sparc -B sparc $(GE_BIN)flash_writer.bin ge_flshwtr.o

  gen_ge_fw:
	  echo "Converting GE firmware from bin to object file to link with"
	  $(OBJCOPY) --rename-section .data=.ge_fw_data -I binary -O elf32-sparc -B sparc $(GE_BIN)ge_asic_bb_companion_debug.bin ge_fw.o

else

  target: gen_ge_flshwtr gen_ge_fw $(if $(findstring testrom_blackbird.ld, $(LDFLAGS)), gen_rom,) gen_programbb $(TARGET)
	  $(OBJDUMP) -t $(targ_dir)/target.elf | sort > $(targ_dir)/target.map
	  $(OBJDUMP) -d $(targ_dir)/target.elf > $(targ_dir)/target.S
	  output=$$(. $(TOP_LEVEL_COMPONENTS_DIR)/ibuild/etc/checkRamUseGeneric.sh; \
	      echo -e "\nBB Ram use for $(TARGET):" && \
	      $(SIZE) $(targ_dir)/target.elf && \
	      echo -n "Lex: " && LexAhbRamUse   "$(targ_dir)/target.elf" 0 && \
	      echo -n "Lex: " && LexIramUse     "$(targ_dir)/target.elf" 160 && \
	      echo -n "Lex: " && LexDramUse     "$(targ_dir)/target.elf" 56 && \
	      echo -n "Rex: " && RexAhbRamUse   "$(targ_dir)/target.elf" 0 && \
	      echo -n "Rex: " && RexIramUse     "$(targ_dir)/target.elf" 160 && \
	      echo -n "Rex: " && RexDramUse     "$(targ_dir)/target.elf" 56 && \
	      FlashUse "$(targ_dir)/target.elf" 384);  \
	  echo "$$output"

  gen_programbb:
	  echo "Converting ProgramBB from bin to object file to link with"
	  $(OBJCOPY) --rename-section .data=.pgmbb_data -I binary -O elf32-sparc -B sparc ../program/programBB_bin/programBB.bin pgmbb.o

  # temporary for testing ROM don't keep
  gen_rom:
	  echo "Converting ROM from bin to object file to link with"
	  $(OBJCOPY) --rename-section .data=.rom_data -I binary -O elf32-sparc -B sparc ../rom/blackbird_rom_bin/blackbird_rom.bin rom.o

  gen_ge_flshwtr:
	  echo "Converting GE flashwriter from bin to object file to link with"
	  $(OBJCOPY) --rename-section .data=.ge_flwr_data -I binary -O elf32-sparc -B sparc $(GE_BIN)flash_writer.bin ge_flshwtr.o

  gen_ge_fw:
	  echo "Converting GE firmware from bin to object file to link with"
	  $(OBJCOPY) --rename-section .data=.ge_fw_data -I binary -O elf32-sparc -B sparc $(GE_BIN)ge_asic_bb_companion_debug.bin ge_fw.o

endif

  .PHONY: clean_misc gen_ge_flshwtr gen_ge_fw gen_rom gen_programbb
