# a project.mak example file.  This file should be copied to the top level project component build directory

#Define the project name and the component directory
export PROJECT=binary_stream_ilog_parsing
export TOP_LEVEL_COMPONENTS_DIR=$(shell pwd)/../../../../


# Define all of the following tools
# Note: CROSS is only a convience for this file only
CROSS=
export AR=${CROSS}ar
export CC=${CROSS}gcc
export LD=${CROSS}gcc
export AS=${CROSS}as
export OBJCOPY=${CROSS}objcopy
export MKDIR=mkdir -p
export RMDIR=rmdir
export RM=rm -f
export SED=sed
export AWK=awk
export CP=cp


# Compiler flags: broken up into different groups for convience, only CFLAGS is exported
WARNINGFLAGS=-Wall
# Used by LG1
# -Werror -Wextra -Wno-unused-parameter -Wdeclaration-after-statement \
# -Waggregate-return -Wswitch-default -Wmissing-noreturn -Wmissing-format-attribute \
# -Wundef -Wformat-nonliteral -Wwrite-strings -Wshadow -Wpointer-arith -Winline \
# -Wformat-security -Winit-self -Wmissing-include-dirs -Wnested-externs -Wredundant-decls
#Useful warnings, but cause too many issues with the current LG1 code
# -Wunreachable-code -- All of the logging levels that are compiled out, are returned as unreachable code
# -Wmissing-declarations  -- This causes issues with the spectra reg code
# -Wcast-align  -- Causes issues with AsicRegisters.h, as there are no checks for alignment in this file
# -Wswitch-enum  -- Very useful, but currently has too many warnings that need to be fixed
OPTIMIZATIONFLAGS=-Os
# Note: that each components "/inc/" directory is automatically included
#       This is only for an additional project component include directory
#       It should normally be left empty
#
#       A use case for using it would be a top level include directory for an options.h
#       Using "-I$(shell pwd)/../inc -include options.h" from the top level /build could be
#       easier than including a bunch of DEFNFLAGS for the whole project
INCLUDEFLAGS=-I$(shell pwd)/../inc
# Note: changing the ABI would require a "make clean" afterwards
MACHINEFLAGS=-fshort-enums
# Note: changing DEFNFLAGS requries the user to do a "make clean" afterwards
DEFNFLAGS=-DIENDIAN=0
export CFLAGS=${OPTIMIZATIONFLAGS} ${WARNINGFLAGS} ${INCLUDEFLAGS} ${MACHINEFLAGS} ${DEFNFLAGS} -g

# Ensure that the icmd won't be built
NO_ICMD_SUPPORT=1

# Linker flags
export LDFLAGS=

# Have ilog use our test harness functions
export ILOG_BACKEND=TEST_HARNESS

# This defines the version of ibuild that we are configured to use
# a future version of ibuild may require different variables to be set
export IBUILD_PROJECT_MAK_CONFIGURED_VERSION=1

# Note the location of the rules.mak file for all the components
export IBUILD_RULES=$(TOP_LEVEL_COMPONENTS_DIR)/ibuild/build/rules.mak

# where is the project_components file
export PROJECT_COMPONENTS=../inc/project_components.h

