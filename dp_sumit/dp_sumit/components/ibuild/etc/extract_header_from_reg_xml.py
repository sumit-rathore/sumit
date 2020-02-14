import xml.etree.ElementTree as ET
import sys

class Module():
    def __init__(self, name, baseAddress):
        self.name = name
        self.baseAddress = baseAddress
        self.registerGroupsAndUngroupedRegisters = []

class RegisterGroup():
    def __init__(self, name, baseAddress):
        self.name = name
        self.baseAddress = baseAddress
        self.registers = []

class Register():
    def __init__(self, name, baseAddress, description, resetValue):
        self.name = name
        self.baseAddress = baseAddress
        self.description = description
        self.resetValue = resetValue
        self.bitfields = []

    def createCDefines(self, moduleName, regGroupName, cppName, value):
        """Create C header entries for registers"""
        headerString = '#define ' + 'REG' + '_' + \
                       moduleName + '_'
        headerString += regGroupName + '_' if regGroupName else ''
        headerString += self.name + '_' + \
                       cppName + ' ' + \
                       hex(value) + '\n'
        return headerString

class Bitfield():
    def __init__(self, name, offset, size, description):
        self.name = name
        self.offset = offset
        self.size = size
        self.description = description

    def createCDefines(self, moduleName, regGroupName, regName, cppName, value):
        """Create C header entries for bitfields"""
        headerString = '#define ' + 'BF' + '_' + \
                       moduleName + '_'
        headerString += regGroupName + '_' if regGroupName else ''
        headerString += regName + '_' + \
                       self.name + '_' + \
                       cppName + ' ' + \
                       hex(value) + '\n'
        return headerString

if __name__ == "__main__":
    inputXmlFilename = sys.argv[1]
    outputHeaderFilename = sys.argv[2]

    namespace = "{http://www.spiritconsortium.org/XMLSchema/SPIRIT/1.5}"
    tree = ET.parse(inputXmlFilename)
    root = tree.getroot()

    memoryMaps = root.find(namespace + 'memoryMaps')
    memoryMap = memoryMaps.find(namespace + 'memoryMap')
    addressBlock = memoryMap.find(namespace + 'addressBlock')

    registerFiles = addressBlock.findall(namespace + 'registerFile')
    name = addressBlock.find(namespace + 'name').text.upper()
    baseAddress = int(addressBlock.find(namespace + 'baseAddress').text, 0)
    module = Module(name, baseAddress)

    def extractRegistersAndBitfields(parentList, registers):
        for register in registers:
            name = register.find(namespace + 'name').text.upper()
            baseAddress = int(register.find(namespace + 'addressOffset').text, 0)
            resetNode = register.find(namespace + 'reset')
            description = "" if register.find(namespace + 'description') is None else register.find(namespace + 'description').text
            resetValue = int(resetNode.find(namespace + 'value').text, 0)
            resetMask = int(resetNode.find(namespace + 'mask').text, 0)
            resetValue = resetValue & resetMask
            reg = Register(name, baseAddress, description, resetValue)
            parentList.append(reg)
            fields = register.findall(namespace + 'field')
            for field in fields:
                name = field.find(namespace + 'name').text.upper()
                description = "" if field.find(namespace + 'description') is None else field.find(namespace + 'description').text
                bitOffset = int(field.find(namespace + 'bitOffset').text, 0)
                bitWidth = int(field.find(namespace + 'bitWidth').text, 0)
                bitfield = Bitfield(name, bitOffset, bitWidth, description)
                reg.bitfields.append(bitfield)

    # Extract grouped registers
    for registerFile in registerFiles:
        name = registerFile.find(namespace + 'name').text.upper()
        baseAddress = int(registerFile.find(namespace + 'addressOffset').text, 0)
        regGroup = RegisterGroup(name, baseAddress)
        module.registerGroupsAndUngroupedRegisters.append(regGroup)
        registers = registerFile.findall(namespace + 'register')
        extractRegistersAndBitfields(regGroup.registers, registers)

    # Extract ungrouped registers
    unGroupedRegisters = addressBlock.findall(namespace + 'register')
    extractRegistersAndBitfields(module.registerGroupsAndUngroupedRegisters, unGroupedRegisters)

    # Sort registers and bitfields by address and offset
    module.registerGroupsAndUngroupedRegisters.sort(key=lambda x: x.baseAddress, reverse=False)
    for regGroup in module.registerGroupsAndUngroupedRegisters:
        regGroup.registers.sort(key=lambda x: x.baseAddress, reverse=False)
        for register in regGroup.registers:
            register.bitfields.sort(key=lambda x: x.offset, reverse=False)

    def createCDefines(module, regGroup, register):
        """Create C defines for the registers and bitfields

        Arguments: module - top-level module
                   regGroup - register group (optional)
                   register - register object  """
        baseAddress = module.baseAddress + regGroup.baseAddress if regGroup is not None else 0 
        headerString = ''
        if register.description:
            headerString += '/* ' + \
                            module.name + '_'
            headerString += regGroup.name + '_' if regGroup is not None else ''
            headerString += register.name + ': ' + \
                            register.description + ' */\n'
        headerString += register.createCDefines(module.name, regGroup.name, 'ADDRESS', register.baseAddress + baseAddress)
        headerString += register.createCDefines(module.name, regGroup.name, 'RESET', register.resetValue)
        headerString += '\n'
        for bitfield in register.bitfields:
            headerString += '/* ' + \
                            module.name + '_'
            headerString += regGroup.name + '_' if regGroup is not None else ''
            headerString += register.name + '_' + \
                            bitfield.name + ': ' + \
                            bitfield.description + ' */\n'
            headerString += bitfield.createCDefines(module.name, regGroup.name, register.name, 'OFFSET', bitfield.offset)
            mask = ((1 << bitfield.size) - 1) << bitfield.offset
            headerString += bitfield.createCDefines(module.name, regGroup.name, register.name, 'MASK', mask)
            headerString += '\n'
        headerString += '\n'
        return headerString

    # Create header file string
    headerString = ""
    for regGroup in module.registerGroupsAndUngroupedRegisters:
        try:
            for register in regGroup.registers:
                headerString += createCDefines(module, regGroup, register)
        except Exception as ex:
            register = regGroup
            headerString += createCDefines(module, None, register)
        headerString += '\n'

    f = open(outputHeaderFilename, 'w')
    f.write(headerString)
