# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        = grg.c gpio.c grg_pll.c mdio_icmds.c pll_measure.c mdio_i2c.c i2c_icmds.c grg_irq.c imath.c led.c led_icmds.c
ASMSOURCES      = 

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/grg_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE = grg_cmd.h

#Specify the name of the icmdresp file for this component, it must be in the src directory
COMPONENT_ICMDRESP_FILE = grg_cmdresp.h

# Name of the target
TARGET          = libgrg.a

# define the type of target to build (lib or elf)
TARGET_TYPE     = lib

# the names of the dependencies of this component
COMPONENT_DEPS  = tasksch icmd ilog spectareg timing leon ibase

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES = 

include $(IBUILD_RULES)

