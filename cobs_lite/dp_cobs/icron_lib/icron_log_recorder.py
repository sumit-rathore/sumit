import logging
import datetime
import gzip
import time
import struct
import threading

class IcronLogRecorder():

    def __init__(self):
        """
        A logger for icron packets.
        """
        self.log_file = None
        self.byte_list = []
        self.ts_bytes = ''
        self.is_recording = threading.Event()

    def record(self, byte):
        """
        Record data to the logger.
        """
        if self.is_recording.is_set():
            self.byte_list.append(byte)
            if len(self.byte_list) == 24:
                self.ts_bytes = struct.pack('!Q', int(time.time() * 1000000))
                self._write_data_to_log_file()

    def _write_data_to_log_file(self):
        raw_bytes = struct.pack('{}B'.format(len(self.byte_list)), *self.byte_list)
        self.byte_list = [] 

        data_bytes = self.ts_bytes + raw_bytes
        self.ts_bytes = ''

        self.log_file.write(data_bytes)

    def stop(self):
        """
        Stop recording data.
        """
        self.is_recording.clear()
        self._write_data_to_log_file()
        self.log_file.close()
        self.log_file = None

    def create_new_log_file(self, prefix):
        """
        Create a new gzip log file.
        """
        self.log_file = gzip.open(
                    "Log\{}_Icron_{}.gz". \
                            format(prefix, datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S")),
                    "wb")
        self.is_recording.set()


