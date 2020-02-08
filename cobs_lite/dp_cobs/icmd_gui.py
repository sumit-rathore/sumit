import clr
clr.AddReference("System.Drawing")
clr.AddReference("System.Windows.Forms")

import System
import System.Array as Array
import System.Windows.Forms as Forms

from cobs_logger import cbs_logger

class iCommandWindow(Forms.Form):
    def __init__(self, dev):
        self.dev = dev

        self.component_name = None
        self.function_name = None
        self.arguments = []

        # create all the other objects; they aren't in a nice table like the above controls so
        # introspection won't be as helpful
        self.MainContainer = Forms.SplitContainer()
        self.ListBoxesContainer = Forms.SplitContainer()
        self.ArgsViewContainer = Forms.SplitContainer()
        self.iComponentsLabel = Forms.Label()
        self.iComponentsListBox = Forms.ListBox()
        self.functionsListBox = Forms.ListBox()
        self.functionsLabel = Forms.Label()
        self.codeTextBox = Forms.TextBox()
        self.codeLabel = Forms.Label()
        self.executeButton = Forms.Button()
        self.helpLabel = Forms.Label()
        self.helpTextBox = Forms.TextBox()
        self.textLabel = Forms.Label()
        self.arg_widget_list = []

        self.MainContainer.SuspendLayout()
        self.ListBoxesContainer.SuspendLayout()
        self.ArgsViewContainer.SuspendLayout()
        self.SuspendLayout()

        #Argument box and label layout
        for arg_index in range(6):
            arg_widget = {}
            # initialize argument label
            arg_label = Forms.Label()
            # disable argument label by default
            arg_label.Enabled = False
            arg_label.Visible = False
            arg_label.AutoSize = True
            arg_label.Text = "Argument" + str(arg_index) + ":"
            arg_widget['arg_label'] = arg_label
            arg_label.Location = System.Drawing.Point(0, arg_index*40)
            self.ArgsViewContainer.Panel1.Controls.Add(arg_label)

            # initialize argument text_box
            text_box = Forms.TextBox()
            # disable textbox by default
            text_box.Enabled = False 
            text_box.Visible = False
            text_box.Size = System.Drawing.Size(185, 20)
            text_box.TextChanged += self.argTextBox_TextChanged
            arg_widget['text_box'] = text_box
            text_box.Location = System.Drawing.Point(65, (arg_index*40))
            self.ArgsViewContainer.Panel1.Controls.Add(text_box)

            #initialize argument combo box
            combo_box = Forms.ComboBox()
            # disable combo box by default
            combo_box.Enabled = False 
            combo_box.Visible = False
            combo_box.Size = System.Drawing.Size(185, 20)
            combo_box.TextChanged += self.argTextBox_TextChanged
            arg_widget['combo_box'] = combo_box
            combo_box.Location = System.Drawing.Point(65, (arg_index*40))
            self.ArgsViewContainer.Panel1.Controls.Add(combo_box)

            type_label = Forms.Label()
            # disable type label by default
            type_label.Enabled = False
            type_label.Visible = False
            type_label.AutoSize = True
            type_label.Text = "Type: "
            arg_widget['type_label'] = type_label
            # append the arg_widget dictionary to the list
            self.arg_widget_list.append(arg_widget)
            type_label.Location = System.Drawing.Point(255, (arg_index*40))
            self.ArgsViewContainer.Panel1.Controls.Add(type_label)
            

        # iComponentsLabel
        self.iComponentsLabel.Location = System.Drawing.Point(0, 5)
        self.iComponentsLabel.Size = System.Drawing.Size(71, 13)
        self.iComponentsLabel.Text = "iComponents:"

        # iComponentsListBox
        self.iComponentsListBox.Dock = Forms.DockStyle.Fill
        self.iComponentsListBox.Size = System.Drawing.Size(177, 355)
        self.iComponentsListBox.SelectedIndexChanged += \
            self.iComponentsListBox_SelectedIndexChanged

        # functionsListBox
        self.functionsListBox.Dock = Forms.DockStyle.Fill
        self.functionsListBox.Size = System.Drawing.Size(205, 355)
        self.functionsListBox.SelectedIndexChanged += \
            self.functionsListBox_SelectedIndexChanged
        self.functionsListBox.BorderStyle = Forms.BorderStyle.Fixed3D

        # functionsLabel
        self.functionsLabel.Location = System.Drawing.Point(0, 5)
        self.functionsLabel.Size = System.Drawing.Size(56, 13)
        self.functionsLabel.Text = "Functions:"
        self.functionsLabel.Margin = Forms.Padding(15)

        # executeButton
        self.executeButton.Size = System.Drawing.Size(120, 28)
        self.executeButton.Anchor = Forms.AnchorStyles.Bottom
        self.executeButton.Text = "Execute"
        self.executeButton.Enabled = False
        self.executeButton.Visible = False
        self.executeButton.Click += self.executeButton_Clicked

       # codeTextBox
        self.codeTextBox.Size = System.Drawing.Size(127, 60)
        self.codeTextBox.Location = System.Drawing.Point(0, 110)
        self.codeTextBox.Anchor = (Forms.AnchorStyles.Right | Forms.AnchorStyles.Left |
                Forms.AnchorStyles.Top)
        self.codeTextBox.BringToFront()
        self.codeTextBox.ReadOnly = True
        self.codeTextBox.Multiline = True
        self.codeTextBox.AutoSize = True
        self.codeTextBox.Enabled = False
        self.codeTextBox.Visible = False

        # codeLabel
        self.codeLabel.Text = "Code:"
        self.codeLabel.Location = System.Drawing.Point(0, 90)
        self.codeLabel.Anchor = (Forms.AnchorStyles.Left |
                Forms.AnchorStyles.Top)
        self.codeLabel.Enabled = False
        self.codeLabel.Visible = False

        #TODO: change the name
        # helpLabel
        self.helpLabel.Text = "Help:"
        self.helpLabel.Location = System.Drawing.Point(0, 0)
        self.codeLabel.Anchor = (Forms.AnchorStyles.Left |
                Forms.AnchorStyles.Top)
        self.helpLabel.Enabled = False
        self.helpLabel.Visible = False

        # helpTextBox
        self.helpTextBox.Size = System.Drawing.Size(127, 60)
        self.helpTextBox.Location = System.Drawing.Point(0, 20)
        self.helpTextBox.Anchor = (Forms.AnchorStyles.Right | Forms.AnchorStyles.Left |
                Forms.AnchorStyles.Top)
        self.helpTextBox.ReadOnly = True
        self.helpTextBox.Enabled = False
        self.helpTextBox.Visible = False
        self.helpTextBox.AutoSize = True
        self.helpTextBox.Multiline = True

        #ArgsViewContainer
        self.ArgsViewContainer.Dock = Forms.DockStyle.Fill
        self.ArgsViewContainer.Width = 100
        self.ArgsViewContainer.Orientation = Forms.Orientation.Horizontal
        self.ArgsViewContainer.Panel2.Controls.Add(self.helpTextBox)
        self.ArgsViewContainer.Panel2.Controls.Add(self.helpLabel)
        self.ArgsViewContainer.Panel2.Controls.Add(self.codeTextBox)
        self.ArgsViewContainer.Panel2.Controls.Add(self.codeLabel)
        self.ArgsViewContainer.Panel2.Controls.Add(self.executeButton)
        self.ArgsViewContainer.Panel2.Padding = Forms.Padding(1,0,5,0)

        #ListBoxesContainer
        self.ListBoxesContainer.Width = 400
        self.ListBoxesContainer.SplitterDistance = 195
        self.ListBoxesContainer.Panel1.Controls.Add(self.iComponentsLabel)
        self.ListBoxesContainer.Panel1.Controls.Add(self.iComponentsListBox)
        self.ListBoxesContainer.Panel2.Controls.Add(self.functionsLabel)
        self.ListBoxesContainer.Panel2.Controls.Add(self.functionsListBox)
        self.ListBoxesContainer.Dock = Forms.DockStyle.Fill
        self.ListBoxesContainer.BorderStyle = Forms.BorderStyle.Fixed3D
        self.ListBoxesContainer.Panel1.Padding = Forms.Padding(10,20,1,10)
        self.ListBoxesContainer.Panel2.Padding = Forms.Padding(10,20,1,10)

        #MainContainer
        self.MainContainer.Width = 800
        self.MainContainer.SplitterDistance = 420
        self.MainContainer.Panel1.Controls.Add(self.ListBoxesContainer)
        self.MainContainer.Panel2.Controls.Add(self.ArgsViewContainer)
        self.MainContainer.Dock = Forms.DockStyle.Fill
        self.MainContainer.BorderStyle = Forms.BorderStyle.Fixed3D
        self.MainContainer.Panel2.Padding = Forms.Padding(10,5,10,5) 

        # iCommandWindow
        self.ClientSize = System.Drawing.Size(800, 500)
        self.MinimumSize = System.Drawing.Size(200, 500)
        self.Controls.Add(self.MainContainer)

        self.Icon = System.Drawing.Icon(dev.programPath + "\\cobsicon.ico")
        self.Text = self.dev.device_name + ": " + self.dev.port_name + " iCommands - Cobs"
        self.FormBorderStyle = Forms.FormBorderStyle.Sizable
        self.MaximizeBox = True 

        self.MainContainer.ResumeLayout(False)
        self.ListBoxesContainer.ResumeLayout(False)
        self.ArgsViewContainer.ResumeLayout(False)
        self.ResumeLayout()

    def _clear_argument_widgets(self):
        for widget_dic in self.arg_widget_list:
            for widget_name, widget_obj in widget_dic.items():
                widget_obj.Enabled = False
                widget_obj.Visible = False
                if widget_name == 'text_box':
                    widget_obj.Clear()

        self.helpLabel.Visible = False
        self.helpTextBox.Visible = False
        self.codeLabel.Visible = False
        self.codeTextBox.Visible = False
        self.executeButton.Enabled = False
        self.executeButton.Visible = False

    def iComponentsListBox_SelectedIndexChanged(self, sender, args):
        """Populates the function listbox based on the iComponent selected.

        Arguments: sender - unused
                   args - unused"""
        try:
            if self.iComponentsListBox.SelectedItem is not None:
                # clear the argument wigets
                self._clear_argument_widgets()

                # clear the list of functions in the flist box
                self.functionsListBox.Items.Clear()

                self.component_name = self.iComponentsListBox.SelectedItem
                function_list = self.dev.icmd_model.get_icmd_function_names(self.component_name)
                self.functionsListBox.Items.AddRange(Array[str](sorted(function_list)))
        except:
            error_message = "Got an error when select an icmd component"

    def functionsListBox_SelectedIndexChanged(self, sender, args):
        """Enables argument widgets and populates help, code textboxes and excute buttion, based on
        the icmd funtion selected.

        Arguments: sender - unused
                   args - unused"""

        try:
            if self.functionsListBox.SelectedItem is not None:
                # clear/disable all arguments widgets
                self._clear_argument_widgets()

                # get selected icmd function name
                self.function_name = self.functionsListBox.SelectedItem

                # get arguments types
                arg_types = self.dev.icmd_model. \
                        get_icmd_arg_types(self.component_name, self.function_name)

                # enable arguments widgets
                for arg_index, arg_type in enumerate(arg_types):
                    self.arg_widget_list[arg_index]['arg_label'].Enabled = True
                    self.arg_widget_list[arg_index]['arg_label'].Visible = True
                    self.arg_widget_list[arg_index]['type_label'].Enabled = True
                    self.arg_widget_list[arg_index]['type_label'].Visible = True
                    self.arg_widget_list[arg_index]['type_label'].Text= "Type: " + arg_type 
                    widget_type = 'combo_box' if arg_type == 'component_t' else 'text_box'
                    self.arg_widget_list[arg_index][widget_type].Visible = True

                    if widget_type == 'combo_box':
                        self.arg_widget_list[arg_index][widget_type].Items. \
                                AddRange(Array[str](sorted(self.dev.icmd_model.componet_list)))

                    if arg_type != 'void':
                        self.arg_widget_list[arg_index][widget_type].Enabled = True

                # enable help label and text box
                self.helpLabel.Enabled = True
                self.helpLabel.Visible = True
                self.helpTextBox.Enabled= True
                self.helpTextBox.Visible = True
                self.helpTextBox.Text = self.dev.icmd_model. \
                        get_icmd_help_string(self.component_name, self.function_name)

                # enable code text label and text box
                self.codeLabel.Enabled= True
                self.codeLabel.Visible = True
                self.codeTextBox.Enabled = True
                self.codeTextBox.Visible = True
                self.codeTextBox.Text = 'currentDevice.send_icmd(' + \
                                        'currentDevice.create_icmd("{}", "{}", False))'. \
                                            format(self.component_name, self.function_name)

                # enable execute button
                self.executeButton.Enabled = True
                self.executeButton.Visible = True
        except:
            error_message = "Got an error when selecting an icmd function"
            cbs_logger.exception(error_message)

    def argTextBox_TextChanged(self, sender, args):
        """Updates the code textbox's text, based on the argument entered.

        Arguments: sender - unused
                   args - unused"""
        try:
            num_args = self.dev.icmd_model. \
                            get_icmd_num_args(self.component_name, self.function_name)

            if num_args != 0:
                arg_types = self.dev.icmd_model. \
                                get_icmd_arg_types(self.component_name, self.function_name)

                self.arguments = []
                for arg_index, arg_type in enumerate(arg_types):
                    if arg_type == 'component_t':
                        component = self.arg_widget_list[arg_index]['combo_box'].Text
                        arg_value = self.dev.icmd_model.get_icomponent_index(component)
                        self.arguments.append(arg_value)
                    else:
                        arg_value = self.arg_widget_list[arg_index]['text_box'].Text
                        if len(arg_value) != 0:
                            self.arguments.append(arg_value)

                self.codeTextBox.Text = 'currentDevice.send_icmd(' + \
                                        'currentDevice.create_icmd("{}", "{}", False, {}))'. \
                                            format(self.component_name,
                                                    self.function_name,
                                                    self.arguments)
        except:
            error_message = "Got an error when modifying arguments"
            cbs_logger.exception(error_message)

    def executeButton_Clicked(self, sender, args):
        """Executes the code corresponding to the current iCommand.

        Arguments: sender - unused
                   args - unused"""
        try:
            num_args = self.dev.icmd_model. \
                            get_icmd_num_args(self.component_name, self.function_name)
            if num_args == 0:
                icmd = self.dev.create_icmd(self.component_name, self.function_name, False)
            else:
                icmd = self.dev.create_icmd(self.component_name, self.function_name, False, self.arguments)
            self.dev.send_icmd(icmd)
        except:
            error_message = "Got an error when clicking Execute button"
            cbs_logger.exception(error_message)

    def run(self):
        """Runs the iCommandWindow."""
        try:
            # populate iComponentsListBox
            componet_list = sorted(self.dev.icmd_model.componet_list)
            self.iComponentsListBox.Items.AddRange(Array[str](componet_list))

            # show the window
            self.Show()
        except:
            error_message = "Got an error when running icmd windows"
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)
