README

    ge_eeprom is a python script that allows users to read/write EEPROM page data on Icron's
    Goldenears platforms using Icron's debugging protocols through serial communication between a PC
    and a Icron's Goldenears product. To execute the script, it requires Python 3 and pySerial
    module pre-installed on the PC. Visit www.python.org to download the latest version of Python
    3. To install pySerial, follow the instruction on the following webpage:
        https://pythonhosted.org/pyserial/pyserial.html

    An example of how to execute script to read data from EEPROM is shown below:
        python3 ge_eeprom.py [serial_port_name] read > [output_file]

    To list the COM port the Icron's unit is connected to, users can execute the following:
        python3 -m serial.tools.list_ports

    If the execution succeeds, data stored in all pages of the EEPROM should be output into the
    given file via stdout in hexadecimal. The size of the data should match the one of the EEPROM.

    Similarly, users can write data from a given input file into EEPROM by executing the following in
    the terminal:
        python3 ge_eeprom.py [serial_port_name] write < [input_file]
    Also, the size of the data in the given input file should match the EEPROM's size.

CONTACT

    If you have problems or questions, please contact Icron's Tech support at: techsupport@icron.com
