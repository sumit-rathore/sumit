import serial
import script_lib.icron_model as im
import script_lib.icron_ilog as ilg
import script_lib.icron_istatus as iis
import script_lib.icron_icmd as icd
import script_lib.icron_packetizer as ipk

class Loaded_icron_file:

    def __init__(self, icron_parsed_file, device):
        self.icron_parsed_file = icron_parsed_file
        self.device_name = device
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
