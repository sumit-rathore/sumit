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


# find USB devices
dev = usb.core.find(find_all=True)
# loop through devices, printing USB data
for cfg in dev:
#  sys.stdout.write('Hexadecimal VendorID=' + hex(cfg.idVendor) + ' & ProductID=' + hex(cfg.idProduct) + '\n\n')
   #print("Decimal VendorID=  " + str(cfg.idVendor))
   #print("ProductID:         " + str(cfg.idProduct))
    print("VendorID:          " + hex(cfg.idVendor))
    print("ProductID:         " + hex(cfg.idProduct))
    print("device speed:      " + str(cfg.speed))
    print("device bus:        " + str(cfg.bus))
    print("device address:    " + str(cfg.address))
    print("device port:       " + str(cfg.port_number))
    print("device class:      " + getClassString(hex(cfg.bDeviceClass)))
    print("device sub class:  " + str(cfg.bDeviceSubClass))
    print("bcd device:        " + str(cfg.bcdDevice))
    try:
        print("Manufacturer:      " + str(util.get_string(cfg, cfg.iManufacturer, None)))
    except:
        print("Manufacturer:      " + "NONE")

    print("______________________________________________________\n")


