import os
import sys
import argparse
import libscrc
import binascii

def crc_file_input():
    """
    Print out the usage of this script.
    """
    if len(sys.argv[1:]) == 0:
        print("\n Missing arguments, try [crc_file.py -h]")
    else:
        # the following part makes an objext containing all the required input arguments from user
        parser = argparse.ArgumentParser(usage="crc_file.py -h", description="This script \
                                                  calcs the crc and adds to the file.")
        parser.add_argument("--crc", help="path to target file", nargs=2, required = False)
        args = parser.parse_args()
        if args.crc:
            add_crc_to_file(args.crc[0], args.crc[1])

def add_crc_to_file(file_name, offset):
#    print("OFFSET ", offset)
    f = open(file_name, "r+b")

    f.seek(int(offset),0)

    startAddress = int.from_bytes(f.read(4), byteorder='big')
#    print("Start Address", hex(startAddress))

    sizeValue = int.from_bytes(f.read(4), byteorder='big')
#    print("Size ", hex(sizeValue))

    versionValue = int.from_bytes(f.read(4), byteorder='big')
#    print("version ", versionValue)

    crcValue = int.from_bytes(f.read(4), byteorder='big')
#    print("file CRC ", hex(crcValue))

    reflectTable = [0] * 256
    for i in range(256):
        j = ((i & 0xF0) >> 4) | ((i & 0x0F) << 4)
        j = ((j & 0xCC) >> 2) | ((j & 0x33) << 2)
        j = ((j & 0xAA) >> 1) | ((j & 0x55) << 1)
        reflectTable[i] = j
    crcTable = [0]*256
    crcTable = _GenerateCRCTable()

    crc32 = 0xFFFFFFFF

    f.seek(startAddress - 0xc0A00000, 0)
    crcfile = f.read()
    f.seek(int(offset) + 12,0)

    crc32 = 0xFFFFFFFF
    crc32 = calcCrc(crcfile, crc32, reflectTable, crcTable)
    crc32 ^= 0xFFFFFFFF

    f.write(crc32.to_bytes(4, 'big'))
#    print("Write = ", hex(crc32))
    f.close()


def _GenerateCRCTable():
        """Generate a CRC-32 table.

        ZIP encryption uses the CRC32 one-byte primitive for scrambling some
        internal keys. We noticed that a direct implementation is faster than
        relying on binascii.crc32().
        poly = 0xedb88320
        """
        poly = 0x04C11DB7
        table = [0] * 256
        for i in range(256):
            c = (i << 24)
            crc = 0
            for j in range(8):
                if ((crc ^ c) & 0x80000000):
                    crc = ((crc << 1) ^ poly)
                else:
                    crc = (crc << 1)
                c = (c << 1)
            table[i] = crc
            """
            print(hex(table[i]))
            """
        return table

def calcCrc(crcFile, crc, reflectTable, crcTable):
        """Generate a CRC-32 table.

        ZIP encryption uses the CRC32 one-byte primitive for scrambling some
        internal keys. We noticed that a direct implementation is faster than
        relying on binascii.crc32().
        poly = 0xedb88320
        """
        remainder = crc

        for i in range(len(crcFile)):
            data  = reflectTable[crcFile[i] & 0xFF]^(remainder >> 24)
            remainder = crcTable[data &0xFF]^(remainder << 8)
            remainder = remainder & 0xFFFFFFFF

        return remainder

if __name__ == "__main__":
    crc_file_input()

