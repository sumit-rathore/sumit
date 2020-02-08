import clr
clr.AddReference("System.Drawing")
clr.AddReference("System.Windows.Forms")
import System.IO as IO
import System.Drawing as Drawing
import System.Windows.Forms as Forms
import binascii
import Scripts.ipxact_xml as ipx
from Scripts.ipxact_tree import IPXTree
from cobs_logger import cbs_logger
import sys
import datetime

class SpectaregWindow(object):
    """Displays a window for viewing and modifying register data."""
    def __init__(self, dev):
        self.form = Forms.Form()
        self.form.Size = Drawing.Size(1050, 900)
        self.dev = dev
        self.form.Text = self.dev.device_name + ": Register Specs - " + \
                self.dev.serial_port_manager.port_name
        self.baseString = "default"
        self.nonPersistentRow = 0
        self.filterNode = None
        self.srchFound = False
        self.srchLevelFound = 0

        # Parse the XML file(s) appropriate to the project
        self.ipxTree = IPXTree(dev, self)

        # Create control objects
        self.itemContainer = Forms.SplitContainer()
        self.tableContainer = Forms.SplitContainer()

        self.regStaticDataGroupBox = Forms.GroupBox()
        self.regBitFieldDataGroupBox = Forms.GroupBox()
        self.titleLabel = Forms.Label()
        self.offsetLabel = Forms.Label()
        self.descTextBox = Forms.TextBox()
        self.resetLabel = Forms.Label()

        self.positionLabel = Forms.Label()
        self.widthLabel = Forms.Label()
        self.valueLabel = Forms.Label()
        self.positionTextBox = Forms.TextBox()
        self.widthTextBox = Forms.TextBox()
        self.valueTextBox = Forms.TextBox()
        self.readButton = Forms.Button()
        self.modifyBitfieldButton = Forms.Button()
        self.resultTextBox = Forms.TextBox()
        self.returnLabel = Forms.Label()
        self.returnTextBox = Forms.TextBox()
        self.searchTextBox = Forms.TextBox()
        self.searchGroupBox = Forms.GroupBox()

        self.regTable = Forms.DataGridView()

        # Configure control objects
        self.searchTextBox.Dock = Forms.DockStyle.Bottom
        self.ipxTree.treeView.Dock = Forms.DockStyle.Fill
        self.ipxTree.treeView.AfterSelect += self.treeViewNodeSelectedHandler

        self.searchGroupBox.Text = 'Search Tree'
        self.searchGroupBox.Dock = Forms.DockStyle.Bottom
        self.searchGroupBox.Padding = Forms.Padding(5)
        self.searchGroupBox.AutoSize = True

        self.regStaticDataGroupBox.Text = 'Static Register Data'
        self.regStaticDataGroupBox.Dock= Forms.DockStyle.Top
        self.regStaticDataGroupBox.Padding = Forms.Padding(5)
        self.regStaticDataGroupBox.AutoSize = True

        self.regBitFieldDataGroupBox.Text = 'Bitfield Data'
        self.regBitFieldDataGroupBox.Dock= Forms.DockStyle.Top
        self.regBitFieldDataGroupBox.Padding = Forms.Padding(5)
        self.regBitFieldDataGroupBox.Height = 60

        self.titleLabel.Text = "Name: "
        self.titleLabel.Dock = Forms.DockStyle.Top
        self.titleLabel.Padding = Forms.Padding(5)
        self.titleLabel.AutoSize = True

        self.offsetLabel.Text = "Offset: "
        self.offsetLabel.Dock = Forms.DockStyle.Top
        self.offsetLabel.Padding = Forms.Padding(5)
        self.offsetLabel.AutoSize = True

        self.descTextBox.Text = "Description: "
        self.descTextBox.Multiline = True
        self.descTextBox.ScrollBars = Forms.ScrollBars.Vertical
        self.descTextBox.WordWrap = True
        self.descTextBox.ReadOnly = True
        self.descTextBox.BackColor = Drawing.SystemColors.Control
        self.descTextBox.BorderStyle = Forms.BorderStyle.FixedSingle
        self.descTextBox.Width = 400
        self.descTextBox.Height = 80
        self.descTextBox.Padding = Forms.Padding(5)
        self.descTextBox.Dock = Forms.DockStyle.Top

        self.resetLabel.Text = "Reset: "
        self.resetLabel.Padding = Forms.Padding(5)
        self.resetLabel.AutoSize = True
        self.resetLabel.Dock = Forms.DockStyle.Top

        self.positionLabel.Text = "Position: "
        self.positionLabel.Padding = Forms.Padding(5,5,5,0)
        self.positionLabel.AutoSize = True
        self.positionLabel.Dock = Forms.DockStyle.Left

        self.positionTextBox.MaxLength = 2
        self.positionTextBox.Padding = Forms.Padding(0,5,5,5)
        self.positionTextBox.AutoSize = True
        self.positionTextBox.Dock = Forms.DockStyle.Left

        self.widthLabel.Text = "Width: "
        self.widthLabel.Padding = Forms.Padding(5,5,5,0)
        self.widthLabel.AutoSize = True
        self.widthLabel.Dock = Forms.DockStyle.Left

        self.widthTextBox.MaxLength = 2
        self.widthTextBox.Padding = Forms.Padding(0,5,5,5)
        self.widthTextBox.AutoSize = True
        self.widthTextBox.Dock = Forms.DockStyle.Left

        self.valueLabel.Text = "Value: "
        self.valueLabel.Padding = Forms.Padding(5,5,5,0)
        self.valueLabel.AutoSize = True
        self.valueLabel.Dock = Forms.DockStyle.Left

        self.valueTextBox.MaxLength = 10
        self.valueTextBox.Padding = Forms.Padding(0,5,5,5)
        self.valueTextBox.AutoSize = True
        self.valueTextBox.Dock = Forms.DockStyle.Left

        self.modifyBitfieldButton.Text = "Modify"
        self.modifyBitfieldButton.Padding = Forms.Padding(5)
        self.modifyBitfieldButton.Click += self.executeModifyBitfield
        self.modifyBitfieldButton.Dock = Forms.DockStyle.Left

        self.readButton.Text = "Read"
        self.readButton.Padding = Forms.Padding(5)
        self.readButton.Click += self.executeReadMemory
        self.readButton.Dock = Forms.DockStyle.Left

        self.returnLabel.Text = "Returned Value: "
        self.returnLabel.Padding = Forms.Padding(5)
        self.returnLabel.Dock = Forms.DockStyle.Left
        self.returnLabel.AutoSize = True

        def updateBaseCheckBoxes(sender, _args):
            for item in self.tableContainer.ContextMenu.MenuItems:
                item.Checked = False
            sender.Checked = True

        def b_default(sender, args):
            self.baseString = "default"
            updateBaseCheckBoxes(sender, args)

        def b_bin(sender, args):
            self.baseString = "binary"
            updateBaseCheckBoxes(sender, args)

        def b_hex(sender, args):
            self.baseString = "hexadecimal"
            updateBaseCheckBoxes(sender, args)

        def b_unsigned(sender, args):
            self.baseString = "unsigned"
            updateBaseCheckBoxes(sender, args)

        def b_ascii(sender, args):
            self.baseString = "ascii"
            updateBaseCheckBoxes(sender, args)

        # regTable - holds all of the register information
        self.regTable.ColumnCount = 8
        self.regTable.RowHeadersVisible = False
        self.regTable.Dock = Forms.DockStyle.Fill
        self.regTable.DefaultCellStyle.WrapMode = Forms.DataGridViewTriState.True
        self.regTable.AutoSizeRowsMode = Forms.DataGridViewAutoSizeRowsMode.AllCells
        self.regTable.AutoSizeColumnsMode = Forms.DataGridViewAutoSizeColumnsMode.Fill

        self.regTable.Columns[0].Name = "Register"
        self.regTable.Columns[0].MinimumWidth = 105
        self.regTable.Columns[0].FillWeight = 1
        self.regTable.Columns[0].ReadOnly = True

        self.regTable.Columns[1].Name = "Bits"
        self.regTable.Columns[1].MinimumWidth = 35
        self.regTable.Columns[1].FillWeight = 1
        self.regTable.Columns[1].ReadOnly = True

        self.regTable.Columns[2].Name = "Field Name"
        self.regTable.Columns[2].MinimumWidth = 105
        self.regTable.Columns[2].FillWeight = 1
        self.regTable.Columns[2].ReadOnly = True

        self.regTable.Columns[3].Name = "Description"
        # default FillWeight is 100, want Description to expand, not other
        # columns
        self.regTable.Columns[3].ReadOnly = True

        self.regTable.Columns[4].Name = "R/W"
        self.regTable.Columns[4].MinimumWidth = 70
        self.regTable.Columns[4].FillWeight = 1
        self.regTable.Columns[4].ReadOnly = True

        self.regTable.Columns[5].Name = "Reset Value"
        self.regTable.Columns[5].MinimumWidth = 75
        self.regTable.Columns[5].FillWeight = 1
        self.regTable.Columns[5].ReadOnly = True

        self.regTable.Columns[6].Name = "Value Type"
        self.regTable.Columns[6].MinimumWidth = 75
        self.regTable.Columns[6].FillWeight = 1
        self.regTable.Columns[6].ReadOnly = True

        self.regTable.Columns[7].Name = "Current Bits"
        self.regTable.Columns[7].MinimumWidth = 85
        self.regTable.Columns[7].FillWeight = 50
        self.regTable.Columns[7].ReadOnly = True

        self.regTable.CurrentCellChanged += self.updateWriteValues
        self.tableContainer.ContextMenu = Forms.ContextMenu()
        contextMenu = self.tableContainer.ContextMenu.MenuItems
        contextMenu.Add(Forms.MenuItem("default", b_default))
        contextMenu.Add(Forms.MenuItem("hexidecimal", b_hex))
        contextMenu.Add(Forms.MenuItem("unsigned", b_unsigned))
        contextMenu.Add(Forms.MenuItem("binary", b_bin))
        contextMenu.Add(Forms.MenuItem("ascii", b_ascii))

        self.form.SuspendLayout()

        self.searchGroupBox.Controls.Add(self.searchTextBox)

        self.regStaticDataGroupBox.Controls.Add(self.resetLabel)
        self.regStaticDataGroupBox.Controls.Add(self.descTextBox)
        self.regStaticDataGroupBox.Controls.Add(self.offsetLabel)
        self.regStaticDataGroupBox.Controls.Add(self.titleLabel)

        self.regBitFieldDataGroupBox.Controls.Add(self.returnLabel)
        self.regBitFieldDataGroupBox.Controls.Add(self.readButton)
        self.regBitFieldDataGroupBox.Controls.Add(self.modifyBitfieldButton)
        self.regBitFieldDataGroupBox.Controls.Add(self.valueTextBox)
        self.regBitFieldDataGroupBox.Controls.Add(self.valueLabel)
        self.regBitFieldDataGroupBox.Controls.Add(self.widthTextBox)
        self.regBitFieldDataGroupBox.Controls.Add(self.widthLabel)
        self.regBitFieldDataGroupBox.Controls.Add(self.positionTextBox)
        self.regBitFieldDataGroupBox.Controls.Add(self.positionLabel)

        self.itemContainer.Dock = Forms.DockStyle.Fill
        self.itemContainer.Width = 1000
        self.itemContainer.Panel1.AutoSize = True
        self.itemContainer.Panel2.AutoSize = True
        self.itemContainer.Panel1.AutoSizeMode = Forms.AutoSizeMode.GrowAndShrink
        self.itemContainer.Panel2.AutoSizeMode = Forms.AutoSizeMode.GrowAndShrink
        self.itemContainer.SplitterDistance = 250
        self.itemContainer.Panel1.Controls.Add(self.searchGroupBox)
        self.itemContainer.Panel1.Controls.Add(self.ipxTree.treeView)
        self.itemContainer.Panel2.Controls.Add(self.tableContainer)

        self.form.Controls.Add(self.itemContainer)

        self.form.ResumeLayout()

        self.tableContainer.Orientation = Forms.Orientation.Horizontal
        self.tableContainer.Dock = Forms.DockStyle.Fill
        self.tableContainer.SplitterDistance = 230
        self.tableContainer.Panel1.AutoSize = True
        self.tableContainer.Panel2.AutoSize = True
        self.tableContainer.Panel1.AutoSizeMode = Forms.AutoSizeMode.GrowAndShrink
        self.tableContainer.Panel2.AutoSizeMode = Forms.AutoSizeMode.GrowAndShrink
        self.tableContainer.FixedPanel = Forms.FixedPanel.Panel1
        self.tableContainer.Panel1.Controls.Add(self.regBitFieldDataGroupBox)
        self.tableContainer.Panel1.Controls.Add(self.regStaticDataGroupBox)
        self.tableContainer.Panel2.Controls.Add(self.regTable)

        self.dev.print_to_device_window(
                    "iRegister is launched successfully",
                    Drawing.Color.DarkGreen)

        Forms.Application.Run(self.form)

    def executeModifyBitfield(self, sender, args):
        """Executes the modifyBitfield iCommand.  Retrieves arguments from control textBoxes

        Arguments: sender - unused
                   args - unused
        """
        # nothing should happen when no address is selected
        if self.selectedAddress is None:
            return
        try:
            icmd_obj = self.dev.create_icmd("ICMD_COMPONENT", "modifyBitfield", False,
                    [self.selectedAddress,
                    int(self.positionTextBox.Text, 0),
                    int(self.widthTextBox.Text, 0),
                    int(self.valueTextBox.Text, 0)])
            self.dev.send_icmd(icmd_obj)

        except Exception :
            error_msg = "Please enter the bit field information"
            cbs_logger.exception(error_msg)
            Forms.MessageBox.Show(error_msg)
            return
        # call read after memory was modified so values update
        self.executeReadMemory(None, None)

    def executeReadMemory(self, sender, args):
        """Executes the readMemory iCommand.  Retrieves arguments from control textBoxes.

        Arguments: sender - unused
                   args - unused
        """
        # nothing should happen when no address is selected
        if self.selectedAddress is None:
            return

        try:
            current_icmd_obj = self.dev.create_icmd("ICMD_COMPONENT", "readMemory", True,
                    [self.selectedAddress])
            self.currentRegValue = self.dev.send_icmd_wait_for_response(current_icmd_obj)
            cbs_logger.info("{}: {}: Return the selected register value={}". \
                    format(self.dev.port_name, self.dev.device_name, self.currentRegValue))
        except Exception:
            error_msg = "Could not execute read32 command response"
            cbs_logger.exception(error_msg)
            Forms.MessageBox.Show(error_msg)
            # save returned value
        try:
            if self.currentRegValue != None:
                self.currentRegValue = self.currentRegValue[0]
                self.currentRegValue = hex(self.currentRegValue)[:10]
                self.returnLabel.Text = "Returned Value: " + \
                    str(self.changeBase(self.currentRegValue,
                                        self.baseString, "hexadecimal"))
            else:
                raise ReadRegisterError("Register value returned as 'None'", self.selectedAddress)
        except:
            error_message ="Got an error when reading register from {} at {}". \
                                format(self.dev.device_name, self.dev.port_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

        # update current bit values in table
        if self.currentRegValue != None:
            self.updateValue()


    def filterTree(self, args, srchStr):
        if not hasattr(args, 'Text'):
            return

        # store main node before recursion
        if self.filterNode == None:
            self.filterNode = args

        if srchStr in args.Text:
            # black search clears highlighting and collapses all
            if srchStr != "":
                if self.srchFound == False:
                    # record highest level of srch so we know upper levels need expansion
                    self.srchLevelFound = args.Level
                self.srchFound = True
                args.Expand()
                args.BackColor = Drawing.Color.Red
                if args.Parent != None:
                    args.Parent.Expand()
            else:
                args.BackColor = Drawing.Color.Empty
                args.Collapse()
        if args.GetNodeCount(True) == 0:
            if not srchStr in args.Text:
                args.BackColor = Drawing.Color.Empty
            return
        for a in args.Nodes:
            self.filterTree(a, srchStr)
        # expand upper tree if someone searches in nodes below
        if args.Level < self.srchLevelFound:
            args.Expand()

    def clearDataGridView(self):
        """Populates register and table data based on item selected

        Arguments: sender - unused
                   args - an instance of Forms.TreeViewEventArgs
        """
        # clear table for new data to be entered
        self.regTable.Rows.Clear()
        self.nonPersistentRow = 0

    def updateRegisterDisplayData(self, args):
        """Updates Stats info

        Arguments: sender - unused
                   args - an instance of Forms.TreeViewEventArgs
        """
        self.titleLabel.Text = 'Name: ' + args.Text

        try:
            self.selectedAddress = args.ipxNode.address
        except AttributeError:
            self.selectedAddress = None

        try:
            self.regSize = args.ipxNode.bit_width
        except AttributeError:
            self.regSize = None

        if self.selectedAddress != None and self.regSize != None:
            hexStr = paddedHexStr(self.selectedAddress, self.regSize)
            self.offsetLabel.Text = 'Offset: %s' % hexStr
        else:
            self.offsetLabel.Text = 'Offset: '

        try:
            self.descTextBox.Text = 'Description: ' + args.ipxNode.description
        except AttributeError:
            self.descTextBox.Text = 'Description: '

        try:
            if self.regSize != None:
                hexStr = paddedHexStr(args.ipxNode.masked_reset_value,
                                      self.regSize)
            else:
                hexStr = ''
            self.resetLabel.Text = 'Reset: ' + hexStr
        except AttributeError:
            self.resetLabel.Text = 'Reset: '

    def treeViewNodeAddPersistentHandler(self, args):
        """Populates register and table data based on item selected

        Arguments: sender - unused
                   args - an instance of Forms.TreeViewEventArgs
        """
        # add highligher
        self.titleLabel.Text = 'Name: ' + args.Text
        attrErr = False
        try:
            self.selectedAddress = args.ipxNode.address
            # we have valid address, so let's foolishly assume all is good
        except AttributeError:
            attrErr = True
            self.selectedAddress = None

        try:
            self.regSize = args.ipxNode.bit_width
        except AttributeError:
            attrErr = True
            self.regSize = None

        if self.selectedAddress != None and self.regSize != None:
            hexStr = paddedHexStr(self.selectedAddress, self.regSize)
            self.offsetLabel.Text = 'Offset: %s' % hexStr
        else:
            self.offsetLabel.Text = 'Offset: '

        try:
            self.descTextBox.Text = 'Description: ' + args.ipxNode.description
        except AttributeError:
            attrErr = True
            self.descTextBox.Text = 'Description: '

        try:
            if self.regSize != None:
                hexStr = paddedHexStr(args.ipxNode.masked_reset_value,
                                      self.regSize)
            else:
                hexStr = ''
            self.resetLabel.Text = 'Reset: ' + hexStr
        except AttributeError:
            attrErr = True
            self.resetLabel.Text = 'Reset: '

        try:
            bitFields = args.ipxNode.bitfields
        except AttributeError:
            attrErr = True
            bitFields = []

        if attrErr == False and self.regTable.Rows.Count > 1:
            self.regTable.Rows.Add() # our blank row between bitfields

        bitFieldStarted = False
        rowIdxStart = self.regTable.Rows.Count - 1

        # Reversed because we wish to display the bitfields in descending
        # order of offset.
        for row in reversed(list(self.bitFieldRowGenerator(bitFields))):
            row.Tag = args
            self.regTable.Rows.Add(row)
            if bitFieldStarted == False:
                if args.Parent != None:
                    p = args.FullPath.replace("\\","\n->")
                    self.regTable.Rows[rowIdxStart].Cells[0].Value = p
                bitFieldStarted = True
        if self.nonPersistentRow > 0:
            row = self.regTable.Rows.Item[self.nonPersistentRow]
            row.DefaultCellStyle.BackColor = Drawing.Color.LightGray

    def treeViewNodeSelectedHandler(self, sender, args):
        """Populates register and table data based on item selected

        Arguments: sender - unused
                   args - an instance of Forms.TreeViewEventArgs
        """
        # don't change anything if not selecting a register
        insertColouredBar = False
        if hasattr(args.Node.ipxNode, 'bitfields'):
            if self.nonPersistentRow == 0 and self.regTable.Rows.Count > self.nonPersistentRow + 1:
                insertColouredBar = True
            while self.nonPersistentRow > 0:
                    row = self.regTable.Rows.Item[self.nonPersistentRow - 1]
                    self.regTable.Rows.Remove(row)
                    self.nonPersistentRow -= 1

            self.nonPersistentRow = 0
            if self.regTable.Rows.Count == 1:
                row = self.regTable.Rows.Item[self.nonPersistentRow]
                row.DefaultCellStyle.BackColor = Drawing.Color.Empty

        self.titleLabel.Text = 'Name: ' + args.Node.Text
        attrErr = False
        try:
            self.selectedAddress = args.Node.ipxNode.address
            # we have valid address, so let's foolishly assume all is good
        except AttributeError:
            attrErr = True
            self.selectedAddress = None

        try:
            self.regSize = args.Node.ipxNode.bit_width
        except AttributeError:
            attrErr = True
            self.regSize = None

        if self.selectedAddress != None and self.regSize != None:
            hexStr = paddedHexStr(self.selectedAddress, self.regSize)
            self.offsetLabel.Text = 'Offset: %s' % hexStr
        else:
            self.offsetLabel.Text = 'Offset: '

        try:
            self.descTextBox.Text = 'Description: ' + args.Node.ipxNode.description
        except AttributeError:
            attrErr = True
            self.descTextBox.Text = 'Description: '

        try:
            if self.regSize != None:
                hexStr = paddedHexStr(args.Node.ipxNode.masked_reset_value,
                                      self.regSize)
            else:
                hexStr = ''
            self.resetLabel.Text = 'Reset: ' + hexStr
        except AttributeError:
            attrErr = True
            self.resetLabel.Text = 'Reset: '

        try:
            bitFields = args.Node.ipxNode.bitfields
        except AttributeError:
            attrErr = True
            bitFields = []

        bitFieldStarted = False
        rowIdxStart = 0

        # Reversed because we wish to display the bitfields in descending
        # order of offset.
        storedRow = None
        for row in reversed(list(self.bitFieldRowGenerator(bitFields))):
            row.Tag = args.Node
            row.DefaultCellStyle.BackColor = Drawing.Color.Empty
            if storedRow == None:
                storedRow = row.Clone()
            self.regTable.Rows.Insert(self.nonPersistentRow, row) #Add(row)
            self.nonPersistentRow += 1
            if bitFieldStarted == False:
                if args.Node.Parent != None:
                    p = args.Node.FullPath.replace("\\","\n->")
                    self.regTable.Rows[rowIdxStart].Cells[0].Value = p
                bitFieldStarted = True
        if storedRow != None and insertColouredBar == True:
            storedRow.DefaultCellStyle.BackColor = Drawing.Color.LightGray
            self.regTable.Rows.Insert(self.nonPersistentRow, storedRow)

    def bitFieldRowGenerator(self, bitFields):
        """Given an iterator of BitField objects, yields a generator of rows with
        any gaps between the BitFields filled in by RESERVED-type rows. Rows
        are sorted in ascending order of bitfield offset.
        """
        def formatRange(high, low):
            if high == low:
                return str(low)
            else:
                return '%d:%d' % (high, low)

        try:
            bfsSorted = sorted(bitFields, key=lambda bf: bf.bit_offset)
        except AttributeError:
            bfsSorted = []
        lastIndex = -1
        for bf in bfsSorted:
            if bf.bit_offset != lastIndex + 1:
                row = self.regTable.Rows[0].Clone()
                row.Cells[1].Value = formatRange(bf.bit_offset - 1, lastIndex + 1)
                row.Cells[2].Value = 'RESERVED_BITS'
                yield row

            row = self.regTable.Rows[0].Clone()
            row.Cells[1].Value = formatRange(bf.bit_offset + bf.bit_width - 1,
                                             bf.bit_offset)
            row.Cells[2].Value = bf.name
            row.Cells[3].Value = bf.description
            row.Cells[4].Value = bf.access
            row.Cells[5].Value = self.changeBase(
                    bf.reset_value, self.baseString, bf.value_type)
            row.Cells[6].Value = bf.value_type
            lastIndex = bf.bit_offset + bf.bit_width - 1
            yield row

        if self.regSize and lastIndex != self.regSize - 1:
            row = self.regTable.Rows[0].Clone()
            row.Cells[1].Value = formatRange(self.regSize - 1, lastIndex + 1)
            row.Cells[2].Value = 'RESERVED_BITS'
            yield row

    # updates the current bits column for the register
    def updateValue(self):
        for row in self.regTable.Rows:
            if row.Cells[1].Value != None:
                tmpList = row.Cells[1].Value.split(":")
                if len(tmpList) == 1:
                    tmpList.append(tmpList[0])
                tmpBin = bin(int(self.currentRegValue, 16))[2:].zfill(
                    self.regSize)[self.regSize - int(tmpList[0])
                                  - 1:self.regSize - int(tmpList[1])]
                if self.baseString == "default":
                    fieldBase = row.Cells[6].Value
                else:
                    fieldBase = self.baseString
                if fieldBase == "unsigned":
                    row.Cells[7].Value = int(tmpBin, 2)
                elif fieldBase == "binary":
                    row.Cells[7].Value = tmpBin
                elif fieldBase == "ascii":
                    row.Cells[7].Value = binascii.a2b_hex(
                        hex(int(tmpBin, 2))[2:])
                elif fieldBase == "boolean":
                    row.Cells[7].Value = bool(int(tmpBin, 16))
                else:
                    # hex
                    row.Cells[7].Value = hex(int(tmpBin, 2))[:10]

    def changeBase(self, value, base, defaultBase):
        """Converts given hex value to base specified.

        Arguments: value - Hex value to convert
                   base - base to convert to
                   defaultBase - base to use if base param is 'default'
        """
        try:
            if base == "default":
                base = defaultBase
            if base == "hexadecimal":
                return value
            elif base == "ascii":
                return binascii.a2b_hex(value[2:])
            elif base == "binary":
                return bin(int(value, 16))[2:]
            elif base == "boolean":
                return bool(int(value, 16))
            else:
                # unsigned
                return int(value, 16)
        except:
            # hex
            # If new base cannot convert, go back to default
            self.baseString = "default"
            for item in self.tableContainer.ContextMenu.MenuItems:
                item.Checked = False
                if item.Name == "default":
                    item.Checked = True
            return value

    def updateWriteValues(self, sender, args):
        """Populates position and width text fields based on selected row

        Arguments: sender - unused
                   args - unused
        """
        # make sure the row selected isn't empty or there will be an error
        if self.regTable.CurrentRow != None:
            try:
                if self.regTable.CurrentRow.Cells[1].Value != None:
                    self.updateRegisterDisplayData(self.regTable.CurrentRow.Tag)
                    tmpList = self.regTable.CurrentRow.Cells[1].Value.split(":")
                    if len(tmpList) == 1:
                        self.positionTextBox.Text = tmpList[0]
                        self.widthTextBox.Text = '1'
                    else:
                        self.positionTextBox.Text = tmpList[1]
                        self.widthTextBox.Text = str(1 + int(tmpList[0]) - int(tmpList[1]))

            except:
                # will occur if the blank row at end of table is clicked,
                # exception can be passed
                #pass
                raise


def paddedHexStr(val, bitWidth):
    """Given an integer and a word size, outputs a zero-padded hex string."""
    numHexDigits = int(bitWidth / 4)
    hexFormatStr = '%0' + str(numHexDigits) + 'x'
    return ('0x' + hexFormatStr % val)

class ReadRegisterError():
    def __init__(self, message, register_address):
        self.message = message
        self.reg_addr = register_address

    def __str__(self):
        return "{}: ".format(self.message) + "\n" \
               "register address: {}".format(hex(self.reg_addr))

# run script
SpectaregWindow(currentDevice)
