import clr
clr.AddReference('IronPython')
from IronPython.Compiler import CallTarget0
import datetime
import os
import System
import System.Array as Array
import System.Windows.Forms as Forms
import System.Drawing as Drawing
from cobs_logger import cbs_logger

class iLogFilterWindow(Forms.Form):
    def __init__(self, dev):
        self.dev = dev
        self.wordAddresses = []
        self.ExpandWindow = True
        self.ItemsToFilter = []
        
        # Define Attributes
        self.MainSplitContainer = Forms.SplitContainer()
        self.ListBoxContainer   = Forms.CheckedListBox()
        self.FilterToolStrip    = Forms.MenuStrip()
        self.SearchToolStrip    = Forms.MenuStrip()
        self.ApplyFilterButton  = Forms.ToolStripMenuItem()
        self.FilteredLogTextBox = Forms.RichTextBox()
        self.SaveButton         = Forms.ToolStripMenuItem()
        self.ClearOutput        = Forms.ToolStripMenuItem()
        self.searchTextBox      = Forms.ToolStripTextBox()
        self.searchLabel        = Forms.ToolStripLabel()
        self.searchResultLabel  = Forms.ToolStripLabel()
        
        # MainSplitContainer
        self.MainSplitContainer.Dock = Forms.DockStyle.Fill
        self.MainSplitContainer.Orientation = Forms.Orientation.Vertical
        self.MainSplitContainer.Size = Drawing.Size(275, 575)
        self.MainSplitContainer.SplitterDistance = 280
        self.MainSplitContainer.SplitterWidth = 10
        self.MainSplitContainer.TabStop = False
        
        # Add Controls
        # Panel1
        self.MainSplitContainer.Panel1.Controls.Add(self.ListBoxContainer)
        self.MainSplitContainer.Panel1.Controls.Add(self.FilterToolStrip)
        self.MainSplitContainer.Panel1.Controls.Add(self.SearchToolStrip)
        self.MainSplitContainer.FixedPanel = Forms.FixedPanel.Panel1
        
        # Panel2
        self.MainSplitContainer.Panel2.Controls.Add(self.FilteredLogTextBox)
        
        # FilteredLogTextBox
        self.FilteredLogTextBox.Dock = Forms.DockStyle.Fill
        self.FilteredLogTextBox.Font = Drawing.Font("Consolas", 10)
        self.FilteredLogTextBox.Multiline = True
        self.FilteredLogTextBox.ReadOnly = True
        self.FilteredLogTextBox.BackColor = Drawing.Color.White
        
        # ToolStrip
        self.FilterToolStrip.Dock = Forms.DockStyle.Bottom
        self.FilterToolStrip.GripStyle = Forms.ToolStripGripStyle.Hidden
        
        # SearchToolStrip
        self.SearchToolStrip.Dock = Forms.DockStyle.Bottom
        self.SearchToolStrip.GripStyle = Forms.ToolStripGripStyle.Hidden

        # Add Tool strip controls
        self.FilterToolStrip.Items.Add(self.ApplyFilterButton)   
        self.FilterToolStrip.Items.Add(Forms.ToolStripSeparator())
        self.FilterToolStrip.Items.Add(self.SaveButton)
        self.FilterToolStrip.Items.Add(self.ClearOutput)

        # Add Search tool strip controls
        self.SearchToolStrip.Items.Add(self.searchLabel)
        self.SearchToolStrip.Items.Add(self.searchTextBox)
        self.SearchToolStrip.Items.Add(self.searchResultLabel)

        # ListBox
        self.ListBoxContainer.Dock = Forms.DockStyle.Fill
        self.ListBoxContainer.MinimumSize = Drawing.Size(215, 500)
        componentList = self.dev.cobs.iparsed_file.get_icomponent_json(self.dev.device_name)
        self.ListBoxContainer.Items.AddRange(Array[str](componentList))
        self.ListBoxContainer.CheckOnClick =True

        # ApplyFilterButton
        self.ApplyFilterButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.ApplyFilterButton.Text = "&Filter"
        self.ApplyFilterButton.Click += self.ApplyFilter
        
        # SaveButton
        self.SaveButton.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.SaveButton.Text = "&ExportLog"
        self.SaveButton.Click += self.SaveFilteredLogs   
        
        # ClearOutput
        self.ClearOutput.DisplayStyle = Forms.ToolStripItemDisplayStyle.Text
        self.ClearOutput.Text = "&ClearOutput"
        self.ClearOutput.Click += self.ClearFilteredLogs

        #seachTextBox
        self.searchTextBox.Text = ""
        self.searchTextBox.Size = Drawing.Size(70, 5)
        self.searchTextBox.BorderStyle = Forms.BorderStyle.Fixed3D;
        self.searchTextBox.BackColor = Drawing.Color.White
        self.searchTextBox.KeyDown += self.searchText_Enter

        #searchLabel
        self.searchLabel.Text = "Search"

        #searchResultLabel
        self.searchResultLabel.Text = "0"
        
        # Window Settings
        self.ClientSize = Drawing.Size(250, 575)
        self.MinimumSize = Drawing.Size(250, 575)

        self.Controls.Add(self.MainSplitContainer)
        self.Icon = System.Drawing.Icon(dev.programPath + "\\cobsicon.ico")
        self.Text = "{} {}: Filter".format(self.dev.port_name, self.dev.device_name)
        
        self.Closed += self.exit
       
    
                
    def ApplyFilter(self, sender, args):
        try:
            
            self.wordAddresses = []
            filter = []
            # First get Checked items
            for item in self.ListBoxContainer.CheckedItems:
                filter.append(item.ToString())
            self.ItemsToFilter = filter
            
            cbs_logger.info("{}: {}: Applying [{}] filter to iLogs". \
                        format(self.dev.port_name, self.dev.device_name,  self.ItemsToFilter))
                                                
            self.dev.iLogFilterValues  = self.ItemsToFilter
            self.dev.FilterIndices = self.ListBoxContainer.CheckedIndices  

            self.ComponentSearch()
            if self.ExpandWindow:
                self.Width = 1000
                self.ExpandWindow  = False
            self.liveFilter_Activate()
        except:
            cbs_logger.info("{}: {}: Error when applying filter to iLogs". \
                        format(self.dev.port_name, self.dev.device_name))
            
    def ComponentSearch(self):
        try:
            startIndex = 44
            RelevantLogs = ""
            Lines = self.dev.devWindow.OutputTextBox.Lines
            for line in Lines:
                if len(line)>68:
                    for string in self.ItemsToFilter:
                        index = line.IndexOf(
                                        string,
                                        startIndex,
                                        len(string),
                                        System.StringComparison.OrdinalIgnoreCase)
                        if index != -1:
                            RelevantLogs = RelevantLogs + line + "\n"
            self.FilteredLogTextBox.Text = RelevantLogs                
        except:
            cbs_logger.info("{}: {}: Error when searching Components".
                                format(self.dev.port_name, self.dev.device_name))
            
    def liveFilter_Activate(self):
        try:
            self.dev.liveFilter = True
        except:
            cbs_logger.info("{}: {}: Error when applying live component Filter". \
                                format(self.dev.port_name, self.dev.device_name))

    def livePrint(self, log):
        try:
            text_box =self.FilteredLogTextBox
            text_box.SelectionStart = text_box.TextLength
            text_box.SelectionLength = 0
            text_box.AppendText(log)
            text_box.ScrollToCaret()
        except:
            cbs_logger.info("{}: {}: Error printing filtered log". \
                                format(self.dev.port_name, self.dev.device_name))

    def SaveFilteredLogs(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Exporting Filtered Logs". \
                                format(self.dev.port_name, self.dev.device_name))
            path = "Log\\FilteredLog_" + str(datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S"))
            os.mkdir(path)
            for (client_name, port_name) in self.dev.cobs.device_clients:
                device = self.dev.cobs.device_clients[(client_name, port_name)]
                if device.deviceFilter is not None:
                    f= open(path + "\\{}_{}_FilteredLog".format(port_name, client_name) + ".log", "w+")
                    f.write(device.deviceFilter.FilteredLogTextBox.Text.ToString())
                    f.close()
            os.startfile(path)
        except:
            cbs_logger.info("{}: {}: Error saving filtered logs". \
                                format(self.dev.port_name, self.dev.device_name))
                                
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
                self.FilteredLogTextBox.Select(wordAddress[LastValue - self.scrollValue],lengthOfWord)
                self.FilteredLogTextBox.SelectionBackColor = Drawing.Color.Yellow
                self.scrollValue +=1
            else:
                self.FilteredLogTextBox.Select(wordAddress[LastValue - self.scrollValue],lengthOfWord)
                self.FilteredLogTextBox.SelectionBackColor = Drawing.Color.Yellow
                self.scrollValue -=1
            if self.scrollValue > LastValue:
                self.scrollValue = 0
            elif self.scrollValue < 0:
                self.scrollValue = LastValue
            self.FilteredLogTextBox.Select(wordAddress[LastValue - self.scrollValue],lengthOfWord)
            self.FilteredLogTextBox.SelectionBackColor = Drawing.Color.Pink
            self.FilteredLogTextBox.ScrollToCaret()
        except:
            cbs_logger.exception("{}: {}: Got an error when Scrolling to text". \
                    format(self.dev.port_name, self.dev.device_name))
					
    def highlight_text(self, word, color=Drawing.Color.Yellow):
        try:
            if word:
                s_start = self.FilteredLogTextBox.SelectionStart
                self.FilteredLogTextBox.SelectionStart = 0;
                self.FilteredLogTextBox.SelectionLength = self.FilteredLogTextBox.TextLength;    
                self.FilteredLogTextBox.SelectionBackColor = Drawing.Color.White
                num_found = 0
                startIndex = 0
                index = self.FilteredLogTextBox.Text.IndexOf(
                                                    word,
                                                    startIndex,
                                                    System.StringComparison.OrdinalIgnoreCase)
                wordAddress=[]
                while index != -1:
                    num_found += 1
                    self.FilteredLogTextBox.Select(index, len(word));
                    self.FilteredLogTextBox.SelectionBackColor = color;
                    
                    startIndex = index + len(word);
                    wordAddress.append(index)
                    index = self.FilteredLogTextBox.Text.IndexOf(
                                                    word,
                                                    startIndex,
                                                    System.StringComparison.OrdinalIgnoreCase)
                if wordAddress != []:							 
                    self.FilteredLogTextBox.Select(wordAddress[-1], len(word));
                    self.FilteredLogTextBox.ScrollToCaret()
                    self.wordAddresses = wordAddress
                    self.FilteredLogTextBox.Select(wordAddress[-1],len(word))
                    self.FilteredLogTextBox.SelectionBackColor = Drawing.Color.Pink
                    self.searchResultLabel.Text = "{}".format(num_found)
                    self.FilteredLogTextBox.SelectionStart = s_start;
                    self.FilteredLogTextBox.SelectionLength = 0;
        except:
            cbs_logger.exception("{}: {}: Got an error when highlighting text". \
                    format(self.dev.port_name, self.dev.device_name))

    def ClearFilteredLogs(self, sender, args):
        """
        Clear the output textbox.

        Arguments: sender - unused
                   args - unused
        """
        try:
            cbs_logger.info("{}: {}: Clear Output Button is clicked". \
                        format(self.dev.port_name, self.dev.device_name))
            self.FilteredLogTextBox.Clear()
        except:
            cbs_logger.info("{}: {}: Error clearing filtered logs". \
                                format(self.dev.port_name, self.dev.device_name))

    def clear_highlight_text(self):
        try:
            cbs_logger.info("{}: {}: Clearing search results". \
                            format(self.dev.port_name, self.dev.device_name))
            self.wordAddresses = []
            self.searchResultLabel.Text = "0 found"
            self.FilteredLogTextBox.SelectionStart = 0;
            self.FilteredLogTextBox.SelectionLength = self.FilteredLogTextBox.TextLength;
            self.FilteredLogTextBox.SelectionBackColor = Drawing.Color.White
            self.FilteredLogTextBox.SelectionStart = self.FilteredLogTextBox.TextLength;
            self.FilteredLogTextBox.SelectionLength = 0
            self.FilteredLogTextBox.ScrollToCaret()
        except:
            cbs_logger.exception("{}: {}: Got an error when clearing the highlight". \
                    format(self.dev.port_name, self.dev.device_name))

    def run(self, initialSettings):
        """Runs the Window."""
        try:  
            #Set initial Settings
            if initialSettings is not None:
                for itemIndex in initialSettings:
                    self.ListBoxContainer.SetItemChecked(itemIndex, True)      
            # show the window
            self.Show()
        except:
            error_message = "Got an error when running Filter window"
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)
                
    def exit(self, sender, args):
        try:
            cbs_logger.info("Closing filter window")
            self.dev.liveFilter = False
            del self.dev.deviceFilter
        except:
            cbs_logger.info("Error closing filter window")
            
