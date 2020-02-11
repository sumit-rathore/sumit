
    def create_icmd(self, component_name, function_name, response, *arguments):
        """
        Create icmd object from component name, function name and arguments.

        Arguments: response - whether sender expects a response for the icmd
        """
        try:
            if response:
                self.icmd_response_validator[(self.icmd_response_id, self.icmd_channel_id)] = \
                            self._build_icmd_response_validator(component_name, function_name)

            if len(arguments) == 0:
                return self.icmd_encoder.encode(component_name, function_name)
            else:
                return self.icmd_encoder.encode(component_name, function_name, arguments[0])
        except:
            error_message = "{}: {}: Got an error when creating icmd:". \
                    format(self.port_name, self.device_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

    def _build_icmd_response_validator(self, component_name, function_name):
        """
        A closure for building icmd response validator.

        Arguments: component_name - icmd associated component name
                   function_name - icmd associated function name
        """
        def validator(response_len):
            expected_num_args = self.icmd_model.get_num_icmd_response_arguments(
                                                                    component_name,
                                                                    function_name)
            expected_lenth = expected_num_args * 4
            return expected_lenth == response_len
        return validator

    def send_icmd(self, icmd_obj, response=False):
        """
        Send icmd by writing the data packet of icmd to serial port.

        Arguments: icmd_obj - icmd object to be sent out
                   response - whether sender expects a response from device
        """
        try:
            packet = ipk.packetize(
                            self.icmd_channel_id,
                            len(icmd_obj.as_integer_list),
                            icmd_obj.as_integer_list,
                            self.icmd_response_id) if response else \
                     ipk.packetize(
                             self.icmd_channel_id,
                             len(icmd_obj.as_integer_list),
                             icmd_obj.as_integer_list)

            cbs_logger.info("{}: {}: Trying to send an icmd with a packet of {}". \
                            format(self.port_name, self.device_name, packet))
            self.serial_port_manager.write(packet)
        except:
            error_message = "{}: {}: Got an error when sending icmd". \
                                format(self.port_name, self.device_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

    def send_icmd_wait_for_response(self, icmd_obj):
        """
        Send icmd to device and wait for its response.

        Arguments: icmd_obj - icmd object to be sent out
        """
        try:
            with self.send_icmd_wait_for_response_lock:
                cbs_logger.info(
                    "{}: {}: Sending an icmd and waiting for response: respon_id={} chan_id={}". \
                        format(
                            self.port_name,
                            self.device_name,
                            self.icmd_response_id,
                            self.icmd_channel_id))

                self.register_packet_handler(
                        self._decode_icmd_response,
                        self.icmd_response_id,
                        self.icmd_channel_id)

                self.send_icmd(icmd_obj, True)
                return self._poll_icmd_response(self.icmd_response_id, self.icmd_channel_id)
        except:
            error_message = "{}: {}: Got an error when sending icmd and waiting for response:" \
                                .format(self.port_name, self.device_name)
            cbs_logger.exception(error_message)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

    def _decode_icmd_response(self, packet, timestamp=None):
        """
        Decode icmd response from received packet.

        Arguments: packet - received packet from icmd channel
        """
        try:
            cbs_logger.debug(
                "{}: {}: Got an icmd response chan_id={} response_id={} len={} payload={}". \
                    format(self.port_name, self.device_name, packet.channel_id,
                        packet.response_id, packet.payload_length, packet.payload_data))

            response_len = packet.payload_length
            response_payload = packet.payload_data
            validator = self.icmd_response_validator.pop((packet.response_id, packet.channel_id))
            if validator(response_len):
                num_args = response_len / 4
                results = []
                for i in range(num_args):
                    response_bytes = b''
                    for j in range(4):
                        response_bytes += struct.pack('B', response_payload[4*i + j])
                    arg_value = struct.unpack('>L', response_bytes)
                    results.append(arg_value[0])
                self.icmd_response_result[(packet.response_id, packet.channel_id)] = tuple(results)
            else:
                raise icr.IcmdResponseError(
                        "Invalid icmd response",
                        response_len,
                        response_payload)

        except icr.IcmdResponseError as e:
            error_message = "{}: {}: ".format(self.port_name, self.device_name) + str(e)
            cbs_logger.exception(error_message)
            Forms.MessageBox.Show(
                error_message,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)
        except:
            error_message = "{}: {}: Got an error when decoding icmd response". \
                    format(self.port_name, self.device_name)
            error_string = error_message + "\n" + \
                    "{}: {}".format(sys.exc_info()[0], sys.exc_info()[1])
            cbs_logger.exception(error_string)
            Forms.MessageBox.Show(
                error_string,
                "ERROR",
                Forms.MessageBoxButtons.OK,
                Forms.MessageBoxIcon.Error)

    def _poll_icmd_response(self, response_id, channel_id, timeout=3):
        """
        Poll icmd response from self.icmd_response_result associated with the specific response id
        and channel id. The default timeout is 3 seconds.

        Arguments: response_id - associated response id
                   channel_id - associated channel id
                   timeout - period of polling
        """
        try:
            self.icmd_response_id = self.icmd_response_id + 1 if self.icmd_response_id < 254 else 0
            start_time = time.time()
            result = None
            while True:
                if self.icmd_response_result.get((response_id, channel_id)):
                    result = self.icmd_response_result.pop((response_id, channel_id))
                    break

                if time.time() - start_time > timeout:
                    cbs_logger.debug("{}: ".format(self.port_name) + \
                                    "{}: ".format(self.device_name) + \
                                    "Timed out on polling icmd response for " + \
                                    "response_id={}, ".format(response_id) + \
                                    "channel_id={}, ".format(channel_id) + \
                                    "timeout={}".format(timeout))
                    break

            self.remove_packet_received_handler(response_id, channel_id)
            return result
        except:
            cbs_logger.exception("{}: {}: Got an error when polling icmd response". \
                    format(self.port_name, self.device_name))
