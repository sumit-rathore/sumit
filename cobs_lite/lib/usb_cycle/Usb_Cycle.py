import usb.core
import win32file
import shutil
import json
import os
import sys
import filecmp
import time
import serial

max_enum_time = 40

# Queries pyUSB to find the specified USB device by using the device's PID and VID
def get_usb_device(vid, pid):
    dev = usb.core.find(idVendor=int(vid, 16), idProduct=int(pid, 16))
    return dev

# Translates the "Bus Speed number" to a more readable form
def get_readable_speed(busSpeed):
    readableSpeed = ""
    if busSpeed == 1:
        readableSpeed = "Low Speed"
    elif busSpeed == 2:
        readableSpeed = "Full Speed"
    elif busSpeed == 3:
        readableSpeed = "High Speed"
    elif busSpeed == 4:
        readableSpeed = "Super Speed"
    else:
        readableSpeed = None

    return readableSpeed

# Gets the time it takes for Windows to see the drive as usbale by using win32file
def windowsEnumTime(driveLetter):
    enumTime = 0.0

    position = ord(driveLetter) - ord('A')                      # gets the offset or position value of the driveletter that the system is looking for
    mask = 1 << position                                        # creats a mask with a 1 in the position of the offset 
    driveBits = win32file.GetLogicalDrives()                    # returns a 26 bit bitmask of all logical drives installed on the system

    startTime = time.perf_counter()                             # start a system timer
    while not (driveBits & mask):                               # loop while the drive is still not visable and keep querying Windows for that drive
        driveBits = win32file.GetLogicalDrives()
        if time.perf_counter() - startTime > max_enum_time:     # if the loop has been going on for greater than the max enum time given, return the max time
            return max_enum_time

    endTime = time.perf_counter()                               # set an end time after the drive was found
    enumTime = round(endTime - startTime, 3)                    # gets enumeration time and rounds it to 3 decimal places
    return enumTime

# Opens and returns the json file of known USB devices
def get_device_data():
    try:
        with open('Usb_Devices.json') as usb_devices_json:
            usb_devices = json.load(usb_devices_json)
    except:
        sys.exit("ERROR: Could not open/find USB device json file, closing program.")
    usb_devices_json.close()
    return usb_devices

# Communicates with the microcontroller and cycles the specified USB devices. After the cycle this
# function also calls usb_test which tests the USB device
# numberOfCycles = number of USB enumeration cycles to do
# comPort = the COM port of the microcontroller
# cycleType = which USB port is being tested (usb_0, usb_1, usb_2, or usb_3)
# userDevice = the name of the device that 
# driveLetter = the letter of the USB device being tested
def usb_cycle(numberOfCycles, comPort, cycleType, userDevice, driveLetter):
    try:
        report = open("{cycleType}_{device}.csv".format(cycleType=cycleType, device=userDevice), "w")   # creats a csv file to store results
    except:
        sys.exit("Could not create or find report file, closing program.")

    report.write("cycle,enum time,enum speed,write time,written successfully\n")                        # writes the header row of the csv file

    usb_devices = get_device_data()                                         # get the dict of all known USB devices specified in the json file
    
    numberOfCyclesCompleted = 0
    serialData = serial.Serial(comPort, 9600)                               # opens the COM port to communicate with microcontroller

    print("Starting {cycleType} Cycles".format(cycleType=cycleType))
    
    while(numberOfCyclesCompleted < numberOfCycles):                        # loop until all cycles are complete
        time.sleep(0.5)                                                     
        if serialData.readline() == "ready\n".encode():                     # if the microcontroller is ready for cycling continue...                     
            serialData.flushInput()                                         # clear all UART input
            time.sleep(0.5)
            numberOfCyclesCompleted = numberOfCyclesCompleted + 1           # increment number of cycles completed
            print("Cycle {cycle}".format(cycle=numberOfCyclesCompleted))
            report.write(str(numberOfCyclesCompleted) + ',')                # write which cycle is being exicuted to the csv file
            serialData.write(cycleType.encode())                            # tell the microcontroller which USB cycle to exicute
            while serialData.readline() != "ready\n".encode():              # while the microcontroller is not done the cycle, do nothing
                pass
            usb_test(report, userDevice, usb_devices, driveLetter)          # test the USB device after the cycle is complete
        
    report.close()                                                          # close both the csv report and the COM port after all cycles have been complete
    serialData.close()
    
# Tests a USB device by getting the windows enumeration time, write time of a test file, confirmation that
# the file was written successfully and the bus speed of the device after enumeration
# report = the csv file that the results will be writen to
# userConnectedDevice = name of the device under test 
# usb_devices = dictionary of known USB devices from Usb_Devices.json
# driveLetter = drive letter of the device under test
def usb_test(report, userConnectedDevice, usb_devices, driveLetter):
    try:
        current_device = usb_devices[userConnectedDevice]                                       # find device under test in the dictionary of known devices
    except:
        sys.exit("ERROR: Could not find " + userConnectedDevice + " in list of known devices, closing program.")

    enumTime = windowsEnumTime(driveLetter)                                                     # get the Windows enumeration time of the device under test
    reportLine = str(enumTime)                                                                  # start building the row that will be written to the csv file

    connectedDevice = get_usb_device(current_device['vid'], current_device['pid'])              # get USB device information from pyUSB
    readableSpeed = get_readable_speed(connectedDevice.speed)                                   # translate the bus speed given by pyUSB to be more readable
    reportLine = reportLine + ',' + str(readableSpeed)                                          # append drive bus speed to the csv row
    
    print("Starting to write testfile to USB drive {letter}...".format(letter=driveLetter))
    driveLetter = driveLetter + ":\\"                                                           # create a file path from the drive letter
    start = time.perf_counter()                                                                 # start time before file is written to the drive
    writtenLocation = shutil.copy('TestFile.log', driveLetter)                                  # copy a file over to the drive
    end = time.perf_counter()                                                                   # end time after the file was written
    writeTime = round(end - start, 3)                                                           # gets the write time and rounds it to 3 decimal places
    reportLine = reportLine + ',' + str(writeTime)                                              # append write time to the csv row

    print("Done writing testfile to drive {letter}".format(letter=driveLetter))

    if filecmp.cmp('TestFile.log', writtenLocation):                                            # verify that the file was written to the drive sucessfully and append result
        reportLine = reportLine + ",yes\n"                                                      # to the csv row
    else:
        reportLine = reportLine + ",no\n"
    print("Done verifying that the file was written properly")

    report.write(reportLine)                                                                    # write the row to the csv file

    if os.path.exists(writtenLocation):                                                         # delete the the file that was written on the USB device
        os.remove(writtenLocation)