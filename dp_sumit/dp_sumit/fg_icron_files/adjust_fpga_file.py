import os
import sys
import argparse
import struct
import libscrc

def fpga_file_input():
    """
    Print out the usage of this script.
    """
    if len(sys.argv[1:]) == 0:
        print("\n Missing arguments, try [adjust_fpga_file.py -h]")
    else:
        # the following part makes an objext containing all the required input arguments from user
        parser = argparse.ArgumentParser(usage="adjust_fpga_file.py -h", description="This script \
                                                        adds the length and crc to an fpga file.")
        parser.add_argument("--crc", help="path to fpga file", nargs=1, required = False)
        args = parser.parse_args()
        exists = os.path.isfile(args.crc[0])
        if exists:
            if args.crc:
                add_fpga_length_and_crc(args.crc[0])

def add_fpga_length_and_crc(file_name):
   
    size = os.path.getsize(file_name)
    f = open(file_name, "r+b")
    crcFile = f.read()
    crc64 = libscrc.ecma182(crcFile)
    f.seek(0,0)
    filesize = size.to_bytes(4, 'big')
    f.write(bytes(filesize))
    f.write(bytes(b'\xFF\xFF\xFF\xFF'))
#    print("Size FPGA = ", hex(size))
    f.write(bytes(crc64.to_bytes(8, 'big')))
#    print("CRC_X = ", hex(crc64))
    for i in range(240):
        f.write(b'\xFF')
    f.write(crcFile)
    f.close()


if __name__ == "__main__":
    fpga_file_input()


