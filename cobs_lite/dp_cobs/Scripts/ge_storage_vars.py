#Create window writing different flash values
import System.Drawing
import System.Windows.Forms

class VariableSpec():
    def __init__(self, label, enumValue, description):
        self.label = label
        self.enumValue = enumValue
        self.description = description

class flashVarWindow():
    #displays a window to select and modify a variable
    def __init__(self, dev):
        self.form = Form()
        self.form.Size = Size(550, 350)
        self.form.FormBorderStyle = FormBorderStyle.FixedDialog
        self.dev = dev
        self.form.Text = "Flash Variable - " + dev.serialPort.PortName
        self.numBytes = 0

        # NOTE: this should be updated after any change to flash_vars.h
        self.flashVariables = [
            VariableSpec(
                'MAC_ADDR', 0,
                '6 bytes packed in the upper 6 bytes of the word.  The MAC ' \
                'address should typically start with the Icron OUI 0x001b13'),
            VariableSpec(
                'CONFIGURATION_BITS', 1,
                'A series of bitfields representing capabilities to ' \
                'enable/disable.  Treat the variable as a 64 bit integer ' \
                'with the first bit listed being in the least significant ' \
                'bit of the 64 bit value.  Bit definitions:\r\n' \
                '0 :SUPPORT_USB2_HISPEED\r\n' \
                '1 :SUPPORT_MSA\r\n' \
                '2 :ALLOW_ISO_DEVICES\r\n' \
                '3 :USE_ETHERNET_FRAMING\r\n' \
                '4 :ENET_PHY_MII_SUPPORT\r\n' \
                '5 :ENET_PHY_GMII_SUPPORT\r\n' \
                '6 :USE_BCAST_NET_CFG_PROTO\r\n' \
                '7 :ENABLE_VHUB\r\n' \
                '8 :ENABLE_DCF_OFFSET\r\n' \
                '9 :BLOCK_MASS_STORAGE\r\n' \
                '10:BLOCK_ALL_BUT_HID_AND_HUB\r\n' \
                '11:DEPRECATED_REFUSE_PAIRING_WITH_UNBRANDED\r\n' \
                '12:DEPRECATED_REFUSE_PAIRING_WITH_LEGACY\r\n' \
                '13:ENABLE_LEX_EXTERNAL_CLOCK\r\n' \
                '14:DISABLE_REX_EXTERNAL_CLOCK\r\n' \
                '15:BLOCK_ALL_BUT_HID_HUB_AND_SMARTCARD\r\n'\
                '16:BLOCK_ALL_BUT_AUDIO_AND_VENDOR_SPECIFIC\r\n'\
                '17:DISABLE_NETWORK_CFG_CHANGES\r\n'),
            VariableSpec(
                'REX_PAIRED_MAC_ADDR/LEX_VPORT1_PAIRED_MAC_ADDR', 2,
                '6 bytes packed in the upper 6 bytes of the word.  The ' \
                'paired MAC address should typically start with the Icron OUI ' \
                '0x001b13.'),
            VariableSpec(
                'LEX_VPORT2_PAIRED_MAC_ADDR', 3,
                '6 bytes packed in the upper 6 bytes of the word.  The ' \
                'paired MAC address should typically start with the Icron OUI ' \
                '0x001b13.'),
            VariableSpec(
                'LEX_VPORT3_PAIRED_MAC_ADDR', 4,
                '6 bytes packed in the upper 6 bytes of the word.  The ' \
                'paired MAC address should typically start with the Icron OUI ' \
                '0x001b13.'),
            VariableSpec(
                'LEX_VPORT4_PAIRED_MAC_ADDR', 5,
                '6 bytes packed in the upper 6 bytes of the word.  The ' \
                'paired MAC address should typically start with the Icron OUI ' \
                '0x001b13.'),
            VariableSpec(
                'LEX_VPORT5_PAIRED_MAC_ADDR', 6,
                '6 bytes packed in the upper 6 bytes of the word.  The ' \
                'paired MAC address should typically start with the Icron OUI ' \
                '0x001b13.'),
            VariableSpec(
                'LEX_VPORT6_PAIRED_MAC_ADDR', 7,
                '6 bytes packed in the upper 6 bytes of the word.  The ' \
                'paired MAC address should typically start with the Icron OUI ' \
                '0x001b13.'),
            VariableSpec(
                'LEX_VPORT7_PAIRED_MAC_ADDR', 8,
                '6 bytes packed in the upper 6 bytes of the word.  The ' \
                'paired MAC address should typically start with the Icron OUI ' \
                '0x001b13.'),
            VariableSpec(
                'NETWORK_ACQUISITION_MODE', 9,
                'Set to 0 for DHCP, set to 1 for static IP'),
            VariableSpec(
                'IP_ADDR', 10,
                'The IP address of this device.  Used for static IP settings'),
            VariableSpec(
                'SUBNET_MASK', 11,
                'The subnet mask for this device.  Used for static IP settings'),
            VariableSpec(
                'DEFAULT_GATEWAY', 12,
                'The gateway IP address for this device.  Used for static IP settings'),
            VariableSpec(
                'DHCP_SERVER_IP', 13,
                'The DHCP Server IP address.'),
            VariableSpec(
                'UNIT_BRAND_0', 14,
                'The brand of the unit.  Bytes 0 and 1 are a 16-bit numeric brand identifier in ' \
                'big endian format.  Bytes 2 through 7 are the first component of the utf-8 ' \
                'encoded string which represents the brand string.'),
            VariableSpec(
                'NETCFG_PORT_NUMBER', 15,
                'The port number to listen on for the network configuration messages'),
            VariableSpec(
                'UNIT_BRAND_1', 16,
                'Continuation of the brand string.'),
            VariableSpec(
                'UNIT_BRAND_2', 17,
                'Continuation of the brand string.'),
            VariableSpec(
                'UNIT_BRAND_3', 18,
                'Continuation of the brand string.'),
            VariableSpec(
                'PSEUDO_RANDOM_SEED', 19,
                'Pseudo Random seed used by the random number generator'),
            VariableSpec(
                'VHUB_CONFIGURATION', 20,
                'Vhub Configuration options:\r\n'
                'Bytes[0] bits 2:0  - # of downstream ports\r\n'
                'Bytes[2], Bytes[3] - VID in big endian\r\n'
                'Bytes[4], Bytes[5] - PID in big endian'),
            ]

        tableLayout = TableLayoutPanel()
        tableLayout.ColumnCount = 2
        tableLayout.RowCount = 4
        tableLayout.Padding = Padding(1, 10, 1, 10)
        tableLayout.AutoSize = True

        # Row 0
        varLabel = Label()
        varLabel.Text = "Select variable to modify:"
        varLabel.AutoSize = True
        varLabel.Anchor = AnchorStyles.Left
        tableLayout.Controls.Add(varLabel, 0, 0)

        self.varComboBox = ComboBox()
        self.varComboBox.Items.AddRange(Array[str]([v.label for v in self.flashVariables]))
        self.varComboBox.DropDownStyle = ComboBoxStyle.DropDownList
        self.varComboBox.SelectedIndexChanged += self.varComboBox_SelectedIndexChanged
        self.varComboBox.Width = 200
        self.selectedIndex = -1
        tableLayout.Controls.Add(self.varComboBox, 1, 0)


        # Row 1
        descriptionLabel = Label()
        descriptionLabel.Text = "Variable description:"
        descriptionLabel.AutoSize = True
        descriptionLabel.Anchor = AnchorStyles.Left
        tableLayout.Controls.Add(descriptionLabel, 0, 1)

        self.descriptionText = TextBox()
        self.descriptionText.Text = ''
        self.descriptionText.Width = 400
        self.descriptionText.Height = 200
        self.descriptionText.ScrollBars = ScrollBars.Vertical;
        self.descriptionText.Multiline = True
        self.descriptionText.WordWrap = True
        self.descriptionText.ReadOnly = True
        tableLayout.Controls.Add(self.descriptionText, 1, 1)


        # Row 2
        dataLabel = Label()
        dataLabel.Text = "Enter data to write:  0x"
        dataLabel.AutoSize = True
        dataLabel.Anchor = AnchorStyles.Left
        tableLayout.Controls.Add(dataLabel, 0, 2)

        dataAndWriteButtonPnl = FlowLayoutPanel()
        dataAndWriteButtonPnl.AutoSize = True
        self.dataTextBox = TextBox()
        self.dataTextBox.MaxLength = 8 * 2
        self.dataTextBox.Width = 150
        self.dataTextBox.AutoSize = True
        dataAndWriteButtonPnl.Controls.Add(self.dataTextBox)

        writeButton = Button()
        writeButton.Text = "Write"
        writeButton.Click += self.writeButton_Clicked
        writeButton.AutoSize = True
        dataAndWriteButtonPnl.Controls.Add(writeButton)

        tableLayout.Controls.Add(dataAndWriteButtonPnl, 1, 2)

        # Row 3
        outputLabel = Label()
        outputLabel.Text = 'Command Executed:'
        outputLabel.AutoSize = True
        outputLabel.Anchor = AnchorStyles.Left
        tableLayout.Controls.Add(outputLabel, 0, 3)

        self.outputText = Label()
        self.outputText.Text = ''
        self.outputText.AutoSize = True
        tableLayout.Controls.Add(self.outputText, 1, 3)


        # Final initialization
        self.form.Controls.Add(tableLayout)
        Application.Run(self.form)


    def varComboBox_SelectedIndexChanged(self, sender, args):
        self.selectedIndex = self.varComboBox.SelectedIndex
        self.dataTextBox.Clear()
        self.outputText.Text = ''
        self.descriptionText.Text = self.flashVariables[self.selectedIndex].description


    def writeButton_Clicked(self, sender, args):
        #only send command if an item has been selected
        if self.selectedIndex != -1:
            tmpData = self.dataTextBox.Text.ljust(8*2, '0') # 8 is for num of bytes, 2 is for 2 digits hex per byte
            data0 = '0x' + tmpData[0:8]
            data1 = '0x' + tmpData[8:16]
            component = 'FLASH_DATA_COMPONENT'
            command = 'FLASH_icmdWriteVar'
            if ('STORAGE_COMPONENT' in self.dev.iCmds):
                component = 'STORAGE_COMPONENT'
                command = 'STORAGE_icmdWriteVar'
            try:
                # int(string, 0) converts hex string to hex integer
                self.dev.iCmds[component][command]\
                    (self.flashVariables[self.selectedIndex].enumValue, int(data0, 0), int(data1, 0))
            except Exception as ex:
                MessageBox.Show("Could not execute the flash component command.\n" +
                        "Please enter the data in the textbox")
                return

#start script
flashVarWindow(currentDevice)
