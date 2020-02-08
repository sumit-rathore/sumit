import System.Drawing
import System.Windows.Forms as Forms
import threading
import clr
clr.AddReference("System.Xml")
from System.Xml import *
from datetime import datetime
from collections import OrderedDict
import logging
import os
import Scripts.ipxact_xml as ipx
from Scripts.ipxact_tree import IPXTree

# save current path
programPath = os.path.dirname(os.path.realpath(__file__))
bitfields = {}

class Parameter:
    def __init__(self, varName, label, stringFunction):
        """Create a new parameter

        Arguments: varName      - name of the parameter
                   label        - parameter string displayed in textbox
                   stringFunction - display format function (str or hex)"""
        self.varName = varName
        self.label = label
        self.stringFunction = stringFunction

class ReadOperation:
    def __init__(self, operationType, params, readFunction):
        """Create a new Read operation type

        Arguments: operationType    - read operation
                   params           - list of parameters
                   readFunction     - function to call for read"""
        self.operationType = operationType
        self.params = []
        self.params.extend(params)
        self.readFunction = readFunction

class ConfigureStatisticsWindow:
    def __init__(self, statWindow, statName = ""):
        """Create the edit or add statistic form

        Arguments: statWindow  - form for the main statistics
                   name        - name of the current form
                   statName    - key for the statistic being edited"""
        self.splitContainer = Forms.SplitContainer()

        self.form = Form()
        self.form.AutoSize = False
        #self.form.AutoSizeMode = AutoSizeMode.GrowAndShrink
        self.form.Width = 700
        self.form.Text = ""
        self.statWindow = statWindow
        self.currentStat = None
        self.currentStatKey = statName
        self.isEditFormMode = False

        self.opSettingsPanel = TableLayoutPanel()
        self.opSettingsPanel.AutoSize = True

        self.operationsPanel = TableLayoutPanel()
        self.operationsPanel.AutoSize = True

        self.settingsPanel = TableLayoutPanel()
        self.settingsPanel.AutoSize = True

        row = 0
        column = 0
        readLabel = Label()
        readLabel.Text = "Select type of Read:"
        readLabel.AutoSize = True
        readLabel.Anchor = AnchorStyles.Left
        self.operationsPanel.Controls.Add(readLabel, column, row)

        self.readOperations = self.statWindow.operationTypes.keys()
        self.readComboBox = ComboBox()
        self.readComboBox.Items.AddRange(Array[str]([v for v in self.readOperations]))
        self.readComboBox.DropDownStyle = ComboBoxStyle.DropDownList
        self.readComboBox.SelectedIndexChanged += self.readComboBox_SelectedIndexChanged
        self.readComboBox.AutoSize = True
        self.readComboBox.SelectedIndex = self.readComboBox.Items.Count - 1;

        # Change the form title
        if not statName:
            self.form.Text = "Add Field"
        else:
            self.isEditFormMode = True
            self.form.Text = "Edit Field"

        # If form is opened in edit mode, populate the previously chosen read operation
        if self.isEditFormMode:
            self.currentStat = self.statWindow.statistics[statName]
            self.readComboBox.SelectedItem = self.currentStat.operation

        self.operationsPanel.Controls.Add(self.readComboBox, 1, 0)

        # Defer other fields until the read operation is selected
        self.splitContainer.Dock = Forms.DockStyle.Fill
        self.splitContainer.Width = 600
        self.splitContainer.Panel1.AutoSize = True
        self.splitContainer.Panel2.AutoSize = True
        self.splitContainer.Panel1.AutoSizeMode = Forms.AutoSizeMode.GrowAndShrink
        self.splitContainer.Panel2.AutoSizeMode = Forms.AutoSizeMode.GrowAndShrink
        self.splitContainer.SplitterDistance = 250
        self.splitContainer.Panel1.Controls.Add(self.statWindow.ipxTree.treeView)
        self.splitContainer.Panel2.Controls.Add(self.opSettingsPanel)

        # Final initialization
        self.opSettingsPanel.Controls.Add(self.operationsPanel)
        self.opSettingsPanel.Controls.Add(self.settingsPanel)
        self.form.Controls.Add(self.splitContainer)
        self.form.ShowDialog()

    def setReadParams(self, sender, args):
        """Populates read parameters based on the treeview item selected

        Arguments: sender - unused
                   args - an instance of Forms.TreeViewEventArgs
        """
        if isinstance(args.Node.ipxNode, ipx.Register):
            self.nameTextBox.Text = args.Node.ipxNode.name
            self.address.Text = hex(args.Node.ipxNode.address)
            global bitfields
            bitfields[args.Node.ipxNode.name] = args.Node.ipxNode.bitfields

    def readComboBox_SelectedIndexChanged(self, sender, args):
        """Populate the parameters based on the chosen read operation

        Arguments: self   - ConfigureStatisticsWindow object
                   sender - none
                   args   - none """

        self.form.SuspendLayout()
        self.settingsPanel.Controls.Clear()

        row = 0
        column = 0

        namelabel = Label()
        namelabel.Text = 'Name:'
        namelabel.AutoSize = True
        namelabel.Anchor = AnchorStyles.Left
        self.settingsPanel.Controls.Add(namelabel, column, row)

        column = 1
        self.nameTextBox = TextBox()
        self.nameTextBox.MaxLength = 16
        self.nameTextBox.AutoSize = True
        if self.isEditFormMode:
            self.nameTextBox.Text = self.currentStatKey
        self.settingsPanel.Controls.Add(self.nameTextBox, column, row)

        row += 1

        # Create label and textbox based on the operation's parameters
        for param in self.statWindow.operationTypes[self.readComboBox.SelectedItem].params:
            column = 0
            label = Label()
            label.Text = param.label
            label.AutoSize = True
            label.Anchor = AnchorStyles.Left
            self.settingsPanel.Controls.Add(label, column, row)

            column = 1
            setattr(self, param.varName, TextBox())
            textbox = getattr(self, param.varName)
            textbox.MaxLength = 16
            textbox.AutoSize = True
            self.settingsPanel.Controls.Add(textbox, column, row)
            # Populate text-boxes if form is opened in edit mode
            if self.isEditFormMode:
                # convert_int_to_str(self.currentStat.param)
                textbox.Text = param.stringFunction(getattr(self.currentStat, param.varName))
            row += 1

        # This looks ridiculous but it ensures that we only ever have
        # one copy of the setReadParams handler registered with treeView's
        # AfterSelect event. Note that it is safe to deregister
        # the event handler before it has been registered.
        self.statWindow.ipxTree.treeView.AfterSelect -= self.setReadParams
        self.statWindow.ipxTree.treeView.AfterSelect += self.setReadParams

        column = 0
        self.addButton = Button()
        if self.isEditFormMode:
            self.addButton.Text = "Edit"
        else:
            self.addButton.Text = "Add"
        self.addButton.AutoSize = True
        self.settingsPanel.Controls.Add(self.addButton, column, row)
        self.addButton.Click += self.addButton_Clicked

        self.form.ResumeLayout()

    def addButton_Clicked(self, sender, args):
        """ Update the read operation parameters based on the selection
            and close the form

        Arguments: self   - ConfigureStatisticsWindow object
                   sender - none
                   args   - none """
        if not self.isEditFormMode:
            # Statistic Name already exists
            if self.nameTextBox.Text in self.statWindow.statistics:
                message = "Please enter a different name for the statistic."
                caption = "Statistic Name already exists"
                MessageBox.Show(message, caption, MessageBoxButtons.OK)
            else:
                row = self.statWindow.regTable.Rows[0].Clone()

                rowValues = [self.nameTextBox.Text, 0, 0, 0, 0]
                for index, value in enumerate(rowValues):
                    row.Cells[index].Value = value

                try:
                    newStat = Statistics(self.readComboBox.SelectedItem)

                    # Find all unique parameters and add attributes to object
                    paramList = []
                    for operationType in self.statWindow.operationTypes.itervalues():
                        for param in operationType.params:
                            paramList.append(param.varName)

                    uniqueParams = list(set(paramList))
                    for param in uniqueParams:
                        setattr(newStat, param, 0)

                    self.copyParametersFromWindowToStat(newStat)

                except ValueError:
                    message = "Please enter numeric values in all the fields with the exception of the name field."
                    caption = "Error Detected in Input"
                    MessageBox.Show(message, caption, MessageBoxButtons.OK)
                else: # Following code will be executed if exception is not raised
                    self.statWindow.regTable.Rows.Add(row)
                    self.statWindow.statistics.update({self.nameTextBox.Text:newStat})
                    self.form.Close()
        # Form in Edit mode
        else:
            showSameNameError = False
            oldName = self.currentStatKey
            newName = self.nameTextBox.Text
            # Edited Statistic Name already exists
            if newName in self.statWindow.statistics:
                if newName == oldName:
                    showSameNameError = False
                # Statistic Name has been changed to something that already exists
                else:
                    showSameNameError = True
            else:
                showSameNameError = False

            if showSameNameError:
                message = "Please enter a different name for the statistic."
                caption = "Statistic Name already exists"
                MessageBox.Show(message, caption, MessageBoxButtons.OK)
            else:
                try:
                    rowWithCurrentStat = None
                    # Find the row object in the table
                    for row in self.statWindow.regTable.Rows:
                        if row.Cells[0].Value == oldName:
                            rowWithCurrentStat = row

                    # Update name in table
                    rowWithCurrentStat.Cells[0].Value = newName

                    self.copyParametersFromWindowToStat(self.currentStat)
                    self.currentStat.operation = self.readComboBox.SelectedItem

                    self.statWindow.statistics[newName] = self.statWindow.statistics.pop(oldName)
                except ValueError:
                    message = "Please enter numeric values in the address, width and mask bit fields."
                    caption = "Error Detected in Input"
                    MessageBox.Show(message, caption, MessageBoxButtons.OK)
                else:
                    self.form.Close()

    def copyParametersFromWindowToStat(self, stat):
        """ Update parameters in stat object
        Arguments: self - ConfigureStatisticsWindow object
                   stat - statistic object """

        for param in self.statWindow.operationTypes[self.readComboBox.SelectedItem].params:
            textbox = getattr(self, param.varName)
            value = int(textbox.Text, 0)
            # stat.param = value
            setattr(stat, param.varName, value)

class Statistics():
    def __init__(self, operation):
        """Create a new statistics object

        Arguments: self   - Statistics object
                   operation - string representing the type of operation
                   NOTE: other attributes like function parameters will be added when creating
                         a new statistic """
        self.operation = operation
        self.maximumValue = 0
        self.minimumValue = 0xffffffff
        self.currentValue = 0
        self.averageValue = 0
        self.numOperations = 0

    def statToXml(self, key, doc, root):
        """Convert statistics to XML

        Arguments: self   - Statistics object
                   key    - Statistics key
                   doc    - Minidom Document object
                   root   - root XML node """
        # vars returns a dictionary of all members in the following format:
        # memberName : memberValue
        members = vars(self)

        stat = doc.CreateNode(XmlNodeType.Element, "statistic", None)
        root.AppendChild(stat)

        statName = doc.CreateNode(XmlNodeType.Element, "statName", None)
        statName.InnerText = key
        stat.AppendChild(statName)

        for key, value in members.iteritems():
            param = doc.CreateNode(XmlNodeType.Element, key, None)
            param.InnerText = str(value)
            stat.AppendChild(param)

    def xmlToStat(self, root):
        """Parse XML into statistic object

        Arguments: self   - Statistics object
                   root   - root XML node """
        # vars returns a dictionary of all members in the following format:
        # memberName : memberValue
        members = vars(self)

        statName = root.SelectSingleNode("statName").InnerText

        # save all variables to object
        for key, value  in members.iteritems():
            node = root.SelectSingleNode(key)

            # if a new parameter was added later, script should be able to
            # load old XML files
            if node is not None:
                newValue = node.InnerText
                # change type to int
                # Assuming all members are either str or int
                if not isinstance(value, str):
                    newValue = int(newValue)

                # stat.key = newValue
                setattr(self, key, newValue)

        return statName

class StatisticsWindow():
    def __init__(self, dev):
        """Initialize the main statistics window

        Arguments: self   - StatisticsWindow object
                   dev    - device object """
        self.form = Form()
        self.form.AutoSize = True 
        self.form.Size = Size(775, 400)
        self.form.FormClosing += self.stat_FormClosing
        self.form.FormBorderStyle = FormBorderStyle.Sizable #FixedDialog
        self.dev = dev
        self.form.Text = "Blackbird Statistics - " + dev.serialPort.PortName
        self.statistics = OrderedDict()

        self.mainPanel = TableLayoutPanel()
        self.mainPanel.AutoSize = True

        self.ipxTree = IPXTree(dev, self)
        self.ipxTree.treeView.Dock = Forms.DockStyle.Fill

        self.tablePanel = TableLayoutPanel()
        self.tablePanel.AutoSize = True

        self.settingsPanel = TableLayoutPanel()
        self.settingsPanel.AutoSize = True

        # Items for the table panel
        self.regTable = DataGridView()
        self.regTable.RowHeadersVisible = False
        self.regTable.Size = Size(755, 300)
        self.regTable.ColumnCount = 5

        columnHeadings = ["Name", "Current", "Average", "Minimum", "Maximum"]

        self.regTable.Columns[0].Width = 200 # Name column is wider

        # Specific to bitfield breakout
        # TODO Need to figure out one button for all or per row or both --- not a required feature
        #      at this time so just leave disabled for future enhancement.
        self.bitFieldsEnabled = False
        self.regTable.AutoSizeRowsMode = DataGridViewAutoSizeRowsMode.AllCells
        self.regTable.Columns[1].DefaultCellStyle.WrapMode = DataGridViewTriState.True

        for colIndex in range(5):
            self.regTable.Columns[colIndex].Name = columnHeadings[colIndex]
            self.regTable.Columns[colIndex].ReadOnly = True

        editButton = DataGridViewButtonColumn()
        editButton.HeaderText = ""
        editButton.Text = "Edit"
        editButton.UseColumnTextForButtonValue = True
        editButton.DisplayIndex = 5
        editButton.Width = 75
        self.regTable.Columns.Add(editButton)

        removeButton = DataGridViewButtonColumn()
        removeButton.HeaderText = ""
        removeButton.Text = "Remove"
        removeButton.UseColumnTextForButtonValue = True
        removeButton.DisplayIndex = 6
        removeButton.Width = 75
        self.regTable.Columns.Add(removeButton)

        self.regTable.CellClick += self.columnButton_Clicked

        self.tablePanel.Controls.Add(self.regTable)

        self.refreshRateHz = 0.5

        # Items for the settings panel
        self.addButton = Button()
        self.saveButton = Button()
        self.loadButton = Button()
        self.resetButton = Button()
        self.startLogButton = Button()
        self.stopLogButton = Button()
        self.stopLogButton.Enabled = False

        settingsButtons = [(self.addButton, "Add Field", self.addButton_Clicked),
                           (self.saveButton, "Save Settings", self.saveButton_Clicked),
                           (self.loadButton, "Load Settings", self.loadButton_Clicked),
                           (self.resetButton, "Reset All Statistics", self.resetButton_Clicked),
                           (self.startLogButton, "Start Log to csv", self.startLogButton_Clicked),
                           (self.stopLogButton, "Stop Log to csv", self.stopLogButton_Clicked)]

        row = 0
        column = 0
        for button, text, func in settingsButtons:
            button.Text = text
            button.AutoSize = True
            button.Click += func
            self.settingsPanel.Controls.Add(button, column, row)
            column += 1

        row = 1
        column = 0
        modifyLabel = Label()
        modifyLabel.Text = "Refresh Rate (Hz):"
        modifyLabel.AutoSize = True
        modifyLabel.Anchor = AnchorStyles.Left
        self.settingsPanel.Controls.Add(modifyLabel, column, row)

        column = 1
        self.modifyTextBox = TextBox()
        self.modifyTextBox.MaxLength = 16
        self.modifyTextBox.AutoSize = True
        self.modifyTextBox.Text = str(self.refreshRateHz)
        self.settingsPanel.Controls.Add(self.modifyTextBox, column, row)

        column = 2
        modifyButton = Button()
        modifyButton.Text = "Modify"
        modifyButton.AutoSize = True
        modifyButton.Click += self.modifyButton_Clicked
        self.settingsPanel.Controls.Add(modifyButton, column, row)

        self.mainPanel.Controls.Add(self.tablePanel)
        self.mainPanel.Controls.Add(self.settingsPanel)

        # Dialog for loading XML settings
        self.InputFileDialog = OpenFileDialog()
        self.InputFileDialog.Filter = "XML files (*.xml)|*.xml|" + \
                                      "All files (*.*)|*.*"
        self.InputFileDialog.Title = "Open an XML file"
        self.InputFileDialog.InitialDirectory = self.dev.programPath

        # List of all valid operations
        readAddr = ReadOperation(   'readAddr', \
                                    [Parameter('address',   'Address:',     hex)], \
                                    self.dev.iCmdResps['ICMD_COMPONENT']['read32'])

        self.operationTypes = {}
        self.operationTypes.update({readAddr.operationType: readAddr})

        # Find all unique parameters and add attributes to class
        paramList = []
        for operationType in self.operationTypes.itervalues():
            for param in operationType.params:
                paramList.append(param.varName)

        uniqueParams = list(set(paramList))
        for param in uniqueParams:
            setattr(Statistics, param, 0)


        # Start the read timer
        self.timer = threading.Timer(1/self.refreshRateHz , self.runReadOperations)
        self.timer.start()

        # Log file for saving statistics
        self.outputHandler = None
        self.outputLogger = logging.getLogger("statLog" + DateTime.Now.ToString("yyyy_MM_dd_HH_mm_ss"))
        self.outputLogger.setLevel(logging.DEBUG)

        # Final initialization
        self.form.Controls.Add(self.mainPanel)
        Application.Run(self.form)

    def addButton_Clicked(self, sender, args):
        """Opens a new form to add new statistics

        Arguments: self   - statWindow object
                   sender - none
                   args   - none """
        ConfigureStatisticsWindow(self)

    def saveButton_Clicked(self, sender, args):
        """Save the current statistics to an XML file

        Arguments: self   - statWindow object
                   sender - none
                   args   - none """
        doc = XmlDocument()
        BasicXml = "<?xml version=\"1.0\"?><StatSettings></StatSettings>"
        doc.LoadXml(BasicXml)

        root = doc.DocumentElement

        # Save all statistic to an XML file
        for key, stat in self.statistics.iteritems():
            stat.statToXml(key, doc, root)

        # create SaveFileDialog and show it
        saveFileDialog = SaveFileDialog()
        saveFileDialog.Title = "Save an XML file"
        saveFileDialog.Filter = "XML files (*.xml)|*.xml|" + \
                                "All files (*.*)|*.*"
        if saveFileDialog.ShowDialog() == DialogResult.OK:
            # get stream to write to file
            fileStream = saveFileDialog.OpenFile()
            if not fileStream == None:
                # create StreamWriter from original stream
                writeStream = StreamWriter(fileStream)
                # write text to file
                writeStream.Write(doc.OuterXml)
                writeStream.Close()

    def loadButton_Clicked(self, sender, args):
        """Load statistics from an XML file using file browser

        Arguments: self   - statWindow object
                   sender - none
                   args   - none """
        # show a dialog, ensure the user wants to load the file
        if self.InputFileDialog.ShowDialog() == DialogResult.OK:

            try:
                doc = XmlDocument()
                rawXml = File.ReadAllText(self.InputFileDialog.FileName)
                doc.LoadXml(rawXml)

                statList = doc.GetElementsByTagName("statistic")

                for statXml in statList:
                    operation = statXml.SelectSingleNode("operation").InnerText

                    stat = Statistics(operation)

                    # Find all unique parameters and add attributes to object
                    paramList = []
                    for operationType in self.operationTypes.itervalues():
                        for param in operationType.params:
                            paramList.append(param.varName)

                    uniqueParams = list(set(paramList))
                    for param in uniqueParams:
                        setattr(stat, param, 0)

                    statName = stat.xmlToStat(statXml)

                    # Reset all statistics variables
                    stat.maximumValue = 0
                    stat.minimumValue = 0xffffffff
                    stat.currentValue = 0
                    stat.averageValue = 0
                    stat.numOperations = 0

                    # Add row and statistic if statistic does not exist
                    if not statName in self.statistics:
                        row = self.regTable.Rows[0].Clone()

                        rowValues = [statName, 0, 0, 0, 0]
                        for index, value in enumerate(rowValues):
                            row.Cells[index].Value = value

                        self.regTable.Rows.Add(row)
                        self.statistics.update({statName:stat})
            except XmlException as ex:
                message = "Please open a valid XML file."
                caption = "XML file is invalid"
                MessageBox.Show(message, caption, MessageBoxButtons.OK)
            except MissingMemberException as ex:
                message = "Xml file is missing statistics variables. Please open a valid XML file."
                caption = "XML file is invalid"
                MessageBox.Show(message, caption, MessageBoxButtons.OK)

    def resetButton_Clicked(self, sender, args):
        """Reset all statistics

        Arguments: self   - statWindow object
                   sender - none
                   args   - none """
        for stat in self.statistics.itervalues():
            # Reset all statistics variables
            stat.maximumValue = 0
            stat.minimumValue = 0xffffffff
            stat.currentValue = 0
            stat.averageValue = 0
            stat.numOperations = 0

    def startLogButton_Clicked(self, sender, args):
        """Start logging statistics to csv

        Arguments: self   - statWindow object
                   sender - none
                   args   - none """
        self.startLogButton.Enabled = False
        self.stopLogButton.Enabled = True
        self.outputHandler = logging.FileHandler(programPath + r"\Log\StatLog_" +
                DateTime.Now.ToString("yyyy_MM_dd_HH_mm_ss") + ".csv")
        self.outputLogger.addHandler(self.outputHandler)

    def stopLogButton_Clicked(self, sender, args):
        """Stop logging statistics to csv

        Arguments: self   - statWindow object
                   sender - none
                   args   - none """
        self.startLogButton.Enabled = True
        self.stopLogButton.Enabled = False
        self.outputHandler.flush()
        self.outputLogger.removeHandler(self.outputHandler)
        self.outputHandler.close()
        self.outputHandler = None

    def modifyButton_Clicked(self, sender, args):
        """Modify Refresh rate

        Arguments: self   - statWindow object
                   sender - none
                   args   - none """
        # TODO: add lock for refresh rate variable
        if not self.modifyTextBox.Text:
            message = "Please enter a valid refresh rate"
            caption = "Incorrect refresh rate"
            MessageBox.Show(message, caption, MessageBoxButtons.OK)
            self.modifyTextBox.Text = str(self.refreshRateHz)
        try:
            refreshRate = float(self.modifyTextBox.Text)
        except ValueError:
            message = "Please enter a valid refresh rate"
            caption = "Incorrect refresh rate"
            MessageBox.Show(message, caption, MessageBoxButtons.OK)
            self.modifyTextBox.Text = str(self.refreshRateHz)
        else:
            self.refreshRateHz = refreshRate

    def stat_FormClosing(self, sender, args):
        """Event called before closing the form.
           Used for stopping the read timer

        Arguments: self   - statWindow object
                   sender - none
                   args   - none """
        self.timer.cancel()
        if self.outputHandler is not None:
            self.outputHandler.flush()
            self.outputLogger.removeHandler(self.outputHandler)
            self.outputHandler.close()
            self.outputHandler = None

    def columnButton_Clicked(self, sender, args):
        """Event for opening a window to edit a statistic
           or removing the statistic

        Arguments: self   - statWindow object
                   sender - none
                   args   - none """
        column = self.regTable.CurrentCell.ColumnIndex
        row = self.regTable.CurrentCell.RowIndex
        statName = self.regTable.CurrentRow.Cells[0].Value

        if statName is not None:
            # Edit window
            if column == 5:
                ConfigureStatisticsWindow(self, statName)
            # Remove row
            elif column == 6:
                # Remove stat item from dictionary
                del self.statistics[statName]
                # Remove row from table
                self.regTable.Rows.Remove(self.regTable.CurrentRow)

    def runReadOperations(self):
        """Read operation timer function.

        Arguments: self   - statWindow object
                   sender - none
                   args   - none """
        print 'Running the following Read operations:'
        global strBfValuesLen

        try:
            startTime = datetime.now()

            printHeader = False
            if self.outputHandler is not None:
                statlogfilesize = os.stat(self.outputHandler.baseFilename).st_size
                printHeader = True if (statlogfilesize == 0) else False

            logKeysString = ""
            logValuesString = ""

            for key in self.statistics.iterkeys():
                logKeysString += key + ","

            for key, stat in self.statistics.iteritems():
                print 'Stat: ' + key

                # add all parameters to list
                paramArgs = []
                for param in self.operationTypes[stat.operation].params:
                    paramArgs.append(getattr(stat, param.varName))

                # *paramArgs will convert [p1, p2, p3] into p1, p2, p3
                resp = self.operationTypes[stat.operation].readFunction(*paramArgs)

                # Find appropriate row
                row = None
                for rowI in self.regTable.Rows:
                    if rowI.Cells[0].Value == key:
                        row = rowI

                stat.numOperations += 1
                stat.currentValue = resp

                logValuesString += str(stat.currentValue) + ","

                # Calculate minimum, maximum and average
                if resp < stat.minimumValue:
                    stat.minimumValue = resp
                if resp > stat.maximumValue:
                    stat.maximumValue = resp
                stat.averageValue = (stat.averageValue * (stat.numOperations - 1)
                                     + stat.currentValue) / stat.numOperations

                if self.bitFieldsEnabled == True:
                    # bitfields
                    strBfValues = str(stat.currentValue) + "\r\n*BITFIELDS*"

                    bfSorted = sorted(bitfields[key], key=lambda bf: bf.bit_offset)
                    bfName = {}
                    bfWidth = {} 
                    bfOffset = {}
                    for bf in bfSorted:
                        bfName[bf.name] = bf.name
                        bfWidth[bf.name] = bf.bit_width
                        bfOffset[bf.name] = bf.bit_offset

                    for bf in bfName:
                        mask = 0
                        for i in range(0, bfWidth[bf]):
                            mask |= 1 << i
                        val = (int(stat.currentValue) >> int(bfOffset[bf])) & mask
                        strBfValues += "\r\n" + bf + ": " + str(val)

                    # Update table values
                    row.Cells[1].Value = strBfValues
                else:
                    row.Cells[1].Value = stat.currentValue

                row.Cells[2].Value = stat.averageValue
                row.Cells[3].Value = stat.minimumValue
                row.Cells[4].Value = stat.maximumValue


        except TypeError:
            message = "Device did not respond to the read operation"
            caption = "Error Detected in Read I/O"
            MessageBox.Show(message, caption, MessageBoxButtons.OK)
        else:
            endTime = datetime.now()

            timeDifference = endTime - startTime

            print "Time to execute commands: " + str(timeDifference.microseconds) + " usec"

            if self.outputHandler is not None:
                if printHeader:
                    self.outputLogger.info(logKeysString)
                self.outputLogger.info(logValuesString)
            # TODO: should I only start timers if all statistics failed or
            # should I remove the erroneous statistic and continue with other ones ???

            # TODO: change next timer to (period - delta_T):
            # where delta_T is difference between previous time stamp and new time stamp
            self.timer = threading.Timer(1/self.refreshRateHz, self.runReadOperations)
            self.timer.start()


#start script
StatisticsWindow(currentDevice)
