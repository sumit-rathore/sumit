###############################################################################
##
##   Icron Technology Corporation - Copyright 2010, 2013
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
##!   @file  -  elf_target.mak
##
##!   @brief -  Provides the elf specific target information to build_rules.mak
##
##
##!   @note  -  This is an ibuild internal file
##
###############################################################################

itarget_elf=true

#This searches all the standard components to see which libraries exist
libs_for_linking=$(foreach lib, $(filter-out $(FORCED_LINK_DEPS),$(COMPONENT_DEPS)), $(if $(shell ls $(TOP_LEVEL_COMPONENTS_DIR)/$(lib)/$(PROJECT)_lib/lib$(lib).a 2>/dev/null), $(lib)))

# Find all of the object files that we should directly link in
extra_objs_w_dir=$(foreach dep, $(COMPONENT_DEPS), $(if $(shell ls $(TOP_LEVEL_COMPONENTS_DIR)/$(dep)/$(PROJECT)_lib/*.o 2> /dev/null), \
                                                     $(TOP_LEVEL_COMPONENTS_DIR)/$(dep)/$(PROJECT)_lib/*.o))

targ_dir = ../$(PROJECT)_bin
targ_build =    $(QUIET_LD)$(LD) $(iLDFLAGS) -o $@ $(OBJS_W_DIR) $(extra_objs_w_dir) $(addprefix -l, $(libs_for_linking))
# DO NOT DEFINE targ_build2.  Other targets that depend on the elf_target add their own logic for targ_build2
# targ_build2 =


COMPONENT_DEPS_BUILD=$(addsuffix .component, $(COMPONENT_DEPS))
# Force the rebuild of the target, such that we will be forced to update all of the components
.PHONY: $(targ_dir)/$(TARGET)

