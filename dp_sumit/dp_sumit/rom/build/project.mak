# project.mak file for the Blackbird ROM

#Define the project name and the component directory
export PROJECT=blackbird_rom

# pwd <p_bb_sw>/rom/build
export TOP_LEVEL_COMPONENTS_DIR=$(shell pwd)/../../components


# Define all of the following tools
# Note: CROSS is only a convience for this file only
CROSS=sparc-elf-
export AR=${CROSS}ar
export CC=${CROSS}gcc
export LD=${CROSS}gcc
export AS=${CROSS}as
export OBJCOPY=${CROSS}objcopy
export OBJDUMP=${CROSS}objdump
export STRIP=${CROSS}strip
export MKDIR=mkdir -p
export RMDIR=rmdir
export RM=rm -f
export SED=sed
export AWK=awk
export CP=cp
export HOSTCC=gcc


# Compiler flags: broken up into different groups for convience, only CFLAGS is exported
WARNINGFLAGS=-Wall \
 -Werror -Wextra -Wno-type-limits -Wno-unused-parameter \
 -Wswitch-default  -Wmissing-format-attribute \
 -Wundef -Wformat-nonliteral -Wwrite-strings -Wshadow -Wpointer-arith -Winline \
 -Wformat-security -Winit-self -Wmissing-include-dirs -Wnested-externs -Wswitch-enum
#Useful warnings, but cause too many issues with the current LG1 code
# -Wredundant-decls -- Causes issues with iCmd, as we try and ensure that our version of the declaration is correct
# -Wunreachable-code -- All of the logging levels that are compiled out, are returned as unreachable code
# -Wmissing-declarations  -- This causes issues with the spectra reg code
# -Wcast-align  -- Causes issues with AsicRegisters.h, as there are no checks for alignment in this file
OPTIMIZATIONFLAGS=-Os -fweb -frename-registers -ffunction-sections -fdata-sections \
 -fsched-pressure  -free -fno-toplevel-reorder

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
# Note: changing DEFNFLAGS requries the user to do a "make clean" afterwards
#For big endian projects using ilog, and use the options.h include file
DEFNFLAGS=-DIENDIAN=1 -DUSE_OPTIONS_H -DBLACKBIRD -DLEON_BOOT_ROM -DIDS_BIG_ENDIAN -DPLATFORM_A7 -DBB_ROM
# -pipe just gets GCC to use pipes between the preprocessor, compiler and assembler, instead of temp files
OTHERCFLAGS=-pipe

LANGUAGEFLAGS=-std=gnu99
ifeq ($(AHBROM),1)
DEFNFLAGS+=-DTEST_ROM
endif

export CFLAGS=${OPTIMIZATIONFLAGS} ${WARNINGFLAGS} ${INCLUDEFLAGS} ${MACHINEFLAGS} ${DEFNFLAGS} ${OTHERCFLAGS} ${LANGUAGEFLAGS}



# Linker flags
ifeq ($(AHBROM),1)
export LDFLAGS=-nostartfiles -T$(TOP_LEVEL_COMPONENTS_DIR)/leon/build/leon.ld -Tahbram_leon_rom.ld -Tignored.ld \
               -msoft-float -Wl,--warn-common,--gc-sections ${CFLAGS} -Wl,--sort-section=alignment
else
export LDFLAGS=-nostartfiles -T$(TOP_LEVEL_COMPONENTS_DIR)/leon/build/leon.ld -Tleon_rom.ld -Tignored.ld \
               -msoft-float -Wl,--warn-common,--gc-sections ${CFLAGS} -Wl,--sort-section=alignment
endif

# This defines the version of ibuild that we are configured to use
# a future version of ibuild may require different variables to be set
export IBUILD_PROJECT_MAK_CONFIGURED_VERSION=1

# Note the location of the rules.mak file for all the components
export IBUILD_RULES=$(shell pwd)/../../components/ibuild/build/rules.mak

# Have iLog use the leon backend
export ILOG_BACKEND=LEON

export NO_ICMD_SUPPORT=1

export RTL_DIR=/data/engdev/designs/blackbird/sandbox/dd/ROM_RLS_20190411T091758/src

