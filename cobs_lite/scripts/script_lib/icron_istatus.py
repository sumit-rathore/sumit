from script_lib import icron_model

def parse_istatus(payload_bytes):
    reserved_byte = payload_bytes[0]
    if reserved_byte != 0:
        raise IStatusFormatError("Invalid istatus format", payload_bytes)
    num_args = payload_bytes[1]
    istatus_id = payload_bytes[2] << 8 | payload_bytes[3]
    device_timestamp = \
        payload_bytes[4] << 24 | payload_bytes[5] << 16 | payload_bytes[6] << 8 | payload_bytes[7]
    arguments = payload_bytes[8:]
    if num_args * 4 + 8 == len(payload_bytes):
        return IStatus(reserved_byte, num_args, istatus_id, device_timestamp, arguments)
    else:
        raise IStatusFormatError("Invalid number of args in istatus packet", payload_bytes)


class IStatusFormatError(Exception):
    def __init__(self, message, icron_payload):
        self.icron_payload = icron_payload
        self.message = message

    def __str__(self):
        return self.message + ": " + str(self.icron_payload)


class IStatus():
    def __init__(self, reserved, num_args, istatus_id, device_timestamp, arguments):
        self.__reserved = reserved
        self.__num_args = num_args
        self.__istatus_id = istatus_id
        self.__device_timestamp = device_timestamp
        self.__arguments = arguments

    @property
    def reserved(self):
        return self.__reserved

    @property
    def num_args(self):
        return self.__num_args

    @property
    def istatus_id(self):
        return self.__istatus_id

    @property
    def device_timestamp(self):
        return self.__device_timestamp

    @property
    def arguments(self):
        args_list = []
        for i in range(self.num_args):
            arg_value = self.__arguments[4 * i] << 24 | \
                        self.__arguments[4 * i + 1] << 16 | \
                        self.__arguments[4 * i + 2] << 8 | \
                        self.__arguments[4 * i + 3]
            args_list.append(arg_value)
        return tuple(args_list)

    def __str__(self):
        return "istatus:" + \
                "reserved={}, ".format(self.reserved) + \
                "num_args={}, ".format(self.num_args) + \
                "istatus_id={}, ".format(self.istatus_id) + \
                "device_timestamp={}, ".format(self.device_timestamp) + \
                "arguments={}".format(self.arguments)


class IStatusDecoder():
    def __init__(self, istatus_model):
        self.istatus_model = istatus_model


    def decode(self, istatus):
        try:
            istatus_string = self.istatus_model.get_istatus_string(istatus.istatus_id)
            istatus_message = istatus_string %(istatus.arguments)
            return "{:010d}".format(istatus.device_timestamp) + ": " + istatus_message
        except Exception as e:
            raise IStatusDecodeError("iStatus decode error: {}: {}".format(type(e), str(e)), istatus)


class IStatusDecodeError(Exception):
    def __init__(self, message, istatus):
        self.istatus = istatus
        self.message = message

    def __str__(self):
        return self.message + ": " + str(self.istatus)

