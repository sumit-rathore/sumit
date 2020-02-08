import sys
import CobsInterpreter as ci
from cobs_logger import cbs_logger

import clr
clr.AddReference("PresentationCore")
clr.AddReferenceToFile("IronPython.dll")
import System
import System.Windows.Forms as Forms
from System.Windows.Forms import Control, Application

# only start the program if this file is the one that was opened
if __name__ == "__main__":
    try:
        # necessary for console input to have an effect on the window
        # Apparently terrible practice that should only ever be used for debugging purposes.
        Control.CheckForIllegalCrossThreadCalls = False

        # open the main window
        Application.EnableVisualStyles()
        Application.SetCompatibleTextRenderingDefault(False)
        interpreter = ci.CobsInterpreter()
        interpreter.run()
    except:
        error_message = "Cobs driver got an exception"
        cbs_logger.exception(error_message)
        error_string = error_message + "\n" + \
                "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
        Forms.MessageBox.Show(
            error_string,
            "ERROR",
            Forms.MessageBoxButtons.OK,
            Forms.MessageBoxIcon.Error)
