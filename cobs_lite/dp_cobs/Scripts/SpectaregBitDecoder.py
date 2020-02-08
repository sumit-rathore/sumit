from System import *

import clr
clr.AddReference("System.Xml")
clr.AddReference("System.Drawing")
clr.AddReference("System.Windows.Forms")

from System.IO import *
from System.Xml import *
from System.Drawing import *
from System.Windows.Forms import *

# For a given register and value, SpectaregBitDecoder.py will print the name and offset of all
# the bitfields that are set based on register's description in the XML file.
# E.g. if the user wants to read ULM register 0x00000114 and the content of the register is 0x1b7
# the script will print the following:
#   ULM: UsrLog: Read ULM Register: 0x00000114, Value: 0x000001b7
#   ULMII - IntEn. The following bitfields are set (bit Offset, bitfield name):
#   (0, BusRstDet), (1, NegDone), (2, BusRstDone), (4, SusDet), (5, HstRsmDet), (7, RsmDone), (8, Conn)
# E.g. for testing bitfield value, run the following command:
# currentDevice.iLogs['CLM_COMPONENT']['CLM_SPECTAREG_READ'].events['CLM'](None, [0x00000218, 0x6C930A9F])
# which will print:
#   Clm - MlpConfig1. The following bitfields are set (bit Offset, bitfield name, bitfield value):
#   (0, TxW4RLimit, 15), (4, TxW4RVpWatch, 1), (7, TxW4RThresh, 21), (15, TxSidThresh, 38), (23, TxQidThresh, 9), (27, ToCntThresh, 13)

def decodeBitFields(register, value, driverName, device):
    spectaregXML = XmlDocument()
    #TODO: change this
    XmlFileString = device.icron_untar(driverName + "_comp.xml")
    spectaregXML.LoadXml(XmlFileString)

    isIDesign = False

    if device.icronFile.get("project")[:9] == "blackbird" and \
        int(device.icronFile.get("version")) > 2:
        isIDesign = True

    #create a namespace manager for parsing xml file
    nsmgr = XmlNamespaceManager(spectaregXML.NameTable)
    if isIDesign:
        version = "1.5"
    else:
        version = "1.4"

    nsmgr.AddNamespace("spirit", "http://www.spiritconsortium.org/XMLSchema/SPIRIT/" + version)
    regRoot = spectaregXML.DocumentElement

    # Hierarchy of registers is in the following format:
    # Component:
    #   Address block # 1:
    #       Register # 1:
    #           Bit field # 1:
    #           Bit field # 2:
    #       Register # 2:
    #           Bit field # 1:
    #           Bit field # 2:
    #   Address block # 2:
    #       Register # 1:
    #           Bit field # 1:
    #           Bit field # 2:
    #       Register # 2:
    #           Bit field # 1:
    #           Bit field # 2:
    if isIDesign:
        addressBlockList = regRoot.SelectNodes("//spirit:registerFile",nsmgr)
        componentOffset = int(device.icronFile.get("idesign_" + driverName + "_comp_offset"), 16)
    else:
        addressBlockList = regRoot.SelectNodes("//spirit:addressBlock",nsmgr)
        componentOffset = int(device.icronFile.get("spectareg_" + driverName + "_comp_offset"), 16)

    # davidM changed component base address to be relative to the absolute base address instead
    # of including the absolute base address; mask only the lower two bytes
    componentOffset = (componentOffset & 0xFFFF)

    # Make all addresses relative to the component
    addressBlockOffset = register - componentOffset

    for addressBlock in addressBlockList:
        addressBlockName = addressBlock.SelectSingleNode("spirit:name", nsmgr).InnerText
        addressBlockBaseAddress = int(addressBlock.SelectSingleNode("spirit:baseAddress", nsmgr).InnerText, 16)
        addressBlockWidth = int(addressBlock.SelectSingleNode("spirit:range", nsmgr).InnerText, 16)

        # address block of interest is within the current address block
        if (addressBlockOffset > addressBlockBaseAddress) \
            and (addressBlockOffset < (addressBlockBaseAddress + addressBlockWidth)):

            # Retrieve a list of registers
            registerDefs = addressBlock.SelectNodes("spirit:register", nsmgr)

            addressOffset = addressBlockOffset - addressBlockBaseAddress

            # Print the names of all Bitfields that are set
            for register in registerDefs:
                registerName = register.SelectSingleNode("spirit:name", nsmgr).InnerText
                registerAddressOffset = int(register.SelectSingleNode("spirit:addressOffset", nsmgr).InnerText, 16)
                if (registerAddressOffset == addressOffset):
                    fieldDefs = register.SelectNodes("spirit:field", nsmgr)
                    headerString = DateTime.Now.ToString("MM/dd/yy HH:mm:ss.ff ") \
                                   + addressBlockName + " - " + registerName + "."
                    bitFieldString = DateTime.Now.ToString("MM/dd/yy HH:mm:ss.ff ")
                    isOneBitSet = False
                    for node in fieldDefs:
                        name = node.SelectSingleNode("spirit:name", nsmgr).InnerText
                        bitOffset = int(node.SelectSingleNode("spirit:bitOffset", nsmgr).InnerText)
                        bitWidth = int(node.SelectSingleNode("spirit:bitWidth", nsmgr).InnerText)
                        # print all bitfields that are set
                        bitMask = (1 << bitWidth) -1

                        bitFieldValue = (value >> bitOffset) & bitMask
                        if (bitFieldValue != 0):
                            bitFieldString += "(" + \
                                              str(bitOffset) + \
                                              ", " + \
                                              name + \
                                              ", " + \
                                              str(bitFieldValue) + \
                                              ")" + \
                                              ", "
                            isOneBitSet = True
                    if isOneBitSet:
                        # Print the address block and the register names
                        headerString += " The following bitfields are set (bit Offset, bitfield name, bitfield value):\n"
                        device.logCache.append(headerString)
                        # Remove the last ", " from the string
                        device.logCache.append(bitFieldString[:-2] + "\n")
                    else:
                        headerString += " None of the bitfields are set.\n" 
                        device.logCache.append(headerString)
                    break
            break

def createCallback(hwName, device):

    def callback(ilog, args):
        """Print the names of all bitFields that are set

           Arguments:
           ilog, args [0, 1]:  {register, value}"""
        register = args[0]
        value = args[1]
        decodeBitFields(register, value, hwName, device)

    return callback

# Script is run at startup in Device.py after loading .icron file and also after reloading .icron file
if ((int(currentDevice.icronFile.get("version")) >= 6 \
        and currentDevice.icronFile.get("project")[:8] == "lg1_vhub") or
    (int(currentDevice.icronFile.get("version")) >= 3 \
        and currentDevice.icronFile.get("project")[:10] == "goldenears") or
    (int(currentDevice.icronFile.get("version")) >= 2 \
        and currentDevice.icronFile.get("project")[:15] == "goldenears_test") or
    (int(currentDevice.icronFile.get("version")) >= 1 \
        and currentDevice.icronFile.get("project")[:9] == "blackbird") or
    (int(currentDevice.icronFile.get("version")) >= 1 \
        and currentDevice.icronFile.get("project")[:14] == "blackbird_test")):

    if (currentDevice.icronFile.get("project")[:8] == "lg1_vhub"):
        driverList = [('GRG', 'grg'), ('ULM', 'ulm'), ('CLM', 'clm'), ('XCSR', 'xcsr')]
    elif (currentDevice.icronFile.get("project")[:10] == "goldenears"):
        driverList = [('GRG', 'grg'), ('ULM', 'ulmii'), ('CLM', 'clm'), ('XLR', 'xlr'), \
                     ('XRR', 'xrr'), ('XCSR', 'xcsr')]
    else: # blackbird
        driverList = [('BGRG', 'bgrg')]

    for (swName, hwName) in driverList:
        try:
            iLog = currentDevice.iLogs[swName + '_COMPONENT'][swName + '_SPECTAREG_READ']
            # provide dictionary key and function
            iLog.addEventHandler(swName, createCallback(hwName, currentDevice))
        except KeyError:
            currentDevice.logCache.append(DateTime.Now.ToString("MM/dd/yy HH:mm:ss.ff ") + \
                swName + '_SPECTAREG_READ' + " ilog message does not exist\n")
else:
    currentDevice.logCache.append(DateTime.Now.ToString("MM/dd/yy HH:mm:ss.ff ") + \
        "Icron file version does not support Spectareg Decoding\n")
