import struct
import icron_programming as ip

class ResetDeviceCommand():
    def __init__(self, command=2, subcommand=1):
        self.__command = command
        self.__subcommand = subcommand

    @property
    def as_integer_list(self):
        struct_bytes = b''
        struct_bytes += struct.pack('B', self.__command)
        struct_bytes += struct.pack('B', self.__subcommand)
        return [ord(byte) for byte in struct_bytes]

    def __str__(self):
        return "ResetDeviceCommand struct: command={} ".format(self.__command) + \
                "subcommand={} ".format(self.__subcommand)

class ResetDeviceCommandResponse():
    def __init__(self, packet_payload):
        if len(packet_payload) != 1:
            raise ip.InvalidCommandResponseError(
                    "Invalid reset device response",
                    packet_payload)
        else:
            self.__command_accepted= packet_payload[0]

    @property
    def is_ack(self):
        return True if self.__command_accepted == 1 else False


class QueryDeviceInfoCommand():
    def __init__(self, command=2, subcommand=2):
        self.__command = command
        self.__subcommand = subcommand

    @property
    def as_integer_list(self):
        struct_bytes = b''
        struct_bytes += struct.pack('B', self.__command)
        struct_bytes += struct.pack('B', self.__subcommand)
        return [ord(byte) for byte in struct_bytes]

    def __str__(self):
        return "QueryDeviceInfoCommand struct: command={} ".format(self.__command) + \
                "subcommand={} ".format(self.__subcommand)

class QueryDeviceInfoCommandResponse():
    def __init__(self, packet_payload):
        if len(packet_payload) != 1:
            raise ip.InvalidCommandResponseError(
                    "Invalid query device info response",
                    packet_payload)
        else:
            self.__command_accepted= packet_payload[0]

    @property
    def is_ack(self):
        return True if self.__command_accepted == 1 else False
