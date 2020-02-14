###############################################################################
##
##   Icron Technology Corporation - Copyright 2013
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
##!   @file  -  icron_target.mak
##
##!   @brief -  Provides the icron specific target information to build_rules.mak
##
##
##!   @note  -  This is an ibuild internal file
##
###############################################################################

# Grab all the elf rules
include $(_IBUILD_SRC_DIR)/elf_target.mak

_IBUILD_SED_REMOVING_LEADING_AND_TRAILING_WHITESPACE=sed -r -e 's/^\s*//g' -e 's/\s*$$//g'

#define the rule for a .icron target
ifeq ($(ICRON_HEADER_PROJECT), LIONSGATE) # {

targ_icron_clean_files=icron_header target.elf target.srec target.icr target.bin flash_writer.icr flash_writer.bin ilog icmd icmdresp icomponent $(SPECTAREG_XML_FILES)

# The following cmd, generates all the necessary files (Stewie + binaries), makes the icron_header & tars up everything into a .icron file
targ_build2 = $(QUIET_ICRON)\
	mv $@ $(targ_dir)/target.elf && \
	$(if $(IBUILD_BUILDING_TEST_HARNESS), $(OBJCOPY) -O srec $(targ_dir)/target.elf $(targ_dir)/target.srec &&) \
	$(if $(IBUILD_BUILDING_TEST_HARNESS), srec_cat $(targ_dir)/target.srec -o $(targ_dir)/target.icr -Stewie -Enable_Sequence_Warnings &&) \
	$(if $(IBUILD_BUILDING_TEST_HARNESS),, $(CP) $(TOP_LEVEL_COMPONENTS_DIR)/flash_writer/$(PROJECT)_flash_bin/flash_writer.icr $(targ_dir) &&) \
	$(if $(IBUILD_BUILDING_TEST_HARNESS),, $(CP) $(TOP_LEVEL_COMPONENTS_DIR)/flash_writer/$(PROJECT)_flash_bin/flash_writer.bin $(targ_dir) &&) \
	$(OBJCOPY) -O binary $(targ_dir)/target.elf $(targ_dir)/target.bin && \
	$(if $(SPECTAREG_XML_FILES), $(CP) $(foreach file, $(SPECTAREG_XML_FILES), $(TOP_LEVEL_COMPONENTS_DIR)/spectareg/etc/$(file)) $(targ_dir) &&) \
	echo -e -n "version:7\n"\
	"project:$(if $(IBUILD_BUILDING_TEST_HARNESS),lg1_test,lg1_vhub)\n"\
	"ilog:ilog\n"\
	"icmd:icmd\n"\
	"icomponent:icomponent\n"\
	"icmdresp:icmdresp\n"\
	$(if $(IBUILD_BUILDING_TEST_HARNESS),"test_image:target.icr\n", "flash_writer:flash_writer.icr\n")\
	$(if $(IBUILD_BUILDING_TEST_HARNESS),"test_image2:target.bin\n", "flash_writer2:flash_writer.bin\n")\
	$(if $(IBUILD_BUILDING_TEST_HARNESS),,"main_firmware:target.bin\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON1), "hobbes_default_icmd_button1: $(HOBBES_DEFAULT_ICMD_BUTTON1)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON2), "hobbes_default_icmd_button2: $(HOBBES_DEFAULT_ICMD_BUTTON2)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON3), "hobbes_default_icmd_button3: $(HOBBES_DEFAULT_ICMD_BUTTON3)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON4), "hobbes_default_icmd_button4: $(HOBBES_DEFAULT_ICMD_BUTTON4)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON5), "hobbes_default_icmd_button5: $(HOBBES_DEFAULT_ICMD_BUTTON5)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON6), "hobbes_default_icmd_button6: $(HOBBES_DEFAULT_ICMD_BUTTON6)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON7), "hobbes_default_icmd_button7: $(HOBBES_DEFAULT_ICMD_BUTTON7)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON8), "hobbes_default_icmd_button8: $(HOBBES_DEFAULT_ICMD_BUTTON8)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON9), "hobbes_default_icmd_button9: $(HOBBES_DEFAULT_ICMD_BUTTON9)\n")\
	$(IBUILD_ICRON_HEADER_SPECTAREG_BLOCK)\
	| $(_IBUILD_SED_REMOVING_LEADING_AND_TRAILING_WHITESPACE) > $(targ_dir)/icron_header && \
	tar cjf $@ -C $(targ_dir) icron_header $(if $(IBUILD_BUILDING_TEST_HARNESS), target.icr, flash_writer.icr flash_writer.bin) target.bin ilog icmd icmdresp icomponent $(SPECTAREG_XML_FILES)

else # } {
ifeq ($(ICRON_HEADER_PROJECT), GOLDENEARS) # {

targ_icron_clean_files=icron_header target.elf target.bin flash_writer.bin ilog icmd icmdresp icomponent $(SPECTAREG_XML_FILES)

# The following cmd, generates all the necessary files makes the icron_header & tars up everything into a .icron file
targ_build2 = $(QUIET_ICRON)\
	mv $@ $(targ_dir)/target.elf && \
	$(if $(IBUILD_BUILDING_TEST_HARNESS),, $(CP) $(TOP_LEVEL_COMPONENTS_DIR)/flash_writer/$(PROJECT)_flash_bin/flash_writer.bin $(targ_dir) &&) \
	$(OBJCOPY) -O binary $(targ_dir)/target.elf $(targ_dir)/target.bin && \
	$(if $(SPECTAREG_XML_FILES), $(CP) $(foreach file, $(SPECTAREG_XML_FILES), $(TOP_LEVEL_COMPONENTS_DIR)/spectareg/etc/$(file)) $(targ_dir) &&) \
	echo -e -n "version:4\n"\
	"project:$(if $(IBUILD_BUILDING_TEST_HARNESS),goldenears_test,goldenears)\n"\
	"ilog:ilog\n"\
	"icmd:icmd\n"\
	"icomponent:icomponent\n"\
	"icmdresp:icmdresp\n"\
	$(if $(IBUILD_BUILDING_TEST_HARNESS),"test_image:target.bin\n", "flash_writer:flash_writer.bin\n")\
	$(if $(IBUILD_BUILDING_TEST_HARNESS),,"main_firmware:target.bin\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON1), "hobbes_default_icmd_button1: $(HOBBES_DEFAULT_ICMD_BUTTON1)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON2), "hobbes_default_icmd_button2: $(HOBBES_DEFAULT_ICMD_BUTTON2)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON3), "hobbes_default_icmd_button3: $(HOBBES_DEFAULT_ICMD_BUTTON3)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON4), "hobbes_default_icmd_button4: $(HOBBES_DEFAULT_ICMD_BUTTON4)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON5), "hobbes_default_icmd_button5: $(HOBBES_DEFAULT_ICMD_BUTTON5)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON6), "hobbes_default_icmd_button6: $(HOBBES_DEFAULT_ICMD_BUTTON6)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON7), "hobbes_default_icmd_button7: $(HOBBES_DEFAULT_ICMD_BUTTON7)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON8), "hobbes_default_icmd_button8: $(HOBBES_DEFAULT_ICMD_BUTTON8)\n")\
	$(if $(HOBBES_DEFAULT_ICMD_BUTTON9), "hobbes_default_icmd_button9: $(HOBBES_DEFAULT_ICMD_BUTTON9)\n")\
	$(IBUILD_ICRON_HEADER_SPECTAREG_BLOCK)\
	| $(_IBUILD_SED_REMOVING_LEADING_AND_TRAILING_WHITESPACE) > $(targ_dir)/icron_header && \
	tar cjf $@ -C $(targ_dir) icron_header $(if $(IBUILD_BUILDING_TEST_HARNESS),, flash_writer.bin) target.bin ilog icmd icmdresp icomponent $(SPECTAREG_XML_FILES)

else # } {
ifeq ($(ICRON_HEADER_PROJECT), BLACKBIRD) # {

targ_icron_clean_files=icron_header target.elf target.bin ilog icmd icmdresp icomponent $(SPECTAREG_XML_FILES) $(IDESIGN_XML_FILES)

# The following cmd, generates all the necessary files makes the icron_header & tars up everything into a .icron file
targ_build2 = $(QUIET_ICRON)\
	mv $@ $(targ_dir)/target.elf && \
	$(OBJCOPY) -O binary $(targ_dir)/target.elf $(targ_dir)/target.bin && \
	$(if $(SPECTAREG_XML_FILES), $(CP) $(foreach file, $(SPECTAREG_XML_FILES), $(TOP_LEVEL_COMPONENTS_DIR)/spectareg/etc/$(file)) $(targ_dir) &&) \
	$(if $(IDESIGN_XML_FILES), $(CP) -f $(foreach file, $(IDESIGN_XML_FILES), $(IDESIGN_XML_DIR)/$(file)) $(targ_dir))

else # } {
ifeq ($(ICRON_HEADER_PROJECT), FALCON) # {
$(error "TODO: FALCON")
else # } {
$(error "ICRON_HEADER_PROJECT not defined!")
endif # }
endif # }
endif # }
endif # }

