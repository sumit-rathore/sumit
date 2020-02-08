import logging
import datetime
from cobs_logger import cbs_logger

class IcronDeviceLogger():
    """"Singleton class for logging purposes."""
    def __init__(self, prefix):
        """
        A logger for the icron device logs.
        """
        self.output_logger = logging.getLogger(prefix)
        self.output_logger.setLevel(logging.DEBUG)
        self.output_handler = logging.FileHandler(
                                "Log\{}_".format(prefix) + \
                                str(datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S")) + ".log")
        self.output_logger.addHandler(self.output_handler)

    def log(self, string):
        """
        Logs the string to the file.
        """
        self.output_logger.info(string.rstrip())

    def create_new_log_file(self, file_name_prefix):
        """
        Save contents of the icron ilogs to a new file and dump all the contents of the log file.
        """
        try:
            self.output_logger.removeHandler(self.output_handler)
            self.output_handler.flush()
            self.output_handler.close()
            new_handler = logging.FileHandler(
                            "Log\{}_".format(file_name_prefix) + \
                            str(datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S"))+ ".log")
            self.output_logger.addHandler(new_handler)
            self.output_handler = new_handler
        except:
            cbs_logger.exception("Got an error when creating new log file") 
