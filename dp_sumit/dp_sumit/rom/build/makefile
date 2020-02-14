#makefile for Blackbird ROM
PRE_BUILD_DEPS =ilog register_definitions

# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        = $(notdir $(wildcard ../src/*.c))
ASMSOURCES      =
ROM_TEMPLATE = rom_template.v

#Specify the name of the ilog file for this component with the path
#COMPONENT_ILOG_FILE = ../src/<Component name goes here>_log.h

# Build a dependency graph
IBUILD_DEP_GRAPH_PNG=$(PROJECT).png
# Exclude the following components from the dependency graph (eg. ibase, ilog, & icmd, which everything uses)
IBUILD_DEP_GRAPH_EXCLUDES=


# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = $(PROJECT).elf
TARGET_BIN      = $(TARGET:%.elf=%.bin)
TARGET_VERILOG  = $(TARGET:%.elf=%.v)

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
TARG_DIR_CLEANUP_FILES  = $(PROJECT).*
OBJ_DIR_CLEANUP_FILES  =

# the names of the dependencies of this component
# For elf targets:
#   The components are passed to the linker in the order given here
#   The linker expects the high level components first, and the drivers last, so it only needs a single pass
#   If there are circular dependencies between the components, use FORCED_LINK_DEPS below
COMPONENT_DEPS  = atmel_crypto sfi top gpio util timing leon core crc register_definitions ibase

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS = 

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES =

# If this is a project component uncomment the following
include project.mak

exposedSymbols=$(shell $(AWK) '/^}/ { x = 0 } \
x == 1 { sub("^.*\\&", "-K "); sub(",", ""); print $0; } \
/^{/ { x = 1 }' < ../src/references.c)


target: $(TARGET)
	# First create the elf file containing the exposed function references
	$(STRIP) --strip-all $(exposedSymbols) $(targ_dir)/$(TARGET) -o $(targ_dir)/$(TARGET:.elf=.syms.elf)
	# Then create the ROM file which contains no large table of exposed functions outside of ROM
	$(OBJCOPY) -R .exposedFunctions $(targ_dir)/$(TARGET)
	# Finally create the verilog ROM file
	@echo "Building binary $(TARGET_BIN)"; $(OBJCOPY) -O binary $(targ_dir)/$(TARGET) $(targ_dir)/$(TARGET_BIN)
	$(OBJDUMP) -t $(targ_dir)/blackbird_rom.elf | sort > $(targ_dir)/blackbird_rom.map
	$(OBJDUMP) -d $(targ_dir)/blackbird_rom.elf > $(targ_dir)/blackbird_rom.S



# Pull in the ibuild make rules
include $(IBUILD_RULES)

