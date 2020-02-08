import sys
import os
import datetime
import threading
import Queue
import time
import struct
import shutil
from cobs_logger import cbs_logger
import CobsInterpreter as ci
import icmd_gui as icg
import iLog_Filter
import icron_lib.icron_model as im
import icron_lib.icron_file_parser
import icron_lib.icron_depacketizer
import icron_lib.icron_ilog as ilg
import icron_lib.icron_icmd as icd
import icron_lib.icron_packetizer as ipk
import icron_lib.icron_icmd_response as icr
import icron_lib.icron_programming as ip
import icron_lib.icron_device_info as idi
import icron_lib.icron_crc as ic
import icron_lib.icron_system_cmd as isc
import icron_lib.icron_istatus as iis
from icron_lib.icron_log_decoder import log_file_decoder

import clr
clr.AddReferenceToFile("Microsoft.Scripting.dll")
clr.AddReference('IronPython')
from IronPython.Compiler import CallTarget0

from Microsoft.Scripting import SourceCodeKind
from Microsoft.Scripting.Hosting import (ExceptionOperations, ScriptEngine, ScriptSource)

import System
import System.Windows.Forms as Forms
import System.Windows.Forms.Clipboard as Clipboard
import System.Threading as Threading
import System.Drawing as Drawing
import System.Array as Array


import Scripts.aux_analyser as aux_analyser
import Scripts.dp_tests as tests
import Scripts.add_comment as usr_comment
# needed to get DLL reference
os.sys.path.append(System.IO.Directory.GetCurrentDirectory())

class DeviceClient():

    def __init__(self, programPath, cobsInterpreter, port_manager, device_name):
        """
        Initialize device client.

        Arguments: programPath - the path containing the cobs program
                   cobsInterpreter - reference to the main cobs interpreter
                   port_manager - serial port manager the device is connected to
                   device_name - name of the device
        """

        # save the program path; we will refer to it in due course
        self.programPath = programPath

        self.cobs = cobsInterpreter

        # the serial port manager the device client registers to
        self.serial_port_manager = port_manager

        self.device_name = device_name

        self.device_type = ""

        #TODO: change how device access logger
        # save the logger; needed later if user needs to output to different log file
        
        self.logger = cobsInterpreter.loggers[(self.device_name,self.port_name)]
        self.log_port_name =""
        self.log_client_name=""
        # the DeviceWindow
        self.devWindow = None

        self.icron_file_watcher = None

        # path to the Device's .icron file
        self.icron_file_path = ""
        self.original_icron_file_path = ""
        self.short_device_name = None

        self.istatus_channel_id = None
        self.istatus_model = None
        self.istatus_decoder = None

        self.ilog_model = None
        self.ilog_channel_id = None
        self.ilog_decoder = None

        self.printf_channel_id = None

        self.icmd_channel_id = None
        self.icmd_encoder = None
        self.icmd_model = None
        
        self.goldenImage = None
        self.fpgaErased = False
        self.flash_writer_image = None
        self.main_firmware_image = None
        self.lex_fpga_image = None
        self.rex_fpga_image = None
        self.program_command_channel_id = None
        self.program_data_channel_id = None
        self.device_info_channel_id = None
        self.program_command_response_id = 0
        self.program_command_response_id_next = 0
        self.program_device_response_result = {}
        self.device_info_result = {}  
        self.ipacket_handlers = {}

        self.icmd_response_id = 0
        self.icmd_response_validator = {}
        self.icmd_response_result = {}
        self.log_queue = Queue.Queue()

        self.iregister_model = None

        self.lex_build_time = None
        self.rex_build_time = None

        self.log_file_decoder = log_file_decoder
		
        self.currBandwidth = 0;
        self.currLC = 0;
        #Used for search auto scroll
        self.scrollValue = 0
        self.wordAddresses = []
		#Used for auto-clearing output
        self.OutputLineLimit = 2000	
        self.iLogFilterValues = []
        self.FilterIndices = None
        self.liveFilter = False
        self.deviceFilter = None
        self.auxEnable = False
		
        # a lock is required for close device window to prevent a race condition
        self.close_window_lock = threading.Lock()
        self.send_icmd_wait_for_response_lock = threading.Lock()
        self.device_command_lock = threading.Lock()
        self.change_baud_rate_lock = threading.Lock()

    def connect(self):
        """
        Connect device client to serial port.
        """
        try:
            self.register_packet_handler(self._decode_ilog, 255, self.ilog_channel_id)
            self.register_packet_handler(self._decode_istatus, 255, self.istatus_channel_id)

            self.register_packet_handler(self._decode_ascii, 255, self.printf_channel_id)

            self.register_packet_handler(
                    self._decode_device_info,
                    255,
                    self.device_info_channel_id)

            channel_id_list = [self.ilog_channel_id,
                                self.istatus_channel_id,
                                self.printf_channel_id,
                                self.icmd_channel_id,
                                self.program_command_channel_id, 
                                self.device_info_channel_id,
                                self.program_data_channel_id]

            cbs_logger.debug("{}: {}: register channel_id_list={}". \
                    format(self.port_name, self.device_name, channel_id_list))

            self.serial_port_manager.register_channel_id(channel_id_list)

            self.serial_port_manager.register_listener(
                                        self.device_name,
                                        self.packet_received_handler)


            if not self.serial_port_manager.is_port_open:
                self.serial_port_manager.open_port()
  
            if self.device_name == 'blackbird':
                thread = Threading.Thread(Threading.ThreadStart(self.query_device_info))
                thread.SetApartmentState(Threading.ApartmentState.STA)
                thread.Start()

        except:
            error_message = "{}: {}: Failed to connect device". \
                    format(self.port_name, self.device_name)
            cbs_logger.exception(error_message)
            Forms.MessageBox.Show(
                    error_message,
                    "ERROR",
                    Forms.MessageBoxButtons.OK,
                    Forms.MessageBoxIcon.Error)

    def query_device_info(self):
        try:
            query_device_info_command = isc.QueryDeviceInfoCommand()
            response = self.send_device_command_wait_for_response(
                                        self.program_command_response_id,
                                        self.program_command_channel_id,
                                        query_device_info_command)
            color = None
            if response == None:
                self.change_baud_rate(115200)
                query_device_info_command = isc.QueryDeviceInfoCommand()
                response = self.send_device_command_wait_for_response(
                                        self.program_command_response_id,
                                        self.program_command_channel_id,
                                        query_device_info_command)
            if response == None:
                message = "{}: {}: Device has no response for device info query". \
                                format(self.port_name, self.device_name)
                color = Drawing.Color.Red
            else:
                query_device_info_cmd_response = isc.QueryDeviceInfoCommandResponse(response)
                if query_device_info_cmd_response.is_ack:
                    message = "{}: {}: Device is connected". \
                                format(self.port_name, self.device_name)
                    color = Drawing.Color.DarkGreen
                else:
                    message = "{}: {}: Device responds device info query command with NAK". \
                                format(self.port_name, self.device_name)
                    color = Drawing.Color.Orange
            time.sleep(0.1)
            cbs_logger.info(message)
            self.print_to_device_window(message, color)
            self.log_port_name = self.cobs.log_port_name
            self.log_client_name = self.cobs.log_client_name
            #Create log files with device type for BB
            self.cobs.loggers[(self.device_name, self.log_port_name)].create_new_log_file("{}\{}_{}_{}". \
                                format(self.cobs.MainLogPath, self.log_port_name, self.device_type, self.device_name))
            #Create log files with device type for GE
            self.cobs.loggers[(self.log_client_name,self.log_port_name)].create_new_log_file("{}\{}_{}_{}". \
                                format(self.cobs.MainLogPath, self.log_port_name, self.device_type, self.log_client_name))
            #delete previous files without device type
            for log_file in (os.listdir("Log\\" + self.cobs.MainLogPath)):
                file_path = "Log\\" + self.cobs.MainLogPath + "/" + log_file
                if log_file.startswith("{}_{}". format(self.log_port_name, self.log_client_name)):
                    os.remove(file_path)
                if log_file.startswith("{}_{}". format(self.log_port_name, self.device_name)):
                    os.remove(file_path)
            #Creates a new dictionary with (client name, port name, device type)
            #this new dict is used to create log files when new log button clicked
            if not (self.log_client_name, self.log_port_name, self.device_type) in self.cobs.device_dict:
                self.cobs.device_dict[(self.log_client_name, self.log_port_name, self.device_type)] = \
                                                    (self.log_client_name, self.log_port_name, self.device_type)
            if not (self.device_name, self.log_port_name, self.device_type) in self.cobs.device_dict:
                self.cobs.device_dict[(self.device_name, self.log_port_name, self.device_type)] = \
                                                    (self.device_name, self.log_port_name, self.device_type)

        except:
            error_message = "{}: {}: Failed to query device info". \
                    format(self.port_name, self.device_name)
            cbs_logger.exception(error_message)

    def change_baud_rate(self, baud_rate):
        """
        Change the baud rate of the serial port on the fly. To do so, it requires to wait for all
        bytes in output buffer are sent out. Then close and reopen the serial port with a delay
        in bewteen. After that, flush the input and output buffer of the serial port.
        #http://stackoverflow.com/questions/10209090/serialport-class-occasionally-hangs-on-dispose
        """
        def get_baud_rate(baud_rate):
            switcher_baud_rate = {
                    115200: 115200, 
                    460800: 460800,
            }
            return switcher_baud_rate[baud_rate]

        try:
            with self.change_baud_rate_lock:
                baud_rate = get_baud_rate(baud_rate)

                while self.serial_port_manager.serial_port.BytesToWrite != 0:
                    pass
                time.sleep(0.1)
                self.serial_port_manager.close_port()
                self.serial_port_manager.serial_port.BaudRate = baud_rate
                time.sleep(0.1)
                self.serial_port_manager.open_port()
                self.serial_port_manager.discard_in_buffer()
                self.serial_port_manager.discard_out_buffer()
        except:
            cbs_logger.exception("{} : {}: Got an exception when changing baud rate to {}". \
                    format(self.port_name, self.device_name, baud_rate))

    def change_edid(self, edid):
        """
        Change the EDID on the fly.
        """
        def get_edid(edid):
            switcher_edid = {
                "Default":   0,
                "4K":        1,
                "1080p":     2,
                "640x480":   6,
            }
            return switcher_edid[edid]

        try:
            cbs_logger.info("{}: {}: EDID Selected". \
                            format(self.port_name, self.device_name))
            icmd_obj = self.create_icmd("DP_COMPONENT", "DP_LEX_SetEdidTypeIcmd", False, [get_edid(edid)])
            self.send_icmd(icmd_obj)
            # self.reset_device()
        except:
            cbs_logger.exception("{} : {}: Got an exception when changing EDID to {}". \
                                 format(self.port_name, self.device_name, edid))

    def change_bpc(self, bpc):
        """
        Change the BPC on the fly.
        """
        def get_bpc(bpc):
            switcher_bpc = {
                "Default": 0,
                "6": 6,
                "8": 8,
                "10": 10,
            }
            return switcher_bpc[bpc]

        try:
            cbs_logger.info("{}: {}: BPC Selected". \
                            format(self.port_name, self.device_name))
            icmd_obj = self.create_icmd("DP_COMPONENT", "DP_LEX_SetBpcModeIcmd", False, [get_bpc(bpc)])
            self.send_icmd(icmd_obj)
            # self.reset_device()
        except:
            cbs_logger.exception("{} : {}: Got an exception when changing BPC mode to {}". \
                                 format(self.port_name, self.device_name, bpc))

    def change_bw(self, bw):
        """
        Change the Bandwidth on the fly.
        """
        def get_bw(bw):
            switcher_bw = {
                "Default": 0,
                "5.4 Gbits/s": 0x14,
                "2.7 Gbits/s": 0x0A,
                "1.62 Gbits/s": 0x06,
            }
            return switcher_bw[bw]

        try:
            cbs_logger.info("{}: {}: EDID Selected". \
                            format(self.port_name, self.device_name))
            self.currBandwidth = get_bw(bw)
            icmd_obj = self.create_icmd("DP_COMPONENT", "DP_SetBwLc", False, [self.currBandwidth, self.currLC])
            self.send_icmd(icmd_obj)
            # self.reset_device()
        except:
            cbs_logger.exception("{} : {}: Got an exception when changing Bandwidth to {}". \
                                 format(self.port_name, self.device_name, bw))

    def change_lc(self, lc):
        """
        Change the Lane Count on the fly.
        """
        def get_lc(lc):
            switcher_lc = {
                "Default": 0,
                "4": 4,
                "2": 2,
                "1": 1,
            }
            return switcher_lc[lc]

        try:
            cbs_logger.info("{}: {}: Lane Count Selected". \
                            format(self.port_name, self.device_name))
            self.currLC = get_lc(lc)
            icmd_obj = self.create_icmd("DP_COMPONENT", "DP_SetBwLc", False, [self.currBandwidth, self.currLC])
            self.send_icmd(icmd_obj)
            # self.reset_device()
        except:
            cbs_logger.exception("{} : {}: Got an exception when changing Lane Count to {}". \
                                 format(self.port_name, self.device_name, lc))

    def clearAuxLog(self):
        try:
            cbs_logger.info("{}: {}: Clear Aux Logs". \
                            format(self.port_name, self.device_name))
            icmd_obj = self.create_icmd("DP_AUX_COMPONENT", "Aux_icmdClearAuxTrans", False)
            self.send_icmd(icmd_obj)
        except Exception as e:
            cbs_logger.exception("{} : {}: Got an exception when Clearing Aux Logs: {}". \
                                 format(self.port_name, self.device_name, e))

    def change_auxMenu(self, auxMenu):
        """
        Change the Aux Analyzer menu item.
        """
        def get_auxMenuItem(self, auxMenu):
            switcher_aux = {
                "Default": None,
                "Clear Aux": self.clearAuxLog,
                "Print Aux Size": self.devWindow.getAuxIndex_Click,
                "Print Aux Log": self.devWindow.printAuxButton_Click,
            }
            return switcher_aux[auxMenu]

        try:
            cbs_logger.info("{}: {}: Aux Menu Item selected". \
                            format(self.port_name, self.device_name))
            self.currAux = get_auxMenuItem(self, auxMenu)
            if self.currAux is not None:
                self.currAux()
        except Exception as e:
            cbs_logger.exception("{} : {}: Got an exception when choosing AUX menu: {}". \
                                 format(self.port_name, self.device_name, e))

    def launch_device_window(self):
        """
        Launch a device client window.
        """
        try:
            #TODO: put this method in device client controller class in the new MVC design
            if self.devWindow == None or self.devWindow.IsDisposed:
                # create the DeviceWindow
                self.devWindow = DeviceWindow(self)

                # change DeviceWindow title
                self.devWindow.Text = self.port_name + " " + \
                        self.device_name + ": " + \
                        self.original_icron_file_path + " - Cobs"
                cbs_logger.info("FPGA image : {}".format((self.rex_fpga_image is None) or (self.lex_fpga_image is None)))
                if (self.rex_fpga_image is None) or (self.lex_fpga_image is None):
                    self.devWindow.ProgramFpgaButton.Enabled = False
                else:
                    self.devWindow.ProgramFpgaButton.Enabled = True
                # watch for deletion of files in the .icron file's directory
                
                # self.icron_file_watcher = System.IO.FileSystemWatcher(
                #                                         os.path.dirname(self.original_icron_file_path))
                # # self.icron_file_watcher.Filter = os.path.basename(self.original_icron_file_path)
                # cbs_logger.info(self.original_icron_file_path)
                # self.icron_file_watcher.Deleted += self.icronFileChanged
                # self.icron_file_watcher.Changed += self.icronFileChanged
                # self.icron_file_watcher.EnableRaisingEvents = True
                # self.icron_file_watcher.NotifyFilter = System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite

                # run the DeviceWindow in a different thread, allowing the user
                #     to interact with both the DeviceWindow and the console
                thread = Threading.Thread(Threading.ThreadStart(self.devWindow.run))
                thread.SetApartmentState(Threading.ApartmentState.STA)
                thread.Name = self.devWindow.Text.replace(" - Cobs","")
                thread.Start()
            else:
                Forms.MessageBox.Show( 
                            "Device window is already launched",
                            "WARNING",
                            Forms.MessageBoxButtons.OK,
                            Forms.MessageBoxIcon.Warning)
                raise Exception("Device window is already launched")
        except:
            cbs_logger.exception("{}: {}: Got an error when launching device window". \
                    format(self.port_name, self.device_name))

    def dispose_window(self):
        """
        Dispose the device window.
        """
        self.devWindow.Dispose()

    def exit(self):
        """
        Exit the device window.
        """
        with self.close_window_lock:
            self.devWindow.exit()

    def packet_received_handler(self, sender, packet, valid):
        """
        A generic packet handler for handling received packets. It invokes packet handlers
        based on their associated request id and channel id.

        Arguments: sender - sender of the packet (unused)
                   packet - received packet
        """
        cbs_logger.debug("{}: {}: {}".format(self.port_name, self.device_name, str(packet)))
        try:
            ts = time.time()
            if valid:
                if self.ipacket_handlers.get((packet.response_id, packet.channel_id)):
                    delegate = CallTarget0(lambda: \
                        self.ipacket_handlers[(packet.response_id, packet.channel_id)](packet, ts))
                    self.devWindow.MainSplitContainer.Invoke(delegate)
            else:
                delegate = CallTarget0(lambda: self.invalid_packet_handler(packet, ts))
                self.devWindow.MainSplitContainer.Invoke(delegate)
        except:
            cbs_logger.exception("{}: {}: Got an error when handling an icron packet". \
                    format(self.port_name, self.device_name))

    def invalid_packet_handler(self, packet, timestamp):
        """
        Handles the invalid packet by printing out the packet in ASCII format in device window.

        Arguments: packet - packet to be handled
        """
        try:
            cbs_logger.info("{}: {}: Received an invalid packet={}". \
                    format(self.port_name, self.device_name, packet))
            timestamp_string = datetime.datetime.fromtimestamp(timestamp). \
                                    strftime('%d %H:%M:%S.%f')[2:-3] + ": "
            ascii_char_list = packet
            ascii_message = ''.join(chr(char) for char in ascii_char_list)
            log_message = timestamp_string + ascii_message
            log = LogOutput(log_message, Drawing.Color.Red, True, False)
            self.logger.log(
                    self.port_name + ": " + \
                    self.short_device_name + ": " + \
                    self.device_type + ": " + \
                    str(log))
            self.log_queue.put(log, True)
            if not self.devWindow.suspend:
                # print out all logs in the queue even though one log is produced as it is harmless
                while not self.log_queue.empty():
                    # block when get an ilog message from the queue
                    log = self.log_queue.get(True)
                    self.print_to_device_window(
                                str(log),
                                log.color,
                                not log.timestamped,
                                not log.newline)
                    

        except:
            cbs_logger.exception("{}: {}: Got an error while handling invalid packet". \
                    format(self.port_name, self.device_name))

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
        cbs_logger.debug("{}: {}: Registered packet_handler={}, response_id={}, channel_id={}". \
                format(self.port_name, self.device_name, packet_handler, response_id, channel_id))
        if not response_id is None and not channel_id is None:
            if (response_id, channel_id) not in self.ipacket_handlers:
                self.ipacket_handlers[(response_id, channel_id)] = packet_handler
            else:
                raise Exception("{}: {}: handler={} for resp_id={} chan_id={} already registered". \
                    format(
                        self.port_name,
                        self.device_name,
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
            cbs_logger.debug("{}: {}: Removed packet_handler={}, response_id={}, channel_id={}". \
                format(self.port_name, self.device_name, packet_handler, response_id, channel_id))
        except:
            message = "{}: {}: Got an error when removing packet handler resp_id={} chan_id={}". \
                        format(self.port_name, self.device_name, response_id, channel_id)
            cbs_logger.exception(message)

    def _decode_istatus(self, packet, timestamp):
        try:
            cbs_logger.debug("istatus={}".format(packet))
            timestamp_string = datetime.datetime.fromtimestamp(timestamp). \
                                    strftime('%d %H:%M:%S') + ": "
            istatus = iis.parse_istatus(packet.payload_data)
            istatus_message = timestamp_string + self.istatus_decoder.decode(istatus)
            color = Drawing.Color.Blue
            log = LogOutput(istatus_message, color, timestamped=True, newline=True)
        except (iis.IStatusDecodeError, iis.IStatusFormatError) as e:
            log = LogOutput(timestamp_string + str(e), timestamped=True, newline=False)
            cbs_logger.exception(str(e))
        except: 
            error_message = "{}: {}: Got an error when decoding istatus". \
                    format(self.port_name, self.device_name)
            log = timestamp_string + \
                    error_message + ": " + "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            cbs_logger.exception(error_message)

        try:
            self.logger.log(
                    self.port_name + ": " + \
                    self.short_device_name + ": " + \
                    self.device_type + ": " + \
                    str(log))

            # block when put ilog message into the queue
            self.log_queue.put(log, True)
            if not self.devWindow.suspend:
                # print out all logs in the queue even though one log is produced as it is harmless
                while not self.log_queue.empty():
                    # block when get an log message from the queue
                    log = self.log_queue.get(True)
                    self.print_to_device_window(
                                            str(log),
                                            log.color,
                                            not log.timestamped,
                                            not log.newline)
        except:
            cbs_logger.exception("{}: {}: Got an error when printing istatus". \
                    format(self.port_name, self.device_name))

    def change_auxEnable(self, status):
        """
        Change enable status of aux analyser.

        Arguments: packet - received packet
        """
        self.auxEnable = status

    def _decode_ilog(self, packet, timestamp):
        """
        Decode ilogs embedded in the received packet.
        
        Arguments: packet - received packet
        """
        component = ""
        try:
            timestamp_string = datetime.datetime.fromtimestamp(timestamp). \
                                    strftime('%d %H:%M:%S')+ ": "
            cbs_logger.debug("{}: {}: ilog_packets={}". \
                    format(self.port_name, self.device_name, str(packet)))
           
            ilog = ilg.parse_ilog(packet.payload_data)

            if ilog.event_log or ilog.user_log:
                # ILOG_DEBUG, ILOG_MINOR_EVENT, ILOG_MAJOR_EVENT ILOG_USER_LOG
                color = Drawing.Color.Black
            elif ilog.event_debug_pink:
                # ILOG_DEBUG_CYAN
                color = Drawing.Color.DarkMagenta
            elif ilog.event_debug_green:
                # ILOG_DEBUG_GREEN
                color = Drawing.Color.DarkGreen
            elif ilog.event_debug_orange:
                # ILOG_DEBUG_ORANGE
                color = Drawing.Color.SaddleBrown
            else:
                # ILOG_WARNING, ILOG_MINOR_ERROR, ILOG_MAJOR_EVENT, ILOG_FATAL_ERROR
                color = Drawing.Color.Red
            ilog_message, component = self.ilog_decoder.decode(ilog)
            ilog_time_message = timestamp_string + ilog_message

            # if self.auxEnable and (ilog.event_debug_pink or ilog.event_debug_green or ilog.event_debug_orange):
            log = LogOutput(ilog_time_message, color, True, True)
            # elif not(self.auxEnable) and not(ilog.event_debug_pink or ilog.event_debug_green or ilog.event_debug_orange):
            #     log = LogOutput(ilog_time_message, color, True, True)

        except (ilg.ILogDecodeError, ilg.ILogFormatError) as e:
            color = Drawing.Color.Black
            log = LogOutput(timestamp_string + str(e), color, True, False)
        except: 
            error_message = "{}: {}: Got an error while decoding ilog". \
                    format(self.port_name, self.device_name)
            log = timestamp_string + \
                    error_message + ": " + "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            cbs_logger.exception(error_message)

        try:
            self.logger.log(
                    self.port_name + ": " + \
                    self.short_device_name + ": " + \
                    self.device_type + ": " + \
                    str(log))

            # block when put ilog message into the queue
            			
            if not self.devWindow.suspend:
                self.log_queue.put(log, True)
                # print out all logs in the queue even though one log is produced as it is harmless
                while not self.log_queue.empty():
                    # block when get an ilog message from the queue
                    log = self.log_queue.get(True)
                    self.print_to_device_window(
                                            str(log),
                                            log.color,
                                            not log.timestamped,
                                            not log.newline)
                    if self.liveFilter:
                        if component in self.iLogFilterValues:
                            self.deviceFilter.livePrint(str(log))
            
        except:
            cbs_logger.exception("{}: {}: Got an error when printing log". \
                    format(self.port_name, self.device_name))

    def _decode_ascii(self, packet, timestamp):
        """
        Decode ascii characters embedded in the received packet.

        Arguments: packet - received packet
        """
        try:
            timestamp_string = datetime.datetime.fromtimestamp(timestamp). \
                                    strftime('%d %H:%M:%S.%f')[2:-3] + ": "
            ascii_char_list = packet.payload_data
            ascii_message = ''.join(chr(char) for char in ascii_char_list)
            log_message = timestamp_string + ascii_message
            color = Drawing.Color.Black
            log = LogOutput(log_message, color, True, True)
            self.logger.log(
                    self.port_name + ": " + \
                    self.short_device_name + ": " + \
                    self.device_type + ": " + \
                    str(log))
            self.log_queue.put(log, True)
            if not self.devWindow.suspend:
                # print out all logs in the queue even though one log is produced as it is harmless
                while not self.log_queue.empty():
                    # block when get an log message from the queue
                    log = self.log_queue.get(True)
                    self.print_to_device_window(
                                            str(log),
                                            log.color,
                                            not log.timestamped,
                                            not log.newline)
        except:
            cbs_logger.exception("{}: {}: Got an error while decoding ASCII". \
                    format(self.port_name, self.device_name))

    def print_to_device_window(self,
                            string,
                            color=Drawing.Color.Black,
                            timestamp=True,
                            newline=True):
        """
        Print a string to device client window text box.

        Arguments: string - string to be printed out
                   color  - color of the string to be printed out
        """
        try:
            delegate = CallTarget0(lambda:
                    self.devWindow.print_to_text_box(string, color, timestamp, newline))
            self.devWindow.MainSplitContainer.Invoke(delegate)
        except:
            cbs_logger.exception("{}: {}: Got an error when printing string to device window". \
                    format(self.port_name, self.device_name))

    def create_icmd(self, component_name, function_name, response, *arguments):
        """
        Create icmd object from component name, function name and arguments.

        Arguments: response - whether sender expects a response for the icmd
        """
        try:
            if response:
                self.icmd_response_validator[(self.icmd_response_id, self.icmd_channel_id)] = \
                            self._build_icmd_response_validator(component_name, function_name)

            if len(arguments) == 0:
                return self.icmd_encoder.encode(component_name, function_name)
            else:
                return self.icmd_encoder.encode(component_name, function_name, arguments[0])
        except:
            error_message = "{}: {}: Got an error when creating icmd:". \
                    format(self.port_name, self.device_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

    def _build_icmd_response_validator(self, component_name, function_name):
        """
        A closure for building icmd response validator.

        Arguments: component_name - icmd associated component name
                   function_name - icmd associated function name
        """
        def validator(response_len):
            expected_num_args = self.icmd_model.get_num_icmd_response_arguments(
                                                                    component_name,
                                                                    function_name)
            expected_lenth = expected_num_args * 4
            return expected_lenth == response_len
        return validator

    def send_icmd(self, icmd_obj, response=False):
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

            cbs_logger.info("{}: {}: Trying to send an icmd with a packet of {}". \
                            format(self.port_name, self.device_name, packet))
            self.serial_port_manager.write(packet)
        except:
            error_message = "{}: {}: Got an error when sending icmd". \
                                format(self.port_name, self.device_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

    def send_icmd_wait_for_response(self, icmd_obj):
        """
        Send icmd to device and wait for its response.

        Arguments: icmd_obj - icmd object to be sent out
        """
        try:
            with self.send_icmd_wait_for_response_lock:
                cbs_logger.info(
                    "{}: {}: Sending an icmd and waiting for response: respon_id={} chan_id={}". \
                        format(
                            self.port_name,
                            self.device_name,
                            self.icmd_response_id,
                            self.icmd_channel_id))

                self.register_packet_handler(
                        self._decode_icmd_response,
                        self.icmd_response_id,
                        self.icmd_channel_id)

                self.send_icmd(icmd_obj, True)
                return self._poll_icmd_response(self.icmd_response_id, self.icmd_channel_id)
        except:
            error_message = "{}: {}: Got an error when sending icmd and waiting for response:" \
                                .format(self.port_name, self.device_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

    def _decode_icmd_response(self, packet, timestamp=None):
        """
        Decode icmd response from received packet.

        Arguments: packet - received packet from icmd channel
        """
        try:
            cbs_logger.debug(
                "{}: {}: Got an icmd response chan_id={} response_id={} len={} payload={}". \
                    format(self.port_name, self.device_name, packet.channel_id,
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
                    cbs_logger.debug("{}: ".format(self.port_name) + \
                                    "{}: ".format(self.device_name) + \
                                    "Timed out on polling icmd response for " + \
                                    "response_id={}, ".format(response_id) + \
                                    "channel_id={}, ".format(channel_id) + \
                                    "timeout={}".format(timeout))
                    break

            self.remove_packet_received_handler(response_id, channel_id)
            return result
        except:
            cbs_logger.exception("{}: {}: Got an error when polling icmd response". \
                    format(self.port_name, self.device_name))

    def _decode_device_info(self, packet, timestamp):
        """
        Decode the device info from received packet.

        Arguments: packet - received packet from device info channel
        """
        try:
            cbs_logger.debug("{}: {}: Received a device info packet={}". \
                    format(self.port_name, self.device_name, packet))
            info_header = idi.InfoMessageHeader(packet.payload_data[:4])
            if info_header.packet_size != packet.payload_length:
                raise idi.InvalidMessageSizeError(
                        "Invalid packet size for the device info message",
                        packet.payload_data)
            
            info = info_header.info_constructor(packet.payload_data[4:])
            if info_header.hardware_info:
                cbs_logger.debug("Hardware info received") #this command was included for excom, not being used in cobs

            elif info_header.bootrom_info or info_header.software_info or info_header.software_info_fw: 
                # get device type
                self.device_type = 'LEX' if info.lex else 'REX'

                info_type = "bootrom" if info_header.bootrom_info else "software"
                cbs_logger.info("{}: {}: {}: Received {} info". \
                            format(self.port_name, self.device_name, self.device_type, info_type))

                # get programming mode
                mode = 'BootromMode' if info_header.bootrom_info else 'FlashMode'

                # get fpga build time
                fpga_build_time = info.fpga_build_time

                self.device_info_result[(packet.response_id, packet.channel_id)] = \
                                                                        (mode, fpga_build_time)

                # update device window title
                new_title = "{} {} {}: {} - Cobs". \
                                            format(self.port_name,
                                                    self.device_name,
                                                    self.device_type,
                                                    self.original_icron_file_path)
                delegate = CallTarget0(lambda: self.update_text(new_title))
                self.devWindow.MainSplitContainer.Invoke(delegate)

                # print out device info in device window
                timestamp_string = datetime.datetime.fromtimestamp(timestamp). \
                                    strftime('%d %H:%M:%S.%f')[2:-3] + ": "
                log_message = timestamp_string + str(info)
                color = Drawing.Color.Black
                log = LogOutput(log_message, color, True, True)
                self.logger.log(
                        self.port_name + ": " + \
                        self.short_device_name + ": " + \
                        self.device_type + ": " + \
                        str(log))
                self.log_queue.put(log, True)
                if not self.devWindow.suspend:
                    # print out all logs in the queue even though only one log is produced as it is
                    # harmless
                    while not self.log_queue.empty():
                        # block when get an log message from the queue
                        log = self.log_queue.get(True)
                        self.print_to_device_window(
                                                str(log),
                                                log.color,
                                                not log.timestamped,
                                                not log.newline)
            else:
                new_title = "{} {} {}: {}". \
                        format(self.port_name, self.device_name, self.device_type, str(info))
                delegate = CallTarget0(lambda: self.update_text(new_title))
                self.devWindow.MainSplitContainer.Invoke(delegate)

        except (idi.InvalidMessageSizeError, idi.UnsupportedInfoMessageError) as e:
            cbs_logger.exception("{}: {}: Got an error when decoding dev info". \
                    format(self.port_name, self.device_name))
            timestamp_string = datetime.datetime.now(). \
                                    strftime('%d %H:%M:%S.%f')[2:-3] + ": "
            message = timestamp_string + str(e)
            self.print_to_device_window(message)
            self.logger.log(
                    self.port_name + ": " + \
                    self.short_device_name + ": " + \
                    self.device_type + ": " + \
                    message)
        except:
            error_message = "{}: {}: Got an error when decoding device info". \
                                format(self.port_name, self.device_name)
            cbs_logger.exception(error_message)

    def _poll_device_info(self, response_id, channel_id, timeout=3):
        """
        Poll the device info associated with the response id and channel id from the device.

        Arguments: response_id - response id associated with the response
                   channel_id - channel id associated with the response
                   timeout - how long for polling the response
        """
        try:
            self.device_info_result = {}
            start_time = time.time()
            result = (None, None)
            while True:
                if self.device_info_result.get((response_id, channel_id)):
                    result = self.device_info_result.pop((response_id, channel_id))
                    cbs_logger.debug("{}: {}: Received device info result={}". \
                            format(self.port_name, self.device_name, result))
                    break
                
                if time.time() - start_time > timeout:
                    cbs_logger.debug("{}: ".format(self.port_name) + \
                                    "{}: ".format(self.device_name) + \
                                    "Timed out on polling icmd response for " + \
                                    "response_id={}, ".format(response_id) + \
                                    "channel_id={}, ".format(channel_id) + \
                                    "timeout={}".format(timeout))
                    break
            return result
        except:
            error_message = "{}: {}: Got an error when polling device info". \
                    format(self.port_name, self.device_name)
            cbs_logger.exception(error_message)

    def reset_device(self):
        """
        Reset device by sending a reset_device_command to the device.
        """
        try:
            message = "{}: {}: Reset device".format(self.port_name, self.device_name)
            cbs_logger.info(message)

            reset_device_command = isc.ResetDeviceCommand()
            response = self.send_device_command_wait_for_response(
                                        self.program_command_response_id,
                                        self.program_command_channel_id,
                                        reset_device_command)

            if response == None:
                cbs_logger.info("{}: {}: Timed out on reset device response". \
                                    format(self.port_name, self.device_name))
            else:
                message = "{}: {}: Got a response={} for reset device". \
                            format(self.port_name, self.device_name, response)
                cbs_logger.info(message)
                reset_device_response = isc.ResetDeviceCommandResponse(response)
                if not reset_device_response.is_ack:
                    raise ip.ProgramDeviceNakError(
                            "Got a NAK for reset device response from {} at {}". \
                                format(self.device_name, self.port_name))
        except:
            message = "{}: {}: Got an error when resetting device". \
                            format(self.port_name, self.device_name)
            cbs_logger.exception(message)

    def handle_program_device(self, parameters):
        """
        A wrapper method for handling program device invoked from devWindow as IronPython seems not
        supporting passing multiple parameters when start new .Net thread
        """
        try:
            image_type = parameters[0]
            golden_image = parameters[1]
            self.program_device(image_type, golden_image)
        except:
            message = "{}: {}: Got an error when handling program device". \
                            format(self.port_name, self.device_name)
            cbs_logger.exception(message)

    def program_device(self, image_type, golden_image=False):
        """
        Beginning of the device programming, first send run_program_bb command to device and decode
        the device info message to determine the programming mode. Then proceed to
        the next stage of device programming.

        Arguments: image_type - type of image to be programmed i.e. FPGA or FW
                   auto_detect - auto program FPGA based on chip build time
        """
        try:
            # update the info log
            message = "{}: {}: {}: Starting to program device". \
                    format(self.port_name, self.device_name, self.device_type)
            cbs_logger.info(message)
            delegate = CallTarget0(lambda: self.update_text(message))
            self.devWindow.MainSplitContainer.Invoke(delegate)

            # send out the run_program_bb command
            cbs_logger.info("{}: {}: {}: Sending runProgramBB comamnd". \
                    format(self.port_name, self.device_name, self.device_type))

            # construct the run_program_bb command
            run_program_bb_cmd = ip.RunProgramBBCommand()
            response = self.send_device_command_wait_for_response(
                                self.program_command_response_id,
                                self.program_command_channel_id,
                                run_program_bb_cmd)

            # response is NACK in bootrom mode so don't raise exception
            if response is None:
                cbs_logger.info("{}: {}: {}: Timed out on runProgramBB comand response". \
                                    format(self.port_name, self.device_name, self.device_type))
                # raise exception if times out
                raise ip.ProgramDeviceTimeoutError(
                            "{}: {}: Timed out on runProgramBB comand response". \
                                format(self.device_name, self.port_name))
            else:
                message = "{}: {}: {}: Got a response={} for runProgramBB". \
                            format(self.port_name, self.device_name, self.device_type, response)
                cbs_logger.info(message)

            if self.serial_port_manager.baud_rate == 115200:
                original_baud_rate = 115200
                self.change_baud_rate(460800)
            message = "{}: {}: {}: Baud rate is {}". \
                        format(
                            self.port_name,
                            self.device_name,
                            self.device_type,
                            self.serial_port_manager.baud_rate)
            cbs_logger.info(message)

            # poll the programming mode for the device info
            programming_mode, chip_build_time = \
                    self._poll_device_info(255, self.device_info_channel_id, 10)

            # update the info log
            message = "{}: {}: {}: Got device info for programming in {} chip build time {}". \
                            format(self.port_name, self.device_name, self.device_type,
                                    programming_mode, chip_build_time)
            cbs_logger.info(message)
            delegate = CallTarget0(lambda: self.update_text(message))
            self.devWindow.MainSplitContainer.Invoke(delegate)
            #block the logs while programming so that we can program both units together
            if self.cobs.startupWindow.suspend_pgmBB_logs != False:
                for port_name in self.cobs.ports:
                    self.print_to_device_window("*******No Log while Programming***********")
                self.devWindow.suspend = True
                if (self.device_type == "REX"):
                    self.cobs.rex_suspend_logs = True
                    cbs_logger.info("REX = {}". format(self.cobs.rex_suspend_logs))
                elif (self.device_type == "LEX"):
                    self.cobs.lex_suspend_logs = True
                    cbs_logger.info("LEX = {}". format(self.cobs.lex_suspend_logs))
            # handle the programming mode
            if programming_mode is None or chip_build_time is None:
                # raise exception if times out
                raise ip.ProgramDeviceTimeoutError(
                            "Times out for getting programming mode from {} at {}". \
                                format(self.device_name, self.port_name))
            else:

                region_type = 'GOLDEN' if golden_image else 'CURRENT'
                # start to program device image
                self.program_device_image(programming_mode, image_type, region_type)
            
        except:
            error_message = "{}: {}: {}: Got an error when programming device". \
                                format(self.port_name, self.device_name, self.device_type)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)
        
        finally:
            message = "{}: {}: {}: Baud rate is {}". \
                        format(
                            self.port_name,
                            self.device_name,
                            self.device_type,
                            self.serial_port_manager.baud_rate)
            cbs_logger.info(message)
            #unfreeze the programming buttons
            self.devWindow.ProgramFwButton.Enabled = True
            if (self.rex_fpga_image == None) or (self.lex_fpga_image == None):
                self.devWindow.ProgramFpgaButton.Enabled = False
            else:
                self.devWindow.ProgramFpgaButton.Enabled = True
            #update the variable to tell programming is done and logs can be unsuspended
            if (self.device_type == "REX"):
                self.cobs.rex_suspend_logs = False
                cbs_logger.info("REX unsuspended")
            elif (self.device_type == "LEX"):
                self.cobs.lex_suspend_logs = False
                cbs_logger.info("LEX unsuspended")
            if self.cobs.startupWindow.suspend_pgmBB_logs != False:
                while(not(self.cobs.rex_suspend_logs == False and self.cobs.lex_suspend_logs == False)):
                    self.devWindow.suspend = True
                #unblock logs if programming is done on both units
                self.devWindow.suspend = False

    def program_device_image(self, mode, image_type, region_type):
        """
        Program device image by sending start_to_program command and wait for the response from
        device.

        Arguments: mode - programming mode i.e. Flash mode or Bootrom mode
                   image_type - type of image to be programmed i.e. FPGA or FW
        """
        try:
            programming_info = {
                ("BootromMode", "FWType", 'LEX', 'GOLDEN'): \
                    ("PROGRAM_REGION_FLASHWRITER", 'flash_writer_image', self.flash_writer_image),

                ("BootromMode", "FWType", 'LEX', 'CURRENT'): \
                    ("PROGRAM_REGION_FLASHWRITER", 'flash_writer_image', self.flash_writer_image),

                ("BootromMode", "FWType", 'REX', 'GOLDEN'): \
                    ("PROGRAM_REGION_FLASHWRITER", 'flash_writer_image', self.flash_writer_image),

                ("BootromMode", "FWType", 'REX', 'CURRENT'): \
                    ("PROGRAM_REGION_FLASHWRITER", 'flash_writer_image', self.flash_writer_image),

                ("BootromMode", "FPGAType", 'LEX', 'GOLDEN'): \
                    ("PROGRAM_REGION_FLASHWRITER", 'flash_writer_image', self.flash_writer_image),

                ("BootromMode", "FPGAType", 'LEX', 'CURRENT'): \
                    ("PROGRAM_REGION_FLASHWRITER", 'flash_writer_image', self.flash_writer_image),

                ("BootromMode", "FPGAType", 'REX', 'GOLDEN'): \
                    ("PROGRAM_REGION_FLASHWRITER", 'flash_writer_image', self.flash_writer_image),

                ("BootromMode", "FPGAType", 'REX', 'CURRENT'): \
                    ("PROGRAM_REGION_FLASHWRITER", 'flash_writer_image', self.flash_writer_image),

                ("FlashMode", "FWType", 'LEX', 'GOLDEN'): \
                    ("PROGRAM_REGION_FW_GOLDEN", 'main_firmware_image', self.main_firmware_image), 

                ("FlashMode", "FWType", 'LEX', 'CURRENT'): \
                    ("PROGRAM_REGION_FW_CURRENT", 'main_firmware_image', self.main_firmware_image), 

                ("FlashMode", "FWType", 'REX', 'GOLDEN'): \
                    ("PROGRAM_REGION_FW_GOLDEN", 'main_firmware_image', self.main_firmware_image),

                ("FlashMode", "FWType", 'REX', 'CURRENT'): \
                    ("PROGRAM_REGION_FW_CURRENT", 'main_firmware_image', self.main_firmware_image),

                ("FlashMode", "FPGAType", 'LEX', 'GOLDEN'): \
                    ("PROGRAM_REGION_FPGA_FW_GOLDEN", 'lex_fpga_image', self.lex_fpga_image),

                ("FlashMode", "FPGAType", 'LEX', 'CURRENT'): \
                    ("PROGRAM_REGION_FPGA_FW_CURRENT", 'lex_fpga_image', self.lex_fpga_image),

                ("FlashMode", "FPGAType", 'REX', 'GOLDEN'): \
                    ("PROGRAM_REGION_FPGA_FW_GOLDEN", 'rex_fpga_image', self.rex_fpga_image),

                ("FlashMode", "FPGAType", 'REX', 'CURRENT'): \
                    ("PROGRAM_REGION_FPGA_FW_CURRENT", 'rex_fpga_image', self.rex_fpga_image)}

            try:
                region, image_name, image = \
                        programming_info.get((mode, image_type, self.device_type, region_type))
            except:
                raise ip.InvalidProgrammingModeError(
                        "Ivalid programming mode for {} at {}". \
                                format(self.device_name, self.port_name),
                        mode,
                        image_type)

            crc = ic.crc32(bytearray(image))
            cbs_logger.info("{}: {}: {}: Computed {} crc={}". \
                format(self.port_name, self.device_name, self.device_type, image_name, hex(crc)))

            message = "{}: {}: {}: Start to program {} in {} to {}". \
                format(self.port_name, self.device_name, self.device_type, image_name, mode, region)
            cbs_logger.info(message)
            delegate = CallTarget0(lambda: self.update_text(message))
            self.devWindow.MainSplitContainer.Invoke(delegate)

            # construct start_to_program command
            programming_start_command = ip.ProgramStartCommand(region, len(image), crc)

            # send out the command and wait for the response
            response = self.send_device_command_wait_for_response(
                                self.program_command_response_id,
                                self.program_command_channel_id,
                                programming_start_command) 

            # handle the response
            if response is None:
                # raise exception if the response times out
                raise ip.ProgramDeviceTimeoutError(
                        "Timed out for receiving start to program response from {} at {} in {}". \
                                format(self.device_name, self.port_name, mode))
            else:
                # update the info log
                message = "{}: {}: {}: Got a response={} for start to program device in {} mode". \
                        format(self.port_name, self.device_name, self.device_type, response, mode)
                cbs_logger.info(message)
                delegate = CallTarget0(lambda: self.update_text(message))
                self.devWindow.MainSplitContainer.Invoke(delegate)

                program_device_response = ip.ProgramDeviceResponse(response)
                if program_device_response.is_ack:
                    # verify the expected number of  blocks to be erased in the response
                    if mode == 'FlashMode' and program_device_response.blocks_to_erase == 0:
                        raise ip.EraseCommandResponseError(
                                "Got response in flash mode but invalid num of blocks to erased",
                                response)

                    if mode == 'BootromMode' and program_device_response.blocks_to_erase > 0:
                        raise ip.EraseCommandResponseError(
                                "Got response in bootrom mode but invalid num of blocks to erased",
                                response)
                    # self.program_data(mode, image_type, region_type)
                    if self.fpgaErased == True and image_type == 'FPGAType': # Got an ack for the response so continue to erase flash
                        self.program_data('FlashMode', 'FPGAType', region_type)
                    else:    
                        self.erase_device_flash(
                                program_device_response.blocks_to_erase,
                                mode,
                                image_type,
                                region_type)
                else:
                    # raise exception if response is nak
                    raise ip.ProgramDeviceNakError(
                    "Got a NAK for start to program cmd response from {} at {} in {} mode". \
                                format(self.device_name, self.port_name, mode))
        except:
            error_message = "{}: {}: {}: Got an error when programming device in {} mode". \
                                format(self.port_name, self.device_name, self.device_type, mode)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

    def erase_device_flash(self, num_blocks_to_erase, mode, image_type, region_type):
        """
        Erase device flash as part of the programming process.

        Arguments: num_blocks_to_erase - number of flash blocks to be erased, given by the device
                                         response of start_to_program command
                   mode - programming mode i.e. Flash mode or Bootrom mode
                   image_type - type of image to be programmed i.e. FPGA or FW
        """
        try:
            # erase one block at a time
            for i in range(num_blocks_to_erase):

                # update the info log
                message = "{}: {}: {}: Trying to erase device flash block {} out of {}". \
                            format(
                                self.port_name,
                                self.device_name,
                                self.device_type,
                                i,
                                num_blocks_to_erase - 1)
                cbs_logger.debug(message)
                delegate = CallTarget0(lambda: self.update_text(message))
                self.devWindow.MainSplitContainer.Invoke(delegate)

                # construct the erase_block command
                erase_block_command = ip.EraseBlockCommand()

                # send out the command and wait for the response
                response = self.send_device_command_wait_for_response(
                                    self.program_command_response_id,
                                    self.program_command_channel_id,
                                    erase_block_command)

                # handle the response
                if response is None:
                    # raise exception if the response times out
                    raise ip.ProgramDeviceTimeoutError(
                                "Times out for {} erase block {} command response of at {}". \
                                    format(self.device_name, i, self.port_name))
                else:
                    # update the info log
                    message = "{}: {}: {}: Got an erase block response={} for block {}". \
                            format(self.port_name, self.device_name, self.device_type, response, i)
                    cbs_logger.debug(message)
                    delegate = CallTarget0(lambda: self.update_text(message))
                    self.devWindow.MainSplitContainer.Invoke(delegate)

                    erase_block_cmd_resp = ip.EraseBlockCommandResponse(response)

                    # verify the current block num  being erased in the response
                    if erase_block_cmd_resp.blocks_to_erase + i != num_blocks_to_erase - 1:
                        raise ip.EraseCommandResponseError(
                                "Incorrect erase command response in blocks to erase",
                                response)

                    # raise exception when erase block command response is nak
                    if not erase_block_cmd_resp.is_ack:
                        raise ip.ProgramDeviceNakError(
                            "Got a NAK for erase block {} command response from {} at {}". \
                                format(i, self.device_name, self.port_name))

            message = "{}: {}: {}: Erased total {} blocks of device flash in {}". \
                        format(
                            self.port_name,
                            self.device_name,
                            self.device_type,
                            num_blocks_to_erase,
                            mode)
            cbs_logger.info(message)
            
            # send data to device
            if mode == 'BootromMode':
                self.program_data(mode, image_type, region_type)

            elif mode == 'FlashMode':
                if image_type == 'FWType':
                    self.program_data(mode, image_type, region_type)
                elif image_type == 'FPGAType':
                    self.fpgaErased = True
                    self.program_device_image('FlashMode', 'FWType', region_type)  

        except:
            error_message = "{}: {}: {}: Got an error when erasing device flash". \
                                format(self.port_name, self.device_name, self.device_type)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

    def program_data(self, mode, image_type, region_type, chunk_size=256):
        """
        Transfer image data to the device, as part of the programming process.

        Arguments: num_blocks_to_erase - number of flash blocks to be erased, given by the device
                                         response of start_to_program command
                   mode - programming mode i.e. Flash mode or Bootrom mode
                   image_type - type of image to be programmed i.e. FPGA or FW
                   chunk_size - size of each data chunk to be transferred.
        """
        try:
            programming_info = {
                ("BootromMode", "FWType", 'LEX'): ('flash_writer_image', self.flash_writer_image),

                ("BootromMode", "FWType", 'REX'): ('flash_writer_image', self.flash_writer_image),

                ("BootromMode", "FPGAType", 'LEX'): ('flash_writer_image', self.flash_writer_image),

                ("BootromMode", "FPGAType", 'REX'): ('flash_writer_image', self.flash_writer_image),

                ("FlashMode", "FWType", 'LEX'): ('main_firmware_image', self.main_firmware_image), 

                ("FlashMode", "FWType", 'REX'): ('main_firmware_image', self.main_firmware_image),

                ("FlashMode", "FPGAType", 'LEX'): ('lex_fpga_image', self.lex_fpga_image),

                ("FlashMode", "FPGAType", 'REX'): ('rex_fpga_image', self.rex_fpga_image)}

            try:
                image_name, image = programming_info.get((mode, image_type, self.device_type))
            except:
                raise ip.InvalidProgrammingModeError(
                        "Ivalid programming mode for {} at {}". \
                                format(self.device_name, self.port_name),
                        mode,
                        image_type)

            cbs_logger.info("{}: {}: {}: Starting to download {}". \
                        format(self.port_name, self.device_name, self.device_type, image_name))

            num_send = len(image) / chunk_size if len(image) % chunk_size == 0 \
                            else len(image) / chunk_size + 1
	    # first packet
            packet_number = 0
            data_0 = [ord(byte) for byte in \
                            image[chunk_size * packet_number : chunk_size *(packet_number + 1)]]
            timeout = 3
            program_data_command = ip.ProgramDataCommand(data_0)
            # self.register_packet_handler(
            #                     self._extract_program_device_response,
            #                     self.program_command_response_id,
            #                     self.program_data_channel_id)
            # packet = self.send_device_command(self.program_command_response_id,
            #                                  self.program_data_channel_id,
            #                                  program_data_command) 
            # self.serial_port_manager.write(packet)
            self.send_program_command(self.program_command_response_id,
                                     self.program_data_channel_id, 
                                     program_data_command)
            packet_number = packet_number + 1  
            while packet_number < num_send :
                
                message = "{}: {}: {}: Trying to send data chunk {} out of {}". \
                        format(self.port_name, self.device_name, self.device_type, packet_number-1, num_send-1)
                cbs_logger.debug(message)
                delegate = CallTarget0(lambda: self.update_text(message))
                self.devWindow.MainSplitContainer.Invoke(delegate)
                if packet_number == num_send - 1:
                    # last send
                    data_next = [ord(byte) for byte in image[chunk_size * packet_number :]]
                    timeout = 30
                else:
                    data_next = [ord(byte) for byte in \
                            image[chunk_size * (packet_number) : chunk_size *(packet_number + 1)]]
                    timeout = 3
                    
                self.program_command_response_id_next = self.program_command_response_id + 1 \
                        if self.program_command_response_id < 254 else 0
                
                program_data_command_1 = ip.ProgramDataCommand(data_next)
                # self.register_packet_handler(
                #                 self._extract_program_device_response,
                #                 self.program_command_response_id_next,
                #                 self.program_data_channel_id)
                # packet = self.send_device_command(self.program_command_response_id_next,
                #                             self.program_data_channel_id,
                #                             program_data_command_1)
                # self.serial_port_manager.write(packet)
                self.send_program_command(self.program_command_response_id_next,
                                         self.program_data_channel_id, 
                                         program_data_command_1)
                cbs_logger.debug("{}-{}". format(self.program_command_response_id, self.program_data_channel_id))
                response = self._poll_device_response(self.program_command_response_id,
                                            self.program_data_channel_id, timeout) 
                if packet_number == num_send - 1:
                    response = self._poll_device_response(self.program_command_response_id_next ,
                                            self.program_data_channel_id, timeout)
                    cbs_logger.debug("Receving Response for last packet {}". format(self.program_command_response_id_next))
            
                packet_number = packet_number + 1
                if response is None:
                    # time out for the response
                    self.remove_packet_received_handler(self.program_command_response_id_next,
                                                        self.program_data_channel_id)
                    timestamp_string = datetime.datetime.now(). \
                        strftime('%Y-%m-%d %H:%M:%S.%f') + ": "
                    raise ip.ProgramDeviceTimeoutError(
                            "Timed out for program data response chunk {} of device {} at {}". \
                                    format(packet_number, self.device_name, self.port_name))
                else:
                    # update the info log
                    "{}: {}: {}: Got a program data response={} for chunk {}". \
                            format(self.port_name, self.device_name, self.device_type, response, packet_number)
                    cbs_logger.debug(message)
                    delegate = CallTarget0(lambda: self.update_text(message))
                    self.devWindow.MainSplitContainer.Invoke(delegate)

                    program_data_response = ip.ProgramDataCommandResponse(response)

                    # program data command response is a nak
                    if not program_data_response.is_ack:
                        raise ip.ProgramDeviceNakError(
                                "Got a NAK for program data chunk {} response from {} at {}". \
                                    format(packet_number, self.device_name, self.port_name))

            message = "{}: {}: {}: Downloaded {} in total of {} chunks in {}". \
                            format(
                                self.port_name,
                                self.device_name,
                                self.device_type,
                                image_name,
                                num_send,
                                mode)
            cbs_logger.info(message)

            if mode == 'BootromMode':
                programming_mode, chip_build_time = \
                        self._poll_device_info(255, self.device_info_channel_id, 3)
                if programming_mode != 'FlashMode':
                    raise ip.InvalidProgrammingModeError(
                                "Invalid programming mode for {} at {}". \
                                format(self.device_name, self.port_name),
                                programming_mode,
                                'FWType')
                if image_type == 'FWType':
                    self.program_device_image(programming_mode, 'FWType', region_type)
                elif image_type == 'FPGAType':
                    self.program_device_image(programming_mode, 'FPGAType', region_type)
                else:
                    raise ip.InvalidProgrammingOperationError(
                            "Invalid programming operation for {} at {} mode={} image_type={}". \
                                    format(self.device_name, self.port_name, mode, image_type))
            elif mode == 'FlashMode':
                if image_type == 'FWType':
                    # self.program_data('FlashMode', 'FPGAType', region_type)
                    if self.fpgaErased == True:
                        self.program_device_image('FlashMode', 'FPGAType', region_type) 
                    else:
                        program_response = ip.ProgramDataCommandResponse(response)
                        if program_response.is_ack:
                        # update the info log
                            message = "{}: {}: {}: Device programming is done". \
                                        format(self.port_name, self.device_name, self.device_type)
                            cbs_logger.info(message)
                            self.print_to_device_window(message, Drawing.Color.DarkGreen)
                            new_title = "{} {} {}: {} - Cobs". \
                                        format(
                                            self.port_name,
                                            self.device_name,
                                            self.device_type,
                                            self.original_icron_file_path)
                            delegate = CallTarget0(lambda: self.update_text(new_title))
                            self.devWindow.MainSplitContainer.Invoke(delegate)
                            if self.devWindow.autoReset.Checked == True:
                                self.reset_device()

                        else:
                        # raise exception if the response is nak, which means crc error
                            raise ip.ProgramDeviceNakError(
                                "Got a NAK for RunProgramBB response from {} at {}". \
                                        format(self.device_name, self.port_name))
                    
                elif image_type == 'FPGAType':
                    program_response = ip.ProgramDataCommandResponse(response)
                    if program_response.is_ack:
                    # update the info log
                        message = "{}: {}: {}: Device programming is done". \
                                    format(self.port_name, self.device_name, self.device_type)
                        cbs_logger.info(message)
                        self.print_to_device_window(message, Drawing.Color.DarkGreen)
                        new_title = "{} {} {}: {} - Cobs". \
                                    format(
                                        self.port_name,
                                        self.device_name,
                                        self.device_type,
                                        self.original_icron_file_path)
                        delegate = CallTarget0(lambda: self.update_text(new_title))
                        self.devWindow.MainSplitContainer.Invoke(delegate)
                        self.fpgaErased = False
                        if self.devWindow.autoReset.Checked == True:
                            self.reset_device()

                    else:
                    # raise exception if the response is nak, which means crc error
                        raise ip.ProgramDeviceNakError(
                            "Got a NAK for RunProgramBB response from {} at {}". \
                                    format(self.device_name, self.port_name))
                else:
                    raise ip.InvalidProgrammingOperationError(
                            "Invalid programming operation for {} at {} mode={} image_type={}". \
                                    format(self.device_name, self.port_name, mode, image_type))
            else:
                raise ip.InvalidProgrammingOperationError(
                        "Invalid programming operation for {} at {} mode={} image_type={}". \
                                format(self.device_name, self.port_name, mode, image_type))

        except:
            error_message = "{}: {}: {}: Got an error while programming image {}". \
                            format(self.port_name, self.device_name, self.device_type, image_type)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

    def _extract_program_device_response(self, packet, timestamp=None):
        """
        Extract program device response from received packet.

        Arguments: packet - received packet.
        """
        try:
            cbs_logger.debug("{}: {}: {}: Got a device program response:" + \
                    "chan_id={} resp_id={} len={} payload={}". \
                                                        format(
                                                            self.port_name,
                                                            self.device_name,
                                                            self.device_type,
                                                            packet.channel_id,
                                                            packet.response_id,
                                                            packet.payload_length,
                                                            packet.payload_data))
            
            self.program_device_response_result[(packet.response_id, packet.channel_id)] = \
                                                                        packet.payload_data
        except:
            cbs_logger.exception("{}: {}: {}: Got an error when decoding program response". \
                    format(self.port_name, self.device_name, self.device_type))

    def send_device_command(self, response_id, channel_id, command_obj):
        """
        Send device command by writing the packet of device command to serial port.

        Arguments: response_id - associated response id of the device command
                   channel_id - associated channel id of the device command
                   command_obj - device command object to be sent out
        """
        try:
            payload_len = 0 if len(command_obj.as_integer_list) == 256 \
                            else len(command_obj.as_integer_list)

            packet = ipk.packetize(
                        channel_id,
                        payload_len,
                        command_obj.as_integer_list,
                        response_id)

            cbs_logger.debug("{}: {}: {}: Trying to send device command with a packet={}". \
                    format(self.port_name, self.device_name, self.device_type, str(packet)))

            self.serial_port_manager.write(packet)
        except:
            message = "{}: {}: {}: Got an error when sending device command". \
                            format(self.port_name, self.device_name, self.device_type)
            cbs_logger.exception(message)

    def send_device_command_wait_for_response(self, response_id, channel_id, cmd_obj, timeout=8):
        """
        Send device command to device and wait for its response. A handler for the expected
        received packet is registered. Then it will be removed after the device response is
        extracted from the received packet in polling device response.

        Arguments: response_id - associated response id of the device command
                   channel_id - associated channel id of the device command 
                   cmd_obj - device command object to be sent out
                   timeout - period of polling
        """
        try:
            with self.device_command_lock:

                self.register_packet_handler(
                                self._extract_program_device_response,
                                response_id,
                                channel_id)

                self.send_device_command(response_id, channel_id, cmd_obj)

                return self._poll_device_response(response_id, channel_id, timeout)
        except:
            message = "{}: {}: {}: Got an error when sending device cmd waiting for response". \
                            format(self.port_name, self.device_name, self.device_type)
            cbs_logger.exception(message)

    def send_program_command(self, response_id, channel_id, command_obj):
        """ 
        Send program command to device and wait for its response. A handler for the expected
        received packet is registered. Then it will be removed after the device response is
        extracted from the received packet in polling device response.

        Arguments: response_id - associated response id of the device command
                   channel_id - associated channel id of the device command 
                   command_obj - device command object to be sent out
                   timeout - period of polling
        """
        try:
            with self.device_command_lock:

                self.register_packet_handler(
                                self._extract_program_device_response,
                                response_id,
                                channel_id)
                payload_len = 0 if len(command_obj.as_integer_list) == 256 \
                            else len(command_obj.as_integer_list)

                packet = ipk.packetize(
                            channel_id,
                            payload_len,
                            command_obj.as_integer_list,
                            response_id)
                self.serial_port_manager.write(packet)

        except:
            message = "{}: {}: {}: Got an error when sending device cmd waiting for response". \
                            format(self.port_name, self.device_name, self.device_type)
            cbs_logger.exception(message)

    def _poll_device_response(self, response_id, channel_id, timeout=3):
        """
        Poll the device response associated with the response id and channel id from the program
        device response result.

        Arguments: response_id - response id associated with the response
                   channel_id - channel id associated with the response
                   timeout - how long for polling the response
                   update_response_id - whether to update response id
        """
        try:
            self.program_command_response_id = self.program_command_response_id + 1 \
                            if self.program_command_response_id < 254 else 0
            start_time = time.time()
            result = None
            while True:
                if self.program_device_response_result.get((response_id, channel_id)):
                    result = self.program_device_response_result.pop((response_id, channel_id))
                    cbs_logger.debug("{}: {}: {}: Received device command response={}". \
                            format(self.port_name, self.device_name, self.device_type, result))
                    break
                
                if time.time() - start_time > timeout:
                    cbs_logger.debug("{}: ".format(self.port_name) + \
                                    "{}: ".format(self.device_name) + \
                                    "{}: ".format(self.device_type) + \
                                    "Timed out on polling device response for " + \
                                    "response_id={}, ".format(response_id) + \
                                    "channel_id={}, ".format(channel_id) + \
                                    "timeout={}".format(timeout))
                    break
            self.remove_packet_received_handler(response_id, channel_id)
            return result
        except:
            error_message = "{}: {}: {}: Got an error when polling device response". \
                    format(self.port_name, self.device_name, self.device_type)
            cbs_logger.exception(error_message)

    def load_icron_model(self, icron_parsed_file, icron_file_path):
        """
        Load Icron model from the icron parsed file and assign the Icron file path.

        Arguments: icron_parsed_file - icron_parsed_file object defined in icron_file_parser.py
                   icron_file_path - path of the loaded icron file
        """
        try:
            self.copied_icron_file_path = icron_file_path
            self.original_icron_file_path = self.cobs.startupWindow.icronFileTextBox.Text 
            self.short_device_name = icron_parsed_file.get_short_device_name(self.device_name)

            if icron_parsed_file.get_golden_current_fpga_image(self.device_name) == 'golden':
                self.goldenImage = True
            else:
                self.goldenImage = False

            self.ilog_model = im.IcronILogModel(
                                    self.device_name,
                                    icron_parsed_file.get_icomponent_json(self.device_name),
                                    icron_parsed_file.get_ilog_json(self.device_name),
                                    icron_parsed_file.get_severity_json(self.device_name))

            self.ilog_decoder = ilg.ILogDecoder(self.ilog_model)

            self.istatus_model = im.IcronIStatusModel(
                                    self.device_name,
                                    icron_parsed_file.get_istatus_json(self.device_name))

            self.istatus_decoder = iis.IStatusDecoder(self.istatus_model)

            self.icmd_model = im.IcronIcmdModel(
                                self.device_name, 
                                icron_parsed_file.get_icomponent_json(self.device_name),
                                icron_parsed_file.get_icmd_json(self.device_name))

            self.icmd_encoder = icd.IcmdEncoder(self.icmd_model)

            ichannel_model = im.IcronChannelIdModel(
                                    self.device_name,
                                    icron_parsed_file.get_ichannel_id_json(self.device_name))

            self.ilog_channel_id = ichannel_model.ilog_channel_id

            self.istatus_channel_id = ichannel_model.istatus_channel_id

            self.printf_channel_id = ichannel_model.printf_channel_id

            self.icmd_channel_id = ichannel_model.icmd_channel_id

            # load flash writer image
            self.flash_writer_image = icron_parsed_file.get_flash_writer_image(self.device_name)

            # load main firmware image
            self.main_firmware_image = icron_parsed_file.get_main_firmware_image(self.device_name)

            # load lex FPGA image
            self.lex_fpga_image = icron_parsed_file.get_lex_fpga_image(self.device_name)

            # load rex FPGA image
            self.rex_fpga_image = icron_parsed_file.get_rex_fpga_image(self.device_name)

            self.lex_build_time = icron_parsed_file.get_lex_fpga_build_time(self.device_name)
            self.rex_build_time = icron_parsed_file.get_rex_fpga_build_time(self.device_name)

            self.iregister_model = im.IcronRegisterModel(
                                        self.device_name, 
                                        icron_parsed_file.get_iregister_settings(self.device_name))

            self.program_command_channel_id = ichannel_model.program_command_channel_id
            self.device_info_channel_id = ichannel_model.program_status_channel_id
            self.program_data_channel_id = ichannel_model.program_data_channel_id

            self.log_file_decoder.register_channel_packet_attributes(
                                                                self.ilog_channel_id,
                                                                self.short_device_name,
                                                                ilg.parse_ilog,
                                                                self.ilog_decoder.decode)

            self.log_file_decoder.register_channel_packet_attributes(
                                                                self.istatus_channel_id,
                                                                self.short_device_name,
                                                                iis.parse_istatus,
                                                                self.istatus_decoder.decode)

            self.log_file_decoder.register_channel_packet_attributes(
                                                                self.printf_channel_id,
                                                                self.short_device_name,
                                                                None,
                                                                None)
            self.log_file_decoder.register_channel_packet_attributes(
                                                        self.device_info_channel_id,
                                                        self.short_device_name,
                                                        None,
                                                        idi.decode_info_message)

            message = "{}: {}: Icron model is loaded {}". \
                        format(self.port_name, self.device_name, self.original_icron_file_path)
            cbs_logger.info(message)

            return True

        except:
            cbs_logger.exception("{}: {}: Got an error when loading icron model". \
                    format(self.port_name, self.device_name))
            return False

    def icronFileChanged(self, sender, args):
        """
        Notifies the user that the .icron file should be reloaded.

        Arguments: sender - unused
                   args - unused
        """
        self.devWindow.ReloadIcronButton.ForeColor = Drawing.Color.Red

    def update_text(self, newText):
        """
        Update the new text to the tile of device window.

        Arguments: newText - new text to be updated
        """
        self.devWindow.Text = newText

    @property
    def device_name(self):
        return self.device_name

    @property
    def port_name(self):
        return self.serial_port_manager.port_name

    @property
    def port_recorder(self):
        return self.serial_port_manager.log_recorder

    @property
    def icron_file_path(self):
        self.icron_file_path

    @icron_file_path.setter
    def icron_file_path(self, new_path):
        self.icron_file_path = new_path

# one should not be able to import DeviceWindow without importing Device
# hence, the placement of the class in here and not DeviceWindow.py
class DeviceWindow(Forms.Form):

    # UI Initialization
    def __init__(self, dev):
        """Creates a DeviceWindow object.

        Argument: dev - Device the DeviceWindow should be connected to"""

        # Eliminates flickering when appending/deleting text quickly from Form
        # According to msdn documentation, "Buffered graphics can reduce or eliminate flicker
        # that is caused by progressive redrawing of parts of a displayed surface."
        self.DoubleBuffered = True

        # make a shallow copy of the Device so it can be accessed from here
        self.dev = dev

        # Dark Mode status
        self.darkModeStatus = False

        # True if iLogs should be suspended, False if not
        self.suspend = False

        self.auxAnalyserPath = ""

        self.clearFlashButtonClicked = False
        # everything from here to the end of the function is quite boring
        # just to let you know

        # create controls
        self.InputToolStrip = Forms.MenuStrip()
        self.InputToolStrip_1 = Forms.MenuStrip()
        self.MainSplitContainer = Forms.SplitContainer()
        self.OutputToolStrip = Forms.MenuStrip()
        self.DpOutputToolStrip = Forms.MenuStrip()
        self.DpOutputToolStrip_1 = Forms.MenuStrip()
        self.DpOutputToolStrip_2 = Forms.MenuStrip()
        self.autoReset = Forms.CheckBox()
        self.autoResetCheckBox = Forms.ToolStripControlHost(self.autoReset)
        self.autoProgram = Forms.CheckBox()
        self.AutoClearOutput = Forms.CheckBox()
        self.AutoClearOutputCheckBox =  Forms.ToolStripControlHost(self.AutoClearOutput)
        self.suspchk = Forms.CheckBox()
        self.SuspendOutputCheckBox = Forms.ToolStripControlHost(self.suspchk)
        self.enableMccs = Forms.CheckBox()
        self.enableMccsCheckbox = Forms.ToolStripControlHost(self.enableMccs)
        self.enableSsc = Forms.CheckBox()
        self.enableSscCheckbox = Forms.ToolStripControlHost(self.enableSsc)
        self.enableIsolate = Forms.CheckBox()
        self.enableIsolateCheckbox = Forms.ToolStripControlHost(self.enableIsolate)
        self.enableYcbcr = Forms.CheckBox()
        self.enableYcbcrCheckbox = Forms.ToolStripControlHost(self.enableYcbcr)
        self.enableAudio = Forms.CheckBox()
        self.enableAudioCheckbox = Forms.ToolStripControlHost(self.enableAudio)
        self.GoButton = Forms.ToolStripMenuItem()
        self.ReadDPCD = Forms.ToolStripMenuItem()
        self.enableAuxAnalyser = Forms.CheckBox()
        self.enableAuxAnalyserCheckbox = Forms.ToolStripControlHost(self.enableAuxAnalyser)
        self.InterpretDPCD = Forms.ToolStripMenuItem()
        self.iCommandsButton = Forms.ToolStripMenuItem()
        self.iRegisterButton = Forms.ToolStripMenuItem()
        self.SWversionButton = Forms.ToolStripMenuItem()
        self.ReloadIcronButton = Forms.ToolStripMenuItem()
        self.ClearOutputButton = Forms.ToolStripMenuItem()
        # self.logLabel = Forms.ToolStripLabel()
        # self.LogDropDownMenu = Forms.ToolStripComboBox()
        self.ExportLogButton = Forms.ToolStripMenuItem()
        self.searchTextBox = Forms.ToolStripTextBox()
        self.searchResultLabel = Forms.ToolStripLabel()
        self.ProgramFwButton = Forms.ToolStripMenuItem()
        self.ProgramFpgaButton = Forms.ToolStripMenuItem()
        self.ResetDeviceButton = Forms.ToolStripMenuItem()
        self.OutputTextBox = Forms.RichTextBox()
        self.InputTextBox = Forms.RichTextBox()
        self.InputFileDialog = Forms.OpenFileDialog()
        self.customEdidLabel = Forms.ToolStripLabel()
        self.customEdidComBox = Forms.ToolStripComboBox()
        self.chooseBpcLabel = Forms.ToolStripLabel()
        self.chooseBpcComBox = Forms.ToolStripComboBox()
        self.chooseBandwidthLabel = Forms.ToolStripLabel()
        self.chooseBandwidthComBox = Forms.ToolStripComboBox()
        self.chooseLCLabel = Forms.ToolStripLabel()
        self.chooseLCComBox = Forms.ToolStripComboBox()
        self.auxAnalyserLabel = Forms.ToolStripLabel()
        self.auxAnalyserComBox = Forms.ToolStripComboBox()
        self.getAuxIndex = Forms.ToolStripMenuItem()
        self.printAuxLog = Forms.ToolStripMenuItem()
        self.testButton = Forms.ToolStripMenuItem()
        self.rexTuButton = Forms.ToolStripMenuItem()
        self.addCommentButton = Forms.ToolStripMenuItem()
        self.icronLogFileDialog = Forms.OpenFileDialog()
        self.InputDropDownMenu = Forms.ToolStripComboBox()
        # self.DpLabel = Forms.ToolStripLabel()
        # self.DpDropDownMenu = Forms.ToolStripComboBox()
        self.DpDebugButton = Forms.ToolStripMenuItem()
        self.DarkModeButton = Forms.ToolStripMenuItem()
        self.DpSdpStatsButton = Forms.ToolStripMenuItem()
        self.TestAssertButton = Forms.ToolStripMenuItem()
        self.dumpDpFlashVarButton = Forms.ToolStripMenuItem()
        self.clearFlashVarButton = Forms.ToolStripMenuItem()
        self.auxTransaction = Forms.ToolStripMenuItem()
        self.auxFolderBrowserDialog = Forms.FolderBrowserDialog()
        self.edidPrint = Forms.ToolStripMenuItem()
        self.printTiming = Forms.ToolStripMenuItem()
        self.readCap = Forms.ToolStripMenuItem()
        # self.enableAudio = Forms.ToolStripMenuItem()
        # self.disableSdp = Forms.ToolStripMenuItem()
        self.dpAllLogs = Forms.ToolStripMenuItem()
        # self.sixBpc = Forms.ToolStripMenuItem()
        # self.eightBpc = Forms.ToolStripMenuItem()
        self.Res4k = Forms.ToolStripMenuItem()
        self.Res1080p = Forms.ToolStripMenuItem()
        # self.enableIsolate = Forms.ToolStripMenuItem()
        # self.disableIsolate = Forms.ToolStripMenuItem()
        self.iLogFilterButton = Forms.ToolStripMenuItem()
        # MainSplitContainer
        self.MainSplitContainer.Dock = Forms.DockStyle.Fill
        self.MainSplitContainer.Orientation = Forms.Orientation.Horizontal
        self.MainSplitContainer.Size = Drawing.Size(950, 491)
        self.MainSplitContainer.SplitterDistance = 450
        self.MainSplitContainer.SplitterWidth = 10
        self.MainSplitContainer.TabStop = False
        self.MainSplitContainer.ForeColor = Drawing.Color.FromName("Black")

        # MainSplitContainer.Panel1
        self.MainSplitContainer.Panel1.Controls.Add(self.OutputTextBox)
        self.MainSplitContainer.Panel1.Controls.Add(self.OutputToolStrip)
        self.MainSplitContainer.Panel1.Controls.Add(self.DpOutputToolStrip_2)
        self.MainSplitContainer.Panel1.Controls.Add(self.DpOutputToolStrip_1)
        self.MainSplitContainer.Panel1.Controls.Add(self.DpOutputToolStrip)
        self.MainSplitContainer.Panel1.Controls.Add(self.InputToolStrip_1)

        # MainSplitContainer.Panel2
        self.MainSplitContainer.Panel2.Controls.Add(self.InputTextBox)
        self.MainSplitContainer.Panel2.Controls.Add(self.InputToolStrip)
        

        # InputToolStrip

        self.InputToolStrip.Dock = Forms.DockStyle.Bottom
        self.InputToolStrip_1.Dock = Forms.DockStyle.Bottom
        self.InputToolStrip.GripStyle = Forms.ToolStripGripStyle.Hidden
        self.InputToolStrip_1.GripStyle = Forms.ToolStripGripStyle.Hidden
        if self.dev.device_name == 'blackbird':
            self.InputToolStrip.Items.Add(self.autoResetCheckBox)
            self.InputToolStrip.Items.Add(Forms.ToolStripSeparator())
            self.InputToolStrip.Items.Add(self.ProgramFwButton)
            self.InputToolStrip.Items.Add(Forms.ToolStripSeparator())
            self.InputToolStrip.Items.Add(self.ProgramFpgaButton)
        self.InputToolStrip.Items.Add(Forms.ToolStripSeparator())
        self.InputToolStrip.Items.Add(self.InputDropDownMenu)
        self.InputToolStrip.Items.Add(Forms.ToolStripSeparator())
        self.InputToolStrip.Items.Add(self.iCommandsButton)
        self.InputToolStrip.Items.Add(self.iRegisterButton)
        self.InputToolStrip.Items.Add(self.SWversionButton)
        self.InputToolStrip.Items.Add(Forms.ToolStripSeparator())

        self.InputToolStrip_1.Items.Add(self.SuspendOutputCheckBox)
        self.InputToolStrip_1.Items.Add(Forms.ToolStripSeparator())
        self.InputToolStrip_1.Items.Add(self.AutoClearOutputCheckBox)
        self.InputToolStrip_1.Items.Add(self.ClearOutputButton)
        self.InputToolStrip_1.Items.Add(Forms.ToolStripSeparator())
        # self.InputToolStrip.Items.Add(self.logLabel)
        # self.InputToolStrip.Items.Add(self.LogDropDownMenu)
        self.InputToolStrip_1.Items.Add(Forms.ToolStripSeparator())
        self.InputToolStrip_1.Items.Add(self.iLogFilterButton)
        self.InputToolStrip_1.Items.Add(self.ExportLogButton)
        self.InputToolStrip_1.Items.Add(Forms.ToolStripSeparator())
        self.InputToolStrip_1.Items.Add(self.addCommentButton)
        self.InputToolStrip_1.Items.Add(Forms.ToolStripSeparator())
        self.InputToolStrip_1.Items.Add(self.testButton)
        self.InputToolStrip_1.Items.Add(Forms.ToolStripSeparator())
        self.InputToolStrip_1.Items.Add(self.rexTuButton)
        self.InputToolStrip_1.Items.Add(Forms.ToolStripSeparator())

        # OutputToolStrip
        self.OutputToolStrip.Dock = Forms.DockStyle.Bottom
        self.OutputToolStrip.GripStyle = Forms.ToolStripGripStyle.Hidden
        #TODO: addd the button attributes in json file to avoid hardcoded code
        if self.dev.device_name == 'blackbird':
            self.DpOutputToolStrip.Items.Add(self.DpDebugButton)        
            self.DpOutputToolStrip.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip.Items.Add(self.DpSdpStatsButton)        
            self.DpOutputToolStrip.Items.Add(Forms.ToolStripSeparator())

            self.DpOutputToolStrip.Items.Add(self.edidPrint)
            self.DpOutputToolStrip.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip.Items.Add(self.printTiming)
            self.DpOutputToolStrip.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip.Items.Add(self.ReadDPCD)
            self.DpOutputToolStrip.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip.Items.Add(self.InterpretDPCD)
            self.DpOutputToolStrip.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip.Items.Add(self.GoButton)
            self.DpOutputToolStrip.Items.Add(Forms.ToolStripSeparator())
            # self.DpOutputToolStrip.Items.Add(self.dpAllLogs)
            # self.DpOutputToolStrip.Items.Add(Forms.ToolStripSeparator())
            # self.DpOutputToolStrip.Items.Add(self.readCap)
            # self.DpOutputToolStrip.Items.Add(Forms.ToolStripSeparator())
            # self.DpOutputToolStrip.Items.Add(self.enableAudio)
            # self.DpOutputToolStrip.Items.Add(Forms.ToolStripSeparator())
            # self.DpOutputToolStrip.Items.Add(self.disableSdp)
            # self.DpOutputToolStrip.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip_1.Items.Add(self.customEdidLabel)
            self.DpOutputToolStrip_1.Items.Add(self.customEdidComBox)
            self.DpOutputToolStrip_1.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip_1.Items.Add(self.chooseBpcLabel)
            self.DpOutputToolStrip_1.Items.Add(self.chooseBpcComBox)
            self.DpOutputToolStrip_1.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip_1.Items.Add(self.chooseBandwidthLabel)
            self.DpOutputToolStrip_1.Items.Add(self.chooseBandwidthComBox)
            self.DpOutputToolStrip_1.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip_1.Items.Add(self.chooseLCLabel)
            self.DpOutputToolStrip_1.Items.Add(self.chooseLCComBox)
            self.DpOutputToolStrip_1.Items.Add(self.auxAnalyserLabel)
            self.DpOutputToolStrip_1.Items.Add(self.auxAnalyserComBox)
            self.DpOutputToolStrip_1.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip_1.Items.Add(self.auxTransaction)
            self.DpOutputToolStrip_1.Items.Add(Forms.ToolStripSeparator())


            # self.DpOutputToolStrip_1.Items.Add(self.getAuxIndex)
            # self.DpOutputToolStrip_1.Items.Add(Forms.ToolStripSeparator())
            # self.DpOutputToolStrip_1.Items.Add(self.printAuxLog)
            # self.DpOutputToolStrip_1.Items.Add(Forms.ToolStripSeparator())
            # self.DpOutputToolStrip_2.Items.Add(self.sixBpc)
            # self.DpOutputToolStrip_2.Items.Add(Forms.ToolStripSeparator())
            # self.DpOutputToolStrip_2.Items.Add(self.eightBpc)
            # self.DpOutputToolStrip_2.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip_2.Items.Add(self.enableMccsCheckbox)
            self.DpOutputToolStrip_2.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip_2.Items.Add(self.enableSscCheckbox)
            self.DpOutputToolStrip_2.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip_2.Items.Add(self.enableIsolateCheckbox)
            self.DpOutputToolStrip_2.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip_2.Items.Add(self.enableAuxAnalyserCheckbox)
            self.DpOutputToolStrip_2.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip_2.Items.Add(self.enableYcbcrCheckbox)
            self.DpOutputToolStrip_2.Items.Add(Forms.ToolStripSeparator())
            self.DpOutputToolStrip_2.Items.Add(self.enableAudioCheckbox)
            self.DpOutputToolStrip_2.Items.Add(Forms.ToolStripSeparator())

            # self.DpOutputToolStrip_2.Items.Add(self.Res4k)
            # self.DpOutputToolStrip_2.Items.Add(Forms.ToolStripSeparator())
            # self.DpOutputToolStrip_2.Items.Add(self.Res1080p)
            # self.DpOutputToolStrip_2.Items.Add(Forms.ToolStripSeparator())
            # self.DpOutputToolStrip_2.Items.Add(self.enableIsolate)
            # self.DpOutputToolStrip_2.Items.Add(Forms.ToolStripSeparator())
            # self.DpOutputToolStrip_2.Items.Add(self.disableIsolate)
            # self.DpOutputToolStrip_2.Items.Add(Forms.ToolStripSeparator())
            self.OutputToolStrip.Items.Add(self.ResetDeviceButton)
            self.OutputToolStrip.Items.Add(Forms.ToolStripSeparator())
            # self.OutputToolStrip.Items.Add(self.customEdidLabel)
            # self.OutputToolStrip.Items.Add(self.customEdidComBox)
            # self.OutputToolStrip.Items.Add(self.chooseBpcLabel)
            # self.OutputToolStrip.Items.Add(self.chooseBpcComBox)
            self.InputToolStrip.Items.Add(self.ReloadIcronButton)
        self.OutputToolStrip.Items.Add(Forms.ToolStripSeparator())
        self.OutputToolStrip.Items.Add(Forms.ToolStripSeparator())
        self.OutputToolStrip.Items.Add(self.searchTextBox)
        self.OutputToolStrip.Items.Add(self.searchResultLabel)
        self.OutputToolStrip.Items.Add(Forms.ToolStripSeparator())
        self.OutputToolStrip.Items.Add(self.clearFlashVarButton)
        self.OutputToolStrip.Items.Add(Forms.ToolStripSeparator())
        self.OutputToolStrip.Items.Add(self.dumpDpFlashVarButton)
        self.OutputToolStrip.Items.Add(Forms.ToolStripSeparator())
        self.OutputToolStrip.Items.Add(self.TestAssertButton)
        self.OutputToolStrip.Items.Add(Forms.ToolStripSeparator())

        self.OutputToolStrip.Items.Add(Forms.ToolStripSeparator())
        self.OutputToolStrip.Items.Add(self.DarkModeButton)
        self.OutputToolStrip.Items.Add(Forms.ToolStripSeparator())

        # GoButton
        self.GoButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.GoButton.Text = "  Go  "
        self.GoButton.Click += self.GoButton_Click

        # getAuxIndex
        self.getAuxIndex.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.getAuxIndex.Text = "Print Aux Size"
        self.getAuxIndex.Click += self.getAuxIndex_Click

        # printAuxLog
        self.printAuxLog.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.printAuxLog.Text = "Print Aux Logs"
        self.printAuxLog.Click += self.printAuxButton_Click

        # ReadDPCD
        self.ReadDPCD.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.ReadDPCD.Text = "Read DPCD"
        self.ReadDPCD.Click += self.ReadDPCD_Click

        # InterpretDPCD
        self.InterpretDPCD.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.InterpretDPCD.Text = "Interpret DPCD"
        self.InterpretDPCD.Click += self.InterpretDPCD_Click
        self.InterpretDPCD.Enabled = False

        # iCommandsButton
        self.iCommandsButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.iCommandsButton.Text = "iCo&mmands"
        self.iCommandsButton.Click += self.iCommandsButton_Click

        #iRegisterButton
        self.iRegisterButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.iRegisterButton.Text = "&iRegister"
        self.iRegisterButton.Click += self.iRegisterButton_Click

        # SWversionButton
        self.SWversionButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.SWversionButton.Text = "SWversion"
        self.SWversionButton.Click += self.swVersionButton_Click

        # icronLogFileDialog
        self.icronLogFileDialog.Filter = ".gz files (*.gz)|*.gz|" + "All files (*.*)|*.*"

        # ProgramButton
        self.ProgramFwButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.ProgramFwButton.Text = "&Program"
        self.ProgramFwButton.Click += self.ProgramFwButton_Click

        # ProgramFPGAButton
        self.ProgramFpgaButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.ProgramFpgaButton.Text = "Program FPGA"
        self.ProgramFpgaButton.Click += self.ProgramFpgaButton_Click

        # ResetDeviceButton
        self.ResetDeviceButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.ResetDeviceButton.Text = "&Reset"
        self.ResetDeviceButton.Click += self.ResetDeviceButton_Click

        # SuspendOutputCheckBox
        self.suspchk.CheckedChanged += self.suspchk_CheckedChanged
        self.SuspendOutputCheckBox.BackColor = Drawing.Color.Transparent
        self.SuspendOutputCheckBox.Text = "Sus&pend"

        # EnableMccsCheckBox
        self.enableMccs.Checked = True
        self.enableMccs.CheckedChanged += self.enableMccs_CheckedChanged
        self.enableMccsCheckbox.BackColor = Drawing.Color.Transparent
        self.enableMccsCheckbox.Text = "Enable MCCS"

        # EnableSscCheckBox
        self.enableSsc.Checked = True
        self.enableSsc.CheckedChanged += self.enableSsc_CheckedChanged
        self.enableSscCheckbox.BackColor = Drawing.Color.Transparent
        self.enableSscCheckbox.Text = "Enable SSC"

        # EnableIsolateCheckBox
        self.enableIsolate.Checked = False
        self.enableIsolate.CheckedChanged += self.enableIsolate_CheckedChanged
        self.enableIsolateCheckbox.BackColor = Drawing.Color.Transparent
        self.enableIsolateCheckbox.Text = "Enable Isolate"

        # EnableYcbcrCheckBox
        self.enableYcbcr.Checked = False
        self.enableYcbcr.CheckedChanged += self.enableYcbcr_CheckedChanged
        self.enableYcbcrCheckbox.BackColor = Drawing.Color.Transparent
        self.enableYcbcrCheckbox.Text = "Enable YCbCr"

        # enableAuxAnalyserCheckbox
        self.enableAuxAnalyser.Checked = False
        self.enableAuxAnalyser.CheckedChanged += self.enableAux_CheckedChanged
        self.enableAuxAnalyserCheckbox.BackColor = Drawing.Color.Transparent
        self.enableAuxAnalyserCheckbox.Text = "Aux Analyser"

        # enableAudioCheckbox
        self.enableAudio.Checked = True
        self.enableAudio.CheckedChanged += self.enableAudio_CheckedChanged
        self.enableAudioCheckbox.BackColor = Drawing.Color.Transparent
        self.enableAudioCheckbox.Text = "Enable Audio"

        # AutoResetFPGACheckBox
        self.autoReset.Checked = True
        self.autoReset.CheckedChanged += self.autoReset_CheckedChanged
        self.autoResetCheckBox.BackColor = Drawing.Color.Transparent
        self.autoResetCheckBox.Text = "AutoReset"

        # AutoClearOutputCheckBox
        self.AutoClearOutput.Checked = False
        self.AutoClearOutput.CheckedChanged += self.autoClearOutput_CheckedChanged
        self.AutoClearOutputCheckBox.BackColor = Drawing.Color.Transparent
        self.AutoClearOutputCheckBox.Text = ""

        # ClearOutputButton
        self.ClearOutputButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.ClearOutputButton.Text = "Clear &Output"
        self.ClearOutputButton.Click += self.ClearOutputButton_Click

        # logLabel
        # self.logLabel.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        # self.logLabel.Text = "Logs:"
        
        # #Logdropdown menu
        # self.LogDropDownMenu.Size = System.Drawing.Size(20, 4)
        # self.LogDropDownMenu.DropDownStyle = Forms.ComboBoxStyle.DropDownList;
        # self.LogDropDownMenu.BackColor = Drawing.Color.Linen
        # ProgramOptions = ["New Log", "Export Log", "Open Log", "LogFilter" ]
        # self.LogDropDownMenu.Items.AddRange(Array[str](ProgramOptions))     
        # self.LogDropDownMenu.SelectedIndexChanged += self.LogDropDownMenuChanged

        #iLogFilter
        self.iLogFilterButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.iLogFilterButton.Text = "&LogFilter"
        self.iLogFilterButton.Click += self.iLogFilterChange

        # ExportLogButton
        self.ExportLogButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.ExportLogButton.Text = "&Export Log"
        self.ExportLogButton.Click += self.ExportLogButton_Click

        #AddCommentbutton
        self.addCommentButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.addCommentButton.Text = "Comment"
        self.addCommentButton.Click += self.addComment_Click

        #Dp buttons
        self.testButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.testButton.Text = "Test"
        self.testButton.Click += self.test_Click

        #Dp buttons
        self.rexTuButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.rexTuButton.Text = "RexTU"
        self.rexTuButton.Click += self.rex_tu_Click

        #seachTextBox
        self.searchTextBox.Text = ""
        self.searchTextBox.Size = Drawing.Size(80, 5)
        self.searchTextBox.BorderStyle = Forms.BorderStyle.Fixed3D;
        self.searchTextBox.KeyDown += self.searchText_Enter

        #searchResultLabel
        self.searchResultLabel.Text = "0 found"

        #customEdidLabel
        self.customEdidLabel.Text = "EDID"

        #customEdidComBox
        self.customEdidComBox.Size = System.Drawing.Size(20, 5)
        self.customEdidComBox.DropDownStyle = Forms.ComboBoxStyle.DropDownList;
        edidList = ["Default", "4K", "1080p", "640x480"]
        self.customEdidComBox.Items.AddRange(Array[str](edidList))
        self.customEdidComBox.SelectedIndex = 0
        self.customEdidComBox.DropDownClosed += self.edidChanged

        # chooseBpcLabel
        self.chooseBpcLabel.Text = "BPC"

        # chooseBpcComBox
        self.chooseBpcComBox.Size = System.Drawing.Size(20, 5)
        self.chooseBpcComBox.DropDownStyle = Forms.ComboBoxStyle.DropDownList;
        bpcList = ["Default", "6", "8", "10"]
        self.chooseBpcComBox.Items.AddRange(Array[str](bpcList))
        self.chooseBpcComBox.SelectedIndex = 0
        self.chooseBpcComBox.DropDownClosed += self.BpcChanged

        # chooseBandwidthLabel
        self.chooseBandwidthLabel.Text = "BW"

        # chooseBandwidthComBox
        self.chooseBandwidthComBox.Size = System.Drawing.Size(20, 5)
        self.chooseBandwidthComBox.DropDownStyle = Forms.ComboBoxStyle.DropDownList;
        bwList = ["Default", "5.4 Gbits/s", "2.7 Gbits/s", "1.62 Gbits/s"]
        self.chooseBandwidthComBox.Items.AddRange(Array[str](bwList))
        self.chooseBandwidthComBox.SelectedIndex = 0
        self.chooseBandwidthComBox.DropDownClosed += self.bandwidthChanged

        # auxAnalyserLabel
        self.auxAnalyserLabel.Text = "Aux"

        # auxAnalyserComBox
        self.auxAnalyserComBox.Size = System.Drawing.Size(40, 5)
        self.auxAnalyserComBox.DropDownStyle = Forms.ComboBoxStyle.DropDownList;
        auxList = ["Default", "Clear Aux", "Print Aux Size", "Print Aux Log"]
        self.auxAnalyserComBox.Items.AddRange(Array[str](auxList))
        self.auxAnalyserComBox.SelectedIndex = 0
        self.auxAnalyserComBox.DropDownClosed += self.auxMenuChanged

        # chooseBandwidthLabel
        self.chooseLCLabel.Text = "LC"

        # chooseLCComBox
        self.chooseLCComBox.Size = System.Drawing.Size(20, 5)
        self.chooseLCComBox.DropDownStyle = Forms.ComboBoxStyle.DropDownList;
        lcList = ["Default", "4", "2", "1"]
        self.chooseLCComBox.Items.AddRange(Array[str](lcList))
        self.chooseLCComBox.SelectedIndex = 0
        self.chooseLCComBox.DropDownClosed += self.lcChanged
        
        # InputDropDownMenu
        self.InputDropDownMenu.Size = System.Drawing.Size(16, 4)
        self.InputDropDownMenu.DropDownStyle = Forms.ComboBoxStyle.DropDownList;
        self.InputDropDownMenu.BackColor = Drawing.Color.Linen
        InputOptions = ["Send", "Clear", "Save...", "Load..."]
        self.InputDropDownMenu.Items.AddRange(Array[str](InputOptions))     
        self.InputDropDownMenu.DropDownClosed += self.InputDropDownMenuChanged        

        # DpDebugButton
        self.DpDebugButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.DpDebugButton.Text = "DP Debug"
        self.DpDebugButton.Click += self.dpDebugButton_Click

        # TestAssertButton
        self.TestAssertButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.TestAssertButton.Text = "Assert"
        self.TestAssertButton.Click += self.testAssertButton_Click

        # DpSdpStatsButton
        self.DpSdpStatsButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.DpSdpStatsButton.Text = "SDP Stats"
        self.DpSdpStatsButton.Click += self.dpSdpStatsButton_Click

        #dumpDpFlashVarButton
        self.dumpDpFlashVarButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.dumpDpFlashVarButton.Text = "DP Flash Var"
        self.dumpDpFlashVarButton.Click += self.dumpDpFlashVarButton_Click

        # clearFlashVarButton
        self.clearFlashVarButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.clearFlashVarButton.Text = "Clear DP Flash Vars"
        self.clearFlashVarButton.Click += self.clearFlashVarButton_Click

        # auxTransaction
        self.auxTransaction.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.auxTransaction.Text = "AUX Trans"
        self.auxTransaction.Click += self.auxTransaction_click

        #edidPrint
        self.edidPrint.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.edidPrint.Text = "EDID"
        self.edidPrint.Click += self.edidPrint_click

        # timingPrint
        self.printTiming.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.printTiming.Text = "Print Timing"
        self.printTiming.Click += self.getTiming_Click
        self.printTiming.Enabled = False

        # readCap
        self.readCap.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.readCap.Text = "Read CAP"
        self.readCap.Click += self.readCap_click

        # # enableAudio
        # self.enableAudio.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        # self.enableAudio.Text = "Enable Audio"
        # self.enableAudio.Click += self.enableAudio_click
        #
        # # disableSdp
        # self.disableSdp.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        # self.disableSdp.Text = "Disable SDP"
        # self.disableSdp.Click += self.disableSdp_click

        # dpAllLogs
        self.dpAllLogs.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.dpAllLogs.Text = "DP All Logs"
        self.dpAllLogs.Click += self.dpAllLogs_click

        # sixBpc
        # self.sixBpc.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        # self.sixBpc.Text = "BPC 6"
        # self.sixBpc.Click += self.sixBpc_click

        # eightBpc
        # self.eightBpc.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        # self.eightBpc.Text = "BPC 8"
        # self.eightBpc.Click += self.eightBpc_click

        # Res4k
        # self.Res4k.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        # self.Res4k.Text = "4K"
        # self.Res4k.Click += self.Res4k_click

        # Res1080p
        # self.Res1080p.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        # self.Res1080p.Text = "1080p"
        # self.Res1080p.Click += self.Res1080p_click

        # enableIsolate
        # self.enableIsolate.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        # self.enableIsolate.Text = "Enable Isolate"
        # self.enableIsolate.Click += self.enableIsolate_click
        #
        # # disableIsolate
        # self.disableIsolate.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        # self.disableIsolate.Text = "Disable Isolate"
        # self.disableIsolate.Click += self.disableIsolate_click

        #DarkModeButton
        self.DarkModeButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.DarkModeButton.Text = "Dark Side"
        self.DarkModeButton.Click += self.darkModeButton_Click

        # OutputTextBox
        self.OutputTextBox.Dock = Forms.DockStyle.Fill
        self.OutputTextBox.Font = Drawing.Font("Consolas", 11)
        self.OutputTextBox.Multiline = True
        self.OutputTextBox.ReadOnly = True
        if self.dev.device_name == 'blackbird':
            self.OutputTextBox.BackColor = Drawing.Color.White
        else:
            self.OutputTextBox.BackColor = Drawing.Color.Ivory
        # InputTextBox
        self.InputTextBox.Dock = Forms.DockStyle.Fill
        self.InputTextBox.Font = Drawing.Font("Consolas", 11)
        self.InputTextBox.Multiline = True
        self.InputTextBox.AcceptsTab = True

        # InputFileDialog
        self.InputFileDialog.Filter = "Python scripts (*.py)|*.py|" + \
                                      "All files (*.*)|*.*"
        self.InputFileDialog.Title = "Open a source code file"
        self.InputFileDialog.InitialDirectory = self.dev.programPath + "\\Scripts"

        # DeviceWindow
        self.ClientSize = Drawing.Size(950, 491)
        self.Controls.Add(self.MainSplitContainer)

        self.Icon = Drawing.Icon(self.dev.programPath + "\cobsicon.ico")
        self.Text = self.dev.serial_port_manager.port_name + " Device - Cobs"

        self.Closed += self.DeviceWindow_Closed

        self.firstTime = True

        def i_cut(sender, args):
            self.InputTextBox.Cut()

        def i_copy(sender, args):
            Clipboard.SetData(Forms.DataFormats.Rtf, self.InputTextBox.SelectedRtf)

        def i_paste(sender, args):
            if Clipboard.ContainsText(Forms.TextDataFormat.Rtf):
                self.InputTextBox.SelectedRtf = Clipboard.GetData(Forms.DataFormats.Rtf).ToString()

        def i_select_all(sender, args):
            self.InputTextBox.SelectAll()

        # do we really need cut for the output textbox?
        # we can't delete text - just use copy
        #def o_cut(sender, args):
        #    self.OutputTextBox.Cut()

        def o_copy(sender, args):
            Clipboard.SetData(Forms.DataFormats.Rtf, self.OutputTextBox.SelectedRtf)

        # we shouldn't need a paste function
        # one can use [dev port]_OUT() to accomplish writing to the textbox
        #def o_paste(sender, args):
        #    if Clipboard.ContainsText(Forms.TextDataFormat.Rtf):
        #        self.OutputTextBox.SelectedRtf = Clipboard.GetData(
        #                                         Forms.DataFormats.Rtf).ToString()

        def o_select_all(sender, args):
            self.OutputTextBox.SelectAll()

        self.InputTextBox.ContextMenu = Forms.ContextMenu()
        self.InputTextBox.ContextMenu.MenuItems.Add(Forms.MenuItem("Cut", i_cut))
        self.InputTextBox.ContextMenu.MenuItems.Add(Forms.MenuItem("Copy", i_copy))
        self.InputTextBox.ContextMenu.MenuItems.Add(Forms.MenuItem("Paste", i_paste))
        self.InputTextBox.ContextMenu.MenuItems.Add(Forms.MenuItem("Select All", i_select_all))

        self.OutputTextBox.ContextMenu = Forms.ContextMenu()
        #self.OutputTextBox.ContextMenu.MenuItems.Add(MenuItem("Cut", o_cut))
        self.OutputTextBox.ContextMenu.MenuItems.Add(Forms.MenuItem("Copy", o_copy))
        #self.OutputTextBox.ContextMenu.MenuItems.Add(MenuItem("Paste", o_paste))
        self.OutputTextBox.ContextMenu.MenuItems.Add(Forms.MenuItem("Select All", o_select_all))

    def print_to_text_box(self, string, color=Drawing.Color.Black, timestamp=True, newline=True):
        """
        Prints a message on the Output textbox.

        Arguments: output - string to be printed
                   color - color of the string to printed
        """
        try:
            #self.AutoClearOutputText()
            text_box = self.OutputTextBox
            text_box.SelectionStart = text_box.TextLength
            text_box.SelectionLength = 0
            text_box.SelectionColor = color
            if timestamp:
                string = \
                    datetime.datetime.now().strftime('%d %H:%M:%S.%f')[2:-3] + ': ' + string
            if newline:
                string = string + "\r\n"
            text_box.AppendText(string)
            text_box.SelectionColor = text_box.ForeColor
            self.AutoClearOutputText()
            text_box.ScrollToCaret()
            
			
        except MemoryError:
            self.OutputTextBox.Clear()
            cbs_logger.exception("{}: {}: Got a MemoryError when outputting text". \
                    format(self.dev.port_name, self.dev.device_name))
        except:
            cbs_logger.exception("{}: {}: Got an error when outputting text". \
                    format(self.dev.port_name, self.dev.device_name))

    def suspchk_CheckedChanged(self, sender, args):
        """Enables or disables suspension of iLogs.

        Arguments: sender - unused
                   args - unused"""
        def handle_suspchk():
            try:
                # modifies iLog suspension based on whether suspchk is checked or not
                self.suspend = self.suspchk.Checked
                # if not self.suspend:
                #     while not self.dev.log_queue.empty():
                        # block when get an log message from the queue
                        # log = self.dev.log_queue.get(True)
                        # delegate = CallTarget0(
                        #             lambda: self.print_to_text_box(
                        #                                         str(log),
                        #                                         log.color,
                        #                                         not log.timestamped,
                        #                                         not log.newline))
                        # self.MainSplitContainer.Invoke(delegate)
            except:
                cbs_logger.exception("{}: {}: Got an error when handling suspend check". \
                        format(self.dev.port_name, self.dev.device_name))
        try:
            cbs_logger.info("{}: {}: Suspend Checkbox is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            thread = Threading.Thread(Threading.ThreadStart(handle_suspchk))
            thread.SetApartmentState(Threading.ApartmentState.STA)
            thread.Start()
        except:
            cbs_logger.exception("{}: {}: Got an error when suspend is unchecked". \
                    format(self.dev.port_name, self.dev.device_name))


    def swVersionButton_Click(self, sender, args):
        """
        This function is executed when SWversion button is clicked. It creates and sends Software
        version icmd.

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: SWversion Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("TOPLEVEL_COMPONENT", "PrintSwVersion", False)
            swVersion_thread = Threading.Thread(
                                    Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "PrintSwVersion")
            swVersion_thread.Start(icmd_obj)
        except:
            error_msg = "{}: {}: Error occurred when clicking on SW Version". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_msg)

    def enableMccs_CheckedChanged(self, sender, args):
        """Enables or disables MCCS reading.

                Arguments: sender - unused
                           args - unused"""
        try:
            cbs_logger.info("{}: {}: Enable MCCS button is checked". \
                            format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_REX_MccsEnable", False, [int(self.enableMccs.Checked)])
            swVersion_thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "Enable MCCS")
            swVersion_thread.Start(icmd_obj)
        except Exception as e:
            cbs_logger.exception("{}: {}: Got an error when MCCS is enabled : {}". \
                                 format(self.dev.port_name, self.dev.device_name, e))

    def enableSsc_CheckedChanged(self, sender, args):
        """Enables or disables SSC. One checkbox for both REX and LEX.
           Based on the device type one will function and the other won't

                        Arguments: sender - unused
                                   args - unused"""
        try:
            cbs_logger.info("{}: {}: Enable SSC button is checked". \
                            format(self.dev.port_name, self.dev.device_name))
            # SSC Enable for LEX
            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_LEX_SscAdvertiseEnable", False, [int(self.enableSsc.Checked)])
            swVersion_thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "Enable SSC")
            swVersion_thread.Start(icmd_obj)

            #SSC Enable for REX
            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_REX_SscAdvertiseEnable", False, [int(self.enableSsc.Checked)])
            swVersion_thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "Enable SSC")
            swVersion_thread.Start(icmd_obj)
        except Exception as e:
            cbs_logger.exception("{}: {}: Got an error when SSC is enabled : {}". \
                                 format(self.dev.port_name, self.dev.device_name, e))

    def enableIsolate_CheckedChanged(self, sender, args):
        """
        Enables or disables Isolate State of Units

        Arguments: sender - unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: Enable Isolate button is checked". \
                            format(self.dev.port_name, self.dev.device_name))

            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_SetIsolateEnable", False, [self.enableIsolate.Checked])
            swVersion_thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "Enable Isolate")
            swVersion_thread.Start(icmd_obj)

        except Exception as e:
            cbs_logger.exception("{}: {}: Got an error when Isolate is enabled : {}". \
                                 format(self.dev.port_name, self.dev.device_name, e))

    def enableYcbcr_CheckedChanged(self, sender, args):
        """
        Enables or disables Isolate State of Units

        Arguments: sender - unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: Enable YCbCr button is checked". \
                            format(self.dev.port_name, self.dev.device_name))

            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_LEX_YCbCrDisableIcmd", False, [self.enableYcbcr.Checked])
            swVersion_thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "Enable YCbCr")
            swVersion_thread.Start(icmd_obj)

        except Exception as e:
            cbs_logger.exception("{}: {}: Got an error when Isolate is enabled : {}". \
                                 format(self.dev.port_name, self.dev.device_name, e))

    def enableAux_CheckedChanged(self, sender, args):
        """
        Enables or disables Aux Analyser in Cobs

        Arguments: sender - unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: Enable Aux Analyser button is checked". \
                            format(self.dev.port_name, self.dev.device_name))
            self.dev.change_auxEnable(self.enableAuxAnalyser.Checked)
            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_EnableAuxTraffic", False,
                                            [self.enableAuxAnalyser.Checked])
            swVersion_thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "Enable AUX traffic over UART")
            swVersion_thread.Start(icmd_obj)

        except Exception as e:
            cbs_logger.exception("{}: {}: Got an error when Aux Analyser is enabled : {}". \
                                 format(self.dev.port_name, self.dev.device_name, e))

    def enableAudio_CheckedChanged(self, sender, args):
        """
        Enables or disables Aux Analyser in Cobs

        Arguments: sender - unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: Enable Audio button is checked". \
                            format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_SetAudioState", False, [not self.enableAudio.Checked])

            enableAudio_thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.send_icmd))
            enableAudio_thread.Name = self.Text.replace("Cobs", "Enable Audio")
            enableAudio_thread.Start(icmd_obj)

        except Exception as e:
            cbs_logger.exception("{}: {}: Got an error when Audio is enabled : {}". \
                                 format(self.dev.port_name, self.dev.device_name, e))

    def resetDefaultControls(self):
        self.enableMccs.Checked = True
        self.enableSsc.Checked = True
        self.enableIsolate.Checked = False
        self.enableAuxAnalyser.Checked = False
        self.enableYcbcr.Checked = False
        self.customEdidComBox.SelectedIndex = 0
        self.chooseBpcComBox.SelectedIndex = 0
        self.chooseBandwidthComBox.SelectedIndex = 0
        self.chooseLCComBox.SelectedIndex = 0
        self.enableAudio.Checked = True


    def GoButton_Click(self, sender, args):
        """
        This function is executed when Go button is clicked. It creates and sends out
        DP_RestartDPStateMachine

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: Go Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_RestartDPStateMachine", False)
            swVersion_thread = Threading.Thread(
                                    Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "RestartDP")
            swVersion_thread.Start(icmd_obj)
            cbs_logger.info("{}: {}: Clear Button Status: {}". \
                            format(self.dev.port_name, self.dev.device_name, self.clearFlashButtonClicked))
            if self.clearFlashButtonClicked:
                self.resetDefaultControls()
                self.clearFlashButtonClicked = False
        except Exception as e:
            error_msg = "{}: {}: Error occurred when clicking on Go : {}". \
                    format(self.dev.port_name, self.dev.device_name, e)
            cbs_logger.exception(error_msg)

    def getAuxIndex_Click(self):
        """
        This function is executed when Go button is clicked. It creates and sends out
        DP_RestartDPStateMachine

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: Aux Index Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("DP_AUX_COMPONENT", "Aux_icmdAuxTransGetReadAndWriteIndex", False)
            auxIndex_thread = Threading.Thread(
                                    Threading.ParameterizedThreadStart(self.dev.send_icmd))
            auxIndex_thread.Name = self.Text.replace("Cobs", "Print Aux Index")
            auxIndex_thread.Start(icmd_obj)

        except Exception as e:
            error_msg = "{}: {}: Error occurred when clicking on Print Aux size : {}". \
                    format(self.dev.port_name, self.dev.device_name, e)
            cbs_logger.exception(error_msg)

    def printAuxButton_Click(self):
        """
        This function is executed when Print Aux Log button is clicked. It creates and sends out
        Aux_icmdAuxTransGetReadAndWriteIndex

        Arguments:  senders -unused
                    args - unused
        """

        def getAuxSize(self):
            try:
                for line in self.OutputTextBox.Text.ToString().split("\n"):
                    line = line + "\n"
                    if "Write Index = " in  line:
                        writeIndex = int(line.split("Write Index = ", 1)[1].split("\n", 1)[0])
                return writeIndex/20
            except:
                error_msg = "{}: {}: Error occurred when reading Write Index. Trying to read size again". \
                    format(self.dev.port_name, self.dev.device_name)
                cbs_logger.exception(error_msg)
            finally:
                self.getAuxIndex_Click()

        def printAuxBlock():
            icmd_obj = self.dev.create_icmd("DP_AUX_COMPONENT", "Aux_icmdReadAuxTrans", False)
            printAux_thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.send_icmd))
            printAux_thread.Name = self.Text.replace("Cobs", "Print Aux block")
            printAux_thread.Start(icmd_obj)
            printAux_thread.Join()

        try:
            cbs_logger.info("{}: {}: Print Aux Log Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            auxLogCount = getAuxSize(self)
            for index in range(0, auxLogCount):
                printAuxBlock()
                time.sleep(0.1)
            self.print_to_text_box("AUX Read done", Drawing.Color.Green)


        except Exception as e:
            error_msg = "{}: {}: Error occurred when clicking on Print Aux Log : {}". \
                    format(self.dev.port_name, self.dev.device_name, e)
            cbs_logger.exception(error_msg)

    def getTiming_Click(self, sender, args):
        """
        This function is executed when auxTransaction button is clicked. It runs python script
        aux_analyser

        Arguments:  senders -unused
                    args - unused
        """
        """
        This function is executed when Read DPCD button is clicked. It creates and sends out
        DP_REX_ReadDpcdCap

        Arguments:  senders -unused
                    args - unused
        """
        def getEdidList(device):
            localList = []
            for line in device.Text.ToString().split('\n'):
                line = line + "\n"
                if "//0x" in line:
                    localList.append(int(line.split("0x", 1)[1].split(",", 1)[0], 16))
            return localList

        try:
            cbs_logger.info("{}: {}: EDID Timing Button is clicked". \
                format(self.dev.port_name, self.dev.device_name))
            edidList = getEdidList(self.OutputTextBox)
            if (len(edidList) >= 128):
                edidInt = aux_analyser.edid(edidList, None, True, self.OutputTextBox)
                self.printTiming.Enabled = False
        except Exception as ex:
            error_message = "{}: {}: Encountered an error while loading aux_analyser.py". \
                    format(self.dev.port_name, self.dev.device_name) + '\n' + str(ex)
            cbs_logger.exception(error_message)


    def ReadDPCD_Click(self, sender, args):
        """
        This function is executed when Read DPCD button is clicked. It creates and sends out
        DP_REX_ReadDpcdCap

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: Read DPCD Button is clicked". \
                            format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_REX_ReadDpcdCap", False)
            readDpcd_thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.send_icmd))
            readDpcd_thread.Name = self.Text.replace("Cobs", "RestartDP")
            readDpcd_thread.Start(icmd_obj)
            self.InterpretDPCD.Enabled = True

        except Exception as e:
            error_msg = "{}: {}: Error occurred when clicking on Go : {}". \
                format(self.dev.port_name, self.dev.device_name, e)
            cbs_logger.exception(error_msg)

    def InterpretDPCD_Click(self, sender, args):
        """
        This function is executed when Read DPCD button is clicked. It creates and sends out
        DP_REX_ReadDpcdCap

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: Interpred DPCD Button is clicked". \
                format(self.dev.port_name, self.dev.device_name))
            dpcdInt = aux_analyser.dpcd(self.dev)
            self.InterpretDPCD.Enabled = False
        except Exception as ex:
            error_message = "{}: {}: Encountered an error while loading aux_analyser.py". \
                    format(self.dev.port_name, self.dev.device_name) + '\n' + str(ex)
            cbs_logger.exception(error_message)

    def dpDebugButton_Click(self, sender, args):
        """
        This function is executed when DP Debug button is clicked. It creates and sends out
        DP_GetVideoInfoIcmd

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: DP Debug Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("DP_STREAM_COMPONENT", "DP_STREAM_GetVideoInfoIcmd", False)
            swVersion_thread = Threading.Thread(
                                    Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "PrintDpDebug")
            swVersion_thread.Start(icmd_obj)
        except:
            error_msg = "{}: {}: Error occurred when clicking on DP Debug". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_msg)

    def dpSdpStatsButton_Click(self, sender, args):
        """
        This function is executed when SDP Stats button is clicked. It creates and sends out
        DP_SdpStatsIcmd

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: DP Stats Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("DP_STREAM_COMPONENT", "DP_STREAM_SdpStatsIcmd", False)
            swVersion_thread = Threading.Thread(
                                    Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "PrintSdpStats")
            swVersion_thread.Start(icmd_obj)
        except:
            error_msg = "{}: {}: Error occurred when clicking on DP Stats". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_msg)

    def darkModeButton_Click(self, sender, args):
        """
        This function is executed when SDP Stats button is clicked. It changes cobs
        color to Dark Mode

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: Dark Mode Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            self.darkModeStatus = not self.darkModeStatus
            if self.darkModeStatus:
                #self.OutputTextBox.BackColor = Drawing.ColorTranslator.FromHtml("#FF1F1F1F")
                self.OutputTextBox.BackColor = Drawing.Color.LightGray
            else:
                self.OutputTextBox.BackColor = Drawing.Color.White
            """
            icmd_obj = self.dev.create_icmd("DP_STREAM_COMPONENT", "DP_STREAM_SdpStatsIcmd", False)
            swVersion_thread = Threading.Thread(
                                    Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "PrintSdpStats")
            swVersion_thread.Start(icmd_obj)
            """
        except:
            error_msg = "{}: {}: Error occurred when clicking on Dark Mode". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_msg)


    def testAssertButton_Click(self, sender, args):
        """
        This function is executed when Test Assert button is clicked. It creates and sends out
        TOPLEVEL_DEBUG_ASSERT_BB icmd

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: Test Assert Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("TOPLEVEL_COMPONENT", "TOPLEVEL_DEBUG_ASSERT", False)
            swVersion_thread = Threading.Thread(
                                    Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "BB_Assert")
            swVersion_thread.Start(icmd_obj)
        except:
            error_msg = "{}: {}: Error occurred when clicking Test Assert". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_msg)

    def dumpDpFlashVarButton_Click(self, sender, args):
        """
        This function is executed when DumpDpFlashVar button is clicked. It creates and sends out
        DP_DumpFlashVarsIcmd icmd

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: Dump DP Flash Variables button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_DumpFlashVarsIcmd",
                    False)
            swVersion_thread = Threading.Thread(
                                    Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "DumpDpFlashVariables")
            swVersion_thread.Start(icmd_obj)
        except:
            error_msg = "{}: {}: Error occurred when clicking DumpDpFlashVar". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_msg)   

    def clearFlashVarButton_Click(self, sender, args):
        """
        This function is executed when clearFlashVar button is clicked. It creates and sends out
        flashDataEraseFlashVars icmd

        Arguments:  senders -unused
                    args - unused
        """
        try:
            icmd_list = [
                ["DP_COMPONENT", "DP_LEX_SetEdidTypeIcmd",      0],
                ["DP_COMPONENT", "DP_REX_SscAdvertiseEnable",   1],
                ["DP_COMPONENT", "DP_SetIsolateEnable",         0],
                ["DP_COMPONENT", "DP_LEX_YCbCrDisableIcmd",     0],
                ["DP_COMPONENT", "DP_REX_MccsEnable",           1],
                ["DP_COMPONENT", "DP_LEX_SetBpcModeIcmd",       0],
                ["DP_COMPONENT", "DP_SetBwLc",                  0],
                ["DP_COMPONENT", "DP_EnableAuxTraffic",         0],
                ["DP_COMPONENT", "DP_SetAudioState",            0]
            ]
            cbs_logger.info("{}: {}: Clear Flash Variables button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            for index in range(0, len(icmd_list)):
                if index == 6:
                    icmd_obj = self.dev.create_icmd(icmd_list[index][0], icmd_list[index][1], False,
                                                    [icmd_list[index][2], icmd_list[index][2]])
                else:
                    icmd_obj = self.dev.create_icmd(icmd_list[index][0], icmd_list[index][1], False,
                                                    [icmd_list[index][2]])
                clear_thread = Threading.Thread(
                                        Threading.ParameterizedThreadStart(self.dev.send_icmd))
                clear_thread.Name = self.Text.replace("Cobs", "ClearFlashVariables")
                clear_thread.Start(icmd_obj)

            self.clearFlashButtonClicked = True
            cbs_logger.info("{}: {}: Clear Flash Variables button is clicked: {}". \
                    format(self.dev.port_name, self.dev.device_name, self.clearFlashButtonClicked))
        except Exception as e:
            error_msg = "{}: {}: Error occurred when clicking ClearFlashVar: {}". \
                    format(self.dev.port_name, self.dev.device_name, e)
            cbs_logger.exception(error_msg) 

    def auxTransaction_click(self, sender, args):
        """
        This function is executed when auxTransaction button is clicked. It runs python script 
        aux_analyser

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: AuxTransaction Button is clicked". \
                format(self.dev.port_name, self.dev.device_name))
            if self.auxFolderBrowserDialog.ShowDialog() == Forms.DialogResult.OK:
                self.auxAnalyserPath = self.auxFolderBrowserDialog.SelectedPath
                cbs_logger.info("{}: {}: Aux Analyser folder is : {}".format(self.dev.port_name, self.dev.device_name, self.auxAnalyserPath))
                aux_trans = aux_analyser.aux_trace(self.dev, self.auxAnalyserPath)
            else:
                error_message = "{}: {}: Encountered an error while opening Folder browser". \
                                    format(self.dev.port_name, self.dev.device_name)
                cbs_logger.exception(error_message)

        except Exception as e:
            error_message = "{}: {}: Encountered an error while loading aux_analyser.py: {}". \
                    format(self.dev.port_name, self.dev.device_name, e)
            cbs_logger.exception(error_message)

    def edidPrint_click(self, sender, args):
        """
        This function is executed when auxTransaction button is clicked. It runs python script 
        aux_analyser

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: edidPrint Button is clicked". \
                format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_REX_EdidReadIcmd",
                    False)
            swVersion_thread = Threading.Thread(
                                    Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "REX_read_EDID")
            swVersion_thread.Start(icmd_obj)

            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_LEX_ReadEdidValues",
                                            False)
            swVersion_thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "REX_read_EDID")
            swVersion_thread.Start(icmd_obj)
            self.printTiming.Enabled = True
        except Exception as ex:
            error_msg = "{}: {}: Error occurred when clicking edidPrint_click". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_msg)


    def readCap_click(self, sender, args):
        """
        This function is executed when auxTransaction button is clicked. It runs python script 
        aux_analyser

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: readCap Button is clicked". \
                format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_REX_CapReadIcmd",
                    False)
            swVersion_thread = Threading.Thread(
                                    Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "Read_Cap")
            swVersion_thread.Start(icmd_obj)

        except Exception as ex:
            error_msg = "{}: {}: Error occurred when clicking readCap_click". \
                    format(self.dev.port_name, self.dev.device_name) + '\n' + str(ex)
            cbs_logger.exception(error_message)

    # def enableAudio_click(self, sender, args):
    #     """
    #     This function is executed when enableAudio button is clicked. It runs DP_STREAM_RexEnableAudioIcmd
    #
    #     Arguments:  senders -unused
    #                 args - unused
    #     """
    #     try:
    #         cbs_logger.info("{}: {}: enableAudio Button is clicked". \
    #             format(self.dev.port_name, self.dev.device_name))
    #         icmd_obj = self.dev.create_icmd("DP_STREAM_COMPONENT", "DP_STREAM_RexEnableAudioIcmd",
    #                 False)
    #         swVersion_thread = Threading.Thread(
    #                                 Threading.ParameterizedThreadStart(self.dev.send_icmd))
    #         swVersion_thread.Name = self.Text.replace("Cobs", "enableAudio")
    #         swVersion_thread.Start(icmd_obj)
    #
    #     except Exception as ex:
    #         error_msg = "{}: {}: Error occurred when clicking enableAudio". \
    #                 format(self.dev.port_name, self.dev.device_name) + '\n' + str(ex)
    #         cbs_logger.exception(error_message)
    #
    # def disableSdp_click(self, sender, args):
    #     """
    #     This function is executed when disableSdp button is clicked. It runs DP_SetSdpSupport
    #
    #     Arguments:  senders -unused
    #                 args - unused
    #     """
    #     try:
    #         cbs_logger.info("{}: {}: eightBpc Button is clicked". \
    #             format(self.dev.port_name, self.dev.device_name))
    #         icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_SetSdpSupport",
    #                 False, (0,) )
    #         swVersion_thread = Threading.Thread(
    #                                 Threading.ParameterizedThreadStart(self.dev.send_icmd))
    #         swVersion_thread.Name = self.Text.replace("Cobs", "disableSdp")
    #         swVersion_thread.Start(icmd_obj)
    #
    #     except Exception as ex:
    #         error_msg = "{}: {}: Error occurred when clicking eightBpc". \
    #                 format(self.dev.port_name, self.dev.device_name) + '\n' + str(ex)
    #         cbs_logger.exception(error_message)

    def dpAllLogs_click(self, sender, args):
        """
        This function is executed when dpAllLogs button is clicked. It runs DP_STREAM_PrintAllLogsIcmd

        Arguments:  senders -unused
                    args - unused
        """
        try:
            cbs_logger.info("{}: {}: dpAllLogs Button is clicked". \
                format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("DP_STREAM_COMPONENT", "DP_STREAM_PrintAllLogsIcmd",
                    False )
            swVersion_thread = Threading.Thread(
                                    Threading.ParameterizedThreadStart(self.dev.send_icmd))
            swVersion_thread.Name = self.Text.replace("Cobs", "dpAllLogs")
            swVersion_thread.Start(icmd_obj)

        except Exception as ex:
            error_msg = "{}: {}: Error occurred when clicking dpAllLogs". \
                    format(self.dev.port_name, self.dev.device_name) + '\n' + str(ex)
            cbs_logger.exception(error_message)

    # def sixBpc_click(self, sender, args):
    #     """
    #     This function is executed when sixBpc button is clicked. It runs DP_SetSdpSupport
    #
    #     Arguments:  senders -unused
    #                 args - unused
    #     """
    #     try:
    #         cbs_logger.info("{}: {}: sixBpc Button is clicked". \
    #             format(self.dev.port_name, self.dev.device_name))
    #         icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_LEX_SetBpcModeIcmd",
    #                 False, (6,) )
    #         swVersion_thread = Threading.Thread(
    #                                 Threading.ParameterizedThreadStart(self.dev.send_icmd))
    #         swVersion_thread.Name = self.Text.replace("Cobs", "sixBpc")
    #         swVersion_thread.Start(icmd_obj)
    #
    #     except Exception as ex:
    #         error_msg = "{}: {}: Error occurred when clicking sixBpc". \
    #                 format(self.dev.port_name, self.dev.device_name) + '\n' + str(ex)
    #         cbs_logger.exception(error_message)
    #
    # def eightBpc_click(self, sender, args):
    #     """
    #     This function is executed when eightBpc button is clicked. It runs DP_SetSdpSupport
    #
    #     Arguments:  senders -unused
    #                 args - unused
    #     """
    #     try:
    #         cbs_logger.info("{}: {}: eightBpc Button is clicked". \
    #             format(self.dev.port_name, self.dev.device_name))
    #         icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_LEX_SetBpcModeIcmd",
    #                 False, (8,) )
    #         swVersion_thread = Threading.Thread(
    #                                 Threading.ParameterizedThreadStart(self.dev.send_icmd))
    #         swVersion_thread.Name = self.Text.replace("Cobs", "eightBpc")
    #         swVersion_thread.Start(icmd_obj)
    #
    #     except Exception as ex:
    #         error_msg = "{}: {}: Error occurred when clicking eightBpc". \
    #                 format(self.dev.port_name, self.dev.device_name) + '\n' + str(ex)
    #         cbs_logger.exception(error_message)

    # def Res4k_click(self, sender, args):
    #     """
    #     This function is executed when Res4k button is clicked. It runs DP_SetSdpSupport
    #
    #     Arguments:  senders -unused
    #                 args - unused
    #     """
    #     try:
    #         cbs_logger.info("{}: {}: Res4k Button is clicked". \
    #             format(self.dev.port_name, self.dev.device_name))
    #         icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_LEX_SetEdidTypeIcmd",
    #                 False, (1,) )
    #         swVersion_thread = Threading.Thread(
    #                                 Threading.ParameterizedThreadStart(self.dev.send_icmd))
    #         swVersion_thread.Name = self.Text.replace("Cobs", "Res4k")
    #         swVersion_thread.Start(icmd_obj)
    #
    #     except Exception as ex:
    #         error_msg = "{}: {}: Error occurred when clicking Res4k". \
    #                 format(self.dev.port_name, self.dev.device_name) + '\n' + str(ex)
    #         cbs_logger.exception(error_message)
    #
    # def Res1080p_click(self, sender, args):
    #     """
    #     This function is executed when Res1080k button is clicked. It runs DP_SetSdpSupport
    #
    #     Arguments:  senders -unused
    #                 args - unused
    #     """
    #     try:
    #         cbs_logger.info("{}: {}: Res1080k Button is clicked". \
    #             format(self.dev.port_name, self.dev.device_name))
    #         icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_LEX_SetEdidTypeIcmd",
    #                 False, (2,) )
    #         swVersion_thread = Threading.Thread(
    #                                 Threading.ParameterizedThreadStart(self.dev.send_icmd))
    #         swVersion_thread.Name = self.Text.replace("Cobs", "Res1080k")
    #         swVersion_thread.Start(icmd_obj)
    #
    #     except Exception as ex:
    #         error_msg = "{}: {}: Error occurred when clicking Res1080k". \
    #                 format(self.dev.port_name, self.dev.device_name) + '\n' + str(ex)
    #         cbs_logger.exception(error_message)

    # def enableIsolate_click(self, sender, args):
    #     """
    #     This function is executed when Enable Isolate button is clicked. It runs DP_SetIsolateEnable
    #
    #     Arguments:  senders -unused
    #                 args - unused
    #     """
    #     try:
    #         cbs_logger.info("{}: {}: Enable Isolate Button is clicked". \
    #             format(self.dev.port_name, self.dev.device_name))
    #         icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_SetIsolateEnable",
    #                 False, (1,) )
    #         swVersion_thread = Threading.Thread(
    #                                 Threading.ParameterizedThreadStart(self.dev.send_icmd))
    #         swVersion_thread.Name = self.Text.replace("Cobs", "enableIsolate")
    #         swVersion_thread.Start(icmd_obj)
    #
    #     except Exception as ex:
    #         error_msg = "{}: {}: Error occurred when clicking enableIsolate". \
    #                 format(self.dev.port_name, self.dev.device_name) + '\n' + str(ex)
    #         cbs_logger.exception(error_message)
    #
    # def disableIsolate_click(self, sender, args):
    #     """
    #     This function is executed when Disable Isolate button is clicked. It runs DP_SetIsolateEnable
    #
    #     Arguments:  senders -unused
    #                 args - unused
    #     """
    #     try:
    #         cbs_logger.info("{}: {}: Enable Isolate Button is clicked". \
    #             format(self.dev.port_name, self.dev.device_name))
    #         icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_SetIsolateEnable",
    #                 False, (0,) )
    #         swVersion_thread = Threading.Thread(
    #                                 Threading.ParameterizedThreadStart(self.dev.send_icmd))
    #         swVersion_thread.Name = self.Text.replace("Cobs", "DisableIsolate")
    #         swVersion_thread.Start(icmd_obj)
    #
    #     except Exception as ex:
    #         error_msg = "{}: {}: Error occurred when clicking DisableIsolate". \
    #                 format(self.dev.port_name, self.dev.device_name) + '\n' + str(ex)
    #         cbs_logger.exception(error_message)
            
    def decodeButton_Click(self, sender, args):
        try:
            cbs_logger.info("Decode Icron log file button is clicked")
            if self.icronLogFileDialog.ShowDialog() == Forms.DialogResult.OK:
                log_file = self.icronLogFileDialog.FileName

            self.dev.log_file_decoder.register_log_message_handler(self.OutputTextBox.AppendText)
            self.dev.log_file_decoder.register_log_message_handler(self.dev.logger.log)
            self.dev.log_file_decoder.decode(log_file)
            self.dev.log_file_decoder.remove_log_message_handler(self.OutputTextBox.AppendText)
            self.dev.log_file_decoder.remove_log_message_handler(self.dev.logger.log)
        except:
            error_msg = "{}: {}: Error occurred when decoding Icron log file". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_msg)

    def recordButton_Click(self, sender, args):
        def handle_record():
            try:
                self.button_recording = not self.button_recording

                if self.button_recording:
                    self.dev.port_recorder.create_new_log_file(self.dev.port_name)
                    self.recordButton.Text = "Stop"
                else:
                    self.dev.port_recorder.stop()
                    self.recordButton.Text = "Record"
            except:
                error_msg = "{}: {}: Error occurred when clicking on Record button". \
                        format(self.dev.port_name, self.dev.device_name)
                cbs_logger.exception(error_msg)

        cbs_logger.info("{}: {}: Record Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
        handle_record()
        #thread = Threading.Thread(Threading.ThreadStart(handle_record))
        #thread.SetApartmentState(Threading.ApartmentState.STA)
        #thread.Start()

    def iRegisterButton_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: iRegister Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            self.dev.print_to_device_window("Loading iRegister GUI ...", Drawing.Color.DarkGreen)
            with open('Scripts/ipxact_gui.py', 'r') as ipxact_file:
                ipxact = ipxact_file.read()
                self.InputTextBox.Text = ipxact.replace(
                                     "\r\n", "\n").replace("\n", "\r\n")
            self.UpdateExecuteCommand()
        except Exception as ex:
            error_message = "{}: {}: Encountered an error while loading IPXACT". \
                    format(self.dev.port_name, self.dev.device_name) + '\n' + str(ex)
            cbs_logger.exception(error_message)
            Forms.MessageBox.Show(
                    error_message,
                    "ERROR",
                    Forms.MessageBoxButtons.OK,
                    Forms.MessageBoxIcon.Error)

    def SendButton_Click(self):
        """Executes the code in the input textbox.

        Arguments: sender - unused
                   args - unused"""
        cbs_logger.info("{}: {}: Send Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
        self.UpdateExecuteCommand()
        #delegate=CallTarget0(lambda: self.UpdateExecuteCommand())
        #self.dev.devWindow.MainSplitContainer.Invoke(delegate)

    def UpdateExecuteCommand(self):
        # get the input text
        code = self.InputTextBox.Text

        # separate the code into lines purely for reproducing them on the
        #     output textbox with Python-like prefixes
        outputLines = code.splitlines()

        # auto scroll
        textbox = self.OutputTextBox
        textbox.ScrollToCaret()

        # execute code in new thread
        # otherwise, the DeviceWindow thread is busy executing the code, which
        #     might result in deadlocks when bytes are available to read but
        #     aren't actually being read
        self.InputTextBox.Clear()

        thread = Threading.Thread(Threading.ParameterizedThreadStart(self.execute_code))
        thread.Name = self.Text.replace("Cobs","executing via Send")
        thread.SetApartmentState(Threading.ApartmentState.STA);
        thread.Start(code)

    def ClearInputButton_Click(self):
        """Clears the input textbox.

        Arguments: sender - unused
                   args - unused"""
        cbs_logger.info("{}: {}: Clear Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
        # self-explanatory enough I think, call on GUI thread
        #self.InputTextBox.Clear()
        delegate=CallTarget0(lambda: self.InputTextBox.Clear())
        self.MainSplitContainer.Invoke(delegate)

    def SaveButton_Click(self):
        """Saves a file from the input textbox.

        Arguments: sender - unused
                   args - unused"""
        cbs_logger.info("{}: {}: Save Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
        # create SaveFileDialog and show it
        saveFileDialog = Forms.SaveFileDialog()
        saveFileDialog.Filter = "Python script (*.py)|*.py|" + \
                                "All files (*.*)|*.*"
        if saveFileDialog.ShowDialog() == Forms.DialogResult.OK:
            # get stream to write to file
            fileStream = saveFileDialog.OpenFile()
            if not fileStream == None:
                # create StreamWriter from original stream
                writeStream = System.IO.StreamWriter(fileStream)
                # write text to file
                writeStream.Write(self.InputTextBox.Text)
                writeStream.Close()

    def LoadButton_Click(self):
        """Loads a file into the input textbox.

        Arguments: sender - unused
                   args - unused"""
        cbs_logger.info("{}: {}: Load Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
        # show a dialog, ensure the user wants to load the file
        if self.InputFileDialog.ShowDialog() == Forms.DialogResult.OK:
            # get the contents of the file and replace the input textbox's
            #     current text with it
            sr = System.IO.StreamReader(self.InputFileDialog.FileName)
            # yes, double replacement
            # the leftmost one replaces all CR LFs with LFs
            # the rightmost one replaces all LFs with CR LFs
            # this way, everything in the textbox will contain CR LFs,
            #     regardless of any mix of CR LFs and LFs
            # and if you use Mac, too bad because pseudo-Unix just won't cut it
            self.InputTextBox.Text = sr.ReadToEnd().replace(
                                     "\r\n", "\n").replace("\n", "\r\n")
            sr.Close()

    def iCommandsButton_Click(self, sender, args):
        """
        Create and run an iCommandWindow.

        Arguments: sender - unused
                   args - unused
        """
        try:
            cbs_logger.info("{}: {}: iCommands Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            icmd_window = icg.iCommandWindow(self.dev)
            icmd_window.run()
        except:
            error_message = "{}: {}: Got an error when launching iCmd GUI window". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_message)
            Forms.MessageBox.Show(
                    error_message,
                    "ERROR",
                    Forms.MessageBoxButtons.OK,
                    Forms.MessageBoxIcon.Error)

    def ClearOutputButton_Click(self, sender, args):
        """
        Clear the output textbox.

        Arguments: sender - unused
                   args - unused
        """
        cbs_logger.info("{}: {}: Clear Output Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
        # again, should be pretty simple to understand here
        #self.OutputTextBox.Clear()
        delegate=CallTarget0(lambda: self.OutputTextBox.Clear())
        self.MainSplitContainer.Invoke(delegate)

    def ReloadIcronButton_Click(self, sender, args):
        """
        Reload the Device's .icron file.

        Arguments: sender - unused
                   args - unused
        """
        def change_icron_file():
            thread_list = []

            for (client_name, port_name) in self.dev.cobs.device_clients:
                if port_name == self.dev.port_name:
                    device = self.dev.cobs.device_clients[(client_name, port_name)]
                    change_icron_thread = threading.Thread(
                            target=lambda obj=device: obj.devWindow.reload_icron_file())
                    change_icron_thread.start()
                    thread_list.append(change_icron_thread)
            for t in thread_list:
                t.join()

        try:
            cbs_logger.info("{}: {}: Reload Icron Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))

            thread = Threading.Thread(Threading.ThreadStart(change_icron_file))
            thread.SetApartmentState(Threading.ApartmentState.STA)
            thread.Start()
            # self.icron_file_watcher = System.IO.FileSystemWatcher(
            #                             os.path.dirname(self.dev.original_icron_file_path))
            # cbs_logger.info(self.dev.original_icron_file_path)
            # # self.icron_file_watcher.Filter = os.path.basename(self.dev.original_icron_file_path)
            # self.icron_file_watcher.Deleted += self.dev.icronFileChanged
            # self.icron_file_watcher.Changed += self.dev.icronFileChanged
            # self.icron_file_watcher.EnableRaisingEvents = True
            # self.icron_file_watcher.NotifyFilter = System.IO.NotifyFilters.FileName | System.IO.NotifyFilters.LastWrite

        except:
            error_message = "{}: {}: Got an error when reloading new icron file". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                    error_string,
                    "ERROR",
                    Forms.MessageBoxButtons.OK,
                    Forms.MessageBoxIcon.Error)

    def download_icron_file(self):
        try:
            message = "Downloading icron file ..."
            self.print_to_text_box(message, Drawing.Color.DarkGreen)
            dest = os.path.join(os.getcwd(), 'icron_file')
            shutil.copy(self.dev.original_icron_file_path, dest)
            message = "New icron file is downloaded. Click Load File button to load it to Device window ".format(self.dev.original_icron_file_path)
            color = Drawing.Color.DarkGreen
            self.print_to_text_box(message, color)
        except:
            error_message = "{}: {}: Got an error when downloading icron file". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_message)

    def update_device_window(self):
        try:
            message = "Loading icron file ..."
            self.print_to_text_box(message, Drawing.Color.DarkGreen)
            delegate = CallTarget0(lambda: self.dev.load_icron_model(
                                                self.dev.cobs.iparsed_file,
                                                self.dev.cobs.startupWindow.icronFileTextBox.Text))
            result = self.MainSplitContainer.Invoke(delegate)

            m = "{}: {}: loading icron file result={} projects={}". \
                    format(self.dev.port_name, self.dev.device_name, result, self.dev.cobs.projects)
            cbs_logger.info(m)

            if result:
                self.ReloadIcronButton.ForeColor = Drawing.Color.Black
                message = "New icron file is loaded {}".format(self.dev.original_icron_file_path)
                color = Drawing.Color.DarkGreen

                new_title = "{} {} {}: {} - Cobs". \
                                            format(
                                                self.dev.port_name,
                                                self.dev.device_name,
                                                self.dev.device_type,
                                                self.dev.original_icron_file_path)
                delegate = CallTarget0(lambda: self.dev.update_text(new_title))
                self.MainSplitContainer.Invoke(delegate)
            else:
                message = "Failed to load new icron file {}".format(self.dev.original_icron_file_path)
                color = Drawing.Color.Red

            if (self.dev.device_type == "LEX"):
                if (self.dev.lex_fpga_image == None):
                    self.ProgramFpgaButton.Enabled = False
                else:
                    self.ProgramFpgaButton.Enabled = True
            elif (self.dev.device_type == "REX"):
                if (self.dev.rex_fpga_image == None):
                    self.ProgramFpgaButton.Enabled = False
                else:
                    self.ProgramFpgaButton.Enabled = True
            else:
                if (self.dev.rex_fpga_image == None) or (self.dev.lex_fpga_image == None):
                    self.ProgramFpgaButton.Enabled = False
                else:
                    self.ProgramFpgaButton.Enabled = True

            self.print_to_text_box(message, color)
        except:
            error_message = "{}: {}: Got an error when reloading file". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_message)

    def reload_icron_file(self):
        try:
            message = "Loading icron file ..."
            self.print_to_text_box(message, Drawing.Color.DarkGreen)
            self.dev.original_icron_file_path = self.dev.cobs.startupWindow.icronFileTextBox.Text

            dest = os.path.join(os.getcwd(), 'icron_file')
            if os.path.exists(dest):
                file_path = os.path.join(dest, os.path.basename(self.dev.original_icron_file_path))
                if os.path.exists(file_path):
                    os.chmod(file_path, 0o777)
            newPath = os.path.join(dest, os.path.basename(self.dev.original_icron_file_path))
            shutil.copy(self.dev.original_icron_file_path, dest)
            # icron_file_path = self.dev.cobs.startupWindow.icronFileTextBox.Text
            projects = self.dev.cobs.load_icron_file(newPath)
            message = None
            color = None
            if projects is not None:
                delegate = CallTarget0(lambda: self.dev.load_icron_model(
                                                self.dev.cobs.iparsed_file,
                                                self.dev.cobs.startupWindow.icronFileTextBox.Text))
                result = self.MainSplitContainer.Invoke(delegate)

                m = "{}: {}: loading icron file result={} projects={}". \
                        format(self.dev.port_name, self.dev.device_name, result, projects)
                cbs_logger.info(m)

                if result:
                    self.ReloadIcronButton.ForeColor = Drawing.Color.Black
                    message = "New icron file is loaded {}".format(self.dev.original_icron_file_path)
                    color = Drawing.Color.DarkGreen

                    new_title = "{} {} {}: {} - Cobs". \
                                            format(
                                                self.dev.port_name,
                                                self.dev.device_name,
                                                self.dev.device_type,
                                                self.dev.original_icron_file_path)
                    delegate = CallTarget0(lambda: self.dev.update_text(new_title))
                    self.MainSplitContainer.Invoke(delegate)
                else:
                    message = "Failed to load new icron file {}".format(self.dev.original_icron_file_path)
                    color = Drawing.Color.Red
            else:
                message = "Failed to load new icron file {}".format(self.dev.original_icron_file_path)
                color = Drawing.Color.Red

            self.print_to_text_box(message, color)
        except:
            error_message = "{}: {}: Got an error when reloading file". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_message)

    def autoReset_CheckedChanged(self, sender, args):
        cbs_logger.info("{}: {}: autoResetCheckbox is clicked boxChecked={}". \
                    format(self.dev.port_name, self.dev.device_name, self.autoReset.Checked))

    def ProgramFwButton_Click(self, sender, args):
        """
        Flashes the firmware image to device. Note that there is only one combined binary file
        that contains the firmware images for both blackbird and goldenears.
        Arguments: sender - unused
                   args - unused
        """
        try:
            cbs_logger.info("{}: {}: ProgramFw Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            program_thread = Threading.Thread(
                                Threading.ParameterizedThreadStart(self.dev.handle_program_device))
            program_thread.Name = self.Text.replace("Cobs", "Programming Device FW")
            parameters = ('FWType', self.dev.goldenImage)
            program_thread.Start(parameters)
            #freeze program buttons
            self.ProgramFwButton.Enabled = False
            self.ProgramFpgaButton.Enabled = False
        except:
            error_message = "{}: {}: Got an error when clicking Program button". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                    error_string,
                    "ERROR",
                    Forms.MessageBoxButtons.OK,
                    Forms.MessageBoxIcon.Error)


    def ProgramFpgaButton_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: ProgramFPGA Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            program_thread = Threading.Thread(
                                Threading.ParameterizedThreadStart(self.dev.handle_program_device))
            program_thread.Name = self.Text.replace("Cobs", "Programming FPGA")
            parameters = ('FPGAType', self.dev.goldenImage)
            program_thread.Start(parameters)
            #freeze program buttons
            self.ProgramFwButton.Enabled = False
            self.ProgramFpgaButton.Enabled = False
        except:
            error_message = "{}: {}: Got an error when clicking Program button". \
                                format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                    error_string,
                    "ERROR",
                    Forms.MessageBoxButtons.OK,
                    Forms.MessageBoxIcon.Error)

    def ResetDeviceButton_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Reset Device Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            reset_thread = Threading.Thread(Threading.ThreadStart(self.dev.reset_device))
            reset_thread.Start()
        except:
            error_message = "{}: {}: Got an error when clicking Reset Device button". \
                                format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                    error_string,
                    "ERROR",
                    Forms.MessageBoxButtons.OK,
                    Forms.MessageBoxIcon.Error)

    def DeviceWindow_Closed(self, sender, args):
        """Removes the Device whose DeviceWindow was closed from deviceList.

        Arguments: sender - unused
                   args - unused"""
        try:
            device = self.dev
            device.cobs.remove_device(device.port_name, device.device_name)
        except:
            cbs_logger.exception("{}: {}: Got an error when closing device window". \
                    format(self.dev.port_name, self.dev.device_name))

    def execute_code(self, code):
        """Executes code using the CobsInterpreter's Python interpreter."""
        # compile code, and attempt to execute
        try:
            codeSource = self.dev.interpreter.CreateScriptSourceFromString(
                     code, SourceCodeKind.Statements)
            # set currentDevice variable in interpreter
            self.dev.interpreterScope.SetVariable("currentDevice", self.dev)
            self.dev.interpreterScope.SetVariable("currentDeviceWindow", self)
            codeSource.Execute(self.dev.interpreterScope)
        except Exception as ex:
            # show the error that's occurred; this is quite similar to the
            #     exception handling in CobsInterpreter
            eo = self.dev.interpreter.GetService[ExceptionOperations]()
            error = eo.FormatException(ex)
            cbs_logger.exception(error)
            Forms.MessageBox.Show(
                    error,
                    "ERROR",
                    Forms.MessageBoxButtons.OK,
                    Forms.MessageBoxIcon.Error)
    # def LogDropDownMenuChanged(self, sender, args):
    #     try:   
    #         if self.LogDropDownMenu.Text is not "":
    #             menuSwitch = {
    #                 "New Log": self.newLogButton_Click,
    #                 "Export Log": self.ExportLogButton_Click,
    #                 "Open Log": self.OpenLogButton_Click,
    #                 "LogFilter": self.iLogFilterChange
    #             }
    #             func = menuSwitch[self.LogDropDownMenu.Text]
    #             func()
    #     except:
    #         cbs_logger.info("{}: {}: Error when log option". \
    #                 format(self.dev.port_name, self.dev.device_name))

    # def newLogButton_Click(self):
    #     """Saves the log file in the format YYYY_MM_DD_HH_MM_SS and create
    #        a new empty cobs log file.

    #     Arguments: sender - unused
    #                args - unused"""
    #     try:
    #         cbs_logger.info("{}: {}: {} New Log Button is clicked". \
    #                 format(self.dev.port_name, self.dev.device_name, self.dev.device_type))
    #         newLogPath = "IcronLogs_" + str(datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S"))
    #         os.mkdir("Log\\" + newLogPath)
    #         for (self.dev.device_name, self.dev.log_port_name, self.dev.device_type) in self.dev.cobs.device_dict:
    #             self.dev.cobs.loggers[(self.dev.device_name, self.dev.log_port_name)].create_new_log_file("{}\{}_{}_{}". 
    #                                 format(newLogPath, self.dev.log_port_name, self.dev.device_type, self.dev.device_name))
    #         for (self.dev.log_client_name, self.dev.log_port_name, self.dev.device_type) in self.dev.cobs.device_dict:
    #             self.dev.cobs.loggers[(self.dev.log_client_name, self.dev.log_port_name)].create_new_log_file("{}\{}_{}_{}". 
    #                                 format(newLogPath, self.dev.log_port_name, self.dev.device_type, self.dev.log_client_name))
    #         self.dev.cobs.MainLogPath = newLogPath
    #         message = "{}: New log file started".format(newLogPath)
    #         color = Drawing.Color.Blue
    #         self.dev.print_to_device_window(str(message), color)
    #         cbs_logger.info(self.dev.cobs.MainLogPath)


        # except:
        #     cbs_logger.exception("{}: {}: Got an error when creating New Logs". \
        #             format(self.dev.port_name, self.dev.device_name))


    def searchText_Enter(self, sender, event):
        try:
            if event.KeyCode == Forms.Keys.Return:
                word = self.searchTextBox.Text
                if word == "": 
                    '''If no text it entered, clear search results'''
                    self.clear_highlight_text()
                else:
                    self.scrollValue = 0
                    cbs_logger.info("{}: {}: Got text={} searched". \
                            format(self.dev.port_name, self.dev.device_name, word))
                    delegate = CallTarget0(lambda: self.highlight_text(word))
                    self.MainSplitContainer.Invoke(delegate)
            elif event.KeyCode == Forms.Keys.Up:
                '''When arrow key up, scroll to previous search result'''
                word = self.searchTextBox.Text
                delegate = CallTarget0(lambda: self.scrollTo_text(self.wordAddresses, len(word), True))
                self.MainSplitContainer.Invoke(delegate)
            elif event.KeyCode == Forms.Keys.Down:
                '''When arrow key down, scroll to next search result'''
                word = self.searchTextBox.Text
                delegate = CallTarget0(lambda: self.scrollTo_text(self.wordAddresses, len(word), False))
                self.MainSplitContainer.Invoke(delegate)
        except:
            cbs_logger.exception("{}: {}: Got an error when searching text". \
                    format(self.dev.port_name, self.dev.device_name))
					
    def scrollTo_text(self, wordAddress, lengthOfWord, UpDown=True):
       #Scrolls to specific search result indicated by searchText_Enter
        try:
            LastValue = len(wordAddress)-1
            if UpDown: #if true, go up
                self.OutputTextBox.Select(wordAddress[LastValue - self.scrollValue],lengthOfWord)
                self.OutputTextBox.SelectionBackColor = Drawing.Color.Yellow
                self.scrollValue +=1
            else:
                self.OutputTextBox.Select(wordAddress[LastValue - self.scrollValue],lengthOfWord)
                self.OutputTextBox.SelectionBackColor = Drawing.Color.Yellow
                self.scrollValue -=1
            if self.scrollValue > LastValue:
                self.scrollValue = 0
            elif self.scrollValue < 0:
                self.scrollValue = LastValue
            self.OutputTextBox.Select(wordAddress[LastValue - self.scrollValue],lengthOfWord)
            self.OutputTextBox.SelectionBackColor = Drawing.Color.Pink
            self.OutputTextBox.ScrollToCaret()
        except:
            cbs_logger.exception("{}: {}: Got an error when Scrolling to text". \
                    format(self.dev.port_name, self.dev.device_name))
					
    def highlight_text(self, word, color=Drawing.Color.Yellow):
        try:
            if word:
                s_start = self.OutputTextBox.SelectionStart
                self.OutputTextBox.SelectionStart = 0;
                self.OutputTextBox.SelectionLength = self.OutputTextBox.TextLength;    
                self.OutputTextBox.SelectionBackColor = Drawing.Color.White
                num_found = 0
                startIndex = 0
                index = self.OutputTextBox.Text.IndexOf(
                                                    word,
                                                    startIndex,
                                                    System.StringComparison.OrdinalIgnoreCase)
                wordAddress=[]
                while index != -1:
                    num_found += 1
                    self.OutputTextBox.Select(index, len(word));
                    self.OutputTextBox.SelectionBackColor = color;
                    
                    startIndex = index + len(word);
                    wordAddress.append(index)
                    index = self.OutputTextBox.Text.IndexOf(
                                                    word,
                                                    startIndex,
                                                    System.StringComparison.OrdinalIgnoreCase)
													 
                self.OutputTextBox.Select(wordAddress[-1], len(word));
                self.OutputTextBox.ScrollToCaret()
                self.wordAddresses = wordAddress
                self.OutputTextBox.Select(wordAddress[-1],len(word))
                self.OutputTextBox.SelectionBackColor = Drawing.Color.Pink
                self.searchResultLabel.Text = "{} found".format(num_found)
                self.OutputTextBox.SelectionStart = s_start;
                self.OutputTextBox.SelectionLength = 0;
                
        except:
            cbs_logger.exception("{}: {}: Got an error when highlighting text". \
                    format(self.dev.port_name, self.dev.device_name))


    def clear_highlight_text(self):
        try:
            cbs_logger.info("{}: {}: Clearing search results". \
                            format(self.dev.port_name, self.dev.device_name))
            self.wordAddresses = []
            self.searchResultLabel.Text = "0 found"
            self.OutputTextBox.SelectionStart = 0;
            self.OutputTextBox.SelectionLength = self.OutputTextBox.TextLength;
            if self.devWindow.darkModeStatus:
                color = Drawing.Color.White
            else:
                color = Drawing.Color.Black
            self.OutputTextBox.SelectionBackColor = color
            self.OutputTextBox.SelectionStart = self.OutputTextBox.TextLength;
            self.OutputTextBox.SelectionLength = 0
            self.OutputTextBox.ScrollToCaret()
        except:
            cbs_logger.exception("{}: {}: Got an error when clearing the highlight". \
                    format(self.dev.port_name, self.dev.device_name))

    def edidChanged(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Changing EDID". \
                            format(self.dev.port_name, self.dev.device_name))
            thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.change_edid))
            edid = self.customEdidComBox.Text
            thread.Start(edid)
        except:
            cbs_logger.exception("{}: {}: Got an error when changeing EDID". \
                                 format(self.dev.port_name, self.dev.device_name))

    def BpcChanged(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Changing BPC". \
                            format(self.dev.port_name, self.dev.device_name))
            thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.change_bpc))
            bpc = self.chooseBpcComBox.Text
            thread.Start(bpc)
        except:
            cbs_logger.exception("{}: {}: Got an error when changeing BPC". \
                                 format(self.dev.port_name, self.dev.device_name))

    def bandwidthChanged(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Changing Bandwidth". \
                            format(self.dev.port_name, self.dev.device_name))
            thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.change_bw))
            bw = self.chooseBandwidthComBox.Text
            thread.Start(bw)
        except Exception as e:
            cbs_logger.exception("{}: {}: Got an error when changeing Bandwidth : {}". \
                                 format(self.dev.port_name, self.dev.device_name, e))

    def lcChanged(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Changing Lane Count". \
                            format(self.dev.port_name, self.dev.device_name))
            thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.change_lc))
            lc = self.chooseLCComBox.Text
            thread.Start(lc)
        except Exception as e:
            cbs_logger.exception("{}: {}: Got an error when changeing Lane Count : {}". \
                                 format(self.dev.port_name, self.dev.device_name, e))

    def auxMenuChanged(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Changing Aux Menu". \
                            format(self.dev.port_name, self.dev.device_name))
            thread = Threading.Thread(
                Threading.ParameterizedThreadStart(self.dev.change_auxMenu))
            auxMenu = self.auxAnalyserComBox.Text
            thread.Start(auxMenu)
        except Exception as e:
            cbs_logger.exception("{}: {}: Got an error when changing Aux Menu : {}". \
                                 format(self.dev.port_name, self.dev.device_name, e))

    def test_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Open test option window". \
                            format(self.dev.port_name, self.dev.device_name))
            testWindow = tests.dp_test(self.dev)
            testWindow.run()
        except Exception as e:
            cbs_logger.exception("{}: {}: Got an error when opening test window : {}". \
                                 format(self.dev.port_name, self.dev.device_name, e))

    def rex_tu_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Rex tu Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_AUX_RexErrorRecovery", False)
            tu_thread = Threading.Thread(
                                    Threading.ParameterizedThreadStart(self.dev.send_icmd))
            tu_thread.Name = self.Text.replace("Cobs", "DP_AUX_RexErrorRecovery")
            tu_thread.Start(icmd_obj)
        except:
            error_msg = "{}: {}: Error occurred when clicking on rex tu button". \
                    format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_msg)


    def addComment_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Add user comment to log". \
                            format(self.dev.port_name, self.dev.device_name))
            testWindow = usr_comment.add_comment(self.dev)
            testWindow.run()
        except Exception as e:
            cbs_logger.exception("{}: {}: Got an error when opening test window : {}". \
                                 format(self.dev.port_name, self.dev.device_name, e))

    def AutoClearOutputText(self):
        try:
            if self.AutoClearOutput.Checked:
                
                if len(self.OutputTextBox.Lines) > self.dev.OutputLineLimit:
                    cbs_logger.info("{}: {}: Clearing Output". \
                        format(self.dev.port_name, self.dev.device_name))

                    self.OutputTextBox.Clear()
        except:
            cbs_logger.exception("{]: {}: Got an error when Auto-Clearing Output". \
                    format(self.dev.port_name, self.dev.device_name))
					
    def autoClearOutput_CheckedChanged(self, sender, args):
            cbs_logger.info("{}: {}: autoClearOutput is clicked boxChecked={}". \
                    format(self.dev.port_name, self.dev.device_name, self.AutoClearOutput.Checked))	
					
    def ExportLogButton_Click(self, sender, args):
        try:
            cbs_logger.info("Exporting Device Logs")
            path = "Log\\ExportedLogs_" + str(datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S"))
            os.mkdir(path)
            for (client_name, port_name) in self.dev.cobs.device_clients:
                device = self.dev.cobs.device_clients[(client_name, port_name)]
                f= open(path + "\\{}_{}_Logs".format(port_name, client_name) + ".log", "w+")
                f.write(device.devWindow.OutputTextBox.Text.ToString())
                f.close()
            os.startfile(path)
        except:
            cbs_logger.exception("{}: {}: Got an error when exporting device logs". \
                    format(self.dev.port_name, self.dev.device_name))

    def iLogFilterChange(self, sender, args):
        try:
            ifltr_window = iLog_Filter.iLogFilterWindow(self.dev)
            self.dev.deviceFilter = ifltr_window
            self.dev.deviceFilter.run(self.dev.FilterIndices)
        except:
            cbs_logger.info("Error filtering logs")
            
    # def OpenLogButton_Click(self):
    #     try:
    #         cbs_logger.info("Opening current Log directory")
    #         os.startfile("Log\\{}".format(self.dev.cobs.MainLogPath))
    #     except:
    #         cbs_logger.info("Failed to open current Log directory")
            
    def InputDropDownMenuChanged(self, sender, args):
        try:   
            if self.InputDropDownMenu.Text is not "":
                menuSwitch = {
                    "Send": self.SendButton_Click,
                    "Clear": self.ClearInputButton_Click,
                    "Save...": self.SaveButton_Click,
                    "Load...": self.LoadButton_Click
                }
                func = menuSwitch[self.InputDropDownMenu.Text]
                func()
        except:
            cbs_logger.info("{}: {}: Error when selecting Input Control". \
                    format(self.dev.port_name, self.dev.device_name))
                            
    def run(self):
        """Runs the DeviceWindow."""
        try:
            Forms.Application.Run(self)
        except:
            # gracefully exit the program if an unhandled exception is thrown
            # write exception info to error log
            error_message = "{}: {}: Run device window got an exception". \
                                format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

    def exit(self):
        Forms.Application.ExitThread()

class LogOutput():
    def __init__(self, string, color=Drawing.Color.Black, timestamped=False, newline=False):
        self.string = string
        self.color = color
        self.timestamped = timestamped
        self.newline = newline

    def __str__(self):
        return self.string

    @property
    def color(self):
        return self.color

    @property
    def newline(self):
        return self.newline

    @property
    def timestamped(self):
        return self.timestamped
