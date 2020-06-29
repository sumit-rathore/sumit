class IcmdResponseError(Exception):
    def __init__(self, message, response_len, response_payload):
        self.message = message
        self.response_len = response_len
        self.response_payload = response_payload

    def __str__(self):
        return "{}: ".format(self.message) + \
                "response_len={} ".format(self.response_len) + \
                "response_payload={}".format(self.response_payload)
