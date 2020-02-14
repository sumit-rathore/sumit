The Atmel configuration test harness
====================================

Well not really a test harness, but a configuration program

This program does the following

1) basic GE initialization to get I2C access to the Atmel chip
2) reads the entire configuration of the Atmel chip into RAM
3) provides icmds to modify the configuration in RAM
4) provides icmds to write data zone slots in RAM
5) provides icmd to burn all RAM configuration & data zone data to the Atmel chip & lock down the chip


# Python Script to set up default settings
# ----------------------------------------
# Configure Atmel chip: Just replace COM18 with whatever com port is being used

COM18_TEST_HARNESS.writeCfgByte(16, 100 << 1) # i2c address
COM18_TEST_HARNESS.writeCfgByte(17, 0x00) # RFU
COM18_TEST_HARNESS.writeCfgByte(18, 0xAA) # OTPmode
COM18_TEST_HARNESS.writeCfgByte(19, 0x00) # SelectorMode

# Slot config, bytes 21 to 51
COM18_TEST_HARNESS.writeCfgSlotConfig(0, 0x808F) # Secret key
COM18_TEST_HARNESS.writeCfgSlotConfig(1, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(2, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(3, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(4, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(5, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(6, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(7, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(8, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(9, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(10, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(11, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(12, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(13, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(14, 0x000F)
COM18_TEST_HARNESS.writeCfgSlotConfig(15, 0x000F)

# UseFlag & UpdateCount, bytes 52 to 67.  Pre-initialized

# LastKeyUse, bytes 68 to 83.  Pre-initialized

# UserExtra/Selector, bytes 84 & 86.  Don't support (yet)

#bytes 86 & 87 are the lock bytes

COM18_TEST_HARNESS.burnAndLock()

