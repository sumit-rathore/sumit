#!/usr/bin/python
import usb.core
import usb.backend.libusb1
import usb.util as util

def getClassString(classId):

    classDict = {
            "0x1" : "Audio",
            "0x2" : "Communications and CDC Control",
            "0x3" : "HID",
            "0x5" : "Physical",
            "0x6" : "Image",
            "0x7" : "Printer",
            "0x8" : "Mass Storage",
            "0x9" : "Hub",
            "0xA" : "CDC-Data",
            "0xB" : "Smart Card",
            "0xC" : "Content Security",
            "0xD" : "Video",
            "0xE" : "Personal HealthCare",
            "0xF" : "Audio/Video Devices",
            "0x11" : "BillBoard Device Class",
            "0x12" : "USB Type-C Bridge Class",
            "0xdc" : "Diagnostic Device",
            "0xe0" : "Wireless Controller",
            "0xef" : "Miscellaneous",
            "0xfe" : "Application Specific",
            "0xff" : "Vendor Specific",
            }

    answer = classDict.get(str(classId))
    if answer is None:
        answer = classId

    return answer

def getSpeedString(deviceSpeed):
    speedDict = {
            "0x1" : "Low Speed",
            "0x2" : "Full Speed",
            "0x3" : "High Speed",
            "0x4" : "Super Speed",
            }
    answer = speedDict.get(str(deviceSpeed))
    if answer is None:
        answer = deviceSpeed

    return answer

def usbData(log_file):
    # find USB devices
    dev = usb.core.find(find_all=True)
    with open(log_file, 'a', encoding='utf-8') as out_file:
        out_file.write("\n ******* NEW CYCLE *******\n")
        # loop through devices, printing USB data
        for cfg in dev:
            #  sys.stdout.write('Hexadecimal VendorID=' + hex(cfg.idVendor) + ' & ProductID=' + hex(cfg.idProduct) + '\n\n')
            #print("Decimal VendorID=  " + str(cfg.idVendor))
            #print("ProductID:         " + str(cfg.idProduct))
            out_file.write("VendorID:          " + hex(cfg.idVendor) + "\n")
            out_file.write("ProductID:         " + hex(cfg.idProduct) + "\n")
            out_file.write("device speed:      " + getSpeedString(hex(cfg.speed)) + "\n")
            out_file.write("device bus:        " + str(cfg.bus) + "\n")
            out_file.write("device address:    " + str(cfg.address) + "\n")
            out_file.write("device port:       " + str(cfg.port_number) + "\n")
            out_file.write("device class:      " + getClassString(hex(cfg.bDeviceClass)) + "\n")
            out_file.write("device sub class:  " + str(cfg.bDeviceSubClass) + "\n")
            out_file.write("bcd device:        " + str(cfg.bcdDevice) + "\n")
            try:
                out_file.write("Manufacturer:      " + str(util.get_string(cfg, cfg.iManufacturer, None)) + "\n")
            except:
                out_file.write("Manufacturer:      " + "NONE\n")

            out_file.write("______________________________________________________\n")
    out_file.close()

