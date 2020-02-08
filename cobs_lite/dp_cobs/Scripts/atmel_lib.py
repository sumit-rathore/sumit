from abc import ABCMeta, abstractproperty
import icron_lib.icron_crc as icron_crc


def word_to_bytes(word, produce_big_endian=False):
    big_bytes = ((word >> 24) & 0xff, (word >> 16) & 0xff, (word >> 8) & 0xff, word & 0xff)
    if produce_big_endian:
        return big_bytes
    else:
        return tuple(reversed(big_bytes))


def bytes_to_word(bytes_, start_index=0, produce_big_endian=False):
    i = start_index  # alias for brevity
    if produce_big_endian:
        return (bytes_[i+0] << 24) | (bytes_[i+1] << 16) | (bytes_[i+2] << 8) | (bytes_[i+3])
    else:
        return (bytes_[i+3] << 24) | (bytes_[i+2] << 16) | (bytes_[i+1] << 8) | (bytes_[i+0])


class AuthChipException(Exception):
    pass


class AuthChipAccessor(object):
    component = 'ATMEL_CRYPTO_COMPONENT'

    def __init__(self, device):
        self._dev = device

    def read_config_word(self, word_offset):
        read_cmd = self._dev.create_icmd(
                self.component, "atmel_readConfigWordIcmd", True, (word_offset,))
        cfg_word = self._send_icmd_with_retries(read_cmd)
        if cfg_word is None:
            raise AuthChipException('Failed to read configuration word at offset %d' % word_offset)
        return cfg_word[0]

    def fetch_config_data(self):
        config_data = bytearray(AuthChipConfigurationZone.array_size)
        for i in xrange(0, AuthChipConfigurationZone.array_size, 4):
            config_data[i:i+4] = word_to_bytes(self.read_config_word(i), False)
        return AuthChipConfigurationZone(config_data)

    def write_config_word(self, word_offset, word):
        write_cmd = self._dev.create_icmd(
                self.component, "atmel_writeConfigWordIcmd", True, (word_offset, word))
        response = self._send_icmd_with_retries(write_cmd)
        if response is None:
            exc_str = 'Failed to write configuration word at offset %d with value %d' % (
                    word_offset, word)
            raise AuthChipException(exc_str)

    def lock_config_zone(self, config_zone_crc):
        lock_cmd = self._dev.create_icmd(
                self.component, 'atmel_lockConfigZoneIcmd', True, (config_zone_crc,))
        response = self._send_icmd_with_retries(lock_cmd)
        if response is None:
            raise AuthChipException('Failed to lock config zone with CRC 0x%04x' % config_zone_crc)

    def lock_data_and_otp_zones(self, data_and_otp_zone_crc):
        lock_cmd = self._dev.create_icmd(
                self.component, 'atmel_lockDataAndOtpZonesIcmd', True, (data_and_otp_zone_crc,))
        response = self._send_icmd_with_retries(lock_cmd)
        if response is None:
            raise AuthChipException('Failed to lock data and OTP zones with CRC 0x%04x' %
                    data_and_otp_zone_crc)

    def write_data_slot(self, slot, data):
        self._generic_block_write(slot, data, 'atmel_writeDataSlotFromBuffer', 'slot')

    def write_otp_block(self, block, data):
        self._generic_block_write(block, data, 'atmel_writeOtpBlockFromBuffer', 'block')

    def _generic_block_write(self, block, data, write_command_str, block_type_str):
        load_data_command_str = 'atmel_setICmdWriteDataBuffer'
        for word_index in xrange(0, AuthChipBlockZone.block_len / 4):
            word = bytes_to_word(data, word_index * 4, True)
            load_data_cmd = self._dev.create_icmd(
                    self.component, load_data_command_str, True, (word_index, word))
            response = self._send_icmd_with_retries(load_data_cmd)
            if response is None:
                ex = '%s failed for %s %d, word index %d, word value 0x%08x'
                raise AuthChipException(
                        ex % (load_data_command_str, block_type_str, block, word_index , word))
        write_cmd = self._dev.create_icmd(self.component, write_command_str, True, (block,))
        response = self._send_icmd_with_retries(write_cmd)
        if response is None:
            raise AuthChipException('%s failed for %s %d' %
                    (write_command_str, block_type_str, block))

    def _send_icmd_with_retries(self, icmd, retries=3):
        retries += 1  # one try, n retries
        response = None
        while response is None and retries > 0:
            response = self._dev.send_icmd_wait_for_response(icmd)
            retries -= 1
        return response


class AuthChipZone(object):
    """Abstract base class for auth chip zone classes."""
    __metaclass__ = ABCMeta

    def __init__(self, array_data_iterable=None, array_fill=0):
        if array_data_iterable is None:
            self._array = bytearray(array_fill for __ in xrange(self.array_size))
        else:
            self._array = bytearray(array_data_iterable)
            if len(self._array) != self.array_size:
                ex = 'Invalid input array length %d; expected %d'
                raise AuthChipException(ex % (len(self._array), self.array_size))

    @abstractproperty
    def array_size(self):
        raise NotImplementedError 

    def get_byte(self, index):
        return self._array[index]

    def set_byte(self, index, byte):
        self._array[index] = byte

    def get_word(self, index, produce_big_endian=False):
        return bytes_to_word(self._array[index:index+4], produce_big_endian=produce_big_endian)

    def get_array(self):
        return bytearray(self._array)

    def crc16(self):
        return icron_crc.crc16(self._array)


class AuthChipBlockZone(AuthChipZone):
    """Abstract base class for auth chip zone classes which have block structures; i.e., data and
    OTP.
    """
    __metaclass__ = ABCMeta

    # The ATSHA204A datasheet uses the term 'slots' when referring to chunks of 32 bytes in the data
    # zone, and the term 'blocks' when referring to chunks of 32 bytes in the config and OTP zones.
    # For simplicity, we always use the term 'blocks' within this class.
    block_len = 32

    def __init__(self,  array_data_iterable=None, array_fill=0):
        super(AuthChipBlockZone, self).__init__(array_data_iterable, array_fill)

    @abstractproperty
    def num_blocks(self):
        raise NotImplementedError 

    @property
    def array_size(self):
        return self.block_len * self.num_blocks

    def set_block(self, block, data):
        if not (0 <= block < self.num_blocks):
            raise AuthChipException('Invalid block index %d; expected 0 <= block index < %d' %
                    (block, self.num_blocks))
        data_array = bytearray(data)
        if len(data_array) != self.block_len:
            raise AuthChipException('Invalid data length %d; expected %d' %
                    (len(data_array), self.block_len))
        self._array[block * self.block_len : (block + 1) * self.block_len] = data_array

    def get_block(self, block):
        return self._array[block * self.block_len : (block + 1) * self.block_len]

    def get_blocks(self):
        return (self.get_block(i) for i in xrange(self.num_blocks))


class AuthChipConfigurationZone(AuthChipZone):
    array_size = 88
    config_lock_data_index = 86
    config_lock_config_index = 87

    config_offset_i2c_address = 16
    config_offset_set_rfu = 17
    config_offset_otp_mode = 18
    config_offset_selector_mode = 19

    config_setting_i2c_addr = 100

    config_byte_offset = 16

    slot_config_base_addr = 20

    def __init__(self, config_zone_data):
        super(AuthChipConfigurationZone, self).__init__(config_zone_data)

    def config_zone_locked(self):
        return self._array[self.config_lock_config_index] == 0

    def data_and_otp_zones_locked(self):
        return self._array[self.config_lock_data_index] == 0

    def serial_number(self):
        return self._array[0:4] + self._array[8:13]

    def set_slot_config(self, slot, slot_config):
        if len(slot_config) != 2:
            raise AuthChipException('Invalid slot_config length: got %d, expected 2' %
                    len(slot_config))
        slot_base = self.slot_config_base_addr + (2 * slot)
        self._array[slot_base:slot_base+2] = slot_config


class AuthChipDataZone(AuthChipBlockZone):
    num_blocks = 16

    def __init__(self, data_zone_data):
        super(AuthChipDataZone, self).__init__(data_zone_data)


class AuthChipOtpZone(AuthChipBlockZone):
    num_blocks = 2

    def __init__(self, otp_zone_data):
        super(AuthChipOtpZone, self).__init__(otp_zone_data)


class KeyData(object):
    def __init__(self, key_iterable):
        # For consistency we are storing all 'binary' data in bytearrays.
        self._keys = tuple(bytearray(key) for key in key_iterable)

        # Validate our key tuple.
        for i, key in enumerate(self._keys):
            if len(key) != AuthChipBlockZone.block_len:
                raise AuthChipException('key %d has length %d; expected %d' %
                        (i, len(key), AuthChipBlockZone.block_len))

    def key(self, index):
        return self._keys[index]

    def get_array(self):
        return reduce(bytearray.__add__, self._keys)

    def num_keys(self):
        return len(self._keys)


# In-file tests because the Cobs component is structured in a way that is
# not conducive to separarate-file tests. Test with:
# $ cd /path/to/cobs/Scripts
# $ python -m unittest atmel_lib
import unittest
class TestAtmelSupportLib(unittest.TestCase):
    def test_word_to_bytes(self):
        self.assertEqual(word_to_bytes(0xdeadbeef, False), (0xef, 0xbe, 0xad, 0xde))
        self.assertEqual(word_to_bytes(0xdeadbeef, True),  (0xde, 0xad, 0xbe, 0xef))

    def test_bytes_to_word(self):
        self.assertEqual(0xdeadbeef, bytes_to_word((0xef, 0xbe, 0xad, 0xde), 0, False))
        self.assertEqual(0xdeadbeef, bytes_to_word((0xde, 0xad, 0xbe, 0xef), 0, True))
