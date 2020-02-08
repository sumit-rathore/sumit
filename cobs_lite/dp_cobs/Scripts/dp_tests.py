###############################################################################
###
###   Icron Technology Corporation - Copyright 2018
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
##!   @file  - dp_tests.py
##
##!   @brief - 
##
##
###############################################################################

import sys
import os
import System
import threading
import System.Windows.Forms as Forms
import System.Threading as Threading
import Device as dev
from cobs_logger import cbs_logger

class dp_test(Forms.Form):
    def __init__(self, dev):
        self.dev = dev
        self.Size = System.Drawing.Size(300, 700)
        self.Text = "Test Options:"
        self.text = []
        self.sscOn_Hbr2_LC4 = Forms.CheckBox()
        self.sscOn_Hbr2_LC2 = Forms.CheckBox()
        self.sscOn_Hbr2_LC1 = Forms.CheckBox()
        self.sscOn_Hbr_LC4 = Forms.CheckBox()
        self.sscOn_Hbr_LC2 = Forms.CheckBox()
        self.sscOn_Hbr_LC1 = Forms.CheckBox()
        self.sscOn_Rbr_LC4 = Forms.CheckBox()
        self.sscOn_Rbr_LC2 = Forms.CheckBox()
        self.sscOn_Rbr_LC1 = Forms.CheckBox()
        self.sscOff_Hbr2_LC4 = Forms.CheckBox()
        self.sscOff_Hbr2_LC2 = Forms.CheckBox()
        self.sscOff_Hbr2_LC1 = Forms.CheckBox()
        self.sscOff_Hbr_LC4 = Forms.CheckBox()
        self.sscOff_Hbr_LC2 = Forms.CheckBox()
        self.sscOff_Hbr_LC1 = Forms.CheckBox()
        self.sscOff_Rbr_LC4 = Forms.CheckBox()
        self.sscOff_Rbr_LC2 = Forms.CheckBox()
        self.sscOff_Rbr_LC1 = Forms.CheckBox()
        self.sscOnLabel = Forms.Label()
        self.sscOffLabel = Forms.Label()
        self.goButton = Forms.Button()
        self.passButton = Forms.Button()
        self.failButton = Forms.Button()
        self.doneButton = Forms.Button()
        self.saveButton = Forms.Button()
        self.commentBox = Forms.TextBox()
        self.resultFileDialog = Forms.SaveFileDialog()
        
        #sscOn_Hbr2_LC4
        self.sscOn_Hbr2_LC4.Size = System.Drawing.Size(150, 20)
        self.sscOn_Hbr2_LC4.Location = System.Drawing.Point(2, 30)
        self.sscOn_Hbr2_LC4.Checked = False
        self.sscOn_Hbr2_LC4.CheckedChanged += self.sscOn_Hbr2_LC4_Click
        self.sscOn_Hbr2_LC4.Text = "SSC ON, HBR2, LC4"

        #sscOn_Hbr2_LC2
        self.sscOn_Hbr2_LC2.Size = System.Drawing.Size(150, 20)
        self.sscOn_Hbr2_LC2.Location = System.Drawing.Point(2, 55)
        self.sscOn_Hbr2_LC2.Checked = False
        self.sscOn_Hbr2_LC2.CheckedChanged += self.sscOn_Hbr2_LC2_Click
        self.sscOn_Hbr2_LC2.Text = "SSC ON, HBR2, LC2"

        #sscOn_Hbr2_LC1
        self.sscOn_Hbr2_LC1.Size = System.Drawing.Size(150, 20)
        self.sscOn_Hbr2_LC1.Location = System.Drawing.Point(2, 80)
        self.sscOn_Hbr2_LC1.Checked = False
        self.sscOn_Hbr2_LC1.CheckedChanged += self.sscOn_Hbr2_LC1_Click
        self.sscOn_Hbr2_LC1.Text = "SSC ON, HBR2, LC1"

        #sscOn_Hbr_LC4
        self.sscOn_Hbr_LC4.Size = System.Drawing.Size(150, 20)
        self.sscOn_Hbr_LC4.Location = System.Drawing.Point(2, 105)
        self.sscOn_Hbr_LC4.Checked = False
        self.sscOn_Hbr_LC4.CheckedChanged += self.sscOn_Hbr_LC4_Click
        self.sscOn_Hbr_LC4.Text = "SSC ON, HBR, LC4"

        #sscOn_Hbr_LC2
        self.sscOn_Hbr_LC2.Size = System.Drawing.Size(150, 20)
        self.sscOn_Hbr_LC2.Location = System.Drawing.Point(2, 130)
        self.sscOn_Hbr_LC2.Checked = False
        self.sscOn_Hbr_LC2.CheckedChanged += self.sscOn_Hbr_LC2_Click
        self.sscOn_Hbr_LC2.Text = "SSC ON, HBR, LC2"

        #sscOn_Hbr_LC1
        self.sscOn_Hbr_LC1.Size = System.Drawing.Size(150, 20)
        self.sscOn_Hbr_LC1.Location = System.Drawing.Point(2, 155)
        self.sscOn_Hbr_LC1.Checked = False
        self.sscOn_Hbr_LC1.CheckedChanged += self.sscOn_Hbr_LC1_Click
        self.sscOn_Hbr_LC1.Text = "SSC ON, HBR, LC1"

        #sscOn_Rbr_LC4
        self.sscOn_Rbr_LC4.Size = System.Drawing.Size(150, 20)
        self.sscOn_Rbr_LC4.Location = System.Drawing.Point(2, 180)
        self.sscOn_Rbr_LC4.Checked = False
        self.sscOn_Rbr_LC4.CheckedChanged += self.sscOn_Rbr_LC4_Click
        self.sscOn_Rbr_LC4.Text = "SSC ON, RBR, LC4"

        #sscOn_Rbr_LC2
        self.sscOn_Rbr_LC2.Size = System.Drawing.Size(150, 20)
        self.sscOn_Rbr_LC2.Location = System.Drawing.Point(2, 205)
        self.sscOn_Rbr_LC2.Checked = False
        self.sscOn_Rbr_LC2.CheckedChanged += self.sscOn_Rbr_LC2_Click
        self.sscOn_Rbr_LC2.Text = "SSC ON, RBR, LC2"

        #sscOn_Rbr_LC1
        self.sscOn_Rbr_LC1.Size = System.Drawing.Size(150, 20)
        self.sscOn_Rbr_LC1.Location = System.Drawing.Point(2, 230)
        self.sscOn_Rbr_LC1.Checked = False
        self.sscOn_Rbr_LC1.CheckedChanged += self.sscOn_Rbr_LC1_Click
        self.sscOn_Rbr_LC1.Text = "SSC ON, RBR, LC1"

        #sscOff_Hbr2_LC4
        self.sscOff_Hbr2_LC4.Size = System.Drawing.Size(150, 20)
        self.sscOff_Hbr2_LC4.Location = System.Drawing.Point(2, 280)
        self.sscOff_Hbr2_LC4.Checked = False
        self.sscOff_Hbr2_LC4.CheckedChanged += self.sscOff_Hbr2_LC4_Click
        self.sscOff_Hbr2_LC4.Text = "SSC OFF, HBR2, LC4"

        #sscOff_Hbr2_LC2
        self.sscOff_Hbr2_LC2.Size = System.Drawing.Size(150, 20)
        self.sscOff_Hbr2_LC2.Location = System.Drawing.Point(2, 305)
        self.sscOff_Hbr2_LC2.Checked = False
        self.sscOff_Hbr2_LC2.CheckedChanged += self.sscOff_Hbr2_LC2_Click
        self.sscOff_Hbr2_LC2.Text = "SSC OFF, HBR2, LC2"

        #sscOff_Hbr2_LC1
        self.sscOff_Hbr2_LC1.Size = System.Drawing.Size(150, 20)
        self.sscOff_Hbr2_LC1.Location = System.Drawing.Point(2, 330)
        self.sscOff_Hbr2_LC1.Checked = False
        self.sscOff_Hbr2_LC1.CheckedChanged += self.sscOff_Hbr2_LC1_Click
        self.sscOff_Hbr2_LC1.Text = "SSC OFF, HBR2, LC1"

        #sscOff_Hbr_LC4
        self.sscOff_Hbr_LC4.Size = System.Drawing.Size(150, 20)
        self.sscOff_Hbr_LC4.Location = System.Drawing.Point(2, 355)
        self.sscOff_Hbr_LC4.Checked = False
        self.sscOff_Hbr_LC4.CheckedChanged += self.sscOff_Hbr_LC4_Click
        self.sscOff_Hbr_LC4.Text = "SSC OFF, HBR, LC4"

        #sscOff_Hbr_LC2
        self.sscOff_Hbr_LC2.Size = System.Drawing.Size(150, 20)
        self.sscOff_Hbr_LC2.Location = System.Drawing.Point(2, 380)
        self.sscOff_Hbr_LC2.Checked = False
        self.sscOff_Hbr_LC2.CheckedChanged += self.sscOff_Hbr_LC2_Click
        self.sscOff_Hbr_LC2.Text = "SSC OFF, HBR, LC2"

        #sscOff_Hbr_LC1
        self.sscOff_Hbr_LC1.Size = System.Drawing.Size(150, 20)
        self.sscOff_Hbr_LC1.Location = System.Drawing.Point(2, 405)
        self.sscOff_Hbr_LC1.Checked = False
        self.sscOff_Hbr_LC1.CheckedChanged += self.sscOff_Hbr_LC1_Click
        self.sscOff_Hbr_LC1.Text = "SSC OFF, HBR, LC1"

        #sscOff_Rbr_LC4
        self.sscOff_Rbr_LC4.Size = System.Drawing.Size(150, 20)
        self.sscOff_Rbr_LC4.Location = System.Drawing.Point(2, 430)
        self.sscOff_Rbr_LC4.Checked = False
        self.sscOff_Rbr_LC4.CheckedChanged += self.sscOff_Rbr_LC4_Click
        self.sscOff_Rbr_LC4.Text = "SSC OFF, RBR, LC4"

        #sscOff_Rbr_LC2
        self.sscOff_Rbr_LC2.Size = System.Drawing.Size(150, 20)
        self.sscOff_Rbr_LC2.Location = System.Drawing.Point(2, 455)
        self.sscOff_Rbr_LC2.Checked = False
        self.sscOff_Rbr_LC2.CheckedChanged += self.sscOff_Rbr_LC2_Click
        self.sscOff_Rbr_LC2.Text = "SSC OFF, RBR, LC2"

        #sscOff_Rbr_LC1
        self.sscOff_Rbr_LC1.Size = System.Drawing.Size(150, 20)
        self.sscOff_Rbr_LC1.Location = System.Drawing.Point(2, 480)
        self.sscOff_Rbr_LC1.Checked = False
        self.sscOff_Rbr_LC1.CheckedChanged += self.sscOff_Rbr_LC1_Click
        self.sscOff_Rbr_LC1.Text = "SSC OFF, RBR, LC1"

        #labelSscOn
        self.sscOnLabel.Location = System.Drawing.Point(2, 5)
        self.sscOnLabel.Size = System.Drawing.Size(150, 20)
        self.sscOnLabel.Text = "***** SSC ON *****"

        #labelSscOff
        self.sscOffLabel.Location = System.Drawing.Point(2, 255)
        self.sscOffLabel.Size = System.Drawing.Size(150, 20)
        self.sscOffLabel.Text = "***** SSC OFF *****"

        # goButton
        self.goButton.Size = System.Drawing.Size(90, 30)
        self.goButton.Location = System.Drawing.Point(40, 605)
        self.goButton.Text = "Go"
        self.goButton.Click += self.GoButton_Click

        # passButton
        self.passButton.Size = System.Drawing.Size(60, 30)
        self.passButton.Location = System.Drawing.Point(20, 570)
        self.passButton.Text = "Pass"
        self.passButton.Click += self.PassButton_Click

        # failButton
        self.failButton.Size = System.Drawing.Size(60, 30)
        self.failButton.Location = System.Drawing.Point(110, 570)
        self.failButton.Text = "Fail"
        self.failButton.Click += self.FailButton_Click

        # failButton
        self.saveButton.Size = System.Drawing.Size(60, 30)
        self.saveButton.Location = System.Drawing.Point(200, 570)
        self.saveButton.Text = "Save"
        self.saveButton.Click += self.saveButton_Click

        # commentBox
        self.commentBox.Size = System.Drawing.Size(240, 50)
        self.commentBox.Location = System.Drawing.Point(20, 510)
        # self.commentBox.Text = "Go"
        # self.commentBox.Click += self.GoButton_Click
        self.commentBox.Multiline = True
        self.commentBox.ScrollBars = Forms.ScrollBars.Vertical

        # doneButton
        self.doneButton.Size = System.Drawing.Size(90, 30)
        self.doneButton.Location = System.Drawing.Point(150, 605)
        self.doneButton.Text = "Done"
        self.doneButton.Click += self.DoneButton_Click

        self.resultFileDialog.InitialDirectory = self.dev.programPath + "\\test_results"
        self.resultFileDialog.Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*"

        self.Controls.Add(self.sscOn_Hbr2_LC4)
        self.Controls.Add(self.sscOn_Hbr2_LC2)
        self.Controls.Add(self.sscOn_Hbr2_LC1)
        self.Controls.Add(self.sscOn_Hbr_LC4)
        self.Controls.Add(self.sscOn_Hbr_LC2)
        self.Controls.Add(self.sscOn_Hbr_LC1)
        self.Controls.Add(self.sscOn_Rbr_LC4)
        self.Controls.Add(self.sscOn_Rbr_LC2)
        self.Controls.Add(self.sscOn_Rbr_LC1)
        self.Controls.Add(self.sscOff_Hbr2_LC4)
        self.Controls.Add(self.sscOff_Hbr2_LC2)
        self.Controls.Add(self.sscOff_Hbr2_LC1)
        self.Controls.Add(self.sscOff_Hbr_LC4)
        self.Controls.Add(self.sscOff_Hbr_LC2)
        self.Controls.Add(self.sscOff_Hbr_LC1)
        self.Controls.Add(self.sscOff_Rbr_LC4)
        self.Controls.Add(self.sscOff_Rbr_LC2)
        self.Controls.Add(self.sscOff_Rbr_LC1)
        self.Controls.Add(self.sscOnLabel)
        self.Controls.Add(self.sscOffLabel)
        self.Controls.Add(self.goButton)
        self.Controls.Add(self.passButton)
        self.Controls.Add(self.failButton)
        self.Controls.Add(self.doneButton)
        self.Controls.Add(self.saveButton)        
        self.Controls.Add(self.commentBox)


    def sscOn_Hbr2_LC4_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC ON, HBR2, LC4  button is clicked". \
                        format(self.dev.port_name, self.dev.device_name))
            if self.sscOn_Hbr2_LC4.Checked:
                self.sscOn_BW_LC(0x14, 0x4)
                self.text.append("###### TEST 1. HBR2 LC=4 SSC ON")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, HBR2, LC4". \
                     format(self.dev.port_name, self.dev.device_name))   
    
    def sscOn_Hbr2_LC2_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC ON, HBR2, LC2  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOn_Hbr2_LC2.Checked:
                self.sscOn_BW_LC(0x14, 0x2)
                self.text.append("###### TEST 2. HBR2 LC=2 SSC ON")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, HBR2, LC42". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOn_Hbr2_LC1_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC ON, HBR2, LC1  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOn_Hbr2_LC1.Checked:
                self.sscOn_BW_LC(0x14, 0x1)
                self.text.append("###### TEST 3. HBR2 LC=1 SSC ON")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, HBR2, LC1". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOn_Hbr_LC4_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC ON, HBR, LC4  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOn_Hbr_LC4.Checked:
                self.sscOn_BW_LC(0x0A, 0x4)
                self.text.append("###### TEST 4. HBR LC=4 SSC ON")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, HBR, LC4". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOn_Hbr_LC2_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC ON, HBR, LC2  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOn_Hbr_LC2.Checked:
                self.sscOn_BW_LC(0x0A, 0x2)
                self.text.append("###### TEST 5. HBR LC=2 SSC ON")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, HBR, LC2". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOn_Hbr_LC1_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC ON, HBR, LC1  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOn_Hbr_LC1.Checked:
                self.sscOn_BW_LC(0x0A, 0x1)
                self.text.append("###### TEST 6. HBR LC=1 SSC ON")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, HBR, LC1". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOn_Rbr_LC4_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC ON, RBR, LC4  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOn_Rbr_LC4.Checked:
                self.sscOn_BW_LC(0x06, 0x4)
                self.text.append("###### TEST 7. RBR LC=4 SSC ON")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, RBR, LC4". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOn_Rbr_LC2_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC ON, RBR, LC2  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOn_Rbr_LC2.Checked:
                self.sscOn_BW_LC(0x06, 0x2)
                self.text.append("###### TEST 8. RBR LC=2 SSC ON")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, RBR, LC2". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOn_Rbr_LC1_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC ON, RBR, LC1  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOn_Rbr_LC1.Checked:
                self.sscOn_BW_LC(0x06, 0x1)
                self.text.append("###### TEST 9. RBR LC=1 SSC ON")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, RBR, LC1". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOff_Hbr2_LC4_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC OFF, HBR2, LC4  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOff_Hbr2_LC4.Checked:
                self.sscOff_BW_LC(0x14, 0x4)
                self.text.append("###### TEST 10. HBR2 LC=4 SSC OFF")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, HBR2, LC4". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOff_Hbr2_LC2_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC OFF, HBR2, LC2  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOff_Hbr2_LC2.Checked:
                self.sscOff_BW_LC(0x14, 0x2)
                self.text.append("###### TEST 11. HBR2 LC=2 SSC OFF")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, HBR2, LC2". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOff_Hbr2_LC1_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC OFF, HBR2, LC1  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOff_Hbr2_LC1.Checked:
                self.sscOff_BW_LC(0x14, 0x1)
                self.text.append("###### TEST 12. HBR2 LC=1 SSC OFF")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, HBR2, LC1". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOff_Hbr_LC4_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC OFF, HBR, LC4  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOff_Hbr_LC4.Checked:
                self.sscOff_BW_LC(0x0A, 0x4)
                self.text.append("###### TEST 13. HBR LC=4 SSC OFF")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, HBR, LC4". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOff_Hbr_LC2_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC OFF, HBR, LC2  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOff_Hbr_LC2.Checked:
                self.sscOff_BW_LC(0x0A, 0x2)
                self.text.append("###### TEST 14. HBR LC=2 SSC OFF")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, HBR, LC2". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOff_Hbr_LC1_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC OFF, HBR, LC1  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOff_Hbr_LC1.Checked:
                self.sscOff_BW_LC(0x0A, 0x1)
                self.text.append("###### TEST 15. HBR LC=1 SSC OFF")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, HBR, LC1". \
                     format(self.dev.port_name, self.dev.device_name))

    def sscOff_Rbr_LC4_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC OFF, RBR, LC4  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOff_Rbr_LC4.Checked:
                self.sscOff_BW_LC(0x06, 0x4)
                self.text.append("###### TEST 16. RBR LC=4 SSC OFF")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, RBR, LC4". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOff_Rbr_LC2_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC OFF, RBR, LC2  button is clicked". \
                format(self.dev.port_name, self.dev.device_name))
            if self.sscOff_Rbr_LC2.Checked:
                self.sscOff_BW_LC(0x06, 0x2)
                self.text.append("###### TEST 17. RBR LC=2 SSC OFF")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, RBR, LC2". \
                     format(self.dev.port_name, self.dev.device_name)) 

    def sscOff_Rbr_LC1_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: SSC OFF, RBR, LC1  button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            if self.sscOff_Rbr_LC1.Checked:
                self.sscOff_BW_LC(0x06, 0x1)
                self.text.append("###### TEST 18. RBR LC=1 SSC OFF")
            else:
                self.defaultValues()
        except:
            cbs_logger.exception("{}: {}: Got an error when adding button SSC ON, RBR, LC1". \
                     format(self.dev.port_name, self.dev.device_name)) 
    
    def sscOn_BW_LC(self, BW, LC):
        ssc_icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_LEX_SscAdvertiseEnable", False, [1])
        bw_lc_icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_SetBwLc", False, [BW, LC])
        icmd_thread_1 = Threading.Thread(
                                Threading.ParameterizedThreadStart(self.dev.send_icmd))
        icmd_thread_2 = Threading.Thread(
                                Threading.ParameterizedThreadStart(self.dev.send_icmd))
        icmd_thread_1.Name = self.Text.replace("Cobs", "ssc")
        icmd_thread_2.Name = self.Text.replace("Cobs", "LC_BW")
        icmd_thread_1.Start(ssc_icmd_obj)
        icmd_thread_2.Start(bw_lc_icmd_obj)

    def sscOff_BW_LC(self, BW, LC):
        ssc_icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_LEX_SscAdvertiseEnable", False, [0])
        bw_lc_icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_SetBwLc", False, [BW, LC])
        icmd_thread_1 = Threading.Thread(
                                Threading.ParameterizedThreadStart(self.dev.send_icmd))
        icmd_thread_2 = Threading.Thread(
                                Threading.ParameterizedThreadStart(self.dev.send_icmd))
        icmd_thread_1.Name = self.Text.replace("Cobs", "ssc")
        icmd_thread_2.Name = self.Text.replace("Cobs", "LC_BW")
        icmd_thread_1.Start(ssc_icmd_obj)
        icmd_thread_2.Start(bw_lc_icmd_obj)

    def defaultValues(self):
        ssc_icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_LEX_SscAdvertiseEnable", False, [2])
        bw_lc_icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_SetBwLc", False, [0, 0])
        icmd_thread_1 = Threading.Thread(
                                Threading.ParameterizedThreadStart(self.dev.send_icmd))
        icmd_thread_2 = Threading.Thread(
                                Threading.ParameterizedThreadStart(self.dev.send_icmd))
        icmd_thread_1.Name = self.Text.replace("Cobs", "ssc")
        icmd_thread_2.Name = self.Text.replace("Cobs", "LC_BW")
        icmd_thread_1.Start(ssc_icmd_obj)
        icmd_thread_2.Start(bw_lc_icmd_obj)

    def GoButton_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Go Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            self.restartStateMachine()
        except Exception as e:
            error_msg = "{}: {}: Error occurred when clicking on Go : {}". \
                    format(self.dev.port_name, self.dev.device_name, e)
            cbs_logger.exception(error_msg)

    def PassButton_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Pass Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            self.text.append("* Test passed ")
        except Exception as e:
            error_msg = "{}: {}: Error occurred when clicking on Pass Button : {}". \
                    format(self.dev.port_name, self.dev.device_name, e)
            cbs_logger.exception(error_msg)

    def FailButton_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Fail Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            self.text.append("* Test Failed ")
        except Exception as e:
            error_msg = "{}: {}: Error occurred when clicking on Fail Button : {}". \
                    format(self.dev.port_name, self.dev.device_name, e)
            cbs_logger.exception(error_msg)

    def saveButton_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Save Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            self.text.append('*' + self.commentBox.Text)
            self.commentBox.Clear()
        except Exception as e:
            error_msg = "{}: {}: Error occurred when clicking on Fail Button : {}". \
                    format(self.dev.port_name, self.dev.device_name, e)
            cbs_logger.exception(error_msg)

    def DoneButton_Click(self, sender, args):
        try:
            cbs_logger.info("{}: {}: Done Button is clicked". \
                    format(self.dev.port_name, self.dev.device_name))
            self.resultFileDialog.ShowDialog()
             
            with open (self.resultFileDialog.FileName, "w+") as f:
                for item in range(len(self.text)):
                    # print self.text[item]
                    f.write(self.text[item] + '\n')
                self.text = []
            self.defaultValues()
            self.restartStateMachine()
            self.Close()
        except Exception as e:
            error_msg = "{}: {}: Error occurred when clicking on Fail Button : {}". \
                    format(self.dev.port_name, self.dev.device_name, e)
            cbs_logger.exception(error_msg)

    def restartStateMachine(self):
        icmd_obj = self.dev.create_icmd("DP_COMPONENT", "DP_RestartDPStateMachine", False)
        swVersion_thread = Threading.Thread(
                            Threading.ParameterizedThreadStart(self.dev.send_icmd))
        swVersion_thread.Name = self.Text.replace("Cobs", "RestartDP")
        swVersion_thread.Start(icmd_obj)

    def run(self):
        """Runs options windows"""
        try:
            self.Show()
        except:
            error_message = "{}: {}: Run test window got an exception". \
                                format(self.dev.port_name, self.dev.device_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)