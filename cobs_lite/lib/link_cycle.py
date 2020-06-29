import argparse
import serial
import serial.tools.list_ports
import script_lib.icron_file_parser as ifp
import script_lib.loaded_icron_file as lif
import threading as Threading
import time

class Aquantia_reset():
    def __init__(self, serial, delay, count):
        self.ser = serial
        for numberoftimes in range(0,count):
            print("Link Cycle " + str(numberoftimes + 1)  + " of " + str(count) + " cycles")
            icmd_obj = loaded_icron_file.create_icmd("LINKMGR_COMPONENT", "LINKMGR_phyLinkDownErrorDetected", False)
            executeIcmd(icmd_obj, self.ser)
            time.sleep(delay)


def executeIcmd(icmd_obj, ser):
    icmd_thread = Threading.Thread(target = loaded_icron_file.send_icmd, args=(icmd_obj, ser,))
    icmd_thread.start()
    icmd_thread.join()


if __name__ == "__main__":
    comPorts = list(serial.tools.list_ports.comports())

#   print("Available COM Ports:")
#   for port in comPorts:
#       if port.description == ('USB Serial Port '+ '(' + port.device + ')'):
#           print(" " + port.device)
#   userSelectedPort = input("\nEnter the COM port you would like to use (default = COM1):")
#   if userSelectedPort == '':
#       userSelectedPort = 'COM1'
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-d", "--device", help="device to read from", default="COM1")
    parser.add_argument("-b", "--baud", help="baud rate in bps", default=460800, type=int)
    parser.add_argument("-t", "--time_delay", help="time delay for loop", default=1, type=int)
    parser.add_argument("-c", "--count", help="Number of link cycels", default=1, type=int)
    parser.add_argument("-i", "--icron_file", help="icron file to be used")
    args = parser.parse_args()

    ser = serial.Serial(args.device, args.baud, timeout=1)
    ser.flushInput()
    ser.flushOutput()

    iparsed_file = ifp.IcronParsedFile(args.icron_file)
    loaded_icron_file = lif.Loaded_icron_file(iparsed_file, "blackbird", args.device, args.baud)

    Aquantia_reset(ser, args.time_delay, args.count)
