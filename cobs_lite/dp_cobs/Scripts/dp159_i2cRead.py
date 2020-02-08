import time

#This is a script to read all dp159 retimer addresses.
#dp159_addresses is a list of all dp159 retimer addresses, each address needs to be padded by 6 zeroes at end
#to make it compatible with icmd format

dp159_addresses = [
    "0x00000000",
    "0x01000000",
    "0x02000000",
    "0x04000000",
    "0x05000000",
    "0x08000000",
    "0x09000000",
    "0x0B000000",
    "0x0D000000",
    "0x0E000000",
    "0xA1000000",
    "0xA4000000",
    "0x10000000",
    "0x11000000",
    "0x12000000",
    "0x13000000",
    "0x14000000",
    "0x30000000",
    "0x31000000",
    "0x32000000",
    "0x33000000",
    "0x34000000",
    "0x4C000000",
    "0x4D000000",
    "0x4E000000",
    "0x4F000000",
    "0xFF000000"
]

#ivoid icmdI2cWriteRead(uint8_t deviceAddress, uint16_t speedPort, uint8_t writeByteCount,
#    uint32_t writeData, uint8_t readByteCount) is an icmd under i2c component, to writeread
#addresses at any i2c device
#Arguements:
    #   deviceAddress       - I2C address of the device to operate on
    #   speedPort           - MSW is the I2C speed corresponding to enum I2cSpeed.  LSW is the
                            # switch port numbered 0 through 7.
    #   writeByteCount      - The number of bytes to write
    #   writeData           - The data to write.  Will be casted into a 4 byte array.
    #   readByteCount       - The number of bytes to read

com_port = currentDevice
I2CD_DP159_DEVICE_ADDRESS = 0x5E                             #address of dp159 retimer
I2CD_DP159_DEVICE_SPEED_PORT = 0x0110                        #speedport of dp_159
writeByteCount = 1                                           #number of bytes to write     
readByteCount = 1                                            #number of bytes to read


for address in dp159_addresses:
	icmd_obj =  currentDevice.create_icmd("I2C_COMPONENT", "icmdI2cWriteRead", False, \
                                        [I2CD_DP159_DEVICE_ADDRESS, I2CD_DP159_DEVICE_SPEED_PORT, writeByteCount, address, readByteCount])
	com_port.send_icmd(icmd_obj)
	time.sleep(8)