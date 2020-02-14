####################################################################################################
## Icron Technology Corporation - Copyright 2016
##
## This source file and the information contained in it are confidential and proprietary to Icron
## Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
## of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
## any employee of Icron who has not previously obtained written authorization for access from the
## individual responsible for the source code, will have a significant detrimental effect on Icron
## and is expressly prohibited.
####################################################################################################
##
##!   @file  - i2cd_idt_clk.py
##
##!   @brief - This script parses diff registers from the Default_v0.tcs, LEX_v0.tcs,
##             REX_NO_SSC_v0.tcs and REX_SSC_v0.tcs files.
##
##!   @note  - This script requires is part of IDT clock generator 5P49V6913 driver. One has to
##             execute this script to generate i2cd_idt_clk_cfg.c file for the implementation of the
##             driver.
####################################################################################################
##  Usage:
##      python3 i2cd_idt_clk.py diff Default_v0.tcs LEX_v0.tcs REX_NO_SSC_v0.tcs REX_SSC_v0.tcs >
##          i2cd_idt_clk_cfg.c
####################################################################################################
import collections
import re
import sys

def print_usage():
    """
    Print out the usage of this script.
    """
    sys.stderr.write("Usage: \n" \
           "\ti2cd_idt_clk.py diff [default.tcs] [lex.tcs] [rex_no_ssc.tcs] [rex_ssc.tcs] > " \
                "[i2cd_idt_clk_cfg.c] \n")

_Reg = collections.namedtuple('Register', ['value', 'offset'])

def parse_register_map(tcs_file):
    """
    Parse the IDT clk generator register map from a tcs file.
    """
    with open(tcs_file, 'r') as f:
        reg_map = []
        for line in f:
            reg_map.extend(_get_register(line))
        return reg_map


def _get_register(line):
    """
    Extract register offset and value from each line of the tcs file.
    """
    register = re.search(r'^\w{2}\s+\d{8}\s+(?P<value>\w{2})\s(?P<offset>\w{2})$', line)
    if register is not None:
        yield _Reg(value = register.group('value'), offset = register.group('offset'))


def _assert_offset(ref_reg_map, reg_map):
    """
    Assert if the register offset between the two register maps do not match.
    """
    assert ref_reg_map.offset == reg_map.offset, \
            "Offset {} does not match.".format(ref_reg_map.offset)
    return ref_reg_map.offset == reg_map.offset


def _assert_offset_multi_3(ref_reg_map, reg_map1, reg_map2, reg_map3):
    """
    Assert if the register offset between the two register maps do not match.
    """
    assert ref_reg_map.offset == reg_map1.offset == reg_map2.offset == reg_map3.offset, \
            "Offset {} does not match.".format(ref_reg_map.offset)
    return ref_reg_map.offset == reg_map1.offset == reg_map2.offset == reg_map3.offset


def _assert_offset_multi_6(ref_reg_map, reg_map1, reg_map2, reg_map3, reg_map4, reg_map5, reg_map6):
    """
    Assert if the register offset between the two register maps do not match.
    """
    assert ref_reg_map.offset == reg_map1.offset == reg_map2.offset == reg_map3.offset == reg_map4.offset == reg_map5.offset == reg_map6.offset, \
            "Offset {} does not match.".format(ref_reg_map.offset)
    return ref_reg_map.offset == reg_map1.offset == reg_map2.offset == reg_map3.offset == reg_map4.offset == reg_map5.offset == reg_map6.offset


def diff_register_map(ref_reg_map, reg_map):
    """
    Diff two register maps and return the resulting diff register.
    """
    assert len(ref_reg_map) == len(reg_map), \
            "Size of register maps are not equal."
    diff_reg = [j for (i, j) in zip(ref_reg_map, reg_map) if i != j and _assert_offset(i, j)]
    return diff_reg


def diff_default_map_6(ref_reg_map, reg_map1, reg_map2, reg_map3, reg_map4, reg_map5, reg_map6):
    """
    Diff two register maps and return the resulting diff register.
    """
    assert len(ref_reg_map) == len(reg_map1) == len(reg_map2) == len(reg_map3) == len(reg_map4) == len(reg_map5) == len(reg_map6), \
            "Size of register maps are not equal."
    diff_reg = [i for (i, j1, j2, j3, j4, j5, j6) in zip(ref_reg_map, reg_map1, reg_map2, reg_map3, reg_map4, reg_map5, reg_map6) if (i != j1 or i !=j2 or i !=j3 or i !=j4 or i !=j5 or i !=j6) and _assert_offset_multi_6(i, j1, j2, j3, j4, j5, j6)]
    return diff_reg


def diff_default_map_3(ref_reg_map, reg_map1, reg_map2, reg_map3):
    """
    Diff two register maps and return the resulting diff register.
    """
    assert len(ref_reg_map) == len(reg_map1) == len(reg_map2) == len(reg_map3), \
            "Size of register maps are not equal."
    diff_reg = [i for (i, j1, j2, j3) in zip(ref_reg_map, reg_map1, reg_map2, reg_map3) if (i != j1 or i !=j2 or i !=j3) and _assert_offset_multi_3(i, j1, j2, j3)]
    return diff_reg


def write_diff_config(diff_config_map, diff_config_name):
    """
    Print the diff register and its size in C format.
    """
    print("const struct IdtClkRegMap {}[] =\n".format(diff_config_name) + "{")
    last = len(diff_config_map) - 1
    for i, reg in enumerate(diff_config_map):
        s = "}" if i == last else "},"
        print("\t{." + "regOffset = {}, .regValue = {}".format(hex(int(reg.offset, 16)), \
            hex(int(reg.value, 16))) + s)
    print("};")
    print("const uint8_t {0}Size = sizeof({0}) / sizeof({0}[0]);\n".format(diff_config_name))


if __name__ == '__main__':
    if len(sys.argv) == 6:      # old IDT configuration until A02
        operation = sys.argv[1]
        default_tcs = sys.argv[2]
        lex_tcs = sys.argv[3]
        rex_tcs = sys.argv[4]
        rex_ssc_tcs = sys.argv[5]

        if operation == 'diff':
            default_reg_map = parse_register_map(default_tcs)
            lex_reg_map = parse_register_map(lex_tcs)
            rex_reg_map = parse_register_map(rex_tcs)
            rex_ssc_reg_map = parse_register_map(rex_ssc_tcs)
            lex_diff_reg = diff_register_map(default_reg_map, lex_reg_map)
            rex_diff_reg = diff_register_map(default_reg_map, rex_reg_map)
            rex_enable_ssc_diff_reg = diff_register_map(rex_reg_map, rex_ssc_reg_map)
            rex_disable_ssc_diff_reg = diff_register_map(rex_ssc_reg_map, rex_reg_map)
            Set_default_reg = diff_default_map_3(default_reg_map, lex_reg_map, rex_reg_map, rex_ssc_reg_map)

            write_diff_config(lex_diff_reg, "lexIdtClkCfg")
            write_diff_config(rex_diff_reg, "rexIdtClkCfg")
            write_diff_config(rex_enable_ssc_diff_reg, "rexIdtClkEnableSscCfg")
            write_diff_config(rex_disable_ssc_diff_reg, "rexIdtClkDisableSscCfg")
            write_diff_config(Set_default_reg, "setDefaultClkCfg")
        else:
            print_usage()

    elif len(sys.argv) == 9:    # new IDT configuration from A03
        operation = sys.argv[1]
        default_tcs = sys.argv[2]
        LRU3_tcs = sys.argv[3]
        RU3DP_tcs = sys.argv[4]
        RU3DPSSC_tcs = sys.argv[5]
        LU2_tcs = sys.argv[6]
        RU2DP_tcs = sys.argv[7]
        RU2DPSSC_tcs = sys.argv[8]

        if operation == 'diff':
            # default_reg_map = parse_register_map(default_tcs)
            default_reg_map = parse_register_map(LRU3_tcs)
            LRU3_map = parse_register_map(LRU3_tcs)
            RU3DP_map = parse_register_map(RU3DP_tcs)
            RU3DPSSC_map = parse_register_map(RU3DPSSC_tcs)
            LU2_map = parse_register_map(LU2_tcs)
            RU2DP_map = parse_register_map(RU2DP_tcs)
            RU2DPSSC_map = parse_register_map(RU2DPSSC_tcs)

            LRU3_diff_reg = diff_register_map(default_reg_map, LRU3_map)
            RU3DP_diff_reg = diff_register_map(default_reg_map, RU3DP_map)
            # RU3DPSSC_en_diff_reg = diff_register_map(RU3DP_map, RU3DPSSC_map)
            RU3DPSSC_en_diff_reg = diff_register_map(default_reg_map, RU3DPSSC_map)
            # RU3DPSSC_di_diff_reg = diff_register_map(RU3DPSSC_map, RU3DP_map)
            LU2_diff_reg = diff_register_map(default_reg_map, LU2_map)
            RU2DP_diff_reg = diff_register_map(default_reg_map, RU2DP_map)
            # RU2DPSSC_en_diff_reg = diff_register_map(RU2DP_map, RU2DPSSC_map)
            RU2DPSSC_en_diff_reg = diff_register_map(default_reg_map, RU2DPSSC_map)
            # RU2DPSSC_di_diff_reg = diff_register_map(RU2DPSSC_map, RU2DP_map)
            # Set_default_reg = diff_default_map_6(default_reg_map, LRU3_map, RU3DP_map, RU3DPSSC_map, LU2_map, RU2DP_map, RU2DPSSC_map)

            write_diff_config(LRU3_diff_reg, "IDT6914_lexRexUsb3IdtClkCfg")
            write_diff_config(RU3DP_diff_reg, "IDT6914_rexUsb3DpIdtClkCfg")
            write_diff_config(RU3DPSSC_en_diff_reg, "IDT6914_rexUsb3DpSscEnableIdtClkCfg")
            # write_diff_config(RU3DPSSC_di_diff_reg, "IDT6914_rexUsb3DpSscDisableIdtClkCfg")
            write_diff_config(LU2_diff_reg, "IDT6914_lexUsb2IdtClkCfg")
            write_diff_config(RU2DP_diff_reg, "IDT6914_rexUsb2DpIdtClkCfg")
            write_diff_config(RU2DPSSC_en_diff_reg, "IDT6914_rexUsb2DpSscEnableIdtClkCfg")
            # write_diff_config(RU2DPSSC_di_diff_reg, "IDT6914_rexUsb2DpSscDisableIdtClkCfg")
            # write_diff_config(Set_default_reg, "IDT6914_setDefaultClkCfg")

        else:
            print_usage()

    else:
        print_usage()

