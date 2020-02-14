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
##!   @file  -  lib_target.mak
##
##!   @brief -  Provides the lib specific target information to build_rules.mak
##
##
##!   @note  -  This is an ibuild internal file
##
###############################################################################

targ_dir = ../$(PROJECT)_lib
targ_build = $(QUIET_AR)$(RM) $@; $(AR) crs $@ $(filter-out $(OBJS_NOT_FOR_LIBS_W_DIR), $(OBJS_W_DIR))
targ_build2 = $(if $(OBJS_NOT_FOR_LIBS_W_DIR), $(call QUIET_CP, $(OBJS_NOT_FOR_LIBS))$(CP) $(OBJS_NOT_FOR_LIBS_W_DIR) $(dir $@))
COMPONENT_DEPS_BUILD=

