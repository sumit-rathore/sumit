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
##!   @file  -  verbosity.mak
##
##!   @brief -  This includes all the variable definitions for displaying
##              pretty progress information
##
##
##!   @note  -  This is an ibuild internal file
##
###############################################################################

ifneq ($(findstring $(MAKEFLAGS),s),s)
ifndef V
    QUIET_CC            = @echo '    CC           ' $(notdir $@);
    QUIET_DEP           = @echo '    DEP          ' $(notdir $@);
    QUIET_LD            = @echo '    LD           ' $(notdir $@);
    QUIET_AR            = @echo '    AR           ' $(notdir $@);
    QUIET_MKDIR         = @echo '    MKDIR        ' $(dir $@);
    QUIET_CP            = @echo '    CP           ' $(1);
    QUIET_CLEAN_COMP    = @echo '    CLEAN        ' $1;
    QUIET_CLEAN_TEST    = @echo '    CLEAN TEST   ' $1;
    QUIET_RM            = @echo '    RM           ' $1;
    QUIET_RMDIR         = @echo '    RMDIR        ' $1;
    QUIET_MAKE_COMP     = @echo -e '\n    MAKE         ' $(@:%.component=%); 
    QUIET_MAKE_TESTS    = @echo -e '\n    MAKE TESTS   ' $(@:%.component_test=%); 
    QUIET_MAKE_TEST     = @echo -e '\n    MAKE TEST    ' $(@:.test=);
    QUIET_ICOMP         = @echo '    ICOMPONENTS  ' $(notdir $(PROJECT_COMPONENTS));
    QUIET_ILOG          = @echo '    ILOG         ' $(COMPONENT_ILOG_FILE);
    QUIET_ILOG_MAIN     = @echo '    ILOG MAIN    ' $(TARGET);
    QUIET_ICMD          = @echo '    ICMD         ' $(COMPONENT_ICMD_FILE);
    QUIET_ICMD_MAIN     = @echo '    ICMD MAIN    ' $(TARGET);
    QUIET_PNG           = @echo '    PNG          ' $(IBUILD_DEP_GRAPH_PNG);
    QUIET_ICMDRESP      = @echo '    ICMDRESP     ' $(COMPONENT_ICMDRESP_FILE); 
    QUIET_ICMDRESP_MAIN = @echo '    ICMDRESP MAIN' $(TARGET);
    QUIET_ICRON         = @echo '    ICRON        ' $(TARGET);
    QUIET               = @ 

ifneq ($(findstring $(MAKEFLAGS),w),w)
ifneq ($(findstring $(MAKEFLAGS),--print-directory),--print-directory)
MAKEFLAGS+=--no-print-directory
else
endif
else
endif

endif #ifndef V
endif #was -s not passed on the command line


