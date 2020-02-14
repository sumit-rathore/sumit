README

    ge_update_adder_eeprom is a python script that allows users to update Adder's EEPROM data on
    Icron's Goldenears platforms using Icron's debugging protocols through serial communication
    between a PC and a Icron's Goldenears product. To execute the script, it requires Python 3 and
    pySerial module pre-installed on the PC. Visit www.python.org to download the latest version of
    Python 3. To install pySerial, follow the instruction on the following webpage:
        https://pythonhosted.org/pyserial/pyserial.html

    An example of how to execute script in command prompt to update the EEPROM is shown below:
        python ge_update_adder_eeprom.py [serial_port_name]

    To list the COM port the Icron's unit is connected to, users can execute the following:
        python -m serial.tools.list_ports

    If the execution succeeds, the command prompt will print out "Update succeeded!".

CONTACT

    If you have problems or questions, please contact Icron's Tech support at: techsupport@icron.com
