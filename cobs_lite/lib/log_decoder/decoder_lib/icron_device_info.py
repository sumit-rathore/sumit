def decode_info_message(packet_payload_data):
    info_header = InfoMessageHeader(packet_payload_data[:4])
    if info_header.packet_size != len(packet_payload_data):
        raise InvalidMessageSizeError(
                "Invalid packet size for the device info message",
                packet_payload_data)
    info = info_header.info_constructor(packet_payload_data[4:])
    message = str(info) if info_header.bootrom_info or info_header.software_info else None
    return message

class InfoMessageHeader():
    def __init__(self, data):
        if len(data) != 4:
            raise InvalidMessageSizeError("Invalid device info message header size", data)
        self.message_id = data[0]
        self.secondary_message_id = data[1]
        self.protocol_version = data[2]
        if self.protocol_version != 0:
            raise UnsupportedInfoMessageError(
                    "Unsupported device info message protocol",
                    self.message_id,
                    self.secondary_message_id,
                    self.protocol_version,
                    self.fill)
        self.fill = data[3]

    @property
    def message_id(self):
        return self.message_id

    @property
    def secondary_message_id(self):
        return self.secondary_message_id

    @property
    def protocol_version(self):
        return self.protocol_version

    @property
    def fill(self):
        return self.fill

    @property
    def bootrom_info(self):
        return self.message_id == 0 and self.secondary_message_id == 1

    @property
    def software_info(self):
        return self.message_id == 0 and self.secondary_message_id == 2

    @property
    def software_info_fw(self):
        return self.message_id == 0 and self.secondary_message_id == 3

    @property
    def packet_size(self):
        try:
            switcher_packet_size = {
                    (0, 1):  20, # sizeof(struct BootromVersion) in BootRom 
                    (0, 2):  30, # sizeof(struct oftwareVersion) in ProgramBB 
                    (0, 3):  30, # sizeof(struct SoftwareVersion) in MainFirmWare  
                    (1, 1):   8, # sizeof(struct EraseBlock) for GE
                    (1, 2):   8, # sizeof(struct ProgramBlock) for GE
                    (2, 1):   8, # sizeof(struct EraseBlock) for FPGA
                    (2, 2):   8  # sizeof(struct ProgramBlock) for FPGA
            }
            return switcher_packet_size[(self.message_id, self.secondary_message_id)] 
        except KeyError:
            raise UnsupportedInfoMessageError(
                    "Got an error when try to verify packet size",
                    self.message_id,
                    self.secondary_message_id,
                    self.protocol_version,
                    self.fill)

    @property
    def info_constructor(self):
        try:
            info_constructors = {
                    (0, 1): BootromVersionInfo,
                    (0, 2): ProgramBbVersionInfo,
                    (0, 3): SoftwareVersionInfo,
                    (1, 1): EraseBlockInfo,
                    (1, 2): ProgramBlockInfo
            }
            return info_constructors[(self.message_id, self.secondary_message_id)]
        except KeyError:
            raise UnsupportedInfoMessageError(
                    "Got an error when try to get info constructor",
                    self.message_id,
                    self.secondary_message_id,
                    self.protocol_version,
                    self.fill)

    def __str__(self):
        return "message_id={}, secondary_message_id={}, protocol_version={}, fill={}". \
                format(self.message_id, self.secondary_message_id, self.protocol_version, self.fill)


class UnsupportedInfoMessageError():
    def __init__(self, message, message_id, secondary_message_id, protocol_version, fill):
        self.message = message
        self.message_id = message_id
        self.secondary_message_id = secondary_message_id
        self.protocol_version = protocol_version
        self.fill = fill

    def __str__(self):
        return "{}: ".format(self.message) + \
                "message_id={}, ".format(self.message_id) + \
                "secondary_message_id={}, ".format(self.secondary_message_id) + \
                "protocol_version={}, ".format(self.protocol_version) + \
                "fill={} ".format(self.fill)


class FPGAVersionInfo():
    def __init__(self, data):
        if len(data) != 12:
            raise InvalidMessageSizeError("Invalid FPGA version info size", data)
        self.major_revision = data[0] << 8 | data[1]
        self.minor_revision = data[2]
        self.debug_revision = data[3]
        self.build_year = data[4] << 8 | data[5]
        self.build_month = data[6]
        self.build_day = data[7]
        self.build_hour = data[8]
        self.build_minute = data[9]
        self.build_second = data[10]
        self.fallbackImage = data[11]

    @property
    def major_revision(self):
        return self.major_revision

    @property
    def minor_revision(self):
        return self.minor_revision

    @property
    def debug_revision(self):
        return self.debug_revision

    @property
    def build_year(self):
        return self.build_year

    @property
    def build_month(self):
        return self.build_month

    @property
    def build_day(self):
        return self.build_day

    @property
    def build_hour(self):
        return self.build_hour

    @property
    def build_minute(self):
        return self.build_minute
        
    @property
    def build_second(self):
        return self.build_second

    @property
    def build_time(self):
        return "{:04x}".format(self.build_year) + \
                "{:02x}".format(self.build_month) + \
                "{:02x}".format(self.build_day) + \
                "{:02x}".format(self.build_hour) + \
                "{:02x}".format(self.build_minute) + \
                "{:02x}".format(self.build_second)

    @property
    def fallbackImage(self):
        return self.fallbackImage

    def __str__(self):
        # Digital design team uses hex number to represent the build time, i.e build_year=0x2016
        # or 8214 in decimal. So we need {:x}.format(build_year) to give 2016 for the correct value
        return "FPGA Chip ID Major {} ".format(hex(self.major_revision)[2:]) + \
                "Minor {} ".format(hex(self.minor_revision)[2:]) + \
                "Debug {} ".format(hex(self.debug_revision)[2:]) + \
                "HW build was done on {:04x}/".format(self.build_year) + \
                "{:02x}/".format(self.build_month) + \
                "{:02x} ".format(self.build_day) + \
                "at {:02x}:".format(self.build_hour) + \
                "{:02x}:".format(self.build_minute) + \
                "{:02x}".format(self.build_second) + \
                " FallbackStatus {}".format(hex(self.fallbackImage))


class BootromVersionInfo():
    def __init__(self, data):
        if len(data) != 16:
            raise InvalidMessageSizeError("Invalid Bootrom version info size", data)
        self.fpga_info = FPGAVersionInfo(data[:12])
        self.major_revision = data[12]
        self.minor_revision = data[13]
        self.is_lex = data[14]

    @property
    def fpga_info(self):
        return self.fpga_info

    @property
    def fpga_build_time(self):
        return self.fpga_info.build_time

    @property
    def major_revision(self):
        return self.major_revision

    @property
    def minor_revision(self):
        return self.minor_revision

    @property
    def lex(self):
        return self.is_lex == 0

    def __str__(self):
        device_type = "LEX" if self.lex else "REX"
        return "Bootrom Info message: {}\n".format(str(self.fpga_info)) + \
                "Bootrom ID Major {} ".format(self.major_revision) + \
                "Minor {} ".format(self.minor_revision) + \
                "this is {}\n".format(device_type)


class ProgramBbVersionInfo():
    def __init__(self, data):
        if len(data) != 26:
            raise InvalidMessageSizeError("Invalid programBb version info size", data)
        self.fpga_info = FPGAVersionInfo(data[:12])
        self.rom_major_revision = data[12]
        self.rom_minor_revision = data[13]
        self.major_revision = data[14]
        self.minor_revision = data[15]
        self.debug_revision = data[16]
        self.fill = data[17]
        self.build_year = data[18] << 8 | data[19]
        self.build_month = data[20]
        self.build_day = data[21]
        self.build_hour = data[22]
        self.build_minute = data[23]
        self.build_second = data[24]
        self.is_lex = data[25]

    @property
    def fpga_info(self):
        return self.fpga_info

    @property
    def fpga_build_time(self):
        return self.fpga_info.build_time

    @property
    def rom_major_revision(self):
        return self.rom_major_revision

    @property
    def rom_minor_revision(self):
        return self.rom_minor_revision

    @property
    def major_revision(self):
        return self.major_revision

    @property
    def minor_revision(self):
        return self.minor_revision

    @property
    def debug_revision(self):
        return self.debug_revision

    @property
    def build_year(self):
        return self.build_year

    @property
    def build_month(self):
        return self.build_month

    @property
    def build_day(self):
        return self.build_day

    @property
    def build_hour(self):
        return self.build_hour

    @property
    def build_minute(self):
        return self.build_minute
        
    @property
    def build_second(self):
        return self.build_second

    @property
    def lex(self):
        return self.is_lex == 0

    @property
    def fill(self):
        return self.fill

    def __str__(self):
        device_type = "LEX" if self.lex else "REX"
        return "ProgramBb Info message: {}\n".format(str(self.fpga_info)) + \
                "Bootrom ID Major {} ".format(self.rom_major_revision) + \
                "Minor {} ".format(self.rom_minor_revision) + \
                "Software revision Major {} ".format(self.major_revision) + \
                "Minor {} ".format(self.minor_revision) + \
                "Debug {} ".format(self.debug_revision) + \
                "SW build was done on {}/".format(self.build_year) + \
                "{:02d}/".format(self.build_month) + \
                "{:02d} ".format(self.build_day) + \
                "at {:02d}:".format(self.build_hour) + \
                "{:02d}:".format(self.build_minute) + \
                "{:02d} ".format(self.build_second) + \
                "this is {}\n".format(device_type)

class SoftwareVersionInfo():
    def __init__(self, data):
        if len(data) != 26:
            raise InvalidMessageSizeError("Invalid software version info size", data)
        self.fpga_info = FPGAVersionInfo(data[:12])
        self.rom_major_revision = data[12]
        self.rom_minor_revision = data[13]
        self.major_revision = data[14]
        self.minor_revision = data[15]
        self.debug_revision = data[16]
        self.fill = data[17]
        self.build_year = data[18] << 8 | data[19]
        self.build_month = data[20]
        self.build_day = data[21]
        self.build_hour = data[22]
        self.build_minute = data[23]
        self.build_second = data[24]
        self.is_lex = data[25]

    @property
    def fpga_info(self):
        return self.fpga_info

    @property
    def fpga_build_time(self):
        return self.fpga_info.build_time

    @property
    def rom_major_revision(self):
        return self.rom_major_revision

    @property
    def rom_minor_revision(self):
        return self.rom_minor_revision

    @property
    def major_revision(self):
        return self.major_revision

    @property
    def minor_revision(self):
        return self.minor_revision

    @property
    def debug_revision(self):
        return self.debug_revision

    @property
    def build_year(self):
        return self.build_year

    @property
    def build_month(self):
        return self.build_month

    @property
    def build_day(self):
        return self.build_day

    @property
    def build_hour(self):
        return self.build_hour

    @property
    def build_minute(self):
        return self.build_minute
        
    @property
    def build_second(self):
        return self.build_second

    @property
    def lex(self):
        return self.is_lex == 0

    @property
    def fill(self):
        return self.fill

    def __str__(self):
        device_type = "LEX" if self.lex else "REX"
        return "Software Info message: {}\n".format(str(self.fpga_info)) + \
                "Bootrom ID Major {} ".format(self.rom_major_revision) + \
                "Minor {} ".format(self.rom_minor_revision) + \
                "Software revision Major {} ".format(self.major_revision) + \
                "Minor {} ".format(self.minor_revision) + \
                "Debug {} ".format(self.debug_revision) + \
                "SW build was done on {}/".format(self.build_year) + \
                "{:02d}/".format(self.build_month) + \
                "{:02d} ".format(self.build_day) + \
                "at {:02d}:".format(self.build_hour) + \
                "{:02d}:".format(self.build_minute) + \
                "{:02d} ".format(self.build_second) + \
                "this is {}\n".format(device_type)

class ProgramBlockInfo():
    def __init__(self, data):
        if len(data) != 4:
            raise InvalidMessageSizeError("Invalid program block info size", data)
        self.block_number = data[0]
        self.region_number = data[1]
        self.total_blocks = data[2]
        self.fill = data[3]

    @property
    def block_number(self):
        return self.block_number

    @property
    def region_number(self):
        return self.region_number

    @property
    def total_blocks(self):
        return self.total_blocks

    @property
    def fill(self):
        return self.fill

    def __str__(self):
        return "Automatic GE download"

# COBS doesn't program GE anymore, it is being programmed by Blackbird but when 
# Golden and Current programs are different and device toggles from one image to another, 
# Blackbird changes GE program and COBS still notices it and prints above message
# It has been changed to avoid any confusion

"""        return "Programming GE block {} ".format(self.block_number) + \
                "in region {} ".format(self.region_number) + \
                "of total blocks of {}".format(self.total_blocks)"""


class EraseBlockInfo():
    def __init__(self, data):
        if len(data) != 4:
            raise InvalidMessageSizeError("Invalid erase block info size", data)
        self.block_number = data[0]
        self.region_number = data[1]
        self.total_blocks = data[2]
        self.fill = data[3]

    @property
    def block_number(self):
        return self.block_number

    @property
    def region_number(self):
        return self.region_number

    @property
    def total_blocks(self):
        return self.total_blocks

    @property
    def fill(self):
        return self.fill

    def __str__(self):
        return "Automatic GE erase"
"""
        return "Erasing GE block {} ".format(self.block_number) + \
                "in region {} ".format(self.region_number) + \
                "of total blocks of {}".format(self.total_blocks)
"""

class InvalidMessageSizeError(Exception):
    def __init__(self, message, data):
        self.message = message
        self.data = data 

    def __str__(self):
        return "{}: ".format(self.message) + \
                "data={}".format(self.data)
