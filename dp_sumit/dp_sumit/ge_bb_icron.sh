#!/bin/bash -i

# Guidelines for ge_bb_icron.sh

# 1. “-clean” cleans everything
# 2. “-ge” builds goldenears
# 3. “-mcs” creates mcs files
# 4. “-non_debug” creates both debug and non-debug files

# To create a Sandbox, please pass an argument ‘-non_debug’ to create both debug and non_debug files
# and then place those files manually in a designated folder.
# Don't forget to change FPGA version manually in imain.c for non_debug file to work

# ExCOM works with only non_debug file i.e. ‘ExCOM.icron’

err_report() {
    echo "Error on line $1"
}

trap 'err_report $LINENO' ERR

# get BB icomponent count and generate new icomponent file, prepending BB component names with BB_

RELEASEDROOT="/data/engdev/designs/blackbird/released/sw/"
RELEASEDROOT_ROM="/data/engdev/designs/blackbird/released/rom/"


BBSRC_DEBUG=./blackbird_a7_debug_bin/

BBSRC=./blackbird_a7_bin/

GEROOT=./bb_ge/
GESRC="${GEROOT}ge_asic_bb_companion_debug_bin/"
GEBIN="/bb_ge/ge_asic_bb_companion_debug_bin/ge_asic_bb_companion_debug.bin"
GEFWBIN="/bb_ge/ge_asic_bb_companion_debug_bin/flash_writer.bin"
GEXMLCLM="/bb_ge/ge_asic_bb_companion_debug_bin/clm_comp.xml"
GEXMLGRG="/bb_ge/ge_asic_bb_companion_debug_bin/grg_comp.xml"
GEXMLULMII="/bb_ge/ge_asic_bb_companion_debug_bin/ulmii_comp.xml"
GEXMLXCSR="/bb_ge/ge_asic_bb_companion_debug_bin/xcsr_comp.xml"
GEXMLXLR="/bb_ge/ge_asic_bb_companion_debug_bin/xlr_comp.xml"
GEXMLXRR="/bb_ge/ge_asic_bb_companion_debug_bin/xrr_comp.xml"
GEICOMP="/bb_ge/ge_asic_bb_companion_debug_bin/icomponent"
GEICMD="/bb_ge/ge_asic_bb_companion_debug_bin/icmd"
GEICMDRESP="/bb_ge/ge_asic_bb_companion_debug_bin/icmdresp"
GEILOG="/bb_ge/ge_asic_bb_companion_debug_bin/ilog"


PGBROOT=./program/
PBSRC="${PGBROOT}programBB_bin/"
PBBIN="/program/programBB_bin/programBB.bin"

ROMROOT=./rom/
ROMSRC="${ROMROOT}blackbird_rom_bin/"
ROMBIN="/rom/blackbird_rom_bin/blackbird_rom.bin"

# key value pair table storing the table entries
declare -A binTable=()
declare -A binTableAddress=()

### FUNCs
# get firmware version and put it to flash version location
function putFirmwareVersion {
    TGSTR=$(awk '
            BEGIN{
            }
            /define SOFTWARE_*[A-Z_]*_REVISION[[:blank:]]/ {
                split($0, a, " ");
                printf("%02x",a[3]);
            }
            END{
                printf("00\n");
            }
            ' "$FSRC")
    echo "${TGSTR}" -n | xxd -r -p | dd of="${BBSRC_DEBUG}target.bin" conv=notrunc bs=1 seek=$(($(($entryBase + 8)))) count=4 &>/dev/null
    if [ ! "$NON_DEBUG_BB" = "" ]
    then
        echo "${TGSTR}" -n | xxd -r -p | dd of="${BBSRC}target.bin" conv=notrunc bs=1 seek=$(($(($entryBase + 8)))) count=4 &>/dev/null
    fi
}

function storeBinTableAddressesInRom {
    #romTableEntry is converted to decimal when reading from temp file
    #convert here from decimal to hexadecimal
    romTableEntry=$(printf "%x" $((10#$romTableEntry)))

    # write address into table
    echo "${romTableEntry}" -n | xxd -r -p | dd of="${ROMSRC}blackbird_rom.bin" conv=notrunc bs=1 seek=$romTableOffsetInBytes count=4
}

function extractFlashBinTableStartAddrOffset {
awk '
    BEGIN{
    }
    /define FLASH_BIN_TABLE_*[A-Z_]*_START/ {
        split($0, a, "define FLASH_BIN_TABLE_");
        split(a[2], b, " ");
        entry = b[1];
        split(a[2], b, "(");
        split(b[2], a, ")");
        value = a[1];
        print entry"="value;
    }
    ' "$FSRC" > "$FDST"
}

function extractFlashBinTableAddress {
awk '
    BEGIN{
    }
    /define FLASH_BIN_[A-Z_]*_TABLE_ADDRESS/ {
        split($0, a, "define FLASH_BIN_");
        split(a[2], b, " ");
        entry = b[1];
        split(a[2], b, "x");
        split(b[2], a, ")");
        value = a[1];
        print entry"="value;
    }
    ' "$FSRC" > "$FDST"
}

function getRomSymsElf {
awk '
    BEGIN{
    }
    /currentRomReleaseTag/ {
        split($0, a, "= ");
        print a[2];
    }
    ' "$FSRC"
}

function getFpgaSrcLocationMaverick {
awk '
    BEGIN{
    }
    /RTL_DIR_MAVERICK/ {
        split($0, a, "=|\n");
        sub(/src/,"build",a[2]);
        print a[2];
    }
    ' "$FSRC"
}

function getFpgaSrcLocationRaven {
awk '
    BEGIN{
    }
    /RTL_DIR_RAVEN/ {
        split($0, a, "=");
        sub(/src/,"build",a[2]);
        print a[2];
    }
    ' "$FSRC"
}

function printHelp {
    echo "** HELP **"
    echo "** ** STANDARD USE CASES ** **"
    echo "Usage: ./ge_bb_icron.sh"
    echo "--> Builds GE, ProgramBB, Maverick BBMainFW for first time and then only Maverick BBMain FW"
    echo "Usage: ./ge_bb_icron.sh -clean"
    echo "--> Cleans GE, ProgramBB, BBMainFW"
    echo "Usage: ./ge_bb_icron.sh -g"
    echo "--> Builds GE only"
    echo "Usage: ./ge_bb_icron.sh -p"
    echo "--> Builds ProgramBB only"
    echo "Usage: ./ge_bb_icron.sh -all"
    echo "--> Builds GE, ProgramBB, Maverick BBMainFW"
    echo "Usage: ./ge_bb_icron.sh -usb"
    echo "--> Builds GE, ProgramBB, Raven BBMainFW for first time and then only Raven BBMain FW"
    echo "Usage: ./ge_bb_icron.sh -all -usb"
    echo "--> Builds GE, ProgramBB, Raven BBMainFW"
    echo "Usage: ./ge_bb_icron.sh -non_debug"
    echo "--> Builds both debug and non debug files for GE, ProgramBB, BBMainFW"
    echo "Usage: ./ge_bb_icron.sh -mcs"
    echo "--> Builds GE, ProgramBB, Maverick BBMainFW and create mcs files"
    echo "-->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>--- "
    echo ""
    echo "** ** ADVANCED USE CASES ** **"
    echo "*** TAGGING ***"
    echo "--> Git will checkout the supplied tag for that submodule (GE) or entire tree (ROM/BBMainFW/ProgramBB)"
    echo "*** use -bb_tag <BBSW_vXX_YY_ZZ> for BBMainFW to checkout a tag before building"
    echo "*** use -rom_tag <BBROMSW_vXX_YY> for ROM to checkout a tag before building"
    echo "*** use -pbb_tag <BBSW_vXX_YY_ZZ> for ProgramBB to checkout a tag before building"
    echo "*** use -ge_tag <BBSW_vXX_YY_ZZ> for GE to checkout a tag before building"
    echo "*** BINARIES ***"
    echo "--> Instead of compiling the binary, we can copy them into place"
    echo "--> The passed argument is a tag, but that tag matches the released/sw/folder where the binaries are located"
    echo "*** use -rom_bin <BBROMSW_vXX_YY> to copy ROM binary from released folder without building"
    echo "*** use -pbb_bin <BBSW_vXX_YY_ZZ> for ProgramBB to copy binary from released folder without building"
    echo "*** use -ge_bin <BBSW_vXX_YY_ZZ> for GE to copy binary from released folder wihtout building"
    echo ""
    echo "NOTE: These advanced flags can be used in conjunction with the target flags, ie: -b or -g"
    echo "--> ./ge_bb_icron.sh -b \"m clean\" m -pbb_tag BBSW_v00_00_12 -ge_bin BBSW_v00_00_11"
    exit
}


### MAIN PROCESS
MAKE_M="make -s -j 12"
ARG1="${MAKE_M}"
ARG2=""

# Build GE then ProgramBB then BBMainFW from source in tree
BUILDBB="Y"
BUILDGE=""
BUILDROM=""
BUILDPGMBB=""
NON_DEBUG_BB=""
USB=""
PLUG_TEST=""
# Tags
ROMSOURCETAG="" # Checkout specific tag for ROM/GE/PGMBB
GESOURCETAG=""
PGMSOURCETAG=""
BBSOURCETAG=""

# Binaries - PGMBB_vXX_YY_ZZ - look in released/sw for folder and
# copy appropriate binary
ROMSOURCEBIN=""
GESOURCEBIN=""
PGMSOURCEBIN=""

#Create mcs file
MCSFILE=""

#GE build
NEWGE=""
GEBUILD_RESULT=""

declare -a ARGLIST=("$@")
GETARGS=""
SKIPARG=0

#for arg in "$@"
#do
for i in "${!ARGLIST[@]}"; do
    case "${ARGLIST[$i]}" in
        -help) printHelp
            exit
            ;;
    #following options are used by release scripts
        -b)
            BUILDBB="Y"
            BUILDGE=""
            BUILDPGMBB=""
            BUILDROM=""
            GETARGS="Y"
            MCSFILE="Y"
            NON_DEBUG_BB="Y"
            ;;
        -ge)
            BUILDBB=""
            BUILDGE="Y"
            BUILDPGMBB=""
            BUILDROM=""
            GETARGS="Y"
            ;;
        -pgmbb)
            BUILDBB=""
            BUILDGE=""
            BUILDPGMBB="Y"
            BUILDROM=""
            GETARGS="Y"
            ;;
        -pgmbb_iso)
            BUILDBB=""
            BUILDGE=""
            BUILDPGMBB="Y"
            BUILDROM=""
            GETARGS="Y"
            USB="Y"
            ;;
        -r)
            BUILDBB=""
            BUILDGE=""
            BUILDPGMBB=""
            BUILDROM="Y"
            GETARGS="Y"
            ;;
        #used by build script to do all iso raven
        -iso_release)
            BUILDBB="Y"
            BUILDGE="Y"
            BUILDPGMBB="Y"
            BUILDROM=""
            GETARGS="Y"
            USB="Y"
            MCSFILE="Y"
            NON_DEBUG_BB="Y"
            export BB_ISO=1
            ;;
        #used by build script to do all non iso raven
        -usb_release)
            BUILDBB="Y"
            BUILDGE="Y"
            BUILDPGMBB="Y"
            BUILDROM=""
            GETARGS="Y"
            USB="Y"
            MCSFILE="Y"
            NON_DEBUG_BB="Y"
            export BB_USB=1
            ;;
        #used by build script to do only iso
        -only_iso)
            BUILDBB="Y"
            BUILDGE=""
            BUILDPGMBB=""
            BUILDROM=""
            GETARGS="Y"
            USB="Y"
            MCSFILE="Y"
            NON_DEBUG_BB="Y"
            export BB_ISO=1
            ;;
        #used by build script to do only non iso raven
        -only_usb)
            BUILDBB="Y"
            BUILDGE=""
            BUILDPGMBB=""
            BUILDROM=""
            GETARGS="Y"
            USB="Y"
            MCSFILE="Y"
            NON_DEBUG_BB="Y"
            export BB_USB=1
            ;;
        #used by build script for all maverick release
        -release)
            BUILDBB="Y"
            BUILDGE="Y"
            BUILDPGMBB="Y"
            BUILDROM=""
            GETARGS="Y"
            MCSFILE="Y"
            NON_DEBUG_BB="Y"
            ;;

    #following options are used for manual build
        -g)
            BUILDBB=""
            BUILDGE="Y"
            BUILDPGMBB=""
            BUILDROM=""
            GETARGS=""
            ;;
        -p)
            BUILDBB=""
            BUILDGE=""
            BUILDPGMBB="Y"
            BUILDROM=""
            GETARGS=""
            ;;
        -p_iso)
            BUILDBB=""
            BUILDGE=""
            BUILDPGMBB="Y"
            BUILDROM=""
            GETARGS=""
            USB="Y"
            ;;        
        -rom)
            BUILDBB=""
            BUILDGE=""
            BUILDPGMBB=""
            BUILDROM="Y"
            GETARGS=""
            ;;
        -all)
            BUILDBB="Y"
            BUILDGE="Y"
            BUILDPGMBB="Y"
            BUILDROM=""
            GETARGS=""
            ;;
        -mcs)
            BUILDBB="Y"
            BUILDGE="Y"
            BUILDPGMBB="Y"
            BUILDROM=""
            GETARGS=""
            MCSFILE="Y"
            NON_DEBUG_BB="Y"
            ARG1="${MAKE_M} NO_DEBUG=1"
            ;;
        -usb)
            USB="Y"
            export BB_USB=1
            ;;
        -iso)
            ISO="Y"
            USB="Y"
            export BB_ISO=1
            ;;
        -plug)
            PLUG_TEST="Y"
            export PLUG_TEST=1
            ;;
        -plug_release)
            BUILDBB="Y"
            BUILDGE=""
            BUILDPGMBB=""
            BUILDROM=""
            GETARGS="Y"
            MCSFILE="Y"
            NON_DEBUG_BB="Y"
            PLUG_TEST="Y"
            export PLUG_TEST=1
            ;;

        #if want to build both debug and non debug, pass non_debug
        -non_debug)
            BUILDBB="Y"
            BUILDGE="Y"
            BUILDPGMBB="Y"
            BUILDROM=""
            GETARGS=""
            NON_DEBUG_BB="Y"
            ARG2="${MAKE_M} NO_DEBUG=1"
            ;;

    #following arguements are passed to copy binaries from
    #specific tag
        -bb_tag)
            if [[ ! ${ARGLIST[$i+1]} == "-"* ]]
            then
                BBSOURCETAG=${ARGLIST[$i+1]}
                SKIPARG=1
            fi
            ;;
        -rom_tag)
            if [[ ! ${ARGLIST[$i+1]} == "-"* ]]
            then
                ROMSOURCETAG=${ARGLIST[$i+1]}
                SKIPARG=1
            fi
            ;;
        -rom_bin)
            if [[ ! ${ARGLIST[$i+1]} == "-"* ]]
            then
                ROMSOURCEBIN=${ARGLIST[$i+1]}
                BUILDROM="Y"
                SKIPARG=1
            fi
            ;;
        -pbb_tag)
            if [[ ! ${ARGLIST[$i+1]} == "-"* ]]
            then
                PGMSOURCETAG=${ARGLIST[$i+1]}
                SKIPARG=1
            fi
            ;;
        -pbb_bin)
            if [[ ! ${ARGLIST[$i+1]} == "-"* ]]
            then
                PGMSOURCEBIN=${ARGLIST[$i+1]}
                BUILDPGMBB="Y"
                SKIPARG=1
            fi
            ;;
        -ge_tag)
            if [[ ! ${ARGLIST[$i+1]} == "-"* ]]
            then
                GESOURCETAG=${ARGLIST[$i+1]}
                SKIPARG=1
            fi
            ;;
        -ge_bin)
            if [[ ! ${ARGLIST[$i+1]} == "-"* ]]
            then
                GESOURCEBIN=${ARGLIST[$i+1]}
                BUILDGE="Y"
                SKIPARG=1
            fi
            ;;
        *) if [ $SKIPARG -eq 0 ]
            then
                if [ $# -eq 1 ]
                then
                    ARG1=$1
                    ARG2=""
                fi
                if [ $# -eq 2 ]
                then
                    ARG1=$1
                    ARG2=$2
                fi
            else
                ((SKIPARG--))
            fi
            ;;
    esac

    if [ "${GETARGS}" == "Y" ]
    then
        # clear ARG1 and ARG2 just in case we are copying binaries only
        # and don't want to enter arguments
        ARG1=""
        ARG2=""
        # check against special options
        if [[ ! ${ARGLIST[$i+1]} == "-"* ]]
        then
            # keep make, change m for make -s -j 12
            if [[ ${ARGLIST[$i+1]} == "make"* ]]
            then
                ARG1="${ARGLIST[$i+1]} NO_DEBUG=1"
            else
                ARG1=${ARGLIST[$i+1]/m/$MAKE_M}
            fi
            SKIPARG=1
        fi
        if [[ ! ${ARGLIST[$i+2]} == "-"* ]]
        then
            if [[ ${ARGLIST[$i+2]} == "make"* ]]
            # NO_DEBUG = 1 so that build script always
            # create both debug and non_debug
            then
                ARG2=${ARGLIST[$i+2]}
            else
                ARG2=${ARGLIST[$i+2]/m/$MAKE_M}
            fi
            SKIPARG=2
        fi
        GETARGS=""
    fi
done
# echo "ARG1: ${ARG1} ARG2: ${ARG2}"

if [ ! -d "${GESRC}" ]
then
    BUILDGE="Y"
fi
if [ ! -d "${PBSRC}" ]
then
    BUILDPGMBB="Y"
fi

find -name "*.stamp"|xargs rm -rf   #workaround: remove all stamp files, so that time stamp get updated
                                    #everytime we compile

 #to clean everything
if [ "${ARG1}" == "-clean" ]
then
    find -name "*_bin"|xargs rm -rf
    find -name "*_obj"|xargs rm -rf
    find -name "*_lib"|xargs rm -rf

    ARG1="m clean"
    # #clean everything
    # exit 0
fi
# echo ">>> $(date)"
# echo "ARG1: ${ARG1}, ARG2: ${ARG2}"
#exit

# Capture the binary table so we can determine offsets and calculate CRC
FSRC="./inc/options.h"
FDST="bintable.temp"
extractFlashBinTableStartAddrOffset

while read -r -a words; do
    set -- "${words[@]}"
    for word; do
        if [[ $word = *"="* ]]; then
            binTable[${word%%=*}]=${word#*=}
        fi
    done
done <"$FDST"

# Capture the binary table start addresses so we can place it in the ROM image
FSRC="./inc/options.h"
FDST="bintable.temp"
extractFlashBinTableAddress

while read -r -a words; do
    set -- "${words[@]}"
    for word; do
        if [[ $word = *"="* ]]; then
            binTableAddress[${word%%=*}]=${word#*=}
        fi
    done
done <"$FDST"
rm "$FDST"

# Build ROM
if [ ! "$BUILDROM" = "" ]
then
    echo ""
    echo "**** Building ROM ****"
    echo ">>> $(date)"
    # checkout source if defined
    if [ ! "$ROMSOURCETAG" = "" ]
    then
        git checkout "${ROMSOURCETAG}"
    fi

    # copy binary if defined
    if [ ! "$ROMSOURCEBIN" = "" ]
    then
        if [ ! -d "${ROMSRC}" ]
        then
            mkdir "${ROMSRC}"
        fi
        cp -f "${RELEASEDROOT_ROM}${ROMSOURCEBIN}/build_dir${ROMBIN}" ".${ROMBIN}"
    else
        echo "Changing directory"
        (cd "rom/build" && eval "${ARG1}" && eval "${ARG2}")
    fi
    echo ">>> $(date)"
    echo ""
# Insert addresses into ROM's table, remember first 2 words are jump and nop
# next 3 words are for FPGA ID, date and time
# ASIC golden address
romTableOffsetInBytes=20
romTableEntry=$((16#${binTableAddress["ASIC_GOLDEN_TABLE_ADDRESS"]}))
storeBinTableAddressesInRom

# ASIC current address
romTableOffsetInBytes=24
romTableEntry=$((16#${binTableAddress["ASIC_CURRENT_TABLE_ADDRESS"]}))
storeBinTableAddressesInRom

# FPGA golden address
romTableOffsetInBytes=28
romTableEntry=$((16#${binTableAddress["FPGA_GOLDEN_TABLE_ADDRESS"]}))
storeBinTableAddressesInRom

# FPGA current address
romTableOffsetInBytes=32
romTableEntry=$((16#${binTableAddress["FPGA_CURRENT_TABLE_ADDRESS"]}))
storeBinTableAddressesInRom
    exit
fi

# Build GE
# Check GE was populated
# GEBUILD_RESULT: we can't use OR in if statement in batch file, so create another variable to decide about GE build
if [ -f "${GEROOT}build/makefile" ]
then
# Build GE if GE files are not in directory
    # if [ ! -d "${GESRC}" ]
    # then
    #     GEBUILD_RESULT="true"
    # else
    #     GEBUILD_RESULT="false"
    # fi

    # if [ ! "$NEWGE" = "" ]
    # then
    #     GEBUILD_RESULT="true"
    # fi

    # if [ "$GEBUILD_RESULT" == "true" ]
    # then
        # module swap
        if [ ! "$BUILDGE" = "" ]
        then
            module unload project/blackbird/v1.0_sw
            module load project/goldenears/v1.0
            echo ""
            echo "**** GE Operation(s) ****"
            # tag check
            if [ ! "$GESOURCETAG" = "" ]
            then
                (cd 'gw_sw' && git checkout "${GESOURCETAG}")
            fi

        # copy binary if defined
            if [ ! "$GESOURCEBIN" = "" ]
            then
                if [ ! -d "${GESRC}" ]
                then
                    mkdir "${GESRC}"
                fi
                # copy ge FW and flashwriter
                cp -f "${RELEASEDROOT}${GESOURCEBIN}/build_dir${GEBIN}" ".${GEBIN}"
                cp -f "${RELEASEDROOT}${GESOURCEBIN}/build_dir${GEFWBIN}" ".${GEFWBIN}"
                # copy XML's needed by json stuff for Cobs
                cp -f "${RELEASEDROOT}${GESOURCEBIN}/build_dir${GEXMLCLM}" ".${GEXMLCLM}"
                cp -f "${RELEASEDROOT}${GESOURCEBIN}/build_dir${GEXMLGRG}" ".${GEXMLGRG}"
                cp -f "${RELEASEDROOT}${GESOURCEBIN}/build_dir${GEXMLULMII}" ".${GEXMLULMII}"
                cp -f "${RELEASEDROOT}${GESOURCEBIN}/build_dir${GEXMLXCSR}" ".${GEXMLXCSR}"
                cp -f "${RELEASEDROOT}${GESOURCEBIN}/build_dir${GEXMLXLR}" ".${GEXMLXLR}"
                cp -f "${RELEASEDROOT}${GESOURCEBIN}/build_dir${GEXMLXRR}" ".${GEXMLXRR}"
                cp -f "${RELEASEDROOT}${GESOURCEBIN}/build_dir${GEICOMP}" ".${GEICOMP}"
                cp -f "${RELEASEDROOT}${GESOURCEBIN}/build_dir${GEICMD}" ".${GEICMD}"
                cp -f "${RELEASEDROOT}${GESOURCEBIN}/build_dir${GEICMDRESP}" ".${GEICMDRESP}"
                cp -f "${RELEASEDROOT}${GESOURCEBIN}/build_dir${GEILOG}" ".${GEILOG}"
            else
                (cd "${GEROOT}build" && eval "${ARG1}" && eval "${ARG2}" )
            fi

            module unload project/goldenears/v1.0
            module load project/blackbird/v1.0_sw
            module unload xilinx/v14.6
            echo ">>> $(date)"
        # fi
        # echo ">>> ${GETARGS}"
        if [ "${GETARGS}" == "Y" ]
        then
            exit 0
        fi
    fi

else
    echo ""
    echo "WARNING: GE is missing build/makefile, did you forget to: ";
    echo "git submodule init";
    echo "git submodule update"
    exit
fi

if [ ! "$BUILDPGMBB" = "" ]
then
    echo ""
    echo "**** Building ProgramBB.bin ****"
    # tag check
    if [ ! "$PGMSOURCETAG" = "" ]
    then
        git checkout "${PGMSOURCETAG}"
    fi

    # copy binary if defined
    if [ ! "$PGMSOURCEBIN" = "" ]
    then
        if [ ! -d "${PBSRC}" ]
        then
            mkdir "${PBSRC}"
        fi
        cp -f "${RELEASEDROOT}${PGMSOURCEBIN}/build_dir${PBBIN}" ".${PBBIN}"
    else
    # Setup variables for extracting last released ROM location to attain rom's syms.elf file
        FSRC="build/build_configuration.py"
        ROMSYMSELF=$(getRomSymsElf)
        # Remove Python single quotes
        ROMSYMSELF=${ROMSYMSELF//\'/}
        # if testing ROM replace the syms.elf with locally built one
        if [ "${ARG2}" == "m AHBROM=1" ]
        then
            cp -f \
            "rom/blackbird_rom_bin/blackbird_rom.syms.elf" \
            "program/build/blackbird_rom.syms.elf"
        else
            cp -f "${RELEASEDROOT_ROM}${ROMSYMSELF}/build_dir/rom/blackbird_rom_bin/blackbird_rom.syms.elf" "program/build/blackbird_rom.syms.elf"
            #cp -f "rom/blackbird_rom_bin/blackbird_rom.syms.elf" "program/build/blackbird_rom.syms.elf"
        fi
        (cd "program/build" && eval "${ARG1}" && eval "${ARG2}")
    fi
    echo ">>> $(date)"
    echo ""
    # exit forced so release script will be happy
    if [ "${GETARGS}" == "Y" ]
    then
        exit 0
    fi
fi

if [ ! "$BUILDBB" = "" ]
then
    echo "**** Blackbird Operation(s) ****"
    # Setup variables for extracting last released ROM location to attain rom's syms.elf file
    FSRC="build/build_configuration.py"
    ROMSYMSELF=$(getRomSymsElf)
    # Remove Python single quotes
    ROMSYMSELF=${ROMSYMSELF//\'/}
    # tag check
    if [ ! "${BBSOURCETAG}" = "" ]
    then
        git checkout "${BBSOURCETAG}"
    fi
    # if testing ROM replace the syms.elf with locally built one
    if [ "${ARG2}" == "m AHBROM=1" ]
    then
        cp -f \
        "rom/blackbird_rom_bin/blackbird_rom.syms.elf" \
        "build/blackbird_rom.syms.elf"
    else
        # Copy previously committed ROM sys.elf file
        cp -f "${RELEASEDROOT_ROM}${ROMSYMSELF}/build_dir/rom/blackbird_rom_bin/blackbird_rom.syms.elf" "build/blackbird_rom.syms.elf"
        #cp -f "rom/blackbird_rom_bin/blackbird_rom.syms.elf" "build/blackbird_rom.syms.elf"

    fi
    (cd build && eval "${ARG1}" && eval "${ARG2}" )
# else
    # exit
fi

if [ ! -f "${BBSRC_DEBUG}target.bin" ]
then
    echo "BB DEBUG target.elf does not exist! Cannot build bbge.icron! Exiting"
    exit
fi

if [ ! "$NON_DEBUG_BB" = "" ]
then
    if [ ! -f "${BBSRC}target.bin" ]
    then
        echo "BB target.elf does not exist! Cannot build bbge.icron! Exiting"
        exit
    fi
fi

if [ ! -f "${GESRC}ge_asic_bb_companion_debug.bin" ]
then
    echo "GE ge_asic_bb_companion_debug.elft does not exist! Cannot build bbge.icron! Exiting"
    exit
fi

    echo ">>> $(date)"

echo ""
echo "Creating icron for Cobs"
# build for new hobbes
FGSRC="fg_icron_files"

# copy these for now until we autogenerate
cp "${FGSRC}/hobbes_buttons.json" "${BBSRC_DEBUG}hobbes_buttons.json"
cp "${FGSRC}/ichannel_id.json" "${BBSRC_DEBUG}ichannel_id.json"
cp "${FGSRC}/ilog_level.json" "${BBSRC_DEBUG}ilog_level.json"
cp "${FGSRC}/excom_channel_id.json" "${BBSRC_DEBUG}excom_channel_id.json"

# copy GE's xml files
cp "${GESRC}clm_comp.xml" "${BBSRC_DEBUG}clm_comp.xml"
cp "${GESRC}grg_comp.xml" "${BBSRC_DEBUG}grg_comp.xml"
cp "${GESRC}ulmii_comp.xml" "${BBSRC_DEBUG}ulmii_comp.xml"
cp "${GESRC}xcsr_comp.xml" "${BBSRC_DEBUG}xcsr_comp.xml"
cp "${GESRC}xlr_comp.xml" "${BBSRC_DEBUG}xlr_comp.xml"
cp "${GESRC}xrr_comp.xml" "${BBSRC_DEBUG}xrr_comp.xml"

# rename files for the tar operation
cp "${PBSRC}programBB.bin" "${BBSRC_DEBUG}bb_flash_writer.bin"
# why this image? it's bundled into target.bin!
cp "${GESRC}ge_asic_bb_companion_debug.bin" "${BBSRC_DEBUG}ge_asic_debug"

# create JSON files for BB
python3 "${FGSRC}/icron_to_json.py" --icomponent "${BBSRC_DEBUG}icomponent" "${BBSRC_DEBUG}bb_icomponent.json"
python3 "${FGSRC}/icron_to_json.py" --icmd "${BBSRC_DEBUG}icmd" "${BBSRC_DEBUG}icmdresp" "${BBSRC_DEBUG}bb_icmd.json"
python3 "${FGSRC}/icron_to_json.py" --ilog "${BBSRC_DEBUG}ilog" "${BBSRC_DEBUG}bb_ilog.json"
python3 "${FGSRC}/icron_to_json.py" --istatus "${FGSRC}/istatus" "${BBSRC_DEBUG}bb_istatus.json"

# create JSON files for GE
python3 "${FGSRC}/icron_to_json.py" --icomponent "${GESRC}icomponent" "${BBSRC_DEBUG}ge_icomponent.json"
python3 "${FGSRC}/icron_to_json.py" --icmd "${GESRC}icmd" "${GESRC}icmdresp" "${BBSRC_DEBUG}ge_icmd.json"
python3 "${FGSRC}/icron_to_json.py" --ilog "${GESRC}ilog" "${BBSRC_DEBUG}ge_ilog.json"


FSRC="./build/project.mak"
FPGASRC_MAVERICK=$(getFpgaSrcLocationMaverick)
FPGASRC_RAVEN=$(getFpgaSrcLocationRaven)


if [ ! "$USB" = "" ]
then
#"/data/engdev/designs/blackbird/working/robertcr/fpga/multiboot/BLACKBIRD_20170307T183442/src/f_blackbird_a7_core/build/bb_top/bb_top_fallback.runs/impl_1/bb_top.bin" \
    cp -f \
    "${FPGASRC_RAVEN}/f_blackbird_a7_core/bit_fallback/bb_top.bin" \
        "${BBSRC_DEBUG}golden_lex_fpga.bin"

#"/data/engdev/designs/blackbird/working/robertcr/fpga/multiboot/BLACKBIRD_20170307T183442/src/f_blackbird_a7_core/build/bb_top/bb_top_fallback.runs/impl_1/bb_top.bin" \
    cp -f  \
    "${FPGASRC_RAVEN}/f_blackbird_a7_core/bit_fallback/bb_top.bin" \
        "${BBSRC_DEBUG}golden_rex_fpga.bin"
# copy current
    cp -f \
    "${FPGASRC_RAVEN}/f_blackbird_a7_core/bit/bb_top.bin" \
        "${BBSRC_DEBUG}current_lex_fpga.bin"
    chmod 777 "${BBSRC_DEBUG}current_lex_fpga.bin"
python3 "${FGSRC}/adjust_fpga_file.py" --crc "${BBSRC_DEBUG}current_lex_fpga.bin"

    cp -f  \
    "${FPGASRC_RAVEN}/f_blackbird_a7_core/bit/bb_top.bin" \
        "${BBSRC_DEBUG}current_rex_fpga.bin"
    chmod 777 "${BBSRC_DEBUG}current_rex_fpga.bin"
python3 "${FGSRC}/adjust_fpga_file.py" --crc "${BBSRC_DEBUG}current_rex_fpga.bin"

else
#"/data/engdev/designs/blackbird/working/robertcr/fpga/multiboot/BLACKBIRD_20170307T183442/src/f_blackbird_a7_core/build/bb_top/bb_top_fallback.runs/impl_1/bb_top.bin" \
    cp -f \
    "${FPGASRC_MAVERICK}/f_blackbird_a7_core/bit_fallback/bb_top.bin" \
        "${BBSRC_DEBUG}golden_lex_fpga.bin"

#"/data/engdev/designs/blackbird/working/robertcr/fpga/multiboot/BLACKBIRD_20170307T183442/src/f_blackbird_a7_core/build/bb_top/bb_top_fallback.runs/impl_1/bb_top.bin" \
    cp -f  \
    "${FPGASRC_MAVERICK}/f_blackbird_a7_core/bit_fallback/bb_top.bin" \
        "${BBSRC_DEBUG}golden_rex_fpga.bin"

    cp -f \
    "${FPGASRC_MAVERICK}/f_blackbird_a7_core/bit/bb_top.bin" \
        "${BBSRC_DEBUG}current_raven_lex_fpga.bin"
    chmod 777 "${BBSRC_DEBUG}current_raven_lex_fpga.bin"
python3 "${FGSRC}/adjust_fpga_file.py" --crc "${BBSRC_DEBUG}current_raven_lex_fpga.bin"

    cp -f  \
    "${FPGASRC_MAVERICK}/f_blackbird_a7_core/bit/bb_top.bin" \
        "${BBSRC_DEBUG}current_raven_rex_fpga.bin"
    chmod 777 "${BBSRC_DEBUG}current_raven_rex_fpga.bin"
python3 "${FGSRC}/adjust_fpga_file.py" --crc "${BBSRC_DEBUG}current_raven_rex_fpga.bin"

    cp -f \
    "${FPGASRC_MAVERICK}/f_blackbird_a7_core/bit_lex/bb_top.bin" \
        "${BBSRC_DEBUG}current_lex_fpga.bin"
    chmod 777 "${BBSRC_DEBUG}current_lex_fpga.bin"
python3 "${FGSRC}/adjust_fpga_file.py" --crc "${BBSRC_DEBUG}current_lex_fpga.bin"
    cp -f  \
    "${FPGASRC_MAVERICK}/f_blackbird_a7_core/bit_rex/bb_top.bin" \
        "${BBSRC_DEBUG}current_rex_fpga.bin"
    chmod 777 "${BBSRC_DEBUG}current_rex_fpga.bin"
python3 "${FGSRC}/adjust_fpga_file.py" --crc "${BBSRC_DEBUG}current_rex_fpga.bin"
fi

if [ ! "$NON_DEBUG_BB" = "" ]
then
# copy these for now until we autogenerate
    cp "${FGSRC}/hobbes_buttons.json" "${BBSRC}hobbes_buttons.json"
    cp "${FGSRC}/ichannel_id.json" "${BBSRC}ichannel_id.json"
    cp "${FGSRC}/ilog_level.json" "${BBSRC}ilog_level.json"
    cp "${FGSRC}/excom_channel_id.json" "${BBSRC}excom_channel_id.json"

    # copy GE's xml files
    cp "${GESRC}clm_comp.xml" "${BBSRC}clm_comp.xml"
    cp "${GESRC}grg_comp.xml" "${BBSRC}grg_comp.xml"
    cp "${GESRC}ulmii_comp.xml" "${BBSRC}ulmii_comp.xml"
    cp "${GESRC}xcsr_comp.xml" "${BBSRC}xcsr_comp.xml"
    cp "${GESRC}xlr_comp.xml" "${BBSRC}xlr_comp.xml"
    cp "${GESRC}xrr_comp.xml" "${BBSRC}xrr_comp.xml"

    cp "${PBSRC}programBB.bin" "${BBSRC}bb_flash_writer.bin"
    cp "${GESRC}ge_asic_bb_companion_debug.bin" "${BBSRC}ge_asic_debug"

    # create JSON files for BB
    python3 "${FGSRC}/icron_to_json.py" --icomponent "${BBSRC}icomponent" "${BBSRC}bb_icomponent.json"
    python3 "${FGSRC}/icron_to_json.py" --icmd "${BBSRC}icmd" "${BBSRC}icmdresp" "${BBSRC}bb_icmd.json"
    python3 "${FGSRC}/icron_to_json.py" --ilog "${BBSRC}ilog" "${BBSRC}bb_ilog.json"
    python3 "${FGSRC}/icron_to_json.py" --istatus "${FGSRC}/istatus" "${BBSRC}bb_istatus.json"

# create JSON files for GE
    python3 "${FGSRC}/icron_to_json.py" --icomponent "${GESRC}icomponent" "${BBSRC}ge_icomponent.json"
    python3 "${FGSRC}/icron_to_json.py" --icmd "${GESRC}icmd" "${GESRC}icmdresp" "${BBSRC}ge_icmd.json"
    python3 "${FGSRC}/icron_to_json.py" --ilog "${GESRC}ilog" "${BBSRC}ge_ilog.json"

    if [ ! "$USB" = "" ]
    then
    #"/data/engdev/designs/blackbird/working/robertcr/fpga/multiboot/BLACKBIRD_20170307T183442/src/f_blackbird_a7_core/build/bb_top/bb_top_fallback.runs/impl_1/bb_top.bin" \
        cp -f \
        "${FPGASRC_RAVEN}/f_blackbird_a7_core/bit_fallback/bb_top.bin" \
            "${BBSRC}golden_lex_fpga.bin"

    #"/data/engdev/designs/blackbird/working/robertcr/fpga/multiboot/BLACKBIRD_20170307T183442/src/f_blackbird_a7_core/build/bb_top/bb_top_fallback.runs/impl_1/bb_top.bin" \
        cp -f  \
        "${FPGASRC_RAVEN}/f_blackbird_a7_core/bit_fallback/bb_top.bin" \
            "${BBSRC}golden_rex_fpga.bin"
    # copy current
        cp -f \
        "${FPGASRC_RAVEN}/f_blackbird_a7_core/bit/bb_top.bin" \
            "${BBSRC}current_lex_fpga.bin"
        chmod 777 "${BBSRC}current_lex_fpga.bin"
        python3 "${FGSRC}/adjust_fpga_file.py" --crc "${BBSRC}current_lex_fpga.bin"

        cp -f  \
        "${FPGASRC_RAVEN}/f_blackbird_a7_core/bit/bb_top.bin" \
            "${BBSRC}current_rex_fpga.bin"
        chmod 777 "${BBSRC}current_rex_fpga.bin"
        python3 "${FGSRC}/adjust_fpga_file.py" --crc "${BBSRC}current_rex_fpga.bin"
    else
    #"/data/engdev/designs/blackbird/working/robertcr/fpga/multiboot/BLACKBIRD_20170307T183442/src/f_blackbird_a7_core/build/bb_top/bb_top_fallback.runs/impl_1/bb_top.bin" \
        cp -f \
        "${FPGASRC_MAVERICK}/f_blackbird_a7_core/bit_fallback/bb_top.bin" \
            "${BBSRC}golden_lex_fpga.bin"

    #"/data/engdev/designs/blackbird/working/robertcr/fpga/multiboot/BLACKBIRD_20170307T183442/src/f_blackbird_a7_core/build/bb_top/bb_top_fallback.runs/impl_1/bb_top.bin" \
        cp -f  \
        "${FPGASRC_MAVERICK}/f_blackbird_a7_core/bit_fallback/bb_top.bin" \
            "${BBSRC}golden_rex_fpga.bin"

        cp -f \
        "${FPGASRC_MAVERICK}/f_blackbird_a7_core/bit/bb_top.bin" \
            "${BBSRC}current_raven_lex_fpga.bin"
        chmod 777 "${BBSRC}current_raven_lex_fpga.bin"
        python3 "${FGSRC}/adjust_fpga_file.py" --crc "${BBSRC}current_raven_lex_fpga.bin"

        cp -f  \
        "${FPGASRC_MAVERICK}/f_blackbird_a7_core/bit/bb_top.bin" \
            "${BBSRC}current_raven_rex_fpga.bin"
        chmod 777 "${BBSRC}current_raven_rex_fpga.bin"
        python3 "${FGSRC}/adjust_fpga_file.py" --crc "${BBSRC}current_raven_rex_fpga.bin"

        cp -f \
        "${FPGASRC_MAVERICK}/f_blackbird_a7_core/bit_lex/bb_top.bin" \
            "${BBSRC}current_lex_fpga.bin"
        chmod 777 "${BBSRC}current_lex_fpga.bin"
        python3 "${FGSRC}/adjust_fpga_file.py" --crc "${BBSRC}current_lex_fpga.bin"
        cp -f  \
        "${FPGASRC_MAVERICK}/f_blackbird_a7_core/bit_rex/bb_top.bin" \
            "${BBSRC}current_rex_fpga.bin"
        chmod 777 "${BBSRC}current_rex_fpga.bin"
        python3 "${FGSRC}/adjust_fpga_file.py" --crc "${BBSRC}current_rex_fpga.bin"
    fi
fi
# copy all of the fpga files and add length and crc64 to the files

# create icron header json files
FPGASRC="/data/engdev/designs/blackbird/released/dd/BLACKBIRD_20170420T123740/build"
LEXFPGARLSNOTES="${FPGASRC}/f_blackbird_a7_core/rlsnotes_f_blackbird_a7_core_lex"
REXFPGARLSNOTES="${FPGASRC}/f_blackbird_a7_core/rlsnotes_f_blackbird_a7_core_rex"

python3 "${FGSRC}/iheader_generator.py" -internal "${LEXFPGARLSNOTES}" "${REXFPGARLSNOTES}" "${BBSRC_DEBUG}/icron_header_golden.json"
python3 "${FGSRC}/iheader_generator.py" -internal "${LEXFPGARLSNOTES}" "${REXFPGARLSNOTES}" "${BBSRC_DEBUG}/icron_header_current.json"
python3 "${FGSRC}/iheader_generator.py" -external "${LEXFPGARLSNOTES}" "${REXFPGARLSNOTES}" "${BBSRC_DEBUG}/excom_header.json"

if [ ! "$NON_DEBUG_BB" = "" ]
then
    python3 "${FGSRC}/iheader_generator.py" -internal "${LEXFPGARLSNOTES}" "${REXFPGARLSNOTES}" "${BBSRC}/icron_header_golden.json"
    python3 "${FGSRC}/iheader_generator.py" -internal "${LEXFPGARLSNOTES}" "${REXFPGARLSNOTES}" "${BBSRC}/icron_header_current.json"
    python3 "${FGSRC}/iheader_generator.py" -external "${LEXFPGARLSNOTES}" "${REXFPGARLSNOTES}" "${BBSRC}/excom_header.json"
fi
#dd bs=1 skip=117 \
#    if="/data/engdev/designs/blackbird/working/robertcr/fpga/blackbird_rom/f_blackbird_a7_core/build/bb_top/bb_top_lex.runs/impl_1/bb_top.bit" \
#    of="${BBSRC}lex_fpga.bin"
#dd bs=1 skip=117 \
#    if="/data/engdev/designs/blackbird/working/robertcr/fpga/blackbird_rom/f_blackbird_a7_core/build/bb_top/bb_top_rex.runs/impl_1/bb_top.bit" \
#    of="${BBSRC}rex_fpga.bin"

echo ""
echo "*** Updating image table CRCs ***"
# ** Target.bin **
python3 "${FGSRC}/crcAdjust.py" --crc "${BBSRC_DEBUG}target.bin" "0"
# ** BB mainFW **
python3 "${FGSRC}/crcAdjust.py" --crc "${BBSRC_DEBUG}target.bin" "32"
# ** programBB **
python3 "${FGSRC}/crcAdjust.py" --crc "${BBSRC_DEBUG}target.bin" "64"
# ** GE flashwriter **
python3 "${FGSRC}/crcAdjust.py" --crc "${BBSRC_DEBUG}target.bin" "96"
# ** GE FW **
python3 "${FGSRC}/crcAdjust.py" --crc "${BBSRC_DEBUG}target.bin" "128"

# ** Target.bin **
if [ ! "$NON_DEBUG_BB" = "" ]
then
python3 "${FGSRC}/crcAdjust.py" --crc "${BBSRC}target.bin" "0"
# ** BB mainFW **
python3 "${FGSRC}/crcAdjust.py" --crc "${BBSRC}target.bin" "32"
# ** programBB **
python3 "${FGSRC}/crcAdjust.py" --crc "${BBSRC}target.bin" "64"
# ** GE flashwriter **
python3 "${FGSRC}/crcAdjust.py" --crc "${BBSRC}target.bin" "96"
# ** GE FW **
python3 "${FGSRC}/crcAdjust.py" --crc "${BBSRC}target.bin" "128"
fi

# ** BB mainFW **
# read offset in flash and size from table
entryBase=$((10#${binTable["BB_FW_START"]} * 4))
# calcAndStoreCRC
FSRC="./inc/options.h"
putFirmwareVersion

# ** GE FW **
# read offset in flash and size from table
entryBase=$((10#${binTable["GE_FW_START"]} * 4))
# calcAndStoreCRC
FSRC="./bb_ge/inc/options.h"
putFirmwareVersion


#FOR DEBUG BUILD
cp "${BBSRC_DEBUG}icron_header_golden.json" "${BBSRC_DEBUG}icron_header.json"
# create the new GOLDEN icron file for Cobs
cd "${BBSRC_DEBUG}"
tar czf cobs_golden_debug.icron icron_header.json bb_flash_writer.bin target.bin bb_ilog.json bb_icmd.json \
bb_icomponent.json clm_comp.xml ge_asic_debug ge_icmd.json ge_icomponent.json ge_ilog.json \
grg_comp.xml ichannel_id.json ilog_level.json bb_istatus.json \
ulmii_comp.xml xcsr_comp.xml xlr_comp.xml xrr_comp.xml bb_chip_a7_regs.ipxact.xml \
golden_lex_fpga.bin golden_rex_fpga.bin
cd ..
    echo ">>> $(date)"

if [ ! "$USB" = "" ]
then
    # create the new CURRENT icron file for Cobs
    cp "${BBSRC_DEBUG}icron_header_current.json" "${BBSRC_DEBUG}icron_header.json"
    cd "${BBSRC_DEBUG}"
    tar czf cobs_current_raven_debug.icron icron_header.json bb_flash_writer.bin target.bin bb_ilog.json bb_icmd.json \
    bb_icomponent.json clm_comp.xml ge_asic_debug ge_icmd.json ge_icomponent.json ge_ilog.json \
    grg_comp.xml ichannel_id.json ilog_level.json bb_istatus.json \
    ulmii_comp.xml xcsr_comp.xml xlr_comp.xml xrr_comp.xml bb_chip_a7_regs.ipxact.xml \
    current_lex_fpga.bin current_rex_fpga.bin
    cd ..
        echo ">>> $(date)"

    cd "${BBSRC_DEBUG}"
    tar czf ExCOM_debug.icron excom_header.json bb_flash_writer.bin target.bin \
    ge_asic_debug \
    excom_channel_id.json bb_istatus.json \
    current_lex_fpga.bin current_rex_fpga.bin
    cd ..
        echo ">>> $(date)"
else
    # create the new CURRENT icron file for Cobs
    if [ ! "$PLUG_TEST" = "" ]
    then
    cp "${BBSRC_DEBUG}icron_header_current.json" "${BBSRC_DEBUG}icron_header.json"
    cd "${BBSRC_DEBUG}"
    tar czf cobs_plug_test_debug.icron icron_header.json bb_flash_writer.bin target.bin bb_ilog.json bb_icmd.json \
    bb_icomponent.json clm_comp.xml ge_asic_debug ge_icmd.json ge_icomponent.json ge_ilog.json \
    grg_comp.xml ichannel_id.json ilog_level.json bb_istatus.json \
    ulmii_comp.xml xcsr_comp.xml xlr_comp.xml xrr_comp.xml bb_chip_a7_regs.ipxact.xml \
    current_lex_fpga.bin current_rex_fpga.bin
    cd ..
        echo ">>> $(date)"
    else
    cp "${BBSRC_DEBUG}icron_header_current.json" "${BBSRC_DEBUG}icron_header.json"
    cd "${BBSRC_DEBUG}"
    tar czf cobs_current_debug.icron icron_header.json bb_flash_writer.bin target.bin bb_ilog.json bb_icmd.json \
    bb_icomponent.json clm_comp.xml ge_asic_debug ge_icmd.json ge_icomponent.json ge_ilog.json \
    grg_comp.xml ichannel_id.json ilog_level.json bb_istatus.json \
    ulmii_comp.xml xcsr_comp.xml xlr_comp.xml xrr_comp.xml bb_chip_a7_regs.ipxact.xml \
    current_lex_fpga.bin current_rex_fpga.bin
    cd ..
        echo ">>> $(date)"
    fi
    cd "${BBSRC_DEBUG}"
    tar czf ExCOM_debug.icron excom_header.json bb_flash_writer.bin target.bin \
    ge_asic_debug \
    excom_channel_id.json bb_istatus.json \
    current_raven_lex_fpga.bin current_raven_rex_fpga.bin \
    current_lex_fpga.bin current_rex_fpga.bin
    cd ..
        echo ">>> $(date)"
fi
# create the new icron file for LogReader


#FOR NON DEBUG BUILD
if [ ! "$NON_DEBUG_BB" = "" ]
then
    cp "${BBSRC}icron_header_golden.json" "${BBSRC}icron_header.json"
    # create the new GOLDEN icron file for Cobs
    cd "${BBSRC}"
    tar czf cobs_golden.icron icron_header.json bb_flash_writer.bin target.bin bb_ilog.json bb_icmd.json \
    bb_icomponent.json  clm_comp.xml ge_asic_debug ge_icmd.json ge_icomponent.json ge_ilog.json \
    grg_comp.xml ichannel_id.json ilog_level.json bb_istatus.json \
    ulmii_comp.xml xcsr_comp.xml xlr_comp.xml xrr_comp.xml bb_chip_a7_regs.ipxact.xml \
    golden_lex_fpga.bin golden_rex_fpga.bin
    cd ..
        echo ">>> $(date)"

    if [ ! "$USB" = "" ]
    then
        # create the new CURRENT icron file for Cobs
        cp "${BBSRC}icron_header_current.json" "${BBSRC}icron_header.json"
        cd "${BBSRC}"
        tar czf cobs_current_raven.icron icron_header.json bb_flash_writer.bin target.bin bb_ilog.json bb_icmd.json \
        bb_icomponent.json  clm_comp.xml ge_asic_debug ge_icmd.json ge_icomponent.json ge_ilog.json \
        grg_comp.xml ichannel_id.json ilog_level.json bb_istatus.json \
        ulmii_comp.xml xcsr_comp.xml xlr_comp.xml xrr_comp.xml bb_chip_a7_regs.ipxact.xml \
        current_lex_fpga.bin current_rex_fpga.bin
        cd ..
            echo ">>> $(date)"

        cd "${BBSRC}"
        tar czf ExCOM.icron excom_header.json bb_flash_writer.bin target.bin \
        ge_asic_debug \
        excom_channel_id.json bb_istatus.json \
        current_lex_fpga.bin current_rex_fpga.bin
        cd ..
            echo ">>> $(date)"
    else
        if [ ! "$PLUG_TEST" = "" ]
        then
        cp "${BBSRC}icron_header_current.json" "${BBSRC}icron_header.json"
        cd "${BBSRC}"
        tar czf cobs_plug_test.icron icron_header.json bb_flash_writer.bin target.bin bb_ilog.json bb_icmd.json \
        bb_icomponent.json  clm_comp.xml ge_asic_debug ge_icmd.json ge_icomponent.json ge_ilog.json \
        grg_comp.xml ichannel_id.json ilog_level.json bb_istatus.json \
        ulmii_comp.xml xcsr_comp.xml xlr_comp.xml xrr_comp.xml bb_chip_a7_regs.ipxact.xml \
        current_lex_fpga.bin current_rex_fpga.bin
        cd ..
            echo ">>> $(date)"
        else
        cp "${BBSRC}icron_header_current.json" "${BBSRC}icron_header.json"
        cd "${BBSRC}"
        tar czf cobs_current.icron icron_header.json bb_flash_writer.bin target.bin bb_ilog.json bb_icmd.json \
        bb_icomponent.json  clm_comp.xml ge_asic_debug ge_icmd.json ge_icomponent.json ge_ilog.json \
        grg_comp.xml ichannel_id.json ilog_level.json bb_istatus.json \
        ulmii_comp.xml xcsr_comp.xml xlr_comp.xml xrr_comp.xml bb_chip_a7_regs.ipxact.xml \
        current_lex_fpga.bin current_rex_fpga.bin
        cd ..
            echo ">>> $(date)"
        fi
        
        cd "${BBSRC}"
        tar czf ExCOM.icron excom_header.json bb_flash_writer.bin target.bin \
        ge_asic_debug \
        excom_channel_id.json bb_istatus.json \
        current_raven_lex_fpga.bin current_raven_rex_fpga.bin \
        current_lex_fpga.bin current_rex_fpga.bin
        cd ..
            echo ">>> $(date)"
    fi

    # create the new icron file for LogReader

fi

# create a mcs which includes all golden and current binaries
# The change-address argument specifies which address the MCS file should start
if [ ! "$MCSFILE" = "" ]
then
    cd "${BBSRC}"
    if [ ! "$USB" = "" ]
    then
        objcopy -I binary --gap-fill 0xFF --pad-to 0x0A00000 ./golden_lex_fpga.bin ./91-00376_raven.bin
        cat ./target.bin >> ./91-00376_raven.bin
        objcopy -I binary --gap-fill 0xFF --pad-to 0x1000000 ./91-00376_raven.bin ./91-00376_raven.bin
        cat ./current_lex_fpga.bin >> ./91-00376_raven.bin
        objcopy -I binary --gap-fill 0xFF --pad-to 0x1A00000 ./91-00376_raven.bin ./91-00376_raven.bin
        cat ./target.bin >> ./91-00376_raven.bin
        objcopy -I binary -O ihex --change-address 0x0000000 ./91-00376_raven.bin ./91-00376_raven.mcs

    else
        objcopy -I binary --gap-fill 0xFF --pad-to 0x0A00000 ./golden_lex_fpga.bin ./91-00377.bin
        cat ./target.bin >> ./91-00377.bin
        objcopy -I binary --gap-fill 0xFF --pad-to 0x1000000 ./91-00377.bin ./91-00377.bin
        cat ./current_lex_fpga.bin >> ./91-00377.bin
        objcopy -I binary --gap-fill 0xFF --pad-to 0x1A00000 ./91-00377.bin ./91-00377.bin
        cat ./target.bin >> ./91-00377.bin
        objcopy -I binary -O ihex --change-address 0x0000000 ./91-00377.bin ./91-00377.mcs

        objcopy -I binary --gap-fill 0xFF --pad-to 0x0A00000 ./golden_rex_fpga.bin ./91-00378.bin
        cat ./target.bin >> ./91-00378.bin
        objcopy -I binary --gap-fill 0xFF --pad-to 0x1000000 ./91-00378.bin ./91-00378.bin
        cat ./current_rex_fpga.bin >> ./91-00378.bin
        objcopy -I binary --gap-fill 0xFF --pad-to 0x1A00000 ./91-00378.bin ./91-00378.bin
        cat ./target.bin >> ./91-00378.bin
        objcopy -I binary -O ihex --change-address 0x0000000 ./91-00378.bin ./91-00378.mcs

    fi
    objcopy -I binary -O ihex --change-address 0x000000 "../${GEBIN}" ./91-00379.mcs
fi
