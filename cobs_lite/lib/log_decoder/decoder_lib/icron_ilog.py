import struct
import decoder_lib.icron_model

def parse_ilog(payload_bytes):
    header_byte = payload_bytes[0]
    is_big_endian = (header_byte >> 3) & 0x1 == 0x1
    is_prev_log_printed = (header_byte >> 2) & 0x1 == 0x1
    num_args = header_byte & 0x3
    component_index = payload_bytes[1]
    message_index = payload_bytes[2]
    severity_level_index = payload_bytes[3]
    device_timestamp = \
        payload_bytes[4] << 24 | payload_bytes[5] << 16 | payload_bytes[6] << 8 | payload_bytes[7]
    arguments = payload_bytes[8:]
    # verify the length of the payload, the definition of first 8 bytes in ilog is shown above
    # i.e. header_byte = payload_bytes[0]. Plus, each argument consists of 4bytes.
    if num_args * 4 + 8 == len(payload_bytes):
        return ILog(
                is_big_endian,
                is_prev_log_printed,
                num_args,
                component_index,
                message_index,
                severity_level_index,
                device_timestamp,
                arguments) 
    else:
        raise ILogFormatError("Invalid ilog format", payload_bytes)


class ILogFormatError(Exception):
    def __init__(self, message, icron_payload):
        self.icron_payload = icron_payload
        self.message = message

    def __str__(self):
        return self.message + ": " + repr(self.icron_payload)


class ILog():
    def __init__(self,
                 is_big_endian,
                 is_prev_log_printed,
                 num_args,
                 component_index,
                 message_index,
                 severity_level_index,
                 device_timestamp,
                 arguments):
        self.__is_big_endian = is_big_endian
        self.__is_prev_log_printed = is_prev_log_printed
        self.__num_args = num_args
        self.__component_index = component_index
        self.__message_index = message_index
        self.__severity_level_index = severity_level_index
        self.__device_timestamp = device_timestamp
        self.__arguments = arguments

    @property
    def is_big_endian(self):
        return self.__is_big_endian

    @property
    def is_prev_log_printed(self):
        return self.__is_prev_log_printed

    @property
    def num_args(self):
        return self.__num_args

    @property
    def component_index(self):
        return self.__component_index

    @property
    def message_index(self):
        return self.__message_index

    @property
    def severity_level_index(self):
        return self.__severity_level_index

    @property
    def event_log(self):
        return self.severity_level_index < 3

    @property
    def user_log(self):
        return self.severity_level_index == 6

    @property
    def device_timestamp(self):
        return self.__device_timestamp

    @property
    def arguments(self):
        args_list = []
        for i in range(self.num_args):
            arg_bytes = b''
            for j in range(4):
                arg_bytes += struct.pack('B', self.__arguments[4 * i + j])
            arg_value = struct.unpack('>L' if self.is_big_endian else '<L', arg_bytes)
            args_list.append(arg_value[0])
        return tuple(args_list)

    def __str__(self):
        """
        Debugging logs
        """
        return "ilog: is_big_endian={}, prev_ilog_printed={}, ". \
                    format(self.is_big_endian, self.is_prev_log_printed) + \
                "num_args={}, component_index={}, message_index={}, ". \
                    format(self.num_args, self.component_index, self.message_index) + \
                "severity_index={}, device_timestamp={}, arguments={}". \
                    format(self.severity_level_index, self.device_timestamp, self.arguments)


class ILogDecoder():
    def __init__(self, ilog_model):
        self.ilog_model = ilog_model

    def decode(self, ilog):
        try:
            severity_string = self.ilog_model.get_severity(ilog.severity_level_index)
            icomponent_string = self.ilog_model.get_icomponent_string(ilog.component_index)
            ilog_string = self.ilog_model.get_ilog_string(icomponent_string, ilog.message_index)
            ilog_message = ilog_string %(ilog.arguments)
            return "{:010d}".format(ilog.device_timestamp) + ": " + \
                    severity_string + ": " + \
                    icomponent_string.ljust(22) + ": " + \
                    ilog_message, \
                    icomponent_string
        except Exception as e:
            raise ILogDecodeError("iLog decode error: {}: {}".format(type(e), str(e)), ilog)


class ILogDecodeError(Exception):
    def __init__(self, message, ilog):
        self.ilog = ilog
        self.message = message

    def __str__(self):
        return self.message + ": " + str(self.ilog)
