# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES = $(notdir $(wildcard ../src/*.c))
ASMSOURCES = $(notdir $(wildcard ../src/*.S))

#Specify the name of the ilog file for this component with the path
COMPONENT_ICMD_FILE = i2cd_cmd.h
COMPONENT_ILOG_FILE = ../src/i2cd_log.h

# Name of the target
TARGET          = libi2cd.a

# define the type of target to build (lib or elf)
TARGET_TYPE     = lib

# the names of the dependencies of this component
COMPONENT_DEPS  = stats_mon ulp configuration uart icmd util callback top timing core ilog i2c gpio register_definitions ibase leon

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES =

include $(IBUILD_RULES)

# setup rule which won't overwrite rules above, just adding another rule
# this rule says when creating dependencies required by CSOURCES also create
# the i2cd_si5326cfg.c file by making it a prerequisit
$(PROJECT)_obj/$(CSOURCES:%.c=%.d): ../src/i2cd_si5326cfg.c

clean:
	rm -rf ../i2cd_lib
	rm -f ../src/i2cd_si5326cfg.c
	rm -rf *_obj

../src/i2cd_si5326cfg.c: ../src/SI5326cfg.txt
	$(shell awk -f si5326cfgToC.awk ../src/SI5326cfg.txt > ../src/i2cd_si5326cfg.c)

