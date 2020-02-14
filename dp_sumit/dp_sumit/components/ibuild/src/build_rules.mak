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
##!   @file  -  build_rules.mak
##
##!   @brief -  Contains all of the rules ibuild uses for building various files
##
##
##!   @note  -  This is an ibuild internal file
##
###############################################################################


#Helper Utilility function to convert from Unix forward slashes to Dos back slashes
_IBUILD_FixPathLinux=$(subst /,\/,$(1))
_IBUILD_FixPathWindows=$(subst /,\\,$(1))
#TODO: We currently hard code this to Linux.  Need to make a cmdline parameter later
_IBUILD_FixPath=_IBUILD_FixPathLinux


# define C Flag rule to help with building icmd object files
## ASM table builder args parser
## This builds an array of all of the icmd functions for this component
## This is done in ASM, as C would need us to declare each function first
## TODO: .align 4 and .long are SPARC specific (and perhaps a bunch of other 32 bit microcontrollers)
## NOTE: This is documented in readme.txt in icmd

_IBUILD_IcmdObjCFlags='-DICMD_PARSER_PREFIX(x)=.global icmd_ \#\# x; .section ".rodata"; .align 4; icmd_ \#\# x:' '-DICMD_PARSER(x, y, z...)=.long x' '-DICMD_PARSER_POSTFIX(x)='


#Our default goal
$(TARGET): $(targ_dir)/$(TARGET)

# update the icmd file
# run through the preprocessor with our special define, and then remove comments and empty lines
# Then if this is an elf target or a library collector collect all the icmd.component files and output either an icmd or icmd.component file
ibuild_update_icmd: $(targ_dir_stamp) $(COMPONENT_DEPS_BUILD)
	$(QUIET)$(RM) $(targ_dir)/icmd $(targ_dir)/icmd.component
	$(if $(COMPONENT_ICMD_FILE), \
	    $(QUIET_ICMD)$(CC) $(iCFLAGS) \
	        '-DICMD_PARSER_PREFIX(x)=component:x' '-DICMD_PARSER(x,y,z...)=F:x H:y A:z' '-DICMD_PARSER_POSTFIX(x)=' \
	        -DCOMPONENT_PARSER_PREFIX= '-DCOMPONENT_PARSER(x)=' -DCOMPONENT_PARSER_POSTFIX= -E $(src_dir)/$(COMPONENT_ICMD_FILE) | \
	        $(AWK) '!/((^\#)|(^[\t ]*$$))/{sub(/^[\t ]*/, ""); sub(/[ \t]*$$/, ""); print}' > $(targ_dir)/icmd.component; )
	$(if $(itarget_elf), $(QUIET_ICMD_MAIN) \
	    icmd_files="$(if $(COMPONENT_ICMD_FILE),$(targ_dir)/icmd.component) \
	                $(strip $(foreach component,$(COMPONENT_DEPS), $(shell ls $(TOP_LEVEL_COMPONENTS_DIR)/$(component)/$(PROJECT)_lib/icmd.component 2>/dev/null))) \
	                $(strip $(foreach component,$(COMPONENT_DEPS), $(shell ls $(TOP_LEVEL_COMPONENTS_DIR)/$(component)/$(PROJECT)_bin/icmd.component 2>/dev/null)))"; \
	    for icmd_file in $$icmd_files; do cat $$icmd_file >> $(targ_dir)/$(if $(itarget_elf),icmd,icmd.component); done )


# Update the host ilog file
# run through the preprocessor with our special define, and then remove comments and empty lines
# Then if this is an elf target or a library collector collect all the ilog.component files and output either an ilog or ilog.component file
ibuild_update_ilog: $(targ_dir_stamp) $(COMPONENT_DEPS_BUILD)
	$(QUIET)$(RM) $(targ_dir)/ilog $(targ_dir)/ilog.component
	$(if $(COMPONENT_ILOG_FILE), \
	    $(QUIET_ILOG)$(CC) $(iCFLAGS) \
	        '-DILOG_PARSER_PREFIX(x)=component:x' '-DILOG_PARSER(x,y)=L:x S:y' '-DILOG_PARSER_POSTFIX(x,y)=' \
	        -DCOMPONENT_PARSER_PREFIX= '-DCOMPONENT_PARSER(x)=' -DCOMPONENT_PARSER_POSTFIX= -E $(COMPONENT_ILOG_FILE) | \
	        $(AWK) '!/((^\#)|(^[\t ]*$$))/{sub(/^[\t ]*/, ""); sub(/[ \t]*$$/, ""); sub(/ S:\"/, " S:"); sub(/\"$$/, ""); print}' > $(targ_dir)/ilog.component; )
	$(if $(or $(itarget_lib_collector), $(itarget_elf)), $(QUIET_ILOG_MAIN) \
	    ilog_files="$(if $(COMPONENT_ILOG_FILE),$(targ_dir)/ilog.component) \
	                $(strip $(foreach component,$(COMPONENT_DEPS), $(shell ls $(TOP_LEVEL_COMPONENTS_DIR)/$(component)/$(PROJECT)_lib/ilog.component 2>/dev/null))) \
	                $(strip $(foreach component,$(COMPONENT_DEPS), $(shell ls $(TOP_LEVEL_COMPONENTS_DIR)/$(component)/$(PROJECT)_bin/ilog.component 2>/dev/null)))"; \
	    for ilog_file in $$ilog_files; do cat $$ilog_file >> $(targ_dir)/$(if $(itarget_elf),ilog,ilog.component); done )


# update the icmdresp file
# run through the preprocessor with our special define, and then remove comments and empty lines
# Then if this is an elf target or a library collector collect all the icmd.component files and output either an icmd or icmd.component file
ibuild_update_icmdresp: $(targ_dir_stamp) $(COMPONENT_DEPS_BUILD)
	$(QUIET)$(RM) $(targ_dir)/icmdresp $(targ_dir)/icmdresp.component
	$(if $(COMPONENT_ICMDRESP_FILE), \
	    $(QUIET_ICMDRESP)$(CC) $(iCFLAGS) \
	        '-DICMDRESP_PARSER_PREFIX(x)=C:x' '-DICMDRESP_PARSER(n,c,l,a)=R:n C:c L:l A:a' '-DICMDRESP_PARSER_POSTFIX(x)=' \
	        -DCOMPONENT_PARSER_PREFIX= '-DCOMPONENT_PARSER(x)=' -DCOMPONENT_PARSER_POSTFIX= -E $(src_dir)/$(COMPONENT_ICMDRESP_FILE) | \
	        $(AWK) '!/((^\#)|(^[\t ]*$$))/{sub(/^[\t ]*/, ""); sub(/[ \t]*$$/, ""); print}' > $(targ_dir)/icmdresp.component; )
	$(if $(itarget_elf), $(QUIET_ICMDRESP_MAIN) \
	    icmdresp_files="$(if $(COMPONENT_ICMDRESP_FILE),$(targ_dir)/icmdresp.component) \
	                $(strip $(foreach component,$(COMPONENT_DEPS), $(shell ls $(TOP_LEVEL_COMPONENTS_DIR)/$(component)/$(PROJECT)_lib/icmdresp.component 2>/dev/null))) \
	                $(strip $(foreach component,$(COMPONENT_DEPS), $(shell ls $(TOP_LEVEL_COMPONENTS_DIR)/$(component)/$(PROJECT)_bin/icmdresp.component 2>/dev/null)))"; \
	    for icmdresp_file in $$icmdresp_files; do cat $$icmdresp_file >> $(targ_dir)/$(if $(itarget_elf),icmdresp,icmdresp.component); done )

# Create the icomponent file for elf targets
# run through the preprocessor with our special define, and then remove comments and empty lines
ibuild_update_icomponent: $(targ_dir_stamp)
	$(QUIET)$(RM) $(targ_dir)/icomponent
	$(if $(and $(PROJECT_COMPONENTS), $(itarget_elf)), \
		$(QUIET_ICOMP)$(CC) $(iCFLAGS) '-DCOMPONENT_PARSER_PREFIX=components:' '-DCOMPONENT_PARSER(x)=C:x' '-DCOMPONENT_PARSER_POSTFIX=' -E $(PROJECT_COMPONENTS) | \
			$(AWK) '!/((^\#)|(^[\t ]*$$))/{sub(/^[\t ]*/, ""); sub(/[ \t]*$$/, ""); print}' > $(targ_dir)/icomponent;)

# Create dependency graph
$(targ_dir)/$(IBUILD_DEP_GRAPH_PNG): $(obj_dir)/$(IBUILD_DEP_GRAPH_PNG).dot $(targ_dir_stamp)
	    $(QUIET_PNG)dot -Tpng $< -o $@

# Return the list of dependencies to fd 99, why 99, well make seems to make assumptions in -j X mode, and it closes 3, before calling sh
_ibuild_return_deps:
	@echo -n "$(COMPONENT_DEPS)" >&99

.PHONY: _ibuild_return_deps

# Create a csv file, well not a real csv with commas, but a list of each component and all of its dependencies
# This will filter out undesired nodes, but not deps
$(obj_dir)/$(IBUILD_DEP_GRAPH_PNG).csv: $(obj_dir_stamp) $(addsuffix .component, $(COMPONENT_DEPS))
	for dep in $(filter-out $(IBUILD_DEP_GRAPH_EXCLUDES), $(COMPONENT_DEPS));								\
	do																										\
		deps=$$($(MAKE) -C $(TOP_LEVEL_COMPONENTS_DIR)/$$dep/build _ibuild_return_deps 99>&1 >/dev/null);	\
		echo -e "$$dep\t: $$deps";																			\
	done > $@

# Produce a .dot file for describing a graph
$(obj_dir)/$(IBUILD_DEP_GRAPH_PNG).dot: $(obj_dir)/$(IBUILD_DEP_GRAPH_PNG).csv $(obj_dir_stamp)
	$(QUIET)edges=$$(cat $< | while read node sep deps; do											\
			for p in $$deps; do																		\
				echo "$(IBUILD_DEP_GRAPH_EXCLUDES)" | grep $$p > /dev/null || echo "$$node -> $$p";	\
			done; 																					\
		done);																						\
	nodes=$$(cat $< | cut -f1 | sed 's/.*/& [label="&"]/');											\
	(																								\
	 	echo 'digraph {' &&																			\
		echo "$$nodes" &&																			\
		echo "$$edges" &&																			\
		echo '}';																					\
	) > $@



# Rule to build tests
# This builds the current tests, and calls test on each component.  To prevent infinite recursion this only happens once by value of BUILD_DEPS
ifneq ("$(BUILD_TEST_DEPS)", "1")
test: $(addsuffix .test, $(TEST_HARNESSES)) $(addsuffix .component_test, $(COMPONENT_DEPS))
else
test: $(addsuffix .test, $(TEST_HARNESSES))
endif

# Build the tests in the current component
%.test:
	$(QUIET_MAKE_TEST)BUILD_DEPS=0 IBUILD_BUILDING_TEST_HARNESS=1 LDFLAGS="$(LDFLAGS_TEST_HARNESS)" $(MAKE) -C ../test/$(@:.test=)/build

# Build the tests for each component
%.component_test:
	$(QUIET_MAKE_TESTS)$(MAKE) -C $(TOP_LEVEL_COMPONENTS_DIR)/$(@:.component_test=)/build BUILD_TEST_DEPS=1 test


.PHONY: ibuild_update_icomponent ibuild_update_ilog ibuild_update_icmdresp test

# Build the target
$(targ_dir)/$(TARGET): $(OBJS_W_DIR) $(targ_dir_stamp) $(COMPONENT_DEPS_BUILD) ibuild_update_icomponent ibuild_update_ilog ibuild_update_icmd ibuild_update_icmdresp $(if $(IBUILD_DEP_GRAPH_PNG), $(targ_dir)/$(IBUILD_DEP_GRAPH_PNG))
	$(targ_build)
	$(targ_build2)



# List all .i .s .flags files as secondary files so they won't get auto deleted
.SECONDARY: $(I_W_DIR) $(S_W_DIR) $(CFLAG_FILES_W_DIR)

# Rule to build components
# Unless BUILD_DEPS is set to 0
%.component:
ifneq ($(BUILD_DEPS),0)
	$(QUIET_MAKE_COMP)$(MAKE) -C $(TOP_LEVEL_COMPONENTS_DIR)/$(@:.component=)/build
else
	@true
endif

# Create a compiler flags file from a C file
$(obj_dir)/%.flags: $(PRE_BUILD_DEPS) $(src_dir)/%.c IBUILD_FORCE $(obj_dir_stamp)
	@echo $(iCFLAGS) $($(notdir $<)_CFLAGS) | diff - $@ >/dev/null 2>&1 || echo $(iCFLAGS) $($(notdir $<)_CFLAGS) > $@

# Create a compiler flags file from a S file
$(obj_dir)/%.flags: $(PRE_BUILD_DEPS) $(src_dir)/%.S IBUILD_FORCE $(obj_dir_stamp)
	@echo $(iCFLAGS) $($(notdir $<)_CFLAGS) | diff - $@ >/dev/null 2>&1 || echo $(iCFLAGS) $($(notdir $<)_CFLAGS) > $@

.PHONY: IBUILD_FORCE 


# Non Debug Builds
ifndef D # {

ifneq ($(COMPONENT_ICMD_FILE),)
# Build .o file from COMPONENT_ICMD_FILE
$(obj_dir)/$(COMPONENT_ICMD_FILE:%.h=%.o): $(src_dir)/$(COMPONENT_ICMD_FILE) $(obj_dir)/$(COMPONENT_ICMD_FILE:%.h=%.d) $(obj_dir_stamp)
	@# First delete any files that could be left over from a debug build
	-$(QUIET)$(RM) $(obj_dir)/$(notdir $(<:%.h=%.s))
	$(QUIET_CC)$(CC) $(iCFLAGS) -x assembler-with-cpp $(_IBUILD_IcmdObjCFlags) -c -o $@ $<
endif



# Build object files from C files
$(obj_dir)/%.o: $(src_dir)/%.c $(obj_dir)/%.d $(obj_dir_stamp)
	@# First delete any files that could be left over from a debug build
	-$(QUIET)$(RM) $(obj_dir)/$(notdir $(<:%.c=%.i)) $(obj_dir)/$(notdir $(<:%.c=%.s))
	$(QUIET_CC)$(CC) $(iCFLAGS) $($(notdir $<)_CFLAGS) -c -o $@ $<


ifneq ($(COMPONENT_ICMD_FILE),)
# Build .d file from COMPONENT_ICMD_FILE
$(obj_dir)/$(COMPONENT_ICMD_FILE:%.h=%.d): $(src_dir)/$(COMPONENT_ICMD_FILE) $(obj_dir_stamp)
	$(QUIET_DEP)$(CC) -M $(iCFLAGS) -x assembler-with-cpp $(_IBUILD_IcmdObjCFlags) $< | \
	    $(SED) 's/$(COMPONENT_ICMD_FILE:%.h=%.o)/$(call $(_IBUILD_FixPath),$(obj_dir)/)& $(call $(_IBUILD_FixPath),$(obj_dir)/)$(COMPONENT_ICMD_FILE:%.h=%.d)/g' > $@
endif

# Create dependency rules for C files
$(obj_dir)/%.d: $(src_dir)/%.c $(obj_dir)/%.flags $(obj_dir_stamp)
	$(QUIET_DEP)$(CC) -M $(iCFLAGS) $($(notdir $<)_CFLAGS) $< | \
	    $(SED) 's/$(notdir $*)\.o/$(call $(_IBUILD_FixPath),$(obj_dir)/)& $(call $(_IBUILD_FixPath),$(obj_dir)/)$(notdir $*).d/g' > $@

# Build object files from S files
$(obj_dir)/%.o: $(src_dir)/%.S $(obj_dir)/%.d $(obj_dir_stamp)
	@# First delete any files that could be left over from a debug build
	-$(QUIET)$(RM) $(obj_dir)/$(notdir $(<:%.S=%.s))
	$(QUIET_CC)$(CC) $(iCFLAGS) $($(notdir $<)_CFLAGS) -c -o $@ $<

# Create dependency rules for S files
$(obj_dir)/%.d: $(src_dir)/%.S $(obj_dir_stamp)
	$(QUIET_DEP)$(CC) -M $(iCFLAGS) $($(notdir $<)_CFLAGS) $< | \
	    $(SED) \
	        -e 's/$(notdir $*)\.o/$(call $(_IBUILD_FixPath),$(obj_dir)/)& $(call $(_IBUILD_FixPath),$(obj_dir)/)$(notdir $*).d/g' \
	        -e '$$s/$$/ $(call $(_IBUILD_FixPath),$($(notdir $<)_DEPS))/'> $@

else # Build a debug build } else {

# Build object files from preproccessed files
$(obj_dir)/%.o: $(obj_dir)/%.s $(obj_dir_stamp)
	$(QUIET_CC)$(CC) $(iCFLAGS) $($(notdir $(<:%.s=%.S))_CFLAGS) -c -o $@ $<

# Build preprocessed asm files from asm files
# on error delete the file so the next invocation of make doesn't think it was successfully created and is newer than dependencies
# also ensure errors return a unsuccessful error code, so make doesn't continue
$(obj_dir)/%.s: $(src_dir)/%.S $(obj_dir_stamp)
	$(QUIET_CC)$(CC) $(iCFLAGS) $($(notdir $<)_CFLAGS) $< -E > $@ || ( rm -f $@; false)


ifneq ($(COMPONENT_ICMD_FILE),)
# Build .s file from COMPONENT_ICMD_FILE
$(obj_dir)/$(COMPONENT_ICMD_FILE:%.h=%.s): $(src_dir)/$(COMPONENT_ICMD_FILE) $(obj_dir)/$(COMPONENT_ICMD_FILE:%.h=%.d) $(obj_dir_stamp)
	$(QUIET_CC)$(CC) $(iCFLAGS) -x assembler-with-cpp $(_IBUILD_IcmdObjCFlags) $< -E > $@ || ( rm -f $@; false)
endif

# Build preproccessed files from C files
# on error delete the file so the next invocation of make doesn't think it was successfully created and is newer than dependencies
# also ensure errors return a unsuccessful error code, so make doesn't continue
$(obj_dir)/%.i: $(src_dir)/%.c $(obj_dir)/%.d $(obj_dir_stamp)
	$(QUIET_CC)$(CC) $(iCFLAGS) $($(notdir $<)_CFLAGS) $< -E > $@ || ( rm -f $@; false)


ifneq ($(COMPONENT_ICMD_FILE),)
# Build .d file from COMPONENT_ICMD_FILE
$(obj_dir)/$(COMPONENT_ICMD_FILE:%.h=%.d): $(src_dir)/$(COMPONENT_ICMD_FILE) $(obj_dir_stamp)
	$(QUIET_DEP)$(CC) -M $(iCFLAGS) -x assembler-with-cpp $(_IBUILD_IcmdObjCFlags) $< | \
	    $(SED) 's/$(COMPONENT_ICMD_FILE:%.h=%.o)/$(call $(_IBUILD_FixPath),$(obj_dir)/)$(notdir $*)\.i $(call $(_IBUILD_FixPath),$(obj_dir)/)$(COMPONENT_ICMD_FILE:%.h=%.d)/g' > $@
endif

# Create dependency rules for C files
$(obj_dir)/%.d: $(src_dir)/%.c $(obj_dir)/%.flags $(obj_dir_stamp)
	$(QUIET_DEP)$(CC) -M $(iCFLAGS) $($(notdir $<)_CFLAGS) $< | \
	    $(SED) 's/$(notdir $*)\.o/$(call $(_IBUILD_FixPath),$(obj_dir)/)$(notdir $*).i $(call $(_IBUILD_FixPath),$(obj_dir)/)$(notdir $*).d/g' > $@

# Create dependency rules for S files
$(obj_dir)/%.d: $(src_dir)/%.S $(obj_dir_stamp)
	$(QUIET_DEP)$(CC) -M $(iCFLAGS) $($(notdir $<)_CFLAGS) $< | \
	    $(SED) \
	        -e 's/$(notdir $*)\.o/$(call $(_IBUILD_FixPath),$(obj_dir)/)$(notdir $*).s $(call $(_IBUILD_FixPath),$(obj_dir)/)$(notdir $*).d/g' \
	        -e '$$s/$$/ $(call $(_IBUILD_FixPath),$($(notdir $<)_DEPS))/'> $@

endif # } Debug/not debug build


# Build object files from preproccessed files
$(obj_dir)/%.s: $(obj_dir)/%.i $(obj_dir_stamp)
	$(QUIET_CC)$(CC) $(iCFLAGS) $($(notdir $(<:%.i=%.c))_CFLAGS) -S -o $@ $<

# Build the object directory
$(obj_dir_stamp):
	$(QUIET_MKDIR)$(MKDIR) $(obj_dir)
	@touch "$@"

# Build the target directory
$(targ_dir_stamp):
	$(QUIET_MKDIR)$(MKDIR) $(targ_dir)
	@touch "$@"

.PHONY: $(PRE_BUILD_DEPS)

# Don't start running PRE_BUILD_DEPS' makefile if already started
# We need to manually create these folders, similar to building the target and
# object directories as described above, however, we're not executing the
# makefile of the PRE_BUILD dependency yet - which means all context (TARGET,
# targ_dir etc) pertains to the depender (whatever submodule has PRE_BUILD_DEPS
# defined and happens to be called by Make first).
# Also note that this isn't a typical target which would follow the rule
# $(targ_dir)/$(TARGET) so it can't use any normal built in stuff like libs or
# elf targets do.
$(PRE_BUILD_DEPS):
ifeq ($(wildcard $(TOP_LEVEL_COMPONENTS_DIR)/$(@)/$(@)_lib/$(@)),)
	$(QUIET_MKDIR)$(MKDIR) $(TOP_LEVEL_COMPONENTS_DIR)/$(@)/$(@)_lib
#	Use flock on make, this allows make to prepare in parallel but execute this particular
#	process, building @, in serial mode, all else runs in parellel.
#	Override all variables assigned in makefile to prevent attemps to build
#	library, this will execute only the pre_build target, nothing else.
	(flock $(TOP_LEVEL_COMPONENTS_DIR)/$(@)/$(@)_lib/.makelock -c "$(MAKE) pre_build -C $(TOP_LEVEL_COMPONENTS_DIR)/$(@)/build " )
endif
	@touch $(TOP_LEVEL_COMPONENTS_DIR)/$(@)/$(@)_lib/$(@)

