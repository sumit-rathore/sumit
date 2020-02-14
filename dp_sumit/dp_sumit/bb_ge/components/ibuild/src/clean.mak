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
##!   @file  -  clean.mak
##
##!   @brief -  Provides the rules for cleaning a component
##
##
##!   @note  -  This is an ibuild internal file
##
###############################################################################

COMPONENT_CLEAN_RULES=clean_user_files clean_dep_graph_files clean_tmp_files clean_target_files clean_ilog_files clean_icmd_files clean_icmdresp_files clean_icomponent clean_objs_not_for_libs

ifneq ($(BUILD_DEPS), 0)
ifeq ($(TARGET_TYPE), lib_collector)
IBUILD_CLEAN_DEPS=clean_deps
else
ifeq ($(TARGET_TYPE), elf)
IBUILD_CLEAN_DEPS=clean_deps
else
ifeq ($(TARGET_TYPE), icron)
IBUILD_CLEAN_DEPS=clean_deps
endif
endif
endif
endif

# Clean up this component
clean_component: $(IBUILD_CLEAN_DEPS) $(COMPONENT_CLEAN_RULES)
ifeq ($(TARGET_TYPE), icron)
	@# Remove the target elf and binary files
	-$(call QUIET_RM, $(targ_icron_clean_files))$(RM) $(foreach file, $(targ_icron_clean_files), $(targ_dir)/$(file))
endif
	@# Remove the object directory
	-$(call QUIET_RMDIR, $(obj_dir))[ -d $(obj_dir) ] && $(RMDIR) $(obj_dir) || true
	@# Remove the target directory
	-$(call QUIET_RMDIR, $(targ_dir))[ -d $(targ_dir) ] && $(RMDIR) $(targ_dir) || true

clean_user_files: 
	@# Remove user files
	-$(call QUIET_RM, $(TARG_DIR_CLEANUP_FILES))$(RM) $(foreach file, $(TARG_DIR_CLEANUP_FILES), $(targ_dir)/$(file))
	-$(call QUIET_RM, $(OBJ_DIR_CLEANUP_FILES))$(RM) $(foreach file, $(OBJ_DIR_CLEANUP_FILES), $(obj_dir)/$(file))

clean_dep_graph_files: 
	@# Remove deps.* files
	-$(if $(IBUILD_DEP_GRAPH_PNG), $(call QUIET_RM, $(IBUILD_DEP_GRAPH_PNG))$(RM) $(obj_dir)/$(IBUILD_DEP_GRAPH_PNG).csv $(obj_dir)/$(IBUILD_DEP_GRAPH_PNG).dot $(targ_dir)/$(IBUILD_DEP_GRAPH_PNG))

clean_tmp_files: 
	@# Remove s,i,d,o,flag, stamp(not printed) files
	-$(call QUIET_RM, $(SFILES) $(IFILES) $(DEPENDS) $(OBJECTS) $(CFLAG_FILES))$(RM) $(S_W_DIR) $(I_W_DIR) $(DEPS_W_DIR) $(OBJS_W_DIR) $(CFLAG_FILES_W_DIR) $(obj_dir)/.stamp $(targ_dir)/.stamp

clean_objs_not_for_libs:
	@# Remove OBJS_NOT_FOR_LIBS
	-$(if $(OBJS_NOT_FOR_LIBS), $(call QUIET_RM, $(OBJS_NOT_FOR_LIBS))$(RM) $(foreach obj,$(OBJS_NOT_FOR_LIBS),$(targ_dir)/$(obj)))

clean_icomponent:
	@# Remove the icomponent file
	-$(if $(targ_dir),$(call QUIET_RM, icomponent)$(RM) $(targ_dir)/icomponent)

clean_target_files: 
	@# Remove target files
	-$(if $(and $(targ_dir),$(TARGET)),$(call QUIET_RM, $(TARGET))$(RM) $(targ_dir)/$(TARGET))

clean_ilog_files: 
	@# Remove the ilog files
	-$(if $(targ_dir),$(call QUIET_RM, ilog ilog.component)$(RM) $(targ_dir)/ilog $(targ_dir)/ilog.component)

clean_icmd_files: 
	@# Remove the icmd files
	-$(if $(targ_dir),$(call QUIET_RM, icmd icmd.component)$(RM) $(targ_dir)/icmd $(targ_dir)/icmd.component)

clean_icmdresp_files: 
	@# Remove the icmdresp files
	-$(if $(targ_dir),$(call QUIET_RM, icmdresp icmdresp.component)$(RM) $(targ_dir)/icmdresp $(targ_dir)/icmdresp.component)

clean_tests: $(addsuffix .clean_component_test, $(TEST_HARNESSES))

%.clean_component_test:
	$(call QUIET_CLEAN_TEST, $(@:.clean_component_test=))BUILD_DEPS=0 $(MAKE) clean -C ../test/$(@:.clean_component_test=)/build

# Dependency cleans up this component, and then it calls down through all of its dependencies
# If we are a library, then the subcomponents are not cleaned as they would also be dependent on by the linker
# Use as: make clean, or make clean_ibuild
#         Either or just references clean_component, which may call clean_deps as well for elf targets
# In case a makefile wants its own clean rule, declared before ours, it needs a way to call this one
# TODO: This needs fixing.  This should only be defined in clean is not previously defined
# Otherwise we go through and try and delete all the _obj directories first when PROJECT isn't defined
clean: clean_ibuild
clean_ibuild: clean_component clean_tests
clean_deps: $(addsuffix .clean_component, $(COMPONENT_DEPS))

%.clean_component:
	$(call QUIET_CLEAN_COMP, $(@:.clean_component=))$(MAKE) clean -C $(TOP_LEVEL_COMPONENTS_DIR)/$(@:.clean_component=)/build


.PHONY: clean clean_component clean_ibuild

