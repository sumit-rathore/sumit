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
        setBwLc(ser, BW_LC[0], BW_LC[1])
        time.sleep(0.1)
        restartLinkTraining(ser)
        time.sleep(time_delay)

def setBwLc(ser, bandwidth, lanecount):
    print(('WROTE BW %d and LC %d') %(bandwidth, lanecount))
    packet = [1, 0, 129, 255, 11, 154, 7, 27, 0, 0, 0, bandwidth, 0, 0, 0, lanecount, 4]
    ser.write(packet)

# def verifyBwLc(ser):


def restartLinkTraining(ser):
    print('RESTART LT \n')
    ser.write([1, 0, 129, 255, 3, 152, 7, 37, 4])



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
    args = parser.parse_args()

    ser = serial.Serial(args.device, args.baud, timeout=1)
    ser.flushInput()
    ser.flushOutput()

    iparsed_file = ifp.IcronParsedFile(args.icron_file)
    loaded_icron_file = lif.Loaded_icron_file(iparsed_file, "blackbird")

    icmd_obj = loaded_icron_file.create_icmd("DP_COMPONENT", "DP_PmLogState", False)
    icmd_thread = Threading.Thread(target = loaded_icron_file.send_icmd, args=(icmd_obj, ser, args.device, args.baud,))
    icmd_thread.start()
    icmd_thread.join()
    num_args, response = loaded_icron_file.get_icmd_resp(ser)
    print(num_args)
    print(response)
    print(icmd_thread.isAlive())

    icmd_obj = loaded_icron_file.create_icmd("CORE_COMPONENT", "BBCORE_printHWModuleVersion", False)
    icmd_thread = Threading.Thread(target = loaded_icron_file.send_icmd, args=(icmd_obj, ser, args.device, args.baud,))
    icmd_thread.start()
    icmd_thread.join()
    num_args, response = loaded_icron_file.get_icmd_resp(ser)
    print(num_args)
    print(response)
    print(icmd_thread.isAlive())


#    icmd_obj = loaded_icron_file.create_icmd("TOPLEVEL_COMPONENT", "PrintSwVersion", False)
#    icmd_thread = Threading.Thread(target = loaded_icron_file.send_icmd_wait_for_response, args=(icmd_obj, ser, args.device, args.baud,))
#    response = loaded_icron_file.send_icmd_wait_for_response(icmd_obj, ser, args.device, args.baud)
    #icmd_thread.Name = self.Text.replace("Cobs", "PrintSwVersion")
#    icmd_thread.start()
#    icmd_thread.join()
#    print("response =")
#    print(response)

    #bw_lc_testSequence(ser, args.device, args.baud, args.time_delay)

    ser.close()
