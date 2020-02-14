###############################################################################
##
##   Icron Technology Corporation - Copyright 2010
##
##
##   This source file and the information contained in it are confidential and
##   proprietary to Icron Technology Corporation. The reproduction or disclosure,
##   in whole or in part, to anyone outside of Icron without the written approval
##   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
##   Icron who has not previously obtained written authorization for access from
##   the individual responsible for the source code, will have a significant
##   detrimental effect on Icron and is expressly prohibited.
##
###############################################################################
##
##!   @file  -  sources_flags_dirs.mak
##
##!   @brief -  defines the source files, the flags varables, and directories
##
##
##!   @note  -  This is an ibuild internal file
##
###############################################################################

# Determine our list of files
DEPENDS = $(CSOURCES:%.c=%.d) $(ASMSOURCES:%.S=%.d) $(notdir $(COMPONENT_ICMD_FILE:%.h=%.d))
OBJECTS = $(CSOURCES:%.c=%.o) $(ASMSOURCES:%.S=%.o) $(notdir $(COMPONENT_ICMD_FILE:%.h=%.o))
IFILES = $(CSOURCES:%.c=%.i)
SFILES = $(CSOURCES:%.c=%.s) $(ASMSOURCES:%.S=%.s) $(notdir $(COMPONENT_ICMD_FILE:%.h=%.s))
CFLAG_FILES = $(CSOURCES:%.c=%.flags) $(ASMSOURCES:%.S=%.flags)

# Objects that we don't want in a library, they just skip that step
OBJS_NOT_FOR_LIBS=$(notdir $(COMPONENT_ICMD_FILE:%.h=%.o))

# Add the component information to the standard flags
# Also we don't want to change an exported variable, so we have our own copy
iCFLAGS = $(CFLAGS)
iLDFLAGS = $(LDFLAGS)
iCFLAGS += $(addprefix -I$(TOP_LEVEL_COMPONENTS_DIR)/, $(addsuffix /inc, $(COMPONENT_DEPS))) -I../inc
iLDFLAGS += $(addprefix -L$(TOP_LEVEL_COMPONENTS_DIR)/, $(addsuffix /$(PROJECT)_lib, $(COMPONENT_DEPS)))
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
iLDFLAGS += -Wl,-whole-archive $(addprefix -l, $(FORCED_LINK_DEPS)) -Wl,-no-whole-archive
# regular library dependencies are done in the elf target, as missing libraries can then be skipped

# Determine the directories and build functions
obj_dir =./$(PROJECT)_obj
src_dir = ../src


# The following part, needs the target specific part for the full expansion


# Note the stamp files placed in directories
# These are used to note the directories existance without the timestamp
# updating that is caused when new files are placed in the directory
targ_dir_stamp=$(targ_dir)/.stamp
obj_dir_stamp=$(obj_dir)/.stamp

# Determine objects and dependencies with directory
OBJS_W_DIR = $(foreach obj,$(OBJECTS),$(obj_dir)/$(obj))
DEPS_W_DIR = $(foreach dep,$(DEPENDS),$(obj_dir)/$(dep))
OBJS_NOT_FOR_LIBS_W_DIR = $(foreach obj,$(OBJS_NOT_FOR_LIBS),$(obj_dir)/$(obj))
CFLAG_FILES_W_DIR = $(foreach flag,$(CFLAG_FILES),$(obj_dir)/$(flag))

# Debug builds create .i files
I_W_DIR = $(foreach src,$(IFILES), $(obj_dir)/$(src))
# Debug builds create .s files
S_W_DIR = $(foreach src,$(SFILES), $(obj_dir)/$(src))



