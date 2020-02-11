import pyevent
import traceback
#from cobs_logger import cbs_logger

class Depacketizer():
    def __init__(self):
        self.__handlers = {}

        self.hook, self._caller = pyevent.make_event()

        self.__state = 0
        self.__start_of_header = 0x1
        self.__version_id = 0
        self.__channel_id = None
        self.__response_id = None
        self.__payload_length = None
        self.__payload_data = []
        self.__end_of_transaction = 0x4
        self.__channel_id_set = set()

        # this is used for recovering icron packet from invalid packet
        self.__data_buffer = []

    def register_packet_received_handler(self, device_client_name, handler):
        """Register a device client's handler for handling byte received event. The handler should
        take two arguments: sender and received data.

        device_client_name - name of the device client to be registered
        handler - the function/method to be registered for handling packet received event
        """
        #print "Before register: {}".format(self.client_packet_handlers)
        if device_client_name not in self.__handlers:
            self.hook += handler
            self.__handlers[device_client_name] = handler
        else:
            raise Exception("{}'s handler {} already exists.".format(device_client_name, handler))
        #print "After register: {}".format(self.client_packet_handlers)

    def remove_packet_received_handler(self, device_client_name):
        """Remove an existing handler from the self.hook.

        device_client_name - name of device client to be removed
        """
        #print "Before remove: {}".format(self.client_packet_handlers)
        if device_client_name in self.__handlers:
            self.hook -= self.__handlers[device_client_name]
            del self.__handlers[device_client_name]
        else:
            raise Exception("No device client {} found to be removed from depacketizer". \
                    format(device_client_name))
        #print "After remove: {}".format(self.client_packet_handlers)

    def update_channel_id(self, channel_id_list):
        self.__channel_id_set.update(channel_id_list)

    def handle_byte_received(self, byte):
        packet, valid = self.parse(byte)
        if packet is not None and valid is not None:
            #cbs_logger.info("pkt={}, valid={}".format(packet, valid))
            self._trigger_packet_received_event(packet, valid)

    def __recover(self, byte_list):
        # byte_list should be the stored bytes captured for the new Icron packet
        assert self.__state != 0
        if self.__state == 1:
            assert len(byte_list) == 1
        if self.__state == 2:
            assert len(byte_list) == 2
        if self.__state == 3:
            assert len(byte_list) == 3
        if self.__state == 4:
            assert len(byte_list) == 4
        if self.__state == 6:
            assert len(byte_list) > 5

        self.__reset()
        dropped_bytes = []
        i = 0
        while i < len(byte_list):
            #print "while i={} byte_list={}".format(i, byte_list)
            if self.__state == 0:
                #print "s0: i={} byte_list={}".format(i, byte_list)
                if i < len(byte_list):
                    if byte_list[i] == self.__start_of_header:
                        self.__state = 1
                        i += 1
                    else:
                        i = 0
                        self.__reset()
                        dropped_bytes.append(byte_list.pop(0))
                        #print "s0 d_bytes={}".format(dropped_bytes)

            if self.__state == 1:
                #print "s1: i={} byte_list={}".format(i, byte_list)
                if i < len(byte_list):
                    if byte_list[1] == self.__version_id:
                        self.__state = 2
                        i += 1
                    else:
                        i = 0
                        self.__reset()
                        dropped_bytes.append(byte_list.pop(0))
                        #print "s1 d_bytes={}".format(dropped_bytes)

            if self.__state == 2:
                #print "s2: i={} byte_list={}".format(i, byte_list)
                if i < len(byte_list):
                    if byte_list[1] in self.__channel_id_set:
                        self.__channel_id = byte_list[i]
                        self.__state = 3
                        i += 1
                    else:
                        i = 0
                        self.__reset()
                        dropped_bytes.append(byte_list.pop(0))
                        #print "s2 d_bytes={}".format(dropped_bytes)

            if self.__state == 3:
                #print "s3: i={} byte_list={}".format(i, byte_list)
                if i < len(byte_list):
                    if byte_list[i] in xrange(0, 256):
                        self.__response_id = byte_list[i]
                        self.__state = 4
                        i += 1
                    else:
                        i = 0
                        self.__reset()
                        dropped_bytes.append(byte_list.pop(0))
                        #print "s3 d_bytes={}".format(dropped_bytes)

            if self.__state == 4:
                #print "s4: i={} byte_list={}".format(i, byte_list)
                if i < len(byte_list):
                    if byte_list[i] in xrange(0, 256):
                        self.__payload_length = byte_list[i]
                        self.__state = 5
                        i += 1
                    else:
                        i = 0
                        self.__reset()
                        dropped_bytes.append(byte_list.pop(0))
                        #print "s4 d_bytes={}".format(dropped_bytes)

            if self.__state == 5:
                #print "s5: i={} byte_list={}".format(i, byte_list)
                if i < len(byte_list):
                    self.__payload_data.append(byte_list[i])
                    if self.__payload_length == 0:
                        self.__state = 5 if len(self.__payload_data) < 256 else 6
                    else:
                        self.__state = 5 if len(self.__payload_data) < self.__payload_length else 6
                    i += 1

            if self.__state == 6:
                #print "s6: i={} byte_list={}".format(i, byte_list)
                if i < len(byte_list):
                    if byte_list[i] == self.__end_of_transaction:
                        self.__data_buffer = [self.__start_of_header, self.__version_id,
                            self.__channel_id, self.__response_id, self.__payload_length] + \
                            self.__payload_data + byte_list[i:] + self.__data_buffer
                        #print "s6_t d_bytes={}".format(dropped_bytes)
                        return dropped_bytes
                    else:
                        i = 0
                        self.__reset()
                        dropped_bytes.append(byte_list.pop(0))
                        #print "s6_f d_bytes={}".format(dropped_bytes)

        #print "dropped_bytes={} state={}".format(dropped_bytes, self.__state)
        return dropped_bytes 

    def parse(self, byte):
        """Parse one byte at a time for depacketizing the Icron packet. If one complete packet
        frame is found, trigger packet received event.

        byte - integer value of the byte to parse
        """
        self.__data_buffer.append(byte)
        #print "byte={} data_buffer={} state={}".format(byte, self.__data_buffer, self.__state)

        while len(self.__data_buffer) > 0:
            byte = self.__data_buffer.pop(0)

            #cbs_logger.info("parsing byte: {}".format(byte))
            if self.__state == 0:
                #cbs_logger.info("In START_OF_HEADER_STATE")
                if byte == self.__start_of_header:
                    #cbs_logger.info("Going to CHANNEL_ID_STATE")
                    self.__state = 1
                else:
                    return [byte], False

            elif self.__state == 1:
                #cbs_logger.info("In VERSION_ID_STATE: {}".format(self.__version_id))
                if byte == self.__version_id:
                    self.__state = 2
                else:
                    # drop the first byte stored which is self.__start_of_header
                    packet = self.__recover([byte])
                    return  [self.__start_of_header] + packet, False

            elif self.__state == 2:
                #cbs_logger.info("In CHANNEL_ID_STATE: {}".format(self.__channel_id))
                if byte in self.__channel_id_set:
                    self.__state = 3
                    self.__channel_id = byte
                else:
                    packet = self.__recover([self.__version_id, byte])
                    return  [self.__start_of_header] + packet, False

            elif self.__state == 3:
                #cbs_logger.info("In RESPONSE_ID_STATE: {}".format(self.__response_id))
                if byte in xrange(0, 256):
                    self.__response_id = byte
                    self.__state = 4
                else:
                    packet = self.__recover([self.__version_id, self.__channel_id, byte])
                    return  [self.__start_of_header] + packet, False

            elif self.__state == 4:
                #cbs_logger.info("In PAYLOAD_LENGTH_STATE: {}".format(self.__payload_length))
                if byte in xrange(0, 256):
                    self.__payload_length = byte
                    self.__state = 5
                else:
                    packet = self.__recover(
                                        [self.__version_id,
                                         self.__channel_id,
                                         self.__response_id,
                                         byte])
                    return  [self.__start_of_header] + packet, False

            elif self.__state == 5:
                self.__payload_data.append(byte)
                #cbs_logger.info("In PAYLOAD_STATE: {}".format(self.__payload_data))
                if self.__payload_length == 0:
                    self.__state = 5 if len(self.__payload_data) < 256 else 6
                else:
                    self.__state = 5 if len(self.__payload_data) < self.__payload_length else 6

            elif self.__state == 6:
                if byte == self.__end_of_transaction:
                    #cbs_logger.info("In END_OF_TRANSCATION_STATE: {}". \
                    #       format(self.__end_of_transaction))
                    #cbs_logger.info(
                    #           "version_id={}, ".format(self.__version_id) + \
                    #           "channel_id={}, ".format(self.__channel_id) + \
                    #           "response_id={}, ".format(self.__response_id) + \
                    #           "payload_length={}, ".format(self.__payload_length) + \
                    #           "payload_data={} ".format(self.__payload_length))
                    packet = IcronPacket(
                                self.__version_id,
                                self.__channel_id,
                                self.__response_id,
                                self.__payload_length,
                                self.__payload_data)
                    valid = True
                    self.__reset()
                else:
                    packet = self.__recover(
                                        [self.__version_id,
                                            self.__channel_id,
                                            self.__response_id,
                                            self.__payload_length] + \
                                         self.__payload_data + [byte])
                    packet = [self.__start_of_header] + packet
                    valid = False
                return packet, valid
            else:
                raise Exception("Invalid depacketizer state")
        return None, None

    def __reset(self):
        self.__state = 0
        self.__channel_id = None
        self.__response_id = None
        self.__payload_length = None
        self.__payload_data = []

    def _trigger_packet_received_event(self, packet, is_valid_packet):
        """Trigger the packet received event and multicast the event along with data to the
        registered handlers. The execution order of the handers is based on the order of
        registration.

        packet - the received icron packet
        """
        try:
            self._caller(self, packet, is_valid_packet)
        except:
            traceback.print_exc()

    @property
    def client_packet_handlers(self):
        return self.__handlers


class IcronPacket():
    def __init__(
            self,
            version_id,
            channel_id,
            response_id,
            payload_length,
            payload_data,
            start_of_header=0x1,
            end_of_transaction=0x4):

        self.__start_of_header = start_of_header
        self.__version_id = version_id
        self.__channel_id = channel_id
        self.__response_id = response_id
        self.__payload_length = payload_length
        self.__payload_data = payload_data
        self.__end_of_transaction = end_of_transaction

    @property
    def start_of_header(self):
        return self.__start_of_header

    @property
    def version_id(self):
        return self.__version_id

    @property
    def channel_id(self):
        return self.__channel_id

    @property
    def response_id(self):
        return self.__response_id

    @property
    def payload_length(self):
        return self.__payload_length

    @property
    def payload_data(self):
        return self.__payload_data

    @property
    def end_of_transaction(self):
        return self.__end_of_transaction

    @property
    def channal_id_set(self):
        return self.__channel_id_set

    def __str__(self):
        return "pkt: soh={} ".format(self.start_of_header) + \
                "version_id={} ".format(self.version_id) + \
                "channel_id={} ".format(self.channel_id) + \
                "response_id={} ".format(self.response_id) + \
                "payload_len={} ".format(self.payload_length) + \
                "payload={} ".format(self.payload_data) + \
                "eot={}".format(self.end_of_transaction)

    def as_integer_list(self):
        packet_int_list = [self.start_of_header,
                            self.version_id,
                            self.channel_id,
                            self.response_id,
                            self.payload_length]
        packet_int_list.extend(self.payload_data)
        packet_int_list.append(self.end_of_transaction)
        return packet_int_list
