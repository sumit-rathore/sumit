#!/usr/bin/env python
# Log data from serial port

import argparse
import serial
import serial.tools.list_ports
import datetime
import time
import os
import struct
import gzip
import lib.icmds as icmd

"""
    The following function send the SWVersion ICMD
"""
def get_sw_version(port, rate):
    ser = serial.Serial(port, rate, timeout=1)
    ser.flushInput()
    ser.flushOutput()
    ser.write(icmd.swVersion)
    response = get_icmd_resp(ser)
    print(('\nSoftware Version is {}.{}.{}\n').format(response[0], response[1], response[2]))

"""
    The following function gets the icmd response string from the firmware and
    returns the arguments from the response
"""
def get_icmd_resp(ser):
    wait_interval = 0.25
    start_time = time.time()
    icmdResp = []
    if(ser.is_open):
        while time.time() - start_time < wait_interval:
            size = ser.inWaiting()
            if size:
                data = ser.read(size)
                for i in data:
                    icmdResp.append(i)
    num_args = len(icmdResp)
    if num_args > 19 and num_args < 25:
        response = [icmdResp[20]]
    elif num_args > 24 and num_args < 30:
        response = [icmdResp[20], icmdResp[24]]
    elif num_args > 29:
        response = [icmdResp[20], icmdResp[24], icmdResp[29]]
    ser.close()
    return response



if __name__ == "__main__":

    comPorts = list(serial.tools.list_ports.comports())

    print("Available COM Ports:")
    for port in comPorts:
        if port.description == ('USB Serial Port '+ '(' + port.device + ')'):
            print(" " + port.device)

    userSelectedPort = input("\nEnter the COM port you would like to use (default = COM1):")
    if userSelectedPort == '':
        userSelectedPort = 'COM1'

    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-d", "--device", help="device to read from", default=userSelectedPort)
    parser.add_argument("-b", "--baud", help="baud rate in bps", default=460800, type=int)
    args = parser.parse_args()

    byte_list = []
    ts_bytes = ''

    outputFileDir = os.path.join(os.path.dirname(__file__), "logs")
    if not os.path.exists(outputFileDir):
        os.mkdir(outputFileDir)

    dateTimeStamp = datetime.datetime.now().strftime("%Y_%m_%dT%H_%M_%S")
    os.mkdir(os.path.join(outputFileDir, dateTimeStamp))
    outputFilePath_1 = os.path.join(os.path.dirname(__file__), "logs", dateTimeStamp, datetime.datetime.now().strftime("%Y_%m_%dT%H_%M_%S") + ".bin")
    outputFilePath_2 = os.path.join(os.path.dirname(__file__), "logs", dateTimeStamp, datetime.datetime.now().strftime("%Y_%m_%dT%H_%M_%S") + ".gz")

    get_sw_version(args.device, args.baud)
    print("Do you want to continue?")
    print("Enter 'y' to continue")
    userSelectedPort = input(":")
    if userSelectedPort != 'y':
        print("Terminating the log capture\n")
        exit()

    with serial.Serial(args.device, args.baud) as ser, open(outputFilePath_1, mode='wb') as outputBinFile:
        print("\nLogging started. Saving as a bin file. \n Ctrl-C to stop.")
        try:
            ser.write(icmd.swVersion)
            while True:
                time.sleep(.1)
                outputBinFile.write((ser.read(ser.inWaiting())))
                outputBinFile.flush()

        except KeyboardInterrupt:
            ser.close()
            with open(outputFilePath_1, "rb") as inputFile:
                with gzip.open(outputFilePath_2, mode='wb') as outputFile:
                    while True:
                        ser_byte = inputFile.read(1)
                        if ser_byte:
                            byte_list.append(ord(ser_byte))
                            if len(byte_list) == 24:
                                ts_bytes = struct.pack('!Q', int(time.time() * 1000000))
                                raw_bytes = struct.pack('{}B'.format(len(byte_list)), byte_list[0], byte_list[1], byte_list[2], byte_list[3], byte_list[4], byte_list[5], byte_list[6], byte_list[7], byte_list[8], byte_list[9], byte_list[10], byte_list[11], byte_list[12], byte_list[13], byte_list[14], byte_list[15], byte_list[16], byte_list[17], byte_list[18], byte_list[19], byte_list[20], byte_list[21], byte_list[22], byte_list[23])
                                byte_list = []
                                data_bytes = ts_bytes + raw_bytes
                                ts_bytes = ''

                                outputFile.write(data_bytes)
                                outputFile.flush()
                        else:
                            inputFile.close()
                            outputFile.close()
                            break
            print("Logging stopped")



