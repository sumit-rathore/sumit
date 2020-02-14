# define the source code
# Just provide a list of source files for C code and assembly code
# NOTE: all sources must be in the src directory of this component
CSOURCES        = net_base.c net_cmd_impl.c net_ethernet.c net_arp.c net_ipv4.c net_icmp.c net_udp.c net_dhcp.c net_link_local.c net_pair_discovery.c
ASMSOURCES      =

#Specify the name of the ilog file for this component with the path
COMPONENT_ILOG_FILE = ../src/net_log.h

#Specify the name of the icmd file for this component, it must be in the src directory
COMPONENT_ICMD_FILE = net_cmd.h

# Name of the target
TARGET          = libnet.a

# define the type of target to build (lib or elf)
TARGET_TYPE     = lib

# the names of the dependencies of this component
COMPONENT_DEPS  = linkmgr clm xcsr flash_data storage tasksch random ilog icmd timing leon ibase grg

# List all the test harnesses that can be built
# The test harnesses should all be in the test directory and contain a build directory where $(MAKE) will be called
TEST_HARNESSES =

include $(IBUILD_RULES)
