class IcronILogModel():
    def __init__(self, project_name, icomponent_json, ilog_json, severity_json):
        self.__project_name = project_name
        self.__icomponent_json = icomponent_json
        self.__ilog_json = ilog_json
        self.__severity_json = severity_json

    def get_icomponent_string(self, component_index):
        return self.__icomponent_json[component_index]

    def get_ilog_name(self, icomponent_string, message_index):
        return self.__ilog_json[icomponent_string][message_index]['ilog_name']

    def get_ilog_string(self, icomponent_string, message_index):
        return self.__ilog_json[icomponent_string][message_index]['ilog_string']

    def get_ilog_num_args(self, icomponent_string, message_index):
        return self.__ilog_json[icomponent_string][message_index]['ilog_num_args']

    def get_severity(self, severity_index):
        return self.__severity_json[severity_index]

    @property
    def project_name(self):
        return self.__project_name


class IcronChannelIdModel():
    def __init__(self, project_name, ichannel_id_json):
        self.__project_name = project_name
        self.__ichannel_id_json = ichannel_id_json

    @property
    def ilog_channel_id(self):
        return self.__ichannel_id_json[self.__project_name]['ilog_channel']

    @property
    def istatus_channel_id(self):
        return self.__ichannel_id_json[self.__project_name]['istatus_channel']

    @property
    def icmd_channel_id(self):
        return self.__ichannel_id_json[self.__project_name]['icmd_channel']

    @property
    def printf_channel_id(self):
        return self.__ichannel_id_json[self.__project_name]['printf_channel']

    @property
    def program_command_channel_id(self):
        return self.__ichannel_id_json[self.__project_name]['program_command_channel']

    @property
    def program_status_channel_id(self):
        return self.__ichannel_id_json[self.__project_name]['program_status_channel']

    @property
    def program_data_channel_id(self):
        return self.__ichannel_id_json[self.__project_name]['program_data_channel']

    @property
    def project_name(self):
        return self.__project_name



class IcronIcmdModel():
    def __init__(self, project_name, icomponent_json, icmd_json):
        self.__project_name = project_name
        self.__icomponent_json = icomponent_json
        self.__icmd_json = icmd_json 

    def get_icomponent_index(self, component_name):
        return self.__icomponent_json.index(component_name)

    def get_icmd_function_index(self,component_name, icmd_fn_name):
        return self.__icmd_json[component_name][icmd_fn_name]['function_index']

    def get_icmd_function_names(self, component_name):
        return self.__icmd_json[component_name].keys()

    def get_icmd_help_string(self, component_name, icmd_fn_name):
        return self.__icmd_json[component_name][icmd_fn_name]['icmd_help_string']

    def get_icmd_arg_types(self, component_name, icmd_fn_name):
        return self.__icmd_json[component_name][icmd_fn_name]['icmd_argument_type']

    def get_icmd_num_args(self, component_name, icmd_fn_name):
        return 0 if self.get_icmd_arg_types(component_name, icmd_fn_name)[0] == 'void' else \
                len(self.__icmd_json[component_name][icmd_fn_name]['icmd_argument_type']) 

    def get_icmd_response_ilog_name(self, component_name, icmd_fn_name):
        return self.__icmd_json[component_name][icmd_fn_name]['ICMDRESP']['ilog_name']

    def get_icmd_response_argument_index_list(self, component_name, icmd_fn_name):
        return self.__icmd_json[component_name][icmd_fn_name]['ICMDRESP'] \
                        ['icmdresp_argument_index_list']

    def get_num_icmd_response_arguments(self, component_name, icmd_fn_name):
        return len(self.get_icmd_response_argument_index_list(component_name, icmd_fn_name))

    @property
    def project_name(self):
        return self.__project_name

    @property
    def componet_list(self):
        return self.__icmd_json.keys()

class IcronRegisterModel():
    def __init__(self, project_name, register_settings):
        self.__project_name = project_name
        self.__register_settings = register_settings

    @property
    def register_settings(self):
        return self.__register_settings

    @property
    def project_name(self):
        return self.__project_name


class IcronIStatusModel():
    def __init__(self, project_name, istatus_json):
        self.__project_name = project_name
        self.__istatus_json = istatus_json

    def get_istatus_name(self, istatus_id):
        return self.istatus_json[istats_id]['istatus_name']

    def get_istatus_string(self, istatus_id):
        return self.istatus_json[istatus_id]['istatus_string']

    def get_istatus_num_args(self, istats_id):
        return self.istatus_json[istatus_id]['istatus_num_args']

    @property
    def project_name(self):
        return self.__project_name

    @property
    def istatus_json(self):
        return self.__istatus_json
