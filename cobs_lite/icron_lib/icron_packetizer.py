def packetize(channel_id, payload_len, payload_list, response_id=255, version_id=0):
    start_of_header = 0x1
    end_of_transaction = 0x4
    if payload_len in range(256) and channel_id in range(256) and response_id in range(256):
        packet = [start_of_header, version_id, channel_id, response_id, payload_len]
        packet.extend(payload_list)
        packet.append(end_of_transaction)
        return packet
    else:
        raise PacketFormatError(
                "packetizing error",
                start_of_header,
                version_id,
                channel_id,
                response_id,
                len(payload_list),
                payload_list,
                end_of_transaction)


class PacketFormatError(Exception):
    def __init__(
            self,
            message,
            start_of_header,
            version_id,
            channel_id,
            response_id,
            payload_len,
            payload,
            end_of_transaction):
        self.message = message
        self.start_of_header = start_of_header
        self.version_id = version_id
        self.channel_id = channel_id
        self.response_id = response_id
        self.payload_len = payload_len
        self.payload = payload
        self.end_of_transaction = end_of_transaction

    def __str__(self):
        return "{}: ".format(self.message) + \
                "start_of_header={} ".format(self.start_of_header) + \
                "version_id={} ".format(self.version_id) + \
                "channel_id={} ".format(self.channel_id) + \
                "response_id={} ".format(self.response_id) + \
                "payload_len={} ".format(self.payload_len) + \
                "payload={} ".format(self.payload) + \
                "eot={}".format(self.end_of_transaction)

