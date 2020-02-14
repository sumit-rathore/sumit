#!/bin/sh

# update the FPGA specific files from an FPGA build

# argument is the directory of the FPGA build

set -e 

if [ $# != 1 ]
then
    echo "Please use as \"$0 <path to fpga build\>" >&2
    exit 1
fi

fpga_dir="$1"

if [ ! -f "$fpga_dir"/f_goldenears/build_s6_lex/ge_top_s6_lex.mcs ]
then
    cp "$fpga_dir"/../rls/ge_top_s6_lex/ge_top_s6_lex.mcs etc/
else
    cp "$fpga_dir"/f_goldenears/build_s6_lex/ge_top_s6_lex.mcs etc/
fi

if [ ! -f "$fpga_dir"/f_goldenears/build_s6_rex/ge_top_s6_rex.mcs ]
then
    cp "$fpga_dir"/../rls/ge_top_s6_rex/ge_top_s6_rex.mcs etc/
else
    cp "$fpga_dir"/f_goldenears/build_s6_rex/ge_top_s6_rex.mcs etc/
fi

( cd components/spectareg   && ./update.sh "$fpga_dir" )
( cd components/xcsr        && ./update.sh "$fpga_dir" )


