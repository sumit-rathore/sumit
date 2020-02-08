###############################################################################
###
###   Icron Technology Corporation - Copyright 2019
###
###
###   This source file and the information contained in it are confidential and
###   proprietary to Icron Technology Corporation. The reproduction or disclosure,
###   in whole or in part, to anyone outside of Icron without the written approval
###   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
###   Icron who has not previously obtained written authorization for access from
###   the individual responsible for the source code, will have a significant
###   detrimental effect on Icron and is expressly prohibited.
###
###############################################################################
##
##!   @file  - add_comment.py
##
##!   @brief - 
##
##
###############################################################################

import sys
import os
import System
import System.Windows.Forms as Forms
import System.Threading as Threading
import Device as dev
from cobs_logger import cbs_logger

class add_comment(Forms.Form):
    def __init__(self, dev):
        self.device = dev
        self.Size = System.Drawing.Size(400, 150)
        self.Text = "COMMENT BOX"
        self.text = []
        self.doneButton = Forms.Button()
        self.commentBox = Forms.TextBox()

        # commentBox
        self.commentBox.Size = System.Drawing.Size(380, 50)
        self.commentBox.Location = System.Drawing.Point(2, 20)
        self.commentBox.Multiline = True
        self.commentBox.ScrollBars = Forms.ScrollBars.Vertical

        # doneButton
        self.doneButton.Size = System.Drawing.Size(90, 30)
        self.doneButton.Location = System.Drawing.Point(9, 80)
        self.doneButton.Text = " ADD "
        self.doneButton.Click += self.AddButton_Click

        self.Controls.Add(self.doneButton)
        self.Controls.Add(self.commentBox)

    def AddButton_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Add Button is clicked". \
                    format(self.device.port_name, self.device.device_name))
            self.device.devWindow.OutputTextBox.AppendText("___________________________________________________________\n")
            self.device.devWindow.OutputTextBox.AppendText(self.commentBox.Text + "\n")
            self.device.devWindow.OutputTextBox.AppendText("___________________________________________________________\n")
            self.commentBox.Clear()
        except Exception as e:
            error_msg = "{}: {}: Error occurred when clicking on Add Button : {}". \
                    format(self.device.port_name, self.device.device_name, e)
            cbs_logger.exception(error_msg)

    def run(self):
        """Runs options windows"""
        try:
            self.Show()
        except:
            error_message = "{}: {}: Run test window got an exception". \
                                format(self.device.port_name, self.device.device_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)
