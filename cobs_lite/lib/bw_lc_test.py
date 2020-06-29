import argparse
import serial
import serial.tools.list_ports
import script_lib.icron_file_parser as ifp
import script_lib.loaded_icron_file as lif
import threading as Threading

import time

def bw_lc_testSequence(ser, port, rate, time_delay):
    bw_lc = [[20,4], [20,2], [20,1], [10,4], [10,2], [10,1], [6,4], [6,2], [6,1]]
    for BW_LC in bw_lc:
        setIsolate(ser, rate)
        time.sleep(0.2)
        setBwLc(ser, BW_LC[0], BW_LC[1])
        time.sleep(0.1)
        restartLinkTraining(ser)
        time.sleep(time_delay)

def setIsolate(ser, rate):
    icmd_obj = loaded_icron_file.create_icmd("DP_COMPONENT", "DP_SetIsolateEnable", False, [1])
    executeIcmd(icmd_obj, ser)

def setBwLc(ser, bandwidth, lanecount):
    print(('WROTE BW %d and LC %d') %(bandwidth, lanecount))
    icmd_obj = loaded_icron_file.create_icmd("DP_COMPONENT", "DP_SetBwLc", False, [bandwidth, lanecount])
    executeIcmd(icmd_obj, ser)

# def verifyBwLc(ser):

def restartLinkTraining(ser):
    print('RESTART LT \n')
    icmd_obj = loaded_icron_file.create_icmd("DP_COMPONENT", "DP_RestartDPStateMachine", False)
    executeIcmd(icmd_obj, ser)

def executeIcmd(icmd_obj, ser):
    icmd_thread = Threading.Thread(target = loaded_icron_file.send_icmd, args=(icmd_obj, ser,))
    icmd_thread.start()
    icmd_thread.join()

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
    parser.add_argument("-t", "--time_delay", help="time delay for loop", default=1, type=int)
    parser.add_argument("-i", "--icron_file", help="icron file to be used")
    parser.add_argument("-c", "--count", help="repeat test this number of times", default=1, type=int)
    args = parser.parse_args()

    ser = serial.Serial(args.device, args.baud, timeout=1)
    ser.flushInput()
    ser.flushOutput()

    iparsed_file = ifp.IcronParsedFile(args.icron_file)
    loaded_icron_file = lif.Loaded_icron_file(iparsed_file, "blackbird", args.device, args.baud)

#   icmd_obj = loaded_icron_file.create_icmd("DP_COMPONENT", "DP_PmLogState", False)
#    executeIcmd(icmd_obj, ser)
#   num_args, response = loaded_icron_file.get_icmd_resp(ser)
#   print(num_args)
#   print(response)
#   print(icmd_thread.isAlive())

#   icmd_obj = loaded_icron_file.create_icmd("CORE_COMPONENT", "BBCORE_printHWModuleVersion", False)
#   executeIcmd(icmd_obj, ser)
#   num_args, response = loaded_icron_file.get_icmd_resp(ser)
#   print(num_args)
#   print(response)

    # icmd_obj = loaded_icron_file.create_icmd("DP_COMPONENT", "DP_SetBwLc", False, [0x14, 0x4])
    # executeIcmd(icmd_obj, ser)
    # num_args, response = loaded_icron_file.get_icmd_resp(ser)
    # print("number of args {}". format(num_args))
    # print("response {}". format(response))

    for count in range (0, args.count):
        bw_lc_testSequence(ser, port, args.baud, args.time_delay)

    ser.close()
