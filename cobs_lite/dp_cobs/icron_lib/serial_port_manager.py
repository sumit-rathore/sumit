import threading
import time
from cobs_logger import cbs_logger
from icron_depacketizer import Depacketizer
import icron_log_recorder as ilr

import System.IO.Ports as Ports
from System import Array, Byte

class SerialPortManager:
    def __init__(self,
                 port_name,
                 baud_rate=460800,
                 parity=Ports.Parity.None,
                 data_bits=8,
                 stop_bits=Ports.StopBits.One,
                 handshake=Ports.Handshake.None,
                 write_timeout=500,
                 read_timeout=500):

        #initialize serial port
        if port_name in Ports.SerialPort.GetPortNames():
            self.serial_port = Ports.SerialPort(port_name, baud_rate, parity, data_bits, stop_bits)
            self.serial_port.Handshake = handshake
            self.serial_port.WriteTimeout = write_timeout
            self.serial_port.ReadTimeout = read_timeout

            self.serial_port.DataReceived += self.serial_port_data_received_handler
            self.serial_port.Disposed += self.serial_port_disposed_handler
            self.serial_port.ErrorReceived += self.serial_port_error_received_handler
            self.serial_port.PinChanged += self.serial_port_pin_changed_handler
        else:
            raise NameError("Serial port name is not valid")

        self.depacketizer = Depacketizer()
        self.log_recorder = ilr.IcronLogRecorder()
        self.write_lock = threading.Lock()

    def open_port(self):
        """
        Open the serial port.
        """
        try:
            self.serial_port.Open()
        except:
            cbs_logger.exception("Can't open serial port {}.".format(self.port_name))

    def close_port(self):
        """
        Close the serial port.
        """
        return self.serial_port.Close()

    def serial_port_data_received_handler(self, sender, event):
        """
        Handle data received from serial port by invoking depacketizer to parse the received byte.

        sender - sender of the event, which is the serial port
        event - self.serial_port_DataReceived
        """
        try:
            while self.serial_port.IsOpen and self.serial_port.BytesToRead:
                byte = self.serial_port.ReadByte()
                self.log_recorder.record(byte)
                self.depacketizer.handle_byte_received(byte)
        except:
            cbs_logger.exception("Failed to access serial port {}.".format(self.port_name))


    def register_listener(self, client_name, packet_received_handler):
        """
        Register a listern by registering a client packet received handler associated with the
        device client name in the depacketizer.

        client_name - device client name
        packet_received_handler - device client's handler for handling received byte
        """
        try:
            self.depacketizer.register_packet_received_handler(client_name, packet_received_handler)
        except:
            cbs_logger.exception("Got an error when registering listener {} for {}". \
                                    format(packet_handler, client_name))

    def remove_listener(self, client_name):
        """
        Remove the existing client from depacketizer and if none is left close the port.

        client name - device client name
        """
        try:
            self.depacketizer.remove_packet_received_handler(client_name)

            # close serial port if depackeitzer has no client packet handler left
            if not self.depacketizer.client_packet_handlers:
                if self.is_port_open:
                    self.close_port()
        except:
            cbs_logger.exception("Got an error when removing listener for {}". \
                                    format(client_name))

    def register_channel_id(self, channel_id_list):
        self.depacketizer.update_channel_id(channel_id_list)

    def write(self, data):
        try:
            with self.write_lock:
                #print "enter the write with data={}".format(data)
                self.serial_port.Write(Array[Byte](data), 0, len(data))
                #time.sleep(5)
                #print "exit the write with data={}".format(data)
        except:
            cbs_logger.exception("Got an error when writing data to serial port {}" \
                    .format(self.port_name))

    def discard_in_buffer(self):
        self.serial_port.DiscardInBuffer()

    def discard_out_buffer(self):
        self.serial_port.DiscardOutBuffer()

    @property
    def is_port_open(self):
        """
        Return True if the port is open otherwise False.
        """
        return self.serial_port.IsOpen

    def serial_port_disposed_handler(self, sender, event):
        cbs_logger.info("{}: Serial port disposed event occurred".format(self.port_name))

    def serial_port_error_received_handler(self, sender, event):
        cbs_logger.info("{}: Serial port error received event occured".format(self.port_name))

    def serial_port_pin_changed_handler(self, sender, event):
        cbs_logger.info("{}: Serial port pin changed event occurred".format(self.port_name))

    @property
    def port_name(self):
        return self.serial_port.PortName

    @property
    def baud_rate(self):
        return self.serial_port.BaudRate

    @property
    def data_bits(self):
        return self.DataBits

    @property
    def parity(self):
        return self.serial_port.Parity

    @property
    def stop_bits(self):
        return self.serial_port.StopBits

    @property
    def handshake(self):
        return self.serial_port.Handshake

    @property
    def num_listeners(self):
        return len(self.depacketizer.client_packet_handlers)

    @property
    def is_disposed(self):
        return self.serial_port.Disposed
