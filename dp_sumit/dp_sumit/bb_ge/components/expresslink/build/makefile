# Makefile for ExpressLink

# Things that would be defined in the project makefile or on the command line
PROJECT ?= dummy
EXPRESS_LINK_RELEASE_NAME ?= Sandbox_Build

# Items that would be defined in a normal makefile
CSHARP_TARGET=ExpressLink.exe

CSHARP_SOURCES=Help.cs \
               Help.Designer.cs \
               CustomExceptions.cs \
               ExpressLinkMain.cs \
               ExpressLinkMain.Designer.cs \
               Program.cs \
               AssemblyInfo.cs \
               Resources.Designer.cs \
               StewieFile.cs \
               StewieFileSender.cs \
               XmodemFile.cs \
               XmodemSender.cs \
               IProgrammer.cs \
               config.Designer.cs

CSHARP_RESOURCES=config.resources,ExpressLink.config.resources \
				 Help.resources,ExpressLink.Help.resources \
				 ExpressLinkMain.resources,ExpressLink.ExpressLink.resources \
				 Resources.resources,ExpressLink.Properties.Resources.resources

CSHARP_REFERENCE=System.Drawing \
                 System.Windows.Forms \
                 System.Management

CSHARP_MAIN_CLASS=ExpressLink.Program

CSHARP_WIN32ICON=icron_fav.ico
CSHARP_TARGET_KIND=winexe


# Items that would be defined in a project.mak file
CSHARP_PLATFORM=anycpu
CSHARP_WARNING_LEVEL=4
CSHARP_OPTIMIZE=+



#Stuff that should be in rules.mak

# Setup tools
RESGEN=resgen
MCS=gmcs
XSLT=xsltproc
SED=sed
ECHO=echo
MKDIR=mkdir -p
RM=rm -f
RMDIR=rmdir

#Stuff that should be in project.mak (used by jump_prog.s)
AS=sparc-elf-as
CC=sparc-elf-gcc
OBJCOPY=sparc-elf-objcopy


targ_dir_stamp=$(targ_dir)/.stamp
obj_dir_stamp=$(obj_dir)/.stamp

# default paths
obj_dir =../$(PROJECT)_obj
src_dir = ../src
targ_dir = ../$(PROJECT)_bin

# The C Sharp files with paths
CSHARP_SOURCES_W_DIR=$(addprefix $(src_dir)/,$(CSHARP_SOURCES))
CSHARP_TARGET_W_DIR=$(targ_dir)/$(CSHARP_TARGET)
# For each resource drop the optional ID part of the arg, to just get the resource name, and then prepend the src directory
CSHARP_RESOURCES_NAMES=$(foreach res,$(CSHARP_RESOURCES), $(shell $(ECHO) $(res) | $(SED) -r 's/\,.*$$//'))
CSHARP_RESOURCES_W_DIR=$(addprefix $(obj_dir)/,$(CSHARP_RESOURCES_NAMES))

#default target
$(CSHARP_TARGET_W_DIR): $(CSHARP_SOURCES_W_DIR) $(CSHARP_RESOURCES_W_DIR) $(targ_dir_stamp)
	$(MCS) \
	-warnaserror \
	 /out:$@ \
	$(if $(CSHARP_MAIN_CLASS), /main:$(CSHARP_MAIN_CLASS)) \
	$(if $(CSHARP_PLATFORM), /platform:$(CSHARP_PLATFORM)) \
	$(if $(CSHARP_TARGET_KIND), /target:$(CSHARP_TARGET_KIND)) \
	$(if $(CSHARP_WIN32ICON), /win32icon:$(src_dir)/$(CSHARP_WIN32ICON)) \
	$(foreach ref, $(CSHARP_REFERENCE), /r:$(ref)) \
	$(if $(CSHARP_WARNING_LEVEL), /warn:$(CSHARP_WARNING_LEVEL)) \
	$(if $(CSHARP_OPTIMIZE), /optimize$(CSHARP_OPTIMIZE)) \
	$(CSHARP_SOURCES_W_DIR) \
	$(foreach res, $(addprefix $(obj_dir)/,$(CSHARP_RESOURCES)), /resource:$(res))

# build rules
# TODO: This hard codes the flash_boot.icr flash_writer.icr & $(PROJECT).bin dependencies to force these to rebuild.  This shouldn't be so hard coded
# NOTE: Every .resx file in src_dir is processed by the XSLT, even though some
# files require no substitutions.
$(obj_dir)/%.resx: $(src_dir)/%.resx parameterizeResx.xslt $(obj_dir_stamp) $(obj_dir)/leon_boot_flash.bin $(obj_dir)/leon_boot_flash_w_recovery.bin $(obj_dir)/leon_boot_flash.icr $(obj_dir)/flash_writer_ge.bin $(obj_dir)/flash_writer_ge_spartan.bin $(obj_dir)/flash_writer_lg.icr $(obj_dir)/firmware_ge.bin $(obj_dir)/firmware_ge_spartan_lex.bin $(obj_dir)/firmware_ge_spartan_rex.bin $(obj_dir)/firmware_lg.bin
	$(XSLT)  \
	    --stringparam expresslink_build_path    "$(obj_dir)" \
	    --stringparam expresslink_release_name  "$(EXPRESS_LINK_RELEASE_NAME)" \
	    parameterizeResx.xslt \
	    $< | unix2dos > $@

$(obj_dir)/%.resources: $(obj_dir)/%.resx $(obj_dir_stamp)
	$(RESGEN) $< $@

# Build the object directory
$(obj_dir_stamp):
	$(QUIET_MKDIR)$(MKDIR) $(obj_dir)
	@touch "$@"

# Build the target directory
$(targ_dir_stamp):
	$(QUIET_MKDIR)$(MKDIR) $(targ_dir)
	@touch "$@"


LEON_CODE_IN_FLASH_ADDR=0x30000000
LEON_CODE_IN_FLASH_W_RECOV_IMG_ADDR=0x30380000

# Jump program to assist in reading the firmware version.
$(obj_dir)/leon_boot_flash.bin: $(src_dir)/jump_prog.S
	$(CC) $< -c -o $(obj_dir)/jump_prog.o -D__ASSEMBLER__ -DJUMP_TO_ADDR=$(LEON_CODE_IN_FLASH_ADDR)
	$(OBJCOPY) -O binary $(obj_dir)/jump_prog.o $(obj_dir)/leon_boot_flash.bin

$(obj_dir)/leon_boot_flash.icr: $(obj_dir)/leon_boot_flash.bin
	$(OBJCOPY) -I binary -O srec --change-address 0x60000000 $(obj_dir)/leon_boot_flash.bin $(obj_dir)/leon_boot_flash.srec
	srec_cat $(obj_dir)/leon_boot_flash.srec -o $@ -Stewie -Enable_Sequence_Warnings

$(obj_dir)/leon_boot_flash_w_recovery.bin: $(src_dir)/jump_prog.S
	$(CC) $< -c -o $(obj_dir)/jump_prog_w_recovery.o -D__ASSEMBLER__ -DJUMP_TO_ADDR=$(LEON_CODE_IN_FLASH_W_RECOV_IMG_ADDR)
	$(OBJCOPY) -O binary $(obj_dir)/jump_prog_w_recovery.o $(obj_dir)/leon_boot_flash_w_recovery.bin


# Targets for fetching the binaries to be embedded.  The goal of these rules is
# to pull a copy of an externally released binary from the released_binaries
# directory if the toplevel makefile didn't previously copy it's own output
# into the obj_dir.
$(obj_dir)/firmware_lg.bin: ../released_binaries/firmware_lg-*.bin
	cp $< $@

$(obj_dir)/firmware_ge.bin: ../released_binaries/firmware_ge-*.bin
	cp $< $@

$(obj_dir)/firmware_ge_spartan_lex.bin: ../released_binaries/firmware_ge_spartan_lex-*.bin
	cp $< $@

$(obj_dir)/firmware_ge_spartan_rex.bin: ../released_binaries/firmware_ge_spartan_rex-*.bin
	cp $< $@

$(obj_dir)/flash_writer_lg.icr: ../released_binaries/flash_writer_lg-*.icr
	cp $< $@

$(obj_dir)/flash_writer_ge.bin: ../released_binaries/flash_writer_ge-*.bin
	cp $< $@

$(obj_dir)/flash_writer_ge_spartan.bin: ../released_binaries/flash_writer_ge_spartan-*.bin
	cp $< $@

# clean rule:
clean:
	@-$(RM) $(obj_dir)/firmware_lg.bin $(obj_dir)/firmware_ge.bin $(obj_dir)/firmware_ge_spartan_lex.bin $(obj_dir)/firmware_ge_spartan_rex.bin
	@-$(RM) $(obj_dir)/jump_prog.o  $(obj_dir)/jump_prog_w_recovery.o $(obj_dir)/leon_boot_flash.bin $(obj_dir)/leon_boot_flash.srec $(obj_dir)/leon_boot_flash.icr $(obj_dir)/leon_boot_flash_w_recovery.bin
	@-$(RM) $(obj_dir)/flash_writer_lg.icr $(obj_dir)/flash_writer_ge.bin $(obj_dir)/flash_writer_ge_spartan.bin
	@-$(RM) $(CSHARP_RESOURCES_W_DIR)
	@-$(RM) $(addprefix $(obj_dir)/,$(CSHARP_RESOURCES_NAMES:%.resources=%.resx))
	@-$(RM) $(targ_dir)/$(CSHARP_TARGET)
	@-$(RM) $(xslt_source)
	@-$(RM) $(obj_dir)/.stamp $(targ_dir)/.stamp
	@-[ -d $(obj_dir) ] && $(RMDIR) $(obj_dir) || true
	@-[ -d $(targ_dir) ] && $(RMDIR) $(targ_dir) || true

