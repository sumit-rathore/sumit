# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES_TEMP = $(notdir $(wildcard ../src/*.c))
ASMSOURCES = $(notdir $(wildcard ../src/*.S))

#Specify the name of the ilog file for this component with the path
#NOTE:  We are too low level to support ilog.  ilog depends on us, not the
#       other way around
#COMPONENT_ILOG_FILE = 

# Name of the target
TARGET          = libleon.a

# define the type of target to build (lib or elf)
TARGET_TYPE     = lib

# the names of the dependencies of this component
COMPONENT_DEPS  = uart core register_definitions ibase

ifeq ("$(PROJECT)", "programBB")
  CSOURCES=$(filter-out bin_table.c, $(CSOURCES_TEMP))
else
  CSOURCES=$(CSOURCES_TEMP)
endif
# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called

TEST_HARNESSES = uart uart_w_irq uart_rx_polling uart_rx_interrupt uart_w_ilog timer_w_ilog timer_blocking time_difference timer_aperiodic timer_periodic_w_stop ilog_with_timer_set ilog_with_timer_set_no_wait flood_ilogs flash_icmds
include $(IBUILD_RULES)

