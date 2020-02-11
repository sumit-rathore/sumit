import serial
import time
import script_lib.icron_model as im
import script_lib.icron_ilog as ilg
import script_lib.icron_istatus as iis
import script_lib.icron_icmd as icd
import script_lib.icron_packetizer as ipk

class Loaded_icron_file:

    def __init__(self, icron_parsed_file, device):
        self.icron_parsed_file = icron_parsed_file
        self.device_name = device
        self.icmd_response_id = 0
        self.ipacket_handlers = {}
        self.icmd_response_validator = {}
        self.icmd_response_result = {}
        self.icmd_channel_id = None
        self.load_icron_model()

    def load_icron_model(self):
        """
        Load Icron model from the icron parsed file and assign the Icron file path.

        Arguments: self.icron_parsed_file - icron_parsed_file object defined in icron_file_parser.py
        """
        try:
            self.short_device_name = self.icron_parsed_file.get_short_device_name(self.device_name)

            if self.icron_parsed_file.get_golden_current_fpga_image(self.device_name) == 'golden':
                self.goldenImage = True
            else:
                self.goldenImage = False

            self.ilog_model = im.IcronILogModel(
                                    self.device_name,
                                    self.icron_parsed_file.get_icomponent_json(self.device_name),
                                    self.icron_parsed_file.get_ilog_json(self.device_name),
                                    self.icron_parsed_file.get_severity_json(self.device_name))

            self.ilog_decoder = ilg.ILogDecoder(self.ilog_model)

            self.istatus_model = im.IcronIStatusModel(
                                    self.device_name,
                                    self.icron_parsed_file.get_istatus_json(self.device_name))

            self.istatus_decoder = iis.IStatusDecoder(self.istatus_model)

            self.icmd_model = im.IcronIcmdModel(
                                self.device_name, 
                                self.icron_parsed_file.get_icomponent_json(self.device_name),
                                self.icron_parsed_file.get_icmd_json(self.device_name))

            self.icmd_encoder = icd.IcmdEncoder(self.icmd_model)

            ichannel_model = im.IcronChannelIdModel(
                                    self.device_name,
                                    self.icron_parsed_file.get_ichannel_id_json(self.device_name))

            self.ilog_channel_id = ichannel_model.ilog_channel_id

            self.istatus_channel_id = ichannel_model.istatus_channel_id

            self.printf_channel_id = ichannel_model.printf_channel_id

            self.icmd_channel_id = ichannel_model.icmd_channel_id

            # load flash writer image
            self.flash_writer_image = self.icron_parsed_file.get_flash_writer_image(self.device_name)

            # load main firmware image
            self.main_firmware_image = self.icron_parsed_file.get_main_firmware_image(self.device_name)

            # load lex FPGA image
            self.lex_fpga_image = self.icron_parsed_file.get_lex_fpga_image(self.device_name)

            # load rex FPGA image
            self.rex_fpga_image = self.icron_parsed_file.get_rex_fpga_image(self.device_name)

            self.lex_build_time = self.icron_parsed_file.get_lex_fpga_build_time(self.device_name)
            self.rex_build_time = self.icron_parsed_file.get_rex_fpga_build_time(self.device_name)

            self.iregister_model = im.IcronRegisterModel(
                                        self.device_name, 
                                        self.icron_parsed_file.get_iregister_settings(self.device_name))

            self.program_command_channel_id = ichannel_model.program_command_channel_id
            self.device_info_channel_id = ichannel_model.program_status_channel_id
            self.program_data_channel_id = ichannel_model.program_data_channel_id

            return True

        except:
            print("Got an error loading icron file\n")
            return False

    def create_icmd(self, component_name, function_name, response, *arguments):
        """
        Create icmd object from component name, function name and arguments.

        Arguments: response - whether sender expects a response for the icmd
        """
        if response:
            self.icmd_response_validator[(self.icmd_response_id, self.icmd_channel_id)] = \
                    self._build_icmd_response_validator(component_name, function_name)

        if len(arguments) == 0:
            return self.icmd_encoder.encode(component_name, function_name)
        else:
            return self.icmd_encoder.encode(component_name, function_name, arguments[0])

    def send_icmd(self, icmd_obj, ser, port, rate, response=False):
        """
        Send icmd by writing the data packet of icmd to serial port.

        Arguments: icmd_obj - icmd object to be sent out
                   response - whether sender expects a response from device
        """
        try:
            packet = ipk.packetize(
                            self.icmd_channel_id,
                            len(icmd_obj.as_integer_list),
                            icmd_obj.as_integer_list,
                            self.icmd_response_id) if response else \
                     ipk.packetize(
                             self.icmd_channel_id,
                             len(icmd_obj.as_integer_list),
                             icmd_obj.as_integer_list)

            ser.write(packet)
            print("{}: {}: Trying to send an icmd with a packet of {}". \
                            format(port, self.device_name, packet))

        except:
            print("{}: {}: Got an error when sending icmd". \
                                format(port, self.device_name))


    def send_icmd_wait_for_response(self, icmd_obj, ser, port, rate):
        """
        Send icmd to device and wait for its response.

        Arguments: icmd_obj - icmd object to be sent out
        """
        self.register_packet_handler(
                self._decode_icmd_response,
                self.icmd_response_id,
                self.icmd_channel_id)
        try:
            print(
                "{}: {}: Sending an icmd and waiting for response: respon_id={} chan_id={}". \
                    format(
                        port,
                        self.device_name,
                        self.icmd_response_id,
                        self.icmd_channel_id))

            self.register_packet_handler(
                    self._decode_icmd_response,
                    self.icmd_response_id,
                    self.icmd_channel_id)

            self.send_icmd(icmd_obj, ser, port, rate, True)
            return self._poll_icmd_response(self.icmd_response_id, self.icmd_channel_id)
        except:
            print("{}: {}: Got an error when sending icmd and waiting for response:" \
                                .format(port, self.device_name))

    def _decode_icmd_response(self, packet, timestamp=None):
        """
        Decode icmd response from received packet.

        Arguments: packet - received packet from icmd channel
        """
        try:
            print(
                "{}: {}: Got an icmd response chan_id={} response_id={} len={} payload={}". \
                    format(packet.channel_id,
                        packet.response_id, packet.payload_length, packet.payload_data))

            response_len = packet.payload_length
            response_payload = packet.payload_data
            validator = self.icmd_response_validator.pop((packet.response_id, packet.channel_id))
            if validator(response_len):
                num_args = response_len / 4
                results = []
                for i in range(num_args):
                    response_bytes = b''
                    for j in range(4):
                        response_bytes += struct.pack('B', response_payload[4*i + j])
                    arg_value = struct.unpack('>L', response_bytes)
                    results.append(arg_value[0])
                self.icmd_response_result[(packet.response_id, packet.channel_id)] = tuple(results)
            else:
                raise icr.IcmdResponseError(
                        "Invalid icmd response",
                        response_len,
                        response_payload)

        except icr.IcmdResponseError as e:
            error_message = "{}: {}: ".format(self.port_name, self.device_name) + str(e)
            cbs_logger.exception(error_message)
            Forms.MessageBox.Show(
                error_message,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)
        except:
            error_message = "{}: {}: Got an error when decoding icmd response". \
                    format(self.port_name, self.device_name)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            cbs_logger.exception(error_string)
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

    def _poll_icmd_response(self, response_id, channel_id, timeout=3):
        """
        Poll icmd response from self.icmd_response_result associated with the specific response id
        and channel id. The default timeout is 3 seconds.

        Arguments: response_id - associated response id
                   channel_id - associated channel id
                   timeout - period of polling
        """
        try:
            self.icmd_response_id = self.icmd_response_id + 1 if self.icmd_response_id < 254 else 0
            start_time = time.time()
            result = None
            while True:
                if self.icmd_response_result.get((response_id, channel_id)):
                    result = self.icmd_response_result.pop((response_id, channel_id))
                    break

                if time.time() - start_time > timeout:
                    print("Timed out on polling icmd response for " + \
                                    "response_id={}, ".format(response_id) + \
                                    "channel_id={}, ".format(channel_id) + \
                                    "timeout={}".format(timeout))
                    break

            self.remove_packet_received_handler(response_id, channel_id)
            return result
        except:
            print("Got an error when polling icmd response")

    def register_packet_handler(self, packet_handler, response_id, channel_id):
        """
        Register a packet handler for handling packets associated with specific request_id and
        channel_id. The handler is stored in ipacket_handlers dictionary as value, and its key
        is the tuple of request id and channel id. When a packet is received, the handler will be
        invoked from the key.

        Arguments: packet_handler - handler to be registered for the packets associated with the
                                    response_id and channel_id
                   response_id    - response_id associate with the packet to be handled
                                    by the packet_handler
                   channel_id     - channel_id associate with the packet to be handled
                                    by the packet_handler
        """
        print(" Registered packet_handler={}, response_id={}, channel_id={}". \
                format(packet_handler, response_id, channel_id))
        if not response_id is None and not channel_id is None:
            if (response_id, channel_id) not in self.ipacket_handlers:
                self.ipacket_handlers[(response_id, channel_id)] = packet_handler
            else:
                print("handler={} for resp_id={} chan_id={} already registered". \
                    format(
                        self.ipacket_handlers[(response_id, channel_id)],
                        response_id,
                        channel_id))

    def remove_packet_received_handler(self, response_id, channel_id):
        """
        Remove packet received handler associated with response id and channel id.

        Arguments: response_id - the associated response id
                   channel_id - the associated channel id
        """
        try:
            packet_handler = self.ipacket_handlers.pop((response_id, channel_id))
            print(" Removed packet_handler={}, response_id={}, channel_id={}". \
                    format(packet_handler, response_id, channel_id))
        except:
            print(" Got an error when removing packet handler resp_id={} chan_id={}". \
                    format(response_id, channel_id))


