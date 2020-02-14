# makefile for Blackbird Software
PRE_BUILD_DEPS =ilog register_definitions

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES = $(notdir $(wildcard ../src/*.c))
ASMSOURCES = $(notdir $(wildcard ../src/*.S))

#default rule
all: programBB


# Stupid fix for calling $(MAKE), since it stupidly adds -w
# Unless the user specifically requests directories to be printed, add the flag --no-print-directory.  No bearing if silent output was selected
ifneq ($(findstring $(MAKEFLAGS),s),s)
ifneq ($(findstring $(MAKEFLAGS),w),w)
ifneq ($(findstring $(MAKEFLAGS),--print-directory),--print-directory)
#MAKEFLAGS+=--no-print-directory
endif
endif
endif

programBB:
	PROJECT=programBB $(MAKE) target DEBUG=1 PLATFORM_A7=1

clean_misc:
#echo ">>> $(shell date)"
	$(RM) ../programBB_bin/*.S
	$(RM) ../programBB_bin/*.map
	$(RM) ../programBB_bin/*.bin


clean: clean_misc
	PROJECT=programBB $(MAKE) clean_ibuild DEBUG=1 PLATFORM_A7=1
#echo ">>> $(shell date)"


#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/toplevel_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE =

# Build a dependency graph
IBUILD_DEP_GRAPH_PNG=$(PROJECT).png
# Exclude the following components from the dependency graph (eg. ibase, ilog, & icmd, which everything uses)
IBUILD_DEP_GRAPH_EXCLUDES=ibase timing leon icmd ilog i2c

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = $(PROJECT).elf
TARGET_BIN      = $(TARGET:%.elf=%.bin)

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
OBJ_DIR_CLEANUP_FILES =

# the names of the dependencies of this component
# For elf targets:
# Note: register_definitions is placed first because it needs to be built first so that the headers
# required by the other components will exist
#   The components are passed to the linker in the order given here.  The linker expects the high
#   level components first, and the drivers last, so it only needs a single pass.  If there are
#   circular dependencies between the components, use FORCED_LINK_DEPS below.
COMPONENT_DEPS  = command ge_program i2cd led \
                  gpio eeprom pcs_pma \
                  i2c mdio xmodem \
                  configuration flash_data timing callback stats_mon event\
                  main_loop \
                  icmd top util \
                  ilog sfi \
                  uart \
                  leon core \
                  ibase register_definitions crc

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS = i2cd

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES =

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

# Set the debug flag
ifeq ($(DEBUG),1)
  CFLAGS += -DDEBUG
endif


# Supply the build time flags for the buildtime file & strip leading zero so nothing is treated as octal
# Note: hour, minute & second could be zero, so we need to ensure we only strip the leading zero from 00
export DATE_CFLAGS=                                         \
 -DMAKE_BUILD_YEAR=$(shell date +"%Y" | sed 's/^0//g')      \
 -DMAKE_BUILD_MONTH=$(shell date +"%m" | sed 's/^0//g')     \
 -DMAKE_BUILD_DAY=$(shell date +"%d" | sed 's/^0//g')       \
 -DMAKE_BUILD_HOUR=$(shell date +"%H" | sed 's/^0//g')      \
 -DMAKE_BUILD_MINUTE=$(shell date +"%M" | sed 's/^0//g')    \
 -DMAKE_BUILD_SECOND=$(shell date +"%S" | sed 's/^0//g')

imain.c_CFLAGS=$(DATE_CFLAGS)

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

GE_BIN = "../../bb_ge/ge_asic_bb_companion_debug_bin/"

# dummy rule to build the target
target: $(TARGET) $(targ_dir)/$(TARGET:.elf=.bin)
#echo ">>> $(shell date)"
	$(OBJDUMP) -t $(targ_dir)/$(TARGET) | sort > $(targ_dir)/$(TARGET).map
	$(OBJDUMP) -d $(targ_dir)/$(TARGET) > $(targ_dir)/$(TARGET).S
	output=$$(. $(TOP_LEVEL_COMPONENTS_DIR)/ibuild/etc/checkRamUseGeneric.sh; \
	    echo -e "\nBB Ram use for $(TARGET):" && \
	    $(SIZE) $(targ_dir)/$(TARGET) && \
	    echo -n "Lex/Rex : " && PgmBBAhbRamUse   "$(targ_dir)/$(TARGET)" 0 && \
	    echo -n "Lex : " && PgmbbLexIramUse     "$(targ_dir)/$(TARGET)" 160 && \
	    echo -n "Lex : " && PgmbbLexDramUse     "$(targ_dir)/$(TARGET)" 56 && \
	    echo -n "Rex : " && PgmbbRexIramUse     "$(targ_dir)/$(TARGET)" 160 && \
	    echo -n "Rex : " && PgmbbLexDramUse     "$(targ_dir)/$(TARGET)" 56); \
	echo "$$output"
#echo ">>> $(shell date)"



$(targ_dir)/$(TARGET:.elf=.bin): $(TARGET)
	$(OBJCOPY) -O binary $(targ_dir)/$(TARGET) $(targ_dir)/$(TARGET:.elf=.bin)


.PHONY: clean_misc

