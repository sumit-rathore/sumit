#!/bin/sh

# This script updates the fpga files for a specific FPGA build

# argument is the directory of the FPGA build

if [ $# != 1 ]
then
    echo "Please use as \"$0 <path to fpga build\>" >&2
    exit 1
fi

fpga_dir="$1"

# convert verilog header files to C Header file
# up to __ICRON_FW_END__ token
verilogToCHeader ()
{
    # Args : inputFile outputFile
    sed -r -e 's/parameter/\#define/' -e 's/=//' -e s'/;//' -e 's/\[.*\]//' -e '/___ICRON_FW_END___/q' < "$1" > "$2"
    # 1st rule converts parameter to #define
    # 2nd rule removes all equal signs
    # 3rd rule removes all semi-colons
    # 4th rule removes all Verilog vector info, IE parameter[06:00] varName = 4;
    # 5th rule defines the end of the parsing
}

verilogToCHeader "$fpga_dir"/m_xusb_ge/src/xsst.vh inc/xsst_fw.h &&
verilogToCHeader "$fpga_dir"/m_xusb_ge/src/xusb.vh inc/xusb_fw.h

