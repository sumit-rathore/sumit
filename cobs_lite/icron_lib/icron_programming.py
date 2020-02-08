import struct

class RunProgramBBCommand():
    def __init__(self, command=1, subcommand=4):
        self.__command = command
        self.__subcommand = subcommand

    @property
    def as_integer_list(self):
        struct_bytes = b''
        struct_bytes += struct.pack('B', self.__command)
        struct_bytes += struct.pack('B', self.__subcommand)
        return [ord(byte) for byte in struct_bytes]

    def __str__(self):
        return "RunProgramBBCommand struct: command={} ".format(self.__command) + \
                "subcommand={} ".format(self.__subcommand)

class RunProgramBBCommandResponse():
    def __init__(self, packet_payload):
        if len(packet_payload) != 1:
            raise InvalidCommandResponseError(
                    "Invalid programming BB response",
                    packet_payload)
        else:
            self.__command_accepted= packet_payload[0]

    @property
    def is_ack(self):
        return True if self.__command_accepted == 1 else False

class ProgramGECommand():
    def __init__(self, command=1, subcommand=3):
        self.__command = command
        self.__subcommand = subcommand

    @property
    def as_integer_list(self):
        struct_bytes = b''
        struct_bytes += struct.pack('B', self.__command)
        struct_bytes += struct.pack('B', self.__subcommand)
        return [ord(byte) for byte in struct_bytes]

    def __str__(self):
        return "ProgramGECommand struct: command={} ".format(self.__command) + \
                "subcommand={} ".format(self.__subcommand)


class ProgramGECommandResponse():
    def __init__(self, packet_payload):
        if len(packet_payload) != 1:
            raise InvalidCommandResponseError(
                    "Invalid programming GE response",
                    packet_payload)
        else:
            self.__command_accepted= packet_payload[0]

    @property
    def is_ack(self):
        return True if self.__command_accepted == 1 else False


class InvalidCommandResponseError(Exception):
    def __init__(self, message, packet_payload):
        self.message = message
        self.packet_payload = packet_payload

    def __str__(self):
        return "{}: ".format(self.message) + \
                "{}".format(self.packet_payload)


class ProgramStartCommand():
    def __init__(
            self,
            region,
            image_size,
            image_crc,
            command=1,
            subcommand=1):

        #TODO change command and subcommand to string
        self.__command = command
        self.__subcommand = subcommand
        self.__fill = 0
        self.__region_num = ProgramStartCommand.get_region_num(region)
        self.__image_size = image_size
        self.__image_crc = image_crc

    @staticmethod
    def get_region_num(region_name):
        switcher_program_region= {
                "PROGRAM_REGION_FLASHWRITER":          0, 
                "PROGRAM_REGION_FPGA_FW_GOLDEN":    0x10,
                "PROGRAM_REGION_FW_GOLDEN":         0x11,        
                "PROGRAM_REGION_FPGA_FW_CURRENT":   0x20,  
                "PROGRAM_REGION_FW_CURRENT":        0x21,       
        }
        return switcher_program_region[region_name]

    @property
    def as_integer_list(self):
        struct_bytes = b''
        struct_bytes += struct.pack('B', self.__command)
        struct_bytes += struct.pack('B', self.__subcommand)
        struct_bytes += struct.pack('B', self.__region_num)
        struct_bytes += struct.pack('B', self.__fill)
        struct_bytes += struct.pack('>L', self.__image_size)
        struct_bytes += struct.pack('>L', self.__image_crc)
        return [ord(byte) for byte in struct_bytes]

    def __str__(self):
        return "ProgramStartCommand struct: command={} ".format(self.__command) + \
                "subcommand={} ".format(self.__subcommand) + \
                "region_num={} ".format(self.__region_num) + \
                "fill={} ".format(self.__fill) + \
                "image_size={} ".format(self.__image_size) + \
                "image_crc={}".format(self.__image_crc)


class ProgramDeviceResponse():
    def __init__(self, packet_payload):
        if len(packet_payload) != 4: 
            raise InvalidCommandResponseError(
                    "Invalid program device response",
                    packet_payload)
        else:
            self.__command_accepted= packet_payload[0]
            self.__fill = packet_payload[1]
            blocks_to_erase_bytes = struct.pack('B', packet_payload[2])
            blocks_to_erase_bytes += struct.pack('B', packet_payload[3])
            self.__blocks_to_erase = struct.unpack('>H', blocks_to_erase_bytes)[0]

    @property
    def is_ack(self):
        return True if self.__command_accepted == 1 else False

    @property
    def blocks_to_erase(self):
        return self.__blocks_to_erase


class EraseBlockCommand():
    def __init__(self, command=1, subcommand=2):
        self.__command = command
        self.__subcommand = subcommand

    @property
    def as_integer_list(self):
        struct_bytes = b''
        struct_bytes += struct.pack('B', self.__command)
        struct_bytes += struct.pack('B', self.__subcommand)
        return [ord(byte) for byte in struct_bytes]

    def __str__(self):
        return "EraseBlockCommand struct: command={} ".format(self.__command) + \
                "subcommand={} ".format(self.__subcommand)


class EraseBlockCommandResponse():
    def __init__(self, packet_payload):
        if len(packet_payload) != 4:
            raise InvalidCommandResponseError("Invalid erase block response", packet_payload)
        else:
            self.__command_accepted = packet_payload[0]
            self.__fill = packet_payload[1]
            blocks_to_erase_bytes = struct.pack('B', packet_payload[2])
            blocks_to_erase_bytes += struct.pack('B', packet_payload[3])
            self.__blocks_to_erase = struct.unpack('>H', blocks_to_erase_bytes)[0]

    @property
    def is_ack(self):
        return True if self.__command_accepted == 1 else False

    @property
    def blocks_to_erase(self):
        return self.__blocks_to_erase


class EraseCommandResponseError(Exception):
    def __init__(self, message, response):
        self.mesage = message
        self.response = response

    def __str__(self):
        return "{}: ".format(message) + \
                "response={}".format(response)


class ProgramDataCommand():
    def __init__(self, data):
        self.__data = data

    @property
    def as_integer_list(self):
        return self.__data


class ProgramDataCommandResponse():
    def __init__(self, packet_payload):
        if len(packet_payload) != 1:
            raise InvalidCommandResponseError(
                    "Invalid programming data response",
                    packet_payload)
        else:
            self.__command_accepted= packet_payload[0]

    @property
    def is_ack(self):
        return True if self.__command_accepted == 1 else False


class ProgramDeviceNakError(Exception):
    def __init__(self, message):
        self. message = message

    def __str__(self):
        return self.message


class ProgramDeviceTimeoutError(Exception):
    def __init__(self, message):
        self. message = message

    def __str__(self):
        return self.message


class InvalidProgrammingModeError(Exception):
    def __init__(self, message, mode, image_type):
        self.message = message
        self.mode = mode
        self.image_type = image_type

    def __str__(self):
        return "InvalidProgrammingModeError: {} ".format(self.message) + \
                "mode: {}".format(self.mode) + \
                "image: {}".format(self.image_type)


class InvalidProgrammingOperationError(Exception):
    def __init__(self, message):
        self.message = message

    def __str__(self):
        return "Invalid programming operation: {}".format(self.message)

class UnknownDeviceTypeProgrammingError(Exception):
    def __init__(self, message, device_type):
        self.message = message
        self.device_type = device_type

    def __str__(self):
        return "Unknown device type for programming: {}".format(self.message) + \
                "device type: {}".format(self.device_type)

    @property
    def is_ack(self):
        return True if self.__command_accepted == 1 else False
