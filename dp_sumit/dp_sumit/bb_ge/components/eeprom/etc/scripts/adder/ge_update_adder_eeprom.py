###############################################################################
###
###   Icron Technology Corporation - Copyright 2016
###
###
###   This source file and the information contained in it are confidential and
###   proprietary to Icron Technology Corporation. The reproduction or disclosure,
###   in whole or in part, to anyone outside of Icron without the written approval
###   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
###   Icron who has not previously obtained written authorization for access from
###   the individual responsible for the source code, will have a significant
###   detrimental effect on Icron and is expressly prohibited.
###
###############################################################################
##
##!   @file  - ge_update_adder_eeprom.py
##
##!   @brief - This script allows users to update the wrong configured GE EEPROM
##             page data for Adder through serial communication between a PC and
##             a GE product via Icron's debugging protocol.
##
##!   @note  - This script requires pySerial module pre-installed on the PC. To
##             install pySerial, follow the instruction here:
##             https://pythonhosted.org/pyserial/pyserial.html
##
###############################################################################


import serial
import sys
import threading
import struct
import queue
import time
from abc import ABCMeta, abstractmethod
import traceback
import collections


def print_usage():
    """
    Print out the usage of this script.
    """
    sys.stderr.write("Usage: \n" +
           "\tge_update_adder_eeprom [serial_port_name]\n")


def update_adder_eeprom(serial_port, ilog_parser):
    """
    Update the specified variables stored in the EEPROM for Adder. The EEPROM pages intended to to
    be updated are listed in adder_eeprom. The listed page data are generated in Hobbes using
    EEPROM_icmdReadPage.
    """
    EepromPage = collections.namedtuple('EepromPage', ['page_num', 'page_data'])
    adder_eeprom = [
        EepromPage(page_num = 14,
            page_data = b'\x01\x00\x04\x55\x53\x42\x20\x45\x58\x0e\x81\x72\x00\x00\x00\x00'),
        EepromPage(page_num = 15,
            page_data = b'\x01\x00\x00\x00\x00\x00\x00\x3f\x09\x0f\xe4\xb9\x00\x00\x00\x00'),
        EepromPage(page_num = 16,
            page_data = b'\x01\x54\x45\x4e\x44\x45\x52\x20\x31\x10\x4c\x15\x00\x00\x00\x00'),
        EepromPage(page_num = 17,
            page_data = b'\x01\x2e\x30\x2e\x30\x00\x00\x00\x00\x11\x31\x71\x00\x00\x00\x00'),
        EepromPage(page_num = 18,
            page_data = b'\x01\x00\x00\x00\x00\x00\x00\x00\x00\x12\x8b\x13\x00\x00\x00\x00')]

    for page in adder_eeprom:
        write_eeprom_page(serial_port, ilog_parser, page.page_num, page.page_data)
        read_data = read_eeprom_page(serial_port, ilog_parser, page.page_num)
        if read_data != page.page_data:
            assert False, "Adder EEPROM update failed at page %d.\n" % page.page_num
    print("Update succeeded!")


def read_eeprom_page(serial_port, ilog_parser, page_num, timeout=10):
    """
    Perform the EEPROM read all pages data operation by sending icmds and receiving ilogs through a
    serial port. The EEPROM on the GE platform has 32 pages with a size of 16 byte in each page.
    The entire operation is required to complete within timeout seconds. Otherwise, the operation
    fails and asserts. The EEPROM contents are written to stdout as raw binary data.
    """
    eeprom_read_page_ilog_handler = EepromReadPageIlogHandler()
    try:
        attempt = 0
        while True:
            eeprom_read_page_icmd = EepromReadPageIcmd(page_num)
            page_data_read = icmd_transaction(serial_port,
                                              ilog_parser,
                                              eeprom_read_page_icmd,
                                              eeprom_read_page_ilog_handler,
                                              timeout)

            if len(page_data_read) == 16:
                return page_data_read
            else:
                sys.stderr.write('EEPROM read operation at page %d with attempt %d failed.\n' \
                        %(page_num, attempt))
                attempt += 1
                assert (attempt < 3) , "Failed to read EEPROM page %d.\n" % page_num

    except IcmdTimeOutException as e:
        assert False, "Reading EEPROM page %d times out.\n" % page_num


def write_eeprom_page(serial_port, ilog_parser, page_num, page_data, timeout=10):
    """
    Write total_pages of data into the EEPROM by sending icmds and receiving ilogs through a serial
    port.  The EEPROM on the GE platform has 32 pages with a size of 16 byte in each page.  The
    entire operation is expected to be complete within timeout seconds. Otherwise, the operation
    fails and asserts. The EEPROM content is read from stdin.  The input data is expected to be raw
    and thus have no additional formatting.
    """
    eeprom_write_page_ilog_handler = EepromWritePageIlogHandler()
    try:
        attempt = 0
        while True:
            if attempt == 0:
                assert len(page_data) == 16, "Size of EEPROM page data is invalid\n"

                eeprom_write_page_icmd = EepromWritePageIcmd(page_num, page_data)

            write_success = icmd_transaction(serial_port,
                                             ilog_parser,
                                             eeprom_write_page_icmd,
                                             eeprom_write_page_ilog_handler,
                                             timeout)

            if write_success:
                break
            else:
                sys.stderr.write('EEPROM write operation at page %d with attempt %d failed.\n' \
                        %(page_num, attempt))
                attempt += 1
                assert (attempt < 3) , "Failed to write EEPROM page %d.\n" % page_num

    except IcmdTimeOutException as e:
        assert False, "Writing EEPROM page %d times out.\n" % page_num


def icmd_transaction(serial_port, ilog_parser, icmd, ilog_handler, timeout):
    """
    Perform an icmd transaction by sending an icmd and processing recieved ilogs. The icmd
    transaction must be complete within the given timeout period. Otherwise, an
    IcmdTimeOutException will be raised.
    """
    ilog_parser.set_recording(True)
    send_icmd(serial_port, icmd)
    start_time = time.time()
    while True:
        if time.time() - start_time > timeout:
            raise IcmdTimeOutException("Icmd transaction times out")
        else:
            try:
                if ilog_handler.consume(ilog_parser.get_ilog(0.25)):
                    ilog_parser.set_recording(False)
                    return ilog_handler.result
            except queue.Empty as e:
                pass
    ilog_parser.set_recording(False)


class IcmdTimeOutException(Exception):
    pass


def send_icmd(serial_port, icmd):
    """
    Send the icmd to the given serial port.
    """
    try:
        serial_port.write(icmd.as_bytes())
    except (serial.SerialException, serial.SerialTimeoutException) as e:
        raise Exception("Fail to send icmd to serial port:{}".format(serial_port.port))


class Icmd():
    def __init__(self, component_index, function_index, *args):
        """
        Construct an icmd object.
        """
        self.__component_index = component_index
        self.__function_index = function_index
        if len(args) > 6:
            raise ValueError("Too many arguments to Icmd")

        self.__args = args

    def as_bytes(self, big_endian=True):
        """
        Convert an icmd into bytes. The format of icmd is big endian by default.
        """
        icmd_header_signature = 0b10011
        header_byte = icmd_header_signature << 3 | len(self.__args)

        args_bytes = b''
        arg_format = '>L' if big_endian else '<L'
        for x in self.__args:
            args_bytes += struct.pack(arg_format, x)

        return bytes([header_byte, self.__component_index, self.__function_index]) +  args_bytes


class EepromReadPageIcmd(Icmd):
    def __init__(self, page_num, display_as_bytes=True):
        """
        Construct an EEPROM read page icmd object. Display EEPROM page data as bytes by default. If
        data displayed as words, make sure to change the corresponding parameter in EEPROM read page
        handler as well.
        """
        component_index = 0x20
        function_index = 0
        super(EepromReadPageIcmd, self). \
            __init__(component_index, function_index, page_num, 0 if display_as_bytes else 1)


class EepromWritePageIcmd(Icmd):
    def __init__(self, page_num, byte_data):
        """
        Construct an EEPROM write page Icmd object.
        """
        component_index = 0x20
        function_index = 1
        assert (len(byte_data) == 16), "Incorrect amount of data for EEPROM page"
        args = [struct.unpack(">L", byte_data[i:i+4])[0] for i in range(0, 16, 4)]
        super(EepromWritePageIcmd, self). \
                __init__(component_index, function_index, page_num, *args)


class IlogHandler(metaclass=ABCMeta):
    @abstractmethod
    def consume(self, ilog):
        """
        Consumes the ilogs after sending an icmd. The derived classes have to implement this
        method based on how the ilogs should be handled.  This method should return True if the
        result is ready to be read or False otherwise.
        """
        pass

    @property
    @abstractmethod
    def result(self):
        pass


class EepromReadPageIlogHandler(IlogHandler):
    """
    This class handles the ilogs after EEPROM read page icmd is issued. Results of EEPROM read page
    operation can be classified as failure or success which is 16 bytes of data. The failure case
    is represented by a result of None.
    """
    def __init__(self, data_as_bytes=True):
        """
        Constuct an EEPROM read page ilog haAdler object. Display EEPROM page data as bytes by
        default. If page data displayed as words, make sure to change the corresponding parameter in
        EEPROM read page icmd as well.
        """
        self.__data_as_bytes = data_as_bytes
        self.__component_index = 0x20
        self.__message_index_fail = 2
        self.__message_index_data = 6 if data_as_bytes else 7
        self.__expecting_index = 0

        # A bytes object representing the value of arguments in the given CPU architecture
        self.__data = b''

        # It represents the result of reading EEPROM page data. If read succeeds, it is an bytes
        # object that contains the page data. Otherwise, it is None.
        self.__result = None

    def consume(self, ilog):
        """
        Consume the ilogs after sending EEPROM read page icmd. The result of EEPROM read page is a
        byte object that represents the EEPROM page data if the operation succeeds. Otherwise, the
        byte object is None.
        """
        handler_done = False
        if ilog.component_index == self.__component_index:
            if ilog.message_code_index == self.__message_index_fail:
                self.__result = None
                handler_done = True
            elif ilog.message_code_index == self.__message_index_data:
                if ilog.arguments[0] == self.__expecting_index:
                    if self.__data_as_bytes:
                        self.__data += struct.pack('B', ilog.arguments[1])
                        if self.__expecting_index == 15:
                            self.__result = self.__data
                            handler_done = True
                    else:
                        self.__data += (struct.pack('>L' if ilog.is_big_endian else '<L', \
                                ilog.arguments[1]))
                        if self.__expecting_index == 3:
                            self.__result = self.__data
                            handler_done = True
                    self.__expecting_index += 1
                else:
                    self.__result = None
                    handler_done = True

        if handler_done:
            self.__expecting_index = 0
            self.__data = b''

        return handler_done

    @property
    def result(self):
        return self.__result


class EepromWritePageIlogHandler(IlogHandler):
    """
    This class handles the ilogs after EEPROM write page icmd is issued. Results of EEPROM write
    page operation can be classified as success or failure represented by False and True
    respectively.
    """
    def __init__(self):
        """
        Construct an EEPROM read page ilog handler object.
        """
        self.__component_index = 0x20
        self.__message_code_failure = 0x3
        self.__message_code_success = 0x4

        # It will be true if the write success ilog shows up, false if write failed ilog shows up
        self.__result = None

    def consume(self, ilog):
        """
        Consume the ilogs after sending EEPROM write page icmd. The result of write page operation
        is true if a write success ilog is recieved and false for catching a failure ilog.
        """
        handler_done = False
        if ilog.component_index == self.__component_index:
            if ilog.message_code_index == self.__message_code_failure:
                handler_done = True
                self.__result = False
            elif ilog.message_code_index == self.__message_code_success:
                handler_done = True
                self.__result = True
        return handler_done

    @property
    def result(self):
        return self.__result


class SerialPortListener(threading.Thread):
    def __init__(self, serial_port):
        """
        Construct a serial port listener object.
        """
        super(SerialPortListener, self).__init__()
        self.__serial_port = serial_port
        self.__is_listening = threading.Event()
        self.__consumers = []

    def register_consumer(self, h):
        """
        Register a consumer that consumes the ilog in the queue.
        """
        self.__consumers.append(h)

    def run(self):
        """
        Serial port listener should keep running until the main program exists.
        """
        self.__is_listening.set()
        while self.__is_listening.is_set():
            bs = self.__serial_port.read()
            for b in bs:
                for c in self.__consumers:
                    c(b)

    def stop(self):
        """
        Clear the listening event.
        """
        self.__is_listening.clear()


class IlogParser():
    def __init__(self):
        self.__reset()
        self.__ilog_queue = queue.Queue()
        self.__recording = False
        self.__queue_lock = threading.Lock()

    def __reset(self):
        self.__is_big_endian = None
        self.__is_prev_log_printed = None
        self.__num_args = None
        self.__component_index = None
        self.__message_code_index = None
        self.__severity_level_index = None
        self.__arg_bytes = []
        self.arg_byte_count = 0
        self.__arguments = []
        self.__state = 'HEADER_STATE'

    def get_ilog(self, timeout):
        """
        Get an ilog object from the queue.
        """
        block = True
        return self.__ilog_queue.get(block, timeout)


    def parse(self, byte):
        """
        Read and parse one byte. If the byte matches the header format, start the state machine
        that records the raw bytes to construct an iLog object.
        """
        if self.__state == 'HEADER_STATE':
            byte_value = byte
            if (byte_value & 0b11110000) == 0b11110000:
                self.__is_big_endian = (byte_value >> 3) & 0x1 == 0x1
                self.__is_prev_log_printed = (byte_value >> 2) & 0x1 == 0x1
                self.__num_args = byte_value & 0x3
                self.__state = 'COMPONENT_INDEX_STATE'

        elif self.__state == 'COMPONENT_INDEX_STATE':
            self.__component_index = byte
            self.__state = 'MESSAGE_CODE_INDEX_STATE'

        elif self.__state == 'MESSAGE_CODE_INDEX_STATE':
            self.__message_code_index = byte
            self.__state = 'SEVERITY_LEVEL_STATE'

        elif self.__state == 'SEVERITY_LEVEL_STATE':
            self.__severity_level_index = byte
            if self.__num_args > 0:
                self.__state = 'ARGUMENT_STATE'
            else:
                self._assemble_ilog()
                self.__state = 'HEADER_STATE'

        elif self.__state == 'ARGUMENT_STATE':
            self.__arg_bytes.append(byte)
            self.arg_byte_count += 1
            if self.arg_byte_count == 4:
                self.__arguments.append(struct.unpack('>L' if self.__is_big_endian else '<L', \
                        bytes(self.__arg_bytes))[0])
                self.__arg_bytes = []
                self.arg_byte_count = 0
            if len(self.__arguments) == self.__num_args:
                self._assemble_ilog()

        else:
            raise Exception("Invalid ilog parser state")

    def _assemble_ilog(self):
        """
        Assemble an ilog by constructing an ilog object if recording is true. Reinitialize all
        member variables.
        """
        with self.__queue_lock:
            if self.__recording:
                ilog = Ilog(
                        self.__is_big_endian,
                        self.__is_prev_log_printed,
                        self.__num_args,
                        self.__component_index,
                        self.__message_code_index,
                        self.__severity_level_index,
                        self.__arguments)
                self.__ilog_queue.put(ilog)
        self.__reset()


    def set_recording(self, is_recording):
        """
        Set recording to start or stop.
        """
        with self.__queue_lock:
            self.__recording = is_recording
            if not is_recording:
                while not self.__ilog_queue.empty():
                    self.__ilog_queue.get()


class Ilog():
    def __init__(self,
                 is_big_endian,
                 is_prev_log_printed,
                 num_args,
                 component_index,
                 message_code_index,
                 severity_level_index,
                 arguments):
        self.__is_big_endian = is_big_endian
        self.__is_prev_log_printed = is_prev_log_printed
        self.__num_args = num_args
        self.__component_index = component_index
        self.__message_code_index = message_code_index
        self.__severity_level_index = severity_level_index
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
    def message_code_index(self):
        return self.__message_code_index

    @property
    def severity_level_index(self):
        return self.__severity_level_index

    @property
    def arguments(self):
        """
        The client must handle ilog arguments' endianness when invoke this method.
        """
        return self.__arguments

    def __str__(self):
        """
        Debugging logs
        """
        return "ILOG:[is_big_endian={}, prev_ilog_printed={}, num_args={}, component_index={}, ". \
                    format(self.__is_big_endian, self.__is_prev_log_printed, self.__num_args, \
                            self.__component_index) + \
               "message_code_index={}, severity_index={}, arguments={}". \
                    format(self.__message_code_index, self.__severity_level_index, self.__arguments)


class SerialPort():
    def __init__(self, port, baudrate=115200, timeout=1):
        self.__port = port
        self.sp = serial.Serial(port, baudrate, timeout=timeout)

    @property
    def port(self):
        return self.__port

    def read(self, num_bytes=1):
        """
        Read byte(s) from the open serial port.
        """
        return self.sp.read(num_bytes)

    def write(self, data):
        """
        Write byte(s) to the open serial port. The client must catch the exceptions.
        """
        return self.sp.write(data)

    def close(self):
        """
        Close the open serial port
        """
        self.sp.close()


if __name__ == '__main__':
    if len(sys.argv) == 2:
        serial_port_name = sys.argv[1]
        sp = SerialPort(serial_port_name)
        spl = SerialPortListener(sp)
        ilp = IlogParser()
        spl.register_consumer(ilp.parse)
        spl.start()

        try:
            update_adder_eeprom(sp, ilp)
        except Exception as e:
            traceback.print_exc(file=sys.stderr)
            sys.stderr.write("Failed to update Adder's EEPROM.\n")

        spl.stop()
        spl.join()
        sp.close()
    else:
        print_usage()

