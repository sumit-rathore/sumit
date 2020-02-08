import sys
import gzip
import struct
import threading
import datetime
import traceback
import icron_depacketizer as idp
import icron_file_parser as ifp
import icron_model as im
import icron_ilog as ilg
import icron_device_info as idi
import icron_istatus as iis
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
                while True:
                    data = f.read(32)

                    if len(data) == 0:
                        break

                    ts_raw = data[:8]
                    timestamp = struct.unpack('!Q', ts_raw)[0] / 1000000.0

                    for d in data[8:]:
                        byte = struct.unpack('B', d)[0]
                        pkt, valid = self.depacketizer.parse(byte)
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

                    if len(data) < 32:
                        break

log_file_decoder = IcronLogFileDecoder()

if __name__ == '__main__':
    if len(sys.argv) == 3:
        try:
            log_file = sys.argv[1]
            icron_file = sys.argv[2]
       
            log_file_decoder = IcronLogFileDecoder()

            iparsed_file = ifp.IcronParsedFile(icron_file) 

            for project in iparsed_file.projects:
                short_device_name = iparsed_file.get_short_device_name(project)
                ilog_model = im.IcronILogModel(
                                            project,
                                            iparsed_file.get_icomponent_json(project),
                                            iparsed_file.get_ilog_json(project),
                                            iparsed_file.get_severity_json(project))

                ilog_decoder = ilg.ILogDecoder(ilog_model)

                ichannel_model = im.IcronChannelIdModel(
                                            project,
                                            iparsed_file.get_ichannel_id_json(project))

                istatus_model = im.IcronIStatusModel(project, iparsed_file.get_istatus_json(project))

                istatus_decoder = iis.IStatusDecoder(istatus_model)

                ilog_channel_id = ichannel_model.ilog_channel_id

                istatus_channel_id = ichannel_model.istatus_channel_id

                printf_channel_id = ichannel_model.printf_channel_id

                device_info_channel_id = ichannel_model.program_status_channel_id

                log_file_decoder.register_channel_packet_attributes(
                                                        ilog_channel_id,
                                                        short_device_name,
                                                        ilg.parse_ilog, 
                                                        ilog_decoder.decode)

                log_file_decoder.register_channel_packet_attributes(
                                                        istatus_channel_id,
                                                        short_device_name,
                                                        iis.parse_istatus,
                                                        istatus_decoder.decode)

                log_file_decoder.register_channel_packet_attributes(
                                                        printf_channel_id,
                                                        short_device_name,
                                                        None,
                                                        None)

                log_file_decoder.register_channel_packet_attributes(
                                                        device_info_channel_id,
                                                        short_device_name,
                                                        None,
                                                        idi.decode_info_message)
                                                        

            log_file_decoder.register_log_message_handler(sys.stdout.write)
            log_file_decoder.decode(log_file)
            log_file_decoder.remove_log_message_handler(sys.stdout.write)
        except:
            traceback.print_exc()
