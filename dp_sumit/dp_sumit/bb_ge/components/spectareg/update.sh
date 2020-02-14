#!/bin/sh

#This component contains the spectareg auto generated header files in the /inc/
#directory, and the source xml files

# This script updates the spectareg files for a specific FPGA build

# argument is the directory of the FPGA build

if [ $# != 1 ]
then
    echo "Please use as \"$0 <path to fpga build\>" >&2
    exit 1
fi

fpga_dir="$1"

# CSS style sheets (note: they are all the same)
# Just grab the first one (ULMII)
cp "$fpga_dir"/m_ulmii_ge/docs/comp_mmreg.css doc/ &&
chmod u+w doc/comp_mmreg.css &&

# ULMII
cp "$fpga_dir"/m_ulmii_ge/docs/ulmii_mmreg_macro.h inc/ &&
cp "$fpga_dir"/m_ulmii_ge/docs/ulmii_comp.xml etc/ &&
cp "$fpga_dir"/m_ulmii_ge/docs/ulmii_mmreg.html doc/ &&
chmod u+w doc/ulmii_mmreg.html &&

# CLM
cp "$fpga_dir"/m_clm_ge/docs/clm_mmreg_macro.h inc/ &&
cp "$fpga_dir"/m_clm_ge/docs/clm_comp.xml etc/ &&
cp "$fpga_dir"/m_clm_ge/docs/clm_mmreg.html doc/ &&
chmod u+w doc/clm_mmreg.html &&

# XCSR
cp "$fpga_dir"/m_xusb_ge/docs/xcsr_mmreg_macro.h inc/ &&
cp "$fpga_dir"/m_xusb_ge/docs/xcsr_comp.xml etc/ &&
cp "$fpga_dir"/m_xusb_ge/docs/xcsr_mmreg.html doc/ &&
chmod u+w doc/xcsr_mmreg.html &&

# XLR
cp "$fpga_dir"/m_xusb_ge/docs/xlr_mmreg_macro.h inc/ &&
cp "$fpga_dir"/m_xusb_ge/docs/xlr_comp.xml etc/ &&
cp "$fpga_dir"/m_xusb_ge/docs/xlr_mmreg.html doc/ &&
chmod u+w doc/xlr_mmreg.html &&

# XRR
cp "$fpga_dir"/m_xusb_ge/docs/xrr_mmreg_macro.h inc/ &&
cp "$fpga_dir"/m_xusb_ge/docs/xrr_comp.xml etc/ &&
cp "$fpga_dir"/m_xusb_ge/docs/xrr_mmreg.html doc/ &&
chmod u+w doc/xrr_mmreg.html &&

# GRG
cp "$fpga_dir"/m_goldenears/docs/grg_mmreg_macro.h inc/ &&
cp "$fpga_dir"/m_goldenears/docs/grg_comp.xml etc/ &&
cp "$fpga_dir"/m_goldenears/docs/grg_mmreg.html doc/ &&
chmod u+w doc/grg_mmreg.html


