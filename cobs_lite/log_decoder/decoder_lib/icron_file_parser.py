import tarfile
import json
import subprocess
#from cobs_logger import cbs_logger

class IcronParsedFile():
    def __init__(self, icron_file, untar_exe=None):
        self.__projects = []
        self.__short_device_names = {}
        self.__icomponent_json = {}
        self.__ilog_json = {}
        self.__istatus_json = {}
        self.__icmd_json = {}
        self.__ichannel_id_json = {}
        self.__severity_json = {}
        self.__flash_writer = {}
        self.__main_firmware = {}
        self.__golden_current_fpga_image = {}
        self.__lex_fpga_image = {}
        self.__rex_fpga_image = {}
        self.__lex_fpga_build_time = {}
        self.__rex_fpga_build_time = {}
        self.__icron_file = icron_file
        self.__iregister_settings= {}
        self.untar_exe = untar_exe

        try:
            if self.untar_exe is None:
                with tarfile.open(self.__icron_file, 'r:gz') as icron_tf:
                    icron_header_file = icron_tf.extractfile('icron_header.json')
                    self.header_json = json.load(icron_header_file)
                    for project in self.header_json.keys():
                        # get project names from icron_header.json
                        self.__projects.append(project)
                        #print self.header_json[project]["icomponent"]

                        self.__short_device_names[project] = \
                                self.header_json[project]['device_name']

                        # load icomponent json file for each project
                        icomponent_file = \
                                icron_tf.extractfile(self.header_json[project]['icomponent'])
                        self.__icomponent_json[project] = json.load(icomponent_file)
                        #print self.__icomponent_json[project]

                        # load ilog json file for each project
                        ilog_file = icron_tf.extractfile(self.header_json[project]['ilog'])
                        self.__ilog_json[project] = json.load(ilog_file)

                        # load icmd json file for each project
                        icmd_file = icron_tf.extractfile(self.header_json[project]['icmd'])
                        self.__icmd_json[project] = json.load(icmd_file)

                        # load ichannel id json file for each project
                        ichannel_id_file = icron_tf.extractfile(
                                                self.header_json[project]['ichannel_id'])
                        self.__ichannel_id_json[project] = json.load(ichannel_id_file)

                        # load ilog level json file for each project
                        severity_file = \
                                icron_tf.extractfile(self.header_json[project]['ilog_level'])
                        self.__severity_json[project] = json.load(severity_file)

                        # load istatus json file for each project
                        if 'istatus' in self.header_json[project]:
                            if self.header_json[project]['istatus'] is None:
                                self.__istatus_json[project] = None
                            else:
                                istatus_file = \
                                        icron_tf.extractfile(self.header_json[project]['istatus'])
                                self.__istatus_json[project] = json.load(istatus_file)
                        else:
                            self.__istatus_json[project] = None

                        # load lex_fpga_build_time for each project
                        if 'lex_fpga_build_time' in self.header_json[project]:
                            if self.header_json[project]['lex_fpga_build_time'] is None:
                                self.__lex_fpga_build_time[project] = None
                            else:
                                self.__lex_fpga_build_time[project] = \
                                        self.header_json[project]['lex_fpga_build_time']
                        else:
                            self.__lex_fpga_build_time[project] = None

                        # load rex_fpga_build_time for each project
                        if 'rex_fpga_build_time' in self.header_json[project]:
                            if self.header_json[project]['rex_fpga_build_time'] is None:
                                self.__rex_fpga_build_time[project] = None
                            else:
                                self.__rex_fpga_build_time[project] = \
                                        self.header_json[project]['rex_fpga_build_time']
                        else:
                            self.__rex_fpga_build_time[project] = None

                        if 'golden_current' in self.header_json[project]:                        
                            if self.header_json[project]['golden_current'] is None:
                                self.__golden_current_fpga_image[project] = None
                            else:
                                self.__golden_current_fpga_image[project] = \
                                        self.header_json[project]['golden_current']
                        else:
                            self.__golden_current_fpga_image[project] = None

                        # load flash writer Note that the flash writer is only for blackbird. For
                        # consistency and clean code, goldenears loads blackbird flash writer. When
                        # users click flash or bootload on goldenears client window, the blackbird
                        # actually will be programmed. On the firmware side, blackbird will take
                        # care of goldenears programming.
                        if self.header_json[project]['flash_writer'] is None:
                            self.__flash_writer[project] = None
                        else:
                            flash_writer_file = icron_tf.extractfile(
                                                    self.header_json[project]['flash_writer'])
                            self.__flash_writer[project] = flash_writer_file.read()

                        # load main firmware
                        # Note that the combined binary file contain the firmware
                        # images for both blackbird and goldenears
                        if self.header_json[project]['main_firmware'] is None:
                            self.__main_firmware[project] = None
                        else:
                            main_fw_file = icron_tf. \
                                            extractfile(self.header_json[project]['main_firmware'])
                            self.__main_firmware[project] = main_fw_file.read()

                        if self.header_json[project]['lex_fpga_image'] is None:
                            self.__lex_fpga_image[project] = None
                        else:
                            lex_fpga_file = icron_tf. \
                                    extractfile(self.header_json[project]['lex_fpga_image'])
                            self.__lex_fpga_image[project] = lex_fpga_file.read()

                        if self.header_json[project]['rex_fpga_image'] is None:
                            self.__rex_fpga_image[project] = None
                        else:
                            rex_fpga_file = icron_tf. \
                                        extractfile(self.header_json[project]['rex_fpga_image'])
                            self.__rex_fpga_image[project] = rex_fpga_file.read()

                        # load xml settings for each project
                        self.__iregister_settings[project] = []
                        iregister_component_list = self.header_json[project]['iregister']
                        for component in iregister_component_list:
                            component_offset = int(component['base_address'], 0)
                            xml_component_file = icron_tf.extractfile(component['xml_file_name'])
                            xml_component_string = xml_component_file.read()
                            self.__iregister_settings[project]. \
                                    append((component_offset, xml_component_string))
            else:
                icron_header_string = self.extract_file_by_bsdtar('icron_header.json')
                self.header_json = json.loads(icron_header_string)
                for project in self.header_json.keys():
                    self.__projects.append(project)

                    # load short device names
                    self.__short_device_names[project] = self.header_json[project]['device_name']
  
                    # load icomponent json file for each project
                    icomponent_file = \
                            self.extract_file_by_bsdtar(self.header_json[project]['icomponent'])
                    self.__icomponent_json[project] = json.loads(icomponent_file)

                    # load ilog json file for each project
                    ilog_file = self.extract_file_by_bsdtar(self.header_json[project]['ilog'])
                    self.__ilog_json[project] = json.loads(ilog_file)


                    # load icmd json file for each project
                    icmd_file = self.extract_file_by_bsdtar(self.header_json[project]['icmd'])
                    self.__icmd_json[project] = json.loads(icmd_file)

                    # load ichannel id json file for each project
                    ichannel_id_file = \
                            self.extract_file_by_bsdtar(self.header_json[project]['ichannel_id'])
                    self.__ichannel_id_json[project] = json.loads(ichannel_id_file)

                    # load ilog level json file for each project
                    severity_file = \
                            self.extract_file_by_bsdtar(self.header_json[project]['ilog_level'])
                    self.__severity_json[project] = json.loads(severity_file)

                    # load istatus json file for each project
                    if 'istatus' in self.header_json[project]:
                        if self.header_json[project]['istatus'] is None:
                            self.__istatus_json[project] = None
                        else:
                            istatus_file = \
                                self.extract_file_by_bsdtar(self.header_json[project]['istatus'])
                            self.__istatus_json[project] = json.loads(istatus_file)
                    else:
                        self.__istatus_json[project] = None

                    # load lex_fpga_build_time for each project
                    if 'lex_fpga_build_time' in self.header_json[project]:
                        if self.header_json[project]['lex_fpga_build_time'] is None:
                            self.__lex_fpga_build_time[project] = None
                        else:
                            self.__lex_fpga_build_time[project] = \
                                    self.header_json[project]['lex_fpga_build_time']
                    else:
                        self.__lex_fpga_build_time[project] = None

                    # load rex_fpga_build_time for each project
                    if 'rex_fpga_build_time' in self.header_json[project]:
                        if self.header_json[project]['rex_fpga_build_time'] is None:
                            self.__rex_fpga_build_time[project] = None
                        else:
                            self.__rex_fpga_build_time[project] = \
                                    self.header_json[project]['rex_fpga_build_time']
                    else:
                        self.__rex_fpga_build_time[project] = None

                    if 'golden_current' in self.header_json[project]:                        
                        if self.header_json[project]['golden_current'] is None:
                            self.__golden_current_fpga_image[project] = None
                        else:
                            self.__golden_current_fpga_image[project] = \
                                    self.header_json[project]['golden_current']
                    else:
                        self.__golden_current_fpga_image[project] = None

                    # load flash writer
                    if self.header_json[project]['flash_writer'] is None:
                        self.__flash_writer[project] = None
                    else:
                        self.__flash_writer[project] = \
                            self.extract_file_by_bsdtar(self.header_json[project]['flash_writer'])

                    # load main firmware
                    # Note that the combined binary file contain the firmware
                    # images for both blackbird and goldenears
                    if self.header_json[project]['main_firmware'] is None:
                        self.__main_firmware[project] = None
                    else:
                        self.__main_firmware[project] = \
                            self.extract_file_by_bsdtar(self.header_json[project]['main_firmware'])

                    if self.header_json[project]['lex_fpga_image'] is None:
                        self.__lex_fpga_image[project] = None
                    else:
                        self.__lex_fpga_image[project] = \
                            self.extract_file_by_bsdtar(self.header_json[project]['lex_fpga_image'])

                    if self.header_json[project]['rex_fpga_image'] is None:
                        self.__rex_fpga_image[project] = None
                    else:
                        self.__rex_fpga_image[project] = \
                            self.extract_file_by_bsdtar(self.header_json[project]['rex_fpga_image'])

                    # load xml settings for each project
                    self.__iregister_settings[project] = []
                    iregister_component_list = self.header_json[project]['iregister']
                    for component in iregister_component_list:
                        component_offset = int(component['base_address'], 0)
                        xml_component_string = \
                                self.extract_file_by_bsdtar(component['xml_file_name'])
                        self.__iregister_settings[project]. \
                                append((component_offset, xml_component_string))
        except:
            #cbs_logger.exception("Got error when extracting icron file")
            raise IcronFileParsingError("Got an error when extracting icron file")

    def extract_file_by_bsdtar(self, file_name):
        cmd = self.untar_exe + ' -xf "{}" -O "{}"'.format(self.__icron_file, file_name)
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stderr_data = ""
        stdout_data, stderr_data = proc.communicate()
        if not stderr_data == "":
            raise Exception(stderr_data)
        return stdout_data


    def get_iheader_json(self, project):
        return self.header_json[project]

    def get_short_device_name(self, project):
        return self.__short_device_names[project]

    def get_icomponent_json(self, project):
        return self.__icomponent_json[project]

    def get_ilog_json(self, project):
        return self.__ilog_json[project]

    def get_icmd_json(self, project):
        return self.__icmd_json[project]

    def get_ichannel_id_json(self, project):
        return self.__ichannel_id_json[project]

    def get_severity_json(self, project):
        return self.__severity_json[project]

    def get_flash_writer_image(self, project):
        return self.__flash_writer[project]

    def get_main_firmware_image(self, project):
        return self.__main_firmware[project]

    def get_lex_fpga_image(self, project):
        return self.__lex_fpga_image[project]

    def get_golden_current_fpga_image(self, project):
        return self.__golden_current_fpga_image[project]

    def get_rex_fpga_image(self, project):
        return self.__rex_fpga_image[project]

    def get_iregister_settings(self, project):
        return self.__iregister_settings[project]

    def get_istatus_json(self, project):
        return self.__istatus_json[project]

    def get_lex_fpga_build_time(self, project):
        return self.__lex_fpga_build_time[project]

    def get_rex_fpga_build_time(self, project):
        return self.__rex_fpga_build_time[project]

    @property
    def projects(self):
        return self.__projects


class IcronFileParsingError(Exception):
    def __init__(self, message):
        self.message = message

    def __str__(self):
        return "Icron file parsing error: {}".format(self.message)
