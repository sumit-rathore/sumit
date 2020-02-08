from collections import OrderedDict

from System import EventHandler

class ge_monitor_xlrc_stats:
    def __init__(self, dev):
        """Creates the first row of what is to become a .csv file, and
initializes the count of each appropriate iLog.

Argument: dev - the Device to monitor"""
        # contains keys of iLog names, and values of pointers to actual iLogs
        self.iLogDict = OrderedDict()
        
        # contains a string that will be written to a .csv file
        self.data_string = "Test Name"
        
        for header in ("FLOW_CONTROL_CTRL_OUT_OVERFLOW",
                       "FLOW_CONTROL_CTRL_OUT",
                       "FLOW_CONTROL_INTRP_OUT_OVERFLOW",
                       "FLOW_CONTROL_INTRP_OUT",
                       "FLOW_CONTROL_ISO_OUT_OVERFLOW",
                       "FLOW_CONTROL_ISO_OUT",
                       "FLOW_CONTROL_BULK_OUT_OVERFLOW",
                       "FLOW_CONTROL_BULK_OUT",
                       "FLOW_CONTROL_CTRL_IN_OVERFLOW",
                       "FLOW_CONTROL_CTRL_IN",
                       "FLOW_CONTROL_INTRP_IN_OVERFLOW",
                       "FLOW_CONTROL_INTRP_IN",
                       "FLOW_CONTROL_ISO_IN_OVERFLOW",
                       "FLOW_CONTROL_ISO_IN",
                       "FLOW_CONTROL_BULK_IN_OVERFLOW",
                       "FLOW_CONTROL_BULK_IN",
                       "STAT_CNT_ERR_BABEL_OVERFLOW",
                       "STAT_CNT_ERR_BABEL",
                       "STAT_CNT_ERR_BITSTUFF_OVERFLOW",
                       "STAT_CNT_ERR_BITSTUFF",
                       "STAT_CNT_ERR_PID_OVERFLOW",
                       "STAT_CNT_ERR_PID",
                       "STAT_CNT_ERR_CRC5_OVERFLOW",
                       "STAT_CNT_ERR_CRC5",
                       "STAT_CNT_ERR_CRC16_OVERFLOW",
                       "STAT_CNT_ERR_CRC16",
                       "STAT_CNT_ERR_LINK_OVERFLOW",
                       "STAT_CNT_ERR_LINK"):
            # populate the first line of data_string
            self.data_string += ", " + header
            
            # find the iLog reference in the Device
            for log in dev.iLogs["XLRC_COMPONENT"]:
                if log.name == header:
                    # populate iLogDict
                    self.iLogDict[header] = log
                    
                    # initialize a counter in the iLog
                    log.count = 0
                    break
    
    def start(self, test_name):
        """Adds event handlers to each iLog to begin a test.

Argument: test_name - will be written to the .csv file"""
        # self.data_string will be written to in stop(), not here
        # thus, save the test name for later
        self.test_name = test_name
        
        # add to each appropriate iLog's event handler
        for iLog in self.iLogDict.values():
            iLog.events += increment_counter
    
    def increment_counter(self, sender, args):
        """Increases the iLog's counter.

Arguments: sender - the iLog containing the counter to increase
           args - unused"""
        # if the iLog has an arg, add it to the count
        # otherwise, it's an overflow iLog, so increment the count
        try:
            sender.count += sender.args[0]
        except IndexError:
            sender.count = sender.count + 1
    
    def stop(self):
        """Removes the event handlers from each iLog, and writes the old
results to what will be a .csv file."""        
        # add test name to data string
        self.data_string += "\n" + self.test_name
        
        for iLog in self.iLogDict.values():
            # remove event handler, write the old count, and reset the count
            iLog.events -= increment_counter
            self.data_string += ", " + iLog.count
            iLog.count = 0
    
    def writeFile(self, file_name):
        """Writes the collected data to a .csv file.

Argument: file_name - the file name to write to"""
        # we assume the user includes a ".csv" extension
        file = open(file_name, "w")
        file.write(self.data_string)
        file.close()