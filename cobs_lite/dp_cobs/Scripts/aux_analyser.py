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
##!   @file  - aux_analyser.py
##
##!   @brief - This file takes a blackbird log file, and filters it to grab aux
##             transaction logs and decode those to meaningful logs (follows the
##             aux transaction format from unigraph aux controller)
##
##
###############################################################################

import sys
import os
from cobs_logger import cbs_logger
import System
import System.Windows.Forms as Forms


LINK_RATE = {
    0x06: "MAX_LINK_RATE = 1.62Gbps",
    0x0a: "MAX_LINK_RATE = 2.7Gbps",
    0x14: "MAX_LINK_RATE = 5.4Gbps",
    0x1e: "MAX_LINK_RATE = 8.1Gbps"
}

RX_CAP = {
    0x0: "DPCD_REV 0x00000                 := ",
    0x1: "MAX_LINK_RATE 0x00001            := ",
    0x2: "MAX_LANE_COUNT 0x00002           := ",
    0x3: "MAX_DOWNSPREAD 0x00003           := ",
    0x4: "NORP 0x00004                     := ",
    0x5: "DOWNSTREAM_PORT_PRESENT 0x00005  := ",
    0x6: "MAIN_LINK_CHANNEL_CODING 0x00006 := ",
    0x7: "DOWN_STREAM_PORT_COUNT 0x00007   := ",
    0x8: "RECEIVER_PORT0_CAP_0 0x00008     := ",
    0x9: "RECEIVER_PORT0_CAP_1 0x00009     := ",
    0xa: "RECEIVER_PORT1_CAP_0 [ 0x0000a   := ",
    0xb: "RECEIVER_PORT1_CAP_1 0x0000b     := ",
    0xc: "I2C_SPEED_CONTROL_CAPS 0x0000c   := ",
    0xd: "eDP_CONFIGURATION_CAP 0x0000d    := ",
    0xe: "TRAINING_AUX_RD_INTERVAL 0x0000e := ",
    0xf: "ADAPTER_CAP 0x0000f              := ",
}

LINK_CONFIG = {
    0x100: "LINK_BW_SET 0x00100          := ",
    0x101: "LANE_COUNT_SET 0x00101       := ",
    0x102: "TRAINING_PATTERN_SET 0x00102 := ",
    0x103: "TRAINING_LANE0_SET 0x00103   := ",
    0x104: "TRAINING_LANE1_SET 0x00104   := ",
    0x105: "TRAINING_LANE2_SET 0x00105   := ",
    0x106: "TRAINING_LANE3_SET 0x00106   := ",
    0x107: "DOWNSPREAD_CTRL 0x00107      := ",
}

SINK_STATUS = {
    0x200: "SINK_COUNT 0x00200                := ",
    0x201: "DEVICE_SERVICE_IRQ_VECTOR 0x00201 :=",
    0x202: "LANE0_1_STATUS 0x00202            :=",
    0x203: "LANE2_3_STATUS 0x00203            :=",
    0x204: "LANE_ALIGN_STATUS_UPDATED 0x00204 :=",
    0x205: "SINK_STATUS 0x00205               :=",
    0x206: "ADJUST_REQUEST_LANE0_1 0x00206    :=",
    0x207: "ADJUST_REQUEST_LANE2_3 0x00207    :="
}

class aux_trace():
    def __init__(self, dev, path):
        self.string_1 = " "
        self.device = dev
        self.byteSize = 0
        self.reqSize = 0
        self.reqAdd = ""
        self.auxAck = ""
        self.auxReq = ""
        self.sourceType = ""
        self.hexDump = ""
        self.reqStatus = False
        self.folderPath = path
        self.path = path
        self.reqType = ""
        cbs_logger.info("Device Type : {}".format(self.device.device_type))
        if self.device.device_type == "REX":
            self.path = self.folderPath + '\\aux_transaction_REX.txt'
            with open(self.path, "w") as aux_transaction_r:
                cbs_logger.info("output REX file opened")
                for line in self.device.devWindow.OutputTextBox.Text.ToString().split('\n'):
                    self.line = line + "\n"
                    if "Word" in self.line:
                        self.hexDump = self.line.split("0x", 1)[1].split("\n", 1)[0]
                        self.wordNum = int(self.line.split("Word = ", 1)[1].split(" |", 1)[0])
                        if self.wordNum is 0:
                            self.reqStatus = not self.reqStatus
                            if (self.reqStatus is False) and ((self.hexDump[0] == "0") or (self.hexDump[0] == "2")):
                                self.reqStatus = False
                            else:
                                self.reqStatus = True


                        if self.reqStatus:
                            if self.wordNum == 0:
                                self.reqType = self.hexDump[0]
                                if (self.reqType == "1") or (self.reqType == "5") or (self.reqType == "9"):
                                    self.reqSize = 4
                                    self.byteSize = int(self.hexDump[7], 16) + 2
                                else:
                                    self.reqSize = int(self.hexDump[7], 16) + 5
                                    self.byteSize = 1
                                self.auxReq = self.hexDump
                            else:
                                self.auxReq += self.hexDump


                            if self.reqSize <= 4 and self.wordNum == 0:
                                self.aux_req_write(aux_transaction_r)
                            elif self.reqSize > 4 and self.reqSize <= 8 and self.wordNum == 1:
                                self.aux_req_write(aux_transaction_r)
                            elif self.reqSize > 8 and self.reqSize <= 12 and self.wordNum == 2:
                                self.aux_req_write(aux_transaction_r)
                            elif self.reqSize > 12 and self.reqSize <= 16 and self.wordNum == 3:
                                self.aux_req_write(aux_transaction_r)
                            elif self.reqSize > 16 and self.reqSize <= 20 and self.wordNum == 4:
                                self.aux_req_write(aux_transaction_r)

                        else:
                            if self.wordNum == 0:
                                if self.hexDump[0] == "2":
                                    self.byteSize = 1
                                    self.auxAck = "Sink\t" + self.sourceType + "\t" + \
                                                  "AUX_DEFER - 1 bytes\t\t\t" + self.hexDump
                                else:
                                    self.auxAck = "Sink\t" + self.sourceType + "\t" + "AUX_ACK - " + \
                                                  str(self.byteSize) + " bytes\t\t\t" + self.hexDump
                            else:
                                self.auxAck += self.hexDump

                            if self.byteSize <= 4 and self.wordNum == 0:
                                self.aux_reply_write(aux_transaction_r)
                            elif self.byteSize > 4 and self.byteSize <= 8 and self.wordNum == 1:
                                self.aux_reply_write(aux_transaction_r)
                            elif self.byteSize > 8 and self.byteSize <= 12 and self.wordNum == 2:
                                self.aux_reply_write(aux_transaction_r)
                            elif self.byteSize > 12 and self.byteSize <= 16 and self.wordNum == 3:
                                self.aux_reply_write(aux_transaction_r)
                            elif self.byteSize > 16 and self.byteSize <= 20 and self.wordNum == 4:
                                self.aux_reply_write(aux_transaction_r)

            aux_transaction_r.close()
            auxFile = open(self.folderPath + "\\AuxInterpreted_REX.txt", "w")
            aux_interpreter(auxFile, self.path)
            auxFile.close()

        elif self.device.device_type == "LEX":
            self.path = self.folderPath + '\\aux_transaction_LEX.txt'
            with open(self.path, "w") as aux_transaction_l:
                cbs_logger.info("output REX file opened")
                for line in self.device.devWindow.OutputTextBox.Text.ToString().split('\n'):
                    self.line = line + "\n"
                    if "Word" in self.line:
                        self.hexDump = self.line.split("0x", 1)[1].split("\n", 1)[0]
                        self.wordNum = int(self.line.split("Word = ", 1)[1].split(" |", 1)[0])
                        if self.wordNum is 0:
                            self.reqStatus = not self.reqStatus
                            if (self.reqStatus is False) and ((self.hexDump[0] == "0") or (self.hexDump[0] == "2")):
                                self.reqStatus = False
                            else:
                                self.reqStatus = True

                        if self.reqStatus:
                            if self.wordNum == 0:
                                self.reqType = self.hexDump[0]
                                if (self.reqType == "1") or (self.reqType == "5") or (self.reqType == "9"):
                                    self.reqSize = 4
                                    self.byteSize = int(self.hexDump[7], 16) + 2
                                else:
                                    self.reqSize = int(self.hexDump[7], 16) + 5
                                    self.byteSize = 1
                                self.auxReq = self.hexDump
                            else:
                                self.auxReq += self.hexDump

                            if self.reqSize <= 4 and self.wordNum == 0:
                                self.aux_req_write(aux_transaction_l)
                            elif self.reqSize > 4 and self.reqSize <= 8 and self.wordNum == 1:
                                self.aux_req_write(aux_transaction_l)
                            elif self.reqSize > 8 and self.reqSize <= 12 and self.wordNum == 2:
                                self.aux_req_write(aux_transaction_l)
                            elif self.reqSize > 12 and self.reqSize <= 16 and self.wordNum == 3:
                                self.aux_req_write(aux_transaction_l)
                            elif self.reqSize > 16 and self.reqSize <= 20 and self.wordNum == 4:
                                self.aux_req_write(aux_transaction_l)

                        else:
                            if self.wordNum == 0:
                                if self.hexDump[0] == "0":
                                    self.auxAck = "Sink\t" + self.sourceType + "\t" + "AUX_ACK - " + str(
                                        self.byteSize) + " bytes\t\t\t" + self.hexDump
                                elif self.hexDump[0] == "2":
                                    self.byteSize = 1
                                    self.auxAck = "Sink\t" + self.sourceType + "\t" + "AUX_DEFER - 1 byte\t\t\t" + self.hexDump
                            else:
                                self.auxAck += self.hexDump

                            if self.byteSize <= 4 and self.wordNum == 0:
                                self.aux_reply_write(aux_transaction_l)
                            elif self.byteSize > 4 and self.byteSize <= 8 and self.wordNum == 1:
                                self.aux_reply_write(aux_transaction_l)
                            elif self.byteSize > 8 and self.byteSize <= 12 and self.wordNum == 2:
                                self.aux_reply_write(aux_transaction_l)
                            elif self.byteSize > 12 and self.byteSize <= 16 and self.wordNum == 3:
                                self.aux_reply_write(aux_transaction_l)
                            elif self.byteSize > 16 and self.byteSize <= 20 and self.wordNum == 4:
                                self.aux_reply_write(aux_transaction_l)

            aux_transaction_l.close()
            auxFile = open(self.folderPath + "\\AuxInterpreted_LEX.txt", "w")
            aux_interpreter(auxFile, self.path)
            auxFile.close()

        os.startfile(self.folderPath)

    def aux_reply_write(self, auxfile):
        self.rawHexDump = self.auxAck.split("bytes\t\t\t", 1)[1]

        self.auxAck = self.auxAck.split("\t\t\t", 1)[0]
        self.auxAck += "\t\t\t" + self.splitHexString(self.rawHexDump, self.byteSize)
        self.auxAck += "\n"
        auxfile.write(self.auxAck)

    def aux_req_write(self, auxfile):
        self.reqString = ""
        localbyteSize = int(self.auxReq[7], 16) + 1
        self.reqAdd = self.auxReq[1:6]
        if self.auxReq[0] == "8":
            self.string_1 = "Source\tNative\tReq WR "
            self.sourceType = "Native"
            self.reqString = self.string_1 + str(localbyteSize) + " bytes to 0x" + self.reqAdd + "\t\t"
        elif self.auxReq[0] == "9":
            self.string_1 = "Source\tNative\tReq RD "
            self.sourceType = "Native"
            self.reqString = self.string_1 + str(localbyteSize) + " bytes from 0x" + self.reqAdd + "\t\t"
        elif self.auxReq[0] == "4":
            self.string_1 = "Source\tI2C\tReq WR, MOT = 0, "
            self.sourceType = "I2C"
            self.reqString = self.string_1 + str(localbyteSize) + " bytes to 0x" + self.reqAdd + "\t"
        elif self.auxReq[0] == "5":
            self.string_1 = "Source\tI2C\tReq RD, MOT = 1, "
            self.sourceType = "I2C"
            self.reqString = self.string_1 + str(localbyteSize) + " bytes from 0x" + self.reqAdd + "\t"
        elif self.auxReq[0] == "1":
            self.string_1 = "Source\tI2C\tReq RD, MOT = 0, "
            self.sourceType = "I2C"
            self.reqString = self.string_1 + str(localbyteSize) + " bytes from 0x" + self.reqAdd + "\t"
        elif self.auxReq[0] == "0":
            self.string_1 = "Source\tI2C\tReq WR, MOT = 1, "
            self.sourceType = "I2C"
            self.reqString = self.string_1 + str(localbyteSize) + " bytes to 0x" + self.reqAdd + "\t"

        self.reqString += self.splitHexString(self.auxReq, self.reqSize) + "\n"
        auxfile.write(self.reqString)

    def splitHexString(self, hexString, count):
        self.spacedHex = ""
        self.hexCount = 0
        for i in range(0, len(hexString)):
            self.spacedHex += hexString[i]
            if i % 2:
                self.spacedHex += " "
                self.hexCount += 1
                if self.hexCount == count:
                    break
        return self.spacedHex

class aux_interpreter():
    def __init__(self, finalFile, auxFilePath):
        self.auxPath = auxFilePath
        self.lineSplit = ""
        self.auxAdd = 0
        self.auxType = ""
        self.auxReqType = ""
        self.auxHexDump = ""
        self.enhancedFrame = 0
        self.tpsSupp = 0
        self.numBytes = 0
        self.currentAdd = 0
        self.edidDump = []
        with open(self.auxPath, "r") as auxHexDump:
            cbs_logger.info("{} opened".format(self.auxPath))
            for line in auxHexDump.read().split('\n'):
                self.line = line + "\n"
                # print(self.line)
                finalFile.write(self.line)
                if "Source" in self.line:
                    self.lineSplit = self.line.split("\t")
                    self.auxType = self.lineSplit[1]
                    self.auxHexDump = self.lineSplit[len(self.lineSplit) - 1]
                    self.auxHexDump = self.auxHexDump.split(" ")
                    self.numBytes = int(self.lineSplit[2].split(" bytes", 1)[0].split(" ")[len(self.lineSplit[2].split(" bytes", 1)[0].split(" ")) - 1])
                    self.auxAdd = int(self.line.split("0x", 1)[1].split("\t", 1)[0], 16)
                    if "Req WR" in self.line:
                        if self.auxAdd in range(0x100, 0x108):
                            for address in range(0, self.numBytes):
                                if address == 0x108:
                                    break
                                finalFile.write(LINK_CONFIG[self.auxAdd + address] + "0x{}".format(self.auxHexDump[4 + address]) + "\n")

                if ("Sink" in self.line) and ("AUX_ACK" in self.line):
                    self.lineSplit = self.line.split("\t")
                    self.auxHexDump = self.lineSplit[len(self.lineSplit) - 1]
                    self.auxHexDump = self.auxHexDump.split(" ")
                    self.numBytes = int(self.line.split("AUX_ACK - ", 1)[1].split(" bytes", 1)[0])
                    if (self.auxAdd in range(0x0, 0x10)) and (self.numBytes > 1):
                        for address in range(0, self.numBytes - 1):
                            # print(RX_CAP[self.auxAdd + address] + "{}".format((self.auxHexDump[1 + address])))
                            finalFile.write(RX_CAP[self.auxAdd + address] + "0x{}".format((self.auxHexDump[1 + address])) + "\n")
                            self.auxAckInterpreter(self.auxAdd + address,  int(self.auxHexDump[1 + address], 16), finalFile)

                    if (self.auxAdd in range(0x100, 0x108)) and (self.numBytes > 1):
                        for address in range(0, self.numBytes - 1):
                            if self.auxAdd + address == 0x108:
                                break
                            # print(LINK_CONFIG[self.auxAdd + address] + "{}".format((self.auxHexDump[1 + address])))
                            finalFile.write(LINK_CONFIG[self.auxAdd + address] + "0x{}".format((self.auxHexDump[1 + address])) + "\n")
                            self.auxAckInterpreter(self.auxAdd + address,  int(self.auxHexDump[1 + address], 16), finalFile)

                    if (self.auxAdd in range(0x200, 0x208)) and (self.numBytes > 1):
                        for address in range(0, self.numBytes - 1):
                            # print(SINK_STATUS[self.auxAdd + address] + "{}".format(self.auxHexDump[1 + address]))
                            finalFile.write(SINK_STATUS[self.auxAdd + address] + "0x{}".format(self.auxHexDump[1 + address]) + "\n")
                            self.auxAckInterpreter(self.auxAdd + address, int(self.auxHexDump[1 + address], 16), finalFile)

                    if (self.auxAdd == 0x50) and (self.numBytes > 16):
                        self.newHexDump = self.auxHexDump[1:len(self.auxHexDump) - 1]
                        for hexNum in self.newHexDump:
                            if (hexNum != '\n'):
                                self.edidDump.append(int(hexNum, 16))
            if (len(self.edidDump) >= 128):
                self.edidInterpreter(self.edidDump, finalFile)

    def dpcdRevInterpreter(self, hexVal, outFile):
        self.minorRevNum = hexVal & 0xF
        self.majorRevNum = (hexVal & 0xF0) >> 4
        # print("\tDPCD V{}.{}".format(self.majorRevNum, self.minorRevNum))
        outFile.write("\tDPCD V{}.{}\n".format(self.majorRevNum, self.minorRevNum))

    def maxLinkRateInterpreter(self, hexVal, outFile):
        # print("\t" + LINK_RATE[hexVal])
        outFile.write("\t" + LINK_RATE[hexVal] + "\n")

    def maxLaneCountInterpreter(self, hexVal, outFile):
        self.maxLaneCount = hexVal & 0x1F
        self.tpsSupp = (hexVal & 0x40) >> 6
        self.enhancedFrameCap = (hexVal & 0x80) >> 7
        # print("\tMAX_LANE_COUNT = {}".format(self.maxLaneCount))
        # print("\tENHANCED_FRAME_CAP = {}".format(self.enhancedFrameCap))
        # print("\tTPS3_SUPPORTED = {}".format(self.tpsSupp))
        outFile.write("\tMAX_LANE_COUNT = {}\n".format(self.maxLaneCount))
        outFile.write("\tENHANCED_FRAME_CAP = {}\n".format(self.enhancedFrameCap))
        outFile.write("\tTPS3_SUPPORTED = {}\n".format(self.tpsSupp))


    def maxDownspreadInterpreter(self, hexVal, outFile):
        self.maxDownspread = hexVal & 0x01
        self.noAuxLT = (hexVal & 0x40) >> 5
        outFile.write("\tMAX_DOWNSPREAD = {}\n".format(self.maxDownspread))
        outFile.write("\tNO_AUX_HANDSHAKE_LINK_TRAINING = {}\n".format(self.noAuxLT))

    def norpInterpreter(self, hexVal, outFile):
        self.norp = hexVal & 0x01
        self.powerCap5V = (hexVal & 0x20) >> 5
        self.powerCap12V = (hexVal & 0x40) >> 6
        self.powerCap18V = (hexVal & 0x80) >> 7
        outFile.write("\tNORP = {}\n".format(self.norp))
        outFile.write("\t5V_DP_PWR_CAP = {}\n".format(self.powerCap5V))
        outFile.write("\t12V_DP_PWR_CAP = {}\n".format(self.powerCap12V))
        outFile.write("\t18V_DP_PWR_CAP = {}\n".format(self.powerCap18V))

    def downstreamPortPresentInterpreter(self, hexVal, outFile):
        self.dfpPresent = hexVal & 0x01
        self.dfpType = (hexVal & 0x6) >> 1
        self.dfpTypeList = {
            0x00 : "DisplayPort",
            0x01 : "Analog VGA or analog video over DVI-I",
            0x02 : "DVI, HDMI, or DP++",
            0x03 : "Others"
        }
        self.formatConversion = (hexVal & 0x8) >> 3
        self.detailedCapInfo = (hexVal & 0x10) >> 4
        outFile.write("\tDWN_STRM_PORT_PRESENT = {}\n".format(self.dfpPresent))
        outFile.write("\tDWN_STRM_PORT_TYPE = {}({})\n".format(self.dfpType, self.dfpTypeList[self.dfpType]))
        outFile.write("\tFormat Conversion = {}\n".format(self.powerCap12V))
        outFile.write("\tDETAILED_CAP_INFO_AVAILABLE = {}\n".format(self.powerCap18V))

    def channelCodingInterpreter(self, hexVal, outFile):
        self.channelCoding = hexVal & 0x01
        outFile.write("\tANSI 8B/10B = {}\n".format(self.channelCoding))

    def dwnstrmPortCountInterpreter(self, hexVal, outFile):
        self.dfpCount = (hexVal & 0xF)
        self.msaTmgParIgnored = (hexVal & 0x40) >> 6
        self.ouiSupp = (hexVal & 0x80) >> 7
        outFile.write("\nDWN_STRM_PORT_COUNT = {}\n".format(self.dfpCount))
        outFile.write("\nMSA_TIMING_PAR_IGNORED = {}\n".format(self.msaTmgParIgnored))
        outFile.write("\nOUI Support = {}\n".format(self.ouiSupp))

    def rxPrt0Cap0Interpreter(self, hexVal, outFile):
        self.lclEdidPrsnt = (hexVal & 0x2) >> 1
        self.assPrecPort = (hexVal & 0x4) >> 2
        self.hblankExpCap = (hexVal & 0x8) >> 3
        self.buffSizeUnit = (hexVal & 0x10) >> 4
        self.buffSizePerPort = (hexVal & 0x20) >> 5
        outFile.write("\nLOCAL_EDID_PRESENT = {}\n".format(self.lclEdidPrsnt))
        outFile.write("\nASSOCIATED_TO_PRECEEDING_PORT = {}\n".format(self.assPrecPort))
        outFile.write("\nHBLANK_EXPANSION_CAPABLE = {}\n".format(self.hblankExpCap))
        outFile.write("\nBUFFER_SIZE_UNIT = {} (0 : Pixel, 1: Byte)\n".format(self.buffSizeUnit))
        outFile.write("\nBUFFER_SIZE_PER_PORT = {}(0: Per-lane, 1: per-port)\n".format(self.buffSizePerPort))

    def rxPrt0Cap1Interpreter(self, hexVal, outFile):
        self.buffSize = (hexVal + 1) * 32
        outFile.write("\nBUFFER_SIZE = {}\n".format(self.buffSize))

    def rxPrt1Cap0Interpreter(self, hexVal, outFile):
        self.lclEdidPrsnt = (hexVal & 0x2) >> 1
        self.assPrecPort = (hexVal & 0x4) >> 2
        self.hblankExpCap = (hexVal & 0x8) >> 3
        self.buffSizeUnit = (hexVal & 0x10) >> 4
        self.buffSizePerPort = (hexVal & 0x20) >> 5
        outFile.write("\nLOCAL_EDID_PRESENT = {}\n".format(self.lclEdidPrsnt))
        outFile.write("\nASSOCIATED_TO_PRECEEDING_PORT = {}\n".format(self.assPrecPort))
        outFile.write("\nHBLANK_EXPANSION_CAPABLE = {}\n".format(self.hblankExpCap))
        outFile.write("\nBUFFER_SIZE_UNIT = {} (0 : Pixel, 1: Byte)\n".format(self.buffSizeUnit))
        outFile.write("\nBUFFER_SIZE_PER_PORT = {}(0: Per-lane, 1: per-port)\n".format(self.buffSizePerPort))

    def rxPrt1Cap1Interpreter(self, hexVal, outFile):
        self.buffSize = (hexVal + 1) * 32
        outFile.write("\nBUFFER_SIZE = {}\n".format(self.buffSize))

    def i2cSpeedCntrlInterpreter(self, hexVal, outFile):
        self.i2cSpeedCntrlCapList = {
            0x0  : "No I2C / Speed control",
            0x01 : "1Kbps",
            0x02 : "5Kbps",
            0x04 : "10Kbps",
            0x08 : "100Kbps",
            0x10 : "400Kbps",
            0x20 : "1Mbps",
            0x40 : "RESERVED",
            0x80 : "RESERVED",
        }
        outFile.write("\n I2C Speed = {}\n".format(self.i2cSpeedCntrlCapList[hexVal]))

    def trngAuxRdIntInterpreter(self, hexVal, outFile):
        self.trngRdIntList = {
            0x00 : "100us for the Main-Link Clock Recovery phase; 400us for the Main-Link Channel Equalization phase.",
            0x01 : "100us for the Main-Link Clock Recovery phase; 4ms for the Main-Link Channel Equalization phase.",
            0x02 : "100us for the Main-Link Clock Recovery phase; 8ms for the Main-Link Channel Equalization phase.",
            0x03 : "100us for the Main-Link Clock Recovery phase; 12ms for the Main-Link Channel Equalization phase.",
            0x04 : "100us for the Main-Link Clock Recovery phase; 16ms for the Main-Link Channel Equalization phase.",
        }
        self.extRxCapPrsnt = (hexVal & 0x80) >> 7
        outFile.write("\tTRAINING_AUX_RD_INTERVAL = {}\n".format(self.trngRdIntList[hexVal & 0x7F]))
        outFile.write("\tEXTENDED_RECEIVER_CAPABILITY_FIELD_PRESENT = {}\n".format(self.extRxCapPrsnt))

    def adaptorCap(self, hexVal, outFile):
        self.frcldsensCap = hexVal & 0x00
        self.altI2cPattCap = (hexVal & 0x02) >> 1
        outFile.write("\tFORCE_LOAD_SENSE_CAP = {}\n".format(self.frcldsensCap))
        outFile.write("\tALTERNATE_I2C_PATTERN_CAP = {}\n".format(self.altI2cPattCap))

    def lnkBwSetInterpreter(self, hexVal, outFile):
        self.bwSet = LINK_RATE[hexVal]
        outFile.write("\tLINK_BW_SET = {}\n".format(self.bwSet))

    def lcSetInterpreter(self, hexVal, outFile):
        self.lcSet = hexVal & 0x1F
        self.adjReqGrtd = (hexVal & 0x20) >> 5
        self.enFrame = (hexVal & 0x80) >> 7
        outFile.write("\tLANE_COUNT_SET = {}\n".format(self.lcSet))
        outFile.write("\tPOST_LT_ADJ_REQ_GRANTED = {}\n".format(self.adjReqGrtd))
        outFile.write("\tENHANCED_FRAME_EN = {}\n".format(self.enFrame))

    def tpsInterpreter(self, hexVal, outFile):
        self.tps = (hexVal & 7)
        if self.tps == 7:
            self.tps = 4
        self.scrambling = (hexVal & 0x20) >> 5
        self.errCntSel = (hexVal & 0xC0) >> 6
        outFile.write("\tTRAINING_PATTERN_SELECT = {}\n".format(self.tps))
        outFile.write("\tSCRAMBLING_DISABLE = {}\n".format(self.scrambling))
        outFile.write("\tSYMBOL_ERROR_COUNT_SEL = {}\n".format(self.errCntSel))

    def trLane0SetInterpreter(self, hexVal, outFile):
        self.vsSetL0 = hexVal & 0x03
        self.maxVsL0 = (hexVal & 0x04) >> 2
        self.peSetL0 = (hexVal & 0x18) >> 3
        self.maxPeL0 = (hexVal & 0x20) >> 5
        outFile.write("\tVOLTAGE_SWING_SET = {}\n".format(self.vsSetL0))
        outFile.write("\tMAX_SWING_REACHED = {}\n".format(self.maxVsL0))
        outFile.write("\tPRE-EMPHASIS_SET = {}\n".format(self.peSetL0))
        outFile.write("\tMAX_PRE-EMPHASIS_REACHED = {}\n".format(self.maxPeL0))

    def trLane1SetInterpreter(self, hexVal, outFile):
        self.vsSetL1 = hexVal & 0x03
        self.maxVsL1 = (hexVal & 0x04) >> 2
        self.peSetL1 = (hexVal & 0x18) >> 3
        self.maxPeL1 = (hexVal & 0x20) >> 5
        outFile.write("\tVOLTAGE_SWING_SET = {}\n".format(self.vsSetL1))
        outFile.write("\tMAX_SWING_REACHED = {}\n".format(self.maxVsL1))
        outFile.write("\tPRE-EMPHASIS_SET = {}\n".format(self.peSetL1))
        outFile.write("\tMAX_PRE-EMPHASIS_REACHED = {}\n".format(self.maxPeL1))

    def trLane2SetInterpreter(self, hexVal, outFile):
        self.vsSetL2 = hexVal & 0x03
        self.maxVsL2 = (hexVal & 0x04) >> 2
        self.peSetL2 = (hexVal & 0x18) >> 3
        self.maxPeL2 = (hexVal & 0x20) >> 5
        outFile.write("\tVOLTAGE_SWING_SET = {}\n".format(self.vsSetL2))
        outFile.write("\tMAX_SWING_REACHED = {}\n".format(self.maxVsL2))
        outFile.write("\tPRE-EMPHASIS_SET = {}\n".format(self.peSetL2))
        outFile.write("\tMAX_PRE-EMPHASIS_REACHED = {}\n".format(self.maxPeL2))

    def trLane3SetInterpreter(self, hexVal, outFile):
        self.vsSetL3 = hexVal & 0x03
        self.maxVsL3 = (hexVal & 0x04) >> 2
        self.peSetL3 = (hexVal & 0x18) >> 3
        self.maxPeL3 = (hexVal & 0x20) >> 5
        outFile.write("\tVOLTAGE_SWING_SET = {}\n".format(self.vsSetL3))
        outFile.write("\tMAX_SWING_REACHED = {}\n".format(self.maxVsL3))
        outFile.write("\tPRE-EMPHASIS_SET = {}\n".format(self.peSetL3))
        outFile.write("\tMAX_PRE-EMPHASIS_REACHED = {}\n".format(self.maxPeL3))

    def dummyInterpreter(self, hexVal, outFile):
        # print("\tDUMMY = {}". format(hex(hexVal)))
        outFile.write("\tDUMMY = {}\n".format(hex(hexVal)))

    def sinkCountInterpreter(self, hexVal, outFile):
        self.sinkCount = hexVal & 0x3F
        self.cpReady = (hexVal & 40) >> 6
        # print("\tSINK_COUNT = {}\n\tCP_READY = {}".format(self.sinkCount, self.cpReady))
        outFile.write("\tSINK_COUNT = {}\n\tCP_READY = {}\n".format(self.sinkCount, self.cpReady))

    def devServiceIRQVectorInterpreter(self, hexVal, outFile):
        self.remoteCtrlCmdPending = hexVal & 0x01
        self.autoTestReq = (hexVal & 0x2) >> 1
        self.cpIrq = (hexVal & 0x4) >> 2
        self.mccsIrq = (hexVal & 0x8) >> 3
        self.downRepMsgRdy = (hexVal & 0x10) >> 4
        self.upReqMsgRdy = (hexVal & 0x20) >> 5
        self.sinkSpcfcIrq = (hexVal & 0x40) >> 6
        # print("\tREMOTE_CONTROL_COMMAND_PENDING = {}".format(self.remoteCtrlCmdPending))
        # print("\tAUTOMATED_TEST_REQUEST = {}".format(self.autoTestReq))
        # print("\tCP_IRQ = {}".format(self.cpIrq))
        # print("\tMCCS_IRQ = {}".format(self.mccsIrq))
        # print("\tDOWN_REP_MSG_RDY = {}".format(self.downRepMsgRdy))
        # print("\tUP_REQ_MSG_RDY = {}".format(self.upReqMsgRdy))
        # print("\tSINK_SPECIFIC_IRQ = {}".format(self.sinkSpcfcIrq))
        outFile.write("\tREMOTE_CONTROL_COMMAND_PENDING = {}\n".format(self.remoteCtrlCmdPending))
        outFile.write("\tAUTOMATED_TEST_REQUEST = {}\n".format(self.autoTestReq))
        outFile.write("\tCP_IRQ = {}\n".format(self.cpIrq))
        outFile.write("\tMCCS_IRQ = {}\n".format(self.mccsIrq))
        outFile.write("\tDOWN_REP_MSG_RDY = {}\n".format(self.downRepMsgRdy))
        outFile.write("\tUP_REQ_MSG_RDY = {}\n".format(self.upReqMsgRdy))
        outFile.write("\tSINK_SPECIFIC_IRQ = {}\n".format(self.sinkSpcfcIrq))

    def laneStatusInterpreter(self, hexVal, laneType, outFile):
        self.laneXCRDone = hexVal & 0x01
        self.laneXChannelEQDone = (hexVal & 0x2) >> 1
        self.laneXSymbolLocked = (hexVal & 0x4) >> 2
        self.laneYCRDone = (hexVal & 0x010) >> 4
        self.laneYChannelEQDone = (hexVal & 0x20) >> 5
        self.laneYSymbolLocked = (hexVal & 0x40) >> 6
        if laneType == 0:
            # print("\tLANE0_CR_DONE = {}".format(self.laneXCRDone))
            # print("\tLANE0_CHANNEL_EQ_DONE = {}".format(self.laneXChannelEQDone))
            # print("\tLANE0_SYMBOL_LOCKED = {}".format(self.laneXSymbolLocked))
            # print("\tLANE1_CR_DONE = {}".format(self.laneYCRDone))
            # print("\tLANE1_CHANNEL_EQ_DONE = {}".format(self.laneYChannelEQDone))
            # print("\tLANE1_SYMBOL_LOCKED = {}".format(self.laneYSymbolLocked))
            outFile.write("\tLANE0_CR_DONE = {}\n".format(self.laneXCRDone))
            outFile.write("\tLANE0_CHANNEL_EQ_DONE = {}\n".format(self.laneXChannelEQDone))
            outFile.write("\tLANE0_SYMBOL_LOCKED = {}\n".format(self.laneXSymbolLocked))
            outFile.write("\tLANE1_CR_DONE = {}\n".format(self.laneYCRDone))
            outFile.write("\tLANE1_CHANNEL_EQ_DONE = {}\n".format(self.laneYChannelEQDone))
            outFile.write("\tLANE1_SYMBOL_LOCKED = {}\n".format(self.laneYSymbolLocked))
        else:
            # print("\tLANE2_CR_DONE = {}".format(self.laneXCRDone))
            # print("\tLANE2_CHANNEL_EQ_DONE = {}".format(self.laneXChannelEQDone))
            # print("\tLANE2_SYMBOL_LOCKED = {}".format(self.laneXSymbolLocked))
            # print("\tLANE3_CR_DONE = {}".format(self.laneYCRDone))
            # print("\tLANE3_CHANNEL_EQ_DONE = {}".format(self.laneYChannelEQDone))
            # print("\tLANE3_SYMBOL_LOCKED = {}".format(self.laneYSymbolLocked))
            outFile.write("\tLANE2_CR_DONE = {}\n".format(self.laneXCRDone))
            outFile.write("\tLANE2_CHANNEL_EQ_DONE = {}\n".format(self.laneXChannelEQDone))
            outFile.write("\tLANE2_SYMBOL_LOCKED = {}\n".format(self.laneXSymbolLocked))
            outFile.write("\tLANE3_CR_DONE = {}\n".format(self.laneYCRDone))
            outFile.write("\tLANE3_CHANNEL_EQ_DONE = {}\n".format(self.laneYChannelEQDone))
            outFile.write("\tLANE3_SYMBOL_LOCKED = {}\n".format(self.laneYSymbolLocked))

    def laneAlignStatusInterpreter(self, hexVal, outFile):
        self.interLaneAlignDone = hexVal & 0x01
        self.downstreamPortStatusChanged = (hexVal & 0x40) >> 6
        self.linkStatusUpdated = (hexVal & 0x80) >> 7
        # print("\tINTERLANE_ALIGN_DONE = {}".format(self.interLaneAlignDone))
        # print("\tDOWNSTREAM_PORT_STATUS_CHANGED = {}".format(self.interLaneAlignDone))
        # print("\tLINK_STATUS_UPDATED = {}".format(self.interLaneAlignDone))
        outFile.write("\tINTERLANE_ALIGN_DONE = {}\n".format(self.interLaneAlignDone))
        outFile.write("\tDOWNSTREAM_PORT_STATUS_CHANGED = {}\n".format(self.interLaneAlignDone))
        outFile.write("\tLINK_STATUS_UPDATED = {}\n".format(self.interLaneAlignDone))

    def sinkStatusInterpreter(self, hexVal, outFile):
        self.rcvPort0Status = hexVal & 0x01
        self.rcvPort1Status = (hexVal & 0x02) >> 1
        # print("\tRECEIVE_PORT_0_STATUS = {}".format(self.rcvPort0Status))
        # print("\tRECEIVE_PORT_1_STATUS = {}".format(self.rcvPort1Status))
        outFile.write("\tRECEIVE_PORT_0_STATUS = {}\n".format(self.rcvPort0Status))
        outFile.write("\tRECEIVE_PORT_1_STATUS = {}\n".format(self.rcvPort1Status))

    def adjustRequestInterpreter(self, hexVal, laneType, outFile):
        self.voltageSwingLaneX = hexVal & 0x3
        self.preEmphasisLaneX = (hexVal & 0xC) >> 2
        self.voltageSwingLaneY = (hexVal & 0x30) >> 4
        self.preEmphasisLaneY = (hexVal & 0xC0) >> 6
        if laneType == 0:
            # print("\tVOLTAGE_SWING_LANE0 = level {}".format(self.voltageSwingLaneX))
            # print("\tPRE-EMPHASIS_LANE0 = level {}".format(self.preEmphasisLaneX))
            # print("\tVOLTAGE_SWING_LANE1 = level {}".format(self.voltageSwingLaneY))
            # print("\tPRE-EMPHASIS_LANE1 = level {}".format(self.preEmphasisLaneY))
            outFile.write("\tVOLTAGE_SWING_LANE0 = level {}\n".format(self.voltageSwingLaneX))
            outFile.write("\tPRE-EMPHASIS_LANE0 = level {}\n".format(self.preEmphasisLaneX))
            outFile.write("\tVOLTAGE_SWING_LANE1 = level {}\n".format(self.voltageSwingLaneY))
            outFile.write("\tPRE-EMPHASIS_LANE1 = level {}\n".format(self.preEmphasisLaneY))
        else:
            # print("\tVOLTAGE_SWING_LANE2 = level {}".format(self.voltageSwingLaneX))
            # print("\tPRE-EMPHASIS_LANE2 = level {}".format(self.preEmphasisLaneX))
            # print("\tVOLTAGE_SWING_LANE3 = level {}".format(self.voltageSwingLaneY))
            # print("\tPRE-EMPHASIS_LANE4 = level {}".format(self.preEmphasisLaneY))
            outFile.write("\tVOLTAGE_SWING_LANE2 = level {}\n".format(self.voltageSwingLaneX))
            outFile.write("\tPRE-EMPHASIS_LANE2 = level {}\n".format(self.preEmphasisLaneX))
            outFile.write("\tVOLTAGE_SWING_LANE3 = level {}\n".format(self.voltageSwingLaneY))
            outFile.write("\tPRE-EMPHASIS_LANE4 = level {}\n".format(self.preEmphasisLaneY))

    def auxAckInterpreter(self, address, hexVal, outFile):
        switcher = {
            0x0: self.dpcdRevInterpreter,
            0x1: self.maxLinkRateInterpreter,
            0x2: self.maxLaneCountInterpreter,
            0x3: self.maxDownspreadInterpreter,
            0x4: self.norpInterpreter,
            0x5: self.downstreamPortPresentInterpreter,
            0x6: self.channelCodingInterpreter,
            0x7: self.dwnstrmPortCountInterpreter,
            0x8: self.rxPrt0Cap0Interpreter,
            0x9: self.rxPrt0Cap1Interpreter,
            0xa: self.rxPrt1Cap0Interpreter,
            0xb: self.rxPrt1Cap1Interpreter,
            0xc: self.i2cSpeedCntrlInterpreter,
            0xd: self.dummyInterpreter,
            0xe: self.trngAuxRdIntInterpreter,
            0xf: self.adaptorCap,
            0x100: self.lnkBwSetInterpreter,
            0x101: self.lcSetInterpreter,
            0x102: self.tpsInterpreter,
            0x103: self.trLane0SetInterpreter,
            0x104: self.trLane1SetInterpreter,
            0x105: self.trLane2SetInterpreter,
            0x106: self.trLane3SetInterpreter,
            0x107: self.dummyInterpreter,
            0x200: self.sinkCountInterpreter,
            0x201: self.devServiceIRQVectorInterpreter,
            0x202: self.laneStatusInterpreter,
            0x203: self.laneStatusInterpreter,
            0x204: self.laneAlignStatusInterpreter,
            0x205: self.sinkStatusInterpreter,
            0x206: self.adjustRequestInterpreter,
            0x207: self.adjustRequestInterpreter,
        }
        func = switcher.get(address)
        if (address == 0x202) or (address == 0x206):
            func(hexVal, 0, outFile)
        elif (address == 0x203) or (address == 0x207):
            func(hexVal, 1, outFile)
        else:
            func(hexVal, outFile)

    def edidInterpreter(self, edidList, outFile):
        edid(edidList, outFile, False, None)

class edid:
    def __init__(self, edidlist, outfile, printStatus, console):
        self.PixelClock = ''
        self.HAV = ''
        self.HB = ''
        self.VAV = ''
        self.VB = ''
        self.HFP = ''
        self.HSPW = ''
        self.VFP = ''
        self.VSPW = ''
        self.HAIS = ''
        self.VAIS = ''
        self.HBS = ''
        self.VBS = ''
        self.interlaced = ''
        self.StereoSupp = ''
        self.SepSync = ''
        self.interpretPreferredTiming(edidlist)
        if printStatus and (console is not None):
            self.printPreferredTiming(console)
        elif not printStatus and (outfile is not None):
            self.writePreferredTiming(outfile)

    def interpretPreferredTiming(self, myvalues):
        self.PixelClock = str(((myvalues[0x36]) | (myvalues[0x37] << 8)) * 10000)
        self.HAV = str(myvalues[0x38] | ((myvalues[0x3A] & 0xF0) << 4))
        self.HB = str(myvalues[0x39] | ((myvalues[0x3A] & 0x0F) << 8))
        self.VAV = str(myvalues[0x3B] | ((myvalues[0x3D] & 0xF0) << 4))
        self.VB = str(myvalues[0x3C] | ((myvalues[0x3D] & 0xF) << 8))
        self.HFP = str(myvalues[0x3E] | ((myvalues[0x41] & 0xC0) << 2))
        self.HSPW = str(myvalues[0x3F] | ((myvalues[0x41] & 0x30) << 4))
        self.VFP = str(((myvalues[0x40] & 0xF0) >> 4) | ((myvalues[0x41] & 0xC) << 2))
        self.VSPW = str((myvalues[0x40] & 0xF) | ((myvalues[0x41] & 0x3) << 4))
        self.HAIS = str(myvalues[0x42] | ((myvalues[0x44] & 0xF0) << 4))
        self.VAIS = str(myvalues[0x43] | ((myvalues[0x44] & 0xF) << 8))
        self.HBS = str(myvalues[0x45])
        self.VBS = str(myvalues[0x46])
        if (myvalues[0x47] & 0x80):
            self.interlaced = 'True'
        else:
            self.interlaced = 'False'

        stereo = {
            0x0: 'Normal Display - No Stereo',
            0x20: 'Field sequential stereo, right image when stereo sync signal = 1',
            0x40: 'Field sequential stereo, left image when stereo sync signal = 1',
            0x21: '2-way interleaved stereo, right image on even lines',
            0x41: '2-way interleaved stereo, left image on even lines',
            0x60: '4-way interleaved stereo',
            0x61: 'Side-by-Side interleaved stereo'
        }

        self.StereoSupp = stereo[myvalues[0x47] & 0x61]

    def printPreferredTiming(self, console):
        console.AppendText('Preferred Timing Mode : \n')
        console.AppendText('Pixel Clock : ' + str(self.PixelClock) + 'Hz ' + '\n')
        console.AppendText('Horizontal Addressable Video : ' + self.HAV + '\n')
        console.AppendText('Horizontal Blanking : ' + self.HB + '\n')
        console.AppendText('Vertical Addressable Video : ' + self.VAV + '\n')
        console.AppendText('Vertical Blanking : ' + self.VB + '\n')
        console.AppendText('Horizontal Front Porch : ' + self.HFP + '\n')
        console.AppendText('Horizontal Sync Pulse Width : ' + self.HSPW + '\n')
        console.AppendText('Vertical Front Porch : ' + self.VFP + '\n')
        console.AppendText('Vertical Sync Pulse Width : ' + self.VSPW + '\n')
        console.AppendText('Horizontal Display size : ' + self.HAIS + 'mm' + '\n')
        console.AppendText('Vertical Display size : ' + self.VAIS + 'mm' + '\n')
        console.AppendText('Horizontal Border in Pixels : ' + self.HBS + '\n')
        console.AppendText('Vertical Border in lines : ' + self.VBS + '\n')
        console.AppendText('Interlaced : ' + self.interlaced + '\n')
        console.AppendText('Stereo Mode : ' + self.StereoSupp + '\n')
        console.AppendText('Signal Definitions : ' + self.SepSync + '\n')

    def writePreferredTiming(self, outFile):
        outFile.write('Preferred Timing Mode : \n')
        outFile.write('Pixel Clock : ' + str(self.PixelClock) + 'Hz ' + '\n')
        outFile.write('Horizontal Addressable Video : ' + self.HAV + '\n')
        outFile.write('Horizontal Blanking : ' + self.HB + '\n')
        outFile.write('Vertical Addressable Video : ' + self.VAV + '\n')
        outFile.write('Vertical Blanking : ' + self.VB + '\n')
        outFile.write('Horizontal Front Porch : ' + self.HFP + '\n')
        outFile.write('Horizontal Sync Pulse Width : ' + self.HSPW + '\n')
        outFile.write('Vertical Front Porch : ' + self.VFP + '\n')
        outFile.write('Vertical Sync Pulse Width : ' + self.VSPW + '\n')
        outFile.write('Horizontal Display size : ' + self.HAIS + 'mm' + '\n')
        outFile.write('Vertical Display size : ' + self.VAIS + 'mm' + '\n')
        outFile.write('Horizontal Border in Pixels : ' + self.HBS + '\n')
        outFile.write('Vertical Border in lines : ' + self.VBS + '\n')
        outFile.write('Interlaced : ' + self.interlaced + '\n')
        outFile.write('Stereo Mode : ' + self.StereoSupp + '\n')
        outFile.write('Signal Definitions : ' + self.SepSync + '\n')

class dpcd:
    def __init__(self, dev):
        self.device = dev
        self.localDpcdList = []
        for line in self.device.devWindow.OutputTextBox.Text.ToString().split('\n'):
            self.line = line + "\n"
            if "DPCD Byte " in self.line:
                self.localDpcdList.append(int(self.line.split("= 0x", 1)[1], 16))
        for address in range(0, len(self.localDpcdList)):
            self.device.devWindow.OutputTextBox.AppendText("__________________________________________\n")
            self.device.devWindow.OutputTextBox.AppendText(RX_CAP[address] + "0x{}".format(hex(self.localDpcdList[address])) + "\n")
            self.dpcdInterpreter(address, self.localDpcdList[address], self.device.devWindow.OutputTextBox)


    def dpcdRevInterpreter(self, hexVal, outFile):
        self.minorRevNum = hexVal & 0xF
        self.majorRevNum = (hexVal & 0xF0) >> 4
        # print("\tDPCD V{}.{}".format(self.majorRevNum, self.minorRevNum))
        outFile.AppendText("\tDPCD V{}.{}\n".format(self.majorRevNum, self.minorRevNum))

    def maxLinkRateInterpreter(self, hexVal, outFile):
        # print("\t" + LINK_RATE[hexVal])
        outFile.AppendText("\t" + LINK_RATE[hexVal] + "\n")

    def maxLaneCountInterpreter(self, hexVal, outFile):
        self.maxLaneCount = hexVal & 0x1F
        self.tpsSupp = (hexVal & 0x40) >> 6
        self.enhancedFrameCap = (hexVal & 0x80) >> 7
        # print("\tMAX_LANE_COUNT = {}".format(self.maxLaneCount))
        # print("\tENHANCED_FRAME_CAP = {}".format(self.enhancedFrameCap))
        # print("\tTPS3_SUPPORTED = {}".format(self.tpsSupp))
        outFile.AppendText("\tMAX_LANE_COUNT     = {}\n".format(self.maxLaneCount))
        outFile.AppendText("\tENHANCED_FRAME_CAP = {}\n".format(self.enhancedFrameCap))
        outFile.AppendText("\tTPS3_SUPPORTED     = {}\n".format(self.tpsSupp))


    def maxDownspreadInterpreter(self, hexVal, outFile):
        self.maxDownspread = hexVal & 0x01
        self.noAuxLT = (hexVal & 0x40) >> 5
        outFile.AppendText("\tMAX_DOWNSPREAD = {}\n".format(self.maxDownspread))
        outFile.AppendText("\tNO_AUX_HANDSHAKE_LINK_TRAINING = {}\n".format(self.noAuxLT))

    def norpInterpreter(self, hexVal, outFile):
        self.norp = hexVal & 0x01
        self.powerCap5V = (hexVal & 0x20) >> 5
        self.powerCap12V = (hexVal & 0x40) >> 6
        self.powerCap18V = (hexVal & 0x80) >> 7
        outFile.AppendText("\tNORP           = {}\n".format(self.norp))
        outFile.AppendText("\t5V_DP_PWR_CAP  = {}\n".format(self.powerCap5V))
        outFile.AppendText("\t12V_DP_PWR_CAP = {}\n".format(self.powerCap12V))
        outFile.AppendText("\t18V_DP_PWR_CAP = {}\n".format(self.powerCap18V))

    def downstreamPortPresentInterpreter(self, hexVal, outFile):
        self.dfpPresent = hexVal & 0x01
        self.dfpType = (hexVal & 0x6) >> 1
        self.dfpTypeList = {
            0x00 : "DisplayPort",
            0x01 : "Analog VGA or analog video over DVI-I",
            0x02 : "DVI, HDMI, or DP++",
            0x03 : "Others"
        }
        self.formatConversion = (hexVal & 0x8) >> 3
        self.detailedCapInfo = (hexVal & 0x10) >> 4
        outFile.AppendText("\tDWN_STRM_PORT_PRESENT   = {}\n".format(self.dfpPresent))
        outFile.AppendText("\tDWN_STRM_PORT_TYPE      = {}({})\n".format(self.dfpType, self.dfpTypeList[self.dfpType]))
        outFile.AppendText("\tFormat Conversion       = {}\n".format(self.powerCap12V))
        outFile.AppendText("\tDETAILED_CAP_INFO_AVAIL = {}\n".format(self.powerCap18V))

    def channelCodingInterpreter(self, hexVal, outFile):
        self.channelCoding = hexVal & 0x01
        outFile.AppendText("\tANSI 8B/10B = {}\n".format(self.channelCoding))

    def dwnstrmPortCountInterpreter(self, hexVal, outFile):
        self.dfpCount = (hexVal & 0xF)
        self.msaTmgParIgnored = (hexVal & 0x40) >> 6
        self.ouiSupp = (hexVal & 0x80) >> 7
        outFile.AppendText("\tDWN_STRM_PORT_COUNT    = {}\n".format(self.dfpCount))
        outFile.AppendText("\tMSA_TIMING_PAR_IGNORED = {}\n".format(self.msaTmgParIgnored))
        outFile.AppendText("\tOUI Support            = {}\n".format(self.ouiSupp))

    def rxPrt0Cap0Interpreter(self, hexVal, outFile):
        self.lclEdidPrsnt = (hexVal & 0x2) >> 1
        self.assPrecPort = (hexVal & 0x4) >> 2
        self.hblankExpCap = (hexVal & 0x8) >> 3
        self.buffSizeUnit = (hexVal & 0x10) >> 4
        self.buffSizePerPort = (hexVal & 0x20) >> 5
        outFile.AppendText("\tLOCAL_EDID_PRESENT            = {}\n".format(self.lclEdidPrsnt))
        outFile.AppendText("\tASSOCIATED_TO_PRECEEDING_PORT = {}\n".format(self.assPrecPort))
        outFile.AppendText("\tHBLANK_EXPANSION_CAPABLE      = {}\n".format(self.hblankExpCap))
        outFile.AppendText("\tBUFFER_SIZE_UNIT              = {} (0 : Pixel, 1: Byte)\n".format(self.buffSizeUnit))
        outFile.AppendText("\tBUFFER_SIZE_PER_PORT          = {}(0: Per-lane, 1: per-port)\n".format(self.buffSizePerPort))

    def rxPrt0Cap1Interpreter(self, hexVal, outFile):
        self.buffSize = (hexVal + 1) * 32
        outFile.AppendText("\tBUFFER_SIZE = {}\n".format(self.buffSize))

    def rxPrt1Cap0Interpreter(self, hexVal, outFile):
        self.lclEdidPrsnt = (hexVal & 0x2) >> 1
        self.assPrecPort = (hexVal & 0x4) >> 2
        self.hblankExpCap = (hexVal & 0x8) >> 3
        self.buffSizeUnit = (hexVal & 0x10) >> 4
        self.buffSizePerPort = (hexVal & 0x20) >> 5
        outFile.AppendText("\tLOCAL_EDID_PRESENT            = {}\n".format(self.lclEdidPrsnt))
        outFile.AppendText("\tASSOCIATED_TO_PRECEEDING_PORT = {}\n".format(self.assPrecPort))
        outFile.AppendText("\tHBLANK_EXPANSION_CAPABLE      = {}\n".format(self.hblankExpCap))
        outFile.AppendText("\tBUFFER_SIZE_UNIT              = {} (0 : Pixel, 1: Byte)\n".format(self.buffSizeUnit))
        outFile.AppendText("\tBUFFER_SIZE_PER_PORT          = {}(0: Per-lane, 1: per-port)\n".format(self.buffSizePerPort))

    def rxPrt1Cap1Interpreter(self, hexVal, outFile):
        self.buffSize = (hexVal + 1) * 32
        outFile.AppendText("\tBUFFER_SIZE = {}\n".format(self.buffSize))

    def i2cSpeedCntrlInterpreter(self, hexVal, outFile):
        self.i2cSpeedCntrlCapList = {
            0x0  : "No I2C / Speed control",
            0x01 : "1Kbps",
            0x02 : "5Kbps",
            0x04 : "10Kbps",
            0x08 : "100Kbps",
            0x10 : "400Kbps",
            0x20 : "1Mbps",
            0x40 : "RESERVED",
            0x80 : "RESERVED",
        }
        outFile.AppendText("\t I2C Speed = {}\n".format(self.i2cSpeedCntrlCapList[hexVal]))

    def trngAuxRdIntInterpreter(self, hexVal, outFile):
        self.trngRdIntList = {
            0x00 : "100us for the Main-Link Clock Recovery phase; 400us for the Main-Link Channel Equalization phase.",
            0x01 : "100us for the Main-Link Clock Recovery phase; 4ms for the Main-Link Channel Equalization phase.",
            0x02 : "100us for the Main-Link Clock Recovery phase; 8ms for the Main-Link Channel Equalization phase.",
            0x03 : "100us for the Main-Link Clock Recovery phase; 12ms for the Main-Link Channel Equalization phase.",
            0x04 : "100us for the Main-Link Clock Recovery phase; 16ms for the Main-Link Channel Equalization phase.",
        }
        self.extRxCapPrsnt = (hexVal & 0x80) >> 7
        outFile.AppendText("\tTRAINING_AUX_RD_INTERVAL       = {}\n".format(self.trngRdIntList[hexVal & 0x7F]))
        outFile.AppendText("\tEXT_RECEIVER_CAP_FIELD_PRESENT = {}\n".format(self.extRxCapPrsnt))

    def adaptorCap(self, hexVal, outFile):
        self.frcldsensCap = hexVal & 0x00
        self.altI2cPattCap = (hexVal & 0x02) >> 1
        outFile.AppendText("\tFORCE_LOAD_SENSE_CAP      = {}\n".format(self.frcldsensCap))
        outFile.AppendText("\tALTERNATE_I2C_PATTERN_CAP = {}\n".format(self.altI2cPattCap))

    def dummyInterpreter(self, hexVal, outFile):
        # print("\tDUMMY = {}". format(hex(hexVal)))
        outFile.AppendText("\tDUMMY = {}\n".format(hex(hexVal)))

    def dpcdInterpreter(self, address, hexVal, devTextBox):
        self.textbox = devTextBox
        switcher = {
            0x0: self.dpcdRevInterpreter,
            0x1: self.maxLinkRateInterpreter,
            0x2: self.maxLaneCountInterpreter,
            0x3: self.maxDownspreadInterpreter,
            0x4: self.norpInterpreter,
            0x5: self.downstreamPortPresentInterpreter,
            0x6: self.channelCodingInterpreter,
            0x7: self.dwnstrmPortCountInterpreter,
            0x8: self.rxPrt0Cap0Interpreter,
            0x9: self.rxPrt0Cap1Interpreter,
            0xa: self.rxPrt1Cap0Interpreter,
            0xb: self.rxPrt1Cap1Interpreter,
            0xc: self.i2cSpeedCntrlInterpreter,
            0xd: self.dummyInterpreter,
            0xe: self.trngAuxRdIntInterpreter,
            0xf: self.adaptorCap,

        }
        func = switcher.get(address)
        func(hexVal, self.textbox)
