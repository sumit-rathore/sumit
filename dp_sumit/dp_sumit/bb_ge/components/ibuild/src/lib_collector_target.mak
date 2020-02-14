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
##!   @file  -  lib_collector_target.mak
##
##!   @brief -  Provides the lib collector specific target information to build_rules.mak
##
##
##!   @note  -  This is an ibuild internal file
##
###############################################################################

itarget_lib_collector=true

targ_dir = ../$(PROJECT)_lib
# The following:
#   removes the old library
#   loops through each component and works in the obj_dir
#       extracts all the objects for that component
#       prints all the objects for that component
#   pipes the output to
#       awk which adds the path to each object name
#   pipes the output to
#       xargs which creates the normal archive from, but appends objects from stdin
targ_build= $(QUIET_AR)$(RM) $@; \
        tmp_objs=$$( \
            for p in $(COMPONENT_DEPS); do ( \
                cd $(obj_dir) && \
                if [ -f $(TOP_LEVEL_COMPONENTS_DIR)/$$p/$(PROJECT)_lib/lib$$p.a ]; \
                then \
                    $(AR) x $(TOP_LEVEL_COMPONENTS_DIR)/$$p/$(PROJECT)_lib/lib$$p.a && \
                    $(AR) t $(TOP_LEVEL_COMPONENTS_DIR)/$$p/$(PROJECT)_lib/lib$$p.a; \
                fi; \
            ); done | \
                $(AWK) '{print "$(obj_dir)/"$$0}';); \
        $(AR) crs $@ $(OBJS_W_DIR) $$tmp_objs; \
        $(RM) $$tmp_objs

# lib_collector support for icmd will not parse sub-components, but instead only operate at this level
targ_build2 = $(if $(OBJS_NOT_FOR_LIBS_W_DIR), $(call QUIET_CP, $(OBJS_NOT_FOR_LIBS))$(CP) $(OBJS_NOT_FOR_LIBS_W_DIR) $(dir $@))
           
COMPONENT_DEPS_BUILD=$(addsuffix .component, $(COMPONENT_DEPS)) $(obj_dir_stamp)
# Force the rebuild of the target, such that we will be forced to update all of the components
.PHONY: $(targ_dir)/$(TARGET)

