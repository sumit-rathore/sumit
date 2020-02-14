import re
import sys
import json
import argparse
import os 

def define_internal_icron_header_dic(lex_fpga_timestamp, rex_fpga_timestamp, golden_current):
    icron_header_dic = \
        {
            "goldenears":
            {
                "version": "5",
                "device_name": "GE",
                "icomponent": "ge_icomponent.json",
                "ilog": "ge_ilog.json",
                "icmd": "ge_icmd.json",
                "ichannel_id": "ichannel_id.json",
                "ilog_level": "ilog_level.json",
                "flash_writer": None,
                "main_firmware": None,
                "lex_fpga_image": None,
                "rex_fpga_image": None,
                "istatus": None,
                "golden_current": None,
                "lex_fpga_build_time": str(lex_fpga_timestamp),
                "rex_fpga_build_time": str(rex_fpga_timestamp),
                "iregister": [
                    {
                        "xml_file_name": "grg_comp.xml",
                        "base_address": "0x20000000"
                    },
                    {
                        "xml_file_name": "ulmii_comp.xml",
                        "base_address": "0x20000100"
                    },
                    {
                        "xml_file_name": "clm_comp.xml",
                        "base_address": "0x20000200"
                    },
                    {
                        "xml_file_name": "xrr_comp.xml",
                        "base_address": "0x20000300"
                    },
                    {
                        "xml_file_name": "xlr_comp.xml",
                        "base_address": "0x20000400"
                    },
                    {
                        "xml_file_name": "xcsr_comp.xml",
                        "base_address": "0x20000500"
                    }
                ]
            },
            "blackbird":
            {
                "version": "4",
                "device_name": "BB",
                "icomponent": "bb_icomponent.json",
                "ilog": "bb_ilog.json",
                "icmd": "bb_icmd.json",
                "ichannel_id": "ichannel_id.json",
                "ilog_level": "ilog_level.json",
                "flash_writer": "bb_flash_writer.bin",
                "main_firmware": "target.bin",
                "lex_fpga_image": golden_current + "_lex_fpga.bin",
                "rex_fpga_image": golden_current + "_rex_fpga.bin",
                "istatus": "bb_istatus.json",
                "golden_current": golden_current,
                "lex_fpga_build_time": str(lex_fpga_timestamp),
                "rex_fpga_build_time": str(rex_fpga_timestamp),
                "iregister": [
                    {
                        "xml_file_name": "bb_chip_a7_regs.ipxact.xml",
                        "base_address": "0"
                    }
                ]
            }
        }
    return icron_header_dic

def define_external_icron_header_dic(lex_fpga_timestamp, rex_fpga_timestamp, sw_version, sw_date):
    icron_header_dic = \
        {
            "raven":
            {
                "version": "0",
                "device_name": "raven",
                "ichannel_id": "excom_channel_id.json",
                "flash_writer": "bb_flash_writer.bin",
                "main_firmware": "target.bin",
                "lex_fpga_image": "current_lex_fpga.bin",
                "rex_fpga_image": "current_rex_fpga.bin",
                "raven_lex_fpga_image": "current_raven_lex_fpga.bin",
                "raven_rex_fpga_image": "current_raven_rex_fpga.bin",
                "istatus": "bb_istatus.json",
                "golden_current": "current",
                "lex_fpga_build_time": str(lex_fpga_timestamp),
                "rex_fpga_build_time": str(rex_fpga_timestamp),
                "sw_version": str(sw_version),
                "sw_date": str(sw_date)
            }

        }
    return icron_header_dic

def get_fpga_build_date(string):
    year, month, day = None, None, None
    s = re.match(r'^DATE:\s+(?P<year>\d+)\/(?P<month>\d+)\/(?P<day>\d+)', string)
    if s is not None:
        return s.group('year'), s.group('month'), s.group('day')
    return year, month, day


def get_fpga_build_time(string):
    hour, minute, second = None, None, None
    s = re.match(r'^TIME:\s+(?P<hour>\d+)\:(?P<minute>\d+)\:(?P<second>\d+)', string)
    if s is not None:
        return s.group('hour'), s.group('minute'), s.group('second')
    return hour, minute, second

def get_fpga_build_timestamp_from_release_notes(file_name):
    with open(file_name, 'r') as f:
        year, month, day, hour, minute, second = None, None, None, None, None, None
        for line in f:
            if year is None and month is None and day is None:
                year, month, day = get_fpga_build_date(line)
            if hour is None and minute is None and second is None:
                hour, minute, second = get_fpga_build_time(line)
            if all((year, month, day, hour, minute, second)):
                fpga_timestamp = year + month + day + hour + minute + second
                return fpga_timestamp
        return None

def get_fw_version(string):
    fw_major_version, fw_minor_version, fw_debug_version = None, None, None
    s_major, s_minor, s_debug = None, None, None
    s_major = re.match(r'^#define SOFTWARE_MAJOR_REVISION\s+(?P<fw_major_version>\d+)', string)
    s_minor = re.match(r'^#define SOFTWARE_MINOR_REVISION\s+(?P<fw_minor_version>\d+)', string)
    s_debug = re.match(r'^#define SOFTWARE_DEBUG_REVISION\s+(?P<fw_debug_version>\d+)', string)
    if s_major is not None:
        return s_major.group('fw_major_version')
    if s_minor is not None:
        return s_minor.group('fw_minor_version')
    if s_debug is not None:
        return s_debug.group('fw_debug_version')
    return None

def get_fw_build_date(string):
    fw_date = None
    if re.match(r'^SOFTWARE_TIME_STAMP\s+', string):
        split     = re.split('[- = "]',string)
        fw_year   = split[3]
        fw_month  = split[6].rjust(2, '0')
        fw_date   = split[9].rjust(2, '0')
        fw_date   = fw_year + '/' + fw_month + '/' + fw_date
        # print(fw_date)
        return fw_date
    return fw_date

def get_fw_version_from_options():
    with open(os.path.join("inc", "options.h"), 'r') as f:
        fw_major_version, fw_minor_version, fw_debug_version = None, None, None
        # if fw_version is None:
        for line in f:
            if fw_major_version is None:
                fw_major_version = get_fw_version(line)
                continue
            if fw_minor_version is None:
                fw_minor_version = get_fw_version(line)
                continue
            if fw_debug_version is None:
                fw_debug_version = get_fw_version(line)
            if all ((fw_major_version, fw_minor_version, fw_debug_version)):  
                fw_version =  fw_major_version + '.' + fw_minor_version + '.' + fw_debug_version
                return fw_version    
        return None

def get_fw_date_time_from_makefile():
    with open(os.path.join("inc", "build_time.txt"), 'r') as f:
        fw_date = None
        for line in f:   
            if fw_date is None:
                fw_date = get_fw_build_date(line)
            if fw_date is not None:
                return fw_date   
        return None 

def write_to_icron_header_files(file_name, icron_header_dictionary):
    with open(file_name, 'w') as f:
        json.dump(icron_header_dictionary, f, indent=4)



if __name__ == '__main__':
    usage_string = "\n\tiheader_generator.py -internal " + \
                        "[lex_rls_notes] [rex_rls_notes] [output_file]\n" + \
                    "\tiheader_generator.py -external " + \
                        "[lex_rls_notes] [rex_rls_notes] [output_file]\n" + \
                    "\tiheader.py --help\n"
    parser = argparse.ArgumentParser(
                            prog='icron_file_header_generator',
                            usage=usage_string,
                            description='generate icron file header')
    parser.add_argument(
                '-internal',
                nargs=3,
                help='generate internal icron header file from FPGA release notes')
    
    parser.add_argument(
                '-external',
                nargs=3,
                help='generate external icron header file form FPGA release notes')

    args = parser.parse_args()

    if len(sys.argv) == 5:
        lex_release_file = sys.argv[2]

        rex_release_file = sys.argv[3]

        output_file = sys.argv[4]

        lex_fpga_timestamp = get_fpga_build_timestamp_from_release_notes(lex_release_file)
        assert(lex_fpga_timestamp), "LEX FPGA timestamp is not found"

        rex_fpga_timestamp = get_fpga_build_timestamp_from_release_notes(rex_release_file)
        assert(rex_fpga_timestamp), "REX FPGA timestamp is not found"

        sw_version = get_fw_version_from_options()
        sw_date = get_fw_date_time_from_makefile()
        if args.internal:
            if "golden" in output_file:
                dic = define_internal_icron_header_dic(lex_fpga_timestamp, rex_fpga_timestamp, "golden")
            if "current" in output_file:
                dic = define_internal_icron_header_dic(lex_fpga_timestamp, rex_fpga_timestamp, "current")
            write_to_icron_header_files(output_file, dic)

        if args.external:
            dic = define_external_icron_header_dic(lex_fpga_timestamp, rex_fpga_timestamp, sw_version, sw_date)
            write_to_icron_header_files(output_file, dic)

    else:
        parser.print_help()
