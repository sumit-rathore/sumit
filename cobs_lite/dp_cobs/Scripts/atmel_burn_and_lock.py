from Scripts.atmel_lib import AuthChipAccessor, AuthChipBlockZone, AuthChipConfigurationZone,\
        AuthChipDataZone, AuthChipException, AuthChipOtpZone, KeyData
import icron_lib.icron_crc as icron_crc
import importlib
import os
import textwrap
import traceback

import clr
clr.AddReference('System.Drawing')
import System.Drawing as Drawing


def format_bytes(iterable_bytes, line_len=11):
    # A default line_len of 11 gives us four space-separated bytes per row.
    return textwrap.fill(' '.join(('%02x' % b) for b in iterable_bytes), line_len)


class UserLogger(object):
    def __init__(self, device):
        self._dev = device

    def log(self, message):
        self._dev.print_to_device_window(message, Drawing.Color.DarkGreen)


def atmel_burn_and_lock_main(dev):
    try:
        ACCZ = AuthChipConfigurationZone  # Alias for brevity

        # These are the script parameters which a user is most likely to want to change.
        num_secret_slots = AuthChipDataZone.num_blocks
        key_file_module_name = 'authentication_keys'
        cfg_settings = (  # Pairs of (address, value) for single-byte config settings
                (ACCZ.config_offset_i2c_address, ACCZ.config_setting_i2c_addr << 1),  # 1-bit offset
                (ACCZ.config_offset_set_rfu, 0x00),  # Reserved for future use
                (ACCZ.config_offset_otp_mode, 0xaa),  # Read-only mode
                (ACCZ.config_offset_selector_mode, 0x00))  # Read-only mode
        # Slot configuration values. Byte order matches the order on the auth chip.
        data_slot_configuration_storage = (0x0f, 0x00)
        data_slot_configuration_secret_key = (0x8f, 0x80)
        # OTP data to configure. Byte order matches the order on the auth chip.
        # TODO TMP: write two blocks of zeros for testing purposes to verify we can
        # write OTP data. This needs to change once we decide how we're going to use
        # the OTP zone.
        otp_data = bytearray(AuthChipOtpZone.num_blocks * AuthChipBlockZone.block_len)

        logger = UserLogger(dev)
        accessor = AuthChipAccessor(dev)

        try:
            # Try to access the key file. This file must be provided by the user.
            key_file_path = dev.programPath + '/' + key_file_module_name + '.py'
            authentication_keys = importlib.import_module(key_file_module_name) 
        except ImportError:
            raise AuthChipException('Expected to find a key file at %s' % key_file_path)

        key_data = KeyData(authentication_keys.keys)
        if key_data.num_keys() != num_secret_slots:
            raise AuthChipException('key_data contains %d keys; expected %d' % 
                    (len(self._keys), expected_num_keys))

        logger.log('Reading config zone ...')
        cfg_data = accessor.fetch_config_data()
        logger.log('Done')

        logger.log('Config zone %s' % ('locked' if cfg_data.config_zone_locked() else 'unlocked'))
        logger.log('Data and OTP zones %s' % ('locked' if cfg_data.data_and_otp_zones_locked()
            else 'unlocked'))
        logger.log('Config zone data:\n%s' % format_bytes(cfg_data.get_array()))

        if not cfg_data.config_zone_locked():
            # The config zone isn't locked, so we need to configure and lock it.
            # First, set up our in-memory config zone in preparation for writing it to the chip.
            for addr, value in cfg_settings:
                cfg_data.set_byte(addr, value)

            # Configure all slots for secret key storage.
            for slot in xrange(num_secret_slots):
                cfg_data.set_slot_config(slot, data_slot_configuration_secret_key)

            # Now we write the in-memory config zone to the chip. Write from the beginning of the
            # writeable part of the config zone up until the last word, which is written during
            # locking.
            logger.log('Writing config zone ...')
            for offset in xrange(ACCZ.config_byte_offset, ACCZ.array_size - 7, 4):
                accessor.write_config_word(offset, cfg_data.get_word(offset, True))
            logger.log('Config zone written')

            # Read back the new config data and log it. It should be the same as what we wrote.
            new_cfg_data = accessor.fetch_config_data()
            logger.log('New config zone data:\n%s' % format_bytes(new_cfg_data.get_array()))

            if new_cfg_data.get_array() != cfg_data.get_array():
                raise AuthChipException('Read-back config zone does not match what we wrote')

            # Compute the CRC16 of the config zone which is required for locking it, then lock it.
            logger.log('Locking config zone ...')
            accessor.lock_config_zone(cfg_data.crc16())
            logger.log('Config zone locked')

        if not cfg_data.data_and_otp_zones_locked():
            # The data and OTP zones are unlocked. Let's write our settings to them and lock them.
            logger.log('Writing data zone ...')
            data_zone = AuthChipDataZone(key_data.get_array())
            for slot, key in enumerate(data_zone.get_blocks()):
                accessor.write_data_slot(slot, key)
            logger.log('Data zone written')

            logger.log('Writing OTP zone ...')
            otp_zone = AuthChipOtpZone(otp_data)
            for block_idx, block_data in enumerate(otp_zone.get_blocks()):
                accessor.write_otp_block(block_idx, block_data)
            logger.log('OTP zone written')

            logger.log('Data zone data:\n%s' % format_bytes(data_zone.get_array()))
            logger.log('OTP zone data:\n%s' % format_bytes(otp_zone.get_array()))

            logger.log('Locking data and OTP zones ...')
            data_and_otp_zone_crc = icron_crc.crc16(data_zone.get_array() + otp_zone.get_array())
            accessor.lock_data_and_otp_zones(data_and_otp_zone_crc)
            logger.log('Data and OTP zones locked')

    except:
        traceback.print_exc()


# Execution starts here.
atmel_burn_and_lock_main(currentDevice)
