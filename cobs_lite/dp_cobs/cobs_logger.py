import logging
import traceback
import os
import datetime

def setup_logging(
        log_file="cobs.log",
        log_level=logging.INFO,
        log_format="%(asctime)s %(levelname)s: %(message)s",
        echo_in_stdout = True):
    """A global function to setup Cobs' logging

    log_file_path - Cobs log file path
    log_level - Cobs log level, DEBUG by default
    log_format - Cobs log format
    echo_in_stdout - whether echo logs in stdout
    """

    log_file = log_file[:-4] + "_" + datetime.datetime.now(). \
            strftime('%Y_%m_%d_%H_%M_%S') + ".log"

    global_logger = logging.getLogger('cobs_logger')
    global_logger.setLevel(log_level)
    file_handler = logging.FileHandler(log_file)
    file_handler.setFormatter(logging.Formatter(log_format))
    global_logger.addHandler(file_handler)

    """
    logging.basicConfig(
            filename = "{}/cobs.log".format(log_file),
            level = log_level,
            format = log_format)
    """

    if echo_in_stdout:
        stdout = logging.StreamHandler()
        stdout.setFormatter(logging.Formatter(log_format))
        global_logger.addHandler(stdout)

    return global_logger

cbs_logger = setup_logging()
