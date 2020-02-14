# project.mak file for the Blackbird SW

# PROJECT is defined in top level makefile
export PROJECT=programBB
#Define the component directory
export TOP_LEVEL_COMPONENTS_DIR=$(shell pwd)/../../components


# Define all of the following tools
# Note: CROSS is only a convience for this file only
CROSS=sparc-elf-
export AR=${CROSS}gcc-ar
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


# Compiler flags: broken up into different groups for convience, only CFLAGS is exported
WARNINGFLAGS=-Wall \
 -Werror -Wextra -Wno-type-limits -Wno-unused-parameter \
 -Wswitch-default -Wmissing-noreturn -Wmissing-format-attribute \
 -Wundef -Wformat-nonliteral -Wwrite-strings -Wshadow -Wpointer-arith -Winline \
 -Wformat-security -Winit-self -Wmissing-include-dirs -Wnested-externs -Wswitch-enum
#Useful warnings, but cause too many issues with the current LG1 code
# -Wredundant-decls -- Causes issues with iCmd, as we try and ensure that our version of the declaration is correct
# -Wunreachable-code -- All of the logging levels that are compiled out, are returned as unreachable code
# -Wmissing-declarations  -- This causes issues with the spectra reg code
# -Wcast-align  -- Causes issues with AsicRegisters.h, as there are no checks for alignment in this file
OPTIMIZATIONFLAGS=-Os -fweb -frename-registers -ffunction-sections -fdata-sections \
 -fsched-pressure -flto -free -fno-toplevel-reorder

# Note: that each components "/inc/" directory is automatically included
#       This is only for an additional project component include directory
#       It should normally be left empty
#
#       A use case for using it would be a top level include directory for an options.h
#       Using "-I$(shell pwd)/../inc -include options.h" from the top level /build could be
#       easier than including a bunch of DEFNFLAGS for the whole project
#
#       Another use case is to pick up on a project_components.h project wide header file
INCLUDEFLAGS=-I$(shell pwd)/../../inc

# Note: changing the ABI would require a "make clean" afterwards
MACHINEFLAGS=-msoft-float -mapp-regs -fshort-enums -freg-struct-return -mtune=leon -mcpu=v8 -mmetal -ffreestanding
#For big endian projects using ilog, and use the options.h include file
DEFNFLAGS=-DDEBUG -DIENDIAN=1 -DUSE_OPTIONS_H -DBLACKBIRD -DIDS_BIG_ENDIAN -DTRAP_NO_RETURN_EN -DPLATFORM_A7 -DBB_PROGRAM_BB #-DGE_PROFILE #-DLEON_NO_MOVE_RODATA #-DLEON_NO_MOVE_FTEXT -DLEON_NO_MOVE_DATA -DLEON_NO_MOVE_ATEXT -DLEON_NO_MOVE_TTEXT -DLEON_NO_CLEAR_BSS
ifeq ($(BB_ISO),1)
  DEFNFLAGS += -DBB_ISO
endif

ifeq ($(BB_USB),1)
  DEFNFLAGS += -DBB_USB
endif
ifeq ($(PLUG_TEST),1)
      DEFNFLAGS += -DPLUG_TEST
  endif
# -pipe just gets GCC to use pipes between the preprocessor, compiler and assembler, instead of temp files
OTHERCFLAGS=-pipe
LANGUAGEFLAGS=-std=gnu99
export CFLAGS=${OPTIMIZATIONFLAGS} ${WARNINGFLAGS} ${INCLUDEFLAGS} ${MACHINEFLAGS} ${DEFNFLAGS} ${OTHERCFLAGS} ${LANGUAGEFLAGS}

# Linker flags
LDFLAGS_WO_LD_SCRIPT=-nostartfiles -msoft-float -Wl,--gc-sections ${CFLAGS} -Wl,--sort-section=alignment
LDFLAGS_ROM_SYMS=-Wl,-R -Wl,$(shell pwd)/blackbird_rom.syms.elf
#export LDFLAGS=-T$(TOP_LEVEL_COMPONENTS_DIR)/leon/build/leon.ld -Tblackbird_flash.ld $(LDFLAGS_WO_LD_SCRIPT) $(LDFLAGS_ROM_SYMS)
export LDFLAGS=-T$(TOP_LEVEL_COMPONENTS_DIR)/leon/build/leon.ld -Tprogram_ram.ld $(LDFLAGS_WO_LD_SCRIPT) $(LDFLAGS_ROM_SYMS)
export LDFLAGS_TEST_HARNESS=-T$(TOP_LEVEL_COMPONENTS_DIR)/leon/build/leon.ld -T$(TOP_LEVEL_COMPONENTS_DIR)/leon/build/leon_ram.ld $(LDFLAGS_WO_LD_SCRIPT) $(LDFLAGS_ROM_SYMS)
export FLASH_WRITER_LD_FLAGS=-T$(TOP_LEVEL_COMPONENTS_DIR)/leon/build/leon.ld -T$(TOP_LEVEL_COMPONENTS_DIR)/leon/build/leon_ram.ld $(LDFLAGS_WO_LD_SCRIPT)


# This defines the version of ibuild that we are configured to use
# a future version of ibuild may require different variables to be set
export IBUILD_PROJECT_MAK_CONFIGURED_VERSION=1

# Note the location of the rules.mak file for all the components
export IBUILD_RULES=$(TOP_LEVEL_COMPONENTS_DIR)/ibuild/build/rules.mak

# Have iLog use the leon backend
export ILOG_BACKEND=LEON

# where is the project_components file
export PROJECT_COMPONENTS=$(shell pwd)/../../inc/project_components.h
export ICRON_HEADER_PROJECT=BLACKBIRD

export RTL_DIR_MAVERICK=/data/engdev/designs/blackbird/released/dd/BLACKBIRD_20191024T221222/src

export RTL_DIR_RAVEN=/data/engdev/designs/blackbird/released/dd/BLACKBIRD_20191024T221222/src
