import sys
import io
import gzip
import struct
import threading
import datetime
import traceback
import script_lib.icron_depacketizer as idp
#from cobs_logger import cbs_logger

class IcronLogFileDecoder():
    def __init__(self):
        self.channel_device_names = {}
        self.channel_parsers = {}
        self.channel_decoders = {}
        self.log_message_handlers = set()
        self.depacketizer = idp.Depacketizer()
        #TODO: update all?
        self.depacketizer.update_channel_id([x for x in range(256)])
        self.decod_lock = threading.Lock()

    def register_channel_packet_attributes(self, channel_id, device_name, parser, decoder):
        """
        Register attributes associated with the channel id.
        """
        self._register_channel_device_name(channel_id, device_name)
        self._register_channel_packet_parser(channel_id, parser)
        self._register_channel_packet_decoder(channel_id, decoder)

    def _register_channel_device_name(self, channel_id, device_name):
        """
        Register device name associated with the channel id.
        """
        self.channel_device_names[channel_id] = device_name

    def _register_channel_packet_parser(self, channel_id, parser):
        """
        Register packet parser associated with the channel id.
        """
        self.channel_parsers[channel_id] = parser

    def _register_channel_packet_decoder(self, channel_id, decoder):
        """
        Register packet decoder associated with the channel id.
        """
        self.channel_decoders[channel_id] = decoder

    def register_log_message_handler(self, handler):
        """
        Register log message handler.
        """
        self.log_message_handlers.add(handler)

    def remove_log_message_handler(self, handler):
        self.log_message_handlers.remove(handler)

    def decode(self, log_file):
        """
        Decode log file.
        """
        with self.decod_lock:
            with gzip.open(log_file, 'rb') as f:
                byte_list = []

                while True:
                    data = f.read(1)
                    if data:
                        byte_list.append(ord(data))
                    else:
                        print("Reached end of file")
                        break

                    if len(byte_list) == 32:
                        ts_raw = bytes(byte_list[:8])
                        timestamp = struct.unpack('!Q', ts_raw)[0] / 1000000.0

                        for d in byte_list[8:]:
                            pkt, valid = self.depacketizer.parse(d)
                            if pkt is not None and valid is not None:
                                if valid:

                                    # parse the packet payload if the packet parser is registered for
                                    # the channel
                                    if pkt.channel_id in self.channel_parsers: 

                                        try:
                                            result = self.channel_parsers[pkt.channel_id](
                                                        pkt.payload_data) if not \
                                                        self.channel_parsers[pkt.channel_id] is None \
                                                        else \
                                                    pkt.payload_data

                                            message = self.channel_decoders[pkt.channel_id](result) \
                                                        if not self.channel_decoders[pkt.channel_id] \
                                                        is None else \
                                                    ''.join(chr(char) for char in result)
                                        except Exception as e:
                                            message = str(e)
                                        if (type(message) == tuple):
                                            message = message[0]
                                            message = ''.join(message)
                                        if pkt.channel_id in self.channel_device_names:
                                            device_name = self.channel_device_names[pkt.channel_id]

                                        timestamp_string = \
                                                datetime.datetime.fromtimestamp(timestamp). \
                                                    strftime('%Y-%m-%d %H:%M:%S.%f')[2:-3] + ": "

                                        if not message is None:
                                            string = device_name + ": " + timestamp_string + message

                                            for handler in self.log_message_handlers:
                                                handler(string)
                                else:
                                    string = ''.join(chr(char) for char in pkt)
                                    for handler in self.log_message_handlers:
                                        handler(string)


                        byte_list = []
