# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        = flash_writer.c command.c command_program.c
ASMSOURCES      =

# iLog file
COMPONENT_ILOG_FILE = ../src/flash_log.h

# define the name of the target file to produce
# libraries MUST be named "lib<name of component>.a"
# elf files can have any name
TARGET          = flash_writer.elf

# define the type of target to build (lib, lib_collector, or elf)
TARGET_TYPE     = elf

# Ensure that the dependencies for this build will not conflict with the main project build
PROJECT_ORIG := $(PROJECT)
PROJECT := $(PROJECT)_flash

# Ensure that the icmd won't be built for our dependencies
NO_ICMD_SUPPORT=1

# Files in the target directory not created by ibuild that should be cleaned up on a make clean
TARG_DIR_CLEANUP_FILES  = $(TARGET:.elf=).*

# the names of the dependencies of this component
# NOTE: icmd is included so it is available to the test harnesses built under $(PROJECT)_flash
COMPONENT_DEPS  = crc grg xmodem flash_data leon ibase

# Forced dependencies for linking
# Add forced library dependencies (all .o files are included in these libraries)
# This must be done for startup libraries
# This isn't applicable to library components
FORCED_LINK_DEPS = 

# We want to over ride the linker file from the project.mak file as we supply our own
export LDFLAGS=$(FLASH_WRITER_LD_FLAGS)

# If the logs are flooding, something has gone very wrong, don't drop logs, assert hard
export CFLAGS+=-DILOG_ASSERT_ON_DROPPED_LOGS -DFLASH_WRITER

# Use a small set of SPARC register windows that is supported on very old FPGA & recovery FPGA
export CFLAGS := $(if $(findstring GOLDENEARS, $(CFLAGS)), -DSTARTING_PSR_VALUE=0x8E,) $(CFLAGS)

flash_writer.c_CFLAGS=$(if $(FLASHWRITER_RECOVERY_IMAGE), -DFLASHWRITER_BASE_ADDRESS=0x30000000 -DRECOVERY, )
flash_writer.c_CFLAGS+=$(if $(FLASHWRITER_FULL_IMAGE), -DFLASHWRITER_BASE_ADDRESS=0x30200000 -DERASE_ALL, )

# define our own default rule
# For LG1 this depends on the stewie rule (.icr file) for the LG1 ROM
# Both LG1 & GE will build binary files for sending the flash writer by X-Modem
target: $(TARGET) $(if $(findstring GOLDENEARS, $(CFLAGS)), , $(targ_dir)/$(TARGET:.elf=.icr)) $(targ_dir)/$(TARGET:.elf=.bin)
	@echo "Building Flash Writer for project:$(PROJECT)"
	$(OBJDUMP) -t $(targ_dir)/$(TARGET) |sort > $(targ_dir)/$(TARGET:.elf=.map)
	$(OBJDUMP) -d $(targ_dir)/$(TARGET) > $(targ_dir)/$(TARGET:.elf=.s)
	$(SIZE) $(targ_dir)/$(TARGET)
	@#Copy over the ilog file
	mkdir -p ../$(PROJECT_ORIG)_lib
	cp $(targ_dir)/ilog.component ../$(PROJECT_ORIG)_lib

#binary for Xmodem
$(targ_dir)/$(TARGET:.elf=.bin): $(TARGET)
	$(OBJCOPY) -O binary $(targ_dir)/$(TARGET) $(targ_dir)/$(TARGET:.elf=.bin)

#Stewie file for LG1 ROM
$(targ_dir)/$(TARGET:.elf=.icr): $(TARGET)
	$(OBJCOPY) -O srec $(targ_dir)/$(TARGET) $(targ_dir)/$(TARGET:.elf=.s19)
	srec_cat $(targ_dir)/$(TARGET:.elf=.s19) -o $(targ_dir)/$(TARGET:.elf=.icr) -Stewie -Enable_Sequence_Warnings

clean: clean_ibuild
	-@$(RM) ../$(PROJECT_ORIG)_lib/ilog.component
	-@[ -d ../$(PROJECT_ORIG)_lib ] && $(RMDIR) ../$(PROJECT_ORIG)_lib

TEST_HARNESSES = burn_flash test_flash verify_checksum verify_image

.PHONY: target


# Pull in the ibuild make rules
include $(IBUILD_RULES)

