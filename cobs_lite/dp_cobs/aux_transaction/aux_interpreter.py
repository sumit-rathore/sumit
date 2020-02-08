import os

LINK_RATE = {
    0x06: "MAX_LINK_RATE = 1.62Gbps",
    0x0a: "MAX_LINK_RATE = 2.7Gbps",
    0x14: "MAX_LINK_RATE = 5.4Gbps",
    0x1e: "MAX_LINK_RATE = 8.1Gbps"
}

RX_CAP = {
    0x0: "DPCD_REV 0x00000 := ",
    0x1: "MAX_LINK_RATE 0x00001 := ",
    0x2: "MAX_LANE_COUNT 0x00002 := ",
    0x3: "MAX_DOWNSPREAD 0x00003 := ",
    0x4: "NORP 0x00004 := ",
    0x5: "DOWNSTREAM_PORT_PRESENT 0x00005 := ",
    0x6: "MAIN_LINK_CHANNEL_CODING 0x00006 := ",
    0x7: "DOWN_STREAM_PORT_COUNT 0x00007 := ",
    0x8: "RECEIVER_PORT0_CAP_0 0x00008 := ",
    0x9: "RECEIVER_PORT0_CAP_1 0x00009 := ",
    0xa: "RECEIVER_PORT1_CAP_0 [ 0x0000a := ",
    0xb: "RECEIVER_PORT1_CAP_1 0x0000b := ",
    0xc: "I2C_SPEED_CONTROL_CAPS 0x0000c := ",
    0xd: "eDP_CONFIGURATION_CAP 0x0000d := ",
    0xe: "TRAINING_AUX_RD_INTERVAL 0x0000e := ",
    0xf: "ADAPTER_CAP 0x0000f := ",
}

LINK_CONFIG = {
    0x100: "LINK_BW_SET 0x00100 := ",
    0x101: "LANE_COUNT_SET 0x00101 := ",
    0x102: "TRAINING_PATTERN_SET 0x00102 := ",
    0x103: "TRAINING_LANE0_SET 0x00103 := ",
    0x104: "TRAINING_LANE1_SET 0x00104 := ",
    0x105: "TRAINING_LANE2_SET 0x00105 := ",
    0x106: "TRAINING_LANE3_SET 0x00106 := ",
    0x107: "DOWNSPREAD_CTRL 0x00107 := ",
}

SINK_STATUS = {
    0x200: "SINK_COUNT 0x00200 := ",
    0x201: "DEVICE_SERVICE_IRQ_VECTOR 0x00201 :=",
    0x202: "LANE0_1_STATUS 0x00202 :=",
    0x203: "LANE2_3_STATUS 0x00203 :=",
    0x204: "LANE_ALIGN_STATUS_UPDATED 0x00204 :=",
    0x205: "SINK_STATUS 0x00205 :=",
    0x206: "ADJUST_REQUEST_LANE0_1 0x00206 :=",
    0x207: "ADJUST_REQUEST_LANE2_3 0x00207 :="
}

finalFile = open("finalAuxFile.txt", "w")

class aux_interpreter():
    def __init__(self):
        self.auxPath = 'aux_transaction.txt'
        self.lineSplit = ""
        self.auxAdd = 0
        self.auxType = ""
        self.auxReqType = ""
        self.auxHexDump = ""
        self.enhancedFrame = 0
        self.tpsSupp = 0
        self.numBytes = 0
        self.currentAdd = 0

        with open(self.auxPath, "r") as auxHexDump:
            print("aux_transaction.txt opened")
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
                    if "Req" in self.line:
                        self.auxAdd = int(self.line.split("0x", 1)[1].split("\t", 1)[0], 16)
                        if self.auxAdd in range(0x100, 0x108):
                            for address in range(0, self.numBytes):
                                # print(LINK_CONFIG[self.auxAdd + address] + "{}".format(self.auxHexDump[4 + address]))
                                finalFile.write(LINK_CONFIG[self.auxAdd + address] + "0x{}".format(self.auxHexDump[4 + address]) + "\n")

                if "Sink" in self.line:
                    self.lineSplit = self.line.split("\t")
                    self.auxHexDump = self.lineSplit[len(self.lineSplit) - 1]
                    self.auxHexDump = self.auxHexDump.split(" ")
                    self.numBytes = int(self.line.split("AUX_ACK - ", 1)[1].split(" bytes", 1)[0])
                    if (self.auxAdd in range(0x0, 0x10)) and (self.numBytes > 1):
                        for address in range(0, self.numBytes - 1):
                            # print(RX_CAP[self.auxAdd + address] + "{}".format((self.auxHexDump[1 + address])))
                            finalFile.write(RX_CAP[self.auxAdd + address] + "0x{}".format((self.auxHexDump[1 + address])) + "\n")
                            self.auxAckInterpreter(self.auxAdd + address,  int(self.auxHexDump[1 + address], 16))

                    if (self.auxAdd in range(0x200, 0x208)) and (self.numBytes > 1):
                        for address in range(0, self.numBytes - 1):
                            # print(SINK_STATUS[self.auxAdd + address] + "{}".format(self.auxHexDump[1 + address]))
                            finalFile.write(SINK_STATUS[self.auxAdd + address] + "0x{}".format(self.auxHexDump[1 + address]) + "\n")
                            self.auxAckInterpreter(self.auxAdd + address, int(self.auxHexDump[1 + address], 16))

    def dpcdRevInterpreter(self, hexVal):
        self.minorRevNum = hexVal & 0xF
        self.majorRevNum = (hexVal & 0xF0) >> 4
        # print("\tDPCD V{}.{}".format(self.majorRevNum, self.minorRevNum))
        finalFile.write("\tDPCD V{}.{}\n".format(self.majorRevNum, self.minorRevNum))

    def maxLinkRateInterpreter(self, hexVal):
        # print("\t" + LINK_RATE[hexVal])
        finalFile.write("\t" + LINK_RATE[hexVal] + "\n")

    def maxLaneCountInterpreter(self, hexVal):
        self.maxLaneCount = hexVal & 0x1F
        self.tpsSupp = (hexVal & 0x40) >> 6
        self.enhancedFrameCap = (hexVal & 0x80) >> 7
        # print("\tMAX_LANE_COUNT = {}".format(self.maxLaneCount))
        # print("\tENHANCED_FRAME_CAP = {}".format(self.enhancedFrameCap))
        # print("\tTPS3_SUPPORTED = {}".format(self.tpsSupp))
        finalFile.write("\tMAX_LANE_COUNT = {}\n".format(self.maxLaneCount))
        finalFile.write("\tENHANCED_FRAME_CAP = {}\n".format(self.enhancedFrameCap))
        finalFile.write("\tTPS3_SUPPORTED = {}\n".format(self.tpsSupp))


    def maxDownspreadInterpreter(self, hexVal):
        self.maxDownspread = hexVal & 0x01
        self.noAuxLT = (hexVal & 0x40) >> 5
        # print("\tMAX_DOWNPREAD = {}".format(self.maxDownspread))
        # print("\tNO_AUX_HANDSHAKE_LINK_TRAINING = {}".format(self.noAuxLT))
        finalFile.write("\tMAX_DOWNPREAD = {}\n".format(self.maxDownspread))
        finalFile.write("\tNO_AUX_HANDSHAKE_LINK_TRAINING = {}\n".format(self.noAuxLT))

    def norpInterpreter(self, hexVal):
        # print("\tNORP = {}".format(hexVal & 0x01))
        finalFile.write("\tNORP = {}\n".format(hexVal & 0x01))

    def dummyInterpreter(self, hexVal):
        # print("\tDUMMY = {}". format(hex(hexVal)))
        finalFile.write("\tDUMMY = {}\n".format(hex(hexVal)))

    def sinkCountInterpreter(self, hexVal):
        self.sinkCount = hexVal & 0x3F
        self.cpReady = (hexVal & 40) >> 6
        # print("\tSINK_COUNT = {}\n\tCP_READY = {}".format(self.sinkCount, self.cpReady))
        finalFile.write("\tSINK_COUNT = {}\n\tCP_READY = {}\n".format(self.sinkCount, self.cpReady))

    def devServiceIRQVectorInterpreter(self, hexVal):
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
        finalFile.write("\tREMOTE_CONTROL_COMMAND_PENDING = {}\n".format(self.remoteCtrlCmdPending))
        finalFile.write("\tAUTOMATED_TEST_REQUEST = {}\n".format(self.autoTestReq))
        finalFile.write("\tCP_IRQ = {}\n".format(self.cpIrq))
        finalFile.write("\tMCCS_IRQ = {}\n".format(self.mccsIrq))
        finalFile.write("\tDOWN_REP_MSG_RDY = {}\n".format(self.downRepMsgRdy))
        finalFile.write("\tUP_REQ_MSG_RDY = {}\n".format(self.upReqMsgRdy))
        finalFile.write("\tSINK_SPECIFIC_IRQ = {}\n".format(self.sinkSpcfcIrq))

    def laneStatusInterpreter(self, hexVal, laneType):
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
            finalFile.write("\tLANE0_CR_DONE = {}\n".format(self.laneXCRDone))
            finalFile.write("\tLANE0_CHANNEL_EQ_DONE = {}\n".format(self.laneXChannelEQDone))
            finalFile.write("\tLANE0_SYMBOL_LOCKED = {}\n".format(self.laneXSymbolLocked))
            finalFile.write("\tLANE1_CR_DONE = {}\n".format(self.laneYCRDone))
            finalFile.write("\tLANE1_CHANNEL_EQ_DONE = {}\n".format(self.laneYChannelEQDone))
            finalFile.write("\tLANE1_SYMBOL_LOCKED = {}\n".format(self.laneYSymbolLocked))
        else:
            # print("\tLANE2_CR_DONE = {}".format(self.laneXCRDone))
            # print("\tLANE2_CHANNEL_EQ_DONE = {}".format(self.laneXChannelEQDone))
            # print("\tLANE2_SYMBOL_LOCKED = {}".format(self.laneXSymbolLocked))
            # print("\tLANE3_CR_DONE = {}".format(self.laneYCRDone))
            # print("\tLANE3_CHANNEL_EQ_DONE = {}".format(self.laneYChannelEQDone))
            # print("\tLANE3_SYMBOL_LOCKED = {}".format(self.laneYSymbolLocked))
            finalFile.write("\tLANE2_CR_DONE = {}\n".format(self.laneXCRDone))
            finalFile.write("\tLANE2_CHANNEL_EQ_DONE = {}\n".format(self.laneXChannelEQDone))
            finalFile.write("\tLANE2_SYMBOL_LOCKED = {}\n".format(self.laneXSymbolLocked))
            finalFile.write("\tLANE3_CR_DONE = {}\n".format(self.laneYCRDone))
            finalFile.write("\tLANE3_CHANNEL_EQ_DONE = {}\n".format(self.laneYChannelEQDone))
            finalFile.write("\tLANE3_SYMBOL_LOCKED = {}\n".format(self.laneYSymbolLocked))

    def laneAlignStatusInterpreter(self, hexVal):
        self.interLaneAlignDone = hexVal & 0x01
        self.downstreamPortStatusChanged = (hexVal & 0x40) >> 6
        self.linkStatusUpdated = (hexVal & 0x80) >> 7
        # print("\tINTERLANE_ALIGN_DONE = {}".format(self.interLaneAlignDone))
        # print("\tDOWNSTREAM_PORT_STATUS_CHANGED = {}".format(self.interLaneAlignDone))
        # print("\tLINK_STATUS_UPDATED = {}".format(self.interLaneAlignDone))
        finalFile.write("\tINTERLANE_ALIGN_DONE = {}\n".format(self.interLaneAlignDone))
        finalFile.write("\tDOWNSTREAM_PORT_STATUS_CHANGED = {}\n".format(self.interLaneAlignDone))
        finalFile.write("\tLINK_STATUS_UPDATED = {}\n".format(self.interLaneAlignDone))

    def sinkStatusInterpreter(self, hexVal):
        self.rcvPort0Status = hexVal & 0x01
        self.rcvPort1Status = (hexVal & 0x02) >> 1
        # print("\tRECEIVE_PORT_0_STATUS = {}".format(self.rcvPort0Status))
        # print("\tRECEIVE_PORT_1_STATUS = {}".format(self.rcvPort1Status))
        finalFile.write("\tRECEIVE_PORT_0_STATUS = {}\n".format(self.rcvPort0Status))
        finalFile.write("\tRECEIVE_PORT_1_STATUS = {}\n".format(self.rcvPort1Status))

    def adjustRequestInterpreter(self, hexVal, laneType):
        self.voltageSwingLaneX = hexVal & 0x3
        self.preEmphasisLaneX = (hexVal & 0xC) >> 2
        self.voltageSwingLaneY = (hexVal & 0x30) >> 4
        self.preEmphasisLaneY = (hexVal & 0xC0) >> 6
        if laneType == 0:
            # print("\tVOLTAGE_SWING_LANE0 = level {}".format(self.voltageSwingLaneX))
            # print("\tPRE-EMPHASIS_LANE0 = level {}".format(self.preEmphasisLaneX))
            # print("\tVOLTAGE_SWING_LANE1 = level {}".format(self.voltageSwingLaneY))
            # print("\tPRE-EMPHASIS_LANE1 = level {}".format(self.preEmphasisLaneY))
            finalFile.write("\tVOLTAGE_SWING_LANE0 = level {}\n".format(self.voltageSwingLaneX))
            finalFile.write("\tPRE-EMPHASIS_LANE0 = level {}\n".format(self.preEmphasisLaneX))
            finalFile.write("\tVOLTAGE_SWING_LANE1 = level {}\n".format(self.voltageSwingLaneY))
            finalFile.write("\tPRE-EMPHASIS_LANE1 = level {}\n".format(self.preEmphasisLaneY))
        else:
            # print("\tVOLTAGE_SWING_LANE2 = level {}".format(self.voltageSwingLaneX))
            # print("\tPRE-EMPHASIS_LANE2 = level {}".format(self.preEmphasisLaneX))
            # print("\tVOLTAGE_SWING_LANE3 = level {}".format(self.voltageSwingLaneY))
            # print("\tPRE-EMPHASIS_LANE4 = level {}".format(self.preEmphasisLaneY))
            finalFile.write("\tVOLTAGE_SWING_LANE2 = level {}\n".format(self.voltageSwingLaneX))
            finalFile.write("\tPRE-EMPHASIS_LANE2 = level {}\n".format(self.preEmphasisLaneX))
            finalFile.write("\tVOLTAGE_SWING_LANE3 = level {}\n".format(self.voltageSwingLaneY))
            finalFile.write("\tPRE-EMPHASIS_LANE4 = level {}\n".format(self.preEmphasisLaneY))

    def auxAckInterpreter(self, address, hexVal):
        switcher = {
            0x0: self.dpcdRevInterpreter,
            0x1: self.maxLinkRateInterpreter,
            0x2: self.maxLaneCountInterpreter,
            0x3: self.maxDownspreadInterpreter,
            0x4: self.norpInterpreter,
            0x5: self.dummyInterpreter,
            0x6: self.dummyInterpreter,
            0x7: self.dummyInterpreter,
            0x8: self.dummyInterpreter,
            0x9: self.dummyInterpreter,
            0xa: self.dummyInterpreter,
            0xb: self.dummyInterpreter,
            0xc: self.dummyInterpreter,
            0xd: self.dummyInterpreter,
            0xe: self.dummyInterpreter,
            0xf: self.dummyInterpreter,
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
            func(hexVal, 0)
        elif (address == 0x203) or (address == 0x207):
            func(hexVal, 1)
        else:
            func(hexVal)

        # os.startfile(path)

aux_interpreter()