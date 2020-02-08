# This file is executed at startup. If you always enter the same command(s) at
# startup, you could put it here and save yourself a bit of trouble.
# 
# For example, to create a Device for the serial port COM42, and load a GE FPGA
# .icron file for it:
# print "Creating Device and loading .icron file for COM42..."
# hobbes.load(hobbes.addDevice("COM42"), "Z:\\designs\\goldenears\\working" + \
# "\\angusl\\p_goldenears_sw\\goldenears_fpga_bin\\goldenears_fpga.icron")
# 
# Also, you could create an instance of an iCommand-sending class:
# icmd42=COM42_ICMD()
# This allows you to execute icmd42.readMemory(42) instead of
# COM42_ICMD.readMemory(42), which you may find beneficial.
# 
# With Hobbes 0.2.1.0-r34 and onwards, you could now import antigravity.
# Assuming you have a common browser (e.g. Firefox) in its default location:
# import antigravity