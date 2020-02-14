# See the user's guide for how to use this file, and the rules it provides

# Ensure that a project has been configured
ifeq ($(IBUILD_PROJECT_MAK_CONFIGURED_VERSION), "")
$(error "Project make file has not defined build tools!")
endif
# And ensure it is version 1, which is this version of ibuild
ifneq ($(IBUILD_PROJECT_MAK_CONFIGURED_VERSION), 1)
$(warning "configured for $(IBUILD_PROJECT_MAK_CONFIGURED_VERSION)")
$(error "Project make file has defined build tools for a different version of ibuild!")
endif

# Check version of make
# .FEATURES should contain 'else-if'
ifeq ($(findstring else-if, $(.FEATURES)), )
$(error "Please use a modern version of GNU Make. ie at least 3.81")
endif

# use IBUILD_RULES to pull in other helper makefiles
_IBUILD_BUILD_DIR=$(dir $(IBUILD_RULES))
_IBUILD_SRC_DIR=$(_IBUILD_BUILD_DIR)/../src

# Check for any special commands
ifneq ($(NO_ICMD_SUPPORT), ) # check for non-null value
export NO_ICMD_SUPPORT
COMPONENT_ICMD_FILE:=
endif

# Create the variables for sources, flags, and directories
include $(_IBUILD_SRC_DIR)/sources_flags_dirs.mak

# Create the variables for build rules for various targets
ifeq ($(TARGET_TYPE), lib) # {
include $(_IBUILD_SRC_DIR)/lib_target.mak
else # Not a lib TARGET_TYPE # } {
ifeq ($(TARGET_TYPE), lib_collector) # {
include $(_IBUILD_SRC_DIR)/lib_collector_target.mak
else # Not a lib or lib_collector TARGET_TYPE # } {
ifeq ($(TARGET_TYPE), elf) # {
include $(_IBUILD_SRC_DIR)/elf_target.mak
else # Not an elf, lib or lib_collector TARGET_TYPE # } {
ifeq ($(TARGET_TYPE), icron) # {
include $(_IBUILD_SRC_DIR)/icron_target.mak
else # Not a known TARGET_TYPE # } {
#TODO: produce rules for SREC, binary, etc.
#TODO: perhaps a default rule that would just build the dependencies
$(error "TARGET_TYPE not defined!")
#Note: new targets need to define the following
targ_dir =
targ_build =
targ_build2 = 
COMPONENT_DEPS_BUILD=
endif # TARGET_TYPE icron # }
endif # TARGET_TYPE elf # }
endif # TARGET_TYPE lib_collector # }
endif # TARGET_TYPE lib # }

# Verbose settings
include $(_IBUILD_SRC_DIR)/verbosity.mak

#Build rules
include $(_IBUILD_SRC_DIR)/build_rules.mak

# Include all of the depend files
# Unless the goal is to clean, or
#   PROJECT is not yet defined,  ie this is some top level makefile
#                                   & we don't want _obj/ directories everywhere
ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), clean_ibuild)
ifneq ($(MAKECMDGOALS), clean_component)
ifneq ($(PROJECT), )
-include $(DEPS_W_DIR)
endif
endif
endif
endif


# Cleaning rules
include $(_IBUILD_SRC_DIR)/clean.mak

