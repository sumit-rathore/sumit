# project.mak file for the Golden Ears SW

# PROJECT is defined in top level makefile

#Define the component directory
export TOP_LEVEL_COMPONENTS_DIR=$(shell pwd)/../components


# Define all of the following tools
# Note: CROSS is only a convience for this file only
CROSS=kleon-elf-
export AR=${CROSS}ar
export CC=${CROSS}gcc
export LD=${CROSS}gcc
export AS=${CROSS}as
export OBJCOPY=${CROSS}objcopy
export OBJDUMP=${CROSS}objdump
export SIZE=${CROSS}size
export MKDIR=mkdir -p
export RMDIR=rmdir
export RM=rm -f
export SED=sed
export AWK=awk
export CP=cp


LANGUAGEFLAGS=-std=gnu99

# Compiler flags: broken up into different groups for convience, only CFLAGS is exported
WARNINGFLAGS=-Wall \
 -Werror -Wextra -Wno-type-limits -Wno-unused-parameter \
 -Wswitch-default -Wmissing-noreturn -Wmissing-format-attribute \
 -Wundef -Wformat-nonliteral -Wwrite-strings -Wshadow -Wpointer-arith -Winline \
 -Wformat-security -Winit-self -Wmissing-include-dirs -Wnested-externs
#Useful warnings, but cause too many issues with the current LG1 code
# -Wredundant-decls -- Causes issues with iCmd, as we try and ensure that our version of the declaration is correct
# -Wmissing-declarations  -- This causes issues with the spectra reg code
# -Wcast-align  -- Causes issues with AsicRegisters.h, as there are no checks for alignment in this file
# -Wswitch-enum  -- Very useful, but currently has too many warnings that need to be fixed

# NOTE: delete null pointer checks is bad, when there is no MMU to capture segfaults.
OPTIMIZATIONFLAGS=-Os -fweb -frename-registers -ffunction-sections -fdata-sections \
 -fsched-pressure -flto -free -fno-toplevel-reorder -fno-delete-null-pointer-checks

# Note: that each components "/inc/" directory is automatically included
#       This is only for an additional project component include directory
#       It should normally be left empty
#
#       A use case for using it would be a top level include directory for an options.h
#       Using "-I$(shell pwd)/../inc -include options.h" from the top level /build could be
#       easier than including a bunch of DEFNFLAGS for the whole project
#
#       Another use case is to pick up on a project_components.h project wide header file
INCLUDEFLAGS=-I$(shell pwd)/../inc
# Note: changing the ABI would require a "make clean" afterwards
MACHINEFLAGS=-msoft-float -mapp-regs -fshort-enums -freg-struct-return -mtune=leon -mcpu=v8 -mmetal -ffreestanding
#For big endian projects using ilog, and use the options.h include file
DEFNFLAGS=-DIENDIAN=1 -DUSE_OPTIONS_H -DGOLDENEARS $(if $(BB_GE_COMPANION), -DBB_GE_COMPANION,)
# -pipe just gets GCC to use pipes between the preprocessor, compiler and assembler, instead of temp files
OTHERCFLAGS=-pipe
export CFLAGS=${LANGUAGEFLAGS} ${OPTIMIZATIONFLAGS} ${WARNINGFLAGS} ${INCLUDEFLAGS} ${MACHINEFLAGS} ${DEFNFLAGS} ${OTHERCFLAGS}



# Linker flags
LDFLAGS_WO_LD_SCRIPT=-nostartfiles -msoft-float -Wl,--gc-sections ${CFLAGS} -Wl,--sort-section=alignment
LDFLAGS_ROM_SYMS=-Wl,-R -Wl,$(shell pwd)/goldenears_rom.syms.elf
LDFLAGS_MEMORY_FILE=$(TOP_LEVEL_COMPONENTS_DIR)/leon/build/$(if $(GE_SPARTAN),leon_spartan.ld,leon.ld)
LD_FLAGS_SECTION_FILE=$(if $(GE_CORE),ge_core.ld,ge.ld)
LD_FLAGS_RAM_SECTION_FILE=$(TOP_LEVEL_COMPONENTS_DIR)/leon/build/leon_ram.ld
export LDFLAGS=-T$(LDFLAGS_MEMORY_FILE) -T$(LD_FLAGS_SECTION_FILE) $(LDFLAGS_WO_LD_SCRIPT) $(if $(GE_CORE), , $(LDFLAGS_ROM_SYMS))
export LDFLAGS_TEST_HARNESS=-T$(LDFLAGS_MEMORY_FILE) -T$(LD_FLAGS_RAM_SECTION_FILE) $(LDFLAGS_WO_LD_SCRIPT) $(LDFLAGS_ROM_SYMS)
export FLASH_WRITER_LD_FLAGS=-T$(LDFLAGS_MEMORY_FILE) -T$(LD_FLAGS_RAM_SECTION_FILE) $(LDFLAGS_WO_LD_SCRIPT)


# This defines the version of ibuild that we are configured to use
# a future version of ibuild may require different variables to be set
export IBUILD_PROJECT_MAK_CONFIGURED_VERSION=1

# Note the location of the rules.mak file for all the components
export IBUILD_RULES=$(TOP_LEVEL_COMPONENTS_DIR)/ibuild/build/rules.mak

# Have iLog use the leon backend
export ILOG_BACKEND=LEON

# where is the project_components file
export PROJECT_COMPONENTS=$(shell pwd)/../inc/project_components.h

# Set up the ibuild settings for making .icron files
export SPECTAREG_XML_FILES=grg_comp.xml ulmii_comp.xml clm_comp.xml xlr_comp.xml xrr_comp.xml xcsr_comp.xml
export IBUILD_ICRON_HEADER_SPECTAREG_BLOCK=\
	"spectareg_xml: $(SPECTAREG_XML_FILES)\n"\
	"spectareg_grg_comp_offset: 0x20000000\n"\
	"spectareg_ulmii_comp_offset: 0x20000100\n"\
	"spectareg_clm_comp_offset: 0x20000200\n"\
	"spectareg_xrr_comp_offset: 0x20000300\n"\
	"spectareg_xlr_comp_offset: 0x20000400\n"\
	"spectareg_xcsr_comp_offset: 0x20000500\n"
export ICRON_HEADER_PROJECT=GOLDENEARS

