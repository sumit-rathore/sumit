import Device
import icron_lib.serial_port_manager as spm
import icron_lib.icron_file_parser as ifp
import shutil
import os
import logging
import datetime
import threading 
import sys
from cobs_logger import cbs_logger
import icron_device_logger as idl

# add contrib/ to path
sys.path.append(os.path.join(os.getcwd(), "contrib"))

# needed for the Python interpreter; this file is in the current directory
import clr
clr.AddReferenceToFile("IronPython.dll")

from IronPython.Compiler import CallTarget0 #testing
from IronPython.Hosting import Python

import System
import System.Windows.Forms as Forms
import System.Threading as Threading
from System.Drawing import Point, Size, Font, Icon
from Microsoft.Scripting import SourceCodeKind
from Microsoft.Scripting.Hosting import ExceptionOperations, ScriptEngine, ScriptSource
import System.Array as Array

cobs_version = "1.1.3 release - June 25, 2019"
# cobs_version = "DP COBS - 30 April 2018"

# save current path
programPath = os.path.dirname(os.path.realpath(__file__))
class CobsInterpreter():
    """A Python interpreter for the main Cobs program."""

    Calvin = Python.CreateEngine() # the Python interpreter
    CalvinScope = Calvin.CreateScope() # the Python interpreter's scope

    def __init__(self):
        # the dictionary of port names corresponding to device clients
        self.device_clients = {}
        self.ports = {}
        self.icronTimeStamp = None
        self.iparsed_file = None
        self.loggers = {}
        self.device_dict = {}
        self.rex_suspend_logs = False
        self.lex_suspend_logs = False
        self.log_client_name = {}
        self.log_port_name = {}
        self.projects = None
	
        # a lock is required for removing device as both GE and BB can excute cobs.remove_device
        # at the same time to avoid a race condition
        self.remove_device_lock = threading.Lock()

        """Starts the main Cobs interpreter."""
        cbs_logger.info(
                    "Cobs " + cobs_version + " on IronPython " + \
                    self.Calvin.LanguageVersion.ToString() + " on .NET " + \
                    System.Environment.Version.ToString())

        self.CalvinScope.SetVariable("cobs", self)

        # execute startup script
        if System.IO.File.Exists("StartupScript.py"):
            cbs_logger.info("Executing startup script")
            scriptSource = self.Calvin.CreateScriptSourceFromFile(
                            "StartupScript.py")
            scriptSource.Execute(self.CalvinScope)
            # clean up
            del(scriptSource)
			
        self.MainLogPath = "IcronLogs_" + str(datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S"))
        os.mkdir("Log\\" + self.MainLogPath)
		
        self.startupWindow = StartupWindow(self)
        self.start_thread = Threading.Thread(Threading.ThreadStart(self.startupWindow.run))
        self.start_thread.SetApartmentState(Threading.ApartmentState.STA)
        self.start_thread.Name = "StartupWindow"

    def run(self):
        # create and start StartupWindow
        self.start_thread.Start()

        # start interpreting in an infinite loop
        while True:
            # compile input
            cobsSource = self.Calvin.CreateScriptSourceFromString(
                                            raw_input(), SourceCodeKind.Statements)
            try:
                cobsSource.Execute(self.CalvinScope)
            except SystemExit:
                os._exit(0)
            except:
                # any exception will be shown on the console Python-style
                cbs_logger.exception("Cobs Interpreter got an exception")

    def add_device_client(self, icron_file_path, port_name, client_name):
        """Create a serial port manager if it doesn't exist and construct a new device client.

        Arguments: icron_file_path - path to icron file
                   port_name - name of the port the device is connected to
                   client_name - name of the device client
        """
        try:
            # ensure one serial port manager is only created for one serial port
            if not port_name in self.ports:
                serial_port_manager = spm.SerialPortManager(port_name)
                self.ports[port_name] = serial_port_manager
            else:
                serial_port_manager = self.ports[port_name]

            # ensure the device client is not duplicated
            if not (client_name, port_name) in self.device_clients:

                # create logger
                
                self.loggers[(client_name, port_name)] = idl.IcronDeviceLogger("{}\{}_{}".format(self.MainLogPath, port_name, client_name))
                device = Device.DeviceClient(programPath, self, serial_port_manager, client_name)
                device.load_icron_model(self.iparsed_file, icron_file_path)
                self.log_client_name = client_name
                self.log_port_name = port_name
                # point the interpreter and interpreter scope to the correct places
                device.interpreter = self.Calvin
                device.interpreterScope = self.CalvinScope
                device.launch_device_window()
                device.connect()
                self.device_clients[(client_name, port_name)] = device
                cbs_logger.info("{}: {}: Device is created".format(port_name, client_name))

            else:
                Forms.MessageBox.Show(
                        "Device client {} already exists at {}".format(client_name, port_name),
                        "WARNING",
                        Forms.MessageBoxButtons.OK,
                        Forms.MessageBoxIcon.Warning)
        except:
            error_msg = "Failed to add device client {} to {}.".format(client_name, port_name)
            cbs_logger.exception(error_msg)
            Forms.MessageBox.Show(
                                error_msg,
                                "ERROR",
                                Forms.MessageBoxButtons.OK,
                                Forms.MessageBoxIcon.Error)

    def remove_device(self, port_name, device_name):
        """
        Remove a Device from the main interpreter's devices.

        Argument: port - string of Device's serial port name to remove
        """
        #TODO: break device client to device client controller, model and view, so we can just
        #call device_controller.remove_device() to remove serial port client
        with self.remove_device_lock:
            try:
                # attempt to get a reference to the device to remove
                device_to_remove = self.device_clients.get((device_name, port_name))

                if device_to_remove is None:
                    # occurs when port is not in self.device_clients.keys(). When a user closes all
                    # windows, this will probably take place as cobs.remove_device() in GE and BB
                    # and exit() get executed at the same time.
                    cbs_logger.warning("No device client {} found to be removed from port {}." \
                            .format(device_name, port_name))
                else:
                    # TODO: When close serial port, Microsoft Bug here causes
                    # ObjectDisposedException Needs workaround. still true?
                    device_to_remove.serial_port_manager.remove_listener(device_name)

                    # remove the device client object
                    del self.device_clients[(device_name, port_name)]
                    cbs_logger.info("Device client {} connected to {} is removed". \
                            format(device_name, port_name))

                    # remove serial port manager if no device is connected to the port
                    if device_to_remove.serial_port_manager.num_listeners == 0:
                        del self.ports[port_name]
                        cbs_logger.info("{} serial port manager is removed".format(port_name))

                    # close the device window of the device client to remove and exit the thread
                    if not device_to_remove.devWindow.IsDisposed:
                        device_to_remove.dispose_window()
                        device_to_remove.exit()
            except:
                cbs_logger.exception("Failed to remove device client {} at port {}" \
                        .format(device_name, port_name))

    def load_icron_file(self, icron_file):
        """Load .icron file from the specified path and extract json objects from the .icron file.

        argument: icron_file_path - the path of the .icron file.
        """
        try:
           
            self.iparsed_file = ifp.IcronParsedFile(icron_file, programPath + "//bsdtar.exe")
            cbs_logger.info("Icron file is loaded {}".format(icron_file))
            return self.iparsed_file.projects
        except:
            error_message = "Loading icron file failed."
            cbs_logger.exception(error_message)
            Forms.MessageBox.Show(
                    error_message,
                    "ERROR",
                    Forms.MessageBoxButtons.OK,
                    Forms.MessageBoxIcon.Error)
            return None

    def exit(self):
        """Close device client windows and serial ports then exits Cobs."""
        try:
            for (device_name, port_name), device_obj in self.device_clients.items():
                self.remove_device(port_name, device_name)

            del self.ports
            del self.device_clients

            self.startupWindow.exit()
            cbs_logger.info("Cobs exits")
            # exit immediately
            os._exit(0)
        except:
            cbs_logger.exception("Got an error when exiting")

class StartupWindow(Forms.Form):
    """Allows easier creation of Devices (for now)."""
    def __init__(self, cobs):
        """Creates the StartupWindow's components.

        Arguments: cobs - cobs interpreter object"""
        # boring boring boring
        self.cobs = cobs
        self.newpath = None
        self.suspend_pgmBB_logs = None
        self.CobsPictureBox = Forms.PictureBox()
        self.StartPosition = Forms.FormStartPosition.CenterScreen
        self.icronFileButton = Forms.Button()
        self.DeviceLabel = Forms.Label()
        self.DeviceComboBox = Forms.ComboBox()
        self.DeviceRefreshButton = Forms.Button()
        self.icronFileLabel = Forms.Label()
        self.versionLabel = Forms.Label()
        self.statusLabel = Forms.Label()
        self.icronFileDialog = Forms.OpenFileDialog()
        # self.loadFileButton = Forms.Button()
        self.addDeviceButton = Forms.Button()
        self.icronFileTextBox = Forms.TextBox()
        self.changeFileButton = Forms.Button()
        self.programDropDownMenu = Forms.ComboBox()

        # CobsPictureBox
        self.CobsPictureBox.Image = System.Drawing.Image.FromFile("cobs_icon.jpg")
        self.CobsPictureBox.Location = Point(62, 12)
        self.CobsPictureBox.Size = Size(268, 268)

        # icronFileButton
        self.icronFileButton.Location = Point(305, 385)
        self.icronFileButton.Size = Size(75, 21)
        self.icronFileButton.Text = "Open..."
        self.icronFileButton.Click += self.icronFileButton_Click

        #loadFileButton
        # self.loadFileButton.Location = Point(305, 410)
        # self.loadFileButton.Size = Size(75, 21)
        # self.loadFileButton.Text = "Load File"
        # self.loadFileButton.Click += self.loadFileButton_Click

        # DeviceLabel
        self.DeviceLabel.Location = Point(9, 351)
        self.DeviceLabel.Size = Size(44, 13)
        self.DeviceLabel.Text = "Device:"

        # DeviceComboBox
        self.DeviceComboBox.Location = Point(59, 348)
        self.DeviceComboBox.Size = Size(60, 21)
        # possibly exciting here, where the combo box is populated with ports
        for port in System.IO.Ports.SerialPort.GetPortNames():
            self.DeviceComboBox.Items.Add(port)
	#programtypecombobox
        self.programDropDownMenu.Location = Point(9, 436)
        self.programDropDownMenu.AutoSize = True
        self.programDropDownMenu.DropDownStyle = Forms.ComboBoxStyle.DropDownList;
        # possibly exciting here, where the combo box is populated with ports
        ProgramOptions = ["LogWhileProgramming", "NoLogWhileProgramming"]
        self.programDropDownMenu.Items.AddRange(Array[str](ProgramOptions))     
        self.programDropDownMenu.DropDownClosed += self.programDropDownMenuChanged

        # DeviceRefreshButton
        self.DeviceRefreshButton.Location = Point(125, 348)
        self.DeviceRefreshButton.Size = Size(79, 21)
        self.DeviceRefreshButton.Text = "Refresh"
        self.DeviceRefreshButton.Click += self.DeviceRefreshButton_Click

        # icronFileLabel
        self.icronFileLabel.Location = Point(12, 389)
        self.icronFileLabel.Size = Size(55, 13)
        self.icronFileLabel.Text = ".icron File:"

        # versionLabel
        self.versionLabel.AutoSize = True
        self.versionLabel.Font = Font("Garamond", 16)
        self.versionLabel.Location = Point(7, 298)
        self.versionLabel.Size = Size(184, 30)
        self.versionLabel.Text = "Cobs " + cobs_version

        # icronFileDialog
        self.icronFileDialog.Filter = ".icron files (*.icron)|*.icron|" + \
                                      "All files (*.*)|*.*"

        # addDeviceButton
        self.addDeviceButton.Location = Point(283, 436)
        self.addDeviceButton.Size = Size(97, 21)
        self.addDeviceButton.Text = "Add Device"
        self.addDeviceButton.Click += self.addDeviceButton_Click

        # icronFileTextBox
        self.icronFileTextBox.Location = Point(73, 385)
        self.icronFileTextBox.Multiline = True
        self.icronFileTextBox.ScrollBars = Forms.ScrollBars.Vertical
        self.icronFileTextBox.Size = Size(226, 45)

        # changeFileButton
        self.changeFileButton.Location = Point(171, 436)
        self.changeFileButton.Size = Size(106, 21)
        self.changeFileButton.Text = "Change .icron file"
        self.changeFileButton.Click += self.changeFileButton_Click

        # StartupWindow
        self.ClientSize = Size(392, 468)
        self.Controls.Add(self.changeFileButton)
        self.Controls.Add(self.icronFileTextBox)
        self.Controls.Add(self.addDeviceButton)
        self.Controls.Add(self.versionLabel)
        self.Controls.Add(self.icronFileLabel)
        self.Controls.Add(self.DeviceRefreshButton)
        self.Controls.Add(self.DeviceComboBox)
        self.Controls.Add(self.DeviceLabel)
        self.Controls.Add(self.icronFileButton)
        # self.Controls.Add(self.loadFileButton)
        self.Controls.Add(self.CobsPictureBox)
        self.Controls.Add(self.programDropDownMenu)
        self.FormBorderStyle = Forms.FormBorderStyle.FixedSingle
        self.MaximizeBox = False
        self.Text = "Cobs"
        self.Icon = Icon("cobsicon.ico")
        self.Closed += self.StartupWindow_Closed

    def load_file(self):
        self.Invoke(CallTarget0(lambda: self.update_title("Cobs - Loading Icron File {}". \
                                                          format(self.newPath))))
        if self.newPath != "":
            self.cobs.projects = self.cobs.load_icron_file(self.newPath)
            self.addDeviceButton.Enabled = True
            thread_list = []
            if len(self.cobs.device_clients) is not 0:
                for (device_name, port_name), device_client_obj in self.cobs.device_clients.items():
                    change_icron_thread = threading.Thread(target=lambda obj=device_client_obj: obj.devWindow.update_device_window())
                    change_icron_thread.start()
                    thread_list.append(change_icron_thread)
                for t in thread_list:
                    t.join()
            else:
                error_message = "No device window open yet"
                cbs_logger.exception(error_message)
                Forms.MessageBox.Show(
                    error_message,
                    "ERROR",
                    Forms.MessageBoxButtons.OK,
                    Forms.MessageBoxIcon.Error)
        else:
            error_message = "Need to choose .icron file"
            cbs_logger.exception(error_message)
            Forms.MessageBox.Show(
                error_message,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)
        self.Invoke(CallTarget0(lambda: self.update_title("Cobs")))

    def icronFileButton_Click(self, sender, args):
        """Opens a dialog allowing the user to choose a .icron file to load.

        Arguments: sender - unused
                   args - unused"""
        try:
            self.addDeviceButton.Enabled = False
            cbs_logger.info("Open Icron file button is clicked")
            # show dialog, save chosen file name if the user didn't cancel
            if self.icronFileDialog.ShowDialog() == Forms.DialogResult.OK:
                self.icronFileTextBox.Text = self.icronFileDialog.FileName
                source = self.icronFileTextBox.Text
                dest = os.path.join(os.getcwd(), 'icron_file')
                if not os.path.exists(dest):
                    os.makedirs(dest)
                file_path = os.path.join(dest, os.path.basename(source))
                if os.path.exists(file_path):
                    os.chmod(file_path, 0o777)
                cbs_logger.info(dest)
                self.Invoke(CallTarget0(lambda: self.update_title("Cobs - Downloading Icron File {}". \
                        format(dest))))
                Forms.Cursor.Current = Forms.Cursors.WaitCursor
                self.Invoke(CallTarget0(lambda: shutil.copy(source, dest)))
                Forms.Cursor.Current = Forms.Cursors.AppStarting
                self.Invoke(CallTarget0(lambda: self.update_title("Cobs")))
                self.newPath = os.path.join(dest, os.path.basename(source))
            cbs_logger.info("Icron file path is changed {}".format(self.newPath))

            #Load .icron File
            # cbs_logger.info("Loading icron file")
            thread = Threading.Thread(Threading.ThreadStart(self.load_file))
            thread.SetApartmentState(Threading.ApartmentState.STA)
            thread.Start()
        except Exception as e:
            cbs_logger.info("Error in copying file : {}".format(e))


    # def loadFileButton_Click(self, sender, args):
    #     """Loads a .icron file for an already existing Device.
    #
    #     Arguments: sender - unused
    #                args - unused"""

        # def load_file():
        #     self.Invoke(CallTarget0(lambda: self.update_title("Cobs - Loading Icron File {}". \
        #             format(self.newPath))))
        #     if self.newPath != "":
        #         self.cobs.projects = self.cobs.load_icron_file(self.newPath)
        #         thread_list = []
        #         if len(self.cobs.device_clients) is not 0:
        #             for (device_name, port_name), device_client_obj in self.cobs.device_clients.items():
        #                 change_icron_thread = threading.Thread(
        #                         target=lambda obj=device_client_obj: obj.devWindow.update_device_window())
        #                 change_icron_thread.start()
        #                 thread_list.append(change_icron_thread)
        #             for t in thread_list:
        #                 t.join()
        #         else:
        #             error_message = "No device window open yet"
        #             cbs_logger.exception(error_message)
        #             Forms.MessageBox.Show(
        #                 error_message,
        #                 "ERROR",
        #                 Forms.MessageBoxButtons.OK,
        #                 Forms.MessageBoxIcon.Error)
        #     else:
        #         error_message = "Need to choose .icron file"
        #         cbs_logger.exception(error_message)
        #         Forms.MessageBox.Show(
        #                 error_message,
        #                 "ERROR",
        #                 Forms.MessageBoxButtons.OK,
        #                 Forms.MessageBoxIcon.Error)
        #     self.Invoke(CallTarget0(lambda: self.update_title("Cobs")))
        # try:
        #     cbs_logger.info("Load icron file Button is clicked")
        #     thread = Threading.Thread(Threading.ThreadStart(load_file))
        #     thread.SetApartmentState(Threading.ApartmentState.STA)
        #     thread.Start()
        # except Exception as e:
        #     cbs_logger.exception("Got an error when Load File button is clicked : {}".format(e))

    def DeviceRefreshButton_Click(self, sender, args):
        """Refreshs the list of available serial ports.

        Arguments: sender - unused
                   args - unused"""
        # remove all items from the combo box, then repopulate it
        self.DeviceComboBox.Items.Clear()
        for port in System.IO.Ports.SerialPort.GetPortNames():
            self.DeviceComboBox.Items.Add(port)

    def programDropDownMenuChanged(self, sender, args):
        try:
            if self.programDropDownMenu.Text is not "":
                menuSwitch = {
                    "NoLogWhileProgramming": self.NoLogWhileProgramming_Click,
                    "LogWhileProgramming": self.LogWhileProgramming_Click
                }
                func = menuSwitch[self.programDropDownMenu.Text]
                func()
        except:
            cbs_logger.info("Error when selecting program type Control")

    def LogWhileProgramming_Click(self): 
        cbs_logger.info("LogWhileProgramming BUTTON CLICKED")
        self.suspend_pgmBB_logs = False
        cbs_logger.debug("Log while Programming")
    
    def NoLogWhileProgramming_Click(self):
        cbs_logger.info("NoLogWhileProgramming BUTTON CLICKED")
        self.suspend_pgmBB_logs = True
        cbs_logger.debug("No Log while Programming")
          
    def addDeviceButton_Click(self, sender, args):
        """Loads .icron file and add device client(s).

        Arguments: sender - unused
                   args - unused
        """
        def add_device():
            try:
                # projects = self.cobs.load_icron_file(self.newPath)
                if self.cobs.projects is not None:
                    for project in self.cobs.projects:
                        self.cobs.add_device_client(
                                self.newPath,
                                self.DeviceComboBox.Text,
                                project)
                else:
                    error_message = "Click on Load File Button before adding device"
                    cbs_logger.exception(error_message)
                    Forms.MessageBox.Show(
                        error_message,
                        "ERROR",
                        Forms.MessageBoxButtons.OK,
                        Forms.MessageBoxIcon.Error)
            except:
                cbs_logger.info("Got an error adding Device : {}".format(self.cobs.projects))
        try:
            if self.newPath != "":
                if self.DeviceComboBox.Text != "":
                    cbs_logger.info("Add Device button is clicked")
                    # self.Invoke(CallTarget0(lambda: self.update_title("Cobs - Loading Icron File {}". \
                    #             format(self.newPath))))
                    Forms.Cursor.Current = Forms.Cursors.WaitCursor
                    self.Invoke(CallTarget0(lambda: add_device()))
                    Forms.Cursor.Current = Forms.Cursors.AppStarting
                    # self.Invoke(CallTarget0(lambda: self.update_title("Cobs")))
                else:
                    error_message = "Need to choose Device"
                    cbs_logger.exception(error_message)
                    Forms.MessageBox.Show(
                            error_message,
                            "ERROR",
                            Forms.MessageBoxButtons.OK,
                            Forms.MessageBoxIcon.Error)
            else:
                error_message = "Need to choose .icron file and Click Load File button"
                cbs_logger.exception(error_message)
                Forms.MessageBox.Show(
                    error_message,
                    "ERROR",
                    Forms.MessageBoxButtons.OK,
                    Forms.MessageBoxIcon.Error)

        except Exception as e:
            cbs_logger.info("Got an error adding Device : {}".format(e))

    def update_title(self, text):
        self.Text = text

    def changeFileButton_Click(self, sender, args):
        """Loads a .icron file for an already existing Device.

        Arguments: sender - unused
                   args - unused"""

        try:
            self.addDeviceButton.Enabled = False
            cbs_logger.info("Open Icron file button is clicked")
            # show dialog, save chosen file name if the user didn't cancel
            if self.icronFileTextBox.Text is not "":
                source = self.icronFileTextBox.Text
                dest = os.path.join(os.getcwd(), 'icron_file')
                if not os.path.exists(dest):
                    os.makedirs(dest)
                file_path = os.path.join(dest, os.path.basename(source))
                if os.path.exists(file_path):
                    os.chmod(file_path, 0o777)
                cbs_logger.info(dest)
                self.Invoke(CallTarget0(lambda: self.update_title("Cobs - Downloading Icron File {}". \
                        format(dest))))
                Forms.Cursor.Current = Forms.Cursors.WaitCursor
                self.Invoke(CallTarget0(lambda: shutil.copy(source, dest)))
                Forms.Cursor.Current = Forms.Cursors.AppStarting
                self.Invoke(CallTarget0(lambda: self.update_title("Cobs")))
                self.newPath = os.path.join(dest, os.path.basename(source))
                # Load .icron File
                thread = Threading.Thread(Threading.ThreadStart(self.load_file))
                thread.SetApartmentState(Threading.ApartmentState.STA)
                thread.Start()
            else:
                cbs_logger.error("Choose .icron file first by clicking on Open...")


        except Exception as e:
            cbs_logger.info("Error in copying file : {}".format(e))


    def StartupWindow_Closed(self, sender, args):
        """Closes the entire Cobs program."""
        self.cobs.exit()

    def execute_code(self, code):
        """Executes code using the CobsInterpreter's Python interpreter."""
        # note this is nearly identical to the execute_code() function in the
        #     Device module
        # compile code, and attempt to execute
        codeSource = CobsInterpreter.Calvin.CreateScriptSourceFromString(
                     code, SourceCodeKind.Statements)
        try:
            codeSource.Execute(CobsInterpreter.CalvinScope)
        except Exception as ex:
            # show the error that's occurred; this is quite similar to the
            #     exception handling in CobsInterpreter
            eo = CobsInterpreter.Calvin.GetService[ExceptionOperations]()
            error = eo.FormatException(ex)
            MessageBox.Show(error)

    def run(self):
        """Runs the StartupWindow."""
        try:
            System.Windows.Forms.Application.Run(self)
        except:
            # gracefully exit the program if an unhandled exception is thrown
            # write exception info to error log
            cbs_logger.exception("Startup window got an exception")
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

    def exit(self):
        try:
            Forms.Application.Exit()
        except:
            cbs_logger.exception("Got an exception when exit startup window")

