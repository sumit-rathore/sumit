import struct 
from script_lib import icron_model
import traceback
import sys

class Icmd():
    def __init__(self, component_index, function_index, *args_list):
        """
        Construct an icmd object.
        """
        self.__component_index = component_index
        self.__function_index = function_index
        if len(args_list) == 0:
            self.__args_list = []
        else:
            if len(args_list[0]) / 4 > 6 or len(args_list[0]) % 4 != 0:
                raise IcmdFormatError(
                        "Too many icmd arguments",
                        component_index,
                        function_index,
                        args_list)
            self.__args_list = args_list[0]

    @property  
    def as_integer_list(self):
        icmd_header_signature = 0b10011
        if len(self.args_list) == 0:
            header = icmd_header_signature << 3
        else:
            header = icmd_header_signature << 3 | (len(self.args_list) / 4)
        icmd = [header, self.component_index, self.function_index] 
        icmd.extend(self.args_list)
        return icmd

    @property
    def component_index(self):
        return self.__component_index

    @property
    def function_index(self):
        return self.__function_index

    @property
    def args_list(self):
        return self.__args_list


class IcmdFormatError(Exception):
    def __init__(self, message, component_index, function_index, arguments):
        self.message = message
        self.component_index = component_index
        self.function_index = function_index
        self.arguments = arguments

    def __str__(self):
        return "{}: ".format(self.message) + \
                "component_string={}".format(self.component_string) + \
                "function_string={}".format(self.function_string) + \
                "arguments={}".format(self.arguments)


class IcmdEncoder():
    def __init__(self, icmd_model):
        self.icmd_model = icmd_model

    def encode(self, component_string, function_string, *arguments):
        component_index = self.icmd_model.get_icomponent_index(component_string)
        function_index= self.icmd_model.get_icmd_function_index(component_string, function_string)
        num_args = self.icmd_model.get_icmd_num_args(component_string, function_string)

        # check if arguments tuple is empty 
        if len(arguments) == 0:
            return Icmd(component_index, function_index)
        else:
            # check if the size of the given arguments list matches the number of args defined in
            # the icmd.json file
            if len(arguments[0]) == num_args:
                if num_args != 0:
                    arg_types = self.icmd_model.get_icmd_arg_types(
                                    component_string,
                                    function_string)

                    # pack each argument to 4 bytes based on the format of the defined c type
                    arg_bytes = b''
                    for arg, arg_type in zip(arguments[0], arg_types):
                        # convert arg to integer if it is a string
                        arg_int = int(arg, 0) if isinstance(arg, str) else int(arg)
                        pack_format = IcmdEncoder.get_pack_format(arg_type)
                        pack_byte = struct.pack(pack_format, arg_int)

                        # TODO: check if the given argument value is within the range bounded by
                        # the defined c type.  struct.pack should raise an exception if the arg_int
                        # is out of the range. However, it doesn't work for unsigned numbers i.e.
                        # 'B', 'H'. This is a bug in Ironpython and Python2.6. The IcmdValueError
                        # can be removed when we migrate to CPython.
                        if arg_int > 0 and struct.unpack('>L', pack_byte)[0] != arg_int:
                            raise IcmdArgValueError(
                                    "Icmd argument is out of range for the defined type {}". \
                                            format(arg_type),
                                    component_string,
                                    function_string,
                                    arguments)
                        arg_bytes += pack_byte
                    args_list = [ord(byte) for byte in arg_bytes]

                return Icmd(component_index, function_index, args_list)
            else:
                raise IcmdEncoderError(
                        "Icmd number of arguments unmatched",
                        component_string,
                        function_string,
                        arguments)

    @staticmethod
    def get_pack_format(arg_type, big_endian=True):
        switcher_big_endian = {
                "bool":         'xxxb',
                "boolT":        'xxxb',
                "sint8":        'xxxb',
                "sint8_t":      'xxxb',
                "uint8":        'xxxB',
                "uint8_t":      'xxxB',
                "sint16":       '>xxh',
                "sint16_t":     '>xxh',
                "uint16":       '>xxH',
                "uint16_t":     '>xxH',
                "sint32":       '>l',
                "sint32_t":     '>l',
                "uint32":       '>L',
                "uint32_t":     '>L',
                "component_t":  '>L',
        }

        switcher_little_endian = {
                "bool":         'bxxx',
                "boolT":        'bxxx',
                "sint8":        'bxxx',
                "sint8_t":      'bxxx',
                "uint8":        'Bxxx',
                "uint8_t":      'Bxxx',
                "sint16":       '<hxx',
                "sint16_t":     '<hxx',
                "uint16":       '<Hxx',
                "uint16_t":     '<Hxx',
                "sint32t":      '<l',
                "sint32_t":     '<l',
                "uint32":       '<L',
                "uint32_t":     '<L',
                "component_t":  '<L',
        }
        return switcher_big_endian[arg_type] if big_endian else \
                switcher_little_endian[arg_type]

class IcmdEncoderError(Exception):
    def __init__(self, message, component_string, function_string, arguments):
        self.message = message
        self.component_string = component_string
        self.function_string = function_string
        self.arguments = arguments

    def __str__(self):
        return "Icmd encoding error: {}: ".format(self.message) + \
                "component_string={}, ".format(self.component_string) + \
                "function_string={}, ".format(self.function_string) + \
                "arguments={}".format(self.arguments)

class IcmdArgValueError(Exception):
    def __init__(self, message, component_string, function_string, arguments):
        self.message = message
        self.component_string = component_string
        self.function_string = function_string
        self.arguments = arguments

    def __str__(self):
        return "{}: ".format(self.message) + \
                "component_string={}, ".format(self.component_string) + \
                "function_string={}, ".format(self.function_string) + \
                "arguments={}".format(self.arguments)
