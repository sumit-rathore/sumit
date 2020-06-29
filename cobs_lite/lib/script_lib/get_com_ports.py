#!/usr/bin/env python

import serial.tools.list_ports

comPorts = list(serial.tools.list_ports.comports())

print("Available COM Ports:")
for port in comPorts:
    if port.description == ('USB Serial Port '+ '(' + port.device + ')'):
        print(" " + port.device)
